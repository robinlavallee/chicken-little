/* ==============================================================================================
	App module 

	Copyright (c) 2000 Nathan Youngman. All rights reserved.
	<contact@nathany.com>

	May 26, 2000?	This module hides WinMain and WindowProc for full screen applications.
					Open/Close window functions
					Callbacks for GameState (MainLoop) and Shutdown function
	
	May 27, 2000	Add "last resort" catch to WinMain that displays error message.
					WinMain also opens/closes error.log
					Implemented GetCommandLine() vs. passing to Main() function

	May 30, 2000	Modified so that Main calls Run() function, this way Main doesn't lose scope
					until exiting, which means objects created in Main persist for the length
					of the program.

					Dropped shutdown function callback (nolonger needed).

	Jun 2, 2000		Renamed namespace from WinApp to just "App"

	Jul 22, 2000	Dropped down to 'C' code.

	Jul 24, 2000	CreateWindow : uses structure for icon, style, dimensions, menu
					CalculateWindowDimensions : create window with -client area- of specified size
					AdjustWindow : modifies size of menu (full screen, windowed, menu or not)
					WindowProc : Uses MessageMap structure
  
	Jul 26, 2000	Default message handlers : App_OnPaint, App_OnDestroy
					Option to specify separate small icon (untested)
					AdjustWindow : Allow setting of (x,y) position. 
					Only allow 1 instance to run at a time.
	
	To do			
					Support message handlers for WM_ERASEBKGND, WM_MOVE, etc.
					Keyboard Accelerator support (LoadAccelerators, TranslateAccelerator)?
					Allow starting full-screen with a visible menu?
============================================================================================== */

#include <windows.h>
#include "mmio.h"
#include "app.h"

#define APPCLASSNAME	"MyAppClassName"

// ==========================================
//  Define APP's Customized Windows Messages
// ==========================================

#ifndef APPWM_USER
#define APPWM_USER WM_USER
#endif

enum
{   APPWM_INSTANCEDATA = APPWM_USER,
};

// ========================================
//  Various per-instance globals and stuff
// ========================================

HINSTANCE  hAppInstance;
HWND       g_hAppWnd;
char       g_szCommandLine[256];

APPMSGMAP *g_MessageMap;
HMENU      g_hOldMenu = NULL;

volatile BOOL  g_Hidden         = false,
               g_Minimized      = false,
               g_Invalid        = false;

LPFUNCV    g_RenderFunc = NULL;
LPFUNCV    g_LogicFunc  = NULL;
LPFUNCV    g_TimerFunc  = NULL;

extern int Main();       // Help me, I'm a dumb language and I can't do pre-processing to resolve function names!

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


static uint Win98Stylize(uint style)
{
    switch(style)
    {   case APPWS_NORMAL:      return WS_OVERLAPPEDWINDOW;
        case APPWS_FULLSCREEN:  return WS_POPUP;
    }

    return WS_OVERLAPPEDWINDOW;
}


// =====================================================================================
    BOOL App_IsMinimized(void)
// =====================================================================================
{
    return g_Minimized;
}

// =====================================================================================
    ulong App_GetTime(void)
// =====================================================================================
{
    return timeGetTime();
}


// =====================================================================================
    static void CalculateWindowDimensions(int *nWidth, int *nHeight, DWORD dwStyle, DWORD dwExStyle, BOOL bMenu)
// =====================================================================================
// Modifies nWidth, nHeight
{
	
	if(dwStyle == APPWS_NORMAL)
    {
        RECT windowrect;	

        if(*nWidth  == 0) *nWidth = 640;
        if(*nHeight == 0) *nHeight = 400;

		// adjust window so -client area- is of the specified size
		windowrect.top = 0; windowrect.left = 0;
		windowrect.right = *nWidth; windowrect.bottom = *nHeight;
		
		AdjustWindowRectEx(&windowrect,Win98Stylize(dwStyle),bMenu,dwExStyle);

		*nWidth = windowrect.right - windowrect.left;
		*nHeight = windowrect.bottom - windowrect.top;
	}
}

static CHAR errmsg_winfail[] = "Could not create the application window.";

// ===============================================================================================
    bool App_CreateWindow(APPWND *wnd)
