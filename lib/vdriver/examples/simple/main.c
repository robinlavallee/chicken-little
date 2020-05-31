/*
  The 'Simple' VDRIVER Example

  By Jake Stine of Divine Entertainment

  Description:
  A basic demonstration of the useage of the VDRIVER.  Everything else should be
  pretty self-explainatory, or explained in comments.  This example does not
  use an event-driven messaging system for refreshing the screen.  In a more
  complete example, we should latch the windows timer and have it send a paint
  message to the engine.  Generally speaking, we want to use events as much as
  possible and avoid Sleep() as much as possible.

*/


#include <windows.h>
#include "vdriver.h"
#include "log.h"

// ==========================
//   The Varibles Section!
// ==========================

static BOOL quitflag;       // for controlling the 'game loop'

// Here is our default video mode search order.

static int xres[3]    = { 640, 640, 640 };
static int yres[3]    = { 480, 480, 480 };
static int bytespp[3] = { 4, 2, 3 };


// Windows-specific message handling.

static LRESULT APIENTRY WndProc(HWND hWnd, UINT message,WPARAM wParam, LPARAM lParam)
{
    // Currently, for simplicity, this procedure is very simple.  However, in a
    // well-structured windows application, all vdriver ini, deinit, and nextpage
    // functionality should be contained within, since it is all window-dependant.
    
    switch(message)
    {   case WM_DESTROY:
            quitflag = 1;
        return 0;
        
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}



int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE woops, CHAR *cmdline, int showme)
{
    
    // VDRIVER Variables (the VD_ should have given that away :)
    VD_RENDERER    *vr;         // the redering device object
    VD_SURFACE     *vs;         // pointer to the surface attached to our renderer.
    VD_COLOR       clr,clr2,black;

    // Windows-specific local variables.

    HWND        ourwindow;
    MSG         msg;
    WNDCLASS    WndClass;


    vr = NULL;           // so our wndProc doesn't bomb out.

    // Initialize logging
    log_init("log",LOG_SILENT);

    // Create the windowclass!
    // This is currently set to a windowed executable, since we cannot debug
    // fullscreen apps.  To switch to fullscreen, just uncomment the WS_POPUP
    // and comment the WS_OVERLAPPED.  Also, modify vdriver initialization to
    // and remove the VMODE_WINDOWED flag.

    memset(&WndClass,0,sizeof(WNDCLASS));

    WndClass.hCursor       = LoadCursor(0, IDC_ARROW);
    WndClass.hIcon         = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
    WndClass.lpszClassName = "vdriver window";
    WndClass.hInstance     = hinstance;
    WndClass.lpfnWndProc   = WndProc;
    RegisterClass(&WndClass);

    ourwindow = CreateWindowEx(0, "vdriver window", "Ball and Paddle Spectacular!",
        WS_OVERLAPPEDWINDOW,
        //WS_POPUP,
        0, 0, 0, 0, NULL, NULL, hinstance, NULL);

    if(!ourwindow)
    {   _mmlog("Init > Couldn't create window");
        return 1;
    }

    // The vdriver attaches to whatever the current window is.  In fact, I made the
    // mistake once of not showing my window before calling VD_Init, and vdriver
    // attached itself to my visual c IDE window!  That was pretty funny. :)
    
    ShowWindow(ourwindow, SW_SHOW);

    
    // ================================
    //   VDRIVER INITIALIZATION CODE
    // ================================
    
    // Register the drivers we want to use (currently only the basic directdraw driver)
    
    VD_RegisterDriver(&drv_ddraw);

    // Initialize the VDRIVER.
    
    vr = VD_InitRenderer(0, xres, yres, bytespp, VMODE_WINDOWED);
    vs = &vr->surface;

    // Define some nice colors.
    // The built in vdriver primitives (only horizontal/vertical lines and rectangles
    // right now, just enough for me to make a basic gui for editor tools), use the
    // VD_COLOR structure which contains color information in the form of the display
    // device pixel format.

    VD_MakeColor(vs, &clr, 0, 200, 0);        // green
    VD_MakeColor(vs, &clr2, 0, 0, 200);       // blue
    VD_MakeColor(vs, &black, 0, 0, 0);


    // =========================
    //   The Main Program Loop
    //
    // This is a primitive beast which just loops and displays stuff.  Fortunately,
    // directdraw is smart.  If you don't make any changes to the video buffer,
    // calls to NextPage(); will not actually use any cpu time.
    
    quitflag = 0;
    while(!quitflag)
    {
        // Draw a couple lines.
        // The vdriver functions work a lot like directx functions.  The function pointers
        // ane encapsulated within the structure.  Most of the time, the first parameter to
        // the function is the structure itself.

        // Type A.  Direct
        vs->h_line(vs, 0, 50, 400, &clr);
        vs->h_line(vs, 0, 100, 400, &clr2);

        // let the user see our beautiful artwork!
        vr->NextPage(vr);

        Sleep(4);

        // Windows-specific --
        // Dispatch messages
        while(PeekMessage(&msg, ourwindow, (int) NULL, (int) NULL, PM_REMOVE))
        {   TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    }

    // Shutdown stuff
    
    VD_Exit(vr);
    log_exit();

    return 0;
}
