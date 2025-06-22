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
	New randomizer logic
*/
#ifndef _VSLM_DOOM_RANDO_LOGIC_H_
#define _VSLM_DOOM_RANDO_LOGIC_H_

#include <doomtype.h>
#include <doom/p_mobj.h>

/* ----------------------------------- */
/* Definitions ----------------------- */
/* ----------------------------------- */

// Enable the randomizer?
extern boolean RANDOMIZER;

typedef enum {
    R_TIER1  = 0,    // Doom/Doom 2
    R_TIER2  = 1,    // Doom/Doom 2
    R_TIER3  = 2,    // Doom/Doom 2
    R_TIER4  = 3,    // Doom 2 only
    R_TIER5  = 4,    // Doom 2 only
    R_BOSS   = 5,    // Doom/Doom 2
    R_SECRET = 6,    // Doom 2 only
    R_NULL   = 7
} randotier_t;

typedef struct {
    mobj_t **enemies;
    size_t length;
    size_t index;
} randopool_enemy_t;

typedef struct {
    uint16_t t2, t3,
    t4, t5, boss;
} odds_t;

/* ----------------------------------- */
/* Function Definitions -------------- */
/* ----------------------------------- */

// Randomize all enemies in the map
void VSLM_StartEnemyRandomizer(void);

#endif