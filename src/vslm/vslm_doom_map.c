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
	Functions related to Doom maps
*/
#include "vslm.h"

#include <doomstat.h>
#include <g_game.h>
#include <p_local.h>
#include <r_defs.h>

// ------------------------
// VSLM_TriggerTag666
// ------------------------
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
                    if (tag667)
                    {
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