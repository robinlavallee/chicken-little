/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-1999)

 Support:
  If you find problems with this code, send mail to:
    air@divent.simplenet.com

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: S3M_IT.C

  Common functions between S3Ms and ITs  (smaller .EXE! :).  Let it be noted
  that I DO take the extra effort to make my programs better and smaller!

 Portability:
  All systems - all compilers (hopefully)

*/

#include "mikmod.h"
#include "itshare.h"

static void WriteVolSlide(int hi, int lo, int oldeffect);
static void WritePanSlide(int hi, int lo, int oldeffect);
static void WriteGlobVolSlide(int hi, int lo);
static void WriteChanVolSlide(int hi, int lo);

// IT / S3M Extended SS effects:

enum
{   SS_GLISSANDO = 1,
    SS_FINETUNE,
    SS_VIBWAVE,
    SS_TREMWAVE,
    SS_PANWAVE,
    SS_FRAMEDELAY,
    SS_S7EFFECTS,
    SS_PANNING,
    SS_SURROUND,
    SS_HIOFFSET,
    SS_PATLOOP,
    SS_NOTECUT,
    SS_NOTEDELAY,
    SS_PATDELAY
};

void S3MIT_ProcessCmd(UBYTE *poslookup, unsigned int cmd, unsigned int inf, BOOL oldeffect, int gxxmem)
{
    UNITRK_EFFECT effdat;
    UBYTE         hi, lo;

    lo = inf & 0xf;
    hi = inf >> 4;

    // process S3M / IT specific command structure

    effdat.framedly       = 0;
    effdat.param.u        = 0;

    if(cmd!=255)
    {   switch(cmd)
        {   case 1:                 // Axx set speed to xx
               if(inf)
               {   effdat.effect   = UNI_GLOB_SPEED;
                   effdat.framedly = UFD_RUNONCE;
                   effdat.param.u  = inf;
                   utrk_write_global(&effdat, STMEM_SPEED);
               } else utrk_memory_global(&effdat, STMEM_SPEED);
            break;

            case 2:                 // Bxx position jump
                pt_write_effect(0xb, poslookup ? poslookup[inf] : inf);
            break;

            case 3:                 // Cxx patternbreak to row xx
                if(oldeffect & 1)
                    pt_write_effect(0xd, (((inf & 0xf0) >> 4) * 10) + (inf & 0xf));
                else
                    pt_write_effect(0xd, inf);
            break;

            case 4:                 // Dxy volumeslide
                WriteVolSlide(hi,lo,oldeffect);
            break;

            case 5:                 // Exy toneslide down
            case 6:                 // Fxy toneslide up
                effdat.effect   = UNI_PITCHSLIDE;
                if(hi >= 0xe)
                    effdat.framedly = UFD_RUNONCE;
                else
                    effdat.framedly = 1;

                if(inf)
                {   effdat.param.s = (hi == 0xe) ? lo*2 : (hi == 0xf) ? lo*8 : inf*8;
                    if(cmd == 5) effdat.param.s = -effdat.param.s;
                    utrk_write_local(&effdat, STMEM_PITCHSLIDE);
                } else utrk_memory_local(&effdat, STMEM_PITCHSLIDE, (cmd == 5) ? -1 : 1);
            break;

            case 7:                 // Gxx Tone portamento, speed xx
                if(oldeffect & 1)
                    pt_write_effect(0x3,inf);
                else
                {   // ImpulseTracker Sucks, Reason #2577:
                    // Lim decides it's a good idea for Exx/Fxx AND Gxx to share memory
                    // because they both have to do with pitch, I guess.

                    effdat.effect   = UNI_PORTAMENTO;
                    if(inf)
                    {   effdat.param.s  = inf<<3;
                        utrk_write_local(&effdat, gxxmem);
                    } else
                        utrk_memory_local(&effdat, gxxmem, (gxxmem == PTMEM_PORTAMENTO) ? 0 : 1);
                } 
            break;

            case 8:                 // Hxy vibrato
                // ImpulseTracker Sucks, Reason #1233:
                // The original version of IT was buggy.. Vibratos vibrated too shallowly.
                // (much like Lim's intellect?)  So he makes it an option.  uGH.
                if(oldeffect & 1)
                    pt_write_effect(0x4, inf);
                else
                {   if(lo)
                    {   effdat.effect  = UNI_VIBRATO_DEPTH;
                        effdat.param.u = lo*8;
                        utrk_write_local(&effdat, PTMEM_VIBRATO_DEPTH);
                    } else utrk_memory_local(NULL, PTMEM_VIBRATO_DEPTH, 0);

                    if(hi)
                    {   effdat.effect  = UNI_VIBRATO_SPEED;
                        effdat.param.u = hi*4;
                        utrk_write_local(&effdat, PTMEM_VIBRATO_SPEED);
                    } else utrk_memory_local(NULL, PTMEM_VIBRATO_SPEED, 0);
                }
            break;

            case 9:                 // Ixy tremor, ontime x, offtime y
                if(inf)
                {   effdat.effect   = UNI_TREMOR;
                    effdat.framedly = 0;
                    if(oldeffect & 1)
                    {   // should this be masked or not? Will we ever know?  Will it ever matter?
                        effdat.param.loword.u = (lo+1)&0xf;
                        effdat.param.hiword.u = (hi+1)&0xf;
                    } else
                    {   effdat.param.loword.u = lo;
                        effdat.param.hiword.u = hi;
                    }
                    utrk_write_local(&effdat, STMEM_TREMOR);
                } else utrk_memory_local(&effdat, STMEM_TREMOR, 0);
            break;

            case 0xa:               // Jxy arpeggio
                if(inf)
                {   effdat.param.byte_a = lo;
                    effdat.param.byte_b = hi;
                    effdat.effect  = UNI_ARPEGGIO;
                    utrk_write_local(&effdat, STMEM_ARPEGGIO);
                } else utrk_memory_local(NULL, STMEM_ARPEGGIO, 0);
            break;

            case 0xb:               // Kxy Dual command H00 & Dxy
                utrk_memory_local(NULL, PTMEM_VIBRATO_DEPTH, 0);
                utrk_memory_local(NULL, PTMEM_VIBRATO_SPEED, 0);
                WriteVolSlide(hi,lo, oldeffect);
            break;

            case 0xc:               // Lxy Dual command G00 & Dxy
                if(gxxmem == PTMEM_PORTAMENTO)
                {   effdat.effect   = UNI_PORTAMENTO_LEGACY;
                    utrk_memory_local(&effdat, gxxmem, 0);
                } else
                {   effdat.effect   = UNI_PORTAMENTO;
                    utrk_memory_local(&effdat, gxxmem, 1);
                }
                WriteVolSlide(hi,lo,oldeffect);
            break;

            case 0xd:               // Mxx Set Channel Volume
               effdat.param.u  = inf;
               effdat.effect   = UNI_CHANVOLUME;
               effdat.framedly = UFD_RUNONCE;
               utrk_write_local(&effdat, UNIMEM_NONE);
            break;       

            case 0xe:               // Nxy Slide Channel Volume
                WriteChanVolSlide(hi,lo);
            break;

            case 0xf:               // Oxx set sampleoffset xx00h
                effdat.effect         = UNI_OFFSET_LEGACY;
                effdat.param.loword.u = inf<<8;
                effdat.framedly       = UFD_RUNONCE;
                utrk_write_local(&effdat, UNIMEM_NONE);              
            break;

            case 0x10:              // Pxy Slide Panning Commands
                WritePanSlide(hi,lo,oldeffect);
            break;

            case 0x11:              // Qxy Retrig (+volumeslide)
                if(inf)
                {   effdat.param.loword.u = lo;
                    effdat.param.hiword.u = hi;
                    effdat.effect   = UNI_RETRIG;
                    effdat.framedly = 0;
                    utrk_write_local(&effdat, STMEM_RETRIG);
                } else utrk_memory_local(&effdat, STMEM_RETRIG, 0);
            break;

            case 0x12:              // Rxy tremolo speed x, depth y
                if(lo)
                {   effdat.param.u   = lo * 4;
                    effdat.effect    = UNI_TREMOLO_DEPTH;
                    utrk_write_local(&effdat, PTMEM_TREMOLO_DEPTH);
                } else utrk_memory_local(NULL, PTMEM_TREMOLO_DEPTH, 0);

                if(hi)
                {   effdat.param.u   = hi * 4;
                    effdat.effect    = UNI_TREMOLO_SPEED;
                    effdat.framedly  = (oldeffect & 1) ? 1 : 0;
                    utrk_write_local(&effdat, PTMEM_TREMOLO_SPEED);
                } else utrk_memory_local(NULL, PTMEM_TREMOLO_SPEED, 0);
            break;

            case 0x13:              // Sxx special commands
                if(inf)
                {   BOOL globeffect = 0;

                    effdat.framedly = UFD_RUNONCE;
                    switch(hi)
                    {   /*case SS_GLISSANDO:      // S1x set glissando voice
                        break;
    
                        case SS_FINETUNE:       // S2x set finetune
                        break;*/ 
    
                        case SS_VIBWAVE:        // S3x set vibrato waveform
                            effdat.param.u  = lo;
                            effdat.effect   = UNI_VIBRATO_WAVEFORM;
                        break;
    
                        case SS_TREMWAVE:       // S4x set tremolo waveform
                            effdat.param.u  = lo;
                            effdat.effect   = UNI_TREMOLO_WAVEFORM;
                        break;
    
                        case SS_PANWAVE:        // The Satanic Panbrello waveform
                            effdat.param.u  = lo;
                            effdat.effect   = UNI_PANBRELLO_WAVEFORM;
                        break;
    
                        case SS_FRAMEDELAY:     // S6x Delay x number of frames (patdly)
                            effdat.effect          = UNI_GLOB_DELAY;
                            effdat.param.loword.u  = lo;
                            globeffect = 1;
                        break;
    
                        case SS_S7EFFECTS:      // ImpulseTracker NNA control effects
                            switch(lo)
                            {   case 0:         // S70 Past note cut
                                    effdat.param.u  = NNA_CUT;
                                    effdat.effect   = UNI_NNA_CHILDREN;
                                break;
    
                                case 1:         // S71 Past note off
                                    effdat.param.u  = NNA_OFF;
                                    effdat.effect   = UNI_NNA_CHILDREN;
                                break;
    
                                case 2:         // S72 Past note fade
                                    effdat.param.u  = NNA_FADE;
                                    effdat.effect   = UNI_NNA_CHILDREN;
                                break;
    
                                // Change NNA behavior (for current note only)
                                case 3:         // S73 Set to cut
                                case 4:         // S74 Set to continue
                                case 5:         // S74 Set to off
                                case 6:         // S74 Set to fade
                                    effdat.param.u  = lo - 3;
                                    effdat.effect   = UNI_NNA_CONTROL;
                                break;
    
                                case 8:         // S78 Volume Envelope on
                                    effdat.param.byte_b = TRUE;
    
                                case 7:         // S77 Volume Envelope off
                                    effdat.effect = UNI_ENVELOPE_CONTROL;
                                break;
    
                                case 0xa:       // S7a Panning Envelope on
                                    effdat.param.byte_b = TRUE;
    
                                case 9:         // S79 Panning Envelope off
                                    effdat.effect       = UNI_ENVELOPE_CONTROL;
                                    effdat.param.byte_a = 1;
                                break;
    
                                case 0xc:       // S7c Pitch Envelope on
                                    effdat.param.byte_b = TRUE;
    
                                case 0xb:       // S7b Pitch Envelope off
                                    effdat.effect       = UNI_ENVELOPE_CONTROL;
                                    effdat.param.byte_a = 2;
                                break;
                            }
                        break;
    
                        case SS_PANNING:        // S8x Set Panning
                            effdat.param.s  = ((lo<=8) ? (lo*32) : (lo*34)) + PAN_LEFT;
                            effdat.effect   = UNI_PANNING;
                        break;
    
                        case SS_SURROUND:       // Set Surround Sound
                            effdat.param.s  = PAN_SURROUND;
                            effdat.effect   = UNI_PANNING;
                        break;

                        case SS_HIOFFSET:       // set the upper bits of the sample offset.
                            effdat.param.hiword.u = lo;
                            effdat.effect         = UNI_OFFSET_LEGACY;
                        break;
    
                        case SS_PATLOOP:        // SBx pattern loop
                            effdat.param.u  = lo;
                            effdat.effect   = lo ? UNI_GLOB_LOOP : UNI_GLOB_LOOPSET;
                            globeffect      = 1;
                        break;
    
                        case SS_NOTECUT:        // SCx Note Cut
                            if(lo==0) lo++;     // make sure we never run on 0
                            effdat.effect    = UNI_NOTEKILL;
                            effdat.framedly |= lo;
                        break;
    
                        case SS_NOTEDELAY:      // Sdx Note Delay
                            effdat.param.u  = lo;
                            effdat.effect   = UNI_NOTEDELAY;
                            effdat.framedly = 0;
                        break;
    
                        case SS_PATDELAY:
                            effdat.param.hiword.u = lo;
                            effdat.effect         = UNI_GLOB_DELAY;
                            globeffect = 1;
                        break;
                    }                    
                    if(globeffect)
                        utrk_write_global(&effdat, STMEM_SEFFECT);
                    else
                        utrk_write_local(&effdat, STMEM_SEFFECT);
                } else utrk_memory_local(NULL, STMEM_SEFFECT, 0);
            break;

            case 0x14:              // Txx tempo
                if(inf)
                {   if(inf < 0x20)
                    {   effdat.effect   = UNI_GLOB_TEMPOSLIDE;
                        effdat.param.s  = (inf < 0x10) ? (0 - inf) : inf;
                        effdat.framedly = 1;
                        utrk_write_global(&effdat, STMEM_TEMPO);
                    } else 
                    {   effdat.effect   = UNI_GLOB_TEMPO;
                        effdat.param.u  = inf;
                        effdat.framedly = UFD_RUNONCE;
                        utrk_write_global(&effdat, STMEM_TEMPO);
                    }
                } else utrk_memory_global(&effdat, STMEM_TEMPO);
            break;

            case 0x15:      // Uxy Fine Vibrato speed x, depth y
                // Impulse Tracker sucks, Reason #1234:
                //  Lim messed up fine vibratos too, of course (see reason # 1233 above).
                if(lo)
                {   effdat.param.u = (oldeffect & 1) ? (lo * 4) : (lo * 2);
                    effdat.effect  = UNI_VIBRATO_DEPTH;
                    utrk_write_local(&effdat, PTMEM_VIBRATO_DEPTH);
                } else utrk_memory_local(NULL, PTMEM_VIBRATO_DEPTH, 0);


                if(hi)
                {   if(oldeffect & 1) effdat.framedly = 1;
                    effdat.param.u = hi*4;
                    effdat.effect  = UNI_VIBRATO_SPEED;
                    utrk_write_local(&effdat, PTMEM_VIBRATO_SPEED);
                } else utrk_memory_local(NULL, PTMEM_VIBRATO_SPEED, 0);
            break;

            case 0x16:      // Vxx Set Global Volume
                effdat.framedly = UFD_RUNONCE;
                effdat.param.u  = inf;
                effdat.effect   = UNI_GLOB_VOLUME;
                utrk_write_global(&effdat, UNIMEM_NONE);
            break;

            case 0x17:      // Wxy Global Volume Slide
                WriteGlobVolSlide(hi,lo);
            break;

            case 0x18:      // Xxx amiga command 8xx
                // S3Ms were range 0 to 80h?
                // ImpulseTracker Sucks, Reason #592:
                //  Lim explains how S3M Panning is range 0-80h, and how IT is 0-ffh,
                //  but he forgets to implement 0-80h in his 'oldeffects' mode.  Not
                //  only was it confusing to me, but any S3Ms saved with IT are not going
                //  to pan properly.  What a total dweeb.

                pt_write_effect(0x8,(oldeffect & 2) ? (inf * 2) : inf);
            break;

            case 0x19:      // Yxy Panbrello  speed x, depth y
                if(lo)
                {   effdat.param.u  = lo;
                    effdat.effect   = UNI_PANBRELLO_DEPTH;
                    utrk_write_local(&effdat, STMEM_PANBRELLO_DEPTH);
                } else utrk_memory_local(&effdat, STMEM_PANBRELLO_DEPTH, 0);

                if(hi)
                {   effdat.param.u  = hi;
                    effdat.effect   = UNI_PANBRELLO_SPEED;
                    utrk_write_global(&effdat, STMEM_PANBRELLO_SPEED);
                } else utrk_memory_local(&effdat, STMEM_PANBRELLO_SPEED, 0);
            break;
       }
   }
}

