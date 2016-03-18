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

RX5808 rx5808(rssiB, SPI_CSB);

//uint8_t curr_channel = 0;
//uint32_t curr_freq;

//uint32_t last_irq;
//uint8_t changing_freq, changing_mode;

//default values used for calibration
uint16_t rssi_min = 1024;
uint16_t rssi_max = 0;

char buf[30];
uint8_t last_post_switch;
char timer;

uint8_t do_nothing;

uint16_t last_used_freq, last_used_band, last_used_freq_id ;

void setup() {
  // Serial.begin(9600);

  //video out init
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);

  //initialize SPI
#ifdef USE_NATIVE_SPI
  SPI.begin();
  SPI.setBitOrder(LSBFIRST);
#else
#define spiDataPin 11
#define spiClockPin 13
  pinMode(spiDataPin, OUTPUT);
  pinMode(spiClockPin, OUTPUT);
#endif

  rx5808.init();
  //rx5808.calibration();

  //tv init
  TV.begin(PAL, 120, 96);
  TV.select_font(font6x8);
  TV.clear_screen();
  TV.print(0, 0, "FPVR FATSHARK\nRX MODULE - V0.01\nby MikyM0use");

  delay(5000);

  last_post_switch = -1; //init menu position

  do_nothing = 0;

  last_used_band = EEPROM.read(EEPROM_ADDR_LAST_BAND_ID); //channel name
  last_used_freq_id = EEPROM.read(EEPROM_ADDR_LAST_FREQ_ID);
  last_used_freq = pgm_read_word_near(channelFreqTable + (8 * last_used_band) + last_used_freq_id); //freq
}

#define MENU_Y_SIZE 12

inline uint8_t readSwitch() {
  return 0x7 - ((digitalRead(CH3) << 2) | (digitalRead(CH2) << 1) | digitalRead(CH1));
}

void autoscan() {
  //  if (changing_freq || changing_mode) {
  //changing_freq = changing_mode = 0;
  rx5808.scan(1, BIN_H);
  uint32_t curr_freq = pgm_read_word_near(channelFreqTable + rx5808.getMaxPos());
  use_freq(curr_freq);
  // rx5808.setFreq(curr_freq);
  // }

  //digitalWrite(5, LOW);
  //do_nothing = 1;
}

void use_freq(uint32_t freq) {
  rx5808.setFreq(freq);

  digitalWrite(5, LOW);
  do_nothing = 1;
}

void set_and_wait(uint8_t band, uint8_t menu_pos) {

  use_freq(pgm_read_word_near(channelFreqTable + (8 * band) + menu_pos)); //set the selected freq

  //save band and freq as "last used"
  EEPROM.write(EEPROM_ADDR_LAST_FREQ_ID, menu_pos); //freq id
  EEPROM.write(EEPROM_ADDR_LAST_BAND_ID, band); //channel name

  //change channel during normal usage
  while (1) {
    menu_pos = readSwitch();

    if (last_post_switch != menu_pos) { //something changed by user
      use_freq(pgm_read_word_near(channelFreqTable + (8 * band) + menu_pos)); //set the selected freq
      EEPROM.write(EEPROM_ADDR_LAST_FREQ_ID, menu_pos);
    }
    last_post_switch = menu_pos;
  }

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
    delay(2000);
  }

  set_and_wait(band, menu_pos);
}

void scanner_mode() {
  timer = 9;
  while (timer-- > 0) {
    rx5808.scan(1, BIN_H);
    TV.clear_screen();
    TV.draw_rect(1, 1, 100, 94,  WHITE);
    TV.select_font(font4x6);
    TV.print(5, 87, "5645");
    TV.print(45, 87, "5800");
    TV.print(85, 87, "5945");

    TV.select_font(font6x8);
    for (int i = CHANNEL_MIN; i < CHANNEL_MAX; i++) {
      uint8_t channelIndex = pgm_read_byte_near(channelList + i); //retrive the value based on the freq order
      TV.draw_rect(10 + 2 * i, 10 + BIN_H - rx5808.getRssi(channelIndex) , 2, rx5808.getRssi(channelIndex), WHITE, WHITE);
    }

    TV.println(92, 3, timer, DEC);
    delay(500);
  }
  timer = 9;
}

void loop(void) {
  if (do_nothing)
    return;

  uint8_t menu_pos = readSwitch();
  uint8_t i;

  if (last_post_switch != menu_pos) {
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
  // simple menu
  TV.select_font(font6x8);
  TV.draw_rect(1, 1, 100, 94,  WHITE);

  //header and countdown
  // TV.printPGM(10, 3, PSTR("SELECT BAND"));
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

  delay(2000);
}
