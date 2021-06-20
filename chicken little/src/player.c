/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: player.c

  The mostly-encapsulated player object!
*/

#include "chicken.h"
//#include "player.h"
#include "app.h"
#include "random.h"

#include <string.h>

extern GAMEDATA chicken;

#define SPEED_FALLFAST 46


SPRITE  portres[PORTRAIT_NUMCHARS];

enum
{
    PORTRAIT_STATE_INTRO = ENTITY_NUMSTATES
};

// =====================================================================================
    static int Portrait_StateHandler(PORTRAIT *portrait)
// =====================================================================================
{
	switch(portrait->entity.state)
    {
        case ENTITY_STATE_WAITING:
        break;

        case PORTRAIT_STATE_INTRO:
            portrait->scrollpos -= 5;
            
            if(portrait->scrollpos <= 0)
                Entity_SetState(&portrait->entity, ENTITY_STATE_WAITING);

            App_Paint();
        return 7;
    }

    return 1000;
}

#define PORTRAIT_TOP      30
#define PORTRAIT_HEIGHT  192
#define PORTRAIT_WIDTH   128

// =====================================================================================
    void Portrait_Render(PORTRAIT *portrait)
// =====================================================================================
{
    VRECT   rect, adjusted;
    int     scrollpos = (portrait->index == 0) ? -portrait->scrollpos : portrait->scrollpos;

    rect.top    = PORTRAIT_TOP + ((PORTRAIT_HEIGHT+3) * portrait->index);
    rect.left   = 256;
    rect.right  = rect.left + PORTRAIT_WIDTH;
    rect.bottom = rect.top + PORTRAIT_HEIGHT;

    adjusted.left    = rect.left + portrait->xrel;
    adjusted.right   = rect.right + portrait->xrel;
    adjusted.top     = rect.top + scrollpos;
    adjusted.bottom  = rect.bottom + scrollpos;

    if(adjusted.bottom <= 0) return;
    if(adjusted.top > 479) return;  // avoid invisible surfaces
    

    chicken.vs->shadowrect(chicken.vs, &adjusted, VD_ALPHA_50);

    sprblit(portres[portrait->pic],adjusted.left,adjusted.top);

}


ENTITY Entity_Portrait = 
{
    "Portrait",
    PORTRAIT_STATE_INTRO,
    0,
    NULL,
    Portrait_StateHandler,
    NULL,
    //Portrait_Render
};

// =====================================================================================
    static PORTRAIT *Portrait_Initialize(ENTITY **entlist, uint pic, uint index, int xrel)
// =====================================================================================
{
    PORTRAIT    *portrait;

    portrait = (PORTRAIT *)Entity_Spawn(entlist, &Entity_Portrait, sizeof(PORTRAIT));

    portrait->pic   = pic;
    portrait->index = index;

    portrait->scrollpos = 480;
    portrait->xrel = xrel;

    return portrait;
}


// =====================================================================================
    void Portrait_LoadResources(CL_RES *respak)
// =====================================================================================
{
    int  i;


    for(i=0; i<PORTRAIT_NUMCHARS; i++)
    {
        VDRes_LoadSprite(respak->videopak, &portres[i], RES_PORTRAIT_CHICKEN + i);
    }
}   


// =====================================================================================
    static void __inline Player_DisplayText(PLAYER *player, const CHAR *text, int x, int y, int timeout)
// =====================================================================================
{
    TEXTINFO *newtext;

    newtext = TextInfo_Initialize(&player->gameplay_ents, text, x, y, font.little);

    newtext->entity.timeout = timeout;    // Your life is short. Use it wisely.
}


// =====================================================================================
    static void __inline Player_DisplayBigText(PLAYER *player, const CHAR *text, int x, int y, int timeout)
// =====================================================================================
{
    TEXTINFO *newtext;

    newtext = TextInfo_Initialize(&player->gameplay_ents, text, x, y, font.big);

    newtext->entity.timeout = timeout;    // Your life is short. Use it wisely.
}


// =====================================================================================
    static void __inline Player_DisplayScore(PLAYER *player, int score)
