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
 Module: nc16.c

 Description:
  Mix 16 bit data with volume ramping.  These functions use special global
  variables that differ fom the oes used by the normal mixers, so that they
  do not interfere with the true volume of the sound.

  (see v16 extern in wrap16.h)

*/

#include "mikmod.h"
#include "wrap16.h"

void __cdecl Mix16StereoNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   v16.left.vol    += v16.left.inc;
        v16.right.vol   += v16.right.inc;
        *dest++ += (v16.left.vol  / BIT16_VOLFAC) * srce[himacro(index)];
        *dest++ += (v16.right.vol / BIT16_VOLFAC) * srce[himacro(index)];
        index  += increment;
    }
}


void __cdecl Mix16StereoInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG  sroot  = srce[himacro(index)];
        register SWORD  sample = (SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        v16.left.vol    += v16.left.inc;
        v16.right.vol   += v16.right.inc;

        *dest++  += (v16.left.vol  / BIT16_VOLFAC) * sample;
        *dest++  += (v16.right.vol / BIT16_VOLFAC) * sample;
        index   += increment;
    }
}


void __cdecl Mix16SurroundNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG sample;

        v16.left.vol += v16.left.inc;
        sample        = (v16.left.vol / BIT16_VOLFAC) * srce[himacro(index)];

        *dest++ += sample;
        *dest++ -= sample;
        index   += increment;
    }
}


void __cdecl Mix16SurroundInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        v16.left.vol += v16.left.inc;
        sroot         = (v16.left.vol  / BIT16_VOLFAC) * (SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        *dest++ += sroot;
        *dest++ -= sroot;
        index  += increment;
    }
}


void __cdecl Mix16MonoNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    SLONG  sample;

    for(; todo; todo--)
    {   v16.left.vol  += v16.left.inc;
        sample         = (v16.left.vol  / BIT16_VOLFAC) * srce[himacro(index)];

        *dest++ += sample;
        index  += increment;
    }
}


void __cdecl Mix16MonoInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        v16.left.vol += v16.left.inc;
        sroot         = (v16.left.vol  / BIT16_VOLFAC) * (SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        *dest++ += sroot;
        index   += increment;
    }
}


 
