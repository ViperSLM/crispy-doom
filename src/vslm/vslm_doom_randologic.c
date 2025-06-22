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
	Randomizer logic (replacing the old algorithm)
*/
#include "vslm.h"
#include "vslm_doom_randologic.h"
#include "vslm_doom_randotiers.h"

#include <doom/doomstat.h>
#include <doom/d_think.h>
#include <doom/p_local.h>
#include <doom/sounds.h>
#include <doom/s_sound.h>
#include <deh_str.h>
#include <i_system.h>
#include <z_zone.h>

/* ----------------------------------- */
/* Function Prototypes --------------- */
/* ----------------------------------- */

void VSLM_Rando_AllocateEnemyPool(int size);
void VSLM_Rando_ClearEnemyPool(void);
void VSLM_Rando_AddEnemy(mobj_t *actor);
void VSLM_Rando_PopulateEnemyPool(void);
void VSLM_Rando_UpdateEnemyTiers(void);
void VSLM_Rando_PrintEnemyStats(void);
void VSLM_Rando_PreserveEnemyType(mobjtype_t actortype);
void VSLM_Rando_SetEnemyOdds(void);
void VSLM_Rando_SetTierOdds(uint16_t t2, uint16_t t3, uint16_t t4, uint16_t t5,
                            uint16_t boss);
const char *VSLM_Rando_GetTierStr(randotier_t t);
randotier_t VSLM_Rando_GetEnemyTier(mobj_t *actor);
boolean VSLM_Rando_MapContainsTag666(boolean *softlock, mobjtype_t *actortype,
                                     mobjtype_t *actortype2);
int VSLM_Rando_CountEnemy(mobjtype_t actortype);
mobjtype_t VSLM_Rando_DetermineType(void);

/* ----------------------------------- */
/* Global Variables ------------------ */
/* ----------------------------------- */

boolean RANDOMIZER;
boolean RAND_CHAOS;
randopool_enemy_t enemypool;
odds_t randoenemy_odds;

// T1, T2, T3, T4, T5, B, S (aligns with randotier_t enum)
const int randocountsize = 7;
int randocount[randocountsize];

/* ----------------------------------- */
/* Function Implementation ----------- */
/* ----------------------------------- */

void VSLM_StartEnemyRandomizer(void)
{
    int count, i;
    mobj_t *actor;
    mobjtype_t bosstype[] = {MT_NULL, MT_NULL};
    boolean bossmap = false, bossmap_softlock_risk = false;

    bossmap = VSLM_Rando_MapContainsTag666(
        &bossmap_softlock_risk, 
        &bosstype[0],
        &bosstype[1]
    );

    // How many enemies are in the map?
    count = VSLM_CountEnemiesInMap();

    // Allocate memory needed for the rando pool
    VSLM_Rando_AllocateEnemyPool(count);

    // Populate the randomizer pool with
    // the enemies in current map
    VSLM_Rando_PopulateEnemyPool();

    // Print stats of original enemies
    VSLM_Rando_PrintEnemyStats();

    // Handle Tag 666 events (before, softlock prevention)
    if (bossmap && bossmap_softlock_risk)
        VSLM_Rando_PreserveEnemyType(bosstype[0]);

    // Randomize all enemies
    VSLM_Rando_SetEnemyOdds();
    for (i = 0; i < enemypool.length; i++)
    {
        actor = enemypool.enemies[i];
        if (!actor->norandom)
            VSLM_ChangeMonsterType(actor, VSLM_Rando_DetermineType());

        // Play a sound if spawning an arch-vile or a boss
        if (actor->type == MT_VILE)
            S_StartSound(actor, sfx_vilact);
        if (actor->type == MT_SPIDER || actor->type == MT_CYBORG)
            S_StartSound(actor,
                         (gamemode == commercial) ? sfx_boscub : sfx_telept);
    }

    // Spawn Tag 666 enemy (if no softlocks)
    if(bossmap)
    {
        if (!bossmap_softlock_risk)
            VSLM_ChangeMonsterType(enemypool.enemies[VSLM_Rand(0, enemypool.length - 1)], bosstype[0]);

        if (bosstype[1] != MT_NULL)
            VSLM_ChangeMonsterType(enemypool.enemies[VSLM_Rand(0, enemypool.length - 1)], bosstype[1]);
    }

    // Print stats after randomization
    DEH_printf("\nAfter randomization:\n");
    VSLM_Rando_PrintEnemyStats();

    // At the end. Free up memory
    VSLM_Rando_ClearEnemyPool();
}

