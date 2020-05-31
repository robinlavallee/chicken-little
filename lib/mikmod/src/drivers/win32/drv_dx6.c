/*

Name:
DRV_dx6.C
by matthew gambrell
zeromus@verge-rpg.com

Description:
Mikmod driver for output via directsound

Portability:

MSDOS:  N/A
Win95:  BC(?)   Watcom(y) MSVC6(y)
Linux:  WINE(?)

(y) - yes
(n) - no (not possible or not useful)
(?) - may be possible, but not tested


Notes about Directsound:

 Directsound is kinda funny about working in cooperative and exclusive
 modes, since it doesn't really behae how the docs would indicate (at
 least by how I read them).

 Our primary buffer always has to be DSSCL_PRIMARY, otherwise we can-
 not alter the mixer settings.
   
 In exclusive mode, we set the primary buffer to DSSCL_EXCLUSIVE,
 which keeps the mixer from continuing while the game is in the back-
 ground.  Without it, some drivers will become unstable and crash,
 and the music will play but not be heard (usually undesired).

 In Cooperative mode, the secondary buffer has to be flagged with
 DSCAPS_GLOBALFOCUS, which works perfectly fine for any soundcard
 (or driver which supports playing multiple waveforms.  This will
 not work on SB16s and other like soundcards, but well, they're
 used to getting shafted.

*/

//#ifdef HAVE_CONFIG_H
//#include "config.h"
//#endif

#include "mikmod.h"
#include "virtch.h"

#include <windows.h>
#include <dsound.h>
#include <stdio.h>

static void DS_PlayStop(void);
static void DS_WipeBuffers(void);

static LPDIRECTSOUND ds = NULL;
static uint  dxMode     = DMODE_16BITS | DMODE_INTERP | DMODE_NOCLICK;
static uint  dxMixspeed = 44100;
static uint  dxChannels = 2;

static LPDIRECTSOUNDBUFFER pb;
static LPDIRECTSOUNDBUFFER bb;

static int   dorefresh = 0;
static int   poppos;
static DWORD MM_drv_dx6_bufsize;
static int   MM_drv_dx6_tolerance;

static DSCAPS DSCaps;


struct
{   HINSTANCE dll;
} hwdata;

static void logerror(const CHAR *function, int code)
{
	static CHAR *error;

	switch (code)
    {   case E_NOINTERFACE:
			error = "Unsupported interface (DirectX version incompatability)";
		break;

        case DSERR_ALLOCATED:
			// This one gets a special action since the user may want to be
            // informed that their sound is tied up.
            error = "Audio device in use.";
            _mmerr_set(MMERR_INITIALIZING_DEVICE, "Audio device is already in use.");
		break;

        case DSERR_BADFORMAT:
			error = "Unsupported audio format";
		break;

        case DSERR_BUFFERLOST:
			error = "Mixing buffer was lost";
		break;

        case DSERR_INVALIDPARAM:
			error = "Invalid parameter";
		break;

        case DSERR_NODRIVER:
			error = "No audio device found";
        break;

		case DSERR_OUTOFMEMORY:
			error = "DirectSound ran out of memory";
		break;

		case DSERR_PRIOLEVELNEEDED:
			error = "Caller doesn't have priority";
		break;

		case DSERR_UNSUPPORTED:
			error = "Function not supported (DirectX driver inferiority)";
		break;
	}
    _mmlog("Mikmod > drv_ds > %s : %s", function, error);
	return;
}


static BOOL DS_IsPresent(void)
{
	HINSTANCE dsdll;
	int       ok;

	// Version check DSOUND.DLL
	ok = 0;
	dsdll = LoadLibrary("DSOUND.DLL");
	
    if (dsdll)
    {
		// DirectSound available.  Good.
        
        ok = 1;
        
        /*  Note: NT has latency problems, it seems.  We may want to uncomment
            this code and force NT users to drop back to winmm audio.
        
        OSVERSIONINFO ver;
		ver.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		GetVersionEx(&ver);
		switch (ver.dwPlatformId)
        {	case VER_PLATFORM_WIN32_NT:
                if (ver.dwMajorVersion > 4)
                {   // Win2K
                    ok = 1;
				} else
                {   // WinNT
                	ok = 0;
                }
		    break;

			default:
                // Win95 or Win98
                dsound_ok = 1;
		    break;
		}
        */

		FreeLibrary(dsdll);
	}
	
    return ok;
}

