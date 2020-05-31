/*
 The VDRIVER - A Classic Video Game Engine

  By Jake Stine and Divine Entertainment (1997-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org
  For additional information and updates, see our website:
    http://www.divent.org

 Disclaimer:
  I could put things here that would make me sound like a first-rate california
  prune.  But I simply can't think of them right now.  In other news, I reserve
  the right to say I wrote my code.  All other rights are just abused legal
  tender in a world of red tape.  Have fun!

 ---------------------------------------------------
 Module: rect32.c

 CPU Target: None (Pentium)

*/

#include "vdriver.h"
#include <string.h>

#ifndef BYTESPP
#define BYTESPP 4
#endif

#include "prims.h"


// =====================================================================================
    void gfx32_shadowrect(const VD_SURFACE *vs, const VRECT *rect, uint mode)
// =====================================================================================
// Renders the rectangle area to either 25, 50, or 75% opacity.
{

    UCAST      *ttmp;
    int         xsize, ysize;
    
    {

    // the following three lines of code ensure our rectangle is completely
    // confined to the area of the given surface!  I enclosed this code in braces to
    // give the compiler hints to the scope of the xloc and yloc variables, which
    // may help some compiler topmize them to registers more efficiently!
    
    register int xloc = ((rect->left < 0) ? 0 : rect->left);
    register int yloc = ((rect->top < 0) ? 0 : rect->top);

    xsize = ((rect->right > (int)vs->xsize) ? vs->xsize : rect->right) - xloc;
    ysize = ((rect->bottom > (int)vs->ysize) ? vs->ysize : rect->bottom) - yloc;

    if((xsize <=0) || (ysize <= 0)) return;

    ttmp = (UCAST *)MK_ScrnPtr(xloc*BYTESPP, yloc);

    }

    switch(mode)
    {
        case VD_ALPHA_0:
            // This would simply display black
            // [...]
        break;
        
        case VD_ALPHA_25:
            do
            {   uint   t;
                for(t=xsize; t; t--)
                {   uint   k = (ttmp[t] & vs->alphamask[0]) >> 1;
                    ttmp[t] = k + ((k & vs->alphamask[0]) >> 1);
                }
                ttmp += vs->physwidth;
            } while(--ysize);
        break;

        case VD_ALPHA_50:
            do
            {   uint   t;
                for(t=xsize; t; t--)
                    ttmp[t] = (ttmp[t] & vs->alphamask[0]) >> 1;
                ttmp += vs->physwidth;
            } while(--ysize);
        break;

        case VD_ALPHA_75:
            do
            {   uint   t;
                for(t=xsize; t; t--)
                    ttmp[t] = ((ttmp[t] & vs->alphamask[1]) >> 2);
                ttmp += vs->physwidth;
            } while(--ysize);
        break;

        case VD_ALPHA_100:
            // This would simply display the image
            // [...]
        break;
    }

}
