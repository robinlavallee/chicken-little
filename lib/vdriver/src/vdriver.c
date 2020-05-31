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
 Module: vdriver.c
 
  Video driver - portable interface to hardware.
                      
  This module, (and other modules in this directory, for that matter) for 
  most practical purposes, is classifiled as "portable".  These functions
  are the interface for the non-portable low-level drivers, which are reg-
  istered at the start of the program.

  Make sure to see VDRIVER.H for further information about the strutures
  used in the VDRIVER (ie, SPRITE, TILE, etc).  Also, VDRIVER.DOC has a gen-
  eral overview of the usage of the library.

 Important Notes:

 -/- About Sprite / Tile blit and clipping:

  - The base unit of display for VDRIVER is the tile.  A tile is very simp-
    le and built for speed.  A tile is simply raw data of a pre-defined
    size (usually 32x32 pixels) with no header.

  - There is only a single generalized sprite blitter function.  This func-
    tion detects the blitter type for the sprite via its header.  It also
    handles all clipping concerns.

  - vd_tilewidth is in bytes and not pixels. (ie, the x size in pels * bytespp)
*/

#include "vdriver.h"
#include <string.h>

void Sprite_FreeBlitters(VD_SURFACE *vs);

// ==========================================================================
//  World of Static!

// > Local VDRIVER variable declarations

static VDRIVER *vdrvbegin = (VDRIVER *)NULL; 
static uint     vd_cpu    = CPU_NONE;


// ------------------------
//  Mouse Rendering Shizat
//   (placebo functions)
// ------------------------

static void (*vd_MouseRestoreBG)(void);
static void (*vd_MouseDrawCursor)(int xmse, int ymse, UBYTE *mcur);


static void dummy_MouseRestoreBG(void)
{
}
static void dummy_MouseDrawCursor(int xmse, int ymse, UBYTE *mcur)
{
}

// =====================================================================================
    void VD_BuildBits(uint mask, uint *pos, uint *len)
// =====================================================================================
// This is a workhorse function I made which converts the cleverly ambiguous bitfields 
// that directdraw offers to something far more subtantial.  It takes a bitmask of the 
// color component and extracts from it the position and the length of the field of bits
// that make up the particular color.
//  -- oh yea, VESA provided you with this information automatically.  Vesa
//     was good.  Directdraw is not.  Imagine that. --

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


// =====================================================================================
    static void BuildAlphaMasks(VD_SURFACE *vs)
// =====================================================================================
// Constructs the alpha masks which are used by the alpha blitters.  There are several
// masks which the game uses, so we simply build a table of them, ragning from 1 bit mask
// to 8 bit mask.
{
    uint   t;
    ULONG  mask;

    for(t=0, mask=1; t<7; t++)
    {   vs->alphamask[t] = (vs->mask.red & ((~mask) << vs->fieldpos.red)) | (vs->mask.green & ((~mask) << vs->fieldpos.green)) | (vs->mask.blue & ((~mask) << vs->fieldpos.blue));
        mask = (mask<<1) | 1;    // progress the mask.. shift then set the low bit.
    }

}


// =====================================================================================
    void VD_SetClipper(VD_SURFACE *surface, const VRECT *rect)
// =====================================================================================
{
    if(!surface) return;
    
    if(rect)
    {   int weetop;
    
        weetop = (rect->top - surface->clipper.top);

        if(weetop > (int)surface->yreal) return;           // invisible surfaces are bad
        if(weetop < 0) weetop = 0;

        surface->clipmem = &surface->vidmem[(weetop * surface->bytewidth) + ((rect->left - surface->clipper.left) * surface->bytespp)];

        surface->clipper = *rect;

        surface->clipper.bottom = _mm_boundscheck(surface->clipper.bottom, 0, (int)(surface->yreal-1));
        surface->clipper.top    = _mm_boundscheck(surface->clipper.top, 0, (int)(surface->yreal-1));

        surface->xsize = ((rect->right <= 0) ? 0 : rect->right) - rect->left;
        surface->ysize = surface->clipper.bottom - surface->clipper.top;

        surface->physheight = surface->ysize + (surface->realheight - surface->yreal);
    } else
    {   surface->clipper.top    = surface->clipper.left = 0;
        surface->xsize = surface->clipper.right  = surface->xreal;
        surface->ysize = surface->clipper.bottom = surface->yreal;

        surface->physheight = surface->realheight;

        surface->clipmem = surface->vidmem;
    }
}

