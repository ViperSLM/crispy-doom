/* Lua scripting support */
#include "deh_str.h"
#include "i_lua.h"
#include "i_system.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

extern int M_CheckParm(const char *check);

lua_State *lvm;

// Override print function
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

void L_LoadMapScript(const char *mapName)
{
    char filename[8];
    char filenameloose[sizeof(filename)+4]; // Extra size is for extension
    char *scriptlump;
    int lumpnum;
    FILE *looseFilePtr;
    size_t looseFileLen;
    size_t readbytes;

    // Initialise variables
    scriptlump = NULL;
    lumpnum = 0;
    looseFilePtr = NULL;
    looseFileLen = 0;
    readbytes = 0;

    M_snprintf(filename, sizeof(filename), "L_%s", mapName);

    lumpnum = W_CheckNumForName(filename);

    // Does a script exist for the map?
    if(lumpnum != -1)
    {
        DEH_printf("L_LoadMapScript: Loading Lua script '%s'\n", filename); 
        scriptlump = W_CacheLumpName(filename, PU_CACHE);
    }
    else 
    {
        DEH_printf("L_LoadMapScript: Cannot find '%s' lump in WAD. Attempting "
                   "to load it loose\n",
                   filename);
        M_snprintf(filenameloose, sizeof(filenameloose), "%s.lua", filename);
        looseFilePtr = fopen(filenameloose, "r");
        if(!looseFilePtr) // Not found
        {
            DEH_printf("L_LoadMapScript: Cannot load script '%s'. Will not "
                       "continue trying.\n",
                       filename);
            return;
        }

        // Successfully loaded?
        looseFileLen = M_FileLength(looseFilePtr);
        scriptlump = Z_Malloc(looseFileLen + 1, PU_STATIC, NULL);

        readbytes = fread(scriptlump, 1, looseFileLen, looseFilePtr);
        if(readbytes != looseFileLen)
        {
            fclose(looseFilePtr);
            DEH_printf("L_LoadMapScript: Cannot load script '%s'. Will not "
                       "continue trying. (Couldn't read file)\n",
                       filename);
            return;
        }

        // Null-terminate
        scriptlump[looseFileLen] = '\0';
        DEH_printf("L_LoadMapScript: Successfully loaded loose script '%s'.\n", filenameloose);
    }


    // Script lump is only needed to load the script into the Lua VM
    if (luaL_dostring(lvm, scriptlump) != LUA_OK)
    {
        DEH_printf("L_LoadMapScript: Lua Error(s) found in "
                   "script '%s'\nContents of script:\n%s\n%s\n\n",
                   filename, scriptlump, lua_tostring(lvm, -1));
        if (M_CheckParm("-vslmdebug"))
        {
            // Crash on Lua error if debug flag is enabled
            I_Error("[VSLM DEBUG] - L_LoadMapScript:: Errors found in Lua "
                    "script (see above).");
        }
    }
    else
    {
        // Run the OnMapLoad function
        lua_getglobal(lvm, "OnMapLoad");
        if (lua_isfunction(lvm, -1)) // OnMapLoad is actually a function
        {
            lua_pcall(lvm, 0, 0, 0);
        }
    }
    // If file was loaded loose, close the file and free up memory
    if(looseFilePtr && scriptlump)
    {
        fclose(looseFilePtr);
        Z_Free(scriptlump);
    }
}

// Overriding functions
int L_Print(lua_State *L)
{
    const char *in = luaL_checkstring(L, 1);
    DEH_printf("[Lua] %s\n", in);
    return 0; // Don't return anything to Lua
}

// Moved into l_doom.c
/*
int L_LinedefSwitchActivate(line_t *line)
{
    lua_getglobal(lvm, "OnSwitchActivate");
    if(lua_isfunction(lvm, -1)) // Make sure it's a function
    {
        // Push tag number as first argument
        lua_pushinteger(lvm, line->tag);
        
        // Run the function
        lua_pcall(lvm, 1, 0, 0);
    }
}
*/