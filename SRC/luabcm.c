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

uint32_t pwm_range = 256;

static int l_delay (lua_State *L)
{
  double d = luaL_checknumber(L, 1);
  if((double)(uint32_t)d == d) {
    bcm2835_delay((uint32_t)d);  
  } else {
    luaL_error(L, "BCM2835 Error: Invalid argument to delay (expected uint32_t).");
  }
  return 0;
}

static int l_fsel (lua_State *L)
{
  double p = luaL_checknumber(L, 1);
  if((double)(uint8_t)p != p) {
    luaL_error(L, "BCM2835 Error: Invalid argument to pin (expected uint8_t).");
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

static int l_write (lua_State *L)
{
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

static int l_lev (lua_State *L)
{
  double p = luaL_checknumber(L, 1);
  if((double)(uint8_t)p != p) {
    luaL_error(L, "BCM2835 Error: Invalid argument for pin (expected uint8_t).");
  }
  
  uint8_t pin = (uint8_t)p;

  lua_pushboolean(L, bcm2835_gpio_lev(pin));
  
  return 1;
}

static int l_pwm_init (lua_State *L)
{
  bcm2835_gpio_fsel(18, BCM2835_GPIO_FSEL_ALT5);
  bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_16);
  bcm2835_pwm_set_mode(0, 1, 1);
  bcm2835_pwm_set_range(0, 256);
  bcm2835_pwm_set_data(0, 0);  
  
  return 0;
}

static int l_pwm_set_range (lua_State *L)
{
  double r = luaL_checknumber(L, 1);
  if((double)(uint32_t)r != r) {
    luaL_error(L, "BCM2835 Error: Invalid argument for range (expected uint32_t).");
  }
  
  uint32_t range = (uint32_t)r;

  if(!range) {
    luaL_error(L, "BCM2835 Error: Range must be greater than 0.");
  }

  pwm_range = range;

  bcm2835_pwm_set_range(0, range);   
  
  return 0;
}

static int l_pwm_write (lua_State *L)
{
  double d = luaL_checknumber(L, 1);
  if((double)(uint32_t)d != d) {
    luaL_error(L, "BCM2835 Error: Invalid argument for data (expected uint32_t).");
  }

  uint32_t data = (uint32_t)d;

  if(data >= pwm_range) {
    luaL_error(L, "BCM2835 Error: PWM data must be below range.");
  }

  bcm2835_pwm_set_data(0, data);    
  
  return 0;
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
void luabcm_register(lua_State *L)
{
  lua_pushcfunction(L, l_delay);
  lua_setglobal(L, "delay");
  lua_pushcfunction(L, l_fsel);
  lua_setglobal(L, "pinMode");
  lua_pushcfunction(L, l_write);
  lua_setglobal(L, "writePin");
  lua_pushcfunction(L, l_lev);
  lua_setglobal(L, "readPin");
  lua_pushcfunction(L, l_pwm_init);
  lua_setglobal(L, "beginPWM");
  lua_pushcfunction(L, l_pwm_set_range);
  lua_setglobal(L, "setPWMRange");
  lua_pushcfunction(L, l_pwm_write);
  lua_setglobal(L, "writePWM");      
  
  lua_pushboolean(L, 1);
  lua_setglobal(L, "ON");
  lua_pushboolean(L, 0);
  lua_setglobal(L, "OFF");
  lua_pushnumber(L, 0);  
  lua_setglobal(L, "INPUT");
  lua_pushnumber(L, 1);
  lua_setglobal(L, "OUTPUT");      
}
