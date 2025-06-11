/* Lua scripting support */
#include "i_lua.h"
#include "i_system.h"
#include "deh_str.h"

#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

lua_State *lvm;

int L_Print(lua_State *L);

void L_Start (void)
{
    DEH_printf("L_Start: Starting Lua VM\n");
    lvm = luaL_newstate();

    if(!lvm)
        I_Error("L_Start: Unable to start Lua VM!");
}

void L_Stop (void)
{
    DEH_printf("L_Stop: Stopping Lua VM\n");
    lua_close(lvm);
}

void L_DefaultLibs (void)
{
    const luaL_Reg defaultLibs[] = {
        {"", luaopen_base},
        {LUA_MATHLIBNAME, luaopen_math},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_TABLIBNAME, luaopen_table},
        {LUA_LOADLIBNAME, luaopen_package},
        {LUA_BITLIBNAME, luaopen_bit},
        {LUA_JITLIBNAME, luaopen_jit},
        {LUA_FFILIBNAME, luaopen_ffi},
        {NULL, NULL}
    };

    // Override default functions
    const luaL_Reg overrideLib[] = {
        {"print", L_Print},
        {NULL,NULL}
    };

    int i;
    for (i = 0; defaultLibs[i].func != NULL; i++)
    {
        DEH_printf("L_DefaultLibs: Registering default library '%s'\n",
                   (i == 0) ? "base" : defaultLibs[i].name);
        lua_pushcfunction(lvm, defaultLibs[i].func);
        lua_pushstring(lvm, defaultLibs[i].name);
        lua_call(lvm, 1, 0);
    }

    lua_getglobal(lvm, "_G");
    luaL_setfuncs(lvm, overrideLib, 0);
    lua_pop(lvm, 1);
}

// Overriding functions
int L_Print(lua_State *L)
{
    const char *in = luaL_checkstring(L, 1);
    DEH_printf("%s\n", in);
    return 0; // Don't return anything to Lua
}