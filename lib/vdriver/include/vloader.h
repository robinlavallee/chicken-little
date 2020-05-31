// ******************************************************
// * Disk and Processing Functions [load/save/convert]  *
// ******************************************************

#ifndef __VLOADER_H__
#define __VLOADER_H__

// loader structure:

typedef struct VL_IMAGELOADER
{   struct VL_IMAGELOADER *next;
    UBYTE    *type;
    UBYTE    *version;
    BOOL    (*Test)(MMSTREAM *fp);
    BOOL    (*Init)(void);
    BOOL    (*Load)(IMAGE *ir, MMSTREAM *fp);
    void    (*Cleanup)(void);
} VL_IMAGELOADER;


extern VL_IMAGELOADER load_pcx;
extern VL_IMAGELOADER load_bmp;

// Image loader function prototypes

extern IMAGE *VL_LoadImage(UBYTE *filename);
extern IMAGE *VL_LoadImageFP(MMSTREAM *fp);
extern void VL_RegisterImageLoader(VL_IMAGELOADER *ldr);
//extern void VL_InfoLoader(void);

extern void VL_Free(IMAGE *img);


extern VL_IMAGELOADER load_bmp;

#endif
