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
 Module: 32-bit.c

  *************************************************************************
            WHITNESS!  The Generic C Sourcefile Header Block!
            Divine Entertainmnt, breaking new ground as always
  *************************************************************************

 This is the 32-bit video primative crap for the vdriver.  Notice how short
 and simple this thing is.  And yet, notce how much the vdriver does.  Yes.
 All functions in this module should never be called directly.  Instead they
 will be called form the subclassed function crap in the VDRIVER surface
 jazz.  They are initialized form the VDRIVER low-level drivers, which
 you shouldn't be calling directly either.

 This block is here just to grab your attention.  Everyone knows that all the
 useful commenting is always well-hidden within the code and procedure head-
 ers anyway.
 
****************************************************************************/


#include "vdriver.h"
#include <string.h>


// This specifies the bytespp for this module.  Through the miracle of really spiffy
// programming technique, you can make this module 16-bit friendly (or very nearly
// so) just by altering this one define.  And, the one below it for typecasting.

#define BYTESPP 4

#include "prims.h"

// =====================================================================================
    void __inline gfx32_line(const VD_SURFACE *vs, int xstart, int ystart, int xend, int yend, int color)
// =====================================================================================
// It's slow, it's bulky.  But, no graphics library would be complete without it.  This 
// line routine, by name, should support all lines, not just horizontal and vertical ones.
// Maybe I'll do that some other day, if I or anyone ever needs it (fat chance! ;).
{
    register UCAST  *scrn;
    register ULONG  len;

    if(ystart == yend)
    {   if(xstart > xend)
        {   len = (xstart - xend);
            scrn = (UCAST *)MK_ScrnPtr(xend*BYTESPP,ystart);
        } else
        {   len = (xend - xstart);
            scrn = (UCAST *)MK_ScrnPtr(xstart*BYTESPP,ystart);
        }
        for(; len; len--, scrn++)
            *scrn = color;
    } else
    {   if(ystart > yend)
        {   len  = (ystart - yend);
            scrn = (UCAST *)MK_ScrnPtr(xstart*BYTESPP,yend);
        } else
        {   len  = (yend - ystart);
            scrn = (UCAST *)MK_ScrnPtr(xstart*BYTESPP,ystart);
        }
        for(; len; len--, scrn+=vs->physwidth) *scrn = color;
    }
}
            

/*******************
  h_line(), v_line() -- Graphics Primitives

  The speedy h_line() [horizontal line] and v_line [vertical line]
  specialized functions.  Due to it's simplicity, h_line has been made into
  a macro (check VDRIVER.H).
*/

// =====================================================================================
    void __inline gfx32_hline(const VD_SURFACE *vs, int xstart, int ystart, int ylen, int color)
// =====================================================================================
{
    register UCAST *scrn = (UCAST *)MK_ScrnPtr(xstart*BYTESPP, ystart);

    for(; ylen; ylen--, scrn++)
        *scrn = color;
}


// =====================================================================================
    void __inline gfx32_vline(const VD_SURFACE *vs, int xstart, int ystart, int ylen, int color)
// =====================================================================================
{
    register UCAST *scrn;
   
    scrn  = (UCAST *)MK_ScrnPtr(xstart*BYTESPP,ystart);
    for(; ylen; ylen--, scrn+=vs->physwidth) *scrn = color;
}


// =====================================================================================
    void gfx32_rect(const VD_SURFACE *vs, const VRECT *rect, int color)
// =====================================================================================
{
    int   ytmp;
    
    for(ytmp=rect->top; ytmp<rect->bottom; ytmp++)
        gfx16_hline(vs, rect->left, ytmp, rect->right - rect->left, color);
} 


/********************************/
/***  Tile Blitter Functions  ***/
/********************************/

void gfx32_TileOpaque(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr)
{
    unsigned int ltemp;

    ltemp = TILE_HEIGHT;

    do
    {   memcpy(tile_scrnptr,tile_bitmap,TILE_BYTEWIDTH);
        tile_scrnptr += BYTESPP;
        tile_bitmap  += TILE_BYTEWIDTH;
    } while(--ltemp);
}  


void gfx32_TileTrans(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr)
{
    unsigned int ltemp,itemp,inc;

    ltemp = TILE_HEIGHT;
    inc   = vs->bytewidth - TILE_BYTEWIDTH;
 
    do
    {   itemp = TILE_WIDTH;
        do
        {   if(*CASTIT(tile_bitmap)) *CASTIT(tile_scrnptr) = *CASTIT(tile_bitmap);
            tile_scrnptr += BYTESPP;
            tile_bitmap  += BYTESPP;
        } while(--itemp);
        tile_scrnptr += inc;
    } while(--ltemp);
}


void gfx32_TileFunky(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr)
{
    UBYTE        cc, *see;
    UCAST        ww;
    unsigned int ltemp,inc;

    ltemp = TILE_HEIGHT;
    inc   = vs->bytewidth - TILE_BYTEWIDTH;

    do
    {   // bit 0   set = opaque; clear = transparent.
        // bit 1   set = no type change; clear = type changes
        cc = ((UBYTE *)tile_bitmap)[0]; tile_bitmap++;
        if(cc & 2)
        {   if(cc & 1)
            {   memcpy(tile_scrnptr,tile_bitmap,TILE_BYTEWIDTH);
                tile_bitmap += TILE_BYTEWIDTH;
            }
            tile_scrnptr += inc;
        } else
        {   if(cc & 1)
            {   see = tile_scrnptr;
                ww  = *CASTIT(tile_bitmap); tile_bitmap+=BYTESPP;
                do
                {   memcpy(see,tile_bitmap,ww);
                    see += ww;  tile_bitmap += ww;
                    ww = *CASTIT(tile_bitmap); tile_bitmap+=BYTESPP;
                    if(ww == 0) break;
                    see += ww;
                    ww = *CASTIT(tile_bitmap); tile_bitmap+=BYTESPP;
                } while(1);
            } else
            {   see = tile_scrnptr;
                ww  = *CASTIT(tile_bitmap); tile_bitmap+=BYTESPP;
                do
                {   see += ww;
                    ww = *CASTIT(tile_bitmap); tile_bitmap+=BYTESPP;
                    memcpy(see,tile_bitmap,ww);
                    see += ww;  tile_bitmap += ww;
                    ww = *CASTIT(tile_bitmap); tile_bitmap+=BYTESPP;
                    if(ww == 0) break;
                } while(1);
            }
            tile_scrnptr += inc;
        }
    } while(--ltemp);
}

