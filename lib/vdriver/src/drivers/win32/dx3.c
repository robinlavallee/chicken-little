/*
 The VDRIVER - A Classic Video Game Engine

  By Jake Stine and Divine Entertainment (1997-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org
  For additional information and updates, see our website:
    http://www.divent.org

 Disclaimer:
  I could put things here that would make me sound like a first-rate california
  prune.  But I simply can't think of them right now.  In other news, I reserve
  the right to say I wrote my code.  All other rights are just abused legal
  tender in a world of red tape.  Have fun!

 ---------------------------------------------------
 Module: dx3.c

  Windows95 DirectX 3 driver.

  Like.  A directDraw Layer.  And Stuff.  Phear it.  Kaboom.
  This driver is so simple it really needs no explaination.  It may *look* complex,
  but that is simply the illusion of the totally real compleity of directdraw masking
  over the utter simplicity of the vdriver.  I am not bias.

 Updates
  Sept 8th, 2000    : Windowed mode is no longer directdraw native.  This driver
                      now automatically falls back on the windows DIB driver when
                      windowed mode is specified.
*/

#define   DIRECTDRAW_VERSION 0x0300

#include <windows.h>
#include <ddraw.h>
#include "vdriver.h"

// =====================================================================================
    static void SetDDerror(const int code, const CHAR *crap)
// =====================================================================================
// In comes the cryptic directdraw error code, and out comes the still-pretty-
// ambiguous diagnostic message.
{
    CHAR *error;

    switch (code)
    {   // it crossed my mind to turn this into one really huge ?: statement,
        // just for kicks, but something convinced me better of it.

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

        default:
            error = "Some god-forsaken error I'm not checking for.";
        break;
    }
    _mmlog("DDraw_Init > %s > %s",crap,error);
    return;
}

// =============================================
//   Air's Ambiguous Structures of Convolution
//
// I dunno.  I think I was trying to 1-up DirectDraw in the department of "exceptionally
// not user-friendly structure useage."  This personal little game of mine allowed me
// maintain my focus and fight through the struggles fo making this thing work.
//
// However, HWDATA does server a purpose.  All per-instance internal information is
// stored there (for when I make vdriver per-instance compatable).  This would allow
// me to create multiple surfaces for various windows.

typedef struct _HWSURFACE
{   LPDIRECTDRAWSURFACE  surface;
    DDSURFACEDESC        desc;
} _HWSURFACE;

typedef struct DX3_HWDATA
{   //HINSTANCE instance;

    // Hmm, A totally unnamed structure just to encapsulate some window variables.
    // For some reason this relatively pointless structure made my coding job a 
    // lot more fun though..

    struct
    {   HWND handle;
        //int  xloc, yloc;
    } window;

    // DirectDraw structural support group. -->>
    
    LPDIRECTDRAW2    dd;         // direct draw handle!
    _HWSURFACE       pri, sec;   // primary and secondary surfaces
    HINSTANCE        dll;        // ddraw dll handle} DX3_HWDATA;
    LPDIRECTDRAWCLIPPER clipper;

    // rectangle of the actual *viewable* area.  This is needed so that we blit
    // without copying the dirty buffers (left and right side of each scanline)
    
    RECT             vrect;

 /*   UBYTE           *vidmem;
    HBITMAP          screen_bmp;
    BITMAPINFO       binfo;*/

} DX3_HWDATA;

// A static variable, kinda defeating the purpose of having the HWDATA encappsulation
// at all, but it is a temporary measure.

static DX3_HWDATA hwdata;


// ===========================================================================
    static void DX3_SetMeUp(VD_RENDERER *vr)
