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

RX5808 rx5808(rssiA, SPI_CSA);

uint8_t last_post_switch, do_nothing, flag_first_pos,  in_mainmenu, menu_band;
int8_t timer;
uint16_t last_used_freq, last_used_band, last_used_freq_id;

#ifdef USE_OLED

#include "U8glib.h"

U8GLIB_SSD1306_128X64 u8g(8, A1, A4, 11 , 13); //CLK, MOSI, CS, DC, RESET
char j_buf[80];

#else //USE OSD

#include <TVout.h>
#include <fontALL.h>

TVout TV;

#endif //USE OSD

//////********* SETUP ************////////////////////
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

#ifdef USE_OLED
  oled_init();
#else
  osd_init();
#endif

  flag_first_pos = 0;
#ifdef FORCE_FIRST_MENU_ITEM
  flag_first_pos = readSwitch();
  last_post_switch = 0;
  timer = 9;
#else
  last_post_switch = -1; //init menu position
#endif
  do_nothing = 0;

  in_mainmenu = 1;

  last_used_band = EEPROM.read(EEPROM_ADDR_LAST_BAND_ID); //channel name
  last_used_freq_id = EEPROM.read(EEPROM_ADDR_LAST_FREQ_ID);
  last_used_freq = pgm_read_word_near(channelFreqTable + (8 * last_used_band) + last_used_freq_id); //freq
}

void autoscan() { //TODO BETA VERSION! diversity not supported
  // RX5808 rx5808B(rssiB, SPI_CSB);
  // rx5808B.init();

  rx5808.scan(1, BIN_H);
  uint32_t curr_freq = pgm_read_word_near(channelFreqTable + rx5808.getMaxPos());
  use_freq(curr_freq, rx5808);

  SELECT_A;
}

#define RX_A 1
#define RX_B 0

void scanner_mode() {

#ifdef USE_OLED
#else
  osd_scanner();
#endif

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
    timer = TIMER_INIT_VALUE;
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
        if (in_mainmenu) {
          in_mainmenu = 0;
          menu_band = menu_pos - 1;
          timer = TIMER_INIT_VALUE;
        } else { //after selection of band AND freq by the user

          //please wait message
#ifdef USE_OLED
          oled_waitmessage();
          delay(800);
#else
          osd_waitmessage() ;
          TV.delay(800);
#endif
          set_and_wait(menu_band, menu_pos);
          timer = 0;
        }
        break;
    }
  }

  if (in_mainmenu) { //on main menu
#ifdef USE_OLED
    oled_mainmenu(menu_pos);
    delay(1000);
#else
    osd_mainmenu(menu_pos) ;
    TV.delay(1000);
#endif
  } else { //on submenu
#ifdef USE_OLED
    oled_submenu(menu_pos,  menu_band);
    delay(1000);
#else
    osd_submenu(menu_pos,  menu_band);
    TV.delay(1000);
#endif
  }
}
