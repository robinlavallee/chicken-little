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
 Module: gfx.c
 
  Graphics Primitives for Off-Screen Video Buffers

       - ( Graphics the Divine Way ) -
       
  General-purpose portable non-game graphics primitives.

  These functions are IMAGE based functions and functions of a nature
  by which they will NOT be used in a finished game product.  Separated
  as they are, they won't be linked in unless they are called.. hence a
  smaller game .EXE!
*/

#include "vdriver.h"
#include <string.h>
#include <stdarg.h>



/********************************
   BIOS Font Output Functions ..
*********************************
    They're small, and used mono-colored bit-compressed font data.
    Not terribly fast (portability considerations), but are always
    there.

  Note:
    I reduced these to working in 16/32 bit color modes only.  I am weak.
*/

//ULONG understart;

extern UBYTE font50[];

#define FONTDATA   font50
#define FONTHEIGHT 8

// =====================================================================================
    UBYTE dispcharb(const VD_SURFACE *vs, unsigned int x, unsigned int y, UBYTE c, int color)
// =====================================================================================
{
    UBYTE *datp;
    int   itmp,ttmp;
              
    datp = &FONTDATA[c*FONTHEIGHT];
    ttmp = (vs->bytewidth - (itmp=8*vs->bytespp));
    
    if(vs->bytespp == 2)
    {   UWORD *scrn;
        int   i;

        scrn = (UWORD *)MK_ScrnPtr(x*2,y);
        for(i=FONTHEIGHT; i; i--, scrn+=ttmp, datp++)
        {   UBYTE  dat = *datp;
            int    k;

            for(k=8; k; k--, dat<<=1)
            {   if(dat & 128) *scrn = color;
                scrn += 2;
            }
        }
    } else
    {   ULONG *scrn;
        int   i;

        scrn = (ULONG *)MK_ScrnPtr(x*4,y);
        for(i=FONTHEIGHT; i; i--, scrn+=ttmp, datp++)
        {   UBYTE  dat = *datp;
            int    k;

            for(k=8; k; k--, dat<<=1)
            {   if(dat & 128) *scrn = color;
                scrn += 4;
            }
        }
    }

    return(8);
}
   
   
// =====================================================================================
    UWORD dispstringb(const VD_SURFACE *vs, unsigned int x, unsigned int y, int color, const UBYTE *text)
// =====================================================================================
// modifies understart - if an ASCII 0x1 (ctrl-a) is found, understart is 
// set to the x location within the current string (in pixels) of the start
// of that character.
{
    ULONG  j=0;

//   understart = 0;

    for(; *text; text++)
    {   //if(*text == 1)
        //   understart = x+j;
        //else

        {   // I should probably check for proper bounding area here.

            if(*text != 32) dispcharb(vs, x, y, *text, color);
            j += 8;
            x += 8;
        } 
    }
    return(j);
}


// =====================================================================================
    UWORD outstrb(const VD_SURFACE *vs, unsigned int x, unsigned int y, int color, const UBYTE *fmt, ... )
// =====================================================================================
{
   va_list argptr;
   UBYTE str[255];
   int   cnt;

   va_start(argptr, fmt);
   cnt = vsprintf(str, fmt, argptr);
   str[cnt + 1] = 0;
   va_end(argptr);
   return(dispstringb(vs, x,y,color,str));
}


/*****************
  rect_store(), rect_restore -- Graphics Functions

  Used for storing and restoring a small area of the screen.
*/

// =====================================================================================
    void rect_store(const VD_SURFACE *vs, int xstart, int ystart, int xend, int yend, UBYTE *buffer)
// =====================================================================================
{
    ULONG tmp;
    UBYTE *scrn, *buf = (UBYTE *)buffer;

    if(xstart > xend)           
    {  tmp = xstart;  xstart = xend;  xend = tmp;  }

    if(ystart > yend)
    {  tmp = ystart;  ystart = yend;  yend = tmp;  }

    scrn = MK_ScrnPtr(xstart,ystart);
    xend -= xstart; xend *= vs->bytespp;
    *((ULONG *)(buf))   = ++xend;
    *((ULONG *)(buf+4)) = (yend - ystart)+1;
 
    buf += 8;
    for(; ystart <= yend; ystart++, scrn+=vs->bytewidth, buf+=xend)
       memcpy(buf,scrn,xend);
}


// =====================================================================================
    void rect_restore(const VD_SURFACE *vs, int xloc, int yloc, const UBYTE *buffer)
// =====================================================================================
{
   UBYTE *scrn, *buf = (UBYTE *)buffer;

   scrn = MK_ScrnPtr(xloc,yloc);
   xloc = *((ULONG *)buf);
   yloc = *((ULONG *)(buf+4));
                    
   buf += 8;

   for(; yloc; yloc--, scrn+=vs->bytewidth, buf+=xloc)
     memcpy(scrn,buf,xloc);
}


