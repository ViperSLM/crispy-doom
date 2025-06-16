/* Lua scripting language */

#ifndef _VSLM_LUA_H_
#define _VSLM_LUA_H_
typedef struct lua_State lua_State;
typedef int (*lua_CFunction)(lua_State *);
typedef struct line_s line_t;

// Lua VM
extern lua_State *lvm;

void L_Start(void);
void L_Stop(void);
void L_Setup(void); // Different, depending on the game
void L_DefaultLibs(void);
void L_LoadScript(const char *script);
void L_RunMainFunction(void); // Different, depending on the game

/* -- Lua events -- */

// Run OnSwitchActivate (output)
void L_Event_LinedefSwitchActivate(line_t *line);

// Run OnMapLoad
void L_Event_MapLoad(void);

/* ---------------- */

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

/*
    Load Lua script file, either from a
    WAD file or loose
*/


#define I_InitLua() \
    L_Start(); \
    L_DefaultLibs(); \
    L_Setup(); \
    L_LoadScript("MAIN"); \
    L_RunMainFunction(); \
    I_AtExit(L_Stop, false)
    

#define Lua_Include()
#endif