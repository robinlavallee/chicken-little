// ******************************************************
// * Disk and Processing Functions [load/save/convert]  *
// ******************************************************

#ifndef __IMAGE_H__
#define __IMAGE_H__

/*******************
 --> Structure :  IMAGE

 This structure is for design-time purposes only.  It is a somewhat limited
 format, but this has been done on purpose.  It is designed to be portable
 and workable - not fast or small. :)

 Bit values for "flags" :
   0     indexed color                 hi-color / true-color
   1-2   Transparent RLE information
          0 = Normal bitmapped storage
          1 = RLE (BYTE length command)
          2 = RLE (WORD length command)
          3 = RLE (DWORD length command)

 Notes:
  - If numcolors = 0, then the number of colors used in the image is un-
    known.

  - 16 bits per pixel data is always stored as 24bpp or 32bpp.

  - true-color (24+ bpp) data is always stored as RGB and RGBI.

  - The first four bytes (stored as an int) of an RLE sprite contain the ab-
    solute length of the RLE stream.
*/

#define IMG_INDEXED       1
#define IMG_TRANSPARENT   2
#define IMG_RLE           4

typedef struct IMAGE
{   uint    flags;
    uint    numcolors,    // total number of different colors used \  usually are the
            numpals;      // total number of entries in palette    /    same number
    uint    bytespp;      // bytes per pixel [if > 1, then numpals should ALWAYS be 0]
    uint    transparent;  // a single value to be considered transparent

    uint    xsize,ysize;  // these are in bytes please.
    uint    bytewidth,    // width of the scanline in bytes.
            bytesize;     // size of the image-data in bytes (used for loading / saving

    UBYTE  *bitmap;       // malloc'ed as bytesize
    UBYTE  *palette;      // malloc'ed as numpals * 3.

    uint    alpha;
    uint    gamma,
            dst_gamma;
} IMAGE;

// loader structure:

typedef struct IMAGE_LOADER
{   struct IMAGE_LOADER *next;
    UBYTE    *type;
    UBYTE    *version;
    BOOL    (*Test)(MMSTREAM *fp);
    BOOL    (*Init)(void);
    BOOL    (*Load)(IMAGE *ir, MMSTREAM *fp);
    void    (*Cleanup)(void);
} IMAGE_LOADER;


// Default distribution image loaders.

extern IMAGE_LOADER load_pcx;
extern IMAGE_LOADER load_bmp;

// Image loader function prototypes

extern IMAGE *Image_Load(CHAR *filename);
extern IMAGE *Image_LoadFP(MMSTREAM *fp);
extern void Image_RegisterLoader(IMAGE_LOADER *ldr);
//extern void VL_InfoLoader(void);
extern void Image_Free(IMAGE *img);

// ===============================
//  Image Manipulation Prototypes
// ===============================

extern IMAGE *Image_CutImage(const IMAGE *source, int x, int y, int dx, int dy);
extern IMAGE *Image_CutVidmem(const VD_SURFACE *vs, int x, int y, int dx, int dy);

extern BOOL  Image_ConvertTo32(IMAGE *image);
extern BOOL  Image_ConvertToTrans(IMAGE *src, uint r, uint g, uint b, BOOL funkyme);

extern BOOL  Image_to_Sprite(VD_SURFACE *vs, SPRITE *dest, const IMAGE *src, uint flags, uint aflags);

extern BOOL  Image_Identical(const IMAGE *imga, const IMAGE *imgb);


// ==========================
//  Image Display Prototypes
// ==========================

extern void  ImageDisp(const VD_SURFACE *vs, int xloc, int yloc, const IMAGE *image);
extern void  ImageDisp_Trans(const VD_SURFACE *vs, int xloc, int yloc, const IMAGE *info);
extern void  ImageDispEx(const VD_SURFACE *vs, const IMAGE *image, int xloc, int yloc, int xlen, int ylen,
                         int xdisp, int ydisp, int xscl, int yscl);


// > Defined in FUNKYIMAGE.C

extern int VD_FunkyEncode(IMAGE *image, uint color);

/*//int VD_W_FunkyEncode(IMAGE *image, VD_COLOR color);
extern BOOL VD_UnifyPalette(IMAGE *uni, IMAGE *input, int threshold);
//extern IMAGE *FMV_Encode(IMAGE *frm1, IMAGE *frm2, BOOL lossy, BOOL speed);
extern void FMV_DispImage(int xloc, int yloc, IMAGE *image);
extern void VD_OptimizePalette(IMAGE *src);
extern void VD_ReserveLowerColors(IMAGE *src, int reserved);*/


#endif
