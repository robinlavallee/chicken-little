; Mikmod Sound System
;  Intel (486) Optimized 8 bit STEREO SAMPLE mixers.
;
; These are just the the ones in resample.asm, only they mix stereo samples.
; Note that there is no support for surround sound mixers with these.  Surround
; sound is lame anyway, so get over it!

.386p
.model flat
.data ; self-modifying code... keep in data segment

        NAME    resample8stereo


        EXTRN   _rvoltab:DWORD
        EXTRN   _lvoltab:DWORD
        EXTRN   _lvolsel:DWORD

        PUBLIC  _AsmStereoSS
        PUBLIC  _AsmStereoSSI
        PUBLIC  _AsmMonoSS
        PUBLIC  _AsmMonoSSI


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
 mov al,[edx+edx]
 mov si,[(edx+edx) + 1]

 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi

 db 08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]
 db 08bh,2ch,0b5h
lab4 dd 0 ; mov ebp,[esi*4+table2]

 add (index*8)[edi],ecx
 add (index*8+4)[edi],ebp
 ENDM
 

; -----------------------------------------
;        Stereo Interpolation Macros
; -----------------------------------------

IS2F MACRO index,lab1,lab2,lab3,lab4 ; 486+
 movsx  eax,byte ptr +2H[edx+edx]
 movsx  ecx,byte ptr [edx+edx]
 movsx  esi,byte ptr +3H[edx+edx]
 movsx  ebp,byte ptr +1H[edx+edx]
  
 sub    esi,ebp       ; next src - src
 sub    eax,ecx
 mov    -2H[idxshft],ebx ; load idxshft andshift it right 16 bits!

 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi

 imul   eax,[idxshft] ; result * frac
 imul   esi,[idxshft] ; result * frac

 sar    eax,16        ; shift off the fixed point portion
 sar    esi,16        ; shift off the fixed point portion

 add    eax,ecx       ; eax = src + ((next src - src) * fractional)
 add    esi,ebp
 and    eax,0ffh
 and    esi,0ffh

 db 08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]
 db 08bh,2ch,0b5h
lab4 dd 0 ; mov ebp,[esi*4+table2]

 add    (index*8)[edi],ecx
 add    (index*8+4)[edi],ebp
ENDM


; -----------------------------------------
;            Mono Standard Macros
; -----------------------------------------

SM2F MACRO index,lab1,lab2,lab3 ; 486+
 mov   al,[edx+edx]
 mov   si,[edx+edx + 1]

 add   eax, esi       ; merge stereo : (lsample + rsample) / 2
 sar   eax, 1 

 db   081h,0c3h
lab1 dd 0 ; add ebx,lo
 db   081h,0d2h
lab2 dd 0 ; adc edx,hi

 db   08bh,0ch,085h
lab3 dd 0 ; mov ecx,[eax*4+table1]

 add (index*4)[edi],ecx
 ENDM

; -----------------------------------------
;         Mono Interpolation Macros
; -----------------------------------------

IM2F MACRO index,lab1,lab2 ; 486+
 movsx  eax,byte ptr +2H[edx+edx]
 movsx  ecx,byte ptr [edx+edx]
 movsx  esi,byte ptr +3H[edx+edx]
 movsx  ebp,byte ptr +1H[edx+edx]
  
 sub    esi,ebp       ; next src - src
 sub    eax,ecx
 mov    -2H[idxshft],ebx ; load idxshft andshift it right 16 bits!

 imul   eax,[idxshft] ; result * frac
 imul   esi,[idxshft] ; result * frac

 sar    eax,16        ; shift off the fixed point portion
 sar    esi,16        ; shift off the fixed point portion

 add    eax,ecx       ; eax = src + ((next src - src) * fractional)
 add    esi,ebp
 
 add    eax,esi       ; combine samples (lsample + rsample) / 2

 db 081h,0c3h
lab1 dd 0 ; add ebx,lo
 db 081h,0d2h
lab2 dd 0 ; adc edx,hi

 sar    eax,1
 imul   eax,[_lvolsel]
 add    (index*4)[edi],eax

 ENDM


; ------------------------------------------
; Data used by interpolation mixers only...
; ------------------------------------------

        dd   0       ; Extra room because we do weird things!
idxshft dd   0       ; index shift memory space!


; -----------------------------------------------
; ------- Actual Code Stuffs starts HERE! -------
; -----------------------------------------------

_AsmStereoSS:
        STUBSTART
        mov    esi,[esp+32] ; get src
        mov    edi,[esp+36] ; get dst
        mov    ebx,[esp+40] ; get index_lo
        mov    edx,[esp+44] ; get index_hi
        mov    ecx,[esp+48] ; get increment_lo
        mov    eax,[esp+52] ; get increment_hi
        mov    ebp,[esp+56] ; get todo

        shl    edx,1
        mov    shi1,eax
        add    edx,esi
        mov    shi2,eax
        mov    shi3,eax
        mov    shi4,eax
        mov    shi5,eax
        sar    edx,1
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

        xor    esi,esi
        xor    eax,eax

        push   ebp
        jmp    s1 ; flush code cache
