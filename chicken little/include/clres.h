// ==========================
//   Graphics Resource Crap
// ==========================

#ifndef _CLRES_H_
#define _CLRES_H_

#include "clgfxres.h"            // generated resource header
#include "clsfxres.h"            // generated resource header
#include "vdrespak.h"
#include "animation.h"

#include "mikmod.h"
#include "audiomanager_c_bridge.h"
#include "uniform.h"

#include "vdfont.h"

// =====================================================================================
    typedef struct MDRES_DATATABLE
// =====================================================================================
{   uint    seek;
    uint    format;
} MDRES_DATATABLE;


// =====================================================================================
    typedef struct MD_RESPAK
// =====================================================================================
{
    MMSTREAM   *headfp,      // filename of the resource headers.
               *datafp;      // filename of the bitmap data (image or audio)

    MDRES_DATATABLE *datatable;

    uint        numsamp;
    int        *bitmap;

    MDRIVER    *md;
    
    uint        header_alloc;
    uint        numhead;
    MD_SAMPLE **header;
} MD_RESPAK;


//======================================================================================
    typedef struct RES_GPSPR
//======================================================================================
// This is the basic gamepiece resource (inert egg/stone/etc), along with their iconized
// versions.  This is used to blitter the non-entity gamepieces in our game, specifically
// the falling eggs and 'next' pieces which are represented by a type index alone.
// I may change this later, but for now it works great!
{
    SPRITE   full,
             icon,
             outline;
} RES_GPSPR;


//======================================================================================
    typedef struct RES_MUSIC
//======================================================================================
{
    UNIMOD   *title;
    UNIMOD   *stage[2];

} RES_MUSIC;

//======================================================================================
    typedef struct RES_FONT
//======================================================================================
{
    VD_FONT     *big, *little;      // big and small (oh, sorry Microsoft!  *LITTLE*.. jesus fucking christ) fonts.
    VD_FONT     *score;             // woo, the little 8x16 score font!    
} RES_FONT;


//======================================================================================
    typedef struct RES_SNDFX
//======================================================================================
{

    int    eggcrack,
                 eggbreak,
                 eggrotate,
                 eggplace;

    int birdie1, birdie2;

    int blip, crumble, thump;

    int go, getready,
                 win, lose;

    int menuhigh, menuselect;

} RES_SNDFX;


// =====================================================================================
    typedef struct CL_RES
// =====================================================================================
{   
    VD_RESPAK   *videopak;       // vdriver resource pack.
    MD_RESPAK   *audiopak;       // mikmod resource pack
} CL_RES;

extern BOOL      CL_LoadResources(struct GAMEDATA *chick);

extern void      CL_OpenResource(VD_SURFACE *vs, MDRIVER *md);
extern void      CL_CloseResource(void);

extern BOOL      CL_LoadAnimation(CL_RES *res, ANIM_SEQUENCE *seq, uint idx, uint count);
extern void      CL_UnloadAnimation(CL_RES *res, ANIM_STATE *augh);


extern MD_RESPAK *MDRes_OpenFN(MDRIVER *md, CHAR *headfn, CHAR *datafn);
extern BOOL       MDRes_LoadDependencies(MD_RESPAK *res);
extern BOOL       MDRes_ReloadResources(MD_RESPAK *res, MDRIVER *md);
extern MD_SAMPLE *MDRes_LoadSample(MD_RESPAK *res, uint idx);
extern void       MDRes_Close(MD_RESPAK *res);


extern CL_RES     *respak;

extern RES_SNDFX  sfx;
extern RES_FONT   font;
extern RES_MUSIC  music;

#endif