// =====================================================================================
    VD_SURFACE *VD_CreateClipSurface(const VD_SURFACE *surface, const VRECT *rect)
// =====================================================================================
{
    VD_SURFACE   *cs;
    
    if(!surface || !rect) return NULL;

    cs = (VD_SURFACE *)_mm_malloc(sizeof(VD_SURFACE));

    *cs = *surface;

    cs->vidmem = &surface->vidmem[(rect->top * surface->bytewidth) + (rect->left * surface->bytespp)];

    return cs;
}


// =====================================================================================
    static void InitColorConversion(VD_SURFACE *surface)
// =====================================================================================
{
	int   i;
    
    // Make a lookup table for color conversions.
    
    for(i=0; i<256; i++)
    {   surface->masktable_red[i]   = ((surface->mask.red   * i) / 255) & surface->mask.red;
        surface->masktable_green[i] = ((surface->mask.green * i) / 255) & surface->mask.green;
        surface->masktable_blue[i]  = ((surface->mask.blue  * i) / 255) & surface->mask.blue;
    }
}

static CHAR errmsg_vidfail[] = "Video Initialization Failure!\r\nPlease check the log file for details.";


// =====================================================================================
    VD_RENDERER *VD_InitRenderer(int driver)
// =====================================================================================
// Inializes a driver, and creates a 'rendering device' through which to
// access it. Depending on the driver.  After the rendering device is cre-
// ated, use VD_SetMode to assign the bitdepth, mode of access, and other
// such properties.
//
// Parameters:
//   driver - driver to initialize, or -1 (VD_AUTODETECT) for autodetection.

// Returns:
//   On failure, returns NULL.  On success, returns the VD_RENDERER struct,
//   filled and happy!
{
    VDRIVER     *l = vdrvbegin;
    VD_RENDERER *vr;

    if(driver != VDRIVER_AUTODETECT)
    {   int t = 0;
        while((t < driver) && l) { l = l->next; t++; }
    } else
    {   while(l && l->Detect && !l->Detect()) { l = l->next; }
    }

    if(!l)
    {   _mmlog("Video > Detection Failed!");
        return NULL;
    }

    _mmlog("Video > Initializing %s driver.", l->name);

    vr = _mm_calloc(1, sizeof(VD_RENDERER));

    vr->vdriver = *l;

    if(l->Init && !l->Init(vr))
    {   _mmlog("Video > Initialization Failed!");
        _mmerr_set(MMERR_INITIALIZING_DEVICE,errmsg_vidfail);
        return NULL;
    }

    // Set up some things in the surface that are the same for all drivers always
    // (ie, these are precaculated thingies using the hardware-specific information
    // already available to us)

    // Configure Shift-formats (used for additive blending and alpha channels)

    //vs->shiftfmt1.shiftmask = vs->fieldpos

    return vr;
}


// ===========================================================================
    VD_SURFACE *VD_SetMode(VD_RENDERER *vr, int *xres, int *yres, int *bytespp, uint flags)
