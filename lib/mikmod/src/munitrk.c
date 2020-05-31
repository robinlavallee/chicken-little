/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-1999)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: MUNITRK.C

  Air's new unitrk builder/player system.  I had to make it a lot faster,
  and make it more usable for the purpose of tracking.  So I made the for-
  mat less compressed, and hence less code is required to seek through the
  stream.  That made it faster.
  
Portability:
All systems - all compilers

*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"


// UniMemory Flags
//  Indicates which portions of the effect data should be flagged as volatile.
//  Volatile its of info are not saved in memory, and are replicated for each
//  'memory' effect.  This is done to allow for several effects to share the
//  same memory space.

#define UMF_PARAM       1
#define UMF_EFFECT      2
#define UMF_FRAMEDLY    4

static UBYTE lmem_flags[64]; // gmem_flags[64];


UBYTE utrk_blanktrack[2] = { 0 , 0 };

/**************************************************************************
>>>>>>>>>>> Next are the routines for PLAYING UNITRK streams: <<<<<<<<<<<<<
**************************************************************************/

void utrk_local_seek(UNITRK_ROW *urow, UBYTE *track, int row)
{
    int   i = 0;
    
    if(row < 0) return;

    while(*track && i<row)
    {   track += *track & TFLG_LENGTH_MASK;
        i++;
    }

    urow->row = track;
    urow->pos = 0;
}

void utrk_global_seek(UNITRK_ROW *urow, UBYTE *track, int row)
{
    int   i = 0;
    
    if(row < 0) return;

    while(*track && i<row)
    {   track += *track & TFLG_GLOBLEN_MASK;
        i++;
    }

    urow->row = track;
    urow->pos = 0;
}


void utrk_local_nextrow(UNITRK_ROW *urow)
{
    urow->row += (urow->row[0] & TFLG_LENGTH_MASK);
    urow->pos = 0;
}


void utrk_global_nextrow(UNITRK_ROW *urow)
{
    urow->row += (urow->row[0] & TFLG_GLOBLEN_MASK);
    urow->pos = 0;
}


void utrk_getnote(UNITRK_NOTE *note, UNITRK_ROW *urow)
{
    if(urow->row[0] & TFLG_NOTE)
    {   note->note = urow->row[1];
        note->inst = urow->row[2];
    } else note->note = note->inst = 0;
}


BOOL utrk_local_geteffect(UE_EFFECT *eff, UNITRK_ROW *urow)
// Gets the next effect from the current row.  Returns 0 if there was no
// effect to be gotten.
{
    UBYTE ch = *urow->row;

    eff->flags = 0;

    if(urow->pos == 0)
    {   if(!(ch & TFLG_EFFECT)) return 0;
        urow->pos += (ch & TFLG_NOTE) ? 3 : 1;
    }

    if(ch = urow->row[urow->pos])
    {   urow->pos++;
        if(ch & TFLG_EFFECT_MEMORY)
        {   eff->memslot = urow->row[urow->pos++];

            if(ch & TFLG_EFFECT_INFO)
            {   // Copy the new effect data to memory
                memcpy(&eff->effect, &urow->row[urow->pos], sizeof(eff->effect));
                urow->pos += sizeof(eff->effect);
            }
        } else
        {   // copy the effect data to reteff for return
            eff->memslot = 0;
            memcpy(&eff->effect, &urow->row[urow->pos], sizeof(eff->effect));
            urow->pos += sizeof(eff->effect);
        }
    } else return 0;

    return 1;
}


