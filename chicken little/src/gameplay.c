
/*
 *  Chicken Little.
 *  Lawgiver (Robin Lavallée - robin_lavallee@videotron.ca)
 *  Copyright by myself and Divine Entertainment. (2000)
 *  
 *
 *  Gameplay related function.
 *
 */

#include <assert.h>
#include <string.h>

#include "app.h"
#include "gameplay.h"


// ========================
//   Grabbing New Pieces
// ========================


// Here are now odd of getting those pieces. They should all add up to 255.

#define ODD_WHITE     0  
#define ODD_COLOR     203
#define ODD_BROWN     256
#define ODD_STONE     256


// =====================================================================================
    uint GP_GetScoreLocation(GAMEPIECE *array, int *xtotal, int *ytotal)
// =====================================================================================
// Returns the total number of matching pieces found.
// Returns in xtotal/ytotal: the running sum of the locations of all matches in the
// given playfield.
//
// The x and y coordinates are based on the location of the match:  We take the average 
// of all the matched pieces and base the score location on that. Unfortunately, this
// method yields funny results if you make two matches which are rather far apart from
// each other.
{
    uint      y,i;

    for(y=0, i=0; y<SIZEY; y++)
    {   uint   x;
        for(x=0; x<SIZEX; x++)
            if(array[gpidx(x,y)].stateinfo.state == GP_STATE_DESTROY)
            {   *xtotal += x;
                *ytotal += y;
                i++;
            }
    }

    return i;
}


// =====================================================================================
    void GP_GetPiece(GAME_INFO *gi, int *piece)
// =====================================================================================
// Retriece a piece (one egg) using our odd declared in gameplay.h
// The player value is used to know which randomizer to use.
// If you want to get a piece without using it, then set player to 0
{
	int random;

    if(!gi) 
    {   // not player-specific, so use the global random number generator.

        random = rand() % 255 + 1;
        
       	if (random < ODD_WHITE)
    		*piece = WILDCARD;
	    else if (random < ODD_COLOR)
        {   random = rand() % (COLOR_END-COLOR_START+1);
            *piece = COLOR_START+random;
        } else if(random < ODD_BROWN)
        {	random = rand() % (BROWN_END-BROWN_START+1);
            *piece = BROWN_START+random;
    	} else
        {   *piece = STONE;
        }
    } else
    {
        // get a random number from 1 to 255 (8 bits)
        random = drand_getlong(gi->randomizer, 8);

    	if (random < ODD_WHITE)
	    	*piece = WILDCARD;
    	else if (random < ODD_COLOR)
        {   random = drand_getlong(gi->randomizer, 8) % (COLOR_END-COLOR_START+1); // choose among the colors.
            *piece = COLOR_START+random;
        } else if(random < ODD_BROWN)
    	{   random = drand_getlong(gi->randomizer, 8) % (BROWN_END-BROWN_START+1);
            *piece = BROWN_START+random;
        } else
        {   *piece = STONE;
        }
    }
}

// =====================================================================================
    void GP_GetNextPiece(GAME_INFO *gi)
// =====================================================================================
// This function get the next piece, and rotate the next to the current, and
// the future next to the other next.
{
	gi->current[0] = gi->next[0];
	gi->current[1] = gi->next[1];

    memcpy(gi->next, &gi->next[2], (NUMPIECES_AHEAD-1) * 2 * sizeof(int));

	// We now need to make a couple new pieces
	GP_GetPiece(gi, &gi->next[(NUMPIECES_AHEAD-1)*2]);
	GP_GetPiece(gi, &gi->next[((NUMPIECES_AHEAD-1)*2)+1]);

    gi->ypart   = 0;
    gi->x       = 4;
    gi->y       = 0;
    gi->pos     = POS_RIGHT;
    gi->disppos = POS_RIGHT;
}


// =====================================================================================
    void GP_GetInitialPieces(GAME_INFO *gi)
// =====================================================================================
// This function is called at the beginning of a game.
// It gets you the starting 4 pieces (2 sets).
{
    int   i;
    
	gi->current[0] = 0;
    gi->current[1] = 0;

	for(i=0; i<NUMPIECES_AHEAD*2; i++)
        GP_GetPiece(gi, &gi->next[i]);
}


// =====================================================================================
    BOOL IsMatchablePair(int typea, int typeb)