/* ----------------------------------- */
/* Internal Functions ---------------- */
/* ----------------------------------- */

void VSLM_Rando_AllocateEnemyPool(int size)
{
    uint32_t memsize;
    memsize = size * sizeof(mobj_t*);

    // Allocate with PU_STATIC tag since we only need
    // this to organise all the enemies in the map.
    // This gets freed once we're done with it.
    enemypool.enemies = Z_Malloc(memsize, PU_STATIC, NULL);
    memset(enemypool.enemies, 0, memsize);

    enemypool.length = size;
    enemypool.index = 0;
    VSLM_DEBUG("VSLM_Rando_AllocateEnemyPool: Allocated %u bytes to heap.",
               memsize);
}

void VSLM_Rando_ClearEnemyPool(void)
{
    VSLM_DEBUG("VSLM_Rando_ClearEnemyPool: Freeing memory used by enemy pool.");
    Z_Free(enemypool.enemies);
    memset(&enemypool, 0, sizeof(randopool_enemy_t));
}

void VSLM_Rando_PopulateEnemyPool(void)
{
    int count;
    thinker_t *th;
    mobj_t *actor;

    count = 0;
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 != (actionf_p1) P_MobjThinker)
            continue; // Not a thinker. Skip.

        actor = (mobj_t*)th;
        if(VSLM_IsMonster(actor)) // If mobj_t is an enemy, add it to the pool
        {
            VSLM_Rando_AddEnemy(actor);
            count++;
        }
    }
    VSLM_DEBUG("VSLM_Rando_PopulateEnemyPool: Added %d enemies to pool", count);
}

void VSLM_Rando_AddEnemy(mobj_t *actor)
{
    // Error checks
    if(enemypool.index >= enemypool.length)
        I_Error("VSLM_Rando_AddEnemy: Enemy pool index has gone out of bounds!");

    if(!enemypool.enemies)
        I_Error("VSLM_Rando_AddEnemy: Enemy pool not allocated into heap!");

    enemypool.enemies[enemypool.index] = actor;
    enemypool.index++;
}

void VSLM_Rando_PrintEnemyStats(void)
{
    int i;
    int total;
    mobj_t *ptr;

    memset(&randocount, 0, randocountsize * sizeof(int));
    total = 0;
    for (i = 0; i < enemypool.length; i++)
    {
        ptr = enemypool.enemies[i];
        randocount[(int)VSLM_Rando_GetEnemyTier(ptr)]++;
    }

    // Print the results
    DEH_printf("VSLM_Rando_PrintEnemyStats:\n\n");
    for (i = 0; i < randocountsize; i++)
    {
        if(gamemode != commercial && (i == 3 || i == 4 || i == 6))
            continue;

        DEH_printf("%s:\t%d\n", VSLM_Rando_GetTierStr((randotier_t)i), randocount[i]);
        total += randocount[i];
    }
    DEH_printf("\nTotal:\t%d\n", total);
}

