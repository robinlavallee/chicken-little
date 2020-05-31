/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: title.c

  The Chicken Little title screen!  Insert here a more detailed description of what
  this title screen sequence involves!

*/

#include "chicken.h"

extern GAMEDATA chicken;

#include "clmenu.h"
#include "app.h"

SPRITE       strong,       // the title screen before the menu / demo
             faded;        // the title screen after!

// =====================================================================================
    static int Title_StateHandler(TITLE *title)
// =====================================================================================
{
    switch(title->entity.state)
    {
        // =============================================================================
        case ENTITY_STATE_WAITING:
        // =============================================================================
        // Waiting for a keystroke or something.  We don't do that here though.  We just
        // wait for something else to change our state and in the meantime do things.
        // Currently 'things' involves nothing.  It may involve spawning birdies later on.

        /*{
            BOOL   side;
            
            // look 'things' now involves spawning birdies!

            side = rand_getbit();
            Birdie_Spawn(entlist, side , rand_getbit(), side ? 639 : 0, (rand_getlong(9)*2) % 480);

            // wait a while before spawning a new one:

            return rand_getlong(12)+500;
        }*/
        return STATE_PRIORITY_LOW;

        // =============================================================================
        case TITLE_STATE_INTRO:
        // =============================================================================
        // For now we just do a lazy fade-in from black.
        // Ideas: fade in from white or blue might look better, although those are changes
        // to the display code below.

            title->fadein += 3;
            if(title->fadein > SPR_ALPHA_RANGE)
            {   title->fadein = 0;
                Entity_SetState(&title->entity, TITLE_STATE_WAITING);
            }
            App_Paint();
        return 9;

        case TITLE_STATE_CROSSFADE:
            // If we didn't finish fading in, then just flip to the final product:
            // (this code should prolly go in the 'state leaving' code)

            if(title->fadein)
            {   title->fadein    = 0;
                title->crossfade = SPR_ALPHA_RANGE;
            }

            title->crossfade += 4;
            if(title->crossfade > SPR_ALPHA_RANGE)
            {   title->crossfade = SPR_ALPHA_RANGE;
                Entity_SetState(&title->entity, TITLE_STATE_WAITING);
            }
            App_Paint();
        return 7;
    }

    return STATE_PRIORITY_LOW;
}


// =====================================================================================
    void Title_Renderer(TITLE *title)
// =====================================================================================
// This renders the title screen, the title-screen crossfade, and the game's auto-demo.
// Hence, it makes a call to the Player_Renderer at the bottom if the player stuff is
// initialized (which makes it so we can see our demo running!)
{
    // Display our background.  We have several possible states:
    //  (a) fading in from black.
    //  (b) standard strong background
    //  (c) crossfading from strong background to faded one.
    //  (d) standard faded background.

	if(title->fadein)
        sprblit_blackfade(strong,0,0,title->fadein);
    else
    {   if(title->crossfade == SPR_ALPHA_RANGE)
            sprblit(faded,0,0);
        else if(!title->crossfade)
            sprblit(strong,0,0);
        else
            sprblit_crossfade(&faded,&strong,0,0,title->crossfade);
    }
}


ENTITY Entity_Title =
{
    "Title",
    TITLE_STATE_INTRO,
    0,
    NULL,
    Title_StateHandler,
};


// =====================================================================================
    TITLE *Title_Initialize(ENTITY **entlist)
// =====================================================================================
// Spawn a title entity using the given SPRITE as the title screen.
{
    TITLE   *title;

    title = (TITLE *)Entity_Spawn(entlist, &Entity_Title, sizeof(TITLE));

    title->fadein = 1;
    return title;
}


// =====================================================================================
    BOOL Title_LoadResources(CL_RES *respak)
// =====================================================================================
// Load the title screens we would like to put to use!
{

    if(!strong.bitmap) VDRes_LoadSprite(respak->videopak, &strong, RES_TITLE_MAIN);
    if(!faded.bitmap)  VDRes_LoadSprite(respak->videopak, &faded, RES_TITLE_SOFT);

    return -1;
}


