/*
  Mikmod mdsfx Management System

  By Jake Stine of Divine Entertainment


*/


#include "mikmod.h"
#include "unisamp.h"

/*MD_SAMPLE mdsfx_load()
{

}
*/

MD_SAMPLE *mdsfx_create(MDRIVER *md)
{
    MD_SAMPLE *si;

    si = (MD_SAMPLE *)_mm_calloc(1,sizeof(MD_SAMPLE));
    
    si->md = md;

    return si;
}

MD_SAMPLE *mdsfx_duplicate(MD_SAMPLE *src)
{
    MD_SAMPLE  *newug;

    newug = (MD_SAMPLE *)_mm_malloc(sizeof(MD_SAMPLE));
    
    // duplicate all information

    *newug = *src;
    if(src->owner)
        newug->owner = src->owner;
    else
        newug->owner = src;

    return newug;
}

void mdsfx_free(MD_SAMPLE *samp)
{
    if(samp->owner)
    {   // deallocate sample from audio drivers.
        MD_SampleUnload(samp->md, samp->handle);
    }

    _mm_free(samp, "Unisample");
}

void mdsfx_play(MD_SAMPLE *s, MD_VOICESET *vs, uint voice, int start)
{
    Voice_Play(vs, vs->vdesc[voice].voice, s->handle, start, s->length, s->reppos, s->repend, s->suspos, s->susend, s->flags);
    Voice_SetVolume(vs,voice, 128);
}

int mdsfx_playeffect(MD_SAMPLE *s, MD_VOICESET *vs, uint start, uint flags)

// Mikmod's automated sound effects sample player.
// Picks a voice from the given voiceset, based upon the voice either being
// empty (silent), or being the oldest sound in the voiceset.  Any sound
// flagged as critical will not be replaced.
//
// Returns the voice that the sound is being played on.  If no voice was
// available (usually by fault of criticals) then -1 is returned.

{
    uint     orig = vs->sfxpool;    // for cases where all channels are critical
    int      voice;

    // check for invalid parameters
    if(!vs || !vs->voices) return -1;

    // Find a suitable voice for this sample to be played in.
    // Use the user-definable callback procedure to do so!

    voice = (s->findvoice_proc) ? s->findvoice_proc(vs, flags) : Voice_Find(vs, flags);
    if(voice == -1) return -1;

    //volume  = _mm_boundscheck(s->volume, 0, VOLUME_FULL);
    //panning = (s->panning == PAN_SURROUND) ? PAN_SURROUND : _mm_boundscheck(s->panning, PAN_LEFT, PAN_RIGHT);
    
    Voice_Play(vs, voice, s->handle, start, s->length, s->reppos, s->repend, s->suspos, s->susend, s->flags);
    Voice_SetVolume(vs, voice, s->volume);
    Voice_SetPanning(vs, voice, s->panning, 0);
    Voice_SetFrequency(vs, voice, s->speed);

    return voice;
}
