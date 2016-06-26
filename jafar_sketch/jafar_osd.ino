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

void osd_init(void) {
  //tv init
  TV.begin(PAL, D_COL, D_ROW);
  TV.select_font(font6x8);

  //splash screen
  TV.clear_screen();
  TV.print(0, 0, "JAFaR Project \n\n  by MikyM0use");
  TV.print(0, 50, "RSSI MIN");
  TV.println(60, 50, rssi_min, DEC); //RSSI
  TV.print(0, 60, "RSSI MAX");
  TV.println(60, 60, rssi_max, DEC); //RSSI

  TV.delay(3000);
}

void osd_submenu(int8_t menu_pos, uint8_t band) {
  int i;
  TV.clear_screen();

  //  uint8_t menu_pos;
  // timer = 9;
  // while (timer-- > 0) {
  menu_pos = readSwitch();

  //  if (last_post_switch != menu_pos) { //something changed by user
  // TV.clear_screen();
  TV.draw_rect(1, 1, 100, 94,  WHITE); //draw frame

  //show the channels list and the % RSSI
  for (i = 0; i < 8; i++) {
    TV.println(10, 3 + i * MENU_Y_SIZE, pgm_read_word_near(channelFreqTable + (8 * band) + i), DEC); //channel name

    TV.println(60, 3 + i * MENU_Y_SIZE, rx5808.getVal(band, i, 100), DEC); //RSSI
    TV.printPGM(78, 3 + i * MENU_Y_SIZE, PSTR("%")); //percentage symbol
  }


  last_post_switch = menu_pos;
  TV.draw_rect(9, 2 + menu_pos * MENU_Y_SIZE, 90, 7,  WHITE, INVERT); //current selection
  //  }

  TV.println(92, 3, timer, DEC);

  //}


  //return menu_pos;

}

void osd_waitmessage(void) {

  TV.clear_screen();
  TV.printPGM(0, 50, PSTR("PLEASE\nWAIT..."));
}

void osd_mainmenu(uint8_t menu_pos) {
  int i;
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


}

void osd_scanner() {
  uint8_t menu_pos;
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
#endif //not USE_OLED
