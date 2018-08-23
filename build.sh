# CirnOS -- Minimalistic scripting environment for the Raspberry Pi
# Copyright (C) 2018 Michael Mamic
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

#!/bin/sh

# stop on errors
set -e

GCC_OPTS=" -Wall -O2 -nostartfiles -nostdlib -mhard-float -ffreestanding -mcpu=arm1176jzf-s -mfpu=vfp -fno-builtin -I. -I/usr/include/newlib -DCPU_ARM"

COMPILE="arm-none-eabi-gcc $GCC_OPTS"

mkdir -p OBJ
$COMPILE -c -o OBJ/hdmi.o hdmi.c
$COMPILE -c -o OBJ/bcm2835.o bcm2835.c
$COMPILE -c -o OBJ/syscalls.o syscalls.c
$COMPILE -c -o OBJ/emmc.o emmc.c
$COMPILE -c -o OBJ/ff.o ff.c
$COMPILE -c -o OBJ/diskio.o diskio.c
$COMPILE -c -o OBJ/ffsystem.o ffsystem.c
$COMPILE -c -o OBJ/ffunicode.o ffunicode.c

$COMPILE -c -o OBJ/lapi.o LUA/lapi.c
$COMPILE -c -o OBJ/lauxlib.o LUA/lauxlib.c
$COMPILE -c -o OBJ/lauxlib.o LUA/lauxlib.c
$COMPILE -c -o OBJ/lcode.o LUA/lcode.c
$COMPILE -c -o OBJ/lctype.o  LUA/lctype.c
$COMPILE -c -o OBJ/ldebug.o LUA/ldebug.c
$COMPILE -c -o OBJ/ldo.o LUA/ldo.c
$COMPILE -c -o OBJ/ldump.o LUA/ldump.c
$COMPILE -c -o OBJ/lfunc.o LUA/lfunc.c
$COMPILE -c -o OBJ/lgc.o LUA/lgc.c
$COMPILE -c -o OBJ/llex.o LUA/llex.c
$COMPILE -c -o OBJ/lmem.o LUA/lmem.c
$COMPILE -c -o OBJ/lobject.o LUA/lobject.c
$COMPILE -c -o OBJ/lopcodes.o LUA/lopcodes.c
$COMPILE -c -o OBJ/lparser.o LUA/lparser.c
$COMPILE -c -o OBJ/lstate.o LUA/lstate.c
$COMPILE -c -o OBJ/lstring.o LUA/lstring.c
$COMPILE -c -o OBJ/ltable.o LUA/ltable.c
$COMPILE -c -o OBJ/ltm.o LUA/ltm.c
$COMPILE -c -o OBJ/lundump.o LUA/lundump.c
$COMPILE -c -o OBJ/lvm.o LUA/lvm.c
$COMPILE -c -o OBJ/lzio.o LUA/lzio.c

$COMPILE -o OBJ/CirnOS.elf -T loader vectors.s main.c\
	 OBJ/hdmi.o OBJ/bcm2835.o OBJ/syscalls.o OBJ/emmc.o OBJ/ff.o OBJ/diskio.o OBJ/ffsystem.o OBJ/ffunicode.o\
	 OBJ/lapi.o OBJ/lcode.o OBJ/lctype.o OBJ/ldebug.o OBJ/ldo.o OBJ/ldump.o OBJ/lfunc.o OBJ/lgc.o OBJ/llex.o\
	 OBJ/lmem.o OBJ/lobject.o OBJ/lopcodes.o OBJ/lparser.o OBJ/lstate.o OBJ/lstring.o OBJ/ltable.o OBJ/ltm.o\
	 OBJ/lundump.o OBJ/lvm.o OBJ/lzio.o OBJ/lauxlib.o\
	 -L/usr/lib/arm-none-eabi/newlib/hard -lc -lgcc -lm

# extract binary image from ELF executable
arm-none-eabi-objcopy OBJ/CirnOS.elf -O binary OBJ/kernel.img
