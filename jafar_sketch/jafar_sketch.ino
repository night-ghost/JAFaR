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

#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include "rx5808.h"
#include "const.h"

#include <TVout.h>
#include <fontALL.h>

TVout TV;

RX5808 rx5808(rssiA, SPI_CSA);

char buf[30];
uint8_t last_post_switch;
char timer;

uint8_t do_nothing, flag_first_pos;

uint16_t last_used_freq, last_used_band, last_used_freq_id;

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif

  //video out init
  pinMode(SW_CTRL1, OUTPUT);
  pinMode(SW_CTRL2, OUTPUT);
  SELECT_OSD;

  //initialize SPI
  pinMode(spiDataPin, OUTPUT);
  pinMode(spiClockPin, OUTPUT);

  //RX module init
  rx5808.init();
  //rx5808.calibration();

  //tv init
  TV.begin(PAL, D_COL, D_ROW);
  TV.select_font(font6x8);

  //splash screen
  TV.clear_screen();
  TV.print(0, 0, "FPVR FATSHARK\nRX MODULE - V0.02s\nby MikyM0use\n\n");
  TV.print(0, 50, "RSSI MIN");
  TV.println(60, 50, rssi_min, DEC); //RSSI
  TV.print(0, 60, "RSSI MAX");
  TV.println(60, 60, rssi_max, DEC); //RSSI

  TV.delay(3000);
  flag_first_pos = 0;
#ifdef FORCE_FIRST_MENU_ITEM
  flag_first_pos = readSwitch();
  last_post_switch = 0;
  timer = 9;
#else
  last_post_switch = -1; //init menu position
#endif
  do_nothing = 0;

  last_used_band = EEPROM.read(EEPROM_ADDR_LAST_BAND_ID); //channel name
  last_used_freq_id = EEPROM.read(EEPROM_ADDR_LAST_FREQ_ID);
  last_used_freq = pgm_read_word_near(channelFreqTable + (8 * last_used_band) + last_used_freq_id); //freq
}

inline uint8_t readSwitch() {
  return 0x7 - ((digitalRead(CH3) << 2) | (digitalRead(CH2) << 1) | digitalRead(CH1));
}

void autoscan() { //TODO BETA VERSION! diversity not supported
  // RX5808 rx5808B(rssiB, SPI_CSB);
  // rx5808B.init();

  rx5808.scan(1, BIN_H);
  uint32_t curr_freq = pgm_read_word_near(channelFreqTable + rx5808.getMaxPos());
  use_freq(curr_freq, rx5808);

  SELECT_A;
}

#ifdef USE_DIVERSITY
void use_freq_diversity(uint32_t freq, RX5808 rx5808, RX5808 rx5808B) {
  rx5808.setFreq(freq);
  rx5808B.setFreq(freq);

  do_nothing = 1;
}
#endif

void use_freq(uint32_t freq, RX5808 rx5808) {
  rx5808.setFreq(freq);

  do_nothing = 1;
}

#define RX_A 1
#define RX_B 0

