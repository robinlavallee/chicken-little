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
 Module: cvt_rle.c

 Transparency RLE Encoding of Sprites and Tiles
 Using Air Richter's RLE Funky Technology

 Conversion functions for encoding the transparent runs of sprites and tiles.
 Functions in this module use the BYTE version of my own internal RLE algorithm
 (see below).

 For more information about the technical workings behind this run-length
 compression algorithm, see the text file RLE.TXT that is distributed
 with this source code.

 Notes:
  - This code only works with 32 bit source images!
*/

#include "vdriver.h"
#include "image.h"
#include <string.h>
#include "mminline.h"

#ifdef _LOGME_
#define _debuglog _mmlog
#else
void __inline _debuglog(CHAR *crap, ...)
{
}
#endif


#define WORKSPACE 16384


// ===========================================================================
    int VD_FunkyEncode(IMAGE *image, uint color)
// ===========================================================================
// Recieves an image of type IMAGE and encodes the image for transparent
// runs using Air Richter's Funky RLE Technology.  The original *bitmap
// of the passed image is destroyed and replaced with a new compressed ver-
// sion.
//
// color:  color value to be treated as transparent
//
// Returns: 1  if an error occured.
//          -1 warning - RLE'd image has a high percentage of runs and is
//                       not a good RLE (unoptimal for size or speed).
{
    ULONG  *workspace,*srcmap,*newmap,*oldmap,*tptr;
    int     cnt,runcnt;         // size of the RLE in dwords, # of runs total
    int     scancnt,wscnt;      // scan counter (image width), workspace counter
    BOOL    currun,             // set = Opaque; clear = Transparent
            fullrun;            // cleared if > 1 run on current scan
    uint    runlen;             // length of current run

    int     precnt;             // assigned == cnt after the preprocessor completes.
                                // used to check for errors.

    ULONG   ww;
    uint    yi;

    if(image->flags & IMG_RLE) return 0;    // return if image is already RLE'd
   
    cnt = 0; currun = runcnt = wscnt = 0;
    if((workspace = (ULONG *)_mm_malloc(WORKSPACE*4)) == NULL) return 1;

    // Pre-processor.
    // Establishes the exact length of the output file and tracks run info
    // for more efficient processing the second time through.  (done mainly
    // to avoid overuse of realloc.

    oldmap = (ULONG *)image->bitmap;
    tptr   = workspace;

    for(yi=image->ysize; yi; yi--)
    {   // Process the first pixel to determine first run's type
        tptr[0] = currun = 0;
        if(*oldmap != color)
        {   *tptr = currun = 1;
            cnt++;
        }

        cnt++;  tptr++;  wscnt++;  oldmap++;
        runlen = 1;

        _debuglog("----------- Scan %d, cnt = %d ---------------",image->ysize-yi,cnt);
        _debuglog("First run type is : %d",currun);

        // Process the rest of the pixels in the current scan
      
        scancnt = image->xsize-1;
        fullrun = 1;        // Cleared if > 1 run on the scan
        do
        {   if(*oldmap != color)
            {   if(!currun)
                {   // run change from transparent to opaque
                    _debuglog("  Encoding Transparent run...  runlen = %d", runlen);
                    *tptr = runlen;
                    cnt++; tptr++; wscnt++;
                    runcnt++;
                    currun = 1;
                    runlen = fullrun = 0;
                }
            } else
            {   if(currun)
                {   // run change from opaque to transparent
                    _debuglog("  Encoding Opaque run...  runlen = %d", runlen);
                    *tptr = runlen;
                    cnt++; tptr++; wscnt++;
                    runcnt++;
                    currun = runlen = fullrun = 0;
                }
            }

            oldmap++; 
            runlen++;
            if(currun) cnt++;

            if(wscnt > WORKSPACE - 10)
            {   _mm_free(workspace, NULL);
                _debuglog("Workspace buffer is full.  Encoding aborted");
                return 1;
            }
        } while(--scancnt);

        // Store information for the last run of the scan.
        if(fullrun)
        {   // A full scan run - set the proper bit
            _debuglog("Encoding Full Run...");
            tptr[-1]+=2;  // tptr++;
        } else
        {   // If an opaque run, throw in runlength.
            if(currun)
            {   _debuglog("  Encoding Opaque run...  runlen = %d", runlen);
                *tptr = runlen;
                cnt++; tptr++; wscnt++;
                runcnt++;
            }
            // Throw in Zero Marker (termination of scanline)
            *tptr = 0;
            cnt++; tptr++; wscnt++;
            _debuglog("  Scanline Termination: cnt = %d ", cnt);
        }
    }

    //_mmlog("Preprocessor Complete > cnt = %d",cnt);

    precnt = cnt;

    // Pre-processor complete - malloc final buffer.

    image->bytesize = cnt*sizeof(ULONG);

    if((newmap = (ULONG *)_mm_malloc(image->bytesize)) == NULL)
    {   _mm_free(workspace, NULL);
        return 1;
    }

    // Main Processor.  Encodes the sprite into the new buffer using a variation
    // of the decoder (neat, eh? :)

    oldmap = (ULONG *)image->bitmap;
    tptr   = workspace;

    // The commented code here is the old y-index generation, which is now done
    // at load-time!
    //image->bytesize = (cnt+image->ysize)*sizeof(ULONG) + 4;   // this went up above the malloc
    //newmap[0] = image->bytesize;
    //cnt    = image->ysize+1;

    cnt = 0;

    yi = 1;
    do
    {   // bit 0   set = opaque; clear = transparent
        // bit 1   set = no type change; clear = type changes
        uint cc = *tptr;
      
        //newmap[yi] = cnt;         // write the index for this scan,
        
        tptr++;
        newmap[cnt++] = cc;
      
        _debuglog("----------- Scan %d, cnt = %d ---------------",image->ysize-yi,cnt);

        if(cc & 2)
        {   // -- Complete Scanline --
            if(cc & 1)
            {   // Complete Opaque.  Copy.
                _mm_memcpy_long(&newmap[cnt],oldmap,image->xsize);
                cnt += image->xsize;
                _debuglog("  FullRun Decode...");
            }
        } else
        {   if(cc & 1)
            {   // First run is opaque.  Copy scans in an even-odd fashion.
                // The first is copied, the second is skipped, so on.

                srcmap = oldmap;
            
                ww  = *tptr;
                newmap[cnt] = *tptr;
                tptr++; cnt++;

                do
                {   _debuglog("  Opaque Run... ww = %d",ww);
                    _mm_memcpy_long(&newmap[cnt],srcmap,ww);  // Copy an opaque run
                    cnt += ww;  srcmap += ww;

                    ww = *tptr;                      // Get next runlength
                    newmap[cnt] = *tptr;
                    tptr++;  cnt++;

                    if(ww == 0) break;               // 0 runlength = end of scan
                    srcmap += ww;                    // skip the transparent run
                    _debuglog("  Transparent Run... ww = %d",ww);

                    ww = *tptr;                      // get next runlength
                    newmap[cnt] = *tptr;
                    tptr++;  cnt++;
                } while(1);
            } else
            {   srcmap = oldmap;

                ww = *tptr;                          // Get next runlength
                newmap[cnt] = *tptr;
                tptr++;  cnt++;

                do
                {   srcmap += ww;                    // skip the transparent run
                    _debuglog("  Transparent Run... ww = %d",ww);
               
                    ww = *tptr;                      // Get next runlength
                    newmap[cnt] = *tptr;
                    tptr++;  cnt++;

                    _debuglog("  Opaque Run... ww = %d",ww);
                    _mm_memcpy_long(&newmap[cnt],srcmap,ww);  // copy an opaque run
                    cnt += ww;  srcmap += ww;

                    ww = *tptr;                      // Get next runlength
                    newmap[cnt] = *tptr;
                    tptr++;  cnt++;

                    if(ww == 0) break;               // 0 runlength = end of scan
                } while(1);
            }
        } 
        oldmap += image->xsize;
    } while(yi++ < image->ysize);

    if(precnt != cnt)
       _mmlogv("Funky-Encoder count mismatch > precnt = %d   postcnt = %d",precnt, cnt);

    _mm_free(image->bitmap, "original bitmap");
    _mm_free(workspace, "run workspace");

    image->bitmap = (UBYTE *)newmap;
    image->flags |= IMG_RLE;

    return 0;
}

