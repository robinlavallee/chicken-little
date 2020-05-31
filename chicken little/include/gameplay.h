#ifndef _GAMEPLAY_H_
#define _GAMEPLAY_H_

#include "entity.h"
#include "random.h"

// ==================================================
//    Gameplay (player and such) related Constants
// ==================================================
// We put many of these defines inside #ifndef checks so that we can optionally override
// them via the compiler command line if we so wanted to.

#ifndef SIZEX
#define SIZEX              8
#endif

#ifndef SIZEY
#define SIZEY             13
#endif

#ifndef NUMPIECES_AHEAD
#define NUMPIECES_AHEAD    2
#endif

#ifndef GP_NUMPIECES
#define GP_NUMPIECES      11
#endif


#define gpidx(x,y) (((y)*SIZEX)+(x))

#define TURN_CLOCKWISE          1
#define TURN_COUNTERCLOCKWISE   2


// POS_xxx Enumeration
// Used to indicate the orientation of the 'partner' egg to the 'master' egg.

enum
{   POS_LEFT = 0,
    POS_ABOVE,
    POS_RIGHT,
    POS_BELOW
};

// These are the result values for the different types of game pieces.

#define WILDCARD     1
#define COLOR_START  2
#define COLOR_END    5
#define BROWN_START  6
#define BROWN_END    8
#define STONE        9
#define METEOR      10


// The gamepiece match types.  Matching logic is currently hard-coded and warrants
// the presence of this enumeration.

enum
{   GP_EMPTY = 0,                // no entity here
    GP_UNMATCHABLE,              // not matchable in any way (frozzen/stone/etc)
    GP_COLOR,                    // matchable via exact color (all eggs)
    GP_BROWN,                    // matchable with each other
    GP_WILDCARD,                 // matches up with anything.
    GP_STONE
};


// =====================================================================================
//  GamePiece Defines and Enumerations
// =====================================================================================
// The gamepiece states, made public so things in gameplay.c and player.c can have fine
// control over what each gamepiece is doing.

enum
{   GP_STATE_WAITING = 1,        // Normal state, the eggs are sitting happily.
    GP_STATE_GRAVITY,            // gravity is being applied.
    GP_STATE_BOUNCE,             // cute bounce after gravity fall.
    GP_STATE_DESTROY,            // Gamepiece being destroyed.
    GP_STATE_FROZEN,             // gamepiece is frozen (inert)
    GP_NUMSTATES
};


// Thanks to these macros, you should never really have to use any of the functions
// above directly.  These macros make it so you can pass any entity which contains
// the STATE structure to the state API procedures.

#define gpState_Update(p,s)        _gpstate_update(p,s)
#define gpState_SetState(o,s)      _gpstate_setstate(&(o)->stateinfo,s)
#define gpState_RemapEntity(x,y)   _gpstate_remap(x,y)


// =====================================================================================
    typedef struct GPSTATE
// =====================================================================================
// A speciailized state structure created especially for use by, and only by, the game
// pieces that make up each players playing field.  This was needed due to the static-
// style of array we use to define the gamefiled (which greatly simplifies many tasks
// as well as greatly enhance speed!)
{	
    int       state,          // current state.
              oldstate;       // the previous state
    int       timeout;

    // the state handler functions 
    // handler - (called when waiting period in 'timeout' expires)
    // leave   - called when a new state is being entered (or on the creation of the entity)

    int      (*handler)(void *owner, int x, int y);
    int      (*leave)(void *owner, int x, int y, int newstate);

    // Animation information
    // not sure if I want to keep it housed here, but it works for now.

    ANIM_STATE  *animstate;      // array of animations for each state type.
    ANIMATION    anim;           // our gamepiece animation struct
} GPSTATE;



// =====================================================================================
    typedef struct GAMEPIECE_ENTITY
// =====================================================================================
{
    int     (*statehandler)(void *owner, int x, int y);
    int     (*loadres)(CL_RES *respak, ANIM_STATE *dest, RES_GPSPR *sprptr);
    BOOL    (*isinert)(uint state);
} GAMEPIECE_ENTITY;


// =====================================================================================
    typedef struct GAMEPIECE
// =====================================================================================
// Each gamepiece in the player's field of play is represented by one of these struc-
// tures.  Note that I removed 'matchtype' because from now on we will always use the
// matchtype array in gameplay.c.  This limits confusion and potential fuckups a lot.
{
    GPSTATE    stateinfo;

    // Notes:
    //  - Type is only used if matchtype is one of the matchable types.

    int        type;          // the specific resource-type of this object (see clres.h/clgfxres.h)
    uint       gravidx;       // gravity table indexer
} GAMEPIECE;


// =====================================================================================
    typedef struct GP_PIECEINFO
// =====================================================================================
// This array descirbes our gamepieces.  The first variable is the matchtype logic they
// will use.  The second is a pointer to the entity that the end user will see.  See
// gameplay.c for the use of this struct.
{
    uint               matchtype;
    GAMEPIECE_ENTITY  *entity;
} GP_PIECEINFO;


extern GP_PIECEINFO gp_info[GP_NUMPIECES];              // table describes matchtype for each piecetype.


// =====================================================================================
    typedef struct GAME_INFO
