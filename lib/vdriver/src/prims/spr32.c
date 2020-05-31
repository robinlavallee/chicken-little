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
 Module: spr32.c

  Generic opaque/transparent sprite blitters for 32-bit video displays.

 CPU Target: None (Pentium)

*/

#include "vdriver.h"
#include <string.h>

#ifndef BYTESPP
#define BYTESPP 4
#endif


#include "prims.h"
#include "mminline.h"

/*void SprBlit32_Tiled(const VD_SURFACE *vs, const SPRITE *info, int xloc, int yloc)
{
    UBYTE        *ttmp,*btmp;
    unsigned int  ltemp, itemp, xtemp, inc, t;
         
    btmp  = info->bitmap;
    itemp = info->ysize;   
    xtemp = info->xsize;

    if(info->flags & SPR_TILED)
    {
    }
}*/


// =====================================================================================
    void SprBlit32_Opaque(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST        *ttmp,*btmp;
    int           xtemp,itemp;
    VD_SURFACE   *vs = info->vs;

    if(yloc >= (int)vs->ysize) return;
    
    xtemp = info->xsize;
    itemp = info->ysize;
    
    btmp = (UCAST *)info->bitmap;

    if(xloc < 0) { xtemp += xloc;  btmp-=xloc; xloc = 0; }
    if(yloc < 0) { itemp += yloc;  btmp-=(yloc*info->physwidth); yloc = 0; }

    if(itemp <= 0) return;

    if(((uint)xloc + xtemp) > vs->physwidth)  xtemp = vs->physwidth  - (uint)xloc;
    if(((uint)yloc + itemp) > vs->physheight) itemp = vs->physheight - (uint)yloc;
     
    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   _mminline_memcpy_long(ttmp, btmp, xtemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}

// =====================================================================================
    void SprBlit32_Trans(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST        *ttmp,*btmp;
    int           itemp, xtemp;
    VD_SURFACE   *vs = info->vs;

    if(yloc >= (int)vs->ysize) return;

    xtemp = info->xsize;
    itemp = info->ysize;
    
    btmp = (UCAST *)info->bitmap;

    if(xloc < 0) { xtemp += xloc;  btmp-=xloc; xloc = 0; }
    if(yloc < 0) { itemp += yloc;  btmp-=(yloc*info->physwidth); yloc = 0; }

    if(itemp <= 0) return;

    if(((uint)xloc + xtemp) > vs->physwidth)  xtemp = vs->physwidth  - (uint)xloc;
    if(((uint)yloc + itemp) > vs->physheight) itemp = vs->physheight - (uint)yloc;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   uint   ltemp = xtemp;
        UCAST *ctmp = btmp;
        UCAST *dtmp = ttmp;
        do
        {   if(*ctmp != VD_MakeColor(vs,0,255,255)) *dtmp = *ctmp;
            dtmp += 1;
            ctmp += 1;
        } while(--ltemp);
        //ttmp += info->xinc;
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}

// =====================================================================================
    void SprBlit32_Funky(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST      *ttmp,*btmp;
    int         itemp, xtemp;
    VD_SURFACE *vs = info->vs;
         
    if(yloc >= (int)vs->ysize) return;

    xtemp = info->xsize;
    itemp = info->ysize;
    
    btmp = (UCAST *)info->bitmap;

    //if(xloc < 0) { xtemp += xloc;  btmp-=xloc; xloc = 0; }
    if(yloc < 0) { itemp += yloc;  btmp = ((UCAST **)info->bitalloc)[-yloc]; yloc = 0; }

    if(itemp <= 0) return;

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
            {   _mminline_memcpy_long(ttmp, btmp, xtemp);
                btmp += xtemp;
            }
        } else
        {   if(cc & 1)
            {   see = ttmp;
                cc  = *btmp; btmp++;
                do
                {   _mminline_memcpy_long(see,btmp,cc);
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
                    _mminline_memcpy_long(see,btmp,cc);
                    see += cc;  btmp += cc;
                    cc = *btmp; btmp++;
                    if(cc == 0) break;
                } while(1);
            }
        }
        ttmp += vs->physwidth;
    } while(--itemp);

}


// =====================================================================================
    static void __inline alphabland(const VD_SURFACE *vs, const UCAST *btmp, UCAST *ttmp, uint len, uint alpha)
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

        r = (r*SPR_ALPHA_RANGE) + ((r2-r) * alpha);
        g = (g*SPR_ALPHA_RANGE) + ((g2-g) * alpha);
        b = (b*SPR_ALPHA_RANGE) + ((b2-b) * alpha);

        *ttmp = recombine(r,g,b);
    }
}


