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

inline uint8_t readSwitch() {
#ifdef STANDALONE
  int but_up = digitalRead(CH3);
  int but_ent = digitalRead(CH2);
  int but_down = digitalRead(CH1);

  if (but_up == LOW && menu_pos < 7)
    menu_pos++;

  if (but_down == LOW && menu_pos > 0)
    menu_pos--;

  if (but_ent == LOW)
    timer = 0;

  return menu_pos;
#else
  return 0x7 - ((digitalRead(CH3) << 2) | (digitalRead(CH2) << 1) | digitalRead(CH1));
#endif
}

#ifdef USE_DIVERSITY
void use_freq_diversity(uint32_t freq, RX5808 rx5808, RX5808 rx5808B) {
  rx5808.setFreq(freq);
  rx5808B.setFreq(freq);

  do_nothing = 1;
}
#endif

void use_freq(uint32_t freq, RX5808 rx5808) {
  rx5808.setFreq(freq);
  do_nothing = 1;
}

void set_and_wait(uint8_t band, uint8_t menu_pos) {
  unsigned rssi_b, rssi_a;
  u8 current_rx;


#ifdef USE_DIVERSITY
  //init of the second module
  RX5808 rx5808B(rssiB, SPI_CSB);
  rx5808B.init();
  use_freq_diversity(pgm_read_word_near(channelFreqTable + (8 * band) + menu_pos), rx5808, rx5808B); //set the selected freq
  SELECT_B;

  current_rx = RX_B;
#else
  use_freq(pgm_read_word_near(channelFreqTable + (8 * band) + menu_pos), rx5808); //set the selected freq
  SELECT_A;
  current_rx = RX_A;
#endif

  //clear memory for log
#ifdef ENABLE_RSSILOG
  uint8_t sample = 0;
  long g_log_offset = 0;
  for (g_log_offset = 0 ; g_log_offset < EEPROM.length() / 2 ; g_log_offset++) {
    EEPROM.write(EEPROM_ADDR_START_LOG + g_log_offset, 0);
  }
  g_log_offset = 0;
#endif

  //save band and freq as "last used"
  EEPROM.write(EEPROM_ADDR_LAST_FREQ_ID, menu_pos); //freq id
  EEPROM.write(EEPROM_ADDR_LAST_BAND_ID, band); //channel name

  //MAIN LOOP - change channel and log
  while (1) {
    rssi_a = rx5808.getCurrentRSSI();
#ifdef USE_DIVERSITY
    rssi_b = rx5808B.getCurrentRSSI();
#endif

#ifdef ENABLE_RSSILOG
    //every loop cycle requires ~100ms
    //total memory available is 492B (512-20) and every sample is 2B -> 246 sample in total
    //if we take 2 sample per seconds we have 123 seconds of recording (~2 minutes)
    if (++sample >= 2) {
      sample = 0;

      //FORMAT IS XXXXXXXRYYYYYYYY (i.e. 7bit for RSSI_B - 1bit for RX used - 8bit for RSSI_A
      if (g_log_offset < EEPROM.length() / 2)
        EEPROM.put(EEPROM_ADDR_START_LOG + g_log_offset, ((uint16_t)(((rssi_b & 0xFE) | (current_rx & 0x1)) & 0xFF) << 8) | (rssi_a & 0xFF));

      g_log_offset += sizeof(uint16_t);
    }
#endif

#ifdef DEBUG
    Serial.print("A: ");
    Serial.print(rssi_a, DEC);

    Serial.print("\tB: ");
    Serial.print(rssi_b, DEC);

    Serial.print("\twe are using: ");
    if (current_rx == RX_A) {
      Serial.print("\tA");
      Serial.print("\twe change at: ");
      Serial.println(rssi_a + RX_HYST, DEC);
    } else {
      Serial.print("\tB");
      Serial.print("\twe change at: ");
      Serial.println(rssi_b + RX_HYST, DEC);
    }
    TV.delay(500);
#endif

#ifdef USE_DIVERSITY
    if (current_rx == RX_B && rssi_a > rssi_b + RX_HYST) {
      SELECT_A;
      current_rx = RX_A;
    }

    if (current_rx == RX_A && rssi_b > rssi_a + RX_HYST) {
      SELECT_B;
      current_rx = RX_B;
    }
#endif

    menu_pos = readSwitch();

    if (last_post_switch != menu_pos) { //something changed by user
#ifdef USE_DIVERSITY
      use_freq_diversity(pgm_read_word_near(channelFreqTable + (8 * band) + menu_pos), rx5808, rx5808B); //set the selected freq
#else
      use_freq(pgm_read_word_near(channelFreqTable + (8 * band) + menu_pos), rx5808); //set the selected freq
#endif
      EEPROM.write(EEPROM_ADDR_LAST_FREQ_ID, menu_pos);
    }
    last_post_switch = menu_pos;

  } //end of loop

}
