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
	Xorshift random number generator
*/
#include "vslm.h"

#include <deh_str.h>
#include <i_system.h>
#include <z_zone.h>

#include <time.h>

/* -- Function prototypes -- */

void VSLM_FreeSeed(void);

/* ------------------------- */

#ifdef VSLM_64BIT
VSLM_INTSIZE rol64(VSLM_INTSIZE x, int k)
{
    return (x << k) | (x >> (64 - k));
}
#endif

typedef struct
{
#ifdef VSLM_64BIT
    // xoshiro256++
    VSLM_INTSIZE s[4];
#else
    // xorwow
    VSLM_INTSIZE x[5];
    VSLM_INTSIZE counter;
#endif
} xorstate_t;

xorstate_t *rngstate;

void VSLM_SetRandomSeed(VSLM_INTSIZE seed)
{
    int i;
    VSLM_INTSIZE t;
    if (!rngstate)
    {
        DEH_printf("VSLM_SetRandomSeed: Allocating memory for random seed "
                   "(Size: %lu bytes)\n",
                   sizeof(xorstate_t));
        rngstate = Z_Malloc(sizeof(xorstate_t), PU_STATIC, NULL);
        I_AtExit(VSLM_FreeSeed, false);
    }

    #ifdef VSLM_64BIT
    DEH_printf("VSLM_SetRandomSeed: Setting random seed to %llu\n", seed);
    for (i = 0; i < 4; i++)
    {
        // Golden ratio-related constant
        seed += 0x9e3779b97f4a7c15;
        t = seed;
        t ^= t >> 30;
        t *= 0xbf58476d1ce4e5b9;
        t ^= t >> 27;
        t *= 0x94d049bb133111eb;
        t ^= t >> 31;
        rngstate->s[i] = t;
    }
#else
    DEH_printf("VSLM_SetRandomSeed: Setting random seed to %u\n", seed);
    for (i = 0; i < 5; i++)
    {
        // Golden ratio-related constant
        seed += 0x9e3779b9;
        t = seed;
        t ^= t >> 16;
        t *= 0x85ebca6b;
        t ^= t >> 13;
        t *= 0xc2b2ae35;
        t ^= t >> 16;
        rngstate->x[i] = t;
    }
    // Set counter
    seed += 0x9e3779b9;
    uint32_t t = seed;
    t ^= t >> 16;
    t *= 0x85ebca6b;
    t ^= t >> 13;
    t *= 0xc2b2ae35;
    t ^= t >> 16;
    rngstate->counter = t;
#endif
}

// Xorshift algorithm (xoshiro256++ for 64-bit, xorwow for 32-bit)
VSLM_INTSIZE VSLM_XOR_Shift(void)
{
#ifdef VSLM_64BIT
    VSLM_INTSIZE *s;
    VSLM_INTSIZE result, t;
#else
    VSLM_INTSIZE t, s;
#endif

	// Generate random seed on first run
    if (!rngstate)
    {
        VSLM_SetRandomSeed((VSLM_INTSIZE) time(NULL));
    }

#ifdef VSLM_64BIT
    /* xoshiro256++ algorithm */
    s = rngstate->s;
    result = rol64(s[0] + s[3], 23) + s[0];
    t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rol64(s[3], 45);

    return result;
#else
    /* xorwow algorithm */
    t = rngstate->x[4];

    // Contrived 32-bit rotate
    s = rngstate->x[0];
    rngstate->x[4] = rngstate->x[3];
    rngstate->x[3] = rngstate->x[2];
    rngstate->x[2] = rngstate->x[1];
    rngstate->x[1] = s;

    t ^= t >> 2;
    t ^= t << 1;
    t ^= s ^ (s << 4);
    rngstate->x[0] = t;
    rngstate->counter += 362437;
    return t + rngstate->counter;
#endif
}

int VSLM_Rand(int min, int max)
{
    return min + (VSLM_XOR_Shift() % (max - min + 1));
}

int VSLM_DoomRand(void)
{
    return VSLM_Rand(0, 255);
}

/* --- Internal functions --- */

// De-allocate rngState pointer
void VSLM_FreeSeed(void)
{
    DEH_printf("VSLM_FreeSeed: Deallocating random seed.\n");
    Z_Free(rngstate);
}

/* -------------------------- */
