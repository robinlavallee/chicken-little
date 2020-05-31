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
 Module: nc8.c

 Description:
  Mix 8 bit data with volume ramping.  These functions use special global
  variables that differ fom the oes used by the normal mixers, so that they
  do not interfere with the true volume of the sound.

  (see v8 extern in wrap16.h)

*/

#include "mikmod.h"
#include "wrap8.h"


// =====================================================================================
    void __cdecl Mix8StereoNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
// =====================================================================================
{
    for(; todo; todo--)
    {   v8.left.vol    += v8.left.inc;
        v8.right.vol   += v8.right.inc;

        *dest++ += voltab[v8.left.vol  / BIT8_TBLSCL][(UBYTE)srce[himacro(index)]];
        *dest++ += voltab[v8.right.vol / BIT8_TBLSCL][(UBYTE)srce[himacro(index)]];

        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Mix8StereoInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
// =====================================================================================
{
    UBYTE  sample;

    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        sample = (UBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        v8.left.vol    += v8.left.inc;
        v8.right.vol   += v8.right.inc;

        *dest++ += voltab[v8.left.vol  / BIT8_TBLSCL][sample];
        *dest++ += voltab[v8.right.vol / BIT8_TBLSCL][sample];

        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Mix8SurroundNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
// =====================================================================================
{
    SLONG  sample;

    for(; todo; todo--)
    {   v8.left.vol  += v8.left.inc;
        sample        = voltab[v8.left.vol / BIT8_TBLSCL][(UBYTE)srce[himacro(index)]];

        *dest++ += sample;
        *dest++ -= sample;
        index   += increment;
    }
}

// =====================================================================================
    void __cdecl Mix8SurroundInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        v8.left.vol += v8.left.inc;
        sroot        = voltab[v8.left.vol / BIT8_TBLSCL][(UBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];

        *dest++ += sroot;
        *dest++ -= sroot;
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Mix8MonoNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
// =====================================================================================
{
    SLONG  sample;

    for(; todo; todo--)
    {   sample  = voltab[v8.left.vol / BIT8_TBLSCL][(UBYTE)srce[himacro(index)]];

        v8.left.vol += v8.left.inc;
        *dest++     += sample;
        index       += increment;
    }
}


// =====================================================================================
    void __cdecl Mix8MonoInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
// =====================================================================================
{
    SLONG  sample;

    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        sample = voltab[v8.left.vol / BIT8_TBLSCL][(UBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];

        v8.left.vol += v8.left.inc;
        *dest++     += sample;
        index       += increment;
    }
}
  