// Selects random enemy of specific type and sets
// the 'norandom' flag to true.
void VSLM_Rando_PreserveEnemyType(mobjtype_t actortype)
{
    int i;
    mobj_t *actor;
    int *actorindex;
    int indexsize, rover;
    int allocsize;

    indexsize = VSLM_Rando_CountEnemy(actortype);
    allocsize = indexsize * sizeof(int);
    actorindex = Z_Malloc(allocsize, PU_STATIC, NULL);
    VSLM_DEBUG("VSLM_Rando_PreserveEnemyType: Allocated %d bytes", allocsize);

    // Populate actorindex
    rover = 0;
    for (i = 0; i < enemypool.length; i++)
    {
        actor = enemypool.enemies[i];
        if (actor->type == actortype)
        {
            actorindex[rover] = i;
            rover++;
        }
    }

    // Select a random index from 'actorindex'
    rover = VSLM_Rand(0,(indexsize - 1));

    // Set 'no random' flag to prevent it from being
    // changed via the randomizer
    actor = enemypool.enemies[actorindex[rover]];
    actor->norandom = true;

    VSLM_DEBUG("VSLM_Rando_PreserveEnemyType: Preserved enemy [%s]", VSLM_GetMonsterType(actortype));

    // We're done here, so free up memory
    Z_Free(actorindex);
}

const char *VSLM_Rando_GetTierStr(randotier_t t)
{
    switch(t)
    {
        default:
            return "NULL";
        case R_TIER1:
            return "Tier 1";
        case R_TIER2:
            return "Tier 2";
        case R_TIER3:
            return "Tier 3";
        case R_TIER4:
            return "Tier 4";
        case R_TIER5:
            return "Tier 5";
        case R_BOSS:
            return "Boss";
        case R_SECRET:
            return "???";
    }
}

randotier_t VSLM_Rando_GetEnemyTier(mobj_t *actor)
{
    switch (actor->type)
        {
            default:
                return R_NULL;

            // Tier 1
            case MT_POSSESSED:
            case MT_SHOTGUY:
            case MT_TROOP:
            case MT_CHAINGUY:
                return R_TIER1;

            // Tier 2
            case MT_SERGEANT:
            case MT_SHADOWS:
            case MT_SKULL:
            case MT_HEAD:
            case MT_KNIGHT:
                return R_TIER2;

            // Tier 3
            case MT_BRUISER:
            case MT_BABY:
            case MT_PAIN:
                return R_TIER3;

            // Tier 4 (Doom 2)
            case MT_UNDEAD:
            case MT_FATSO:
                return R_TIER4;

            // Tier 5 (Doom 2)
            case MT_VILE:
                return R_TIER5;

            // Boss
            case MT_SPIDER:
            case MT_CYBORG:
                return R_BOSS;

            // Secret (Doom 2)
            case MT_WOLFSS:
                return R_SECRET;
        }
}

int VSLM_Rando_CountEnemy(mobjtype_t actortype)
{
    mobj_t *actor;
    int count;
    int i;

    count = 0;
    for (i = 0; i < enemypool.length; i++)
    {
        actor = enemypool.enemies[i];
        if (actor->type == actortype)
            count++;
    }
    return count;
}

boolean VSLM_Rando_MapContainsTag666(boolean *softlock, mobjtype_t *actortype,
                                     mobjtype_t *actortype2)
{
    boolean result = false;
    switch (gamemode)
    {
        case commercial: // Doom 2
            // MAP07: Dead Simple (7 Mancubus + 12 Arachnotrons)
            if (gamemap == 7)
            {
                // MAP07 has two of these events, one of them
                // having a softlock risk.
                *softlock = true;
                result = true;

                if (actortype)
                    *actortype = MT_FATSO; // Mancubus

                if (actortype2)
                    *actortype2 = MT_BABY; // Arachnotron
            }
            break;

            // MAP32 is not counted because Commander Keen
            // is never randomized, so no need to do anything.

        default: // Doom
            switch (gameepisode)
            {
                case 1:
                    // E1M8: Phobos Anomaly (2 Barons of Hell)
                    if (gamemap == 8)
                    {
                        // Potential softlock in E1M8
                        *softlock = true;
                        result = true;

                        if (actortype)
                            *actortype = MT_BRUISER;
                    }
                    break;

                case 2:
                    // E2M8: Tower of Babel (Cyberdemon)
                    if (gamemap == 8) 
                    {
                        result = true;
                        if (actortype)
                            *actortype = MT_CYBORG;
                    }
                    break;
                case 3:
                    // E3M8: Dis (Spider Mastermind)
                    if (gamemap == 8)
                    {
                        result = true;
                        if (actortype)
                            *actortype = MT_SPIDER;
                    }
                    break;
                case 4:
                    // E4M6: Against Thee Wickedly (Cyberdemon)
                    if (gamemap == 6)
                    {
                        result = true;
                        if (actortype)
                            *actortype = MT_CYBORG;
                    }
                    // E4M8: Unto the Cruel (Spider Mastermind)
                    if (gamemap == 8)
                    {
                        result = true;
                        if (actortype)
                            *actortype = MT_SPIDER;
                    }
                    break;
            }
    }
    return result;
}

