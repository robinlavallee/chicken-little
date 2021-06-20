
#include "chicken.h"
#include "clmenu.h"
#include "app.h"

#include <windows.h>
#include "assert.h"
#include "mplayer.h"


extern GAMEDATA chicken;


TITLE       *cl_title    = NULL;
MENU        *cl_mainmenu = NULL,
            *cl_pausemenu = NULL;

int     cursong;

enum
{   KBD_SPACE  = 0,
    KBD_LEFT,
    KBD_RIGHT,
    KBD_DOWN,
    KBD_UP,
    KBD_ESCAPE
};


#define MMENU_NUMSEL  4

SELECTION selset_mainmenu[MMENU_NUMSEL] = 
{   
    {   174,124, 0, NULL,
        "Stage 1"   },

    {   174,200, 0, NULL,
        "Stage 2"   },

    /*{   174,276-35, 0, NULL,
        "Options"   },*/

    {   174, 280, 0, NULL,
        "Single" },

    {   260,352-35, 0, NULL,
        "Quit"      }
};


#define MENU_PAUSE_NUMSEL  2

SELECTION selset_pausemenu[MENU_PAUSE_NUMSEL] = 
{
    {   257,215, 0, NULL,
        "Resume"   },

    {   226,253, 0, NULL,
        "Surrender"   },
};


// =====================================================================================
    BOOL kbd_getkey(KBDATA *kb, uint key)
// =====================================================================================
// wow oh wow this is a hack.  I can't wait until after the competition when I can
// clean this up big time!
{
    BOOL   retval;
    
    switch(key)
    {
        case KBD_SPACE:
            retval    = kb->space;
            kb->space = 0;
        break;

        case KBD_LEFT:
            retval    = kb->left;
            kb->left  = 0;
        break;

        case KBD_RIGHT:
            retval    = kb->right;
            kb->right = 0;
        break;

        case KBD_DOWN:
            retval    = kb->down;
            kb->down  = 0;
        break;

        case KBD_UP:
            retval    = kb->up;
            kb->up    = 0;
        break;

        case KBD_ESCAPE:
            retval      = kb->escape;
            kb->escape  = 0;
        break;
    }

    return retval;
}


// =====================================================================================
    static void StartMatch(uint p1ctrl, uint p2ctrl)
// =====================================================================================
{
	ulong time;


    // Set stage specific info.
    switch (chicken.stage)
    {
    case 0:
        chicken.speed = 550;
        break;
    case 1:
        chicken.speed = 385;
        break;
    }


    time = App_GetTime();

    if (p2ctrl == CONTROL_NONE)
        chicken.player = (PLAYER **)_mm_calloc(1,sizeof(PLAYER *));
    else
        chicken.player = (PLAYER **)_mm_calloc(2,sizeof(PLAYER *));
    
    chicken.player[0] = (PLAYER *)Entity_Spawn(&chicken.playerlist, &Entity_Player,sizeof(PLAYER));

    if (p2ctrl != CONTROL_NONE)
        chicken.player[1] = (PLAYER *)Entity_Spawn(&chicken.playerlist, &Entity_Player,sizeof(PLAYER));

    Player_Initialize(chicken.player[0], 0, PORTRAIT_CHICKEN, chicken.speed, time);

    if (p2ctrl != CONTROL_NONE)
        Player_Initialize(chicken.player[1], 1, chicken.stage ? PORTRAIT_TURKEY : PORTRAIT_GOOSEY, chicken.speed, time);

	chicken.player[0]->control = p1ctrl;

    if (p2ctrl != CONTROL_NONE)
        chicken.player[1]->control = p2ctrl;


    if (p1ctrl == CONTROL_CPU) // That is, a demo game.
    {   
        AI_Init(AI_THINK_NORMAL, AI_MOVE_PERFECTFAST, &chicken.player[0]->ai, chicken.player[0]->gp);
        AI_Init(AI_THINK_NORMAL, AI_MOVE_PERFECTFAST, &chicken.player[1]->ai, chicken.player[1]->gp);
    } 
    else
    {
        if (p2ctrl != CONTROL_NONE) 
        {
            if(chicken.stage)
                AI_Init(AI_THINK_NORMAL, AI_MOVE_PERFECTFAST, &chicken.player[1]->ai, chicken.player[1]->gp);
            else
                AI_Init(AI_THINK_NORMAL, AI_MOVE_PERFECTSLOW, &chicken.player[1]->ai, chicken.player[1]->gp);
        }
    }

	// Let's give a starting place for our egg
    // Theoretically this could change from stage to stage.

    if (p2ctrl != CONTROL_NONE)
    {
	    chicken.player[0]->board.left   = 15;
	    chicken.player[0]->board.top    = 25;
        chicken.player[0]->board.right  = (30*8) + 15;
        chicken.player[0]->board.bottom = (30*13) + 25 + 3;

        chicken.player[1]->board.left   = 383;
	    chicken.player[1]->board.top    = 25;
        chicken.player[1]->board.right  = (30*8) + 383;
        chicken.player[1]->board.bottom = (30*13) + 25 + 3;
    }
    else // single player game
    {
   	    chicken.player[0]->board.left   = 184;
	    chicken.player[0]->board.top    = 25;
        chicken.player[0]->board.right  = (30*8) + 184;
        chicken.player[0]->board.bottom = (30*13) + 25 + 3;
    }

    Player_SetState(chicken.player[0],PLAYER_STATE_INTRO);

    if (p2ctrl != CONTROL_NONE)
        Player_SetState(chicken.player[1],PLAYER_STATE_INTRO);
}


