/*
	Copyright(C) 2025 ViperSLM

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	DESCRIPTION:
	Miscellaneous utilities
*/
#ifndef _VSLM_H_
#define _VSLM_H_
#include <doomtype.h>
#include <deh_str.h>
#include <m_argv.h>
#include <doom/p_mobj.h>
#include <stdint.h>

// Determine whether to use 32 or 64 bits (set via CMake)
#ifdef VSLM_64BIT
#define VSLM_INTSIZE uint64_t
#else
#define VSLM_INTSIZE uint32_t
#endif

// Debug macro
#define VSLM_DEBUG(x, ...) \
	if (M_CheckParm("-vslmdebug")) \
		DEH_printf("[VSLM DEBUG]: "); \
		DEH_printf(x, ##__VA_ARGS__); \
		DEH_printf("\n")

/* ----------------------------------- */
/* Random Number Generator ----------- */
/* ----------------------------------- */

// Set seed for the generator
void VSLM_SetRandomSeed(VSLM_INTSIZE seed);

// Generate random integer within range
int VSLM_Rand(int min, int max);

// Replacement function for P_Random/M_Random
int VSLM_DoomRand(void);

/* ----------------------------------- */
/* Map Functions ------------------- */
/* ----------------------------------- */

// Trigger Tag 666/667 events in current map
void VSLM_TriggerTag666(boolean tag667);

/* ----------------------------------- */
/* Enemy Functions ------------------- */
/* ----------------------------------- */

// Respawn a monster as if being resurrected by an Arch-vile.
// Falls back to Nightmare-style respawning if Mobj pointer 
// doesn't have a raise state.
void VSLM_RespawnMonster(mobj_t *actor);

// Resurrect all monsters in the current map.
// Returns the total amount of monsters revived.
int VSLM_ReviveMonsters(void);

// Shortcut for spawning fire (MT_SPAWNFIRE).
// Does nothing in DOOM/Ultimate DOOM.
void VSLM_SpawnFire(mobj_t *actor);

// Is Mobj an enemy?
boolean VSLM_IsMonster(mobj_t *actor);

// Has a monster been gibbed?
boolean VSLM_IsMonsterGibbed(mobj_t *actor);

// Get monster type
const char *VSLM_GetMonsterType(mobjtype_t type);

// Is specific monster found in map?
boolean VSLM_MonsterFound(mobjtype_t type);

// Change Mobj to selected type
void VSLM_ChangeMonsterType(mobj_t *actor, mobjtype_t type);

/* ----------------------------------- */
/* Randomizer Functions -------------- */
/* ----------------------------------- */

// Randomize all monsters in current map.
// 'spawnfog' variable spawns a fire effect
// (or teleport fog in Doom 1) as a monster
// is randomized.
// Returns the amount of monsters randomized.
int VSLM_RandomizeMonsters(boolean spawnfog);

// Set randomizer pool pointer to NULL
// Doom's zone memory allocator takes
// care of freeing this (PU_LEVEL).
void VSLM_ClearRandoPool(void);

// Select random monster in map
mobj_t *VSLM_Rando_GetMonster(mobjtype_t *type);

#endif
