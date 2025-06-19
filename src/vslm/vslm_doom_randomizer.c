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
	Randomizer functions for Doom/Doom 2
*/
#include "vslm.h"
#include <doomstat.h>
#include <sounds.h>

#include <i_system.h>
#include <m_argv.h>
#include <p_local.h>
#include <p_mobj.h>
#include <s_sound.h>
#include <z_zone.h>

// ------------------------
// Prototypes
// ------------------------
void VSLM_GenRandoPool(void);
mobjtype_t VSLM_DetermineEnemy(void);
boolean VSLM_IsTier2Monster(mobj_t *actor);
boolean VSLM_IsTier3Monster(mobj_t *actor);
boolean VSLM_IsBossMonster(mobj_t *actor);

// ------------------------
// Globals
// ------------------------

// Enemy randomizer pool. This'll just be
// mainly filled with common enemies.
// However for Chaos mode, everything is
// added into the pool.
typedef struct
{
    mobjtype_t *pool;
    int poolsize;
} r_enemypool_t;
r_enemypool_t enemy_randopool = {NULL, 0};

boolean RAND_CHAOS = false;

// This is set after rando pool has been
// generated so as to not break when
// RAND_CHAOS is changed on the current map,
// requiring a restart to set the randomizer
// to Chaos mode.
boolean chaosmode_cache;

// Tier 1 [Common]: Zombieman, Sergeant, Imp
const mobjtype_t tier1[] = {
    MT_POSSESSED,
    MT_SHOTGUY,
    MT_TROOP,

    // Doom 2
    MT_CHAINGUY,
};

// Tier 2 [Uncommon]: Pinky, Spectre, Lost Soul, Cacodemon
// (D2: Chaingunner, Hell Knight)
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

int t1size, t2size, t3size, t4size, t5size, bossize;

// Spawn odds for monsters
typedef struct
{
    int t2, t3, t4, t5, boss;
} odds_t;
odds_t *rando_odds;


// ------------------------
// VSLM_RandomizeMonsters
// ------------------------
int VSLM_RandomizeMonsters(boolean spawnfog)
{
    int monstercount = 0;
    int ambush;
    int i;
    mobjtype_t mapboss = MT_NULL;
    mobj_t *actor, *fogactor;
    thinker_t *th;
    boolean mapboss_kept = false;

    // Populate the randomizer pool
    if (!enemy_randopool.pool)
    {
        VSLM_GenRandoPool();
    }

	for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1) P_MobjThinker)
        {
            actor = (mobj_t *) th;

            // Boss check (softlock prevention)
            if (!mapboss_kept && !chaosmode_cache)
            {
                switch (gamemode)
                {
                    case shareware:
                    case registered:
                    case retail:
                        if (gameepisode == 1 && gamemap == 8)
                        {
                            mapboss = MT_BRUISER;
                            fogactor = VSLM_GetRandomMonster(&mapboss);
                            fogactor->norandom = true;
                            mapboss_kept = true;
                        }
                        break;

                    case commercial:
                        if (gamemap == 7 && actor->type == MT_FATSO)
                        {
                            mapboss = MT_FATSO;
                            fogactor = VSLM_GetRandomMonster(&mapboss);
                            fogactor->norandom = true;
                            mapboss_kept = true;
                        }
                        break;
                }
            }

            // Is Mobj a monster (and not dead)?
            if (((actor->flags & MF_COUNTKILL) && actor->type != MT_KEEN ||
                 actor->type == MT_SKULL) &&
                 actor->health >= 0 && !actor->norandom)
            {
                // Retain the ambush flag if there is one
                ambush = (actor->flags & MF_AMBUSH) ? MF_AMBUSH : 0;

                // Store position and angle
                mobjtype_t newenemy = VSLM_DetermineEnemy();

                if (spawnfog)
                {
                    mobjtype_t fog =
                        (gamemode == commercial) ? MT_SPAWNFIRE : MT_TFOG;

                    fogactor = P_SpawnMobj(actor->x, actor->y, actor->z, fog);
                }

                // Update actor's type and re-init properties

                // Temporarily store old type
                // mapboss = actor->type;

                VSLM_ChangeMonsterType(actor, newenemy);

                /*if (mapboss == MT_SKULL)
                    actor->flags &= ~MF_COUNTKILL;*/

                // If boss, play sound indicator
                if (VSLM_IsBossMonster(actor))
                {
                    int snd =
                        (gamemode == commercial) ? sfx_boscub : sfx_metal;
                    S_StartSound(actor, snd);
                }

                monstercount++;
            }
        }
    }
    mapboss = MT_NULL;

    // Boss check
    if (!chaosmode_cache)
    {
        switch (gamemode)
        {
            case commercial: // Doom 2
                if (gamemap == 7)
                {
                    if (!VSLM_MonsterFound(MT_BABY))
                        mapboss = MT_BABY;
                }
                break;

            default: // Doom
                switch (gameepisode)
                {
                    case 2:
                        if (gamemap == 8)
                        {
                            if (!VSLM_MonsterFound(MT_CYBORG))
                                mapboss = MT_CYBORG;
                        }
                        break;
                    case 3:
                        if (gamemap == 8)
                        {
                            if (!VSLM_MonsterFound(MT_SPIDER))
                                mapboss = MT_SPIDER;
                        }
                        break;
                    case 4:
                        switch (gamemap)
                        {
                            case 6:
                                if (!VSLM_MonsterFound(MT_CYBORG))
                                    mapboss = MT_CYBORG;
                                break;

                            case 8:
                                if (!VSLM_MonsterFound(MT_SPIDER))
                                    mapboss = MT_SPIDER;
                                break;
                        }
                        break;
                }
        }
    }
    return monstercount;
}

