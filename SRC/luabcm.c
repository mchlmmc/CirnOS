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

#include "bcm2835.h"
#include "stdio.h"
#include "LUA/lua.h"
#include "LUA/lauxlib.h"

static int l_delay (lua_State *L) {
  double d = luaL_checknumber(L, 1);
  if((double)(uint32_t)d == d) {
    bcm2835_delay((uint32_t)d);  
  } else {
    luaL_error(L, "BCM2835 Error: Invalid argument to delay (expected uint32_t).");
  }
  return 0;
}

static int l_fsel (lua_State *L) {
  double p = luaL_checknumber(L, 1);
  if((double)(uint8_t)p != p) {
    luaL_error(L, "BCM2835 Error: Invalid argument to pin (expected uint32_t).");
  }
  
  uint8_t pin = (uint8_t)p;

  double k = luaL_checknumber(L, 2);  
  if((double)(uint8_t)k != k) {
    luaL_error(L, "BCM2835 Error: Invalid argument to mode (expected uint32_t).");
  }
  uint8_t mode = (uint8_t)k;
  if (mode > 8) {
    luaL_error(L, "BCM2835 Error: Invalid mode value.");
  }

  bcm2835_gpio_fsel(pin, mode);
  
  return 0;
}

static int l_write (lua_State *L) {
  double p = luaL_checknumber(L, 1);
  if((double)(uint8_t)p != p) {
    luaL_error(L, "BCM2835 Error: Invalid argument for pin (expected uint32_t).");
  }
  
  uint8_t pin = (uint8_t)p;

  int k = 0;

  switch(lua_type(L, 2)) {
  case LUA_TBOOLEAN:
    k = lua_toboolean(L, 2);
    break;
  case LUA_TNUMBER:
    k = lua_tonumber(L, 2);
    break;
  default:
    luaL_error(L, "BCM2835 Error: Invalid type for status (expected boolean or number).");    
    break;
  }

  // LED on Pi Zero
  if(pin == 47) k = !k;

  bcm2835_gpio_write(pin, k);
  
  return 0;
}

static int l_lev (lua_State *L) {
  double p = luaL_checknumber(L, 1);
  if((double)(uint8_t)p != p) {
    luaL_error(L, "BCM2835 Error: Invalid argument for pin (expected uint32_t).");
  }
  
  uint8_t pin = (uint8_t)p;

  lua_pushboolean(L, bcm2835_gpio_lev(pin));
  
  return 1;
}

/**
 * luabcm_register - Adds BCM library to Lua
 *
 * @L: Lua environment to add to
 *
 * Registers various constants and 
 * functions to allow Lua user code to
 * access the rPi's peripherals.t
 */
void luabcm_register(lua_State *L) {
  lua_pushcfunction(L, l_delay);
  lua_setglobal(L, "delay");
  lua_pushcfunction(L, l_fsel);
  lua_setglobal(L, "pinMode");
  lua_pushcfunction(L, l_write);
  lua_setglobal(L, "writePin");
  lua_pushcfunction(L, l_lev);
  lua_setglobal(L, "readPin");
  
  lua_pushboolean(L, 1);
  lua_setglobal(L, "ON");
  lua_pushboolean(L, 0);
  lua_setglobal(L, "OFF");
  lua_pushnumber(L, 0);  
  lua_setglobal(L, "INPUT");
  lua_pushnumber(L, 1);
  lua_setglobal(L, "OUTPUT");      
}
