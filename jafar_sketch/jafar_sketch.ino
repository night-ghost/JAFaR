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
float timer;
uint16_t last_used_freq, last_used_band, last_used_freq_id;
uint8_t menu_pos;

#ifdef USE_OLED

#include "U8glib.h"

#ifdef USE_I2C_OLED
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST); // Fast I2C / TWI
#else
U8GLIB_SSD1306_128X64 u8g(8, A1, A4, 11 , 13); //CLK, MOSI, CS, DC, RESET
#endif

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

#ifdef STANDALONE
  pinMode(CH1, INPUT_PULLUP); //UP
  pinMode(CH2, INPUT_PULLUP); //ENTER
  pinMode(CH3, INPUT_PULLUP); //DOWN
#endif
  //menu_pos = 5;

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
#else
  last_post_switch = -1; //init menu position
#endif
  do_nothing = 0;

  in_mainmenu = 1;
  timer = TIMER_INIT_VALUE;

  last_used_band = EEPROM.read(EEPROM_ADDR_LAST_BAND_ID); //channel name
  last_used_freq_id = EEPROM.read(EEPROM_ADDR_LAST_FREQ_ID);
  last_used_freq = pgm_read_word_near(channelFreqTable + (8 * last_used_band) + last_used_freq_id); //freq
}

void autoscan() {
  int reinit = 1; //only the first time, re-init the oled
  last_post_switch = -1; //force first draw
  timer = TIMER_INIT_VALUE;
  rx5808.scan(1, BIN_H); //refresh RSSI
  rx5808.compute_top8();

  while (timer) {
    menu_pos = readSwitch();

    if (menu_pos != last_post_switch)  //user moving
      timer = TIMER_INIT_VALUE;

#ifdef USE_OLED
    oled_autoscan(reinit);
    reinit = 0;
#else
    osd_autoscan();
#endif

    last_post_switch = menu_pos;

#ifdef USE_OLED  //debounce and peace
    delay(LOOPTIME);
#else
    TV.delay(LOOPTIME);
#endif //OLED 
    timer -= (LOOPTIME / 1000.0);
  }

  set_and_wait((rx5808.getfrom_top8(menu_pos) & 0b11111000) / 8, rx5808.getfrom_top8(menu_pos) & 0b111);
}

#define RX_A 1
#define RX_B 0

void scanner_mode() {

#ifdef USE_OLED
  oled_scanner();
#else
  osd_scanner();
#endif

  timer = TIMER_INIT_VALUE;
}

void loop(void) {
  uint8_t i;

  if (do_nothing)
    return;

  menu_pos = readSwitch();

  //force always the first menu item (last freq used)
#ifdef FORCE_FIRST_MENU_ITEM
  if (flag_first_pos == menu_pos)
    menu_pos = 0;
#endif


  if (last_post_switch != menu_pos) {
    flag_first_pos = 0;
    timer = TIMER_INIT_VALUE;
  }
  else {
#ifdef STANDALONE
    if (timer > 0)
      return; //force no refresh of OSD
#else
    timer -= (LOOPTIME / 1000.0);
#endif
  }

  last_post_switch = menu_pos;

  if (timer <= 0) { //end of time for selection

    if (in_mainmenu) { //switch from menu to submenu (band -> frequency)
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
          in_mainmenu = 0;
          menu_band = menu_pos - 1;
          timer = TIMER_INIT_VALUE;

#ifdef USE_OLED  //debounce and peace
          delay(200);
#else
          TV.delay(200);
#endif //OLED 
          break;
      } //switch
    } else { //if in submenu

#ifdef USE_DIVERSITY
#ifdef USE_OLED
      oled_waitmessage(); //please wait message
      delay(800);
#else
      osd_waitmessage() ;
      TV.delay(100);
#endif //OLED 
#endif //DIVERSITY

      //after selection of band AND freq by the user
      set_and_wait(menu_band, menu_pos);
      timer = 0;
    } //else
  } //timer

  //time still running
  if (in_mainmenu) { //on main menu
#ifdef USE_OLED
    oled_mainmenu(menu_pos);
    delay(LOOPTIME);
#else
    osd_mainmenu(menu_pos) ;
    TV.delay(LOOPTIME);
#endif
  } else { //on submenu
#ifdef USE_OLED
    oled_submenu(menu_pos,  menu_band);
    delay(LOOPTIME);
#else
    osd_submenu(menu_pos,  menu_band);
    TV.delay(LOOPTIME);
#endif
  }
}