// ===========================================================================
{
    // ** Alignment Features **
    // This is a somewhat sloppy method of me adding alignment features to vdriver.
    // if the VMODE_ALIGN option is enabled, then this driver ensures that the work-
    // space buffer scanline length is aligned to 8 bytes.  If the option is disabled,
    // the alignment is made on a four byte boundry.

    if(vr->flags & VMODE_ALIGN)
    {   vr->surface.bytewidth = ((vr->surface.xsize * vr->surface.bytespp) + 7) & ~7;
    } else
        vr->surface.bytewidth = ((vr->surface.xsize * vr->surface.bytespp) + 3) & ~3;

    // ** MMX Features ** (more sloppiness!)
    // If VMODE_MMX is enabled, then our workspace will be ensured of having at least
    // eight bytes of buffer to both the left and right sides of each scanline.
    // Coupled with the sprite buffers of the same type, sprites can always be blitted
    // using 8 byte loops - no remainders.

    if(vr->flags & VMODE_MMX)
    {   vr->surface.bytewidth += 16;
        hwdata.vrect.left      = 8 / vr->surface.bytespp;
    } else if(vr->surface.bytespp==2)
    {   vr->surface.bytewidth += 8;
        hwdata.vrect.left      = 2;
    }

    vr->surface.alignment = ((vr->flags & VMODE_ALIGN) ? 8 : 4) / vr->surface.bytespp;
        
    if(vr->flags & VMODE_MMX)
        vr->surface.prebuffer = 8 / vr->surface.bytespp;
    else
        vr->surface.prebuffer = (vr->surface.bytespp == 2) ? 2 : 0;

    vr->surface.physwidth  = vr->surface.bytewidth / vr->surface.bytespp;

    hwdata.vrect.top    = 0;
    hwdata.vrect.bottom = vr->surface.ysize;
    hwdata.vrect.right  = (hwdata.vrect.left = vr->surface.prebuffer) + vr->surface.xsize;

    vr->surface.ptilewidth = TILE_WIDTH;
    vr->surface.tilewidth  = vr->surface.ptilewidth * vr->surface.bytespp;
    vr->surface.tileheight = TILE_HEIGHT;
    vr->surface.ptilesize  = vr->surface.ptilewidth * vr->surface.tileheight;
    vr->surface.tilesize   = vr->surface.tilewidth * vr->surface.tileheight;

    vr->surface.gamma      = 128;
    vr->surface.alpha      = 128;

    switch(vr->surface.bytespp)
    {
        case 2:      // 16 bit powerhousing!
            vr->surface.tileopaque    = gfx16_TileOpaque;
            vr->surface.tiletrans     = gfx16_TileTrans;
            vr->surface.tilefunky     = gfx16_TileFunky;
            vr->surface.line          = gfx16_line;
            //vr->surface.v_line        = gfx16_vline;
            //vr->surface.h_line        = gfx16_hline;

            vr->surface.rect          = gfx16_rect;
            //vr->surface.alpharect     = gfx16_alpharect;
            vr->surface.shadowrect    = gfx16_shadowrect;

            sprblit_crossfade         = Spr16_Crossfade;
        break;

        case 3:     // 24 bit lameness!
        case 4:     // 32 bit eliteness!  BAM!
            vr->surface.tileopaque    = gfx32_TileOpaque;
            vr->surface.tiletrans     = gfx32_TileTrans;
            vr->surface.tilefunky     = gfx32_TileFunky;
            vr->surface.line          = gfx32_line;
            //vr->surface.v_line        = gfx32_vline;
            //vr->surface.h_line        = gfx32_hline;

            vr->surface.rect          = gfx32_rect;
            //vr->surface.alpharect     = gfx32_alpharect;
            vr->surface.shadowrect    = gfx32_shadowrect;

            sprblit_crossfade         = Spr32_Crossfade;
        break;
    }

    //vr->surface.box           = gfx_box;
    //vr->surface.fillbox       = gfx_fillbox;
    //vr->surface.rect          = gfx_rect;


    vr->surface.physheight = vr->surface.ysize;
    vr->surface.bytesize   = vr->surface.bytewidth * vr->surface.physheight;
    vr->surface.physsize   = vr->surface.physwidth * vr->surface.physheight;

    // Determine the extra bitfield information
    VD_BuildBits(vr->surface.mask.red,&vr->surface.fieldpos.red,&vr->surface.fieldsize.red);
    VD_BuildBits(vr->surface.mask.green,&vr->surface.fieldpos.green,&vr->surface.fieldsize.green);
    VD_BuildBits(vr->surface.mask.blue,&vr->surface.fieldpos.blue,&vr->surface.fieldsize.blue);

#ifdef MM_LOG_VERBOSE
    _mmlog("vdriver > verbose driver information dump...");
    _mmlog(" > xsize, ysize, physwidth: %d  %d  %d", vr->surface.xsize, vr->surface.ysize, vr->surface.physwidth);
    _mmlog(" > rect coords: left = %d, right = %d, top = %d, bottom = %d", hwdata.vrect.left, hwdata.vrect.right, hwdata.vrect.top, hwdata.vrect.bottom);
    _mmlog(" > prebuffer: %d", vr->surface.prebuffer);
#endif
}