// =====================================================================================
// Checks to see if both pieces given are a suitable matching pair.  This pays no real
// attention to what *kind* of match is made, just so as long as a match is possible.
// If you need to differentiate perfect matches from normal ones, then use IsPerfectMatch
// (below) once you know the match is valid using this function.
// 
{
    if (!IsMatchable(typea) || !IsMatchable(typeb)) return FALSE;

    if (typea == typeb)                             return TRUE;

    if (IsWildcard(typea) || IsWildcard(typeb))     return TRUE;

	if (IsBrown(typea) && IsBrown(typeb))           return TRUE;

    return FALSE;
}

// =====================================================================================
    BOOL IsPerfectMatch(int typea, int typeb)
// =====================================================================================
// Checks to see if both pieces given are a suitable 'perfect match.'  This is only useful
// when you know at least one of the pieces is a brown egg, or if you only want to check
// for exact matches (colored or brown).
//
// Notes:
//  - I do not check if the pieces are matchable, so this will return TRUE for things like
//    stones, which could still be useful (ie, to tell an AI to keep all stones together)
//
//  - This ignores wildcards for now.  I'm not sure I want wildcards to complete perfect
//    brown matches, so.
{
    if (typea == typeb)                             return TRUE;
//    if (IsWildcard(typea) || IsWildcard(typeb))     return TRUE;

    return FALSE;
}


// =====================================================================================
    BOOL GP_IsInert(const GAMEPIECE *array)
// =====================================================================================
// Checks the entire gameboard for any game pieces which are not in one of the following
// states:
//   either 1 : waiting
//   or 2     : empty
//  Addition : To make sure everything is working fine :
//             If a piece has no piece under it, then we return false.
//             If a stone has 4 pieces or more above, we return false.
{
    int  t;

    for(t=0; t<SIZEX*SIZEY; t++)
    {   if(array[t].type != GP_EMPTY)
        {   if(!gp_info[array[t].type].entity->isinert(array[t].stateinfo.state))
                return FALSE;

            // Extra check for sone
            if (array[t].type == STONE)
            {
                // Check its height
                if (GP_FindNumAbove(array, t%SIZEX, t/SIZEX) >= 4) // you should crush soon
                    return FALSE;
            }
            if (t/SIZEX != SIZEY-1) // not last row
            {
                if (array[t+SIZEX].type == GP_EMPTY) // If we have a piece and nothing under it, then not inert
                    return FALSE;
            }
        }
    }

    return TRUE;
}


// =====================================================================================
    BOOL GP_NoGravity(const GAMEPIECE *array)
// =====================================================================================
// Checks the entire gameboard for any game pieces which are falling.  If any piece is
// is in the state of GP_STATE_GRAVITY, then we return false.
{
    int  t;

    for(t=0; t<SIZEX*SIZEY; t++)
    {   if(array[t].type != GP_EMPTY)
        {   if(array[t].stateinfo.state == GP_STATE_GRAVITY)
                return FALSE;
            if (t/SIZEX != SIZEY-1) // not last row
            {
                if (array[t+SIZEX].type == GP_EMPTY) // look down... if empty... HAHA
                    return FALSE;
            }
        }
    }

    return TRUE;
}


// =====================================================================================
    static int MatchRecurse(int type, int sizex, int sizey, int x, int y, int *temparray, GAMEPIECE *array)
// =====================================================================================
// Uses a recursive logic pattern to search all neighboring entities for matches to the
// the given entity type.  Returns the number of matching eggs found.
// ** Note to self: We have to search up and to the left because in a game like this it is
// possible to have an L-shaped match which would only be properly found when searching
// either left and/or up.
{
	int   pos;
    int   count = 1;

	temparray[gpidx(x,y)] = 1;

	if (x > 0) // check left
	{
		pos = y*sizex + x-1;
		if (temparray[pos] == 0)
		{
            if ((array[pos].type == type) || (gp_info[array[pos].type].matchtype == GP_WILDCARD))
				count += MatchRecurse(type, sizex, sizey, x-1, y, temparray, array);
        }
	}

    if (x != sizex-1) // check right
    {
        pos = y*sizex + x+1;
        if (temparray[pos] == 0)
        {
            if ((array[pos].type == type) || (gp_info[array[pos].type].matchtype == GP_WILDCARD))
                count += MatchRecurse(type, sizex, sizey, x+1, y, temparray, array);
		}
	}

	if (y >0) // check up
	{
		pos = (y-1)*sizex + x;
		if (temparray[pos] == 0)
		{
            if ((array[pos].type == type) || (gp_info[array[pos].type].matchtype == GP_WILDCARD))
				count += MatchRecurse(type, sizex, sizey, x, y-1, temparray, array);
		}
	}

	if (y != sizey-1) // check down
	{
		pos = (y+1)*sizex + x;
		if (temparray[pos] == 0)
		{
            if ((array[pos].type == type) || (gp_info[array[pos].type].matchtype == GP_WILDCARD))
				count += MatchRecurse(type, sizex, sizey, x, y+1, temparray, array);
		}
	}
    return count;
}