static DWORD CurrBufPlayPos, CurrBufWritePos;

static BOOL DS_Init(uint latency, void *optstr)
{
    HRESULT (WINAPI *DSoundCreate)(LPGUID, LPDIRECTSOUND *, LPUNKNOWN);
    HRESULT  hres;

	// Load the dsound DLL
    
    hwdata.dll = LoadLibrary("DSOUND.DLL");
	
    if(!hwdata.dll)
    {   _mmlog("Mikmod > drv_ds > Failed loading DSOUND.DLL!");
        return 1;
    }

    // we safely assume that the user detected the dsound first.
    DSoundCreate = (void *)GetProcAddress(hwdata.dll, "DirectSoundCreate");

    //SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS); //HIGH_PRIORITY_CLASS);
    //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    
    hres = DSoundCreate(NULL,&ds,NULL);
    if(hres != DS_OK)
    {   logerror("DSoundCreate",hres);
        return 1;
    }

    if(VC_Init()) return 1;

    return 0;
}

static void DS_Exit(void)
{
    if(hwdata.dll)
    {   if(bb) IDirectSoundBuffer_Stop(bb);
        dorefresh = 0;
    
        VC_Exit();

        if(bb) IDirectSoundBuffer_Release(bb);
        if(pb) IDirectSoundBuffer_Release(pb);
        if(ds) IDirectSound_Release(ds);

        ds = NULL;

        FreeLibrary(hwdata.dll);
        hwdata.dll = NULL;
    }
}

static void DS_Update(MDRIVER *md)
{
    static int   last = 0;

    if(dorefresh)
    {   int      diff;
        DWORD    ptr1len, ptr2len;
        void    *ptr1, *ptr2;
        HRESULT  h;

        IDirectSoundBuffer_GetCurrentPosition(bb,&CurrBufPlayPos,&CurrBufWritePos);

        diff = CurrBufWritePos-last;

        if(diff<0)  diff=MM_drv_dx6_bufsize-last+CurrBufWritePos;
        if(diff!=0)
        {   last=CurrBufWritePos;

            h = IDirectSoundBuffer_Lock(bb,poppos,diff,&ptr1,&ptr1len,&ptr2,&ptr2len,0);
            if(h==DSERR_BUFFERLOST)
            {   IDirectSoundBuffer_Restore(bb);
                h=IDirectSoundBuffer_Lock(bb,poppos,diff,&ptr1,&ptr1len,&ptr2,&ptr2len,0);
            }

            if(h==DS_OK)
            {   //if player is paused?
                //VC_SilenceBytes(ptr1,ptr1len);
                //if(ptr2) VC_SilenceBytes(ptr2,ptr2len);

                VC_WriteBytes(md, ptr1,ptr1len);
                if(ptr2) VC_WriteBytes(md, ptr2,ptr2len);

                IDirectSoundBuffer_Unlock(bb,ptr1,ptr1len,ptr2,ptr2len);
                poppos+=diff;
                if(poppos >= MM_drv_dx6_bufsize) poppos -= MM_drv_dx6_bufsize;
            }
        }
        else return;
    }
}

static void DS_WipeBuffers(void)
{
    if(dorefresh)
    {   DWORD    ptr1len;
        void    *ptr1;
        HRESULT  h;

        h = IDirectSoundBuffer_Lock(bb,0,0,&ptr1,&ptr1len, NULL, NULL, DSBLOCK_ENTIREBUFFER);
        if(h==DSERR_BUFFERLOST)
        {   IDirectSoundBuffer_Restore(bb);
            h=IDirectSoundBuffer_Lock(bb,0,0,&ptr1,&ptr1len, NULL, NULL, DSBLOCK_ENTIREBUFFER);
        }

        if(h==DS_OK)
        {   VC_SilenceBytes(ptr1, ptr1len);
            IDirectSoundBuffer_Unlock(bb,ptr1,ptr1len,0,0);
        }
    }
}

// These need code!

