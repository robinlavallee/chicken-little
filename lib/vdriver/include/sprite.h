
/* 
  Notes about Sprites and sprite blitters:

   - sprblit_gamma is a basic brightness blitter with saturation.

   - sprblit_addblend does a weighted additive blend of the source and destination.
     This function can be potentially very slow, but is capable of additive blending,
     alpha blending, alpha blending with brightness control, etc.  A good 'general-
     purpose' routine to test wierd blending ideas and stuff.

   - 

   - sprblit_fastshadow work with RLE (funky) sprites only!  It generates an alpha
     mask at 50% based on the runs of the given sprite.


*/

#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "vdriver.h"

#define sprblit(a,b,c)             a.api.blit(&a,b,c)
#define sprblit_fastshadow(a,b,c)  a.api.fastshadow(&a,b,c)
#define sprblit_alpha(a,b,c,d)     a.api.alphablit(&a,b,c,d)

#define sprblit_blackfade(a,c,d,e) a.api.blackfade(&a,c,d,e)


/*********************************
  --> Structure :  SPRITE

  A sprite is, in short and simple form, an image in a very raw hardware-friendly
  format.  This means that when you start getting to know it, you realize it sucks
  for anything except being blitted to a surface. :)

  Sprites have some rigid requirements in order to display properly.  All of these
  are geared toward optimal speed:

   - Sprites must have a leading (left-side) buffer of a given length (de-
     termined by the driver).
   - Sprites must have a width that is evenly divisible by a given value (de-
     termined by the driver) -- usually 4 or 8.


 Values for "flag" :
   Bits 0-1 : Sprite Type
     0  =  Full Opaque
     1  =  Transparent
     2  =  Translucent

   Bit  2   : Transparent Run  Compression / Optimization (Funky)

 NOTES:

  - a sprite can be of two catagories:
     1) bitmap data - possible types:
        a) opaque  b) transparent c) funky (transparent RLE)
     2) tiled data - a map of indexes to tiles.
     
*/

#define SPR_ALPHA_RANGE  256

// =================================
//  Opacity Modes
//
//  Notes:
//   - SPR_OPAQUE, SPR_TRANSPARENT, and SPR_FUNKY are mutually exclusive.  If
//     you set more than one of those flags, the blit detection will fail.
//
//   - SPR_TRANSPARENT is the default sprite type.  If no type is specified
//     then transparent is assumed.
//
//   - SPR_TILED consists of multiple tiles, each of which can be fully trans-
//     parent or fully opaque (but never funky!).  This is a good measure for avoiding
//     large color-key transparent regions when RLE is not an option (texturing and stuff).
//
//   - SPR_TILED is currently unsupported.
//
//  Standard Format flags:
//
//   - Embedded alpha information alters the data storage format of images which
//     are less than 32bpp.
//
//   - ALPHA_MASK and ALPHA_EMBEDDED can be combined, however the mask takes precedence
//     for now (I will make some super lame slow-ass blitter later which actually merges
//     the two alpha channels and stuff :)

enum
{
    SPR_TRANSPARENT = 0,
    SPR_OPAQUE,
    SPR_FUNKY,
    SPR_TILED
};

#define SPR_CLIPPED         (1<<0)    // use the clipper blitters (slower, but they clip)
#define SPR_ALPHA_MASK      (1<<2)    // sprite->alphamask contains alpha mask data.
#define SPR_ALPHA_EMBEDDED  (1<<4)    // alpha mask info is embedded into the sprite data


// =====================================================================================
    typedef struct SPR_TILEMAP
// =====================================================================================
// Used to represent each piece of a sprite tilemap.
{
    uint    mode;
    uint    idx;
} SPR_TILEMAP;


// =====================================================================================
    typedef struct SPR_API
// =====================================================================================
// The sprite blitter API set, which is shared by the sprite and SPR_BLITTER plugin info
// set.
{
    void       (*blit)(const struct SPRITE *spr, int xloc, int yloc);
    void       (*alphablit)(const struct SPRITE *spr, int xloc, int yloc, uint alpha);
    void       (*addblit)(const struct SPRITE *spr, int xloc, int yloc);

    // extracurricular alpha blitters (faders)

    void       (*blackfade)(const struct SPRITE *spra, int xloc, int yloc, uint alpha);
    void       (*fastshadow)(const struct SPRITE *spr, int xloc, int yloc);

    // extracurricular 'super' blitters.  Potentially very slow!

    void       (*addblend)(const struct SPRITE *spr, int xloc, int yloc, uint src, uint dest);
    void       (*gammablit)(const struct SPRITE *spr, int xloc, int yloc, uint gamma);

    // Color-component blitters.

    void       (*c_alphablit)(const struct SPRITE *spr, int xloc, int yloc, uint ra, uint ba, uint ca);
    void       (*c_gamma)(const struct SPRITE *spr, int xloc, int yloc, uint ra, uint ba, uint ca );

} SPR_API;


// =====================================================================================
    typedef struct SPRITE
