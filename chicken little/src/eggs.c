/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: eggs.c

  Contains both the normal egg and the chocolate egg state extension to GamePiece.

  Currently, the chocolate egg is pretty boring - in fact it has no extension
  of GamePiece, in terms of state, at all!

  Normal eggs, on the other hand, are a bit more complex: They have crack and 
  break states.  After the crack finishes, a birdie (or something else) is spawned,
  then the egg is destroyed.  To do this, we override GP_STATE_DESTROY and have it
  pass us directly into EGG_STATE_CRACK and then onto EGG_STATE_BREAK.

  My only concern is if we happen to want code that checks specifically for eggs
  in the state of 'being destroyed.'  Although, I doub that will be a problem
  since usually we are only concerned if they are in GP_STATE_WAITING or not.
  
*/

#include "player.h"
#include "chicken.h"

enum
{   EGG_STATE_CRACK = GP_NUMSTATES,
    EGG_STATE_BREAK,
    EGG_NUMSTATES
};

enum
{   CHAOSEGG_STATE_BLIP = GP_NUMSTATES,
    CHAOSEGG_NUMSTATES,
};

// =====================================================================================
    static int Egg_StateHandler(PLAYER *player, int x, int y)
// =====================================================================================
{
	GAMEPIECE   *gparray = player->gp->gamearray,
                *gp      = &gparray[gpidx(x,y)];

	switch(gp->stateinfo.state)
    {   
        case GP_STATE_DESTROY:
            // oh woops, we don't do anything here, yet.  However, if we add the 
            // option of multiple random breaking sequences, then this would be
            // the spot to pick a number and switch to the proper state.

            gpState_SetState(gp, EGG_STATE_CRACK);
            Entity_PlaySound(&player->entity, sfx.eggcrack);
        break;


        case EGG_STATE_CRACK:
            // Wait for the animation to finish, then spawn a birdie and go to break state.

            if(!Anim_Active(&gp->stateinfo.anim))
            {   gpState_SetState(gp, EGG_STATE_BREAK);
                Entity_PlaySound(&player->entity, sfx.eggbreak);
            }
        break;


        case EGG_STATE_BREAK:
            // when this animation ends, we delete the egg!

            if(!Anim_Active(&gp->stateinfo.anim))
            {   //Entity_PlaySound(&player->entity,sfx.birdie1);
                GamePiece_Initialize(gparray,x,y, GP_EMPTY);
            }
        break;

        default:
            // The fallback code.  By returning 0 we tell GamePiece to use its own
            // state code!

        return 0;
    }

    return STATE_PRIORITY_HIGH;
}


// =====================================================================================
    static int ChaosEgg_StateHandler(PLAYER *player, int x, int y)
// =====================================================================================
// The chocolate egg state handler.  Doesn't do anythign for now.
{
	GAMEPIECE   *gparray = player->gp->gamearray,
                *gp      = &gparray[gpidx(x,y)];

	switch(gp->stateinfo.state)
    {
        case GP_STATE_DESTROY:
            gpState_SetState(gp, CHAOSEGG_STATE_BLIP);
            Entity_PlaySound(&player->entity, sfx.blip);
        return STATE_PRIORITY_LOW;

        case CHAOSEGG_STATE_BLIP:
            // when this animation ends, we delete the egg!

            if(!Anim_Active(&gp->stateinfo.anim))
            {   //Entity_PlaySound(&player->entity,sfx.birdie1);
                GamePiece_Initialize(gparray,x,y, GP_EMPTY);
            }
        break;
    }
    return 0;
}

typedef struct EGGRES
{
    uint icon, outline, wait, crack, crumble;
} EGGRES;

typedef struct CEGGRES
{
    uint icon, outline, wait, destroy;
} CEGGRES;


static EGGRES EggResources[RES_EGG_NUM] = 
{   
    { RES_ICON_WHITE, RES_OUTLINE_WHITE, RES_EGG_WHITE, RES_EGGCRACK_WHITE, RES_EGGCRUMBLE_WHITE },
    { RES_ICON_BLUE,  RES_OUTLINE_BLUE,  RES_EGG_BLUE,  RES_EGGCRACK_BLUE,  RES_EGGCRUMBLE_BLUE  },
    { RES_ICON_PINK,  RES_OUTLINE_PINK,  RES_EGG_PINK,  RES_EGGCRACK_PINK,  RES_EGGCRUMBLE_PINK  },
    { RES_ICON_GREEN, RES_OUTLINE_GREEN, RES_EGG_GREEN, RES_EGGCRACK_GREEN, RES_EGGCRUMBLE_GREEN },
    { RES_ICON_GOLD,  RES_OUTLINE_GOLD,  RES_EGG_GOLD,  RES_EGGCRACK_GOLD,  RES_EGGCRUMBLE_GOLD  },
};