// ===========================================================================
// Initializing the DirectDraw driver in fullscreen mode will automatically
// set the video display to the requested size and bitdepth.  Attaching a
// 'windowed' mode renderer to a window will do nothing more than refresh
// that wondow/object when it is visible.
//
// Parameters:
//   xres/yres/bytespp
//      Arrays of appropriate values.  They are searched in order, together,
//      when detecting an appropriate video mode.
//
//   flags   - See 'VMODE_*' flagset in vdriver.h.
{
    if(!vr) return NULL;

    VD_ExitMode(vr);

    if(vr->vdriver.SetMode && !vr->vdriver.SetMode(vr, xres, yres, bytespp, flags))
    {   _mmlog("Video > Could not set a video mode!");
        _mmerr_set(MMERR_INITIALIZING_DEVICE,errmsg_vidfail);
        return NULL;
    }

    _mmlog("Vdriver > Video Modeset Successful!");
    _mmlog("        > xres: %d  yres: %d  bytespp: %d",vr->surface.xsize, vr->surface.ysize, vr->surface.bytespp);

    InitColorConversion(&vr->surface);
    BuildAlphaMasks(&vr->surface);

    vr->surface.xreal = vr->surface.xsize;
    vr->surface.yreal = vr->surface.ysize;
    vr->surface.realheight = vr->surface.physheight;
    vr->surface.setclipper(&vr->surface, NULL);

    // Throw the surface change event
    
    VD_ThrowEvent(vr, VDEVT_SURFACE_CHANGE);
    
    vr->visible     = 1;
    vr->initialized = 1;

    return &vr->surface;
}


// ===========================================================================
    void VD_SetEvent(VD_RENDERER *vr, int event, void *data, BOOL (*eventproc)(VD_RENDERER *vr, void *data))
// ===========================================================================
{
    if(!eventproc) return;   // bah, silly nulls.

    vr->event[event].data = data;
    vr->event[event].proc = eventproc;
}

// ===========================================================================
    void VD_ThrowEvent(VD_RENDERER *vr, int event)
// ===========================================================================
{
    if(vr && vr->event[event].proc) vr->event[event].proc(vr, vr->event[event].data);
}


// ===========================================================================
    BOOL VD_CreateSurface_System(int xres, int yres, int bytespp)
// ===========================================================================
// Create a surface in system ram.  This surface will rely on software render-
// ing only, and hence can take full advantage of all RLE sprites and tiles.
// This surface can have any bitdepth and be rendered to any VD_RENDERER (as
// long as any needed color-conversion info has been properly initialized).
{

    return 0;
}


