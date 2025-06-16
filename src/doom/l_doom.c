// Lua bindings
#ifdef USE_LUAJIT
#include <luajit.h>
#else
#include <lua.h>
#endif
#include <lauxlib.h>

#include "deh_str.h"
#include "sounds.h"
#include "s_sound.h"
#include "g_game.h"
#include "r_defs.h"

void ST_PrintMsg(const char *format, ...);

extern lua_State *lvm;
extern void L_LoadLib(lua_CFunction func);
extern void L_LoadScript(const char *script);

int luaopen_doom(lua_State *L);

// Setup function
void L_Setup(void)
{
    L_LoadScript("DOOM");
    L_LoadLib(luaopen_doom);
}

void L_RunMainFunction(void)
{
    lua_getglobal(lvm, "DoomMain");
    if(lua_isfunction(lvm, -1))
        lua_pcall(lvm, 0, 0, 0);
}

int L_G_ExitLevel(lua_State *L)
{
    // Exits current level
    G_ExitLevel();
    return 0;
}

int L_S_ChangeMusic(lua_State *L)
{
    int musicid = lua_tointeger(lvm, 1);
    int looping = lua_tointeger(lvm, 2);
    S_ChangeMusic(musicid, looping);
    S_StartMusic(musicid);
    return 0;
}

int L_ST_PrintMsg(lua_State *L)
{
    const char *msg = luaL_checkstring(lvm, 1);
    ST_PrintMsg("%s", msg);
    return 0;
}

static const luaL_Reg doomLib[] = {
    {"G_ExitLevel", L_G_ExitLevel},
    {"S_ChangeMusic", L_S_ChangeMusic},
    {"PrintToHUD", L_ST_PrintMsg},
    {NULL, NULL}
};

int luaopen_doom(lua_State *L)
{
    DEH_printf("luaopen_doom: Loaded Doom bindings for Lua\n");
    luaL_newlib(L, doomLib);
    lua_setglobal(L, "Doom");
    return 1;
}

/* --- Lua Events --- */

void L_Event_MapLoad(void)
{
    lua_getglobal(lvm, "OnMapLoad");
    if (lua_isfunction(lvm, -1))
        lua_pcall(lvm, 0, 0, 0);
}

void L_Event_LinedefSwitchActivate(line_t *line)
{
    lua_getglobal(lvm, "OnSwitchActivate");
    if (lua_isfunction(lvm, -1)) // Ensure global is a function
    {
        // Push linedef tag as first arg
        lua_pushinteger(lvm, line->tag);

        // Run
        lua_pcall(lvm, 1, 0, 0);
    }
}
/* ------------------ */
