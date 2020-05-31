#include "vdriver.h"

SPR_BLITTER *SprAdd_Blit32[8] =
{
    &SprAddRGB_Blit32Opaque,
    &SprAddBGR_Blit32Opaque,
    &SprAdd_Blit32Opaque,
    NULL
};
