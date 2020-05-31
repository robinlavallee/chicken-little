/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: birdies.c


*/

#include "entity.h"
#include "app.h"


#define NUM_BIRDIES  (RES_BIRDIES_NUM / 4)

// Notes:
//  - The stationary pirch state is the same as the entity waiting state.

enum
{
    BIRD_STATE_PIRCHED = ENTITY_STATE_WAITING,
    BIRD_STATE_FLYING_LEFT = ENTITY_NUMSTATES,
    BIRD_STATE_FLYING_RIGHT,
    BIRD_STATE_PIRCHING_LEFT,
    BIRD_STATE_PIRCHING_RIGHT,

    BIRD_STATE_DANCE,           // a little dance animation.  cute.
    BIRD_STATE_CLEAN,           // a clean bird is next to godliness.

    BIRD_NUMSTATES
};

//SPRITE redbird[NUM_BIRDIES], bluebird[NUM_BIRDIES];

static ANIM_STATE redbird[BIRD_NUMSTATES], bluebird[BIRD_NUMSTATES];


typedef struct BIRD
{
    ENTITY   entity;

    int     xvec, yvec;      // direction + velocity of the bird.
    int     flapspeed;       // flap speed (# of flaps per second)


} BIRD;

// =====================================================================================
    static int Bird_StateHandler(BIRD *title)
// =====================================================================================
{
    switch(title->entity.state)
    {
        // =============================================================================
        case BIRD_STATE_FLYING_LEFT:
        case BIRD_STATE_FLYING_RIGHT:
        // =============================================================================
        // A bird's flying animation is independant of the actual direction it is travel-
        // ling.  So a bird could fly backwards.  Hence, both states use the same logic
        // (but show different animations!)

        return STATE_PRIORITY_LOW;
    }

    return STATE_PRIORITY_LOW;
}


ENTITY Entity_Bird =
{
    "Birdies",
    ENTITY_STATE_WAITING,
    0,
    NULL,
    Bird_StateHandler,
};


// =====================================================================================
    BIRD *Bird_Spawn(ENTITY **entlist, uint color, uint state, int xloc, int yloc, int xvec, int yvec, uint flapspeed)
// =====================================================================================
// Spawn a new little birdie.
{
    BIRD   *bird;

    bird = (BIRD *)Entity_Spawn(entlist, &Entity_Bird, sizeof(BIRD));

    return bird;
}

