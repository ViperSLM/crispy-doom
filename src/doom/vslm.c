#include "sounds.h"

#include "d_mode.h"
#include "m_fixed.h"
#include "p_local.h"
#include "p_spec.h"
#include "p_mobj.h"
#include "r_defs.h"

extern int extrakills;
extern int numbraintargets;
extern void A_PainDie(mobj_t *);

// Game-related externs
extern GameMode_t gamemode;
extern GameMission_t gamemission;
extern skill_t gameskill;
extern int gameepisode;
extern int gamemap;

// Thinker externs
extern thinker_t thinkercap;
extern void P_InitThinkers(void);
extern void P_AddThinker(thinker_t *thinker);
extern void P_RemoveThinker(thinker_t *thinker);
extern mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
extern void P_RemoveMobj(mobj_t *th);
extern boolean P_SetMobjState(mobj_t *mobj, statenum_t state);
extern void P_MobjThinker(mobj_t *mobj);
extern void P_DamageMobj(mobj_t *target, mobj_t *inflictor, mobj_t *source,
                         int damage);
                         
// External functions
extern void S_StartSound(void *origin, int sound_id);
extern void G_ExitLevel(void);
extern void ST_PrintMsg(const char *format, ...);
extern int M_CheckParm(const char *check);
extern boolean P_CheckPosition(mobj_t * thing, fixed_t x, fixed_t y);
extern subsector_t *R_PointInSubsector(fixed_t x, fixed_t y);

// Forward declarations for local functions
void VSLM_FireCheat(void);
void VSLM_RespawnMonster(mobj_t *actor);
void VSLM_ReviveMonsters(int *outReviveCount);
void VSLM_SpawnFire(mobj_t *actor);
void VSLM_ToggleInvisMonsters(boolean *outInvisFlag);
void VSLM_TriggerTag666(boolean tag667);
boolean VSLM_IsMonster(mobj_t *actor);
boolean VSLM_IsMonsterGibbed(mobj_t *actor);
// ----------------------------------------

// Message buffer

// const char *VSLM_GetCurrentMap(void)
// {
//     switch(gamemode)
//     {
//         case shareware:
//         case registered:
//         case retail:
//             ST_PrintMsg("E%dM%d", gameepisode, gamemap);
//         break;

//         case commercial:
//             ST_PrintMsg("%s%d", (gamemap < 10) ? "MAP" : "MAP0", gamemap);
//         break;

//         default:
//         ST_PrintMsg(msg, sizeof(msg), "UNKNOWN");
//     }
//     return ST_GetMsgBuffer();
// }

/*
    What Tag 666/667 does in each game and map:

    -- DOOM/Ultimate DOOM
    E1M8: Lowers floor (triggered after killing the two Barons of Hell)
    E2M8: Ends the level (triggered after killing the Cyberdemon)
    E3M8: Ends the level (triggered after killing the Spider Mastermind)

    (Specific to Ultimate DOOM)
    E4M6: Open door [Fast] (triggered after killing the Cyberdemon)
    E4M8: Lowers floor (triggered after killing the Spider Mastermind)

    -- DOOM II
    MAP07: Lowers floor (triggered after killing all Mancubus enemies)
    MAP07 (Tag 667): Raises floor (triggered after killing all Arachnotron enemies)
    MAP32: Opens door (triggered after killing the four Commander Keens)
*/

