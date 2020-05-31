/*
 The VDRIVER - A Classic Video Game Engine

  By Jake Stine and Divine Entertainment (1997-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org
  For additional information and updates, see our website:
    http://www.divent.org

 Distribution/Rights:
  I could put things here that would make me sound like a first-rate california
  prune.  But I simply can't think of them right now.  In other news, I reserve
  the right to say I wrote my code.  All other rights are just abused legal
  tender in a world of red tape.  Have fun!

 ---------------------------------------------------
 Module: image.c

  Image loading and conversion API.
  Images feature file-type auto-detection and a common API for loading common
  image types in a pluggable fashion (PCX, GIF, etc).

 *Non-Game Content*
   If you are using anything within this module in your finished product
   then you aren't doing somehting right.

 New Features:
   Relatively multi-thread friendly now.  Boy, I know you all are just jumping
   on the opportunity to use THAT feature!

*/


#include "vdriver.h"
#include "image.h"
#include <string.h>


// ===========================================================================
// Image Loading API
//
//  This is a standard Divine Entertainment style loader layer.  The end user reg-
//  isters the loaders he will be using for the application in question, then uses
//  calls to Image_Load, Image_LoadFP to autodetect the filetype and load it using
//  the appropriate registered loader.
// ===========================================================================

static IMAGE_LOADER *list_imgload = NULL;

/*void VL_InfoLoader(void)
{
    int t;
    VLOADER *l;

    // list all registered devicedrivers:
    for(t=1,l=firstloader; l!=NULL; l=l->next, t++)
        printf("%d. %s\n",t,l->version);
}*/


void Image_RegisterLoader(IMAGE_LOADER *ldr)
{
    if(list_imgload == NULL)
    {   list_imgload = ldr;
        ldr->next    = NULL;
    } else
    {   ldr->next    = list_imgload;
        list_imgload = ldr;
    }
}


// ===========================================================================
    void Image_Free(IMAGE *img)
// ===========================================================================
{
   if(img)
   {   
       _mmlogd("Image > Unloading image...");
       _mm_free(img->bitmap, "bitmap");
       _mm_free(img->palette, "palette");
       _mm_free(img, "Done!");
   }
}


// ===========================================================================
    IMAGE *Image_LoadFP(MMSTREAM *fp)
// ===========================================================================
{
    IMAGE_LOADER *l;
    IMAGE   *mf;
    BOOL     ok;

    // Try to find a loader that recognizes the module
    for(l=list_imgload; l; l=l->next)
    {   _mm_rewind(fp);
        if(l->Test(fp)) break;
    }

    if(l == NULL)
    {   _mmerr_set(MMERR_UNSUPPORTED_FILE, "This is an unsupported image type or a corrupted file.");
        return NULL;
    }

    // We'll need some memory to load this thing into.
    if((mf=(IMAGE *)_mm_calloc(1,sizeof(IMAGE))) == NULL)
    {   return NULL;
    }

    if(l->Init())
    {  _mm_rewind(fp);
       ok = l->Load(mf, fp);
    }

    // free loader allocations even if error!
    l->Cleanup();

    // In this case, the called procedure sets our error.
    if(!ok) return NULL;

    return mf;
}


// ===========================================================================
    IMAGE *Image_Load(CHAR *filename)
// ===========================================================================
{
    MMSTREAM *fp;
    IMAGE    *mf;
    
    if((fp=_mm_fopen(filename,"rb"))==NULL) return NULL;

    mf = Image_LoadFP(fp);
    _mm_fclose(fp);

    return mf;
}


// ===========================================================================
// Image Manipulation API
//
//  Color and image conversion.  Image ripping from surfaces.  That sort of
//  wholesome brownie goodness.
// ===========================================================================

// ===========================================================================
    BOOL Image_to_Sprite(VD_SURFACE *vs, SPRITE *dest, const IMAGE *src, uint flags, uint aflags)
