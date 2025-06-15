/* Stub out all Lua functions if disabled from CMake */
#include "i_lua.h"

void L_Start(void) {}
void L_Stop(void) {}
void L_DefaultLibs(void) {}
void L_LoadMapScript(const char *mapName) {}

void L_LinedefSwitchActivate(line_t *line) {}
void L_LoadLib(lua_CFunction func) {}

// l_doom
int L_G_ExitLevel(lua_State *L) { return 0; }
int luaopen_doom(lua_State *L) { return 0; }