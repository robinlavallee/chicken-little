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
 Module: sloader.c

  Routines for loading samples.  The sample loader utilizes the routines
  provided by the "registered" sample loader.  See SAMPLELOADER in
  MIKMOD.H for the sample loader structure.

 Portability:
  All systems - all compilers

*/

#include "mikmod.h"
#include <string.h>

static int       sl_rlength;
static SWORD     sl_old;
static SWORD    *sl_buffer = NULL;
static UBYTE    *sl_compress = NULL;
static SAMPLOAD *staticlist = NULL;


// Courtesy of Justin Frankel!
//
// ImpulseTrcker 8 and 16 bit compressed sample loaders.
// 100% tested and proven!
// These routines recieve src and dest buffers, the dest which must be no more
// and no less than 64K in size (that means equal folks).  The cbcount is the
// number of bytes to be read (never > than 32k).

void Decompress8Bit(UBYTE *src, SWORD *dest, int cbcount1)
{
    unsigned int ebx = 0x00000900;
    unsigned int ecx = 0;
    unsigned int edx = 0;
    unsigned int eax = 0;

D_Decompress8BitData1:
    eax   = (eax&~0xffff) | (*((short *)src));
    eax >>= ((ecx>>8) & 0xff);
    {   unsigned char ch = (ecx>>8) & 0xff, dl;

        ch  += (ebx&0xff00)>>8;
        dl   = ch>>3;
        edx  = (edx&~0xff) | dl;
        ch  &= 7;
        ecx  = (ecx&~0xff00) | (ch<<8);
        src += edx;
    }
    if (((ebx&0xff00)>>8) > 0x06) goto D_Decompress8BitA;
    eax <<= ecx&0xff;
    if ((eax&0xff) == 0x80) goto D_Decompress8BitDepthChange1;

D_Decompress8BitWriteData2:
    {   signed char c = (eax & 0xff);

        c >>= (ecx&0xff);
        eax = (eax&~0xff) | c;
    }
D_Decompress8BitWriteData:
    {   unsigned char c = (ebx & 0xff);

        c  += (eax&0xff);
        ebx = (ebx&~0xff) | c;
    }
    *dest++ = ((ebx<<8) & 0xffff);
        
    if (--cbcount1) goto D_Decompress8BitData1;
    return;

D_Decompress8BitDepthChange1:
    eax=(eax&~0xff)|((eax>>8)&0x7);
    {   unsigned char ch=(ecx&0xff00)>>8;
        ch  += 3;
        edx  = (edx&~0xff)|(ch>>3);
        ch  &= 7;
        ecx  = (ecx&~0xff00)|(ch<<8);
    }
    src += edx;
    goto D_Decompress8BitD;

D_Decompress8BitA:
    if (((ebx&0xFF00)>>8) >  0x8) goto D_Decompress8BitC;
    if (((ebx&0xFF00)>>8) == 0x8) goto D_Decompress8BitB;

    {
        unsigned char al=(eax&0xff);
        al <<= 1;
        eax  = (eax&~0xff) | al;
        if (al < 0x78) goto D_Decompress8BitWriteData2;
        if (al > 0x86) goto D_Decompress8BitWriteData2;
        al >>= 1;
        al  -= 0x3c;
        eax  = (eax&~0xff)|al;
    }
    goto D_Decompress8BitD;

D_Decompress8BitB:
    if ((eax & 0xff)  < 0x7C) goto D_Decompress8BitWriteData;
    if ((eax & 0xff)  > 0x83) goto D_Decompress8BitWriteData;

    {   unsigned char al = (eax&0xff);
        al -= 0x7c;
        eax=(eax&~0xff) | al;
    }

D_Decompress8BitD:
        ecx = (ecx&~0xff)|0x8;
        {   unsigned short int ax=eax&0xffff;
            ax++;
            eax = (eax&~0xffff) | ax;
        }

        if (((ebx&0xff00)>>8) <= (eax&0xff))
        {   unsigned char al=(eax&0xff);
            al -= 0xff;
            eax = (eax&~0xff)|al;
        }

        ebx = (ebx&~0xff00) | ((eax&0xff)<<8);

        {   unsigned char cl = (ecx&0xff);
            unsigned char al = (eax&0xff);
            cl -= al;
            if ((eax&0xff) > (ecx&0xff)) cl++;
            ecx = (ecx&~0xff) | cl;             
        }
        goto D_Decompress8BitData1;
D_Decompress8BitC:
        eax &= 0x1ff;
        if (!(eax&0x100)) goto D_Decompress8BitWriteData;

    goto D_Decompress8BitD;
}