// =====================================================================================
// This defines the current controllable piece of whatever player entities there happen
// to be.  This has been encapsulated as such because botht he gameplay and AI code must
// manipulate this information frequently.
{   
    ENTITY     entity;

    GAMEPIECE  gamearray[SIZEY*SIZEX];  
    int        current[2];      // current piece the player has
    int        next[4];         // The two next pieces.

    int        timeout;

    int        pos;             // relative position to the other egg
    int        disppos;         // displayed position to the other egg

    int        index;           // current index into RotateX / RotateY charts,
                                // modifies disppos.

    int        x;               // current piece position, where it is.
    int        y;
    int        ypart;           // this tells the renderer which 'step' in falling we are at.

    DE_RAND    *randomizer;     // a handle to the random device.

} GAME_INFO;

extern void     _gpstate_setstate(GPSTATE *stateinfo, int type);
extern void     _gpstate_remap(int dx, int dy);

extern void  GP_GetInitialPieces(GAME_INFO *gi);
extern void  GP_GetNextPiece(GAME_INFO *gi);
extern void  GP_GetPiece(GAME_INFO *gi, int *piece);
extern uint  GP_GetScoreLocation(GAMEPIECE *array, int *xtotal, int *ytotal);

extern void  GetRelativeLine(int deep, int *line, GAMEPIECE *array, BOOL sStone);
extern void  GP_FindOneMinimumHeight(GAME_INFO *gp, int *x, int *y);
extern int   GP_FindHeight(const GAMEPIECE *array, int x);
extern int   GP_FindNumAbove(const GAMEPIECE *array, int x, int y);

extern BOOL  GP_IsInert(const GAMEPIECE *array);
extern BOOL  GP_IsDead(const GAME_INFO *p);
extern BOOL  GP_NoGravity(const GAMEPIECE *array);

extern int   GP_FindMatch(GAMEPIECE *array, BOOL dest);    // find matches in the array

extern void  GP_ResetPlayField(GAMEPIECE *gamearray);    // clear the playfield
extern void  GP_Solidify(GAME_INFO *p);	                 // solidify the current piece into the array
extern void  GP_ApplyGravity(GAME_INFO *p, int *result);    // apply gravity to the playfield

//extern void  GP_DestroyEggs(GAMEPIECE *gamearray, int *array); // Destroy the eggs of array, those that have 1's.


// Piece movement and rotation functions.
// --------------------------------------
// These check for validity of the movement, and if valid perform the move.

extern BOOL  GP_CheckDown(const GAME_INFO *p);
extern BOOL  GP_CheckLeft(const GAME_INFO *p);
extern BOOL  GP_CheckRight(const GAME_INFO *p);
extern BOOL  GP_CheckTurn(const GAME_INFO *p, int direction);
extern void  GP_MoveTurn(GAME_INFO *p, int direction);


extern GAME_INFO *GamePlay_Initialize(ENTITY **entlist, uint time);


// GAMEPIECE.C Defined Prototypes
// --------------------------------

extern BOOL     GamePiece_LoadResources(CL_RES *respak, RES_GPSPR *spr);
extern void     GamePiece_UnloadResources(CL_RES *respak);
extern void     GamePiece_Initialize(GAMEPIECE *array, int x, int y, int type);
extern void     GamePiece_Move(GAMEPIECE *array, int x, int y, int dx, int dy);

// =====================================================================================
    static BOOL GamePiece_CheckGravity(GAMEPIECE *gp, GAMEPIECE *gparray, int x, int y)
// =====================================================================================
{
    if(y < SIZEY-1)
    {   if(gparray[gpidx(x,y+1)].type == GP_EMPTY)
        {   gp->gravidx = 0;
            gpState_SetState(gp, GP_STATE_GRAVITY);
            return -1;
        }
    }
    return 0;
}


// =====================================================================================
//  MATCH MACROS (inline functions)
// =====================================================================================
// IsMatchable - 
// 

extern BOOL   IsMatchablePair(int typea, int typeb);    // return true if both types match

// =========================================
static BOOL __inline IsMatchable(int type)
// =========================================
// Returns TRUE if the piece type is not unmatchable or empty (meaning the piece
// can at least be matched to itself, if nothing else).
{
    return (gp_info[type].matchtype == GP_UNMATCHABLE || gp_info[type].matchtype == GP_STONE || gp_info[type].matchtype == GP_EMPTY) ? FALSE : TRUE;
}

// =========================================
static BOOL __inline IsBrown(int type)
// =========================================
{
    return (gp_info[type].matchtype == GP_BROWN);
}

// =========================================
static BOOL __inline IsWildcard(int type)
// =========================================
{
    return (gp_info[type].matchtype == GP_WILDCARD);
}



// =====================================================================================
//  GP_MoveXXXX MACROS (inline functions)
// =====================================================================================
// Simple stupid functions that move the player's current piece around.  They do not
// perform any checks (yet?). No need to, since they should be called after GP_CheckLeft,
// etc.

void __inline GP_MoveDown(GAME_INFO *p)
{
    // uses 'player->ypart' to make the movement of the piece smoother.

    if(p->ypart)
    {   
        // ypart was set, so clear it and move us down a line.
        p->ypart = 0;
    	p->y++;
    } else
    {   
        p->ypart++;
    }

}

void __inline GP_MoveLeft(GAME_INFO *p)
{
	p->x--;
}

void __inline GP_MoveRight(GAME_INFO *p)
{
	p->x++;
}


#endif // _GAMEPLAY_H_
