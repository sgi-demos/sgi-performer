/*
 * Copyright 1994, 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * This source code ("Source Code") was originally derived from a
 * code base owned by Silicon Graphics, Inc. ("SGI")
 * 
 * LICENSE: SGI grants the user ("Licensee") permission to reproduce,
 * distribute, and create derivative works from this Source Code,
 * provided that: (1) the user reproduces this entire notice within
 * both source and binary format redistributions and any accompanying
 * materials such as documentation in printed or electronic format;
 * (2) the Source Code is not to be used, or ported or modified for
 * use, except in conjunction with OpenGL Performer; and (3) the
 * names of Silicon Graphics, Inc.  and SGI may not be used in any
 * advertising or publicity relating to the Source Code without the
 * prior written permission of SGI.  No further license or permission
 * may be inferred or deemed or construed to exist with regard to the
 * Source Code or the code base of which it forms a part. All rights
 * not expressly granted are reserved.
 * 
 * This Source Code is provided to Licensee AS IS, without any
 * warranty of any kind, either express, implied, or statutory,
 * including, but not limited to, any warranty that the Source Code
 * will conform to specifications, any implied warranties of
 * merchantability, fitness for a particular purpose, and freedom
 * from infringement, and any warranty that the documentation will
 * conform to the program, or any warranty that the Source Code will
 * be error free.
 * 
 * IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
 * LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
 * ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
 * SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
 * OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
 * PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
 * OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
 * USE, THE SOURCE CODE.
 * 
 * Contact information:  Silicon Graphics, Inc., 
 * 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
 * or:  http://www.sgi.com
 */

/*
 * pfm.c: $Revision: 1.6 $ $Date: 2002/11/07 04:02:08 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#include <windows.h>
//#include "wintypes.h"

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#ifdef	_POSIX_SOURCE
extern int strcasecmp (const char *s1, const char *s2);
#endif

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

/* length of longest input line in ".m" file */
#define	BUFFER_SIZE	2048

/* initial allocation size for growable arrays */
#define	CHUNK		2048

/* case insensitive string equality test */
#ifdef WIN32
#define SAME(_a, _b)	(_stricmp(_a, _b) == 0)
#else
#define	SAME(_a, _b)	(strcasecmp(_a,_b) == 0)
#endif

#define	GROW(_v, _t, _num) \
    if (_v == NULL) \
    { \
	_v ## Available = CHUNK; \
	if (_v ## Available < _num) \
	    _v ## Available = _num; \
	_v = (_t *)pfMalloc(sizeof(_t)*_v ## Available, NULL); \
    } \
    else \
    if (_num >= _v ## Available) \
    { \
	_v ## Available *= 2; \
	if (_v ## Available < _num) \
	    _v ## Available = _num; \
	_v = (_t *)pfRealloc(_v, sizeof(_t)*_v ## Available); \
    } 

typedef struct polyVertex 
{
    pfVec3	xyz;
} polyVertex;

/*
 * pfdLoadFile_m -- Load Univ. Wash ".m" files 
 */

static char *
getNextLine (FILE *file, char *buffer, int bufferSize)
{
    char *next;

    /* read file */
    while (fgets(buffer, bufferSize, file) != NULL)
    {
	/* find first non-"space" character in line */
	for (next = buffer; *next != '\0' && isspace(*next); next++)
	    /* EMPTY */ {};
	
	/* skip blank lines and comments */
	if (*next == '\0' || *next == '#' || *next == '!' || *next == '$')
	    continue;
	
	/* return pointer to first non-"space" character in line */
	return next;
    }

    /* fgets() failed -- must be EOF or ERROR */
    return NULL;
}
	
/*
 * pfdLoadFile_m -- Load ".m" files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_m (char *fileName)
{
    FILE	*polyFile;
    pfdGeom	*geom;
    pfNode	*node		= NULL;
    int		 width		= 0;
    int		 i;
    int		 numTris	= 0;
    int		 maxIndex	= 0;
    int		 geomSize	= 4;
    char	 input[BUFFER_SIZE];
    char	 token[BUFFER_SIZE];
    char	*next		= NULL;
    pfMaterial  *m      	= NULL;
    double	 startTime	= pfGetTime();

    /* growable vertex list (xyz, rgba, norm, st) */
    polyVertex	*v		= NULL; 
    unsigned int vCount		= 0;
    unsigned int vAvailable	= 0;

    int		t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open ".m" file */
    if ((polyFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* allocate a new primitive buffer */
    geom = pfdNewGeom(geomSize);

#define RANDOM_COLOR
#ifdef  RANDOM_COLOR
    /* pick a random not-too-dark color */
    pfuRandomize(*(int *)&startTime);
    pfuRandomColor(geom->colors[0], 0.2f, 0.7f);
#endif

    /* establish material definition for model */
    m = (pfMaterial*)pfdGetTemplateObject(pfGetMtlClassType());
    pfMtlColor(m, PFMTL_AMBIENT,  0.2f, 0.2f, 0.2f);
    pfMtlColor(m, PFMTL_DIFFUSE,  0.8f, 0.8f, 0.8f);
    pfMtlColor(m, PFMTL_SPECULAR, 1.0f, 1.0f, 1.0f);
    pfMtlShininess(m, 30.0f);
    pfMtlColorMode(m, PFMTL_FRONT, PFMTL_CMODE_AD);
    pfdBldrStateAttr(PFSTATE_FRONTMTL, m);
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_ON);
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);

    /* read "Vertex" definitions from ".m" file */
    while (1)
    {
	int	index;

	/* get next significant line from file */
	next = getNextLine(polyFile, input, BUFFER_SIZE);

	if (next == NULL)
	    return NULL;

	/* if token is not vertex number then look for polys */
	if (sscanf(next, "Vertex %d%n", &index, &width) != 1)
	    break;
	next += width;

	if (maxIndex < index)
	    maxIndex = index;

	/* enlarge vertex list */
	GROW(v, polyVertex, index);

	/* parse vertex coordinates */
	sscanf(next, "%f %f %f%n", 
	    &v[index - 1].xyz[0], 
	    &v[index - 1].xyz[1], 
	    &v[index - 1].xyz[2], 
	    &width);
	next += width;

	/* advance vertex count */
	++vCount;
    }

    /* read polygon definitions from file */
    while (next = getNextLine(polyFile, input, BUFFER_SIZE))
    {
	int	index;
	int	a, b, c;
	
	/* extract token from line */
	if (sscanf(next, "%s %d %d %d %d", token, &index, &a, &b, &c) != 5)
	    continue;

	/* if token is not "Face" then skip this line */
	if (!SAME(token, "Face"))
	    continue;

	/* copy valid vertex attributes to builder */
	pfCopyVec3(geom->coords[0], v[a - 1].xyz);
	pfCopyVec3(geom->coords[1], v[b - 1].xyz);
	pfCopyVec3(geom->coords[2], v[c - 1].xyz);

	/* add polygon to builder */
	geom->numVerts = 3;
	geom->primtype = PFGS_POLYS;
	geom->cbind = PFGS_PER_PRIM;
	geom->nbind = PFGS_OFF;

	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    geom->tbind[t] = PFGS_OFF;

	pfdAddBldrGeom(geom, 1);
	numTris++;
    }

    /* close ".m" file */
    fclose(polyFile);
    pfdDelGeom(geom);

    /* release dynamically allocated vertex data */
    if (v != NULL) 
	pfFree(v);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_m: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", vCount);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input triangles:    %8ld", numTris);

    /* return pointer to in-memory scene-graph */
    return node;
}
