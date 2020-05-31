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
 Module: VIRTCH.C

  Sample mixing routines, using a 32 bits mixing buffer.

  Optional features include:
   (a) Interpolation of sample data during mixing
   (b) Dolby Surround Sound
   (c) Optimized assembly mixers for the Intel platform
   (d) Optional high-speed (8 bit samples) or high-quality (16 bit samples) modes
   (e) Declicking via use of micro-volume ramps.

 C Mixer Portability:
  All Systems -- All compilers.

 Assembly Mixer Portability:

  MSDOS:  BC(?)   Watcom(y)   DJGPP(y)
  Win95:  ?
  Os2:    ?
  Linux:  y

 (y) - yes
 (n) - no (not possible or not useful)
 (?) - may be possible, but not tested

*/

#include <stddef.h>
#include <string.h>
#include "mikmod.h"
#include "unisamp.h"
#include "vchcrap.h"

static long TICK, TICKLEFT, TICKREMAIN, samplesthatfit;

static VIRTCH *vc;


// =====================================================================================
    void VC_RegisterMixer(VMIXER *mixer)
// =====================================================================================
{
    VMIXER  *cruise;

    // Should I check for duplicates here? :)
    
    if(cruise = vc->mixerlist)
    {   while(cruise->next) cruise = cruise->next;
        cruise->next = mixer;
    } else
        vc->mixerlist = mixer;

    mixer->next      = NULL;
}


// =====================================================================================
    static BOOL configsample(int handle, uint mixmode)
// =====================================================================================
{
    VMIXER  *cruise, *goodmix;
    uint    goodone;
    
    // Mask out unimportant mixer settings that have no effect on mixers.

    mixmode &= DMODE_INTERP;

    // Search the list of registered sample mixers for one that matches the format
    // of the current sample.

    cruise  = vc->mixerlist;
    goodone = 1;         // set to one, so that we fail if no good matches
    goodmix = NULL;

    while(cruise)
    {   if((cruise->channels == vc->channels) && (cruise->format == (vc->sample[handle].format & (SF_16BITS | SF_STEREO))))
        {   uint   weight = 0;
            if((cruise->mixmode == mixmode) && (cruise->cpumode == vc->cpu))
            {   // found a winner.
                // check if this mixer supports declicking.  If not, turn it off, so that
                // we don't end up calling a NULL function!

                if(cruise->Init)
                {   if(cruise->Init(cruise))
                    {   _mmlog("Mikmod > Virtch > Mixer '%s' Failed Init!",cruise->name);
                        return 1;
                    }
                }

                vc->sample[handle].mixer = cruise;
                return 0;
            }

            // weigh the 'matchability' of this driver. Generally, we are 
            // looking for the fastest and/or best sounding driver. This 
            // makes the assumptions that any driver that requires special
            // CPUs is probably pretty fast.

            // only compare if the cpumode is acceptable (a match or if mixer
            // has no cpu requirements!).  

            if((vc->cpu == cruise->cpumode) || !vc->cpu)
            {   if(vc->cpu == cruise->cpumode)
                {   weight += 40;
                    if(vc->cpu) weight += 30;
                }
                if(!vc->cpu) weight += 10;
                
                // this statement compares interp only, since we mask out everything else above.
                if(mixmode == cruise->mixmode) weight += 50;
            }

            if(weight > goodone)
            {   goodone = weight;
                goodmix = cruise;
            }
        }

        cruise = cruise->next;
    }

    if(goodmix)
    {   vc->sample[handle].mixer = goodmix;
        if(goodmix->Init) goodmix->Init(goodmix);
    } else
    {   //woops, we can't mix this sample.
        _mmlog("Mikmod > virtch > Attempted to configure sample, but no suitable mixer was available!");
        vc->sample[handle].mixer = NULL;
    }

    return 0;
}

// =====================================================================================
    static int getfreehandle(void)
// =====================================================================================
{
    uint t, handle;
    
    for(handle=0; handle<vc->samplehandles; handle++)
        if(!vc->sample[handle].data) break;

    if(handle==vc->samplehandles)
    {   // Allocate More sample handles
        t = vc->samplehandles;
        vc->samplehandles += 32;
        if((vc->sample = _mm_realloc(vc->sample, sizeof(VSAMPLE) * vc->samplehandles)) == NULL) return -1;
        for(; t<vc->samplehandles; t++) vc->sample[t].data = NULL;
    }

    return handle;
}

/*static void (*calculate_volumes)(VINFO *vnf);
static void volcalc_Quad(VINFO *vnf)
{
    // this isn't complete yet.  It requires new lvoltab/rvoltab for the rear
    // speakers, and some other changes!

    vnf->vol.front.left = vnf->volume.front.left * vc->volume.front.left;
    vnf->vol.front.right = vnf->volume.front.right * vc->volume.front.right;
    vnf->vol.rear.left  = vnf->volume.rear.left  * vc->volume.rear.left;
    vnf->vol.rear.right  = vnf->volume.rear.right  * vc->volume.rear.right;

    if(vnf->panflg & VC_SURROUND)
    {   lvoltab = rvoltab = voltab[(vnf->vol.front.left+1) / BIT8_TBLSCL];
        lvolsel = rvolsel = vnf->vol.front.left / BIT16_VOLFAC;
    } else
    {   lvolsel = vnf->vol.front.left / BIT16_VOLFAC;
        rvolsel = vnf->vol.front.right / BIT16_VOLFAC;
        lvoltab = voltab[vnf->vol.front.left / BIT8_TBLSCL];
        rvoltab = voltab[vnf->vol.front.right / BIT8_TBLSCL];
        //_mmlog("  Virtch : %-3d, %-3d",vnf->vol.front.left/ BIT8_TBLSCL, vnf->vol.front.right/ BIT8_TBLSCL);
    }

    // declicker: Set us up to volramp!
    if((vc->mode & DMODE_NOCLICK) && !vnf->volramp)
    {   if((vnf->vol.front.left != vnf->oldvol.front.left) || (vnf->vol.front.right != vnf->oldvol.front.right))
        {   vnf->volramp = vc->mixspeed / 2048;
            vnf->lvolinc = ((vnf->vol.front.left - vnf->oldvol.front.left) / (int)vnf->volramp);
            vnf->rvolinc = ((vnf->vol.front.right - vnf->oldvol.front.right) / (int)vnf->volramp);
            //_mmlog("vols:  %-5d, %-5d  %-5d, %-5d   volincs: %-7d, %-7d",vnf->lvol, vnf->loldvol, vnf->rvol, vnf->roldvol, vnf->lvolinc, vnf->rvolinc);
        }
    }
}*/


