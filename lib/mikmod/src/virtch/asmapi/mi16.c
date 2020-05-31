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
 Module: mi16.c

  Assembly Mixer Plugin : Mono 16 bit Sample data / Interpolation

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
#include "..\wrap16.h"
#include "asmapi.h"

VMIXER ASM_M16_MONO_INTERP =
{   NULL,

    "Assembly Mono-16 (Mono/Interp) v0.1",

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
    Asm16MonoInterp,
    NULL,
};

VMIXER ASM_M16_STEREO_INTERP =
{   NULL,

    "Assembly Mono-16 (Stereo/Interp) v0.1",

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
    Asm16StereoInterp,
    Asm16SurroundInterp,
}; 

void __cdecl Asm16StereoInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo) {}
void __cdecl Asm16SurroundInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo) {}
void __cdecl Asm16MonoInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo) {}
