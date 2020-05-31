/*

 Mikmod Portable System Management Facilities (the MMIO)

  By Jake Stine of Divine Entertainment (1996-2000) and

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 ------------------------------------------------------
 
 module: mmalloc.c  (that's mm - alloc!)
 
 The Mikmod Portable Memory Allocation Layer

 Generally speaking, Mikmod only allocates nice and simple blocks of
 memory.  None-the-less, I allow the user the option to assign their own
 memory manage routines, for whatever reason it may be that they want them
 used.

 Notes:
  - All default allocation procedures can now ensure a fixed byte alignment on
    any allocated memory block.  The alignment is adjustable via the _MM_ALIGN
    compiler define.

*/

#include "mmio.h"
#include <string.h>

#define _MM_ALIGN_OFF

// Default memory allocation alignment setting (8 byte boundry).
#ifndef _MM_ALIGN_OFF
#ifndef _MM_ALIGNMENT
#define _MM_ALIGNMENT   8
#endif

// Default buffer size threshold (2 * alignment, or 16 bytes).
#ifndef _MM_ALLOC_THRESHOLD
#define _MM_ALLOC_THRESHOLD (_MM_ALIGNMENT * 2)
#endif   // _MM_ALLOC_THRESHOLD

// local macro
#define MASK_THINGIE (_MM_ALIGNMENT - 1)

#endif   // _MM_ALIGN_OFF

static void *(*fptr_malloc)(size_t size)                                 = NULL;
static void *(*fptr_calloc)(size_t nitems, size_t size)                  = NULL;
static void *(*fptr_realloc)(void *old_blk, size_t size)                 = NULL;
static void *(*fptr_recalloc)(void *old_blk, size_t nitems, size_t size) = NULL;
static void *(*fptr_free)(void *d)										 = NULL;

static CHAR *msg_fail = "%s > Failure allocating block of size: %d";
static CHAR *msg_set  = "Out of memory!  Please check the logfile for more information.";


// Add the buffer threshold value and the size of the 'pointer storage space'
// onto the buffer size. Then, align the buffer size to the set alignment.

// =====================================================================================
    void *_mm_malloc(size_t size)
// =====================================================================================
{
    void   *d;

    if(fptr_malloc)
    {   d = fptr_malloc(size);
    } else
    {   
        #ifndef _MM_ALIGN_OFF
        uint    t;
        void   *old;
        size = ((size + _MM_ALLOC_THRESHOLD + sizeof(void *)) + MASK_THINGIE) & ~MASK_THINGIE;
        #else
        #ifdef _MM_ALLOC_THRESHOLD
        size += _MM_ALLOC_THRESHOLD;
        #endif
        #endif  // _MM_ALIGN

        d = malloc(size);

        #ifndef _MM_ALIGN_OFF
        old = d;
        t   = ((ulong)(d) + MASK_THINGIE) & ~MASK_THINGIE;
        d   = (void *)((((ulong)(d) + sizeof(void *)) > t) ? (t + _MM_ALLOC_THRESHOLD) : t);
        *((void **)(d)-1) = old;
        #endif  // _MM_ALIGN_OFF
    }

    if(!d)
    {   _mmlog(msg_fail, "_mm_malloc", size);
        _mmerr_set(MMERR_OUT_OF_MEMORY, msg_set);
    }

    return d;
}


// =====================================================================================
    void _mm_a_free(void **d)
// =====================================================================================
{
    if(*d)
    {   if(fptr_free)
            fptr_free(*d);
        else
        {
            #ifndef _MM_ALIGN_OFF
            free((ulong **)(*d)-1);
            #else
            free(*d);
            *d = NULL;
            #endif
        }
    }
}


// =====================================================================================
    void _mm_b_free(void **d, const CHAR *logmsg)
// =====================================================================================
{
	
    if(*d)
    {   if(fptr_free)
            fptr_free(*d);
        else
        {
            if(logmsg) _mmlog(" > mmfree > %s",logmsg);
            #ifndef _MM_ALIGN_OFF
            free((ulong **)(*d)-1);
            #else
            free(*d);
            *d = NULL;
            #endif
        }
    }
}


// =====================================================================================
    void *_mm_calloc(size_t nitems, size_t size)
