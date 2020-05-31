#ifndef _PIECEDUMP_H_
#define _PIECEDUMP_H_

//#include "player.h"

typedef struct THROWNPIECES // structure used for when you are sending pieces to your neighboor. :)
{                         // This one keeps track of where the pieces are falling
    int value[SIZEX];
} THROWNPIECES;


#define DUMP_NOTHING -1
#define DUMP_EGG 0
#define DUMP_STONE 1

#define NUM_DUMP 2      // Number of different type of dumpable object.

typedef struct DUMP     // Structure used to find out what many stuff are going to fall at you
{
    int nPiece[NUM_DUMP];   // So that it is versatile if we want to add more stuff later on
} DUMP;

int DumpCount(DUMP *d); // Return the number of piece that are going to fall
int GetDumpPiece(DUMP *d);
void ThrowPiecesFillStruct(struct PLAYER *p, DUMP *d, THROWNPIECES *fall);
void Attack(struct PLAYER *p, DUMP *d);

#endif // _PIECEDUMP_H_
