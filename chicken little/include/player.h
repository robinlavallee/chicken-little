/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: player.h

  Contains entities and state information that are directly related to the player:

   PLAYER     - the player information struct
   GAMEPIECE  - the gamepiece entity (one for each sqare in the playfield!)

*/


#ifndef _CLPLAYER_H_
#define _CLPLAYER_H_

#include "stat.h"
#include "gameplay.h"
#include "ai.h"
#include "vdfont.h"

#include "textinfo.h"

enum
{   PORTRAIT_NONE  = 0,
    PORTRAIT_CHICKEN = 0,
    PORTRAIT_GOOSEY,
    PORTRAIT_TURKEY,

    PORTRAIT_NUMCHARS
};


// =====================================================================================
//  Player Defines and Enumerations
// =====================================================================================
// Stuff related to the Player as a whole.
// Important!  If you add any new 'controllable' states (in which the player's piece is
// moveable or rotate-able) then you must add them in between STATE_WAITING and STATE_SOLIDIFY!
// (See Player_IsControllable at the bottom of this header for more info).

enum
{   
    PLAYER_STATE_INTRO = ENTITY_NUMSTATES,
    PLAYER_STATE_GETREADY,
    PLAYER_STATE_GO,
    PLAYER_STATE_START,
    PLAYER_STATE_GETNEXTPIECE,       // Launch the next piece.

    // ** Start of player control states **
    PLAYER_STATE_WAITING,            // a waiting state after the piece has been launched
    PLAYER_STATE_EGGFALLING,         // the eggs are falling, normal speed
    PLAYER_STATE_EGGFALLING_FAST,    // the eggs are falling FULL speed!
    PLAYER_STATE_SOLIDIFY,           // the egg is about to become part of the gameboard
    // ** End of player control states **

    PLAYER_STATE_FINDMATCHES,        // Looking for matches and stuff.
    PLAYER_STATE_WIN,                // the 'we won' sequence
    PLAYER_STATE_LOSE,               // the 'we lost' sequence
    PLAYER_NUMSTATES
};

#define Player_SetState(p,x) Entity_SetState(&(p)->entity,x)


enum
{   DUMP_NOTHING = -1,
    DUMP_EGG,
    DUMP_STONE,
    DUMP_METEOR,
    NUM_DUMP            // Number of different type of dumpable object.
};

#define ATTACK_ICE     1
#define ATTACK_FIRE    2
#define ATTACK_QUAKE   4


// =====================================================================================
    typedef struct THROWNPIECES 
// =====================================================================================
// structure used for when you are sending pieces to your neighboor. :)
// This one keeps track of where the pieces are falling
{
    int value[SIZEX];
} THROWNPIECES;


// =====================================================================================
    typedef struct DUMP
// =====================================================================================
// Structure used to find out what many stuff are going to fall at you
{
    int nPiece[NUM_DUMP];   // So that it is versatile if we want to add more stuff later on
} DUMP;


// =====================================================================================
    typedef struct ATTACK
// =====================================================================================
// Want to subject your opponent to nasty things?  Then use me!  Woohoo!
// Kaboooom!  Crash!  Rapid death by german blitzkreig!  Whooosh!
{
    DUMP   dump;                // What is falling from the sky? Bombs? Missiles? Stones?
    int    special;             // special attack flag.
    BOOL   ready;
} ATTACK;


typedef struct PORTRAIT
{
    ENTITY    entity;

    uint      index;               // player index we are attached to.
    uint      pic;

    int       scrollpos;
    int       xrel;                // x adjustement to the portrait
} PORTRAIT;


// =====================================================================================
    typedef struct PLAYER
// =====================================================================================
{
    ENTITY     entity;               // we 'inherit' the entity structure :)
    GAME_INFO  *gp;                   // our game information (so we can use gameplay.c!)

    int        index;                // player index (player 1/2/3?)

    int        control;              // what is controlling this player, human or cpu?
    int        speed;                // player speed (in ms delay between 'ticks')

    BOOL       gameover, winner;

    VRECT      board;                // location of and size of the player gameboard.

    int        scrollpos;

    ATTACK     attack;               // queue attack on you! (prepare to die!)
    int        combo;                // combo counter.
    int        clear;                // number of eggs cleared counter
    STAT       stat;                 // stat related information

    SCORE     *score;               // our score entity!

    ENTITY    *gameplay_ents;        // list of gameplay-area entities

    PORTRAIT  *portrait;             // 

    AI        ai;                    // Artificial intelligence related stuff
    uint      lastmove;              // time the last move was performed, for the AI so that it doesn't play too fast
} PLAYER;

extern int      _gpstate_update(PLAYER *player, ulong timepass);

// this is provided so child classes can make callacks if they want
extern int      GamePiece_StateHandler(PLAYER *player, int x, int y);


// PLAYER.C Defined Prototypes
// -----------------------------

extern void     Player_Initialize(PLAYER *player, uint index, uint character, int speed, ulong time);
extern void     Player_GhostifyPiece(PLAYER *player, int x, int y);
extern int      Player_UpdateEntities(PLAYER *player, uint timepass);
extern void     Player_MoveTurn(PLAYER *player, int direction);


// PIECEDUMP.C Defined Prototypes
// ------------------------------

extern int      DumpCount(DUMP *d); // Return the number of piece that are going to fall
extern int      GetDumpPiece(DUMP *d);
extern void     ThrowPiecesFillStruct(GAME_INFO *p, DUMP *d, THROWNPIECES *fall);
extern void     Attack(GAME_INFO *p, DUMP *d);
extern void     DumpReset(DUMP *d);


// ------------------------
//    Global Variables
// ------------------------

extern GAMEPIECE_ENTITY  Entity_Egg, Entity_ChaosEgg;
extern GAMEPIECE_ENTITY  Entity_Stone, Entity_Meteor;

extern ENTITY Entity_Player;

// player/gamepiece related sound effects!

#define PCTRL_STATE_START  PLAYER_STATE_WAITING
#define PCTRL_STATE_END    PLAYER_STATE_SOLIDIFY

// =====================================================================================
    static BOOL __inline Player_IsControllable(PLAYER *player)
// =====================================================================================
// Checks the state of the given player (based on the defives above) and returns:
//   TRUE   if the player is accepting input (their piece is allowed to be moved.
//   FALSE  if the piece is either not-present or in an unmoveable state.
{
    return ((player->entity.state >= PCTRL_STATE_START) && (player->entity.state <= PCTRL_STATE_END));
}


// =====================================================================================
    static void __inline Player_FallFast(PLAYER *player)
// =====================================================================================
// Given the proper conditions, sets the player state to EGGFALLING_FAST.  If the player
// is not in one of the states where he is allowed to do that, then nothing happens.
{

    if(player->entity.state == PLAYER_STATE_EGGFALLING || (player->entity.state == PLAYER_STATE_SOLIDIFY))
        Player_SetState(player,PLAYER_STATE_EGGFALLING_FAST);
}


// =====================================================================================
    static void __inline Player_FallSlow(PLAYER *player)
// =====================================================================================
// Given the proper conditions, sets the player state to EGGFALLING.  If the player
// is not in one of the states where he is allowed to do that, then nothing happens.
{

    if(player->entity.state == PLAYER_STATE_EGGFALLING_FAST)
        Player_SetState(player,PLAYER_STATE_EGGFALLING);
}


#endif
