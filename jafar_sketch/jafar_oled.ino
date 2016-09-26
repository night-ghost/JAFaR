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
#ifdef USE_OLED

#include "text.h"


#define MENU_ITEMS 8

static const u8g_pgm_uint8_t* channels[] = {
  chan_1_bits,
  chan_2_bits,
  chan_3_bits,
  chan_4_bits,
  chan_5_bits,
  chan_6_bits,
  chan_7_bits,
  chan_8_bits,
};

static const u8g_pgm_uint8_t* bands[] = {
  band_a_bits,
  band_b_bits,
  band_e_bits,
  band_f_bits,
  band_r_bits,
};

void display_init(void) { // flip screen, if required
#ifdef FLIP_SCREEN
  u8g.setRot180();
#endif
  // set SPI backup if required
  //u8g.setHardwareBackup(u8g_backup_avr_spi);

  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }

  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_8x13);
    u8g.drawStrP( 0, 20, U8G_PSTR("JAFaR Project"));
    u8g.drawStrP( 0, 35, U8G_PSTR("by MikyM0use"));
  } while ( u8g.nextPage() );
}

void display_splash_rssi() {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_8x13);
    u8g.drawStrP( 0, 20, U8G_PSTR("JAFaR Project"));
    u8g.drawStrP( 0, 35, U8G_PSTR("by MikyM0use"));

    u8g.setFont(u8g_font_6x10);
    sprintf_P (j_buf, PSTR("RSSI MIN %d"), rssi_min); //Rssi min
    u8g.drawStr(0, 50, j_buf);

    sprintf_P (j_buf, PSTR("RSSI MAX %d"), rssi_max); //Rssi max
    u8g.drawStr(0, 60, j_buf);
  } while ( u8g.nextPage() );
  delay(2000);
}

void display_mainmenu(uint8_t menu_pos) {
  int i;
  u8g.setFont(u8g_font_6x10);

  if (last_used_chans[0]<40) {
    sprintf_P (j_buf, PSTR("FAVORITES   %X %d"), pgm_read_byte_near(channelNames + last_used_chans[0]), pgm_read_word_near(channelFreqTable + last_used_chans[0]));
  } else {
    sprintf_P (j_buf, PSTR("FAVORITES   -- ----"));
  }
  const char *menu_strings[MENU_ITEMS] {j_buf, PSTR("BAND A"), PSTR("BAND B"), PSTR("BAND E"), PSTR("BAND FATSHARK"), PSTR("RACEBAND"), PSTR("SCANNER"), PSTR("AUTOSCAN")};

  u8g.firstPage();
  do {
    for (i = 0; i < MENU_ITEMS; i++) {
      u8g.setDefaultForegroundColor();

      if (i == menu_pos) { //current selection
        u8g.drawBox(0, 1 + menu_pos * 8, 123, 7);
        u8g.setDefaultBackgroundColor();
      }

      if (i==0)
        u8g.drawStr( 0, 8 + i * 8, j_buf); //menu item
      else
        u8g.drawStrP( 0, 8 + i * 8, (u8g_pgm_uint8_t *)(menu_strings[i])); //menu item

      if (i > 0 && i < 6) {
        //only for the "real" bands (not for scanner, autoscan etc) do we draw RSSI
        sprintf_P(j_buf, PSTR("%3d"), rx5808.getMaxValBand(i - 1, 100));
        u8g.drawStr( 80, 8 + i * 8, j_buf);
      }
    }

    u8g.setDefaultForegroundColor();
    int height = counter*64/500;
    if (height > 0)
      u8g.drawBox(124,64-height,4,height);
  } while ( u8g.nextPage() );
}

