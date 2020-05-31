; Mikmod Sound System
;  Intel (486) Optimized 8 bit mixers.
;
; These mixers are indeed optimized for 486's still.  When I retarget
; them toward Pentium class CPUs, I will simply remove an extra mov
; step done for each 32-bit buffer add (which avoids indexing penalties
; on 486's).
;
; Otherwise, every instruction in this code pipelines except for the
; mul needed to do interpolation.  Optimizing for Pentiums would only save,
; at best, two or three cycles per sample.

.386p
.model flat
.data ; self-modifying code... keep in data segment

        NAME    resample


        EXTRN   _rvoltab:DWORD
        EXTRN   _lvoltab:DWORD

        PUBLIC  _AsmStereoNormal
        PUBLIC  _AsmStereoInterp
        PUBLIC  _AsmSurroundNormal
        PUBLIC  _AsmSurroundInterp
        PUBLIC  _AsmMonoNormal
        PUBLIC  _AsmMonoInterp


STUBSTART macro
  push eax
  push ebx
  push ecx
  push edx
  push esi
  push edi
  push ebp
  endm


STUBEND macro
  pop ebp
  pop edi
  pop esi
  pop edx
  pop ecx
  pop ebx
  pop eax
  endm

; -----------------------------------------

SS2F MACRO index,lab1,lab2,lab3,lab4 ; 486+
 mov al,[edx]
 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi
 mov esi,(index*8)[edi]
 db 08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]
 mov ebp,(index*8+4)[edi]
 add esi,ecx
 db 08bh,0ch,085h
lab4 dd 0 ; mov ecx,[eax*4+table2]
 mov (index*8)[edi],esi
 add ebp,ecx
 ENDM

SS2M MACRO index,lab1,lab2,lab3,lab4 ; 486+
 mov al,[edx]
 mov (index*8-4)[edi],ebp
 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi
 mov esi,(index*8)[edi]
 db 08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]
 mov ebp,(index*8+4)[edi]
 add esi,ecx
 db 08bh,0ch,085h
lab4 dd 0 ; mov ecx,[eax*4+table2]
 mov (index*8)[edi],esi
 add ebp,ecx
 ENDM

SS3F MACRO index,lab1,lab2,lab3 ; 486+
 mov al,[edx]
 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi
 mov esi,(index*8)[edi]
 db 08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]
 mov ebp,(index*8+4)[edi]
 add esi,ecx
 sub ebp,ecx
 mov (index*8)[edi],esi
 ENDM

SS3M MACRO index,lab1,lab2,lab3 ; 486+
 mov al,[edx]
 mov (index*8-4)[edi],ebp
 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi
 mov esi,(index*8)[edi]
 db 08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]
 mov ebp,(index*8+4)[edi]
 add esi,ecx
 sub ebp,ecx
 mov (index*8)[edi],esi
 ENDM

SS2L MACRO index ; 486+
 mov (index*8-4)[edi],ebp
 ENDM

; -----------------------------------------
;        Stereo Interpolation Macros
; -----------------------------------------

IS2F MACRO index,lab1,lab2,lab3,lab4 ; 486+
 movsx  eax,byte ptr +1H[edx]
 movsx  ecx,byte ptr[edx]
  
 mov    esi,ebx
 sub    eax,ecx       ; next src - src
 shr    esi,16
 imul   eax,esi       ; result * frac
 sar    eax,16        ; shift off the fixed point portion

 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi

 add    eax,ecx       ; eax = src + ((next src - src) * fractional)
 and    eax,0ffh

 mov    esi,(index*8)[edi]    ; get left channel mix

 db 08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]

 mov    ebp,(index*8+4)[edi]  ; get right channel mix
 add    esi,ecx

 db 08bh,0ch,085h
lab4 dd 0 ; mov ecx,[eax*4+table2]

 mov    (index*8)[edi],esi
 add    ebp,ecx
ENDM