// =====================================================================================
    static void SprAlpha32_Opaque(const SPRITE *info, int xloc, int yloc, uint alpha)
// =====================================================================================
{
    UCAST      *ttmp,*btmp;
    int         itemp;
    VD_SURFACE *vs = info->vs;
         
    if(yloc >= (int)vs->ysize) return;

    btmp  = (UCAST *)info->bitmap;
    itemp = info->ysize;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   alphabland(vs, btmp, ttmp, info->xsize, alpha);

        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}


// =====================================================================================
    static void SprAlpha32_Trans(const SPRITE *info, int xloc, int yloc, uint alpha)
// =====================================================================================
{
    UCAST      *ttmp,*btmp;
    uint        itemp;
    VD_SURFACE *vs = info->vs;
         
    if(yloc >= (int)vs->ysize) return;

    btmp  = (UCAST *)info->bitmap;
    itemp = info->ysize;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   uint  ltemp = info->xsize;
        UCAST *ctmp  = (UCAST *)btmp,
              *dtmp  = (UCAST *)ttmp;
        do
        {   //if(*ctmp) *dtmp = (((*ctmp & vs->alphamask[MASKIDX]) / ALPHA_RANGE) * (ALPHA_RANGE-alpha))
            //                + (((*dtmp & vs->alphamask[MASKIDX]) / ALPHA_RANGE) * alpha);
            ctmp += 1;
            dtmp += 1;
        } while(--ltemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}
           

// =====================================================================================
    static void SprAlpha32_Funky(const SPRITE *info, int xloc, int yloc, uint alpha)
// =====================================================================================
{
    UCAST      *ttmp,*btmp;
    int         itemp, xtemp;
    VD_SURFACE *vs = info->vs;
         
    if(yloc >= (int)vs->ysize) return;

    xtemp = info->xsize;
    itemp = info->ysize;
    
    btmp = (UCAST *)info->bitmap;

    //if(xloc < 0) { xtemp += xloc;  btmp-=xloc; xloc = 0; }
    if(yloc < 0) { itemp += yloc;  btmp = ((UCAST **)info->bitalloc)[-yloc]; yloc = 0; }

    if(itemp <= 0) return;

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
            {   alphabland(vs, btmp, ttmp, xtemp, alpha);
                btmp += xtemp;
            }
        } else
        {   if(cc & 1)
            {   see = ttmp;
                cc  = *btmp; btmp++;
                do
                {   alphabland(vs, btmp, see, cc, alpha);
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
                    alphabland(vs, btmp, see, cc, alpha);
                    see += cc;  btmp += cc;
                    cc = *btmp; btmp++;
                    if(cc == 0) break;
                } while(1);
            }
        }
        ttmp += vs->physwidth;
    } while(--itemp);

}


// =====================================================================================
    static void __inline crossfader(const VD_SURFACE *vs, const UCAST *btmp, const UCAST *ctmp, UCAST *ttmp, uint len, uint alpha)
// =====================================================================================
// Does an alpha blend between btmp and ctmp and puts the info in the destination ttmp.
{
    int   t;

    for(t=len; t; t--, ttmp++, btmp++, ctmp++)
    {   
        int    r,g,b,r2,g2,b2;

        r = (*ctmp & MASK_RED)   >> SHIFT_RED;
        g = (*ctmp & MASK_GREEN) >> SHIFT_GREEN;
        b = (*ctmp & MASK_BLUE)  >> SHIFT_BLUE;

        r2 = (*btmp & MASK_RED)   >> SHIFT_RED;
        g2 = (*btmp & MASK_GREEN) >> SHIFT_GREEN;
        b2 = (*btmp & MASK_BLUE)  >> SHIFT_BLUE;

        r = (r*SPR_ALPHA_RANGE) + ((r2-r) * alpha);
        g = (g*SPR_ALPHA_RANGE) + ((g2-g) * alpha);
        b = (b*SPR_ALPHA_RANGE) + ((b2-b) * alpha);

        *ttmp = recombine(r,g,b);
    }
}


// =====================================================================================
    static void __inline blackfader(const VD_SURFACE *vs, const UCAST *btmp, UCAST *ttmp, uint len, uint alpha)
