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
 Module: i386-mmx/spr32rgb_add.c

  Additive blending sprite blitters for 16-bit video displays.  Note that there
  is a nifty law of additive blending: Sprites blitted using additive blending
  are always transparently blit (hence the 'Trans' sprite bliiter you will find
  in this module is good for both opaque and transparent sprites!).

 CPU Target: MMX (Pentium)

  This module has been designed with MMX in mind and is not optimal for use in
  non-MMX compiler situations.  ie, use with Vector C compiler only (unless some
  other elite compiler comes out which is as good or better).

  Specifically: This uses an opaque blitter rather than a transparent one!

*/

#ifdef __MMX__

#include "vdriver.h"

#ifndef BYTESPP
#define BYTESPP 4
#endif

#define FORMAT_BGR

#include "../prims.h"
#include "spr32add.h" 

static SPR_BLITTER SprAddBGRX32Trans =
{
    NULL,
    Spr32_Opaque,
    SPR_TRANSPARENT,
    CPU_MMX,
    SPRA_ADDITIVE,
    
    BYTESPP,
    BITSHIFT_SET,

    2,
    2                    
};

SPR_BLITTER SprAddBGRX32Opaque =
{
    &SprAddBGRX32Trans,
    Spr32_Opaque,
    SPR_OPAQUE,
    CPU_MMX,
    SPRA_ADDITIVE,
    
    BYTESPP,
    BITSHIFT_SET,

    2,
    2    
};

#endif