// =====================================================================================
{
    CHAR      stmp[10];
    TEXTINFO *newtext;
    int       xtotal,ytotal;
    uint      i;

    itoa(score, stmp, 10);

    xtotal = ytotal = 0;
    i = GP_GetScoreLocation(player->gp->gamearray, &xtotal, &ytotal);

    // Translate the score location stuff into on-screen coordinates.
    // (remember our coordinates are relative to the player gameplay field)

    newtext = TextInfo_Initialize(&player->gameplay_ents, stmp, (xtotal * 30) / i, (ytotal * 30) / i, font.little);

    // A quick fix to make the text more centered on the targetted match,
    // and a second one to make sure our text doesn't wrap around the right
    // side of the screen.

    newtext->x -= strlen(stmp) * 5;
    if(newtext->x > (SIZEX-2) * 30) xtotal = (SIZEX-2) * 30;

    // Arbitrary values I picked which seem to provide a nice effect:
    
    newtext->floating       = 32;
    newtext->entity.timeout = 64;
    newtext->floattime      = 4;

    newtext->fadeout        = SPR_ALPHA_RANGE;
    newtext->entity.state   = TEXT_STATE_FLOATING;

}

extern int gp_perfect;

// =====================================================================================
    static int Player_StateHandler(PLAYER *player)
