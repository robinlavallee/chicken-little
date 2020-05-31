
#include "vdriver.h"

extern SPR_BLITTER Spr_Blit16Opaque_565;
extern SPR_BLITTER Spr_Blit16Opaque_555;

SPR_BLITTER *SprList_Blit16[8] =
{
    &Spr_Blit16Opaque_565,
    &Spr_Blit16Opaque_555,
    &Spr_Blit16Opaque,
    NULL
};
