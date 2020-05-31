/*
   The VDRIVER  -  It Lives!

   Windows95 DirectX 5/6 driver.

   Like.  A directDraw Layer.  And Stuff.  Phear it.  Kaboom.

*/

#include <windows.h>
#include "ddraw.h"
#include "vdriver.h"

static void BuildBits(uint mask, uint *pos, uint *len)
{
    int   i;
    for(i=0, *pos=0, *len=0; i<32; i++)
    {   if(mask & (1ul<<i))
        {   // bit set - inc length
            if(*len)
                (*len)++;
            else
            {   *len = 1;
                *pos = i;
            }
        } else
        {   // bit clear
            if(*len) return;
        }
    }
}

static void SetDDerror(int code, const CHAR *crap)
{
    static char *error;

    switch (code) {
        case DDERR_GENERIC:
            error = "Undefined error!";
            break;
        case DDERR_EXCEPTION:
            error = "Exception encountered";
            break;
        case DDERR_INVALIDOBJECT:
            error = "Invalid object";
            break;
        case DDERR_INVALIDPARAMS:
            error = "Invalid parameters";
            break;
        case DDERR_NOTFOUND:
            error = "Object not found";
            break;
        case DDERR_INVALIDRECT:
            error = "Invalid rectangle";
            break;
        case DDERR_INVALIDCAPS:
            error = "Invalid caps member";
            break;
        case DDERR_INVALIDPIXELFORMAT:
            error = "Invalid pixel format";
            break;
        case DDERR_OUTOFMEMORY:
            error = "Out of memory";
            break;
        case DDERR_OUTOFVIDEOMEMORY:
            error = "Out of video memory";
            break;
        case DDERR_SURFACEBUSY:
            error = "Surface busy";
            break;
        case DDERR_SURFACELOST:
            error = "Surface was lost";
            break;
        case DDERR_WASSTILLDRAWING:
            error = "DirectDraw is still drawing";
            break;
        case DDERR_INVALIDSURFACETYPE:
            error = "Invalid surface type";
            break;
        case DDERR_NOEXCLUSIVEMODE:
            error = "Not in exclusive access mode";
            break;
        case DDERR_NOPALETTEATTACHED:
            error = "No palette attached";
            break;
        case DDERR_NOPALETTEHW:
            error = "No palette hardware";
            break;
        case DDERR_NOT8BITCOLOR:
            error = "Not 8-bit color";
            break;
        case DDERR_EXCLUSIVEMODEALREADYSET:
            error = "Exclusive mode was already set";
            break;
        case DDERR_HWNDALREADYSET:
            error = "Window handle already set";
            break;
        case DDERR_HWNDSUBCLASSED:
            error = "Window handle is subclassed";
            break;
        case DDERR_NOBLTHW:
            error = "No blit hardware";
            break;
        case DDERR_IMPLICITLYCREATED:
            error = "Surface was implicitly created";
            break;
        case DDERR_INCOMPATIBLEPRIMARY:
            error = "Incompatible primary surface";
            break;
        case DDERR_NOCOOPERATIVELEVELSET:
            error = "No cooperative level set";
            break;
        case DDERR_NODIRECTDRAWHW:
            error = "No DirectDraw hardware";
            break;
        case DDERR_NOEMULATION:
            error = "No emulation available";
            break;
        case DDERR_NOFLIPHW:
            error = "No flip hardware";
            break;
        case DDERR_NOTFLIPPABLE:
            error = "Surface not flippable";
            break;
        case DDERR_PRIMARYSURFACEALREADYEXISTS:
            error = "Primary surface already exists";
            break;
        case DDERR_UNSUPPORTEDMODE:
            error = "Unsupported mode";
            break;
        case DDERR_WRONGMODE:
            error = "Surface created in different mode";
            break;
        case DDERR_UNSUPPORTED:
            error = "Operation not supported";
            break;
    }
    _mmlog("DDraw_Init > %s > %s",crap,error);
    return;
}


