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
  // u8g.setRot180();

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

        u8g.drawBox(0, 1 + menu_pos * 8, 110, 8);
        u8g.setDefaultBackgroundColor();
      }

      sprintf (j_buf, "%d %d", pgm_read_word_near(channelFreqTable + (8 * band) + i), rx5808.getVal(band, i, 100));
      u8g.drawStr( 0, 8 + i * 8, j_buf);
    }

    u8g.setPrintPos(110, 10);
    u8g.print(timer);

  } while ( u8g.nextPage() );
}

void oled_mainmenu(uint8_t menu_pos) {
  int i;
  u8g.setFont(u8g_font_6x10);

  sprintf (j_buf, "LAST USED: %x:%d  %d", pgm_read_byte_near(channelNames + (8 * last_used_band) + last_used_freq_id), last_used_freq, timer); //last used freq
  char *menu_strings[MENU_ITEMS] {j_buf, "BAND A", "BAND B", "BAND E", "BAND FATSHARK", "RACEBAND", "SCANNER", "AUTOSCAN"};

  u8g.firstPage();
  do {

    for (i = 0; i < MENU_ITEMS; i++) {
      u8g.setDefaultForegroundColor();
      if (i == menu_pos) {

        u8g.drawBox(0, 1 + menu_pos * 8, 110, 7);
        u8g.setDefaultBackgroundColor();
      }
      u8g.drawStr( 0, 8 + i * 8, menu_strings[i]);
    }

  } while ( u8g.nextPage() );
}


#endif //OLED