// ===========================================================================
// Converts an image to a sprite capable of being rendered to the specified
// display surface.  Involves altering the image bitdepth to match the
// surface and flags given.
{
    // The 'flags' indicates what type of stuff this sprite will be used for.
    // Sprites that will be rotated cannot be compressed.

    Sprite_Init(vs, dest, src->xsize, src->ysize, flags, aflags);

    if(!dest->bitmap) return 1;

    if(vs->bytespp == 4)
    {   // -- 32 bit destination formatting! --

        ULONG  *crap = (ULONG *)dest->bitmap;
        UBYTE  *slap = src->bitmap;
        uint    t;

        for(t=0; t<src->ysize; t++, crap+=dest->physwidth, slap+=src->bytewidth)
        {   uint   i;
            ULONG  *dbmp = crap;
            UBYTE  *sbmp = slap;

            for(i=0; i<src->xsize; i++, sbmp+=src->bytespp, dbmp++)
            {   *dbmp = VD_MakeColor(vs, sbmp[2], sbmp[1], sbmp[0]);
            }
        }
    } else if(vs->bytespp == 2)
    {   UWORD  *crap = (UWORD *)dest->bitmap;
        UBYTE  *slap = src->bitmap;
        uint    t;

        for(t=0; t<src->ysize; t++, crap+=dest->physwidth, slap+=src->bytewidth)
        {   uint   i;
            UWORD  *dbmp = crap;
            UBYTE  *sbmp = slap;

            for(i=0; i<src->xsize; i++, sbmp += src->bytespp, dbmp++)
            {   *dbmp = VD_MakeColor(vs, sbmp[2], sbmp[1], sbmp[0]);
            }
        }
    }

    return 0;
}


// ===========================================================================
    BOOL Image_ConvertTo32(IMAGE *src)
// ===========================================================================
// convert an image to 32 bit.  Cha-cha!
{
    UBYTE   *newmap;
    int      i, t, newbytewidth;

    if(!src) return 0;
    
    if(src->bytespp == 4) return -1;   // split if already right format.

    newbytewidth = src->xsize * 4;
    if((newmap = _mm_malloc(src->ysize * newbytewidth)) == NULL) return 0;

    if(src->bytespp == 3)
    {   // -- 32 bit destination formatting! --

        ULONG  *crap = (ULONG *)newmap;
        UBYTE  *slap = src->bitmap;
        uint    t;

        for(t=0; t<src->ysize; t++, crap+=src->xsize, slap+=src->bytewidth)
        {   uint   i;
            ULONG  *dbmp = crap;
            UBYTE  *sbmp = slap;

            for(i=0; i<src->xsize; i++, sbmp+=src->bytespp, dbmp++)
            {   *dbmp = sbmp[2] | (sbmp[1] << 8) | (sbmp[0] << 16);
            }
        }
    } else
    {   UBYTE  *oldmap;

        oldmap = src->bitmap;

        for(t=src->ysize; t; t--, newmap+=newbytewidth, oldmap+=src->bytewidth)
        {   UBYTE *nmap, *omap;
            for(i=src->xsize, nmap=newmap, omap=oldmap; i; i--, nmap+=4, omap++)
            {   nmap[3] = src->palette[(*oldmap * 3)];
                nmap[2] = src->palette[(*oldmap * 3) + 1];
                nmap[1] = src->palette[(*oldmap * 3) + 2];
                nmap[0] = 0;
            }
        }
        
        _mm_free(src->palette, "image palette (ConvertTo32)");
        src->numpals = 0;
    }

    _mm_free(src->bitmap, "source bitmap (ConvertTo32)");
    src->bitmap  = newmap;
    src->bytespp = 4;

    return -1;
}


// ===========================================================================
    BOOL Image_ConvertToTrans(IMAGE *src, uint r, uint g, uint b, BOOL funkyme)
// ===========================================================================
// Convert the image data to appropriate transparency, based on the key color
// provided.  If funkyme is set, then the image is RLE'd if it is deemed a
// suitable candidate.
// 
// Notes:
//  - This function assumes that the source data is already in 32 bpp
//     mode.  Perhaps I will add support for 16 bit later - if I ever need it.
{
    ULONG  *bits = (ULONG *)src->bitmap;
    uint    i;
    uint    color = r | (g << 8) | (b << 16);

    src->flags |= IMG_TRANSPARENT;

    if(funkyme)
    {    VD_FunkyEncode(src, color);

    } else
    {   for(i=0; i<src->ysize * src->xsize; i++, bits++)
            if(*bits == color) *bits = 0;
    }

    return 1;
}


// ===========================================================================
    IMAGE *Image_CutImage(const IMAGE *source, int x, int y, int dx, int dy)
// ===========================================================================
// Grabs an image from another image source of type IMAGE
// Mallocs a new IMAGE structure and returns it.
// Returned image has same bytes-per-pixel depth as source.
//
//  dx - x size of rectangle to cut  \  used as the xsize and ysize
//  dy - y size of rectangle to cut  /  parameters in returned image
{
    UBYTE   *offset,*destptr;
    IMAGE   *dest;

    dest           = (IMAGE *)_mm_calloc(1,sizeof(IMAGE));
    dest->bytespp  = source->bytespp;
    dest->xsize    = dx;
    dest->ysize    = dy;
    dest->bytesize = dx*dy*dest->bytespp;
    dest->flags    = 0;

    destptr = dest->bitmap = (UBYTE *)_mm_malloc(dest->bytesize);

    dy+=y;       
    for(; y<dy; y++, destptr+=dx*source->bytespp)
    {   offset = &source->bitmap[((y*source->xsize*source->bytespp)+(x*source->bytespp))];
        memcpy(destptr,offset,dx*source->bytespp);
    }

    return(dest);      
}


