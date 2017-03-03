#!/bin/sh

/usr/local/pkg-i686-pc-linux-gnu/bin/avrdude -q -V -p atmega328p -C /usr/local/pkg-i686-pc-linux-gnu/etc/avrdude.conf -D -c arduino -b 57600 -P /dev/ttyUSB1 -U flash:w:build-atmega328/jafar_sketch.hex:i
