#if 0
#include "info.h"
#include "sounds.h"

#include "deh_str.h"
#include "d_mode.h"
#include "i_system.h"
#include "m_fixed.h"
#include "p_local.h"
#include "p_spec.h"
#include "p_mobj.h"
#include "r_defs.h"
#include "z_zone.h"

#include <time.h>
#include <stddef.h>

#ifdef VSLM_64BIT
#define VSLM_INTSIZE uint64_t
#else
#define VSLM_INTSIZE uint32_t
#endif

extern int extrakills;
extern int numbraintargets;
void A_PainDie(mobj_t *);

// Game-related externs
extern GameMode_t gamemode;
extern GameMission_t gamemission;
extern skill_t gameskill;
extern int gameepisode;
extern int gamemap;

// Thinker externs
extern thinker_t thinkercap;
void P_InitThinkers(void);
void P_AddThinker(thinker_t *thinker);
void P_RemoveThinker(thinker_t *thinker);
mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
void P_RemoveMobj(mobj_t *th);
boolean P_SetMobjState(mobj_t *mobj, statenum_t state);
void P_DamageMobj(mobj_t *target, mobj_t *inflictor, mobj_t *source,
                  int damage);
                         
// External functions
void S_StartSound(void *origin, int sound_id);
void G_ExitLevel(void);
void ST_PrintMsg(const char *format, ...);
int M_CheckParm(const char *check);
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args);
int M_snprintf(char *buf, size_t buf_len, const char *s, ...);
int P_Random(void);
boolean P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y);
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y);

// Forward declarations for local functions
void VSLM_FireCheat(void);
void VSLM_RespawnMonster(mobj_t *actor);
void VSLM_ReviveMonsters(int *outReviveCount);
void VSLM_SpawnFire(mobj_t *actor);
void VSLM_ToggleInvisMonsters(boolean *outInvisFlag);
void VSLM_TriggerTag666(boolean tag667);
boolean VSLM_IsMonster(mobj_t *actor);
boolean VSLM_IsMonsterGibbed(mobj_t *actor);

char mapName[5];

// Gets the name of the current map.
// Returns NULL if no map is loaded
const char *VSLM_GetCurrentMap(void);
// ----------------------------------------

// 16 entries (8 for Doom 1)
const mobjtype_t monsterTypes[] = {
    MT_POSSESSED, // Former human
    MT_SHOTGUY,   // Former human sergeant
    MT_TROOP,     // Imp
    MT_SERGEANT,  // Pinky/Demon
    MT_SHADOWS,   // Spectre
    MT_BRUISER,   // Baron of Hell
    MT_SKULL,     // Lost soul
    MT_HEAD,      // Cacodemon

    // Doom II enemies
    MT_CHAINGUY, // Former chaingunner
    MT_KNIGHT,   // Hell Knight
    MT_UNDEAD,   // Revenant
    MT_FATSO,    // Mancubus
    MT_BABY,     // Arachnotron
    MT_PAIN,     // Pain elemental
    MT_VILE,     // Arch-vile
    MT_WOLFSS,   // Wolfenstein SS
};
const int MONSTER_ARRAYCOUNT = sizeof(monsterTypes) / sizeof(monsterTypes[0]);

// 2 entries
const mobjtype_t bossTypes[] = {
    MT_CYBORG, // Cyberdemon
    MT_SPIDER, // Spider-Mastermind
};
const int BOSS_ARRAYCOUNT = sizeof(bossTypes) / sizeof(bossTypes[0]);

// Merges the above 2 arrays together
mobjtype_t *randopool;
int poolsize;

void VSLM_AllocateRandoPool(void)
{

    // Populate the randomizer pools
    switch (gamemode)
    {
        // DOOM/Ultimate DOOM
        case shareware:
        case registered:
        case retail:
            // We can just cut the pool size by half to
            // remove the Doom II monsters from the pool
            poolsize = (MONSTER_ARRAYCOUNT / 2);
            break;

        // DOOM II
        case commercial:
            poolsize = MONSTER_ARRAYCOUNT;
            break;

        default:
            return;
    }

    DEH_printf("VSLM_AllocateRandoPool: Allocating memory pool for monster randomizer (%s).\n",
               (gamemode == commercial) ? "Doom 2" : "Doom");
    randopool = Z_Malloc(poolsize * sizeof(mobjtype_t), PU_STATIC, NULL);
    memcpy(randopool, monsterTypes, poolsize * sizeof(mobjtype_t));

    // (Deprecated)
    // Randomizer pool specific to chaos mode
    /*randopool_chaos = Z_Malloc(
        (poolsize + BOSS_ARRAYCOUNT) *
                                   sizeof(mobjtype_t),
                               PU_STATIC, NULL);
    memcpy(randopool_chaos, monsterTypes, poolsize * sizeof(mobjtype_t));
    memcpy(randopool_chaos + poolsize, bossTypes,
           BOSS_ARRAYCOUNT * sizeof(mobjtype_t));*/
}


// Kill all monsters in current map
// Not needed in Crispy Doom
#if 0
void VSLM_KillAllMonsters(int *outKillCount)
{
    int killcount = 0;
    thinker_t *th;

    for (th = thinkercap.next; th != &thinkercap; th = th->next) {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mobj_t *mo = (mobj_t*)th;
            if(mo->flags & MF_COUNTKILL || mo->type == MT_SKULL)
            {
                if(mo->health > 0)
                {
                    P_DamageMobj(mo, NULL, NULL, 50000);
                    killcount++;
                }
                if(mo->type == MT_PAIN)
                {
                    A_PainDie(mo);
                    P_SetMobjState(mo, S_PAIN_DIE6);
                }
            }
        }
    }

    numbraintargets = -1;

    if(outKillCount != NULL)
      *outKillCount = killcount;
}
#endif


