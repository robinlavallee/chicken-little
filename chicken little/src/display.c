/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: display.c

  This was made to get some stuff out of main.c and reduce the amount of clutter
  in said main.c.  Thie proceure contains all display rendering code used in
  Chicken Little currently.

  The only non-static (externally useable) function is:  Renderer
*/

#include "chicken.h"
#include "clmenu.h"
#include <string.h>



extern GAMEDATA chicken;


extern void    Title_Renderer(TITLE *title);

extern TITLE      *cl_title;
extern MENU       *cl_mainmenu;

// =====================================================================================
    void Intro_Renderer(void)
// =====================================================================================
{
    if(!chicken.vr->visible) return;

    // Renders the appropriate title background
    // if unset, we clear the screen!

    if(cl_title)
        Title_Renderer(cl_title);
    else
        memset(chicken.vs->vidmem, 0, chicken.vs->bytesize);

    Entity_Render(chicken.playerlist);
    Entity_Render(chicken.entitylist);

	chicken.vr->NextPage(chicken.vr);
}

void Portrait_Render(PORTRAIT *portrait);


// =====================================================================================
    void Match_Renderer(void)
// =====================================================================================
{
    if(!chicken.vr->visible) return;

	// First, display the background.

	sprblit(chicken.background,0,0);
    
    Entity_Render(chicken.playerlist);

    Portrait_Render(chicken.player[0]->portrait);

    if (chicken.options.single == FALSE)
        Portrait_Render(chicken.player[1]->portrait);

    if(chicken.paused)
    {   chicken.vs->shadowrect(chicken.vs, &chicken.vs->clipper, VD_ALPHA_50);
        vd_dispstring(font.big, 194, 80, "PAUSED");
    }

	// Finally, display our top-level entities:

    Entity_Render(chicken.entitylist);

    // let the user see our beautiful artwork!
	chicken.vr->NextPage(chicken.vr);
}

