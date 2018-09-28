/* CirnOS -- Minimalistic scripting environment for the Raspberry Pi
   Copyright (C) 2018 Michael Mamic

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

RAM_AMOUNT = 0x20000000;

MEMORY
{
    ram : ORIGIN = 0x8000, LENGTH = RAM_AMOUNT - 0x8000
}

SECTIONS
{
    .text : { *(.text*) } > ram
    .data : { *(.data*) } > ram    
    _bss = .;        
    .bss : { *(.bss*) } > ram
    _end = .;
    __exidx_start = .;
    __exidx_end = .;
}