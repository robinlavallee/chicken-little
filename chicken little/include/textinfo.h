#ifndef _TEXTINFO_H_
#define _TEXTINFO_H_

#include "vdfont.h"
#include "entity.h"

enum
{
    TEXT_STATE_FLOATING = ENTITY_NUMSTATES,
    TEXT_STATE_FADEOUT,
};


// =====================================================================================
    typedef struct TEXTINFO
// =====================================================================================
// Player text stuff.  This is a very flexable structure which informs the display blitter
// of the font, location, and various characteristics.
//
// Notes:
//  - floattime is used only if you force the textinfo state to 'TEXT_STATE_FLOATING'
//    It describes the amount of time before the text goes to fadeout.
//
//  - if you use floating text, set the entity.timeout to however many milliseconds until
//    the text should start floating upward.
{
    ENTITY    entity;                // we 'inherit' the entity structure
    VD_FONT  *font;
    CHAR      text[255];             // display something
    int       x, y;                  // where!
    int       fadeout;               // alpha fade value, 0 (transparent) to 128 (opaque).
    int       floating;              // distance to float upward.  If 0, no floating!
    uint      floattime;             // timeout value for floating text (don't use entity.timeout!)
} TEXTINFO;


extern TEXTINFO  *TextInfo_Initialize(ENTITY **entlist, const CHAR *text, int x, int y, VD_FONT *font);

#endif
