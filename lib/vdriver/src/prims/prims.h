/*
 The use of this header file may seem a little tricky at first.  This was made
 to allow me to, with as little code changes as possible, make most of the basic
 sprite blitters needed for full software-emulation.

 Things you should define before including this header:

  BYTESPP
    Number of bytes per pixel.  This controls the UCAST type. 
    Only values 2 and 4 are supported.

  FORMAT_555
    This is only meaningful in 16 bit (BYTESPP == 2) mode.
    Alters the default mask and shift values for the 16 bit constants to
    support matrox-style 15 bit (5:5:5).

  FORMAT_BGR

 Things returned by this header:

  UCAST

  CASTIT

  TILE_BYTEWIDTH

  if bytespp == 2
    These are only meaningful (defined) in 16 bit mode.

    MASK_RED / MASK_BLUE / MASK_GREEN
    SHIFT_RED / SHIFT_BLUE / SHIFT_GREEN
      Mask and shift values configured for either the matrox-style 5:5:5 RGB
      or the standard 5:6:5 stuff.

  end if

  if bytespp == 4
    These are only meaningful (defined) in 32 bit mode.

    sprRGB (struct)
    sprBGR (struct)
      Structures which allow RGB ro BGR ordered access to the image and
      surface data.
  end if

*/
#ifndef _PRIMS_H_
#define _PRIMS_H_

#undef MASK_RED
#undef MASK_GREEN
#undef MASK_BLUE

#undef MASKHI_RED
#undef MASKHI_GREEN
#undef MASKHI_BLUE

#undef SHIFT_RED
#undef SHIFT_GREEN
#undef SHIFT_BLUE

#if (FORMAT == 0)
    #define MASK_RED       vs->mask.red
    #define MASK_GREEN     vs->mask.green
    #define MASK_BLUE      vs->mask.blue

    #define SHIFT_RED      vs->fieldpos.red
    #define SHIFT_GREEN    vs->fieldpos.green
    #define SHIFT_BLUE     vs->fieldpos.blue

    #define BITSHIFT_SET   { SPRBLT_UNUSED, 0, 0 }

    #if (BYTESPP == 2)
        #define recombine(r,g,b) ( (((r)>>8) & MASK_RED) | (((g)>>8) & MASK_GREEN) | (((b)>>8) & MASK_BLUE) )
    #else
        #define recombine(r,g,b) (vs->masktable_red[r/SPR_ALPHA_RANGE] | vs->masktable_green[g/SPR_ALPHA_RANGE] | vs->masktable_blue[b/SPR_ALPHA_RANGE])
    #endif
#endif

#if (BYTESPP == 2)

    // =============================
    //   ***  16 bit mode shit ***
    // =============================

    typedef UWORD UCAST;

    #ifndef BITSHIFT_SET

    #if (FORMAT == 555)
        #define SHIFT_RED    10
        #define SHIFT_GREEN   5
        #define SHIFT_BLUE    0

        #define SMASK_RED    0x1fl
        #define SMASK_GREEN  0x1fl
        #define SMASK_BLUE   0x1fl

    #else

        #define SHIFT_RED    11
        #define SHIFT_GREEN   5
        #define SHIFT_BLUE    0

        #define SMASK_RED    0x1fl
        #define SMASK_GREEN  0x3fl
        #define SMASK_BLUE   0x1fl

    #endif

        #define MASK_RED     (SMASK_RED   << SHIFT_RED)
        #define MASK_GREEN   (SMASK_GREEN << SHIFT_GREEN)
        #define MASK_BLUE    (SMASK_BLUE  << SHIFT_BLUE)

        #define MASKHI_RED   (SMASK_RED   << (SHIFT_RED   + 16))
        #define MASKHI_GREEN (SMASK_GREEN << (SHIFT_GREEN + 16))
        #define MASKHI_BLUE  (SMASK_BLUE  << (SHIFT_BLUE  + 16))

        #define BITSHIFT_SET { SHIFT_RED, SHIFT_GREEN, SHIFT_BLUE }

        #define recombine(r,g,b) ( (((r)>>8) & MASK_RED) | (((g)>>8) & MASK_GREEN) | ((b)>>8) )
    #endif