// ===========================================================================
    void drvDDraw_SetData(HWND handle)
// ===========================================================================

// A temporary solution to an ugly problem - This allows you to tell this driver what
// window it should attach the surface to.  I could think of no system-independant
// way to do that through the vdriver itself.
//  **ALSO: This function wouldn't work too well in a multi-instance situation.

{
    hwdata.window.handle = handle;
}


#define dxdd  hwdata.dd
#define dxdll hwdata.dll
#define dxpri hwdata.pri
#define dxsec hwdata.sec


// ===========================================================================
static void DX_WindowFlip(VD_RENDERER *vr)
// ===========================================================================
// This procedure currently does a very big and stupid blockblt from the
// secondary (work) surface to the primary (window display) surface.
// NOTE: The code that allows for enable/disable of surface scaling is in
// the window resize event message handler.

{    
    HRESULT res;

    // stupid wait mode for now.  Will toy with async mode later.

    if(!vr->visible)
    {   res = IDirectDrawSurface_Restore(dxpri.surface);
        if(res == DD_OK)
        {   vr->visible = 1;
            IDirectDrawSurface_Restore(dxsec.surface);
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

        res = IDirectDrawSurface_Blt(dxpri.surface, &dr, dxsec.surface, &hwdata.vrect, DDBLT_WAIT, NULL);

        if(res == DDERR_SURFACELOST)
        {   res = IDirectDrawSurface_Restore(dxpri.surface);

            if(res != DD_OK)
            {   vr->visible = 0;
                return;
            }

            IDirectDrawSurface_Restore(dxsec.surface);
        }

        if(res != DD_OK)
        {   SetDDerror(res, "WindowFlip Failure");
			vr->visible = 0;
            return;
        }

        //IDirectDrawSurface_Restore(dxsec.surface);

    }
}
/*
// ===========================================================================
static void DX_WindowFlipRect(VD_RENDERER *vr, VRECT *rect)
// ===========================================================================
// This guy only works if window-resizing makes a different canvas size.
// This guy doesn't work if you want the window resizing to stretch the image.

{
    // stupid wait mode for now.  Will toy with async mode later.
    IDirectDrawSurface_Blt(dxpri.surface, (RECT *)rect, dxsec.surface, (RECT *)rect, DDBLT_WAIT, NULL);
}
*/

// ===========================================================================
    static void DX_DirectFlip(VD_RENDERER *vr)
// ===========================================================================
// Fullscreen single-page flipper.  This uses Blt (instead of flip), and rumor
// has it this will not be impeeded by a VR wait like flip is.

{
    HRESULT res;

    res = IDirectDrawSurface_Blt(dxpri.surface, NULL, dxsec.surface, NULL, DDBLT_WAIT, NULL);
    if(res == DDERR_SURFACELOST)
        res = IDirectDrawSurface_Restore(dxpri.surface);

    if(res != DD_OK)
    {   vr->visible = 0;
        return;
    }
}


// ===========================================================================
    static void DX_DirectFlip2(VD_RENDERER *vr)
// ===========================================================================
// Flips between two video pages.  DX3 is inferior and this function will always
// wait for a vertical retrace before flipping which can cause some major per-
// formance issues on some machines.

{
    HRESULT hr;
    hr = IDirectDrawSurface_Flip(dxpri.surface, NULL, DDFLIP_WAIT);

    if(hr == DDERR_SURFACELOST)
        hr = IDirectDrawSurface_Restore(dxpri.surface);

    if(hr != DD_OK)
    {   vr->visible = 0;
        return;
    }

    IDirectDrawSurface_Lock(dxsec.surface, 0, &dxsec.desc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);

    if(hr == DDERR_SURFACELOST)
        hr = IDirectDrawSurface_Restore(dxsec.surface);

    if(hr != DD_OK)
    {   vr->visible = 0;
        return;
    }

    vr->visible = 1;

    // update our surface with the new page ptr:
    vr->surface.vidmem = &((UBYTE *)dxsec.desc.lpSurface)[vr->surface.prebuffer * vr->surface.bytespp];

    IDirectDrawSurface_Unlock(dxsec.surface,0);
}

// There should probably be a DX_DirectFlip3 here.  Maybe later, after we know
// 1 and 2 and everything else are working satisfactorily.


/*
// ===========================================================================
    static void DIB_NextPage(VD_RENDERER *vr)
// ===========================================================================
{
    HDC           mdc, hdc;
    
    // I need to add scrollbar support to this puppy in the future.
    // then it will be a full grown dog.  or something.
    
    hdc = GetDC(hwdata.window.handle);
    mdc = CreateCompatibleDC(hdc);
    SelectObject(mdc, hwdata.screen_bmp);
    BitBlt(hdc, 0, 0, vr->surface.xsize, vr->surface.ysize, mdc, hwdata.vrect.left*2, hwdata.vrect.top, SRCCOPY);
    DeleteDC(mdc);
    ReleaseDC(hwdata.window.handle, hdc);

}*/

/*static LRESULT APIENTRY WndProc(HWND hWnd, UINT message,WPARAM wParam, LPARAM lParam)

// This is mostly just a basic example of how to handle some of the messages
// in a generic Vdriver/directx-friendly way.

{
    switch(message)
    {   // Check for application activation/deactivation and enable/disable
        // the Nextpage function on that.  These are not really necessary,
        // but they are probably good coding practice.
        
        case WM_ACTIVATE:
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
}*/


// ===========================================================================
    static BOOL DX_Detect(void)
// ===========================================================================
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

                // Try to create a DirectDraw Surface....
                memset(&desc, 0, sizeof(desc));
                desc.dwSize = sizeof(desc);
                desc.dwFlags = DDSD_CAPS;
                desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
                if (IDirectDraw_CreateSurface(sucks, &desc, &DDrawSurf, NULL) == DD_OK)
                {   ddraw_ok = 1;
                    IDirectDrawSurface_Release(DDrawSurf);
                }
            }
            IDirectDraw_Release(sucks);
        }
        FreeLibrary(dll);
    }
    return(ddraw_ok);
}


