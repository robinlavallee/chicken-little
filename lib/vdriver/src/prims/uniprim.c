
#include "vdriver.h"
#include <string.h>

/*******************
  box(), fillbox(), rect() -- Graphics Primitives

  Utilizing the speedy h_line and v_line functions for doing simple boxes.
  box and fillbox differ from the box() and rect() functions that follow
  because these use box size and not screen coords.

  Note:  None of these functions perform any type checking on any level.
         Pass outrageous values at your own risk!

*/   

void gfx_box(const VD_SURFACE *vs, int xstart, int ystart, int xwidth, int ywidth, const VD_COLOR *color)
{
    vs->h_line(vs,xstart,ystart,xwidth,color);
    vs->h_line(vs,xstart,(ystart+ywidth)-1,xwidth,color);
    vs->v_line(vs,xstart,ystart,ywidth,color);
    vs->v_line(vs,(xstart+xwidth)-1,ystart,ywidth,color);
} 


void gfx_fillbox(const VD_SURFACE *vs, int xstart, int ystart, int xwidth, int ywidth, const VD_COLOR *color)
{
    ywidth = ystart + ywidth;
    for(; ystart < ywidth; ystart++)
        vs->h_line(vs,xstart, ystart, xwidth, color);
}

 
void gfx_rect(const VD_SURFACE *vs, int xstart, int ystart, int xend, int yend, const VD_COLOR *color)
{
    ULONG tmp;

    if(xstart > xend)  {  tmp = xstart; xstart = xend; xend = tmp;  }
    xend = xend - xstart + 1;
    if(ystart > yend)  {  tmp = ystart; ystart = yend; yend = tmp;  }

    for(; ystart < yend; ystart++)
        vs->h_line(vs,xstart, ystart, xend, color);
} 

