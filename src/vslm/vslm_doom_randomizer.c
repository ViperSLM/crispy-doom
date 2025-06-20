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
#include "vslm_doom_randomizer.h"

#include <doomstat.h>
#include <sounds.h>
#include <i_system.h>
#include <m_argv.h>
#include <p_local.h>
#include <p_mobj.h>
#include <s_sound.h>
#include <z_zone.h>

/* ----------------------------------- */
/* Prototypes ------------------------ */
/* ----------------------------------- */
void VSLM_GenRandoPool(void);
void VSLM_Rando_SetEnemyOdds(void);
mobjtype_t VSLM_DetermineEnemy(void);
boolean VSLM_Rando_BossCheck(void);
mobjtype_t VSLM_Rando_MapBoss(void);

/* ----------------------------------- */
/* Globals --------------------------- */
/* ----------------------------------- */
// Enable randomizer
boolean RANDOMIZER = false;

// Tier 1 [Common]: Zombieman, Sergeant, Imp
const mobjtype_t tier1[] = {
    MT_POSSESSED,
    MT_SHOTGUY,
    MT_TROOP,
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

boolean RAND_CHAOS = false;
r_enemypool_t enemy_randopool = {NULL, 0};

// This is set after rando pool has been
// generated so as to not break when
// RAND_CHAOS is changed on the current map,
// requiring a restart to set the randomizer
// to Chaos mode.
boolean chaosmode_cache;

int t1size, t2size, t3size, t4size, t5size, bossize;

// Spawn odds for monsters
odds_t *rando_odds;

/* ----------------------------------- */
/* Functions ------------------------- */
/* ----------------------------------- */

// ------------------------
// VSLM_RandomizeMonsters
// ------------------------
int VSLM_RandomizeMonsters(boolean spawnfog)
{
    int monstercount = 0, snd = 0;
    mobjtype_t mapboss = MT_NULL;
    mobj_t *actor, *fogactor;
    thinker_t *th;
    boolean mapboss_kept = false;

    // Populate the randomizer pool
    if (!enemy_randopool.pool)
    {
        VSLM_GenRandoPool();
    }

    // Set randomizer odds
    VSLM_Rando_SetEnemyOdds();

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1) P_MobjThinker)
        {
            actor = (mobj_t *) th;

            // Boss check before randomization.
            // Specifically used for E1M8 and MAP07
            // to prevent softlocking the Tag 666 event
            if (!mapboss_kept && !chaosmode_cache)
                mapboss_kept = VSLM_Rando_BossCheck();

            // Is Mobj a monster (and not dead)?
            if (VSLM_IsMonster(actor) && actor->health > 0 &&
                actor->type != MT_KEEN && !actor->norandom)
            {
                mobjtype_t newenemy = VSLM_DetermineEnemy();
                if (spawnfog)
                {
                    mobjtype_t fog =
                        (gamemode == commercial) ? MT_SPAWNFIRE : MT_TFOG;

                    fogactor = P_SpawnMobj(actor->x, actor->y, actor->z, fog);
                }

                // Update actor's type and re-init properties
                VSLM_ChangeMonsterType(actor, newenemy);

                // If boss or arch-vile, play sound indicator
                if (actor->type == MT_SPIDER || actor->type == MT_CYBORG)
                {
                    snd = (gamemode == commercial) ? sfx_boscub : sfx_telept;
                }
                else if (actor->type == MT_VILE)
                    snd = sfx_vilact;

                if (snd != 0)
                    S_StartSound(actor, snd);

                monstercount++;
            }
        }
    }
    mapboss = MT_NULL;

    // Boss check (After randomization)
    if (!chaosmode_cache)
    {
        mapboss = VSLM_Rando_MapBoss();
        if (mapboss !=
            MT_NULL) // If spawning a monster from above check, select random actor and spawn it
        {
            actor = VSLM_Rando_GetMonster(NULL);
            VSLM_ChangeMonsterType(actor, mapboss);
        }
    }

    // Print results to stdout
    DEH_printf("\nVSLM_RandomizeMonsters - Results\n"
               "Tier 1:\t%u\nTier 2:\t%u\nTier 3:\t%u\n"
               "Tier 4:\t%u\nTier 5:\t%u\nBoss:\t%u\n???:\t%u\n"
               "\nTotal:\t%d\n\n",
               enemy_randopool.count.tier1, enemy_randopool.count.tier2,
               enemy_randopool.count.tier3, enemy_randopool.count.tier4,
               enemy_randopool.count.tier5, enemy_randopool.count.boss,
               enemy_randopool.count.secret, monstercount);

    return monstercount;
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
    {
        VSLM_DEBUG("VSLM_ClearRandoPool: Clearing randomizer odds pool.");
        rando_odds = NULL;
    }
}