BOOL utrk_global_geteffect(UE_EFFECT *eff, UNITRK_ROW *urow)
// Gets the next effect from the current row.  Returns 0 if there was no
// effect to be gotten.
{
    UBYTE ch = *urow->row;

    eff->flags = UEF_GLOBAL;

    if(urow->pos == 0)
    {   if(!(ch & TFLG_EFFECT)) return 0;
        urow->pos++;
    }

    if(ch = urow->row[urow->pos])
    {   urow->pos++;
        eff->memchan = urow->row[urow->pos++];
        if(ch & TFLG_EFFECT_MEMORY)
        {   eff->memslot = urow->row[urow->pos++];

            if(ch & TFLG_EFFECT_INFO)
            {   // Copy the new effect data to memory
                memcpy(&eff->effect, &urow->row[urow->pos], sizeof(eff->effect));
                urow->pos += sizeof(eff->effect);
            }
        } else
        {   // copy the effect data to reteff for return
            eff->memslot = eff->memchan = 0;
            memcpy(&eff->effect, &urow->row[urow->pos], sizeof(eff->effect));
            urow->pos += sizeof(eff->effect);
        }
    } else return 0;

    return 1;
}

/***************************************************************************
>>>>>>>>>>> Next are the routines for CREATING UNITRK streams: <<<<<<<<<<<<<
***************************************************************************/

#define ROWSIZE    64          // max. size of the row buffer
#define BUFPAGE   128          // initial size for the track buffer
#define THRESHOLD 128          // threshold increse size for track buffer

// inuse member:
// The inuse is used to determine if a track is blank or not.  utrk_dup has
// no easy way to tell if the track has meaningful content or is simply
// filled with a series of 0's.  So, inuse is set to 1 by utrk_newline when-
// ever a row actually has any meaningful content.

typedef struct _UNI_EFFTRK
{   UBYTE    *buffer;          // unitrk buffer allocation!
    int      writepos;         // current writepos in the buffer (and length!)

    UBYTE    *output;
    int      outsize;          // current buffer size (increases as needed).
    int      outpos;

    int      note, inst;
    BOOL     inuse;            // indicates if track is in use or just blank
} UNI_EFFTRK;

typedef struct _UNI_GLOBTRK
{   UBYTE    *buffer;          // unitrk buffer allocation!
    int      writepos;         // current writepos in the buffer (and length!)

    UBYTE    *output;
    int      outsize;          // current buffer size (increases as needed).
    int      outpos;

    BOOL     inuse;            // indicates if track is in use or just blank
} UNI_GLOBTRK;

static UNI_EFFTRK   *curtrk, *unitrk;
static UNI_GLOBTRK  globtrk;
static int          curchn, unichn;


void utrk_reset(void)
// Resets index-pointers to create a new track.
{
    int  i;

    for(i=0; i<unichn; i++)
    {   unitrk[i].buffer[0] = unitrk[i].output[0] = 0; // Row Length/Flag clear
        unitrk[i].writepos  = unitrk[i].outpos    = 0; // commence writing at the first byte.
        unitrk[i].note      = unitrk[i].inst      = 0;
        unitrk[i].inuse     = 0;
    }
    globtrk.buffer[0] = globtrk.output[0] = 0;
    globtrk.writepos  = globtrk.outpos    = 0;
    globtrk.inuse     = 0;
}

void utrk_settrack(int trk)
{
    curtrk = &unitrk[trk];
    curchn = trk;
}

void utrk_write_global(UNITRK_EFFECT *data, int memslot)
// Appends an effect to the global effects column
{
    int   eflag = globtrk.writepos++;

    globtrk.buffer[eflag] = TFLG_EFFECT_INFO;
    globtrk.buffer[globtrk.writepos++] = curchn;

    if(memslot != UNIMEM_NONE)
    {   globtrk.buffer[globtrk.writepos++] = memslot;
        globtrk.buffer[eflag] |= TFLG_EFFECT_MEMORY;
    }

    memcpy(&globtrk.buffer[globtrk.writepos], data, sizeof(UNITRK_EFFECT));
    globtrk.writepos += sizeof(UNITRK_EFFECT);
}


