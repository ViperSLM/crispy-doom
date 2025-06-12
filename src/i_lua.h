/* Lua scripting language */

#ifndef _VSLM_LUA_H_
#define _VSLM_LUA_H_
typedef struct lua_State lua_State;
typedef int (*lua_CFunction)(lua_State *);

// Lua VM
extern lua_State *lvm;

void L_Start(void);
void L_Stop(void);
void L_DefaultLibs(void);
void L_LoadMapScriptFromWAD(const char *mapName);

/*
    Used to load game-specific bindings

    Examples:
    (Doom and Doom II): luaopen_doom
    (Heretic): luaopen_heretic
    (Strife): luaopen_strife

    There won't be any bindings for Hexen
    as that game already uses it's own
    scripting language (ACS).
*/
void L_LoadLib(lua_CFunction func);

#define I_InitLua() \
    L_Start(); \
    L_DefaultLibs(); \
    I_AtExit(L_Stop, false)
    
#endif