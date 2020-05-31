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
 Module: vc16ss.c - 16bit STEREO sample mixers!

 Description:
  Stereo, stereo.  It comes from all around.  Stereo, stereo, the multi-
  pronged two-headed attack of sound!  Stereo, Stereo, it owns you-hu!
  Stereo, Stereo, It rings soooooo... oh sooooooo... truuuuuuee!

*/

#include "mikmod.h"
#include "wrap16.h"

void __cdecl Mix16StereoSS(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   index += increment;

        *dest++ += lvolsel * srce[(himacro(index)*2)];
        *dest++ += rvolsel * srce[(himacro(index)*2)+1];
    }
}


void __cdecl Mix16StereoSSI(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)*2];
        *dest++ += lvolsel * (SWORD)(sroot + ((((SLONG)srce[(himacro(index)*2) + 2] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        sroot = srce[(himacro(index)*2)+1];
        *dest++ += rvolsel * (SWORD)(sroot + ((((SLONG)srce[(himacro(index)*2) + 3] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        index  += increment;
    }
}

void __cdecl Mix16MonoSS(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   *dest++ += (lvolsel * ((srce[himacro(index)*2] + srce[(himacro(index)*2)+1]))) / 2;
        index  += increment;
    }
}


void __cdecl Mix16MonoSSI(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)*2], crap;
        crap = (sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        sroot = srce[(himacro(index)*2)+1];
        *dest++ += lvolsel * (crap + (sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))) / 2;

        index  += increment;
    }
}


VMIXER S16_MONO_INTERP =
{   NULL,

    "Stereo-16 (Mono/Interp) v0.1",

    DMODE_INTERP,
    SF_16BITS | SF_STEREO,
    0,
    MD_MONO,

    NULL,
    NULL,

    VC_Volcalc16_Mono,
    VC_Volramp16_Mono,
    Mix16MonoSSI_NoClick,
    NULL,
    Mix16MonoSSI,
    NULL,
};

    
VMIXER S16_STEREO_INTERP =
{   NULL,

    "Stereo-16 (Stereo/Interp) v0.1",

    DMODE_INTERP,
    SF_16BITS | SF_STEREO,
    0,
    MD_STEREO,

    NULL,
    NULL,

    VC_Volcalc16_Stereo,
    VC_Volramp16_Stereo,
    Mix16StereoSSI_NoClick,
    NULL,
    Mix16StereoSSI,
    NULL,
};

VMIXER S16_MONO =
{   NULL,

    "Stereo-16 (Mono) v0.1",

    0,
    SF_16BITS | SF_STEREO,
    0,
    MD_MONO,

    NULL,
    NULL,

    VC_Volcalc16_Mono,
    VC_Volramp16_Mono,
    Mix16MonoSS_NoClick,
    NULL,
    Mix16MonoSS,
    NULL,
};

VMIXER S16_STEREO =
{   NULL,

    "Stereo-16 (Stereo) v0.1",

    0,
    SF_16BITS | SF_STEREO,
    0,
    MD_STEREO,

    NULL,
    NULL,

    VC_Volcalc16_Stereo,
    VC_Volramp16_Stereo,
    Mix16StereoSS_NoClick,
    NULL,
    Mix16StereoSS,
    NULL
};

