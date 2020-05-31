
static void __inline dopixel(const VD_SURFACE *vs, alignvalue(8) sprRGB restrict *sap, alignvalue(8) sprRGB restrict *crap)
{
    int   r,g,b;

    r = crap->r + sap->r;
    g = crap->g + sap->g;
    b = crap->b + sap->b;

    if(r < 0)   r = 0;
    if(g < 0)   g = 0;
    if(b < 0)   b = 0;
    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;

    crap->r = r;
    crap->g = g;
    crap->b = b;
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

    btmp += 2;

    if(xloc < 0) { xtemp += (xloc & ~1);  btmp -= (xloc & ~1); xloc = -(xloc & 1); }
    if(yloc < 0) { itemp += yloc;  btmp-=(yloc*info->physwidth); yloc = 0; }

    if(((uint)xloc + xtemp) > vs->physwidth)  xtemp = vs->physwidth  - (uint)xloc;
    if(((uint)yloc + itemp) > vs->physheight) itemp = vs->physheight - (uint)yloc;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    xtemp >>= 1;

    do
    {   uint ltemp = xtemp;
        spr_RGB *ctmp = (spr_RGB *)btmp;
        do
        {   dopixel(vs, ctmp, (sprRGB *)ttmp);
            ttmp += 1;
            ctmp += 1;
        } while(--ltemp);
        ttmp += info->xinc;
        btmp += info->physwidth;
    } while(--itemp);
}

