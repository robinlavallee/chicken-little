/*


*/


#ifndef _app_h
#define _app_h_

//#include <windows.h>

#include "mmio.h"

typedef void FUNCV(void);
typedef FUNCV * LPFUNCV;

// we use memset instead of Windows-specific ZeroMemory because it is more
// cross-platform friendly.
#define CLEAR_STRUCT(s)			memset(&s,0,sizeof(s))

#define KEY_DOWN(vk_code)		((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

/* =====================================================================================
	Message map
======================================================================================== */
#define WMWL	uint uMsg, uint wParam, uint lParam

typedef uint FUNCWMWL(WMWL);
typedef FUNCWMWL * LPFUNCWMWL;

typedef struct
{
	uint       uMsg;
	uint       wParam;
	LPFUNCWMWL MsgHandlerFunc;
} APPMSGMAP;

extern uint App_OnPaint(WMWL);
extern uint App_OnDestroy(WMWL);
extern uint App_OnSize(WMWL);

#define BEGIN_MESSAGE_MAP(s)    APPMSGMAP s[] = { 
#define ON_COMMAND(id,func)     {WM_COMMAND,id,func},
#define ON_PAINT(func)          {WM_PAINT,0,func},
#define ON_DISPLAYCHANGE(func)  {WM_DISPLAYCHANGE,0,func},
#define ON_CREATE(func)	        {WM_CREATE,0,func},
#define ON_DESTROY(func)        {WM_DESTROY,0,func},
#define ON_SIZE(func)           {WM_SIZE,0,func},
#define END_MESSAGE_MAP()       {0,0,NULL} };

/* ===============================================================================================
	Window creation
=============================================================================================== */
typedef struct
{
    CHAR        *WindowName;     // "<program name>"
    CHAR        *IconName;       // MAKEINTRESOURCE(IDI_APPICON)
    CHAR        *IconSmName;     // MAKEINTRESOURCE(IDI_APPICON)
    DWORD        Style;          // APPWS_NORMAL || APPWS_FULLSCREEN
    int          Width;          // width of client area
    int          Height;         // height of client area
    const CHAR  *MenuName;       // menu name (NULL for none)
    APPMSGMAP   *MessageMap;     // message map structure
} APPWND;

#define	APPWS_NORMAL		0        // WS_OVERLAPPEDWINDOW
#define APPWS_FULLSCREEN	1        // WS_POPUP

/* ===============================================================================================
	Function declarations
=============================================================================================== */
extern bool    App_CreateWindow(APPWND *wnd);
extern bool    App_AdjustWindow(uint dwStyle, int x, int y, int Width, int Height, BOOL bMenu);
extern void    App_Close(void);

extern char   *App_GetCommandLine(void);
//HWND    App_GetWindowHandle(void);

extern void    App_Run(void);
extern void    App_SetState(LPFUNCV renderfunc, LPFUNCV logicfunc);
extern void    App_SetRenderer(LPFUNCV renderfunc);
extern void    App_Paint(void);
extern ulong   App_GetTime(void);
extern BOOL    App_IsMinimized(void);

extern DWORD   App_GetDXVersion(void);

extern int     App_MessageBox(const CHAR *title, const CHAR *text, int style);
extern int     App_DialogBox(uint res);


// ==============
//   Timer Jazz
// ==============

volatile ulong  systemtime,   // don't modify this.  only read it.
                timer;        // set this to 0 for easy wait-loops.

extern BOOL    AppTimer_Init(LPFUNCV timerfunc, int hz);
extern void    AppTimer_Exit(void);
extern BOOL    AppTimer_SetEvent(uint framedelay, void (*proc)(uint message, void *data), uint message, void *data);


#define _app_h
#endif
