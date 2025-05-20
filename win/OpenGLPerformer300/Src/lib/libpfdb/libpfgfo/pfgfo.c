/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
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
 * pfgfo.c: $Revision: 1.44 $ $Date: 2002/11/17 01:40:32 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef	_POSIX_SOURCE
extern int strcasecmp (const char *s1, const char *s2);
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#ifdef WIN32
#define strcasecmp _stricmp
#endif

#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFGFO_DLLEXPORT __declspec(dllexport)
#else
#define PFGFO_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFGFO_DLLEXPORT
#endif

/* length of longest input line in ".gfo" file */
#define	BUFFER_SIZE	20000

/* maximum number of vertices in a single polygon */
#define	FACE_SIZE	4096

/* initial allocation size for growable arrays */
#define	CHUNK		20000

/* case insensitive string equality test */
#define	SAME(_a, _b)	(strcasecmp(_a,_b) == 0)

/* function type and argument declarations */
static pfGeoState* defaultGeoState(void);

#define	GROW(_v, _t) \
    if (_v == NULL) \
    { \
	_v ## Available = CHUNK; \
	_v = (_t *)pfMalloc(sizeof(_t)*_v ## Available, NULL); \
    } \
    else \
    if (_v ## Count >= _v ## Available) \
    { \
	_v ## Available *= 2; \
	_v = (_t *)pfRealloc(_v, sizeof(_t)*_v ## Available); \
    } 

/*
 * pfdLoadFile_gfo -- Load SGI ".gfo" files into IRIS Performer
 */

PFGFO_DLLEXPORT /*extern */ pfNode *
pfdLoadFile_gfo (char *fileName)
{
    FILE	*gfoFile;
    pfdGeom	*buffer;
    pfNode	*node		= NULL;
    int		 width		= 0;
    int		 i;
    int		 numTris	= 0;
    int		 numGroups	= 0; 
    int		 numSkip	= 0;
    int		 numOther	= 0;
    char	 input[BUFFER_SIZE];
    char	 token[BUFFER_SIZE];
    char	*next		= NULL;
    char	*backslash	= NULL;

    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;

    /* growable coordinate list (X, Y, Z) */
    pfVec3	*v		= NULL; 
    unsigned int	 vCount		= 0;
    unsigned int	 vAvailable	= 0;

    /* growable color list (r, g, b, a) */
    pfVec4	*c		= NULL; 
    unsigned int	 cCount		= 0;
    unsigned int	 cAvailable	= 0;

    int			t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open SGI ".gfo" file file */
    if ((gfoFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* allocate a new primitive buffer */
    buffer = pfdNewGeom(4096);

    /* disable lighting (that's what radiosity is all about ;-) */
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_OFF);
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);

    /* disable undesired automatic builder actions */
    pfdBldrMode(PFDBLDR_AUTO_NORMALS, PF_OFF);
    pfdBldrMode(PFDBLDR_AUTO_ORIENT,  PF_OFF);

    /* read ".gfo" file */
    while (fgets(input, BUFFER_SIZE, gfoFile) != NULL)
    {
	/* find first non-"space" character in line */
	for (next = input; *next != '\0' && isspace(*next); next++)
	    /* EMPTY */ {};
	
	/* skip blank lines and comments */
	if (*next == '\0' || *next == '#' || *next == '!' || *next == '$')
	    continue;
	
	/* extract token */
	sscanf(next, "%s%n", token, &width);
	next += width;

	/* identify token */
	if (SAME(token, "cpack"))
	{
	    int		packed = 0;

	    /* enlarge color list */
	    GROW(c, pfVec4);

	    /* read color value from string */
	    next = strchr(next, '{');
	    sscanf(++next, "0x%x", &packed);

	    /* add color to list */
            c[cCount][0] =  (packed        & 0xff)/255.0f;
            c[cCount][1] = ((packed >>  8) & 0xff)/255.0f;
            c[cCount][2] = ((packed >> 16) & 0xff)/255.0f;
            c[cCount][3] =   1.0f;

	    /* advance color count */
	    ++cCount;
	}
	else
	if (SAME(token, "v3f"))
	{
	    /* enlarge vertex coordinate list */
	    GROW(v, pfVec3);

	    /* read coordinate from string */
	    next = strchr(next, '{');
	    sscanf(++next, "%f %f %f", 
		&v[vCount][PF_X], 
		&v[vCount][PF_Y], 
		&v[vCount][PF_Z]);

	    /* advance vertex count */
	    ++vCount;
	}
	else
	if (SAME(token, "polygon") ||
	    SAME(token, "lines"))
	{
	    int	count;
	    int	ci[FACE_SIZE];
	    int	vi[FACE_SIZE];

	    /* parse vertex data from input buffer */
	    for (count = 0; count < FACE_SIZE && next != NULL; count++)
	    {
		/* read next color index */
		if (((next = strchr(next, '[')) == NULL) ||
		    (sscanf(++next, "%d ", &ci[count]) != 1))
		    break;

		/* read next vertex index */
		if (((next = strchr(next, '[')) == NULL) ||
		    (sscanf(++next, "%d ", &vi[count]) != 1))
		    break;
	    }

	    buffer->numVerts = count;
	    buffer->primtype = PFGS_POLYS;
	    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
		buffer->tbind[t] = PFGS_OFF;
	    buffer->nbind = PFGS_OFF;
	    buffer->cbind = PFGS_PER_VERTEX;
	    for (i = 0; i < count; i++)
	    {
		pfCopyVec3(buffer->coords[i], v[vi[count - i - 1]]);
		pfCopyVec4(buffer->colors[i], c[ci[count - i - 1]]);
	    }
	    pfdAddBldrGeom(buffer, 1);
	    numTris += buffer->numVerts - 2;
	}
	else
	if (SAME(token, "instance"))
	    break;
	else
	if (SAME(token, "scope") || SAME(token, "}"))
	    ++numSkip;
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_gfo: \"%s\"", input);
	    ++numOther;
	}
    }

    /* close ".gfo" file */
    fclose(gfoFile);
    pfdDelGeom(buffer);

    /* release dynamically allocated vertex and color data */
    if (v != NULL) pfFree(v);
    if (c != NULL) pfFree(c);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* release storage allocated by the builder */
    pfdResetBldrGeometry();

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_gfo: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    if (numSkip != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    commands skipped:   %8ld", numSkip);
    if (numOther != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    commands unknown:   %8ld", numOther);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", vCount);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input colors:       %8ld", cCount);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input triangles:    %8ld", numTris);

    /* return pointer to in-memory scene-graph */
    return node;
}
