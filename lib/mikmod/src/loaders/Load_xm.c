/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-1999)

 Support:
  If you find problems with this code, send mail to:
    air@divent.simplenet.com

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: LOAD_XM.C

  Fasttracker (XM) module loader.  This is my second favorite loader, because
  there are so many good .XMs out there.  Plus, it's a solid disk-format, func-
  tional, and relatively easy to load and support (unlike.. Cough *IT* cough).

 Portability:
  All systems - all compilers (hopefully)
*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"


/**************************************************************************
**************************************************************************/

enum
{   FT2MEM_VOLSLIDEUP = PTMEM_LAST,
    FT2MEM_VOLSLIDEDN,          // Fine Volume slide (PT Exx effect w/ memory)
    FT2MEM_PITCHSLIDEUP,        // Fine pitch slide (PT Exx effect w/ memory)
    FT2MEM_PITCHSLIDEDN,
    FT2MEM_EFSLIDEUP,           // Extra Fine Pitch Slide memory!
    FT2MEM_EFSLIDEDN,
    FT2MEM_GLOB_VOLSLIDE,
    FT2MEM_TREMOR,
    FT2MEM_RETRIG,
    FT2MEM_VOLSLIDE,
    FT2MEM_LAST
};

typedef struct XMHEADER
{   CHAR  id[17];                   // ID text: 'Extended module: '
    CHAR  songname[21];             // Module name, padded with zeroes and 0x1a at the end
    CHAR  trackername[20];          // Tracker name
    UWORD version;                  // (word) Version number, hi-byte major and low-byte minor
    ULONG headersize;               // Header size
    UWORD songlength;               // (word) Song length (in patten order table)
    UWORD restart;                  // (word) Restart position
    UWORD numchn;                   // (word) Number of channels (2,4,6,8,10,...,32)
    UWORD numpat;                   // (word) Number of patterns (max 256)
    UWORD numins;                   // (word) Number of instruments (max 128)
    UWORD flags;                    // (word) Flags: bit 0: 0 = Amiga frequency table (see below) 1 = Linear frequency table
    UWORD tempo;                    // (word) Default tempo
    UWORD bpm;                      // (word) Default BPM
    UBYTE orders[256];              // (byte) Pattern order table 
} XMHEADER;


typedef struct XMINSTHEADER
{   ULONG size;                     // (dword) Instrument size
    CHAR  name[22];                 // (char) Instrument name
    UBYTE type;                     // (byte) Instrument type (always 0)
    UWORD numsmp;                   // (word) Number of samples in instrument
    ULONG ssize;                    //
} XMINSTHEADER;


typedef struct XMPATCHHEADER
{   UBYTE what[96];         // (byte) Sample number for all notes
    UWORD volenv[24];       // (byte) Points for volume envelope
    UWORD panenv[24];       // (byte) Points for panning envelope
    UBYTE volpts;           // (byte) Number of volume points
    UBYTE panpts;           // (byte) Number of panning points
    UBYTE volsus;           // (byte) Volume sustain point
    UBYTE volbeg;           // (byte) Volume loop start point
    UBYTE volend;           // (byte) Volume loop end point
    UBYTE pansus;           // (byte) Panning sustain point
    UBYTE panbeg;           // (byte) Panning loop start point
    UBYTE panend;           // (byte) Panning loop end point
    UBYTE volflg;           // (byte) Volume type: bit 0: On; 1: Sustain; 2: Loop
    UBYTE panflg;           // (byte) Panning type: bit 0: On; 1: Sustain; 2: Loop
    UBYTE vibflg;           // (byte) Vibrato type
    UBYTE vibsweep;         // (byte) Vibrato sweep
    UBYTE vibdepth;         // (byte) Vibrato depth
    UBYTE vibrate;          // (byte) Vibrato rate
    UWORD volfade;          // (word) Volume fadeout
    UWORD reserved[11];     // (word) Reserved
} XMPATCHHEADER;