#define MENU_ALPHA_UNSELECT  ((SPR_ALPHA_RANGE * 4) / 10)


// =====================================================================================
    static int Menu_StateHandler(MENU *menu)
// =====================================================================================
{
    int    i;
    
    switch(menu->entity.state)
    {
        // =============================================================================
        case ENTITY_STATE_WAITING:
        // =============================================================================
        // This is the only state for our menu, oddly enough.  It checks each of the items
        // in the menu and moves their alpha channels closer to their proper destination
        // (wich depends on if they are selected or not).

        for(i=0; i<menu->numsel; i++)
        {   
            if(i == menu->cursel)
            {   
                // This is the currently selected item.  Make sure it moves toward being
                // fully opaque!

                if(menu->sel[i].alpha < SPR_ALPHA_RANGE)
                {   menu->sel[i].alpha += 6;
                    if(menu->sel[i].alpha > SPR_ALPHA_RANGE) menu->sel[i].alpha = SPR_ALPHA_RANGE;
                    App_Paint();
                }
            } else
            {
                // This is a non-selected item, so make sure it is fading out toward the
                // the arbitrary 'fade out' value.

                if(menu->sel[i].alpha > MENU_ALPHA_UNSELECT)
                {   menu->sel[i].alpha -= 6;
                    if(menu->sel[i].alpha < MENU_ALPHA_UNSELECT) menu->sel[i].alpha = MENU_ALPHA_UNSELECT;
                    App_Paint();
                } else
                {
                    // If our value is less than the 'minimum alpha' (the alpha for unselected
                    // items).  Then we need to fade in (not out).  This is a fix for
                    // when the menu is first shown and all alpha are 0.

                    if(menu->sel[i].alpha < MENU_ALPHA_UNSELECT)
                    {   menu->sel[i].alpha += 6;
                        if(menu->sel[i].alpha > MENU_ALPHA_UNSELECT) menu->sel[i].alpha = MENU_ALPHA_UNSELECT;
                        App_Paint();
                    }
                }
            }
        }
        break;

    }

    return 10;
}



// =====================================================================================
    static void Menu_Renderer(MENU *menu)
// =====================================================================================
{
    int  i;

    for(i=0; i<menu->numsel; i++)
        vd_dispstring_a(menu->sel[i].font, menu->sel[i].xloc, menu->sel[i].yloc, menu->sel[i].alpha, menu->sel[i].text);
}


static ENTITY Entity_Menu = 
{
    "Menu",
    ENTITY_STATE_WAITING,
    0,
    NULL,
    Menu_StateHandler,
    NULL,
    Menu_Renderer
};


// =====================================================================================
    MENU *Menu_Initialize(ENTITY **entlist, SELECTION *selset, uint numsel)
// =====================================================================================
// Spawn a title entity using the given SPRITE as the title screen.
{
    MENU   *menu;
    uint    i;

    menu = (MENU *)Entity_Spawn(entlist, &Entity_Menu, sizeof(MENU));

    menu->sel    = selset;
    menu->numsel = numsel;
    
    for(i=0; i<numsel; i++)
    {
        menu->sel[i].font  = font.big;
        menu->sel[i].alpha = (i) ? 128 : 256;
    }

    return menu;
}


// =====================================================================================
    void Menu_MoveDown(MENU *menu)
// =====================================================================================
{
    menu->cursel++;
    if(menu->cursel >= menu->numsel) menu->cursel = 0;

    mdsfx_playeffect(sfx.menuhigh,vs_sndfx,SF_START_BEGIN,0);
}


// =====================================================================================
    void Menu_MoveUp(MENU *menu)
// =====================================================================================
{
    menu->cursel--;
    if(menu->cursel < 0) menu->cursel = menu->numsel-1;

    mdsfx_playeffect(sfx.menuhigh,vs_sndfx,SF_START_BEGIN,0);
}
