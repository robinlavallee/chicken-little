/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: wrap8.c

  Basic common wrapper functions for all Mikmod-standard 8-bit mixers (both
  C and assembly versions).  The Init/Deinit functions create the 65-entry
  lookup table used to increase 8 bit mixing performance on pre-Pentium 2
  class CPUs.

  - Init
  - Exit
  - CalculateVolumes
  - RampVolume
  
*/

#include "mikmod.h"
#include "wrap8.h"

int      **voltab = NULL;      // Volume table for 8 bit sample mixing
int       *lvoltab, *rvoltab;
VOLINFO8   v8;

static int  alloc;

// =====================================================================================
    BOOL __cdecl VC_Lookup8_Init(VMIXER *mixer)
// =====================================================================================
{
    int   t;
    
    if(voltab) { alloc++; return 0; }    // quit if already allocated
    
    // Set up the volume tables for the 8 bit sample mixer
    // Chart is 0 - 64 (65 entries)

    alloc = 1;
    if((voltab = (long **)_mm_calloc(65,sizeof(SLONG *))) == NULL) return 1;
    for(t=0; t<65; t++)
       if((voltab[t] = (long *)_mm_calloc(256,sizeof(SLONG))) == NULL) return 1;

    for(t=0; t<65; t++)
    {   long volmul = (65536l*t) / BIT8_VOLFAC;
        int  c;
        for(c=-128; c<128; c++)
            voltab[t][(UBYTE)c] = (SLONG)c*volmul;
    }

    return 0;
}


// =====================================================================================
    void __cdecl VC_Lookup8_Exit(VMIXER *mixer)
// =====================================================================================
{
    if(voltab)
    {   
        alloc--;
        if(alloc==0)
        {   int t;

            _mmlogd("Virtch > Unloading 8 bit mixer volume table");
            for(t=0; t<65; t++) _mm_free(voltab[t], NULL);
            _mm_free(voltab, NULL);
        }
    }
}


// =====================================================================================
    void __cdecl VC_Volcalc8_Mono(VIRTCH *vc, VINFO *vnf)
// =====================================================================================
{
    vnf->vol.front.left = vnf->volume.front.left * vc->volume.front.left;
    //vnf->volume.front.right = 0;

    lvoltab = voltab[(vnf->vol.front.left+1) / BIT8_TBLSCL];

    // declicker: Set us up to volramp!
    if(vc->mode & DMODE_NOCLICK)
    {   if(vnf->vol.front.left != vnf->oldvol.front.left)
        {   if(!vnf->volramp) vnf->volramp = vc->mixspeed / 2048;
            v8.left.inc = ((vnf->vol.front.left - (v8.left.vol = vnf->oldvol.front.left)) / (int)vnf->volramp);
            //_mmlog("vols:  %-5d, %-5d  %-5d, %-5d   volincs: %-7d, %-7d",vnf->lvol, vnf->loldvol, vnf->rvol, vnf->roldvol, vnf->lvolinc, vnf->rvolinc);
        }
    }
}

// =====================================================================================
    void __cdecl VC_Volcalc8_Stereo(VIRTCH *vc, VINFO *vnf)
// =====================================================================================
{
    vnf->vol.front.left = vnf->volume.front.left * vc->volume.front.left;
    vnf->vol.front.right = vnf->volume.front.right * vc->volume.front.right;

    if(vnf->panflg & VC_SURROUND)
    {   lvoltab = rvoltab = voltab[(vnf->vol.front.left+1) / BIT8_TBLSCL];
    } else
    {   lvoltab = voltab[vnf->vol.front.left / BIT8_TBLSCL];
        rvoltab = voltab[vnf->vol.front.right / BIT8_TBLSCL];
    }

    // declicker: Set us up to volramp!
    if(vc->mode & DMODE_NOCLICK)
    {   if((vnf->vol.front.left != vnf->oldvol.front.left) || (vnf->vol.front.right != vnf->oldvol.front.right))
        {   if(!vnf->volramp) vnf->volramp = vc->mixspeed / 2048;
            v8.left.inc  = ((vnf->vol.front.left - (v8.left.vol = vnf->oldvol.front.left)) / (int)vnf->volramp);
            v8.right.inc = ((vnf->vol.front.right - (v8.right.vol = vnf->oldvol.front.right)) / (int)vnf->volramp);
            //_mmlog("left: %-5d %-5d  right: %-5d %-5d  volincs: %-7d, %-7d",vnf->vol.front.left, vnf->oldvol.front.left, vnf->vol.front.right, vnf->oldvol.front.right, v8.left.inc, v8.right.inc);
        }
    }
}

// =====================================================================================
    void __cdecl VC_Volramp8_Mono(VINFO *vnf, int done)
// =====================================================================================
{
    vnf->oldvol.front.left += (v8.left.inc * done);
}

// =====================================================================================
    void __cdecl VC_Volramp8_Stereo(VINFO *vnf, int done)
// =====================================================================================
{
    vnf->oldvol.front.left  += (v8.left.inc * done);
    vnf->oldvol.front.right += (v8.right.inc * done);
//    _mmlog("RAMP : %-5d %-5d .. Done: %-5d",vnf->oldvol.front.left,vnf->oldvol.front.right,done);
}
