
/*
 *  Chicken Little.
 *  Lawgiver (Robin Lavallée - robin_lavallee@videotron.ca)
 *  Copyright by myself and Divine Entertainment. (2000)
 *  
 *  Artifical Intelligence, (or dumbness?)
 *  
 */

#include "ai.h"
#include <string.h>
#include <stdlib.h>
#include "chicken.h"
#include "player.h"


#define NUM_NEARMATCH 6
NEARMATCH nm[NUM_NEARMATCH];

void AI_Init(int thinklevel, int movelevel, AI *ai, GAME_INFO *gi)
{
	/*
	 *  Init artificial intelligence engine. (Wow, sounds cool)
	 *  Input : level, one of the following :
	 *  AI_EASY
	 *  AI_NORMAL
	 *  AI_HARD
	 *  x = horizontal size of the gameplay
	 *  y = vertical size of the gameplay
	 *  array = The game array.
	 *  Next = The eggs in the 'next' area. See convention in ai.h
	 */

    ai->ae.thinklevel = thinklevel;
    ai->ae.movelevel = movelevel;

    ai->ae.current[0] = gi->current[0];
    ai->ae.current[1] = gi->current[1];
    
    ai->ae.next[0] = gi->next[0];
    ai->ae.next[1] = gi->next[1];
    ai->ae.next[2] = gi->next[2];
    ai->ae.next[3] = gi->next[3];


    AI_Convert(gi->gamearray, ai->ae.aiarray);

    switch (thinklevel)
    {
    case AI_THINK_TURKEYLURKEY:
        ai->ae.threshold = 15; // At least that.
        break;
    default:
        ai->ae.threshold = 15;
    }
    
}

void AI_Feed(AI *ai, int cur1, int cur2, int next1, int next2, int next3, int next4)
{
    ai->ae.current[0] = cur1;
    ai->ae.current[1] = cur2;

    ai->ae.next[0] = next1;
    ai->ae.next[1] = next2;
    ai->ae.next[2] = next3;
    ai->ae.next[3] = next4;
}


