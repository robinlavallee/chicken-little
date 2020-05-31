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
 Module: gamegfx.c

  Graphics Primitives for Off-Screen Video Buffers

       - ( Graphics the Divine Way ) -
       
  This module contains MOST of my general purpose GFX routines.  Because
  the entire game system works on an off-screen memory buffer, all routines
  SHOULD be fully portable to any other system known to man (though perhaps
  not very efficient on those machines).

  Although all of these functions can have driver-specific counterparts
  the ones in this module are 100% portable and flexable (assuming virtual
  screen buffer has been ported).
*/

#include "vdriver.h"
#include <string.h>
#include <stdarg.h>


/*
// =====================================================================================
    SPRITE *cut_sprite(UBYTE *source, int srcx, int x, int y, int dx, int dy, int bytespp)
// =====================================================================================
{
    UBYTE  *offset, *destptr;
    SPRITE *dest;

    dest    = (SPRITE *)_mm_malloc(sizeof(SPRITE));
    destptr = dest->bitmap = (UBYTE *)_mm_malloc(dx * dy * bytespp);
    dest->xsize   = dx;
    dest->ysize   = dy;

    dy += y;             
    for(; y<dy; y++, destptr+=dx)
    {   offset = &source[((y*srcx)+x)*bytespp];
        memcpy(destptr,offset,dx*bytespp);
    }

    return(dest);
}


// =====================================================================================
    UBYTE *cut_tile(UBYTE *source, int srcx, int tilex, int tiley, int x, int y)
// =====================================================================================
{
    UBYTE  *destptr, *dest;

    dest   = destptr = (UBYTE *)_mm_malloc(tilex * tiley);
    tiley += y;

    for(; y<tiley; y++, destptr+=tilex)
        memcpy(destptr,&source[(y*srcx)+x],tilex);

    return(dest);
}
*/

