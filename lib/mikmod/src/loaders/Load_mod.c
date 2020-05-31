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
 Module: LOAD_MOD.C

  Generic MOD loader (Protracker, StarTracker, FastTracker, etc).  Personally
  my favorite loader, because it loads most of my favorite tunes! :)

 Portability:
  All systems - all compilers (hopefully)

*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"


/*************************************************************************
*************************************************************************/


typedef struct MSAMPINFO                  // sample header as it appears in a module
{   CHAR  samplename[22];
    UWORD length;
    UBYTE finetune;
    UBYTE volume;
    UWORD reppos;
    UWORD replen;
} MSAMPINFO;


typedef struct MODULEHEADER              // verbatim module header
{   CHAR       songname[20];             // the songname..
    MSAMPINFO  samples[31];              // all sampleinfo
    UBYTE      songlength;               // number of patterns used
    UBYTE      magic1;                   // should be 127
    UBYTE      positions[128];           // which pattern to play at pos
    UBYTE      magic2[4];                // string "M.K." or "FLT4" or "FLT8"
} MODULEHEADER;

#define MODULEHEADERSIZE 1084


typedef struct MODTYPE              // struct to identify type of module
{   CHAR    id[5];
    UBYTE   channels;
    CHAR    *name;
} MODTYPE;


typedef struct MODNOTE
{   UBYTE a,b,c,d;
} MODNOTE;


/*************************************************************************
*************************************************************************/


CHAR protracker[]   = "Protracker";
CHAR startrekker[]  = "Startrekker";
CHAR fasttracker[]  = "Fasttracker";
CHAR ins15tracker[] = "15-instrument";
CHAR oktalyzer[]    = "Oktalyzer";
CHAR taketracker[]  = "TakeTracker";


MODTYPE modtypes[] =
{  { "M.K.",4,protracker   },   // protracker 4 channel
   { "M!K!",4,protracker   },   // protracker 4 channel
   { "FLT4",4,startrekker  },   // startracker 4 channel
   { "2CHN",2,fasttracker  },   // fasttracker 2 channel
   { "4CHN",4,fasttracker  },   // fasttracker 4 channel
   { "6CHN",6,fasttracker  },   // fasttracker 6 channel
   { "8CHN",8,fasttracker  },   // fasttracker 8 channel
   { "10CH",10,fasttracker },   // fasttracker 10 channel
   { "12CH",12,fasttracker },   // fasttracker 12 channel
   { "14CH",14,fasttracker },   // fasttracker 14 channel
   { "16CH",16,fasttracker },   // fasttracker 16 channel
   { "18CH",18,fasttracker },   // fasttracker 18 channel
   { "20CH",20,fasttracker },   // fasttracker 20 channel
   { "22CH",22,fasttracker },   // fasttracker 22 channel
   { "24CH",24,fasttracker },   // fasttracker 24 channel
   { "26CH",26,fasttracker },   // fasttracker 26 channel
   { "28CH",28,fasttracker },   // fasttracker 28 channel
   { "30CH",30,fasttracker },   // fasttracker 30 channel
   { "CD81",8,oktalyzer    },   // atari oktalyzer 8 channel
   { "OKTA",8,oktalyzer    },   // atari oktalyzer 8 channel
   { "16CN",16,taketracker },   // taketracker 16 channel
   { "32CN",32,taketracker },   // taketracker 32 channel
   { "    ",4,ins15tracker }    // 15-instrument 4 channel
};

//static int modtype = 0;

BOOL MOD_Test(MMSTREAM *mmfile)
{
    int   modtype;
    UBYTE id[4];

    _mm_fseek(mmfile,MODULEHEADERSIZE-4,SEEK_SET);
    if(!fread(id,4,1,mmfile->fp)) return 0;

    // find out which ID string

    for(modtype=0; modtype<23; modtype++)
        if(!memcmp(id,modtypes[modtype].id,4)) return 1;

    return 0;
}


void *MOD_Init(void)
{
    return _mm_calloc(1,sizeof(MODULEHEADER));
}


void MOD_Cleanup(void *handle)
{
    _mm_free(handle, "Protracker loader handle");
}


/*
Old (amiga) noteinfo:

 _____byte 1_____   byte2_    _____byte 3_____   byte4_
/                \ /      \  /                \ /      \
0000          0000-00000000  0000          0000-00000000

Upper four    12 bits for    Lower four    Effect command.
bits of sam-  note period.   bits of sam-
ple number.                  ple number.

*/


void ConvertNote(MODNOTE *n)
{
    unsigned int instrument,effect,effdat,note;
    UWORD period;

    // extract the various information from the 4 bytes that
    // make up a single note

    instrument = (n->a&0x10)|(n->c>>4);
    period     = (((UWORD)n->a&0xf)<<8)+n->b;
    effect     = n->c&0xf;
    effdat     = n->d;

    // Convert the period to a note number

    note=0;
    if(period!=0)
    {   for(note=0; note<60; note++)
            if(period >= npertab[note]) break;
        note++;
        if(note==61) note = 0;
    }

    if(instrument!=0) utrk_write_inst(instrument);
    if(note!=0) utrk_write_note(note+24);

    // Convert pattern jump from Dec to Hex
    if(effect == 0xd)
        effdat = (((effdat&0xf0)>>4)*10)+(effdat&0xf);

    pt_write_effect(effect,effdat);
}