void AI_Think(AI *ainf, GAME_INFO *p, int ms)
{
	/*
	 *  Make the AI think for a while, trying to find move.
	 *  ms is the number of milisecond the AI should think (for now),
	 *  the function return as soon as the time has elapsed.
	 *  You can call this function again later to make the AI think more
	 *  deeply (it will restart from where it left).
     *  The AI's superclass each other. That is, if AI_NORMAL does not return
     *  Then AI_EASY get into action.
     *  This is the current behavior for now.
     */

    AI_ENGINE *ai = &ainf->ae;
    int i;
    int score[30];
    int max;
    int index=0;
    int test=0;

    int x, y;
    int type;

    
    

    // Return immidiatly if nothing to do

    if (ainf->donethinking == TRUE)
        return;


    // More new and weird AI!
    // This one will try all possible moves and assign a score to each of them
    // Then it will do the best move among them.
    // Easy enough concept?
    // The score are calculated through very mathematical complex calculus formulas
    // No joke, they are pretty easy. :P

    
    for (i=0;i!=30;i++) // 30 possible moves
    {
        AI_Convert(p->gamearray, ai->aiarray); // convert it! Get back our original array
        score[i] = AI_ScoreMove(ainf, i); // What is the score for move i ?
        
    }
    // We now have 30 scores. We get the max of it.
    
    max = score[0];
    for (i=0;i!=30;i++) 
    {
        if (score[i] > max)
        {
            max = score[i];
            index = i;
        }
    }

    // If we have achieved the wanted threshold value
    // do the move
    // In other case, we will get into planner
    
    if (max >= ai->threshold)
    {
        // Move to do is the one with index
        if (index >= 0 && index <= 6)
        {
            ai->ad.x = index;
            ai->ad.pos = POS_LEFT;
        }
        else if (index >= 7 && index <= 13)
        {
            ai->ad.x = index-6;
            ai->ad.pos = POS_RIGHT;
        }
        else if (index >= 14 && index <= 21)
        {
            ai->ad.x = index-14;
            ai->ad.pos = POS_ABOVE;
        }
        else
        {
            ai->ad.x = index-22;
            ai->ad.pos = POS_BELOW;
        }
         ainf->donethinking = TRUE; 
    }
    {
        // Okay, this AI is activated when the score AI didn't meet a certain
        // threshold level. This AI should try to modify the gameboard in order
        // to create it more toward the plan.

        // This will still use the scoring system to try the move.
        // Except the scoring formula are going to be way different, they will
        // not be related on matches. They will give more points to move that
        // enable the board to go toward the plan.

        switch (ai->thinklevel)
        {
        case AI_THINK_TURKEYLURKEY:
            // Make it play like turkeylurkey
            break;
        default:
            // Normal plan :
            // 1) Plan toward creation of combo.

            // First try to find a reachable nearmatch pattern

            type = FindReachableNearMatch(ainf, &x, &y);
            if (type != -1)
            {
                _mmlog("AI : Reachable NearMatch was found at position (%i, %i) of type %i", x, y, type);
                
                // Now we need to check if we can do a combo preparation.
                // Once/if we set this combo preparation, we flag the pieces as being combo chained.
                // (flag them together). We will also flag the linking chain with a pointer. We repeat
                // the process till it is done with.

                if (ComboPrepare(ainf, type, x, y, &ai->ad)) // we could find a combo preparation move!
                {
                    ainf->donethinking = TRUE;

                    return;
                }
            }

         
         

            break;
        }

    }


    // Just go back into easy mode for now.
	{
		// This should not take long. I will not even look at the time left.
		// I will just try to put the eggs where they have a chance at matching.

		int *line;

		line = (int *)malloc(sizeof(int)*SIZEX);
		GetRelativeLine(0, line, p->gamearray, TRUE);

		// Let's look at our eggs. We will have two cases.
		// 1 : Both eggs are the same color.
		// 2 : Eggs have different color.

		if (MatchColor(p->current[0], p->current[1]))
		{
			// Considered to have the same color. You may place them in any relative position as you see fit.

			int play;

			play = AI_Find1Color(p->current[0], p->current[0], line);
			if (play == AI_BAD)
			{
				// this should NEVER HAPPEN
				// play a dummy move anyway
				ai->ad.pos = POS_LEFT;
				ai->ad.x   = 0;

                ainf->donethinking = TRUE;
			}
			else
			{
				ai->ad.pos = POS_ABOVE;
				ai->ad.x   = play;

                ainf->donethinking = TRUE;
			}
		}
		else
		{
			// Considered to have two different colors. Place them horizontaly only.
			// Or you could place them vertically if you had 3 next to each other in the same color
			// But I guess I will leave all these stuff to the normal mode.
			int play;

			play = AI_Find2Colors(p->current[0], p->current[1], line);  // Try to find something cool to play
			if (play == AI_BAD) 
			{
				// Oops, we are in the shit. Just try to place it somewhere with 1 color then.
				
				play = AI_Find1Color(p->current[0], p->current[1], line);
				if (play < 0) // reverse play
				{
					ai->ad.pos = POS_RIGHT;
					ai->ad.x = -play;

                    ainf->donethinking = TRUE;
				}
				else
				{
					ai->ad.pos = POS_LEFT;
					ai->ad.x = play;

                    ainf->donethinking = TRUE;
				}
			}
			else
			{
				// Yay! We know where to play!
				if (play < 0) // reverse play
				{
					ai->ad.pos = POS_RIGHT;
					ai->ad.x = -play;

                    ainf->donethinking = TRUE;
				}
				else
				{
					ai->ad.pos = POS_LEFT;
					ai->ad.x = play;

                    ainf->donethinking = TRUE;
				}
			}
		}

		free(line);
	}

        

}


void AI_GetDecision(const AI *ainf, AI_DECISION *ad)
{
	/*
	 *  Return immediatly a move to be played. The move is chosen
	 *  According to the difficulty setting
	 */

	ad->pos = ainf->ae.ad.pos;
	ad->x   = ainf->ae.ad.x;
}


int AI_DoMove(AI *ai, GAME_INFO *p)
{
	/*
	 *  Go go go! CPU, you can do it! Tell me what to do, and it will be done.
	 *
	 */

	// First thing to do, turn the eggs in the 'right' side.
	// If this cannot be done, you sure are screwed

    // Redundant code here to make it either turn or move 
    // at random

    if (rand() % 2 == 0)
    {

    	if (ai->ae.ad.pos != p->pos) // make it turn
	    {
		    if (GP_CheckTurn(p, TURN_CLOCKWISE) == TRUE)
    		{
	    		// [...]
		    	return AI_MOVE_TURNCLOCKWISE;
    		}
	    	else
		    {
    			// you sure are screwed

	    		return AI_MOVE_NONE;
		    }
    	}
    
	
	    if (ai->ae.ad.x != p->x) // then we need to make it move horizontally
    	{
	    	if (ai->ae.ad.x > p->x) // move to the right
		    {
    			if (GP_CheckRight(p) == TRUE)
	    		{
		    		return AI_MOVE_RIGHT;
			    }
    			else
	    		{
		    		return AI_MOVE_NONE;
    			}
	    	}
    		else // move to the left
	    	{
		    	if (GP_CheckLeft(p) == TRUE)
			    {
    				return AI_MOVE_LEFT;
	    		}
		    	else
			    {
    				return AI_MOVE_NONE;
	    		}
		    }
	    }
    }
    else
    {
        if (ai->ae.ad.x != p->x) // then we need to make it move horizontally
    	{
	    	if (ai->ae.ad.x > p->x) // move to the right
		    {
    			if (GP_CheckRight(p) == TRUE)
	    		{
		    		return AI_MOVE_RIGHT;
			    }
    			else
	    		{
		    		return AI_MOVE_NONE;
    			}
	    	}
    		else // move to the left
	    	{
		    	if (GP_CheckLeft(p) == TRUE)
			    {
    				return AI_MOVE_LEFT;
	    		}
		    	else
			    {
    				return AI_MOVE_NONE;
	    		}
		    }
	    }

        if (ai->ae.ad.pos != p->pos) // make it turn
	    {
		    if (GP_CheckTurn(p, TURN_CLOCKWISE) == TRUE)
    		{
	    		// [...]
		    	return AI_MOVE_TURNCLOCKWISE;
    		}
	    	else
		    {
    			// you sure are screwed

	    		return AI_MOVE_NONE;
		    }
    	}
    
    }

    // You are at the right place! Act depending!

    if (ai->ae.movelevel == AI_MOVE_PERFECTSLOW)
        return AI_MOVE_NONE;
    else if (ai->ae.movelevel == AI_MOVE_PERFECTFAST)
        return AI_MOVE_FALLFAST;


	return AI_MOVE_NONE;
}

