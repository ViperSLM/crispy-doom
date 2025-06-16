/* Stub out Lua functions if disabled from CMake */
#include "i_lua.h"

void L_Start(void) {}
void L_Stop(void) {}
void L_Setup(void) {}
void L_DefaultLibs(void) {}
void L_LoadScript(const char *mapName) {}
void L_RunMainFunction(void) {}

void L_LoadLib(lua_CFunction func) {}

void L_Event_LinedefSwitchActivate(line_t *line) {}
void L_Event_MapLoad(void) {}
