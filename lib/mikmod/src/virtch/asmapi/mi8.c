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
 Module: mi8.c
 
  Assembly Mixer Plugin : Mono 8 bit Sample data / Interpolation
  
  Generic all-purpose assembly wrapper for mikmod VMIXER plugin thingie.
  As long as your platform-dependant asm code uses the naming conventions
  below (Asm StereoInterp, SurroundInterp, etc), then you can use this
  module and the others related to it to register your mixer into mikmod.

  See Also:

    mn8.c
    sn8.c
    si8.c
    mn16.c
    sn16.c
    mi16.c
    si16.c
*/

#include "mikmod.h"
#include "..\wrap8.h"
#include "asmapi.h"


VMIXER ASM_M8_MONO_INTERP =
{   NULL,

    "Assembly Mono-8 (Mono/Interp) v0.1",

    DMODE_INTERP,
    0,
    0,
    MD_MONO,

    VC_Lookup8_Init,
    VC_Lookup8_Exit,
    VC_Volcalc8_Mono,
    VC_Volramp8_Mono,
    Mix8MonoInterp_NoClick,
    NULL,
    AsmMonoInterp,
    NULL
};

VMIXER ASM_M8_STEREO_INTERP =
{   NULL,

    "Assembly Mono-8 (Stereo/Interp) v0.1",

    DMODE_INTERP,
    0,
    0,
    MD_STEREO,

    VC_Lookup8_Init,
    VC_Lookup8_Exit,
    VC_Volcalc8_Stereo,
    VC_Volramp8_Stereo,
    Mix8StereoInterp_NoClick,
    Mix8SurroundInterp_NoClick,
    AsmStereoInterp,
    AsmSurroundInterp
};

void __cdecl AsmStereoInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{}

void __cdecl AsmSurroundInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{}

void __cdecl AsmMonoInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{}
