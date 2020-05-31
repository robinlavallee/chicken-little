 
#ifndef _WRAP16_H_
#define _WRAP16_H_

#include "vchcrap.h"
 
typedef struct VOLCHAN16
{
    int vol, inc;
} VOLCHAN16;


typedef struct VOLINFO16
{   VOLCHAN16  left, right;
} VOLINFO16;

extern VOLINFO16  v16;
extern int        lvolsel, rvolsel;

void __cdecl VC_Volcalc16_Mono(VIRTCH *vc, VINFO *vnf);
void __cdecl VC_Volramp16_Mono(VINFO *vnf, int done);

void __cdecl VC_Volcalc16_Stereo(VIRTCH *vc, VINFO *vnf);
void __cdecl VC_Volramp16_Stereo(VINFO *vnf, int done);

void __cdecl Mix16MonoNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix16StereoNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix16SurroundNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);

void __cdecl Mix16MonoInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix16SurroundInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix16StereoInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);

void __cdecl Mix16StereoSSI_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix16MonoSSI_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);

void __cdecl Mix16StereoSS_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix16MonoSS_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);


#endif