s1:
        shr    ebp,2
        jz     sskip16
        push   ebp
sagain16:
        SS2F   0,slo1,shi1,sltab1,srtab1
        SS2F   1,slo2,shi2,sltab2,srtab2
        SS2F   2,slo3,shi3,sltab3,srtab3
        SS2F   3,slo4,shi4,sltab4,srtab4
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
        add    edi,8
        dec    dword ptr [esp]
        jnz    sagain1
        pop    ebp
sskip1:
        STUBEND
        ret


_AsmStereoSSI:
        STUBSTART
        mov    esi,[esp+32] ; get src
        mov    edi,[esp+36] ; get dst
        mov    ebx,[esp+40] ; get index_lo
        mov    edx,[esp+44] ; get index_hi
        mov    ecx,[esp+48] ; get increment_lo
        mov    eax,[esp+52] ; get increment_hi
        mov    ebp,[esp+56] ; get todo

        shl    edx,1
        mov    sihi1,eax
        add    edx,esi
        mov    sihi2,eax
        mov    sihi3,eax
        mov    sihi4,eax
        mov    sihi5,eax
        sar    edx,1
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
        IS2F   1,silo2,sihi2,slitab2,sritab2
        IS2F   2,silo3,sihi3,slitab3,sritab3
        IS2F   3,silo4,sihi4,slitab4,sritab4
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
        add    edi,8
        dec    dword ptr [esp]
        jnz    siagain1
        pop    ebp
siskip1:
        STUBEND
        ret


_AsmMonoSS:
        STUBSTART
        mov    esi,[esp+32] ; get src
        mov    edi,[esp+36] ; get dst
        mov    ebx,[esp+40] ; get index_lo
        mov    edx,[esp+44] ; get index_hi
        mov    ecx,[esp+48] ; get increment_lo
        mov    eax,[esp+52] ; get increment_hi
        mov    ebp,[esp+56] ; get todo

        shl    edx,1
        mov    mhi1,eax
        add    edx,esi
        mov    mhi2,eax
        mov    mhi3,eax
        mov    mhi4,eax
        mov    mhi5,eax
        sar    edx,1
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
        mov    mltab9,eax

        xor    esi,esi
        xor    eax,eax

        push   ebp
        jmp    m1 ; flush code cache
m1:
        shr    ebp,2
        jz     mskip16
magain16:
        SM2F   0,mlo1,mhi1,mltab1
        SM2F   1,mlo2,mhi2,mltab2
        SM2F   2,mlo3,mhi3,mltab3
        SM2F   3,mlo4,mhi4,mltab4
        add    edi,(4*4)
        dec    ebp
        jnz    magain16
mskip16:
        pop    ebp
        and    ebp,3
        jz     mskip1
magain1:
        SM2F   0,mlo5,mhi5,mltab9
        add    edi,4
        dec    ebp
        jnz    magain1
mskip1:
        STUBEND
        ret


_AsmMonoSSI:
        STUBSTART
        mov    esi,[esp+32] ; get src
        mov    edi,[esp+36] ; get dst
        mov    ebx,[esp+40] ; get index_lo
        mov    edx,[esp+44] ; get index_hi
        mov    ecx,[esp+48] ; get increment_lo
        mov    eax,[esp+52] ; get increment_hi
        mov    ebp,[esp+56] ; get todo

        shl    edx,1
        mov    mihi1,eax
        add    edx,esi
        mov    mihi2,eax
        sar    edx,1
        mov    mihi3,eax
        mov    mihi4,eax
        mov    mihi5,eax
        mov    milo1,ecx
        mov    milo2,ecx
        mov    milo3,ecx
        mov    milo4,ecx
        mov    milo5,ecx

        push   ebp
        jmp    mi1 ; flush code cache
mi1:
        shr    ebp,2
        jz     miskip16
        push   ebp
miagain16:
        IM2F   0,milo1,mihi1
        IM2F   1,milo2,mihi2
        IM2F   2,milo3,mihi3
        IM2F   3,milo4,mihi4
        add    edi,(4*4)
        dec    dword ptr [esp]
        jnz    miagain16
        pop    ebp
miskip16:
        pop    ebp
        and    ebp,3
        jz     miskip1
        push   ebp
miagain1:
        IM2F   0,milo5,mihi5
        add    edi,4
        dec    dword ptr [esp]
        jnz    miagain1
        pop    ebp
miskip1:
        STUBEND
        ret

        END
