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
 Module: dib.c

  Windows95/98/NT/2000  Device Independant Bitmap Driver.

  A basic DIB driver.  This allows use ot create many many surfaces, generally
  for use with editors and utilities.  This sucker also supports windows that
  have scrollbars.

  Notes:
   - Fullscreen DIB is currently unsupported - and may never be supported!
   - DIB scaling is currently unsupported, but will be supported eventually.
*/

#include <windows.h>
#include "vdriver.h"


typedef struct DIB_HWDATA
{   HINSTANCE instance;

    // Hmm, A totally unnamed structure just to encapsulate some window variables.
    // For some reason this relatively pointless structure made my coding job a 
    // lot more fun though..

    struct
    {   HWND handle;
        int  xloc, yloc;
    } window;

    // rectangle of the actual *viewable* area.  This is needed so that we blit
    // without copying the dirty buffers (left and right side of each scanline)
    
    RECT             vrect;
    void            *vidmem;
    HBITMAP          screen_bmp;
    BITMAPINFO       binfo;
} DIB_HWDATA;



static BOOL DIB_Detect(void)
{
    // As long as we can compile, we are available.
    return 1;
}


static void DIB_NextPage(VD_RENDERER *vr)
{
    DIB_HWDATA   *hwdata = (DIB_HWDATA *)vr->hwdata;
    HDC        mdc, hdc;
    
    // I need to add scrollbar support to this puppy in the future.
    // then it will be a full grown dog.  or something.
    
    hdc = GetDC(hwdata->window.handle);
    mdc = CreateCompatibleDC(hdc);
    SelectObject(mdc, hwdata->screen_bmp);
    BitBlt(hdc, 0, 0, vr->surface.xsize, vr->surface.ysize, mdc, 0, 0, SRCCOPY);
    DeleteDC(mdc);
    ReleaseDC(hwdata->window.handle, hdc);

}

static BOOL DIB_SetMode(VD_RENDERER *vr, int *xres, int *yres, int *bytespp, uint flags)

// Set the current window's settings as according the users' desires.
// Note the out DIB driver supports scrollbars in addition to scaling and
// fixed windows options.

{
    DIB_HWDATA    *hwdata = (DIB_HWDATA *)vr->hwdata;
    HDC            hdc;

    // if the user doesn't specify a window, use the foreground one.
    if(!hwdata->window.handle) hwdata->window.handle = GetForegroundWindow();

    // Note: We leave it up to the end user to make sure the window is set
    // to the desired size.

	// Determine the screen depth

	hdc = GetDC(hwdata->window.handle);
    vr->surface.bytespp   = ((GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL)) + 7) / 8;
    vr->surface.xsize     = xres[0];
    vr->surface.ysize     = yres[0];

    vr->surface.bytewidth  = ((vr->surface.xsize * vr->surface.bytespp) + 3) & ~3;
    vr->surface.alignment  = ((flags & VMODE_ALIGN) ? 8 : 4) / vr->surface.bytespp;
    vr->surface.physwidth  = vr->surface.bytewidth / vr->surface.bytespp;
    vr->surface.physheight = vr->surface.ysize;

    vr->surface.ptilewidth = TILE_WIDTH;
    vr->surface.tilewidth  = vr->surface.ptilewidth * vr->surface.bytespp;
    vr->surface.tileheight = TILE_HEIGHT;
    vr->surface.ptilesize  = vr->surface.ptilewidth * vr->surface.tileheight;
    vr->surface.tilesize   = vr->surface.tilewidth * vr->surface.tileheight;

    vr->surface.gamma      = 128;
    vr->surface.alpha      = 128;

    
    switch(vr->surface.bytespp)
    {   case 2:
            // GDI is always 5-5-5
            vr->surface.mask.red   = 0x00007c00;
            vr->surface.mask.green = 0x000003e0;
            vr->surface.mask.blue  = 0x0000001f;

            vr->surface.tileopaque = gfx16_TileOpaque;
            vr->surface.tiletrans  = gfx16_TileTrans;
            vr->surface.tilefunky  = gfx16_TileFunky;
            vr->surface.line       = gfx16_line;
            //vr->surface.v_line     = gfx16_vline;
            //vr->surface.h_line     = gfx16_hline;
        break;

        case 3:
        case 4:
            // GDI is always 8-8-8
            vr->surface.mask.red   = 0x00ff0000;
            vr->surface.mask.green = 0x0000ff00;
            vr->surface.mask.blue  = 0x000000ff;

            vr->surface.tileopaque = gfx32_TileOpaque;
            vr->surface.tiletrans  = gfx32_TileTrans;
            vr->surface.tilefunky  = gfx32_TileFunky;
            vr->surface.line       = gfx32_line;
            //vr->surface.v_line     = gfx32_vline;
            //vr->surface.h_line     = gfx32_hline;
        break;
	}

    vr->surface.bytesize   = vr->surface.bytewidth * vr->surface.physheight;
    vr->surface.physsize   = vr->surface.physwidth * vr->surface.physheight;

    // Determine the extra bitfield information
    VD_BuildBits(vr->surface.mask.red,&vr->surface.fieldpos.red,&vr->surface.fieldsize.red);
    VD_BuildBits(vr->surface.mask.green,&vr->surface.fieldpos.green,&vr->surface.fieldsize.green);
    VD_BuildBits(vr->surface.mask.blue,&vr->surface.fieldpos.blue,&vr->surface.fieldsize.blue);

	hwdata->binfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	hwdata->binfo.bmiHeader.biWidth         = vr->surface.xsize;
	hwdata->binfo.bmiHeader.biHeight        = -(int)vr->surface.ysize;
	hwdata->binfo.bmiHeader.biPlanes        = 1;
	hwdata->binfo.bmiHeader.biBitCount      = GetDeviceCaps(hdc,BITSPIXEL);
	hwdata->binfo.bmiHeader.biCompression   = BI_RGB;
	hwdata->binfo.bmiHeader.biSizeImage     = vr->surface.ysize * vr->surface.bytewidth;
	hwdata->binfo.bmiHeader.biXPelsPerMeter = 0;
	hwdata->binfo.bmiHeader.biYPelsPerMeter = 0;
	hwdata->binfo.bmiHeader.biClrUsed       = 0;
	hwdata->binfo.bmiHeader.biClrImportant  = 0;

	hwdata->screen_bmp = CreateDIBSection(hdc, &hwdata->binfo, DIB_RGB_COLORS,
				(void **)(&hwdata->vidmem), NULL, 0);

    ReleaseDC(hwdata->window.handle, hdc);

    // Note: We leave it up to the end user to make sure the window is set
    // to the desired size.

    // =======================================================================
    // Find out the bit-format of the surface so we can match it internally

    vr->NextPage = DIB_NextPage;
    vr->NextRect = NULL;
    vr->surface.vidmem = hwdata->vidmem;


    #ifndef _VD_NO_BLIT_ALPHA_
    if(!(flags & VMODE_DISABLE_ALPHA))
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
    if(!(flags & VMODE_DISABLE_ADDITIVE))
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

    //Sprite_RegisterBlitter(&vr->surface, &Spr_Blit32Opaque);
    //Sprite_RegisterBlitter(&vr->surface, &Spr_Blit16Opaque);

    Sprite_RegisterBlitterList(&vr->surface, SprList_Blit32);
    Sprite_RegisterBlitterList(&vr->surface, SprList_Blit16);

    vr->surface.setclipper = VD_SetClipper;

    return 1;
}


