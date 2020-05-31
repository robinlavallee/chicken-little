/*
 The VDRIVER - A Classic Video Game Engine

  By Jake Stine and Divine Entertainment (1997-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org
  For additional information and updates, see our website:
    http://www.divent.org

 Disclaimer:
  I could put things here that would make me sound like a first-rate california
  prune.  But I simply can't think of them right now.  In other news, I reserve
  the right to say I wrote my code.  All other rights are just abused legal
  tender in a world of red tape.  Have fun!

 ---------------------------------------------------
 Module: i386-mmx/spr32rgb_alpha.c

  Alpha blending sprite blitters for 16-bit video displays.

 CPU Target: MMX (Pentium)

*/

#ifdef __MMX__

#include "vdriver.h"

#ifndef BYTESPP
#define BYTESPP 4
#endif

#include "../prims.h"

#ifndef ALPHA_RANGE
#define ALPHA_RANGE 16l
#endif

#define AMASK       (ALPHA_RANGE - 1l)
#define ALPHA_MASK  (AMASK | (AMASK<<8) | (AMASK<<16))

// ===========================================================================
//  Alpha Blending Routines
//
//  Note that we use the 'slow' method of interpolation instead of the optimized
//  a + ((a-b) * fraction).  We do this because that method requires signed math
//  and would break when applied to our Alpha Mask method.  In the long run, the
//  alpha mask method remains more optimal than using the optimized inteprolation
//  on each iondividual component-- not only does it avoid using ANY non-dword
//  values, but it also uses one less MUL per pixel.  It does, however suffer
//  in accuracy compared to the alternative.


static void SprBlit32_Opaque(const SPRITE *info, int xloc, int yloc)
{
    UCAST      *ttmp,*btmp;
    uint        itemp;
    VD_SURFACE *vs = info->vs;
         
    btmp  = (UCAST *)info->bitmap;
    itemp = info->ysize;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   uint  t;
        for(t=info->physwidth; t; t--)
        {   *ttmp = (((*btmp & ALPHA_MASK) / ALPHA_RANGE) * (ALPHA_RANGE-info->alpha))
                  + (((*ttmp & ALPHA_MASK) / ALPHA_RANGE) * info->alpha);
        }

        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}

static void SprBlit32_Trans(const SPRITE *info, int xloc, int yloc)
{
    UCAST *ttmp,*btmp;
    uint   ltemp, itemp;
    VD_SURFACE *vs = info->vs;
         
    btmp  = (UCAST *)info->bitmap;
    itemp = info->ysize;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   ltemp = info->xsize;
        do
        {   if(*btmp) *ttmp = (((*btmp & ALPHA_MASK) / ALPHA_RANGE) * (ALPHA_RANGE-info->alpha))
                            + (((*ttmp & ALPHA_MASK) / ALPHA_RANGE) * info->alpha);
            ttmp += 1;
            btmp += 1;
        } while(--ltemp);
        ttmp += info->xinc;
    } while(--itemp);
}       
           

/*SPR_BLITTER SprAlpha_MMX32Opaque =
{
    &SprAlpha_Blit32Trans,
    SprBlit32_Trans,
    SPR_OPAQUE,
    CPU_NONE,
    SPRA_ALPHA,
    
    BYTESPP,
    1,
    0                    
};

SPR_BLITTER SprAlpha_MMX32Trans =
{
    NULL,
    SprBlit32_Trans,
    SPR_TRANSPARENT,
    CPU_NONE,
    SPRA_ALPHA,
    
    BYTESPP,
    1,
    0                    
};*/

#endif