static CEGGRES ChaosEggResources[RES_CHAOS_NUM] = 
{   
    { RES_ICON_CHAOSRED,   RES_OUTLINE_CHAOS, RES_CHAOS_RED,   RES_CHAOSBLIP_RED   },
    { RES_ICON_CHAOSBLUE,  RES_OUTLINE_CHAOS, RES_CHAOS_BLUE,  RES_CHAOSBLIP_BLUE  },
    { RES_ICON_CHAOSGREEN, RES_OUTLINE_CHAOS, RES_CHAOS_GREEN, RES_CHAOSBLIP_GREEN },
};


// =====================================================================================
    static BOOL Egg_LoadResources(CL_RES *respak, ANIM_STATE *dest, RES_GPSPR *destspr)
// =====================================================================================
// Loads the animation resources for the normal eggs into the given destination buffer.
// They contain three states: normal (1 frame), crack (3 frames), and break (4 frames).
// We load the eggs in the order they are stored in the resource file.
//
// Notes:
//  - The animation state buffer must be allocated properly!
//  - We allocate the needed number og sequences ourselves so the caller does *not*
//    do that.
{
    uint   t;
    static uint  i=0;    
        
    if(i >= RES_EGG_NUM) i = 0;
    
    // first we load some sprites (for falling pieces and the iconized 'next' pieces)

    if(!destspr->full.bitmap)
        VDRes_LoadSprite(respak->videopak, &destspr->full, EggResources[i].wait);

    if(!destspr->icon.bitmap)
        VDRes_LoadSprite(respak->videopak, &destspr->icon, EggResources[i].icon);

    if((EggResources[i].outline) && !destspr->outline.bitmap)
        VDRes_LoadSprite(respak->videopak, &destspr->outline, EggResources[i].outline);

    if(dest->sequence) return -1;
    
    dest->sequence = (ANIM_SEQUENCE *)_mm_calloc(EGG_NUMSTATES,sizeof(ANIM_SEQUENCE));
    dest->count    = EGG_NUMSTATES;

    CL_LoadAnimation(respak, &dest->sequence[GP_STATE_WAITING], EggResources[i].wait, 1);

    // -------------------------------
    //  Second state, crack animation
    // -------------------------------
    
    CL_LoadAnimation(respak, &dest->sequence[EGG_STATE_CRACK], EggResources[i].crack, 3);
    
    for(t=0; t<2; t++)
        dest->sequence[EGG_STATE_CRACK].frame[t].delay  = 160;
            
    dest->sequence[EGG_STATE_CRACK].frame[2].delay  = 360;

    // ------------------------------
    //  Third state, break animation
    // ------------------------------

    CL_LoadAnimation(respak, &dest->sequence[EGG_STATE_BREAK], EggResources[i].crumble, 4);

    for(t=0; t<4; t++)
        dest->sequence[EGG_STATE_BREAK].frame[t].delay  = 72;

    i++;
    
    return 5;
}


// =====================================================================================
    static BOOL ChaosEgg_LoadResources(CL_RES *respak, ANIM_STATE *dest, RES_GPSPR *destspr)
// =====================================================================================
// Loads the animation resources for the Chaosolate eggs into the given destination buffer.
// They contain only two states: normal (1 frame), and destroy (11 frames).  We load the
// eggs in the order they are stored in the resource file.
//
// Notes:
//  - The animation state buffer must be allocated properly!
//  - We allocate the needed number of sequences ourselves so the caller does *not*
//    do that.
{
    uint         t;
    static uint  i=0;    
        
    if(i >= RES_CHAOS_NUM) i = 0;

    // first we load some sprites (for falling pieces and the iconized 'next' pieces)

    if(!destspr->full.bitmap)
        VDRes_LoadSprite(respak->videopak, &destspr->full, ChaosEggResources[i].wait);

    if(!destspr->icon.bitmap)
        VDRes_LoadSprite(respak->videopak, &destspr->icon, ChaosEggResources[i].icon);

    if((ChaosEggResources[i].outline) && !destspr->outline.bitmap)
        VDRes_LoadSprite(respak->videopak, &destspr->outline, ChaosEggResources[i].outline);

    if(dest->sequence) return -1;

    dest->sequence = (ANIM_SEQUENCE *)_mm_calloc(CHAOSEGG_NUMSTATES,sizeof(ANIM_SEQUENCE));
    dest->count    = CHAOSEGG_NUMSTATES;

    CL_LoadAnimation(respak, &dest->sequence[GP_STATE_WAITING], ChaosEggResources[i].wait, 1);

    // ----------------------------------------
    //  Second state, destroy (blip) animation
    // ----------------------------------------

    CL_LoadAnimation(respak, &dest->sequence[CHAOSEGG_STATE_BLIP], ChaosEggResources[i].destroy, 10);
    
    for(t=0; t<10; t++)
        dest->sequence[CHAOSEGG_STATE_BLIP].frame[t].delay  = 100;

    i++;
    
    return 5;
}

static BOOL Egg_IsInert(uint state)
{
    return (state == GP_STATE_WAITING);
}

GAMEPIECE_ENTITY Entity_Egg =
{
    Egg_StateHandler,
    Egg_LoadResources,
    Egg_IsInert
};

GAMEPIECE_ENTITY Entity_ChaosEgg =
{
    ChaosEgg_StateHandler,
    ChaosEgg_LoadResources,
    Egg_IsInert
};