static BOOL DS_SetMode(uint mixspeed, uint mode, uint channels, uint cpumode)
{
    DSBUFFERDESC bf;
    WAVEFORMATEX pcmwf;
    HRESULT      hres;

    int          latency = 200;

    if(!ds) return 0;

    if(mixspeed) dxMixspeed = mixspeed;
    if(dxMixspeed > 44100) dxMixspeed = 44100;

    if(!(mode & DMODE_DEFAULT)) dxMode = mode;

    switch(channels)
    {   case MD_MONO:
            dxChannels = 1;
        break;

        default:
            dxChannels = 2;
            channels   = MD_STEREO;
        break;
    }

    hres = ds->lpVtbl->SetCooperativeLevel(ds,GetForegroundWindow(),DSSCL_PRIORITY | ((dxMode & DMODE_EXCLUSIVE) ? DSSCL_EXCLUSIVE : 0));

    if(hres != DS_OK)
    {   logerror("SetCooperativeLevel",hres);
        return 1;
    }

    memset(&pcmwf,0,sizeof(PCMWAVEFORMAT));
    pcmwf.wFormatTag      = WAVE_FORMAT_PCM;
    pcmwf.nChannels       = dxChannels;
    pcmwf.nSamplesPerSec  = dxMixspeed;
    pcmwf.wBitsPerSample  = (dxMode&DMODE_16BITS) ? 16 : 8;
    pcmwf.nBlockAlign     = (pcmwf.wBitsPerSample*pcmwf.nChannels)/8;
    pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec*pcmwf.nBlockAlign;

    MM_drv_dx6_tolerance  = (pcmwf.nAvgBytesPerSec*(latency/2)) / 1000;
    MM_drv_dx6_tolerance &= ~3;
    MM_drv_dx6_bufsize    = MM_drv_dx6_tolerance*2;
    poppos                = MM_drv_dx6_tolerance;
       
    memset(&bf,0,sizeof(DSBUFFERDESC));
    bf.lpwfxFormat   = NULL; 
    bf.dwSize        = sizeof(DSBUFFERDESC);
    bf.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    bf.dwBufferBytes = 0;

    hres = ds->lpVtbl->CreateSoundBuffer(ds,&bf,&pb,NULL);
    if(hres != DS_OK)
    {   logerror("CreateSoundBuffer",hres);
        return 1;
    }

    hres = pb->lpVtbl->SetFormat(pb,&pcmwf);
    if(hres != DS_OK)
    {   logerror("Primary_SetFormat",hres);
        return 1;
    }

    pb->lpVtbl->Play(pb,0,0,DSBPLAY_LOOPING);
    
    memset(&pcmwf,0,sizeof(PCMWAVEFORMAT));
    pcmwf.wFormatTag      = WAVE_FORMAT_PCM;
    pcmwf.nChannels       = dxChannels;
    pcmwf.wBitsPerSample  = (dxMode&DMODE_16BITS) ? 16 : 8;
    pcmwf.nSamplesPerSec  = dxMixspeed;
    pcmwf.nBlockAlign     = (pcmwf.wBitsPerSample*pcmwf.nChannels)/8;
    pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec*pcmwf.nBlockAlign;
    
    memset(&bf,0,sizeof(DSBUFFERDESC));
    bf.dwSize          = sizeof(DSBUFFERDESC);
    bf.dwFlags         = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STATIC | ((dxMode & DMODE_EXCLUSIVE) ? 0 : DSBCAPS_GLOBALFOCUS);
    bf.dwBufferBytes   = MM_drv_dx6_bufsize;
    bf.lpwfxFormat     = &pcmwf;
    
    hres = IDirectSound_CreateSoundBuffer(ds,&bf,&bb,NULL);
    if(hres != DS_OK)
    {   logerror("CreateSoundBuffer",hres);
        return 1;
    }

    CurrBufPlayPos = CurrBufWritePos = 0;

    VC_SetMode(dxMixspeed, dxMode, channels, cpumode);
    
    // Finally, start playing the buffer (and wipe it first)
    dorefresh = 1;
    DS_WipeBuffers();
    IDirectSoundBuffer_Play(bb,0,0,DSBPLAY_LOOPING);

    return 0;
}


MD_DEVICE drv_ds =
{
    "DirectSound",
    "DirectSound Driver (DX6) v0.1",
    0,VC_MAXVOICES,
    NULL,       // Linked list!

    // sample Loading
    VC_SampleAlloc,
    VC_SampleGetPtr,
    VC_SampleLoad,
    VC_SampleUnload,
    VC_SampleSpace,
    VC_SampleLength,

    // Detection and initialization
    DS_IsPresent,
    DS_Init,
    DS_Exit,
    DS_Update,
    VC_Preempt,

    NULL,
    VC_SetSoftVoices,

    DS_SetMode,
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
    VC_VoiceRealVolume,

    DS_WipeBuffers
};
