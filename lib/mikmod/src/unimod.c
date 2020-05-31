/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: unimod.c
 
  Structure handling and manipulation.  Includes loading, freeing, and track
  manipulation functions for the UNIMOD format.

 Portability:
  All compilers - All Systems.

*/

#include <string.h>
#include <stdarg.h>
#include "mikmod.h"
#include "uniform.h"


#ifdef __MM_WINAMP__
extern int config_modpan;
#endif


// =====================================================================================
    static BOOL loadsamples(MDRIVER *md, UNIMOD *of, MMSTREAM *fp)
// =====================================================================================
{
    UNISAMPLE  *s;
    int         u;

    for(u=of->numsmp, s=of->samples; u; u--, s++)
        if(s->length)
        {
            #ifdef __MM_WINAMP__
            s->old16bits = s->format & SF_16BITS;
            #endif
            SL_RegisterSample(md, &s->handle, s->format, s->length, s->compress, fp, s->seekpos);
        }

    return 1;
}


// =====================================================================================
    static void ML_XFreeSample(MDRIVER *md, UNISAMPLE *s)
// =====================================================================================
{
    if(s->handle >= 0)
        MD_SampleUnload(md, s->handle);

    _mm_free(s->samplename, NULL);
}


// =====================================================================================
    static void ML_XFreeInstrument(INSTRUMENT *i)
// =====================================================================================
{
    _mm_free(i->insname, NULL);
}


/******************************************

    Next are the user-callable functions

******************************************/


// =====================================================================================
    void Unimod_Free(UNIMOD *mf)
// =====================================================================================
{
    int t;

    if(!mf) return;

    _mmlog("Unimod > Unloading module \'%s\'", mf->songname);

    _mm_free(mf->songname, "Songname");
    _mm_free(mf->composer, "Composer");
    _mm_free(mf->comment, "Comment");

    _mm_free(mf->modtype, "Modtype");
    _mm_free(mf->positions, "Positions");
    _mm_free(mf->patterns, "Patterns");
    _mm_free(mf->pattrows, "Pattern Rows");

    if(mf->tracks)
    {   
        _mmlogd(" > Unloading tracks");

        for(t=0; t<mf->numtrk; t++)
            if(mf->tracks[t] != utrk_blanktrack) _mm_free(mf->tracks[t], NULL);
        _mm_free(mf->tracks, NULL);
    }

    if(mf->globtracks)
    {
        _mmlogd(" > Unloading global tracks");
        for(t=0; t<mf->numpat; t++)
            if(mf->globtracks[t] != utrk_blanktrack) _mm_free(mf->globtracks[t], NULL);
        _mm_free(mf->globtracks, NULL);
    }

    if(mf->instruments)
    {   
        _mmlogd(" > Unloading tracks");

        for(t=0; t<mf->numins; t++)
            ML_XFreeInstrument(&mf->instruments[t]);
        _mm_free(mf->instruments, NULL);
    }

    if(mf->samples)
    {   
        _mmlogd(" > Unloading Samples");

        for(t=0; t<mf->numsmp; t++)
            ML_XFreeSample(mf->md, &mf->samples[t]);
        _mm_free(mf->samples, NULL);
        _mm_free(mf->extsamples, "Extended sample information");
    }
    _mmlogd(" > Done!");
}

extern MLOADER *firstloader;

#ifndef __MM_WINAMP__
// =====================================================================================
    CHAR *Unimod_LoadTitle(CHAR *filename)
// =====================================================================================
{
    MLOADER *l;
    CHAR    *retval;
    MMSTREAM *mmp;

    if((mmp = _mm_fopen(filename,"rb"))==NULL) return NULL;

    // Try to find a loader that recognizes the module

    for(l=firstloader; l; l=l->next)
    {   _mm_rewind(mmp);
        if(l->Test(mmp)) break;
    }

    if(l==NULL)
    {   _mmerr_set(MMERR_UNSUPPORTED_FILE, "Unknown module format or corrupted file.");
        return NULL;
    }

    retval = l->LoadTitle(mmp);

    _mm_fclose(mmp);
    return(retval);
}
#endif


// =====================================================================================
    UNIMOD *Unimod_LoadFP(MDRIVER *md, MMSTREAM *modfp, MMSTREAM *smpfp, int mode)