IS2M MACRO index,lab1,lab2,lab3,lab4 ; 486+
 movsx  eax,byte ptr +1H[edx]
 movsx  ecx,byte ptr[edx]

 mov    (index*8-4)[edi],ebp  
 mov    esi,ebx
 sub    eax,ecx       ; next src - src
 shr    esi,16

 imul   eax,esi       ; result * frac
 sar    eax,16        ; shift off the fixed point portion

 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi

 add    eax,ecx       ; eax = src + ((next src - src) * fractional)
 and    eax,0ffh

 mov esi,(index*8)[edi]   ; get left channel mix

 db 08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]

 mov ebp,(index*8+4)[edi] ; get  right channel mix
 add esi,ecx

 db 08bh,0ch,085h
lab4 dd 0 ; mov ecx,[eax*4+table2]

 mov (index*8)[edi],esi
 add ebp,ecx
ENDM

IS2L MACRO index ; 486+
 mov (index*8-4)[edi],ebp
 ENDM


; -----------------------------------------
;       Surround Interpolation Macros
; -----------------------------------------

IS3F MACRO index,lab1,lab2,lab3 ; 486+
 movsx  eax,byte ptr +1H[edx]
 movsx  ecx,byte ptr[edx]
  
 sub    eax,ecx       ; next src - src
 mov    esi,ebx
 shr    esi,16
 imul   eax,esi       ; result * frac
 sar    eax,16        ; shift off the fixed point portion

 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi

 add    eax,ecx       ; eax = src + ((next src - src) * fractional)
 and    eax,0ffh

 mov esi,(index*8)[edi]

 db 08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]

 mov ebp,(index*8+4)[edi]
 add esi,ecx
 sub ebp,ecx
 mov (index*8)[edi],esi
 ENDM

IS3M MACRO index,lab1,lab2,lab3 ; 486+
 movsx  eax,byte ptr +1H[edx]
 movsx  ecx,byte ptr[edx]

 mov   (index*8-4)[edi],ebp  
 mov    esi,ebx
 sub    eax,ecx       ; next src - src
 shr    esi,16
 imul   eax,esi       ; result * frac
 sar    eax,16        ; shift off the fixed point portion

 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi

 add    eax,ecx       ; eax = src + ((next src - src) * fractional)
 and    eax,0ffh

 mov esi,(index*8)[edi]
 db 08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]
 mov ebp,(index*8+4)[edi]
 add esi,ecx
 sub ebp,ecx
 mov (index*8)[edi],esi
 ENDM


; -----------------------------------------
;            Mono Standard Macros
; -----------------------------------------

SM2F MACRO index,lab1,lab2,lab3 ; 486+
 mov  al,[edx] ; AGI-3
 db   081h,0c3h
lab1 dd 0 ; add ebx,lo
 db   081h,0d2h
lab2 dd 0 ; adc edx,hi
 mov  esi,(index*4)[edi]
 db   08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]
 ENDM

SM2M MACRO index,lab1,lab2,lab3 ; 486+
 mov  al,[edx] ; AGI-3
 add  esi,ecx
 db   081h,0c3h
lab1 dd 0 ; add ebx,lo
 mov  (index*4-4)[edi],esi
 db   081h,0d2h
lab2 dd 0 ; adc edx,hi
 mov  esi,(index*4)[edi]
 db   08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]
 ENDM

SM2L MACRO index ; 486+
 add  esi,ecx
 mov  (index*4-4)[edi],esi
 ENDM


; -----------------------------------------
;         Mono Interpolation Macros
; -----------------------------------------

IM2F MACRO index,lab1,lab2,lab3 ; 486+
 movsx  eax,byte ptr +1H[edx]
 movsx  ecx,byte ptr[edx]
  
 sub    eax,ecx       ; next src - src
 mov    esi,ebx
 shr    esi,16
 imul   eax,esi       ; result * frac
 sar    eax,16        ; shift off the fixed point portion

 db   081h,0c3h
lab1 dd 0       ; add ebx,lo  [increment lo to index]
 db   081h,0d2h
lab2 dd 0       ; adc edx,hi  [increment hi to index]

 add    eax,ecx       ; eax = src + ((next src - src) * fractional)
 and    eax,0ffh

 mov  esi,(index*4)[edi]
 db   08bh,0ch,085h
lab3 dd 0       ; mov ecx,[eax*4+table1]
 add    esi,ecx
 ENDM

