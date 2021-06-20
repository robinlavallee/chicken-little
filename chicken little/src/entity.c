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
#include "mikmod.h"
#include "audiomanager_c_bridge.h"


static int ghosthandler(ENTITY *entity)
{
    switch(entity->state)
    {   case ENTITY_STATE_WAITING:
            // Wait for our animation finish, then delete us!
            if(!Anim_Active(&entity->anim))
                Entity_KillSelf(entity);
        break;

        default:
            _mmlog("GhostEntity: Bad state: %d",entity->state);
        break;
    }
    return STATE_PRIORITY_LOW;
}

ENTITY entity_ghost = 
{
    "Ghost",
    ENTITY_STATE_WAITING,
    0,
    NULL,
    ghosthandler

    // rest is NULL - I think - I *hope* - this works
};


// =====================================================================================
    ENTITY *Entity_Spawn(ENTITY **entitylist, const ENTITY *src, int size)
// =====================================================================================
// Creates a new entity based on the entity 'template' provided.  The size of the entity
// must be provied because of our 'interited' style of pseudo-c++ coding.
// Notes:
//  - If 'size' is 0 then sizeof(ENTITY) is used.
{
    ENTITY *newp;

    if(!src) return NULL;

    newp = (ENTITY *)_mm_calloc(1, size ? size : sizeof(ENTITY));

    // Copy the complete contents of the entity
    *newp = *src;

    // Add us to the state list!

    newp->next        = *entitylist;
    *entitylist       = newp;
    if(newp->next) newp->next->prev = newp;

    newp->entlist     =  entitylist;

    return newp;
}


// =====================================================================================
    void Entity_Kill(ENTITY *src)
// =====================================================================================
{
    if(!src) return;

    if(src->deinitialize) src->deinitialize(src);

    // Remove us from the linked list!

    if(src->prev)
        src->prev->next = src->next;
    else
        *src->entlist = src->next;

    if(src->next)
        src->next->prev = src->prev;

    _mm_free(src, NULL);
}

// =====================================================================================
    void Entity_KillList(ENTITY **entitylist)
// =====================================================================================
{
    ENTITY   *cruise;

    cruise = *entitylist;
    
    while(cruise)
    {
        ENTITY *next = cruise->next;
        
        if(cruise->deinitialize) cruise->deinitialize(cruise);
        _mm_free(cruise, NULL);

        cruise = next;
    }

    *entitylist = NULL;
}


static volatile BOOL killedself;
static ENTITY       *nextofkin;


// =====================================================================================
    void Entity_KillSelf(ENTITY *src)
// =====================================================================================
{
    nextofkin  = src->next;
    killedself = 1;
    Entity_Kill(src);
}


// =====================================================================================
    void Entity_SetAnimation(ENTITY *dest, ANIMATION *animation)
// =====================================================================================
{
    dest->anim = *animation;
}


// =====================================================================================
    void Entity_SetState(ENTITY *entity, int type)
// =====================================================================================
// Sets the current state of the entity to the specified state.  Resets the timeout of
// the entity, which means the new state will be called immediately.
{
    // Leave the old state
    //if(state->leave) state->leave(state, App_GetTime());

    entity->state     = type;

    // Only rset the timeout if we aren't already lagging behind!
    if(entity->timeout > 0) entity->timeout = 0;
}

typedef struct ENT_SOUND
{
    int              sample;
    ENTITY           *entity;
    struct ENT_SOUND *next;

} ENT_SOUND;


static uint      es_count = 0;
static ENT_SOUND entsound[10];

// =====================================================================================
    void Entity_PlaySound(ENTITY *src, int sample)
// =====================================================================================
{
    if(es_count)
    {   uint    i;
        for(i=es_count; i; i--)
            if((src == entsound[i-1].entity) && (sample == entsound[i-1].sample)) return;
    }

    entsound[es_count].sample = sample;
    entsound[es_count].entity = src;
    es_count++;
}


// =====================================================================================
    void Entity_Render(ENTITY *entitylist)
// =====================================================================================
// Ever wanted to display an entity rather than just play around with all of this pseudo-
// virtual bullcrap of creating, destroying, state changing, and crap?  well, here ye are.
// Notes:
//  - I did not include an x/y offset option.  Instead, use the VDRIVER's surface->setclipper
//    function to change the effective offset of the screen to be displayed to.
{
    ENTITY   *cruise;

    if(cruise = entitylist)
    {   while(cruise)
        {   Anim_Blitter(&cruise->anim);
            if(cruise->render) cruise->render(cruise);
            cruise = cruise->next;
        }
    }
}


