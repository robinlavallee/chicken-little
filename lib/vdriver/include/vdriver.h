/*  
   VDRIVER.H -- The DivEnt Game Graphics Engine <---

   By Jake Stine and Divine Entertainment

          - (  Graphics the Divine Way  ) -

*/

#ifndef _VDRIVER_H_
#define _VDRIVER_H_

// throw in those header files that are just necessary

#include "mmio.h"


// ===============================================
//  Tile-Related Compiler Definitions / Constants
// ===============================================

#ifndef TILE_WIDTH
#define TILE_WIDTH   32
#endif

#ifndef TILE_HEIGHT
#define TILE_HEIGHT  32
#endif

typedef UBYTE TILE;


// =========================
//  Function macro defines:
//
//  ScrnLookUp, MK_ScrnPtr - Returns a pointer to the given location on the virtual
//                           screen buffer.
//  Box, FillBox           - Uppercase versions of the lowercase functions (same)

#define MK_ScrnPtr(x,y)    (&vs->clipmem[((y)*vs->bytewidth) + (x)])

#define Box(x,y,a,b,c)     box(x,y,a,b,c)   

#define FillBox(x,y,a,b,c) fillbox(x,y,a,b,c)

#define VD_SetMouseColor(x,y,z) VD_MakeColor(&vd_cursorcolor, x, y, z)

#define VD_MakeColor(s,r,g,b) (s->masktable_red[r] | s->masktable_green[g] | s->masktable_blue[b])

#define VD_GetSurface(v) (&vr->surface)



// The vdriver custom rectangle structure.
// ---------------------------------------

typedef struct Gates_Is_a_DorK
{   SLONG    left;
    SLONG    top;
    SLONG    right;
    SLONG    bottom;
} VRECT;


// ==========================
//  video mode flags/options
// ==========================

#define VMODE_WINDOWED        1
#define VMODE_SYSMEM          2
#define VMODE_DOUBLEBUFFER    4
#define VMODE_ALIGN           8
#define VMODE_MMX            16
#define VMODE_SCROLLBARS     32

#define VMODE_DISABLE_ALPHA    1024
#define VMODE_DISABLE_GAMMA    2048
#define VMODE_DISABLE_ADDITIVE 4096

#define VDRIVER_AUTODETECT   -1


// ==================================
//  Alpha renderer flags and options
// ==================================

enum
{   
    // These enumaerations are for use with AlphatizeSurface and the 
    // FastAlpha_Blit functions.

    VD_ALPHA_0 = 0,
    VD_ALPHA_25,
    VD_ALPHA_50,
    VD_ALPHA_75,
    VD_ALPHA_100
};


enum
{   
    // Event Enumerations.
    // Aren't too many of them yet. :)

    VDEVT_SURFACE_CHANGE = 0,
    VDEVT_MAX
};



typedef struct VD_ATTRIB
{   uint  red, green, blue;
} VD_ATTRIB;

typedef struct VD_SHIFTFORMAT
{   uint      shiftmask;             // add this to the RGB int to make shifted value.

    VD_ATTRIB fieldpos,              // start bit position of the attribute mask.
              fieldsize,             // size of the mask (factor of the number of shades avail!)
              mask;                  // da Mask!
} VD_SHIFTFORMAT;


// =====================================================================================
    typedef struct VD_SURFACE
