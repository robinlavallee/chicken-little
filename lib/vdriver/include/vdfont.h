/*
  VDRIVER Font Stuff

  This code is very dated.  I originally intended it for displaying diagnostic
  messages and creating basic a basic gui for utility apps.
*/


#ifndef __VFONT_H__
#define __VFONT_H__

#include "vdriver.h"

#define vd_dispchar(f,x,y,c)      f->dispchar(f,x,y,c)
#define vd_dispstring(f,x,y,s)    f->dispstring(f,x,y,s)
#define vd_dispchar_a(f,x,y,a,c)    f->dispchar_a(f,x,y,a,c)
#define vd_dispstring_a(f,x,y,a,s)  f->dispstring_a(f,x,y,a,s)


#define FONT_MAXCHARS  256

// BIOS font definitions

#define LARGE_FONT 0
#define SMALL_FONT 1

#include "vdrespak.h"

#define VDFNT_STRLWR      1      // indicates the font is lowercase only.

// =====================================================================================
    typedef struct VD_FONT
// =====================================================================================
{
    uint       flags;            // see VDFNT_crap flagset.
    uint       numchr;           // number of characters in the font.
    SPRITE    *chr;              // array of sprites for each character in the alphabet.

    uint       height,           // height of the font.
               spacewidth;       // width fo the spaces.

    int        xbuffer;

    uint      (*dispchar)(const struct VD_FONT *fnt, int x, int y, CHAR c);
    uint      (*dispstring)(const struct VD_FONT *fnt, int x, int y, const CHAR *s);

    uint      (*dispchar_a)(const struct VD_FONT *fnt, int x, int y, uint alpha, CHAR c);
    uint      (*dispstring_a)(const struct VD_FONT *fnt, int x, int y, uint alpha, const CHAR *s);

    uint      (*dispstring_shadow)(const struct VD_FONT *fnt, int x, int y, const CHAR *s);
    
    void      (*free)(struct VD_FONT *fnt);
} VD_FONT;


// ==============
//   Prototypes
// ==============

extern VD_FONT *FontSpr_Load(int chrtab[256], uint height, uint spacewidth, int xbuffer, uint flags, VD_RESPAK *respak, int baseidx);

// Bios font stuff in gfx.c -->

extern uint    dispcharb(const VD_SURFACE *vs, uint x, uint y, CHAR c, int color);
extern uint    dispstringb(const VD_SURFACE *vs, uint x, uint y, int color, const CHAR *text);
extern uint    outstrb(const VD_SURFACE *vs, uint x, uint y, int color, const CHAR *fmt, ... );

#ifdef __WATCOMC__
#pragma aux dispcharb   parm nomemory modify nomemory;
#pragma aux dispstringb parm nomemory modify nomemory;
#pragma aux outstrb     parm nomemory modify nomemory;
#endif


#endif
