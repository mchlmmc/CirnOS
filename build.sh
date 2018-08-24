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

$COMPILE -o OBJ/CirnOS.elf -T loader vectors.s main.c\
	 hdmi.c bcm2835.c syscalls.c emmc.c ff.c diskio.c ffsystem.c ffunicode.c\
	 LUA/lapi.c LUA/lbaselib.c LUA/lauxlib.c LUA/lbitlib.c LUA/lcode.c\
	 LUA/lcorolib.c LUA/lctype.c LUA/ldblib.c LUA/ldebug.c LUA/ldo.c\
	 LUA/ldump.c LUA/lfunc.c LUA/lgc.c LUA/linit.c LUA/liolib.c LUA/llex.c\
	 LUA/lmathlib.c LUA/lmem.c LUA/loadlib.c LUA/lobject.c LUA/lopcodes.c LUA/loslib.c\
	 LUA/lparser.c LUA/lstate.c LUA/lstring.c LUA/lstrlib.c LUA/ltable.c\
	 LUA/ltablib.c LUA/ltm.c LUA/lundump.c LUA/lutf8lib.c LUA/lvm.c LUA/lzio.c\
	 -L/usr/lib/arm-none-eabi/newlib/hard -lc -lgcc -lm

# extract binary image from ELF executable
arm-none-eabi-objcopy OBJ/CirnOS.elf -O binary OBJ/kernel.img
