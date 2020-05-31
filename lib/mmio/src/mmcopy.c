/*
 Mikmod Portable Memory Allocation

 By Jake Stine of Divine Entertainment (1996-2000) and
    Jean-Paul Mikkers (1993-1996).

 File: mmcopy.c

 Description:

*/

#include "mmio.h"
#include <string.h>

void _mm_memcpy_long(void *dst, const void *src, int count)
{
    register ULONG *dest = (ULONG *)dst, *srce = (ULONG *)src, i;
    for(i=count; i; i--, dest++, srce++) *dest = *srce;
}


void _mm_memcpy_word(void *dst, const void *src, int count)
{
    memcpy(dst, src, count*2);
}

void _mm_memcpy_quad(void *dst, const void *src, int count)
{
    register ULONG *dest = (ULONG *)dst, *srce = (ULONG *)src, i;
    for(i=count*2; i; i--, dest++, srce++) *dest = *srce;
}


// Same as Watcom's strdup function - allocates memory for a string
// and makes a copy of it.  Ruturns NULL if failed.
CHAR *_mm_strdup(const CHAR *src)
{
    CHAR *buf;

    if(!src) return NULL;
    
    if((buf = (CHAR *)_mm_malloc(strlen(src)+1)) == NULL) return NULL;

    strcpy(buf,src);
    return buf;
}

void *_mm_structdup(const void *src, size_t size)
{
    void *tribaldance;
    
    if(!src) return NULL;
    
    tribaldance = _mm_malloc(size);
    if(tribaldance) memcpy(tribaldance, src, size);
    
    return tribaldance;
}
