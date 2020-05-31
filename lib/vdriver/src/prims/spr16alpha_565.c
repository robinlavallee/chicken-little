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
 Module: i386-mmx/spr16_alpha565.c

  Alpha blending sprite blitters for 16-bit video displays.

 CPU Target: MMX (Pentium)

*/

#include "vdriver.h"

#ifndef BYTESPP
#define BYTESPP 2
#endif

#define FORMAT  565

#include "prims.h"

#include "spr16alpha.h"

/*
static SPR_BLITTER Spr_Blit16Trans_565 =
{
    NULL,

    SprBlit16_Opaque,
    SprAlpha16_Opaque,
    NULL,
    NULL,
    NULL,
    
    SPR_TRANSPARENT,
    CPU_NONE,
    0,

    BYTESPP,
    BITSHIFT_SET,

    2,
    0
};
*/

static SPR_BLITTER Spr_Blit16Funky_565 =
{
    //&Spr_Blit16Trans_565
    NULL,

    0,
    SPR_FUNKY,
    CPU_NONE,
    
    BYTESPP,
    BITSHIFT_SET,

    2,
    0,

    {
        SprBlit16_Funky,
        SprAlpha16_Funky,
        NULL,
        NULL,
        SprAlpha16_FastShadow,
    }
};


SPR_BLITTER Spr_Blit16Opaque_565 =
{
    &Spr_Blit16Funky_565,

    0,
    SPR_OPAQUE,
    CPU_NONE,

    BYTESPP,
    BITSHIFT_SET,

    2,
    0,

    {
        SprBlit16_Opaque,
        SprAlpha16_Opaque,
        NULL,
        Spr16_BlackfadeOpaque,
    }
};