/*BOOL VD_CreateSurface(VD_RENDERER *vr, int xres, int yres, uint mode)

// Creates a working surface area for the given renderer.  All surfaces
// attached to a renderer must be fo the same bitdepth as the renderer itself.
//
// mode - can be: VDSURFACE_AUTODETECT, VDSURFACE_VIDMEM, VDSURFACE_SYSMEM.
//   AUTODETECT : will automagically be located in either the video
//   memory or in system memory, depending on the acceleration capabilities
//   of the video hardware and drivers.

{
    int    t = 0;

    vd_physwidth  = l->xres;      vd_tilewidth  = DEFAULT_TILE_WIDTH * l->bytespp;
    vd_physheight = l->yres;      vd_tileheight = DEFAULT_TILE_HEIGHT;
    vd_bytespp    = l->bytespp;   vd_tilesize   = vd_tilewidth * DEFAULT_TILE_HEIGHT;

    vd_ptilewidth = DEFAULT_TILE_WIDTH;
    vd_ptilesize  = DEFAULT_TILE_WIDTH * DEFAULT_TILE_HEIGHT;
    vd_bytewidth  = vd_physwidth * vd_bytespp;
    vd_bytesize   = vd_bytewidth * vd_physheight;
    vd_physsize   = vd_physwidth * vd_physheight;

    vd_nextpage      = l->NextPage;
    vd_nextrect      = l->NextRect;
    vd_clearscreen   = l->ClearScreen;

    vd_GetPalette    = l->GetPalette;    vd_SetPalette   = l->SetPalette;
    vd_FastFadeInit  = l->FastFadeInit;  vd_FastFade     = l->FastFade;

    vd_MouseRestoreBG  = (l->MouseRestoreBG)  ? l->MouseRestoreBG  : dummy_MouseRestoreBG;
    vd_MouseDrawCursor = (l->MouseDrawCursor) ? l->MouseDrawCursor : dummy_MouseDrawCursor;

    v_line  = l->vLine;    h_line  = l->hLine;
    line    = l->Line;     rect    = l->Rect;
    box     = l->Box;      fillbox = l->FillBox;

    vd_spriteblitter = l->SpriteBlitter;
    vd_tileopaque    = l->TileOpaque;
    vd_tiletrans     = l->TileTrans;
    vd_tilefunky     = l->TileFunky;

    vd_red_fieldpos    = l->FieldPosRed;
    vd_red_fieldsize   = l->FieldSizeRed;
    vd_red_maxvalue    = (1 << vd_red_fieldsize);
    vd_green_fieldpos  = l->FieldPosGreen;
    vd_green_fieldsize = l->FieldSizeGreen;
    vd_green_maxvalue  = (1 << vd_green_fieldsize);
    vd_blue_fieldpos   = l->FieldPosBlue;
    vd_blue_fieldsize  = l->FieldSizeBlue;
    vd_blue_maxvalue   = (1 << vd_blue_fieldsize);

    vd_driver = l;

    printlog("Video > Mode selected: %dx%dx%d.",vd_physwidth,vd_physheight,vd_bytespp);

    // Allocate off-screen memory [if not already allocated]

    if(vd_allocflag && vd_vidmem==NULL)
    {   if((vd_vidmem = (UBYTE *)_mm_malloc(vd_bytewidth * vd_physheight)) == NULL) return 1;
        printlog("Video > Off-screen buffer allocated.");
    }

    // Allocate y-scan index chart [if not already allocated]

    if(vd_scrtable == NULL)
    {   if((vd_scrtable = (UBYTE **)_mm_malloc(vd_physheight * sizeof(UBYTE *))) == NULL) return 1;
        VD_InitScrTable(vd_vidmem,vd_bytewidth,vd_physheight,vd_bytespp);
        printlog("Video > Lookup table allocated and initialized.");
    }

    VD_SetMouseColor(255, 255, 255);

    return 0;
}*/


/*void VD_InfoDriver(void)
{
    int t;
    VDRIVER *l;

    for(t=1,l=vdrvbegin; l!=NULL; l=l->next, t++);
       printf("%d. %s\n",t,l->version);
}*/


// ===========================================================================
void VD_RegisterDriver(VDRIVER *drv)
// ===========================================================================
{
    VDRIVER *cruise = vdrvbegin;

    if(cruise)
    {   while(cruise->next)  cruise = cruise->next;
        cruise->next = drv;
    } else
        vdrvbegin = drv; 
}


// ===========================================================================
void VD_Exit(VD_RENDERER *vr)
// ===========================================================================
{
    if(vr)
    {   
        _mmlogd("vdrive > Deinitializing renderer...");
        
        if(vr->initialized && vr->vdriver.ExitMode) vr->vdriver.ExitMode(vr);
        if(vr->vdriver.Exit) vr->vdriver.Exit(vr);
        
        _mmlogd(" > Unregistering blitters...");
        Sprite_FreeBlitters(&vr->surface);

        _mm_free(vr, "Done!");
    }
}


// ===========================================================================
void VD_ExitMode(VD_RENDERER *vr)
// ===========================================================================
{
    if(vr)
    {   vr->visible = 0;
        if(vr->initialized && vr->vdriver.ExitMode) vr->vdriver.ExitMode(vr);
        Sprite_FreeBlitters(&vr->surface);
        vr->initialized = 0;
    }
}

/*void VD_InitScrTable(UBYTE *base, int scrnwidth, int scrnheight, int bytespp)
{
   int i;

   for(i=0; i<scrnheight; i++, base+=scrnwidth) vd_scrtable[i] = base;
}
*/

