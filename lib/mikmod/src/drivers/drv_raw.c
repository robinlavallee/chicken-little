/*

Name:
DRV_RAW.C

Description:
Mikmod driver for output to a file called MUSIC.RAW

MS-DOS Programmers:
 !! DO NOT CALL MD_UPDATE FROM AN INTERRUPT IF YOU USE THIS DRIVER !!

Portability:

MSDOS:  BC(y)   Watcom(y)   DJGPP(y)
Win95:  BC(y)
Linux:  y

(y) - yes
(n) - no (not possible or not useful)
(?) - may be possible, but not tested

*/

#include "mikmod.h"
#include "virtch.h"

#ifdef __GNUC__
#include <sys/types.h>
#else
#include <io.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

#define RAWBUFFERSIZE 8192

static int rawout;

static SBYTE RAW_DMABUF[RAWBUFFERSIZE];


static BOOL RAW_IsThere(void)
{
    return 1;
}


static BOOL RAW_Init(uint latency, void *optstr)
{
    if(-1 == (rawout = open("music.raw", 
#ifndef __GNUC__
                O_BINARY | 
#endif
                O_RDWR | O_TRUNC | O_CREAT, S_IREAD | S_IWRITE)))
        return 1;

    if(VC_Init()) return 1;

    return 0;
}


static void RAW_Exit(void)
{
    VC_Exit();
    close(rawout);
}


static void RAW_Update(MDRIVER *md)
{
    VC_WriteBytes(md, RAW_DMABUF, RAWBUFFERSIZE);
    write(rawout, RAW_DMABUF, RAWBUFFERSIZE);
}

MD_DEVICE drv_raw =
{   "music.raw file",
    "RAW [music.raw] file output driver v1.0",
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
    RAW_IsThere,
    RAW_Init,
    RAW_Exit,
    RAW_Update,
    VC_Preempt,

    NULL,
    VC_SetSoftVoices,

    VC_SetMode,
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

    VC_VoiceRealVolume
};