void set_and_wait(uint8_t band, uint8_t menu_pos) {
  unsigned rssi_b, rssi_a;
  u8 current_rx;



#ifdef USE_DIVERSITY
  //init of the second module
  RX5808 rx5808B(rssiB, SPI_CSB);

  rx5808B.init();
  use_freq_diversity(pgm_read_word_near(channelFreqTable + (8 * band) + menu_pos), rx5808, rx5808B); //set the selected freq
  SELECT_B;

  current_rx = RX_B;
#else
  use_freq(pgm_read_word_near(channelFreqTable + (8 * band) + menu_pos), rx5808); //set the selected freq
  SELECT_A;
  current_rx = RX_A;
#endif

  //clear memory for log
#ifdef ENABLE_RSSILOG
  uint8_t sample = 0;
  long g_log_offset = 0;
  for (g_log_offset = 0 ; g_log_offset < EEPROM.length() / 2 ; g_log_offset++) {
    EEPROM.write(EEPROM_ADDR_START_LOG + g_log_offset, 0);
  }
  g_log_offset = 0;
#endif

  //save band and freq as "last used"
  EEPROM.write(EEPROM_ADDR_LAST_FREQ_ID, menu_pos); //freq id
  EEPROM.write(EEPROM_ADDR_LAST_BAND_ID, band); //channel name



  //MAIN LOOP - change channel and log
  while (1) {
    rssi_a = rx5808.getCurrentRSSI();
#ifdef USE_DIVERSITY
    rssi_b = rx5808B.getCurrentRSSI();
#endif

#ifdef ENABLE_RSSILOG
    //every loop cycle requires ~100ms
    //total memory available is 492B (512-20) and every sample is 2B -> 246 sample in total
    //if we take 2 sample per seconds we have 123 seconds of recording (~2 minutes)
    if (++sample >= 2) {
      sample = 0;

      //FORMAT IS XXXXXXXRYYYYYYYY (i.e. 7bit for RSSI_B - 1bit for RX used - 8bit for RSSI_A
      if (g_log_offset < EEPROM.length() / 2)
        EEPROM.put(EEPROM_ADDR_START_LOG + g_log_offset, ((uint16_t)(((rssi_b & 0xFE) | (current_rx & 0x1)) & 0xFF) << 8) | (rssi_a & 0xFF));

      g_log_offset += sizeof(uint16_t);
    }
#endif

#ifdef DEBUG
    Serial.print("A: ");
    Serial.print(rssi_a, DEC);

    Serial.print("\tB: ");
    Serial.print(rssi_b, DEC);

    Serial.print("\twe are using: ");
    if (current_rx == RX_A) {
      Serial.print("\tA");
      Serial.print("\twe change at: ");
      Serial.println(rssi_a + RX_HYST, DEC);
    } else {
      Serial.print("\tB");
      Serial.print("\twe change at: ");
      Serial.println(rssi_b + RX_HYST, DEC);
    }
    TV.delay(500);
#endif

#ifdef USE_DIVERSITY
    if (current_rx == RX_B && rssi_a > rssi_b + RX_HYST) {
      SELECT_A;
      current_rx = RX_A;
    }

    if (current_rx == RX_A && rssi_b > rssi_a + RX_HYST) {
      SELECT_B;
      current_rx = RX_B;
    }
#endif

    menu_pos = readSwitch();

    if (last_post_switch != menu_pos) { //something changed by user
#ifdef USE_DIVERSITY
      use_freq_diversity(pgm_read_word_near(channelFreqTable + (8 * band) + menu_pos), rx5808, rx5808B); //set the selected freq
#else
      use_freq(pgm_read_word_near(channelFreqTable + (8 * band) + menu_pos), rx5808); //set the selected freq
#endif
      EEPROM.write(EEPROM_ADDR_LAST_FREQ_ID, menu_pos);
    }
    last_post_switch = menu_pos;

  } //end of loop

}

void submenu(uint8_t pos) {
  timer = 9;
  uint8_t i;
  last_post_switch = -1; //force start
  uint8_t band = pos - 1;
  uint8_t menu_pos;

  while (timer-- > 0) {
    menu_pos = readSwitch();

    if (last_post_switch != menu_pos) { //something changed by user
      TV.clear_screen();
      TV.draw_rect(1, 1, 100, 94,  WHITE); //draw frame

      //show the channels list and the % RSSI
      for (i = 0; i < 8; i++) {
        TV.println(10, 3 + i * MENU_Y_SIZE, pgm_read_word_near(channelFreqTable + (8 * band) + i), DEC); //channel name

        TV.println(60, 3 + i * MENU_Y_SIZE, rx5808.getVal(band, i, 100), DEC); //RSSI
        TV.printPGM(78, 3 + i * MENU_Y_SIZE, PSTR("%")); //percentage symbol
      }

      timer = 9;
      last_post_switch = menu_pos;
      TV.draw_rect(9, 2 + menu_pos * MENU_Y_SIZE, 90, 7,  WHITE, INVERT); //current selection
    }

    TV.println(92, 3, timer, DEC);
    TV.delay(1000);
  }

  TV.clear_screen();
  TV.printPGM(0, 50, PSTR("SETTING\nFREQUENCY..."));
  set_and_wait(band, menu_pos);

  //force OSD disable
  TV.delay(1000);
  TV.clear_screen();
}

