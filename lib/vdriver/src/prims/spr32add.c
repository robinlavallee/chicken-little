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
 Module: spr32add.c

  Additive blending sprite blitters for 32-bit video displays.  Note that there
  is a nifty law of additive blending: Sprites blitted using additive blending
  are always transparently blit (hence the 'Trans' sprite bliiter you will find
  in this module is good for both opaque and transparent sprites!).

 CPU Target: None (Pentium)

 Note to VectorC Users:
   The code in this module may not be MMX friendly.  It has been specifically
   designed for optimization in a non-MMX environment and may not utilize
   MMX optimization features properly!

*/

#include "vdriver.h"

#ifndef BYTESPP
#define BYTESPP 4
#endif

#include "prims.h"
 
static void SprBlit32_Trans(const SPRITE *info, int xloc, int yloc)

// Remember: This is actually a suitable opaque or transparent blitter, because
// this is the world of additive blending and wierd things like that happen.

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

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   uint ltemp = xtemp;
        UCAST *ctmp  = (UCAST *)btmp,
              *dtmp  = (UCAST *)ttmp;
        do
        {   if(*ctmp)
            {   uint  r, g, b;

                r = (*dtmp & vs->mask.red)   >> vs->fieldpos.red;
                g = (*dtmp & vs->mask.green) >> vs->fieldpos.green;
                b = (*dtmp & vs->mask.blue)  >> vs->fieldpos.blue;

                r += (*ctmp & vs->mask.red)   >> vs->fieldpos.red;
                g += (*ctmp & vs->mask.green) >> vs->fieldpos.green;
                b += (*ctmp & vs->mask.blue)  >> vs->fieldpos.blue;
                
                if(r > 255) r = 255;
                if(g > 255) g = 255;
                if(b > 255) b = 255;

                *dtmp = vs->masktable_red[r] | vs->masktable_green[g] | vs->masktable_blue[b];
            }
            dtmp += 1;
            ctmp += 1;
        } while(--ltemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}

static void SprBlit32_Funky(const SPRITE *info, int xloc, int yloc)
{

}