// =====================================================================================
{

    int nClear;

	switch(player->entity.state)
    {   
        case ENTITY_STATE_WAITING:
        return 1000;

        case PLAYER_STATE_WIN:
            // Nothing spectacular here for the moment

            if(!player->gameover)
            {
                player->winner = 1;
                player->gameover = 1;
                Player_DisplayBigText(player, "Won", 58, 160, 4000);
                App_Paint();
            } else
                Player_SetState(player, ENTITY_STATE_WAITING);
        return 3600;


        case PLAYER_STATE_LOSE:
            if(!player->gameover)
            {
                player->winner = 0;
                player->gameover = 1;
                Player_DisplayBigText(player, "Lost",35, 160, 4000);
                App_Paint();
            } else
                Player_SetState(player, ENTITY_STATE_WAITING);
        return 3600;


        case PLAYER_STATE_INTRO:
            if(player->portrait->entity.state == ENTITY_STATE_WAITING)
                Player_SetState(player, PLAYER_STATE_GETREADY);
        return STATE_PRIORITY_LOW;

        
        case PLAYER_STATE_GETREADY:
            
            // Here is where the announcer says Get-Ready and the protraits are
            // told to do their 'intro thingie.'

            if (chicken.player[0] == player) // just do it for player 1
                Entity_PlaySound(&chicken.player[0]->entity, sfx.getready);

            // Display accompanying text (for both players)
            // [...]

            //Player_DisplayBigText(player, "Get", 70, 85, 1000);
            //Player_DisplayBigText(player, "Ready", 30, 140, 1000); 

            Player_SetState(player, PLAYER_STATE_GO);


        case PLAYER_STATE_GO:
            // This is our intro sequence.  Our player's gamefield is scrolled in from the
            // current position to the home'd position.

            player->scrollpos -= 4;

            if(player->scrollpos <= 0)
            {
                Player_SetState(player, PLAYER_STATE_START);
                GP_GetInitialPieces(player->gp);
                App_Paint();
                return 850;
            }
            App_Paint();
        return 6;

        case PLAYER_STATE_START:
            // We want the announcer to say "Go" here.
            // But we don't want it to be repeated for both players. What do we do?
            // Just do it for player1

            if (chicken.player[0] == player) // that is player1
                Entity_PlaySound(&chicken.player[0]->entity, sfx.go);

            Player_SetState(player, PLAYER_STATE_GETNEXTPIECE);
            App_Paint();
        return 250;

        case PLAYER_STATE_GETNEXTPIECE:
            // Get the next set of pieces and set their state so that they will
            // go through the proper 'piece introduction' animation sequence.

            Player_SetState(player, PLAYER_STATE_WAITING);
        return player->speed;


        case PLAYER_STATE_WAITING:
    	    // This is the state we are in after eggs have fallen and matches
            // have been done. It's also in this state that we can rain eggs
            // on the playfield.

            // Wait for any animations or egg activities to complete.

            //if(!GP_IsInert(player->gamearray)) return STATE_PRIORITY_NORMAL;

            // Now, all of that is quite fine. Except that we may have a problem.
            // What if in the middle of our "combo doing" the other player's state
            // get to state wiating? Then half of our combo is going to get sent
            // To him only... and the other part will act as if it was another combo.
            // That's why we need to set some kind of "handshaking" flag between
            // The two. Hence, once we are done with the combo & stuff.

            {
            PLAYER *opp;

            opp = GetOpponent(player); 

            if (opp != NULL) // not a single player game
                opp->attack.ready = TRUE; // ready to blow your ass!

            }

            // Check for and execute queued 'attacks' from the opponent...

            if (DumpCount(&player->attack.dump) != 0 && player->attack.ready) // we have pieces waiting to fall
            {

                Attack(player->gp, &player->attack.dump); // start the attack

                // When this function return, attack.dump will have been updated. 

                if (DumpCount(&player->attack.dump) != 0) // more to come!
                {
                    if (GP_IsInert(player->gp->gamearray))
                    {
                        // Okay man. You may have have wanted to send 60 eggs, but only 10 could get there
                        // Just slow down and set it back to 0, you probably have killed your opponent anyway.
                        
                        // Air: by commenting out these lines and adding another in FINDMATCHES I seemed
                        //      to have fixed the 'premature dumping' bug.  Hope I didn't break something
                        //      else in the process.

                        //DumpReset(&player->attack.dump);
                        //player->attack.special = 0;
                        player->attack.ready = FALSE;
                        Player_SetState(player, PLAYER_STATE_FINDMATCHES);
                    }
                    
                    return STATE_PRIORITY_HIGH;
                }

                // Else, get yourself into FindMatches as those eggs may have helped you.
                Player_SetState(player, PLAYER_STATE_FINDMATCHES); // those new eggs may create matches.

                // And reset the attack
                player->attack.special = 0;
                player->attack.ready   = FALSE; // return back the token

                return STATE_PRIORITY_HIGH;  // nothing to do here really anymore
            }
            
            // For now the next piece getting is here
            // Later on it should be encapsulated into a state.

            GP_GetNextPiece(player->gp);

            if (player->control == CONTROL_CPU)
                player->ai.donethinking = FALSE;


            // Perhaps this getpiece killed you!

            if (GP_IsDead(player->gp))
            {
                if (player->control == CONTROL_HUMAN) // you have lost
                    Entity_PlaySound(&player->entity, sfx.lose);
                else
                    Entity_PlaySound(&player->entity, sfx.win);

                Player_SetState(player, PLAYER_STATE_LOSE);
                
                PLAYER *oponent = GetOpponent(player);
                if (oponent) {
                  Player_SetState(oponent, PLAYER_STATE_WIN);
                }
                

                return STATE_PRIORITY_HIGH;
            }

            // And don't forget to change the state
            Player_SetState(player, PLAYER_STATE_EGGFALLING);
            App_Paint();
        return player->speed;


        case PLAYER_STATE_EGGFALLING:

            // If we hit rock bottom, we attempt to place the pieces.
            // Otherwise, we know we have room to increment ypart.

            if (GP_CheckDown(player->gp) == FALSE)
            {
                player->gp->ypart = 0;    // make sure our egg isn't halfway into the one below it

                // change the state to SOLIDIFY then set a delay, during which time the
                // player can move or rotate the piece as he sees fit.

                // Little sound effect for this
                Entity_PlaySound(&player->entity, sfx.eggplace);

                Player_SetState(player, PLAYER_STATE_SOLIDIFY);
                return player->speed;
            } else
            {   
                GP_MoveDown(player->gp);
                App_Paint();
            }
        return player->speed / 2;


        case PLAYER_STATE_EGGFALLING_FAST:

            // If we hit rock bottom, we attempt to place the pieces.
            // Otherwise, we know we have room to increment ypart.

            if (GP_CheckDown(player->gp) == FALSE)
            {
                player->gp->ypart = 0;    // make sure our egg isn't halfway into the one below it

                // change the state to SOLIDIFY then set a delay, during which time the
                // player can move or rotate the piece as he sees fit.

                Entity_PlaySound(&player->entity, sfx.eggplace);
                Player_SetState(player, PLAYER_STATE_SOLIDIFY);
            } else
            {   
                GP_MoveDown(player->gp);
                App_Paint();
                return SPEED_FALLFAST / 2;    // we fall fast!
            }

            // Neat trick: move directly to solidify, no delay.


        case PLAYER_STATE_SOLIDIFY:

            if (GP_CheckDown(player->gp) == FALSE)
            {
                // The pieces are smart, they will do gravity themselves!
                GP_Solidify(player->gp);

                // Hide the current egg!
                player->gp->current[0] = GP_EMPTY;
                player->gp->current[1] = GP_EMPTY;

                Player_SetState(player, PLAYER_STATE_FINDMATCHES);
            } else
                Player_SetState(player, PLAYER_STATE_EGGFALLING);
        return STATE_PRIORITY_HIGH;          // waste little time, my friend.


		case PLAYER_STATE_FINDMATCHES:
            // first, we wait until the gameboard is 'inert'
            // For this, we go through all the egg state and make sure they are all
            // either 1 : waiting
            // or 2     : empty

            // No multithreadedness today: if our eggs aren't waiting, then we are!

            if(!GP_IsInert(player->gp->gamearray)) return STATE_PRIORITY_NORMAL;

            // Search for any matches and force those entities to a state of 'destruction.'

            if (nClear = GP_FindMatch(player->gp->gamearray, TRUE))
            {
                uint    score = 0;
                PLAYER *opp;

                opp = GetOpponent(player);

                if (opp != NULL)
                {
                    opp->attack.ready = FALSE;

                    // Increase our combo meter
                    player->combo++;

                    // Add them to our running counter
                    player->clear += nClear;

                    // Not forgetting, combos eggs are cumulative. Hence we must store our stuff
                    // Somewhere.

                    if (player->combo == 1) // This was our first. Let's get our stuff right :
                    {
                        if (nClear > 4)
                        {
                            opp->attack.dump.nPiece[DUMP_STONE]  += nClear / 4;
                            opp->attack.dump.nPiece[DUMP_METEOR] += (nClear-5) / 2;
                            opp->attack.dump.nPiece[DUMP_EGG]    += (nClear-3) / 2; 
                        }

                    // Add to the score!

                        score  = AddScore(player->score, player->combo-1, nClear, gp_perfect);
                    } else 
                    {

                        // We need to accumulate to our running counter
                    
                        player->combo--;
                        opp->attack.dump.nPiece[DUMP_METEOR] += (6*player->combo + player->clear) / 3;
                        opp->attack.dump.nPiece[DUMP_EGG]    += (6*player->combo + player->clear) / 5;
                        opp->attack.dump.nPiece[DUMP_STONE]  += (6*player->combo + player->clear) / 4;

                        score  = AddScore(player->score, player->combo, player->clear, gp_perfect);
                        player->combo++;
                    }
                }
                else
                {
                    player->combo++;
                    player->clear += nClear;

                    if (player->combo == 1) // first move
                        score  = AddScore(player->score, player->combo-1, nClear, gp_perfect);
                    else
                        score = AddScore(player->score, player->combo-1, player->clear, gp_perfect);
                }

                // If we have a score to display, display it:
                if(score)
                    Player_DisplayScore(player, score);

                return STATE_PRIORITY_LOW;
            }

            if(player->combo > 1) Entity_PlaySound(&player->entity, rand_getbit() ? sfx.birdie1 : sfx.birdie2);

            // In any case, we reset the combo meter and eggs counter
            player->combo = 0; // no combo, set this back to 0.
            player->clear = 0;

            // Switch back to state
            Player_SetState(player, PLAYER_STATE_GETNEXTPIECE);
        return STATE_PRIORITY_HIGH;

    }

    // By default we wait the current speed.

    return player->speed;
}