// ===========================================================================
    static BOOL DX_SetMode(VD_RENDERER *vr, int *xres, int *yres, int *bytespp, uint flags)
// ===========================================================================
{
    HRESULT             result;

    // if the user doesn't specify a window, use the foreground one.
    if(!hwdata.window.handle) hwdata.window.handle = GetForegroundWindow();

    vr->flags = flags;
    
    memset(&dxpri.desc, 0, sizeof(dxpri.desc));
    dxpri.desc.dwSize         = sizeof(dxpri.desc);
    dxpri.desc.dwFlags        = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    dxpri.desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
    dxpri.desc.dwBackBufferCount = 1;

    if(!(vr->flags & VMODE_WINDOWED))
    {   int   i;

        // Fullscreen means we need to set the video mode...

        IDirectDraw2_SetCooperativeLevel(dxdd,hwdata.window.handle, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT);

        vr->resizing = 1;    // make sure our windproc ignores the resize.. sigh.
        SetWindowPos(hwdata.window.handle, NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_NOCOPYBITS | SWP_NOZORDER);
        vr->resizing = 0;

        ShowWindow(hwdata.window.handle, SW_SHOW);
        while(GetForegroundWindow() != hwdata.window.handle)
        {   SetForegroundWindow(hwdata.window.handle);
            Sleep(100);
        }

        result     = DDERR_UNSUPPORTEDMODE;
        i          = 0;
        vr->flags |= VMODE_WINDOWED;   // it gets set back if we find a legit fullscreen mode

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
                    vr->flags &= ~VMODE_WINDOWED;
                break;
            }
        }
    }

    if(vr->flags & VMODE_WINDOWED)
    {   result = IDirectDraw2_SetCooperativeLevel(dxdd,NULL,DDSCL_NORMAL);
        if(result != DD_OK)
        {   SetDDerror(result, "SetCooperative Failure");
            return 0;
        }

        vr->flags &= ~VMODE_DOUBLEBUFFER;

        // Note: We leave it up to the end user to make sure the window is set
        // to the desired size.

        ShowWindow(hwdata.window.handle, SW_SHOW);
		HWND foreground = GetForegroundWindow();
        while(foreground != hwdata.window.handle)
        {   SetForegroundWindow(hwdata.window.handle);
            Sleep(100);
			foreground = GetForegroundWindow();
        }
    }

    if(!(vr->flags & VMODE_DOUBLEBUFFER))
    {   dxpri.desc.dwFlags          &= ~DDSD_BACKBUFFERCOUNT;
        dxpri.desc.ddsCaps.dwCaps   &= ~(DDSCAPS_COMPLEX | DDSCAPS_FLIP);
        dxpri.desc.dwBackBufferCount = 0;
    }

    if((result = IDirectDraw2_CreateSurface(dxdd, &dxpri.desc, &dxpri.surface, 0)) != DD_OK)
    {   // you know this error MIGHT actually matter!
        SetDDerror(result, "CreateSurface Failure");
        return 0;
    }

    // Get our primary surface characteristics.

    memset(&dxpri.desc, 0, sizeof(dxpri.desc));
    dxpri.desc.dwSize  = sizeof(dxpri.desc);
    dxpri.desc.dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    result = IDirectDrawSurface_GetSurfaceDesc(dxpri.surface, &dxpri.desc);
    if(result != DD_OK)
    {   SetDDerror(result, "GetPrimarySurfaceDesc");
        return 0;
    }

    // We don't support 8 bit palette mode, but we don't care.
    // let them play with garbage!

    // Find out the bit-format of the surface so we can match it internally

    if(vr->flags & VMODE_WINDOWED)
    {   vr->surface.xsize  = xres[0];
        vr->surface.ysize  = yres[0];
    } else
    {   vr->surface.xsize  = dxpri.desc.dwWidth;
        vr->surface.ysize  = dxpri.desc.dwHeight;
    }

    vr->surface.bytespp    = ((dxpri.desc.ddpfPixelFormat.dwRGBBitCount-1) / 8) + 1;
    vr->surface.mask.red   = dxpri.desc.ddpfPixelFormat.dwRBitMask;
    vr->surface.mask.green = dxpri.desc.ddpfPixelFormat.dwGBitMask;
    vr->surface.mask.blue  = dxpri.desc.ddpfPixelFormat.dwBBitMask;

    vr->NextRect = NULL;

    // ============================
    //  Secondary Buffer Creation!
    // ============================

    if(vr->flags & VMODE_DOUBLEBUFFER)
    {   // ----------------------------------------
        //  Double Buffer (system or video memory)
        // ----------------------------------------

        vr->NextPage = DX_DirectFlip2;

        memset(&dxsec.desc, 0, sizeof (dxsec.desc));
        dxsec.desc.dwSize = sizeof (dxsec.desc);
        dxsec.desc.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER | ((vr->flags & VMODE_SYSMEM) ? DDSCAPS_SYSTEMMEMORY : 0);
        result = IDirectDrawSurface_GetAttachedSurface(dxpri.surface,&dxsec.desc.ddsCaps, &dxsec.surface);
        if(result != DD_OK)
        {   SetDDerror(result, "GetAttachedSurface");
            return 0;
        }

        vr->surface.physwidth  = dxpri.desc.dwWidth / vr->surface.bytespp;
        vr->surface.physheight = dxpri.desc.dwHeight;
        vr->surface.bytewidth  = dxpri.desc.lPitch;

        IDirectDrawSurface_Lock(dxsec.surface, 0, &dxsec.desc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);
        vr->surface.vidmem = &((UBYTE *)dxsec.desc.lpSurface)[vr->surface.prebuffer * vr->surface.bytespp];
        IDirectDrawSurface_Unlock(dxsec.surface,0);

    } else
    {   uint tmp;

        // -------------------------------
        //  Single Buffer (system memory)
        // -------------------------------

        if(vr->flags & VMODE_WINDOWED)
        {   vr->NextPage = DX_WindowFlip;
        } else
        {   vr->NextPage = DX_DirectFlip;
        }

        tmp = vr->surface.bytespp;
        if(vr->surface.bytespp==3)
        {   vr->surface.bytespp = 4;
            //_mmerr_set(MMERR_INITIALIZING_DEVICE,
                       //"Warning!  This program may not display in a window.  If it does not, try changing "
                       //"your desktop bitdepth from 24 bit to either 16 or 32 bit.  We appologize for this "
                       //"inconvienence.");
            _mmlog("vdriver > drv_ddraw > User has 24 bit desktop!  Argh!");
        }

        memset(&dxsec.desc, 0, sizeof(dxsec.desc));
        dxsec.desc.dwSize    = sizeof(dxsec.desc);
        dxsec.desc.dwFlags   = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_CAPS;
        dxsec.desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

        DX3_SetMeUp(vr);

        // Note:  We base dwWidth on physwidth because directdraw might check
        // for validity of coordinates and get mad if we try to display stuff
        // that it considers 'whitespace' at the end of a scanline.

        dxsec.desc.dwWidth   = vr->surface.physwidth;
        dxsec.desc.dwHeight  = vr->surface.ysize;
        dxsec.desc.lPitch    = vr->surface.bytewidth;

        dxsec.desc.ddpfPixelFormat.dwSize        = sizeof(dxsec.desc.ddpfPixelFormat);
        dxsec.desc.ddpfPixelFormat.dwFlags       = DDPF_RGB;
        dxsec.desc.ddpfPixelFormat.dwRGBBitCount = (tmp == 3) ? 32 : dxpri.desc.ddpfPixelFormat.dwRGBBitCount;
        dxsec.desc.ddpfPixelFormat.dwRBitMask    = dxpri.desc.ddpfPixelFormat.dwRBitMask;
        dxsec.desc.ddpfPixelFormat.dwGBitMask    = dxpri.desc.ddpfPixelFormat.dwGBitMask;
        dxsec.desc.ddpfPixelFormat.dwBBitMask    = dxpri.desc.ddpfPixelFormat.dwBBitMask;
 
        result = IDirectDraw2_CreateSurface(dxdd, &dxsec.desc, &dxsec.surface, NULL);
        if(result != DD_OK)
        {   SetDDerror(result, "CreateSurface (Secondary)");
            return 0;
        }
    }

    IDirectDrawSurface_Lock(dxsec.surface, 0, &dxsec.desc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, 0);
    vr->surface.vidmem = &((UBYTE *)dxsec.desc.lpSurface)[vr->surface.prebuffer * vr->surface.bytespp];
    IDirectDrawSurface_Unlock(dxsec.surface,0);

    if(vr->flags & VMODE_WINDOWED)
    {   IDirectDraw2_CreateClipper(hwdata.dd, 0, &hwdata.clipper, NULL);
        IDirectDrawClipper_SetHWnd(hwdata.clipper, 0, hwdata.window.handle);
        IDirectDrawSurface_SetClipper(hwdata.pri.surface,hwdata.clipper);
    }