// ------------------------
// Internal Functions
// ------------------------

// Prototypes
void VSLM_SetTierOdds(int t2, int t3, int t4, int t5, int boss);

// Generate randomizer pool
void VSLM_GenRandoPool(void)
{
    // Reset counter
    memset(&enemy_randopool.count, 0, sizeof(enemycount_t));
    /*
    VSLM_DEBUG("T1: %d, T2: %d, T3: %d, T4: %d, T5: %d, B: %d, S: %d",
    enemy_randopool.count.tier1,enemy_randopool.count.tier2,enemy_randopool.count.tier3,
    enemy_randopool.count.tier4,enemy_randopool.count.tier5,enemy_randopool.count.boss,
    enemy_randopool.count.secret);
    */

    // Allocate memory for the odds_t struct
    // if not done already
    rando_odds = Z_Malloc(sizeof(odds_t), PU_LEVEL, NULL);
    memset(rando_odds, 0, sizeof(odds_t));

    /*
        Calculate array lengths depending on the game
    */
    t1size = sizeof(tier1) / sizeof(tier1[0]);

    // The Tier 2/3 sizes is only relevant to chaos mode
    t2size = (gamemode == commercial)
        ? sizeof(tier2) / sizeof(tier2[0])
        : (sizeof(tier2) / sizeof(tier2[0])) - 1;

    // Shareware (Remove Doom 2 enemies + Lost soul and Cacodemon)
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

    // Chaos rando pool (All tiers, except arch-viles and bosses)
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
        // Tier 4
        memcpy(enemy_randopool.pool + (t1size + t2size + t3size), tier4,
               t4size * sizeof(mobjtype_t));
    }
    VSLM_DEBUG("VSLM_GenRandoPool: Allocated %u bytes "
               "for monster randomizer pool. (Chaos mode)",
               enemy_randopool.poolsize);
}

