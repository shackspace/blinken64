#!/bin/bash

while read foo
do
make fuse flash textconvert clear_eeprom eeflash
OUT=$'Blinken64 Firmware Info\n'
OUT+=$'Date:  '
OUT+=$(date +%Y-%m-%d-%H-%M-%S)
OUT+=$'\n'
OUT+=$'HFuse: 0x9D\n'
OUT+=$'LFuse: 0x4E\n'
OUT+=$'This is the last IC of the first blinken64 batch\n'
OUT+=$'Flashed and packed by momo with love\n'
OUT+=$'Good Night!\n'

echo "$OUT" | lpr -o DocCutType=1PartialCutDoc -P "TSP143-(STR_T-001)"

done