void AI_Cleanup()
{
	/*
	 *  This function should be called right after AI_GetDecision and prior
	 *  to further move in order to cleanup all the memory allocated by those function.
	 */
}

int AI_Find2Colors(int colora, int colorb, int *line)
{
	// Not to be called externally. This is used in the AI_Think section.
	// Return : Absolute value : What position the match is done.
	//          Positive : colora and colorb from left ro right.
	//          Negative : colora and colorb from right to left.

	int x;
    int random[SIZEX];
    int flag[SIZEX];


    // This is a little algoritmn to get an array of n numbers
    // ranging from 0 to n-1 in complete disorder.
    // This is used so that this function doesn't always
    // scan from left to right.

    memset(&flag, 0, sizeof(int)*SIZEX);
    for (x=0;x!=SIZEX;x++)
    {
        int pos;
        int i=0;

        pos = rand() % (SIZEX-x); // get a numbet between 0 and SIZEX-X

        do
        {

            while (flag[i] == 1)
                i++;

            if (pos == 0) // found it
            {
                random[i]=x;
                flag[i] = 1;
                break;
            }
            pos--;
            i++;
        } while (1);
     
    }


	for (x=0;x!=SIZEX-1;x++)
	{
        if (random[x] != SIZEX-1)
        {
		    if (MatchColor(colora, line[random[x]]) && MatchColor(colorb, line[random[x]+1]))
			    return random[x];
        }
	}

	for (x=0;x!=SIZEX-1;x++)
	{
        if (random[x] != SIZEX-1)
        {
		    if (MatchColor(colorb, line[random[x]]) && MatchColor(colora, line[random[x]+1]))
			    return -random[x]-1;
        }
	}

	return AI_BAD;
}

int AI_Find1Color(int colora, int colorb, int *line)
{
	// Same as for finding 2 colors, instead you just try to find one.
	// This should work all the time...
    // I want to check at random too!

	int x;
    int random[SIZEX];
    int flag[SIZEX];

    memset(&flag, 0, sizeof(int)*SIZEX);

    for (x=0;x!=SIZEX;x++)
    {
        int pos;
        int i=0;

        pos = rand() % (SIZEX-x); // get a numbet between 0 and SIZEX-X

        do
        {

            while (flag[i] == 1)
                i++;

            if (pos == 0) // found it
            {
                random[i]=x;
                flag[i] = 1;
                break;
            }
            pos--;
            i++;
        } while (1);
     
    }

	for (x=0;x!=SIZEX-1;x++)
	{
		if (MatchColor(colora, line[random[x]]))
			return random[x];
		if (MatchColor(colorb, line[random[x]]))
			return -random[x]-1;
	}

	return AI_BAD;
}

void AI_Convert(GAMEPIECE *gi, AIARRAY *array)
{
    /*
     *  Convert the gamefield array to a much simplier one
     *  One that will be able to be modified by the AI to do internal calculations
     *
     *
     */

    int i;

    if (!gi)
        return;

    for (i=0;i!=SIZEX*SIZEY;i++)
    {
        array[i] = gi[i].type;
    }
}

BOOL AI_WillMatch(AIARRAY *array, int x, int y, int type)
{
    /*
     *  Would you have a match if you played there?
     *  This is what the function shall return!
     */

    array[gpidx(x, y)] = type;

    if (AI_Match(array, x, y, type))
        return TRUE;

    return FALSE;
}