// Set the odds for each tier depending on the map
void VSLM_Rando_SetEnemyOdds(void)
{
    memset(&randoenemy_odds, 0, sizeof(odds_t));

    // Dynamically determine tier odds depending on the map
    switch (gamemode)
    {
        case commercial: // Doom 2
            if (gamemap < 3)
                VSLM_Rando_SetTierOdds(128, 512, 2048, 8192, 0);
            else if (gamemap < 7)
                VSLM_Rando_SetTierOdds(80, 250, 1024, 8192, 65535);
            else if (gamemap == 7)
                VSLM_Rando_SetTierOdds(80, 0, 0, 8192, 65535);
            else if (gamemap < 12)
                VSLM_Rando_SetTierOdds(64, 175, 512, 4096, 8192);
            else if (gamemap < 20)
                VSLM_Rando_SetTierOdds(32, 130, 256, 1024, 4096);
            else if (gamemap < 25)
                VSLM_Rando_SetTierOdds(24, 50, 64, 512, 2048);
            else
                VSLM_Rando_SetTierOdds(8, 25, 32, 128, 1024);
            break;

        case shareware: // Doom Shareware (Same as retail E1M1, but no boss chance)
            if (gamemap < 3)
                VSLM_Rando_SetTierOdds(75, 750, 0, 0, 0);
            else if (gamemap < 5)
                VSLM_Rando_SetTierOdds(50, 500, 0, 0, 0);
            else if (gamemap < 7)
                VSLM_Rando_SetTierOdds(25, 250, 0, 0, 0);
            else
                VSLM_Rando_SetTierOdds(10, 0, 0, 0, 0);
            break;

        case registered:
        case retail: // Ultimate Doom (Episode 4)
            switch (gameepisode)
            {
                case 1:
                    if (gamemap < 3)
                        VSLM_Rando_SetTierOdds(75, 750, 0, 0, 0);
                    else if (gamemap < 5)
                        VSLM_Rando_SetTierOdds(50, 500, 0, 0, 8192);
                    else if (gamemap < 7)
                        VSLM_Rando_SetTierOdds(25, 250, 0, 0, 4096);
                    else
                        VSLM_Rando_SetTierOdds(10, 0, 0, 0, 1024);
                    break;
                case 2:
                    if (gamemap < 3)
                        VSLM_Rando_SetTierOdds(50, 500, 0, 0, 8192);
                    else if (gamemap < 5)
                        VSLM_Rando_SetTierOdds(50, 250, 0, 0, 4096);
                    else if (gamemap < 7)
                        VSLM_Rando_SetTierOdds(40, 250, 0, 0, 2048);
                    else
                        VSLM_Rando_SetTierOdds(30, 50, 0, 0, 1024);
                    break;
                case 3:
                    if (gamemap < 3)
                        VSLM_Rando_SetTierOdds(20, 250, 0, 0, 8192);
                    else if (gamemap < 5)
                        VSLM_Rando_SetTierOdds(15, 100, 0, 0, 4096);
                    else if (gamemap < 7)
                        VSLM_Rando_SetTierOdds(10, 50, 0, 0, 2048);
                    else
                        VSLM_Rando_SetTierOdds(5, 20, 0, 0, 1024);
                    break;
                case 4:
                    if (gamemap == 1 || gamemap == 9)
                        VSLM_Rando_SetTierOdds(50, 100, 0, 0, 4096);
                    else if (gamemap < 6)
                        VSLM_Rando_SetTierOdds(25, 75, 0, 0, 2048);
                    else
                        VSLM_Rando_SetTierOdds(20, 50, 0, 0, 1024);
                    break;
            }
            break;

        default:
            break;
    }
}