// =====================================================================================
    static void Mix32To16(SWORD *dste, SLONG *srce, SLONG count)
// =====================================================================================
{
    register SLONG x1, x2;
    int           remain;


#ifdef P_MMX
    if(vc->cpu & CPU_MMX)
    {   __asm
        {
            mov         eax, count
            mov         edi, dste
            mov         ecx, eax
            mov         esi, srce
            shr         ecx, 4                // counter / 16 (ecx)
            jz          getOuttaHere

        mainLoop:

            movq        mm7, [esi]
            movq        mm6, [esi+8]
            psrad       mm7, BITSHIFT
            movq        mm5, [esi+16]
            psrad       mm6, BITSHIFT
            movq        mm4, [esi+24]
            psrad       mm5, BITSHIFT
            movq        mm3, [esi+32]
            psrad       mm4, BITSHIFT
            movq        mm2, [esi+40]
            psrad       mm3, BITSHIFT
            movq        mm1, [esi+48]
            psrad       mm2, BITSHIFT
            movq        mm0, [esi+56]
            psrad       mm1, BITSHIFT
            add         esi, 64
            psrad       mm0, BITSHIFT

            add         edi, 32
            packssdw    mm7, mm6         // left/right or backwards?
            packssdw    mm5, mm4
            movq        [edi-32], mm7
            packssdw    mm3, mm2
            movq        [edi-24], mm5
            packssdw    mm1, mm0

            movq        [edi-16], mm3
            movq        [edi-8], mm1

            dec         ecx
            jnz         mainLoop

            emms

        getOuttaHere:
            and         eax, 15
            mov         dste, edi
            mov         srce, esi
            mov         count, eax
        }
    }

#endif
    {   remain = count & 1;
    
        for(count>>=1; count; count--)
        {   x1 = *srce++ >> BITSHIFT;
            x2 = *srce++ >> BITSHIFT;

            #ifdef BOUNDS_CHECKING
            x1 = (x1 > 32767) ? 32767 : (x1 < -32768) ? -32768 : x1;
            x2 = (x2 > 32767) ? 32767 : (x2 < -32768) ? -32768 : x2;
            #endif

            *dste++ = x1;
            *dste++ = x2;
        }

        for(; remain; remain--)
        {   x1 = *srce++ >> BITSHIFT;
            #ifdef BOUNDS_CHECKING
            x1 = (x1 > 32767) ? 32767 : (x1 < -32768) ? -32768 : x1;
            #endif
            *dste++ = x1;
        }
    }
}


// =====================================================================================
    static void Mix32To8(SBYTE *dste, SLONG *srce, SLONG count)
// =====================================================================================
{
    register int   x1, x2;
    int   remain;
    
#ifdef CPU_MMX
    if(vc->cpu & CPU_MMX)
    {   static unsigned long q128[2] = { 0x80808080, 0x80808080 };
        __asm
        {
            mov         eax, count
            mov         edi, dste
            mov         ecx, eax
            mov         esi, srce
            shr         ecx, 4                // counter / 16 (ecx)
            jz          getOuttaHere

        // if the order of this code looks confusing, that is because it
        // is.  In order to do good pipelinging in MMX, I have to write
        // ass backwards code.  Live with it. :)
        
        mainLoop:
            movq        mm7, [esi]
            movq        mm6, [esi+8]
            psrad       mm7, (BITSHIFT + 8)
            movq        mm5, [esi+16]
            psrad       mm6, (BITSHIFT + 8)
            movq        mm4, [esi+24]
            psrad       mm5, (BITSHIFT + 8)
            movq        mm3, [esi+32]
            psrad       mm4, (BITSHIFT + 8)
            movq        mm2, [esi+40]
            psrad       mm3, (BITSHIFT + 8)
            movq        mm1, [esi+48]
            psrad       mm2, (BITSHIFT + 8)
            movq        mm0, [esi+56]

            psrad       mm1, (BITSHIFT + 8)
            psrad       mm0, (BITSHIFT + 8)
            add         esi, 64

            packssdw    mm7, mm6         // left/right or backwards
            add         edi, 16
            packssdw    mm5, mm4
            packssdw    mm3, mm2
            packsswb    mm7, mm5

            packssdw    mm1, mm0

            packsswb    mm3, mm1            
            paddb       mm7, q128        // convert -128..127 to 0..255

            movq        [edi-16], mm7
            paddb       mm3, q128        // convert -128..127 to 0..255

            movq        [edi-8], mm3

            dec         ecx
            jnz         mainLoop

            emms

        getOuttaHere:
            and         eax, 15
            mov         dste, edi
            mov         srce, esi
            mov         count, eax
        }
    }

#endif
    {   remain = count & 1;
    
        for(count>>=1; count; count--)
        {   x1 = *srce++ >> (BITSHIFT + 8);
            x2 = *srce++ >> (BITSHIFT + 8);

            #ifdef BOUNDS_CHECKING
            x1 = (x1 > 127) ? 127 : (x1 < -128) ? -128 : x1;
            x2 = (x2 > 127) ? 127 : (x2 < -128) ? -128 : x2;
            #endif

            *dste++ = x1 + 128;
            *dste++ = x2 + 128;
        }

        for(; remain; remain--)
        {   x1 = *srce++ >> (BITSHIFT + 8);
            #ifdef BOUNDS_CHECKING
            x1 = (x1 > 127) ? 127 : (x1 < -128) ? -128 : x1;
            #endif
            *dste++ = x1 + 128;
        }
    }
}


