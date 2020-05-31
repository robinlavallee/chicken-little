/*
  VDRIVER Font Stuff

  This code is very dated.  I originally intended it for displaying diagnostic
  messages and creating basic a basic gui for utility apps.
*/


#ifndef __VFONT_H__
#define __VFONT_H__

#define FONT_MAXCHARS  256

// BIOS font definitions

#define LARGE_FONT 0
#define SMALL_FONT 1


//*******************
//  --> Structure :  FONT
//
typedef struct FONT
{  UBYTE   xsize;       // Universal font width (0 = non-proportional / non-std)
                        //  xsize is a shift factor - 1 = 2; 2 = 4; 3 = 8; 4 = 16; etc
   UBYTE   ysize;       // Universal height
   UBYTE   *info[FONT_MAXCHARS];  // Pointers to the font characters
} FONT;


extern FONT     *font;          // Currently used font


// ==============
//   Prototypes
// ==============

extern FONT   *LoadFont(UBYTE *fontfile);
extern void   FreeFont(FONT *fnt);
extern UBYTE  DispChar(int x, int y, int c);
extern UWORD  DispString(int x, int y, UBYTE *s);

extern UBYTE  dispcharb(const VD_SURFACE *vs, uint x, uint y, UBYTE c, int color);
extern UWORD  dispstringb(const VD_SURFACE *vs, uint x, uint y, int color, const UBYTE *text);
extern UWORD  outstrb(const VD_SURFACE *vs, uint x, uint y, int color, const UBYTE *fmt, ... );

#ifdef __WATCOMC__
#pragma aux dispcharb   parm nomemory modify nomemory;
#pragma aux dispstringb parm nomemory modify nomemory;
#pragma aux outstrb     parm nomemory modify nomemory;
#endif


#endif
