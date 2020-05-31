/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: main.c

  This is where the big things happen.  The game execution is directed by
  the code within this module.  See for yourself!

  (someone write a better description here, please! :)

*/

#include "log.h"
#include "chicken.h"
#include "clres.h"

#include <windows.h>
#include "..\resource.h"

#include "uniform.h"

bool g_bFullScreen = false;
int  WindowX = 100, WindowY = 100;

static int cfg_xres[3]    = { 640, 640, 0 };
static int cfg_yres[3]    = { 480, 480, 0 };
static int cfg_bytespp[3] = { 2, 4, 0 };

GAMEDATA    chicken;        // global stuff needed for the game.


// =====================================================================================
    static void mmerr(int num, CHAR *str)
// =====================================================================================
// Hey, look: a really simple and stupid error message pop-up.  Called when some function
// decides the error merits it and calls _mmerr_set.
{
    App_MessageBox("Chicken Little Error/Warning!",str,0);
}


// =====================================================================================
    static uint OnReset(WMWL)
// =====================================================================================
// Reset the game state to START, and clear out unneeded crap:
{
    
    CL_SetGameState(&chicken, CL_STATE_START);

    return 1;
}


// =====================================================================================
    BOOL CALLBACK AboutProc(HWND hWnd, uint uMsg, WPARAM wParam, LPARAM lParam)
// =====================================================================================
// The dialog procedure for the dialog box.... !
{
    switch (uMsg)
    {
       case WM_INITDIALOG:
            return TRUE;
        break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                EndDialog(hWnd, 0);
                return TRUE;
            }
            if (LOWORD(wParam) == IDC_WEBPAGE)
            {
                ShellExecute(HWND_DESKTOP, "open", "http://www.gratisgames.com", NULL, NULL, SW_SHOWNORMAL);
            }            
        break;
    }

    return 0;
}


// =====================================================================================
    static uint OnAbout(WMWL)
// =====================================================================================
// All-important.  No, really...
{
    App_DialogBox(IDD_ABOUT);
    return 0;
}


// =====================================================================================
    static uint OnExit(WMWL)
// =====================================================================================
{
	App_Close();
	return 0;
}

// =====================================================================================
    static uint OnSwitchModes(WMWL)
// =====================================================================================
{
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
	{   VD_SetMode(chicken.vr, cfg_xres, cfg_yres, cfg_bytespp, VMODE_WINDOWED);
        App_AdjustWindow(APPWS_NORMAL,WindowX,WindowY,640,480,TRUE);
		g_bFullScreen = false;
    } else
	{   App_AdjustWindow(APPWS_FULLSCREEN,0,0,0,0,FALSE);
        g_bFullScreen = true;
        // New 'real' fullscreen mode, instead of stretch mode.
        VD_SetMode(chicken.vr, cfg_xres, cfg_yres, cfg_bytespp, 0);
        App_Paint();               // this ensures we get repainted properly
	}

	return 0;
}

// =====================================================================================
    static uint OnDisplayChange(WMWL)
// =====================================================================================
{
    if(!g_bFullScreen)
	{   VD_SetMode(chicken.vr, cfg_xres, cfg_yres, cfg_bytespp, VMODE_WINDOWED);
        App_AdjustWindow(APPWS_NORMAL,WindowX,WindowY,640,480,TRUE);
    }

	return 0;
}

// =====================================================================================
//	Message Map (MainWndMap)
// =====================================================================================

BEGIN_MESSAGE_MAP( MainWndMap )
    ON_COMMAND(IDM_RESET, OnReset)		
    ON_COMMAND(IDM_EXIT, OnExit)
    ON_COMMAND(IDM_ABOUT, OnAbout)
    ON_COMMAND(IDM_SWITCH_MODES, OnSwitchModes)
    ON_DISPLAYCHANGE(OnDisplayChange)
    ON_PAINT(App_OnPaint)
    ON_DESTROY(App_OnDestroy)
    ON_SIZE(App_OnSize)
END_MESSAGE_MAP()


