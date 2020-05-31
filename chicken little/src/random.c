/* Divine Entertainment Presents -->

  == The Random Bit
  == A comp-sci standard in decent random number generation

  This is a replacement random number generator for the woefully not-so-random
  Visual C random number generator.  I have tested this bitch and it *is*
  quite effectively random.  And fast.  Good stuff!

  For added coolness:
   I have encapsulated the generation code into a mini-object which makes
   this thing nice and easy to use for generating full integers.

  Notes:
   - It seems the random number sequence in the beginning (after a new seed has
     been assigned) isn't very random.  As a fix, I now automatically call the
     getbit function 30 times.  That seems to get us past that area of crappy
     generations.

*/

#include "random.h"
#include <stdlib.h>

#define INIT_BITS   29    // See notes above.

#define IB1        1
#define IB2        2
#define IB5       16
#define IB18  131072
#define MASK  (IB1+IB2+IB5)

static ulong globseed;

// ===========================================================================
    BOOL __inline rand_getbit(void)
// ===========================================================================
// generates a random bit!
{
    if(globseed & IB18)
    {   globseed = ((globseed - MASK) << 1) | IB1;
        return 1;
    } else
    {   globseed <<= 1;
        return 0;
    }
}


// ===========================================================================
    void rand_setseed(ulong seed)
// ===========================================================================
{
    int  i;
    
    globseed = seed;
    for(i=0; i<INIT_BITS; i++) rand_getbit();
}

// ===========================================================================
long rand_getlong(uint bits)
// ===========================================================================
// Generates an integer with the specified number of bits.
{
    ulong retval = 0;
    uint  i;
    
    for(i=0; i<bits; i++)
        retval |= rand_getbit() << i;

    return (long)retval;
}


// ===========================================================================
BOOL __inline getbit(ulong *iseed)
// ===========================================================================
// generates a random bit!  Used to generate the noise sample.
{
    if(*iseed & IB18)
    {   *iseed = ((*iseed - MASK) << 1) | IB1;
        return 1;
    } else
    {   *iseed <<= 1;
        return 0;
    }
}

// ===========================================================================
DE_RAND *drand_create(ulong seed)
// ===========================================================================
{
    DE_RAND  *newer_than_you;

    newer_than_you = (DE_RAND *)malloc(sizeof(DE_RAND));

    drand_setseed(newer_than_you, seed);

    return newer_than_you;
}


// ===========================================================================
void drand_setseed(DE_RAND *Sette, ulong seed)
// ===========================================================================
{
    if(Sette)
    {   int       i;

        Sette->seed  = seed;
        Sette->iseed = seed;

        for(i=0; i<INIT_BITS; i++) getbit(&Sette->iseed);
    }
}


// ===========================================================================
long drand_getlong(DE_RAND *hand, uint bits)
// ===========================================================================
// Generates an integer with the specified number of bits.
{
    ulong retval = 0;
    uint  i;
    
    for(i=0; i<bits; i++)
        retval |= getbit(&hand->iseed) << i;

    return retval;
}

// ===========================================================================
void drand_free(DE_RAND *freeme)
// ===========================================================================
{
    if(freeme) free(freeme);
}


// ===========================================================================
void drand_reset(DE_RAND *resetme)
// ===========================================================================
{
    resetme->iseed = resetme->seed;
}