// =====================================================================================
    static void UpdateControlStatus(KBDATA *kbd)
// =====================================================================================
//
{
    uint  time = App_GetTime();

    if(KEY_DOWN(VK_SPACE) || KEY_DOWN(VK_UP))
    {   DoKeyLogic(space,160)
    } else
    {   kbd->space    = 0;
        kbd->space_to = 0;
    }

    if(KEY_DOWN(VK_LEFT))
    {   DoKeyLogic(left,115)
    } else
    {   kbd->left    = 0;
        kbd->left_to = 0;
    }

    if(KEY_DOWN(VK_RIGHT))
    {   DoKeyLogic(right,115)
    } else
    {   kbd->right    = 0;
        kbd->right_to = 0;
    }
}

#define DoKeyStall(var)         \
{                               \
    kbd->var      = 0;          \
    kbd->var##_in = 1;          \
    kbd->var##_to = time+350;   \
}

// =====================================================================================
    void kbd_stall(KBDATA *kbd)
// =====================================================================================
{
    uint time = App_GetTime();
    
    DoKeyStall(space);
    DoKeyStall(up);
    DoKeyStall(down);
    DoKeyStall(left);
    DoKeyStall(right);
}

// =====================================================================================
    static void GetPlayerInput(GAMEDATA *chick, KBDATA *kbd)
