/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: gampiece.c

  The generic gamepiece entity.
*/

#include <string.h>
#include "player.h"
#include "app.h"

#define L_GRAVITY_MAX   63      // Number of entries in our gravity table (minus 1)



// =====================================================================================
    GP_PIECEINFO gp_info[GP_NUMPIECES] =
// =====================================================================================
// This array describes our gamepieces.  The first variable is the matchtype logic they
// will use.  The second is a pointer to the entity that the end user will see.  So, in 
// theory we can just alter this structure and change how our game works - although in
// practice some other little things would probably need changed as well to make the
// game play properly (Player_GetPiece, for example).
{
    { GP_EMPTY,       NULL },
    { GP_COLOR,       &Entity_Egg },
    { GP_COLOR,       &Entity_Egg },
    { GP_COLOR,       &Entity_Egg },
    { GP_COLOR,       &Entity_Egg },
    { GP_COLOR,       &Entity_Egg },
    { GP_BROWN,       &Entity_ChaosEgg },
    { GP_BROWN,       &Entity_ChaosEgg },
    { GP_BROWN,       &Entity_ChaosEgg },
    { GP_STONE,       &Entity_Stone  },
    { GP_UNMATCHABLE, &Entity_Meteor },
};


// =====================================================================================
    static int gp_fallvec[L_GRAVITY_MAX+1] =
// =====================================================================================
// Cheap-and-Dumb Gravity Acceleration Table!
// Yes, this is actually a quite-linear table, but it works pretty darned well I think.
{
    64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024,
    1088, 1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792, 1856, 1920, 1984, 2048,
    2112, 2176, 2240, 2304, 2368, 2432, 2496, 2560, 2624, 2688, 2752, 2816, 2880, 2944, 3008, 3072,
    3136, 3200, 3264, 3328, 3392, 3456, 3520, 3584, 3648, 3712, 3776, 3840, 3904, 3968, 4032, 4096
};

// =====================================================================================
    int GamePiece_StateHandler(PLAYER *player, int x, int y)
// =====================================================================================
// This generalized gamepiece state handler will do those things which almost every
// gamepiece in the game shares:
//  (a) Gravity
{
	GAMEPIECE   *gparray = player->gp->gamearray,
	            *gp = &gparray[gpidx(x,y)];

    if((gp_info[gp->type].entity) && (gp_info[gp->type].entity->statehandler))
    {   
        int   t = gp_info[gp->type].entity->statehandler(player,x,y);
        if(t) return t;
    }

    switch(gp->stateinfo.state)
    {   case GP_STATE_WAITING:
            // Check around us for stuff and take the appropriate actions:
            // (a) if nothing is below us, then fall.

            GamePiece_CheckGravity(gp,gparray,x,y);
        break;

        case GP_STATE_GRAVITY:

            // Move our piece down some, then sleep it off
   
            // Done with the animation, so switch back to waiting

            // Gravity is based on a simple acceleration equation.  Gravity continues to
            // be applied until the egg can no longer fall freely.  The calculation
            // is based on a small table

            if((y<(SIZEY-1)) && gparray[gpidx(x,y+1)].type == GP_EMPTY)
            {   
                int   yloc;
                
                // Move our piece down some

                yloc = Anim_TranslateY(&gp->stateinfo.anim, gp_fallvec[gp->gravidx]);

                // Find out how far we have fallen.  When we fall halfway down the 'grid block'
                // then drop down into the next space.  This will allow the egg above us to
                // start his descent early - for a nice cascade effect.
                
                if(yloc > (4<<8))
                {   Anim_TranslateY(&gp->stateinfo.anim,-(30<<8)); // make sure we maintain proper location
                    GamePiece_Move(gparray,x,y,x,y+1); y++;
                    gpState_RemapEntity(x,y);
                    gp = &gparray[gpidx(x,y)];
                }

            } else
            {   
                if((Anim_GetTranslationY(&gp->stateinfo.anim) + gp_fallvec[gp->gravidx]) >= -(2<<8))
                {   
                    // Done with the animation, so home us and switch to our bounce state

                    Anim_SetTranslationY(&gp->stateinfo.anim,0);
                    gpState_SetState(gp, gp->stateinfo.oldstate);
                } else
                    Anim_TranslateY(&gp->stateinfo.anim, gp_fallvec[gp->gravidx]);
            }

            // increment the gravity table indexer, but don't let it go too far!

            gp->gravidx++;
            if(gp->gravidx > L_GRAVITY_MAX) gp->gravidx = L_GRAVITY_MAX;

            // refresh and sleep for a bit.
            App_Paint();

        return 13 + player->speed / 40;


        case GP_STATE_DESTROY:
            // Generic gamepiece destruction process: wait for animation
            // sequence to complete, then set the gamepiece to empty.

            if(!Anim_Active(&gp->stateinfo.anim))
            {   GamePiece_Initialize(gparray,x,y, GP_EMPTY);
            }
        break;
    }
    
    // By default, eggs process logic continuously.

    return STATE_PRIORITY_HIGH;
}

static ANIM_STATE  gp_res[GP_NUMPIECES];

// =====================================================================================
    BOOL GamePiece_LoadResources(CL_RES *respak, RES_GPSPR *destspr)
// =====================================================================================
// Load up all those wacky game piece animations.  Currently this loads everything itself,
// but I suspect I will delegate some of the task to independant gamepiece extension
// objects (egg, grownegg, stone, etc).
{
    int  i;

    memset(gp_res,0, GP_NUMPIECES * sizeof(ANIM_STATE));

    for(i=0; i<GP_NUMPIECES; i++)
    {   if(gp_info[i].entity && gp_info[i].entity->loadres)
        {   gp_info[i].entity->loadres(respak, &gp_res[i], &destspr[i]);
        }
    }

    return -1;
}


// =====================================================================================
    void GamePiece_UnloadResources(CL_RES *respak)
// =====================================================================================
// Unload all the resources we loaded in LoadResources.  This is a lot simpler than loading
// thank goodness!
{
    int   i;
    
    if(!gp_res) return;
    
    for(i=0; i<GP_NUMPIECES; i++)
        CL_UnloadAnimation(respak, &gp_res[i]);

    memset(gp_res,0, GP_NUMPIECES * sizeof(ANIM_STATE));
}


// =====================================================================================
    void GamePiece_Initialize(GAMEPIECE *array, int x, int y, int type)
// =====================================================================================
// Initializes the gamepiece at the given coordinates to the defaults for the given type.
// You MUST use this, unless you absolutely know what you are doing!  This procedure,
// as you can see, sets up some important information that, if let unset, will crash!
{
    GAMEPIECE *gp = &array[gpidx(x,y)];

    gp->type      = type;

    gp->stateinfo.handler   = GamePiece_StateHandler;
    gp->stateinfo.leave     = NULL;     // for now.  Will add support if/when we need it. :)
    gp->stateinfo.timeout   = 0;
    gp->stateinfo.animstate = &gp_res[type];

    gpState_SetState(gp, (type == GP_EMPTY) ? STATE_NONE : GP_STATE_WAITING);
}


// =====================================================================================
    void GamePiece_Move(GAMEPIECE *array, int x, int y, int dx, int dy)
// =====================================================================================
// Moves the given piece at x,y to the new location at dx,dy.  Any piece in the destination
// will be overwritten.  The old piece is wiped from existence.
{
    array[gpidx(dx,dy)] = array[gpidx(x,y)];

    // Clear out the dude who is now empty!
    array[gpidx(x,y)].type            = GP_EMPTY;
    array[gpidx(x,y)].stateinfo.state = STATE_NONE;
}


