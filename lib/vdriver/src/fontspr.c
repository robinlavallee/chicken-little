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
 Module: fontspr.c

 Sprite-based font loading and rendering.
*/

#include "vdfont.h"


// =====================================================================================
    static uint __inline DispChar(const VD_FONT *font, int x, int y, CHAR c)
// =====================================================================================
{
    sprblit(font->chr[c],x,y);

    return font->chr[c].xsize;
}


// =====================================================================================
    static uint DispString(const VD_FONT *font, int x, int y, const CHAR *s)
// =====================================================================================
{
    uint  p = 0;
    
    while(*s)
    {   uint c = DispChar(font,x,y,*s) + font->xbuffer;
        x += c;
        p += c;

        s++;
    }
    
    return p;
}


// =====================================================================================
    static uint __inline DispCharA(const VD_FONT *font, int x, int y, uint alpha, CHAR c)
// =====================================================================================
{
    sprblit_alpha(font->chr[c],x,y,alpha);

    return font->chr[c].xsize;
}


// =====================================================================================
    static uint DispStringA(const VD_FONT *font, int x, int y, uint alpha, const CHAR *s)
// =====================================================================================
{
    uint  p = 0;
    
    while(*s)
    {   uint c = DispCharA(font,x,y,alpha,*s) + font->xbuffer;
        x += c;
        p += c;

        s++;
    }
    
    return p;
}

    
// =====================================================================================
    static uint DispStringShadow(const VD_FONT *font, int x, int y, const CHAR *s)
// =====================================================================================
{
    uint  p = 0;
    
/*    while(*s)
    {   uint c = DispCharA(font,x,y,alpha,*s) + font->xbuffer;
        sprblit_shadow(font->chr[*s],x+3,y+3);
        x += c;
        p += c;

        s++;
    }*/
    
    return p;
}


// =====================================================================================
    static void FontSpr_Free(VD_FONT *fnt)
// =====================================================================================
{
    if(fnt)
    {   
        _mmlogd("FontSpr > Unloading font...");
        _mm_free(fnt->chr, NULL);
        _mm_free(fnt, "Done!");
    }
}


// =====================================================================================
    VD_FONT *FontSpr_Load(int chrtab[256], uint height, uint spacewidth, int xbuffer, uint flags, VD_RESPAK *respak, int baseidx)
// =====================================================================================
// Builds the sprite array for all 256 charcters of the given character table.  Loads the
// sprites using the given resource pack loader API.
//
//  Returns NULL on error:
//   - Incomplete File
//   - Out of Memory Error
{
    VD_FONT   *fnt;
    int        i;

    fnt        = (VD_FONT *)_mm_malloc(sizeof(VD_FONT));
    fnt->chr   = (SPRITE *)_mm_calloc(256, sizeof(SPRITE));

    fnt->flags      = flags;
    fnt->height     = height;
    fnt->spacewidth = spacewidth;
    fnt->xbuffer    = xbuffer;

    for(i=0; i<256; i++)
    {   if(chrtab[i] != -1)
            VDRes_LoadSprite(respak, &fnt->chr[i], baseidx+chrtab[i]);
        else
            Sprite_Init(respak->vs, &fnt->chr[i], fnt->spacewidth, 0, 0, 0);
    }

    fnt->dispchar   = DispChar;
    fnt->dispstring = DispString;

    fnt->dispchar_a   = DispCharA;
    fnt->dispstring_a = DispStringA;

    fnt->dispstring_shadow = DispStringShadow;

    fnt->free       = FontSpr_Free;

    return fnt;
}                          
