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
	Randomizer utilities
*/
#include "vslm.h"
#include "vslm_doom_randomizer.h"

#include <doom/p_local.h>

/* ----------------------------------- */
/* Globals --------------------------- */
/* ----------------------------------- */

extern odds_t *rando_odds;

/* ----------------------------------- */
/* Functions ------------------------- */
/* ----------------------------------- */

// ------------------------
// VSLM_Rando_GetMonster
// ------------------------
mobj_t *VSLM_Rando_GetMonster(mobjtype_t *type)
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