// Mikmod Sound System
// Probably Version 3.5 or 4 or something near there.
// Dated sometime in the early 21st century.
//
// Code by Jake Stine of Divine Entertainment.
//
// Description:
//   virtch.c externals defined for use by the mikmod drivers. I see little
//   reason for any non-driver applications to call these functions directly,
//   but feel free to anyway.  Otherwise, don't bother including this thing,
//   you probably don't need it!

#ifndef __VIRTCH_C__
#define __VIRTCH_C__

#include "mmtypes.h"

typedef struct VMIXER
{   struct VMIXER *next;

    CHAR  *name;            // name and version of this mixer!

    uint  mixmode,          // proper mixing mode for this mixer.
          format,           // proper sample format (input) for this mixer.
          cpumode,          // cpu mode required.
          channels;         // number of channels in output (1=mono, 2=stereo, 3=surrond, 4=quad, etc).

    BOOL  (__cdecl *Init)(struct VMIXER *mixer);
    void  (__cdecl *Exit)(struct VMIXER *mixer);
    void  (__cdecl *CalculateVolumes)(struct VIRTCH *vc, struct VINFO *vnf);
    void  (__cdecl *RampVolume)(struct VINFO *vnf, int done);

    void  (__cdecl *NoClick)(void *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
    void  (__cdecl *NoClickSurround)(void *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);

    void  (__cdecl *Mix)(void *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
    void  (__cdecl *MixSurround)(void *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);

    MMVOLUME  *vol;
    void      *mixdat;
} VMIXER;

typedef struct
{   VMIXER *mixer;

    uint  flags;        // looping flags (used for resetting sustain loops only)
    uint  format;       // dataformat flags (16bit, stereo) -- *unchangeable*
    SBYTE *data;
} VSAMPLE;

#define VC_SURROUND   1

typedef struct VINFO
{   // These should be first, since they benefit from 8 byte alignment
    // (or, well, as according to Intel's VTune).

    INT64S    current;        // current index in the sample
    INT64S    increment;      // fixed-point increment value

    // Local instance sample information

    VSAMPLE  *handle;           // driver sampledata handle pointer
                                // also used to hold streambuf info
    int       samplehandle;     // sample handle by number (indexes vsample[])

    uint      flags;            // sample flags
    uint      size;             // sample size (length) in samples
    int       reppos,           // loop start
              repend;           // loop end
    int       suspos,           // sustain loop start
              susend;           // sustain loop end

    uint      start;            // start index
    BOOL      kick;             // =1 -> sample has to be restarted
    ULONG     frq;              // current frequency

    int       panflg;           // Panning Flags (VC_SURROUND, etc)
    MMVOLUME  volume;

    // micro volume ramping (declicker)
    int       volramp;
    MMVOLUME  vol, oldvol;

    // Callback functionality

    struct CALLBACK
    {   void  (*proc)(SBYTE *dest, void *userinfo);
        long  pos;
        void  *userinfo;
    } callback;

    UBYTE     loopbuf[32];     // 32 byte loop buffer for clickless looping
} VINFO;


typedef struct VIRTCH
{
    BOOL     initialized;

    uint     mode;
    uint     mixspeed;
    uint     cpu;
    uint     channels;
    uint     numchn;
    uint     memory;

    MMVOLUME volume;

    uint     samplehandles;

    VINFO   *vinf, *vold;
    VSAMPLE *sample;
    SLONG   *TICKBUF;

    VMIXER  *mixerlist;
} VIRTCH;


#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
****** Virtual channel stuff: *********************************************
**************************************************************************/

#define VC_MAXVOICES 0xfffffful

extern BOOL    VC_Init(void);
extern void    VC_Exit(void);
extern void    VC_Preempt(void);
extern BOOL    VC_SetSoftVoices(uint num);
extern ULONG   VC_SampleSpace(int type);
extern ULONG   VC_SampleLength(int type, SAMPLOAD *s);

extern int     VC_SampleAlloc(uint length, uint *flags);
extern void   *VC_SampleGetPtr(uint handle);
extern int     VC_SampleLoad(SAMPLOAD *sload, int type);
extern void    VC_SampleUnload(uint handle);

extern int     VC_GetActiveVoices(void);

extern BOOL    VC_SetMode(uint mixspeed, uint mode, uint channels, uint cpumode);
extern void    VC_GetMode(uint *mixspeed, uint *mode, uint *channels, uint *cpumode);
extern void    VC_SetVolume(const MMVOLUME *volume);
extern void    VC_GetVolume(MMVOLUME *volume);

extern void    VC_VoiceSetVolume(uint voice, const MMVOLUME *volume);
extern void    VC_VoiceGetVolume(uint voice, MMVOLUME *volume);
extern void    VC_VoiceSetFrequency(uint voice, ulong frq);
extern ulong   VC_VoiceGetFrequency(uint voice);
extern void    VC_VoiceSetPosition(uint voice, ulong pos);
extern ulong   VC_VoiceGetPosition(uint voice);

extern void VC_VoiceSetSurround(uint voice, int flags);

extern void    VC_VoicePlay(uint voice, uint handle, uint start, uint length, int reppos, int repend, int suspos, int susend, uint flags);
extern void    VC_VoiceResume(uint voice);

extern void    VC_VoiceStop(uint voice);
extern BOOL    VC_VoiceStopped(uint voice);
extern void    VC_VoiceReleaseSustain(uint voice);
extern ULONG   VC_VoiceRealVolume(uint voice);

// These are functions the drivers use to update the mixing buffers.

extern void    VC_WriteSamples(MDRIVER *md, SBYTE *buf, long todo);
extern ULONG   VC_WriteBytes(MDRIVER *md, SBYTE *buf, long todo);
extern void    VC_SilenceBytes(SBYTE *buf, long todo);


// =====================================
//    Mikmod Dynamic Pluggable Mixers
// =====================================

// Default 'C' Mixers.

extern VMIXER M8_MONO_INTERP, M8_STEREO_INTERP,
              S8_MONO_INTERP, S8_STEREO_INTERP,
              M8_MONO, M8_STEREO,
              S8_MONO, S8_STEREO;

extern VMIXER M16_MONO_INTERP, M16_STEREO_INTERP,
              S16_MONO_INTERP, S16_STEREO_INTERP,
              M16_MONO, M16_STEREO,
              S16_MONO, S16_STEREO;

// Intel Assembly Mixers.

extern VMIXER ASM_M8_MONO_INTERP, ASM_M8_STEREO_INTERP,
              ASM_S8_MONO_INTERP, ASM_S8_STEREO_INTERP,
              ASM_M8_MONO, ASM_M8_STEREO,
              ASM_S8_MONO, ASM_S8_STEREO;

extern VMIXER ASM_M16_MONO_INTERP, ASM_M16_STEREO_INTERP,
              ASM_S16_MONO_INTERP, ASM_S16_STEREO_INTERP,
              ASM_M16_MONO, ASM_M16_STEREO,
              ASM_S16_MONO, ASM_S16_STEREO;

#ifdef __cplusplus
}
#endif

#endif