IM2M MACRO index,lab1,lab2,lab3 ; 486+
 movsx  eax,byte ptr +1H[edx]
 movsx  ecx,byte ptr [edx]

 mov    (index*4-4)[edi],esi
 mov    esi,ebx
 sub    eax,ecx       ; next src - src
 shr    esi,16
 imul   eax,esi       ; result * frac
 sar    eax,16        ; shift off the fixed point portion

 db   081h,0c3h
lab1 dd 0 ; add ebx,lo
 db   081h,0d2h
lab2 dd 0 ; adc edx,hi

 add    eax,ecx       ; eax = src + ((next src - src) * fractional)
 and    eax,0ffh

 mov    esi,(index*4)[edi]
 db   08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]
 add    esi,ecx
 ENDM

IM2L MACRO index ; 486+
 mov    (index*4-4)[edi],esi
 ENDM


; -----------------------------------------------
; ------- Actual Code Stuffs starts HERE! -------
; -----------------------------------------------

_AsmStereoNormal:
        STUBSTART
        mov    esi,[esp+32] ; get src
        mov    edi,[esp+36] ; get dst
        mov    ebx,[esp+40] ; get index_lo
        mov    edx,[esp+44] ; get index_hi
        mov    ecx,[esp+48] ; get increment_lo
        mov    eax,[esp+52] ; get increment_hi
        mov    ebp,[esp+56] ; get todo

        mov    shi1,eax
        mov    shi2,eax
        mov    shi3,eax
        mov    shi4,eax
        mov    shi5,eax
        add    edx,esi
        mov    eax,_lvoltab
        mov    slo1,ecx
        mov    slo2,ecx
        mov    slo3,ecx
        mov    slo4,ecx
        mov    slo5,ecx
        mov    sltab1,eax
        mov    sltab2,eax
        mov    sltab3,eax
        mov    sltab4,eax
        mov    ecx,_rvoltab
        mov    sltab5,eax
        mov    srtab1,ecx
        mov    srtab2,ecx
        mov    srtab3,ecx
        mov    srtab4,ecx
        mov    srtab5,ecx

        push   ebp
        xor    eax,eax
        jmp    s1 ; flush code cache
s1:
        shr    ebp,2
        jz     sskip16
        push   ebp
sagain16:
        SS2F   0,slo1,shi1,sltab1,srtab1
        SS2M   1,slo2,shi2,sltab2,srtab2
        SS2M   2,slo3,shi3,sltab3,srtab3
        SS2M   3,slo4,shi4,sltab4,srtab4
        SS2L   4
        add    edi,(4*8)
        dec    dword ptr [esp]
        jnz    sagain16
        pop    ebp
sskip16:
        pop    ebp
        and    ebp,3
        jz     sskip1
        push   ebp
sagain1:
        SS2F   0,slo5,shi5,sltab5,srtab5
        SS2L   1
        add    edi,8
        dec    dword ptr [esp]
        jnz    sagain1
        pop    ebp
sskip1:
        STUBEND
        ret


_AsmStereoInterp:
        STUBSTART
        mov    esi,[esp+32] ; get src
        mov    edi,[esp+36] ; get dst
        mov    ebx,[esp+40] ; get index_lo
        mov    edx,[esp+44] ; get index_hi
        mov    ecx,[esp+48] ; get increment_lo
        mov    eax,[esp+52] ; get increment_hi
        mov    ebp,[esp+56] ; get todo

        mov    sihi1,eax
        mov    sihi2,eax
        mov    sihi3,eax
        mov    sihi4,eax
        mov    sihi5,eax
        add    edx,esi
        mov    eax,_lvoltab
        mov    silo1,ecx
        mov    silo2,ecx
        mov    silo3,ecx
        mov    silo4,ecx
        mov    silo5,ecx
        mov    slitab1,eax
        mov    slitab2,eax
        mov    slitab3,eax
        mov    slitab4,eax
        mov    ecx,_rvoltab
        mov    slitab5,eax
        mov    sritab1,ecx
        mov    sritab2,ecx
        mov    sritab3,ecx
        mov    sritab4,ecx
        mov    sritab5,ecx

        push   ebp
        jmp    si1 ; flush code cache
