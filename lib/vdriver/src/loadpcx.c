/*
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
 Module: loadpcx.c

  (Air's attempt at big and fancy comment header)
  
  This PCX loader supports 256 color and true color PCX's (the format does
  not support high-color, so naturally, neither does this loader :).  This
  should even support PCX MMSTREAMs with alpha channels, if such actually
  exists.
  
  The loader has been tested mildly, but seems to work fine on all PCX
  MMSTREAMs generated with Paint Shop Pro and Photoshop.  Considering the
  simplicity of the format and this loader, any problems encountered
  would no doubt be pretty easy to fix.

  NOTES on ENDIAN:
    If you are compiling this for use with a Big Endian (Mac, Alpha) machine,
    make sure to add the __BIG_ENDIAN__ definition.  Otherwise, the wrong
    read_SWORD procedure will be used and the PCX will not load properly.

  PCX_Test(MMSTREAM *imgfp)
     Checks the MMSTREAM for a valid PCX image header.  Returns 1 on failure.

  PCX_Load(MMSTREAM *fp)
     Loads the PCX MMSTREAM into an IMAGE struct.  Returns NULL on error.

  PCX_CleanUp(void)
     Should be called immidiately after PCX_Load, even if PCX_Load failed
     and returned NULL.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vdriver.h"
#include "image.h"

typedef struct PCXHEADER
{   SBYTE  manflag,           // Constant flag (10 = ZSoft .PCX)
           version,           // Version Number (only v5 supported)
           encode,            // 1 = PCX runlength encoding
           bitpix;            // # of bits/pixel per plane
    SWORD  xmin,ymin,         // x and y minimum dimensions
           xmax,ymax;         //  ..  .. maximum ..  ..
    SWORD  hres,vres;         // horizontal and vertical res of creating device
    SBYTE  colormap[48],      // EGA palette map (unused)
           reserved,          // take a guess, g-nut!
           numplanes;         // number of color planes
    SWORD  bytesperline,      // number of bytes/line per color plane
           paletteinfo;       // 1 = color/BW, 2 = greyscale
    SBYTE  filler[58];        // blank stuff rounds out 128 bytes!
} PCXHEADER;      


static PCXHEADER *pcx  = NULL;
static UBYTE     *scan = NULL;      // load a full scan at a time

BOOL PCX_Test(MMSTREAM *imgfp)
{
    UBYTE manflag,version,encode;

    manflag = _mm_read_UBYTE(imgfp);
    version = _mm_read_UBYTE(imgfp);
    encode  = _mm_read_UBYTE(imgfp);
    if((manflag != 10) || (version != 5) || (encode != 1)) return 0;

    return 1;
}

BOOL PCX_Init(void)
{
    return 1;
}

void PCX_Cleanup(void)
{
    _mm_free(pcx, "load_pcx handle");
    _mm_free(scan, "load_pcx scanline buffer");
}


BOOL PCX_Load(IMAGE *ir, MMSTREAM *imgfp)
{
    UBYTE  *bitmap, *curscan, c, c2;
    int    width, height, lala, i;

    if((pcx = (PCXHEADER *)_mm_malloc(sizeof(PCXHEADER))) == NULL) return 0;

    pcx->manflag = _mm_read_UBYTE(imgfp);
    pcx->version = _mm_read_UBYTE(imgfp);
    pcx->encode  = _mm_read_UBYTE(imgfp);
    pcx->bitpix  = _mm_read_UBYTE(imgfp);
    pcx->xmin    = _mm_read_I_SWORD(imgfp);
    pcx->ymin    = _mm_read_I_SWORD(imgfp);
    pcx->xmax    = _mm_read_I_SWORD(imgfp);
    pcx->ymax    = _mm_read_I_SWORD(imgfp);
    pcx->hres    = _mm_read_I_SWORD(imgfp);
    pcx->vres    = _mm_read_I_SWORD(imgfp);

    _mm_read_SBYTES(pcx->colormap,48,imgfp);

    pcx->reserved     = _mm_read_UBYTE(imgfp);
    pcx->numplanes    = _mm_read_UBYTE(imgfp);
    pcx->bytesperline = _mm_read_I_SWORD(imgfp);
    pcx->paletteinfo  = _mm_read_I_SWORD(imgfp);
    _mm_read_SBYTES(pcx->filler,58,imgfp);

    // check to make sure the header loaded

    if(_mm_feof(imgfp)) return 0;

    width  = (pcx->xmax - pcx->xmin) + 1;
    height = (pcx->ymax - pcx->ymin) + 1;

    if((bitmap=(UBYTE *)_mm_malloc(height * width * (ir->bytespp = pcx->numplanes))) == NULL) return 0;
    if((scan=(UBYTE *)_mm_malloc((pcx->bytesperline+1)*pcx->numplanes)) == NULL) return 0;

    ir->xsize  = width;
    ir->ysize  = height;
    ir->bitmap = bitmap;
    
    for(i=height; i; i--)
    {   int   i2,k;
        //for(k=0; k<pcx->numplanes; k++)
        {   lala    = pcx->bytesperline*3;
            curscan = scan; //&scan[k * pcx->bytesperline];
            do
            {   c = _mm_read_UBYTE(imgfp); 
                if((c & 0xc0) == 0xc0)
                {   c2 = _mm_read_UBYTE(imgfp);
                    for(i2=(c & 0x3f); i2 && lala; i2--, lala--, curscan++)
                       *curscan = c2;
                } else
                {   *curscan = c;
                    curscan++; lala--;
                }
            } while(lala);
        }

        curscan = scan;
        for(i2=width; i2; i2--, curscan++)
        {   for(k=pcx->numplanes; k; k--, bitmap++)
                *bitmap = curscan[(k-1) * pcx->bytesperline];
        }
    }   

   ir->flags     = 0;
   ir->bytespp   = (pcx->bitpix / 8) * pcx->numplanes;
   ir->bytewidth = ir->xsize * ir->bytespp;

   // ====================================
   // Check for a palette, and load it if one present.
   // Palette presence is determined by (a) PCX verson 5 and
   // (b) the presence of 12 dec at 768 bytes form the end of MMSTREAM.
   
   if((pcx->version == 5) && (ir->bytespp == 1))
   {   if((ir->palette = (UBYTE *)_mm_malloc(768)) == NULL)
       {   return 0;
       }
       _mm_fseek(imgfp,-769,SEEK_END);
       if(_mm_read_UBYTE(imgfp) == 12)
       {   for(i=0; i<768; i++)
              ir->palette[i] = _mm_read_UBYTE(imgfp);
       }
   }

   return 1;
}

IMAGE_LOADER load_pcx =
{   NULL,
    "PCX",
    "PCX Loader v0.3",
    PCX_Test,
    PCX_Init,
    PCX_Load,
    PCX_Cleanup
};

