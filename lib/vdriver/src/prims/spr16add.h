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
 Module: spr16add.h

  This is a 'universal' optimized sprite blitter.  It should work with any 16 bit
  format which has been properly preped first using prims.h.  Hewnce, this code
  is used in a couple modules to produce working 5:6:5 and 5:5:5 additive blending
  blitters.

  Note that there is a nifty law of additive blending: Sprites blitted using ad-
  ditive blending are always transparently blit (hence the 'Trans' sprite blitter
  you will find in this module is good for both opaque and transparent sprites!).

 CPU Target: None (Pentium)

*/


// =====================================================================================
    static void SprBlit16_Trans(const SPRITE *info, int xloc, int yloc)
// =====================================================================================
// Remember: This is actually a suitable opaque or transparent blitter, because
// this is the world of additive blending and wierd things like that happen.
{
    UCAST        *ttmp;
    UCAST        *btmp;
    uint          itemp, xtemp;
    VD_SURFACE   *vs = info->vs;

    xtemp = info->xsize;
    itemp = info->ysize;
    
    btmp = (UCAST *)info->bitmap;

    // Make sure ttmp and btmp end up being on dword boundries.

    if(xloc < 0)
        { xtemp += (xloc & ~1);  btmp -= (xloc & ~1); xloc = -(xloc & 1); }
    else
        { btmp -= (xloc & 1); xtemp += (xloc & 1); xloc -= (xloc & 1);}

    if(yloc < 0) { itemp += yloc;  btmp-=(yloc*info->physwidth); yloc = 0; }

    if(((uint)xloc + xtemp) > vs->physwidth)  xtemp = vs->physwidth  - (uint)xloc;
    if(((uint)yloc + itemp) > vs->physheight) itemp = vs->physheight - (uint)yloc;

    
    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    xtemp >>= 1;

    do
    {   uint   ltemp = xtemp;
        ULONG *ctmp  = (ULONG *)btmp,
              *dtmp  = (ULONG *)ttmp;
        do
        {   if(*ctmp)
            {   uint  r, g, b, r2, g2, b2;

                r = (*ctmp & MASK_RED);
                g = (*ctmp & MASK_GREEN);
                b = (*ctmp & MASK_BLUE);

                r += (*dtmp & MASK_RED);
                g += (*dtmp & MASK_GREEN);
                b += (*dtmp & MASK_BLUE);

                if(r > MASK_RED)   r = MASK_RED;
                if(g > MASK_GREEN) g = MASK_GREEN;
                if(b > MASK_BLUE)  b = MASK_BLUE;

                r2 = (*ctmp & MASKHI_RED) >> 16;
                g2 = (*ctmp & MASKHI_GREEN);
                b2 = (*ctmp & MASKHI_BLUE);

                r2 += (*ttmp & MASKHI_RED) >> 16;
                g2 += (*ttmp & MASKHI_GREEN);
                b2 += (*ttmp & MASKHI_BLUE);

                if(r2 > MASK_RED)     r2 = MASK_RED;
                if(g2 > MASKHI_GREEN) g2 = MASKHI_GREEN;
                if(b2 > MASKHI_BLUE)  b2 = MASKHI_BLUE;

                *ttmp = r | g | b | (r2 << 16) | g2 | b2;

            }
            ttmp += 1;
            ctmp += 1;
        } while(--ltemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}