// ------------------------
// VSLM_IsTier2Monster
// ------------------------
boolean VSLM_IsTier2Monster(mobj_t *actor)
{
    int i;
    for (i = 0; i < t2size; i++)
    {
        if (actor->type == tier2[i])
            return true;
    }
    return false;
}

// ------------------------
// VSLM_IsTier3Monster
// ------------------------
boolean VSLM_IsTier3Monster(mobj_t *actor)
{
    int i;
    for (i = 0; i < t3size; i++)
    {
        if (actor->type == tier3[i])
            return true;
    }
    return false;
}

// ------------------------
// VSLM_IsBossMonster
// ------------------------
boolean VSLM_IsBossMonster(mobj_t *actor)
{
    if (actor->type == MT_SPIDER || actor->type == MT_CYBORG)
        return true;
    return false;
}

// ------------------------
// VSLM_ClearRandoPool
// ------------------------
void VSLM_ClearRandoPool(void)
{
    if (enemy_randopool.pool)
    {
        VSLM_DEBUG("VSLM_ClearRandoPool: Clearing enemy pool.");
        enemy_randopool.pool = NULL;
    }

    if (rando_odds)
        rando_odds = NULL;
}

// ------------------------
// Internal Functions
// ------------------------

// Prototypes
void VSLM_SetTierOdds(int t2, int t3, int t4, int t5, int boss);