typedef struct _HWSURFACE
{   LPDIRECTDRAWSURFACE3 surface;
    DDSURFACEDESC        desc;
} _HWSURFACE;

typedef struct _HWDATA
{   HINSTANCE instance;
    struct
    {   HWND handle;
        int  xloc, yloc;
    } window;

    LPDIRECTDRAWCLIPPER clipper;
    LPDIRECTDRAW2    dd;         // direct draw handle!
    _HWSURFACE       pri, sec;   // primary and secondary surfaces
    HINSTANCE        dll;        // ddraw dll handle} _HWDATA;

    VD_RENDERER      *vr;
} _HWDATA;

_HWDATA hwdata;

void drvDDraw_SetData(HINSTANCE instance, HWND handle, int xpos, int ypos)
{
    hwdata.instance      = instance;
    hwdata.window.handle = handle;
    hwdata.window.xloc   = xpos;
    hwdata.window.yloc   = ypos;
}


#define dxdd  hwdata.dd
#define dxdll hwdata.dll
#define dxpri hwdata.pri
#define dxsec hwdata.sec

static void DX_WindowFlip(VD_RENDERER *vr)

// This procedure currently does a very big and stupid blockblt from the
// secondary (work) surface to the primary (window display) surface.
// NOTE: The code that allows for enable/disable of surface scaling is in
// the window resize event handler in DX_WinProc.

{    
    HRESULT res;

    // stupid wait mode for now.  Will toy with async mode later.

    if(!vr->visible)
    {   res = IDirectDrawSurface3_Restore(dxpri.surface);
        if(res == DD_OK)
        {   vr->visible = 1;
            IDirectDrawSurface3_Restore(dxsec.surface);
        }
    }

    if(vr->visible)
    {   RECT  dr;
        POINT pt;

        GetClientRect(hwdata.window.handle,&dr);

        pt.x = dr.left; pt.y = dr.top;
        ClientToScreen(hwdata.window.handle, &pt);
        dr.left = pt.x; dr.top = pt.y;

        pt.x = dr.right; pt.y = dr.bottom;
        ClientToScreen(hwdata.window.handle,&pt);
        dr.right = pt.x; dr.bottom = pt.y;

        res = IDirectDrawSurface3_Blt(dxpri.surface, &dr, dxsec.surface, NULL, DDBLT_WAIT, NULL);

        if(res == DDERR_SURFACELOST)
        {   res = IDirectDrawSurface3_Restore(dxpri.surface);

            if(res != DD_OK)
            {   vr->visible = 0;
                return;
            }

            IDirectDrawSurface3_Restore(dxsec.surface);
        }

        if(res != DD_OK)
        {   vr->visible = 0;
            return;
        }

        //IDirectDrawSurface3_Restore(dxsec.surface);

    }
}