// =====================================================================================
// Note that you should not modify most of this structure directly, unless you really know
// what you are doing.  Instead, use Sprite_Init();
{   
    int          residx;               // the resource index, for reloading sprites from wad files.

    // -----------------------------------------------
    // From here on out is stuff that you should only set or change via reinitializing
    // the sprite using a call to Sprite_Init().

    uint         flags;        // various flags (see above)
    uint         opacity;      // SPR_OPAQUE / SPR_TRANSPARENT / SPR_FUNKY

    uint         xsize,ysize;  // Actual pixel-width of sprite, minus any left/right side buffers.

    UBYTE       *bitmap,       // the bitmap data
                *bitalloc;     // allocated memory block (may differ due to prebuffer)
    UBYTE       *alphamask;    // optionally used alphamask data (if the alphamask flag is set).
    SPR_TILEMAP *tilemap;      // for tiled sprites!

    VD_SURFACE  *vs;           // surface this sprite is attached to.
    uint         physwidth;    // physical width of the sprite including buffers.

    SPR_API      api;

} SPRITE;


// SPRBLT_UNUSED:
//  Assign this constant to the red component of the fieldpos to disable
//  fieldpos and bitmask checking.  I assume this value is big enough that
//  it wouldn't interfere with normal values (heh).

#define SPRBLT_UNUSED 0xff


// =====================================================================================
    typedef struct SPR_BLITTER
// =====================================================================================
// A sprite driver is a procedure and a description of it's requirements and its capa-
// bilities.
//
// Notes:
//  - flags *must* match the flags of the sprite exactly; no exceptions.  Unfortunately
//    this means that a lot of blending blitters will have to duplicate structures for 
//    opaque and transparent sprites, even though the bitmap data is identical.
{
    struct SPR_BLITTER *next;
    
    uint        flags;         // flags required (alpha flag is non-required)
    uint        opacity;       // same as sprite->opacity.
    uint        cpu;           // CPU required for this renderer.

    // This info describes how this driver wants its sample data to be formatted,
    // and therefore it must match the format requirements of the surface this
    // blitter is being attached to.

    uint        bytespp;       // req. bytespp of the surface for this driver to work.
    VD_ATTRIB   fieldpos;      // pixel format it expects (optional - see SPRBLIT_UNUSED above)
    
    uint        align;         // required byte alignment of video surface
    uint        prebuf;        // size, in pixels, of the video surface and sprite left-side prefuffers 

    SPR_API     api;

} SPR_BLITTER;


extern void     gfxSpriteBlitter(const VD_SURFACE *vs, int xloc, int yloc, const SPRITE *info);
extern void     gfxTileOpaque(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
extern void     gfxTileTrans(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
extern void     gfxTileFunky(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);


extern void  (*sprblit_crossfade)(const struct SPRITE *spra, const struct SPRITE *sprb, int xloc, int yloc, uint alpha);

extern SPR_BLITTER *SprList_Blit16[8];
extern SPR_BLITTER *SprList_Blit32[8];


//  Standard Blitters
// -------------------
extern SPR_BLITTER Spr_Blit32Opaque;
extern SPR_BLITTER Spr_Blit16Opaque;

//  Additive Blitters
// -------------------
extern SPR_BLITTER SprAdd_Blit32Opaque, SprAddRGB_Blit32Opaque, SprAddBGR_Blit32Opaque;
extern SPR_BLITTER SprAdd_Blit16Opaque, SprAdd565_Blit32Opaque, SprAdd555_Blit32Opaque;

extern SPR_BLITTER SprAddRGB_Blit32Opaque, SprAddBGR_Blit32Opaque;
extern SPR_BLITTER SprAdd565_Blit16Opaque, SprAdd555_Blit16Opaque;

extern SPR_BLITTER SprAddRGB_MMX32Opaque, SprAddBGR_MMX32Opaque;
extern SPR_BLITTER SprAdd565_MMX16Opaque, SprAdd555_MMX16Opaque;


//  Alpha Blitters
// ----------------
//extern SPR_BLITTER SprAlpha_Blit32Opaque;
//extern SPR_BLITTER SprAlpha_Blit16Opaque;

//extern SPR_BLITTER SprAlphaRGB_Blit32Opaque, SprAlphaBGR_Blit32Opaque;
//extern SPR_BLITTER SprAlpha565_Blit16Opaque, SprAlpha555_Blit16Opaque;

//  Weighted Blend Blitters
// -------------------------
extern SPR_BLITTER SprBlend_Blit32Opaque, SprBlend_Blit32Trans;
extern SPR_BLITTER SprBlend_Blit16Opaque, SprBlend_Blit16Trans;

extern SPR_BLITTER SprBlendRGB_MMX32Opaque, SprBlendBGR_MMX32Trans;
extern SPR_BLITTER SprBlend565_MMX16Opaque, SprBlend555_MMX16Trans;


extern BOOL Sprite_Init(VD_SURFACE *vs, SPRITE *dest, int xsize, int ysize,  uint flags, uint opacity);
extern void Sprite_Free(SPRITE *sprite);
extern void Sprite_RegisterBlitter(VD_SURFACE *vs, SPR_BLITTER *sprdrv);
extern void Sprite_RegisterBlitterList(VD_SURFACE *vs, SPR_BLITTER *sprlst[]);

extern void sprblit_placebo(const SPRITE *spr, int xloc, int yloc);
extern void spralpha_placebo(const SPRITE *spr, int xloc, int yloc, uint alpha);
extern void sprcross_placebo(const SPRITE *spra, const SPRITE *sprb, int xloc, int yloc, uint alpha);

extern void Spr16_Crossfade(const SPRITE *spra, const SPRITE *sprb, int xloc, int yloc, uint alpha);
extern void Spr32_Crossfade(const SPRITE *spra, const SPRITE *sprb, int xloc, int yloc, uint alpha);

#endif
