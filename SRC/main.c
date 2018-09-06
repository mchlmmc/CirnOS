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

#include <stdio.h>

#include "bcm2835.h"
#include "hdmi.h"
#include "ff.h"
#include "luabcm.h"

#include "LUA/lua.h"
#include "LUA/lauxlib.h"
#include "LUA/lualib.h"

/**
 * print_init - Prints initial messages.
 * 
 * Prints build version and copyright notice.
 */
void print_init()
{
  printf("CirnOS Alpha Version 2.0\n");
  printf("Copyright (c) 2018 Michael Mamic\n\n");
}

/**
 * clear_bss - Clears .BSS section
 * 
 * CirnOS expects this section to be cleared
 * with zeroes before running. Information on
 * the layout of the CirnOS binary can be
 * found in the loader linker script.
 */
void clear_bss()
{
  extern char _bss;  
  char *bss_addr = &_bss;
  extern char _end;  
  char *end_addr = &_end;  

  while(bss_addr < end_addr) {
    *bss_addr = 0;
    bss_addr++;
  }
}

/**
 * notmain - OS entry point
 * 
 * First code to be run in C, started by
 * the init code in vectors.s.
 * Responsible for booting the user into
 * a Lua environment and initializing
 * CirnOS's drivers and libraries.
 */

// Wait 10 seconds and hold on sharp angle
int notmain()
{
  clear_bss();
  
  bcm2835_init();  
  hdmi_init(SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH);
  f_mount(&SDFS, "", 0);
  print_init();   

  // Start Lua
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  // CirnOS does not support these functions
  lua_pushnil(L);
  lua_setglobal(L, "os");
  lua_pushnil(L);  
  lua_setglobal(L, "utf8");

  luabcm_register(L);
  
  if (!L) {
    perror("Error creating Lua state");
    return 0;
  }

  // Make sure there is code for Lua to run
  FILE *check_file;
  if (!(check_file = fopen("main.lua", "r")))
  {
    printf("File main.lua does not exist. Halting.\n");      
    return 0;
  }
  
  fclose(check_file);

  // Run user's code
  if (luaL_dofile(L, "main.lua")) {
    char const *errmsg = lua_tostring(L, 1);
    printf("Failed to execute main.lua:\n%s\n", errmsg);
  }  

  lua_close(L);

  return 0;
}
