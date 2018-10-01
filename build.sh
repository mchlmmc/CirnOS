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

#!/usr/bin/env bash

# Stop on errors
set -e


GCC_OPTS=-nostartfiles -ffreestanding -mcpu=arm1176jzf-s -mfpu=vfp -mhard-float
GCC_LIBS=-lluajit -lc -lgcc -lnosys -lm

# Switch custom parameters
if [[ -z $CC ]]; then
  CC=arm-none-eabi-gcc
fi
if [[ -z $OBJCOPY ]]; then
  OBJCOPY=arm-none-eabi-objcopy
fi
if [[ -z $CFLAGS ]]; then
  CFLAGS=-Wall -g0 -O2 -nostdlib
fi
if [[ -z $INC ]]; then
  INC=-I. -Isrc -Iinclude -I/usr/lib/arm-none-eabi/include
fi
if [[ -z $LIBS ]]; then
  LIBS=-L. -Lsrc -Llib -L/usr/lib/arm-none-eabi/newlib/hard
fi


mkdir -p obj
mkdir -p bin/test


# Compile OS
$CC $CFLAGS $GCC_OPTS $INC $LIBS -c src/*.c src/*.s
mv -u *.o obj

# Link OS
$CC $CFLAGS $GCC_OPTS $INC $LIBS -o CirnOS.elf -T src/linker.ld obj/*.o $GCC_LIBS
mv -u *.elf obj
mv -u *.img obj

# Extract binary image from ELF executable, load  into common dir
$OBJCOPY obj/CirnOS.elf -O binary bin/kernel.img
cp root/* bin/
cp test/* bin/test
