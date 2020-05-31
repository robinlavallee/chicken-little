#ifndef _AI_H_
#define _AI_H_

#include "gameplay.h"


#define AI_BAD -5000


// All the different type of AI
// These are the 'thinking' component of the AI.

#define AI_THINK_EASY 0
#define AI_THINK_NORMAL 1
#define AI_THINK_HARD 2
#define AI_THINK_TURKEYLURKEY 3



// This is the moving part of the AI

#define AI_MOVE_PERFECTSLOW 0
#define AI_MOVE_PERFECTFAST 1
#define AI_MOVE_HESITANT 2
#define AI_MOVE_MISTAKE 3
#define AI_MOVE_AWFUL 4


enum
{
    AI_MOVE_NONE = 0 ,
    AI_MOVE_TURNCLOCKWISE,
    AI_MOVE_TURNCOUNTERCLOCKWISE,
    AI_MOVE_RIGHT,
    AI_MOVE_LEFT,
    AI_MOVE_DOWN,
    AI_MOVE_FALLSLOW,
    AI_MOVE_FALLFAST,
};


typedef struct AI_DECISION
{
	int x;     // Position in x of egg1.
	int pos;   // Where egg1 is relative to egg2
} AI_DECISION;

// How do you get the move associate with it? Simply do the following :
// Move % 30 = Move type, specifically :
// 0 --> 6 = POS_LEFT
// 7 -> 13 = POS_RIGHT
// 14 -> 21 = POS_ABOVE
// 22 -> 29 = POS_BELOW 
// Move from 0 to 29 are 1st move
// Move from 30 to 929 are 2nd move (1st move associate : /30 - 1)
// Move from 930 to 27929 are 3rd move. (1st move associate /30 - 31 ; 2nd move associate -930 / 900)

// Most of the time, at least for now. We will just be using 1st move stuff.
// Giving them a 'score' is the tricky part. Score can be calculated for matches, good moves, etc.


typedef struct MOVEBANK
{
    int score; // value associate with the score
} MOVEBANK;

#define AIARRAY int // for now that's all it is. :P

typedef struct AI_ENGINE
{
    int         thinklevel;             // Thinking component
    int         movelevel;              // Moving component
    int         threshold;              // the minimum score to achieve to be any good
    GAMEPIECE  *gamearray;
    AIARRAY     aiarray[SIZEX*SIZEY]; // Do your internal calculation in this thing
    int         current[2];
    int         next[4];
    MOVEBANK    movebank[27930];       // account of move, with their score to each
    AI_DECISION ad;                     // Current decision to reach
} AI_ENGINE;

typedef struct AI
{
    BOOL        donethinking;              // Are you done thinking?
    AI_ENGINE   ae;
} AI;



// Define for the AI's move
int AI_DoMove(AI *ai, GAME_INFO *gp);

// current is a 2 bytes array. It is the current eggs we are playing, in THIS position
// 

//+----+----+
//|    |    |
//|egg1|egg2|
//|----+----+

// The same thing goes for next, which are egg3, and egg4. Then egg5 and egg6


void AI_Init(int thinklevel, int movelevel, AI *ai, GAME_INFO *gi);
void AI_Feed(AI *ai, int cur1, int cur2, int next1, int next2, int next3, int next4);

void  AI_Think(AI *ainf, GAME_INFO *p, int ms);
void  AI_GetDecision(const AI *ainf, AI_DECISION *ad);
void  AI_Cleanup();

int   AI_Find2Colors(int colora, int colorb, int *line);
int   AI_Find1Color(int colora, int colorb, int *line);

BOOL MatchColor(int colora, int colorb);


// AI internal calculation stuff prototype goes here!

int AI_ScoreMove(AI *ai, int move); // Give a score to the chosen move
int AI_FindMatch(AIARRAY *array, int *global); // Find matches
void AI_InternalKill(AIARRAY *array, int *flag); // Kill flagged pieces.
void AI_InternalWork(AIARRAY *array); // Fix the field (kill stones, apply gravity, etc)
BOOL AI_InternalPlay(AI *ai, int x, int pos, int *dx, int *dy, int *dx2, int *dy2); // Play a move (internally) with the current falling piece
void AI_Convert(GAMEPIECE *gp, AIARRAY *array); // Convert the gameplay array to one compatible with the AI
BOOL AI_Match(AIARRAY *array, int x, int y, int type);     // Return TRUE or FALSE if there is a match.
BOOL AI_WillMatch(AIARRAY *array, int x, int y, int type); // Would we have a match if we placed an egg there?
int AI_MatchRecurse(AIARRAY *array, int x, int y, int type, int *temp); // Match recursive function for AI


// Related to combo building

void AI_Initialization();
BOOL CheckReachable(AIARRAY *array, int tx, int ty, int pattern);
int FindReachableNearMatch(AI *ai, int *x, int *y);
BOOL ComboPrepare(AI *ai, int type, int x, int y, AI_DECISION *pos);

typedef struct COMBOPREPARTION
{
  int x, y;
} COMBOPREPARATION;

typedef struct NEARMATCH
{
	int x[2];
	int y[2];
	int nCombo;
	COMBOPREPARATION cp[5]; 
} NEARMATCH;

#endif // _AI_H_
