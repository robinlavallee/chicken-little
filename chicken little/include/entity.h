/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: states.h

  State-related strcutures, constants, and prototypes.

  Contains:
   (a) STATE - general state structure for use with most types of independant entities,
       in particular the PLAYER.

   (b) GPSTATE - then gamepiece state.  Special because of the static way in which we
       store the gamepiece playfield array, and the frequency in which it is called.
       Hence, it benefits greatly from being somewhat specialized.

  NOTES (Important!):

   - For entities to work properly when using STATE structure, they *must* have it as
     the first element in their structure (see PLAYER in player.h for an example).  If
     not, then the entity's state callback will get crap passed to it!

   - Due to the almost-inherited nature of the state structure, I created a nifty set of
     macros which make things feel a little more like a C++ structure inheritence sit-
     uation (only without the stupid typecasting warnings!).  For mor information, see
     the actual macro defines at the bottom of this file.
*/


#ifndef _CLSTATES_H_
#define _CLSTATES_H_

#include "mmtypes.h"
#include "clres.h"


// Default Entity States
// These are important states that the entity management system will expect to be present
// in one compacity or another.  Whenever a new entity is spawned the state will be
// ENTITY_STATE_CREATE(unless the template sets the state otherwise).  Whenever something
// forcefully destroys an entity, the state is et to ENTITY_STATE_DESTROY.

enum
{   STATE_NONE = 0,
    ENTITY_STATE_WAITING,
    ENTITY_STATE_DESTROY,
    ENTITY_NUMSTATES
};

// ----------------------
// State priority defines
// ----------------------
// We need to use these appropriately in order to reduce CPU consumption by waiting
// loops (especially ones that must check arrays and not just a single conditional).
// Generally speaking, there should be very few situations which merit the use of
// the CRITICAL priority.

#define STATE_PRIORITY_LOW       25
#define STATE_PRIORITY_NORMAL    10
#define STATE_PRIORITY_HIGH       5
#define STATE_PRIORITY_CRITICAL   1


// =====================================================================================
    typedef struct ENTITY
// =====================================================================================
// Generalized state information which can be used on any suitable 'independant' entity.
{
    CHAR      *name;

	int       state;             // current state.
    int       timeout;         // waiting period before next call to handler.

    // the state handler functions 
    // handler - (called when waiting period in 'timeout' expires)
    // leave   - called when a new state is being entered (or on the creation of the entity)

    void     (*deinitialize) (void *entity);
    int      (*handler)      (void *entity);
    int      (*leave)        (void *entity, int newstate);

    void     (*render)       (void *entity);

    // Animation information
    // not sure if I want to keep it housed here, but it works for now.

    ANIM_STATE  *animstate;      // array of animations for each state type.
    ANIMATION    anim;           // our gamepiece animation struct

    struct ENTITY   *next,
                    *prev;

    struct ENTITY  **entlist;

} ENTITY;


extern ENTITY   entity_ghost;

extern void     Entity_SetState(ENTITY *entity, int type);
extern int      Entity_Update(ENTITY **entlist, ulong timepass);


extern ENTITY  *Entity_Spawn(ENTITY **entitylist, const ENTITY *src, int size);
extern void     Entity_Kill(ENTITY *src);
extern void     Entity_KillList(ENTITY **entitylist);
extern void     Entity_KillSelf(ENTITY *src);

extern void     Entity_SetAnimation(ENTITY *dest, ANIMATION *animation);
extern void     Entity_Render(ENTITY *entitylist);

extern void     Entity_PlaySound(ENTITY *src, int sample);

#endif