// =====================================================================================
    static void __inline alphabland(VD_SURFACE *vs, UCAST *btmp, UCAST *ttmp, uint len, uint alpha)
// =====================================================================================
{
    for(; len; len--, ttmp++, btmp++)
    {   
        int    r,g,b;

        r = (*ttmp & MASK_RED);
        g = (*ttmp & MASK_GREEN);
        b = (*ttmp & MASK_BLUE);

        r = (r*SPR_ALPHA_RANGE) + (((*btmp & MASK_RED)-r) * alpha);
        g = (g*SPR_ALPHA_RANGE) + (((*btmp & MASK_GREEN)-g) * alpha);
        b = (b*SPR_ALPHA_RANGE) + (((*btmp & MASK_BLUE)-b) * alpha);

        *ttmp = recombine(r,g,b);
    }
}


// =====================================================================================
    static void __inline crossfader(const VD_SURFACE *vs, const UCAST *btmp, const UCAST *ctmp, UCAST *ttmp, uint len, uint alpha)
// =====================================================================================
// Does an alpha blend between btmp and ctmp and puts the info in the destination ttmp.
{
    for(; len; len--, ttmp++, btmp++, ctmp++)
    {   
        int    r,g,b;

        r = (*ctmp & MASK_RED);
        g = (*ctmp & MASK_GREEN);
        b = (*ctmp & MASK_BLUE);

        r = (r*SPR_ALPHA_RANGE) + (((*btmp & MASK_RED)-r) * alpha);
        g = (g*SPR_ALPHA_RANGE) + (((*btmp & MASK_GREEN)-g) * alpha);
        b = (b*SPR_ALPHA_RANGE) + (((*btmp & MASK_BLUE)-b) * alpha);

        *ttmp = recombine(r,g,b);
    }
}


// =====================================================================================
    static void __inline blackfader(const VD_SURFACE *vs, const UCAST *btmp, UCAST *ttmp, uint len, uint alpha)
// =====================================================================================
// alpha blends the given source against black, then puts the result in the dest 'ttmp'
{
    for(; len; len--, ttmp++, btmp++)
    {   
        int    r,g,b;

        r = (*btmp & MASK_RED)   * alpha;
        g = (*btmp & MASK_GREEN) * alpha;
        b = (*btmp & MASK_BLUE)  * alpha;

        *ttmp = recombine(r,g,b);
    }
}

#else

// =============================
//   ***  32 bit mode shit ***
// =============================

    typedef ULONG UCAST;

    #pragma pack(1)
    typedef struct sprRGB
    {   
    #if (FORMAT == 321)
        UBYTE  alpha;
        UBYTE  r;
        UBYTE  g;
        UBYTE  b;
    #else
        UBYTE  b;
        UBYTE  g;
        UBYTE  r;
        UBYTE  alpha;
    #endif
    } sprRGB;
    #pragma pack()

    #ifndef BITSHIFT_SET
    #if (FORMAT == 321)
        #define BITSHIFT_SET { 0, 8, 16 }
    #else
        #define BITSHIFT_SET { 16, 8, 0 }
    #endif
    #endif

    #define recombine(r,g,b) (vs->masktable_red[r/SPR_ALPHA_RANGE] | vs->masktable_green[g/SPR_ALPHA_RANGE] | vs->masktable_blue[b/SPR_ALPHA_RANGE])

#endif

#define CASTIT(x) ((UCAST *)(x))
#define TILE_BYTEWIDTH (TILE_WIDTH * BYTESPP)


// =====================================================================================
    static void __inline surfacefade50(const VD_SURFACE *vs, UCAST *ttmp, uint len)
// =====================================================================================
// A completely platform independant inline surface fader.
{

    for(; len; len--, ttmp++)
    {
        *ttmp = (*ttmp & vs->alphamask[0]) >> 1;
    }
}



#endif
