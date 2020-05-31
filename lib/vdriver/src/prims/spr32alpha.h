extern void     SprBlit32_Opaque(const SPRITE *info, int xloc, int yloc);
extern void     SprBlit32_Trans(const SPRITE *info, int xloc, int yloc);
extern void     SprBlit32_Funky(const SPRITE *info, int xloc, int yloc);

extern void     SprAlpha32_FastShadow(const SPRITE *info, int xloc, int yloc);

// =====================================================================================
    static void __inline alphabland(const VD_SURFACE *vs, const alignvalue(4) sprRGB restrict *sap, alignvalue(4) sprRGB restrict *crap, uint len, uint alpha)
// =====================================================================================
{

    for(; len; len--, sap++, crap++)
    {
        int   r,g,b;

        r = (crap->r*SPR_ALPHA_RANGE) + ((sap->r - crap->r)*alpha);
        g = (crap->g*SPR_ALPHA_RANGE) + ((sap->g - crap->g)*alpha);
        b = (crap->b*SPR_ALPHA_RANGE) + ((sap->b - crap->b)*alpha);

        crap->r = r/SPR_ALPHA_RANGE;
        crap->g = g/SPR_ALPHA_RANGE;
        crap->b = b/SPR_ALPHA_RANGE;
    }
}


// =====================================================================================
    static void __inline crossfader(const VD_SURFACE *vs, const alignvalue(4) sprRGB restrict *sap, const alignvalue(4) sprRGB restrict *tap, alignvalue(4) sprRGB restrict *crap, uint len, uint alpha)
// =====================================================================================
// Does an alpha blend between btmp and ctmp and puts the info in the destination ttmp.
{
    for(; len; len--, crap++, sap++, tap++)
    {
        int    r,g,b;

        r = (tap->r*SPR_ALPHA_RANGE) + ((sap->r - tap->r)*alpha);
        g = (tap->g*SPR_ALPHA_RANGE) + ((sap->g - tap->g)*alpha);
        b = (tap->b*SPR_ALPHA_RANGE) + ((sap->b - tap->b)*alpha);

        crap->r = r/SPR_ALPHA_RANGE;
        crap->g = g/SPR_ALPHA_RANGE;
        crap->b = b/SPR_ALPHA_RANGE;
    }
}


// =====================================================================================
    static void __inline blackfader(const VD_SURFACE *vs, const alignvalue(4) sprRGB restrict *sap, alignvalue(4) sprRGB restrict *crap, uint len, uint alpha)
// =====================================================================================
{

    for(; len; len--, sap++, crap++)
    {
        int   r,g,b;

        r = (sap->r * alpha);
        g = (sap->g * alpha);
        b = (sap->b * alpha);

        crap->r = r/SPR_ALPHA_RANGE;
        crap->g = g/SPR_ALPHA_RANGE;
        crap->b = b/SPR_ALPHA_RANGE;
    }
}


// =====================================================================================
    static void SprAlpha32_Opaque(const SPRITE *info, int xloc, int yloc, uint alpha)
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
    {   alphabland(vs, (sprRGB *)btmp, (sprRGB *)ttmp, info->xsize, alpha);

        ttmp += vs->physwidth;
        btmp += info->physwidth;
    } while(--itemp);
}


// =====================================================================================
    static void SprAlpha32_Trans(const SPRITE *info, int xloc, int yloc, uint alpha)
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
            {   alphabland(vs, (sprRGB *)btmp, (sprRGB *)ttmp, xtemp, alpha);
                btmp += xtemp;
            }
        } else
        {   if(cc & 1)
            {   see = ttmp;
                cc  = *btmp; btmp++;
                do
                {   alphabland(vs, (sprRGB *)btmp, (sprRGB *)see, cc, alpha);
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
                    alphabland(vs, (sprRGB *)btmp, (sprRGB *)see, cc, alpha);
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
    static void Spr32_Crossfade(const SPRITE *spra, const SPRITE *sprb, int xloc, int yloc, uint alpha)
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
    uint        itemp;
    VD_SURFACE *vs = spra->vs;
         
    btmp  = (UCAST *)spra->bitmap;
    ctmp  = (UCAST *)sprb->bitmap;

    itemp = spra->ysize;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   crossfader(vs, (sprRGB *)btmp, (sprRGB *)ctmp, (sprRGB *)ttmp, spra->xsize, alpha);

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
    uint        itemp;
    VD_SURFACE *vs = spra->vs;
         
    btmp  = (UCAST *)spra->bitmap;

    itemp = spra->ysize;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP,yloc);

    do
    {   blackfader(vs, (sprRGB *)btmp, (sprRGB *)ttmp, spra->xsize, alpha);

        ttmp += vs->physwidth;
        btmp += spra->physwidth;
    } while(--itemp);
}


