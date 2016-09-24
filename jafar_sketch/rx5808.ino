/*
This file is part of Fatshark© goggle rx module project (JAFaR).

    JAFaR is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    JAFaR is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nome-Programma.  If not, see <http://www.gnu.org/licenses/>.

    Copyright © 2016 Michele Martinelli
  */

#include <Wire.h>
#include <EEPROM.h>
#include "rx5808.h"
#define RSSI_THRESH (scanVec[getMaxPos()]-5) //strong channels are near to the global max

RX5808::RX5808(uint16_t RSSIpin, uint16_t CSpin) {
  _rssiPin = RSSIpin;
  _csPin = CSpin;
  _stop_scan = 0;
}

uint8_t RX5808::getfrom_top8(uint8_t index) {
  return scanVecTop8[index];
}

void RX5808::compute_top8(void) {
  uint16_t scanVecTmp[CHANNEL_MAX];
  uint8_t _chan;

  memcpy(scanVecTmp, scanVec, sizeof(uint16_t)*CHANNEL_MAX);

  for (int8_t i = 7; i >= 0; i--) {
    uint16_t maxVal = 0;
    uint8_t maxPos = 0;
    for (_chan = CHANNEL_MIN; _chan < CHANNEL_MAX; _chan++) {
      if (maxVal < scanVecTmp[_chan]) { //new max
        maxPos = _chan;
        maxVal = scanVecTmp[_chan];
      }
    }

    //maxPos is the maximum
    scanVecTop8[i] = maxPos;
    scanVecTmp[maxPos] = 0;
  }
}

uint16_t RX5808::getRssi(uint16_t channel) {
  return scanVec[channel];
}

//stop scan
void RX5808::abortScan(void) {
  _stop_scan = 1;
}

//get the rssi value of a certain channel of a band and map it to 1...norm
uint16_t RX5808::getVal(uint8_t band, uint8_t channel, uint8_t norm) {
  return map(scanVec[8 * band + channel], 1, BIN_H, 1, norm);
}

//get the rssi value of a certain channel and map it to 1...norm
uint16_t RX5808::getVal(uint8_t pos, uint8_t norm) {
  return map(scanVec[pos], 1, BIN_H, 1, norm);
}

//get the maximum rssi value for a certain band and map it to 1...norm
uint16_t RX5808::getMaxValBand(uint8_t band, uint8_t norm) {
  int16_t _chan;
  uint16_t maxVal = 0;
  for (_chan = 8 * band; _chan < 8 * band + 8; _chan++) {
    if (maxVal < scanVec[_chan]) { //new max
      maxVal = scanVec[_chan];
    }
  }
  return map(maxVal, 1, BIN_H, 1, norm);
}

//get the channel with max rssi value for a certain band
uint16_t RX5808::getMaxPosBand(uint8_t band) {
  int16_t _chan;
  uint16_t maxVal = 0, maxPos = 8 * band;
  for (_chan = 8 * band; _chan < 8 * band + 8; _chan++) {
    if (maxVal < scanVec[_chan]) { //new max
      maxPos = _chan;
      maxVal = scanVec[_chan];
    }
  }
  return maxPos;
}

//get the minimum rssi value for a certain band
uint16_t RX5808::getMinPosBand(uint8_t band) {
  int16_t _chan;
  uint16_t minVal = 1000, minPos = 8 * band;
  for (_chan = 8 * band; _chan < 8 * band + 8; _chan++) {
    if (minVal > scanVec[_chan]) { //new max
      minPos = _chan;
      minVal = scanVec[_chan];
    }
  }
  return minPos;
}

//get global max
uint16_t RX5808::getMaxPos() {
  uint8_t _chan;
  uint16_t maxVal = 0, maxPos = 0;
  for (_chan = CHANNEL_MIN; _chan < CHANNEL_MAX; _chan++) {
    if (maxVal < scanVec[_chan]) { //new max
      maxPos = _chan;
      maxVal = scanVec[_chan];
    }
  }
  return maxPos;
}

