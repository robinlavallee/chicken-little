/*

  Vdriver Resource-based Loader API

*/

#include "vdriver.h"
#include "vdrespak.h"
#include <string.h>

#define COMPACT_DISKFORM

#ifndef SPRHEAD_SIZE
#ifdef COMPACT_DISKFORM
#define SPRHEAD_SIZE 20
#else
#define SPRHEAD_SIZE (20+60)
#endif
#endif


// ===========================================================================
    static void LoadShit32(VD_SURFACE *vs, ULONG *dbmp, uint xsize, BOOL alpha, MMSTREAM *datafp)
// ===========================================================================
// For Loading funky (RLE) sprites when the rendering surface is 32 bpp.
// It is used to convert runs (or scans) to the proper RGB order.
// If 'alpha' is ture, then the source file is assumed to be 32 bit data, else
// it is assumed 24 bit!
{
    uint   x;

    for(x=0; x<xsize; x++, dbmp++)
    {   UBYTE  t1,t2,t3;
        t1 = _mm_read_UBYTE(datafp);
        t2 = _mm_read_UBYTE(datafp);
        t3 = _mm_read_UBYTE(datafp);
        if(alpha) _mm_read_UBYTE(datafp);

        *dbmp = VD_MakeColor(vs, t1, t2, t3);
    }
}

// ===========================================================================
    static void LoadShit16(VD_SURFACE *vs, UWORD *dbmp, uint xsize, BOOL alpha, MMSTREAM *datafp)
// ===========================================================================
// For Loading funky (RLE) sprites when the rendering surface is 16 bpp.
// It is used to convert runs (or scans) to the proper RGB order and to 16
// bit from the default 32 bit.
// If 'alpha' is ture, then the source file is assumed to be 32 bit data, else
// it is assumed 24 bit!
{
    uint   x;

    for(x=0; x<xsize; x++, dbmp++)
    {   UBYTE  t1,t2,t3;
        t1 = _mm_read_UBYTE(datafp);
        t2 = _mm_read_UBYTE(datafp);
        t3 = _mm_read_UBYTE(datafp);
        if(alpha) _mm_read_UBYTE(datafp);

        *dbmp = VD_MakeColor(vs, t1, t2, t3);
    }
}

// ===========================================================================
    static void *VDRes_LoadBitmap(VD_RESPAK *res, SPRITE *spr)