static BOOL DIB_Init(VD_RENDERER *vr)
{
    vr->hwdata = _mm_calloc(1,sizeof(DIB_HWDATA));

    return 1;
}


static BOOL ResizeViewport(VD_RENDERER *vr, WPARAM wParam, LPARAM lParam)

// Should be called whenever the viewport has been resized.  In Windows
// terms, this means whenever the following messages are recieved:
//
//   WM_CREATE
//   WM_SIZE

{
    DIB_HWDATA      *hwdata = (DIB_HWDATA *)vr->hwdata;

    if(vr->flags & VMODE_SCROLLBARS)
    {   uint   style = GetWindowLong(hwdata->window.handle, GWL_STYLE);
        RECT   wrect;
    
        // If the window area is smaller than our surface area, then turn
        // on the scrollbars as needed!

        GetClientRect(hwdata->window.handle,&wrect);
        
        if((uint)(wrect.right - wrect.left)  < vr->surface.xsize)  style |= WS_HSCROLL;
        if((uint)(wrect.bottom - wrect.top)  < vr->surface.ysize)  style |= WS_VSCROLL;
        SetWindowLong(hwdata->window.handle, GWL_STYLE, style);
    }
}


static BOOL ScrollVertical(VD_RENDERER *vr, WPARAM wParam, LPARAM lParam)
{
    DIB_HWDATA    *hwdata = (DIB_HWDATA *)vr->hwdata;
    SCROLLINFO  si;

    si.cbSize  = sizeof(si);
    si.fMask   = SIF_ALL;
    GetScrollInfo(hwdata->window.handle, SB_VERT, &si);
    
    switch(LOWORD(wParam))
    {   case SB_TOP:
            si.nPos = si.nMin;
        break;
    
        case SB_BOTTOM:
            si.nPos = si.nMax;
        break;
    
        case SB_LINEUP:
            si.nPos -= 1;
        break;

        case SB_LINEDOWN:
            si.nPos += 1;
        break;

        case SB_PAGEUP:
            si.nPos -= si.nPage;
        break;

        case SB_PAGEDOWN:
            si.nPos += si.nPage;
        break;

        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
        break;
    }

    // Set the position then retrieve it, because windows has a mind of
    // its own - a very twisted one at that - and could do almost anything
    // to our value.

    SetScrollInfo(hwdata->window.handle, SB_VERT, &si, TRUE);
    GetScrollInfo(hwdata->window.handle, SB_VERT, &si);
    
    // Future room for acceleration:
    //  a) Only redraw if we actually moved around.
    //  b) use ScrollWindow function instead of refreshing completely, then
    //     use NextRect to redraw the uncovered portions only.

    // For now: Tell vdriver to redraw the entire viewport.

}


void DIB_Exit(VD_RENDERER *vr)
{

}

VDRIVER drv_dib =
{  NULL,
   "DivEnt Windows DIB",
   "0.231",

   // driver and mode-dependant procedures and functions

   DIB_Detect,
   DIB_Init,
   DIB_Exit,
   DIB_SetMode
};