// =====================================================================================
// We need to do 2 passes, for both players (this might change later).  Checks the
// control type and acts accordingly: if the player is the CPU, then it makes the CPU per-
// form moves and stuff.
{
	int     p = 0;
    int     nPass;

	if(!chick->player) return;

    if (chick->options.single == TRUE)
        nPass = 1;
    else
        nPass = 2;

    for (; p!=nPass; p++)
	{
		PLAYER     *player = chick->player[p];
        
        // We only process input if the player is in the 'falling' state

        if(Player_IsControllable(player))
		{   
            switch(player->control)
            {   
                case CONTROL_HUMAN:

                    if (kbd_getkey(kbd,KBD_SPACE))
                    {
                        if(GP_CheckTurn(player->gp, TURN_CLOCKWISE) == TRUE)
                            Player_MoveTurn(player, TURN_CLOCKWISE);
                        else
                        {
                            // All is not lost! We may be able to move it right
                            if (GP_CheckRight(player->gp))
                            {
                                GP_MoveRight(player->gp);
                                if (GP_CheckTurn(player->gp, TURN_CLOCKWISE)) // Bug fix
                                    Player_MoveTurn(player, TURN_CLOCKWISE);
                                else // This else actually happens when the egg is at the top
                                {
                                    GP_MoveLeft(player->gp); // Bring it back
                                }
                            }
                            else if (GP_CheckLeft(player->gp))
                            {
                                GP_MoveLeft(player->gp);
                                if (GP_CheckTurn(player->gp, TURN_CLOCKWISE))
                                    Player_MoveTurn(player, TURN_CLOCKWISE);
                                else
                                {
                                    GP_MoveRight(player->gp); // oops
                                }
                            }
                        }

                        App_Paint();
                    } else if (kbd_getkey(kbd,KBD_LEFT))
                    {   if(GP_CheckLeft(player->gp)) GP_MoveLeft(player->gp);
                        App_Paint();
                    } else if (kbd_getkey(kbd,KBD_RIGHT))
                    {   if (GP_CheckRight(player->gp)) GP_MoveRight(player->gp);
                        App_Paint();
                    }
                    
                    // Down arrow works differently - as long as it is pressed,
                    // the eggs fall fast.

                    if(KEY_DOWN(VK_DOWN))
                        Player_FallFast(player);
                    else
                        Player_FallSlow(player);
                break;
		

                case CONTROL_CPU:
                {   // Now is the time to let our great AI comes in!
                    // 0) Should the AI play or not?
                    // 1) We need to make it think.
                    // 2) We need to know its decision
                    // 3) We need to make it play in order to achieve 2)


                    // Making the AI think is relatively easy because all you have to do is the following :

                    AI_DECISION ad;
                    int         move;

                    if (App_GetTime() < player->lastmove + player->speed/1.3)
                        break;

                    AI_Feed(&player->ai, player->gp->current[0], player->gp->current[1], player->gp->next[0], player->gp->next[1], player->gp->next[2], player->gp->next[3]);
                    AI_Think(&player->ai, player->gp, 100);      // This makes it think. If the AI has thought enough, the function will return immediatly.
                    
                    // Then we get its decision
                    AI_GetDecision(&player->ai, &ad); // This fill out the decision structure. Basically it says where in x it is playing
                                                      // And in what relative position the eggs should be.

                    move = AI_DoMove(&player->ai, player->gp);
                    player->lastmove = App_GetTime();

                    // move may contain nothing, this happen when the AI is really acting dumb, or for some
                    // reason, the AI is unable to find a path to get to its decision

                    // Set it to slow, it will be overriden by the fast if it is the
                    // case

                    Player_FallSlow(player);

                    switch(move)
                    {
                        case AI_MOVE_LEFT:
                            GP_MoveLeft(player->gp);
                            break;
                        
                        case AI_MOVE_RIGHT:
                            GP_MoveRight(player->gp);
                            break;

                        case AI_MOVE_TURNCLOCKWISE:
                            Player_MoveTurn(player, TURN_CLOCKWISE);
                            break;

                        case AI_MOVE_TURNCOUNTERCLOCKWISE:
                            Player_MoveTurn(player, TURN_COUNTERCLOCKWISE);
                            break;
                        case AI_MOVE_FALLFAST:
                            Player_FallFast(player);
                            break;

                    }
                    App_Paint();

                }
                break;
            }
        }
    }  
}


KBDATA  p_kbd;


// =====================================================================================
    void DoGameWork(KBDATA *kbd)