void VSLM_GenRandoPool(void)
{

    t1size = (gamemode == commercial) ? sizeof(tier1) / sizeof(tier1[0])
                                      : (sizeof(tier1) / sizeof(tier1[0])) - 1;

    // The Tier 2/3 sizes is only relevant to chaos mode
    t2size = (gamemode == commercial)
        ? sizeof(tier2) / sizeof(tier2[0])
        : (sizeof(tier2) / sizeof(tier2[0])) - 1;

    // Shareware (Remove D2 enemies + Lost soul and Cacodemon)
    if (gamemode == shareware)
        t2size = (sizeof(tier2) / sizeof(tier2[0])) - 4;

    t3size = (gamemode == commercial)
        ? sizeof(tier3) / sizeof(tier3[0])
        : (sizeof(tier3) / sizeof(tier3[0])) - 2;

    t4size = (gamemode == commercial) ? sizeof(tier4) / sizeof(tier4[0]) : 0;
    t5size = (gamemode == commercial) ? 1 : 0;
    bossize = 2;

    chaosmode_cache = RAND_CHAOS;

    if (!chaosmode_cache)
    {
        enemy_randopool.poolsize = t1size;

        enemy_randopool.pool = Z_Malloc(
            enemy_randopool.poolsize * sizeof(mobjtype_t), PU_LEVEL, NULL);
        // Remove leftover memory
        memset(enemy_randopool.pool, 0,
               enemy_randopool.poolsize * sizeof(mobjtype_t));

        // Populate the allocated pool
        memcpy(enemy_randopool.pool, tier1,
               enemy_randopool.poolsize * sizeof(mobjtype_t));

        VSLM_DEBUG("VSLM_GenRandoPool: Allocated %u bytes "
                   "for monster randomizer pool.",
                   enemy_randopool.poolsize);
        return;
    }

    // Chaos rando pool (All tiers, except bosses)
    enemy_randopool.poolsize = t1size + t2size + t3size + t4size;
    enemy_randopool.pool =
        Z_Malloc(enemy_randopool.poolsize * sizeof(mobjtype_t), PU_LEVEL, NULL);
    // Remove leftover memory
    memset(enemy_randopool.pool, 0,
           enemy_randopool.poolsize * sizeof(mobjtype_t));

    // Populate all enemies except bosses
    memcpy(enemy_randopool.pool, tier1, t1size * sizeof(mobjtype_t));
    memcpy(enemy_randopool.pool + t1size, tier2, t2size * sizeof(mobjtype_t));
    memcpy(enemy_randopool.pool + (t1size + t2size), tier3,
           t3size * sizeof(mobjtype_t));
    if (gamemode == commercial)
    {
        // Tiers 4 and 5
        memcpy(enemy_randopool.pool + (t1size + t2size + t3size), tier4,
               t4size * sizeof(mobjtype_t));
    }
    VSLM_DEBUG("VSLM_GenRandoPool: Allocated %u bytes "
               "for monster randomizer pool. (Chaos mode)",
               enemy_randopool.poolsize);
}