typedef struct XMWAVHEADER
{   ULONG length;           // (dword) Sample length
    ULONG loopstart;        // (dword) Sample loop start
    ULONG looplength;       // (dword) Sample loop length
    UBYTE volume;           // (byte) Volume 
    SBYTE finetune;         // (byte) Finetune (signed byte -128..+127)
    UBYTE type;             // (byte) Type: Bit 0-1: 0 = No loop, 1 = Forward loop,
                            //                       2 = Ping-pong loop;
                            //                    4: 16-bit sampledata
    UBYTE panning;          // (byte) Panning (0-255)
    SBYTE relnote;          // (byte) Relative note number (signed byte)
    UBYTE reserved;         // (byte) Reserved
    CHAR  samplename[22];   // (char) Sample name

    UBYTE vibtype;          // (byte) Vibrato type
    UBYTE vibsweep;         // (byte) Vibrato sweep
    UBYTE vibdepth;         // (byte) Vibrato depth
    UBYTE vibrate;          // (byte) Vibrato rate
} XMWAVHEADER;


typedef struct XMPATHEADE
{   ULONG size;                     // (dword) Pattern header length 
    UBYTE packing;                  // (byte) Packing type (always 0)
    UWORD numrows;                  // (word) Number of rows in pattern (1..256)
    UWORD packsize;                 // (word) Packed patterndata size
} XMPATHEADER;

typedef struct XMNOTE
{    UBYTE note,ins,vol,eff,dat;
}XMNOTE;

/**************************************************************************
**************************************************************************/


BOOL XM_Test(MMSTREAM *mmfile)
{
    UBYTE id[17];
    
    if(!_mm_read_UBYTES(id,17,mmfile)) return 0;
    if(!memcmp(id,"Extended Module: ",17)) return 1;
    return 0;
}


void *XM_Init(void)
{
    return _mm_calloc(1,sizeof(XMHEADER));
}


void XM_Cleanup(void *handle)
{
    _mm_free(handle, "Fasttracker 2 loader handle");
}


