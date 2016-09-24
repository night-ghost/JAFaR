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
#ifdef USE_DIVERSITY
RX5808 rx5808B(rssiB, SPI_CSB);
#endif

volatile int counter;
volatile bool buttonPressed;
volatile bool timerExpired;

uint8_t flag_first_pos, menu_band, current_rx;
uint8_t last_used_chans[8];
uint8_t prev_last_used_chans[8];
uint8_t menu_pos;
int16_t chan_scan;

bool saved;

enum modes {
  MAINMENU,
  SCANNER,
  LASTUSED,
  BANDMENU,
  AUTOSCAN
} mode;

char j_buf[40];

#ifdef USE_OLED

#include "U8glib.h"

#ifdef USE_I2C_OLED
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);
#else
U8GLIB_SSD1306_128X64 u8g(8, A1, A4, 11 , 13); //CLK, MOSI, CS, DC, RESET
#endif

#else //USE OSD

#include <TVout.h>
#include <TVoutfonts/fontALL.h>

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

  display_init();

  //RX module init
  rx5808.init();
#ifdef USE_DIVERSITY
  rx5808B.init();
#endif
  //rx5808.calibration();

  display_splash_rssi();

  chan_scan = 0;

  flag_first_pos = 0;
#ifdef FORCE_FIRST_MENU_ITEM
  flag_first_pos = readSwitch();
#endif

  for(int i=0 ; i<8 ; i++) {
    last_used_chans[i] = EEPROM.read(EEPROM_ADDR_LAST_CHAN_ID + i);
  }

  buttonPressed = false;
  pciSetup(2);
  pciSetup(3);
  pciSetup(4);

  menu_pos = readSwitch();
  mode = MAINMENU;
  counter = 500;

  saved = false;

  display_mainmenu(menu_pos);

  cli();
  //set timer2 interrupt at ~100Hz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for ~100Hz increments
  OCR2A = 155;// = (16*10^6) / (100*1024) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS20 & CS22 bit for 1024 prescaler
  TCCR2B |= (1 << CS20) | (1 << CS21) | (1 << CS22);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  timerExpired = false;
  sei();
}

void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

ISR(PCINT2_vect) {
  buttonPressed = true;
}

ISR(TIMER2_COMPA_vect) {
  if (!timerExpired) {
    timerExpired = (--counter == 0);
  }
}

void select_freq(uint8_t channel) {
  uint16_t freq = pgm_read_word_near(channelFreqTable + channel);
  rx5808.setFreq(freq);
#ifdef USE_DIVERSITY
  rx5808B.setFreq(freq);
#endif
  chan_scan = -1;
}

void processButton() {
  SELECT_OSD;
  menu_pos = readSwitch();
  saved = false;
  switch (mode) {
    case MAINMENU:
      // a key was pressed so we do nothing
      break;

    case SCANNER:
      // we hit a key in scanner mode so go back to mainmenu
      mode = MAINMENU;
      break;

    case LASTUSED:
      cancelOrSelect(prev_last_used_chans[menu_pos]);
      break;

    case BANDMENU:
      cancelOrSelect(menu_band * 8 + menu_pos);
      break;

    case AUTOSCAN:
      cancelOrSelect(rx5808.getfrom_top8(menu_pos));
      break;
  }
  counter = 500;
}

void cancelOrSelect(uint8_t channel) {
  // if we get a keypress in under 1 second then exit to mainmenu
  if (counter > 400) {
    mode = MAINMENU;
  } else {
    select_freq(channel);
  }
}

void processTimer() {
  // select option and enter
  switch(mode) {
    case MAINMENU:
      switch (menu_pos) {
        case 0: //LAST USED
          if (last_used_chans[0] < 40) {
            mode = LASTUSED;
            select_freq(last_used_chans[0]);
            memcpy(prev_last_used_chans, last_used_chans, 8);
          }
          counter = 500;
          break;

        case 6: //SCANNER
          mode = SCANNER;
          counter = 500;
          break;

        case 7: //AUTOSCAN
          rx5808.compute_top8();
          mode = AUTOSCAN;
          select_freq(rx5808.getfrom_top8(7));
          counter = 500;
          break;

        default:
          menu_band = menu_pos-1;
          mode = BANDMENU;
          select_freq(menu_band * 8 + menu_pos);
          counter = 500;
          break;
      }
      break;

    case SCANNER:
      counter = 500;
      break;

    case LASTUSED:
      if (!saved) {
        updateLastUsed(prev_last_used_chans[menu_pos]);
        SELECT_A;
        saved = true;
      }
      counter = 300;
      break;

    case BANDMENU:
      if (!saved) {
        updateLastUsed(menu_band * 8 + menu_pos);
        SELECT_A;
        saved = true;
      }
      counter = 300;
      break;

    case AUTOSCAN:
      if (!saved) {
        updateLastUsed(rx5808.getfrom_top8(menu_pos));
        SELECT_A;
        saved = true;
      }
      counter = 300;
      break;
  }
}

void updateLastUsed(uint8_t channel) {
  int pos = 7;
  for(int i=0 ; i<8 ; i++) {
    if (last_used_chans[i] == channel) {
      pos = i;
      break;
    }
  }
  memmove(last_used_chans+1, last_used_chans, pos);
  last_used_chans[0] = channel;
  for(int i=0 ; i<8 ; i++) {
    EEPROM.write(EEPROM_ADDR_LAST_CHAN_ID, last_used_chans[i]);
  }
}

void redisplay() {
  switch(mode) {
    case MAINMENU:
      display_mainmenu(menu_pos);
      break;

    case LASTUSED:
      display_favorites();
      break;

    case SCANNER:
      display_scanner();
      break;

    case BANDMENU:
      display_bandmenu(menu_pos, menu_band);
      break;

    case AUTOSCAN:
      display_autoscan();
      break;
  }
}

void loop(void) {
  if (mode == MAINMENU || mode == SCANNER) {
    if (chan_scan != -1) {
      rx5808.updateRssi(1, BIN_H, chan_scan);
      if (mode == SCANNER) {
        display_scanner_update(chan_scan);
      }
      chan_scan = (chan_scan + 1) % CHANNEL_MAX;
    } else {
      chan_scan = 0;
    }
    rx5808.setFreq(pgm_read_word_near(channelFreqTable + chan_scan));
  } else {

#ifdef USE_DIVERSITY
    // select best reciever
    uint16_t rssi_a = rx5808.getCurrentRSSI();
    uint16_t rssi_b = rx5808B.getCurrentRSSI();
    if (current_rx == RX_B && rssi_a > rssi_b + RX_HYST) {
      SELECT_A;
      current_rx = RX_A;
    }
    if (current_rx == RX_A && rssi_b > rssi_a + RX_HYST) {
      SELECT_B;
      current_rx = RX_B;
    }
#endif
  }

#ifdef USE_OLED
  delay(30);
#else
  TV.delay(40);
#endif

  if (buttonPressed) {
    processButton();
    buttonPressed = false;
    redisplay();
  } else if (timerExpired) {
    processTimer();
    timerExpired = false;
    redisplay();
  } else {
    if (mode != SCANNER) {
#ifdef USE_OLED
      redisplay();
#else
      display_timer();
#endif
    }
  }
}