// =====================================================================================
{
    uint    xsize, ysize;            // width and height of the visible surface area, in pixels.
    
    uint    physwidth,physheight,    // physical width and height of entire surface (including buffers)
            physsize;                // total size of screen buffer in pixels / bytes.

    uint    realheight;

    uint    bytewidth,               // width of screen buffer in bytes (bytes per line)
            bytesize;                // total size of the bufer

    uint    tilewidth, tileheight,   // width and height of std. tiles [width in bytes]
            ptilewidth, ptilesize,   // width and total size in PIXELS!
            tilesize;                // total size [in bytes] of a tile

    uint    bytespp;                 // bytes per pixel of video memory  

    VD_ATTRIB fieldpos,              // start bit position of the attribute mask.
              fieldsize,             // size of the mask (factor of the number of shades avail!)
              mask;                  // da Mask!

    ULONG   masktable_red[256],
            masktable_green[256],
            masktable_blue[256];

    ULONG   alphamask[8];            // alpha masks used by the alpha blitters for optimizations

    VD_SHIFTFORMAT shiftfmt1,        // single-bit extended valuerange bit format.
                   shiftfmt2,        // double-bit extended valuerange bit format.
                   shiftfmt3;        // triple-bit extended valuerange bit format.

    UBYTE  *vidmem;                 // pointer to surface.

    int     gamma;                   // Color brightness factor [fade in/out]
    int     alpha;                   // alpha blending [fancy fade in/out]


    // Surface clipping
    // ----------------
    
    UBYTE  *clipmem;                 // pointer to upper-left corner of the clipper area.
    VRECT   clipper;                 // current clipper region
    uint    xreal, yreal;
    void    (*setclipper)(struct VD_SURFACE *surface, const VRECT *rect);

    // Surface-related Methods
    // -----------------------



    // Sprite / geometry-related methods
    // ---------------------------------
    // The following used to format loaded sprites, and to determine the proper
    // blitters to be attached to this surface!

    uint    alignment,               // determines the pixel-based alignment (padding at the end of scans)
            prebuffer;               // fixed-length (in pixels) of pre-scan padding.


    void    (*tileopaque)(const struct VD_SURFACE *surface, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
    void    (*tiletrans) (const struct VD_SURFACE *surface, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
    void    (*tilefunky) (const struct VD_SURFACE *surface, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);

    void    (*line)   (const struct VD_SURFACE *surface, int xstart, int ystart, int xend, int yend, int color);
    void    (*rect)   (const struct VD_SURFACE *surface, VRECT *rect, int color);

    void    (*alphaline)   (const struct VD_SURFACE *surface, int xstart, int ystart, int xend, int yend, int color, uint alpha);
    void    (*alpharect)   (const struct VD_SURFACE *surface, VRECT *rect, int color, uint alpha);

    void    (*shadowrect)  (const struct VD_SURFACE *surface, VRECT *rect, uint darkmode);

    struct SPR_BLITTER   *sprdrv;


    //void    (*v_line) (const struct VD_SURFACE *surface, int xstart, int ystart, int ylen, int color);
    //void    (*h_line) (const struct VD_SURFACE *surface, int xstart, int ystart, int xlen, int color);
    //void    (*v_alphaline) (const struct VD_SURFACE *surface, int xstart, int ystart, int ylen, int color, uint alpha);
    //void    (*h_alphaline) (const struct VD_SURFACE *surface, int xstart, int ystart, int xlen, int color, uint alpha);
} VD_SURFACE;


// =====================================================================================
    typedef struct VDRIVER
// =====================================================================================
// The key to my portability.  This structure is all the entire game engine needs for
// portability to nearly any platform.  All hardware specific routines are pointed to by
// it, and all hardware specific numbers are defined in it.
//
// Notes:
//  - Init(void) should do two things in particular:
//      (1)  Initialize (malloc) the off-screen buffer / buffers.
//      (2)  Set the video mode.
//  - Exit(void) should undo what Init does, freeing buffers and resetting the video mode
//    to a std. native textmode or whatever.
{  
    struct VDRIVER *next;   // Set up a linked list, bud
    CHAR   *name,           // Driver Name
           *version;        // Driver version number

    // driver and mode-dependant procedures and functions

    BOOL   (*Detect)(void);            // does this driver exist on this system?
    int    (*Init)(struct VD_RENDERER *vr);
    void   (*Exit)(struct VD_RENDERER *vr);   // Clear memory mallocs + set "std" video (textmode / whatever)
    int    (*SetMode)(struct VD_RENDERER *vr, int *xres, int *yres, int *bytespp, uint flags);
    void   (*ExitMode)(struct VD_RENDERER *vr);

} VDRIVER;


// =====================================================================================
typedef struct VD_RENDERER
// =====================================================================================
{
    uint        flags;
    BOOL        initialized;        // set to true when a video mode is set.
    VD_SURFACE  surface;            // information for rendering to the display.
    VDRIVER     vdriver;

    BOOL    cursorint;              // interrupt-cursor draw enable / disable
    int     cursorcolor;            // color of the mouse cursor [default white]

    // Stuff you, as the user, need to check for in your window handler
    
    BOOL    visible;                // renderer-visible flag.  If unset, your program should
                                    // not perform screen updates to conserve CPU.
    BOOL    resizing;               // software window resize (not user), so ignore!

    // Prototypes (function ptr)
    // =========================
    
    void    (*NextPage)(struct VD_RENDERER *vr);
    void    (*NextRect)(struct VD_RENDERER *vr, VRECT *rect);
    void    (*ClearScreen)(struct VD_RENDERER *vr);

    // Events
    // ======
    
    struct
    {   void   *data;
        BOOL  (*proc)(struct VD_RENDERER *vr, void *data);
    } event[VDEVT_MAX];

    // Driver Data Block

    void    *hwdata;                 // driver defined data block.

    // Features for 256 color modes ...
    // Except I don't support that anymore.. WOOOHOOO!
    
    //void    (*GetPalette)(UBYTE *dest, int firstcol, int numcols);
    //void    (*SetPalette)(UBYTE *src, int firstcol, int numcols);
    //void    (*FastFadeInit)(int red, int green, int blue);
    //void    (*FastFade)(int step);

    //CHAR   *ApplicationName;

} VD_RENDERER;

#include "sprite.h"

// ==========================================================================
//  Prototypes!
// ==========================================================================

// Defined in VDRIVER.C

extern void         VD_SetClipper(VD_SURFACE *surface, const VRECT *rect);

extern VD_RENDERER *VD_InitRenderer(int driver);
extern void         VD_Exit(VD_RENDERER *vr);
extern void         VD_ExitMode(VD_RENDERER *vr);
extern VD_SURFACE  *VD_SetMode(VD_RENDERER *vr, int *xres, int *yres, int *bytespp, uint flags);
extern void         VD_SetEvent(VD_RENDERER *vr, int event, void *data, BOOL (*eventproc)(VD_RENDERER *vr, void *data));
extern void         VD_ThrowEvent(VD_RENDERER *vr, int event);

extern void         VD_RegisterDriver(VDRIVER *ldr);
extern void         VD_InfoDriver(void);
extern void         VD_SetTileSize(int xsize, int ysize);
extern void         VD_ConvertBitmap(UBYTE *src, UBYTE *dest, int length);

//extern void  VD_SetIntensity(int intensity);
//extern void  VD_AllocateBuffer(BOOL val);


// ===================================
// === Global Procedure Prototypes ===
// ===================================

// ======================
//  Defined in GAMEGFX.C
// ======================

extern UBYTE  *cut_tile(UBYTE *source, int srcx, int tilex, int tiley, int x, int y);

// ==================
//  Defined in GFX.C
// ==================

//extern void  rect_store(const VD_SURFACE *vs, int xstart, int ystart, int xend, int yend, UBYTE *buffer);
//extern void  rect_restore(const VD_SURFACE *vs, int xloc, int yloc, const UBYTE *buffer);
//extern void  gfx_box(const VD_SURFACE *vs, int xstart, int ystart, int xwidth, int ywidth, int color);
//extern void  gfx_fillbox(const VD_SURFACE *vs, int xstart, int ystart, int xwidth, int ywidth, int color);
//extern void  gfx_rect(const VD_SURFACE *vs, int xstart, int ystart, int xend, int yend, int color);

// =================================================
//  Various Primatives (see files in prims/ folder)
// =================================================
// These should never be used directly, but rather referenced through the 
// VDRIVER driver system.  The drivers will know which of these primatives 
// it will work best with (if any).

//extern void gfx256_line(VD_SURFACE *vs, int xstart, int ystart, int xend, int yend, VD_COLOR *color);
//extern void gfx256_vline(VD_SURFACE *vs, int xstart, int ystart, int ylen, VD_COLOR *color);
//extern void gfx256_hline(VD_SURFACE *vs, int xstart, int ystart, int xlen, VD_COLOR *color);

extern void gfx16_TileOpaque(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
extern void gfx16_TileTrans(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
extern void gfx16_TileFunky(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
extern void gfx16_line(const VD_SURFACE *vs, int xstart, int ystart, int xend, int yend, int color);
extern void gfx16_vline(const VD_SURFACE *vs, int xstart, int ystart, int ylen, int color);
extern void gfx16_hline(const VD_SURFACE *vs, int xstart, int ystart, int xlen, int color);
extern void gfx16_rect(const VD_SURFACE *vs, const VRECT *rect, int color);
extern void gfx16_alpharect(const VD_SURFACE *vs, const VRECT *rect, int color, uint alpha);
extern void gfx16_shadowrect(const VD_SURFACE *vs, const VRECT *rect, uint mode);

extern void gfx32_TileOpaque(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
extern void gfx32_TileTrans(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
extern void gfx32_TileFunky(const VD_SURFACE *vs, const UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
extern void gfx32_line(const VD_SURFACE *vs, int xstart, int ystart, int xend, int yend, int color);
extern void gfx32_vline(const VD_SURFACE *vs, int xstart, int ystart, int ylen, int color);
extern void gfx32_hline(const VD_SURFACE *vs, int xstart, int ystart, int xlen, int color);
extern void gfx32_rect(const VD_SURFACE *vs, const VRECT *rect, int color);
extern void gfx32_alpharect(const VD_SURFACE *vs, const VRECT *rect, int color, uint alpha);
extern void gfx32_shadowrect(const VD_SURFACE *vs, const VRECT *rect, uint mode);

// ==================
//  Standard Drivers
// ==================
// These are all the drivers that come available with the standard distributions
// of the VDRIVER.  These have been included here for convienence.

extern VDRIVER drv_v320x200;    // Std. VGA 320x200 driver (chained, no double-buffer)
extern VDRIVER drv_x320x240;    // ModeX VGA 320x240 driver (planar, double-buffered)
extern VDRIVER drv_vesa256;     // VESA VBE driver [256 colors modes only]
extern VDRIVER drv_vesahi;      // VESA VBE driver [hi and true color modes only]

extern VDRIVER drv_ddraw;
extern VDRIVER drv_dib;


// =====================================================================================
// Driver-Use-Only stuff: This is stuff that generally only the drivers should reference
// directly, and the user references through the use of the surface and sprite methods

extern void VD_BuildBits(uint mask, uint *pos, uint *len);

extern void AlphatizeSurface32(const VD_SURFACE *vs, uint type);
extern void AlphatizeSurface16(const VD_SURFACE *vs, uint type);

// ==================================================
//  CRAP ERRATA - Shit I don't want to delete... yet
// ==================================================

//extern BOOL vd_sameformat;   // set if the video format is RGB 24 bit

//extern void  (*vd_spriteblitter)(int xloc, int yloc, SPRITE *sprite);
//extern void  (*vd_tileopaque)(UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
//extern void  (*vd_tiletrans)(UBYTE *tile_bitmap, UBYTE *tile_scrnptr);
//extern void  (*vd_tilefunky)(UBYTE *tile_bitmap, UBYTE *tile_scrnptr);

/*extern void (*vd_MouseRestoreBG)(void);
extern void (*vd_MouseDrawCursor)(int xmse, int ymse, UBYTE *mcur);

#pragma aux vd_MouseRestoreBG  modify nomemory;
#pragma aux vd_MouseDrawCursor modify nomemory;*/


#endif