// =====================================================================================
// Responsible for ensuring that all the entities get updated properly.
// This thing has some pretty evil logic right now (read: HACK!).  We want to make sure
// we get the entity state updates all 'caught up' if we lose some time due to a bad
// framerate.  Hence, I call the state updates, and if they return negative values
// it means they aren't totally caught up yet.  Then I loop until the values are non-
// negative, passing 0 as the timepass.
{

	ulong timepass, time;
	int   i;

    time         = App_GetTime();
    timepass     = time - chicken.time;

    if(timepass)
    {   int  zeb, jeb = 0;

        chicken.time = time;

        switch(chicken.state)
        {
            // =========================================================================
            case CL_STATE_INTRO:
            // =========================================================================
            // just wait a while for the text to go away:

                if(chicken.passtime < time)
                    CL_SetGameState(&chicken, CL_STATE_TITLE);
            break;

            // =========================================================================
            case CL_STATE_TITLE:
            // =========================================================================
            // after some time passes, let's spawn our demo game.
                
                if(chicken.passtime < time)
                    CL_SetGameState(&chicken, CL_STATE_DEMO);

            // =========================================================================
            case CL_STATE_DEMO:
            // =========================================================================
            // During the intro and title sequences we merely wait for a keypress
            // to jump us to the menu sequence.

                if(kbd_getkey(kbd,KBD_ESCAPE) || kbd_getkey(kbd,KBD_SPACE))
                //if(KEY_DOWN(VK_RETURN) || KEY_DOWN(VK_SPACE) || KEY_DOWN(VK_ESCAPE))
                {
                    CL_SetGameState(&chicken, CL_STATE_MENU);
                    //kbd_stall(kbd);              // keeps user input from happening too fast.
                }

                GetPlayerInput(&chicken, &p_kbd);

                if(chicken.passtime < time)
                    CL_SetGameState(&chicken, CL_STATE_TITLE);
            break;

            // =========================================================================
            case CL_STATE_MENU:
            // =========================================================================

                if(kbd_getkey(kbd,KBD_DOWN))
                    Menu_MoveDown(cl_mainmenu);

                if(kbd_getkey(kbd,KBD_UP))
                    Menu_MoveUp(cl_mainmenu);
                
                if(kbd_getkey(kbd,KBD_SPACE))
                {
                    // Take whatever is the current selection in our menu and run it:

                    // In any case, you play the usual "menu select" sound.
                    // Later on we may decide to give different sound to different option
                    // Air: Also, later on this will probably be moved to the menu entity

                    XAudioBuffer_Play(sfx.menuselect);
                    //mdsfx_playeffect(sfx.menuselect,vs_sndfx,SF_START_BEGIN,0);

                    switch(cl_mainmenu->cursel)
                    {
                        case 0:
                            // Start a new game on stage one (clouds!)

                        	chicken.stage = 0;
                            CL_SetGameState(&chicken, CL_STATE_MATCH);
                        break;

                        case 1:
                            // Start a new game on stage two (Turkey Lurkey!)

                        	chicken.stage = 1;
                            CL_SetGameState(&chicken, CL_STATE_MATCH);
                        break;

                        case 2:
                            // Start a single player game
                            chicken.stage = 1;
                            CL_SetGameState(&chicken, CL_STATE_SINGLEMATCH);
                        break;

                        case 3:
                            // Say goodbye, gracie!
                            App_Close();
                        break;
                    }
                }

            break;

            // =========================================================================
            case CL_STATE_MATCH:
            case CL_STATE_SINGLEMATCH:
            // =========================================================================
            // We get the player input here, if the player is a computer this function
            // will make it do something

                if(chicken.paused)
                {
                    BOOL a,b;
                    
                    if(kbd_getkey(kbd,KBD_DOWN))
                        Menu_MoveDown(cl_pausemenu);

                    if(kbd_getkey(kbd,KBD_UP))
                        Menu_MoveUp(cl_pausemenu);

                    if((a=kbd_getkey(kbd,KBD_ESCAPE)) || (b=kbd_getkey(kbd,KBD_SPACE)))
                    {   
                        BOOL quit = 0;
                        chicken.paused = 0;

                        if(a) cl_pausemenu->cursel = 0;
                        
                        switch(cl_pausemenu->cursel)
                        {
                            case 1:
                                CL_SetGameState(&chicken, CL_STATE_TITLE);
                                quit = 1;
                            break;
                        }

                        Entity_Kill(&cl_pausemenu->entity);
                        cl_pausemenu = NULL;

                        if(quit) return;
                    }

                } else
                {
                    if(chicken.player[0]->entity.state == ENTITY_STATE_WAITING)
                    {
                        // oh, the game's over.  Change the state of the game.

                        if(!chicken.player[0]->winner)
                            CL_SetGameState(&chicken, CL_STATE_TITLE);

                        else if(chicken.stage)
                        {
                            CL_SetGameState(&chicken, CL_STATE_GAMEOVER);
                        } else
                        {   
                            chicken.stage = 1;
                            CL_SetGameState(&chicken, CL_STATE_MATCH);
                        }
                    }

                    UpdateControlStatus(&p_kbd);
                    GetPlayerInput(&chicken, &p_kbd);
                    
                    if(kbd_getkey(kbd,KBD_ESCAPE))
                    {
                        chicken.paused = 1;
                        cl_pausemenu = Menu_Initialize(&chicken.entitylist, selset_pausemenu, MENU_PAUSE_NUMSEL);
                        
                        cl_pausemenu->cursel = 0;
                        
                        selset_pausemenu[0].font  = font.little;
                        selset_pausemenu[1].font  = font.little;
                    }
                }
            break;


            case CL_STATE_GAMEOVER:
            
                CL_SetGameState(&chicken, CL_STATE_START);
            break;
        }


        // The first call.  We actually give the stateUpdate procedures
        // the *real* timepass at this point.
        
        zeb = Entity_Update(&chicken.entitylist, timepass);
        if(jeb > zeb) jeb = zeb;

        if(!chicken.paused)
        {   if(chicken.player)
            {   
                if (chicken.options.single == FALSE)
                {
                    for (i=0; i!=2; i++)
                    {   zeb = Player_UpdateEntities(chicken.player[i],timepass);
                        if(jeb > zeb) jeb = zeb;
                    }
                }
                else
                {
                    zeb = Player_UpdateEntities(chicken.player[0], timepass);
                    if(jeb > zeb) jeb = zeb;
                }
            }

            zeb = Entity_Update(&chicken.playerlist, timepass);
            if(jeb > zeb) jeb = zeb;
        }

        while(jeb < 0)
        {
            // Uh oh, we're behind.  Need to catch up.  No time has passed
            // since the last calls (obviously), so pass 0 as timepass and
            // wait until everything is back on track.

            jeb = 0;

            zeb = Entity_Update(&chicken.entitylist, 0);
            if(jeb > zeb) jeb = zeb;

            if(!chicken.paused)
            {
                if(chicken.player)
                {   
                    if (chicken.options.single == FALSE)
                    {
                        for (i=0; i!=2; i++)
                        {   zeb = Player_UpdateEntities(chicken.player[i],0);
                            if(jeb > zeb) jeb = zeb;
                        }
                    }
                    else
                    {
                        zeb = Player_UpdateEntities(chicken.player[0], 0);
                        if (jeb > zeb) jeb = zeb;
                    }
                }

                zeb = Entity_Update(&chicken.playerlist, 0);
                if(jeb > zeb) jeb = zeb;
            }
        }
    }

}

