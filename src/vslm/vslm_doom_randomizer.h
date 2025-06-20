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
	Things used with randomizer functions (e.g. tier lists, flags, etc.)
*/
#ifndef _VSLM_DOOM_RANDO_H_
#define _VSLM_DOOM_RANDO_H_
#include <doomtype.h>
#include <doom/info.h>

#include <stdint.h>

// Determines whether to enable
// the randomizer or not
extern boolean RANDOMIZER;

// Chaos mode. When set to true, monsters (except bosses)
// are randomized regardless of their tier.
extern boolean RAND_CHAOS;

typedef struct {
    uint16_t tier1;
    uint16_t tier2;
    uint16_t tier3;
    uint16_t tier4;
    uint16_t tier5;
    uint16_t boss;
    uint16_t secret;
} enemycount_t;

// Enemy randomizer pool. This is
// mainly filled with tier 1 enemies.
// However for Chaos mode, everything is
// added into the pool (except tier 5 and bosses)
typedef struct
{
    mobjtype_t *pool;
    int poolsize;
    enemycount_t count;
} r_enemypool_t;

typedef struct
{
    int t2, t3, t4, t5, boss;
} odds_t;

#endif