void utrk_memory_global(UNITRK_EFFECT *data, int memslot)
// Appends an effect memory request to the global effects column.
{
    int   eflag = globtrk.writepos++;

    globtrk.buffer[eflag] = TFLG_EFFECT_MEMORY;
    globtrk.buffer[globtrk.writepos++] = curchn;
    globtrk.buffer[globtrk.writepos++] = memslot;

    /*if(gmem_flags[memslot] & UMF_EFFECT)
    {   globtrk.buffer[eflag] |= TFLG_OVERRIDE_EFFECT;
        globtrk.buffer[globtrk.writepos++] = data->effect;
    }
        
    if(gmem_flags[memslot] & UMF_FRAMEDLY)
    {   globtrk.buffer[eflag] |= TFLG_OVERRIDE_FRAMEDLY;
        globtrk.buffer[globtrk.writepos++] = data->framedly;
    }
    globtrk.buffer[eflag] |= (signs == 0) ? 0 : ((signs > 0) ? TFLG_PARAM_POSITIVE : TFLG_PARAM_NEGATIVE;
    */
}


void utrk_write_local(UNITRK_EFFECT *data, int memslot)
{
    // write effect data to current position and update

    int   eflag = curtrk->writepos++;

    curtrk->buffer[eflag] = TFLG_EFFECT_INFO;

    if(memslot != UNIMEM_NONE)
    {   curtrk->buffer[eflag] |= TFLG_EFFECT_MEMORY;
        curtrk->buffer[curtrk->writepos++] = memslot;
    }

    memcpy(&curtrk->buffer[curtrk->writepos], data, sizeof(UNITRK_EFFECT));
    curtrk->writepos += sizeof(UNITRK_EFFECT);
}


void utrk_memory_local(UNITRK_EFFECT *data, int memslot, int signs)
// Appends an effect memory request to the global effects column.
{
    int   eflag = curtrk->writepos++;

    curtrk->buffer[eflag] = TFLG_EFFECT_MEMORY;
    curtrk->buffer[curtrk->writepos++] = memslot;

    if(data)
    {   if(lmem_flags[memslot] & UMF_EFFECT)
        {   curtrk->buffer[eflag] |= TFLG_OVERRIDE_EFFECT;
            curtrk->buffer[curtrk->writepos++] = data->effect;
        }

        if(lmem_flags[memslot] & UMF_FRAMEDLY)
        {   curtrk->buffer[eflag] |= TFLG_OVERRIDE_FRAMEDLY;
            curtrk->buffer[curtrk->writepos++] = data->framedly;
        }
    }

    curtrk->buffer[eflag] |= (signs == 0) ? 0 : ((signs > 0) ? TFLG_PARAM_POSITIVE : TFLG_PARAM_NEGATIVE);
}


static BOOL buffer_threshold_check(UNI_EFFTRK *trk, int size)
{
    size += THRESHOLD;
    if((trk->outpos+size) >= trk->outsize)
    {   UBYTE *newbuf;

        // We've reached the end of the buffer, so expand
        // the buffer by BUFPAGE bytes

        newbuf = (UBYTE *)realloc(trk->output, trk->outsize+BUFPAGE+size);

        // Check if realloc succeeded

        if(newbuf)
        {   trk->output    = newbuf;
            trk->outsize  += BUFPAGE+size;
        } else return 0;
    }
    return 1;
}    

static BOOL buffer_threshold_check2(int size)
{
    size += THRESHOLD;
    if((globtrk.outpos+size) >= globtrk.outsize)
    {   UBYTE *newbuf;

        // We've reached the end of the buffer, so expand
        // the buffer by BUFPAGE bytes

        newbuf = (UBYTE *)realloc(globtrk.output, globtrk.outsize+BUFPAGE+size);

        // Check if realloc succeeded

        if(newbuf)
        {   globtrk.output    = newbuf;
            globtrk.outsize  += BUFPAGE+size;
        } else return 0;
    }
    return 1;
}


