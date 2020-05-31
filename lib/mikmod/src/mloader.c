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
 Module:  MLOADER.C

  Frontend interface for the dozen or so mikmod loaders.

 Portability:
  All systems - all compilers

*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"


UWORD finetune[16] =
{   8363,   8413,   8463,   8529,   8581,   8651,   8723,   8757,
    7895,   7941,   7985,   8046,   8107,   8169,   8232,   8280
};


MLOADER *firstloader = NULL;

// =====================================================================================
    int MikMod_GetNumLoaders(void)
// =====================================================================================
{
    int      t;
    MLOADER *l;

    for(t=1,l=firstloader; l; l=l->next, t++);

    return t;
}


// =====================================================================================
    MLOADER *MikMod_LoaderInfo(int loader)
// =====================================================================================
{
    int      t;
    MLOADER *l;

    // list all registered module loaders:

    for(t=loader,l=firstloader; t && l; l=l->next, t--);

    return l;
}


// =====================================================================================
    void ML_RegisterLoader(MLOADER *ldr)
// =====================================================================================
{
    MLOADER *cruise = firstloader;

    if(cruise!=NULL)
    {   while(cruise->next!=NULL)  cruise = cruise->next;
        cruise->next = ldr;
    } else
        firstloader = ldr; 
}


// =====================================================================================
    CHAR *DupStr(CHAR *s, UWORD len)
// =====================================================================================
//  Creates a CSTR out of a character buffer of 'len' bytes, but strips
//  any terminating non-printing characters like 0, spaces etc.
{
    UWORD t;
    CHAR  *d = NULL;

    // Scan for first printing char in buffer [includes high ascii up to 254]
    while(len)
    {   if(s[len-1] > 0x20) break;
        len--;
    }

    // When the buffer wasn't completely empty, allocate
    // a cstring and copy the buffer into that string, except
    // for any control-chars

#ifdef __GNUC__
    if(len<16) len = 16;
#endif

    if((d=(CHAR *)_mm_malloc(len+1)) != NULL)
    {   for(t=0; t<len; t++) d[t] = (s[t]<32) ? ' ' : s[t];
        d[t] = 0;
    }

    return d;
}


// =====================================================================================
    BOOL AllocPositions(UNIMOD *of, int total)
// =====================================================================================
{
    if((of->positions = (UWORD *)_mm_calloc(total,sizeof(UWORD))) == NULL) return 0;
    return 1;
}


// =====================================================================================
    BOOL AllocPatterns(UNIMOD *of)
// =====================================================================================
{
    int t;

    // Allocate track sequencing array

    if(!(of->patterns   = (UWORD *)_mm_calloc((ULONG)(of->numpat+2)*of->numchn,sizeof(UWORD)))) return 0;
    if(!(of->pattrows   = (UWORD *)_mm_calloc(of->numpat+2,sizeof(UWORD)))) return 0;
    if(!(of->globtracks = (UBYTE **)_mm_calloc(of->numpat+2,sizeof(UBYTE *)))) return 0;

    for(t=0; t<of->numpat+1; t++) of->pattrows[t] = 64;

    return 1;
}


// =====================================================================================
    BOOL AllocTracks(UNIMOD *of)
// =====================================================================================
{
    if(!(of->tracks = (UBYTE **)_mm_calloc(of->numtrk+2,sizeof(UBYTE *)))) return 0;
    return 1;
}


// =====================================================================================
    BOOL AllocInstruments(UNIMOD *of)
// =====================================================================================
{
    int t,n;

    if((of->instruments = (INSTRUMENT *)_mm_calloc(of->numins,sizeof(INSTRUMENT)))==NULL) return 0;

    for(t=0; t<of->numins; t++)
    {  for(n=0; n<120; n++)     // Init note / sample lookup table
       {  of->instruments[t].samplenote[n]   = n;
          of->instruments[t].samplenumber[n] = t;
       }   
       of->instruments[t].globvol = 64;
    }
    return 1;
}