extern void Renderer(void);

// =====================================================================================
    static BOOL SurfaceChange(VD_RENDERER *vr, void *ignore)
// =====================================================================================
// This procedure is triggered everytime the vdriver's primary surface is changed
// for some reason (usually because of a mode switch to/from fullscreen mode).
{
    // Get our new attached surface

    chicken.vs = VD_GetSurface(vr);

    // Reload all the resources which are currently in use:

    if(respak)
        VDRes_ReloadResources(respak->videopak,chicken.vs);

    chicken.time = App_GetTime();             // make sure the game doesn't think we're behind.

    return 1;
}



static KBDATA  kbd;

// =====================================================================================
    static void UpdateKeyStatus(KBDATA *kbd)
// =====================================================================================
// Fills the given keyboard structure with the state of the 'important buttons' of the
// keyboard (rotate, left, right).  I set this procedure up so that holding down a key
// induces a 'repeat delay.'  However, rapidly clicking a button has no delay.  This way
// the player won't get 'penalized' for clicking too quickly, as he would in the old hack
// method.
{

    uint  time = App_GetTime();
    
    if(KEY_DOWN(VK_SPACE) || KEY_DOWN(VK_RETURN))
    {   if(kbd->space_in)
            DoKeyLogic(space,350)
        else
            DoKeyLogic(space,160)
        kbd->space_in = 0;
    } else
    {   kbd->space    = 0;
        kbd->space_to = 0;
        kbd->space_in = 1;
    }

    if(KEY_DOWN(VK_LEFT))
    {   if(kbd->left_in)
            DoKeyLogic(left,350)
        else
            DoKeyLogic(left,160)
        kbd->left_in = 0;
    } else
    {   kbd->left    = 0;
        kbd->left_to = 0;
        kbd->left_in = 1;
    }

    if(KEY_DOWN(VK_RIGHT))
    {   if(kbd->right_in)
            DoKeyLogic(right,400)
        else
            DoKeyLogic(right,160)
        kbd->right_in = 0;
    } else
    {   kbd->right    = 0;
        kbd->right_to = 0;
        kbd->right_in = 1;
    }

    if(KEY_DOWN(VK_DOWN))
    {   if(kbd->down_in)
            DoKeyLogic(down,400)
        else
            DoKeyLogic(down,160)
        kbd->down_in = 0;
    } else
    {   kbd->down    = 0;
        kbd->down_to = 0;
        kbd->down_in = 1;
    }

    if(KEY_DOWN(VK_UP))
    {   if(kbd->up_in)
            DoKeyLogic(up,400)
        else
            DoKeyLogic(up,160)
        kbd->up_in = 0;
    } else
    {   kbd->up    = 0;
        kbd->up_to = 0;
        kbd->up_in = 1;
    }

    if(KEY_DOWN(VK_ESCAPE))
    {   if(kbd->escape_in)
            DoKeyLogic(escape,400)
        else
            DoKeyLogic(escape,300)
        kbd->escape_in = 0;
    } else
    {   kbd->escape    = 0;
        kbd->escape_to = 0;
        kbd->escape_in = 1;
    }
}


// =====================================================================================
    static void FancyTimings(void)
// =====================================================================================
{
    UpdateKeyStatus(&kbd);
    Mikmod_Update(chicken.md);
}


// =====================================================================================
    static void GameLoop(void)
// =====================================================================================
{
	// As long as video is not initalized, we don't go past here:

    //if(!chicken.vr->initialized) return;

    // Check for alt-enter and switch video mods if found:
    
    if( KEY_DOWN(VK_RETURN) && KEY_DOWN(VK_MENU) ) 
	{
		OnSwitchModes(0,0,0);
		while( KEY_DOWN(VK_RETURN) && KEY_DOWN(VK_MENU) );		// delay
	}

#ifdef _DEBUG
    // I update mikmod and the keyboard here in debug mode because if I update
    // it in the thread very bad things tend to happen when it crashes (As the 
    // thread keeps running).

    FancyTimings();
#endif

    // This will check the state of the game and do all the appropriate actions
    // base don game state and state of the keyboard input data.
    
    DoGameWork(&kbd);
}

    
// =====================================================================================
    PLAYER *GetOpponent(PLAYER *p)
