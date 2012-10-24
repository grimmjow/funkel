#!/bin/sh

avr-gcc -I/usr/lib/avr/include -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields \
	-mmcu=attiny26 -DF_CPU=8000000UL -MMD -MP -MF"funkel.d" -MT"funkel.d" -c -o"funkel.o" "../src/c/funkel.c" || exit 1

avr-gcc -Wl,-Map,Funkelhobber.map -mmcu=attiny26 -o"Funkelhobber.elf"  ./funkel.o || exit 1

avr-objcopy -R .eeprom -O ihex Funkelhobber.elf  "Funkelhobber.hex" || exit 1

avr-size --format=avr --mcu=attiny26 Funkelhobber.elf || exit 1
