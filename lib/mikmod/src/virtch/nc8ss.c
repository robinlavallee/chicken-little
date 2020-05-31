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
  Mix 8 bit *STEREO* (oh yea!) data with volume ramping.  These functions 
  use special global variables that differ fom the ones used by the normal 
  mixers, so that they do not interfere with the true volume of the sound.

  (see v8 extern in wrap16.h)

*/

#include "mikmod.h"
#include "wrap8.h"

void __cdecl Mix8StereoSS_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   v8.left.vol    += v8.left.inc;
        v8.right.vol   += v8.right.inc;

        *dest++ += voltab[v8.left.vol  / BIT8_TBLSCL][(UBYTE)srce[himacro(index)*2]];
        *dest++ += voltab[v8.right.vol / BIT8_TBLSCL][(UBYTE)srce[himacro(index)*2+1]];

        index  += increment;
    }
}

void __cdecl Mix8StereoSSI_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG sroot;

        v8.left.vol    += v8.left.inc;
        v8.right.vol   += v8.right.inc;

        sroot    = srce[himacro(index)*2];
        *dest++ += voltab[v8.left.vol / BIT8_TBLSCL][(UBYTE)(sroot + ((((SLONG)srce[(himacro(index)*2) + 2] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];

        sroot    = srce[himacro(index)*2];
        *dest++ += voltab[v8.right.vol / BIT8_TBLSCL][(UBYTE)(sroot + ((((SLONG)srce[(himacro(index)*2) + 3] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];

        index   += increment;
    }
}


void __cdecl Mix8MonoSS_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   v8.left.vol  += v8.left.inc;
        *dest         += voltab[v8.left.vol / BIT8_TBLSCL][(UBYTE)srce[himacro(index)*2]];
        *dest++       += voltab[v8.left.vol / BIT8_TBLSCL][(UBYTE)srce[himacro(index)*2+1]];
        index   += increment;
    }
}


void __cdecl Mix8MonoSSI_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)*2];
        v8.left.vol  += v8.left.inc;
        *dest         += voltab[v8.left.vol / BIT8_TBLSCL][(UBYTE)(sroot + ((((SLONG)srce[(himacro(index)*2) + 2] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];

        sroot = srce[(himacro(index)*2) + 1];
        *dest++ += voltab[v8.left.vol / BIT8_TBLSCL][(UBYTE)(sroot + ((((SLONG)srce[(himacro(index)*2) + 3] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))];
        index   += increment;
    }
}
  
