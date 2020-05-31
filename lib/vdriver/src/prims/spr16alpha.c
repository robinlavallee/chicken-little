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
 Module: spr16alpha.c

  Alpha blending sprite blitters for 16-bit video displays.

 CPU Target: None (Pentium)

*/

#include "vdriver.h"

#ifndef BYTESPP
#define BYTESPP 2
#endif

#include "prims.h"
#include "mminline.h"

 
static void SprBlit16_Opaque(const SPRITE *info, int xloc, int yloc)
{
    UCAST        *ttmp,*btmp;
    uint          itemp, xtemp;
    VD_SURFACE   *vs = info->vs;

    xtemp = info->physwidth;
    itemp = info->ysize;

    btmp = (UCAST *)info->bitmap;

    if(xloc < 0) { xtemp += (xloc & ~1);  btmp -= (xloc & ~1); xloc &= 1; }
    if(yloc < 0) { itemp += yloc;  btmp -= (yloc*info->physwidth); yloc = 0; }

    if(((uint)xloc + xtemp) > vs->physwidth)  xtemp = vs->physwidth  - (uint)xloc;
    if(((uint)yloc + itemp) > vs->physheight) itemp = vs->physheight - (uint)yloc;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   _mminline_memcpy_word(ttmp, btmp, xtemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}

static void SprBlit16_Trans(const SPRITE *info, int xloc, int yloc)
{
    UCAST        *ttmp,*btmp;
    uint          itemp, xtemp;
    VD_SURFACE   *vs = info->vs;

    xtemp = info->xsize;
    itemp = info->ysize;
    
    btmp = (UCAST *)info->bitmap;

    if(xloc < 0) { xtemp += xloc;  btmp-=xloc; xloc = 0; }
    if(yloc < 0) { itemp += yloc;  btmp-=(yloc*info->physwidth); yloc = 0; }

    if(((uint)xloc + xtemp) > vs->physwidth)  xtemp = vs->physwidth  - (uint)xloc;
    if(((uint)yloc + itemp) > vs->physheight) itemp = vs->physheight - (uint)yloc;

    ttmp = (UCAST *)MK_ScrnPtr(xloc,yloc);

    do
    {   uint  ltemp = xtemp;
        UCAST *ctmp = btmp,
              *dtmp = ttmp;

        do
        {   if(*ctmp) *dtmp = *ctmp;
            ctmp += 1;
            dtmp += 1;
        } while(--ltemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}



SPR_BLITTER SprAlpha_Blit16Trans =
{
    NULL,

    SprBlit16_Trans,
    NULL,
    NULL,
    NULL,

    SPR_TRANSPARENT,
    CPU_NONE,
    SPRA_ALPHA,
    
    BYTESPP,
    { SPRBLT_UNUSED, 0, 0 },

    2,             // two pixel alignment
    2              // two pixel pre-buffer
};

SPR_BLITTER SprAlpha_Blit16Opaque =
{
    &SprAlpha_Blit16Trans,

    SprBlit16_Opaque,
    NULL,
    NULL,
    NULL,

    SPR_OPAQUE,
    CPU_NONE,
    SPRA_ALPHA,
    
    // opaque sprites cannot have alignment or prebuffer since
    // they are opaque and any such thing would be pretty noticable.

    BYTESPP,
    { SPRBLT_UNUSED, 0, 0 },

    2,             // two pixel alignment
    0              // two pixel pre-buffer                    
};
