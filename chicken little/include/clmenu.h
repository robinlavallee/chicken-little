
#ifndef _CLMENU_H_
#define _CLMENU_H_

#include "entity.h"
#include "vdfont.h"

enum
{   TITLE_STATE_INTRO = ENTITY_NUMSTATES,  // introduction of the screen
    TITLE_STATE_WAITING,                   // waiting for the player to press a button.
    TITLE_STATE_CROSSFADE,
};


// =====================================================================================
    typedef struct SELECTION
// =====================================================================================
{
    int      xloc, yloc;          // location fo the selection on-screen.
    int      alpha;               // the current alpha factor for this menu.
    VD_FONT *font;

    CHAR    *text;                // text shown

} SELECTION;


// =====================================================================================
    typedef struct MENU
// =====================================================================================
{
    ENTITY      entity;

    int         cursel,
                numsel;

    SELECTION  *sel;
} MENU;


// =====================================================================================
    typedef struct TITLE
// =====================================================================================
{
    ENTITY        entity;       // yup, we are an entity.  It is in our blood.
    int           crossfade;
    int           fadein;
} TITLE;

extern TITLE     *Title_Initialize(ENTITY **entlist);
extern BOOL       Title_LoadResources(CL_RES *respak);

extern MENU      *Menu_Initialize(ENTITY **entlist, SELECTION *elset, uint numsel);
extern void       Menu_MoveDown(MENU *menu);
extern void       Menu_MoveUp(MENU *menu);

#endif