// =====================================================================================
    static ULONG samples2bytes(ULONG samples)
// =====================================================================================
{
    if(vc->mode & DMODE_16BITS) samples <<= 1;
    samples <<= (vc->channels / 2);
    return samples;
}


// =====================================================================================
    static ULONG bytes2samples(ULONG bytes)
// =====================================================================================
{
    if(vc->mode & DMODE_16BITS) bytes >>= 1;
    bytes >>= (vc->channels / 2);
    return bytes;
}


// =====================================================================================
    static void AddChannel(SLONG *ptr, SLONG todo, VINFO *vnf) //, int idxsize, int idxlpos, int idxlend)
// =====================================================================================
{
    SLONG      end, *current_hi;
    int        done;
    int        idxsize = vnf->size;
    VSAMPLE    *sinf = vnf->handle;

    if(!sinf->data)
    {   vnf->current = 0;
        vnf->handle  = NULL;
        vnf->samplehandle = -1;  // make sure it can't be resumed.
        return;
    }

    current_hi = _mm_HI_SLONG(vnf->current);

    // Sampledata PrePrep --
    //   Looping requires us to copy the sample data from the front of the loop to the
    //   end of it, in order to avoid the interpolation mixer from reading bad data.
    // a) save sample data into buffer
    // b) copy start of loop to the space after the loop
    // c) special case for bidi: copy the end of the loop to the space after the loop, in reverse.
        
    if(sinf->format & SF_16BITS)
    {   if(vnf->flags & SL_SUSTAIN_LOOP)
        {   memcpy(vnf->loopbuf, &sinf->data[vnf->susend*2], 16); 
            memcpy(&sinf->data[vnf->susend*2], &sinf->data[vnf->suspos*2], 16); 
        } else if(vnf->flags & SL_LOOP)
        {   memcpy(vnf->loopbuf, &sinf->data[vnf->repend*2], 16); 
            if(vnf->flags & SL_BIDI)
            {   // WHITNESS! Ugly Code!
                // I needed to copy the words in reverse order, hence this
                //  tricky little loop was concieved.
                
                int   i;
                for(i=0; i<8; i++)
                {   sinf->data[(vnf->repend*2) + i] = sinf->data[(vnf->repend*2) - i - 2];
                    sinf->data[(vnf->repend*2) + i + 1] = sinf->data[(vnf->repend*2) - i - 1];
                }
            } else
                memcpy(&sinf->data[vnf->repend*2], &sinf->data[vnf->reppos*2], 16); 
        }
    } else
    {   if(vnf->flags & SL_SUSTAIN_LOOP)
        {   memcpy(vnf->loopbuf, &sinf->data[vnf->susend], 16); 
            memcpy(&sinf->data[vnf->susend], &sinf->data[vnf->suspos], 16); 
        } else if(vnf->flags & SL_LOOP)
        {   memcpy(vnf->loopbuf, &sinf->data[vnf->repend], 16); 
            if(vnf->flags & SL_BIDI)
            {   int   i;
                for(i=0; i<16; i++)
                    sinf->data[(vnf->repend) + i] = sinf->data[(vnf->repend) - i - 1];
            } else
                memcpy(&sinf->data[vnf->repend], &sinf->data[vnf->reppos], 16); 
        }
    }

    while(todo > 0)
    {   // Callback Check.  Thanks to code below all callbacks will always end up
        // running this code when the index is exactly equal to the current!

        //if(vnf->callback.proc && (*current_hi == vnf->callback.pos))
        //    vnf->callback.proc(&sinf->data[*current_hi], vnf->callback.userinfo);

        // update the 'current' index so the sample loops, or
        // stops playing if it reached the end of the sample

        if(vnf->flags & SL_REVERSE)
        {   // The sample is playing in reverse

            if(vnf->flags & SL_SUSTAIN_LOOP)
            {   end = vnf->suspos;
                if(*current_hi <= vnf->suspos)
                {   // the sample is looping, and it has
                    // reached the loopstart index

                    if(vnf->flags & SL_SUSTAIN_BIDI)
                    {   // sample is doing bidirectional loops, so 'bounce'
                        // the current index against the idxlpos

                        vnf->flags     &= ~SL_REVERSE;
                        vnf->increment  = -vnf->increment;
                        *current_hi     = vnf->suspos + (vnf->suspos - *current_hi);
                        end             = vnf->susend;
                    } else
                    {   // normal backwards looping, so set the
                        // current position to loopend index

                        *current_hi = vnf->susend - (vnf->suspos - *current_hi);
                    }
                }
            } else if(vnf->flags & SL_LOOP)
            {   end = vnf->reppos;
                if(*current_hi <= vnf->reppos)
                {   // the sample is looping, and it has
                    // reached the loopstart index

                    if(vnf->flags & SL_BIDI)
                    {   // sample is doing bidirectional loops, so 'bounce'
                        // the current index against the idxlpos

                        vnf->flags     &= ~SL_REVERSE;
                        vnf->increment  = -vnf->increment;
                        *current_hi     = vnf->reppos + (vnf->reppos - *current_hi);
                        end             = vnf->repend;
                    } else
                        // normal backwards looping, so set the
                        // current position to loopend index

                        *current_hi = vnf->repend - (vnf->reppos - *current_hi);
                }
            } else
            {   // the sample is not looping, so check
                // if it reached index 0

                if(*current_hi <= 0)
                {   // playing index reached 0, so stop
                    // playing this sample
                    vnf->current = 0;
                    vnf->handle  = NULL;
                    vnf->samplehandle = -1;
                    break;
                }
                end = 0;
            }
        } else
        {   // The sample is playing forward

            if(vnf->flags & SL_SUSTAIN_LOOP)
            {   register int idxsend = vnf->susend;

                end  = idxsend;

                if(*current_hi >= idxsend)
                {   // the sample is looping, so check if
                    // it reached the loopend index

                    if(vnf->flags & SL_SUSTAIN_BIDI)
                    {   // sample is doing bidirectional loops, so 'bounce'
                        //  the current index against the idxlend

                        vnf->flags    |= SL_REVERSE;
                        vnf->increment = -vnf->increment;
                        *current_hi    = idxsend - (*current_hi - idxsend);
                        end = vnf->suspos;
                    } else
                    {   // normal forwards looping, so set the
                        // current position to loopstart index

                        *current_hi = vnf->suspos + (*current_hi - idxsend);
                        end         = idxsend;
                    }
                }
            } else if(vnf->flags & SL_LOOP)
            {   register int idxlend = vnf->repend;

                end = idxlend;

                if(*current_hi >= idxlend)
                {   // the sample is looping, so check if
                    // it reached the loopend index

                    if(vnf->flags & SL_BIDI)
                    {   // sample is doing bidirectional loops, so 'bounce'
                        //  the current index against the idxlend

                        vnf->flags    |= SL_REVERSE;
                        vnf->increment = -vnf->increment;
                        *current_hi    = idxlend - (*current_hi - idxlend);
                        end            = vnf->reppos+1;
                    } else
                    {   // normal forwards looping, so set the
                        // current position to loopstart index

                        *current_hi = vnf->reppos + (*current_hi - idxlend);
                    }
                }
            } else
            {   // sample is not looping, so check
                // if it reached the last position

                if(*current_hi >= idxsize)
                {   // yes, so stop playing this sample

                    vnf->current = 0;
                    vnf->handle  = NULL;
                    vnf->samplehandle = -1;
                    break;
                }
                end = idxsize;
            }
        }

        done = (((INT64S)end << FRACBITS) - vnf->current) / vnf->increment + 1;
        if(done > todo) done = todo;
        
        /*if(!done)
        {   // not enough room to mix
            vnf->current += vnf->increment;
            continue;
        }*/

        // Callback Functionality:
        // -----------------------

        /*if(vnf->callback.proc)
        {   // Callback is active, so see if we are going to cross over the position

            if(vnf->flags & SL_REVERSE)
            {   if((*current_hi > vnf->callback.pos) && ((*current_hi - done) < vnf->callback.pos))
                    done = *current_hi - vnf->callback.pos;
            } else
            {   if((*current_hi < vnf->callback.pos) && ((*current_hi + done) > vnf->callback.pos))
                    done = vnf->callback.pos - *current_hi;
            }
        }*/

        
        if(vnf->volramp)
        {   // Only mix the number of samples in volramp...
            if(done > vnf->volramp) done = vnf->volramp;

            if(vnf->panflg & VC_SURROUND)
                sinf->mixer->NoClickSurround(sinf->data,ptr,vnf->current,vnf->increment,done);
            else
                sinf->mixer->NoClick(sinf->data,ptr,vnf->current,vnf->increment,done);

            // Declicking addtions:
            // Ramp volumes and reset them if need be.

            vnf->volramp -= done;
            sinf->mixer->RampVolume(vnf, done);
            if(vnf->volramp == 0)
            {   vnf->oldvol  = vnf->vol;
            }
        } else if(vnf->vol.front.left || vnf->vol.front.right)
        {   if(vnf->panflg & VC_SURROUND)
               sinf->mixer->MixSurround(sinf->data,ptr,vnf->current,vnf->increment,done);
            else
               sinf->mixer->Mix(sinf->data,ptr,vnf->current,vnf->increment,done);
        }

        vnf->current += (vnf->increment*done);
        todo -= done;
        ptr  += done << (vc->channels / 2);
    }

    // undo our sample declicking modifications.
    if(sinf->format & SF_16BITS)
    {   if(vnf->flags & SL_SUSTAIN_LOOP) memcpy(&sinf->data[vnf->susend*2], vnf->loopbuf, 16);
        else if(vnf->flags & SL_LOOP)    memcpy(&sinf->data[vnf->repend*2], vnf->loopbuf, 16);
    } else
    {   if(vnf->flags & SL_SUSTAIN_LOOP) memcpy(&sinf->data[vnf->susend], vnf->loopbuf, 16);
        else if(vnf->flags & SL_LOOP)    memcpy(&sinf->data[vnf->repend], vnf->loopbuf, 16);
    }
}