/*
    // ==================================
    //  Windowed Mode!  Notes on this:
    //  This is ordered in the way it is because the fullscreen mode set util
    //  could potentially clear the fullscreen bit and force us to windowed
    //  mode (if none of the requested video modes are supported).

    if(flags & VMODE_WINDOWED)
    {   HDC   hdc;
    
        // ----------------------------------------
        // This is old code: DirectDraw-style windoed mode.
        //result = IDirectDraw2_SetCooperativeLevel(dxdd,hwdata.window.handle,DDSCL_NORMAL);
        ///if(result != DD_OK)
        //{   SetDDerror(result, "SetCooperative Failure");
        //    return 0;
        //}
        // ----------------------------------------

        vr->flags &= ~VMODE_DOUBLEBUFFER;

        // Note: We leave it up to the end user to make sure the window is set
        // to the desired size.

        ShowWindow(hwdata.window.handle, SW_SHOW);
        while(GetForegroundWindow() != hwdata.window.handle)
        {   SetForegroundWindow(hwdata.window.handle);
            Sleep(100);
        }

        hdc = GetDC(hwdata.window.handle);
        vr->surface.bytespp   = ((GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL)) + 7) / 8;
        vr->surface.xsize     = xres[0];
        vr->surface.ysize     = yres[0];

        vr->NextPage = DIB_NextPage;

        switch(vr->surface.bytespp)
        {   case 2:
                vr->surface.mask.red   = 0x00007c00;
                vr->surface.mask.green = 0x000003e0;
                vr->surface.mask.blue  = 0x0000001f;
            break;

            case 3:
            case 4:
                vr->surface.mask.red   = 0x00ff0000;
                vr->surface.mask.green = 0x0000ff00;
                vr->surface.mask.blue  = 0x000000ff;
            break;
        }


        DX3_SetMeUp(vr);

    	memset(&hwdata.binfo,0,sizeof(hwdata.binfo));
        hwdata.binfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    	hwdata.binfo.bmiHeader.biWidth         = vr->surface.physwidth;
    	hwdata.binfo.bmiHeader.biHeight        = -(int)vr->surface.physheight;
    	hwdata.binfo.bmiHeader.biPlanes        = 1;
    	hwdata.binfo.bmiHeader.biBitCount      = GetDeviceCaps(hdc,BITSPIXEL);
    	hwdata.binfo.bmiHeader.biCompression   = BI_RGB;
    	hwdata.binfo.bmiHeader.biSizeImage     = vr->surface.physheight * vr->surface.bytewidth;
    	hwdata.binfo.bmiHeader.biXPelsPerMeter = 0;
    	hwdata.binfo.bmiHeader.biYPelsPerMeter = 0;
    	hwdata.binfo.bmiHeader.biClrUsed       = 0;
    	hwdata.binfo.bmiHeader.biClrImportant  = 0;

    	hwdata.screen_bmp = CreateDIBSection(hdc, &hwdata.binfo, DIB_RGB_COLORS, (void **)(&hwdata.vidmem), NULL, 0);

        ReleaseDC(hwdata.window.handle, hdc);

        vr->surface.vidmem = &(hwdata.vidmem)[vr->surface.prebuffer * vr->surface.bytespp];
    }*/

    // Register the sprite blitters usable with this driver.
    // ======================================================
    // Currently this entals registering software blitters only.  Someday this may
    // be enhanced to provide support via hardware directx blitters.
    
    // Notes:
    //  - I have included compiler defines that allow end users to avoid linking in
    //    completely unused blitters.  I wish there was an easier way. If you
    //    think of one, tell me!
    