void XM_ReadNote(MMSTREAM *mmfile)
{
    unsigned int cmp, note=0, ins=0, vol=0, eff=0, dat=0;
    unsigned int hi, lo;
    UNITRK_EFFECT effdat;

    effdat.framedly = 0;

    cmp = _mm_read_UBYTE(mmfile);

    if(cmp & 0x80)
    {   if(cmp &  1) note = _mm_read_UBYTE(mmfile);
        if(cmp &  2) ins  = _mm_read_UBYTE(mmfile);
        if(cmp &  4) vol  = _mm_read_UBYTE(mmfile);
        if(cmp &  8) eff  = _mm_read_UBYTE(mmfile);
        if(cmp & 16) dat  = _mm_read_UBYTE(mmfile);
    } else
    {   note = cmp;
        ins  = _mm_read_UBYTE(mmfile);
        vol  = _mm_read_UBYTE(mmfile);
        eff  = _mm_read_UBYTE(mmfile);
        dat  = _mm_read_UBYTE(mmfile);
    }

    if(note==97) 
    {   effdat.effect  = UNI_KEYOFF;
        effdat.param.u = 0;
        utrk_write_local(&effdat, UNIMEM_NONE);
    } else
        utrk_write_note(note);

    utrk_write_inst(ins);

    // Process volume column effects!

    hi = vol>>4;
    lo = vol & 15;

    switch(hi)
    {   case 0x6:                   // volslide down
            effdat.effect   = UNI_VOLSLIDE;
            effdat.param.s  = (0 - lo) * 2;
            effdat.framedly = 1;
            utrk_write_local(&effdat, UNIMEM_NONE);
        break;

        case 0x7:                   // volslide up
            effdat.effect   = UNI_VOLSLIDE;
            effdat.param.s  = lo * 2;
            effdat.framedly = 1;
            utrk_write_local(&effdat, UNIMEM_NONE);
        break;

        // volume-row fine volume slide is compatible with protracker
        // EBx and EAx effects i.e. a zero nibble means DO NOT SLIDE, as
        // opposed to 'take the last sliding value'.

        case 0x8:                       // finevol down
            effdat.effect   = UNI_VOLSLIDE;
            effdat.param.s  = (0 - lo) * 2;
            effdat.framedly = UFD_RUNONCE;
            utrk_write_local(&effdat, UNIMEM_NONE);
        break;

        case 0x9:                       // finevol up
            effdat.effect   = UNI_VOLSLIDE;
            effdat.param.s  = lo * 2;
            effdat.framedly = UFD_RUNONCE;
            utrk_write_local(&effdat, UNIMEM_NONE);
        break;

        case 0xa:                       // set vibrato speed
            // is this supposed to actually vibrato, using memory?
            if(lo)
            {   effdat.effect   = UNI_VIBRATO_SPEED;
                effdat.param.s  = lo*4;
                effdat.framedly = UFD_RUNONCE;
                utrk_write_local(&effdat, PTMEM_VIBRATO_SPEED);
            }
        break;

        case 0xb:                       // vibrato
            if(lo)
            {   effdat.param.u   = lo*16;
                effdat.framedly  = 0;
                effdat.effect    = UNI_VIBRATO_DEPTH;
                utrk_write_local(&effdat, PTMEM_VIBRATO_DEPTH);
            } else utrk_memory_local(NULL, PTMEM_VIBRATO_DEPTH, 0);
        break;

        case 0xc:                       // set panning
            pt_write_exx(0x8,lo);
        break;

        case 0xd:                       // panning slide left
            effdat.effect  = UNI_PANSLIDE;
            effdat.param.s = (0 - lo);
            utrk_write_local(&effdat, UNIMEM_NONE);
        break;

        case 0xe:                       // panning slide right
            effdat.effect  = UNI_PANSLIDE;
            effdat.param.s = lo;
            utrk_write_local(&effdat, UNIMEM_NONE);
        break;

        case 0xf:                       // tone porta
            effdat.param.u  = lo<<6;
            effdat.effect   = UNI_PORTAMENTO_LEGACY;
            effdat.framedly = 0;
            utrk_write_local(&effdat, PTMEM_PORTAMENTO);
        break;

        default:
            if(vol>=0x10 && vol<=0x50)
                pt_write_effect(0xc,vol-0x10);
    }

    hi = dat>>4;
    lo = dat & 15;

    switch(eff)
    {   case 0x4:                       // Effect 4: Vibrato
            // this is different from PT in that the depth nor speed are updated
            // until the second tick?  Least the speed is right... (will anyone
            // notice depth? :)
            pt_write_effect(0x4, dat);
        break;

        case 0x5:                       // Portamento + volume slide!
            if(dat)
            {   effdat.effect    = UNI_VOLSLIDE;
                effdat.framedly  = 1;
                effdat.param.s   = (hi ? hi : (0 - lo)) * 2;
                utrk_write_local(&effdat, FT2MEM_VOLSLIDE);
            } else utrk_memory_local(&effdat, FT2MEM_VOLSLIDE, 0);

            effdat.effect   = UNI_PORTAMENTO_LEGACY;
            utrk_memory_local(&effdat, PTMEM_PORTAMENTO, 0);
        break;

        case 0x6:                    // Vibrato + Volume Slide
            if(dat)
            {   effdat.effect    = UNI_VOLSLIDE;
                effdat.framedly  = 1;
                effdat.param.s   = (hi ? hi : (0 - lo)) * 2;
                utrk_write_local(&effdat, FT2MEM_VOLSLIDE);
            } else utrk_memory_local(&effdat, FT2MEM_VOLSLIDE, 0);

            utrk_memory_local(NULL, PTMEM_VIBRATO_DEPTH, 0);
            utrk_memory_local(NULL, PTMEM_VIBRATO_SPEED, 0);
        break;

        case 0xa:
            if(dat)
            {   effdat.effect    = UNI_VOLSLIDE;
                effdat.framedly  = 1;
                effdat.param.s   = (hi ? hi : (0 - lo)) * 2;
                utrk_write_local(&effdat, FT2MEM_VOLSLIDE);
            } else utrk_memory_local(&effdat, FT2MEM_VOLSLIDE, 0);
        break;

        case 0xe:
            switch(hi)
            {  case 0x1:      // XM fine porta up
                    if(lo)
                    {   effdat.effect    = UNI_PITCHSLIDE;
                        effdat.framedly  = UFD_RUNONCE;
                        effdat.param.s   = lo * 4;
                        utrk_write_local(&effdat, FT2MEM_PITCHSLIDEUP);
                    } else
                        utrk_memory_local(&effdat, FT2MEM_PITCHSLIDEUP, 0);
               break;

               case 0x2:      // XM fine porta down
                    if(lo)
                    {   effdat.effect    = UNI_PITCHSLIDE;
                        effdat.framedly  = UFD_RUNONCE;
                        effdat.param.s   = 0 - (lo * 4);
                        utrk_write_local(&effdat, FT2MEM_PITCHSLIDEDN);
                    } else utrk_memory_local(&effdat, FT2MEM_PITCHSLIDEDN, 0);
               break;

               case 0xa:      // XM fine volume up
                    if(lo)
                    {   effdat.effect    = UNI_VOLSLIDE;
                        effdat.framedly  = UFD_RUNONCE;
                        effdat.param.s   = lo * 2;
                        utrk_write_local(&effdat, FT2MEM_VOLSLIDEUP);
                    } else utrk_memory_local(&effdat, FT2MEM_VOLSLIDEUP, 0);
               break;

               case 0xb:      // XM fine volume down
                    if(lo)
                    {   effdat.effect    = UNI_VOLSLIDE;
                        effdat.framedly  = UFD_RUNONCE;
                        effdat.param.s   = (0 - lo) * 2;
                        utrk_write_local(&effdat, FT2MEM_VOLSLIDEDN);
                    } else utrk_memory_local(&effdat, FT2MEM_VOLSLIDEDN, 0);
               break;

               default:
                  pt_write_effect(0x0e,dat);
            }
        break;

        case 'G'-55:                    // G - set global volume
            effdat.effect   = UNI_GLOB_VOLUME;
            effdat.param.u  = (dat > 0x40) ? 0x80 : (dat << 1);
            effdat.framedly = UFD_RUNONCE;
            utrk_write_global(&effdat, UNIMEM_NONE);
        break;

        case 'H'-55:                    // H - global volume slide
            if(dat)
            {   effdat.effect   = UNI_GLOB_VOLSLIDE;
                effdat.param.s  = (hi ? hi : (0 - lo)) * 2;
                effdat.framedly = 1;
                utrk_write_global(&effdat, FT2MEM_GLOB_VOLSLIDE);
            } else utrk_memory_global(&effdat, FT2MEM_GLOB_VOLSLIDE);
        break;

        case 'K'-55:                    // K - keyOff and KeyFade
            effdat.effect   = UNI_KEYOFF;
            effdat.framedly = dat;
            effdat.param.u  = 0;
            utrk_write_local(&effdat, UNIMEM_NONE);
        break;

        case 'L'-55:                    // L - set envelope position
            effdat.param.hiword.u = dat;
            effdat.param.byte_b   = TRUE;
            effdat.effect         = UNI_ENVELOPE_CONTROL;
            effdat.framedly       = UFD_RUNONCE;
            utrk_write_local(&effdat, UNIMEM_NONE);
            // do exact same thing to panning envelope too ?
            effdat.param.byte_a   = 1;
            utrk_write_local(&effdat, UNIMEM_NONE);
        break;

        case 'P'-55:                    // P - panning slide
            effdat.effect  = UNI_PANSLIDE;
            effdat.param.s = hi ? hi : (0-lo);
            utrk_write_local(&effdat, UNIMEM_NONE);
        break;

        case 'R'-55:                    // R - multi retrig note
            if(dat)
            {   effdat.param.loword.u = lo;
                effdat.param.hiword.u = hi;
                effdat.effect   = UNI_RETRIG;
                effdat.framedly = 0;
                utrk_write_local(&effdat, FT2MEM_RETRIG);
            } else utrk_memory_local(&effdat, FT2MEM_RETRIG, 0);
        break;

        case 'T'-55:                    // T - Tremor !! (== S3M effect I)
            effdat.param.loword.u = lo + 1;
            effdat.param.hiword.u = hi + 1;
            effdat.effect   = UNI_TREMOR;
            effdat.framedly = 1;
            utrk_write_local(&effdat, FT2MEM_TREMOR);
        break;

        case 'X'-55:
            if((dat>>4) == 1)           // X1 - Extra Fine Porta up
            {   if(dat & 0xf)
                {   effdat.effect    = UNI_PITCHSLIDE;
                    effdat.framedly  = UFD_RUNONCE;
                    effdat.param.s   = (dat&0xf) * 2;
                    utrk_write_local(&effdat, FT2MEM_EFSLIDEUP);
                } else utrk_memory_local(&effdat, FT2MEM_EFSLIDEUP, 0);
            } else if((dat>>4) == 2)    // X2 - Extra Fine Porta down
            {   if(dat & 0xf)
                {   effdat.effect    = UNI_PITCHSLIDE;
                    effdat.framedly  = UFD_RUNONCE;
                    effdat.param.s   = 0 - (dat&0xf) * 2;
                    utrk_write_local(&effdat, FT2MEM_EFSLIDEDN);
                } else utrk_memory_local(&effdat, FT2MEM_EFSLIDEDN, 0);
            }
        break;

        default:
            if(eff <= 0xf)
            {   // Convert pattern jump from Dec to Hex
                if(eff == 0xd)
                    dat = (((dat&0xf0)>>4)*10)+(dat&0xf);
                pt_write_effect(eff,dat);
            }
        break;
    }

}


