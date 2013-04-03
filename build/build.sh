#!/bin/sh

set -e

debug=0
file=test.jpg
x=32
z=32
noflash=0

# target device specification
mcu=attiny461a
dude_mcu=t461

# prgramm name
program=funkel

usage() {

	echo "usage: $0 [options]"
	echo ""
	echo "Options:"
	echo "-x, -z    : set resolution (default: z: 32, x: 32)"
	echo "-d        : debug"
	echo "-f        : image to display"
	echo "-p        : programm name (default: funkel)"
	echo "-n        : no flash, just build"

}

while getopts "ndf:x:z:p:" OPTION
do
  case $OPTION in
    d)  debug=1;;
    f)  file=${OPTARG};;
    x)  x=${OPTARG};;
    z)  z=${OPTARG};;
    p)  program=${OPTARG};;
    n)  noflash=1;;
    *) usage; exit 0;;
  esac
done

echo "file: $file"
echo "debug: $debug"

cp ../src/c/${program}.c .
cp ../src/c/imgdata.h.template .

python ../src/python/convert.py -img:$file -x:$x -z:$z

avr-gcc -I/usr/lib/avr/include -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields \
	-mmcu=${mcu} -DF_CPU=8000000UL -MMD -MP -MF"${program}.d" -MT"${program}.d" -c -o"${program}.o" "${program}.c"

avr-gcc -Wl,-Map,${program}.map -mmcu=${mcu} -o"${program}.elf"  ./${program}.o

avr-objcopy -R .eeprom -O ihex ${program}.elf  "${program}.hex"

avr-size --format=avr --mcu=${mcu} ${program}.elf

if [ "$noflash" != "1" ]; then
    sudo /usr/bin/avrdude -p${dude_mcu} -cavrisp2 -Pusb -F -Uflash:w:${program}.hex:a

    if [ "$debug" != "1" ]; then
        rm ${program}.* imgdata.*
    fi
fi