static volatile BOOL preemption;

// =====================================================================================
    void VC_Preempt(void)
// =====================================================================================
// Preempt the mixer as soon as possible to allow the mdriver to update
// the player status and timings.
{
    preemption = 1;
}


// =====================================================================================
    void VC_VoiceSetCallback(uint voice, ulong pos, void *userinfo, void (*callback)(SBYTE *dest, void *userinfo))
// =====================================================================================
// Sets a user callback for when a voice reaches a certain sample during mixing.
// This feature allows Mikmod to attach streaming audio mixers to any voice with
// relative simplicity, and complete efficiency.
{
    vc->vinf[voice].callback.proc     = callback;
    vc->vinf[voice].callback.pos      = pos;
    vc->vinf[voice].callback.userinfo = userinfo;
}


// =====================================================================================
    void VC_WriteSamples(MDRIVER *md, SBYTE *buf, long todo)
// =====================================================================================
{
    int    left;
    uint   portion, count;
    SBYTE  *buffer;
    uint   t;


    while(todo)
    {   // Check if the mdriver has asked for a preemption into the MD_Player
        if(preemption || (TICKLEFT==0))
        {   ulong  timepass;

            preemption = 0;

            // Declicker:  Save the current state of all channels.
            // This info is used later to ramp dying channels down while new
            // kicked ones are ramped up.

            if(vc->mode & DMODE_NOCLICK)
                memcpy(vc->vold, vc->vinf, sizeof(VINFO) * vc->numchn);

            // timepass : get the amount of time that has passed since the
            // last MD_Player update (could be volatile thanks to player
            // preemption!)

            timepass = ((INT64U)(TICK - TICKLEFT) * 100000UL) / vc->mixspeed;

            // ADD ONE: Effectively causes the math to round up, instead of down,
            // which prevents us from returning, and in turn getting back, really
            // small values that stall the system!  No accuracy is lost, however,
            // since MD_Player is smart and tracks over/undertimings.

            TICK = TICKLEFT = (((INT64U)(MD_Player(md, timepass)) * vc->mixspeed) / 100000ul) + 1;
        }

        left = MIN(TICKLEFT, todo);
        
        buffer    = buf;
        TICKLEFT -= left;
        todo     -= left;

        buf += samples2bytes(left);

        while(left)
        {   VINFO *vnf;

            portion = MIN(left, samplesthatfit);
            count   = portion << (vc->channels / 2);
            
            memset(vc->TICKBUF, 0, count<<2);

            // Mix in the old channels that are being ramped to 0 volume first...

            if(vc->mode & DMODE_NOCLICK)
            {   uint   tpor, mddiv = vc->mixspeed / 2048;

                tpor = MIN(mddiv, portion);
                vnf  = vc->vold;
                for(t=0; t<vc->numchn; t++, vnf++)
                {
                    if(vc->vinf[t].kick)
                    {   // always ramp to 0 (special coded where vnf->lvol and vnf->rvol
                        // are assumed to always be 0.

                        memset(&vnf->volume,0,sizeof(MMVOLUME));                        
						if(vnf->handle) vnf->handle->mixer->CalculateVolumes(vc, vnf);
                    } else vnf->handle       = NULL;
    
                    if(vnf->handle && vnf->volramp && vnf->increment)
                    {   AddChannel(vc->TICKBUF, tpor, vnf);
                    }
                }
            }

            // Now mix in the real deal...

            vnf = vc->vinf;
            for(t=0; t<vc->numchn; t++, vnf++)
            {   
                if(vnf->kick)
                {   vnf->current = (INT64S)(vnf->start) << FRACBITS;
                    vnf->kick    = 0;
                    vnf->volramp = 0;
                    memset(&vnf->oldvol,0,sizeof(MMVOLUME));
                }

                if(vnf->frq && vnf->handle)
                {   vnf->increment = ((INT64S)(vnf->frq) << FRACBITS) / (INT64S)vc->mixspeed;
                    //_mmlog("SetIncrement freq %d  hi %d  lo %d", vnf->frq, *_mm_HI_SLONG(vnf->increment), *_mm_LO_ULONG(vnf->increment)),
                    vnf->increment -= 5;
                    if(vnf->flags & SL_REVERSE) vnf->increment =- vnf->increment;

                    vnf->handle->mixer->CalculateVolumes(vc, vnf);

                    AddChannel(vc->TICKBUF, portion, vnf);
                }
            }
                

            if(vc->mode & DMODE_16BITS)
                Mix32To16((SWORD *) buffer, vc->TICKBUF, count);
            else
                Mix32To8((SBYTE *) buffer, vc->TICKBUF, count);
            buffer += samples2bytes(portion);
            left   -= portion;
        }
    }
}


