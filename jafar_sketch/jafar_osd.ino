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

#ifndef USE_OLED

void display_init(void) {
  //tv init
  TV.begin(PAL, D_COL, D_ROW);
  TV.select_font(font6x8);

  //splash screen
  TV.clear_screen();
  TV.print(0, 0, "JAFaR Project \n\n  by MikyM0use");
}

void display_splash_rssi(void) {
  TV.select_font(font6x8);

  //splash screen
  TV.clear_screen();
  TV.print(0, 0, "JAFaR Project \n\n  by MikyM0use");
  TV.print(0, 50, "RSSI MIN");
  TV.println(60, 50, rssi_min, DEC); //RSSI
  TV.print(0, 60, "RSSI MAX");
  TV.println(60, 60, rssi_max, DEC); //RSSI

  TV.delay(2000);
}

void display_timer() {
  if (counter > 0) {
    int fh = D_ROW-4;
    int height = ((long)counter)*fh/500;
    if (D_ROW-4-height > 0)
      TV.draw_rect(D_COL-6, 2, 4, fh-height, BLACK, BLACK);
    if (height > 0)
      TV.draw_rect(D_COL-6, fh-height+1, 4, height, WHITE, WHITE);
  }
}

void display_mainmenu(uint8_t menu_pos) {
  int i;
  TV.clear_screen();
  TV.select_font(font6x8);
  TV.draw_rect(0, 0, D_COL, D_ROW, WHITE);

  //last used band,freq
  if (last_used_chans[0]<40) {
    sprintf (j_buf, "FAVORITES %X %d", pgm_read_byte_near(channelNames + last_used_chans[0]), pgm_read_word_near(channelFreqTable + last_used_chans[0]));
  } else {
    sprintf (j_buf, "FAVORITES -- ----");
  }
  TV.println(3, 3 , j_buf);

  //entire menu
  TV.printPGM(3, 3 + 1 * MENU_Y_SIZE, PSTR("BAND A"));
  TV.printPGM(3, 3 + 2 * MENU_Y_SIZE, PSTR("BAND B"));
  TV.printPGM(3, 3 + 3 * MENU_Y_SIZE, PSTR("BAND E"));
  TV.printPGM(3, 3 + 4 * MENU_Y_SIZE, PSTR("FATSHARK"));
  TV.printPGM(3, 3 + 5 * MENU_Y_SIZE, PSTR("RACEBAND"));
  TV.printPGM(3, 3 + 6 * MENU_Y_SIZE, PSTR("SCANNER"));
  TV.printPGM(3, 3 + 7 * MENU_Y_SIZE, PSTR("AUTOSCAN"));

  for (i = 0; i < 5; i++) {
    TV.println(65, 3 + (1 + i) * MENU_Y_SIZE, rx5808.getMaxValBand(i, 100), DEC);
    TV.printPGM(85, 3 + (1 + i) * MENU_Y_SIZE, PSTR("%"));
  }

  TV.draw_rect(2, 2 + menu_pos * MENU_Y_SIZE, D_COL-10, 8, WHITE, INVERT); //current selection
}

void display_scanner() {
    TV.clear_screen();
    TV.draw_rect(0, 0, D_COL, D_ROW, WHITE);
    TV.select_font(font4x6);
    TV.printPGM(5, 87, PSTR("5645"));
    TV.printPGM(45, 87, PSTR("5800"));
    TV.printPGM(85, 87, PSTR("5945"));

    TV.select_font(font6x8);
    for (int i = CHANNEL_MIN; i < CHANNEL_MAX; i++) {
      uint8_t channelIndex = pgm_read_byte_near(channelList + i); //retrive the value based on the freq order
      TV.draw_rect(10 + 2 * i, 10 + BIN_H - rx5808.getRssi(channelIndex) , 2, rx5808.getRssi(channelIndex), WHITE, WHITE);
    }
}

void display_scanner_update(int16_t channel) {
  uint8_t channelIndex = pgm_read_byte_near(channelList + channel); //retrive the value based on the freq order
  TV.draw_rect(10 + 2 * channel, 10 , 2, BIN_H, BLACK, BLACK);
  TV.draw_rect(10 + 2 * channel, 10 + BIN_H - rx5808.getRssi(channelIndex) , 2, rx5808.getRssi(channelIndex), WHITE, WHITE);
}

void display_bandmenu(uint8_t menu_pos, uint8_t band) {
  int i;
  TV.clear_screen();
  TV.select_font(font6x8);
  TV.draw_rect(0, 0, D_COL, D_ROW, WHITE);

  for (i = 0; i < 8; i++) {
    TV.println(10, 3 + i * MENU_Y_SIZE, pgm_read_word_near(channelFreqTable + (8 * band) + i), DEC); //channel name

    TV.println(60, 3 + i * MENU_Y_SIZE, rx5808.getVal(band, i, 100), DEC); //RSSI
    TV.printPGM(78, 3 + i * MENU_Y_SIZE, PSTR("%")); //percentage symbol
  }

  TV.draw_rect(2, 2 + menu_pos * MENU_Y_SIZE, D_COL-10, 8, WHITE, INVERT); //current selection
}

void display_autoscan() {
  TV.clear_screen();
  TV.select_font(font6x8);
  TV.draw_rect(0, 0, D_COL, D_ROW, WHITE);

  for (uint8_t i = 0; i < 8; i++) {
    TV.println(10, 3 + i * MENU_Y_SIZE, pgm_read_word_near(channelFreqTable + rx5808.getfrom_top8(i)), DEC); //channel freq
    TV.println(45, 3 + i * MENU_Y_SIZE , pgm_read_byte_near(channelNames + rx5808.getfrom_top8(i)), HEX); //channel name
    TV.println(65, 3 + i * MENU_Y_SIZE, rx5808.getVal(rx5808.getfrom_top8(i), 100), DEC); //RSSI
    TV.printPGM(85, 3 + i * MENU_Y_SIZE, PSTR("%"));
  }

  TV.draw_rect(2, 2 + menu_pos * MENU_Y_SIZE, D_COL-10, 8, WHITE, INVERT); //current selection
}

void display_favorites() {
  TV.clear_screen();
  TV.select_font(font6x8);
  TV.draw_rect(0, 0, D_COL, D_ROW, WHITE);

  for (uint8_t i = 0; i < 8; i++) {
    TV.println(10, 3 + i * MENU_Y_SIZE, pgm_read_word_near(channelFreqTable + prev_last_used_chans[i]), DEC); //channel freq
    TV.println(45, 3 + i * MENU_Y_SIZE , pgm_read_byte_near(channelNames + prev_last_used_chans[i]), HEX); //channel name
    TV.println(65, 3 + i * MENU_Y_SIZE, rx5808.getVal(prev_last_used_chans[i], 100), DEC); //RSSI
    TV.printPGM(85, 3 + i * MENU_Y_SIZE, PSTR("%"));
  }

  TV.draw_rect(2, 2 + menu_pos * MENU_Y_SIZE, D_COL-10, 8, WHITE, INVERT); //current selection
}
#endif //not USE_OLED