void VSLM_Rando_SetTierOdds(uint16_t t2, uint16_t t3, uint16_t t4, uint16_t t5,
                            uint16_t boss)
{
    randoenemy_odds.t2 = t2;
    randoenemy_odds.t3 = t3;
    randoenemy_odds.t4 = t4;
    randoenemy_odds.t5 = t5;
    randoenemy_odds.boss = boss;
}

// This is where the enemy randomization actually happens
mobjtype_t VSLM_Rando_DetermineType(void)
{
    // Determine sizes of tier arrays
    const size_t t1size = sizeof(tier1) / sizeof(tier1[0]);
    const size_t t2size = (gamemode == commercial)
                              ? sizeof(tier2) / sizeof(tier2[0])
                              : (sizeof(tier2) / sizeof(tier2[0])) - 1;
    const size_t t3size = (gamemode == commercial)
                              ? sizeof(tier3) / sizeof(tier3[0])
                              : (sizeof(tier3) / sizeof(tier3[0])) - 2;
    const size_t t4size = sizeof(tier4) / sizeof(tier4[0]);
    const size_t t5size = sizeof(tier5) / sizeof(tier5[0]);
    const size_t bsize = sizeof(boss) / sizeof(boss[0]);
    const int secret_odds = (gamemode == commercial && gamemap > 30) ? 5 : 8192;
    const int chaingunner_odds = (gamemode == commercial) ? 25 : 0;

    // Only use Tier 4 and 5 enemies in Doom 2
    if (gamemode == commercial)
    {
        // The secret enemy (Common in secret levels, )
        if (VSLM_Rand(1, secret_odds) == secret_odds)
            return MT_WOLFSS;

        if (randoenemy_odds.t5 != 0 && VSLM_Rand(1, randoenemy_odds.t5) == randoenemy_odds.t5)
            return tier5[VSLM_DoomRand() % t5size]; // Tier 5 enemy (Arch-vile)

        if (randoenemy_odds.t4 != 0 && VSLM_Rand(1, randoenemy_odds.t4) == randoenemy_odds.t4)
            return tier4[VSLM_DoomRand() % t4size]; // Tier 4 enemy
    }

    // Tier 3
    if (randoenemy_odds.t3 != 0 && VSLM_Rand(1, randoenemy_odds.t3) == randoenemy_odds.t3)
        return tier3[VSLM_DoomRand() % t3size];

    // Tier 2
    if (randoenemy_odds.t2 != 0 && VSLM_Rand(1, randoenemy_odds.t2) == randoenemy_odds.t2)
        return tier2[VSLM_DoomRand() % t2size];

    // Chaingunner
    if (chaingunner_odds != 0 && VSLM_Rand(1, chaingunner_odds) == chaingunner_odds)
        return MT_CHAINGUY;

    // Boss
    if (randoenemy_odds.boss != 0 && VSLM_Rand(1,randoenemy_odds.boss) == randoenemy_odds.boss)
        return boss[VSLM_DoomRand() % bsize];

    // If all above checks fail, spawn a Tier 1 monster
    return tier1[VSLM_DoomRand() % t1size];
}