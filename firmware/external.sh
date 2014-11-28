#!/bin/bash

#cat text.orig | sed "s/VORNAME/$1/" > text.txt
#make clear_eeprom textconvert eeflash
make clear_eeprom
make textconvert eeflash
