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
  switch (pin) {
  case 6:
  case 9:
  case 14:
  case 20:
  case 25:
  case 30:
  case 34:
  case 39:
    luaL_error(L, "BCM2835 Error: Invalid GPIO pin number.");        
  default:
    break;
  }

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
  switch (pin) {
  case 6:
  case 9:
  case 14:
  case 20:
  case 25:
  case 30:
  case 34:
  case 39:
    luaL_error(L, "BCM2835 Error: Invalid GPIO pin number.");        
  default:
    break;
  }

  if(lua_type(L, 2) != LUA_TBOOLEAN) {
    luaL_error(L, "BCM2835 Error: Invalid type for status (expected boolean).");
  }

  int k = lua_toboolean(L, 2);

  if(k) bcm2835_gpio_clr(pin);
  else bcm2835_gpio_set(pin);
  
  return 0;
}

void luabcm_register(lua_State *L) {
  lua_pushcfunction(L, l_delay);
  lua_setglobal(L, "delay");
  lua_pushcfunction(L, l_fsel);
  lua_setglobal(L, "pinmode");
  lua_pushcfunction(L, l_write);
  lua_setglobal(L, "pinwrite");  
}