#ifndef _VD_NO_BLIT_ALPHA_
    if(!(vr->flags & VMODE_DISABLE_ALPHA))
    {   
        #ifdef __MMX__
        Sprite_RegisterBlitterList(&vr->surface, SprAlpha_MMX32);
        Sprite_RegisterBlitterList(&vr->surface, SprAlpha_MMX16);
        #else
        //Sprite_RegisterBlitterList(&vr->surface, SprAlpha_Blit32);
        //Sprite_RegisterBlitterList(&vr->surface, SprAlpha_Blit16);
        #endif
    }
#endif

#ifndef _VD_NO_BLIT_ADDITIVE_
    if(!(vr->flags & VMODE_DISABLE_ADDITIVE))
    {
        #ifdef __MMX__
        Sprite_RegisterBlitterList(&vr->surface, SprAdd_MMX32);
        Sprite_RegisterBlitterList(&vr->surface, SprAdd_MMX16);
        #else
        //Sprite_RegisterBlitterList(&vr->surface, SprAdd_Blit32);
        //Sprite_RegisterBlitterList(&vr->surface, SprAdd_Blit16);
        #endif
    }
#endif

    Sprite_RegisterBlitterList(&vr->surface, SprList_Blit32);
    Sprite_RegisterBlitterList(&vr->surface, SprList_Blit16);

    
    /*if(vr->surface.bytespp == 2)
    {   vr->surface.alphablit = AlphatizeSurface16;
    } else
    {   vr->surface.alphablit = AlphatizeSurface32;
    }*/
    
    vr->surface.setclipper = VD_SetClipper;

    UpdateWindow(hwdata.window.handle);   // not necessary, but 'good coding practice'    
    return 1;
}