// =====================================================================================
    void Player_GhostifyPiece(PLAYER *player, int x, int y)
// =====================================================================================
// Creates a 'dummy' entity which simply sits there and executes the current animation 
// for the given piece.  It also deletes the given piece.
{
    GAMEPIECE *gp = &player->gp->gamearray[gpidx(x,y)];
    ENTITY    *ent;

    ent = Entity_Spawn(&player->gameplay_ents, &entity_ghost, 0);
    Entity_SetAnimation(ent, &gp->stateinfo.anim);
    Anim_SetTranslation(&ent->anim,(x*30)<<8, (y*30)<<8);
}


// =====================================================================================
    void Player_MoveTurn(PLAYER *player, int direction)
// =====================================================================================
// This function simply calls GP_MoveTurn and then plays the appropriate rotation sound.
// You had better check if you can rotate (using GP_CheckTurn) before you call me.  Or else!
{
    GP_MoveTurn(player->gp, direction);
    Entity_PlaySound(&player->entity, sfx.eggrotate);

}


// =====================================================================================
    void Player_Initialize(PLAYER *player, uint index, uint character, int speed, ulong time)
// =====================================================================================
// Initializes the player and loads resources, but leaves him idle until his 
// state is changed to 'start' manually by the game.
{

    player->attack.ready = FALSE;
    player->speed        = speed;

    // Reset the score system

    ResetStat(&player->stat);

    player->gp       = GamePlay_Initialize(&player->gameplay_ents, time);
    player->score    = Score_Initialize(&player->gameplay_ents);

    player->index     = index;

    if (chicken.options.single == FALSE)
        player->portrait = Portrait_Initialize(&chicken.entitylist, character, player->index, 0);
    else
        player->portrait = Portrait_Initialize(&chicken.entitylist, character, player->index, 200);

    // Resource loading goes here
    // [...]

    player->scrollpos = 580;    // we're completely off-screen;

    player->gameover = 0;
    player->winner   = 0;

}


