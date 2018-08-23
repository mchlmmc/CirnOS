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

extern void PUT32(unsigned int, unsigned int);
extern void PUT16(unsigned int, unsigned int);
extern void PUT8(unsigned int, unsigned int);
extern unsigned int GET32(unsigned int);
extern unsigned int GETPC(void);
extern void dummy(unsigned int);

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

// Support for BE to LE conversion
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
