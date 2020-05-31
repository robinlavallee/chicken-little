/*
  --> Transparency RLE Encoding of Full Motion Video
   -> Using Air Richter's RLE Funky Technology

  Copyright © 1997 by Jake Stine and Divine Entertainment

  File: CVT_FMV.C

  This module is for the encoding of standard bitmap images into full-
  motion video sequences.  It uses a WORD version of my own internal
  run-length encoding of transparent (unchanged) areas of each frame (see
  below) to achieve adequate compression and outstanding speed perform-
  ance.  See CVT_RLE.C for the BYTE version of my RLE algorithm, which
  is better suited for smaller sprite and tile images.

  For more information about the technical workings behind this run-length
  compression algorithm, see the text file RLE.TXT that is distributed
  with this source code.
*/

#include "vdriver.h"
#include <string.h>

// The RUNTYPE is the integeral type to use when storing the image's run-
// length encoding.  Legal values would be any of the unsigned integer-based
// types: UBYTE(char), UWORD(short), or ULONG(long)
typedef UBYTE RUNTYPE;

// The maximum workspace for RLE-encoder pre-processing.  Approx 2 bytes
// are needed for each run.
#define WORKSPACE 32760


#define FMV_OPAQUE     (1<<7)
#define FMV_COLOR      (1<<6)
#define FMV_FULLRUN    (1<<2)
#define FMV_TRANSCOLOR (1<<3)
#define FMV_TRANS      (1<<4)

#define MAX_RUNLEN     63
#define HALF_RUNLEN    31
#define TWICE_RUNLEN  127
#define FULL_RUNLEN   255


ULONG GetCommonValue(UBYTE *data, int len, int size);
ULONG GetUsedColors(ULONG *dest, UBYTE *data, int len, int size);


int colorcnt[256];

ULONG GetCommonValue(UBYTE *data, int len, int size)
// Determines the most-used value in a string of values of the given bytesize
// Supports only byte values currently!
{
    int   i,j,ret;

    memset(colorcnt,0,256*sizeof(int));

    for(; len; len--, data+=size)
       colorcnt[*data]++;

    j=0;
    for(i=0; i<256; i++)
    {   if(colorcnt[i] > j)
        {   j = colorcnt[i];
            ret = i;
        }
    }

    return ret;
}


ULONG GetUsedColors(ULONG *dest, UBYTE *data, int len, int size)
// Determines the most-used value in a string of values of the given bytesize
// Supports only byte values currently!
{
    int   j;

    j=0;
    memset(colorcnt,0,256*sizeof(int));

    for(; len; len--, data+=size)
    {   if(colorcnt[*data] != 1)
        {   colorcnt[*data] = 1;
            *dest = *data;
            dest++; j++;
        }
    }

    return j;
}


int FMV_FunkyEncode(IMAGE *image, IMAGE *lastframe, BOOL lossy, BOOL speed)