// ===========================================================================
// Loads a bitmap into the given sprite structure, using the residx value of
// the sprite as the resource index of the bitmap to load.
{
    if(!spr) return NULL;

    if(!res->bitmap[spr->residx].alloc) 
    {   BOOL usealpha;
    
        // Nope.  Gotta load before we go home!

        //_mm_fseek(res->datafp, (spr->residx*4) + 4, SEEK_SET);
        //_mm_fseek(res->datafp, _mm_read_I_ULONG(res->datafp) + res->dataseek, SEEK_SET);
        _mm_fseek(res->datafp, res->dataseek[spr->residx], SEEK_SET);

        usealpha = 0; //spr->aflags & SPRA_ALPHA;

        if(spr->opacity == SPR_FUNKY)
        {   uint        itemp = spr->ysize;
            ULONG      *ttmp;
            uint        seeker;

            ULONG     **lookup;

            // Currently I alloc the same memory for both 32 and 16 bit images.
            // Note: I have to allocate extra for the yloc indexes!
            seeker = _mm_read_I_ULONG(res->datafp);
            spr->bitalloc = (UBYTE *)_mm_malloc(4*(seeker + spr->ysize));

            if(!spr->bitalloc)
            {   _mmlog("LoadBitmap > Funky allocation failure: %d, %d",seeker, spr->ysize);
                return NULL;
            }

            // Ugh.  Have to process and load the sprite.
            
            ttmp    = (ULONG *)spr->bitalloc;
            lookup  = (ULONG **)spr->bitalloc;

            ttmp        += spr->ysize;
            spr->bitmap  = (UBYTE *)ttmp;

            do
            {   uint    cc;

                // bit 0   set = opaque; clear = transparent.
                // bit 1   set = no type change; clear = type changes

                cc = _mm_read_I_ULONG(res->datafp);
                *lookup = ttmp;  lookup++;
                *ttmp = cc; ttmp++;

                if(cc & 2)
                {   if(cc & 1)
                    {   if(spr->vs->bytespp == 4)
                        {   LoadShit32(spr->vs, ttmp, spr->xsize, usealpha, res->datafp);
                            ttmp += spr->xsize;
                        } else
                        {   LoadShit16(spr->vs, (UWORD *)ttmp, spr->xsize, usealpha, res->datafp);
                            ttmp += (spr->xsize+1)>>1;
                        }
                    }
                } else
                {   if(cc & 1)
                    {   cc  = _mm_read_I_ULONG(res->datafp);
                        do
                        {   *ttmp = cc; ttmp++;
                            if(spr->vs->bytespp == 4)
                            {   LoadShit32(spr->vs, ttmp, cc, usealpha, res->datafp);
                                ttmp += cc;
                            } else
                            {   LoadShit16(spr->vs, (UWORD *)ttmp, cc, usealpha, res->datafp);
                                ttmp += (cc+1)>>1;
                            }
                                
                            cc = _mm_read_I_ULONG(res->datafp);
                            *ttmp = cc; ttmp++;
                            if(cc == 0) break;
                            cc = _mm_read_I_ULONG(res->datafp);
                        } while(1);
                    } else
                    {   cc = _mm_read_I_ULONG(res->datafp);
                        *ttmp = cc; ttmp++;
                                
                        do
                        {   cc  = _mm_read_I_ULONG(res->datafp);
                            *ttmp = cc; ttmp++;
                            if(spr->vs->bytespp == 4)
                            {   LoadShit32(spr->vs, ttmp, cc, usealpha, res->datafp);
                                ttmp += cc;
                            } else
                            {   LoadShit16(spr->vs, (UWORD *)ttmp, cc, usealpha, res->datafp);
                                ttmp += (cc+1)>>1;
                            }
                            cc = _mm_read_I_ULONG(res->datafp);
                            *ttmp = cc; ttmp++;
                            if(cc == 0) break;
                        } while(1);
                    }
                }
            } while(--itemp);
            res->bitmap[spr->residx].map    = spr->bitmap;
            res->bitmap[spr->residx].alloc  = spr->bitalloc;
        } else
        {   res->bitmap[spr->residx].alloc  = spr->bitalloc;
            res->bitmap[spr->residx].map    = spr->bitmap;

            if(!spr->bitalloc)
            {   _mmlog("LoadBitmap > Allocation failure: %d, %d",spr->xsize, spr->ysize);
                return NULL;
            }

            if(spr->vs->bytespp == 4)
            {   uint   y;
                ULONG  *crap = (ULONG *)res->bitmap[spr->residx].map;

                for(y=0; y<spr->ysize; y++, crap+=spr->physwidth)
                    LoadShit32(spr->vs, crap, spr->xsize, usealpha, res->datafp);
            } else if(spr->vs->bytespp == 2)
            {   uint   y;
                UWORD  *crap = (UWORD *)res->bitmap[spr->residx].map;
            
                for(y=0; y<spr->ysize; y++, crap+=spr->physwidth)
                {   LoadShit16(spr->vs, crap, spr->xsize, usealpha, res->datafp);
                }
            }
            spr->bitalloc = NULL;       // we do our own memory management
            spr->bitmap   = res->bitmap[spr->residx].map;
        }
    } else
    {   spr->bitalloc = res->bitmap[spr->residx].alloc;
        spr->bitmap   = res->bitmap[spr->residx].map;
    }

    //for(i=0; i<res->numspr; i++)
//        _mmlog("Sprite Assign %d  %d", i, res->sprlist[i]);

    return res->bitmap[spr->residx].map;
}


// ===========================================================================
    BOOL VDRes_LoadDependencies(VD_RESPAK *res)
// ===========================================================================
// Loads any bitmap resources that are needed by the currently-loaded list of
// sprites.  This is normally used in conjunction with a surface change or
// some situations in cacheing logic.
{
    uint   i;

//    _mmlog("Reload Dependencies!\n===================================");
    for(i=0; i<res->numspr; i++)
        VDRes_LoadBitmap(res, res->sprlist[i]);

    return -1;
}


// ===========================================================================
    BOOL VDRes_ReloadResources(VD_RESPAK *res, VD_SURFACE *vs)
// ===========================================================================
// Notes:
//  - It would be very bad to try to refresh the screen or something while this
//    function is executing, so make sure any threads are suspended!
{
    uint   i;

    res->vs = vs;

    // Unload old snazz jazz
    // Erm, those are the raw bitmap resource data thingies, for those of you not
    // up on my terminology book. :)
    
    _mmlogd("vdres > ReloadResources > Unloading Bitmaps...");
    for(i=0; i<res->numbit; i++)
        _mm_free(res->bitmap[i].alloc, NULL);

    // Remember that we need to reinitialize the sprite to suit the new
    // display surface properties!

    _mmlogd(" > Reinitializing Sprites...");

    for(i=0; i<res->numspr; i++)
    {   SPRITE *spr;

        int   residx;

        spr       = res->sprlist[i];
        residx    = spr->residx;

        Sprite_Init(res->vs, spr, spr->xsize, spr->ysize, spr->flags, spr->opacity);

        spr->residx = residx;
    }

    // Reload all dependant resources.
    
    VDRes_LoadDependencies(res);

    return -1;
}


