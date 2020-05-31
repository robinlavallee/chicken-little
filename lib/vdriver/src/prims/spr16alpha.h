

extern void     SprBlit16_Opaque(const SPRITE *info, int xloc, int yloc);
extern void     SprBlit16_Trans(const SPRITE *info, int xloc, int yloc);
extern void     SprBlit16_Funky(const SPRITE *info, int xloc, int yloc);

extern void     SprAlpha16_FastShadow(const SPRITE *info, int xloc, int yloc);

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
    static void Spr16_Crossfade(const SPRITE *spra, const SPRITE *sprb, int xloc, int yloc, uint alpha)
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
    int        itemp;
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
    int        itemp;
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