// ===============================================================================================
{	
	WNDCLASSEX  wcx;
	BOOL        bMenu;
	DWORD       dwExStyle = 0;

	// default values	
	if(!wnd->WindowName) wnd->WindowName = "untitled application";
	//if(!wnd->Style)      wnd->Style = APPWS_FULLSCREEN;
	
	if(wnd->MenuName == NULL) bMenu = FALSE; else bMenu = true;
	CalculateWindowDimensions(&wnd->Width, &wnd->Height, wnd->Style, dwExStyle, bMenu);

	g_MessageMap = wnd->MessageMap;		// message map

	CLEAR_STRUCT(wcx);
	// register new window class
	wcx.cbSize        = sizeof(WNDCLASSEX);
	wcx.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wcx.lpfnWndProc   = WindowProc;
	wcx.cbClsExtra    = wcx.cbWndExtra = 0;
	wcx.hInstance     = hAppInstance;

    if(wnd->IconName)
        wcx.hIcon     = LoadIcon(hAppInstance, wnd->IconName);
	else 
        wcx.hIcon     = LoadIcon(NULL, IDI_WINLOGO);

    wcx.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
	wcx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcx.lpszMenuName  = wnd->MenuName;
	wcx.lpszClassName = APPCLASSNAME;
	
	if(wnd->IconSmName) wcx.hIconSm = LoadIcon(hAppInstance, wnd->IconSmName);
	else if(wnd->IconName) wcx.hIconSm = LoadIcon(hAppInstance, wnd->IconName);
	else wcx.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	

	if( !RegisterClassEx(&wcx) ) 
    {   _mmlog("App_CreateWindow > WindowClassEx Failure!");
        _mmerr_set(MMERR_FATAL, errmsg_winfail);
		return false; 
	}
	
	// create our window.  It starts out very plain and quite invisible, and
    // we customize it to whatever we want later.

	g_hAppWnd = CreateWindowEx(dwExStyle, APPCLASSNAME, 
		wnd->WindowName, 
		Win98Stylize(wnd->Style) | WS_VISIBLE,
		100, 100, wnd->Width, wnd->Height,
		HWND_DESKTOP,
		NULL,
		hAppInstance,
		NULL);

	if(!g_hAppWnd)
	{   _mmlog("App_CreateWindow > CreateWindowEx Failure!");
        _mmerr_set(MMERR_FATAL, errmsg_winfail);
		return false; 
	}

	// if there is a menu, but we are starting full screen... leave the menu off
	// (this should probably be optional!)
	if(bMenu == true && (wnd->Style == APPWS_FULLSCREEN))
	{	g_hOldMenu = GetMenu(g_hAppWnd);
		SetMenu(g_hAppWnd, NULL);
	}

	return true;
}

// ===============================================================================================
    bool App_AdjustWindow(uint dwStyle, int x, int y, int Width, int Height, BOOL bMenu)
// ===============================================================================================
{	
	DWORD dwExStyle = 0;
//	int nWidth, nHeight;

	ShowWindow(g_hAppWnd, SW_HIDE);

	SetWindowLong(g_hAppWnd, GWL_STYLE, Win98Stylize(dwStyle));

	// drop/add menu
	if(bMenu == false)
	{
		if(g_hOldMenu) return false;		// strange error

		g_hOldMenu = GetMenu(g_hAppWnd);
		SetMenu(g_hAppWnd, NULL);
	}
	else if(bMenu == true)
	{
		if( g_hOldMenu )
		{
			SetMenu(g_hAppWnd, g_hOldMenu);    
			g_hOldMenu = NULL;
		}
	}	
	
	CalculateWindowDimensions(&Width, &Height, dwStyle, dwExStyle, bMenu);
	MoveWindow(g_hAppWnd, x, y, Width, Height, true);

	if(dwStyle == APPWS_NORMAL) ShowWindow(g_hAppWnd, SW_SHOW);

    return true;
}



// =====================================================================================
    void App_Close(void)
// =====================================================================================
{
	PostMessage(g_hAppWnd, WM_CLOSE, 0, 0); 
	g_Hidden = true; 
}


// =====================================================================================
    char * App_GetCommandLine(void)
// =====================================================================================
{
	return g_szCommandLine;
}


// =====================================================================================
    HWND App_GetWindowHandle(void)