// =====================================================================================
    static int BrownMatchRecurse(int sizex, int sizey, int x, int y, int *temparray, GAMEPIECE *array)
// =====================================================================================
// See MatchRecurse above!
{
    int  pos;
    int  count = 1;

	temparray[y*sizex+x] = 1;

	if (x > 0) // check left
	{
		pos = y*sizex + x-1;
		if (temparray[pos] == 0)
		{
			if (IsBrown(array[pos].type) || (gp_info[array[pos].type].matchtype == GP_WILDCARD))
				count += BrownMatchRecurse(sizex, sizey, x-1, y, temparray, array);
		}
	}

	if (x != sizex-1) // check right
	{
		pos=y*sizex+x+1;
		if (temparray[pos] == 0)
		{
			if (IsBrown(array[pos].type) || (gp_info[array[pos].type].matchtype == GP_WILDCARD))
				count += BrownMatchRecurse(sizex, sizey, x+1, y, temparray, array);
		}
	}

	if (y >0) // check up
	{
		pos = (y-1)*sizex + x;
		if (temparray[pos] == 0)
		{
			if (IsBrown(array[pos].type) || (gp_info[array[pos].type].matchtype == GP_WILDCARD))
				count += BrownMatchRecurse(sizex, sizey, x, y-1, temparray, array);
		}
	}

	if (y != sizey-1) // check down
	{
		pos=(y+1)*sizex+x;
		if (temparray[pos] == 0)
		{
			if (IsBrown(array[pos].type) || (gp_info[array[pos].type].matchtype == GP_WILDCARD))
				count += BrownMatchRecurse(sizex, sizey, x, y+1, temparray, array);
		}
	}
    return count;
}

int   gp_perfect;

// =====================================================================================
    int GP_FindMatch(GAMEPIECE *array, BOOL dest)