// =====================================================================================
// Loads a module given a file pointer.  Useable for situations that involve
// packed file formats - a single file that contains all module data, or
// UniMod modules which use the sample library feature.
//
// - Songs and samples are loaded from the file positions specified.
// - The file positions will remain unchanged after the call to this procedure.
// - mode specifies the module as MM_STATIC or MM_DYNAMIC
{
    int        t;
    MLOADER   *l;
    BOOL       ok;
    UNIMOD    *mf;
    ML_HANDLE *lh;

    if((mf=_mm_calloc(1,sizeof(UNIMOD))) == NULL)
    {   Unimod_Free(mf);
        return NULL;
    }

    // Try to find a loader that recognizes the module

    for(l=firstloader; l; l=l->next)
    {   _mm_rewind(modfp);
        if(l->Test(modfp)) break;
    }

    if(l==NULL)
    {   _mmerr_set(MMERR_UNSUPPORTED_FILE, "Unknown module format or corrupted file.");
        return NULL;
    }

    mf->initvolume = 128;
    mf->memsize    = PTMEM_LAST;
    utrk_memory_reset();
    utrk_local_memflag(PTMEM_PORTAMENTO, TRUE, FALSE);

    // init panning array

#ifdef __MM_WINAMP__
    for(t=0; t<64; t++) mf->panning[t] = ((t+1)&2) ? (PAN_RIGHT-config_modpan) : (PAN_LEFT+config_modpan);
#else
    for(t=0; t<64; t++) mf->panning[t] = ((t+1)&2) ? PAN_RIGHT : PAN_LEFT;
#endif
    
    // default channel volume set to max...
    for(t=0; t<64; t++) mf->chanvol[t] = 64;

    // init module loader and load the header / patterns
    if(lh = l->Init())
    {   _mm_rewind(modfp);
        ok = l->Load(lh, mf, modfp);
    } else ok = 0;

    // free loader and unitrk allocations
    l->Cleanup(lh);
    utrk_cleanup();

    if(!ok)
    {   Unimod_Free(mf);
        return NULL;
    }

    if(smpfp)
    {   if(!loadsamples(md, mf, smpfp))
        {   Unimod_Free(mf);
            return NULL;
        }
    }

    mf->md = md;
    return mf;
}

#ifdef __MM_WINAMP__
extern void MP_QuickClean(UNIMOD *mf);    //defined in MPLAYER.C
extern int config_loopcount;


// =====================================================================================
    UNIMOD *Unimod_LoadInfo(CHAR *filename)
// =====================================================================================
// Open a module via it's filename and loads only the header information.
// Includes calcutaing the songlength, which can be a costly exercise de-
// pending on the complexity of the song.
{
    FILE   *fp;
    UNIMOD *mf;
    
    if((fp = _mm_fopen(filename,"rb"))==NULL) return NULL;
    if(mf = Unimod_LoadFP(fp, NULL))
    {   int   i;
        if(mf->loopcount = config_loopcount) mf->loop = 1; else mf->loop = 0;
        mf->extspd = 1;
        mf->patloop = (PATLOOP *)_mm_calloc(mf->numchn, sizeof(PATLOOP));
        mf->memory  = (MP_EFFMEM **)_mm_calloc(mf->numchn, sizeof(MP_EFFMEM *));
        for(i=0; i<mf->numchn; i++)
            if((mf->memory[i]  = (MP_EFFMEM *)_mm_calloc(mf->memsize, sizeof(MP_EFFMEM))) == NULL)
            {   MikMod_FreeSong(mf);  return NULL;  }
        
        MP_QuickClean(mf);
        PredictSongLength(mf);

        _mm_fseek(fp,0,SEEK_END);
        mf->filesize = _mm_ftell(fp);

    }
    fclose(fp);
    
    return mf;
}
#endif


// =====================================================================================
    UNIMOD *Unimod_Load(MDRIVER *md, const CHAR *filename)
// =====================================================================================
// Open a module via it's filename.  This is fairly automated song loader,
// which does not support fancy things like sample libraries (for shared
// samples and instruments).  See song_loadfp for those.
//
// In addition, this puppy will also load the samples automatically, and it
// sets the filesize.
{
    MMSTREAM *fp;
    UNIMOD  *mf;
    
    //_mmlog_debug("Mikmod > unimod_load > Loading Module : %s",filename);

    if((fp = _mm_fopen(filename,"rb"))==NULL) return NULL;
    if(mf = Unimod_LoadFP(md, fp, fp, MM_STATIC))
    {   if(SL_LoadSamples(md))
        {   Unimod_Free(mf);
            mf = NULL;
        } else
        {   _mm_fseek(fp,0,SEEK_END);
            mf->filesize = _mm_ftell(fp);
        }
    }

    _mm_fclose(fp);
    if(!mf) _mmlog("Unimod > Module load failed : %s",filename);
    return mf;
}


// =====================================================================================
    UNIMOD *Unimod_LoadMem(MDRIVER *md, void *module)
// =====================================================================================
// loads a song from memory instead of a filename.  pointers to both the module
// header and samples should be given.
{
    MMSTREAM *fp;
    UNIMOD  *mf;
    
    if((fp = _mmstream_createmem(module,0))==NULL) return NULL;
    if(mf = Unimod_LoadFP(md, fp, fp, MM_STATIC))
    {   if(SL_LoadSamples(md))
        {   Unimod_Free(mf);
            mf = NULL;
        } else
        {   _mm_fseek(fp,0,SEEK_END);
            mf->filesize = _mm_ftell(fp);
        }
    }

    _mm_fclose(fp);

    return mf;
}