// =====================================================================================
{
	return g_hAppWnd;
}


// =====================================================================================
    int App_MessageBox(const CHAR *title, const CHAR *text, int style)
// =====================================================================================
{
    int  result;

    // convert App style flags to Windows style flags
    // [...]
    
    result = MessageBox(NULL, text, title, style);

    // convert windows return code to App return code.
    // [...]

    return result;
}

extern BOOL CALLBACK AboutProc(HWND hWnd, uint uMsg, WPARAM wParam, LPARAM lParam);

// =====================================================================================
    int App_DialogBox(uint res)
// =====================================================================================
{
    uint  result;
    
    result = DialogBox(hAppInstance, MAKEINTRESOURCE(res), NULL, /*g_hAppWnd,*/ AboutProc);

    return result;
}


// =====================================================================================
    void App_SetState(LPFUNCV renderfunc, LPFUNCV logicfunc)
// =====================================================================================
{
    g_RenderFunc = renderfunc;
    g_LogicFunc  = logicfunc;

    App_Paint();
}


// =====================================================================================
    void App_SetRenderer(LPFUNCV renderfunc)
// =====================================================================================
{
    g_RenderFunc = renderfunc;
    App_Paint();
}

    
// =====================================================================================
    void App_Run(void)
// =====================================================================================
// Sets initial MainLoopFunc (game state) and then waits in message loop until quit received
{
	MSG msg;

	/*if(!funcptr)
    {   _mmlog("App_Run > Invalid Parameters: funcptr callback is NULL.");
		return; 
	}*/
	
	//App_SetState(renderfunc, logicfunc, timerfunc, logictimer);
    //App_Paint();

	//message loop
	while(TRUE)
	{
		if( PeekMessage(&msg, NULL,0,0,PM_REMOVE) )
		{
			if(msg.message == WM_QUIT) break;

            TranslateMessage(&msg);		// translate accelerator keys
			DispatchMessage(&msg);
        }

        if(!g_Minimized)
        {   if(g_LogicFunc) g_LogicFunc();
        } else Sleep(20);
	}

    // Better make sure the timer is shut down, or else it'll keep
    // going without us!
    
    AppTimer_Exit();
}


// =====================================================================================
    int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
// =====================================================================================
{	
	HWND hWnd;
	
	hWnd = FindWindow (APPCLASSNAME, NULL);
	if (hWnd)
    {	// We found another version of ourself. Let's defer to it:
		if (IsIconic(hWnd))
			ShowWindow(hWnd, SW_RESTORE);

		SetForegroundWindow (hWnd);
		return FALSE;
	}

	hAppInstance = hInstance;
	lstrcpy(g_szCommandLine,lpCmdLine);
	
	return Main();		// call main() function		
}


// =====================================================================================
    LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