void utrk_write_inst(unsigned int ins)
// Appends UNI_INSTRUMENT opcode to the unitrk stream.
{
    curtrk->inst = ins;
}

void utrk_write_note(unsigned int note)
// Appends UNI_NOTE opcode to the unitrk stream.
{
    curtrk->note = note;
}


void utrk_newline(void)
// Closes the current row of a unitrk stream and sets pointers to
// start a new row.  Crunches the current track data into a more stream-
// like format.
{

    UNI_EFFTRK     *trk = unitrk;
    int            lfbyte, len, i;
    
    for(i=unichn; i; i--, trk++)
    {   if(!buffer_threshold_check(trk,2))
            return;
        trk->output[lfbyte = trk->outpos++] = 0;
        len = 0;
        if(trk->note || trk->inst)
        {   trk->output[lfbyte]       |= TFLG_NOTE;
            trk->output[trk->outpos++] = trk->note;
            trk->output[trk->outpos++] = trk->inst;
            len += 2;
        }

        if(trk->writepos)
        {   trk->buffer[trk->writepos++] = 0;  // effects terminator
            if(!buffer_threshold_check(trk, trk->writepos))
                return;
            memcpy(&trk->output[trk->outpos], trk->buffer, trk->writepos);
            trk->outpos += trk->writepos;
            len         += trk->writepos;
            trk->output[lfbyte] |= TFLG_EFFECT;
        }

        if(len) trk->inuse = 1;
        trk->output[lfbyte] |= len+1;

        // reset temp buffer and index
        trk->buffer[0] = 0;  trk->writepos = 0;
        trk->note = trk->inst = 0;
    }

    // Now do the global track.
    
    if(globtrk.writepos)
    {   globtrk.buffer[globtrk.writepos++] = 0;  // effects terminator
        if(!buffer_threshold_check2(globtrk.writepos)) return;
        memcpy(&globtrk.output[globtrk.outpos+1], globtrk.buffer, globtrk.writepos);
        globtrk.output[globtrk.outpos] = TFLG_EFFECT | (globtrk.writepos+1);
        globtrk.outpos += globtrk.writepos+1;
        globtrk.inuse = 1;
    } else
    {   if(!buffer_threshold_check2(2)) return;
        globtrk.output[globtrk.outpos++] = 1;
    }

    // reset temp buffer and index
    globtrk.buffer[0] = 0;
    globtrk.writepos  = 0;
}


UBYTE *utrk_dup_track(int chn)
// Dups a single track, returns NULL if allocation failed.
{
    UBYTE   *d = utrk_blanktrack;
    UNI_EFFTRK  *trk = &unitrk[chn];

    if(trk->inuse)
    {   trk->output[trk->outpos++] = 0;    // track terminator

        if((d=(UBYTE *)_mm_malloc(trk->outpos))==NULL) return NULL;
        memcpy(d,trk->output,trk->outpos);
    }

    return d;
}


static int trkwrite;
static int patwrite;

UBYTE *utrk_dup_global(void)
// Dups the global track
{
    UBYTE   *d = utrk_blanktrack;

    if(globtrk.inuse)
    {   globtrk.output[globtrk.outpos] = 0;
        if((d=(UBYTE *)_mm_malloc(globtrk.outpos))==NULL) return NULL;
        memcpy(d, globtrk.output, globtrk.outpos);
    }

    return d;
}


BOOL utrk_dup_pattern(UNIMOD *mf)
// Terminates the current array of unitrk streams and assigns it into the
// track array.
{
    UBYTE   *d;
    int     i;
    UNI_EFFTRK  *trk = unitrk;

    if(trkwrite==0)
    {   mf->tracks[0] = utrk_blanktrack;
        trkwrite++;  mf->numtrk++;
    }
    
    for(i=0; i<unichn; i++, trk++)
    {   if(trk->inuse)
        {   trk->output[trk->outpos] = 0;  // track termination character
            if((d=(UBYTE *)_mm_malloc(trk->outpos))==NULL) return 0;
            memcpy(d,trk->output,trk->outpos);
            mf->tracks[trkwrite] = d;
            mf->patterns[(patwrite * mf->numchn) + i] = trkwrite++;
        } else
        {   // blank track, so write ptr to track 0 (always blank)
            mf->patterns[(patwrite * mf->numchn) + i] = 0;
        }

    }

    mf->globtracks[patwrite++] = utrk_dup_global();

    return 1;
}