// =====================================================================================
    BOOL AllocSamples(UNIMOD *of, BOOL ext)
// =====================================================================================
// Allocates memory for of->numsmp number of samples.  if 'ext' is true, additional extended
// sample information is allocated.
{
    UWORD u;

    if(ext) if((of->extsamples = (EXTSAMPLE *)_mm_calloc(of->numsmp, sizeof(EXTSAMPLE)))==NULL) return 0;
    if((of->samples = (UNISAMPLE *)_mm_calloc(of->numsmp, sizeof(UNISAMPLE))) == NULL) return 0;

    for(u=0; u<of->numsmp; u++)
    {   of->samples[u].volume     = 128;
        of->samples[u].panning    = PAN_CENTER;
        of->samples[u].handle     = -1;
        if(ext) of->extsamples[u].globvol = 64;
    }
    return 1;
}


// =====================================================================================
    void pt_write_exx(unsigned int eff, unsigned int dat)
// =====================================================================================
// ** Protracker Standard Effects Processor **
// These are used by almost every loader since every module at least bears
// some similarity to PT in one effect or another.
{
    UNITRK_EFFECT  effdat;
    BOOL globeffect = 0;

    if(eff ==0) return;
    effdat.framedly = UFD_RUNONCE;

    switch(eff)
    {   case 0x1:                  // Fineslide up
            effdat.param.s  = dat << 3;
            effdat.effect   = UNI_PITCHSLIDE;
        break;

        case 0x2:                  // Fineslide down
            effdat.param.s  = 0 - (dat << 3);
            effdat.effect   = UNI_PITCHSLIDE;
        break;

        //case 0x3:                  // Glissando control (not supported).
            //effdat.param.u = dat;
            //effdat.effect  = UNI_GLISSANDO_CTRL;
        //break;

        case 0x4:                  // set vibrato waveform
            effdat.param.u  = dat;
            effdat.effect   = UNI_VIBRATO_WAVEFORM;
        break;

        case 0x5: break;           // Set finetune, unsupported?

        case 0x6:                  // Pattern loop (now global effect)
            effdat.param.u  = dat;
            effdat.effect   = dat ? UNI_GLOB_LOOP : UNI_GLOB_LOOPSET;
            globeffect      = 1;
        break;

        case 0x7:                  // Set Tremolo waveform
            effdat.param.u  = dat;
            effdat.effect   = UNI_TREMOLO_WAVEFORM;
        break;

        case 0x8:                  // channel panning (dmp?)
            effdat.param.s  = ((dat<=8) ? (dat*16) : (dat*17)) + PAN_LEFT;
            effdat.effect   = UNI_PANNING;
        break;

        case 0x9:                  // Retrigger with no volume modifier.
            effdat.param.u  = dat;
            effdat.effect   = UNI_RETRIG;
            effdat.framedly = 0;
        break;

        case 0xa:                  // fine volume slide up
            effdat.param.s  = dat*2;
            effdat.effect   = UNI_VOLSLIDE;
        break;

        case 0xb:                  // fine volume slide dn
            effdat.param.s  = 0 - (dat*2);
            effdat.effect   = UNI_VOLSLIDE;
        break;

        case 0xc:                  // Note Cut
            effdat.framedly = dat;
            effdat.effect   = UNI_VOLUME;
            effdat.param.u  = 0;
        break;

        case 0xd:                  // note delay
            effdat.param.u  = dat;
            effdat.effect   = UNI_NOTEDELAY;
            effdat.framedly = 0;
        break;

        case 0xe:                  // pattern delay
            effdat.param.hiword.u = dat;
            effdat.effect         = UNI_GLOB_DELAY;
            //effdat.framedly       = 0;
            globeffect = 1;
        break;

        default:
            effdat.effect = 0;
        break;

    }

    if(effdat.effect)
    {   if(globeffect)
            utrk_write_global(&effdat, UNIMEM_NONE);
        else
            utrk_write_local(&effdat, UNIMEM_NONE);
    }
}


