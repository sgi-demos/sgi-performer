/*--------------------------Message start-------------------------------

This software is copyright (C) 1994-1995 by Medit Productions


This software is provided under the following terms, if you do not
agree to all of these terms, delete this software now. Any use of
this software indicates the acceptance of these terms by the person
making said usage ("The User").

1) Medit Productions provides this software "as is" and without
warranty of any kind. All consequences of the use of this software
fall completely on The User.

2) This software may be copied without restriction, provided that an
unaltered and clearly visible copy of this message is placed in any
copies of, or derivatives of, this software.


---------------------------Message end--------------------------------*/




#ifndef __USEFUL__
#define __USEFUL__

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>
#ifndef WIN32
#include <values.h>
#include <unistd.h>
#endif
#include <errno.h>

#ifndef __linux__
#ifdef	_POSIX_SOURCE
extern char *strdup (const char *s1);
extern char *sys_errlist[];
#endif
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef ON
#define ON 1
#define OFF 0
#endif

typedef int int32;
typedef unsigned int uint32;

typedef int flag;
#ifndef WIN32
typedef int boolean;
#endif
typedef uint32 lcolour;
typedef unsigned char byte;

#define IS ==
#define ISNT !=
#define AND &&
#define OR ||
#define TheCowsAreHome FALSE

#define reg register
#define until(x) while(!(x))
#define MaxOf(x,y) (((x)>(y))?x:y)
#define MinOf(x,y) (((x)<(y))?x:y)
#define swap(x,y) { temp = x; x = y; y = temp; }
#define square(x) ((x)*(x))
#define deg(x) ((x) * (180/M_PI))
#define rad(x) ((x) * (M_PI/180))
#define sgn(n) ((n>0)?1:((n<0)?-1:0))
#ifndef WIN32
void itoa(int n, char *s);
#endif

#define X 0
#define Y 1
#define Z 2
#define W 3
#define XY 2
#define XYZ 3
#define XYZW 4

#ifdef USEFULMATH
typedef double real;
typedef float smallreal;
typedef real MatrixRow[4];
typedef MatrixRow MMatrix[4];
typedef MatrixRow *MatrixPtr;
#define Matrix MMatrix
typedef real Coordinate[XYZ];
typedef real *CoordinatePtr;
typedef real Vector[XYZ];
typedef real *VectorPtr;
#endif

/* Not really true, but I'm not sure what it should be. This should be enough for anything */
/* ps. even WorkSpace is confused about this, if you make a really long pathname it crashes! */
#define MaxFileName 4096


#endif