/*static void DX_WindowFlip_24(VD_RENDERER *vr)
{    
    HRESULT res;

    if(!vr->visible)
    {   res = IDirectDrawSurface3_Restore(dxpri.surface);
        if(res == DD_OK)
        {   vr->visible = 1;
            IDirectDrawSurface3_Restore(dxsec.surface);
        }
    }

    if(vr->visible)
    {   RECT  dr;
        POINT pt;
        UBYTE *crappy, *sappy;
        uint  t,i;

        GetClientRect(hwdata.window.handle,&dr);

        pt.x = dr.left; pt.y = dr.top;
        ClientToScreen(hwdata.window.handle, &pt);
        dr.left = pt.x; dr.top = pt.y;

        pt.x = dr.right; pt.y = dr.bottom;
        ClientToScreen(hwdata.window.handle,&pt);
        dr.right = pt.x; dr.bottom = pt.y;

        //res = IDirectDrawSurface3_Blt(d xpri.surface, &dr, dxsec.surface, NULL, DDBLT_WAIT, NULL);

        res = IDirectDrawSurface3_Lock(dxpri.surface, 0, &dxpri.desc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);

        if(res == DDERR_SURFACELOST)
        {   res = IDirectDrawSurface3_Restore(dxpri.surface);

            if(res != DD_OK)
            {   vr->visible = 0;
                return;
            }
            IDirectDrawSurface3_Restore(dxsec.surface);
            res = IDirectDrawSurface3_Lock(dxpri.surface, 0, &dxpri.desc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);
        }

        if(res != DD_OK)
        {   vr->visible = 0;
            return;
        }

        crappy = &((UBYTE *)dxpri.desc.lpSurface)[(dr.top * dxpri.desc.lPitch) + dr.left * 3];
        sappy  = vr->surface.vidmem;

        for(t=0; t<vr->surface.physheight; t++, crappy+=dxpri.desc.lPitch, sappy+=vr->surface.bytewidth)
        {   UBYTE *crap, *sap;
            for(i=0, crap=crappy, sap=sappy; i<vr->surface.physwidth; i++, crap+=3, sap+=4)
            {   crap[0] = sap[0];
                crap[1] = sap[1];
                crap[2] = sap[2];
            }
        }

        //IDirectDrawSurface3_Restore(dxsec.surface);
        IDirectDrawSurface3_Unlock(dxpri.surface,0);
    }
}*/


static void DX_WindowFlipRect(VD_RENDERER *vr, RECT *rect)

// This guy only works if window-resizing makes a different canvas size.
// This guy doesn't work if you want the window resizing to stretch the image.

{
    // stupid wait mode for now.  Will toy with async mode later.
    IDirectDrawSurface3_Blt(dxpri.surface, rect, dxsec.surface, rect, DDBLT_WAIT, NULL);
}

#define VD_VSYNC 1

static void DX_DirectFlip(VD_RENDERER *vr)
{
    HRESULT res;

    res = IDirectDrawSurface3_Blt(dxpri.surface, NULL, dxsec.surface, NULL, DDBLT_WAIT, NULL);
    if(res == DDERR_SURFACELOST)
        res = IDirectDrawSurface3_Restore(dxpri.surface);

    if(res != DD_OK)
    {   vr->visible = 0;
        return;
    }
}

static void DX_DirectFlip2(VD_RENDERER *vr)
{
    HRESULT hr;
    hr = IDirectDrawSurface3_Flip(dxpri.surface, 0, DDFLIP_WAIT | ((vr->flags & VD_VSYNC) ? 0 : DDFLIP_NOVSYNC));

    if(hr == DDERR_SURFACELOST)
        hr = IDirectDrawSurface3_Restore(dxpri.surface);

    if(hr != DD_OK)
    {   vr->visible = 0;
        return;
    }

    IDirectDrawSurface3_Lock(dxsec.surface, 0, &dxsec.desc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);

    if(hr == DDERR_SURFACELOST)
        hr = IDirectDrawSurface3_Restore(dxsec.surface);

    if(hr != DD_OK)
    {   vr->visible = 0;
        return;
    }

    vr->visible = 1;

    // update our surface with the new page ptr:
    vr->surface.vidmem = (UBYTE *)dxsec.desc.lpSurface;

    IDirectDrawSurface3_Unlock(dxsec.surface,0);
}


