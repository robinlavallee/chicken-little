/*

  Mikmod Resource-based Loader API

*/

#include "clres.h"          // temporary measure until I clean up later!
#include <string.h>


#define COMPACT_DISKFORM

#ifndef SAMPLE_HEADSIZE
#ifdef COMPACT_DISKFORM
#define SAMPLE_HEADSIZE 40
#else
#define SAMPLE_HEADSIZE (40 + 60)
#endif
#endif


// ===========================================================================
    MD_RESPAK *MDRes_OpenFN(MDRIVER *md, CHAR *headfn, CHAR *datafn)
// ===========================================================================
// Creates a handle for accessing resources.
{
    MD_RESPAK     *newres;
    uint           i;

    newres = (MD_RESPAK *)_mm_calloc(1, sizeof(MD_RESPAK));

    newres->headfp = _mm_fopen(headfn,"rb");
    if(!newres->headfp) return NULL;
    newres->datafp = _mm_fopen(datafn,"rb");
    if(!newres->datafp)
    {   _mm_fclose(newres->headfp);
        return NULL;
    }

    newres->numsamp   = _mm_read_I_ULONG(newres->datafp);
    newres->datatable = (MDRES_DATATABLE *)_mm_malloc(sizeof(MDRES_DATATABLE) * newres->numsamp);

    for(i=0; i<newres->numsamp; i++)
    {   newres->datatable[i].seek   = _mm_read_I_ULONG(newres->datafp) + ((newres->numsamp * 8)+4);
        newres->datatable[i].format = _mm_read_I_ULONG(newres->datafp);
    }

    newres->bitmap   = _mm_calloc(newres->numsamp, sizeof(int));
    newres->md       = md;

    return newres;

}


// ===========================================================================
    void MDRes_Close(MD_RESPAK *res)
// ===========================================================================
{
    uint  i;


    for(i=0; i<res->numsamp; i++)
        MD_SampleUnload(res->md, res->bitmap[i]);
}


// ===========================================================================
    BOOL MDRes_LoadDependencies(MD_RESPAK *res)
// ===========================================================================
{
    uint   i;

    for(i=0; i<res->numhead; i++)
    {   if(!res->bitmap[res->header[i]->residx])
            SL_RegisterSample(res->md, &res->bitmap[res->header[i]->residx], res->datatable[i].format, res->header[i]->length,
                          0, res->datafp, res->datatable[res->header[i]->residx].seek);
    }

    SL_LoadSamples(res->md);

    for(i=0; i<res->numhead; i++)
    {
        res->header[i]->handle = res->bitmap[res->header[i]->residx];
    }

    return -1;
}


// ===========================================================================
    BOOL MDRes_ReloadResources(MD_RESPAK *res, MDRIVER *md)
// ===========================================================================
{
    uint   i;

    res->md = md;

    // Unload old snazz jazz
    // Erm, those are the raw bitmap resource data thingies, for those of you not
    // up on my terminology book. :)
    
//    _mmlog("Unloading Bitmapped Sample Data!");
    for(i=0; i<res->numsamp; i++)
        MD_SampleUnload(res->md, res->bitmap[i]);

    // Reconfigure the MD_SAMPLE headers here -->
    // [...]

    /*for(i=0; i<res->numhead; i++)
    {   
    }*/

    // Reload all dependant resources.
    
    MDRes_LoadDependencies(res);

    return -1;
}

int MDRes_LoadSampleXAudio(MD_RESPAK* res, uint idx) {
  char sample_path[255];
  sprintf_s(sample_path, sizeof(sample_path), "data/sounds/%d.bin", idx);
  return XAudioBuffer_Create(sample_path);
}

// ===========================================================================
    MD_SAMPLE *MDRes_LoadSample(MD_RESPAK *res, uint idx)
// ===========================================================================
// Returns 0 on failure.
{
#ifndef COMPACT_DISKFORM
    CHAR  catname[60];
#endif

    MD_SAMPLE  *samp;

    samp = (MD_SAMPLE *)_mm_calloc(1,sizeof(MD_SAMPLE));

    // Seek to the proper locations in the header

    _mm_fseek(res->headfp, idx*SAMPLE_HEADSIZE, SEEK_SET);

    samp->speed   = _mm_read_I_ULONG(res->headfp);
    samp->volume  = _mm_read_I_SLONG(res->headfp);
    samp->panning = _mm_read_I_SLONG(res->headfp);

    samp->length  = _mm_read_I_SLONG(res->headfp);
    samp->reppos  = _mm_read_I_SLONG(res->headfp);
    samp->repend  = _mm_read_I_SLONG(res->headfp);
    samp->suspos  = _mm_read_I_SLONG(res->headfp);
    samp->susend  = _mm_read_I_SLONG(res->headfp);

    samp->flags   = _mm_read_I_ULONG(res->headfp);

    //samp->volume  = 384;

#ifndef COMPACT_DISKFORM
    _mm_read_UBYTES(catname,60,res->headfp);
    _mmlog("Resource Load > %s", catname);
#endif

    samp->residx  = _mm_read_I_ULONG(res->headfp);

    // Add this sprite to our global list of loaded sprites!

    if(res->header_alloc <= res->numhead)
    {   // allocate more sprite list room!
        res->header = (MD_SAMPLE **)_mm_realloc(res->header, sizeof(MD_SAMPLE *) * (res->header_alloc + 128));
        memset(&res->header[res->header_alloc],0,127*sizeof(MD_SAMPLE *));
        res->header_alloc += 128;
    }

    res->header[res->numhead] = samp;

    res->numhead++;

    return samp;
}