// =====================================================================================
// Result will be the number of eggs cleared from the board (used by the caller to calc-
// ulate the eggdumps).
//
// Two passes are done, in this order:
// 1 : Check for any perfect match (this includes all matchable egg types).
// 2 : Check for texture eggs match.
{
    BOOL  *temp, *global;
    int    y;
    int    count = 0;

    temp = (int *)calloc(SIZEX*SIZEY, sizeof(BOOL));
    global = (int *)calloc(SIZEX*SIZEY, sizeof(BOOL));

	// We will find patterns through a recursive way. Watch the brain power at work. :)

    // ----------------------------------------
    //   FIRST PASS : Check for perfect match
    // ----------------------------------------

    gp_perfect = 0;

    for (y=0; y<SIZEY; y++)
	{   int   x;
        for (x=0; x<SIZEX; x++)
	    {   
            if (global[gpidx(x, y)] == 1)
                continue;

            if (IsMatchable(array[gpidx(x,y)].type))
            {   int  t;

                t = MatchRecurse(array[gpidx(x,y)].type, SIZEX, SIZEY, x, y, temp, array);
                if(t > 3)
                {   
                    int        o;
                    int tempx, tempy;

                    

                    GAMEPIECE *gp = array;

                    // We have a valid match, so change the egg states to crack and be destroyed.

    		        if(IsBrown(array[gpidx(x,y)].type))
                        gp_perfect += t;
                
                    for(o=0,tempx=0,tempy=0; o<SIZEX*SIZEY; o++, gp++,tempx++)
                    {
                        // Make sure the temp are fine
                        if (tempx == SIZEX)
                        {
                            tempx=0;
                            tempy++;
                        }

                        if(temp[o]) 
                        {
                            if (dest)
                            {   
                                gpState_SetState(gp,GP_STATE_DESTROY);

                                if(tempx <SIZEX-1) if(gp_info[array[gpidx(tempx+1,tempy)].type].matchtype == GP_UNMATCHABLE) gpState_SetState(&array[gpidx(tempx+1,tempy)],GP_STATE_DESTROY);
                                if(tempx>0)       if(gp_info[array[gpidx(tempx-1,tempy)].type].matchtype == GP_UNMATCHABLE) gpState_SetState(&array[gpidx(tempx-1,tempy)],GP_STATE_DESTROY);
                                if(tempy<SIZEY-1) if(gp_info[array[gpidx(tempx,tempy+1)].type].matchtype == GP_UNMATCHABLE) gpState_SetState(&array[gpidx(tempx,tempy+1)],GP_STATE_DESTROY);
                                if(tempy>0)       if(gp_info[array[gpidx(tempx,tempy-1)].type].matchtype == GP_UNMATCHABLE) gpState_SetState(&array[gpidx(tempx,tempy-1)],GP_STATE_DESTROY);

                            }

                            global[o] = 1; // add to global
                        }
                    }

                    count += t;

                }
                memset(temp, 0, SIZEX*SIZEY*sizeof(BOOL));
            }
	    }
    }

    memset(temp, 0, SIZEX*SIZEY*sizeof(BOOL));

    // -------------------------------------------------------------------------
    //   BEGIN OF SECOND PASS - Match all brown eggs, reguardless of sub-color
    // -------------------------------------------------------------------------

    for (y=0; y<SIZEY; y++)
	{   int   x;
        for (x=0; x<SIZEX; x++)
	    {
            if (global[gpidx(x, y)] == 1)
                continue;

            if (IsBrown(array[gpidx(x,y)].type))
            {   
                int   t;
                t = BrownMatchRecurse(SIZEX, SIZEY, x, y, temp, array);
                
                if(t > 3)
                {   
                    int        o;
                    int tempx, tempy;

                    GAMEPIECE *gp = array;

                    // We have a valid match, so change the egg states to crack and be destroyed.

    		        if(IsBrown(array[gpidx(x,y)].type))
                        gp_perfect += t;
                
                    for(o=0,tempx=0,tempy=0; o<SIZEX*SIZEY; o++, gp++,tempx++)
                    {
                        // Make sure the temp are fine
                        if (tempx == SIZEX)
                        {
                            tempx=0;
                            tempy++;
                        }

                        if(temp[o]) 
                        {
                            if (dest)
                            {   
                                gpState_SetState(gp,GP_STATE_DESTROY);

                                if(tempx <SIZEX-1) if(gp_info[array[gpidx(tempx+1,tempy)].type].matchtype == GP_UNMATCHABLE) gpState_SetState(&array[gpidx(tempx+1,tempy)],GP_STATE_DESTROY);
                                if(tempx>0)       if(gp_info[array[gpidx(tempx-1,tempy)].type].matchtype == GP_UNMATCHABLE) gpState_SetState(&array[gpidx(tempx-1,tempy)],GP_STATE_DESTROY);
                                if(tempy<SIZEY-1) if(gp_info[array[gpidx(tempx,tempy+1)].type].matchtype == GP_UNMATCHABLE) gpState_SetState(&array[gpidx(tempx,tempy+1)],GP_STATE_DESTROY);
                                if(tempy>0)       if(gp_info[array[gpidx(tempx,tempy-1)].type].matchtype == GP_UNMATCHABLE) gpState_SetState(&array[gpidx(tempx,tempy-1)],GP_STATE_DESTROY);

                            }

                            global[o] = 1;
                        }
                    }

                    count += t;
                }
                memset(temp, 0, SIZEX*SIZEY*sizeof(BOOL));
            }
	    }
    }

	free(temp);
    free(global);
    
    return count;

}


void GetRelativeLine(int deep, int *line, GAMEPIECE *array, BOOL sStone)
{
    /*
	 *  This get you the eggs that are all at the 'top' of each column.
	 *  deep just tell you how deep you should go. 
	 *  This is used by the AI mainly.
     *  cStone is a boolean flag, is set to TRUE, it will skip stone
     *  and look beyond them.
	 */ 

	int x, y;

	for (x=0;x!=SIZEX;x++)
	{
		for (y=0;y<SIZEY-deep;y++)
		{
			if (array[y*SIZEX+x].type != GP_EMPTY)
			{
                if (array[gpidx(x, y)].type == STONE && sStone) // need to skip it then
                    continue; 
                                
                line[x] = array[(y+deep)*SIZEX+x].type;
				break;
			}
			// If we get here, then there was NOTHING at all on that column.
			line[x] = GP_EMPTY;
		}
	}

	return;
}