si1:
        shr    ebp,2
        jz     siskip16
        push   ebp
siagain16:
        IS2F   0,silo1,sihi1,slitab1,sritab1
        IS2M   1,silo2,sihi2,slitab2,sritab2
        IS2M   2,silo3,sihi3,slitab3,sritab3
        IS2M   3,silo4,sihi4,slitab4,sritab4
        IS2L   4
        add    edi,(4*8)
        dec    dword ptr [esp]
        jnz    siagain16
        pop    ebp
siskip16:
        pop    ebp
        and    ebp,3
        jz     siskip1
        push   ebp
siagain1:
        IS2F   0,silo5,sihi5,slitab5,sritab5
        IS2L   1
        add    edi,8
        dec    dword ptr [esp]
        jnz    siagain1
        pop    ebp
siskip1:
        STUBEND
        ret


_AsmSurroundNormal:
        STUBSTART
        mov    esi,[esp+32] ; get src
        mov    edi,[esp+36] ; get dst
        mov    ebx,[esp+40] ; get index_lo
        mov    edx,[esp+44] ; get index_hi
        mov    ecx,[esp+48] ; get increment_lo
        mov    eax,[esp+52] ; get increment_hi
        mov    ebp,[esp+56] ; get todo

        add    edx,esi
        mov    s2hi1,eax
        mov    s2hi2,eax
        mov    s2hi3,eax
        mov    s2hi4,eax
        mov    s2hi5,eax
        mov    eax,_lvoltab
        mov    s2lo1,ecx
        mov    s2lo2,ecx
        mov    s2lo3,ecx
        mov    s2lo4,ecx
        mov    s2lo5,ecx
        mov    s2ltab1,eax
        mov    s2ltab2,eax
        mov    s2ltab3,eax
        mov    s2ltab4,eax
        mov    s2ltab5,eax
        push   ebp
        xor    eax,eax
        jmp    s3 ; flush code cache
s3:
        shr    ebp,2
        jz     s2skip16
        push   ebp
s2again16:
        SS3F   0,s2lo1,s2hi1,s2ltab1  ;,s2rtab1
        SS3M   1,s2lo2,s2hi2,s2ltab2  ;,s2rtab2
        SS3M   2,s2lo3,s2hi3,s2ltab3  ;,s2rtab3
        SS3M   3,s2lo4,s2hi4,s2ltab4  ;,s2rtab4
        SS2L   4
        add    edi,(4*8)
        dec    dword ptr [esp]
        jnz    s2again16
        pop    ebp
s2skip16:
        pop    ebp
        and    ebp,3
        jz     s2skip1
        push   ebp
s2again1:
        SS3F   0,s2lo5,s2hi5,s2ltab5  ;,s2rtab5
        SS2L   1
        add    edi,8
        dec    dword ptr [esp]
        jnz    s2again1
        pop    ebp
s2skip1:
        STUBEND
        ret


_AsmSurroundInterp:
        STUBSTART
        mov    esi,[esp+32] ; get src
        mov    edi,[esp+36] ; get dst
        mov    ebx,[esp+40] ; get index_lo
        mov    edx,[esp+44] ; get index_hi
        mov    ecx,[esp+48] ; get increment_lo
        mov    eax,[esp+52] ; get increment_hi
        mov    ebp,[esp+56] ; get todo

        add    edx,esi
        mov    si2hi1,eax
        mov    si2hi2,eax
        mov    si2hi3,eax
        mov    si2hi4,eax
        mov    si2hi5,eax
        mov    eax,_lvoltab
        mov    si2lo1,ecx
        mov    si2lo2,ecx
        mov    si2lo3,ecx
        mov    si2lo4,ecx
        mov    si2lo5,ecx
        mov    sli2tab1,eax
        mov    sli2tab2,eax
        mov    sli2tab3,eax
        mov    sli2tab4,eax
        mov    sli2tab5,eax
        push   ebp
        xor    eax,eax
        jmp    si3 ; flush code cache
sI3:
        shr    ebp,2
        jz     si2skip16
        push   ebp
