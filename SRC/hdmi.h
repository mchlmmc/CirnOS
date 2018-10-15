// CirnOS -- Minimalistic scripting environment for the Raspberry Pi
// Copyright (C) 2018 Michael Mamic
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <stdint.h>

#ifndef HDMI_H
#define HDMI_H

void hdmi_init();
void hdmi_write_char(char c);

#define SCREEN_WIDTH            1280
#define SCREEN_HEIGHT           720

#define CHAR_W                  8
#define CHAR_H                  12
#define BIT_DEPTH               16

#define CONSOLE_WIDTH           SCREEN_WIDTH / CHAR_W
#define CONSOLE_HEIGHT          SCREEN_HEIGHT / CHAR_H

#define ORANGE                  0xFD60
#define BLACK                   0x0000
#define BLUE                    0x001F
#define RED                     0xF800
#define GREEN                   0x07E0
#define CYAN                    0x07FF
#define MAGENTA                 0xF81F
#define YELLOW                  0xFFE0
#define WHITE                   0xFFFF

#endif
