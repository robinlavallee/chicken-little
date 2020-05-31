/*
 --> Image Loader API
  -> Divine Entertainment's VDRIVER

 Dated 1997-2000 by Jake Stine and Divine Entertainment

 File :  VLOADER.C

 Description:
   Image loader.  This module has file-type auto-detection and a common
   API for loading common image types.

 New Features:
   Relatively multi-thread friendly now.  Boy, I know you all are just jumping
   on the opportunity to use THAT feature!
*/


#include "vdriver.h"
#include "vloader.h"
#include <string.h>


static VL_IMAGELOADER *list_imgload = NULL;

/*void VL_InfoLoader(void)
{
    int t;
    VLOADER *l;

    // list all registered devicedrivers:
    for(t=1,l=firstloader; l!=NULL; l=l->next, t++)
        printf("%d. %s\n",t,l->version);
}*/


void VL_RegisterImageLoader(VL_IMAGELOADER *ldr)
{
    if(list_imgload == NULL)
    {   list_imgload = ldr;
        ldr->next    = NULL;
    } else
    {   ldr->next    = list_imgload;
        list_imgload = ldr;
    }
}


void VL_Free(IMAGE *img)
{
   if(img)
   {   if(img->bitmap)  free(img->bitmap);
       if(img->palette) free(img->palette);
       free(img);
   }
}


IMAGE *VL_LoadImageFP(MMSTREAM *fp)
{
    VL_IMAGELOADER *l;
    IMAGE   *mf;
    BOOL     ok;

    // Try to find a loader that recognizes the module
    for(l=list_imgload; l; l=l->next)
    {   _mm_rewind(fp);
        if(l->Test(fp)) break;
    }

    if(l == NULL)
    {   _mmerr_set(MMERR_UNSUPPORTED_FILE, "This is an unsupported image type or a corrupted file.");
        return NULL;
    }

    // We'll need some memory to load this thing into.
    if((mf=(IMAGE *)_mm_malloc(sizeof(IMAGE))) == NULL)
    {   VL_Free(mf);
        return NULL;
    }

    if(l->Init())
    {  _mm_rewind(fp);
       ok = l->Load(mf, fp);
    }

    // free loader allocations even if error!
    l->Cleanup();

    // In this case, the called procedure sets our error.
    if(!ok) return NULL;

    return mf;
}


IMAGE *VL_LoadImage(UBYTE *filename)
{
    MMSTREAM *fp;
    IMAGE    *mf;
    
    if((fp=_mm_fopen(filename,"rb"))==NULL) return NULL;

    mf = VL_LoadImageFP(fp);
    _mm_fclose(fp);

    return mf;
}


