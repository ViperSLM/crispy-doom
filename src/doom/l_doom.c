// Lua bindings
#ifdef USE_LUAJIT
#include <luajit.h>
#else
#include <lua.h>
#endif
#include <lauxlib.h>

#include "deh_str.h"
#include "m_argv.h"
#include "p_local.h"
#include "p_spec.h"
#include "s_sound.h"
#include "g_game.h"
#include "r_defs.h"

#include <vslm.h>

void ST_PrintMsg(const char *format, ...);
int M_snprintf(char *buf, size_t buf_len, const char *s, ...);

extern lua_State *lvm;
extern void L_LoadLib(lua_CFunction func);
extern void L_LoadScript(const char *script, const char *entry);

int luaopen_doom(lua_State *L);

// Setup function
void L_Setup(void)
{
    L_LoadLib(luaopen_doom);
}

void L_RunMainFunction(void)
{
    lua_getglobal(lvm, "DoomMain");
    if(lua_isfunction(lvm, -1))
        lua_pcall(lvm, 0, 0, 0);
}

/* ------ Doom function bindings ------ */
int L_G_ExitLevel(lua_State *L)
{
    // Exits current level
    G_ExitLevel();
    return 0;
}

int L_S_ChangeMusic(lua_State *L)
{
    int musicid = (musicenum_t) lua_tointeger(lvm, 1);
    int looping = (int) lua_tointeger(lvm, 2);
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

int L_P_SpawnMobj(lua_State *L)
{
    fixed_t x,y;
    mobjtype_t type;
    mobj_t *actor; // Fog is used for the teleport effect
    subsector_t *subsector;

    x = (fixed_t)lua_tointeger(L, 1);
    y = (fixed_t)lua_tointeger(L, 2);
    type = (mobjtype_t) lua_tointeger(L, 3);
    //z = lua_tointeger(L, 3); // Maybe Z can be overidden

    subsector = R_PointInSubsector(x, y);
    actor = P_SpawnMobj(x, y, subsector->sector->floorheight, MT_TFOG);
    S_StartSound(actor, sfx_telept);

    actor = P_SpawnMobj(x, y, subsector->sector->floorheight, type);

    lua_pushinteger(L, actor->x);
    lua_pushinteger(L, actor->y);
    lua_pushinteger(L, actor->z);
    lua_pushinteger(L, actor->angle);
    lua_pushinteger(L, actor->type);
    lua_pushlightuserdata(L, actor);
    return 6;
}

// LineDef bindings
int L_Linedef_OpenDoor(lua_State *L)
{
    line_t line;
    // Get linedef tag from input
    line.tag = (short)lua_tointeger(lvm, 1);
    EV_DoDoor(&line, vld_open);
    return 0;
}

int L_Linedef_CloseDoor(lua_State *L)
{
    line_t line;
    // Get linedef tag from input
    line.tag = (short)lua_tointeger(lvm, 1);
    EV_DoDoor(&line, vld_close);
    return 0;
}

int L_Sector_LightLevel(lua_State *L)
{
    line_t line;
    int lightlevel;
    line.tag = (short)lua_tointeger(lvm, 1); // Sector tag
    lightlevel = (int)lua_tointeger(lvm, 2); // Light level to change to

    EV_LightTurnOn(&line, lightlevel);
    VSLM_DEBUG("L_Sector_Lightlevel: Tag = %d, Light level = %d", line.tag, lightlevel);
    return 0;
}

// Monster randomizer (this could be fun)
int L_Map_RandomizeMonsters(lua_State* L)
{
    boolean spawnfog = lua_toboolean(L, 1);
    int count = VSLM_RandomizeMonsters(spawnfog);

    lua_pushinteger(lvm, count);
    return 1;
}

// Clear randomizer pool
int L_Map_ClearRandoPool(lua_State* L)
{
    VSLM_ClearRandoPool();
    return 0;
}

int L_Map_ToggleRandoChaos(lua_State* L)
{
    RAND_CHAOS = (!RAND_CHAOS) ? true : false;
    
    char msg[64];
    M_snprintf(msg, sizeof(msg), "Enemy Randomizer: Chaos mode %s",
               (RAND_CHAOS) ? "ON" : "OFF");

    lua_getglobal(L, "print");
    if (lua_isfunction(L, -1))
    {
        lua_pushstring(L, msg);
        lua_pcall(L, 1, 0, 0);
    }

    return 0;
}

int L_Map_GetRandoChaos(lua_State* L)
{
    lua_pushboolean(L, RAND_CHAOS);
    return 1;
}

/* ------------------------------------ */

static const luaL_Reg doomLib[] = {
    {"G_ExitLevel", L_G_ExitLevel},
    {"S_ChangeMusic", L_S_ChangeMusic},
    {"P_SpawnMobj", L_P_SpawnMobj},
    {"PrintHUD", L_ST_PrintMsg},
    {"OpenDoor", L_Linedef_OpenDoor},
    {"CloseDoor", L_Linedef_CloseDoor},
    {"SectorLightLevel", L_Sector_LightLevel},
    {"RandomizeMonsters", L_Map_RandomizeMonsters},
    {"ClearRandoPool", L_Map_ClearRandoPool},
    {"ToggleRandoChaos", L_Map_ToggleRandoChaos},
    {"GetRandoChaos", L_Map_GetRandoChaos},
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

void L_Event_GlobalMapLoad(void)
{
    lua_getglobal(lvm, "OnGlobalMapLoad");
    if (lua_isfunction(lvm, -1))
        lua_pcall(lvm, 0, 0, 0);
}


void L_Event_MapExit(void)
{
    lua_getglobal(lvm, "OnMapExit");
    if (lua_isfunction(lvm, -1))
        lua_pcall(lvm, 0, 0, 0);
}

void L_Event_GlobalMapExit(void)
{
    lua_getglobal(lvm, "OnGlobalMapExit");
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
