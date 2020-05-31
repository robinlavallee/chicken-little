/*
  --> Divent FMV Animation File Playback Procedure

  Copyright © 1997 by Jake Stine and Divine Entertainment

  File: PLAY_FMV.C

  Functions for efficient playback of DivEnt's FMV format (.DEA files).

  For more information about the technical workings behind this run-length
  compression algorithm, see the text file RLE.TXT that is distributed
  with this source code.
*/

#include "vdriver.h"
#include <string.h>

#define FMV_OPAQUE     (1<<7)
#define FMV_COLOR      (1<<6)
#define FMV_FULLRUN    (1<<2)
#define FMV_TRANSCOLOR (1<<3)
#define FMV_TRANS      (1<<4)


void FMV_DispImage(int xloc, int yloc, IMAGE *image)
// Displays an image stored using our own internal FMV compression algorithm.
{
    UBYTE *src, *dest, *destmap;
    int   cc, i, xbytes, yi;
    int   transcolor;

    src  = image->bitmap;
    dest = MK_ScrnPtr(xloc,yloc);
    xbytes = image->bytespp * image->xsize;
    
    yi = image->ysize;
    do
    {   // bit 7 = Opaque flag
        // bit 6 = Color Run flag (present only if Opaque flag set!)
        // bit 5 = Full Scan flag (first run only!)

        cc = ((UBYTE *)src)[0]; src++;

        if(cc!=0)
        {   if(cc & FMV_FULLRUN)
            {   if(cc & FMV_OPAQUE)
                {   if(cc & FMV_COLOR)
                    {   memset(dest,*src,xbytes);
                        src++;
                    } else
                    {   memcpy(dest,src,xbytes);
                        src+=xbytes;
                    }
                }
            } else
            {   if(cc & FMV_TRANSCOLOR)
                {   transcolor = ((UBYTE *)src)[0]; src++;

                    destmap = dest;
                    i = ((UBYTE *)src)[0]; src++;
                    if(cc & FMV_OPAQUE)
                    {   if(cc & FMV_COLOR)
                        {   memset(destmap,*src,i);
                            src++;
                        } else
                        {   memcpy(destmap,src,i);
                            src     += i;
                        }
                    } else
                    {   memset(destmap,transcolor,i);
                    }
                    destmap += i;

                    do
                    {   cc = ((UBYTE *)src)[0]; src++;
                        if(cc==0) break;
    
                        if(cc & FMV_OPAQUE)
                        {   i = (cc & ~(FMV_COLOR | FMV_OPAQUE));
                            if(cc & FMV_COLOR)
                            {   memset(destmap,*src,i);
                                destmap += i;
                                src++;
                            } else
                            {   memcpy(destmap,src,i);
                                destmap += i;
                                src     += i;
                            }
                        } else
                        {   memset(destmap,transcolor,i=(cc & ~FMV_OPAQUE));
                            destmap += i;
                        }
                    } while(1);
                } else
                {   destmap = dest;
                    i = ((UBYTE *)src)[0]; src++;
                    if(cc & FMV_OPAQUE)
                    {   if(cc & FMV_COLOR)
                        {   memset(destmap,*src,i);
                            destmap += i;
                            src++;
                        } else
                        {   memcpy(destmap,src,i);
                            destmap += i;
                            src     += i;
                        }
                    } else
                        destmap += i;

                    do
                    {   cc = ((UBYTE *)src)[0]; src++;
                        if(cc==0) break;

                        if(cc & FMV_OPAQUE)
                        {   i = (cc & ~(FMV_COLOR | FMV_OPAQUE));
                            if(cc & FMV_COLOR)
                            {   memset(destmap,*src,i);
                                destmap += i;
                                src++;
                            } else
                            {   memcpy(destmap,src,i);
                                destmap += i;
                                src     += i;
                            }
                        } else
                            destmap += cc & ~FMV_OPAQUE;
                    } while(1);
                }
            }
        }
        dest += vd_bytewidth;
    } while(--yi);
}



