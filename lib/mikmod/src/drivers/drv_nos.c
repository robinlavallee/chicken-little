/*

Name:
DRV_NOS.C

Description:
Mikmod driver for no output on any soundcard, monitor, keyboard, or whatever :)

Portability:
All systems - All compilers

*/

#include "mikmod.h"
#include <string.h>


static BOOL NS_IsThere(void)
{
    return 1;
}


static int NS_SampleLoad(SAMPLOAD *s, int type)
{
    return 0;
}


static void NS_SampleUnload(uint h)
{
}


static ULONG NS_SampleSpace(int type)
{
    return 0;
}


static ULONG NS_SampleLength(int type, SAMPLOAD *s)
{
    return 0;
}


static BOOL NS_Init(uint latency, void *optstr)
{
    return 0;
}


static void NS_Exit(void)
{
}


static void NS_Update(MDRIVER *md)
{
}


static void NS_Preempt(void)
{
}


static BOOL NS_SetHardVoices(uint num)
{
    return 0;
}

static BOOL NS_SetSoftVoices(uint num)
{
    return 0;
}


static void NS_VoiceSetVolume(uint voice, const MMVOLUME *volume)
{
}

static void NS_VoiceGetVolume(uint voice, MMVOLUME *volume)
{
    memset(volume,0,sizeof(MMVOLUME));
}


static void NS_VoiceSetFrequency(uint voice,ULONG frq)
{
}

static ULONG NS_VoiceGetFrequency(uint voice)
{
    return 0;
}


static void NS_VoicePlay(uint voice, uint handle, uint start, uint length, int reppos, int repend, int suspos, int susend, uint flags)
{
}


static void NS_VoiceStop(uint voice)
{
}


static BOOL NS_VoiceStopped(uint voice)
{
   return 0;
}


static void NS_VoiceReleaseSustain(uint voice)
{
}


static void NS_VoiceSetPosition(uint voice, ulong pos)
{
}

static ulong NS_VoiceGetPosition(uint voice)
{
   return 0;
}

static void NS_VoiceSetSurround(uint voice, int flags)
{

}

static int NS_GetActiveVoices(void)
{
   return 0;
}


static ULONG NS_VoiceRealVolume(uint voice)
{
   return 0;
}

static BOOL NS_SetMode(uint mixspeed, uint mode, uint channels, uint cpumode)
{
    return 0;
}

static void NS_SetVolume(const MMVOLUME *volume)
{


}

static void NS_GetMode(uint *mixspeed, uint *mode, uint *channels, uint *cpumode)
{
    *mixspeed = 0; *mode = 0; *channels = 0; *cpumode = 0;
}

static void NS_GetVolume(MMVOLUME *volume)
{
    memset(volume,0,sizeof(MMVOLUME));
}

static void NS_VoiceResume(uint voice)
{

}

static int NS_SampleAlloc(uint length, uint *flags)
{
    return 0;
}

static void *NS_SampleGetPtr(uint length)
{
    return NULL;
}


MD_DEVICE drv_nos =
{   "No Sound",
    "Nosound Driver v2.0 - (c) Creative Silence",
    0,0,
    NULL,

    // Sample loading
    NS_SampleAlloc,
    NS_SampleGetPtr,
    NS_SampleLoad,
    NS_SampleUnload,
    NS_SampleSpace,
    NS_SampleLength,

    // Detection and initialization
    NS_IsThere,
    NS_Init,
    NS_Exit,
    NS_Update,
    NS_Preempt,

    NS_SetHardVoices,
    NS_SetSoftVoices,

    NS_SetMode,
    NS_GetMode,
    NS_SetVolume,
    NS_GetVolume,

    // Voice control and voice information
    NS_GetActiveVoices,
    NS_VoiceSetVolume,
    NS_VoiceGetVolume,
    NS_VoiceSetFrequency,
    NS_VoiceGetFrequency,
    NS_VoiceSetPosition,
    NS_VoiceGetPosition,

    NS_VoiceSetSurround,

    NS_VoicePlay,
    NS_VoiceResume,
    NS_VoiceStop,
    NS_VoiceStopped,
    NS_VoiceReleaseSustain,
    NS_VoiceRealVolume
};

