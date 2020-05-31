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
 Module: spr16add.c

  Additive blending sprite blitters for 16-bit video displays.

 CPU Target: None (Pentium)

*/

#include "vdriver.h"
#include <string.h>

#ifndef BYTESPP
#define BYTESPP 2
#endif

#include "prims.h"


static void __inline additivecrap(VD_SURFACE *vs, const UCAST *btmp, UCAST *ttmp, uint ltemp)
{

    do
    {   if(*btmp)
        {   uint  r, g, b;

            r = (*ttmp & vs->mask.red)   >> vs->fieldpos.red;
            g = (*ttmp & vs->mask.green) >> vs->fieldpos.green;
            b = (*ttmp & vs->mask.blue)  >> vs->fieldpos.blue;

            r += (*btmp & vs->mask.red)   >> vs->fieldpos.red;
            g += (*btmp & vs->mask.green) >> vs->fieldpos.green;
            b += (*btmp & vs->mask.blue)  >> vs->fieldpos.blue;
            
            if(r > 255) r = 255;
            if(g > 255) g = 255;
            if(b > 255) b = 255;

            *ttmp = vs->masktable_red[r] | vs->masktable_green[g] | vs->masktable_blue[b];
        }
        ttmp += 1;
        btmp += 1;
    } while(--ltemp);
}


static void SprBlit16_Trans(const SPRITE *info, int xloc, int yloc)

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
    {   additivecrap(vs, btmp, ttmp, xtemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}


