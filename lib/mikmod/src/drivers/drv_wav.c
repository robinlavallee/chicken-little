
#include "mikmod.h"
#include "virtch.h"

#ifdef __GNUC__
#include <sys/types.h>
#else
#include <io.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

#define WAVBUFFERSIZE 8192

static BOOL initialized = 0;

static MMSTREAM *wavout;
static SBYTE *WAV_DMABUF;
static ULONG dumpsize;

// driver mode settings
static uint  wmode = DMODE_16BITS | DMODE_INTERP | DMODE_NOCLICK | DMODE_SAMPLE_DYNAMIC;
static uint  wmixspeed = 44100;
static uint  wChannels = 2;

static BOOL WAV_IsThere(void)
{
    return 1;
}


static BOOL WAV_Init(uint latency, void *optstr)
{
    if(NULL == (wavout = _mm_fopen("music.wav", "wb"))) return 1;
    if(NULL == (WAV_DMABUF = _mm_malloc(WAVBUFFERSIZE))) return 1;

    if(VC_Init()) return 1;
    
    return 0;
}


static void WAV_Exit(void)
{
    VC_Exit();

    // write in the actual sizes now

    if(wavout)
    {   _mm_fseek(wavout,4,SEEK_SET);
        _mm_write_I_ULONG(dumpsize + 34, wavout);
        _mm_fseek(wavout,42,SEEK_SET);
        _mm_write_I_ULONG(dumpsize, wavout);

        _mm_fclose(wavout);

        if(WAV_DMABUF) free(WAV_DMABUF);
    }
}


static void WAV_Update(MDRIVER *md)
{
    VC_WriteBytes(md, WAV_DMABUF, WAVBUFFERSIZE);
    _mm_write_SBYTES(WAV_DMABUF, WAVBUFFERSIZE, wavout);
    dumpsize += WAVBUFFERSIZE;
}


static BOOL WAV_SetMode(uint mixspeed, uint mode, uint channels, uint cpumode)
{
    if(mixspeed) wmixspeed = mixspeed;
    if(!(mode & DMODE_DEFAULT)) wmode = mode;

    wmode |= DMODE_SAMPLE_DYNAMIC;   // software mixer only.

    switch(channels)
    {   case MD_MONO:
            wChannels = 1;
        break;

        case MD_STEREO:
            wChannels = 2;
        break;

        case MD_QUADSOUND:
            wChannels = 4;
        break;
    }

    VC_SetMode(wmixspeed, wmode, channels, cpumode);

    _mm_write_string("RIFF    WAVEfmt ",wavout);
    _mm_write_I_ULONG(18,wavout);     // length of this RIFF block crap

    _mm_write_I_UWORD(1, wavout);     // microsoft format type
    _mm_write_I_UWORD(wChannels, wavout);
    _mm_write_I_ULONG(wmixspeed, wavout);
    _mm_write_I_ULONG(wmixspeed * wChannels *
                      ((wmode & DMODE_16BITS) ? 2 : 1), wavout);

    _mm_write_I_UWORD(((wmode & DMODE_16BITS) ? 2 : 1) * (wChannels), wavout);    // block alignment (8/16 bit)

    _mm_write_I_UWORD((wmode & DMODE_16BITS) ? 16 : 8, wavout);
    _mm_write_I_UWORD(0,wavout);      // No extra data here.

    _mm_write_string("data    ",wavout);

    dumpsize = 0;

    return 0;
}


MD_DEVICE drv_wav =
{   "music.wav file",
    "WAV [music.wav] file output driver v1.0",
    0,VC_MAXVOICES,
    NULL,

    // Sample loading
    VC_SampleAlloc,
    VC_SampleGetPtr,
    VC_SampleLoad,
    VC_SampleUnload,
    VC_SampleSpace,
    VC_SampleLength,

    // Detection and initialization
    WAV_IsThere,
    WAV_Init,
    WAV_Exit,
    WAV_Update,
    VC_Preempt,

    NULL,
    VC_SetSoftVoices,

    WAV_SetMode,
    VC_GetMode,

    VC_SetVolume,
    VC_GetVolume,

    // Voice control and voice information
    VC_GetActiveVoices,

    VC_VoiceSetVolume,
    VC_VoiceGetVolume,
    VC_VoiceSetFrequency,
    VC_VoiceGetFrequency,
    VC_VoiceSetPosition,
    VC_VoiceGetPosition,
    VC_VoiceSetSurround,

    VC_VoicePlay,
    VC_VoiceResume,
    VC_VoiceStop,
    VC_VoiceStopped,
    VC_VoiceReleaseSustain,
    VC_VoiceGetPosition
};