BOOL utrk_init(int nc)
{
    if(nc)
    {   int   i;
        unitrk = (UNI_EFFTRK *)calloc(sizeof(UNI_EFFTRK),nc);
        for(i=0; i<nc; i++)
        {   unitrk[i].buffer = (UBYTE *)malloc(ROWSIZE);
            unitrk[i].output = (UBYTE *)malloc(unitrk[i].outsize = BUFPAGE);
        }
    }

    globtrk.buffer = (UBYTE *)malloc(ROWSIZE*2);
    globtrk.output = (UBYTE *)malloc(globtrk.outsize = BUFPAGE);
    
    unichn   = nc;
    curtrk   = unitrk; curchn   = 0;
    trkwrite = 0;      patwrite = 0;

    return 1;
}


void utrk_memory_reset(void)
{
    // make sure all unimemory flags default to full memory.
    memset(lmem_flags,0,64);
    //memset(gmem_flags,0,64);
}

void utrk_local_memflag(int memslot, int eff, int fdly)
{
    lmem_flags[memslot] = (eff ? UMF_EFFECT : 0) | (fdly ? UMF_FRAMEDLY : 0);
}


/*void utrk_global_memflag(int memslot, int flags)
{
    gmem_flags[memslot] = flags;
}*/


void utrk_cleanup(void)
{
    int  i;

    if(unitrk!=NULL)
    {   for(i=0; i<unichn; i++)
        {   if(unitrk[i].buffer) free(unitrk[i].buffer);
            if(unitrk[i].output) free(unitrk[i].output);
            unitrk[i].buffer = NULL; unitrk[i].output = NULL;
        }
        free(unitrk);
    }

    if(globtrk.buffer) free(globtrk.buffer);
    if(globtrk.output) free(globtrk.output);
    globtrk.buffer = NULL;  globtrk.output = NULL;
}


UWORD TrkLen(UBYTE *t)
// Determines the length (in rows) of a unitrk stream 't'
{
    UWORD len = 0;
    UBYTE c;

    while(c = (*t & 0x1f))
    {   len += c;
        t   += c;
    }
    len++;

    return len;
}


BOOL MyCmp(UBYTE *a, UBYTE *b, UWORD l)
{
    int t;

    for(t=0; t<l; t++)
        if(*(a++) != *(b++)) return 0;
    return 1;
}


UBYTE *utrk_global_copy(UBYTE *track, int chn)
{
    UBYTE ch;
    int   pos = 1;

    if(track)
    {   if(track[0] & TFLG_EFFECT)
        {   curchn = chn;

            while(ch = track[pos])
            {   pos += 2;           // ignore channel byte
                if(ch & TFLG_EFFECT_MEMORY)
                {   int memslot = track[pos++];
                    if(ch & TFLG_EFFECT_INFO)
                    {   utrk_write_global((UNITRK_EFFECT *)(&track[pos]), memslot);
                        pos += sizeof(UNITRK_EFFECT);
                    } else
                        utrk_memory_global(NULL, memslot);
                } else
                {   utrk_write_global((UNITRK_EFFECT *)(&track[pos]), UNIMEM_NONE);
                    pos += sizeof(UNITRK_EFFECT);
                }
            }
        }
        track += (track[0] & TFLG_GLOBLEN_MASK);
        if(!track[0]) track = NULL;     // Set it to NULL if end-of-track.
    }

    return track;
}
