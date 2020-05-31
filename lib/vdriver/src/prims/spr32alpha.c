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
 Module: spr32alpha.c

 Alpha blending sprite blitters for 32-bit video displays.

 Note that we use the 'slow' method of interpolation instead of the optimized
 a + ((a-b) * fraction).  We do this because that method requires signed math
 and would break when applied to our Alpha Mask method.  In the long run, the
 alpha mask method remains more optimal than using the optimized inteprolation
 on each iondividual component-- not only does it avoid using ANY non-dword
 values, but it also uses one less MUL per pixel.  It does, however suffer
 in accuracy compared to the alternative.


 CPU Target: None (Pentium)

*/

#include "vdriver.h"
#include <string.h>

#ifndef BYTESPP
#define BYTESPP 4
#endif

#include "prims.h"

// =====================================================================================
    static void __inline alphabland(VD_SURFACE *vs, UCAST *btmp, UCAST *ttmp, uint len, uint alpha)
// =====================================================================================
{
    int   t;

    for(t=len; t; t--, ttmp++, btmp++)
    {   
        int    r,g,b,r2,g2,b2;

        r = (*ttmp & MASK_RED)   >> SHIFT_RED;
        g = (*ttmp & MASK_GREEN) >> SHIFT_GREEN;
        b = (*ttmp & MASK_BLUE)  >> SHIFT_BLUE;

        r2 = (*btmp & MASK_RED)   >> SHIFT_RED;
        g2 = (*btmp & MASK_GREEN) >> SHIFT_GREEN;
        b2 = (*btmp & MASK_BLUE)  >> SHIFT_BLUE;

        r = (r*SPR_ALPHA_RANGE) + ((r-r2) * alpha);
        g = (g*SPR_ALPHA_RANGE) + ((g-g2) * alpha);
        b = (b*SPR_ALPHA_RANGE) + ((b-b2) * alpha);

        *ttmp = recombine(r,g,b);
    }
}


// =====================================================================================
    static void SprBlit32_Opaque(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST      *ttmp,*btmp;
    uint        itemp;
    VD_SURFACE *vs = info->vs;
         
    btmp  = (UCAST *)info->bitmap;
    itemp = info->ysize;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   alphabland(vs, btmp, ttmp, info->xsize, info->alpha);

        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}


// =====================================================================================
    static void SprBlit32_Trans(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST      *ttmp,*btmp;
    uint        itemp;
    VD_SURFACE *vs = info->vs;
         
    btmp  = (UCAST *)info->bitmap;
    itemp = info->ysize;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   uint  ltemp = info->xsize;
        UCAST *ctmp  = (UCAST *)btmp,
              *dtmp  = (UCAST *)ttmp;
        do
        {   //if(*ctmp) *dtmp = (((*ctmp & vs->alphamask[MASKIDX]) / ALPHA_RANGE) * (ALPHA_RANGE-info->alpha))
            //                + (((*dtmp & vs->alphamask[MASKIDX]) / ALPHA_RANGE) * info->alpha);
            ctmp += 1;
            dtmp += 1;
        } while(--ltemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}
           

// =====================================================================================
    static void SprBlit32_Funky(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST      *ttmp,*btmp;
    uint        itemp, xtemp;
    VD_SURFACE *vs = info->vs;
         
    xtemp = info->xsize;
    itemp = info->ysize;
    
    btmp = (UCAST *)info->bitmap;

    //if(xloc < 0) { xtemp += xloc;  btmp-=xloc; xloc = 0; }
    if(yloc < 0) { itemp += yloc;  btmp = ((UCAST **)info->bitalloc)[-yloc]; yloc = 0; }

    //if(((uint)xloc + xtemp) > vs->physwidth)  xtemp = vs->physwidth  - (uint)xloc;
    if(((uint)yloc + itemp) > vs->physheight) itemp = vs->physheight - (uint)yloc;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   UCAST  *see;
        uint    cc;

        // bit 0   set = opaque; clear = transparent.
        // bit 1   set = no type change; clear = type changes

        cc = *btmp;  btmp++;

        if(cc & 2)
        {   if(cc & 1)
            {   alphabland(vs, btmp, ttmp, xtemp, info->alpha);
                btmp += xtemp;
            }
        } else
        {   if(cc & 1)
            {   see = ttmp;
                cc  = *btmp; btmp++;
                do
                {   alphabland(vs, btmp, see, cc, info->alpha);
                    see += cc;  btmp += cc;
                    cc = *btmp; btmp++;
                    if(cc == 0) break;
                    see += cc;
                    cc  = *btmp; btmp++;
                } while(1);
            } else
            {   see = ttmp;
                cc = *btmp; btmp++;
                do
                {   see += cc;
                    cc = *btmp; btmp++;
                    alphabland(vs, btmp, see, cc, info->alpha);
                    see += cc;  btmp += cc;
                    cc = *btmp; btmp++;
                    if(cc == 0) break;
                } while(1);
            }
        }
        ttmp += vs->physwidth;
    } while(--itemp);

}


static SPR_BLITTER SprAlpha_Blit32Funky =
{
    &SprAlpha_Blit32Funky,

    SprBlit32_Funky,
    SprAlpha32_Funky,
    NULL,
    NULL,

    SPR_FUNKY,
    CPU_NONE,
    SPRA_ALPHA,
    
    BYTESPP,
    { SPRBLT_UNUSED, 0, 0 },

    1,      // no alignment - must be 1!!
    0
};


static SPR_BLITTER SprAlpha_Blit32Trans =
{
    NULL,

    SprBlit32_Trans,
    SprAlpha32_Trans,
    NULL,
    NULL,

    SPR_TRANSPARENT,
    CPU_NONE,
    SPRA_ALPHA,
    
    BYTESPP,
    { SPRBLT_UNUSED, 0, 0 },

    1,      // no alignment - must be 1!!
    0
};


SPR_BLITTER SprAlpha_Blit32Opaque =
{
    &SprAlpha_Blit32Trans,

    SprBlit32_Opaque,
    SprAlpha32_Opaque,
    NULL,
    NULL,

    SPR_OPAQUE,
    CPU_NONE,
    SPRA_ALPHA,
    
    BYTESPP,
    { SPRBLT_UNUSED, 0, 0 },

    1,      // no alignment - must be 1!!
    0
};