// =====================================================================================
{
    void   *d;
   
    if(fptr_calloc)
    {   d = fptr_calloc(nitems, size);
    } else
    {   
        #ifndef _MM_ALIGN_OFF
        uint    t;
        void   *old;
        #endif

        // The purpose of calloc : to save others the work of the multiply!
        size = nitems * size;

        #ifndef _MM_ALIGN_OFF
        size = ((size + _MM_ALLOC_THRESHOLD + sizeof(void *)) + MASK_THINGIE) & ~MASK_THINGIE;
        #else
        #ifdef _MM_ALLOC_THRESHOLD
        //size += _MM_ALLOC_THRESHOLD;
        #endif
        #endif  // _MM_ALIGN_OFF

        d = malloc(size);

        if(d)
        {   memset(d, 0, size);
            #ifndef _MM_ALIGN_OFF
            old = d;
            t   = ((ulong)(d) + MASK_THINGIE) & ~MASK_THINGIE;
            d   = (void *)((((ulong)(d) + sizeof(void *)) > t) ? (t + _MM_ALLOC_THRESHOLD) : t);
            *((void **)(d)-1) = old;
            #endif  // _MM_ALIGN_OFF
        }
    }

    if(!d)
    {   _mmlog(msg_fail, "_mm_calloc", size);
        _mmerr_set(MMERR_OUT_OF_MEMORY, msg_set);
    }

    return d;
}


// =====================================================================================
    void *_mm_realloc(void *old_blk, size_t size)
// =====================================================================================
{
    void *d;    

    if(fptr_realloc)
    {   d = fptr_realloc(old_blk, size);
    } else
    {   
        #ifndef _MM_ALIGN_OFF
        uint    t;
        void   *old;
        size = ((size + _MM_ALLOC_THRESHOLD + sizeof(void *)) + MASK_THINGIE) & ~MASK_THINGIE;
        #else
        #ifdef _MM_ALLOC_THRESHOLD
        //size += _MM_ALLOC_THRESHOLD;
        #endif
        #endif  // _MM_ALIGN

        d = realloc(old_blk,size);

        #ifndef _MM_ALIGN_OFF
        old = d;
        t   = ((ulong)(d) + MASK_THINGIE) & ~MASK_THINGIE;
        d   = (void *)((((ulong)(d) + sizeof(void *)) > t) ? (t + _MM_ALLOC_THRESHOLD) : t);
        *((void **)(d)-1) = old;
        #endif  // _MM_ALIGN_OFF
    }

    if(!d)
    {   _mmlog(msg_fail, "_mm_realloc", size);
        _mmerr_set(MMERR_OUT_OF_MEMORY, msg_set);
    }

    return d;
}


// =====================================================================================
    void *_mm_recalloc(void *old_blk, size_t nitems, size_t size)
// =====================================================================================
// My own special reallocator, which works like calloc by allocating memory by both block-
// size and numitems.  However, due to the reason that I do not internally track the size
// of allocations, I cannot clear the new portion like I'd like to.
// Oh well.. Maybe later! ;)
{
    void   *d;
   
    if(fptr_recalloc)
    {   d = fptr_recalloc(old_blk, nitems, size);
    } else
    {   
        #ifndef _MM_ALIGN_OFF
        uint    t;
        void   *old;
        #endif

        // The purpose of calloc : to save others the work of the multiply!
        size = nitems * size;

        #ifndef _MM_ALIGN_OFF
        size = ((size + _MM_ALLOC_THRESHOLD + sizeof(void *)) + MASK_THINGIE) & ~MASK_THINGIE;
        #else
        #ifdef _MM_ALLOC_THRESHOLD
        //size += _MM_ALLOC_THRESHOLD;
        #endif
        #endif  // _MM_ALIGN_OFF

        d = realloc(old_blk, size);

        if(d)
        {   memset(d, 0, size);
            #ifndef _MM_ALIGN_OFF
            old = d;
            t   = ((ulong)(d) + MASK_THINGIE) & ~MASK_THINGIE;
            d   = (void *)((((ulong)(d) + sizeof(void *)) > t) ? (t + _MM_ALLOC_THRESHOLD) : t);
            *((void **)(d)-1) = old;
            #endif  // _MM_ALIGN_OFF
        }
    }

    if(!d)
    {   _mmlog(msg_fail, "_mm_recalloc", size);
        _mmerr_set(MMERR_OUT_OF_MEMORY, msg_set);
    }

    return d;
}
