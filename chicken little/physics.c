
/*
 *  Chicken Little.
 *  Lawgiver (Robin Lavallée - robin_lavallee@videotron.ca)
 *  Copyright by myself and Divine Entertainment. (2000)
 *  
 *  Physics engine. For animation & other stuff.
 *  
 */

#include <windows.h>
#include <math.h>
#include "physics.h"

ANIMATE *list;
int handle[256]; // handle list.
struct PHYSICS phys;

void InitPhysics(int x, int y)
{
    int i;

    list = NULL;

    for (i=0;i!=256;i++)
        handle[i]=-1;

    // Also, set the range for our physics.
    // Keep in mind, since we are working on graphics, our 'y' axis is reversed. Top = 0, down = max.

    phys.sizex = x;
    phys.sizey = y;
}

int AddLinearCartesian(double x1, double y1, double x2, double y2, double velocity, double duration)
{
    /*
     *  Return an handle to an animation handle.
     *  Linear translation from (x1, y1) to (x2, y2) in speed pixel/sec
     */

    // We are going to convert this into a vector type.

    double angle;

    // Avoid divide by 0

    if (x2 == x1)
    {
        if (y2 > y1)
            angle = PI/2;
        else
            angle = 3*PI/2;

        return (AddLinearVector(x1, y1, angle, velocity, duration));
    }

    angle=atan((y2-y1)/(float)(x2-x1));

    // Since we are using arctan, we need to convert back to the right 
    // numerical.

    if (x2 < x1 && y2 > y1) // 2nd quadran
    {
        angle = PI + angle;
    }
    else if (x2 < x1 && y2 < y1) // 3rd quadran
    {
        angle = angle + PI;
    }
    else if (x2 > x1 && y2 < y1) // 4th quadran
    {
        angle = 2 * PI + angle;
    }

    return (AddLinearVector(x1, y1, angle, velocity, duration)); // Call the linear vector function

}

int AddLinearVector(double x1, double y1, double dir, double velocity, double duration)
{
    /*
     *  Return an handle to an animation handle.
     *  Linear translation from (x1, y1) in direction 'dir' in speed pixel/sec.
     */

    ANIMATE temp;
    int handle;

    handle = FindFreeHandle();
    if (handle == -1)
        return -1;

    temp.type = ANIMATE_LINEAR;
    temp.x1 = x1;
    temp.y1 = y1;
    temp.extra1 = dir;
    temp.extra2 = velocity;
    temp.duration = duration;
    temp.time = timeGetTime();
    temp.handle = handle;

    LinkListAdd(&temp);

    return handle;
}

void LinkListAdd(ANIMATE *add)
{
    ANIMATE *cruise;

    cruise = list;

    if (cruise == NULL)
        cruise = (ANIMATE *)malloc(sizeof(ANIMATE));
    else
    {
        while (cruise->next != NULL)
            cruise=cruise->next;

        cruise->next = (ANIMATE *)malloc(sizeof(ANIMATE));
        cruise = cruise->next;
    }

    cruise->next = NULL;
   
    cruise->handle = add->handle;
    cruise->type = add->type;
    cruise->x1 = add->x1;
    cruise->y1 = add->y1;
    cruise->extra1 = add->extra1;
    cruise->extra2 = add->extra2;
    cruise->time = add->time;
    cruise->duration = add->duration;
}

int FindFreeHandle()
{
    int i;

    for (i=0;i!=256;i++)
    {
        if (handle[i] == -1) // this one is free
        {
            handle[i] = i;
            return i;
        }
    }

    return -1;
}

BOOL RetrievePosition(int handle, int *x, int *y)
{
    /*
     *  Give the physics handle, it will return its x and y coordonate.
     *  Return TRUE if function was succeful. It will return FALSE
     *  If the animation has ended, it will then kill the handle.
     */

    int newtime;
    ANIMATE *anim;

    newtime = timeGetTime();

    anim = FindAnimation(handle); // Get a pointer to our animation.

    if (anim == NULL)
        return FALSE;

    if ((newtime - anim->time) >= anim->duration)
    {
        KillAnimate(handle);
        return FALSE;
    }

    // Now the physics part!
    // So far, only ANIMATE_LINEAR works.

    switch (anim->type)
    {
    case ANIMATE_LINEAR :
        // My math tells me that we need to do a linear interpolation
        // extra1 = direction
        // extra2 = velocity

        *x = (int) (cos(anim->extra1) * anim->extra2 * (newtime - anim->time)/1000);
        *y = (int) (sin(anim->extra1) * anim->extra2 * (newtime - anim->time)/1000);
        *x += (int) anim->x1;
        *x += (int)anim->y1;
        
        if (*x >= phys.sizex)
            return FALSE;
        if (*y >= phys.sizey)
            return FALSE;
        if (*x < 0)
            return FALSE;
        if (*y < 0)
            return FALSE;
        break;
    }

    return TRUE;

}

ANIMATE *FindAnimation(int handle)
{
    ANIMATE *anim;

    anim = list;

    while (anim != NULL)
    {
        if (anim->handle == handle)
            return anim;
        anim=anim->next;
    }

    return NULL;
}

double Deg2Rad(double deg)
{
    return PI * deg / 180;   
}

double Rad2Deg(double rad)
{
    return rad * 180 / PI;
}

void KillAnimate(int h)
{
    /*
     *  Two things we need to do.
     *  - Delete from the handle list.
     *  - Take out from the linked list.
     */ 

    ANIMATE *cruise, *other;

    // The first one is very easy to do.

    handle[h] = -1; // there, that mean it is not used anymore.

    // The second thing is that we have to go through the linked list.

    cruise = list;

    if (cruise->handle == h) // begining
    {
        if (cruise->next == NULL) // nothing after
        {
            free(cruise);
            list = NULL;
        }
        else
        {
            other = cruise->next;
            free(cruise);
            list = other;
        }
    }
    else
    {
        while (cruise->next->handle != h)
            cruise = cruise->next;

        if (cruise->next->next == NULL)
        {
            free(cruise->next);
            cruise->next = NULL;
        }
        else
        {
            other = cruise->next->next;
            free(cruise->next);
            cruise->next = other;
        }
    }
}