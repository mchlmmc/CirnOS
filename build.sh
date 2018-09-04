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

GCC_OPTS=" -Wall -O2 -nostartfiles -nostdlib -mhard-float -ffreestanding -mcpu=arm1176jzf-s -mfpu=vfp -fno-builtin -ISRC -I/usr/lib/arm-none-eabi/include -DCPU_ARM"

COMPILE="arm-none-eabi-gcc $GCC_OPTS"

mkdir -p OBJ

$COMPILE -o OBJ/CirnOS.elf -T SRC/loader SRC/vectors.s SRC/main.c\
	 SRC/hdmi.c SRC/bcm2835.c SRC/syscalls.c SRC/emmc.c SRC/ff.c SRC/diskio.c SRC/ffsystem.c SRC/ffunicode.c SRC/luabcm.c\
	 SRC/LUA/lapi.c SRC/LUA/lbaselib.c SRC/LUA/lauxlib.c SRC/LUA/lbitlib.c SRC/LUA/lcode.c\
	 SRC/LUA/lcorolib.c SRC/LUA/lctype.c SRC/LUA/ldblib.c SRC/LUA/ldebug.c SRC/LUA/ldo.c\
	 SRC/LUA/ldump.c SRC/LUA/lfunc.c SRC/LUA/lgc.c SRC/LUA/linit.c SRC/LUA/liolib.c SRC/LUA/llex.c\
	 SRC/LUA/lmathlib.c SRC/LUA/lmem.c SRC/LUA/loadlib.c SRC/LUA/lobject.c SRC/LUA/lopcodes.c SRC/LUA/loslib.c\
	 SRC/LUA/lparser.c SRC/LUA/lstate.c SRC/LUA/lstring.c SRC/LUA/lstrlib.c SRC/LUA/ltable.c\
	 SRC/LUA/ltablib.c SRC/LUA/ltm.c SRC/LUA/lundump.c SRC/LUA/lutf8lib.c SRC/LUA/lvm.c SRC/LUA/lzio.c\
	 -L/usr/lib/arm-none-eabi/newlib/hard -lc -lgcc -lnosys -lm

# extract binary image from ELF executable
arm-none-eabi-objcopy OBJ/CirnOS.elf -O binary OBJ/kernel.img
