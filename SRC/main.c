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

#include "LUA/luajit.h"
#include "LUA/lauxlib.h"
#include "LUA/lualib.h"


#ifndef DEFAULT_MAIN
#define DEFAULT_MAIN "main.lua"
#endif


/**
 * print_init - Prints initial messages.
 * 
 * Prints build version and copyright notice.
 */
void print_init()
{
  printf("CirnOS Alpha Version 3.0\n");
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

int abort()
{
  for(;;) {}
}




/**
 * stack_dump - Tests metatables for non-string errors
 * 
 * Author: tilkinsc
 * Dumps out any items on the stack. Able
 * to be called in lua if desired. Useful
 * as an easy `print(item, type(item))`
 * loop.
 */
static int stack_dump(lua_State *L) {
    static int i;
    static int t;
  
  i = lua_gettop(L);
  printf("--------------- Stack Dump ----------------\n");
  while(i) {
    t = lua_type(L, i); // get type number
    switch (t) { // switch type number
      case LUA_TSTRING:
        printf("%d:(String):`%s`\n", i, lua_tostring(L, i));
        break;
      case LUA_TBOOLEAN:
        printf("%d:(Boolean):`%s`\n", i, lua_toboolean(L, i) ? "true" : "false");
        break;
      case LUA_TNUMBER:
        printf("%d:(Number):`%g`\n", i, lua_tonumber(L, i));
        break;
      case LUA_TFUNCTION:
        printf("%d:(Function):`@0x%p`\n", i, lua_topointer(L, i));
        break;
      case LUA_TTABLE:
        printf("%d:(Table):`@0x%p`\n", i, lua_topointer(L, i));
        break;
      case LUA_TUSERDATA:
        printf("%d:(Userdata):`@0x%p`\n", i, lua_topointer(L, i));
        break;
      case LUA_TLIGHTUSERDATA:
        printf("%d:(LUserdata):`0x@%p`\n", i, lua_topointer(L, i));
        break;
      case LUA_TTHREAD:
        printf("%d:(Thread):`0x%p`\n", i, lua_topointer(L, i));
        break;
      case LUA_TNONE:
        printf("%d:(None)\n", i);
        break;
      default:
        printf("%d:(Object):%s:`0x@%p`\n", i, lua_typename(L, t), lua_topointer(L, i));
        break;
    }
    i--;
  }
  printf("----------- Stack Dump Finished -----------\n");
  return 0;
}

/**
 * error_test_meta - Tests metatables for non-string errors
 * 
 * Author: tilkinsc
 * Very important because the environment
 * just might error as a real error could
 * be raised. Probably because variables
 * not being setup properly.
 */
static const char *error_test_meta(const char **out_type) {
    static const char *msg;
    static int meta;
    static int ret;
  
  msg = lua_tostring(L, -1); // attempt tostring
  if(msg == 0) { // if failed
    meta = luaL_callmeta(L, -1, "__tostring"); // call the metatable __tostring
    ret = lua_type(L, -1); 
    if(meta != 0) {
      if(ret == LUA_TSTRING)
        msg = lua_tostring(L, -1);
    } else {
      msg = "Warning: Error return type is ";
      *out_type = luaL_typename(L, -1);
    }
  }
  return msg;
}

/**
 * l_print_error - Verbose, useful error reporting function
 * 
 * Author: tilkinsc
 * Runtime errors need some way of
 * correctly reporting the environment
 * as well as a stack trace, which helps
 * immensely.
 */
static int l_print_error(lua_State *L) {
    static const char *type;
    static const char *msg;
    static const char *tb;
    static size_t top;
  
  msg = error_test_meta(&type);
  lua_pop(L, 1); // err msg
  luaL_traceback(L, L, "--", 1);
  tb = lua_tostring(L, -1);
  top = lua_gettop(L);
  printf(" (Runtime) | Stack Top: %zu | %s%s\n", top, msg, type);
  printf("%s\n", tb);
  if(top > 1)
    stack_dump(L);
  // Note: Shouldn't pop arguments
  return 1;
}

typedef enum LuaError {
  INTERNAL_ERROR = 0,
  SYNTAX_ERROR = 1,
  RUNTIME_ERROR = 2,
} LuaError;

/**
 * print_error - Verbose, useful error reporting function
 * 
 * Author: tilkinsc
 * Runtime errors need some way of
 * correctly reporting the environment
 * as well as a stack trace, which helps
 * immensely. Extendable for later use.
 * `offset` - bool whether err handler
 *   is present.
 */
static void print_error(LuaError error, int offset) {
    static const char *type;
    static const char *msg;
    static size_t top;
  
  msg = error_test_meta(&type);
  switch(error) {
  case INTERNAL_ERROR:
    printf(" (Internal)");
    break;
  case SYNTAX_ERROR:
    printf(" (Syntax)");
    break;
  case RUNTIME_ERROR:
    printf(" (Runtime)");
    break;
  }
  top = lua_gettop(L);
  printf(" | Stack Top: %zu | %s%s\n", top - offset, msg, type);
  if(top - offset > 1)
    stack_dump(L);
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
int notmain()
{
    static int base;
    static int status;
  
  clear_bss();
  
  bcm2835_init();  
  hdmi_init(SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH);
  f_mount(&SDFS, "", 0);
  print_init();   

  // Start Lua
  lua_State *L;
  if ((L = luaL_newstate()) != 0) {
    perror("Error creating Lua state");
    return 0;
  }
  
  luaopen_base(L);
  luaopen_math(L);
  luaopen_string(L);
  luaopen_table(L);
  luaopen_io(L);
  // luaopen_os(L); // unsupported
  luaopen_package(L);
  luaopen_debug(L);
  luaopen_bit(L);
  luaopen_jit(L);
  luaopen_ffi(L);
  
  luabcm_register(L);
  
  
  lua_pushcclosure(L, l_print_error, 0);
  base = lua_gettop(L);
  status = 0;
  if((status = luaL_loadfile(L, DEFAULT_MAIN)) != 0) {
    print_error(SYNTAX_ERROR, 1);
    lua_pop(L, 2); // err msg, err handler
    return 0;
  }
  if((status = lua_pcall(L, 0, 0, base)) != 0) {
    lua_pop(L, 2); // err msg, err handler
    return 0;
  }
  lua_pop(L, 1); // err handler
  
  lua_close(L);
  
  
  // This should only be run on errors. Lua code should end in a loop.
  printf("Warning: Lua code should end in a loop!\n");
  while(bcm2835_delay((uint32_t) 1000));

  return 0;
}
