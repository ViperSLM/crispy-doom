/* Lua scripting language */

#ifndef _VSLM_LUA_H_
#define _VSLM_LUA_H_
typedef struct lua_State lua_State;

// Lua VM
extern lua_State *lvm;

void L_Start(void);
void L_Stop(void);
void L_DefaultLibs(void);

#define I_InitLua() \
    L_Start(); \
    L_DefaultLibs(); \
    I_AtExit(L_Stop, false)

#endif