// ===========================================================================
// ** Notes about the GP_Checkxxxx Functions **
// it is usually faster to check if the egg *can* move to an adacent space
// than it is to check if it CANNOT (since it will be able to more often than
// not). So we check if it can move, and if not, it will execute the return
// FALSE; at the bottom of each function.
// ===========================================================================

// ===========================================================================
    BOOL GP_CheckDown(const GAME_INFO *p)
// ===========================================================================
// This function check if the piece can be moved down. It returns false if it
// cannot be moved down.  It returns TRUE otherwise.
{
    int   x[2]; // coordinate of the 2 eggs.
    int   y[2];

    x[0] = p->x;
	y[0] = p->y;
        
    switch(p->pos)
    {   case POS_LEFT:
	        x[1] = p->x+1;
		    y[1] = p->y;
        break;

        case POS_RIGHT:
		    x[1] = p->x-1;
		    y[1] = p->y;
        break;

	    case POS_ABOVE:
    		x[1] = p->x;
	    	y[1] = p->y+1;
        break;

	    case POS_BELOW:
		    x[1] = p->x;
		    y[1] = p->y-1;
        break;
    }

	// Now that we have the coordinate, we can do a double check down.

	// First, check if we are at the floor of the screen

	if (y[0] == SIZEY-1 || y[1] == SIZEY-1) return FALSE;

	// If not, check if we have eggs under.

	if( (p->gamearray[gpidx(x[0],y[0]+1)].type == GP_EMPTY) && (p->gamearray[gpidx(x[1],y[1]+1)].type == GP_EMPTY))
        return TRUE;
    else
        return FALSE;
}


// ===========================================================================
    BOOL GP_CheckLeft(const GAME_INFO *p)
// ===========================================================================
// Return true if we can move the egg to the left.
// Return false otherwise
// Two checks are done :
// First for the game play area
// Second if an egg is there.
{    
    switch(p->pos)
    {   case POS_LEFT:      // then just check on the left
    		if ((p->x) && (p->gamearray[gpidx(p->x-1,p->y)].type == GP_EMPTY)) return TRUE;
        break;

        case POS_RIGHT: // just check on the left, minus 1
            if ((p->x > 1) && (p->gamearray[gpidx(p->x-2,p->y)].type == GP_EMPTY)) return TRUE;
        break;

        case POS_ABOVE:
            if ((p->x) && (p->gamearray[gpidx(p->x-1,p->y)].type   == GP_EMPTY) 
		                && (p->gamearray[gpidx(p->x-1,p->y+1)].type == GP_EMPTY)) return TRUE;
        break;

        case POS_BELOW:
            if ((p->x) && (p->gamearray[gpidx(p->x-1,p->y)].type   == GP_EMPTY) 
		                && (p->gamearray[gpidx(p->x-1,p->y-1)].type == GP_EMPTY)) return TRUE;
        break;
    }

    return FALSE;
}

// ===========================================================================
    BOOL GP_CheckRight(const GAME_INFO *p)
// ===========================================================================
{
    switch(p->pos)
    {   case POS_LEFT:
    		if ((p->x < SIZEX-2) && (p->gamearray[gpidx(p->x+2,p->y)].type == GP_EMPTY)) return TRUE;
        break;

        case POS_RIGHT:
            if ((p->x < SIZEX-1) && (p->gamearray[gpidx(p->x+1,p->y)].type == GP_EMPTY)) return TRUE;
        break;

        case POS_ABOVE:
            if ((p->x < SIZEX-1) && (p->gamearray[gpidx(p->x+1,p->y)].type   == GP_EMPTY) 
		                          && (p->gamearray[gpidx(p->x+1,p->y+1)].type == GP_EMPTY)) return TRUE;
        break;

        case POS_BELOW:
            if ((p->x < SIZEX-1) && (p->gamearray[gpidx(p->x+1,p->y)].type   == GP_EMPTY) 
		                          && (p->gamearray[gpidx(p->x+1,p->y-1)].type == GP_EMPTY)) return TRUE;
        break;
    }

    return FALSE;
}

