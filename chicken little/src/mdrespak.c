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
    MD_RESPAK *MDRes_OpenFN(CHAR *headfn, CHAR *datafn)
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

    return newres;

}


// ===========================================================================
    void MDRes_Close(MD_RESPAK *res)
// ===========================================================================
{
}


// ===========================================================================
    BOOL MDRes_LoadDependencies(MD_RESPAK *res)
// ===========================================================================
{
    return -1;
}


// ===========================================================================
    BOOL MDRes_ReloadResources(MD_RESPAK *res)
// ===========================================================================
{
    // uint   i;

    // Unload old snazz jazz
    // Erm, those are the raw bitmap resource data thingies, for those of you not
    // up on my terminology book. :)
    
//    _mmlog("Unloading Bitmapped Sample Data!");
    // for(i=0; i<res->numsamp; i++)
    //     MD_SampleUnload(res->bitmap[i]);

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
  sprintf_s(sample_path, sizeof(sample_path), "data/sounds/%d.pcm", idx);
  return XAudioBuffer_Create(sample_path);
}