si2again16:
        IS3F   0,si2lo1,si2hi1,sli2tab1
        IS3M   1,si2lo2,si2hi2,sli2tab2
        IS3M   2,si2lo3,si2hi3,sli2tab3
        IS3M   3,si2lo4,si2hi4,sli2tab4
        IS2L   4
        add    edi,(4*8)
        dec    dword ptr [esp]
        jnz    si2again16
        pop    ebp
si2skip16:
        pop    ebp
        and    ebp,3
        jz     si2skip1
        push   ebp
si2again1:
        IS3F   0,si2lo5,si2hi5,sli2tab5
        IS2L   1
        add    edi,8
        dec    dword ptr [esp]
        jnz    si2again1
        pop    ebp
sI2skip1:
        STUBEND
        ret


_AsmMonoNormal:
        STUBSTART
        mov    esi,[esp+32] ; get src
        mov    edi,[esp+36] ; get dst
        mov    ebx,[esp+40] ; get index_lo
        mov    edx,[esp+44] ; get index_hi
        mov    ecx,[esp+48] ; get increment_lo
        mov    eax,[esp+52] ; get increment_hi
        mov    ebp,[esp+56] ; get todo

        mov    mhi1,eax
        mov    mhi2,eax
        mov    mhi3,eax
        mov    mhi4,eax
        mov    mhi5,eax
        add    edx,esi
        mov    eax,_lvoltab
        mov    mlo1,ecx
        mov    mlo2,ecx
        mov    mlo3,ecx
        mov    mlo4,ecx
        mov    mlo5,ecx
        mov    mltab1,eax
        mov    mltab2,eax
        mov    mltab3,eax
        mov    mltab4,eax
        mov    mltab5,eax

        jmp    m1 ; flush code cache
m1:
        push   ebp
        shr    ebp,2
        jz     mskip16
magain16:
        SM2F   0,mlo1,mhi1,mltab1
        SM2M   1,mlo2,mhi2,mltab2
        SM2M   2,mlo3,mhi3,mltab3
        SM2M   3,mlo4,mhi4,mltab4
        SM2L   4
        add    edi,(4*4)
        dec    ebp
        jnz    magain16
mskip16:
        pop    ebp
        and    ebp,3
        jz     mskip1
magain1:
        SM2F   0,mlo5,mhi5,mltab5
        SM2L   1
        add    edi,4
        dec    ebp
        jnz    magain1
mskip1:
        STUBEND
        ret


_AsmMonoInterp:
        STUBSTART
        mov    esi,[esp+32] ; get src
        mov    edi,[esp+36] ; get dst
        mov    ebx,[esp+40] ; get index_lo
        mov    edx,[esp+44] ; get index_hi
        mov    ecx,[esp+48] ; get increment_lo
        mov    eax,[esp+52] ; get increment_hi
        mov    ebp,[esp+56] ; get todo

        mov    mihi1,eax
        mov    mihi2,eax
        mov    mihi3,eax
        mov    mihi4,eax
        mov    mihi5,eax
        add    edx,esi
        mov    eax,_lvoltab
        mov    milo1,ecx
        mov    milo2,ecx
        mov    milo3,ecx
        mov    milo4,ecx
        mov    milo5,ecx
        mov    miltab1,eax
        mov    miltab2,eax
        mov    miltab3,eax
        mov    miltab4,eax
        mov    miltab5,eax

        xor    eax,eax
        jmp    mi1 ; flush code cache
mi1:
        push   ebp
        shr    ebp,2
        jz     miskip16
miagain16:
        IM2F   0,milo1,mihi1,miltab1
        IM2M   1,milo2,mihi2,miltab2
        IM2M   2,milo3,mihi3,miltab3
        IM2M   3,milo4,mihi4,miltab4
        IM2L   4
        add    edi,(4*4)
        dec    ebp
        jnz    miagain16
miskip16:
        pop    ebp
        and    ebp,3
        jz     miskip1
miagain1:
        IM2F   0,milo5,mihi5,miltab5
        IM2L   1
        add    edi,4
        dec    ebp
        jnz    miagain1
miskip1:
        STUBEND
        ret

        END