// ===========================================================================
    IMAGE *Image_CutSurface(const VD_SURFACE *vs, int x, int y, int dx, int dy)
// ===========================================================================
// Grabs an image from the vdriver video memory work-space (*vidmem).
// Mallocs a new IMAGE structure and returns it.
// Returned image has same bytes-per-pixel depth as the current screen mode.
//
//  dx - x size of rectangle to cut  \  used as the xsize and ysize
//  dy - y size of rectangle to cut  /  parameters in returned image
{
    UBYTE *offset,*destptr; 
    IMAGE *dest;

    dest    = (IMAGE *)_mm_malloc(sizeof(IMAGE));
    destptr = dest->bitmap = (UBYTE *)_mm_malloc(dx * dy * vs->bytespp);
    dest->bytespp = vs->bytespp;
    dest->xsize   = dx;
    dest->ysize   = dy;
    dest->flags   = 0;
   
    dy+=y;             
    for(; y<dy; y++, destptr+=dx)
    {   offset = MK_ScrnPtr(x,y);
        memcpy(destptr,offset,dx*vs->bytespp);
    }

    return(dest);
}



// ===========================================================================
    BOOL Image_Identical(const IMAGE *imga, const IMAGE *imgb)
// ===========================================================================
// compares the two images and returns TRUE if identical, FALSE if they are
// different somehow (either via header/dimensions/etc or by a pixel-by-pixel
// comparison).
{
    if(!memcmp(imga, imgb, sizeof(IMAGE)) && !memcmp(imga->bitmap, imgb->bitmap, imga->bytesize))
        return TRUE;
    else
        return FALSE;
}


// ===========================================================================
// Image Display API
//
//  All sorts of functions for putting an image onto a given surface.  Notice
//  The tricky change in function naming convention!  All image display func-
//  tions are ImageDisp prefix.  ImageDisp and ImageDispEx will probably be 
//  the procedures of most interest.
// ===========================================================================


// ===========================================================================
    void ImageDisp(const VD_SURFACE *vs, int xloc, int yloc, const IMAGE *image)
// ===========================================================================
// Displays an image of type IMAGE.  Displays opaque, transparent, or funky
// (byte and word based) images.

{
   UBYTE        *ttmp,*btmp;
   unsigned int itemp,ltemp,xtemp;

   ttmp  = MK_ScrnPtr(xloc*vs->bytespp,yloc);

   if(image->flags & IMG_RLE)
   {  //UBYTE       *see;
      //UBYTE        cc;
      //UBYTE        bb;    // RLE (funky) image blitter variables
      //UWORD        ww;    // one for each RLE storage method

      ltemp = image->ysize;
      itemp = image->xsize * vs->bytespp;
//      inc   = vs->bytewidth - itemp;
      btmp  = image->bitmap;

      switch(image->flags & IMG_RLE)
      {   /*case IMG_RLE_WORD:
              do
              {  cc = ((UWORD *)btmp)[0]; btmp++;
                 if(cc & 2)
                 {  if(cc & 1)
                    {  memcpy(ttmp,btmp,itemp);
                       btmp += itemp;
                    }
                 } else
                 {  if(cc & 1)
                    {  see = ttmp;
                       ww  = ((UWORD *)btmp)[0]; btmp+=sizeof(UWORD);
                       do
                       {  memcpy(see,btmp,ww);
                          see += ww;  btmp += ww;
                          ww = ((UWORD *)btmp)[0]; btmp+=sizeof(UWORD);
                          if(ww == 0) break;
                          see += ww;
                          ww = ((UWORD *)btmp)[0]; btmp+=sizeof(UWORD);
                       } while(1);
                    } else
                    {  see = ttmp;
                       ww  = ((UWORD *)btmp)[0]; btmp+=sizeof(UWORD);
                       do
                       {  see += ww;
                          ww = ((UWORD *)btmp)[0]; btmp+=sizeof(UWORD);
                          memcpy(see,btmp,ww);
                          see += ww;  btmp += ww;
                          ww = ((UWORD *)btmp)[0]; btmp+=sizeof(UWORD);
                          if(ww == 0) break;
                       } while(1);
                    }
                 }
                 ttmp += vs->bytewidth;
              } while(--ltemp);
          break;

          case IMG_RLE_LONG: break;*/
      }
   } else
   {  xtemp = image->bytewidth;
      itemp = image->ysize;
      btmp  = image->bitmap;

      do
      {  memcpy(ttmp, btmp, xtemp);
         btmp += xtemp;
         ttmp += vs->bytewidth;
      } while(--itemp);
   }
}


