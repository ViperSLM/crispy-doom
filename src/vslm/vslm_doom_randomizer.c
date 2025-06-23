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
	Randomizer - Main source file
*/
#include "vslm_doom_randomizer.h"

/* ----------------------------------- */
/* Function Prototypes --------------- */
/* ----------------------------------- */

// Enemy randomizer
int VSLM_Rando_EnemyRandomizer(boolean spawnfx);
void VSLM_Rando_ClearEnemyPool(void);

/* ----------------------------------- */
/* Global Variables ------------------ */
/* ----------------------------------- */
boolean RANDOMIZER, RAND_CHAOS;


/* ----------------------------------- */
/* Function Implementation ----------- */
/* ----------------------------------- */

// -----------------------------
// VSLM_StartRandomizer
// -----------------------------
void VSLM_StartRandomizer(boolean spawnfx, int *count)
{
    int enemycount = VSLM_Rando_EnemyRandomizer(spawnfx);

    if(count)
        *count = enemycount;
}

// -----------------------------
// VSLM_ClearRandomizer
// -----------------------------
void VSLM_ClearRandomizer(void)
{
    VSLM_Rando_ClearEnemyPool();
}