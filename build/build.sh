#!/bin/sh

debug=0
file=test.jpg
x=16
z=32
noflash=0
while getopts "ndf:x:z:" OPTION
do
  case $OPTION in
    d)  debug=1;;
    f)  file=${OPTARG};;
    x)  x=${OPTARG};;
    z)  z=${OPTARG};;
    n)  noflash=1;;
  esac
done

echo "file: $file"
echo "debug: $debug"

cp ../src/c/funkel.c . || exit 1
cp ../src/c/imgdata.h.template . || exit 1

python ../src/python/convert.py -img:$file -x:$x -z:$z || exit 1

avr-gcc -I/usr/lib/avr/include -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields \
	-mmcu=attiny461a -DF_CPU=8000000UL -MMD -MP -MF"funkel.d" -MT"funkel.d" -c -o"funkel.o" "funkel.c" || exit 1

avr-gcc -Wl,-Map,funkel.map -mmcu=attiny26 -o"funkel.elf"  ./funkel.o || exit 1

avr-objcopy -R .eeprom -O ihex funkel.elf  "funkel.hex" || exit 1

avr-size --format=avr --mcu=attiny26 funkel.elf || exit 1

if [ "$noflash" != "1" ]; then
    sudo /usr/bin/avrdude -pt461 -cavrisp2 -Pusb -F -Uflash:w:funkel.hex:a || exit 1

    if [ "$debug" != "1" ]; then
        rm funkel.* imgdata.* || exit 1
    fi
fi
