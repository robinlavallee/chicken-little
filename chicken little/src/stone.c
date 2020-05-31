/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: stone.c

  The semi-inert stone piece:

  Crubles when 5 or 6 piece have been placed on top of it.  Prior to crumbling, it cracks
  under the weight of said pieces.

*/

#include "player.h"

enum
{   STONE_STATE_CRACK1 = GP_NUMSTATES,
    STONE_STATE_CRACK2,
    STONE_STATE_CRUMBLE,
    STONE_NUMSTATES
};


// =====================================================================================
    static int Stone_StateHandler(PLAYER *player, int x, int y)
// =====================================================================================
{
	GAMEPIECE   *gparray = player->gp->gamearray,
                *gp      = &gparray[gpidx(x,y)];

    switch(gp->stateinfo.state)
    {   
        case GP_STATE_WAITING:
            // We only crack after all gravity is done and over with...

            if(GamePiece_CheckGravity(gp,gparray,x,y)) return STATE_PRIORITY_NORMAL;
            //if(!GP_NoGravity(gparray)) return STATE_PRIORITY_LOW;

            // Take a look to see how many pieces are stacked on top of us.  If there are
            // three or more, pass us off to the Crack1 state.

            if (GP_FindNumAbove(gparray, x, y) >= 2)
                gpState_SetState(gp, STONE_STATE_CRACK1);

        break;

        /*case GP_STATE_DESTROY:
            // This has been removed because the stone has a 'destory' animation now,
            // which includes the cracks and then goes through the destruction process.

            if(!Anim_Active(&gp->stateinfo.anim))
            {   GamePiece_Initialize(gparray,x,y, GP_EMPTY);
            }
            gpState_SetState(gp, STONE_STATE_CRUMBLE);
        break;*/


        case STONE_STATE_CRACK1:
            // We only crack after all gravity is done and over with...
            
            if(GamePiece_CheckGravity(gp,gparray,x,y)) return STATE_PRIORITY_NORMAL;
            //if(!GP_NoGravity(gparray)) return STATE_PRIORITY_LOW;

            // Look-see how many on top of us.  If there are four or more, go to crack2.

            if (GP_FindNumAbove(gparray, x, y) >= 3)
            {
                gpState_SetState(gp, STONE_STATE_CRACK2);
            }

        break;


        case STONE_STATE_CRACK2:
            // We only crack after all gravity is done and over with...

            if(GamePiece_CheckGravity(gp,gparray,x,y)) return STATE_PRIORITY_NORMAL;
            //if(!GP_NoGravity(gparray)) return STATE_PRIORITY_LOW;

            // Look-see how many on top of us.  If there are five or more, go to crumble!
            
            if (GP_FindNumAbove(gparray, x, y) >= 4)
                gpState_SetState(gp, STONE_STATE_CRUMBLE);

        break;


        case STONE_STATE_CRUMBLE:
            // We delete our stone before the animation completes so that it looks like
            // the stone is being 'crushed.'  To make sure the stone continues to be rendered,
            // we spawn a new animation entity which sole purpose is to show the proper image.

            Entity_PlaySound(&player->entity, sfx.crumble);
            Player_GhostifyPiece(player,x,y);
            GamePiece_Initialize(gparray,x,y, GP_EMPTY);

            // All Done!
        break;

        default:
            // The fallback code.  By returning 0 we tell GamePiece to use its own
            // state code!

        return 0;
    }

    return STATE_PRIORITY_HIGH;
}


// =====================================================================================
    static BOOL Stone_LoadResources(CL_RES *respak, ANIM_STATE *dest, RES_GPSPR *destspr)
// =====================================================================================
{
    uint   t;

    // first we load some sprites (for falling pieces and the iconized 'next' pieces)
    if(!destspr->full.bitmap)
        VDRes_LoadSprite(respak->videopak, &destspr->full, RES_STONE_WAITING);

    if(!destspr->icon.bitmap)
        VDRes_LoadSprite(respak->videopak, &destspr->icon, RES_ICON_STONE);

    if(!destspr->outline.bitmap)
        VDRes_LoadSprite(respak->videopak, &destspr->outline, RES_OUTLINE_STONE);

    if(dest->sequence) return -1;

    dest->sequence = (ANIM_SEQUENCE *)_mm_calloc(STONE_NUMSTATES,sizeof(ANIM_SEQUENCE));
    dest->count    = STONE_NUMSTATES;

    CL_LoadAnimation(respak, &dest->sequence[GP_STATE_WAITING], RES_STONE_WAITING, 1);

    // -------------------------------------------
    //  Second and third states: crack animations
    // -------------------------------------------
    // (although they are not animations, but may be someday!)
    
    CL_LoadAnimation(respak, &dest->sequence[STONE_STATE_CRACK1], RES_STONE_CRACK1, 1);
    CL_LoadAnimation(respak, &dest->sequence[STONE_STATE_CRACK2], RES_STONE_CRACK2, 1);

    dest->sequence[STONE_STATE_CRACK1].frame[0].delay  = 300;
    dest->sequence[STONE_STATE_CRACK2].frame[0].delay  = 300;
            
    // ------------------------------
    //  Fourth state, Crumbling
    // ------------------------------

    CL_LoadAnimation(respak, &dest->sequence[GP_STATE_DESTROY], RES_STONE_CRACK1, 10);

    dest->sequence[GP_STATE_DESTROY].frame[0].delay  = 200;
    dest->sequence[GP_STATE_DESTROY].frame[1].delay  = 200;

    for(t=2; t<10; t++)
        dest->sequence[GP_STATE_DESTROY].frame[t].delay  = 72;

    CL_LoadAnimation(respak, &dest->sequence[STONE_STATE_CRUMBLE], RES_STONE_CRUMBLE, 8);

    for(t=0; t<8; t++)
        dest->sequence[STONE_STATE_CRUMBLE].frame[t].delay  = 50;

    return 1;
}

// =====================================================================================
    static BOOL Stone_IsInert(uint state)
// =====================================================================================
{
    return (state == GP_STATE_WAITING || state == STONE_STATE_CRACK1 || state == STONE_STATE_CRACK2);
}



// =====================================================================================
    static BOOL Meteor_LoadResources(CL_RES *respak, ANIM_STATE *dest, RES_GPSPR *destspr)
// =====================================================================================
{
    uint   t;

    if(dest->sequence) return -1;

    dest->sequence = (ANIM_SEQUENCE *)_mm_calloc(GP_NUMSTATES,sizeof(ANIM_SEQUENCE));
    dest->count    = GP_NUMSTATES;

    CL_LoadAnimation(respak, &dest->sequence[GP_STATE_WAITING], RES_METEOR_WAITING, 1);

    CL_LoadAnimation(respak, &dest->sequence[GP_STATE_DESTROY], RES_METEOR_CRUMBLE, 8);

    for(t=2; t<8; t++)
        dest->sequence[GP_STATE_DESTROY].frame[t].delay  = 72;

    return 1;
}

// =====================================================================================
    static BOOL Meteor_IsInert(uint state)
// =====================================================================================
{
    return (state == GP_STATE_WAITING);
}

GAMEPIECE_ENTITY Entity_Meteor =
{
    NULL,
    Meteor_LoadResources,
    Meteor_IsInert
};

GAMEPIECE_ENTITY Entity_Stone =
{
    Stone_StateHandler,
    Stone_LoadResources,
    Stone_IsInert
};
