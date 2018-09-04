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

mkdir -p SOBJ

$COMPILE -o SOBJ/CirnOS.elf -T SRC/loader SRC/vectors.s SRC/*.c SRC/LUA/*.c  -L/usr/lib/arm-none-eabi/newlib/hard -lc -lgcc -lnosys -lm

# extract binary image from ELF executable
arm-none-eabi-objcopy SOBJ/CirnOS.elf -O binary SOBJ/kernel.img