// Determine what enemy to spawn into the map
mobjtype_t VSLM_DetermineEnemy(void)
{
    mobjtype_t newtype;
    int rand, randindex;
    if (!chaosmode_cache) // Calculate tier chances
    {
        // Tier 2
        rand = (rando_odds->t2 != 0) ? VSLM_Rand(1, rando_odds->t2) : 0;
        if (rand == rando_odds->t2 && rand != 0)
        {
            randindex = VSLM_DoomRand() % t2size;
            newtype = tier2[randindex];
            VSLM_DEBUG("VSLM_DetermineEnemy: Spawned tier 2 monster [%s]",
                       VSLM_GetMonsterType(newtype));
            enemy_randopool.count.tier2++;
            return newtype;
        }

        // Tier 3
        rand = (rando_odds->t3 != 0) ? VSLM_Rand(1, rando_odds->t3) : 0;
        if (rand == rando_odds->t3 && rand != 0)
        {
            randindex = VSLM_DoomRand() % t3size;
            newtype = tier3[randindex];
            VSLM_DEBUG("VSLM_DetermineEnemy: Spawned tier 3 monster [%s]",
                       VSLM_GetMonsterType(newtype));
            enemy_randopool.count.tier3++;
            return newtype;
        }

        // Tier 4
        rand = (rando_odds->t4 != 0) ? VSLM_Rand(1, rando_odds->t4) : 0;
        if (rand == rando_odds->t4 && rand != 0)
        {
            randindex = VSLM_DoomRand() % t4size;
            newtype = tier4[randindex];
            VSLM_DEBUG("VSLM_DetermineEnemy: Spawned tier 4 monster [%s]",
                       VSLM_GetMonsterType(newtype));
            enemy_randopool.count.tier4++;
            return newtype;
        }
    }

    // Arch-vile & Wolf SS spawn chance (Doom 2 only)
    if (gamemode == commercial)
    {
        // Tier 5
        rand = (rando_odds->t5 != 0) ? VSLM_Rand(1, rando_odds->t5) : 0;
        if (rand == rando_odds->t5 && rand != 0)
        {
            randindex = VSLM_DoomRand() % t5size;
            newtype = tier5[randindex];
            VSLM_DEBUG("VSLM_DetermineEnemy: Spawned tier 5 monster [%s]",
                       VSLM_GetMonsterType(newtype));
            enemy_randopool.count.tier5++;
            return newtype;
        }

        // Wolf SS troops will be extremely rare outside
        // of the secret levels
        rand = (gamemap > 30) ? 8 : 8192;
        if(rand == VSLM_Rand(0,rand))
        {
            newtype = MT_WOLFSS;
            VSLM_DEBUG("VSLM_DetermineEnemy: Spawned secret monster [%s]",
                       VSLM_GetMonsterType(newtype));
            enemy_randopool.count.secret++;
            return newtype;
        }

        // Chance of spawning a Heavy (chaingunner)
        rand = 16;
        if(rand == VSLM_Rand(0,rand))
        {
            newtype = MT_CHAINGUY;
            VSLM_DEBUG("VSLM_DetermineEnemy: Spawned chaingunner");
            enemy_randopool.count.tier1++;
            return newtype;
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
        enemy_randopool.count.boss++;
        return newtype;
    }

    // Tier 1
    randindex = VSLM_DoomRand() % enemy_randopool.poolsize;
    newtype = enemy_randopool.pool[randindex];
    enemy_randopool.count.tier1++;
    return newtype;
}

// Set the spawn chance of each tier
void VSLM_SetTierOdds(int t2, int t3, int t4, int t5, int boss)
{
    rando_odds->t2 = t2;
    rando_odds->t3 = t3;
    rando_odds->t4 = t4;
    rando_odds->t5 = t5;
    rando_odds->boss = boss;
}

// Sets overall progression of tier chances.
// Higher tiers have a higher chance of spawning
// as the player progresses through the game
void VSLM_Rando_SetEnemyOdds(void)
{
    if (chaosmode_cache)
    {
        // Chaos mode odds
        VSLM_SetTierOdds(0, 0, 0, 512, 1024);
        return;
    }
    // Dynamically determine tier odds depending on the map
    switch (gamemode)
    {
        case commercial: // Doom 2
            if (gamemap < 3)
                VSLM_SetTierOdds(128, 512, 2048, 8192, 0);
            else if (gamemap < 7)
                VSLM_SetTierOdds(80, 250, 1024, 8192, 65535);
            else if (gamemap == 7)
                VSLM_SetTierOdds(80, 0, 0, 8192, 65535);
            else if (gamemap < 12)
                VSLM_SetTierOdds(64, 175, 512, 4096, 8192);
            else if (gamemap < 20)
                VSLM_SetTierOdds(32, 130, 256, 1024, 4096);
            else if (gamemap < 25)
                VSLM_SetTierOdds(24, 50, 64, 512, 2048);
            else
                VSLM_SetTierOdds(8, 25, 32, 128, 1024);
            break;


        case shareware: // Doom Shareware (Same as retail E1M1, but no boss chance)
            if (gamemap < 3)
                VSLM_SetTierOdds(75, 750, 0, 0, 0);
            else if (gamemap < 5)
                VSLM_SetTierOdds(50, 500, 0, 0, 0);
            else if (gamemap < 7)
                VSLM_SetTierOdds(25, 250, 0, 0, 0);
            else
                VSLM_SetTierOdds(10, 0, 0, 0, 0);
            break;

        case retail: // Ultimate Doom (Episode 4)
            if (gameepisode == 4)
            {
                if (gamemap == 1 || gamemap == 9)
                    VSLM_SetTierOdds(50, 100, 0, 0, 4096);
                else if (gamemap < 6)
                    VSLM_SetTierOdds(25, 75, 0, 0, 2048);
                else
                    VSLM_SetTierOdds(20, 50, 0, 0, 1024);
            }
            break;

        default: // Doom
            switch (gameepisode)
            {
                case 1:
                    if (gamemap < 3)
                        VSLM_SetTierOdds(75, 750, 0, 0, 0);
                    else if (gamemap < 5)
                        VSLM_SetTierOdds(50, 500, 0, 0, 8192);
                    else if (gamemap < 7)
                        VSLM_SetTierOdds(25, 250, 0, 0, 4096);
                    else
                        VSLM_SetTierOdds(10, 0, 0, 0, 1024);
                    break;

                case 2:
                    if (gamemap < 3)
                        VSLM_SetTierOdds(50, 500, 0, 0, 8192);
                    else if (gamemap < 5)
                        VSLM_SetTierOdds(50, 250, 0, 0, 4096);
                    else if (gamemap < 7)
                        VSLM_SetTierOdds(40, 250, 0, 0, 2048);
                    else
                        VSLM_SetTierOdds(30, 50, 0, 0, 1024);
                    break;

                default:
                    if (gamemap < 3)
                        VSLM_SetTierOdds(25, 500, 0, 0, 8192);
                    else if (gamemap < 5)
                        VSLM_SetTierOdds(25, 250, 0, 0, 4096);
                    else if (gamemap < 7)
                        VSLM_SetTierOdds(25, 50, 0, 0, 2048);
                    else
                        VSLM_SetTierOdds(10, 20, 0, 0, 1024);
                    break;
            }
    }
}

// Checks for a specific monster depending on the
// map and sets the 'norandom' flag so it doesn't
// get replaced during randomization. This prevents
// softlocking certain maps due to tag 666 events.
boolean VSLM_Rando_BossCheck(void)
{
    mobj_t *actor;
    mobjtype_t actortype;
    boolean keptboss;

    keptboss = false;
    switch (gamemode)
    {
        case commercial:
            if (gamemap == 7) // Doom 2 - MAP07
            {
                actortype = MT_FATSO; // Mancubus
                actor = VSLM_Rando_GetMonster(&actortype);
                actor->norandom = true;
                keptboss = true;
                enemy_randopool.count.tier4++;
            }
            break;
        default:
            if (gameepisode == 1 && gamemap == 8) // Doom - E1M8
            {
                actortype = MT_BRUISER; // Baron
                actor = VSLM_Rando_GetMonster(&actortype);
                actor->norandom = true;
                keptboss = true;
                enemy_randopool.count.tier3++;
            }
            break;
    }
    return keptboss;
}

// Spawn a specific monster after randomization,
// depending on the map. Resolves softlocking
// Tag 666 events.
//
// Example: In E2M8, a Cyberdemon is always
// guaranteed to spawn somewhere in the map.
mobjtype_t VSLM_Rando_MapBoss(void)
{
    mobjtype_t bosstype;

    bosstype = MT_NULL;
    switch (gamemode)
    {
        case commercial: // Doom 2
            if (gamemap == 7)
            {
                if (!VSLM_MonsterFound(MT_BABY))
                {
                    bosstype = MT_BABY;
                    enemy_randopool.count.tier3++;
                }
            }
            break;

        default: // Doom
            switch (gameepisode)
            {
                case 2:
                    if (gamemap == 8)
                    {
                        if (!VSLM_MonsterFound(MT_CYBORG))
                        {
                            bosstype = MT_CYBORG;
                            enemy_randopool.count.boss++;
                        }
                    }
                    break;
                case 3:
                    if (gamemap == 8)
                    {
                        if (!VSLM_MonsterFound(MT_SPIDER))
                        {
                            bosstype = MT_SPIDER;
                            enemy_randopool.count.boss++;
                        }
                    }
                    break;
                case 4:
                    switch (gamemap)
                    {
                        case 6:
                            if (!VSLM_MonsterFound(MT_CYBORG))
                            {
                                bosstype = MT_CYBORG;
                                enemy_randopool.count.boss++;
                            }
                            break;

                        case 8:
                            if (!VSLM_MonsterFound(MT_SPIDER))
                            {
                                bosstype = MT_SPIDER;
                                enemy_randopool.count.boss++;
                            }
                            break;
                    }
                    break;
            }
    }
    return bosstype;
}