// =====================================================================================
void VC_SilenceBytes(SBYTE *buf, long todo)
// =====================================================================================
//  Fill the buffer with 'todo' bytes of silence (it depends on the mixing
//  mode how the buffer is filled)
{
    // clear the buffer to zero (16 bits
    // signed ) or 0x80 (8 bits unsigned)

    if(vc->mode & DMODE_16BITS)
        memset(buf,0,todo);
    else
        memset(buf,0x80,todo);
}


// =====================================================================================
    ULONG VC_WriteBytes(MDRIVER *md, SBYTE *buf, long todo)
// =====================================================================================
//  Writes 'todo' mixed SBYTES (!!) to 'buf'. It returns the number of
//  SBYTES actually written to 'buf' (which is rounded to number of samples
//  that fit into 'todo' bytes).
{
    if(vc->numchn == 0)
    {   VC_SilenceBytes(buf,todo);
        return todo;
    }

    todo = bytes2samples(todo);
    VC_WriteSamples(md, buf,todo);

    return samples2bytes(todo);
}


// =====================================================================================
    BOOL VC_SetMode(uint mixspeed, uint mode, uint channels, uint cpumode)
// =====================================================================================
{
    if(!vc) return 0;      // driver not initialized, so do nothing.

    if(mixspeed) vc->mixspeed = mixspeed;
    vc->channels = channels;
    vc->cpu      = cpumode;

    // reformat all loaded sample callback functions (based on interpolation,
    // cpumode, channels, etc chnges).

    if(vc->mode != mode)
    {   uint    t;
        vc->mode = mode;
        for(t=0; t<vc->samplehandles; t++) configsample(t, vc->mode);
    }

    // Recalculate constants based on new values

    samplesthatfit = TICKLSIZE >> (vc->channels / 2);

    return 0;
}