mobjtype_t VSLM_DetermineEnemy(void)
{
    mobjtype_t newtype;
    int rand, randindex;

    // Get random type for pool
    randindex = VSLM_DoomRand() % enemy_randopool.poolsize;
    newtype = enemy_randopool.pool[randindex];
    VSLM_SetTierOdds(0, 0, 0, 1024, 2048);

    if (!chaosmode_cache) // Calculate tier chances
    {
        // Dynamically determine odds depending on the map
        switch (gamemode)
        {
            case commercial: // Doom 2
                if (gamemap < 3)
                    VSLM_SetTierOdds(100, 300, 2000, 8192, 0);
                else if (gamemap < 7)
                    VSLM_SetTierOdds(80, 250, 2000, 8192, 0);
                else if (gamemap < 12)
                    VSLM_SetTierOdds(75, 200, 1000, 4096, 8192);
                else if (gamemap < 20)
                    VSLM_SetTierOdds(50, 100, 200, 1024, 4096);
                else if (gamemap < 25)
                    VSLM_SetTierOdds(25, 50, 75, 512, 1024);
                else
                    VSLM_SetTierOdds(10, 25, 50, 256, 512);
                break;


            case shareware: // Doom Shareware (Tier 1 monsters only + Tier 3 low odds [Baron])
                if (gamemap < 3)
                    VSLM_SetTierOdds(75, 750, 0, 0, 0);
                else if (gamemap < 5)
                    VSLM_SetTierOdds(50, 500, 0, 0, 0);
                else if (gamemap < 7)
                    VSLM_SetTierOdds(30, 250, 0, 0, 0);
                else
                    VSLM_SetTierOdds(25, 0, 0, 0, 0);
                break;

            case retail: // Ultimate Doom (Episode 4)
                if (gameepisode == 4)
                {
                    if (gamemap == 1 || gamemap == 9)
                        VSLM_SetTierOdds(50, 100, 0, 0, 3000);
                    else if (gamemap < 6)
                        VSLM_SetTierOdds(25, 75, 0, 0, 3000);
                    else
                        VSLM_SetTierOdds(20, 50, 0, 0, 3000);
                }
                break;

            default: // Doom
                switch (gameepisode)
                {
                    case 1:
                        if (gamemap < 3)
                            VSLM_SetTierOdds(75, 750, 0, 0, 0);
                        else if (gamemap < 5)
                            VSLM_SetTierOdds(50, 500, 0, 0, 0);
                        else if (gamemap < 7)
                            VSLM_SetTierOdds(30, 250, 0, 0, 0);
                        else
                            VSLM_SetTierOdds(25, 0, 0, 0, 1000);
                        break;

                    case 2:
                        if (gamemap < 3)
                            VSLM_SetTierOdds(50, 500, 0, 0, 0);
                        else if (gamemap < 5)
                            VSLM_SetTierOdds(50, 250, 0, 0, 8192);
                        else if (gamemap < 7)
                            VSLM_SetTierOdds(40, 250, 0, 0, 8192);
                        else
                            VSLM_SetTierOdds(30, 50, 0, 0, 200);
                        break;

                    default:
                        if (gamemap < 3)
                            VSLM_SetTierOdds(25, 500, 0, 0, 0);
                        else if (gamemap < 5)
                            VSLM_SetTierOdds(25, 250, 0, 0, 0);
                        else if (gamemap < 7)
                            VSLM_SetTierOdds(25, 50, 0, 0, 0);
                        else
                            VSLM_SetTierOdds(10, 20, 0, 0, 100);
                        break;

                }
        }


        rand = (rando_odds->t2 != 0) ? VSLM_Rand(1, rando_odds->t2) : 0;
        if (rand == rando_odds->t2 && rand != 0)
        {
            randindex = VSLM_DoomRand() % t2size;
            newtype = tier2[randindex];
            VSLM_DEBUG("VSLM_DetermineEnemy: Spawned tier 2 monster [%s]", VSLM_GetMonsterType(newtype));
        }

        rand = (rando_odds->t3 != 0) ? VSLM_Rand(1, rando_odds->t3) : 0;
        if (rand == rando_odds->t3 && rand != 0)
        {
            randindex = VSLM_DoomRand() % t3size;
            newtype = tier3[randindex];
            VSLM_DEBUG("VSLM_DetermineEnemy: Spawned tier 3 monster [%s]", VSLM_GetMonsterType(newtype));
        }

        rand = (rando_odds->t4 != 0) ? VSLM_Rand(1, rando_odds->t4) : 0;
        if (rand == rando_odds->t4 && rand != 0)
        {
            randindex = VSLM_DoomRand() % t4size;
            newtype = tier4[randindex];
            VSLM_DEBUG("VSLM_DetermineEnemy: Spawned tier 4 monster [%s]",
                       VSLM_GetMonsterType(newtype));
        }
    }

    // Arch-vile spawn chance (Doom 2 only)
    if (gamemode == commercial)
    {
        rand = (rando_odds->t5 != 0) ? VSLM_Rand(1, rando_odds->t5) : 0;
        if (rand == rando_odds->t5 && rand != 0)
        {
            randindex = VSLM_DoomRand() % t5size;
            newtype = tier5[randindex];
            VSLM_DEBUG("VSLM_DetermineEnemy: Spawned tier 5 monster [%s]",
                       VSLM_GetMonsterType(newtype));
        }
    }

    // Will this actor become a boss?
    rand = (rando_odds->boss != 0) ? VSLM_Rand(1, rando_odds->boss) : 0;
    if (rand == rando_odds->boss && rand != 0)
    {
        randindex = VSLM_DoomRand() % bossize;
        newtype = boss[randindex];
        VSLM_DEBUG("VSLM_DetermineEnemy: Spawned BOSS monster [%s]",
                   VSLM_GetMonsterType(newtype));
    }
    return newtype;
}

void VSLM_SetTierOdds(int t2, int t3, int t4, int t5, int boss)
{
    // Allocate memory for the odds_t struct
    // if not done already
    if (!rando_odds)
    {
        rando_odds = Z_Malloc(sizeof(odds_t), PU_LEVEL, NULL);
        memset(rando_odds, 0, sizeof(odds_t));
    }
    rando_odds->t2 = t2;
    rando_odds->t3 = t3;
    rando_odds->t4 = t4;
    rando_odds->t5 = t5;
    rando_odds->boss = boss;
}