
#include "textinfo.h"
#include "app.h"

#include <string.h>

// =====================================================================================
    static void TextInfo_Renderer(TEXTINFO *text)
// =====================================================================================
// Renders a text object
{
    if(text->fadeout)
        vd_dispstring_a(text->font, text->x, text->y, text->fadeout, text->text);
    else
        vd_dispstring(text->font, text->x, text->y, text->text);

}

// =====================================================================================
    static int TextInfo_StateHandler(TEXTINFO *text)
// =====================================================================================
{
    switch(text->entity.state)
    {
        case ENTITY_STATE_DESTROY:
            // Check the timeout here, if it is, destroy it
            Entity_KillSelf(&text->entity);
        break;

        case TEXT_STATE_FLOATING:
            text->floattime--;
            if(!text->floattime)
                Entity_SetState(&text->entity, TEXT_STATE_FADEOUT);
        break;

        case TEXT_STATE_FADEOUT:
            text->fadeout -= 6;
            if(text->fadeout < 1)
            {   Entity_KillSelf(&text->entity);
                return 0;
            }

            if(text->floating)
            {   text->floating--;
                text->y--;
            }
            App_Paint();

        break;

    }
    return 40;
}

static ENTITY Entity_TextInfo =
{
    "TextInfo",
    TEXT_STATE_FADEOUT,
    0,

    NULL,
    TextInfo_StateHandler,
    NULL,
    TextInfo_Renderer
} ;


// =====================================================================================
    TEXTINFO *TextInfo_Initialize(ENTITY **entlist, const CHAR *text, int x, int y, VD_FONT *font)
// =====================================================================================
{
    TEXTINFO *newtext;

    newtext = (TEXTINFO *)Entity_Spawn(entlist, &Entity_TextInfo, sizeof(TEXTINFO));
    strcpy(newtext->text, text);

    newtext->x = x;
    newtext->y = y;

    newtext->fadeout = SPR_ALPHA_RANGE;
    newtext->font    = font;

    return newtext;
}

