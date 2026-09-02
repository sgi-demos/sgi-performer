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
 * pflsa.c: $Revision: 1.29 $ $Date: 2002/11/17 01:40:32 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFLSA_DLLEXPORT __declspec(dllexport)
#else
#define PFLSA_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFLSA_DLLEXPORT
#endif

/* length of longest input line in ".lsa" file */
#define	BUFFER_SIZE	256

#ifdef WIN32
#define strcasecmp _stricmp
#endif

/* case insensitive string equality test */
#define	SAME(_a, _b)	(strcasecmp(_a,_b) == 0)

/* function type and argument declarations */
static char *readLine(FILE *fp);
static void unpackColor(pfVec4 unpacked, int packed);

/*
 * pfdLoadFile_lsa -- Load Lightscape ".lsa" files into IRIS Performer
 */

PFLSA_DLLEXPORT /*extern */pfNode *
pfdLoadFile_lsa (char *fileName)
{
    FILE	*lsaFile;
    pfdGeom	*buffer;
    pfList	*gsetList;
    char	*input		= NULL;
    pfNode	*node		= NULL;
    int		 i		= 0;
    int		 numTris	= 0;
    int		 numVerts	= 0;
    int		 numOther	= 0;

    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;

    int		 t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open ".lsa" file */
    if ((lsaFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* disable lighting (that's what radiosity is all about ;-) */
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_OFF);
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);

    /* disable undesired automatic builder actions */
    pfdBldrMode(PFDBLDR_AUTO_NORMALS, PF_OFF);
    pfdBldrMode(PFDBLDR_AUTO_ORIENT,  PF_OFF);

    /* allocate geometry buffer structure */
    buffer = pfdNewGeom(4);

    /* skip initial view matrix in ".lsa" file */
    (void)readLine(lsaFile);
    (void)readLine(lsaFile);
    (void)readLine(lsaFile);
    (void)readLine(lsaFile);

    /* skip list of counts in ".lsa" file */
    (void)readLine(lsaFile);

    /* read polygons from ".lsa" file */
    while ((input = readLine(lsaFile)) != NULL)
    {
	char	which[BUFFER_SIZE];
	int	count;
	int	packed[4];

	/* read the polygon's definition */
	count = sscanf(input, 
	    "%s "
	    "%f %f %f %d "
	    "%f %f %f %d "
	    "%f %f %f %d "
	    "%f %f %f %d", 
	    which,
	    &buffer->coords[0][0], 
	    &buffer->coords[0][1],
	    &buffer->coords[0][2], 
	    &packed[0],
	    &buffer->coords[1][0], 
	    &buffer->coords[1][1],
	    &buffer->coords[1][2], 
	    &packed[1],
	    &buffer->coords[2][0], 
	    &buffer->coords[2][1],
	    &buffer->coords[2][2], 
	    &packed[2],
	    &buffer->coords[3][0], 
	    &buffer->coords[3][1],
	    &buffer->coords[3][2], 
	    &packed[3]);

	/* process triangles */
	if (which[0] == 't' && which[1] == '\0')
	{
	    buffer->numVerts = 3; 
	    buffer->primtype = PFGS_POLYS;
	    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
		buffer->tbind[t] = PFGS_OFF;
	    buffer->nbind = PFGS_OFF;
	    buffer->cbind = PFGS_PER_VERTEX;

	    for (i = 0; i < buffer->numVerts; i++)
		unpackColor(buffer->colors[i], packed[i]);

	    numTris  += buffer->numVerts - 2;
	    numVerts += buffer->numVerts;

	    pfdAddBldrGeom(buffer, 1);
	}
	else
	/* process triangle meshes */
	if (which[0] == 't' && which[1] == 'm')
	{
	    pfVec3	v[3];
	    pfVec4	c[3];
	    int	p;

	    sscanf(input, "%s %d", which, &count);

	    for (i = 0; i < count; i++)
	    {
		/* save old vertices */
		pfCopyVec3(v[0], v[1]); 
		pfCopyVec3(v[1], v[2]);
		pfCopyVec4(c[0], c[1]); 
		pfCopyVec4(c[1], c[2]);

		/* get new vertex */
		readLine(lsaFile);
		sscanf(input, "%f %f %f %d",
		    &v[2][0], &v[2][1], &v[2][2], &p);
		unpackColor(c[2], p);

		/* genetate facet */
		if (i >= 2)
		{
		    buffer->numVerts = 3; 
		    buffer->primtype = PFGS_POLYS;
		    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
			buffer->tbind[t] = PFGS_OFF;
		    buffer->nbind = PFGS_OFF;
		    buffer->cbind = PFGS_PER_VERTEX;

		    if ((i & 1) == 0)
		    {
			pfCopyVec3(buffer->coords[0], v[0]);
			pfCopyVec3(buffer->coords[1], v[1]);
			pfCopyVec3(buffer->coords[2], v[2]);

			pfCopyVec4(buffer->colors[0], c[0]);
			pfCopyVec4(buffer->colors[1], c[1]);
			pfCopyVec4(buffer->colors[2], c[2]);
		    }
		    else
		    {
			pfCopyVec3(buffer->coords[0], v[0]);
			pfCopyVec3(buffer->coords[1], v[2]);
			pfCopyVec3(buffer->coords[2], v[1]);

			pfCopyVec4(buffer->colors[0], c[0]);
			pfCopyVec4(buffer->colors[1], c[2]);
			pfCopyVec4(buffer->colors[2], c[1]);
		    }

		    numTris  += buffer->numVerts - 2;
		    numVerts += buffer->numVerts;

		    pfdAddBldrGeom(buffer, 1);
		}
	    }
	}
	else
	/* process quads */
	if (which[0] == 'q' && which[1] == '\0')
	{
	    buffer->numVerts = 4; 
	    buffer->primtype = PFGS_POLYS;
	    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
		buffer->tbind[t] = PFGS_OFF;
	    buffer->nbind = PFGS_OFF;
	    buffer->cbind = PFGS_PER_VERTEX;

	    for (i = 0; i < buffer->numVerts; i++)
		unpackColor(buffer->colors[i], packed[i]);

	    numTris  += buffer->numVerts - 2;
	    numVerts += buffer->numVerts;

	    pfdAddBldrGeom(buffer, 1);
	}
	else
	/* process quad meshes */
	if (which[0] == 'q' && which[1] == 'm')
	{
	    pfVec3	v[3];
	    pfVec4	c[3];
	    int	p;

	    sscanf(input, "%s %d", which, &count);

	    for (i = 0; i < count; i++)
	    {
		/* save old vertices */
		pfCopyVec3(v[0], v[1]); 
		pfCopyVec3(v[1], v[2]);
		pfCopyVec4(c[0], c[1]); 
		pfCopyVec4(c[1], c[2]);

		/* get new vertex */
		readLine(lsaFile);
		sscanf(input, "%f %f %f %d",
		    &v[2][0], &v[2][1], &v[2][2], &p);
		unpackColor(c[2], p);

		/* genetate facet */
		if (i >= 2)
		{
		    buffer->numVerts = 3; 
		    buffer->primtype = PFGS_POLYS;
		    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
			buffer->tbind[t] = PFGS_OFF;
		    buffer->nbind = PFGS_OFF;
		    buffer->cbind = PFGS_PER_VERTEX;

		    if ((i & 1) == 1)
		    {
			pfCopyVec3(buffer->coords[0], v[0]);
			pfCopyVec3(buffer->coords[1], v[1]);
			pfCopyVec3(buffer->coords[2], v[2]);

			pfCopyVec4(buffer->colors[0], c[0]);
			pfCopyVec4(buffer->colors[1], c[1]);
			pfCopyVec4(buffer->colors[2], c[2]);
		    }
		    else
		    {
			pfCopyVec3(buffer->coords[0], v[0]);
			pfCopyVec3(buffer->coords[1], v[2]);
			pfCopyVec3(buffer->coords[2], v[1]);

			pfCopyVec4(buffer->colors[0], c[0]);
			pfCopyVec4(buffer->colors[1], c[2]);
			pfCopyVec4(buffer->colors[2], c[1]);
		    }

		    numTris  += buffer->numVerts - 2;
		    numVerts += buffer->numVerts;

		    pfdAddBldrGeom(buffer, 1);
		}
	    }
	}
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_lsa: unrecognized: %s", input);
	    ++numOther;
	}
    }

    /* close ".lsa" file */
    fclose(lsaFile);

    /* release allocated storage */
    pfdDelGeom(buffer);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* release storage allocated by the builder */
    pfdResetBldrGeometry();

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_lsa: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", numVerts);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input polygons:     %8ld", numTris);
    if (numOther != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Unknown commands:   %8ld", numOther);

    return node;
}

/*
 *	read next significant line from file
 */
static char *
readLine (FILE *fp)
{
    char	*next;
    static char	 input[BUFFER_SIZE];

    while (fgets(input, BUFFER_SIZE, fp) != NULL)
    {
	/* find first non-"space" character in line */
	for (next = input; *next != '\0' && isspace(*next); next++)
	    /* EMPTY */ {};
	
	/* skip blank lines and comments */
	if (*next == '\0' || *next == '#' || *next == '!' || *next == '$')
	    continue;

	/* found something */
	return input;
    }

    /* reached EOF */
    return NULL;
}

static void 
unpackColor(pfVec4 unpacked, int packed)
{
    unpacked[0] =  (packed        & 0xff)/255.0f;
    unpacked[1] = ((packed >>  8) & 0xff)/255.0f;
    unpacked[2] = ((packed >> 16) & 0xff)/255.0f;
    unpacked[3] =   1.0f;
}
