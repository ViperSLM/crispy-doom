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
	Randomizer tiers
*/
#ifndef _VSLM_DOOM_RANDO_TIERS_H_
#define _VSLM_DOOM_RANDO_TIERS_H_

#include <doom/info.h>

/* Enemy tiers */
// Tier 1 [Common]: Zombieman, Sergeant, Imp
// (D2: Chaingunner)
const mobjtype_t tier1[] = {
    MT_POSSESSED,
    MT_SHOTGUY,
    MT_TROOP,
};

// Tier 2 [Uncommon]: Pinky, Spectre, Lost Soul, Cacodemon
// (D2: Hell Knight)
const mobjtype_t tier2[] = {
    MT_SERGEANT,
    MT_SHADOWS,
    MT_SKULL,
    MT_HEAD,
    
    // Doom 2
    MT_KNIGHT,
};

// Tier 3 [Rare]: Baron of Hell
// (D2: Arachnotron, Pain Elemental)
const mobjtype_t tier3[] = {
    MT_BRUISER,

    // Doom 2
    MT_BABY,
    MT_PAIN,
};

// Tier 4 [Very Rare, D2 only]: Revenant, Mancubus
const mobjtype_t tier4[] = {
    MT_UNDEAD,
    MT_FATSO,
};

// Tier 5 [Extremely Rare, D2 only]: Arch-vile
const mobjtype_t tier5[] = {MT_VILE};

// Bosses [Legendary]: Spider Mastermind, Cyberdemon
const mobjtype_t boss[] = {MT_SPIDER, MT_CYBORG};

#endif