// =====================================================================================
    void VC_GetMode(uint *mixspeed, uint *mode, uint *channels, uint *cpumode)
// =====================================================================================
{
    *mixspeed = vc->mixspeed;  *mode    = vc->mode;
    *channels = vc->channels;  *cpumode = vc->cpu;
}


// =====================================================================================
    void VC_SetVolume(const MMVOLUME *volume)
// =====================================================================================
{
    vc->volume = *volume;
}

// =====================================================================================
    void VC_GetVolume(MMVOLUME *volume)
// =====================================================================================
{
    *volume = vc->volume;
}

// =====================================================================================
    BOOL VC_Init(void)
// =====================================================================================
{
    _mmlog("Mikmod > virtch > Entering Initialization Sequence.");
    
    vc = (VIRTCH *)calloc(1, sizeof(VIRTCH));
    vc->volume.front.left  = 
    vc->volume.front.right = 
    vc->volume.rear.left   = 
    vc->volume.rear.right  = 128;
    
    if((vc->sample = (VSAMPLE *)_mm_calloc(vc->samplehandles=64, sizeof(VSAMPLE))) == NULL) return 0;

    if(!vc->TICKBUF)
        if((vc->TICKBUF=(SLONG *)_mm_malloc((TICKLSIZE+32) * sizeof(SLONG))) == NULL) return 0;

    TICKLEFT = TICKREMAIN = 0;
    vc->numchn = 0;

    _mmlog("Mikmod > virtch > Initialization successful!");

#ifndef VC_NO_MIXERS

#ifdef CPU_INTEL
    VC_RegisterMixer(&ASM_M8_MONO_INTERP);
    VC_RegisterMixer(&ASM_M16_MONO_INTERP);
    VC_RegisterMixer(&ASM_M8_STEREO_INTERP);
    VC_RegisterMixer(&ASM_M16_STEREO_INTERP);

    VC_RegisterMixer(&ASM_S8_MONO_INTERP);
    VC_RegisterMixer(&ASM_S16_MONO_INTERP);
    VC_RegisterMixer(&ASM_S8_STEREO_INTERP);
    VC_RegisterMixer(&ASM_S16_STEREO_INTERP);
#else
    VC_RegisterMixer(&M8_MONO_INTERP);
    VC_RegisterMixer(&M16_MONO_INTERP);
    VC_RegisterMixer(&M8_STEREO_INTERP);
    VC_RegisterMixer(&M16_STEREO_INTERP);

    VC_RegisterMixer(&S8_MONO_INTERP);
    VC_RegisterMixer(&S16_MONO_INTERP);
    VC_RegisterMixer(&S8_STEREO_INTERP);
    VC_RegisterMixer(&S16_STEREO_INTERP);
#endif

#endif VC_NO_MIXERS

    return 0;
}

// =====================================================================================
void VC_Exit(void)
// =====================================================================================
{
    if(!vc) return;

    _mmlog("Virtch Exit Sequence (deallocation) ...");

    _mm_free(vc->TICKBUF, "tick (mix) buffer");
    _mm_free(vc->vinf, "Voice control array");
    _mm_free(vc->vold, "Voice declicker array");

    if(vc->sample)
    {   uint t;

        _mmlogd(" > Unloading all samples...");
        for(t=0; t<vc->samplehandles; t++)
            VC_SampleUnload(t);
        _mm_free(vc->sample, NULL);
    }

    _mm_free(vc, "Done! (handle unloaded)");
}


// =====================================================================================
    BOOL VC_SetSoftVoices(uint voices)
// =====================================================================================
{
    uint t;

    if(vc->numchn == voices) return 0;
    
    if(!voices)
    {   vc->numchn = 0;
        _mm_free(vc->vinf, "Voice control array (SetSoftVoices)");
        _mm_free(vc->vold, "Voice declicker array (SetSoftVoices)");
        return 0;
    }

    // reallocate the stuff to the new values.
    
    if((vc->vinf = (VINFO *)realloc(vc->vinf, sizeof(VINFO) * voices)) == NULL) return 1;
    if(vc->mode & DMODE_NOCLICK)
        if((vc->vold = (VINFO *)realloc(vc->vold, sizeof(VINFO) * voices)) == NULL) return 1;
    
    for(t=vc->numchn; t<voices; t++)
    {   vc->vinf[t].frq    = 10000;
        vc->vinf[t].panflg = 0;
        vc->vinf[t].kick   = 0;    
        vc->vinf[t].handle = NULL;
        vc->vinf[t].samplehandle = -1;

        memset(&vc->vinf[t].volume,0,sizeof(MMVOLUME));

        if(vc->mode & DMODE_NOCLICK)
        {   vc->vold[t].kick   = 0;
            vc->vold[t].handle = NULL;
        }
    }

    vc->numchn = voices;

    return 0;
}


// =====================================================================================
    void VC_VoiceSetVolume(uint voice, const MMVOLUME *volume)
// =====================================================================================
{
    vc->vinf[voice].volume = *volume;
}

