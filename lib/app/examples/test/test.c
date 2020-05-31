/* ==============================================================================================
	Test app
	Copyright (c) 2000 Nathan Youngman. All rights reserved.
	<contact@nathany.com>

	May 26, 2000?	Main() is called from WinMain which sets up Callbacks for
					GameLoop and GameShutdown	

	May 30, 2000	Main persists for length of app, allowing objects to be created in it.

	Jun 3, 2000		Makes use of DDraw wrapper and Primitive->PutPixel

	Jul 4, 2000		Rolled back to Jun 4th archive wo/24-bit mode and such removed.

	Jul 22, 2000	Split off from piX library. Dropped down to 'C' code. -n8

	Jul 24, 2000	Makes use of new app features for window creation and modifications.
					Message map defined.

	Jul 26, 2000	Saves window location between mode switches.

	Jul 27, 2000	Transferred VDRIVER simple app to this test app (draws lines)

    Aug 04, 2000    Added some VDRIVER image loading stuff, and reorganized code so that
                    switching to and from windowed/fullscreen will properly reload resources
                    to match the new bitdepths.
============================================================================================== */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "app.h"
#include "zfile.h"
#include "vdriver.h"
#include "log.h"

#include "resource.h"

bool g_bFullScreen = false;
int  WindowX = 0, WindowY = 0;

int   clr,clr2,black;


static int xres[3]    = { 640, 640, 0 };
static int yres[3]    = { 480, 480, 0 };
static int bytespp[3] = { 4, 2, 0 };

VD_RENDERER    *vr;         // the redering device object
VD_SURFACE     *vs;         // pointer to the surface attached to our renderer.

typedef struct TESTDATA
{   SPRITE   background;
} TESTDATA;

TESTDATA *instance;

/* ===============================================================================================
	OnExit
=============================================================================================== */
LRESULT OnExit(WMWL)
{
	App_Close();
	return 0;
}

/* ===============================================================================================
	OnSwitchModes
=============================================================================================== */
LRESULT OnSwitchModes(WMWL)
{
//	RECT rect;

    // Notes:
    //  Notice the order that VD_SetMode and App_AdjustWindow are called for
    //  each mode change.  This is *important* for proper clean mode switching.
    //  - When switching to windowed mode, we want the vdriver to deinitialize
    //    the fullscreen surface first, then create our new window.
    //  - When switching to fullscreen mode, we want to set our App window
    //    (which hides the window and changes it to a popup with no menu),
    //    then let vdriver do the work of showing it once the surface has been
    //    created.

    if(g_bFullScreen)
	{		
        VD_SetMode(vr, xres, yres, bytespp, VMODE_WINDOWED);
        App_AdjustWindow(APPWS_NORMAL,WindowX,WindowY,640,480,TRUE);
		g_bFullScreen = false;
 
	}
	else
	{
        App_AdjustWindow(APPWS_FULLSCREEN,0,0,0,0,FALSE);
		g_bFullScreen = true;

        // New 'real' fullscreen mode, instead of stretch mode.
        VD_SetMode(vr, xres, yres, bytespp, 0);

	}

	return 0;
}

/* ===============================================================================================
	Message Map (MainWndMap)
=============================================================================================== */
BEGIN_MESSAGE_MAP( MainWndMap )		
	ON_COMMAND(IDM_EXIT, OnExit)
	ON_COMMAND(IDM_SWITCH_MODES, OnSwitchModes)
	ON_PAINT(App_OnPaint)
	ON_DESTROY(App_OnDestroy)
END_MESSAGE_MAP()

/* ===============================================================================================
	GameLoop
=============================================================================================== */

void GameLoop(void)
{

    if( KEY_DOWN(VK_ESCAPE) )
	{		
		App_Close();
		return;
	}

	if( KEY_DOWN(VK_RETURN) && KEY_DOWN(VK_MENU) ) 
	{
		OnSwitchModes(NULL,0,0,0);
		
		while( KEY_DOWN(VK_RETURN) && KEY_DOWN(VK_MENU) );		// delay
	}

    // Display a sprite

    
    // Draw a couple lines. 

    vs->h_line(vs, 0, 50, 400, clr);
    vs->h_line(vs, 0, 100, 400, clr2);
    
    vs->v_line(vs, 0, 0, 350, clr);
    vs->v_line(vs, 400, 0, 350, clr2);

    instance->background.blit(&instance->background,99,100);
        //instance->foreground.blit(&instance->foreground,100,100);

    // let the user see our beautiful artwork!
	vr->NextPage(vr);

}

#include "image.h"


static BOOL SurfaceChange(VD_RENDERER *vr, TESTDATA *instance)

// This procedure is triggered everytime the vdriver's primary surface is changed
// for some reason (usually because of a mode switch to/from fullscreen mode).

{
    IMAGE  *limg;

    // Get our new attached surface
    
    vs = VD_GetSurface(vr);

    // Define some nice colors used throughout our program.

    clr   = VD_MakeColor(vs, 0, 200, 0);    // green
	clr2  = VD_MakeColor(vs, 0, 0, 200);    // blue
	black = VD_MakeColor(vs, 0, 0, 0);

    // Notes:
    //  - This would be the appropriate place to sift through the master list
    //    of loaded resources and reload them as according to the new surface
    //    parameters.

    // Our list of loaded resources - a small bmp image

    if(instance->background.bitmap)
    {   Sprite_Free(&instance->background);
    }
    
    limg = Image_Load("background.pcx");

    if(limg)
    {   Image_to_Sprite(vs, &instance->background, limg, SPR_TRANSPARENT,SPRA_ADDITIVE);
        //Image_to_Sprite(vs, &instance->background, limg, SPR_TRANSPARENT,0);
        //
        //Sprite_Duplicate(&instance->foreground, &instance->background, 0, SPRA_ADDITIVE);
    }


    return 1;
}

/* ===============================================================================================
	Main
=============================================================================================== */
int Main()
{		
	APPWND    wnd;
    TESTDATA  data;

    // Initialize logging
    log_init("log",LOG_SILENT);

    instance = &data;

    // Set the instance-specific data
    // ==============================

    CLEAR_STRUCT(data);
    
    // Initialize VDRIVER Studly stuff
    // ===============================

    VD_RegisterDriver(&drv_ddraw);
    vr = VD_InitRenderer(0);
    VD_SetEvent(vr,VDEVT_SURFACE_CHANGE, &data, SurfaceChange);

    // Register the software blitters we like and use.

    Image_RegisterLoader(&load_bmp);
    Image_RegisterLoader(&load_pcx);

    // Window Creation
    // ===============================

	CLEAR_STRUCT(wnd);
	wnd.WindowName = "Test Application 0.01";
	wnd.IconName   = MAKEINTRESOURCE(IDI_APPICON);

    wnd.Width      = 640;
    wnd.Height     = 480;
	
	wnd.Style      = APPWS_NORMAL;   g_bFullScreen  = false; 
    //wnd.Style  = APPWS_FULLSCREEN; g_bFullScreen = true;	
	wnd.MenuName   = MAKEINTRESOURCE(IDR_APPMENU);
	wnd.MessageMap = MainWndMap;
	
	App_CreateWindow(&wnd);

    // Make sure out inital video mode gets set.
    VD_SetMode(vr, xres, yres, bytespp, VMODE_WINDOWED);

	// run GameLoop
	App_Run(GameLoop, NULL, 50);

	// Shutdown stuff
	VD_Exit(vr);
	log_exit();
	
	return 0;
}

