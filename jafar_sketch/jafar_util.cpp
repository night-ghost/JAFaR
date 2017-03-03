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


#include "Arduino.h" 
#include "const.h" 

uint8_t readSwitch() {
#ifdef STANDALONE
  int but_up = digitalRead(CH1);
  int but_ent = digitalRead(CH2);
  int but_down = digitalRead(CH3);

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


// чтение и запись мелких объектов
void NOINLINE eeprom_read_len(byte *p, uint16_t e, uint16_t l){
    for(;l!=0; l--) {
        *p++ = (byte)eeprom_read_byte( (byte *)e++ );
    }
}

NOINLINE void eeprom_write_len(byte *p, uint16_t e, uint16_t l){
    byte b;
    for(;  l!=0; l--, e++) {
        b = *p++;
        if(eeprom_read_byte((byte *)e) != b)
            eeprom_write_byte( (byte *)e, b);
    }

}

uint16_t  eeprom_read_word(uint16_t addr){
    uint16_t w;
    
    eeprom_read_len((byte *)&w, addr, sizeof(w));
    return w;
}

void eeprom_write_word(uint16_t addr, uint16_t w){
    eeprom_write_len((byte *)&w, addr, sizeof(w));
}

