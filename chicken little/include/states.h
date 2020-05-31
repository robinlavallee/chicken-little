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

#define STATE_NONE     0      // universal 'blank' state - might be useful.

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
    typedef struct STATE
// =====================================================================================
// Generalized state information which can be used on any suitable 'independant' entity.
{
	int       state;             // current state.
    int       countdown;         // waiting period before next call to handler.

    // the state handler functions 
    // handler - (called when waiting period in 'timeout' expires)
    // leave   - called when a new state is being entered (or on the creation of the entity)

    int      (*handler)(void *entity);
    int      (*leave)(void *entity, int newstate);

    // Animation information
    // not sure if I want to keep it housed here, but it works for now.

    ANIM_STATE  *animstate;      // array of animations for each state type.
    ANIMATION    anim;           // our gamepiece animation struct
} STATE;


// =====================================================================================
    typedef struct GPSTATE
// =====================================================================================
// A speciailized state structure created especially for use by, and only by, the game
// pieces that make up each players playing field.  This was needed due to the static-
// style of array we use to define the gamefiled (which greatly simplifies many tasks
// as well as greatly enhance speed!)
{	
    int       state;       // current state.
    int       countdown;

    // the state handler functions 
    // handler - (called when waiting period in 'timeout' expires)
    // leave   - called when a new state is being entered (or on the creation of the entity)

    int      (*handler)(void *owner, int x, int y);
    int      (*leave)(void *owner, int x, int y, int newstate);

    // Animation information
    // not sure if I want to keep it housed here, but it works for now.

    ANIM_STATE  *animstate;      // array of animations for each state type.
    ANIMATION  anim;             // our gamepiece animation struct
} GPSTATE;

extern void     _state_setstate(STATE *state, int type);
extern int      _state_update(STATE *state, ulong timepass);

extern void     _gpstate_setstate(GPSTATE *stateinfo, int type);
extern int      _gpstate_update(struct PLAYER *player, ulong timepass);
extern void     _gpstate_remap(int dx, int dy);


// ----------------
//   STATE MACROS
// ----------------

// Thanks to these macros, you should never really have to use any of the functions
// above directly.  These macros make it so you can pass any entity which contains
// the STATE structure to the state API procedures.

#define State_Update(o,s)          _state_update(&(o)->stateinfo,s)
#define State_SetState(o,s)        _state_setstate(&(o)->stateinfo,s)

#define gpState_Update(p,s)        _gpstate_update(p,s)
#define gpState_SetState(o,s)      _gpstate_setstate(&(o)->stateinfo,s)
#define gpState_RemapEntity(x,y)   _gpstate_remap(x,y)


#endif
