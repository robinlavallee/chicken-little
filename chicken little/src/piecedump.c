
//  Egg dumping stuff! Related to combo, counter attack and more!
//  Chicken Little
//  More details here --> 



#include <assert.h>
#include "player.h"


// =====================================================================================
    int DumpCount(DUMP *d)
// =====================================================================================
{
    // Function that return the number of pieces
    // That are going to be dumped (for the DUMP structure).

    int x;
    int count=0;

    for (x=0;x!=NUM_DUMP;x++)
    {
        count += d->nPiece[x];
    }

    return count;
}

// =====================================================================================
    void DumpReset(DUMP *d)
// =====================================================================================
{
    int x;

    for (x=0;x!=NUM_DUMP;x++)
    {
        d->nPiece[x] = 0;
    }

}


// =====================================================================================
    int GetDumpPiece(DUMP *d)
// =====================================================================================
{
    // What are we sending today.
    // For example, if dump structure contains 3 eggs, and 4 stones.
    // Then this function should give 3/7th of chance to send an egg
    // And 4/7th chance to send a stone.

    int x;
    int piece;
        
    int r= rand() % DumpCount(d); // number between 0 and nPiece-1
    int running = d->nPiece[0];



    for (x=0;x!=NUM_DUMP;x++)
    {
        if (r < running) // we hit d[x]... (if x = 0, then we are throwing eggs, if x = 1, then other stuff)
        {
            // In any case, we need to subtract 1 from the count
            d->nPiece[x]--; // take it off
            if (x == DUMP_EGG) // You want to dump an egg, don't you??? 
            {
                GP_GetPiece(NULL, &piece); // Get a piece
                return piece;        // And return it!
            }
            else if (x == DUMP_STONE)
            {
                return STONE;  // Dump a stone to the opponent
            }
            else if (x == DUMP_METEOR)
            {
                return METEOR;  // Dump a stone to the opponent
            }
        }
        else
            running += d->nPiece[x+1]; // perhaps it is the other piece
    }

    assert(0); // you should never come here

    return 0;
}



// =====================================================================================
    void ThrowPiecesFillStruct(GAME_INFO *p, DUMP *d, THROWNPIECES *fall)
// =====================================================================================
{
    /*
     *  Call this function. Put in DUMP the pieces you want to make fall.
     *  The function will fill the THROWNPIECES structure appropriately.
     *  This structure was suposed to contains multiple layer, but now it doesn't
     *  anymore. Hence, THROWNPIECE is just an array right now, but it may become
     *  something else later.
     *
     */

    // Those pieces should be thrown at random
    // We are not going to fill more than one layer at a time.
    // So basically.

    int flag[SIZEX]; // those that have 1's in them cannot be filled
    int x;
    int counter=0;
    int piece;
    int i;
    int nPieces;

    nPieces = DumpCount(d); // Total number of stuff to fire

    for (x=0;x!=SIZEX;x++)
        fall->value[x] = DUMP_NOTHING; // initilize it to 0

    for (x=0;x!=SIZEX;x++)
    {
        if (GP_FindHeight(p->gamearray, x) == 0 || p->gamearray[gpidx(x, 0)].type != GP_EMPTY) // 
            flag[x] = 1;
        else
        {
            flag[x] = 0;
            counter++;
        }
    }

    // We have found where we could throw those silly pieces.
    // Now we need to check if we are filling more than the total
    // we have to dump.

    if (nPieces >= counter) // then we are filling all the holes
    {
        for (x=0;x!=SIZEX;x++)
        {
            if (flag[x] == 1) // cannot send anything there
                continue;

            // Then we need to find out what we are sending there!
            piece = GetDumpPiece(d);       // Return the value of the piece to send
            fall->value[x] = piece;
        }
    }
    else // that means we have more empty space than eggs to throw
    {
        while (nPieces > 0)
        {
            i = rand() % counter+1; // get a number between 1 and counter
            x = -1;
            do
            {
                x++;
                while (flag[x] == 1) // skip it
                    x++;
                i--;
            } while (i > 0);

            // Then x should be pointing to the right place
            piece = GetDumpPiece(d);
            fall->value[x] = piece;
            counter--; // take down the counter
            nPieces--;
            flag[x] = 1; // not set it to skip this one
        }
    }
}


// =====================================================================================
    void Attack(GAME_INFO *p, DUMP *d)
// =====================================================================================
{
    /*
     *  Function that is called when you want to throw the pieces at the player
     *  d is the number of each type to throw
     */

    THROWNPIECES te;
    int x;
    int nPiece;

    nPiece = DumpCount(d);

    // This function also takes care of 'subtracting' the stuff from the DUMP structure
    // So if some stuff remains (if we have filled the layer)
    // It will be trigger again by the STATE_WAITING for the player

    ThrowPiecesFillStruct(p, d, &te); // fill in the structure so that we know
                                         // where in the gameplay to throw the pieces.

    // Notice to self : How do we handle multiple layer of pieces falling?
    // Basically, what we do is send as many pieces as possible. If we still
    // Have pieces left to send, then the STATE in player will still remain
    // As waiting. Other pieces are going to be sent by magic!

    for (x=0;x!=SIZEX;x++)
    {
        if (te.value[x] != DUMP_NOTHING) // something to throw!
        {
            GamePiece_Initialize(p->gamearray, x, 0, te.value[x]);
        }
    }

    // We then return home. 

    return;
    
}