BOOL AI_Match(AIARRAY *array, int x, int y, int type)
{
    int temparray[SIZEX*SIZEY];

    memset(temparray, 0, SIZEX*SIZEY*sizeof(int));

    if (AI_MatchRecurse(array, x, y, type, temparray) >= 4)
        return TRUE;
    else
        return FALSE;
}

BOOL AI_InternalPlay(AI *ai, int x, int pos, int *dx, int *dy, int *dx2, int *dy2)
{
    /*
     *  Do as if you played there.
     *  Change the array according
     *  Put gravity too and all that stuff
     *  God this feel redundant from the gameplay code
     *  I will create an undo buffer too so that we can return
     *  fast enough to the old array
     *  Or perhaps this is more trouble.
     *  Actually it is, to get the old array back you need to copy SIZEX*SIZEY * sizeof(int)
     *  bytes, which is really not a lot, especially if it is optimized. Doing all this under
     *  buffer crap would take longer I believe.
     */

    int x2;
    int y2=0;
    int y=0;

    switch (pos)
    {
    case POS_LEFT:
        x2 = x+1;
        break;
    case POS_RIGHT:
        x2 = x-1;
        break;
    case POS_ABOVE:
        x2=x;
        y2=1;
        break;
    case POS_BELOW:
        y = 1;
        y2 = 0;
        x2 = x;
        break;
    }

    // Now we need to check if that position is possible
    if (x2 >= SIZEX) return FALSE; // do nothing then
    if (x2 < 0)      return FALSE;

    // We are not able to play either if there is already something there

    if (ai->ae.aiarray[gpidx(x, y)] != GP_EMPTY)
        return FALSE;

    if (ai->ae.aiarray[gpidx(x2, y2)] != GP_EMPTY)
        return FALSE;

    // Still alive? Eh.. Then put each of them down independently and fix them
    // into the internal field

    if (pos == POS_LEFT || pos == POS_RIGHT)
    {

        while (ai->ae.aiarray[gpidx(x, y+1)] == GP_EMPTY && y+1 != SIZEY)
            y++;

        // Fix the first one!

        ai->ae.aiarray[gpidx(x, y)] = ai->ae.current[0]; // fix that one


        while (ai->ae.aiarray[gpidx(x2, y2+1)] == GP_EMPTY && y2+1 != SIZEY)
            y2++;

        ai->ae.aiarray[gpidx(x2, y2)] = ai->ae.current[1]; // fix that one
    }
    else
    {
        // It's more or so the same, get the lowest one, apply gravity to it, and add the other
        // just to its top

        if (y2 > y) // y2 is lower
        {
            while (ai->ae.aiarray[gpidx(x, y2+1)] == GP_EMPTY && y2+1 != SIZEY)
                y2++;

            // Fix both of them!
            ai->ae.aiarray[gpidx(x, y2)] = ai->ae.current[1];
            ai->ae.aiarray[gpidx(x, y2-1)] = ai->ae.current[0];

        }
        else
        {
             while (ai->ae.aiarray[gpidx(x, y+1)] == GP_EMPTY && y+1 != SIZEY)
                y++;

            // Fix both of them!
            ai->ae.aiarray[gpidx(x, y)] = ai->ae.current[0];
            ai->ae.aiarray[gpidx(x, y-1)] = ai->ae.current[1];

        }

    }

    // We are done, get out!

    *dx = x;
    *dy = y;
    *dx2 = x2;
    *dy2 = y2;

    return TRUE;

}

int AI_FindMatch(AIARRAY *array, int *global)
{
    /*
     *  Find matches! Flag them!
     */

    int x, y;
    int temp[SIZEX*SIZEY];
    int count=0;
    int tempcount;
    memset(temp, 0, SIZEX*SIZEY*sizeof(int));

    for (x=0;x!=SIZEX;x++)
    {
        for (y=0;y!=SIZEY;y++)
        {
            tempcount = AI_MatchRecurse(array, x, y, -1, temp);
            if (tempcount >= 4) // woohoo, add it then
            {
                int t;

                count+= tempcount;
                
                for (t=0;t!=SIZEX*SIZEY;t++)
                {
                    if (temp[t] == 1)
                        global[t] = 1;
                }
            }
            // In any case, reset the temp array
            memset(temp, 0, SIZEX*SIZEY*sizeof(int));
        }
    }

    return count;
}