// =====================================================================================
    int Entity_Update(ENTITY **entitylist, ulong timepass)
// =====================================================================================
{
    int       loval = 0xffffffl;
    ENTITY   *cruise;

    if(cruise = *entitylist)
    {   while(cruise)
        {   if(cruise->state != STATE_NONE && cruise->handler)
            {   if(cruise->timeout)
                    cruise->timeout -= timepass;

                if(cruise->timeout <= 0)
                {   int  t;
                
                    killedself = 0;
                    
                    t = cruise->handler(cruise);
                    if(killedself)
                    {   cruise = nextofkin;
                        continue;
                    }

                    if(t)
                        cruise->timeout += t;
                    else
                        cruise->timeout = 0;
                }
                if(cruise->timeout < loval) loval = cruise->timeout;
            }

            // Update the animation info if it is present.
            Anim_Update(&cruise->anim, timepass);

            cruise = cruise->next;
        }
    }

    // Check the sound system, see if there are any queued sounds that we
    // need to play.  Notes on this thing:
    //  - We check loval to see if it is above 0 first.  If not, then we are 'running
    //    behind' and playing the queued sounds would only slow down the system or get
    //    things confused.
    // Robin: We don't really need this system anymore since we are using XAudioBuffer now with multiple XSourceVoice
    // but keeping it now for compatibility

    if(es_count && (loval > 0))
    {   uint   i;
      for (i = 0; i < es_count; i++)
        XAudioBuffer_Play(entsound[i].sample);
        
        es_count = 0;

        // Preempt the mixer here, to activate declicking features!
        // [...]
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
    {   stateinfo->oldstate = stateinfo->state;
        stateinfo->state    = type;
        if(stateinfo->animstate->sequence)
            Anim_SetSequence(&stateinfo->anim, &stateinfo->animstate->sequence[type], ANIMF_FASTSHADOW);

        stateinfo->timeout = 0;
        App_Paint();
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
// This updates the entire field of gamepieces in the gameplay area.  Because of the way
// the piece timings work, we have to ensure that all pieces in 'GP_STATE_WAITING' get
// called at regular intervals (ie, they all get called at the same time).  This makes
// logic in the game marketably easier.
{
    GAME_INFO  *gi     = player->gp;
    GAMEPIECE *cruise = gi->gamearray;
    int        x,y;
    int        loval = 0xffffffl;

    BOOL       updateWaiting = 0;

    // Find out if it is time to trigger our waiting timer or not.

    gi->timeout -= timepass;
    if(gi->timeout <= 0)
    {   updateWaiting = 1;
        gi->timeout += STATE_PRIORITY_NORMAL;
    }

    for(y=0; y<SIZEY; y++)
    {   for(x=0; x<SIZEX; x++, cruise++)
        {   if(cruise->type != GP_EMPTY)
            {   
                // State update sequence:
                // (a) we update the state handlers
                // (b) we update the animation hoopla.

                if(cruise->stateinfo.handler)
                {   if(cruise->stateinfo.state != GP_STATE_WAITING)
                    {   
                        // This is where we update those game pieces which are running on thier
                        // own timers.
                    
                        if(cruise->stateinfo.timeout)
                            cruise->stateinfo.timeout -= timepass;

                        if(cruise->stateinfo.timeout <= 0)
                        {   int t;
                    
                            remapping = FALSE;
                            t = cruise->stateinfo.handler(player, x, y);

                            if(remapping)
                            {   player->gp->gamearray[gpidx(remap_x, remap_y)].stateinfo.timeout = cruise->stateinfo.timeout + t;
                                t = 0;
                            }

                            if(t)
                                cruise->stateinfo.timeout += t;
                            else
                                cruise->stateinfo.timeout = 0;

                            if(cruise->stateinfo.timeout < loval) loval = cruise->stateinfo.timeout;
                        }
                    } else if(updateWaiting)
                    {
                        int   t;
                        
                        // Update those entities which are running on the 'waiting' timer.

                        t = cruise->stateinfo.handler(player, x, y);
                        if(cruise->stateinfo.state != GP_STATE_WAITING)
                        {   
                            // We switched from waiting to something else, so set the individual
                            // timeout for this gamepiece.
                            
                            cruise->stateinfo.timeout = t;
                            if(t < loval) loval = t;
                        }
                    }
                }

                // Update the animation info if it is present.
                Anim_Update(&cruise->stateinfo.anim, timepass);
            }
        }
    }

    return loval;
}
