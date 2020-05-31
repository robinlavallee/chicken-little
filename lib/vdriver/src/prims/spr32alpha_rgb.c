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
 Module: spr32rgb_alpha.c

  Alpha blending sprite blitters for 32-bit video displays.

 CPU Target: None (Pentium)

*/

#include "vdriver.h"
#include <string.h>

#ifndef BYTESPP
#define BYTESPP 4
#endif

#include "prims.h"
#include "spr32alpha.h"

static SPR_BLITTER Spr_Blit32Funky_RGB =
{
    NULL,

    0,
    SPR_FUNKY,
    CPU_NONE,

    BYTESPP,
    BITSHIFT_SET,

    1,      // no alignment - must be 1!!
    0,

    {
        SprBlit32_Funky,
        SprAlpha32_Funky,
        NULL,
        NULL,
        SprAlpha32_FastShadow,
    }
};


static SPR_BLITTER Spr_Blit32Trans_RGB =
{
    &Spr_Blit32Funky_RGB,

    0,
    SPR_TRANSPARENT,
    CPU_NONE,

    BYTESPP,
    BITSHIFT_SET,

    1,      // no alignment - must be 1!!
    0,

    {
        SprBlit32_Trans,
        SprAlpha32_Trans,
    }
};


SPR_BLITTER Spr_Blit32Opaque_RGB =
{
    &Spr_Blit32Trans_RGB,

    0,
    SPR_OPAQUE,
    CPU_NONE,

    BYTESPP,
    BITSHIFT_SET,

    1,      // no alignment - must be 1!!
    0,

    {
        SprBlit32_Opaque,
        SprAlpha32_Opaque,
        NULL,
        Spr32_BlackfadeOpaque,
    }

};

