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
 Module: vc8norm.c

  Low-level mixer functions for mixing 8 bit sample data.  Includes normal and
  interpolated mixing, without declicking (no microramps, see vcmix8_noclick.c
  for those).

*/

#include "mikmod.h"
#include "wrap8.h"

void __cdecl Mix8StereoNormal(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    UBYTE  sample1, sample2, sample3, sample4;
    int    remain;

    remain = todo & 3;

    for(todo>>=2; todo; todo--)
    {   sample1 = srce[himacro(index)];
        index  += increment;
        sample2 = srce[himacro(index)];
        index  += increment;
        sample3 = srce[himacro(index)];
        index  += increment;
        sample4 = srce[himacro(index)];
        index  += increment;
        
        *dest++ += lvoltab[sample1];
        *dest++ += rvoltab[sample1];
        *dest++ += lvoltab[sample2];
        *dest++ += rvoltab[sample2];
        *dest++ += lvoltab[sample3];
        *dest++ += rvoltab[sample3];
        *dest++ += lvoltab[sample4];
        *dest++ += rvoltab[sample4];
    }

    for(; remain--; )
    {   sample1    = srce[himacro(index)];
        *dest++   += lvoltab[sample1];
        *dest++   += rvoltab[sample1];
        index    += increment;
    }
}


void __cdecl Mix8StereoInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    UBYTE  sample;
   
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        sample = (UBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));
        *dest++ += lvoltab[sample];
        *dest++ += rvoltab[sample];

        index  += increment;
    }
}


void __cdecl Mix8SurroundNormal(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    SLONG sample1, sample2, sample3, sample4;
    int   remain;

    remain = todo & 3;
    
    for(todo>>=2; todo; todo--)
    {
        sample1 = lvoltab[(UBYTE)srce[himacro(index)]];
        index += increment;
        sample2 = lvoltab[(UBYTE)srce[himacro(index)]];
        index += increment;
        sample3 = lvoltab[(UBYTE)srce[himacro(index)]];
        index += increment;
        sample4 = lvoltab[(UBYTE)srce[himacro(index)]];
        index += increment;
        
        *dest++ += sample1;
        *dest++ -= sample1;
        *dest++ += sample2;
        *dest++ -= sample2;
        *dest++ += sample3;
        *dest++ -= sample3;
        *dest++ += sample4;
        *dest++ -= sample4;
    }

    for(; remain--; )
    {   sample1   = lvoltab[(UBYTE)srce[himacro(index)]];
        *dest++  += sample1;
        *dest++  -= sample1;
        index   += increment;
    }
}


void __cdecl Mix8SurroundInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    SLONG sample;

    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        sample = lvoltab[(UBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];

        *dest++ += sample;
        *dest++ -= sample;
        index  += increment;
    }
}


void __cdecl Mix8MonoNormal(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    SLONG  sample1, sample2, sample3, sample4;
    int    remain;

    remain = todo & 3;

    for(todo>>=2; todo; todo--)
    {
        sample1  = lvoltab[(UBYTE)srce[himacro(index)]];
        index  += increment;
        sample2  = lvoltab[(UBYTE)srce[himacro(index)]];
        index  += increment;
        sample3  = lvoltab[(UBYTE)srce[himacro(index)]];
        index  += increment;
        sample4  = lvoltab[(UBYTE)srce[himacro(index)]];
        index  += increment;

        *dest++ += sample1;
        *dest++ += sample2;
        *dest++ += sample3;
        *dest++ += sample4;
    }

    for(; remain--;)
    {   sample1  = lvoltab[(UBYTE)srce[himacro(index)]];
        index  += increment;
        *dest++ += sample1;
    }
}


void __cdecl Mix8MonoInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    SLONG  sample;

    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        sample = lvoltab[(UBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];

        *dest++ += sample;
        index += increment;
    }
}


VMIXER M8_MONO_INTERP =
{   NULL,

    "Mono-8 (Mono/Interp) v0.1",

    DMODE_INTERP,
    0, 0,
    MD_MONO,

    VC_Lookup8_Init,
    VC_Lookup8_Exit,

    VC_Volcalc8_Mono,
    VC_Volramp8_Mono,
    Mix8MonoInterp_NoClick,
    NULL,
    Mix8MonoInterp,
    NULL,
};

VMIXER M8_STEREO_INTERP =
{   NULL,

    "Mono-8 (Stereo/Interp) v0.1",

    DMODE_INTERP,
    0, 0,
    MD_STEREO,

    VC_Lookup8_Init,
    VC_Lookup8_Exit,

    VC_Volcalc8_Stereo,
    VC_Volramp8_Stereo,
    Mix8StereoInterp_NoClick,
    Mix8SurroundInterp_NoClick,
    Mix8StereoInterp,
    Mix8SurroundInterp,
};

VMIXER M8_MONO =
{   NULL,

    "Mono-8 (Mono) v0.1",

    0, 0, 0,
    MD_MONO,

    VC_Lookup8_Init,
    VC_Lookup8_Exit,

    VC_Volcalc8_Mono,
    VC_Volramp8_Mono,
    Mix8MonoNormal_NoClick,
    NULL,
    Mix8MonoNormal,
    NULL,
};

VMIXER M8_STEREO =
{   NULL,

    "Mono-8 (Stereo) v0.1",

    0, 0, 0,
    MD_STEREO,

    VC_Lookup8_Init,
    VC_Lookup8_Exit,

    VC_Volcalc8_Stereo,
    VC_Volramp8_Stereo,
    Mix8StereoNormal_NoClick,
    Mix8SurroundNormal_NoClick,
    Mix8StereoNormal,
    Mix8SurroundNormal,
};
