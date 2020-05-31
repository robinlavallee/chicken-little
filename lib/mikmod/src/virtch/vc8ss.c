/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.simplenet.com

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: vc8ss.c  - Stereo Sample Mixer!

  Low-level mixer functions for mixing 8 bit STEREO sample data.  Includes
  normal and interpolated mixing, without declicking (no microramps, see
  vc8ssnc.c for those).

  Note: Stereo Sample Mixing does not support Dolby Surround.  Dolby Surround
  is lame anyway, so get over it!

*/

#include "mikmod.h"
#include "wrap8.h"


void __cdecl Mix8StereoSS(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    int    remain;

    remain = todo & 1;

    for(todo>>=1; todo; todo--)
    {   *dest++ += lvoltab[srce[(himacro(index)*2)]];
        *dest++ += rvoltab[srce[(himacro(index)*2) + 1]];
        index  += increment;
        
        *dest++ += lvoltab[srce[(himacro(index)*2)]];
        *dest++ += rvoltab[srce[(himacro(index)*2) + 1]];
        index  += increment;
    }

    if(remain)
    {   *dest++   += lvoltab[srce[(himacro(index)*2)]];
        *dest++   += rvoltab[srce[(himacro(index)*2) + 1]];
        index    += increment;
    }
}


void __cdecl Mix8StereoSSI(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)*2];
        *dest++ += lvoltab[(UBYTE)(sroot + ((((SLONG)srce[(himacro(index)*2) + 2] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];

        sroot = srce[(himacro(index)*2) + 1];
        *dest++ += rvoltab[(UBYTE)(sroot + ((((SLONG)srce[(himacro(index)*2) + 3] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];

        index  += increment;
    }
}


void __cdecl Mix8MonoSS(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    int    remain;

    remain = todo & 1;

    // To mix mono samples, just mix in both channels of the stereo sample using the
    // v8.left.sel.  Question is, will this result in a sample that is louder than it
    // should be?  (ie, divide the v8.left.sel lookup by 2 first maybe?)

    for(todo>>=1; todo; todo--)
    {   *dest++ += lvoltab[(UBYTE)((srce[(himacro(index)*2)] + srce[(himacro(index)*2)+1]) / 2)];
        index  += increment;

        *dest++ += lvoltab[(UBYTE)((srce[(himacro(index)*2)] + srce[(himacro(index)*2)+1]) / 2)];
        index  += increment;        
    }

    if(remain)
    {   *dest++ += lvoltab[(UBYTE)((srce[(himacro(index)*2)] + srce[(himacro(index)*2)+1]) / 2)];
        index  += increment;
    }
}


void __cdecl Mix8MonoSSI(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[(himacro(index)*2)], crap;
        crap = lvoltab[(UBYTE)(sroot + ((((SLONG)srce[(himacro(index)*2) + 2] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];
        index += increment;

        sroot = srce[(himacro(index)*2)+1];
        *dest++ += (crap + lvoltab[(UBYTE)(sroot + ((((SLONG)srce[(himacro(index)*2) + 3] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))]) / 2;
        index += increment;
    }
}


VMIXER S8_MONO_INTERP =
{   NULL,

    "Stereo-8 (Mono/Interp) v0.1",

    DMODE_INTERP,
    0, 0,
    MD_MONO,

    VC_Lookup8_Init,
    VC_Lookup8_Exit,

    VC_Volcalc8_Mono,
    VC_Volramp8_Mono,
    Mix8MonoSSI_NoClick,
    Mix8MonoSSI,
};

VMIXER S8_STEREO_INTERP =
{   NULL,

    "Stereo-8 (Stereo/Interp) v0.1",

    DMODE_INTERP,
    0, 0,
    MD_STEREO,

    VC_Lookup8_Init,
    VC_Lookup8_Exit,

    VC_Volcalc8_Stereo,
    VC_Volramp8_Stereo,
    Mix8StereoSSI_NoClick,
    Mix8StereoSSI,
};

VMIXER S8_MONO =
{   NULL,

    "Stereo-8 (Mono) v0.1",

    0, 0, 0,
    MD_MONO,

    VC_Lookup8_Init,
    VC_Lookup8_Exit,

    VC_Volcalc8_Mono,
    VC_Volramp8_Mono,
    Mix8MonoSS_NoClick,
    NULL,
    Mix8MonoSS,
    NULL
};

VMIXER S8_STEREO =
{   NULL,

    "Stereo-8 (Stereo) v0.1",

    0, 0, 0,
    MD_STEREO,
    
    VC_Lookup8_Init,
    VC_Lookup8_Exit,

    VC_Volcalc8_Stereo,
    VC_Volramp8_Stereo,
    Mix8StereoSS_NoClick,
    NULL,
    Mix8StereoSS,
    NULL
};

