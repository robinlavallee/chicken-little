// *** MMX ***

#ifdef __MMX__

#include "vdriver.h"

SPR_BLITTER *SprAlpha_MMX32[8] =
{
//    &SprAlphaRGB_MMX32Opaque,
//    &SprAlphaBGR_MMX32Opaque,
    &SprAlpha_Blit32Opaque,
    NULL
};

#endif
