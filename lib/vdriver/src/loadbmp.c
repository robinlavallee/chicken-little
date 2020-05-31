/*
 The VDRIVER - A Classic Video Game Engine

  By Jake Stine and Divine Entertainment (1997-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org
  For additional information and updates, see our website:
    http://www.divent.org

 Disclaimer:
  I could put things here that would make me sound like a first-rate california
  prune.  But I simply can't think of them right now.  In other news, I reserve
  the right to say I wrote my code.  All other rights are just abused legal
  tender in a world of red tape.  Have fun!

 ---------------------------------------------------
 Module: loadbmp.c

  Loads Windows Device Independant Bitmaps.. A dumb loader to IMAGE.  It loads
  the file in the bit depth of the original disk image and all that crap.

*/


#include "vdriver.h"
#include "image.h"
#include <string.h>


typedef struct BMPHEADER
{   SWORD   filetype;
    ULONG   filesize;
    SWORD   reserved1;
    SWORD   reserved2;
    ULONG   offbits;
    ULONG   size;
    SLONG   width;
    SLONG   height;
    SWORD   planes;
    SWORD   bitcount;
    ULONG   compress;
    ULONG   sizeimage;
    SLONG   xppmeter;
    SLONG   yppmeter;
    ULONG   clrsused;
    ULONG   clrsimportant;
} BMPHEADER;


static BMPHEADER *bmp = NULL;

BOOL BMP_Test(MMSTREAM *imgfile)
{
    if(_mm_read_M_UWORD(imgfile) == 0x424d) return 1;
    return 0;
}

BOOL BMP_Init(void)
{
    if((bmp = (BMPHEADER *)_mm_malloc(sizeof(BMPHEADER))) == NULL) return 0;
    return 1;
}


void BMP_Cleanup(void)
{
   _mm_free(bmp, "load_bmp handle");
}


#define bmp_read(type) (headsize ? _mm_read_##type(imgfile) : 0); if(headsize) headsize -= 4

#define BI_RGB  0
#define BI_RLE8 1


BOOL BMP_Load(IMAGE *ir, MMSTREAM *imgfile)
{
   UBYTE   *bitmap;
   ULONG   i, headsize;
   BOOL    reverse;

   // load the BMP 'file header'

   bmp->filetype   = _mm_read_I_SWORD(imgfile);
   bmp->filesize   = _mm_read_I_ULONG(imgfile);
   bmp->reserved1  = _mm_read_I_SWORD(imgfile);
   bmp->reserved2  = _mm_read_I_SWORD(imgfile);
   bmp->offbits    = _mm_read_I_ULONG(imgfile);

   // load the BMP 'Image Header'
   // An image header can have three known versions, with the only difference
   // between them indicated by the header size.

   bmp->size       = _mm_read_I_ULONG(imgfile);
   bmp->width      = _mm_read_I_SLONG(imgfile); 
   bmp->height     = _mm_read_I_SLONG(imgfile);
   bmp->planes     = _mm_read_I_SWORD(imgfile);
   bmp->bitcount   = _mm_read_I_SWORD(imgfile);
   headsize = bmp->size - 16;

   bmp->compress   = bmp_read(I_ULONG);
   bmp->sizeimage  = bmp_read(I_ULONG);
   bmp->xppmeter   = bmp_read(I_SLONG);
   bmp->yppmeter   = bmp_read(I_SLONG);
   bmp->clrsused   = bmp_read(I_ULONG);
   bmp->clrsimportant = bmp_read(I_ULONG);

   // skip past any other unsupported header info
   if(headsize > 0) _mm_fseek(imgfile, headsize, SEEK_CUR);

   ir->xsize = bmp->width;
   if(bmp->height < 0)
   {   ir->ysize = -bmp->height;
       reverse = 0;
   } else
   {   ir->ysize = bmp->height;
       reverse = 1;
   }

   // Load the palette information

    if(bmp->clrsused==0)
    {   ir->bytespp = (bmp->bitcount / 8);
    } else
    {   ir->numpals = bmp->clrsused;
        if((ir->palette = (UBYTE *)_mm_malloc(ir->numpals*3)) == NULL) return 0;
        for(i=0; i<ir->numpals*3; i+=3)
        {   ir->palette[i]   = _mm_read_UBYTE(imgfile);
            ir->palette[i+1] = _mm_read_UBYTE(imgfile);
            ir->palette[i+2] = _mm_read_UBYTE(imgfile);
            _mm_read_UBYTE(imgfile);
        }
    }

    // Determine the bytewidth.  make sure for dword alignment.
   
    ir->bytewidth = (ir->xsize*ir->bytespp + 3) & ~3;
   
    // Load the actual bitmap data

    if((ir->bitmap=bitmap=(UBYTE *)_mm_malloc(ir->bytewidth * ir->ysize)) == NULL) return 0;
    bitmap += ir->bytewidth * (ir->ysize-1);

    if(bmp->compress == BI_RGB)
    {   // No compression, read image data flat out, but bottom-up
        for(i=0; i<ir->ysize; i++, bitmap-=ir->bytewidth)
           _mm_read_UBYTES(bitmap,ir->bytewidth,imgfile);
    }

    ir->flags  = 0;

    return 1;
}

IMAGE_LOADER load_bmp =
{   NULL,
    "BMP",
    "BMP Loader v0.4",
    BMP_Test,
    BMP_Init,
    BMP_Load,
    BMP_Cleanup
};

