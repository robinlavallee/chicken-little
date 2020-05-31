#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#define PI 3.14159


typedef struct ANIMATE
{
    int      handle;         // Handle of the animation
    int      type;           // Type of animation (linear, accelerated, rotative, etc)
    double   x1, y1;         // x and y position, depend of type
    double   extra1, extra2; // depend of type.
    double   time;           // time the animation was created
    double   duration;
	struct ANIMATE *next;  // Pointer to next animation
} ANIMATE;

typedef struct PHYSICS
{
    int sizex, sizey;       // The size of the physical field. (640x480 for us)
} PHYSICS; 

enum
{   PHYSICS_LINEAR  = 1,
    PHYSICS_ROTATE,
    PHYSICS_ACCELERATE,
    PHYSICS_HARMONIC,
    PHYSICS_INTERPOLATION,   // peforms interpolative math on chart values.
};

int AddLinearVector(double x1, double y1, double dir, double velocity, double duration);
int AddLinearCartesian(double x1, double y1, double x2, double y2, double velocity, double duration);

void InitPhysics(int x, int y);
BOOL RetrievePosition(int handle, int *x, int *y);


void LinkListAdd(ANIMATE *add);

int FindFreeHandle();
ANIMATE *FindAnimation(int handle);

void KillAnimate(int handle);

double Rad2Deg(double rad);
double Deg2Rad(double deg);

#endif // _PHYSICS_H_
