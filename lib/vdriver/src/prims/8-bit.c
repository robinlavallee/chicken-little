
#include "vdriver.h"
#include <string.h>


/*******************
  line() -- Graphics Primitive

  It's slow, it's bulky.  But, no graphics library would be complete without 
  it.  This line routine, by name, should support all lines, not just 
  horizontal and vertical ones.  Maybe I'll do that some other day, if I or 
  anyone ever needs it (fat chance! ;).
*/

void gfx256_line(int xstart, int ystart, int xend, int yend, COLOR *color)
{
    register UBYTE  *scrn;
    register ULONG  len, col = color->color;

    if(ystart == yend)
    {   if(xstart > xend)
        {   len = (xstart - xend)+1;
            memset(MK_ScrnPtr(xend,ystart),col,len);
        } else
        {   len = (xend - xstart)+1;
            memset(MK_ScrnPtr(xstart,ystart),col,len);
        }
    } else
    {   if(ystart > yend)
        {   len  = (ystart - yend)+1;
            scrn = MK_ScrnPtr(xstart,yend);
        } else
        {   len  = (yend - ystart)+1;
            scrn = MK_ScrnPtr(xstart,ystart);
        }
        for(; len; len--, scrn+=vd_bytewidth) *scrn = col;
    }
}
            

/*******************
  h_line(), v_line() -- Graphics Primitives

  The speedy h_line() [horizontal line] and v_line [vertical line]
  specialized functions.  Due to it's simplicity, h_line has been made into
  a macro (check VDRIVER.H).
*/

void gfx256_hline(int xstart, int ystart, int ylen, COLOR *color)
{
    memset(MK_ScrnPtr(xstart,ystart),color->color,ylen);
}


void gfx256_vline(int xstart, int ystart, int ylen, COLOR *color)
{
   register UBYTE *scrn;
   register int col = color->color;
   
   scrn  = MK_ScrnPtr(xstart,ystart);
   for(ylen++; ylen; ylen--, scrn+=vd_physwidth) *scrn = col;
}