// Volume slides need to be split into their specialized effects categories.


static BOOL SuperDuperSliderMan(UNITRK_EFFECT *eff, int hi, int lo, int oldeffect)
// Used by all the s3m volume slider commands to properly assign eff.param and
// eff.framedly based on the hi/lo prarameters passed to it.  Returns true if
// the effect is valid or false if it should be ignored.
{
    
    if(hi == 0)
    {   eff->param.s  = 0 - lo;
        eff->framedly = (!(oldeffect & 1) && (lo != 0xf)) ? 1 : 0;
    } else if(lo == 0)
    {   eff->param.s  = hi;
        eff->framedly = (!(oldeffect & 1) && (lo != 0xf)) ? 1 : 0;
    } else if(hi == 0xf)
    {   eff->param.s  = 0 - lo;
        eff->framedly = UFD_RUNONCE;
    } else if(lo == 0xf)
    {   eff->param.s  = hi;
        eff->framedly = UFD_RUNONCE;
    } else return 0;

    return 1;
}


static BOOL SuperSliderMan(UNITRK_EFFECT *eff, int hi, int lo)
{
    if(hi == 0)
    {   eff->param.s  = 0 - lo;
        eff->framedly = 1;
    } else if(lo == 0)
    {   eff->param.s  = hi;
        eff->framedly = 1;
    } else if(hi == 0xf)
    {   eff->param.s  = 0 - lo;
        eff->framedly = UFD_RUNONCE;
    } else if(lo == 0xf)
    {   eff->param.s  = hi;
        eff->framedly = UFD_RUNONCE;
    } else return 0;

    return 1;
}