// Recieves an image of type IMAGE and encodes the image for transparent
// runs using Air Richter's Funky RLE Technology.  The original *bitmap
// of the passed image is destroyed and replaced with a new compressed ver-
// sion.  See Funky_Decode for an image-based decoder, and SpriteBlitter [in
// GAMEGFX.C] for a sprite-based decoder.
//
// color:  color value to be treated as transparent
//
// Returns: 1  if an error occured.
//          -1 warning - RLE'd image has a high percentage of runs and is
//                       not a good RLE (unoptimal for size or speed).
{
   UBYTE *workspace,*srcmap,*newmap,*oldmap,*tptr,*color,*workptr;
   int   runlen;             // lenght of current run
   int   xbytes,bytespp;     // length of scan in bytes; bytes per pixel of images
   int   cnt,fullcnt;        // size of the RLE in bytes
   int   col;
   int   scancnt;            // scan counter (image width), workspace counter
   int   i,jj,samerun;       // misc vars; length of current same-color run
   BOOL  currun,             // 0 = trans, 1 = opaque  .. 0 = opaque, 1 = color
         fullrun;            // cleared if > 1 run on current scan
   int   ww, yi, cc, best;
   int   scancolor;
   int   wscnt,wstmp;

   BOOL  stopout;
   ULONG *idxclr;            // list of colors actually used in the animation
   int   pntman,
         pass = 0;           // pass counter.  Each pass uses different opt techniques.
                             // pass 0 = std transparent encoding.
                             // pass 1 = transparency against a color, not prev image.
                             // pass 2 = ignore transparency - color-run encoding only.


   if(image->flags & IMG_RLE) return 0;    // return if image is already RLE'd
   
   fullcnt = currun = wstmp = 0;
   if((workspace = (UBYTE *)_mm_malloc(WORKSPACE)) == NULL) return 1;
   idxclr = (ULONG *)_mm_malloc(256*sizeof(ULONG));

   // Pre-processor.  Establishes the exact length of the output file and
   // tracks run info for more efficient processing the second time through.
   // (done mainly to avoid the evil realloc, which always seems to cause
   // problems for me :)

   xbytes  = image->xsize * image->bytespp;
   srcmap  = image->bitmap;
   bytespp = image->bytespp;
   newmap  = lastframe->bitmap;
   workptr = workspace;

   for(yi=image->ysize; yi; yi--)
   {   ww = 0x7fff; stopout = 0;

       if(speed)
       {   pass = 3;
           pntman = 10;
       } else
       {   pntman = GetUsedColors(idxclr,oldmap,image->xsize,image->bytespp);
           pass   = (pntman * 2) + 1;
           pntman += 2;
       }

       while(pass >= 0)
       {   // Prepare for the current pass.
           if(pass>2)
           {   // ColorPasses - determine the most used color, and set that as
               // the background / transparency.
               scancolor = speed ? GetCommonValue(oldmap,image->xsize,image->bytespp) : idxclr[pass>pntman ? (pass-pntman) : (pass - 3)];
           }

           cnt = 0;
           wscnt  = wstmp;   tptr  = workptr;
           oldmap = srcmap;  color = newmap;

           // Process the first pixel to determine first run's type
           if(pass>2) col = scancolor;  else col = color[0];
           for(jj=0; jj<bytespp; jj++, col>>=8)
           {   if(oldmap[jj] != col & 255)
               {   currun = 1;
                   break;
               }
           }
           if(jj==bytespp)
           {   if(pass>2) col = scancolor;  else col = color[1];
               for(; jj<bytespp*2; jj++, col>>=8)
               {   if(oldmap[jj] != col & 255) break;
               }
               if(jj==bytespp*2) currun = 0; else currun = 1;
           }

           #ifdef LOG_ME
           if(stopout) printlog("----------- Scan %d, pass = %d,  cnt = %d ---------------",240-yi,pass,fullcnt);
           if(stopout) printlog("First run type is : %d",currun);
           #endif
           oldmap += bytespp;
           color  += bytespp;
           runlen  = bytespp;
           if(pass!=2 && pass<=pntman) samerun = bytespp; else samerun=0;  // manages the current runlength (if any) of same-colored pixels

           // Process the rest of the pixels in the current scan        
           scancnt = image->xsize-1;
           fullrun = 1;         // Cleared if > 1 run on the scan
        
           do
           {  // =================================================================
              // Check for transparent to opaque run-change and store new runs
              // if new runs are ready to be stored.  Note that transparent runs
              // take priority over color-runs!

              if(pass>2) col = scancolor; else col = *color;
              for(jj=0; jj<bytespp; jj++, col>>=8)
              {   if(oldmap[jj] != col & 255)
                  {   if(!currun)
                      {   // Check the opaque runlen .. if it's only one, it could
                          // be a good opportunity to do some lossy compression
                          /*canceled = 0;
                          if(lossy)
                          {   col = color[1];
                              for(jj=bytespp; jj<bytespp*2; jj++, col>>=8)
                                 if(oldmap[jj] != col & 255) break;
        
                              if(jj==bytespp*2)
                              {   col = *color  * 3;
                                  i   = *oldmap * 3;
                                  if((abs(lastframe->palette[col]-image->palette[i]) < 6) &&
                                     (abs(lastframe->palette[col+1]-image->palette[i+1]) < 6) &&
                                     (abs(lastframe->palette[col+2]-image->palette[i+2]) < 6))
                                  {   runlen++;
                                      canceled = 1;
                                  }
                              }
                          }*/

                          if(pass!=1)
                          {   // Indeed, it's a run change from transparent to opaque, so
                              // save out the previous run information.
        
                              // --- Encode a transparent run ---
                              #ifdef LOG_ME
                              if(stopout) printlog("  Encoding Transparent run...  runlen = %d", runlen);
                              #endif
                              if(fullrun == 1)
                              {
                                  #ifdef LOG_ME
                                  if(stopout) printlog("  > First Run encode. runlen = %d; cnt = %d",runlen,cnt);
                                  #endif
                                  *tptr = FMV_TRANS;
                                  if(pass>2)
                                  {   // Pass Type 1 .. assign the transparent color type
                                      *tptr |= FMV_TRANSCOLOR; tptr++; cnt++; wscnt++;
                                      *tptr = scancolor;
                                      tptr+=bytespp; cnt+=bytespp; wscnt+=bytespp;
                                  } else
                                  {   tptr++; cnt++; wscnt++;  }

                                  i = (runlen > FULL_RUNLEN) ? FULL_RUNLEN : runlen;
                                  ((RUNTYPE *)tptr)[0] = i;
                                  runlen -= i;
                                  cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                              }
                              // Note that transparent runs have TWICE the MAX_RUNLEN!
                              while(runlen > 0)
                              {   i=(runlen > TWICE_RUNLEN ? TWICE_RUNLEN : runlen);
                                  ((RUNTYPE *)tptr)[0] = i;
                                  #ifdef LOG_ME
                                  if(stopout) printlog("  > MaxRuns encode. runlen = %d; cnt = %d",runlen,cnt);
                                  #endif
                                  runlen-=i;
                                  cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                              }
                              fullrun = 0;
                              runlen  = 0;
                              samerun = 0;
                          }
                          currun  = 1;
                      }
                      break;
                  }
              }
        
        
              // =========================================================
              // Check for a run change from opaque to transparent.  (no
              // priority :)  Ignore check if this is the last pixel in the
              // scan.
        
              if(jj==bytespp && currun /*&& scancnt > 1*/)
              {   // Check the next pixel to make sure this run is long enough
                  if(pass>2) col = scancolor; else col = color[1];
                  for(; jj<bytespp*2; jj++, col>>=8)
                     if(oldmap[jj] != col & 255) break;
                 
                  if(jj==bytespp*2)
                  {   // Scan is long enough, so run change from opaque to transparent
                      if(samerun>0)
                      {   // We have a same-color run we can store, so modify runlen to
                          // store only the opaque in front
                          runlen -= samerun;
                      } else samerun = 0;
        
                      // --- Encode an opaque run ---
                      if(runlen > 0)
                      {
                          #ifdef LOG_ME
                          if(stopout) printlog("  Encoding Opaque run...");
                          #endif
        
                          if(fullrun == 1)
                          {   // first run has a max length half of MAX_RUNLEN
                              *tptr = FMV_OPAQUE;
                              if(pass>2)
                              {   // Pass Type 1 .. assign the transparent color type
                                  *tptr |= FMV_TRANSCOLOR; tptr++; cnt++; wscnt++;
                                  *tptr = scancolor;
                                  tptr+=bytespp; cnt+=bytespp; wscnt+=bytespp;
                              } else
                              {   tptr++;  cnt++;  wscnt++;  }

                              i = (runlen > FULL_RUNLEN) ? FULL_RUNLEN : runlen;
                              #ifdef LOG_ME
                              if(stopout) printlog("  > First Run encode. runlen = %d; cnt = %d",runlen,cnt);
                              #endif
                              ((RUNTYPE *)tptr)[0] = i;
                              runlen -= i;
                              cnt+=i;
                              cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                          }
                          while(runlen >= MAX_RUNLEN)
                          {   //i = runlen > MAX_RUNLEN ? MAX_RUNLEN : runlen;
                              ((RUNTYPE *)tptr)[0] = MAX_RUNLEN + FMV_OPAQUE;
                              #ifdef LOG_ME
                              if(stopout) printlog("  > MaxRuns encode. runlen = %d; cnt = %d",runlen,cnt);
                              #endif
                              runlen-=MAX_RUNLEN;
                              cnt+=MAX_RUNLEN;
                              cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                          }

                          // Check for a near-color match - if one, just stream this with
                          // the next color run
                          if(pass!=1 && runlen == bytespp && lossy)
                          {   // Use lossy compression to sap an extra byte
                              if(samerun > 1)
                              {   /*col = oldmap[-1] * 3;
                                  i   = oldmap[-(samerun)] * 3;
                                  if((abs(image->palette[col]-image->palette[i]) < 6) &&
                                     (abs(image->palette[col+1]-image->palette[i+1]) < 6) &&
                                     (abs(image->palette[col+2]-image->palette[i+2]) < 6))
                                  {   samerun++;
                                      runlen = 0;
                                  }*/
                              } else
                              {   col = color[-1]  * 3;
                                  i   = oldmap[-1] * 3;
                                  if((abs(lastframe->palette[col]-image->palette[i]) < 6) &&
                                     (abs(lastframe->palette[col+1]-image->palette[i+1]) < 6) &&
                                     (abs(lastframe->palette[col+2]-image->palette[i+2]) < 6))
                                  {   runlen = 1;
                                      currun = 0;
                                  }
                              }
                          }
                          if(currun && runlen > 0)
                          {   ((RUNTYPE *)tptr)[0] = runlen + FMV_OPAQUE;
                              #ifdef LOG_ME
                              if(stopout) printlog("  > Remainder Run encode. runlen = %d; cnt = %d",runlen,cnt);
                              #endif
                              cnt+=runlen;
                              cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                              runlen = 0;
                          }
                          fullrun = 0;
                      }

                      runlen = 0;
        
                      if(samerun > 0)
                      {   // --- Encode a same-color run ---
                          #ifdef LOG_ME
                          if(stopout) printlog("  Encoding Samecolor run v.2 of color %d...",oldmap[-1]);
                          #endif
                          if(fullrun == 1)
                          {   // first run has a max length half of MAX_RUNLEN
                              #ifdef LOG_ME
                              if(stopout) printlog("    > First Run encode. runlen = %d; cnt = %d",samerun,cnt);
                              #endif
                              *tptr = FMV_COLOR | FMV_OPAQUE;
                              if(pass>2)
                              {   // Pass Type 1 .. assign the transparent color type
                                  *tptr |= FMV_TRANSCOLOR; tptr++; cnt++; wscnt++;
                                  *tptr = scancolor;
                                  tptr+=bytespp; cnt+=bytespp; wscnt+=bytespp;
                              } else
                              {   tptr++; cnt++; wscnt++;  }
                              i = (samerun > FULL_RUNLEN) ? FULL_RUNLEN : samerun;
                              ((RUNTYPE *)tptr)[0] = i;
                              samerun-=i;
                              tptr[1] = oldmap[-1];
                              cnt+=bytespp;  tptr+=bytespp;  wscnt+=bytespp;
                              cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                          }
                          while(samerun > 0)
                          {   i = (samerun > MAX_RUNLEN ? MAX_RUNLEN : samerun);
                              ((RUNTYPE *)tptr)[0] = i + (FMV_COLOR | FMV_OPAQUE);
                              #ifdef LOG_ME
                              if(stopout) printlog("  > MaxRuns encode. runlen = %d; cnt = %d",samerun,cnt);
                              #endif
                              samerun -= i;
                              tptr[1] = oldmap[-1];
                              cnt+=bytespp;  tptr+=bytespp;  wscnt+=bytespp;
                              cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                          }
        
                          // check for a near-color match with the previous frame.  If one, stream
                          // this run in with the next transparent.
                          /*if(samerun == bytespp && lossy)
                          {   // Use lossy compression to sap an extra byte
                              col = *color  * 3;
                              i   = *oldmap * 3;
                              if((abs(lastframe->palette[col]-image->palette[i]) < 5) &&
                                 (abs(lastframe->palette[col+1]-image->palette[i+1]) < 5) &&
                                 (abs(lastframe->palette[col+2]-image->palette[i+2]) < 5))
                              {   runlen=1;
                              }
                          } else */
                      }
                      currun  = fullrun = 0;
                      samerun = 0;
                  }
              }

              // ===============================================================
              // Check / Update the same-color run counter if no trans-to-opaque
              // runchange was detected and resolved above.  This gets priority
              // over basic opaque runs, and is only processed if the current run
              // type is opaque!

              if(pass!=2 && pass<=pntman)
              {   for(jj=0; jj<bytespp; jj++)
                  {   if(oldmap[jj] != oldmap[jj-bytespp])
                      {   if((currun || pass==1) && (samerun > bytespp*2))
                          {   runlen -= samerun;
                              #ifdef LOG_ME
                              if(stopout) printlog("  Encoding Same-Color run of color %d... ",oldmap[-1]);
                              #endif
    
                              // ok, we have some opaque before the color-run - store it first
                              // --- Encode an opaque run ---
    
                              if(runlen > 0)
                              {
                                  #ifdef LOG_ME
                                  if(stopout) printlog("  - Encoding Opaque run... ");
                                  #endif
                                 
                                  if(fullrun == 1)
                                  {   // first run has to setup the special scanline header info
                                      #ifdef LOG_ME
                                      if(stopout) printlog("    > First Run encode. runlen = %d; cnt = %d",runlen,cnt);
                                      #endif
                                      *tptr = FMV_OPAQUE;
                                      if(pass>2)
                                      {   // Pass Type 1 .. assign the transparent color type
                                          *tptr |= FMV_TRANSCOLOR;  tptr++; cnt++; wscnt++;
                                          *tptr = scancolor;
                                          tptr+=bytespp; cnt+=bytespp; wscnt+=bytespp;
                                      } else
                                      {   tptr++; cnt++; wscnt++;   }
    
                                      i = (runlen > FULL_RUNLEN) ? FULL_RUNLEN : runlen;
                                      ((RUNTYPE *)tptr)[0] = i;
                                      runlen -= i;
                                      cnt+=i;
                                      cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                                  }
                                  while(runlen > 0)
                                  {   i = (runlen > MAX_RUNLEN ? MAX_RUNLEN : runlen);
                                      ((RUNTYPE *)tptr)[0] = i + FMV_OPAQUE;
                                      #ifdef LOG_ME
                                      if(stopout) printlog("    > MaxRuns encode. runlen = %d; cnt = %d",runlen,cnt);
                                      #endif
                                      runlen -= i;
                                      cnt+=i;
                                      cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                                  }
                                  fullrun = 0;
                              }
                              runlen = 0;
    
                              if(samerun > bytespp*2)
                              {   // --- Encode a same-color run ---
                                  if(fullrun == 1)
                                  {   // first run has a max length half of MAX_RUNLEN
                                      #ifdef LOG_ME
                                      if(stopout) printlog("  > First Run encode. runlen = %d; cnt = %d",samerun,cnt);
                                      #endif
                                      *tptr = FMV_COLOR | FMV_OPAQUE;
                                      if(pass>2)
                                      {   // Pass Type 1 .. assign the transparent color type
                                          *tptr |= FMV_TRANSCOLOR; tptr++; cnt++; wscnt++;
                                          *tptr = scancolor;
                                          tptr+=bytespp; cnt+=bytespp; wscnt+=bytespp;
                                      } else
                                      {   tptr++; cnt++; wscnt++; }
    
                                      i = (samerun > FULL_RUNLEN) ? FULL_RUNLEN : samerun;
                                      ((RUNTYPE *)tptr)[0] = i;
                                      samerun -= i;
                                      tptr[1] = oldmap[-1];
                                      cnt+=bytespp;  tptr+=bytespp;  wscnt+=bytespp;
                                      cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                                  }
                                  while(samerun > bytespp*2)
                                  {   i = (samerun > MAX_RUNLEN ? MAX_RUNLEN : samerun);
                                      ((RUNTYPE *)tptr)[0] = i + (FMV_COLOR | FMV_OPAQUE);
                                      tptr[1] = oldmap[-1];
                                      #ifdef LOG_ME
                                      if(stopout) printlog("  > MaxRuns encode. runlen = %d; cnt = %d",samerun,cnt);
                                      #endif
                                      samerun -= i;
                                      cnt+=bytespp;  tptr+=bytespp;  wscnt+=bytespp;
                                      cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                                  }
                                  runlen = samerun;
                              }
                              fullrun = 0;
                          }
                          samerun = 0;
                      }
                  }
              }

              runlen  += bytespp;
              if(pass!=2 && pass<=pntman) samerun += bytespp;
        
              if(wscnt > WORKSPACE - 10)
              {   _mm_free(workspace);
                  printlog("Workspace buffer is full.  Encoding Aborted!");
                  return -1;
              }
        
              oldmap+=bytespp;
              color +=bytespp;
           } while(--scancnt);

           // Store information for the last run of the scan.
           if(fullrun)
           {   // A full scan run - set the proper bit
               tptr[0] = FMV_FULLRUN;
               #ifdef LOG_ME
               if(stopout) printlog("Encoding Full Run...");
               #endif
               // Now check the runtype.
               if(currun || pass==1)
               {   if(samerun==xbytes)       // no hicolor support here yet
                   {   tptr[0] |= FMV_COLOR | FMV_OPAQUE;
                       tptr[1] = oldmap[-1];
                       tptr+=2; wscnt+=2; cnt+=2;
                       #ifdef LOG_ME
                       if(stopout) printlog("  .. Color!");
                       #endif
                   } else
                   {   tptr[0] |= FMV_OPAQUE;
                       tptr++; wscnt++; cnt++;
                       cnt  += xbytes;
                       #ifdef LOG_ME
                       if(stopout) printlog("  .. Opaque!");
                       #endif
                   }
               } else
               {   if(pass>2)
                   {   // Pass Type 1 .. assign the transparent color type
                       *tptr |= FMV_COLOR | FMV_OPAQUE; tptr++; cnt++; wscnt++;
                       *tptr = scancolor;
                       tptr+=bytespp; cnt+=bytespp; wscnt+=bytespp;
                       #ifdef LOG_ME
                       if(stopout) printlog("  .. Color v.2!");
                       #endif                       
                   } else
                   {   *tptr = 0; tptr++; cnt++; wscnt++;
                       #ifdef LOG_ME
                       if(stopout) printlog("  .. Transparent!");
                       #endif
                   }
               }
           } else
           {  // Resolve the left-over runlength at the end
              #ifdef LOG_ME
              if(stopout) printlog("Resolving Left-over runlengh...",runlen,cnt);
              #endif
              if(currun || pass==1)
              {   if(samerun > bytespp * 2)
                  {   // We have a same-color run we can store, so modify runlen to
                      // store the opaque in front
                      runlen -= samerun;
                  }

                  #ifdef LOG_ME
                  if(stopout) printlog("  Encoding Opaque run...");
                  #endif
                  while(runlen > 0)
                  {   i = (runlen > MAX_RUNLEN ? MAX_RUNLEN : runlen);
                      ((RUNTYPE *)tptr)[0] = i + FMV_OPAQUE;
                      #ifdef LOG_ME
                      if(stopout) printlog("  > MaxRuns encode. runlen = %d; cnt = %d",runlen,cnt);
                      #endif
                      runlen-=i;
                      cnt+=i;
                      cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                  }
                 
                  if(samerun > bytespp*2)
                  {   // --- Encode a same-color run ---
                      #ifdef LOG_ME
                      if(stopout) printlog("  - Encoding Samecolor run...");
                      #endif
                      while(samerun > 0)
                      {   i = (samerun > MAX_RUNLEN ? MAX_RUNLEN : samerun);
                          ((RUNTYPE *)tptr)[0] = i + (FMV_COLOR | FMV_OPAQUE);
                          #ifdef LOG_ME
                          if(stopout) printlog("    > MaxRuns encode. runlen = %d; cnt = %d",samerun,cnt);
                          #endif
                          samerun-=i;
                          tptr[1] = oldmap[-1];
                          cnt+=bytespp;  tptr+=bytespp; wscnt+=bytespp;
                          cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                      }
                  }
              }

              if(pass==0)
              {   // Transparent runs at end of scans need no storage!  The
                  // Zero marker automatically tells the displayer to jump to the
                  // next scan, leaving the transparent part untouched.
              } else
              {   if(runlen > 0)
                  {   // --- Encode a transparent run ---
                      #ifdef LOG_ME
                      if(stopout) printlog("  - Encoding Transparent run...");
                      #endif
                      while(runlen > 0)
                      {   i = (runlen > TWICE_RUNLEN ? TWICE_RUNLEN : runlen);
                          ((RUNTYPE *)tptr)[0] = i;
                          #ifdef LOG_ME
                          if(stopout) printlog("    > MaxRuns encode. runlen = %d; cnt = %d",samerun,cnt);
                          #endif
                          runlen-=i;
                          cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
                      }
                  }
              }

              // Throw in Zero Marker (termination of scanline)
              ((RUNTYPE *)tptr)[0] = 0;
              cnt+=sizeof(RUNTYPE); tptr+=sizeof(RUNTYPE); wscnt+=sizeof(RUNTYPE);
           }

           if(stopout) break;

           if(cnt <= ww)
           {   ww   = cnt;
               best = pass;
           }
           pass--;
           if(pass < 0)
           {   pass = best;
               stopout = 1;
           }
       }

       srcmap  += xbytes;  newmap += xbytes;
       workptr += (wscnt-wstmp);
       wstmp = wscnt;
       fullcnt += cnt;
   }

   cnt = fullcnt;

   // Pre-processor complete - malloc final buffer.
   printlog("hmm.. %d",cnt);

   if((newmap = (UBYTE *)_mm_malloc(cnt)) == NULL)
   {   _mm_free(workspace);
       return 1;
   }

   // Main Processor.  Encodes the sprite into the new buffer using a variation
   // of the decoder (neat, eh? :)

   oldmap = image->bitmap;
   tptr   = workspace;
   cnt    = 0;                             // cnt is the index into newmap now

   yi = image->ysize;
   do
   {   // bit 0   set = opaque; clear = transparent
       // bit 1   set = no type change; clear = type changes
       cc = ((UBYTE *)tptr)[0]; tptr++;
       newmap[cnt] = cc;  cnt++;
       #ifdef LOG_ME_2
       printlog("----------- Scan %d, cc = %d ---------------",240-yi,cc);
       #endif

       if(cc!=0)
       {
           if(cc & FMV_FULLRUN)
           {
               #ifdef LOG_ME_2
               printlog("Full Run, Cnt = %d",cnt);
               #endif
              
               if(cc & FMV_OPAQUE)
               {   if(cc & FMV_COLOR)
                   {   ww = tptr[0]; tptr++;
                       newmap[cnt] = ww;  cnt++;
                       #ifdef LOG_ME_2
                       printlog(" .. Color, Count = %d",cnt);
                       #endif
                   } else
                   {   memcpy(&newmap[cnt],oldmap,xbytes);
                       cnt+=xbytes;
                       #ifdef LOG_ME_2
                       printlog(" .. Opaque, Count = %d",cnt);
                       #endif
                   }
               }
           } else
           {   // The first runlen needs special processing...
               // The runlen is stored by itself on the first run, and not with
               // the flag byte, which has more information in it too.
              
               if(cc & FMV_TRANSCOLOR)
               {   ww = tptr[0];  tptr++;
                   newmap[cnt] = ww;  cnt++;
                   #ifdef LOG_ME_2
                   printlog("TransColor Assign, color = %d; cnt = %d",ww,cnt);
                   #endif
               }
              
               srcmap = oldmap;
               i = ((UBYTE *)tptr)[0]; tptr++;
               newmap[cnt] = i;   cnt++;
               if(cc & FMV_OPAQUE)
               {   if(cc & FMV_COLOR)
                   {   ww = tptr[0];  tptr++;
                       newmap[cnt] = ww;  cnt++;
                       #ifdef LOG_ME_2
                       printlog("First Run (Color), len = %d; cnt = %d",i,cnt);
                       #endif
                   } else 
                   {   memcpy(&newmap[cnt],srcmap,i);
                       cnt += i;
                       #ifdef LOG_ME_2
                       printlog("First Run (Opaque), len = %d; cnt = %d",i,cnt);
                       #endif
                   }
               }
               #ifdef LOG_ME_2
               else
               {   printlog("First Run (Trans), len = %d; cnt = %d",i,cnt);
               }
               #endif
               srcmap += i;

               do
               {   cc = ((UBYTE *)tptr)[0]; tptr++;
                   newmap[cnt] = cc;  cnt++;
                   if(cc==0) break;
        
                   if(cc & FMV_OPAQUE)
                   {   i = cc & ~(FMV_OPAQUE | FMV_COLOR);
                       if(cc & FMV_COLOR)
                       {   i = cc & ~(FMV_OPAQUE | FMV_COLOR);
                           ww = tptr[0];  tptr++;
                           newmap[cnt] = ww;  cnt++;
                           #ifdef LOG_ME_2
                           printlog("Color Run, len = %d; cnt = %d",i,cnt);
                           #endif
                       } else
                       {   memcpy(&newmap[cnt],srcmap,i);
                           cnt += i;
                           #ifdef LOG_ME_2
                           printlog("Opaque Run, len = %d; cnt = %d",i,cnt);
                           #endif
                       }
                   } else
                   {   i = cc & ~FMV_OPAQUE;
                   #ifdef LOG_ME_2
                       printlog("Transparent Run, len = %d; cnt = %d",i,cnt);
                   #endif
                   }
                   srcmap += i;
               } while(1);
           }
       }
       oldmap += xbytes;
   } while(--yi);

   image->bytesize = cnt;

   _mm_free(image->bitmap); _mm_free(workspace);
   image->bitmap = newmap;
   image->flags |= IMG_RLE_BYTE;
   return 0;
}


IMAGE *FMV_Encode(IMAGE *frm1, IMAGE *frm2, BOOL lossy, BOOL speed)
// This function enhances the standard funky encoder to be optimal for Full
// Motion Video (FMV) situations.  Given two frames, it compares them and
// encodes them to redraw only those portions of the frames that change.
//
// Notes:
//   Both frames must be of the same size!
//   It is assumed that the palettes are MATCHED.
//
// Returns:
//   A newly allocated image structure, RLE'd and ready to save to disk!
{
    int     tsize;
    IMAGE  *nimg;

    nimg = _mm_calloc(1,sizeof(IMAGE));
    if((nimg->bitmap = _mm_malloc(tsize=frm1->xsize*frm1->ysize)) == NULL) return NULL;
    nimg->xsize = frm1->xsize;
    nimg->ysize = frm1->ysize;
    nimg->bytespp = frm1->bytespp;

    memcpy(nimg->bitmap,frm2->bitmap,tsize);
    if(FMV_FunkyEncode(nimg,frm1,lossy,speed) > 0)
    {   _mm_free(nimg->bitmap);
        _mm_free(nimg);
        return NULL;
    }
    return nimg;
}

