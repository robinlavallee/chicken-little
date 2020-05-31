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
 Module: vc16norm.c

 Description:
  Low-level mixer functions for mixing 16 bit sample data.  Includes normal and
  interpolated mixing, without declicking (no microramps, see vc16nc.c
  for those).

*/

#include "mikmod.h"
#include "wrap16.h"

void __cdecl Mix16StereoNormal(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    SWORD  sample;

    for(; todo; todo--)
    {
        sample = srce[himacro(index)];
        index += increment;

        *dest++ += lvolsel * sample;
        *dest++ += rvolsel * sample;
    }
}


void __cdecl Mix16StereoInterp(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG  sroot  = srce[himacro(index)];
        register SWORD  sample = (SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        *dest++ += lvolsel * sample;
        *dest++ += rvolsel * sample;
        index  += increment;
    }
}


void __cdecl Mix16SurroundNormal(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    SLONG  sample;

    for (; todo; todo--)
    {   sample = lvolsel * srce[himacro(index)];
        index += increment;

        *dest++ += sample;
        *dest++ -= sample;
    }
}


void __cdecl Mix16SurroundInterp(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for (; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        sroot = lvolsel * (SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        *dest++ += sroot;
        *dest++ -= sroot;
        index  += increment;
    }
}


void __cdecl Mix16MonoNormal(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   *dest++ += lvolsel * srce[himacro(index)];
        index  += increment;
    }
}


void __cdecl Mix16MonoInterp(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        sroot = lvolsel * (SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));
        *dest++ += sroot;
        index  += increment;
    }
}



VMIXER M16_MONO_INTERP =
{   NULL,

    "Mono-16 (Mono/Interp) v0.1",

    DMODE_INTERP,
    SF_16BITS,
    0,
    MD_MONO,

    NULL,
    NULL,

    VC_Volcalc16_Mono,
    VC_Volramp16_Mono,
    Mix16MonoInterp_NoClick,
    NULL,
    Mix16MonoInterp,
    NULL,
};

VMIXER M16_STEREO_INTERP =
{   NULL,

    "Mono-16 (Stereo/Interp) v0.1",

    DMODE_INTERP,
    SF_16BITS,
    0,
    MD_STEREO,

    NULL,
    NULL,

    VC_Volcalc16_Stereo,
    VC_Volramp16_Stereo,
    Mix16StereoInterp_NoClick,
    Mix16SurroundInterp_NoClick,
    Mix16StereoInterp,
    Mix16SurroundInterp,
};

VMIXER M16_MONO =
{   NULL,

    "Mono-16 (Mono) v0.1",

    0,
    SF_16BITS,
    0,
    MD_MONO,
    
    NULL,
    NULL,

    VC_Volcalc16_Stereo,
    VC_Volramp16_Stereo,
    Mix16MonoNormal_NoClick,
    Mix16MonoNormal
};

VMIXER M16_STEREO =
{   NULL,

    "Mono-16 (Stereo) v0.1",

    0,
    SF_16BITS,
    0,
    MD_STEREO,
    
    NULL,
    NULL,
    
    VC_Volcalc16_Mono,
    VC_Volramp16_Mono,
    Mix16StereoNormal_NoClick,
    Mix16SurroundNormal_NoClick,
    Mix16StereoNormal,
    Mix16SurroundNormal,
};