//only one screen to show all the channels
//funtction taken from the project: github.com/MikyM0use/OLED-scanner
#define FRAME_START_X 0
#define FRAME_START_Y 7
void display_scanner() {
  uint8_t i;
  u8g.setFont(u8g_font_6x10);

  u8g.firstPage();
  do {
    u8g.drawStrP(FRAME_START_X, FRAME_START_Y, U8G_PSTR("BAND"));
    u8g.drawStrP(FRAME_START_X + 80, FRAME_START_Y, U8G_PSTR("FREE CH"));

    u8g.drawStrP(FRAME_START_X + 15, FRAME_START_Y + 12, U8G_PSTR("A"));
    u8g.drawStrP(FRAME_START_X + 15, FRAME_START_Y + 22, U8G_PSTR("B"));
    u8g.drawStrP(FRAME_START_X + 15, FRAME_START_Y + 32, U8G_PSTR("E"));
    u8g.drawStrP(FRAME_START_X + 15, FRAME_START_Y + 42, U8G_PSTR("F"));
    u8g.drawStrP(FRAME_START_X + 15, FRAME_START_Y + 52, U8G_PSTR("R"));

    u8g.drawLine(25, 0, 25, 60); //start
    u8g.drawLine(76, 0, 76, 60); //end

#define START_BIN FRAME_START_X+29

#define BIN_H_LITTLE 9
#define START_BIN_Y 13

    //computation of the min value
    for (i = 0; i < 5; i++) {
      uint16_t chan = rx5808.getMinPosBand(i);
      sprintf_P (j_buf, PSTR("%x %d"), pgm_read_byte_near(channelNames + chan), pgm_read_word_near(channelFreqTable + chan));
      u8g.drawStr(FRAME_START_X + 80, FRAME_START_Y + 10 * i + 12, j_buf);
    }

    for (i = 0; i < 8; i++) {
      uint8_t bin = rx5808.getVal(0, i, BIN_H_LITTLE);
      u8g.drawBox(START_BIN + i * 6, FRAME_START_Y + START_BIN_Y - bin, 2, bin);

      bin = rx5808.getVal(1, i, BIN_H_LITTLE);
      u8g.drawBox(START_BIN + i * 6, FRAME_START_Y + START_BIN_Y - bin + 10, 2, bin);

      bin = rx5808.getVal(2, i, BIN_H_LITTLE);
      u8g.drawBox(START_BIN + i * 6, FRAME_START_Y + START_BIN_Y - bin + 20, 2, bin);

      bin = rx5808.getVal(3, i, BIN_H_LITTLE);
      u8g.drawBox(START_BIN + i * 6, FRAME_START_Y + START_BIN_Y - bin + 30, 2, bin);

      bin = rx5808.getVal(4, i, BIN_H_LITTLE);
      u8g.drawBox(START_BIN + i * 6, FRAME_START_Y + START_BIN_Y - bin + 40, 2, bin);
    }

    int height = counter*64/500;
    if (height > 0)
      u8g.drawBox(124,64-height,4,height);
  } while ( u8g.nextPage() );
}

void display_scanner_update(int16_t channel) {
  // wait for 5 seconds then update the screen
}

void display_bandmenu(uint8_t menu_pos, uint8_t band) {
  uint8_t chans[8];
  for(int i=0;i<8;i++) chans[i] = band*8+i;
  display_group(menu_pos, chans);
}

void display_autoscan(uint8_t menu_pos) {
  display_group(menu_pos, rx5808.getTop8());
}

void display_favorites(uint8_t menu_pos) {
  display_group(menu_pos, prev_last_used_chans);
}

void display_group(uint8_t menu_pos, uint8_t *group) {
  int band = group[menu_pos] / 8;
  int chan = group[menu_pos] % 8;

  u8g.setFont(u8g_font_6x10);
  u8g.firstPage();
  do {
    // Display the current selected channel
    if (group[menu_pos]<CHANNEL_MAX) {
      u8g.setDefaultForegroundColor();
      u8g.drawXBMP( 0, 0, 32, 32, bands[band]);
      u8g.drawXBMP( 32-7, 0, 32, 32, channels[chan]);

      // draw the RSSI
      u8g.drawStrP(0,49,U8G_PSTR("A"));
      u8g.drawFrame(8,40,45,10);
      u8g.drawBox(9,41,rx5808.getCurrentRSSI(0,43),8);

#ifdef USE_DIVERSITY
      u8g.drawStrP(0,61,U8G_PSTR("B"));
      u8g.drawFrame(8,52,45,10);
      u8g.drawBox(9,53,rx5808B.getCurrentRSSI(0,43),8);
#endif
    }

    for (int i = 0; i < MENU_ITEMS; i++) {
      if (group[i]>=CHANNEL_MAX)
        break;
      u8g.setDefaultForegroundColor();
      if (i == menu_pos) {
        u8g.drawBox(64-7, 1 + menu_pos * 8, 63+7-4, 7);
        u8g.setDefaultBackgroundColor();
      }
      sprintf_P (j_buf, PSTR("%X %d %3d "), pgm_read_byte_near(channelNames + group[i]), pgm_read_word_near(channelFreqTable + group[i]), rx5808.getVal(group[i], 100));
      u8g.drawStr(64-7, 8 + i * 8, j_buf);
    }

    if (!saved) {
      u8g.setDefaultForegroundColor();
      int height = counter*64/500;
      if (height > 0)
        u8g.drawBox(124,64-height,4,height);
    }
  } while ( u8g.nextPage() );
}


#endif //OLED
