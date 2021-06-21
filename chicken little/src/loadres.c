/*

  module: loadres.c
  
  The Chicken Little Resource Loader.

  This module contains code which decides what resources to load, based on the current
  state of the game.

*/


#include "chicken.h"
#include "clmenu.h"

#include <string.h>


CL_RES    *respak = NULL;           // current resource pak we load stuff from

RES_FONT   font;
RES_MUSIC  music;
RES_SNDFX  sfx;
RES_GPSPR  gpspr[GP_NUMPIECES];  // array of gamepiece resources (see struct define above)

static int chrtab[256];

static SPRITE   blanksprite;


// Quick hack: to load portrait resources:
extern void Portrait_LoadResources(CL_RES *respak);


// =====================================================================================
    BOOL CL_LoadResources(GAMEDATA *chick)
// =====================================================================================
{
    int              i,t;

    // STEP 1
    //  Load all those 'necessary' resources, if they aren't already loaded:

    // Air's incredibly Evil Font loading code!
    // ========================================

    if(!font.big || !font.little)
    {   memset(chrtab,-1,sizeof(int) * 256);

        t=0;
        for(i=0; i<26; i++) chrtab[i+0x41] = t++;
        t=0;
        for(i=0; i<26; i++) chrtab[i+0x61] = t++;
        for(i=0; i<10; i++) chrtab[i+0x30] = t++;

        chrtab['!'] = t++;
        chrtab['?'] = t++;
        chrtab['+'] = t++;
        chrtab['-'] = t++;
        chrtab[':'] = t++;
        chrtab['='] = t++;
        chrtab['*'] = t++;
        chrtab['/'] = t++;
        chrtab['('] = t++;
        chrtab[')'] = t++;
        chrtab['.'] = t++;
        chrtab[','] = t++;

        if(!font.big)    font.big    = FontSpr_Load(chrtab, 47, 39, -8, 0, respak->videopak, RES_MAINFONT_BIG);
        if(!font.little) font.little = FontSpr_Load(chrtab, 24, 19, -4, 0, respak->videopak, RES_MAINFONT_SMALL);
    }

    //chick->blanksprite.ablit = sprblita_placebo;

    // Now, load stuff based on the state of the game:
    // -----------------------------------------------

    switch(chick->state)
    {    
        // =============================================================================
        case CL_STATE_START:
        // =============================================================================
        // Load the menu screens.

            Title_LoadResources(respak);

            if(!music.title)
                music.title = XAudioMusic_Load("data/music/menu_theme.pcm");

            if (!sfx.menuhigh)
              sfx.menuhigh = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_MENU_HIGHLIGHT);
            if (!sfx.menuselect)
              sfx.menuselect = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_MENU_SELECT);
        break;

        // =============================================================================
        case CL_STATE_DEMO:
        case CL_STATE_MATCH:
        case CL_STATE_SINGLEMATCH:
        // =============================================================================
        // Load all player and gameplay/gamepiece-related resources.  This includes back-
        // grounds, eggs, etc. etc.

            if(!music.stage[chick->stage])
                music.stage[chick->stage] = XAudioMusic_Load(chick->stage ? "data/music/puzzle9.pcm" : "data/music/metro_a.pcm");

            VDRes_LoadSprite(respak->videopak, &chick->layout, RES_LAYOUT_FIRST);
            
            // only load the background for match state, else we use the title screen
            // as our demo background.

            if(chick->state == CL_STATE_MATCH || chick->state == CL_STATE_SINGLEMATCH)
                VDRes_LoadSprite(respak->videopak, &chick->background, RES_BACKGROUND_TYPE_A + chick->stage);

            GamePiece_LoadResources(respak, gpspr);
            Portrait_LoadResources(respak);

            // Score font
            // ==========
            // The score font defaults to a sprite for all blank/space characters.
            // it shows a dimmed 8-figure, LCD style.  Hence we memset 0 instead of -1.

            memset(chrtab,0,sizeof(int) * 256);
            t=1;
            for(i=0; i<10; i++) chrtab[i+0x30] = t++;
            chrtab['-'] = t++;
        
            font.score = FontSpr_Load(chrtab, 18, 10, 1, 0, respak->videopak, RES_SCOREFONT_NORMAL);

            if (!sfx.birdie1)
              sfx.birdie1 = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_EGG_CHIRP1);
            if (!sfx.birdie2)
              sfx.birdie2 = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_EGG_CHIRP2);

            if (!sfx.eggcrack)
              sfx.eggcrack = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_EGG_CRACK);
            if (!sfx.eggbreak)
              sfx.eggbreak = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_EGG_BREAK);

            if (!sfx.blip)
              sfx.blip = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_EGG_BLIP);
            if (!sfx.crumble)
              sfx.crumble = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_STONE_ROTATE);
            if (!sfx.thump)
              sfx.thump = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_GAMEPIECE_THUMP);

            if (!sfx.eggrotate)
              sfx.eggrotate = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_GAMEPIECE_ROTATE);
            if (!sfx.eggplace)
              sfx.eggplace = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_GAMEPIECE_PLACE);

            if (!sfx.go)
              sfx.go = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_ANNOUNCER_GO);
            if (!sfx.getready)
              sfx.getready = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_ANNOUNCER_GETREADY);
            if (!sfx.win)
              sfx.win = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_ANNOUNCER_WIN);
            if (!sfx.lose)
              sfx.lose = MDRes_LoadSampleXAudio(respak->audiopak, SNDFX_ANNOUNCER_LOSE);

        break;
    }


    return -1;
}


