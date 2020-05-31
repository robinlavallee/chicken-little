#ifndef _CHICKEN_H_
#define _CHICKEN_H_


enum
{
    CL_STATE_START = 1,
    CL_STATE_INTRO,
    CL_STATE_TITLE,
    CL_STATE_DEMO,
    CL_STATE_MENU,
    CL_STATE_MATCH,
    CL_STATE_SINGLEMATCH,
    CL_STATE_GAMEOVER,
};



// ===================================
//    Useful (used-by-all) includes
// ===================================

#include "app.h"
#include "vdriver.h"
#include "vdfont.h"

#include "random.h"
#include "clres.h"
#include "player.h"


#define DoKeyLogic(var, tout)          \
{                                      \
    if(kbd->var##_to)                  \
    {   if(kbd->var##_to <= time)      \
        {   kbd->var = 1;              \
            kbd->var##_to += tout;     \
        }                              \
    } else                             \
    {   kbd->var##_to = time + tout;   \
        kbd->var = 1;                  \
    }                                  \
}


//======================================================================================
    typedef struct KBDATA
//======================================================================================
// Thisnis really a temporary structure until we can get the 'real' keyboard/joystick
// input handler in place using the DIVE's KeyStick libraries!
{
    BOOL   space, escape;    // tracked as both (either) up or space
    BOOL   left, right;

    BOOL   up, down;         // up and down are tracked for menu-use only.

    // The following are timeouts for each key above.

    uint   left_to, right_to;
    uint   space_to, escape_to;

    uint   up_to, down_to;

    // The following are used to make the inital repeat delay longer than
    // all subsequent repeat keys!

    uint   left_in, right_in;
    uint   space_in, escape_in;

    uint   up_in, down_in;
} KBDATA;

extern void    DoGameWork(KBDATA *kbd);


//======================================================================================
    typedef struct GAMEOPTIONS
//======================================================================================    
{
    // These are user configuration options for 2 players game. (Against CPU or human)
    // They are saved in configuration files. So that the user doesn't have to set
    // Them again each time.

    // Other game options are not included in this structure. (Hence we should probably
    // rename it.

    // I think each game type should have its own configuration structure since option

    // Since there are many options. Some of them should be able to get automatically
    // set for certain level of difficulties. (Example, choosing easy will make speed=750
    // speedincrease to false, etc. Normal will do other things. Hard others.

    // Keep in mind that some of these boolean options should be mutually exclusive.
    
    int speed;      // Like the global speed of the game
    int scoregame;  // A score game is a normal game, except that it has a time limit.
                    // Once the time has passed, the one with the most points win.
                    // To enable this, simply set scoregame to something other than 0.
                    // The time is in minutes.
                    // Of course if you die, you are the loser.
    BOOL speedincrease; // Does the speed increase through the game ?
    BOOL stone;     // Are stones allowed or not? TRUE = yep!
    BOOL wildcard;  // Is the wildcard in or not? TRUE = yep!
    BOOL piecedump; // Are piece dump allowed?
    BOOL counterdump; // Are counter dump in the game?
    BOOL chaos;     // Are Chaos eggs effects enabled ?
    int survival;   // Survival mode. Normally this is set to 0 which means it is not enabled
                    // If set to like, 5, that means that after 5 mins. The game will start
                    // throwing at both player stones from time to time.
    int crazydump;  // Normally this is set to 0, which means that the game use the normal
                    // formula for dumping. If set to 1, 2, 3 and 4. The game will send
                    // Even more pieces to the other player. (Probably some kind of multiplier)
    BOOL helper;    // Like Puyo Puyo perhaps. When a match can be done with the current falling
                    // piece, the places where to place it will "flash". Good option for little
                    // brother
    int opponentlevel; // 0 = human player, other should be defined. (Probably has to do with AI)
                       // As the player play story mode, he should be able to "unlock" other game
                       // characters and then choose to play against them in this setting too.
    BOOL royalflush;// Another wacky idea. Once you have completed story mode. (Probably in hard
                    // mode in order to get all characters). Setting this will put you in a hellish
                    // game where you are alone against ALL characters. That is, each time you beat
                    // one, the game automatically clear its field, and replace it with another.
                    // The goal is to beat all characters without losing a single time.
    int nWin;       // number of win to win the game. (If you want to make 2 of 3, 3 of 5, 4 of 7 games).
    BOOL single;


} GAMEOPTIONS;


//======================================================================================
//                 Like.. the actual *CHICKEN LITTLE* Global Stuff
//======================================================================================


//======================================================================================
    typedef struct GAMEDATA
//======================================================================================
{
    int         state;              // current state of our game (see enumeration above)
    int         stage;              // tells resource loader what background to load

    BOOL        paused;

    ulong       passtime;

    ENTITY     *entitylist,
               *playerlist;
    PLAYER    **player;             // player related info

    uint        speed;              // time in ms before a block fall to the next dept, this control the speed of the game.
    ulong       time;               // current time (or time of last logic tick), used for timing game logic.

    // Here are the background resources.  Currently these are just sprites,
    // but if and when we implement animations, we will have to turn them into
    // entities or at least create more in-depth information block for them.

    SPRITE       background;        // current background
    SPRITE       layout;            // current layout

    VD_RENDERER  *vr;               // the rendering device object
    VD_SURFACE   *vs;               // pointer to the surface attached to our renderer.

    MDRIVER      *md;
    GAMEOPTIONS  options;           // gameoption

} GAMEDATA;

#define CONTROL_NONE    0
#define CONTROL_HUMAN   1
#define CONTROL_CPU     2


extern void    CL_SetGameState(GAMEDATA *chick, int state);
extern PLAYER *GetOpponent(PLAYER *p);           // Get the other player


extern MD_VOICESET *vs_sndfx;
 
#endif // _CHICKEN_H_