// =====================================================================================
    void VC_VoiceGetVolume(uint voice, MMVOLUME *volume)
// =====================================================================================
{
    *volume = vc->vinf[voice].volume;
}


// =====================================================================================
    void VC_VoiceSetFrequency(uint voice, ulong frq)
// =====================================================================================
{
    vc->vinf[voice].frq = frq;
}


// =====================================================================================
    ulong VC_VoiceGetFrequency(uint voice)
// =====================================================================================
{
    return vc->vinf[voice].frq;
}


void VC_VoicePlay(uint voice, uint handle, uint start, uint size, int reppos, int repend, int suspos, int susend, uint flags)
{
    if((handle < vc->samplehandles) && vc->sample[handle].data)
    {   vc->vinf[voice].samplehandle = handle;
        vc->vinf[voice].handle = &vc->sample[handle];
        vc->vinf[voice].flags  = vc->sample[handle].flags = flags;
        vc->vinf[voice].start  = start;
        vc->vinf[voice].size   = size;
        vc->vinf[voice].reppos = reppos;
        vc->vinf[voice].repend = repend;
        vc->vinf[voice].suspos = suspos;
        vc->vinf[voice].susend = susend;
		//zero 8.26.00 
		//i dont see why these would hurt anything. they make sense, if you ask me,
		//and fix some bugs
		vc->vinf[voice].volramp = 0;
		vc->vinf[voice].current = 0;
		vc->vinf[voice].increment = 0;
        vc->vinf[voice].kick   = 1;
		//--
    } else
        vc->vinf[voice].handle = NULL;

}


// =====================================================================================
    void VC_VoiceResume(uint voice)
// =====================================================================================
{
    if((vc->vinf[voice].samplehandle >= 0) && vc->sample[vc->vinf[voice].samplehandle].data)
        vc->vinf[voice].handle = &vc->sample[vc->vinf[voice].samplehandle];
}


// =====================================================================================
    void VC_VoiceStop(uint voice)
// =====================================================================================
{
    vc->vinf[voice].handle       = NULL;
    //vc->vinf[voice].samplehandle = -1;
    //vc->vinf[voice].volramp=0;
	//vc->vinf[voice].current=0;
}


// =====================================================================================
    BOOL VC_VoiceStopped(uint voice)
// =====================================================================================
{
    return(vc->vinf[voice].handle == NULL);
}


// =====================================================================================
    void VC_VoiceReleaseSustain(uint voice)
// =====================================================================================
{
    // Release the sustain loop, and make sure the playing continues in
    // the 'right' direction. (matching the direction of the sample)

    if(vc->vinf[voice].handle && (vc->vinf[voice].flags & SL_SUSTAIN_LOOP))
    {   vc->vinf[voice].flags &= ~SL_SUSTAIN_LOOP;
        if(vc->vinf[voice].handle->flags & SL_REVERSE)
            vc->vinf[voice].flags |= SL_REVERSE;
        else
            vc->vinf[voice].flags &= ~SL_REVERSE;
    }
}


// =====================================================================================
    ulong VC_VoiceGetPosition(uint voice)
// =====================================================================================
{
    return(vc->vinf[voice].current >> FRACBITS);
}


// =====================================================================================
    void VC_VoiceSetPosition(uint voice, ulong pos)
// =====================================================================================
{
    vc->vinf[voice].current = (INT64S)pos << FRACBITS;
}


// =====================================================================================
    void VC_VoiceSetSurround(uint voice, int flags)
// =====================================================================================
// This is the best way I can think to implement surround sound.  This is
// surely better than my old method of using a special 'illegal' panning value.
{
    if(vc->mode & DMODE_SURROUND) return;

    if(flags & MD_ENABLE_SURROUND)
    {   // reconfigure the voice to 'surround' mode.
        vc->vinf[voice].panflg |= VC_SURROUND;
    } else
    {   // reconfigure the voice to normal mode.
        vc->vinf[voice].panflg &= VC_SURROUND;
    }
}


/**************************************************
***************************************************
***************************************************
**************************************************/

// =====================================================================================
    int VC_SampleAlloc(uint length, uint *flags)
// =====================================================================================
// Allocates memory for a sample of the given size.
// flags is a pointer because it is potentially modified during the allocation
// process to reflect the actual format of the buffer in the driver.
// Returns the handle, or -1 if failed!

{
    int    handle;
    uint   t;

    if((handle = getfreehandle()) == -1) return -1;

    // everything is always signed in our mixer!
    *flags |= SF_SIGNED;
    if(*flags & SF_STEREO) length *= 2;

    if(!(vc->mode & DMODE_SAMPLE_8BIT) && (*flags & SF_16BITS))
    {   SWORD  *samp16;

        *flags |= SF_16BITS;

        if((vc->sample[handle].data = _mm_malloc((length+12)<<1))==NULL) return -1;

        samp16 = (SWORD *)vc->sample[handle].data;
        for(t=0; t<8; t++) samp16[t+length] = 0;
    } else
    {   SBYTE  *samp8;
        *flags &= ~SF_16BITS;
        if((vc->sample[handle].data = _mm_malloc(length+12))==NULL) return -1;

        samp8 = vc->sample[handle].data;
        for(t=0; t<8; t++) samp8[t+length] = 0;
    }

    vc->sample[handle].format = *flags;
    if(configsample(handle, vc->mode)) return -1;

    return handle;
}


// =====================================================================================
    void *VC_SampleGetPtr(uint handle)
// =====================================================================================
{
    return vc->sample[handle].data;
}

// =====================================================================================
    void VC_SampleUnload(uint handle)
