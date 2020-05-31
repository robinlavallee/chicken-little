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
 Module: spr32add.h

  This is a 'universal' optimized sprite blitter.  It should work with any 32 bit
  format which has been properly preped first using prims.h.  Hewnce, this code
  is used in a couple modules to produce working RGB and BGR additive blending
  blitters.

  Note that there is a nifty law of additive blending: Sprites blitted using ad-
  ditive blending are always transparently blit (hence the 'Trans' sprite blitter
  you will find in this module is good for both opaque and transparent sprites!).

 CPU Target: None (Pentium)

*/


static void __inline dopixels(const VD_SURFACE *vs, alignvalue(4) sprRGB restrict *sap, alignvalue(4) sprRGB restrict *crap, uint length)
{
    int   r,g,b;

    do
    {
        r = crap->r + sap->r;
        g = crap->g + sap->g;
        b = crap->b + sap->b;

        if(r > 255) r = 255;
        if(g > 255) g = 255;
        if(b > 255) b = 255;

        crap->r = r;
        crap->g = g;
        crap->b = b;

        sap  += 1;
        crap += 1;
    } while(--length);
}


static void Spr32_Opaque(const SPRITE *info, int xloc, int yloc)

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
    {   dopixels(vs, (sprRGB *)btmp, (sprRGB *)ttmp, xtemp);
        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}

