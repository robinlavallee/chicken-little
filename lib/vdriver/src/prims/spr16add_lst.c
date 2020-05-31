/*
 -- Air's rather small module-like code snippet fettish --

 ---------------------------------------------------
 Module: spr16add_lst.c

  This is a sprite blitter list which registers all pertinent 16 bit sprite
  blitters that come with VDRIVER.  Erm, that's non-MMX, etc. type anyway.

*/


#include "vdriver.h"

SPR_BLITTER *SprAdd_Blit16[8] =
{
    &SprAdd565_Blit16Opaque,
    &SprAdd555_Blit16Opaque,
    &SprAdd_Blit16Opaque,
    NULL
};