// ===========================================================================
    BOOL GP_CheckTurn(const GAME_INFO *p, int direction)
// ===========================================================================
// Since the main 'egg' never turns, this one is rather easy to code. Get the
// new 'x' and 'y' of the second egg, and check if it collides with something.
// Added Logic: If it collides with a wall, then we move the base piece to
// allow the rotation to proceed.
{

	int x, y;
	int pos;

	pos = p->pos;

	if (direction == TURN_CLOCKWISE)
        pos++;
    else
        pos--;

    pos &= 3;         // wraps the value (-1 becomes 3, 4 becomes 0).

	// Now that we have our new position, let's get its x and y.

	switch(pos)
    {   case POS_LEFT:
    		x = p->x+1;
	    	y = p->y;
	    break;

        case POS_RIGHT:
    		x = p->x-1;
	    	y = p->y;
	    break;

        case POS_ABOVE:
    		x = p->x;
	    	y = p->y+1;
        break;

		case POS_BELOW:
    	    x = p->x;
		    y = p->y-1;
        break;
	}

    // Don't check for top of playfield.  Display handles that.
    if (y > -1)
    {
        // Check first if we are still in the play field:
    
        if (x == -1 || x == SIZEX) return FALSE;
        if (y == SIZEY)            return FALSE;

	    // Now check in the array if something is there

        if (p->gamearray[gpidx(x,y)].type != GP_EMPTY) return FALSE;
    }

	return TRUE;
}


// =====================================================================================
    void GP_ResetPlayField(GAMEPIECE *gamearray)
// =====================================================================================
// Blank out the array of the playfield, cleaning it up.
// Note tht we don't call GamePiece_Init here, because we are slick and know how to 
// do it properly without that.
{
    int i;

	for (i=0; i!=SIZEY*SIZEX; i++)
	{   gamearray[i].type             = GP_EMPTY;
        gamearray[i].stateinfo.state  = STATE_NONE;
	}
}


// =====================================================================================
    void GP_Solidify(GAME_INFO *p)
// =====================================================================================
// Make the current 'falling piece' unable to move anymore, it becomes part of the 
// playfield.
{
	// Do it for the first egg first.

	GamePiece_Initialize(p->gamearray, p->x, p->y, p->current[0]);

	// Then for the second, find its position first

	switch(p->pos)
    {   case POS_LEFT:
             GamePiece_Initialize(p->gamearray, p->x+1, p->y, p->current[1]);
        break;

        case POS_RIGHT:
            GamePiece_Initialize(p->gamearray, p->x-1, p->y, p->current[1]);
        break;

        case POS_ABOVE:
            GamePiece_Initialize(p->gamearray, p->x, p->y+1, p->current[1]);
        break;

        case POS_BELOW:
    		if (p->y == 0)
	    	{
                // If this happens player dies?
                // do something...
                // [...]
            } else
                GamePiece_Initialize(p->gamearray, p->x, p->y-1, p->current[1]);
        break;
	}
}


// =====================================================================================
    void GP_MoveTurn(GAME_INFO *p, int direction)
// =====================================================================================
{
	if (direction == TURN_CLOCKWISE)
        p->pos++;
    else
        p->pos--;

    p->pos &= 3;         // wraps the value (-1 becomes 3, 4 becomes 0).
}


void GP_FindOneMinimumHeight(GAME_INFO *p, int *x, int *y)
{
    /*
     *  Find the lowest y. If there is many of them, choose
     *  an x at random for the lowest one.
     */

    char  flag[SIZEX];
    int   i;
    int   min=0;
    int   count=0;
    int   random;

    for (i=0;i!=SIZEX;i++)
        flag[i] = 0; // flag them as not minimum

    // For each row we find its minimum, if it's lower than the previous minimum
    // We unflag everything. If it's the same, then we add to the flag.

    for (i=0;i!=SIZEX;i++) // go through all row
    {
        int y;
        y = GP_FindHeight(p->gamearray, i); // give me the height of that one

        if (y > min)   // keep in mind that the y axis is reversed
        {
            int z;

            min=y; // set the new minimum

            
            for (z=i-1;z>=0;z--) // reset all the flag
                flag[z] = 0; 
            flag[i] = 1; // set the new flag
        }
        else if (y == min) // same, add to flag
            flag[i] = 1;
    }

    // We know what y is now.

    if (min == 0)  // the whole playfield is filled!
    {
        *x = -1;
        *y = -1; 
        return;
    }

    // Now we need to know what 'x' is. We may have more than one.

    *y = min;

    for (i=0;i!=SIZEX;i++)
    {
        if (flag[i] == 1)
            count++;
    }

    // Now get a random number, modulo by count, then get the x that correspond to that one

    random = rand() % count;

    for (i=0;i!=SIZEX;i++)
    {
        if (flag[i] == 1)
            random--;

        if (random == 0) // found it!
        {
            *x = i;
            return;
        }
    }

    // you should never get here

    assert(0);

    return;
}