void Decompress16Bit(UBYTE *src, SWORD *dest, int cbcount1)
{
        unsigned int ecx,edx,ebx,eax;
        unsigned int ecx_save,edx_save;
        // esi=src,edi=dest, ebp=cbcount1
        ecx = 0x1100;
        edx = ebx = eax = 0;
D_Decompress16BitData1:
        ecx_save = ecx;
        eax   = *((int *)src);
        ecx   = (ecx&~0xff) | (edx&0xff);
        eax >>= edx & 0xff;
        {   unsigned char c=edx&0xff;
            c   += (ecx&0xff00)>>8;
            edx &= ~0xff;
            edx |= c;
        }
        ecx  = edx>>3;
        src += ecx;
        edx &= 0xffffff07;
        ecx  = ecx_save;
        if ((ecx & 0xff00) > 0x0600) goto D_Decompress16BitA;
        eax <<= ecx&0xff;
        if ((eax & 0xffff) == 0x8000) goto D_Decompress16BitDepthChange1;
D_Decompress16BitD:
        {   short d = eax&0xffff;
            d >>= (ecx & 0xff);
            eax=(eax&~0xffff) | d;
        }
D_Decompress16BitC:
        ebx += eax;
        *((short int *)dest) = ebx & 0xffff;
        dest++;
        if (--cbcount1) goto D_Decompress16BitData1;
        return;
D_Decompress16BitDepthChange1:
        eax >>= 16;
        eax  &= 0xffffff0f;
        eax++;
        {   unsigned char d = edx&0xff;
            d  += 4;
            edx = (edx&~0xff) | d;
        }
D_Decompress16BitDepthChange3:
        {   unsigned char a = eax&0xff;
            if (a >= ((ecx>>8)&0xff)) a -= 255;
            eax = (eax&~0xff) | a;
        }
        ecx = (ecx&~0xff) | 0x10;
        ecx = (ecx&~0xff00) | ((eax&0xff)<<8);
        {   unsigned char c;
            c   = ecx&0xff;
            c  -= eax&0xff;
            if ((eax&0xff) > (ecx&0xff)) c++;
            ecx = (ecx&~0xff) | c;
        }
    goto D_Decompress16BitData1;

D_Decompress16BitA:

        if ((ecx&0xff00)>0x1000) goto D_Decompress16BitB;
        edx_save = edx;
        edx      = 0x10000;
        edx    >>= (ecx&0xff);
        edx--;
        eax     &= edx;
        edx    >>= 1;
        edx     += 8;
        if (eax > edx) goto D_Decompress16BitE;
        edx     -= 16;
        if (eax <= edx) goto D_Decompress16BitE;
        eax     -= edx;
        edx      = edx_save;
    goto D_Decompress16BitDepthChange3;

D_Decompress16BitE:
        edx      = edx_save;
        eax    <<= (ecx&0xff);
        goto D_Decompress16BitD;

D_Decompress16BitB:
        if (!(eax&0x10000)) goto D_Decompress16BitC;

        ecx = (ecx&~0xff)|0x10;
        eax++;
        {   unsigned char c = (ecx&0xff);
            c  -= (eax&0xff);
            ecx = (ecx&~0xffff) | c | ((eax&0xff)<<8);
        }
    goto D_Decompress16BitData1;
}

// ===================================================================
//  Sample Loader API class structure.
//  To make it sound all OO fancy-like!  Woo, because it matters!  yes!
//  Rememeber: Terminology does not make good code.  I do!

// =====================================================================================
    BOOL SL_Init(SAMPLOAD *s)        // returns 0 on error!
// =====================================================================================
{
    // Allocate a buffer capable of holding a total of 32,768 samples
    // (ie 65536 bytes).  This is necessary for decompressing the 32k
    // frames of ImpulseTracker samples.
    
    if(!sl_buffer)
         if((sl_buffer=(SWORD *)_mm_malloc(65536)) == NULL) return 0;

    // Get the actual byte length of the sample buffer.
    sl_rlength = (s->infmt & SF_16BITS) ? (s->length*2) : s->length;
    if(s->infmt & SF_STEREO) sl_rlength *= 2;
    
    sl_old     = 0;

    return 1;
}


// =====================================================================================
    void SL_Exit(SAMPLOAD *s)
// =====================================================================================
{
    // Done to ensure that sample libs/modules which don't use absolute seek indexes
    // still load sample data properly... because sometimes the scaling routines
    // won't load every last byte of the sample

    if(sl_rlength > 0) _mm_fseek(&s->mmfp,sl_rlength,SEEK_CUR);
}


// =====================================================================================
    void SL_Reset(void)
// =====================================================================================
{
    sl_old = 0;
}


// =====================================================================================
    void SL_Load(void *buffer, SAMPLOAD *smp, int length)