//get global min
uint16_t RX5808::getMinPos() {
  uint8_t _chan;
  uint16_t minVal = 1000, minPos = 0;
  for (_chan = CHANNEL_MIN; _chan < CHANNEL_MAX; _chan++) {
    if (minVal > scanVec[_chan]) { //new max
      minPos = _chan;
      minVal = scanVec[_chan];
    }
  }
  return minPos;
}

uint16_t RX5808::getCurrentRSSI() {
  return _readRSSI();
}

void RX5808::init() {
  pinMode (_csPin, OUTPUT);
  pinMode (_rssiPin, INPUT);

  rssi_min = ((EEPROM.read(EEPROM_ADR_RSSI_MIN_H) << 8) | (EEPROM.read(EEPROM_ADR_RSSI_MIN_L)));
  rssi_max = ((EEPROM.read(EEPROM_ADR_RSSI_MAX_H) << 8) | (EEPROM.read(EEPROM_ADR_RSSI_MAX_L)));

  /*
    digitalWrite(_csPin, LOW);
    SPI.transfer(0x10);
    SPI.transfer(0x01);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    digitalWrite(_csPin, HIGH);
  */
  if (abs(rssi_max - rssi_min > 300) || abs(rssi_max - rssi_min < 50))
    calibration();

  scan(1, BIN_H);
}

//do a complete scan and normalize all the values
void RX5808::scan(uint16_t norm_min, uint16_t norm_max) {

  for (uint16_t _chan = CHANNEL_MIN; _chan < CHANNEL_MAX; _chan++) {
    if (_stop_scan) {
      _stop_scan = 0;
      return;
    }

    uint32_t freq = pgm_read_word_near(channelFreqTable + _chan);
    setFreq(freq);
    _wait_rssi();

    uint16_t rssi =  _readRSSI();
#ifdef RXDEBUG
    Serial.print(_chan, DEC);
    Serial.print("\t");
    Serial.print(freq, DEC);
    Serial.print("\t");
    Serial.println(rssi, DEC);
    delay(500);
#endif

    rssi = constrain(rssi, rssi_min, rssi_max);
    rssi = map(rssi, rssi_min, rssi_max, norm_min, norm_max);   // scale from 1..100%
    scanVec[_chan] = rssi;
  }
}

void RX5808::updateRssi(uint16_t norm_min, uint16_t norm_max, uint16_t channel) {
  uint16_t rssi =  _readRSSI();
  rssi = constrain(rssi, rssi_min, rssi_max);
  rssi = map(rssi, rssi_min, rssi_max, norm_min, norm_max);   // scale from 1..100%
  scanVec[channel] = rssi;
}

//same as scan, but raw values, used for calibration
void RX5808::_calibrationScan() {
  for (uint16_t _chan = CHANNEL_MIN; _chan < CHANNEL_MAX; _chan++) {

    uint32_t freq = pgm_read_word_near(channelFreqTable + _chan);
    setFreq(freq);
    _wait_rssi();
    scanVec[_chan] = _readRSSI();
  }
}

void RX5808::_wait_rssi() {
  // 30ms will to do a 32 channels scan in 1 second
#define MIN_TUNE_TIME 30
  delay(MIN_TUNE_TIME);
}

//simple avg of 4 value
uint16_t RX5808::_readRSSI() {
  volatile uint32_t sum = 0;
  delay(9);
  sum = analogRead(_rssiPin);
  sum += analogRead(_rssiPin);
  delay(1);
  sum += analogRead(_rssiPin);
  sum += analogRead(_rssiPin);
  return sum / 4.0;
}