BOOL XM_Load(XMHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    INSTRUMENT  *d;
    UNISAMPLE   *q;
    EXTSAMPLE   *eq;
    XMWAVHEADER *wh, *s;
    int          t,u,p;
    long         next;
    ULONG        nextwav[256];
    BOOL         dummypat = 0;
 
    // try to read module header

    _mm_read_string(mh->id,17,mmfile);
    _mm_read_string(mh->songname,21,mmfile);
    _mm_read_string(mh->trackername,20,mmfile);
    mh->version     =_mm_read_I_UWORD(mmfile);
    mh->headersize  =_mm_read_I_ULONG(mmfile);
    mh->songlength  =_mm_read_I_UWORD(mmfile);
    mh->restart     =_mm_read_I_UWORD(mmfile);
    mh->numchn      =_mm_read_I_UWORD(mmfile);
    mh->numpat      =_mm_read_I_UWORD(mmfile);
    mh->numins      =_mm_read_I_UWORD(mmfile);
    mh->flags       =_mm_read_I_UWORD(mmfile);
    mh->tempo       =_mm_read_I_UWORD(mmfile);
    mh->bpm         =_mm_read_I_UWORD(mmfile);
    _mm_read_UBYTES(mh->orders,256,mmfile);

    if(_mm_feof(mmfile))
    {   _mmlog("load_xm > Failure: Unexpected end of file reading module header");
        return 0;
    }

    // set module variables
    of->memsize   = FT2MEM_LAST;      // Number of memory slots to reserve!
    of->initspeed = mh->tempo;
    of->inittempo = mh->bpm;
    of->modtype   = DupStr(mh->trackername,20);
    of->numchn    = mh->numchn;
    of->numpat    = mh->numpat;
    of->numtrk    = (UWORD)of->numpat*of->numchn;   // get number of channels
    of->songname  = DupStr(mh->songname,20);      // make a cstr of songname
    of->numpos    = mh->songlength;               // copy the songlength
    of->reppos    = mh->restart;
    of->numins    = mh->numins;
    of->flags |= UF_XMPERIODS | UF_INST;
    if(mh->flags&1) of->flags |= UF_LINEAR;

    if(!AllocPositions(of, of->numpos+3)) return 0;
    for(t=0; t<of->numpos; t++)
        of->positions[t] = mh->orders[t];

    // check for XM pattern discrepency:
    // If there are blank patterns at the end of the pattern data, XM doesn't count them,
    // but it WILL reference them from the order list.  So we have to check the orders
    // for any pattern references that are too high, and reference them to a legal blank
    // pattern.

    for(t=0; t<of->numpos; t++)
    {   if(of->positions[t] >= of->numpat)
        {  of->positions[t] = of->numpat;
           dummypat = 1;
        }
    }      

    if(dummypat) { of->numpat++; of->numtrk+=of->numchn; }

    if(!AllocTracks(of)) return 0;
    if(!AllocPatterns(of)) return 0;

    utrk_init(of->numchn);
    for(t=0; t<mh->numpat; t++)
    {   XMPATHEADER ph;

        ph.size     =_mm_read_I_ULONG(mmfile);
        ph.packing  =_mm_read_UBYTE(mmfile);
        ph.numrows  =_mm_read_I_UWORD(mmfile);
        ph.packsize =_mm_read_I_UWORD(mmfile);

        of->pattrows[t] = ph.numrows;

        //  Gr8.. when packsize is 0, don't try to load a pattern.. it's empty.
        //  This bug was discovered thanks to Khyron's module..

        utrk_reset();
        if(ph.packsize > 0)
        {   uint  v;
            for(u=0; u<ph.numrows; u++)
            {   for(v=0; v<of->numchn; v++)
                {   utrk_settrack(v);
                    XM_ReadNote(mmfile);
                }
                utrk_newline();
            }            
        }

        if(_mm_feof(mmfile))
	    {   _mmlog("load_xm > Failure: Unexpected end of file reading pattern %d",t);
            return 0;
        }

        utrk_dup_pattern(of);
    }

    if(dummypat)
    {   of->pattrows[t] = 64;
        utrk_reset();
        utrk_dup_pattern(of);
    }

    if(!AllocInstruments(of)) return 0;
    if((wh = (XMWAVHEADER *)_mm_calloc(256,sizeof(XMWAVHEADER))) == NULL) return 0;
    d = of->instruments;
    s = wh;

    for(t=0; t<of->numins; t++)
    {   XMINSTHEADER ih;
        int          headend;

        memset(d->samplenumber,255,120);

        // read instrument header

        headend     = _mm_ftell(mmfile);
        ih.size     = _mm_read_I_ULONG(mmfile);
        headend    += ih.size;
        _mm_read_string(ih.name, 22, mmfile);
        ih.type     = _mm_read_UBYTE(mmfile);
        ih.numsmp   = _mm_read_I_UWORD(mmfile);
        d->insname  = DupStr(ih.name,22);

        if(ih.size > 29)
        {   ih.ssize    = _mm_read_I_ULONG(mmfile);
            if(ih.numsmp > 0)
            {   XMPATCHHEADER pth;
                BOOL          inuse;        // used when checking vol/pan envelopes
    
                _mm_read_UBYTES (pth.what, 96, mmfile);
                _mm_read_I_UWORDS (pth.volenv, 24, mmfile);
                _mm_read_I_UWORDS (pth.panenv, 24, mmfile);
                pth.volpts      =  _mm_read_UBYTE(mmfile);
                pth.panpts      =  _mm_read_UBYTE(mmfile);
                pth.volsus      =  _mm_read_UBYTE(mmfile);
                pth.volbeg      =  _mm_read_UBYTE(mmfile);
                pth.volend      =  _mm_read_UBYTE(mmfile);
                pth.pansus      =  _mm_read_UBYTE(mmfile);
                pth.panbeg      =  _mm_read_UBYTE(mmfile);
                pth.panend      =  _mm_read_UBYTE(mmfile);
                pth.volflg      =  _mm_read_UBYTE(mmfile);
                pth.panflg      =  _mm_read_UBYTE(mmfile);
                pth.vibflg      =  _mm_read_UBYTE(mmfile);
                pth.vibsweep    =  _mm_read_UBYTE(mmfile);
                pth.vibdepth    =  _mm_read_UBYTE(mmfile);
                pth.vibrate     =  _mm_read_UBYTE(mmfile);
                pth.volfade     =  _mm_read_I_UWORD(mmfile);
    
                // read the remainder of the header
                for(u=headend-_mm_ftell(mmfile); u; u--)  _mm_read_UBYTE(mmfile);
    
                if(_mm_feof(mmfile))
        	    {   _mmlog("load_xm > Failure: Unexpected end of file reading instrument header %d",t);
                    return 0;
                }
    
                for(u=0; u<96; u++)         
                   d->samplenumber[u] = pth.what[u] + of->numsmp;
    
                d->volfade = pth.volfade;
    
                memcpy(d->volenv,pth.volenv,24);
                if(pth.volflg & 1)  d->volflg |= EF_ON;
                if(pth.volflg & 2)  d->volflg |= EF_SUSTAIN;
                if(pth.volflg & 4)  d->volflg |= EF_LOOP;
                d->volsusbeg = d->volsusend = pth.volsus;
                d->volbeg    = pth.volbeg;
                d->volend    = pth.volend;
                d->volpts    = pth.volpts;
    
                // scale volume envelope:
    
                inuse = 0;
                for(p=0; p<12; p++)
                {   if(d->volenv[p].val != 64) inuse = 1;
                    d->volenv[p].val <<= 2;
                }

                if(!inuse) d->volpts = 0;

                memcpy(d->panenv,pth.panenv,24);
                d->panflg    = pth.panflg;
                d->pansusbeg = d->pansusend = pth.pansus;
                d->panbeg    = pth.panbeg;
                d->panend    = pth.panend;
                d->panpts    = pth.panpts;

                // scale panning envelope:
    
                inuse = 0;
                for(p=0; p<12; p++)
                {   if(d->panenv[p].val != 0) inuse = 1;
                    d->panenv[p].val = (d->panenv[p].val-32) * 4;
                }

                if(!inuse) d->panpts = 0;

                next = 0;
    
                //  Samples are stored outside the instrument struct now, so we have
                //  to load them all into a temp area, count the of->numsmp along the
                //  way and then do an AllocSamples() and move everything over 
    
                for(u=0; u<ih.numsmp; u++,s++)
                {   s->length       =_mm_read_I_ULONG (mmfile);
                    s->loopstart    =_mm_read_I_ULONG (mmfile);
                    s->looplength   =_mm_read_I_ULONG (mmfile);
                    s->volume       =_mm_read_UBYTE (mmfile);
                    s->finetune     =_mm_read_SBYTE (mmfile);
                    s->type         =_mm_read_UBYTE (mmfile);
                    s->panning      =_mm_read_UBYTE (mmfile);
                    s->relnote      =_mm_read_SBYTE (mmfile);
                    s->vibtype      = pth.vibflg;
                    s->vibsweep     = pth.vibsweep;
                    s->vibdepth     = pth.vibdepth*4;
                    s->vibrate      = pth.vibrate;
                    s->reserved     =_mm_read_UBYTE (mmfile);

                    _mm_read_string(s->samplename, 22, mmfile);
    
                    nextwav[of->numsmp+u] = next;
                    next += s->length;
    
                    if(_mm_feof(mmfile))
                	{   _mmlog("load_s3m > Failure: Unexpected end of file loading sample header %d.",u);
                        return 0;
                    }
                }
    
                for(u=0; u<ih.numsmp; u++) nextwav[of->numsmp++] += _mm_ftell(mmfile);
                _mm_fseek(mmfile,next,SEEK_CUR);
            }
        }

        d++;
    }

    if(!AllocSamples(of, 1)) return 0;
    q  = of->samples;
    eq = of->extsamples;
    s  = wh;

    for(u=0; u<of->numsmp; u++,q++,eq++,s++)
    {   q->samplename   = DupStr(s->samplename,22);
        q->seekpos      = nextwav[u];

        q->length       = s->length;
        q->loopstart    = s->loopstart;
        q->loopend      = s->loopstart+s->looplength;
        q->volume       = s->volume*2;
        q->speed        = s->finetune+128;
        q->panning      = s->panning+PAN_LEFT;
        eq->vibtype     = s->vibtype;
        eq->vibsweep    = s->vibsweep;
        eq->vibdepth    = s->vibdepth;
        eq->vibrate     = s->vibrate;

        if(s->type & 0x10)
        {   q->length    >>= 1;
            q->loopstart >>= 1;
            q->loopend   >>= 1;
            q->format     |= SF_16BITS;
        }

        q->flags |= PSF_OWNPAN;
        if(s->type & 0x3) q->flags |= SL_LOOP;
        if(s->type & 0x2) q->flags |= SL_BIDI;

        q->format |= SF_DELTA | SF_SIGNED; 
    }

    d = of->instruments;
    s = wh;

    for(u=0; u<of->numins; u++, d++)
    {   for(t=0; t<96; t++)
            d->samplenote[t] = (d->samplenumber[t]==of->numsmp) ? 255 : (t+s[d->samplenumber[t]].relnote);
    }

    _mm_free(wh, "Fasttracker 2 temporary sample array");
    return 1;
}



#ifndef __MM_WINAMP__
CHAR *XM_LoadTitle(MMSTREAM *mmfile)
{
    CHAR s[21];

    _mm_fseek(mmfile,17,SEEK_SET);
    if(!fread(s,21,1,mmfile->fp)) return NULL;
  
    return(DupStr(s,21));
}
#endif


MLOADER load_xm =
{   "XM",
    "Portable XM loader v0.5",

    NULL,
    XM_Test,
    XM_Init,
    XM_Cleanup,

    XM_Load,
#ifndef __MM_WINAMP__
    XM_LoadTitle
#endif
};