BOOL ML_LoadPatterns(UNIMOD *of, MMSTREAM *mmfile)
//  Loads all patterns of a modfile and converts them into the
//  3 byte format.
{
    int  t,s;
    uint Kabooopowie;
    MODNOTE  n;

    if(!AllocTracks(of))   return 0;
    if(!AllocPatterns(of)) return 0;

    for(t=0; t<of->numpat; t++)
    {   utrk_reset();
        for(s=64; s; s--)
        {   for(Kabooopowie=0; Kabooopowie<of->numchn; Kabooopowie++)
            {   n.a = _mm_read_UBYTE(mmfile);
                n.b = _mm_read_UBYTE(mmfile);
                n.c = _mm_read_UBYTE(mmfile);
                n.d = _mm_read_UBYTE(mmfile);
                utrk_settrack(Kabooopowie);
                ConvertNote(&n);
            }
            utrk_newline();
        }

        if(!utrk_dup_pattern(of)) return 0;
    }

    return 1;
}


BOOL MOD_Load(MODULEHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    int        t, modtype;
    UNISAMPLE  *q;
    MSAMPINFO  *s;           // old module sampleinfo

    // try to read module header

    _mm_read_string((CHAR *)mh->songname,20,mmfile);

    for(t=0; t<31; t++)
    {   s = &mh->samples[t];
        _mm_read_string(s->samplename,22,mmfile);
        s->length   =_mm_read_M_UWORD(mmfile);
        s->finetune =_mm_read_UBYTE(mmfile);
        s->volume   =_mm_read_UBYTE(mmfile);
        s->reppos   =_mm_read_M_UWORD(mmfile);
        s->replen   =_mm_read_M_UWORD(mmfile);
    }

    mh->songlength  =_mm_read_UBYTE(mmfile);
    mh->magic1      =_mm_read_UBYTE(mmfile);

    // Fix for FREAKY mods..  Not sure if it works for all of 'em tho.. ?
    if(mh->songlength >= 129)  mh->songlength = mh->magic1;

    _mm_read_UBYTES(mh->positions,128,mmfile);
    _mm_read_UBYTES(mh->magic2,4,mmfile);

    if(_mm_feof(mmfile))
	{   _mmlog("load_mod > Failure: Unexpected end of file reading module header");
        return 0;
    }

    // find out which ID string

    for(modtype=0; modtype<23; modtype++)
        if(!memcmp(mh->magic2,modtypes[modtype].id,4)) break;

    // set module variables

    of->initspeed = 6;
    of->inittempo = 125;
    of->numchn    = modtypes[modtype].channels;        // get number of channels
    of->modtype   = _mm_strdup(modtypes[modtype].name);// get ascii type of mod
    of->songname  = DupStr(mh->songname,20);           // make a cstr of songname
    of->numpos    = mh->songlength;                    // copy the songlength

    if(!AllocPositions(of, of->numpos)) return 0;
    for(t=0; t<of->numpos; t++)
        of->positions[t] = mh->positions[t];

    // Count the number of patterns

    of->numpat = 0;

    for(t=0; t<128; t++)
    {   if(mh->positions[t] > of->numpat)
            of->numpat = mh->positions[t];
    }
    of->numpat++;
    of->numtrk = of->numpat*of->numchn;

    // Finally, init the sampleinfo structures 
    of->numsmp = 31;

    if(!AllocSamples(of, 0))     return 0;

    s = mh->samples;       // init source pointer 
    q = of->samples;
    
    for(t=0; t<of->numsmp; t++)
    {   // convert the samplename
        q->samplename = DupStr(s->samplename, 22);

        // init the sampleinfo variables and
        // convert the size pointers to longword format

        q->speed     = finetune[s->finetune & 0xf];
        q->volume    = s->volume * 2;
        q->loopstart = (ULONG)s->reppos << 1;
        q->loopend   = q->loopstart + ((ULONG)s->replen << 1);
        q->length    = (ULONG)s->length << 1;

        q->format    = SF_SIGNED;
        if(s->replen > 1) q->flags |= SL_LOOP;

        // fix replen if repend > length
        if(q->loopend > q->length) q->loopend = q->length;

        s++;    // point to next source sampleinfo
        q++;
    }

    utrk_init(of->numchn);
    if(!ML_LoadPatterns(of, mmfile)) return 0;
    return 1;
}


#ifndef __MM_WINAMP__
CHAR *MOD_LoadTitle(MMSTREAM *mmfile)
{
   CHAR s[20];

   _mm_fseek(mmfile,0,SEEK_SET);
   if(!fread(s,20,1,mmfile->fp)) return NULL;
   
   return(DupStr(s,20));
}
#endif


MLOADER load_mod =
{   "Standard module",
    "Portable MOD loader v0.5",

    NULL,
    MOD_Test,
    MOD_Init,
    MOD_Cleanup,

    MOD_Load,
#ifndef __MM_WINAMP__
    MOD_LoadTitle
#endif
};

