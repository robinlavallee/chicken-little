#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "sprite.h"

#define ANIMF_CLIP           1           // enable clipping
#define ANIMF_LOOP           2           // loop animation
#define ANIMF_REVERSE        4           // play animation in reverse
#define ANIMF_FINISHED       8 
#define ANIMF_FASTSHADOW   128


// =====================================================================================
//                                 All that is *Animation*
// =====================================================================================
// Once again, perhaps this segment of the header file should be separated out into its
// own header file.  We'll save execuative descisions like that for later. :)


// =====================================================================================
typedef struct ANIM_FRAME
// =====================================================================================
// Data for each frame of an animation.  Considered static for most purposes (see
// ANIM_SEQUENCE for more info).
{
    SPRITE   sprite;            // this is what I look like.
    uint     delay;             // this is how long I am me.
    int      transx, transy;    // this is where I am. (my translated location).
} ANIM_FRAME;


// =====================================================================================
typedef struct ANIM_SEQUENCE
// =====================================================================================
// This is the data for the entire collection of frames in an animation sequence.
// After its creation, this structure is considered static for most purposes.  We may
// modify it in really advanced situations - but for now, no. :)
{
    uint        count;          // number of frames in this sequence.
    ANIM_FRAME *frame;          // array of frames (alloc'd to 'count')
} ANIM_SEQUENCE;


// =====================================================================================
typedef struct ANIM_STATE
// =====================================================================================
// A collection of animation states!  This has a list of animation sequences (See above). 
{
    uint           count;        // number of sequences
    ANIM_SEQUENCE *sequence;     // array of sequences (alloc'd to 'count')
} ANIM_STATE;


// =====================================================================================
typedef struct ANIMATION
// =====================================================================================
// The active in-game animation structure.  This is what the animation sequencer uses to
// keep track of what the animation is doing.
{
    uint        flags;           // see those ANIMF_xxxxx defines!
    uint        curframe;        // current frame;
    int         countdown;       // time until this frame 'expires'
    int         x,y;             // physical location of the animation
    VRECT       clip;            // clipping rectangle

    ANIM_SEQUENCE *seq;          // Current animation sequence
} ANIMATION;


extern BOOL   Anim_Active(ANIMATION *anim);
extern void   Anim_BlitterEx(ANIMATION *anim, int x, int y);
extern void   Anim_Update(ANIMATION *anim, uint timepass);
extern void   Anim_SetSequence(ANIMATION *dest, ANIM_SEQUENCE *seq, uint flags);

extern void   Anim_SetTranslation(ANIMATION *anim, int x, int y);

extern int    Anim_GetTranslationX(ANIMATION *anim);
extern int    Anim_GetTranslationY(ANIMATION *anim);
extern void   Anim_SetTranslationX(ANIMATION *anim, int x);
extern void   Anim_SetTranslationY(ANIMATION *anim, int y);
extern int    Anim_TranslateX(ANIMATION *anim, int x);
extern int    Anim_TranslateY(ANIMATION *anim, int y);


// =====================================================================================
    static void _inline Anim_Blitter(ANIMATION *anim)
// =====================================================================================
// Render the specified animation to the video display.  Blits the current frame indicated
// by the animation information
{
    if(anim->seq) sprblit(anim->seq->frame[anim->curframe].sprite, (anim->x>>8) + anim->seq->frame[anim->curframe].transx, (anim->y>>8) + anim->seq->frame[anim->curframe].transy);
}


#endif
