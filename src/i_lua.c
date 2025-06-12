/* Lua scripting support */
#include "i_lua.h"
#include "i_system.h"
#include "deh_str.h"
#include "z_zone.h"

#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

extern int M_snprintf(char *buf, size_t buf_len, const char *s, ...);
extern void *W_CacheLumpName(const char *name, int tag);
extern void W_ReleaseLumpName(const char *name);
extern int M_CheckParm(const char *check);

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

void L_LoadLib(lua_CFunction func)
{
    lua_pushcfunction(lvm, func);
    lua_pcall(lvm, 0, 1, 0);
}

void L_LoadMapScriptFromWAD(const char *mapName)
{
    char filename[8];
    char *scriptlump;
    M_snprintf(filename, sizeof(filename), "L_%s", mapName);
    DEH_printf("L_LoadMapScriptFromWAD: Loading Lua script '%s'\n", filename);

    // Script lump is only needed to load the script into the Lua VM
    scriptlump = W_CacheLumpName(filename, PU_LEVEL);
    
    if(M_CheckParm("-vslmdebug"))
        DEH_printf("[VSLM DEBUG] L_LoadMapScriptFromWAD: Contents of %s:\n\n%s\n", filename, scriptlump);

    if(luaL_dostring(lvm, scriptlump) != LUA_OK)
        DEH_printf("L_LoadMapScriptFromWAD: Lua Error - %s\n", lua_tostring(lvm, -1));
    else
    {
        // Run the OnMapLoad function
        lua_getglobal(lvm, "OnMapLoad");
        if (lua_isfunction(lvm, -1)) // OnMapLoad is actually a function
        {
            // OnMapLoad doesn't take any input arguments nor does
            // it return anything
            lua_pcall(lvm, 0, 0, 0);
            W_ReleaseLumpName(filename);
        }
    }

    /*
    M_snprintf(scriptname, sizeof(scriptname), "L_%s", maplumpinfo->name);
    DEH_printf("P_SetupLevel: Loading Lua script '%s'\n", scriptname);

    scriptlump = W_CacheLumpName(scriptname, PU_CACHE);

    if(M_CheckParm("-vslmdebug"))
        DEH_printf("[VSLM DEBUG] Contents of %s:\n\n%s\n", scriptname, scriptlump);

    if (luaL_dostring(lvm, scriptlump) != LUA_OK)
    {
        DEH_printf("!! LUA ERROR: %s\n", lua_tostring(lvm, -1));
    }
    else
    {
        // Run the OnMapLoad function
        lua_getglobal(lvm, "OnMapLoad"); // Get 'OnMapLoad' function on the stack
        if (lua_isfunction(lvm))
    }
    */
}

// Overriding functions
int L_Print(lua_State *L)
{
    const char *in = luaL_checkstring(L, 1);
    DEH_printf("[Lua] %s\n", in);
    return 0; // Don't return anything to Lua
}