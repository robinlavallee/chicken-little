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
 Module: spr16.c

  Basic non-specific sprite blitters for 16-bit video displays.  Opaque, transparent,
  funky (RLE), and maybe some others.  Who knows.  Be suprised.

 CPU Target: None (Pentium)

*/


#include "vdriver.h"
#include <string.h>

#ifndef BYTESPP
#define BYTESPP 2
#endif

#include "prims.h"
#include "mminline.h"


// =====================================================================================
    void SprBlit16_Opaque(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST        *ttmp,*btmp;
    int           itemp, xtemp;
    VD_SURFACE   *vs = info->vs;

    if((yloc >= (int)vs->ysize) || (xloc >= (int)vs->xsize)) return;

    xtemp = info->xsize;
    itemp = info->ysize;

    btmp = (UCAST *)info->bitmap;

    if(xloc < 0) { xtemp += xloc;  btmp-=xloc; xloc = 0; }
    if(xtemp <= 0) return;

    if(yloc < 0) { itemp += yloc;  btmp -= (yloc*info->physwidth); yloc = 0; }
    if(itemp <= 0) return;

    if(((uint)xloc + xtemp) > vs->physwidth)  xtemp = vs->physwidth  - (uint)xloc;
    if(((uint)yloc + itemp) > vs->physheight) itemp = vs->physheight - (uint)yloc;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   _mminline_memcpy_word(ttmp, btmp, xtemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}


// =====================================================================================
    static void __inline blitscan_trans(ULONG *ctmp, ULONG *ttmp, int len)
// =====================================================================================
{
    do
    {   ULONG src = *ctmp;
        if(src)
        {   if(src & 0xfffful)
            {   if(src & ~0xfffful)
                    *ttmp = src;
                else
                    *ttmp = (*(ULONG *)ttmp & ~0xffff) | src;
            } else *ttmp = (*ttmp & 0xffff) | src;
        }
        ttmp += 1;
        ctmp += 1;
    } while(--len);
}


// =====================================================================================
    void SprBlit16_Trans(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST        *ttmp,*btmp;
    int           itemp, xtemp;
    VD_SURFACE   *vs = info->vs;

    if(yloc >= (int)vs->ysize) return;

    xtemp = info->xsize;
    itemp = info->ysize;
    
    btmp = (UCAST *)info->bitmap;

    if(xloc < 0) { xtemp += (xloc & ~1);  btmp -= (xloc & ~1); xloc = -(xloc & 1); }
    if(yloc < 0) { itemp += yloc;  btmp-=(yloc*info->physwidth); yloc = 0; }

    if(itemp <= 0) return;

    if(((uint)xloc + xtemp) > vs->physwidth)  xtemp = vs->physwidth  - (uint)xloc;
    if(((uint)yloc + itemp) > vs->physheight) itemp = vs->physheight - (uint)yloc;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    xtemp >>= 1;

    do
    {   blitscan_trans((ULONG *)btmp, (ULONG *)ttmp, xtemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}


// =====================================================================================
    void SprBlit16_Funky(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST      *ttmp;
    UCAST      *btmp;
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

        cc = *((ULONG *)btmp);  btmp+=2;

        if(cc & 2)
        {   if(cc & 1)
            {   _mminline_memcpy_word(ttmp,btmp, xtemp);
                btmp += xtemp;
            }
        } else
        {   if(cc & 1)
            {   see = ttmp;
                cc  = *((ULONG *)btmp); btmp+=2;
                do
                {   _mminline_memcpy_word(see,btmp,cc);
                    see += cc;  btmp += cc + (cc & 1);
                    cc = *((ULONG *)btmp); btmp+=2;
                    if(cc == 0) break;
                    see += cc;
                    cc  = *((ULONG *)btmp); btmp+=2;
                } while(1);
            } else
            {   see = ttmp;
                cc = *((ULONG *)btmp); btmp+=2;
                do
                {   see += cc;
                    cc = *((ULONG *)btmp); btmp+=2;
                    _mminline_memcpy_word(see,btmp,cc);
                    see += cc;  btmp += cc + (cc & 1);
                    cc = *((ULONG *)btmp); btmp+=2;
                    if(cc == 0) break;
                } while(1);
            }
        }
        ttmp += vs->physwidth;
        //btmp += info->physwidth;
    } while(--itemp);

}


// =====================================================================================
    static void SprAlpha16_Opaque(const SPRITE *info, int xloc, int yloc, uint alpha)
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
    static void SprAlpha16_Trans(const SPRITE *info, int xloc, int yloc, uint alpha)
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
    static void SprAlpha16_Funky(const SPRITE *info, int xloc, int yloc, uint alpha)
// =====================================================================================
{
    UCAST      *ttmp;
    UCAST      *btmp;
    int         itemp, xtemp;
    VD_SURFACE *vs = info->vs;
         
    if(yloc >= (int)vs->ysize) return;

    xtemp = info->xsize;
    itemp = info->ysize;
    
    if(yloc <= -(int)itemp) return;            // no render if not visible!

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

        cc = *((ULONG *)btmp);  btmp+=2;

        if(cc & 2)
        {   if(cc & 1)
            {   alphabland(vs,btmp, ttmp, xtemp, alpha);
                btmp += xtemp;
            }
        } else
        {   if(cc & 1)
            {   see = ttmp;
                cc  = *((ULONG *)btmp); btmp+=2;
                do
                {   alphabland(vs,btmp, see, cc, alpha);
                    see += cc;  btmp += cc + (cc & 1);
                    cc = *((ULONG *)btmp); btmp+=2;
                    if(cc == 0) break;
                    see += cc;
                    cc  = *((ULONG *)btmp); btmp+=2;
                } while(1);
            } else
            {   see = ttmp;
                cc = *((ULONG *)btmp); btmp+=2;
                do
                {   see += cc;
                    cc = *((ULONG *)btmp); btmp+=2;
                    alphabland(vs,btmp, see, cc, alpha);
                    see += cc;  btmp += cc + (cc & 1);
                    cc = *((ULONG *)btmp); btmp+=2;
                    if(cc == 0) break;
                } while(1);
            }
        }
        ttmp += vs->physwidth;
    } while(--itemp);
}


// =====================================================================================
    void Spr16_Crossfade(const SPRITE *spra, const SPRITE *sprb, int xloc, int yloc, uint alpha)
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
    static void Spr16_BlackfadeOpaque(const SPRITE *spra, int xloc, int yloc, uint alpha)
// =====================================================================================
{
    UCAST      *ttmp,*btmp;
    uint        itemp;
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
    void SprAlpha16_FastShadow(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
{
    UCAST      *ttmp;
    UCAST      *btmp;
    int         itemp, xtemp;
    VD_SURFACE *vs = info->vs;
         
    if(yloc >= (int)vs->ysize) return;

    xtemp = info->xsize;
    itemp = info->ysize;

    if(yloc <= -itemp) return;            // no render if not visible!

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

        cc = *((ULONG *)btmp);  btmp+=2;

        if(cc & 2)
        {   if(cc & 1)
            {   surfacefade50(vs, ttmp, xtemp);
                btmp += xtemp;
            }
        } else
        {   if(cc & 1)
            {   see = ttmp;
                cc  = *((ULONG *)btmp); btmp+=2;
                do
                {   surfacefade50(vs, see, cc);
                    see += cc;  btmp += cc + (cc & 1);
                    cc = *((ULONG *)btmp); btmp+=2;
                    if(cc == 0) break;
                    see += cc;
                    cc  = *((ULONG *)btmp); btmp+=2;
                } while(1);
            } else
            {   see = ttmp;
                cc = *((ULONG *)btmp); btmp+=2;
                do
                {   see += cc;
                    cc = *((ULONG *)btmp); btmp+=2;
                    surfacefade50(vs, see, cc);
                    see += cc;  btmp += cc + (cc & 1);
                    cc = *((ULONG *)btmp); btmp+=2;
                    if(cc == 0) break;
                } while(1);
            }
        }
        ttmp += vs->physwidth;
    } while(--itemp);
}


static SPR_BLITTER Spr_Blit16Funky =
{
    NULL,

    0,
    SPR_FUNKY,
    CPU_NONE,
    
    BYTESPP,
    { SPRBLT_UNUSED, 0, 0 },

    2,
    0,

    {   
        SprBlit16_Funky,
        SprAlpha16_Funky,
        NULL,

        NULL,
        SprAlpha16_FastShadow,
    }
};


static SPR_BLITTER Spr_Blit16Trans =
{
    &Spr_Blit16Funky,

    0,
    SPR_TRANSPARENT,
    CPU_NONE,
    
    BYTESPP,
    { SPRBLT_UNUSED, 0, 0 },

    2,             // two pixel alignment
    0,

    {
        SprBlit16_Trans,
        SprAlpha16_Trans,
    }
};

SPR_BLITTER Spr_Blit16Opaque =
{
    &Spr_Blit16Trans,

    0,
    SPR_OPAQUE,
    CPU_NONE,
    
    // opaque sprites cannot prebuffer since they are opaque and any such
    // thing would be pretty noticable.

    BYTESPP,
    { SPRBLT_UNUSED, 0, 0 },

    2,             // two pixel alignment
    0,

    {
        SprBlit16_Opaque,
        SprAlpha16_Opaque,
        NULL,

        Spr16_BlackfadeOpaque,
    }

};