int GP_FindHeight(const GAMEPIECE *array, int x)
{
    /*
     *  Return the height. 0 = top, SIZEY = bottom
     */

    int height=SIZEY-1;

    if (array[gpidx(x,height)].type == GP_EMPTY)
        return SIZEY;

    while (array[gpidx(x,height)].type != GP_EMPTY)
    {
        height--;
        if (height == -1)
            return 0;
    }

    return height+1;
}

int GP_FindNumAbove(const GAMEPIECE *array, int x, int y)
{
    // How many pieces are above us?
    // This function shall tell you!

    return y - GP_FindHeight(array, x); // Just think about it, this formula makes sense. :P
}

BOOL GP_IsDead(const GAME_INFO *p)
{
    // Are you dead? Should you die? Perhaps so!
    // This check if the current piece is at the same
    // place of an existing one (ie, it appeared on it,
    // when a new piece was fetched --> you die).

    int x2, y2;

    switch (p->pos)
    {
        case POS_LEFT:
            x2 = p->x + 1;
            y2 = p->y;
        break;

        case POS_RIGHT:
            x2 = p->x - 1;
            y2 = p->y;
        break;

        case POS_ABOVE:
            x2 = p->x;
            y2 = p->y+1;
        break;

        case POS_BELOW:
            x2 = p->x;
            y2 = p->y - 1;
        break;
    }

    if (p->gamearray[gpidx(p->x, p->y)].type != GP_EMPTY) // tsk tsk tsk
        return TRUE;
    else if (p->gamearray[gpidx(x2, y2)].type != GP_EMPTY) // tsk tsk tsk
        return TRUE;
    else
        return FALSE;
}





enum
{
    GI_STATE_ROTATING = ENTITY_NUMSTATES
};

// =====================================================================================
    static int GameInfo_StateHandler(GAME_INFO *gi)
// =====================================================================================
{

    switch(gi->entity.state)
    {
        case ENTITY_STATE_WAITING:
            if(gi->pos != gi->disppos)
            {   
                gi->index = 0;
                Entity_SetState(&gi->entity, GI_STATE_ROTATING);
                return STATE_PRIORITY_HIGH;
            }
        break;
        
        case GI_STATE_ROTATING:

            // we have to go from point 'disppos' to point 'pos'

            gi->index++;
            gi->index &= 7;
            
            if(!gi->index)
            {   
                gi->disppos++;
                gi->disppos &= 3;

                if(gi->pos == gi->disppos)
                {
                    // wow, we've made it.
                    
                    Entity_SetState(&gi->entity, ENTITY_STATE_WAITING);
                }
            }
            App_Paint();
        
        return 18;

    }

    return STATE_PRIORITY_LOW;
}

static ENTITY Entity_GameInfo = 
{
    "Gameinfo",
    ENTITY_STATE_WAITING,
    0,
    NULL,
    GameInfo_StateHandler
};

// =====================================================================================
    GAME_INFO *GamePlay_Initialize(ENTITY **entlist, uint time)
// =====================================================================================
// Spawns a gameplay entity which must be attached to a reguarly-updated entity list.
// This creates the randomizer and initializes variables which gameplay.c code expects
// and uses.
{
    GAME_INFO *retval;

    retval = (GAME_INFO *)Entity_Spawn(entlist, &Entity_GameInfo, sizeof(GAME_INFO));

    retval->randomizer = drand_create(time);

    retval->timeout  = 0;
    retval->pos      = POS_LEFT;
    retval->disppos  = POS_LEFT;

    GP_ResetPlayField(retval->gamearray);

    return retval;
}

