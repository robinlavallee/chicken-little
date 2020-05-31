/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: state.c

  Handles the updating of states of entities in the game engine.

  There are currently two types of entities, and hence two state managers:
   - Player entities
   - Gamepiece entities
*/

#include "chicken.h"

// =====================================================================================
void _state_setstate(STATE *state, int type)
// =====================================================================================
{
    // Leave the old state
    //if(state->leave) state->leave(state, App_GetTime());

    state->state = type;
}

// =====================================================================================
int _state_update(STATE *stateinfo, ulong timepass)
// =====================================================================================
{
    int  loval = 0xffffffl;
    
    if(stateinfo->state != STATE_NONE && stateinfo->handler)
    {   if(stateinfo->countdown)
            stateinfo->countdown -= timepass;

        if(stateinfo->countdown <= 0)
        {   int  t = stateinfo->handler(stateinfo);
            if(t)
                stateinfo->countdown += t;
            else
                stateinfo->countdown = 0;
        }
        if(stateinfo->countdown < loval) loval = stateinfo->countdown;
    }

    return loval;
}

// =====================================================================================
void _gpstate_setstate(GPSTATE *stateinfo, int type)
// =====================================================================================
{
    // Leave the old state
    //if(state->leave) state->leave(player, state, x, y, timeGetTime());
    
    if(stateinfo->state != type)
    {   stateinfo->state = type;
        if(stateinfo->animstate->sequence)
            Anim_SetSequence(&stateinfo->anim, &stateinfo->animstate->sequence[type]);
    }
}



volatile BOOL remapping;
volatile int  remap_x, remap_y;

// =====================================================================================
void _gpstate_remap(int dx, int dy)
// =====================================================================================
{
    remapping = TRUE;

    remap_x = dx;
    remap_y = dy;

}

// =====================================================================================
int _gpstate_update(PLAYER *player, ulong timepass)
// =====================================================================================
{
    GAMEPIECE *cruise = player->gamearray;
    int        x,y;
    int       loval = 0xffffffl;

    for(y=0; y<SIZEY; y++)
    {   for(x=0; x<SIZEX; x++, cruise++)
        {   if(cruise->type != GP_EMPTY)
            {   
                // State update sequence:
                // (a) we update the state handlers
                // (b) we update the animation hoopla.

                if(cruise->stateinfo.handler)
                {   
                    // Call the state handler since it is present.
                    
                    if(cruise->stateinfo.countdown)
                        cruise->stateinfo.countdown -= timepass;

                    if(cruise->stateinfo.countdown <= 0)
                    {   int t;
                    
                        remapping = FALSE;
                        t = cruise->stateinfo.handler(player->gamearray, x, y);

                        if(remapping)
                        {   player->gamearray[gpidx(remap_x, remap_y)].stateinfo.countdown = cruise->stateinfo.countdown + t;
                            t = 0;
                        }
                        
                        if(t)
                            cruise->stateinfo.countdown += t;
                        else
                            cruise->stateinfo.countdown = 0;

                        if(cruise->stateinfo.countdown < loval) loval = cruise->stateinfo.countdown;
                    }
                }

                // Update the animation info if it is present.
                Anim_Update(&cruise->stateinfo.anim, timepass);
            }
        }
    }

    return loval;
}
