

#ifndef _VD_RESPAK_H_
#define _VD_RESPAK_H_

#include "vdriver.h"

typedef struct SPR_ALLOC
{   void       *alloc,
               *map;
} SPR_ALLOC;


typedef struct VD_RESPAK
{   MMSTREAM   *headfp,      // filename of the resource headers.
               *datafp;      // filename of the bitmap data (image or audio)

    uint       *dataseek;    // The seekbase of the data (past the indexes).
                             // I may want to use MMIO file stuff for this later.

    // list of bitmap resources.  Each entry is either a pointer to the bitmap or
    // NULL if the bitmap has not been loaded yet.

    uint        numbit;
    SPR_ALLOC  *bitmap;

    // List of headers (or pointers) where the resources are loaded.  Used to
    // automate the reloading of resources when the device properties change.

    uint        numspr;
    uint        sprlist_alloc;
    SPRITE    **sprlist;

    VD_SURFACE *vs;
   
} VD_RESPAK;

extern BOOL       VDRes_LoadDependencies(VD_RESPAK *res);
extern BOOL       VDRes_ReloadResources(VD_RESPAK *res, VD_SURFACE *vs);
extern BOOL       VDRes_LoadSprite(VD_RESPAK *res, SPRITE *spr, uint idx);
extern VD_RESPAK *VDRes_OpenFN(VD_SURFACE *vs, CHAR *headfn, CHAR *datafn);
extern void       VDRes_Close(VD_RESPAK *res);

#endif
