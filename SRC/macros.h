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

#ifndef MACROS_H
#define MACROS_H

/**
 * PUT32 - Moves 32 bit memory
 * 
 * @address: The address to write to.
 * @value: The value to write at address. 
 *
 * Writes value to the memory address
 * specified by address.
 */
extern void PUT32(uint32_t address, uint32_t value);

/**
 * PUT16 - Moves 16 bit memory
 * 
 * @address: The address to write to.
 * @value: The value to write at address. 
 *
 * Writes value to the memory address
 * specified by address.
 */
extern void PUT16(uint32_t address, uint16_t value);

/**
 * GET32 - Reads 32 bit memory
 * 
 * @address: The address to read from.
 *
 * Returns the 32 bits starting at
 * the byte specified in address.
 */
extern uint32_t GET32(uint32_t address);

// Cannot figure out what these do.
// They originate from rpi-boot by jncronin.

extern void memory_barrier();

inline void mmio_write(uintptr_t reg, uint32_t data)
{
	memory_barrier();
	*(volatile uint32_t *)(reg) = data;
	memory_barrier();
}

inline uint32_t mmio_read(uintptr_t reg)
{
	memory_barrier();
	return *(volatile uint32_t *)(reg);
	memory_barrier();
}

/**
 * byte_swap - Converts BE to LE
 * 
 * @in: The value to convert
 *
 * Returns the in value swapped
 * between BE to LE.
 */
#ifdef __GNUC__
#define byte_swap __builtin_bswap32
#else
static inline uint32_t byte_swap(uint32_t in)
{
  uint32_t b0 = in & 0xff;
  uint32_t b1 = (in >> 8) & 0xff;
  uint32_t b2 = (in >> 16) & 0xff;
  uint32_t b3 = (in >> 24) & 0xff;
  uint32_t ret = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
  return ret;
}
#endif // __GNUC__
#endif