extern void Intro_Renderer(void);
extern void Match_Renderer(void);

// =====================================================================================
    void CL_SetGameState(GAMEDATA *chick, int state)
// =====================================================================================
// Set the state of the entire game.  Anytime there is a state change resources will be
// unloaded and new ones loaded as needed.
{

    // Run the old state 'leaving' code:

    switch(chick->state)
    {
        case CL_STATE_START:
        break;

        case CL_STATE_TITLE:
        break;

        case CL_STATE_MENU:
            // Remove the menu entity

            Entity_Kill(&cl_mainmenu->entity);
            cl_mainmenu = NULL;

            Entity_Kill(&cl_title->entity);
            cl_title = NULL;
        break;

        case CL_STATE_MATCH:
        case CL_STATE_SINGLEMATCH:
          XAudioMusic_Stop(cursong);
            cursong = 0;

        case CL_STATE_DEMO:
            // Remove the player entities.
            if(chick->player)
            {   
                Entity_KillList(&chick->playerlist);
                _mm_free(chick->player, "player array (demo)");
            }

            if(cl_pausemenu) Entity_Kill(&cl_pausemenu->entity);
            cl_pausemenu = NULL;
        break;
    }


#ifdef _DEBUG
    {
        ENTITY *cruise;

        cruise = chick->entitylist;
        while(cruise)
        {
            _mmlog("Left over entity: %s", cruise->name);
            cruise = cruise->next;
        }
    }
#endif

    chick->state = state;

    CL_LoadResources(chick);

    // This should probably change to a 'loading' game state which calls these
    // procedures.  But for now, this will do!

    VDRes_LoadDependencies(respak->videopak);
    MDRes_LoadDependencies(respak->audiopak);

    // Set our time, so that the loading time doesn't make the game
    // think it is running behind:

    chick->time = App_GetTime();

    switch(state)
    {
        case CL_STATE_START:
            chick->state = CL_STATE_INTRO;

        // =============================================================================
        case CL_STATE_INTRO:
        // =============================================================================

            App_SetRenderer(Intro_Renderer);

            if(!cursong) {
              cursong = music.title;
              XAudioMusic_Play(cursong);
            }

            {
                TEXTINFO *text;
                text = TextInfo_Initialize(&chick->entitylist, "Gratisgames", 88, 172, font.big);
                text->entity.timeout = 1150;
                text = TextInfo_Initialize(&chick->entitylist, "Presents", 148, 226, font.big);
                text->entity.timeout = 1150;
            }
            
            chicken.passtime = chicken.time + 2050;
        break;

        // =============================================================================
        case CL_STATE_TITLE:
        // =============================================================================

            App_SetRenderer(Intro_Renderer);

            // Spawn the 'title screen' entity, which will fade in the title screen
            // and then run whatever sequence we feel like running.

            cl_title = Title_Initialize(&chicken.entitylist);
            chick->passtime = chick->time + 17000;

            if(!cursong) {
              cursong = music.title;
              XAudioMusic_Play(cursong);
            }

        break;


        // =============================================================================
        case CL_STATE_DEMO:
        // =============================================================================
        // Fade our title out and spawn the gameplay entities.

            App_SetRenderer(Intro_Renderer);

            Entity_SetState(&cl_title->entity,TITLE_STATE_CROSSFADE);

            chicken.stage = 1;
            StartMatch(CONTROL_CPU, CONTROL_CPU);

            chick->passtime = chick->time + 43800;
        break;


        // =============================================================================
        case CL_STATE_MENU:
        // =============================================================================
        // Fade our title out and spawn the menu entities.

            App_SetRenderer(Intro_Renderer);

            Entity_SetState(&cl_title->entity,TITLE_STATE_CROSSFADE);
            cl_mainmenu = Menu_Initialize(&chicken.entitylist, selset_mainmenu, MMENU_NUMSEL);
        break;

        // =============================================================================
        case CL_STATE_MATCH:
        // =============================================================================

            CLEAR_STRUCT(p_kbd);
            App_SetRenderer(Match_Renderer);

            if(cursong) {
                XAudioMusic_Stop(cursong);
                cursong = 0;
            }

            cursong = music.stage[chick->stage];
            XAudioMusic_Play(cursong);

            StartMatch(CONTROL_HUMAN, CONTROL_CPU);
        break;

        // =============================================================================
        case CL_STATE_SINGLEMATCH:
        // =============================================================================
        // A single player game is started!

            CLEAR_STRUCT(p_kbd);
            App_SetRenderer(Match_Renderer);

            if(cursong)
            {   
                XAudioMusic_Stop(cursong);
                cursong = 0;
            }
            
            
              cursong = music.stage[chick->stage];
              //Player_SetVolume(cursong, chick->stage ? 38 : 60);
              XAudioMusic_Play(cursong);
            
            chicken.options.single = TRUE;
            StartMatch(CONTROL_HUMAN, CONTROL_NONE);
                   
        break;

        // =============================================================================
        case CL_STATE_GAMEOVER:
        // =============================================================================
        // This would be a prime place to put the continue screen or something.

        break;
    }

}