// =====================================================================================
{
    // Function that return a reference to the opponent

    if (chicken.options.single == TRUE)
        return NULL;

    if (chicken.player[0] == p) // this is player 1, hence opponent is player 2
        return chicken.player[1];
    else
        return chicken.player[0];
}


MD_VOICESET   *vs_sndfx;

extern void drvDDraw_SetData(HWND handle);

// =====================================================================================
    void ShutDown(void)
// =====================================================================================
{
	VD_Exit(chicken.vr);
    chicken.vr = NULL;
    Mikmod_Exit(chicken.md);
    chicken.md = NULL;

    CL_CloseResource();

	log_exit();
	
}

enum
{
    CLERR_RESOURCE_OPEN = 1024
};

// =====================================================================================
    int Main(HWND hwnd)
// =====================================================================================
{		
	APPWND    wnd;
	SYSTEMTIME DateTime;
    
    // Initialize logging
    log_init("log",LOG_SILENT);

    _mmerr_sethandler(mmerr);
	
    	
	// Set the instance-specific data
    // ==============================

    CLEAR_STRUCT(chicken);
    CLEAR_STRUCT(kbd);
    
    Mikmod_RegisterLoader(load_it);
    Mikmod_RegisterLoader(load_xm);

	Mikmod_RegisterDriver(drv_ds);
	chicken.md = Mikmod_Init(22050, 60, NULL, MD_STEREO, CPU_NONE, 
		DMODE_16BITS | DMODE_INTERP | DMODE_NOCLICK);

    vs_sndfx = Voiceset_Create(chicken.md, NULL, 8, 0);
    Voiceset_SetVolume(vs_sndfx, 384);

    // Initialize VDRIVER Studly stuff
    // ===============================

    VD_RegisterDriver(&drv_ddraw);
    //VD_RegisterDriver(&drv_dib);

    chicken.vr = VD_InitRenderer(0);
    VD_SetEvent(chicken.vr,VDEVT_SURFACE_CHANGE, NULL, SurfaceChange);

    // Open the Chicken Little Resource Files
    // ======================================

    CL_OpenResource(chicken.vs, chicken.md);

    if(!respak)
    {   _mmlog("Chicken Little > Cannot find graphics resource files!  Terminating...");
        _mmerr_set(CLERR_RESOURCE_OPEN, "Missing data files: Your install of Chicken little is incomplete or corrupted.");
        ShutDown();
        return 0;
    }

    // One time AI initialization
    // ==========================

    AI_Initialization();
    
    
    // Window Creation
    // ===============================

	CLEAR_STRUCT(wnd);
	wnd.WindowName = "Chicken Little - Alpha 1";
	wnd.IconName   = MAKEINTRESOURCE(IDI_APPICON);

    wnd.Width      = 640;
    wnd.Height     = 480;

	wnd.Style      = APPWS_NORMAL;   g_bFullScreen  = false; 
    //wnd.Style      = APPWS_FULLSCREEN; g_bFullScreen = true;	
	wnd.MenuName   = MAKEINTRESOURCE(IDR_APPMENU);
	wnd.MessageMap = MainWndMap;
	
	App_CreateWindow(&wnd);

    // Make sure out inital video mode gets set.
    drvDDraw_SetData(App_GetWindowHandle());
    VD_SetMode(chicken.vr, cfg_xres, cfg_yres, cfg_bytespp, VMODE_WINDOWED);

    srand(App_GetTime());
    rand_setseed(App_GetTime());

	App_SetState(NULL, GameLoop);
    CL_SetGameState(&chicken, CL_STATE_START);

#ifndef _DEBUG
    AppTimer_Init(FancyTimings, 80);
#endif

    App_Run();

    ShutDown();

	return 0;
}

