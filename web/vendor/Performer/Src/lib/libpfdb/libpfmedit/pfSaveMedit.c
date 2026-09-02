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




/************************************************************************
Write a Performer database to a Medit file
************************************************************************/

/************************************************************************
Defines for Perfomer version
************************************************************************/
#if 1
#define PERFORMER2
#endif

#ifdef PERFORMER2
typedef int pflong;
#else
typedef long pflong;
#endif

/************************************************************************
Header files
************************************************************************/
#include <Performer/pf.h>
#ifdef PERFORMER2
#include <Performer/pfdu.h>
#endif
#define USEFULMATH
#include "Meditstuff/useful.h"
#include "libmedit.h"

#define _DEBUGGING
FILE *debug;
#define DEBUG fprintf(debug,


/************************************************************************
Include files
************************************************************************/
#include "Meditstuff/data.c"
#include "Meditstuff/mat+tex.c"
#include "Meditstuff/maths.c"
#include "Meditstuff/objects.c"
#include "Meditstuff/geometry.c"
#include "Meditstuff/saver.c"

/************************************************************************
Do it
************************************************************************/
int
pfdStoreFile_medit(pfNode *root, char *file)
{
    int ret;
#ifdef DEBUGGING
    debug = stderr;
#else
    debug = fopen("/dev/null", "w");
#endif
    printf("\n\n<<<<< pfSaveMedit version 1.5.01 >>>>>\n");
    printf("Saving Medit file: %s\n",file);
    if (ConvertToMedit(root)) {
	printf("Materials=%d\n", MaterialsCreated);
	printf("Objects=%d\n", ObjectsCreated);
	printf("Replications=%d\n", ObjectsReplicated);
	printf("Polygons=%d\n", PolygonsCreated);
	ret = WriteMedit(file);
    }
    else {
	printf("Medit saver: Failed to convert Performer data :(\n");
	ret = FALSE;
    }
#ifndef DEBUGGING
	fclose(debug);
#endif
    return ret;
}