/*void VD_SetIntensity(int intensity)
{
    vd_intensity = intensity;
}


void VD_AllocateBuffer(BOOL val)
{
    vd_allocflag = val;
}
*/

/*void VD_SetTileSize(int xsize, int ysize)
{
    vd_ptilesize = (vd_ptilewidth = xsize) * ysize;
    vd_tilesize  = (vd_tilewidth = xsize * vd_bytespp) * (vd_tileheight = ysize);
}
*/

// ====================================================================================
    static SPR_BLITTER *spr_structdup(VD_SURFACE *vs, SPR_BLITTER *sprdrv)
// ====================================================================================
{
    // Duplicate the sprite structure list that we are given.

    SPR_BLITTER *weenie;
    
    if(!sprdrv) return NULL;

    if((sprdrv->bytespp == vs->bytespp) && ((sprdrv->cpu == vd_cpu) || (sprdrv->cpu == CPU_NONE))
    && ((sprdrv->fieldpos.red == SPRBLT_UNUSED) || !memcmp(&sprdrv->fieldpos, &vs->fieldpos, sizeof(vs->fieldpos))))
    {   weenie = (SPR_BLITTER *)_mm_structdup(sprdrv, sizeof(SPR_BLITTER));
        if(weenie) weenie->next = spr_structdup(vs, sprdrv->next);
    } else weenie = spr_structdup(vs, sprdrv->next);

    return weenie;
}


// ====================================================================================
    void Sprite_RegisterBlitterList(VD_SURFACE *vs, SPR_BLITTER *sprlst[])
// ====================================================================================
{
    if(sprlst)
    {   uint   i;
        for(i=0; i<16, sprlst[i]; i++)
            Sprite_RegisterBlitter(vs, sprlst[i]);
    }
}


// ====================================================================================
    void Sprite_RegisterBlitter(VD_SURFACE *vs, SPR_BLITTER *sprdrv)
// ====================================================================================
{
    SPR_BLITTER *cruise;

    if(vs->sprdrv)
    {   cruise = vs->sprdrv;
        while(cruise->next) cruise = cruise->next;
        cruise->next = spr_structdup(vs, sprdrv);
    } else vs->sprdrv = spr_structdup(vs, sprdrv);
}


// ====================================================================================
    void Sprite_FreeBlitters(VD_SURFACE *vs)
// ====================================================================================
{
    SPR_BLITTER *cruise;

    cruise = vs->sprdrv;
    while(cruise)
    {   SPR_BLITTER  *ftmp = cruise;
        cruise = cruise->next;
        free(ftmp);
    }
    vs->sprdrv = NULL;
}


// ====================================================================================
    static SPR_BLITTER *configsprite(const VD_SURFACE *vs, uint flags, uint opacity)
// ====================================================================================
{
    SPR_BLITTER   *cruise, *goodblt;
    uint           goodone;

    // Find a driver that is suitable for this sample

    cruise  = vs->sprdrv;
    goodone = 1;         // set to one, so that we fail if no good matches
    goodblt = NULL;

    while(cruise)
    {   // Check for:
        //  (a) blit flag match.
        //  (b) surface prebuffer and alignment are greater than or equal to sprite's
        //  (c) sprite align is a factor of surface alignment (to make sure they are compat)

        //_mmlog("opacity: %d, %d; prebuf: %d, %d; align %d, %d;",cruise->opacity,opacity,cruise->prebuf, vs->prebuffer, cruise->align, vs->alignment);

        if((cruise->opacity == opacity)  && (cruise->prebuf <= vs->prebuffer)
        && (cruise->align <= vs->alignment) && !(vs->alignment % cruise->align))
        {   uint   weight = 0;

            // Check for non-critials:  CPU and alpha blit flag compatability
            if((cruise->flags == flags) && (cruise->cpu == vd_cpu))
                return cruise;

            // weigh the 'matchability' of this driver. Generally, we are 
            // looking for the fastest and/or best quality driver. This 
            // makes the assumptions that any driver that requires special
            // CPUs is probably pretty fast.

            if(flags != cruise->flags)
            {   // we know that cruise->cpu == vd_cpu.
                if(!cruise->flags)
                {   weight += 40;
                    if(cruise->cpu != CPU_NONE) weight += 30;
                }
            } else if(cruise->cpu == CPU_NONE) weight += 50;

            if(weight > goodone)
            {   goodone = weight;
                goodblt = cruise;
            }
        }

        cruise = cruise->next;
    }

    if(goodblt)
    {   return goodblt;
    } else
    {   //woops, we can't blit this sprite!
        _mmlog("vdriver > configsprite > Attempted to configure sprite, but no suitable blit was available!");
        return NULL;
    }
}


