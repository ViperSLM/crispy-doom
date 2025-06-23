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
	Randomizer - Main header
*/
#ifndef _VSLM_DOOM_RANDO_H_
#define _VSLM_DOOM_RANDO_H_
#include <doomtype.h>
#include <doom/info.h>

/* ----------------------------------- */
/* Definitions ----------------------- */
/* ----------------------------------- */

// Determines whether to enable
// the randomizer or not
extern boolean RANDOMIZER;

// Chaos mode. When set to true, monsters (except bosses)
// are randomized regardless of their tier.
extern boolean RAND_CHAOS;


/* ----------------------------------- */
/* Function Definitions -------------- */
/* ----------------------------------- */

// Starts the randomizer by calling the related
// functions.
void VSLM_StartRandomizer(boolean spawnfx, int *count);

// Clean up memory before a level loads
void VSLM_ClearRandomizer(void);

#endif