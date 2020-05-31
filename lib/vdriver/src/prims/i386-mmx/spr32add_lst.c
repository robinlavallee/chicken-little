// *** MMX ***

#ifdef __MMX__

#include "vdriver.h"

SPR_BLITTER *SprAdd_MMX32[8] =
{
    &SprAddRGB_MMX32Opaque,
    &SprAddBGR_MMX32Opaque,
    &SprAdd_Blit32Opaque,
    NULL
};

#endif