// ===========================================================================
    void spralpha_placebo(const SPRITE *spr, int xloc, int yloc, uint alpha)
// ===========================================================================
{

}

    
// ===========================================================================
    void sprcross_placebo(const SPRITE *spra, const SPRITE *sprb, int xloc, int yloc, uint alpha)
// ===========================================================================
{

}


// ===========================================================================
    void sprblit_placebo(const SPRITE *spr, int xloc, int yloc)
// ===========================================================================
{

}

// ====================================================================================
    BOOL Sprite_Init(VD_SURFACE *vs, SPRITE *dest, int xsize, int ysize,  uint flags, uint opacity)
// ====================================================================================
{
    SPR_BLITTER *sprdrv = NULL;

    memset(dest,0,sizeof(SPRITE));
    
    if(xsize && ysize) sprdrv = configsprite(vs, flags, opacity);

    if(sprdrv)
    {    // Set up stuff based on the driver...
        dest->api.blit       = sprdrv->api.blit       ? sprdrv->api.blit       : sprblit_placebo;
        dest->api.alphablit  = sprdrv->api.alphablit  ? sprdrv->api.alphablit  : spralpha_placebo;
        dest->api.blackfade  = sprdrv->api.blackfade  ? sprdrv->api.blackfade  : spralpha_placebo;
        dest->api.fastshadow = sprdrv->api.fastshadow ? sprdrv->api.fastshadow : sprblit_placebo;

        dest->physwidth = (xsize + vs->prebuffer + vs->alignment) & ~(vs->alignment-1);

        // Note: We only allocate crap if we're not dealing with RLE sprites.
        // Otherwise, it is up to the end user to allocate shizat.
        
        if(!(dest->opacity == SPR_FUNKY))
        {   dest->bitalloc    = _mm_calloc(dest->physwidth*vs->bytespp, ysize);

            if(!dest->bitalloc)
            {   _mmlog("Vdriver > SpriteInit > Could not create sprite bitmap!");
                return 0;
            }
            dest->bitmap = dest->bitalloc + (vs->bytespp * vs->prebuffer);      // tack on the prebuffer.
        }
    } else
    {
        dest->api.blit       = sprblit_placebo;
        dest->api.alphablit  = spralpha_placebo;
        dest->api.blackfade  = spralpha_placebo;
        dest->api.fastshadow = sprblit_placebo;
    }

    // Configure givens (independent of platform/driver)
    dest->vs        = vs;
    dest->xsize     = xsize;
    dest->ysize     = ysize;

    dest->opacity   = opacity;
    //dest->flags     = flags;
    dest->flags    = flags;

    return 1;
}


// ====================================================================================
    void Sprite_Free(SPRITE *sprite)
// ====================================================================================
{
    if(sprite && sprite->bitalloc)
        _mm_free(sprite->bitalloc, NULL);
}

// Crossfading works on opaque sprites only (for now).

void  (*sprblit_crossfade)(const struct SPRITE *spra, const struct SPRITE *sprb, int xloc, int yloc, uint alpha);