// =====================================================================================
    static void Player_Deinitialize(PLAYER *player)
// =====================================================================================
{
    // I will automate this stuff later, so that there will be an Entity_CleanUp
    // which will kill all the entities in a gievn list!

    Entity_KillList(&player->gameplay_ents);

    Entity_Kill(&player->portrait->entity);
}


// =====================================================================================
    int Player_UpdateEntities(PLAYER *player, uint timepass)
// =====================================================================================
{
    int   jeb=0, zeb;
    
    if(!player) return 0;
    
    zeb = gpState_Update(player, timepass);
    if(jeb > zeb) jeb = zeb;

    zeb = Entity_Update(&player->gameplay_ents, timepass);
    if(jeb > zeb) jeb = zeb;

    //zeb = Entity_Update(&player->textlist, timepass);
    //if(jeb > zeb) jeb = zeb;

    return jeb;
}


// =====================================================================================
    static void __inline DisplayNextEggs(const GAME_INFO *p, const RES_GPSPR *gp, int xloc, int yloc)
// =====================================================================================
// Renders iconized eggs in the next box.
{
    // need to redo this so player->board info is unneeded.

    sprblit(gp[p->next[0]].icon, 177+xloc, 395+yloc);
    sprblit(gp[p->next[1]].icon, 193+xloc, 395+yloc);
    
    sprblit_alpha(gp[p->next[2]].icon, 177+xloc, 410+yloc, 134);
    sprblit_alpha(gp[p->next[3]].icon, 193+xloc, 410+yloc, 134);

}

static int RotateX[8] = 
{   
    0, -2, -5, -8, -13, -18, -24, -29
};

static int RotateY[8] = 
{
    0, 5, 11, 16, 21, 24, 27, 29
};

// =====================================================================================
    static void DisplayEggs(const GAME_INFO *player, const RES_GPSPR *gp)