static void WriteVolSlide(int hi, int lo, int oldeffect)
{
    if(hi || lo)
    {   UNITRK_EFFECT  eff;
        if(SuperDuperSliderMan(&eff, hi, lo, oldeffect))
        {   eff.effect   = UNI_VOLSLIDE;
            eff.param.s *= 2;
            utrk_write_local(&eff, STMEM_VOLSLIDE);
        }
    } else utrk_memory_local(NULL, STMEM_VOLSLIDE, 0);
}

static void WritePanSlide(int hi, int lo, int oldeffect)
// Split this sucker up into a set of specialized commands.
{
    if(hi || lo)
    {   UNITRK_EFFECT  eff;

        if(SuperDuperSliderMan(&eff, lo, hi, oldeffect))
        {   eff.effect   = UNI_PANSLIDE;
            eff.param.s *= 4;    // adust from 0->64 to -128->128 scale.
            utrk_write_local(&eff, STMEM_PANSLIDE);
        }
    } else utrk_memory_local(NULL, STMEM_PANSLIDE, 0);
}

// ImpulseTracker Sucks, Reason #3662:
//   Lim decides to treat these effects the same, whether oldeffects is on or off,
//   since these were not effects available in ST3, destroying any semblance of
//   consistancy, and not allowing for the S3M format to ever be properly expanded
//   upon in the future.  Sigh.

static void WriteChanVolSlide(int hi, int lo)
{
    if(hi || lo)
    {   UNITRK_EFFECT  eff;
        if(SuperSliderMan(&eff, hi, lo))
        {   eff.effect   = UNI_CHANVOLSLIDE;
            utrk_write_local(&eff, STMEM_CHANVOLSLIDE);
        }
    } else utrk_memory_local(NULL, STMEM_CHANVOLSLIDE, 0);
}

static void WriteGlobVolSlide(int hi, int lo)
{
    if(hi || lo)
    {   UNITRK_EFFECT  eff;
        if(SuperSliderMan(&eff, hi,lo))
        {   eff.effect   = UNI_GLOB_VOLSLIDE;
            utrk_write_global(&eff, STMEM_GLOBVOLSLIDE);
        }
    } else utrk_memory_global(NULL, STMEM_GLOBVOLSLIDE);
}