// =====================================================================================
// length  - number of samples to read.  Any unread data will be skipped.
//    This way, the drivers can dictate to only load a portion of the
//    sample, if such behaviour is needed for any reason.
{
    UWORD infmt = smp->infmt, outfmt = smp->outfmt;
    SBYTE *bptr = (SBYTE *)buffer;
    SWORD *wptr = (SWORD *)buffer;
    int   stodo, todo;
    int   t, u;
    MMSTREAM *mmfp = &smp->mmfp;

    if(outfmt & SF_STEREO) length*=2;

    while(length)
    {   // Get the # of samples to process.
        todo  = (sl_rlength < 32768) ? sl_rlength : 32768;
        stodo = todo;
        //if(outfmt & SF_16BITS) stodo >>= 1;

        if(stodo > length) stodo = length;

        if(smp->decompress == DECOMPRESS_IT214)
        {   UWORD tlen;

            // create a 32k loading buffer for reading in the compressed frames of ImpulseTracker
            // samples. Air notes: I had to change this to 36000 bytes to make room for very
            // rare samples which are about 32k in length and don't compress - IT tries to com-
            // press them anyway and makes the compressed block > 32k (and larger than the 
            // original sample!).  Woops!

            if(!sl_compress) sl_compress = (UBYTE *)_mm_malloc(36000);

            // load the compressed data into a buffer.  Has to be done
            // because we can't effectively stream this compression algorithm

            tlen = _mm_read_I_UWORD(mmfp);
            _mm_read_UBYTES(sl_compress,tlen,mmfp);

            if(infmt & SF_16BITS)
            {   if(todo > 16384)
                {   // In this case, we have to load two 32k chunks to have
                    // a match between the number of bytes and samples.
                    Decompress16Bit(sl_compress, sl_buffer, 16384);
                    tlen = _mm_read_I_UWORD(mmfp);
                    _mm_read_UBYTES(sl_compress,tlen,mmfp);
                    Decompress16Bit(sl_compress,&sl_buffer[16384], todo - 16384);
                } else
                    Decompress16Bit(sl_compress,sl_buffer, todo);
            } else
                Decompress8Bit(sl_compress,(SWORD *)sl_buffer, todo);

        } else if(infmt & SF_16BITS)
        {   _mm_read_I_SWORDS(sl_buffer,stodo,mmfp);
        } else
        {   SBYTE  *s;
            SWORD  *d;

            _mm_read_SBYTES((SBYTE *)sl_buffer,stodo,mmfp);

            // convert 8 bit data to 16 bit for conversions!
            
            s  = (SBYTE *)sl_buffer;
            d  = sl_buffer;
            s += stodo;
            d += stodo;

            for(t=0; t<stodo; t++)
            {   s--;
                d--;
                *d = (*s) << 8;
            }
        }

        if(infmt & SF_DELTA)
        {   for(t=0; t<stodo; t++)
            {   sl_buffer[t] += sl_old;
                sl_old        = sl_buffer[t];
            }
        }

        if((infmt^outfmt) & SF_SIGNED)
        {   for(t=0; t<stodo; t++)
               sl_buffer[t] ^= 0x8000;
        }

        if(infmt & SF_STEREO)
        {   if(!(outfmt & SF_STEREO))
            {   // convert stereo to mono!  Easy!
                // NOTE: Should I divide the result by two, or not?
                SWORD  *s, *d;
                s = d = sl_buffer;

                for(t=0; t<stodo; t++, s++, d+=2)
                    *s = (*d + *(d+1)) / 2;

                stodo /= 2;
            }
        } else
        {   if(outfmt & SF_STEREO)
            {   // Yea, it might seem stupid, but I am sure someone will do
                // it someday - convert a mono sample to stereo!
                SWORD  *s, *d;
                s = d = sl_buffer;
                s += stodo;
                d += stodo;

                for(t=0; t<stodo; t++)
                {   s-=2;
                    d--;
                    *s = *(s+1) = *d;
                }
                stodo *= 2;
            }
        }

        if(smp->scalefactor)
        {   int   idx = 0;
            SLONG scaleval;

            // Sample Scaling... average values for better results.
            t = 0;
            while((t < stodo) && length)
            {   scaleval = 0;
                for(u=smp->scalefactor; u && (t < todo); u--, t++)
                    scaleval += sl_buffer[t];
                sl_buffer[idx++] = scaleval / (smp->scalefactor-u);
                length--;
            }
            sl_rlength -= todo;
            todo = idx;
        } else
        {   length -= stodo;
            sl_rlength -= todo;
        }

        if(outfmt & SF_16BITS)
        {   memcpy(wptr, sl_buffer, stodo*sizeof(SWORD));
            wptr += stodo;
        } else
        {   for(t=0; t<stodo; t++) *(bptr++) = sl_buffer[t] >> 8;
        }
    }
}


// =====================================================================================
    SAMPLOAD *SL_RegisterSample(MDRIVER *md, int *handle, uint infmt, uint length, int decompress, MMSTREAM *fp, long seekpos)
