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
 Module:  LOAD_ULT.C

  Ultratracker (ULT) module loader.  I used to have a couple cool .ULTs, but
  lost them some time ago.  Hope this loader still works! :)

 Portability:
  All systems - all compilers (hopefully)

 If this module is found to not be portable to any particular platform,
 please contact Jake Stine at dracoirs@epix.net (see MIKMOD.TXT for
 more information on contacting the author).

*/

#include <string.h>
#include "mikmod.h"


#define ULTS_16BITS     4
#define ULTS_LOOP       8
#define ULTS_REVERSE    16


// Raw ULT header struct:

typedef struct ULTHEADER
{   CHAR  id[16];
    CHAR  songtitle[32];
    UBYTE reserved;
} ULTHEADER;


// Raw ULT sampleinfo struct:

typedef struct ULTSAMPLE
{   CHAR   samplename[32];
    CHAR   dosname[12];
    SLONG  loopstart;
    SLONG  loopend;
    SLONG  sizestart;
    SLONG  sizeend;
    UBYTE  volume;
    UBYTE  flags;
    SWORD  finetune;
} ULTSAMPLE;


typedef struct ULTEVENT
{   UBYTE note,sample,eff,dat1,dat2;
} ULTEVENT;


CHAR *ULT_Version[]=
{   "Ultra Tracker V1.3",
    "Ultra Tracker V1.4",
    "Ultra Tracker V1.5",
    "Ultra Tracker V1.6"
};

static UBYTE **globtrack;


BOOL ULT_Test(void)
{
    CHAR id[16];

    if(!_mm_read_string(id,15,modfp)) return 0;
    return(!strncmp(id,"MAS_UTrack_V00",14));
}


BOOL ULT_Init(void)
{
    return 1;
}


void ULT_Cleanup(void)
{
    int t;
    if(globtrack)
    {   for(t=0; t<of.numtrk; t++) if(globtrack[t]) _mm_free(globtrack[t]);
        _mm_free(globtrack);
        globtrack = NULL;
    }
}

ULTEVENT ev;



int ReadUltEvent(ULTEVENT *event)
{
    UBYTE flag,rep=1;

    flag = _mm_read_UBYTE(modfp);

    if(flag==0xfc)
    {   rep = _mm_read_UBYTE(modfp);
        event->note =_mm_read_UBYTE(modfp);
    } else
        event->note = flag;

    event->sample   =_mm_read_UBYTE(modfp);
    event->eff      =_mm_read_UBYTE(modfp);
    event->dat1     =_mm_read_UBYTE(modfp);
    event->dat2     =_mm_read_UBYTE(modfp);

    return rep;
}