//compute the min and max RSSI value and store the values in EEPROM
void RX5808::calibration() {
  int i = 0, j = 0;
  uint16_t  rssi_setup_min = 1024, minValue = 1024;
  uint16_t  rssi_setup_max = 0, maxValue = 0;

  for (j = 0; j < 5; j++) {
    _calibrationScan();

    for (i = CHANNEL_MIN; i < CHANNEL_MAX; i++) {
      uint16_t rssi = scanVec[i];

      minValue = min(minValue, rssi);
      maxValue = max(maxValue, rssi);
    }

    rssi_setup_min = min(rssi_setup_min, minValue); //?minValue:rssi_setup_min;
    rssi_setup_max = max(rssi_setup_max, maxValue); //?maxValue:rssi_setup_max;
  }

  // save 16 bit
  EEPROM.write(EEPROM_ADR_RSSI_MIN_L, (rssi_setup_min & 0xff));
  EEPROM.write(EEPROM_ADR_RSSI_MIN_H, (rssi_setup_min >> 8));
  // save 16 bit
  EEPROM.write(EEPROM_ADR_RSSI_MAX_L, (rssi_setup_max & 0xff));
  EEPROM.write(EEPROM_ADR_RSSI_MAX_H, (rssi_setup_max >> 8));

  rssi_min = ((EEPROM.read(EEPROM_ADR_RSSI_MIN_H) << 8) | (EEPROM.read(EEPROM_ADR_RSSI_MIN_L)));
  rssi_max = ((EEPROM.read(EEPROM_ADR_RSSI_MAX_H) << 8) | (EEPROM.read(EEPROM_ADR_RSSI_MAX_L)));


  delay(3000);

  return;
}

void RX5808::setFreq(uint32_t freq) {
  uint8_t i;
  uint16_t channelData;

  uint32_t _if = (freq - 479);

  uint32_t N = floor(_if / 64);
  uint32_t A = floor((_if / 2) % 32);
  channelData = (N << 7) | (A & 0x7F);

#ifdef DEBUG
  Serial.print("N: ");
  Serial.println(N, DEC);

  Serial.print("A: ");
  Serial.println(A, DEC);

  Serial.print("setting ");
  Serial.println(channelData, HEX);
#endif

  serialEnable(HIGH);
  delayMicroseconds(1);
  serialEnable(LOW);

  //REGISTER 1 - selection
  // bit bash out 25 bits of data
  // Order: A0-3, !R/W, D0-D19
  // A0=0, A1=0, A2=0, A3=1, RW=0, D0-19=0
  uint16_t reg1 = 0x10;
  for (i = 0; i < 25; i++)
    serialSendBit((reg1 >> i) & 1);

  // Clock the data in
  serialEnable(HIGH);
  delayMicroseconds(1);
  serialEnable(LOW);

  // Second is the channel data from the lookup table
  // 20 bytes of register data are sent, but the MSB 4 bits are zeros
  // register address = 0x1, write, data0-15=channelData data15-19=0x0
  serialEnable(HIGH);
  serialEnable(LOW);

  // Register 0x1
  serialSendBit(HIGH);
  serialSendBit(LOW);
  serialSendBit(LOW);
  serialSendBit(LOW);

  // Write to register
  serialSendBit(HIGH);

  // D0-D15
  //   note: loop runs backwards as more efficent on AVR
  for (i = 16; i > 0; i--) {
    serialSendBit(channelData & 0x1);
    // Shift bits along to check the next one
    channelData >>= 1;
  }

  // Remaining D16-D19
  for (i = 4; i > 0; i--)
    serialSendBit(LOW);

  // Finished clocking data in
  serialEnable(HIGH);
  delayMicroseconds(1);

  //  digitalWrite(_csPin, LOW);
  digitalWrite(spiClockPin, LOW);
  digitalWrite(spiDataPin, LOW);
}

void RX5808::serialSendBit(const uint8_t _b) {
  digitalWrite(spiClockPin, LOW);
  delayMicroseconds(1);

  digitalWrite(spiDataPin, _b);
  delayMicroseconds(1);
  digitalWrite(spiClockPin, HIGH);
  delayMicroseconds(1);

  digitalWrite(spiClockPin, LOW);
  delayMicroseconds(1);
}

void RX5808::serialEnable(const uint8_t _lev) {
  delayMicroseconds(1);
  digitalWrite(_csPin, _lev);
  delayMicroseconds(1);
}
