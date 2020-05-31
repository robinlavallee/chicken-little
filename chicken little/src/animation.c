/*
  Chicken Little

  By the CL Dev Team and Gratis Games
  Uses the DIVE game engine by Divine Entertainment

  -------------------------------------------------------
  module: animation.c

  Animation takes coordination. Animation, a game we all can play.  You take a simple
  sprite, put it on the screen.  Change it once, change it again.  Woodewooodewooo.

  Notes:
   - The translation system works based on 8 bit fixed point values, to provide more
     accurate movements and stuff.
*/

#include <assert.h>
#include <string.h>
#include "animation.h"
#include "app.h"


// =====================================================================================
    BOOL Anim_Active(ANIMATION *anim)
// =====================================================================================
// Returns TRUE if the animation is in the process of playing.
// Animations with the ANIMF_LOOP flag set will never report FALSE here.  Be weary.
{
    return (anim->flags & ANIMF_FINISHED) ? 0 : 1;
}


// =====================================================================================
    void Anim_BlitterEx(ANIMATION *anim, int x, int y)
// =====================================================================================
// Render the specified animation to the video display.  Blits the current frame indicated
// by the animation information, and it properly translates the sprite location based on 
// the animation coordinates and the frame's individual 'hot spot'
{
    #ifdef _DEBUG
    assert(x >= 0 && x<= 640);
    assert(y >= 0 && y<= 480);
    #endif

    if(anim->seq)
    {   
        if(anim->flags & ANIMF_FASTSHADOW)
            sprblit_fastshadow(anim->seq->frame[anim->curframe].sprite, (anim->x>>8) + anim->seq->frame[anim->curframe].transx + x+3, (anim->y>>8) + anim->seq->frame[anim->curframe].transy + y+3);

        sprblit(anim->seq->frame[anim->curframe].sprite, (anim->x>>8) + anim->seq->frame[anim->curframe].transx + x, (anim->y>>8) + anim->seq->frame[anim->curframe].transy + y);
    }
}


// =====================================================================================
    void Anim_Update(ANIMATION *anim, uint timepass)
// =====================================================================================
// Update the animation by checking the timepass against the animation's countdown and
// changing frames if need be.
{
    if(!anim || !anim->seq || anim->flags == ANIMF_FINISHED) return;

    anim->countdown -= timepass;
    if(anim->countdown <= 0)
    {   
        // Lets find the next frame, shall we?

        anim->curframe++;

        if(anim->curframe >= anim->seq->count)
        {   if(anim->flags & ANIMF_LOOP)
            {   anim->curframe = 0;
            } else
            {   anim->curframe = (anim->seq->count-1);
                anim->flags   |= ANIMF_FINISHED;
            }
        }

        anim->countdown += anim->seq->frame[anim->curframe].delay;
        App_Paint();
    }
}


// =====================================================================================
    void Anim_SetSequence(ANIMATION *dest, ANIM_SEQUENCE *seq, uint flags)
// =====================================================================================
// Initialize the give animation structure and set the state info for it.
// Note:  If the sequence is 'empty' (is NULL), then the call to this function is ignored
// and the animation REMAINS UNCHANGED.  ie, it is not stopped, or removed or anything 
// like that.  it continues unchanged.
{
    if(seq && seq->frame)
    {   memset(dest,0, sizeof(ANIMATION));
    
        dest->seq       = seq;
        dest->countdown = seq->frame[0].delay;
        dest->flags     = flags;
    }
}


// =====================================================================================
    int Anim_TranslateX(ANIMATION *anim, int x)
// =====================================================================================
// Sets the x-coordinate translation for the given animation.
// Returns the new x translation value.
// Both the incoming and outgoing values are *8 BIT FIXED*
{
    anim->x += x;
    return anim->x;
}


// =====================================================================================
int Anim_TranslateY(ANIMATION *anim, int y)
// =====================================================================================
// Sets the y-coordinate translation for the given animation.
// Returns the new y translation value.
// Both the incoming and outgoing values are *8 BIT FIXED*
{
    anim->y += y;
    return anim->y;
}


// =====================================================================================
    int Anim_GetTranslationY(ANIMATION *anim)
// =====================================================================================
// Returns the new y translation value in 8-bit fixed point format.
{
    return anim->y;
}


// =====================================================================================
    int Anim_GetTranslationX(ANIMATION *anim)
// =====================================================================================
// Returns the new x translation value in 8-bit fixed point format.
{
    return anim->x;
}



// =====================================================================================
    void Anim_SetTranslationX(ANIMATION *anim, int x)
// =====================================================================================
// hard sets the X translation, reguardless of what the old value was.
{
    anim->x = x;
}


// =====================================================================================
    void Anim_SetTranslationY(ANIMATION *anim, int y)
// =====================================================================================
// hard sets the Y translation, reguardless of what the old value was.
{
    anim->y += y;
}


// =====================================================================================
    void Anim_SetTranslation(ANIMATION *anim, int x, int y)
// =====================================================================================
{
    anim->x = x;
    anim->y = y;
}