int AI_MatchRecurse(AIARRAY *array, int x, int y, int type, int *temp)
{
    /*
     *  Do the same thing as we used to do in the gamepiece
     *  But this time we give it a "where to check" value
     *  If type is -1, then we get the type of the current piece
     */

    // For now we do not care about the rect structure

   	int   pos;
    int   count = 1;

	temp[gpidx(x,y)] = 1;

    if (type == -1)
        type = array[gpidx(x, y)];

    if (type == STONE || type == GP_EMPTY)
        return 0; // nothing to do with such crap
    

	if (x > 0) // check left
	{
		pos = y*SIZEX + x-1;
		if (temp[pos] == 0)
		{
            if ((array[pos] == type))
				count += AI_MatchRecurse(array, x-1, y, type, temp);
		}
	}

    if (x != SIZEX-1) // check right
    {
        pos = y*SIZEX + x+1;
        if (temp[pos] == 0)
        {
            if ((array[pos] == type))
                count += AI_MatchRecurse(array, x+1, y, type, temp);
		}
	}

	if (y >0) // check up
	{
		pos = (y-1)*SIZEX + x;
		if (temp[pos] == 0)
		{
            if ((array[pos] == type))
				count += AI_MatchRecurse(array, x, y-1, type, temp);
		}
	}

	if (y != SIZEY-1) // check down
	{
		pos = (y+1)*SIZEX + x;
		if (temp[pos] == 0)
		{
            if ((array[pos] == type))
				count += AI_MatchRecurse(array, x, y+1, type, temp);
		}
	}
    return count;
}

int AI_ScoreMove(AI *ai, int move)
{
    /*
     *  You are playing a move and you want to know if it is any good!
     *  I tell you already, your move sucks probably! :P
     *  0-6 are POS_LEFT
     *  7-13 are POS_RIGHT
     *  14-21 are POS_ABOVE
     *  22->29 are POS_BELOW
     */

    int dx1, dy1, dx2, dy2;
    int *temp;
    int score=0;
    int score2=0;
    int combo=0;
    int hold;
    int realmove;
    int pos;

    temp = (int *)malloc(SIZEX*SIZEY*sizeof(int));
    memset(temp, 0, SIZEX*SIZEY*sizeof(int));

    if (move >= 0 && move <= 6)
    {
        realmove = move;
        pos = POS_LEFT;
    }
    else if (move >= 7 && move <= 13)
    {
        realmove = move-6;
        pos = POS_RIGHT;
    }
    else if (move >= 14 && move <= 21)
    {
        realmove = move-14;
        pos = POS_ABOVE;
    }
    else
    {
        realmove = move-22;
        pos = POS_BELOW;
    }

    if (!AI_InternalPlay(ai, realmove, pos, &dx1, &dy1, &dx2, &dy2))
        return -10; // boo
    // Wow! You could do that move! Get the number of match that it does :
    score = AI_MatchRecurse(ai->ae.aiarray, dx1, dy1, -1, temp);
    if (score <= 3) // this like really sucked
    {
       memset(temp, 0, SIZEX*SIZEY*sizeof(int)); // reset the temp array

       score2 = AI_MatchRecurse(ai->ae.aiarray, dx2, dy2, -1, temp);

        if (score2 <= 3) // none of them returned a match
        {
            // First check if the move would kill you, it kills you when it is a the new piece
            // place

            if (ai->ae.aiarray[gpidx(3, 0)] != GP_EMPTY)
                return -10;
            if (ai->ae.aiarray[gpidx(4, 0)] != GP_EMPTY)
                return -10;

            return __max(score, score2); 

        }
    }

    // Wow, we had a match! Apply internal gravity, check match again, repeat (add multiplier count there)
    score = __max(score, score2); // running score updated

    AI_InternalKill(ai->ae.aiarray, temp);
    AI_InternalWork(ai->ae.aiarray); // kill those stones! apply gravity!
    memset(temp, 0, SIZEX*SIZEY*sizeof(int));

    // I just repeat the whole process many times till we have no more matches
    // in order to get all possible combos.
    while (hold = AI_FindMatch(ai->ae.aiarray, temp) >= 4) // combo!
    {
        combo++;
        score += combo*3*hold; // silly formula I guess
        AI_InternalKill(ai->ae.aiarray, temp);
        AI_InternalWork(ai->ae.aiarray);
        memset(temp, 0, SIZEX*SIZEY*sizeof(int));
    }

    free(temp);

    if (ai->ae.aiarray[gpidx(3, 0)] != GP_EMPTY)
        return -10;
    if (ai->ae.aiarray[gpidx(4, 0)] != GP_EMPTY)
        return -10;


    return score;
}



// =====================================================================================
BOOL MatchColor(int typea, int typeb)
// =====================================================================================
// Returns TRUE if the gamepiece types match. (White with anything, brown with another brown, etc)
{

    if (typea == GP_EMPTY || typeb == GP_EMPTY)     // Comparing against nothing will always return true, this help
        return TRUE;                                // the AI for a few functions.

    if (IsMatchablePair(typea, typeb)) return TRUE;

    return FALSE;
}