// =====================================================================================
    void CL_OpenResource(VD_SURFACE *vs)
// =====================================================================================
// Creates a handle for accessing resources.
{
    uint      i;

    if(respak!=NULL)
        CL_CloseResource();
    
    respak = (CL_RES *)_mm_calloc(1, sizeof(CL_RES));

    respak->videopak = VDRes_OpenFN(vs, "gamegfx.idx", "gamegfx.dat");
    if(!respak->videopak) return;

    respak->audiopak = MDRes_OpenFN("gamesfx.idx", "gamesfx.dat");
    if(!respak->audiopak) return;

    CLEAR_STRUCT(music);
    CLEAR_STRUCT(sfx);
    CLEAR_STRUCT(font);

    Sprite_Init(vs, &blanksprite,0,0,0,0);

    for(i=0; i<GP_NUMPIECES; i++)
    {   gpspr[i].full    = blanksprite;
        gpspr[i].icon    = blanksprite;
        gpspr[i].outline = blanksprite;
    }

}

// =====================================================================================
    void CL_CloseResource()
// =====================================================================================
// Closes the resource file and unloads all resources associated with it.
{
    if(!respak) return;
    
    _mmlogd("Chicken Little > Unloading game resources...");

    if(music.title) XAudioMusic_Free(music.title);

    if (music.stage[0])
      XAudioMusic_Free(music.stage[0]);

    if (music.stage[1])
      XAudioMusic_Free(music.stage[1]);

    if(font.big)    font.big->free(font.big);
    if(font.little) font.little->free(font.little);
    if(font.score)  font.score->free(font.score);

    if(respak->videopak) VDRes_Close(respak->videopak);
    if(respak->audiopak) MDRes_Close(respak->audiopak);

    GamePiece_UnloadResources(respak);

    _mm_free(respak, "Done!");
}


// =====================================================================================
    BOOL CL_LoadAnimation(CL_RES *res, ANIM_SEQUENCE *seq, uint idx, uint count)
// =====================================================================================
// Load an animation sequence from the resource file.  
// the ANIM_FRAME portion of the sequence will automatically be allocated, and all
//
// Currently, the information for each frame is 'hard coded', which means it is not
// stored in some file, but is actually assigned manually by the caller.  The sequence
// is assumed to be properly stored 'in order.' so that idx to idx+count will all be
// valid frames of the animation.
//
// (in another lifetime, this will use an index file to know the proper indexes of each sprite)
{
    uint   i;
    
    if(!res || !seq) return 0;

    seq->count  = count;
    seq->frame  = (ANIM_FRAME *)_mm_calloc(count, sizeof(ANIM_FRAME));

    for(i=0; i<count; i++, idx++)
        VDRes_LoadSprite(res->videopak, &seq->frame[i].sprite, idx);

    return -1;
}


// =====================================================================================
    void CL_UnloadAnimation(CL_RES *res, ANIM_STATE *augh)
// =====================================================================================
// Unload all allocated structures in the specified animation state data.  This does *NOT*
// remove the bitmap resources.  Only CL_CloseResource does that!
{
    uint  i;

    if(!augh || !augh->sequence) return;

    _mmlogd("Chicken Little > Unloading animation sequence...");

    for(i=0; i<augh->count; i++)
    {   if(augh->sequence[i].frame)
            _mm_free(augh->sequence[i].frame, NULL);
    }
    _mm_free(augh->sequence, "Done!");
}
