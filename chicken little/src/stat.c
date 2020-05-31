
// Statistics related stuff! Among other things --> score system
// Chicken Little


// Air notes his score-system enhancements:
//  - I set the score up as an entity.  This way we can use the entity timing code
//    and do other fancy things down the road if we so please.



// But moot now, with entity-mode!  woo!

#include <string.h>
#include "stat.h"
#include "app.h"



void GetScore(SCORE *score, char *buffer, int len)
{
    /*
     *  Return as a string the score to be displayed.
     *  The score will be written in buffer, and it will
     *  Not be longer than len.
     */ 
    
    char temp[34];

    _itoa(score->current, temp, 10);    // Get a string
    memcpy(buffer, temp, len);          // and copy it into the user wanted buffer
}


int AddScore(SCORE *score, int ComboCount, int nMatch, int perfect)
{
    /*
     *  Pop up a magic formula here, and add to the score.
     *  ComboCount tells you which combo you are at.
     *  nMatch is the number of matching eggs you had.
     */

    int   retval;
    
    // Air: I worked out the math, and this formula seemed pretty good.  Most importantly:
    // it makes use of the bottom two digits of the score (instead of everything being
    // factors of 100)

    retval = (perfect * 750) + (ComboCount*(nMatch+ComboCount))*250 + nMatch*25;
    score->target += retval;

    return retval;
}


void ResetStat(STAT *s)
{
    int x;

    for (x=0;x!=STAT_MAXCOMBO;x++)
        s->nCombo[x] = 0;

    s->nEggs    = 0;
    s->nMatches = 0;
    s->nStones  = 0;
}


#define MIN_INCREMENT   1

enum
{
    SCORE_STATE_DIALING = ENTITY_NUMSTATES
};


// =====================================================================================
    static int Score_StateHandler(SCORE *score)
// =====================================================================================
{
    switch(score->entity.state)
    {    
        case ENTITY_STATE_WAITING:
            if (score->target != score->current)
                Entity_SetState(&score->entity, SCORE_STATE_DIALING);

        break;        
        
        case SCORE_STATE_DIALING:
            
            // Dialing rules:
            //  We want our dialer to kinda 'slow down' as it nears the end of its 
            //  dialing, so we just use the following equation:
            //   [yay, my striving for simplicity actually worked this time, unlike
            //    my botch-up of the matching code]

            score->current += ((int)(score->target - score->current) / 8) + MIN_INCREMENT;

            // if we are pretty close, then just dial over to it:
            
            if (score->target == score->current + MIN_INCREMENT)
            {
                score->current = score->target;
                Entity_SetState(&score->entity, ENTITY_STATE_WAITING);
                return STATE_PRIORITY_LOW;
            }

            App_Paint();
        return 40;
    }
    return STATE_PRIORITY_LOW;
}


// =====================================================================================
    static ENTITY Entity_Score =
// =====================================================================================
{
    "Score",
    ENTITY_STATE_WAITING,
    0,
    NULL,
    Score_StateHandler

};

// =====================================================================================
    SCORE *Score_Initialize(ENTITY **entlist)
// =====================================================================================
{
    SCORE *score;
    
    score = (SCORE *)Entity_Spawn(entlist, &Entity_Score, sizeof(SCORE));
    
    score->current   = 0;
    score->target    = 0;

    return score;
}
