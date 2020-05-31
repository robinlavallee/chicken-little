#include "vdriver.h"

extern SPR_BLITTER Spr_Blit32Opaque_RGB;
extern SPR_BLITTER Spr_Blit32Opaque_BGR;

SPR_BLITTER *SprList_Blit32[8] =
{
    &Spr_Blit32Opaque_RGB,
    &Spr_Blit32Opaque_BGR,
    &Spr_Blit32Opaque,
    NULL
};