void AI_InternalKill(AIARRAY *array, int *flag)
{
    /*
     *  Kill all pieces that were flagged :
     *  Also kill nerby meteors.
     */

    int i;

    for (i=0;i!=SIZEX*SIZEY;i++)
    {
        if (flag[i] == 1)
        {
            array[i] = GP_EMPTY;

            // Check the bound, if we can, check around and if a meteor is found, kill it.

            {
                int x, y;

                x = i%SIZEX;
                y = i/SIZEX;

                if (x > 0) // may check left
                {
                    if (array[gpidx(x-1, y)] == METEOR)
                        array[gpidx(x-1, y)] = GP_EMPTY;
                }

                if (x < SIZEX-1) // may check right
                {
                    if (array[gpidx(x+1, y)] == METEOR)
                        array[gpidx(x+1, y)] = GP_EMPTY;
                }

                if (y > 0) // may check up
                {
                    if (array[gpidx(x, y-1)] == METEOR)
                        array[gpidx(x, y-1)] = GP_EMPTY;
                }

                if (x < SIZEY-1) // may check down
                {
                    if (array[gpidx(x, y+1)] == METEOR)
                        array[gpidx(x, y+1)] = GP_EMPTY;
                }

            }
        }
    }
}

void AI_InternalWork(AIARRAY *array)
{
    /*
     * Change the internal AI array so that it reflects something that makes
     * sense now. (Apply gravity, kill rocks, and so on)
     * This need to be keep in synch with how the gameplay works
     * so that the AI will play on valid move.
     */

    // First thing to do is to kill all stones :
    // This is done by going from the top of each column, counting 5 pieces
    // Then all pieces under them are destroyed
    // Then once this is done, we apply gravity everywhere. That should be it.

    int count=0;
    int x=0;
    int y=0;

    for (x=0;x!=SIZEX;x++) // for each column
    {
        while (count <= 3)
        {
            y=0;
            while (y != SIZEY-1) // while we haven't hit the floor
            {
                if (array[gpidx(x, y)] != GP_EMPTY)
                {
                    count++;
                    y++;
                    break; // we may hit the 4th piece
                }
                y++;
            }

            // We have hit 4 pieces, all stones that we encounter from now on are killed

            while (y != SIZEY-1)
            {
                if (array[gpidx(x, y)] == STONE)
                    array[gpidx(x, y)] = GP_EMPTY;
                y++;
            }
            break; // over with it now
        }
    }

    // Wow, if we are here, then we have probably killed all the stones! This is like... great
    // Now we need to apply gravity everywhere
    // For this we use a "bottom-up" approach. When we see holes, we add them to a "moving counter"
    // which we use to find out the displacement between pieces

    for (x=0;x!=SIZEX;x++)
    {
        for (y=SIZEY-1,count=0;y >= 0; y--)
        {
        
            if (array[gpidx(x, y)] == GP_EMPTY)
                count++;
            else
            {
                if (count != 0) // only something to do then
                {
                    array[gpidx(x, y+count)] = array[gpidx(x,y)];
                    array[gpidx(x, y)] = GP_EMPTY; // set the current one as empty, it will be filled later on
                                                   // by the next itteration, or never if it is so.
                }
            }
        }
    }

    // Wow, did the computer really got out of these awful loops?
}


