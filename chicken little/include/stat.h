#ifndef _STAT_H_
#define _STAT_H_

#include "entity.h"

#define SCORE_DIGITS     9         // Maximum length of the score

#define STAT_MAXCOMBO   10         // range of combo types we track


// =====================================================================================
    typedef struct SCORE
// =====================================================================================
// This is our score.  It was turned into an entity so that it could increment itself 
// slowly, which gives the pleasing effect of the rolling score-counter.
{
    ENTITY  entity;                 // look, magic, we're an entity now! :)

    uint    target;                 // the targetted score
    uint    current;                // the score being displayed currently
    int     digit[SCORE_DIGITS];    // the value of each digit
} SCORE;


// =====================================================================================
    typedef struct STAT
// =====================================================================================
// Statistical information reguarding gameplay pieces.  this should be updated by the
// player entity and/or gameplay.c code as seen fit.
{
    uint    nEggs;                 // number of eggs destroyed since the begining
    uint    nStones;               // Number of stones destroyed.
    uint    nMatches;
    uint    nCombo[STAT_MAXCOMBO]; // This hold the number of combo of each type 
                                   // last one nCombo[STAT_MAXCOMBO] is the number of combo of MAXCOMBO and more.
} STAT;


extern SCORE  *Score_Initialize(ENTITY **entlist);

extern void    GetScore(SCORE *s, char *buffer, int len);
extern int     AddScore(SCORE *score, int ComboCount, int nMatch, int perfect);

extern void    ResetStat(STAT *s);

#endif // _STAT_H_
