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
  TV.printPGM(0, 0, PSTR("JAFaR Project \n\n  by MikyM0use"));
}

void display_splash_rssi(void) {
  TV.select_font(font6x8);

  //splash screen
  TV.clear_screen();
  TV.printPGM(0, 0, PSTR("JAFaR Project \n\n  by MikyM0use"));
  TV.printPGM(0, 50, PSTR("RSSI MIN"));
  TV.println(60, 50, rssi_min, DEC); //RSSI
  TV.printPGM(0, 60, PSTR("RSSI MAX"));
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
    sprintf_P (j_buf, PSTR("FAVORITES %X %d"), pgm_read_byte_near(channelNames + last_used_chans[0]), pgm_read_word_near(channelFreqTable + last_used_chans[0]));
    TV.println(3, 3 , j_buf);
  } else {
    TV.printPGM(3, 3 , PSTR("FAVORITES -- ----"));
  }

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
  TV.clear_screen();
  TV.select_font(font6x8);
  TV.draw_rect(0, 0, D_COL, D_ROW, WHITE);

  for (uint8_t i = 0; i < 8; i++) {
    if (group[i]<0 || group[i]>40)
      break;
    TV.println(10, 3 + i * MENU_Y_SIZE, pgm_read_word_near(channelFreqTable + group[i]), DEC); //channel freq
    TV.println(45, 3 + i * MENU_Y_SIZE , pgm_read_byte_near(channelNames + group[i]), HEX); //channel name
    TV.println(65, 3 + i * MENU_Y_SIZE, rx5808.getVal(group[i], 100), DEC); //RSSI
    TV.printPGM(85, 3 + i * MENU_Y_SIZE, PSTR("%"));
  }

  TV.draw_rect(2, 2 + menu_pos * MENU_Y_SIZE, D_COL-10, 8, WHITE, INVERT); //current selection
}
#endif //not USE_OLED