// =====================================================================================
// Re-renders all the eggs in the gameplay area.
{
	GAMEPIECE *array;
    int        ypart;

    int    x,y,i;
    
    array = (GAMEPIECE *)player->gamearray;

    // Draw all the eggs in the playfield:

    for(y=0, i=0; y<SIZEY; y++)
        for(x=0; x<SIZEX; x++, i++)
            if(array[i].type) Anim_BlitterEx(&array[i].stateinfo.anim, x*30, y*30 );

    // We also need to display the falling eggs.

    ypart = player->ypart ? 15: 0;

    // Display the shadow and then the egg itself, and our 'master' egg gets
    // the added privlidge of an outline!  ooooh... ahhhh.
    
    sprblit_fastshadow(gp[player->current[0]].full, player->x*30 + 3, player->y*30 + ypart + 3);
    sprblit(gp[player->current[0]].full, player->x*30, player->y*30 + ypart);
    sprblit_alpha(gp[player->current[0]].outline, player->x*30, player->y*30 + ypart, 192);

    // Check the position of the other egg, and find its displacement

    switch(player->disppos)
    {   case POS_LEFT:
            sprblit_fastshadow(gp[player->current[1]].full, ((player->x+1)*30)+RotateX[player->index] + 3, (player->y*30) + RotateY[player->index] + ypart + 3);
            sprblit(gp[player->current[1]].full, ((player->x+1)*30)+RotateX[player->index], (player->y*30) + RotateY[player->index] + ypart);
        break;

        case POS_ABOVE:
            sprblit_fastshadow(gp[player->current[1]].full, (player->x*30) - RotateY[player->index] + 3, ((player->y+1)*30) + RotateX[player->index] + ypart + 3);
            sprblit(gp[player->current[1]].full, (player->x*30) - RotateY[player->index], ((player->y+1)*30) + RotateX[player->index] + ypart);
        break;

        case POS_RIGHT:
            sprblit_fastshadow(gp[player->current[1]].full, ((player->x-1)*30)-RotateX[player->index] + 3, (player->y*30) - RotateY[player->index] + ypart + 3);
            sprblit(gp[player->current[1]].full, ((player->x-1)*30)-RotateX[player->index], (player->y*30) - RotateY[player->index] + ypart);
        break;

        case POS_BELOW:
            if(player->y)
            {   sprblit_fastshadow(gp[player->current[1]].full, (player->x*30) + RotateY[player->index] + 3, ((player->y-1)*30) - RotateX[player->index] + ypart + 3);
                sprblit(gp[player->current[1]].full, (player->x*30) + RotateY[player->index], ((player->y-1)*30) - RotateX[player->index] + ypart);
            }
        break;
    }
}



// =====================================================================================
    static void __inline DisplayScore(PLAYER *player, VRECT *locale)
// =====================================================================================
{
    // This function displays your score in the green box!

    char buf[40];
    char buf2[40];
    
    GetScore(player->score, buf, sizeof(buf)); // get the score

    sprintf(buf2, "%9s", buf);
    
    vd_dispstring(font.score, locale->left+16, locale->bottom+5, buf2);

}

extern RES_GPSPR   gpspr[GP_NUMPIECES];

// =====================================================================================
    static void Player_Render(PLAYER *player)
// =====================================================================================

{
    VRECT      adjusted;
    int        scrollpos = (player->index == 0) ? -player->scrollpos : player->scrollpos;

    if(player->scrollpos)
    {   adjusted.left    = player->board.left;
        adjusted.right   = player->board.right;
        adjusted.top     = player->board.top + scrollpos;
        adjusted.bottom  = player->board.bottom + scrollpos;

        if(adjusted.bottom <= 0) return;
        if(adjusted.top > 479) return;  // avoid invisible surfaces
    } else adjusted = player->board;

    // darken the gameplay area
    
    chicken.vs->shadowrect(chicken.vs, &adjusted, VD_ALPHA_25);


    // Display the pieces.
    chicken.vs->setclipper(chicken.vs, &adjusted);
	if(!chicken.paused) DisplayEggs(player->gp, gpspr);
    chicken.vs->setclipper(chicken.vs, NULL);

    sprblit(chicken.layout, adjusted.left-15,scrollpos);

    // -----------------------------------
    // Display Layout first (before we set the clipper), includes (in this order):
    //  (a) border overlay
    //  (b) score
    //  (c) 'next' pieces

    DisplayScore(player, &adjusted);
    if(!chicken.paused) DisplayNextEggs(player->gp, gpspr, player->board.left, player->board.top);

    // -----------------------------------
    // render the gamplay and text entities, which are happily confined to the
    // respective player's gameplay area via the following clipper:

    chicken.vs->setclipper(chicken.vs, &adjusted);

    Entity_Render(player->gameplay_ents);

    chicken.vs->setclipper(chicken.vs, NULL);

    // -----------------------------------
    // Finally, display the portraits, which again do not adhere to the clipper

    //Entity_Render(player->portrait_ents);

    // [...]
}
   



ENTITY Entity_Player = 
{
    "Player",
    STATE_NONE,
    0,
    Player_Deinitialize,
    Player_StateHandler,
    NULL,
    Player_Render
};