// =====================================================================================
// alpha blends the given source against black, then puts the result in the dest 'ttmp'
{
    int   t;

    for(t=len; t; t--, ttmp++, btmp++)
    {   
        int    r,g,b;

        r = ((*btmp & MASK_RED)   >> SHIFT_RED)   * alpha;
        g = ((*btmp & MASK_GREEN) >> SHIFT_GREEN) * alpha;
        b = ((*btmp & MASK_BLUE)  >> SHIFT_BLUE)  * alpha;

        *ttmp = recombine(r,g,b);
    }
}


// =====================================================================================
    void Spr32_Crossfade(const SPRITE *spra, const SPRITE *sprb, int xloc, int yloc, uint alpha)
// =====================================================================================
// Blits a combo-blit of both sprites.  Basically, this is like doing a standard alpha
// blitter, only the data in sprb is used instead of the surface contents.
//
// Notes:
//  - Both sprites must be the same size (x and y dimensions) and be attached to the same
//    surface.  Not meeting these requirements will produce unexpected results!
//
//  - It only supports opaque sprite right now, and may support color-key transparent
//    later on, but funky sprites are very doubtful!
{
    UCAST      *ttmp,*btmp,*ctmp;
    int         itemp;
    VD_SURFACE *vs = spra->vs;
         
    btmp  = (UCAST *)spra->bitmap;
    ctmp  = (UCAST *)sprb->bitmap;

    itemp = spra->ysize;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   crossfader(vs, btmp, ctmp, ttmp, spra->xsize, alpha);

        ttmp += vs->physwidth;
        btmp += spra->physwidth;
        ctmp += sprb->physwidth;
    } while(--itemp);
}


// =====================================================================================
    static void Spr32_BlackfadeOpaque(const SPRITE *spra, int xloc, int yloc, uint alpha)
// =====================================================================================
{
    UCAST      *ttmp,*btmp;
    int         itemp;
    VD_SURFACE *vs = spra->vs;
         
    btmp  = (UCAST *)spra->bitmap;

    itemp = spra->ysize;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   blackfader(vs, btmp, ttmp, spra->xsize, alpha);

        ttmp += vs->physwidth;
        btmp += spra->physwidth;
    } while(--itemp);
}


// =====================================================================================
    void SprAlpha32_FastShadow(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST      *ttmp,*btmp;
    int         itemp, xtemp;
    VD_SURFACE *vs = info->vs;
         
    if(yloc >= (int)vs->ysize) return;

    xtemp = info->xsize;
    itemp = info->ysize;
    
    btmp = (UCAST *)info->bitmap;

    //if(xloc < 0) { xtemp += xloc;  btmp-=xloc; xloc = 0; }
    if(yloc < 0) { itemp += yloc;  btmp = ((UCAST **)info->bitalloc)[-yloc]; yloc = 0; }

    if(itemp <= 0) return;

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
            {   surfacefade50(vs, ttmp, xtemp);
                btmp += xtemp;
            }
        } else
        {   if(cc & 1)
            {   see = ttmp;
                cc  = *btmp; btmp++;
                do
                {   surfacefade50(vs, see, cc);
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
                    surfacefade50(vs, see, cc);
                    see += cc;  btmp += cc;
                    cc = *btmp; btmp++;
                    if(cc == 0) break;
                } while(1);
            }
        }
        ttmp += vs->physwidth;
    } while(--itemp);

}


static SPR_BLITTER Spr_Blit32Funky =
{
    NULL,

    0,
    SPR_FUNKY,
    CPU_NONE,

    BYTESPP,
    BITSHIFT_SET,

    1,      // no alignment - must be 1!!
    0,

    {
        SprBlit32_Funky,
        SprAlpha32_Funky,
        NULL,
        NULL,
        SprAlpha32_FastShadow,
    }

};


static SPR_BLITTER Spr_Blit32Trans =
{
    &Spr_Blit32Funky,

    0,
    SPR_TRANSPARENT,
    CPU_NONE,

    BYTESPP,
    BITSHIFT_SET,

    1,      // no alignment - must be 1!!
    0,

    {
        SprBlit32_Trans,
        SprAlpha32_Trans,
    }
};


SPR_BLITTER Spr_Blit32Opaque =
{
    &Spr_Blit32Trans,

    0,
    SPR_OPAQUE,
    CPU_NONE,

    BYTESPP,
    BITSHIFT_SET,

    1,      // no alignment - must be 1!!
    0,                    

    {
        SprBlit32_Opaque,
        SprAlpha32_Opaque,
        NULL,
        Spr32_BlackfadeOpaque,
    }
};