/*
Remind me why I am commented out?

void ImageDisp_Opaque(int xloc, int yloc, IMAGE *info)
{
   UBYTE *ttmp,*btmp;
   unsigned int itemp, ttemp;

   ttmp  = MK_ScrnPtr(xloc,yloc);
   btmp  = info->bitmap;
   itemp = info->ysize;
   ttemp = info->xsize*info->bytespp;                           

   do
   {  memcpy(ttmp, btmp, ttemp);
      btmp += ttemp;
      ttmp += vs->bytewidth;
   } while(--itemp);
}
*/

// ===========================================================================
    void ImageDisp_Trans(const VD_SURFACE *vs, int xloc, int yloc, const IMAGE *info)
// ===========================================================================
{
   UBYTE        *ttmp,*btmp;
   unsigned int itemp,inc;

    ttmp  = MK_ScrnPtr(xloc*vs->bytespp,yloc);
    btmp  = info->bitmap;
    itemp = info->ysize;
    inc   = vs->bytewidth - info->xsize*info->bytespp;
   
    do
    {   int    j, ltemp=info->xsize;
        UBYTE *ktmp = btmp;
        
        do
        {   for(j=info->bytespp; j; j--)
                if(ktmp[j]) break;
            if(j) memcpy(ttmp, ktmp, info->bytespp);

            ttmp += vs->bytespp;
            ktmp += info->bytespp;
        } while(--ltemp);

        ttmp += inc;
        btmp += info->bytewidth;
    } while(--itemp);
}


// ===========================================================================
    void ImageDispEx(const VD_SURFACE *vs, const IMAGE *image, int xloc, int yloc, int xlen, int ylen, int xdisp, int ydisp, int xscl, int yscl)
// ===========================================================================
//  Does everything one coupld possibly want (and is pretty slow at it too, but
//  that is beside the point :).
//
//  Images are assumed to be in the same bytes-per-pixel format as the display.
//  Supports images with inline alpha channels (but does not yet utilize the
//  alpha channel)
//
//  Doesn't support RLE images, but will in the near future, no doubt!
//
//  xlen,  ylen  = used for clipping on the bottom and right sides
//                 (length of display window, essentially)
//  xdisp, ydisp = used for clipping on the upper and left sides
//                 (start of x and y display inside the picture, essentially)
//  xscl,  yscl  = Scaling factor.  Big numbers = big images. (8 bit fixed)
//  color        = color of transparency [0xffffffff = no transparency];
{
    UBYTE *scrn, *img;
    uint   i, k, ack, xsize;
    uint   ilenx, ileny;
    ULONG  fsclx, fscly, xcnt, ycnt;
    ULONG  col;

    scrn = MK_ScrnPtr(xloc,yloc);

    xsize  = image->xsize * image->bytespp;
    xdisp *= image->bytespp;
    fsclx  = ((ULONG)(1<<16) * image->bytespp) / xscl;
    fscly  = (ULONG)(1<<16) / yscl;
    ilenx  = (xlen * fsclx) >> 8;
    ileny  = (ylen * fscly) >> 8;

    if((xdisp + ilenx) > xsize) xlen = ((xsize - xdisp) * xscl) >> 8;
    if((ydisp + ileny) > image->ysize) ylen = ((image->ysize - ydisp) * yscl) >> 8;

    ack = vs->bytewidth - xlen;

    for(i=ylen, ycnt=0; i; i--, scrn+=ack)                        
    {   img  = &image->bitmap[(ydisp*xsize) + (xdisp*image->bytespp)];
        xcnt = 0;

        if(image->flags & IMG_TRANSPARENT)
        {   uint  j;
            for(k=xlen; k; k--, scrn++)
            {   col = image->transparent;
                for(j=vs->bytespp; j; j--, col>>=8)
                    if(img[j] != col) break;

                if(j) for(j=0; j<vs->bytespp; j--) *scrn++ = img[j];
                
                xcnt += fsclx;
                img   = &img[xcnt >> 8];
                xcnt &= (1<<10)-1;
            }
        } else
        {   for(k=xlen; k; k--, scrn++)
            {   *scrn = *img;
                xcnt += fsclx;
                img   = &img[xcnt >> 8];
                xcnt &= (1<<10)-1;
            }
        }
        ycnt += fscly;
        (ydisp += (ycnt / 1024));
        ycnt &= (1<<10)-1;
    }
}