BOOL ULT_Load(void)
{
    int        t,u,tracks=0;
    MP_SAMPLE  *q;
    ULTSAMPLE  s;
    ULTHEADER  mh;
    UBYTE      nos,noc,nop;

    // try to read module header

    _mm_read_string(mh.id,15,modfp);
    _mm_read_string(mh.songtitle,32,modfp);
    mh.reserved = _mm_read_UBYTE(modfp);

    if(feof(modfp))
    {   _mm_errno = MMERR_LOADING_HEADER;
        return 0;
    }

    if(mh.id[14]<'1' || mh.id[14]>'4')
    {   _mm_errno = MMERR_NOT_A_MODULE;
        return 0;
    }

    of.modtype   = _mm_strdup(ULT_Version[mh.id[14]-'1']);
    of.initspeed = 6;
    of.inittempo = 125;

    // read songtext

    if(!ReadComment((UWORD)mh.reserved*32)) return 0;

    nos = _mm_read_UBYTE(modfp);

    if(feof(modfp))
    {   _mm_errno = MMERR_LOADING_HEADER;
        return 0;
    }

    of.songname = DupStr(mh.songtitle,32);
    of.numsmp   = nos;

    if(!AllocSamples(0)) return 0;

    q = of.samples;

    for(t=0; t<nos; t++)
    {   // try to read sample info

        _mm_read_string(s.samplename,32,modfp);
        _mm_read_string(s.dosname,12,modfp);
        s.loopstart     =_mm_read_I_ULONG(modfp);
        s.loopend       =_mm_read_I_ULONG(modfp);
        s.sizestart     =_mm_read_I_ULONG(modfp);
        s.sizeend       =_mm_read_I_ULONG(modfp);
        s.volume        =_mm_read_UBYTE(modfp);
        s.flags         =_mm_read_UBYTE(modfp);
        s.finetune      =_mm_read_I_SWORD(modfp);

        if(feof(modfp))
        {   _mm_errno = MMERR_LOADING_SAMPLEINFO;
            return 0;
        }

        q->samplename = DupStr(s.samplename,32);

        q->s.speed    = 8363;

        if(mh.id[14]>='4')
        {   _mm_read_I_UWORD(modfp);       // read 1.6 extra info(??) word
            q->s.speed = s.finetune;
        }

        q->s.length    = s.sizeend-s.sizestart;
        q->s.volume    = s.volume/2;
        q->s.loopstart = s.loopstart;
        q->s.loopend   = s.loopend;

        q->s.flags = SF_SIGNED;

        if(s.flags & ULTS_LOOP)
            q->s.flags |= SF_LOOP;

        if(s.flags & ULTS_16BITS)
        {   q->s.flags |= SF_16BITS;
            q->s.loopstart >>= 1;
            q->s.loopend   >>= 1;
        }

        q++;
    }

    if(!AllocPositions(256)) return 0;
    for(t=0; t<256; t++)
        of.positions[t] = _mm_read_UBYTE(modfp);
    for(t=0; t<256; t++)
        if(of.positions[t]==255) break;

    of.numpos = t;

    noc = _mm_read_UBYTE(modfp);
    nop = _mm_read_UBYTE(modfp);

    of.numchn = noc+1;
    of.numpat = nop+1;
    of.numtrk = of.numchn*of.numpat;

    if(!AllocTracks()) return 0;
    if(!AllocPatterns()) return 0;

    for(u=0; u<of.numchn; u++)
    {   for(t=0; t<of.numpat; t++)
            of.patterns[(t*of.numchn)+u] = tracks++;
    }

    // read pan position table for v1.5 and higher

    if(mh.id[14]>='3')
        for(t=0; t<of.numchn; t++) of.panning[t] = (_mm_read_UBYTE(modfp) * 32) + PAN_LEFT;

    utrk_init(1);
    if(!(globtrack = (UBYTE **)_mm_calloc(of.numtrk, sizeof(UBYTE *)))) return 0;

    for(t=0; t<of.numtrk; t++)
    {   int rep,s,done;

        utrk_reset();
        done = 0;

        while(done < 64)
        {   rep = ReadUltEvent(&ev);
            if(feof(modfp))
            {   _mm_errno = MMERR_LOADING_TRACK;
                return 0;
            }

            for(s=0; s<rep; s++)
            {   unsigned int eff;

                utrk_write_inst(ev.sample);
                if(ev.note) utrk_write_note(ev.note+24);

                eff = ev.eff>>4;

                //  ULT panning effect fixed by Alexander Kerkhove :

                if(eff==0xc) pt_write_effect(eff,ev.dat2>>2);
                else if(eff==0xb) pt_write_effect(8,ev.dat2*0xf);
                else pt_write_effect(eff,ev.dat2);

                eff = ev.eff&0xf;

                if(eff==0xc) pt_write_effect(eff,ev.dat1>>2);
                else if(eff==0xb) pt_write_effect(8,ev.dat1*0xf);
                else pt_write_effect(eff,ev.dat1);

                done++;
                utrk_newline();
            }
        }

        if(!(of.tracks[t] = utrk_dup_track(0))) return 0;
        globtrack[t] = utrk_dup_global();
    }

    // Pattern setup is read, so now process global tracks.
    pt_global_consolidate(globtrack);

    return 1;
}


#ifndef __MM_WINAMP__
CHAR *ULT_LoadTitle(void)
{
   CHAR s[32];

   _mm_fseek(modfp,15,SEEK_SET);
   if(!fread(s,32,1,modfp)) return NULL;
   
   return(DupStr(s,32));
}
#endif


MLOADER load_ult =
{   "ULT",
    "Portable ULT loader v0.1",

    NULL,
    ULT_Init,
    ULT_Test,
    ULT_Load,
    ULT_Cleanup,
#ifndef __MM_WINAMP__
    ULT_LoadTitle
#endif
};