// ===========================================================================
    BOOL VDRes_LoadSprite(VD_RESPAK *res, SPRITE *spr, uint idx)
// ===========================================================================
// Returns 0 on failure.
// Loads the requested resource (res == index) from the currently active game
// resource file.  We keep track of ALL sprites we load, so that we can quick-
// ly and painlessly reload them in the event of a surfacechange.
{
#ifndef COMPACT_DISKFORM
    CHAR  catname[60];
#endif
    uint  flags, opacity;
    int   xsize, ysize;

    // Seek to the proper locations in the header and data files

    _mm_fseek(res->headfp, idx*SPRHEAD_SIZE, SEEK_SET);

    flags     = _mm_read_I_ULONG(res->headfp);
    opacity   = _mm_read_I_ULONG(res->headfp);
    xsize     = _mm_read_I_ULONG(res->headfp);
    ysize     = _mm_read_I_ULONG(res->headfp);

    // configure our sprite, then load rest stuff

    Sprite_Init(res->vs, spr, xsize, ysize, flags, opacity);

    //spr->alpha     = _mm_read_I_ULONG(res->headfp);
    //spr->gamma     = _mm_read_I_ULONG(res->headfp);
    //spr->dst_gamma = _mm_read_I_ULONG(res->headfp);

#ifndef COMPACT_DISKFORM
    _mm_read_UBYTES(catname,60,res->headfp);
    _mmlog("Resource Load > %s", catname);
#endif

    spr->residx = _mm_read_I_ULONG(res->headfp);

    // Add this sprite to our global list of loaded sprites!

    if(res->sprlist_alloc <= res->numspr)
    {   // allocate more sprite list room!
        res->sprlist = (SPRITE **)_mm_realloc(res->sprlist, sizeof(SPRITE *) * (res->sprlist_alloc + 128));
        memset(&res->sprlist[res->sprlist_alloc],0,127*sizeof(SPRITE *));
        res->sprlist_alloc += 128;
    }

    res->sprlist[res->numspr] = spr;

    res->numspr++;

    // Load the bitmap data (if it isn't loaded already)

    //VDRes_LoadBitmap(res, spr);

    return -1;
}


// ===========================================================================
    VD_RESPAK *VDRes_OpenFN(VD_SURFACE *vs, CHAR *headfn, CHAR *datafn)
// ===========================================================================
// Creates a handle for accessing resources.
{
    VD_RESPAK     *newres;
    uint           i;

    newres = (VD_RESPAK *)_mm_calloc(1, sizeof(VD_RESPAK));

    newres->headfp = _mm_fopen(headfn,"rb");
    if(!newres->headfp) return NULL;
    newres->datafp = _mm_fopen(datafn,"rb");
    if(!newres->datafp)
    {   _mm_fclose(newres->headfp);
        return NULL;
    }

    newres->numbit   = _mm_read_I_ULONG(newres->datafp);
    newres->dataseek = (uint *)_mm_malloc(sizeof(int) * newres->numbit);

    for(i=0; i<newres->numbit; i++)
        newres->dataseek[i] = _mm_read_I_ULONG(newres->datafp) + ((newres->numbit * 4) + 4);

    newres->bitmap   = (SPR_ALLOC *)_mm_calloc(newres->numbit, sizeof(SPR_ALLOC));
    newres->vs       = vs;

    return newres;

}


// ===========================================================================
    void VDRes_Close(VD_RESPAK *res)
// ===========================================================================
// Closes the resource file and unloads all resources associated with it.
//
{
    uint   i;
    
    if(!res) return;

    _mmlogd("vdres > Closing resource pack");

    if(res->bitmap)
    {   
        _mmlogd(" > Unloading Bitmaps...");
        for(i=0; i<res->numbit; i++)
            _mm_free(res->bitmap[i].alloc, NULL);
    }
    
    _mm_free(res->dataseek, "dataseek");

    _mm_free(res->sprlist, "sprite list array");

    _mm_fclose(res->headfp);
    _mm_fclose(res->datafp);

    _mm_free(res, "Done!");
}