static LRESULT APIENTRY WndProc(HWND hWnd, UINT message,WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {   case WM_ACTIVATE:
            if(LOWORD(wParam)==WA_INACTIVE)
                hwdata.vr->visible = 0;
            else
                hwdata.vr->visible = 1;
        break;

        case WM_ACTIVATEAPP:
            if((BOOL)wParam==0)
                hwdata.vr->visible = 0;
            else
                hwdata.vr->visible = 1;
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
        return 0;
        
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


/*static BOOL DX_CreateWindow(VD_RENDERER *vr)
{
    WNDCLASS WndClass;
    //HRESULT  hr;

    //initialise the window
    memset(&WndClass,0,sizeof(WNDCLASS));

    WndClass.hCursor       = LoadCursor(0, IDC_ARROW);
    WndClass.hIcon         = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
    WndClass.lpszClassName = "vdriver window";
    WndClass.hInstance     = hwdata.instance;
    WndClass.lpfnWndProc   = WndProc;
    RegisterClass(&WndClass);

    vr->WindowHandle = CreateWindow(vr->ApplicationName, vr->ApplicationName,
                        (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),
                        0, 0, 0, 0, NULL, NULL, (HANDLE)vr->Instance, NULL);

    if(!hwdata.window.handle)
    {   printlog("DirectDraw_Init > Couldn't create window");
        return 1;
    }

    ShowWindow(hwdata.window.handle, SW_HIDE);

    return 0;
}*/


static BOOL DX_Detect(void)
{
    int          ddraw_ok;
    HINSTANCE    dll;

    ddraw_ok = 0;

    dll = LoadLibrary("DDRAW.DLL");
    if (dll)
    {   HRESULT      (WINAPI *DDeadGates)(GUID *,LPDIRECTDRAW *,IUnknown *);
        LPDIRECTDRAW sucks;

        DDeadGates = (void *)GetProcAddress(dll, "DirectDrawCreate");
        if(DDeadGates && (DDeadGates(NULL,&sucks,NULL) == DD_OK))
        {   if(IDirectDraw_SetCooperativeLevel(sucks, NULL, DDSCL_NORMAL) == DD_OK)
            {   DDSURFACEDESC        desc;
                LPDIRECTDRAWSURFACE  DDrawSurf;
                LPDIRECTDRAWSURFACE3 DDrawSurf3;

                // Try to create a DirectDraw Surface....
                memset(&desc, 0, sizeof(desc));
                desc.dwSize = sizeof(desc);
                desc.dwFlags = DDSD_CAPS;
                desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
                if (IDirectDraw_CreateSurface(sucks, &desc, &DDrawSurf, NULL) == DD_OK)
                {   if (IDirectDrawSurface_QueryInterface(DDrawSurf, &IID_IDirectDrawSurface3, (LPVOID *)&DDrawSurf3) == DD_OK)
                    {   ddraw_ok = 1;
                        IDirectDrawSurface3_Release(DDrawSurf3);
                    }
                    IDirectDrawSurface_Release(DDrawSurf);
                }
            }
            IDirectDraw_Release(sucks);
        }
        FreeLibrary(dll);
    }
    return(ddraw_ok);
}

static BOOL DX_SetMode(VD_RENDERER *vr, int *xres, int *yres, int *bytespp, uint flags)
{
    HRESULT             result;
    LPDIRECTDRAWSURFACE mofo;    // temporary bullshit for queryinterface

    memset(&dxpri.desc, 0, sizeof(dxpri.desc));
    dxpri.desc.dwSize         = sizeof(dxpri.desc);
    dxpri.desc.dwFlags        = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    dxpri.desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
    dxpri.desc.dwBackBufferCount = 1;

    if(!(flags & VMODE_WINDOWED))
    {   int   i;

        // Fullscreen means we need to set the video mode...

        IDirectDraw_SetCooperativeLevel(dxdd,hwdata.window.handle, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT);

        vr->resizing = 1;    // make sure our windproc ignores the resize.. sigh.
        SetWindowPos(hwdata.window.handle, NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_NOCOPYBITS | SWP_NOZORDER);
        vr->resizing = 0;

        ShowWindow(hwdata.window.handle, SW_SHOW);
        while(GetForegroundWindow() != hwdata.window.handle)
        {   SetForegroundWindow(hwdata.window.handle);
            Sleep(100);
        }

        result = DDERR_UNSUPPORTEDMODE;
        i      = 0;
        flags |= VMODE_WINDOWED;   // it gets set back if we find a legit fullscreen mode

        while(result != DD_OK)
        {   result = IDirectDraw2_SetDisplayMode(dxdd, xres[i], yres[i], bytespp[i]*8, 0, 0);  // parm 4 is refreshrate.
            switch(result)
            {   case DDERR_UNSUPPORTEDMODE:
                    // Mode unsupported, so check next desired mode -
                    i++;
                break;

                case DDERR_NOEXCLUSIVEMODE:
                    // We don't support fullscreen mode, so force windowed mode ..
                    result = DD_OK;
                break;

                case DD_OK:
                    flags &= ~VMODE_WINDOWED;
                    //dxpri.desc.dwWidth           = xres[i];
                    //dxpri.desc.dwHeight          = yres[i];
                break;
            }
        }
    }

    if(flags & VMODE_WINDOWED)
    {   result = IDirectDraw2_SetCooperativeLevel(dxdd,hwdata.window.handle,DDSCL_NORMAL);
        if(result != DD_OK)
        {   SetDDerror(result, "SetCooperative Failure");
            return 1;
        }

        flags &= ~VMODE_DOUBLEBUFFER;

        vr->resizing = 1;    // make sure our windproc ignores the resize.. sigh.
        SetWindowPos(hwdata.window.handle, NULL, hwdata.window.xloc, hwdata.window.yloc,
                    xres[0] + (GetSystemMetrics(SM_CXFRAME)*2),
                    yres[0] + (GetSystemMetrics(SM_CYFRAME)*2) + GetSystemMetrics(SM_CYCAPTION),
                    SWP_NOCOPYBITS | SWP_NOZORDER);
        vr->resizing = 0;

        ShowWindow(hwdata.window.handle, SW_SHOW);
        while(GetForegroundWindow() != hwdata.window.handle)
        {   SetForegroundWindow(hwdata.window.handle);
            Sleep(100);
        }
    }

    if(!(flags & VMODE_DOUBLEBUFFER))
    {   dxpri.desc.dwFlags          &= ~DDSD_BACKBUFFERCOUNT;
        dxpri.desc.ddsCaps.dwCaps   &= ~(DDSCAPS_COMPLEX | DDSCAPS_FLIP);
        dxpri.desc.dwBackBufferCount = 0;
    }

    if((result = IDirectDraw2_CreateSurface(dxdd, &dxpri.desc, &mofo, 0)) != DD_OK)
    {   // you know this error MIGHT actually matter!
        SetDDerror(result, "CreateSurface Failure");
        return 1;
    }

    IDirectDrawSurface_QueryInterface(mofo, &IID_IDirectDrawSurface3, &dxpri.surface);
    IDirectDrawSurface_Release(mofo);
    
    // Get our primary surface characteristics.

    memset(&dxpri.desc, 0, sizeof(dxpri.desc));
    dxpri.desc.dwSize  = sizeof(dxpri.desc);
    dxpri.desc.dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    result = IDirectDrawSurface3_GetSurfaceDesc(dxpri.surface, &dxpri.desc);
    if(result != DD_OK)
    {   SetDDerror(result, "GetPrimarySurfaceDesc");
        return 1;
    }

    // We're stupid.  Everyone has RGB as far as we care
    //if (!(ddsd.ddpfPixelFormat.dwFlags&DDPF_RGB))

    // We don't support 8 bit palette mode, but we don't care.
    // let them play with garbage!
    //if(bsd.ddpfPixelFormat.dwRGBBitCount == 8)

    // Find out the bit-format of the surface so we can match it internally

    vr->surface.bytespp    = ((dxpri.desc.ddpfPixelFormat.dwRGBBitCount-1) / 8) + 1;
    vr->surface.red_mask   = dxpri.desc.ddpfPixelFormat.dwRBitMask;
    vr->surface.green_mask = dxpri.desc.ddpfPixelFormat.dwGBitMask;
    vr->surface.blue_mask  = dxpri.desc.ddpfPixelFormat.dwBBitMask;

    vr->surface.ptilewidth = TILE_WIDTH;
    vr->surface.tilewidth  = vr->surface.ptilewidth * vr->surface.bytespp;
    vr->surface.tileheight = TILE_HEIGHT;
    vr->surface.ptilesize  = vr->surface.ptilewidth * vr->surface.tileheight;
    vr->surface.tilesize   = vr->surface.tilewidth * vr->surface.tileheight;

    vr->surface.intensity  = 128;
    vr->surface.alpha      = 128;

    switch(vr->surface.bytespp)
    {
        case 2:      // 16 bit powerhousing!
            vr->surface.spriteblitter = gfxHi_SpriteBlitter;
            vr->surface.tileopaque    = gfxHi_TileOpaque;
            vr->surface.tiletrans     = gfxHi_TileTrans;
            vr->surface.tilefunky     = gfxHi_TileFunky;
            vr->surface.line          = gfxHi_line;
            vr->surface.v_line        = gfxHi_vline;
            vr->surface.h_line        = gfxHi_hline;
        break;

        case 3:     // 24 bit lameness!
        /*    vr->surface.spriteblitter = gfxTrue_SpriteBlitter;
            vr->surface.tileopaque    = gfxTrue_TileOpaque;
            vr->surface.tiletrans     = gfxTrue_TileTrans;
            vr->surface.tilefunky     = gfxTrue_TileFunky;
            vr->surface.line          = gfxTrue_line;
            vr->surface.v_line        = gfxTrue_vline;
            vr->surface.h_line        = gfxTrue_hline;
        break;*/

        case 4:     // 32 bit eliteness!  BAM!
            vr->surface.spriteblitter = gfx32_SpriteBlitter;
            vr->surface.tileopaque    = gfx32_TileOpaque;
            vr->surface.tiletrans     = gfx32_TileTrans;
            vr->surface.tilefunky     = gfx32_TileFunky;
            vr->surface.line          = gfx32_line;
            vr->surface.v_line        = gfx32_vline;
            vr->surface.h_line        = gfx32_hline;
        break;
    }

    vr->surface.box           = gfx_box;
    vr->surface.fillbox       = gfx_fillbox;
    vr->surface.rect          = gfx_rect;

    if(flags & VMODE_WINDOWED)
    {   vr->NextRect = DX_WindowFlipRect;
        vr->NextPage = /*(vr->surface.bytespp==3) ? DX_WindowFlip_24 :*/ DX_WindowFlip;
    } else
    {   vr->NextRect = NULL;

        if(flags & VMODE_DOUBLEBUFFER)
        {   vr->NextPage = DX_DirectFlip2;

        } else
        {   vr->NextPage = DX_DirectFlip;
        }
    }

    //vr->NextPage    = (flags & VMODE_WINDOWED) ? DX_WindowFlip : ((flags & VMODE_DOUBLEBUFFER) ? DX_DirectFlip2 : DX_DirectFlip);
    //vr->ClearScreen = NULL; //(flags & VMODE_WINDOWED) ? DX_WindowFlipRect : NULL;

    // create our secondary page thingie
    if(flags & VMODE_DOUBLEBUFFER)
    {   memset(&dxsec.desc, 0, sizeof (dxsec.desc));
        dxsec.desc.dwSize = sizeof (dxsec.desc);
        dxsec.desc.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER | ((flags & VMODE_SYSMEM) ? DDSCAPS_SYSTEMMEMORY : 0);
        result = IDirectDrawSurface3_GetAttachedSurface(dxpri.surface,&dxsec.desc.ddsCaps, &dxsec.surface);
        if(result != DD_OK)
        {   SetDDerror(result, "GetAttachedSurface");
            return 1;
        }

        vr->surface.physwidth  = dxpri.desc.dwWidth;
        vr->surface.physheight = dxpri.desc.dwHeight;
        vr->surface.bytewidth  = dxpri.desc.lPitch;
    } else
    {   // Single buffer, so create a system memory buffer!
        //vr->surface.vidmem = (UBYTE *)malloc(xres[0] * yres[0] * vr->surface.bytespp);

        if(vr->surface.bytespp==3) vr->surface.bytespp = 4;

        memset(&dxsec.desc, 0, sizeof(dxsec.desc));
        dxsec.desc.dwSize    = sizeof(dxsec.desc);
        dxsec.desc.dwFlags   = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_CAPS;
        dxsec.desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

        dxsec.desc.dwWidth   = xres[0];
        dxsec.desc.dwHeight  = yres[0];
        dxsec.desc.lPitch    = xres[0] * vr->surface.bytespp;
        //dxsec.desc.lpSurface = (void *)vr->surface.vidmem;
 
        dxsec.desc.ddpfPixelFormat.dwSize        = sizeof(dxsec.desc.ddpfPixelFormat);
        dxsec.desc.ddpfPixelFormat.dwFlags       = DDPF_RGB;
        dxsec.desc.ddpfPixelFormat.dwRGBBitCount = 32; //dxpri.desc.ddpfPixelFormat.dwRGBBitCount;
        dxsec.desc.ddpfPixelFormat.dwRBitMask    = dxpri.desc.ddpfPixelFormat.dwRBitMask;
        dxsec.desc.ddpfPixelFormat.dwGBitMask    = dxpri.desc.ddpfPixelFormat.dwGBitMask;
        dxsec.desc.ddpfPixelFormat.dwBBitMask    = dxpri.desc.ddpfPixelFormat.dwBBitMask;
 
        result = IDirectDraw2_CreateSurface(dxdd, &dxsec.desc, &mofo, NULL);
        if(result != DD_OK)
        {   SetDDerror(result, "CreateSurface (Secondary)");
            return 1;
        }
        IDirectDrawSurface_QueryInterface(mofo, &IID_IDirectDrawSurface3, &dxsec.surface);
        IDirectDrawSurface_Release(mofo);

        vr->surface.physwidth  = xres[0];
        vr->surface.physheight = yres[0];
        vr->surface.bytewidth  = xres[0] * vr->surface.bytespp;
    }

    vr->surface.bytesize   = vr->surface.bytewidth * vr->surface.physheight;
    vr->surface.physsize   = vr->surface.physwidth * vr->surface.physheight;

    // Determine the extra bitfield information
    BuildBits(vr->surface.red_mask,&vr->surface.red_fieldpos,&vr->surface.red_fieldsize);
    BuildBits(vr->surface.green_mask,&vr->surface.green_fieldpos,&vr->surface.green_fieldsize);
    BuildBits(vr->surface.blue_mask,&vr->surface.blue_fieldpos,&vr->surface.blue_fieldsize);

    // lock surfaces and get a pointer .. (do I really need to do it this way?)
    IDirectDrawSurface3_Lock(dxsec.surface, 0, &dxsec.desc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);
    vr->surface.vidmem = (UBYTE *)dxsec.desc.lpSurface;
    IDirectDrawSurface3_Unlock(dxsec.surface,0);
    //IDirectDrawSurface3_Lock(dxpri.surface, 0, &dxpri.desc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);
    // IDirectDrawSurface3_Unlock(dxpri.surface,0);

    if(flags & VMODE_WINDOWED)
    {   IDirectDraw2_CreateClipper(hwdata.dd, 0, &hwdata.clipper, NULL);
        IDirectDrawClipper_SetHWnd(hwdata.clipper, 0, hwdata.window.handle);
        IDirectDrawSurface3_SetClipper(hwdata.pri.surface,hwdata.clipper);
    }

    vr->flags = flags;

    UpdateWindow(hwdata.window.handle);
    return 0;
}


static BOOL DX_Init(VD_RENDERER *vr, int *xres, int *yres, int *bytespp, uint flags)
{
    LPDIRECTDRAW ugh;    // temp bullshit for getting the 'real' interface.
    HRESULT      (WINAPI *usuk)(GUID *,LPDIRECTDRAW *,IUnknown *);

    // Create our window, if it doesn't already exist!
    //if(!hwdata.window.handle) DX_CreateWindow(vr);

    // directdraw init ---- CRAP CRAP CRAP
    
    dxdll = LoadLibrary("DDRAW.DLL");
    usuk  = (void *)GetProcAddress(dxdll, "DirectDrawCreate");
    if(!usuk || (usuk(NULL,&ugh,NULL) != DD_OK)) return 1;

    IDirectDraw_QueryInterface(ugh, &IID_IDirectDraw2, (LPVOID *)&dxdd);
    IDirectDraw_Release(ugh);

    // this is for WndProc mostly.
    hwdata.vr = vr;

    return(DX_SetMode(vr, xres, yres, bytespp, flags));
}

/*static BOOL DX_GetCaps(VD_RENDERER *vr)
{
#if DIRECTDRAW_VERSION <= 0x500
    DDCAPS     caps;    // i axed 'im, 'wut you want, Gates motherfucker?!"
#else
    DDCAPS_DX5 caps;    // and then I put a cap in 'is 'ead. 
#endif
    HRESULT result;

    // Determine the hardware acceleration abilities and select the
    // appropriate stuff based on that.

    memset(&caps, 0, sizeof(caps));
    DDCaps.dwSize = sizeof(caps);
    IDirectDraw2_GetCaps(ddraw2, (DDCAPS *)caps, NULL);

    if(caps.dwCaps & DDCAPS_BLT)
    {   // hardware blitting supported - BAM!
        
    }

    if(DDCaps.dwCaps & DDCAPS_CANBLTSYSMEM)
    {   // Blitter from system to video memory.  Very handy.
    }

    if((DDCaps.dwCaps & DDCAPS_COLORKEY) && (DDCaps.dwCKeyCaps & DDCKEYCAPS_SRCBLT))
    {   // Kick it up a notch!  We can blit sprites with transparent parts!
        
    }
    
    if(DDCaps.dwCaps & DDCAPS_ALPHA)
    {   // Sike!  Microsoft hasn't implemented this yet!  WOOPS!
        // Yet?  Did I say yet?  I mean "Microsoft will never implement this."
    }

    // Video memory detection crap.
    {   DDSCAPS ddsCaps;
        DWORD total_mem;

        ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
        result = IDirectDraw2_GetAvailableVidMem(ddraw2, &ddsCaps, &total_mem, NULL);

        if (result != DD_OK)  total_mem = DDCaps.dwVidMemTotal; 
        this->info.video_mem = total_mem/1024;
    }
    return(0);
}*/

static void DX_Exit(VD_RENDERER *vr)
{
    if(dxdll)
    {   if(dxdd)
        {   if(dxsec.surface) IDirectDrawSurface_Release(dxsec.surface);
            if(dxpri.surface) IDirectDrawSurface_Release(dxpri.surface);
            IDirectDraw_Release(dxdd);
        }
        FreeLibrary(dxdll);
    }

    if(!(vr->flags & VMODE_DOUBLEBUFFER))
    {   if(vr->surface.vidmem) free(vr->surface.vidmem);
    }

    /*if (DDrawDLL)
    {   FreeLibrary(DDrawDLL);
        DDrawCreate = NULL;
        DDrawDLL = NULL;
    }*/
}


VDRIVER drv_ddraw6 =
{  NULL,
   "DivEnt DirectDraw 6",
   "0.666",

   // driver and mode-dependant procedures and functions

   DX_Detect,
   DX_Init,
   DX_Exit,
};