// =====================================================================================
{	
	int i;

	if( g_MessageMap == NULL ) return DefWindowProc(hWnd, uMsg, wParam, lParam);	// no message map!

	i = 0;

    if(uMsg == WM_ERASEBKGND) return 0;

    if(uMsg == WM_COMMAND) 
	{	while( g_MessageMap[i].uMsg != 0 )	// loop through message map
		{	if(	uMsg == g_MessageMap[i].uMsg && LOWORD(wParam) == g_MessageMap[i].wParam )	// find the message to process
			 	return g_MessageMap[i].MsgHandlerFunc(uMsg, wParam, lParam);
			i++;
		}

		return 0;	// don't process default message (?)
    } else
	{	while( g_MessageMap[i].uMsg != 0 )
		{	if(	uMsg == g_MessageMap[i].uMsg )	// find the message to process
				return (LRESULT)g_MessageMap[i].MsgHandlerFunc(uMsg, wParam, lParam);
			i++;
		}
	}
	
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


// =====================================================================================
    uint App_OnPaint(WMWL)
// =====================================================================================
{
    // Paint the screen, and stuff.
    if(!g_Minimized && !g_Hidden && g_RenderFunc)
    {   g_RenderFunc();
        ValidateRect(g_hAppWnd,NULL);
        g_Invalid = 0;
    }
	return 0;
}


// =====================================================================================
    uint App_OnDestroy(WMWL)
// =====================================================================================
{
	PostQuitMessage(0);
	return 0;
}


// =====================================================================================
    uint App_OnSize(WMWL)
// =====================================================================================
{
    // See if we're minimized/restored.

    switch(wParam)
    {   case SIZE_MINIMIZED:  g_Minimized = true;   break;
        case SIZE_MAXIMIZED:
        case SIZE_RESTORED:   g_Minimized = false;  break;

        case SIZE_MAXHIDE:    g_Hidden    = true;   break;
        case SIZE_MAXSHOW:    g_Hidden    = false;  break;
    }
    return 0;
}


// =====================================================================================
    void App_Paint(void)
// =====================================================================================
{
    if(!g_Invalid)
    {   InvalidateRect(g_hAppWnd, NULL, 0);
        g_Invalid = 1;
    }
}

// =====================================================================================
//  AppTimer (maybe this should be in a separate module)
//
//  Stuff for setting up a timer which can be used to execute game logic and especially
//  for updating Mikmod.  This Windows-version uses the Windows multimedia timer which
//  has proven itself to be very reliable and quite simple to use as well.
// =====================================================================================

volatile ulong systemtime, timer;

typedef struct TIMER_THINGIE
{   struct TIMER_THINGIE *next;
    volatile uint counter;
    uint          message;               // message value.
    void          *data;                 // data pointer

    void          (*proc)(uint message, void *data);
} TIMER_THINGIE;

static TIMER_THINGIE *eventlist    = NULL;
static MMRESULT      mmtimerhandle = 0;

// =====================================================================================
    static void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
// =====================================================================================
{
//    TIMER_THINGIE  *cruise, *old;
    
    systemtime++;
    timer++;

    if(g_TimerFunc) g_TimerFunc();

    // sift through our list of events and trigger any that are set to go off.

/*    cruise = eventlist;

    while(cruise)
    {   if(cruise->counter)
            cruise->counter--;
        else
        {   TIMER_THINGIE *freeme;
        
            // our timer has ticked to 0.  Throw it, then delete it!
            if(cruise->proc) cruise->proc(cruise->message, cruise->data);
            if(cruise == eventlist)
            {   eventlist = cruise->next;
                old = cruise;
                cruise = cruise->next;
            } else
                old->next = cruise->next;

            freeme = cruise;
            cruise = cruise->next;
            _mm_free(freeme);
        }
    }*/
}

// Only one timer can be initialized

// =====================================================================================
    BOOL AppTimer_Init(LPFUNCV timerfunc, int hz)
// =====================================================================================
{
    if(!timerfunc) return -1;
    
    timer       = systemtime = 0;
    eventlist   = NULL;

    g_TimerFunc  = timerfunc;

    if(mmtimerhandle)
    {   timeKillEvent(mmtimerhandle);
        mmtimerhandle = 0;
        timeEndPeriod(0);
    }

    timeBeginPeriod(0);
    mmtimerhandle = timeSetEvent(1000/hz,0,TimeProc,0,TIME_PERIODIC);

    if(!mmtimerhandle)
    {   _mmlog("Timer > timeSetEvent failure.");
        _mmerr_set(MMERR_INITIALIZING_DEVICE, "Oh no! Could not grab the Windows multimedia timer.\r\nCan't run without it, sorry.");
    }

    return -1;
}

// =====================================================================================
    void AppTimer_Exit(void)
// =====================================================================================
{
    TIMER_THINGIE  *cruise;

    if(mmtimerhandle)
    {   timeKillEvent(mmtimerhandle);
        mmtimerhandle = 0;
    }
    timeEndPeriod(0);

    cruise = eventlist;
    
    while(cruise)
    {   
        TIMER_THINGIE *old = cruise;
        cruise = cruise->next;
        _mm_free(old, NULL);
    }
}


// =====================================================================================
    BOOL AppTimer_SetEvent(uint framedelay, void (*proc)(uint message, void *data), uint message, void *data)
// =====================================================================================
{
    TIMER_THINGIE *narf;

    if(!(narf = (TIMER_THINGIE *)_mm_malloc(sizeof(TIMER_THINGIE)))) return 0;

    narf->counter = framedelay;
    narf->message = message;
    narf->data    = data;
    
    // NARF!  Add me to the list!
    
    narf->next = eventlist;
    eventlist  = narf;

    return 1;
}