// Trigger Tag 666/667 events
void VSLM_TriggerTag666(boolean tag667)
{
    // Dummy lineDef, used to set and trigger the tag
    line_t a;
    a.tag = 666;

    switch (gamemode)
    {
        // DOOM/Ultimate DOOM (including the Shareware version)
        case shareware:
        case registered:
        case retail:
            switch (gameepisode)
            {
                // Episode 1
                case 1:
                    if (gamemap == 8) // E1M8: Phobos Anomaly
                        EV_DoFloor(&a, lowerFloorToLowest);
                    break;

                // Episodes 2 & 3
                case 2:
                case 3:
                    if (gamemap == 8) // E2M8: Tower of Babel + E3M8: Dis
                        G_ExitLevel();
                    break;

                // Episode 4
                case 4:
                    switch (gamemap)
                    {
                        case 6: // E4M6: Against Thee Wickedly
                            EV_DoDoor(&a, vld_blazeOpen);
                            break;

                        case 8: // E4M8: Unto The Cruel
                            EV_DoFloor(&a, lowerFloorToLowest);
                            break;
                    }
                    break;
            }
            break;

        // DOOM II
        case commercial:
            switch (gamemap)
            {
                // MAP07: Dead Simple
                case 7:
                    if(tag667) {
                        a.tag = 667;
                        EV_DoFloor(&a, raiseToTexture);
                        return;
                    }
                    EV_DoFloor(&a, lowerFloorToLowest);
                break;

                // MAP30: Icon of Sin
                // (Not actually tag 666, but added for convenience)
                case 30:
                    G_ExitLevel();
                break;

                // MAP32: Grosse
                case 32:
                    EV_DoDoor(&a, vld_open);
                break;
            }
            break;

        default:
            return;
    }
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

void VSLM_RespawnMonster(mobj_t *actor)
{
    // Modified P_NightmareRespawn
    mobjinfo_t *info;
    fixed_t x, y, z;
    subsector_t *ss;
    mobj_t *fog;
    mapthing_t *mthing;
    statenum_t state;

    x = actor->spawnpoint.x << FRACBITS;
    y = actor->spawnpoint.y << FRACBITS;

    if (!P_CheckPosition(actor, x, y))
        return; // Position is occupied

    // Does actor have a raisestate?
    info = actor->info;
    if (info->raisestate != S_NULL)
    {
        // Does monster have a xraisestate (gibbed)?
        if(info->xraisestate != S_NULL && VSLM_IsMonsterGibbed(actor))
            state = info->xraisestate;
        else
            state = info->raisestate;

        P_SetMobjState(actor, state);
        actor->height = info->height;
        actor->radius = info->radius;
        actor->flags = info->flags;
        actor->health = info->spawnhealth;
        actor->target = NULL;
        actor->tracer = NULL;
        S_StartSound(actor, sfx_slop);
        return;
    }
    // -- Fallback (teleport)

    // Spawn teleport fog at old spot
    fog = P_SpawnMobj(actor->x, actor->y, actor->subsector->sector->floorheight,
                      MT_TFOG);
    // Teleport sound
    S_StartSound(actor, sfx_telept);

    // Spawn teleport fog at new spot
    ss = R_PointInSubsector(x, y);
    fog = P_SpawnMobj(x, y, ss->sector->floorheight, MT_TFOG);
    S_StartSound(fog, sfx_telept);

    // Spawn new monster
    mthing = &actor->spawnpoint;

    if (actor->info->flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else
        z = ONFLOORZ;

    // Inherit attributes from deceased one
    fog = P_SpawnMobj(x, y, z, actor->type);
    fog->spawnpoint = actor->spawnpoint;
    fog->angle = ANG45 * (mthing->angle / 45);

    // Count respawned monsters
    extrakills++;

    if (mthing->options & MTF_AMBUSH)
        fog->flags |= MF_AMBUSH;

    fog->reactiontime = 18;

    // Remove old monster
    P_RemoveMobj(actor);
}

void VSLM_ReviveMonsters(int *outReviveCount)
{
    thinker_t *th;
    int count;
    count = 0;
    for(th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if(th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mobj_t *mo = (mobj_t*)th;
            if((mo->flags & MF_COUNTKILL) && mo->health <= 0)
            {
                VSLM_RespawnMonster(mo);
                count++;
            }
        }
    }

    if(outReviveCount != NULL)
        *outReviveCount = count;
}

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

void VSLM_SpawnFire(mobj_t *actor)
{
    if(gamemode == commercial)
    {
        P_SpawnMobj(actor->x, actor->y, actor->z, MT_SPAWNFIRE);
    }
    // No fire sprite in Ultimate DOOM (above check prevents a crash)
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

// Has a monster been gibbed?
boolean VSLM_IsMonsterGibbed(mobj_t *actor)
{
    // If mobj has no health (dead), and is a monster
    if(actor->health <= 0 && (actor->flags & MF_COUNTKILL))
    {
        switch(actor->type)
        {
            case MT_POSSESSED:
            case MT_SHOTGUY:
            case MT_TROOP:
                if(actor->frame == 20)
                    return true;
            break;

            case MT_CHAINGUY:
                if(actor->frame == 19)
                    return true;
            break;

            case MT_WOLFSS:
                if(actor->frame == 21)
                    return true;
            break;

            default:
                return false;
        }
    }
    return false;
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