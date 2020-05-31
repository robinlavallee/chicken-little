
#ifndef _WRAP8_H_
#define _WRAP8_H_

#include "vchcrap.h"
 
typedef struct VOLCHAN8
{   int     vol, inc;
} VOLCHAN8;


typedef struct VOLINFO8
{   VOLCHAN8  left, right;
} VOLINFO8;

extern VOLINFO8   v8;
extern int      **voltab;
extern int      *lvoltab, *rvoltab;

extern BOOL __cdecl VC_Lookup8_Init(VMIXER *mixer);
extern void __cdecl VC_Lookup8_Exit(VMIXER *mixer);

void __cdecl VC_Volcalc8_Mono(VIRTCH *vc, VINFO *vnf);
void __cdecl VC_Volramp8_Mono(VINFO *vnf, int done);

void __cdecl VC_Volcalc8_Stereo(VIRTCH *vc, VINFO *vnf);
void __cdecl VC_Volramp8_Stereo(VINFO *vnf, int done);

void __cdecl Mix8MonoNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix8StereoNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix8SurroundNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);

void __cdecl Mix8MonoInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix8SurroundInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix8StereoInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);

void __cdecl Mix8StereoSSI_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix8MonoSSI_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);

void __cdecl Mix8StereoSS_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);
void __cdecl Mix8MonoSS_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo);

#endif