void scanner_mode() {
  timer = 9;
  while (timer-- > 0) {
    rx5808.scan(1, BIN_H);
    TV.clear_screen();
    TV.draw_rect(1, 1, 100, 94,  WHITE);
    TV.select_font(font4x6);
    TV.printPGM(5, 87, PSTR("5645"));
    TV.printPGM(45, 87, PSTR("5800"));
    TV.printPGM(85, 87, PSTR("5945"));

    TV.select_font(font6x8);
    for (int i = CHANNEL_MIN; i < CHANNEL_MAX; i++) {
      uint8_t channelIndex = pgm_read_byte_near(channelList + i); //retrive the value based on the freq order
      TV.draw_rect(10 + 2 * i, 10 + BIN_H - rx5808.getRssi(channelIndex) , 2, rx5808.getRssi(channelIndex), WHITE, WHITE);
    }

    TV.println(92, 3, timer, DEC);
    TV.delay(500);
  }
  timer = 9;
}

void loop(void) {
  if (do_nothing)
    return;

  uint8_t menu_pos = readSwitch();

#ifdef FORCE_FIRST_MENU_ITEM
  if (flag_first_pos == menu_pos)
    menu_pos = 0;
#endif

  uint8_t i;

  if (last_post_switch != menu_pos) {
    flag_first_pos = 0;
    timer = 9;
  }
  else timer--;

  last_post_switch = menu_pos;

  if (timer <= 0) { //end of time for selection
    switch (menu_pos) {
      case 0: //LAST USED
        set_and_wait(last_used_band, last_used_freq_id);
        break;
      case 6: //SCANNER
        scanner_mode();
        break;
      case 7: //AUTOSCAN
        autoscan();
        break;
      default:
        submenu(menu_pos);
        break;
    }
  }

  //MAIN MENU
  TV.clear_screen();
  TV.select_font(font6x8);
  TV.draw_rect(1, 1, 100, 94,  WHITE);

  //header and countdown
  TV.println(92, 3, timer, DEC);

  //last used band,freq
  TV.printPGM(10, 3 , PSTR("LAST:"));
  TV.println(45, 3 , pgm_read_byte_near(channelNames + (8 * last_used_band) + last_used_freq_id), HEX);
  TV.println(60, 3 , last_used_freq, DEC);

  //entire menu
  TV.printPGM(10, 3 + 1 * MENU_Y_SIZE, PSTR("BAND A"));
  TV.printPGM(10, 3 + 2 * MENU_Y_SIZE, PSTR("BAND B"));
  TV.printPGM(10, 3 + 3 * MENU_Y_SIZE, PSTR("BAND E"));
  TV.printPGM(10, 3 + 4 * MENU_Y_SIZE, PSTR("FATSHARK"));
  TV.printPGM(10, 3 + 5 * MENU_Y_SIZE, PSTR("RACEBAND"));
  TV.printPGM(10, 3 + 6 * MENU_Y_SIZE, PSTR("SCANNER"));
  TV.printPGM(10, 3 + 7 * MENU_Y_SIZE, PSTR("AUTOSCAN"));

  for (i = 0; i < 5; i++) {
    TV.println(65, 3 + (1 + i) * MENU_Y_SIZE, rx5808.getMaxValBand(i, 100), DEC);
    TV.printPGM(85, 3 + (1 + i) * MENU_Y_SIZE, PSTR("%"));
  }

  TV.draw_rect(9, 2 + menu_pos * MENU_Y_SIZE, 90, 7,  WHITE, INVERT); //current selection

  TV.delay(1000);
}