// =====================================================================================
{
    if(handle < vc->samplehandles)
    {   
        _mm_free(vc->sample[handle].data, NULL);

        if(vc->sample[handle].mixer)
        {   if(vc->sample[handle].mixer->Exit) vc->sample[handle].mixer->Exit(vc->sample[handle].mixer);
            vc->sample[handle].mixer = NULL;
        }
    }
}


// =====================================================================================
    int VC_SampleLoad(SAMPLOAD *sload, int type)
// =====================================================================================
// the driver-optimized 'efficient sample loader.'
// returns a driver sample handle, or -1 if it failed!
// Notes:
//   type is provided for API compliance only, as virtch loads both static
//   and dynamic samples.
{
    int    handle;
    uint   length;
    uint   t;

    if((handle = getfreehandle()) == -1) return -1;

    SL_SampleSigned(sload);

    length = sload->length;
    if(sload->outfmt & SF_STEREO) length *= 2;

    if(!(vc->mode & DMODE_SAMPLE_8BIT) && (sload->outfmt & SF_16BITS))
    {   SWORD  *samp16;

        SL_Sample8to16(sload);
        if((vc->sample[handle].data = _mm_malloc((length+16)<<1))==NULL) return -1;
        SL_Load(samp16 = (SWORD *)vc->sample[handle].data,sload,sload->length);

        for(t=0; t<8; t++) samp16[t+length] = 0;
    } else
    {   SBYTE  *samp8;
        SL_Sample16to8(sload);
        if((vc->sample[handle].data = _mm_malloc(length+16))==NULL) return -1;
        SL_Load(samp8 = vc->sample[handle].data,sload,sload->length);

        for(t=0; t<8; t++) samp8[t+length] = 0;
    }

    vc->sample[handle].format = sload->outfmt;
    if(configsample(handle, vc->mode)) return -1;

    return handle;
}


// =====================================================================================
    ULONG VC_SampleSpace(int type)
// =====================================================================================
{
    return vc->memory;
}


// =====================================================================================
    ULONG VC_SampleLength(int type, SAMPLOAD *s)
// =====================================================================================
{
    return (s->length * ((s->infmt & SF_16BITS) ? 2 : 1)) + 16;
}


/**************************************************
***************************************************
***************************************************
**************************************************/

// =====================================================================================
    ULONG VC_VoiceRealVolume(uint voice)
// =====================================================================================
{
/*    ULONG i,s,size;
    int k,j;
#ifdef __FASTMIXER__
    SBYTE *smp;
#else
    SWORD *smp;
#endif
    SLONG t;
                    
    t = vc->vinf[voice].current>>FRACBITS;
    if(vc->vinf[voice].active==0) return 0;

    s    = vc->vinf[voice].handle;
    size = vc->vinf[voice].size;

    i=64; t-=64; k=0; j=0;
    if(i>size) i = size;
    if(t<0) t = 0;
    if(t+i > size) t = size-i;

    i &= ~1;  // make sure it's EVEN.

    smp = &Samples[s][t];
    for(; i; i--, smp++)
    {   if(k<*smp) k = *smp;
        if(j>*smp) j = *smp;
    }

#ifdef __FASTMIXER__
    k = abs(k-j)<<8;
#else
    k = abs(k-j);
#endif

    return k;
    */
    return 0;
}

#ifdef __MM_WINAMP__

// -----------------------------------------
// For Seamless skip forward/backard Stuff.


typedef struct VCBOOM
{   UBYTE   channel;
    VINFO   vc->vinf;
} VCBOOM;

typedef struct VCSTATE
{   UBYTE   numchn;
    VCBOOM  *boom;
} VCSTATE;

void VC_SaveState(PMSTATE *pmstate)
{
    int   i, total;
    VCSTATE *vcs;
    VCBOOM  *boom;
    
    for(i=0, total=0; i<vc->numchn; i++)
        if(vc->vinf[i].active && vc->vinf[i].vol) total++;

    if(total == 0)
    {   pmstate->virtch = NULL;
        return;
    }

    vcs  = _mm_malloc(sizeof(VCSTATE));

    vcs->numchn = total;
    boom = vcs->boom = _mm_malloc(sizeof(VCBOOM) * total);

    for(i=0; i<vc->numchn; i++)
    {   if(vc->vinf[i].active && vc->vinf[i].vol)
        {   boom->channel = i;
            memcpy(&boom->vc->vinf,&vc->vinf[i],sizeof(VINFO));
            boom++;
        }
    }

    pmstate->virtch = vcs;
}

void VC_FreeState(PMSTATE *pmstate)
{
    VCSTATE *vcs;

    if(pmstate->virtch == NULL) return;
    vcs = (VCSTATE *)pmstate->virtch;
    _mm_free(vcs->boom);
    _mm_free(pmstate->virtch);
}

#endif

int VC_GetActiveVoices(void)
{
    uint vcnt=0, i=0;
    
    for(; i<vc->numchn; i++)
    {   if(vc->vinf[i].handle && (vc->vinf[i].vol.front.left || vc->vinf[i].vol.front.right)) vcnt++;
    }

    return vcnt;
}

// =========================
// Per-Sample Interpolation
//
// Ever wanted that magical ability to enable or disable interpolation on a
// per-sample basis?  Don't deny it!  Sure you have!  And here is your big
// chance:  Virtch supports it!

void VC_EnableInterpolation(int handle)
{
    if(handle != -1) configsample(handle, vc->mode | DMODE_INTERP);
}

void VC_DisableInterpolation(int handle)
{
    if(handle != -1) configsample(handle, vc->mode & ~DMODE_INTERP);
}


