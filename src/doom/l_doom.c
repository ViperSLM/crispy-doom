// Lua bindings
#include <luajit.h>
#include <lauxlib.h>

#include "deh_str.h"
#include "g_game.h"

// _G functions
int L_G_ExitLevel(lua_State *L)
{
    // Exits current level
    G_ExitLevel();
    return 0;
}

static const luaL_Reg doomLib[] = {
    {"G_ExitLevel", L_G_ExitLevel},
    {NULL, NULL}
};

int luaopen_doom(lua_State *L)
{
    DEH_printf("luaopen_doom: Loaded Doom bindings for Lua\n");
    luaL_newlib(L, doomLib);
    lua_setglobal(L, "Doom");
    return 1;
}