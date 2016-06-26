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

void oled_waitmessage() {
  u8g.setFont(u8g_font_8x13);
  u8g.firstPage();
  do {
    u8g.drawStr( 0, 20, "PLEASE");
    u8g.drawStr( 0, 35, "WAIT...");
  } while ( u8g.nextPage() );
}

void oled_splash() {

  u8g.setFont(u8g_font_8x13);
  u8g.firstPage();
  do {
    u8g.drawStr( 0, 20, "JAFaR Project");
    u8g.drawStr( 0, 35, "by MikyM0use");

    u8g.setFont(u8g_font_6x10);
    sprintf (j_buf, "RSSI MIN %d", rssi_min); //Rssi min
    u8g.drawStr(0, 50, j_buf);

    sprintf (j_buf, "RSSI MAX %d", rssi_max); //Rssi max
    u8g.drawStr(0, 60, j_buf);
  } while ( u8g.nextPage() );
  delay(2000);

}


void oled_init(void) { // flip screen, if required
  u8g.setRot180();

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

  oled_splash();
}


#define MENU_ITEMS 8
uint8_t oled_submenu(uint8_t menu_pos, uint8_t band) {
  int i;

  u8g.setFont(u8g_font_6x10);

  u8g.firstPage();
  do {
    for (i = 0; i < MENU_ITEMS; i++) {
      u8g.setDefaultForegroundColor();
      if (i == menu_pos) {

        u8g.drawBox(0, 1 + menu_pos * 8, 127, 7);
        u8g.setDefaultBackgroundColor();
      }

      sprintf (j_buf, "%d     %d", pgm_read_word_near(channelFreqTable + (8 * band) + i), rx5808.getVal(band, i, 100));
      u8g.drawStr( 0, 8 + i * 8, j_buf);
    }

    u8g.setPrintPos(110, 10);
    u8g.print(timer);

  } while ( u8g.nextPage() );
}

void oled_mainmenu(uint8_t menu_pos) {
  int i;
  u8g.setFont(u8g_font_6x10);

  sprintf (j_buf, "last used: %x:%d  %d", pgm_read_byte_near(channelNames + (8 * last_used_band) + last_used_freq_id), last_used_freq, timer); //last used freq
  char *menu_strings[MENU_ITEMS] {j_buf, "BAND A", "BAND B", "BAND E", "BAND FATSHARK", "RACEBAND", "SCANNER", "AUTOSCAN"};

  u8g.firstPage();
  do {

    for (i = 0; i < MENU_ITEMS; i++) {
      u8g.setDefaultForegroundColor();
      if (i == menu_pos) { //selection
        u8g.drawBox(0, 1 + menu_pos * 8, 127, 7);
        u8g.setDefaultBackgroundColor();
      }
      u8g.drawStr( 0, 8 + i * 8, menu_strings[i]);
      u8g.setPrintPos(80, 8 + i * 8); //RSSI value  (only for the "real" bands
      if (i > 0 && i<6)
        u8g.print(rx5808.getMaxValBand(i-1, 100));
    }

  } while ( u8g.nextPage() );
}

//only one screen to show all the channels
//funtction taken from the project: github.com/MikyM0use/OLED-scanner
#define FRAME_START_X 0
#define FRAME_START_Y 7
void oled_scanner() {

  uint8_t i;
  u8g.setFont(u8g_font_6x10);

  uint8_t s_timer = 9;
  while (s_timer-- > 0) {
    u8g.firstPage();
    do {
      u8g.drawStr(FRAME_START_X, FRAME_START_Y, "BAND");
      u8g.drawStr(FRAME_START_X + 80, FRAME_START_Y, "FREE CH");

      u8g.drawStr(FRAME_START_X + 15, FRAME_START_Y + 12, "A");
      u8g.drawStr(FRAME_START_X + 15, FRAME_START_Y + 22, "B");
      u8g.drawStr(FRAME_START_X + 15, FRAME_START_Y + 32, "E");
      u8g.drawStr(FRAME_START_X + 15, FRAME_START_Y + 42, "F");
      u8g.drawStr(FRAME_START_X + 15, FRAME_START_Y + 52, "R");

      u8g.drawLine(25, 0, 25, 60); //start
      u8g.drawLine(76, 0, 76, 60); //end

#define START_BIN FRAME_START_X+29

#define BIN_H_LITTLE 9
#define START_BIN_Y 13

      //computation of the min value
      for (i = 0; i < 5; i++) {
        uint16_t chan = rx5808.getMinPosBand(i);
        sprintf (j_buf, "%x %d", pgm_read_byte_near(channelNames + chan), pgm_read_word_near(channelFreqTable + chan));
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

    } while ( u8g.nextPage() );
    delay(1000);
  }
}

#endif //OLED