void AI_Initialization()
{
    /*
     *  Load up a bunch of things that are needed by the AI.
     *  In particular, I have to load a few patterns.
     */

    // Nearmatch 1 :

    nm[0].nCombo = 3;
    
    nm[0].x[0] = 0;
    nm[0].y[0] = 1;
    nm[0].x[1] = 0;
    nm[0].y[1] = 2;

    // Combopreparation 1a)

    nm[0].cp[0].x = 0;
    nm[0].cp[0].y = -2;

    // Combopreparation 1b)

    nm[0].cp[0].x = -1;
    nm[0].cp[0].y = -1;

    // Combopreparation 1c)

    nm[0].cp[0].x = 1;
    nm[0].cp[0].y = -1;


    
    // Nearmatch 2 :

    nm[1].nCombo = 5;

    nm[1].x[0] = 1;
    nm[1].y[0] = 0;
    nm[1].x[1] = 2;
    nm[1].y[1] = 0;

    // Combopreparation 2a)
    nm[1].cp[0].x = -1;
    nm[1].cp[0].y = -1;

    // Combopreparation 2b)
    nm[1].cp[1].x = 0;
    nm[1].cp[1].y = -2;

    // Comborepreation 2c)
    nm[1].cp[2].x = 1;
    nm[1].cp[2].y = -2;

    // Combopreparation 2d)
    nm[1].cp[3].x = 2;
    nm[1].cp[3].y = -2;

    // Comborepreation 2e)
    nm[1].cp[4].x = 3;
    nm[1].cp[4].y = -1;

    // Nearmatch 3 :

    nm[2].nCombo = 3;
    nm[2].x[0] = 1;
    nm[2].y[0] = 0;
    nm[2].x[1] = 0;
    nm[2].y[1] = 1;


    // Combopreparation 3a)
    nm[2].cp[0].x = -1;
    nm[2].cp[0].y = -1;

    // Combopreparation 3b)
    nm[2].cp[1].x = 0;
    nm[2].cp[1].y = -2;

    // Comborepreation 3c)
    nm[2].cp[2].x = 1;
    nm[2].cp[2].y = -2;

    // Combopreparation 3d)
    nm[2].cp[3].x = 2;
    nm[2].cp[3].y = -1;


    // Nearmatch 4 :

    nm[3].nCombo = 3;
    nm[3].x[0] = 1;
    nm[3].y[0] = 0;
    nm[3].x[1] = 1;
    nm[3].y[1] = 1;


    // Combopreparation 4a)
    nm[3].cp[0].x = -1;
    nm[3].cp[0].y = -1;

    // Combopreparation 4b)
    nm[3].cp[1].x = 0;
    nm[3].cp[1].y = -2;

    // Comborepreation 4c)
    nm[3].cp[2].x = 1;
    nm[3].cp[2].y = -2;

    // Combopreparation 4d)
    nm[3].cp[3].x = 2;
    nm[3].cp[3].y = -1;


    // Nearmatch 5 :

    nm[4].nCombo = 3;
    nm[4].x[0] = 1;
    nm[4].y[0] = 0;
    nm[4].x[1] = 1;
    nm[4].y[1] = -1;


    // Combopreparation 5a)
    nm[4].cp[0].x = -1;
    nm[4].cp[0].y = -1;

    // Combopreparation 5b)
    nm[4].cp[1].x = 0;
    nm[4].cp[1].y = -2;

    // Comborepreation 5c)
    nm[4].cp[2].x = 1;
    nm[4].cp[2].y = -3;

    // Combopreparation 5d)
    nm[4].cp[3].x = 2;
    nm[4].cp[3].y = -2;


    // Nearmatch 6 :

    nm[5].nCombo = 3;
    nm[5].x[0] = 0;
    nm[5].y[0] = -1;
    nm[5].x[1] = 1;
    nm[5].y[1] = 0;


    // Combopreparation 5a)
    nm[5].cp[0].x = -1;
    nm[5].cp[0].y = -2;

    // Combopreparation 5b)
    nm[5].cp[1].x = 0;
    nm[5].cp[1].y = -3;

    // Comborepreation 5c)
    nm[5].cp[2].x = 1;
    nm[5].cp[2].y = -2;

    // Combopreparation 5d)
    nm[5].cp[3].x = 2;
    nm[5].cp[3].y = -1;

}

int FindReachableNearMatch(AI *ai, int *x, int *y)
{
    /*
     *  Try to find a reachable near match.
     *  This match must also have the same color as one of our piece type.
     *  It returns the nearmatch type and set x and y to the origin of the 
     *  nearmatch.
     */
    
    int tx, ty;
    int pattern;

    // There are many way to optimize this loop.
    // Currently it goes checking each position and each pattern for all of them.
    // That is pretty bad actually and we have a lot of redundant check.
    // A more appropriate approach might be to go through recursion and create
    // some kind of "array map" that would show where all near match pattern are
    // Then it would be just a matter of detecting which pattern it is and return it.
    // But this will do for now, readability over optimization, once I see it works
    // I will change it to other stuff.

    for (tx=0;tx!=SIZEX;tx++)
    {
        for (ty=0;ty!=SIZEY;ty++)
        {
            if (ai->ae.aiarray[gpidx(tx, ty)] == GP_EMPTY)
                continue;

            if (ai->ae.aiarray[gpidx(tx, ty)] == STONE)
                continue;

            if (ai->ae.aiarray[gpidx(tx, ty)] == METEOR)
                continue;

            // Check if the color is okay with our current falling color

            if (ai->ae.aiarray[gpidx(tx, ty)] != ai->ae.current[0] && ai->ae.aiarray[gpidx(tx, ty)] != ai->ae.current[1])
                continue;

            for (pattern=0;pattern != NUM_NEARMATCH;pattern++)
            {
                // First check if we go out of bound for this pattern

                if (nm[pattern].x[0] + tx < 0)
                    continue;
                if (nm[pattern].x[1] + tx < 0)
                    continue;
                if (nm[pattern].x[0] + tx > SIZEX-1)
                    continue;
                if (nm[pattern].x[1] + tx > SIZEX-1)
                    continue;

                if (nm[pattern].y[0] + ty < 0)
                    continue;
                if (nm[pattern].y[1] + ty < 0)
                    continue;
                if (nm[pattern].y[0] + ty > SIZEY-1)
                    continue;
                if (nm[pattern].y[1] + ty > SIZEY-1)
                    continue;

                // Wow, we are still alive.
                // Check if the other two pieces are of the same type

                if (ai->ae.aiarray[gpidx(tx+nm[pattern].x[0], ty+nm[pattern].y[0])] == ai->ae.aiarray[gpidx(tx+nm[pattern].x[1], ty+nm[pattern].y[1])])
                {
                    // Make sure the current piece IS the same type too
                    if (ai->ae.aiarray[gpidx(tx+nm[pattern].x[0], ty+nm[pattern].y[0])] == ai->ae.aiarray[gpidx(tx, ty)])
                    {
                        // Wow, we found a near match pattern! Can you believe it?
                        // Now we need to tell if it is a reachable one or not.

                        if (CheckReachable(ai->ae.aiarray, tx, ty, pattern))
                        {
                            // Wow, we found one!

                            *x = tx;
                            *y = ty;

                            return pattern;
                        }

                    }
                }

            }
        }
    }

    // Couldn't find anything, so return -1

    return -1;
}

