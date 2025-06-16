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

void ST_PrintMsg(const char *format, ...);

extern lua_State *lvm;
extern void L_LoadLib(lua_CFunction func);
extern void L_LoadScript(const char *script);

int luaopen_doom(lua_State *L);

// Setup function
void L_Setup(void)
{
    L_LoadLib(luaopen_doom);
    L_LoadScript("DOOM");
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

int L_P_SpawnMobj(lua_State *L)
{
    fixed_t x,y,z;
    mobjtype_t type;
    mobj_t *actor; // Fog is used for the teleport effect
    subsector_t *subsector;

    x = lua_tointeger(L, 1);
    y = lua_tointeger(L, 2);
    z = ONFLOORZ;
    type = lua_tointeger(L, 3);
    //z = lua_tointeger(L, 3); // Maybe Z can be overidden

    subsector = R_PointInSubsector(x, y);
    actor = P_SpawnMobj(x, y, subsector->sector->floorheight, MT_TFOG);
    S_StartSound(actor, sfx_telept);

    actor = P_SpawnMobj(x, y, z, type);

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
    line.tag = lua_tointeger(lvm, 1);
    EV_DoDoor(&line, vld_open);
    return 0;
}

int L_Linedef_CloseDoor(lua_State *L)
{
    line_t line;
    // Get linedef tag from input
    line.tag = lua_tointeger(lvm, 1);
    EV_DoDoor(&line, vld_close);
    return 0;
}

int L_Sector_LightLevel(lua_State *L)
{
    line_t line;
    int lightlevel;
    line.tag = lua_tointeger(lvm, 1);       // Sector tag
    lightlevel = lua_tointeger(lvm, 2); // Light level to change to

    EV_LightTurnOn(&line, lightlevel);
    if(M_CheckParm("-vslmdebug"))
    {
        DEH_printf("[VSLM DEBUG] L_Sector_Lightlevel: Tag = %d, Light level = %d\n", line.tag, lightlevel);
    }
    return 0;
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
