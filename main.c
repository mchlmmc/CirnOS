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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bcm2835.h"
#include "hdmi.h"
#include "emmc.h"
#include "ff.h"
#include "LUA/lua.h"
#include "LUA/lauxlib.h"

void print_init(){
  char* VERSION = "Alpha";
  printf("CirnOS %s Version\n", VERSION);
  printf("Copyright (c) 2018 Michael Mamic\n\n");
}

void endloop(){
  while(1) {
    bcm2835_gpio_clr(47);    
    bcm2835_delay(1000);
    bcm2835_gpio_set(47);    
    bcm2835_delay(1000);    
  }  
}

uint16_t notmain ( void )
{
  // Initialize syscall data
  for(int i = 0; i < 20; i++)
    openfiles[i] = NULL;

  extern char _end;
  highest_addr = &_end;  
  
  // Initialize hardware controller
  bcm2835_init();
  bcm2835_gpio_fsel(47, BCM2835_GPIO_FSEL_OUTP);

  hdmi_init(SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH);

  f_mount(&SDFS, "", 0);
  print_init();

  lua_State *L = luaL_newstate();
  if (!L) {
    perror("Error creating Lua state");
    return 1;
  }

  printf("Starting hello.lua...\n");  

  if (luaL_dofile(L, "hello.lua")) {
    char const *errmsg = lua_tostring(L, 1);
    fprintf(stderr, "Failed to execute hello.lua: %s\n", errmsg);
  }

  char const *result = lua_tostring(L, 1);
  if (!result) {
    fprintf(stderr, "hello.lua did not return a string\n");
    return 3;
  }
  printf("%s\n", result);

  lua_close(L);

  endloop();  
  return(0);
}
