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
	Functions related to Doom enemies
*/
#include "vslm.h"

#include <doomstat.h>
#include <p_local.h>
#include <p_mobj.h>
#include <r_defs.h>
#include <s_sound.h>

// ------------------------
// Prototypes
// ------------------------
boolean VSLM_IsMonsterGibbed(mobj_t *actor);

// ------------------------
// VSLM_RespawnMonster
// ------------------------
void VSLM_RespawnMonster(mobj_t* actor)
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
        if (info->xraisestate != S_NULL && VSLM_IsMonsterGibbed(actor))
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


// ------------------------
// VSLM_ReviveMonsters
// ------------------------
int VSLM_ReviveMonsters(void)
{
    thinker_t *th;
    int count = 0;
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1) P_MobjThinker)
        {
            mobj_t *mo = (mobj_t *) th;


            // Lost souls + pain elementals can't be revived, so skip
            if (mo->type == MT_SKULL || mo->type == MT_PAIN)
                continue;

            if ((mo->flags & MF_COUNTKILL) && mo->health <= 0)
            {
                VSLM_RespawnMonster(mo);
                count++;
            }
        }
    }
    return count;
}

// ------------------------
// VSLM_SpawnFire
// ------------------------
void VSLM_SpawnFire(mobj_t *actor)
{
    if (gamemode == commercial)
    {
        P_SpawnMobj(actor->x, actor->y, actor->z, MT_SPAWNFIRE);
    }
    // No fire sprite in DOOM 1 (above check prevents a crash)
}

// ------------------------
// VSLM_IsMonsterGibbed
// ------------------------
boolean VSLM_IsMonsterGibbed(mobj_t *actor)
{
    // If mobj has no health (dead), and is a monster
    if (actor->health <= 0 && (actor->flags & MF_COUNTKILL))
    {
        switch (actor->type)
        {
            case MT_POSSESSED:
            case MT_SHOTGUY:
            case MT_TROOP:
                if (actor->frame == 20)
                    return true;
                break;

            case MT_CHAINGUY:
                if (actor->frame == 19)
                    return true;
                break;

            case MT_WOLFSS:
                if (actor->frame == 21)
                    return true;
                break;

            default:
                return false;
        }
    }
    return false;
}

// ------------------------
// VSLM_GetMonsterType
// ------------------------
const char* VSLM_GetMonsterType(mobjtype_t type)
{
    switch (type)
    {
        case MT_POSSESSED:
            return "Former human";

        case MT_SHOTGUY:
            return "Former sergeant";

        case MT_CHAINGUY:
            return "Former commando";

        case MT_TROOP:
            return "Imp";

        case MT_SERGEANT:
            return "Demon";

        case MT_SHADOWS:
            return "Spectre";

        case MT_SKULL:
            return "Lost soul";

        case MT_HEAD:
            return "Cacodemon";

        case MT_KNIGHT:
            return "Hell Knight";

        case MT_BRUISER:
            return "Baron of Hell";

        case MT_BABY:
            return "Arachnotron";

        case MT_PAIN:
            return "Pain elemental";

        case MT_UNDEAD:
            return "Revenant";

        case MT_FATSO:
            return "Mancubus";

        case MT_VILE:
            return "Arch-vile";

        case MT_SPIDER:
            return "Spider Mastermind";

        case MT_CYBORG:
            return "Cyberdemon";

        case MT_WOLFSS:
            return "Wolfenstein SS";

        case MT_KEEN:
            return "Commander Keen";

        default:
            return NULL;
    }
}

// ------------------------
// VSLM_MonsterFound
// ------------------------
boolean VSLM_MonsterFound(mobjtype_t type)
{
    thinker_t *th;
    mobj_t *actor;
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            actor = (mobj_t *) th;
            if (actor->type == type)
                return true;
        }
    }
    return false;
}

// ------------------------
// VSLM_GetRandomMonster
// ------------------------
mobj_t *VSLM_GetRandomMonster(mobjtype_t *type)
{
    thinker_t *th;
    mobj_t *actor, *selected;
    do
    {
        for (th = thinkercap.next; th != &thinkercap; th = th->next)
        {
            if (th->function.acp1 != (actionf_p1) P_MobjThinker)
                continue;
            actor = (mobj_t *) th;
            if (actor->flags & MF_COUNTKILL || actor->type == MT_SKULL)
            {
                if (type)
                {
                    if (actor->type == *type)
                    {
                        selected = actor;
                        if (VSLM_DoomRand() == 255)
                            return selected;
                    }
                }
                else
                {
                    selected = actor;
                    if (VSLM_DoomRand() == 255)
                        return selected;
                }
            }
        }
    } while (VSLM_DoomRand() != 255);
    return selected;
}

// ------------------------
// VSLM_ChangeMonsterType
// ------------------------
void VSLM_ChangeMonsterType(mobj_t* actor, mobjtype_t type)
{
    // Retain ambush flag if any
    int ambush = (actor->flags & MF_AMBUSH) ? MF_AMBUSH : 0;

    actor->type = type;
    actor->info = &mobjinfo[type];
    actor->flags = actor->info->flags;
    actor->health = actor->info->spawnhealth;
    if (ambush != 0)
        actor->flags ^= ambush;

    P_SetMobjState(actor, actor->info->spawnstate);
}