// ===========================================================================
    static BOOL DX_Init(VD_RENDERER *vr)
// ===========================================================================
{
    LPDIRECTDRAW ugh;    // temp bullshit for getting the 'real' interface.
    HRESULT      (WINAPI *usuk)(GUID *,LPDIRECTDRAW *,IUnknown *);

    // Create our window, if it doesn't already exist!
    //if(!hwdata.window.handle) DX_CreateWindow(vr);

    // directdraw init ---- CRAP CRAP CRAP
    
    dxdll = LoadLibrary("DDRAW.DLL");
    usuk  = (void *)GetProcAddress(dxdll, "DirectDrawCreate");
    if(!usuk || (usuk(NULL,&ugh,NULL) != DD_OK)) return 0;

    IDirectDraw_QueryInterface(ugh, &IID_IDirectDraw2, (LPVOID *)&dxdd);
    IDirectDraw_Release(ugh);

    return 1;
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


static void DX_ExitMode(VD_RENDERER *vr)
{
    /*if(hwdata.screen_bmp)
    {   DeleteObject(hwdata.screen_bmp);
		hwdata.screen_bmp = NULL;
	}*/

    if(dxdll)
    {   if(dxdd) IDirectDraw2_SetCooperativeLevel(dxdd,hwdata.window.handle,DDSCL_NORMAL);
        if(dxsec.surface) IDirectDrawSurface_Release(dxsec.surface);
        if(dxpri.surface) IDirectDrawSurface_Release(dxpri.surface);    

        dxsec.surface = NULL;
        dxpri.surface = NULL;
    }
}

// ===========================================================================
    static void DX_Exit(VD_RENDERER *vr)
// ===========================================================================
{
    if(dxdll)
    {   if(dxdd)
        {   IDirectDraw_Release(dxdd);
            dxdd = NULL;
        }
        FreeLibrary(dxdll);
        dxdll = NULL;
    }

    /*if(!(vr->flags & VMODE_DOUBLEBUFFER))
    {   if(vr->surface.vidmem) free(vr->surface.vidmem);
    }*/
}


VDRIVER drv_ddraw =
{  NULL,
   "DivEnt DirectDraw 3",
   "0.400",

   // driver and mode-dependant procedures and functions

   DX_Detect,
   DX_Init,
   DX_Exit,
   DX_SetMode,
   DX_ExitMode
};