// =====================================================================================
// Registers a sample for loading when SL_LoadSamples() is called.
// All samples are assumed to be static (else they would be allocated differently! ;)
{
    SAMPLOAD *news, *cruise;

    cruise = staticlist;

    // Allocate and add structure to the END of the list

    if((news=(SAMPLOAD *)_mm_calloc(1,sizeof(SAMPLOAD))) == NULL) return NULL;

    if(cruise)
    {   while(cruise->next)  cruise = cruise->next;
        cruise->next = news;
    } else
        staticlist = news;

    news->infmt         = news->outfmt = infmt;
    news->decompress    = decompress;
    news->seekpos       = seekpos;
    memcpy(&news->mmfp,fp, sizeof(MMSTREAM));
    news->handle        = handle;
    news->length        = length;

    return news;
}


static void FreeSampleList(SAMPLOAD *s)
{
    SAMPLOAD *old;

    while(s)
    {   old = s;
        s = s->next;
        _mm_free(old, NULL);
    }
}


/*static ULONG SampleTotal(SAMPLOAD *samplist, int type)
// Returns the total amount of memory required by the samplelist queue.
{
    int total = 0;

    while(samplist!=NULL)
    {   samplist->sample->flags = (samplist->sample->flags&~63) | samplist->outfmt;
        total += MD_SampleLength(type,samplist->sample);
        samplist = samplist->next;
    }

    return total;
}


static ULONG RealSpeed(SAMPLOAD *s)
{
    return(s->sample->speed / ((s->scalefactor==0) ? 1 : s->scalefactor));
}    
*/

// =====================================================================================
    static BOOL DitherSamples(MDRIVER *md, SAMPLOAD *samplist)
// =====================================================================================
{
    //SAMPLOAD  *c2smp;
    //ULONG     maxsize, speed;
    SAMPLOAD  *s;

    if(!samplist) return 0;

    // make sure the samples will fit inside available RAM
    /*if((maxsize = MD_SampleSpace(type)*1024) != 0)
    {   while(SampleTotal(samplist, type) > maxsize)
        {   // First Pass - check for any 16 bit samples
            s = samplist;
            while(s!=NULL)
            {   if(s->outfmt & SF_16BITS)
                {   SL_Sample16to8(s);
                    break;
                }
                s = s->next;
            }
    
            // Second pass (if no 16bits found above) is to take the sample
            // with the highest speed and dither it by half.
            if(s==NULL)
            {   s = samplist;
                speed = 0;
                while(s!=NULL)
                {   if((s->sample->length) && (RealSpeed(s) > speed))
                    {   speed = RealSpeed(s);
                        c2smp = s;
                    }
                    s = s->next;
                }
                SL_HalveSample(c2smp);
            }
        }
    }*/


    // Samples dithered, now load them!
    // ================================

    s = samplist;
    while(s)
    {   // sample has to be loaded ? -> increase number of
        // samples, allocate memory and load sample.

        if(s->length)
        {   if(s->seekpos)
                _mm_fseek(&s->mmfp, s->seekpos, SEEK_SET);

            // Call the sample load routine of the driver module.
            // It has to return a 'handle' (>=0) that identifies
            // the sample.

            *s->handle = MD_SampleLoad(md, MM_STATIC, s);

            if(*s->handle < 0)
            {   _mmlog("SampleLoader > Load failed: length = %d; seekpos = %d",s->length, s->seekpos);
                return 1;
            }
        }
        s = s->next;
    }

    return 0;
}


// =====================================================================================
    BOOL SL_LoadSamples(MDRIVER *md)      // Returns 1 on error!
// =====================================================================================
{
    BOOL ok;

    if(!staticlist) return 0;

    ok = DitherSamples(md, staticlist);
    FreeSampleList(staticlist);

    staticlist = NULL;

    return ok;
}


void SL_Sample16to8(SAMPLOAD *s)
{
    s->outfmt &= ~SF_16BITS;
}


void SL_Sample8to16(SAMPLOAD *s)
{
    s->outfmt |= SF_16BITS;
}


void SL_SampleSigned(SAMPLOAD *s)
{
    s->outfmt |= SF_SIGNED;
}


void SL_SampleUnsigned(SAMPLOAD *s)
{
    s->outfmt &= ~SF_SIGNED;
}


void SL_HalveSample(SAMPLOAD *s)
{
    if(s->scalefactor)
        s->scalefactor++;
    else
        s->scalefactor = 2;

    //s->length    = s->diskfmt.length    / s->sample->scalefactor;
    //s->loopstart = s->diskfmt.loopstart / s->sample->scalefactor;
    //s->loopend   = s->diskfmt.loopend   / s->sample->scalefactor;
}