void VSLM_ToggleInvisMonsters(boolean *outInvisFlag)
{
    thinker_t *th;
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1) P_MobjThinker)
        {
            mobj_t *mo = (mobj_t *) th;
            if (VSLM_IsMonster(mo) && mo->type != MT_SHADOWS)
            {
                if(!*outInvisFlag)
                {
                    mo->flags |= MF_SHADOW;
                }
                else
                {
                    mo->flags &= ~MF_SHADOW;
                }

            }
        }
    }

    if(*outInvisFlag)
    {
        *outInvisFlag = false;
        return;
    }
    *outInvisFlag = true;
}

void VSLM_FireCheat(void)
{
    thinker_t *th;
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1) P_MobjThinker)
        {
            mobj_t *mo = (mobj_t *) th;
            if(VSLM_IsMonster(mo))
            {
                VSLM_SpawnFire(mo);
            }
        }
    }
}

boolean VSLM_IsMonster(mobj_t *actor)
{
    switch(actor->type)
    {
        case MT_POSSESSED: // Former human
        case MT_SHOTGUY:   // Forman human sergeant
        case MT_TROOP:     // Imp
        case MT_SERGEANT:  // Pinky/Demon
        case MT_SHADOWS:   // Spectre
        case MT_SKULL:     // Lost soul
        case MT_HEAD:      // Cacodemon
        case MT_BRUISER:   // Baron of Hell
        case MT_CYBORG:    // Cyberdemon
        case MT_SPIDER:    // Spider-Mastermind
        // Doom II enemies
        case MT_CHAINGUY:  // Former chaingunner
        case MT_KNIGHT:    // Hell Knight
        case MT_UNDEAD:    // Revenant
        case MT_FATSO:     // Mancubus
        case MT_BABY:      // Arachnotron
        case MT_PAIN:      // Pain elemental
        case MT_VILE:      // Arch-vile
        case MT_WOLFSS:    // Wolfenstein SS
        case MT_KEEN:      // Commander Keen
        return true;

        default:
        return false;
    }
}

const char *VSLM_GetCurrentMap(void)
{
    switch (gamemode)
    {
        // DOOM/Ultimate DOOM
        case shareware:
        case registered:
        case retail:
            M_snprintf(mapName, sizeof(mapName), "E%dM%d", gameepisode,
                       gamemap);
            break;

        // DOOM II
        case commercial:
            M_snprintf(mapName, sizeof(mapName), "%s%d",
                       (gamemap < 10) ? "MAP0" : "MAP", gamemap);
            break;

        default:
            // Unknown game, return blank
            return "";
    }
    return mapName;
}

// Randomizes all monsters in current map. Returns amount of
// monsters randomized
int VSLM_RandomizeMonsters(boolean chaos, boolean teleptFog)
{
    thinker_t *th;
    mobj_t *actor, *fog;
    mobjtype_t newtype;
    int randindex;
    byte spawnboss;
    int monstercount;
    const int BOSS_CHANCE = 100;


    // Make sure rando pools are allocated
    if (!randopool)
        VSLM_AllocateRandoPool();

    monstercount = 0;
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 != (actionf_p1) P_MobjThinker)
            continue; // Nope

        actor = (mobj_t *) th;

        // Make sure the Mobj is actually an enemy and not dead
        if ((actor->flags & MF_COUNTKILL || actor->type == MT_SKULL) &&
            actor->health >= 0)
        {
            if (chaos)
                spawnboss = (VSLM_Rand(0, BOSS_CHANCE) == 1) ? 1 : 0;
            else if (VSLM_IsBossMonster(actor))
                spawnboss = 1;
            else
                spawnboss = 0;

            // Choose random monster type

            //randindex = VSLM_DoomRand() % poolsize;
            randindex = (spawnboss == 1) ? VSLM_DoomRand() % BOSS_ARRAYCOUNT
                                         : VSLM_DoomRand() % poolsize;
            newtype =
                (spawnboss == 1) ? bossTypes[randindex] : randopool[randindex];

            // Spawn fire?
            if (teleptFog)
            {
                fog = P_SpawnMobj(actor->x, actor->y, actor->z, (gamemode == commercial) ? MT_SPAWNFIRE : MT_TFOG);
            }

            // Update actor's type and re-init properties
            actor->type = newtype;
            actor->info = &mobjinfo[newtype];
            actor->flags = actor->info->flags;

            P_SetMobjState(actor, actor->info->spawnstate);
            actor->health = actor->info->spawnhealth;
            monstercount++;

            // Indicator sound effect and debug
            if (spawnboss == 1)
            {
                S_StartSound(actor, (gamemode == commercial) ? sfx_boscub : sfx_telept);

                if (M_CheckParm("-vslmdebug"))
                    DEH_printf("[VSLM DEBUG]: VSLM_RandomizeMonsters: Spawned "
                               "boss monster (%s)\n",
                               (newtype == MT_SPIDER) ? "Spider Mastermind"
                                                      : "Cyberdemon");
            }
        }
    }
    return monstercount;
}

// May or may not be pointless
#if 0
boolean VSLM_EnemyRevivable(mobj_t *actor)
{
    switch (actor->type)
    {
        case MT_SKULL:
        case MT_VILE:
        case MT_CYBORG:
        case MT_SPIDER:
        case MT_KEEN:
            return false;
            break;

        default:
            return true;
    }
    return false;
}
#endif
#endif