BOOL CheckReachable(AIARRAY *array, int tx, int ty, int pattern)
{
    int t;

    // Check for origin

    if (tx + 1 < SIZEX && array[gpidx(tx+1, ty)] == GP_EMPTY)
        return TRUE;


    if (tx - 1 > -1 && array[gpidx(tx-1, ty)] == GP_EMPTY)
        return TRUE;

    if (ty - 1 > 0 && array[gpidx(tx, ty-1)] == GP_EMPTY)
        return TRUE;

    // Check for the other two.

    for (t=0;t!=2;t++)
    {
        if (tx + 1 + nm[pattern].x[t] < SIZEX && array[gpidx(tx+1 + nm[pattern].x[t], ty + nm[pattern].y[t])] == GP_EMPTY)
            return TRUE;

        if (tx - 1 + nm[pattern].x[t]> -1 && array[gpidx(tx-1+nm[pattern].x[t], ty+nm[pattern].y[t])] == GP_EMPTY)
            return TRUE;

        if (ty - 1 + nm[pattern].y[t] > 0 && array[gpidx(tx+nm[pattern].x[t], ty-1+nm[pattern].y[t])] == GP_EMPTY)
            return TRUE;
    }

    // All else fail?

    return FALSE;
}


BOOL ComboPrepare(AI *ai, int type, int x, int y, AI_DECISION *pos)
{
    /*
     *  It is assumed that there is a near match of type 'type'
     *  at origin (x, y). This function tries to find a valid
     *  combo preparation move for it. If found, it sets
     *  the move in pos and return true, otherwise it returns
     *  false.
     */

    // 1) Must have empty space.
    // 2) Must be reachable.
    // 3) The chain linker must also be reachable.
    // 4) Must not do any match.

    int i;
    int color=ai->ae.aiarray[gpidx(x, y)];

    // for all combo preparation
    for (i=0;i!=nm[type].nCombo;i++)
    {
        if (ai->ae.aiarray[gpidx(x+nm[type].cp[i].x, y+nm[type].cp[i].y)] == GP_EMPTY)
        {
            // The space is empty! Can we fill it?
            // If the space below is empty, then we have to place the type above it.
            if (y+nm[type].cp[i].y+1 < SIZEY && ai->ae.aiarray[gpidx(x+nm[type].cp[i].x, y+nm[type].cp[i].y+1)] == GP_EMPTY)
            {
                // This is only good if the place below that one is not empty
                if (y+nm[type].cp[i].y+2 > SIZEY || ai->ae.aiarray[gpidx(x+nm[type].cp[i].x, y+nm[type].cp[i].y+2)] != GP_EMPTY)
                {
                    // This will not always work, but we may as well try it.
                    if (ai->ae.current[0] == color)
                    {
                        pos->pos = POS_ABOVE;
                        pos->x = x+nm[type].cp[i].x;
                    }
                    else
                    {
                        pos->pos = POS_BELOW;
                        pos->x = x+nm[type].cp[i].x;
                    }
                    _mmlog("Found combo preparation type %i for position (%i, %i)", i, nm[type].cp[i].x+x, nm[type].cp[i].y+y);
                    _mmlog("Using the empty space below");
                    return TRUE;
                }
            }
            else
            {
                // The place is not empty? Then just put the type over it
                if (ai->ae.current[0] == color)
                {
                    pos->pos = POS_BELOW;
                    pos->x = x+nm[type].cp[i].x;

                    _mmlog("Found combo preparation type %i for position (%i, %i)", i, nm[type].cp[i].x+x, nm[type].cp[i].y+y);
                    _mmlog("Placing it directly");

                }
                else
                {
                    pos->pos = POS_ABOVE;
                    pos->x = x+nm[type].cp[i].x;
                }
            }
            
        }
    }

    return FALSE;
}