// =====================================================================================
    void pt_write_effect(unsigned int eff, unsigned int dat)
// =====================================================================================
// Translates Protracker effects into Unimod effects.  Works for all standard PT effects
// 0 through f (hex) except Exx extended commands.  For those, use pt_write_exx (above).
{
    UNITRK_EFFECT  effdat;
    unsigned int hi = dat>>4, lo = dat & 15;

    effdat.framedly = 0;
    effdat.param.u  = 0;
    
    if(eff!=0 || dat!=0)                // don't write empty effect
    {   // global effects get special treatments now.
        switch(eff)
        {   case 0:                     // arpeggio!
                effdat.param.byte_a = lo;
                effdat.param.byte_b = hi;
                effdat.effect  = UNI_ARPEGGIO;
                utrk_write_local(&effdat, UNIMEM_NONE);
            break;

            case 0x1:                   // pitch slide up
                if(dat)
                {   effdat.param.s  = dat<<2;
                    effdat.effect   = UNI_PITCHSLIDE;
                    effdat.framedly = 1;
                    utrk_write_local(&effdat, PTMEM_PITCHSLIDEUP);
                } else utrk_memory_local(&effdat, PTMEM_PITCHSLIDEUP, 0);
            break;

            case 0x2:                   // pitch slide down
                if(dat)
                {   effdat.param.s  = 0 - (dat<<2);
                    effdat.effect   = UNI_PITCHSLIDE;
                    effdat.framedly = 1;
                    utrk_write_local(&effdat, PTMEM_PITCHSLIDEDN);
                } else utrk_memory_local(&effdat, PTMEM_PITCHSLIDEDN, 0);
            break;

            case 0x3:                   // Portamento to Note
                effdat.effect   = UNI_PORTAMENTO_LEGACY;
                if(dat)
                {   effdat.param.u  = dat*8;
                    utrk_write_local(&effdat, PTMEM_PORTAMENTO);
                } else utrk_memory_local(&effdat, PTMEM_PORTAMENTO, 0);
            break;

            case 0x4:                    // Vibrato
                if(lo)
                {   effdat.param.u   = lo*16;
                    effdat.effect    = UNI_VIBRATO_DEPTH;
                    utrk_write_local(&effdat, PTMEM_VIBRATO_DEPTH);
                } else utrk_memory_local(NULL, PTMEM_VIBRATO_DEPTH, 0);

                if(hi)
                {   effdat.param.u   = hi*4;
                    effdat.framedly  = 1;
                    effdat.effect    = UNI_VIBRATO_SPEED;
                    utrk_write_local(&effdat, PTMEM_VIBRATO_SPEED);
                } else utrk_memory_local(NULL, PTMEM_VIBRATO_SPEED, 0);
            break;
            
            case 0x5:                    // Portamento + Volume Slide
                // Note: Make sure we use the 'net' slide speed
                // (hi - lo) because protracker works that way.
                effdat.effect    = UNI_VOLSLIDE;
                effdat.framedly  = 1;
                effdat.param.s   = (hi - lo) << 1;
                utrk_write_local(&effdat, UNIMEM_NONE);

                effdat.effect    = UNI_PORTAMENTO_LEGACY;
                utrk_memory_local(&effdat, PTMEM_PORTAMENTO, 0);
            break;

            case 0x6:                    // Vibrato + Volume Slide
                effdat.effect    = UNI_VOLSLIDE;
                effdat.framedly  = 1;
                effdat.param.s   = (hi - lo) << 1;
                utrk_write_local(&effdat, UNIMEM_NONE);

                utrk_memory_local(NULL, PTMEM_VIBRATO_DEPTH, 0);
                utrk_memory_local(NULL, PTMEM_VIBRATO_SPEED, 0);
            break;

            case 0x7:                    // Tremolo!
                if(lo)
                {   effdat.param.u  = lo * 8;
                    effdat.effect   = UNI_TREMOLO_DEPTH;
                    utrk_write_local(&effdat, PTMEM_TREMOLO_DEPTH);
                } else utrk_memory_local(NULL, PTMEM_TREMOLO_SPEED, 0);

                if(hi)
                {   effdat.param.u  = hi * 4;
                    effdat.effect   = UNI_TREMOLO_SPEED;
                    effdat.framedly = 1;
                    utrk_write_local(&effdat, PTMEM_TREMOLO_SPEED);
                } else utrk_memory_local(NULL, PTMEM_TREMOLO_SPEED, 0);
            break;

            case 0x8:                    // OttO Panning!
                effdat.param.s  = (dat*2) + PAN_LEFT;
                effdat.param.s  = _mm_boundscheck(effdat.param.s,PAN_LEFT,PAN_RIGHT);
                effdat.effect   = UNI_PANNING;
                effdat.framedly = UFD_RUNONCE;
                utrk_write_local(&effdat, UNIMEM_NONE);
            break;

            case 0x9:                    // Sample Offset
                if(dat)
                {   effdat.effect    = UNI_OFFSET;
                    effdat.framedly  = 0;
                    effdat.param.s   = dat<<8;
                     utrk_write_local(&effdat, PTMEM_OFFSET);
                } else utrk_memory_local(&effdat, PTMEM_OFFSET, 0);
            break;

            case 0xa:                    // Volume Slide
                // Note: Make sure we use the 'net' slide speed
                // (hi - lo) because protracker works that way.
                effdat.effect    = UNI_VOLSLIDE;
                effdat.framedly  = 1;
                effdat.param.s   = (hi - lo) << 1;
                utrk_write_local(&effdat, UNIMEM_NONE);
            break;

            case 0xb:                    // Pattern jump
                effdat.param.u = dat;
                effdat.effect  = UNI_GLOB_PATJUMP;
                utrk_write_global(&effdat, UNIMEM_NONE);
            break;

            case 0xc:                     // set volume
                effdat.param.u  = (dat > 64) ? VOLUME_FULL : (dat << 1);
                effdat.effect   = UNI_VOLUME;
                effdat.framedly = UFD_RUNONCE;
                utrk_write_local(&effdat, UNIMEM_NONE);
            break;

            case 0xd:                     // Pattern break
                effdat.param.u = dat;
                effdat.effect  = UNI_GLOB_PATBREAK;
                utrk_write_global(&effdat, UNIMEM_NONE);
            break;

            case 0xe:
                pt_write_exx(hi,lo);
            break;

            case 0xf:                     // Set Speed / BPM
                if(dat)
                {   effdat.param.u  = dat;
                    effdat.framedly = UFD_RUNONCE;
                    effdat.effect   = (dat >= 0x20) ? UNI_GLOB_TEMPO : UNI_GLOB_SPEED;
                    utrk_write_global(&effdat, PTMEM_TEMPO);
                } else
                    utrk_memory_global(&effdat, PTMEM_TEMPO);
            break;
        }
    }
}


// =====================================================================================
    void pt_global_consolidate(UNIMOD *of, UBYTE **globtrack)
// =====================================================================================
// Copies the effects from the current read global track to the current
// write global track.  Used for modules that store their formats in track
// format (not pattern-format like mods/it/s3m).
{
    uint         u;
    int          t;
    UBYTE        *urow[64];

    // build the row indexes for this pattern
    for(t=0; t<of->numpat; t++)
    {   uint sigh;
        utrk_reset();
        for(u=0; u<of->numchn; u++)
            urow[u] = globtrack[of->patterns[(t*of->numchn)+u]];

        // now consolidate all global tracks into the main one.
        for(u=0; u<64; u++)
        {   for(sigh=0; sigh<of->numchn; sigh++)
                urow[sigh] = utrk_global_copy(urow[sigh], sigh);
            utrk_newline();
        }
        of->globtracks[t] = utrk_dup_global();
    }
}


