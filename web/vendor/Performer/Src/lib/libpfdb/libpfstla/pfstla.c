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
 * pfstla.c: $Revision: 1.30 $ $Date: 2002/11/07 18:06:03 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef	_POSIX_SOURCE
extern int strcasecmp (const char *s1, const char *s2);
#endif

#ifdef WIN32
#include <windows.h>
#include <limits.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

/* length of longest input line in ".stla" file */
#define	input_SIZE	256

/* case insensitive string equality test */
#ifdef WIN32
#define SAME(_a, _b)	(_stricmp(_a, _b) == 0)
#else
#define	SAME(_a, _b)	(strcasecmp(_a,_b) == 0)
#endif

/*
 * pfdLoadFile_stla -- Load 3D-lithography ".stla" files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_stla (char *fileName)
{
    FILE	*stlaFile	= NULL;
    pfdGeom	*buffer		= NULL;
    pfNode	*node		= NULL;
    char	*next		= NULL;
    int		 i		= 0;
    int		 width		= 0;
    int		 numTris	= 0;
    int		 numVerts	= 0;
    int		 numOther	= 0;
    char	 input[input_SIZE];
    char	 token[input_SIZE];
    pfMaterial  *m      	= NULL;

    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;

    int		 t              = 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open ".stla" file */
    if ((stlaFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* initialize utility library triangle/geoset builder */
    buffer = pfdNewGeom(256);

    /* pick a random not-too-dark color */
    pfuRandomize(*(int *)&startTime);
    pfuRandomColor(buffer->colors[0], 0.2f, 0.7f);

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

    /* read ".stla" file */
    while (fgets(input, input_SIZE, stlaFile) != NULL)
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
	if (SAME(token, "facet"))
	{
	    int	 count;

	    /* read face normal */
	    sscanf(next, "%*s %e %e %e",
		&buffer->norms[0][0],
		&buffer->norms[0][1],
		&buffer->norms[0][2]);

	    /* skip "outer loop" line */
	    fgets(input, input_SIZE, stlaFile);

	    /* read vertex coordinates */
	    buffer->numVerts = 0; 
	    do  
	    {
		fgets(input, input_SIZE, stlaFile);
		count = sscanf(input, "%*s %e %e %e",
		    &buffer->coords[buffer->numVerts][0],
		    &buffer->coords[buffer->numVerts][1],
		    &buffer->coords[buffer->numVerts][2]);
	    } while (count == 3 && ++buffer->numVerts < 256);

	    /* skip "endfacet" line */
	    fgets(input, input_SIZE, stlaFile);

	    /* add the polygon to the builder */
	    buffer->primtype = PFGS_POLYS;

	    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
		buffer->tbind[t] = PFGS_OFF;
	
	    buffer->nbind = PFGS_PER_PRIM;
	    buffer->cbind = PFGS_PER_PRIM;
	    pfdAddBldrGeom(buffer, 1);

	    numTris += buffer->numVerts - 2;
	    numVerts += buffer->numVerts;
	}
	else
	if (SAME(token, "color"))
	{
	    /* read new polygon color */
	    sscanf(next, "%*s %f %f %f %f",
		&buffer->colors[0],
		&buffer->colors[1],
		&buffer->colors[2],
		&buffer->colors[3]);
	}
	else
	if (SAME(token, "solid"))
	    pfuRandomColor(buffer->colors[0], 0.2f, 0.7f);
	else 
	if (SAME(token, "endsolid"))
	    ++numOther;
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_stla: unrecognized: %s", input);
	    ++numOther;
	}
    }

    /* close ".stla" file */
    fclose(stlaFile);
    pfdDelGeom(buffer);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_stla: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", numVerts);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input triangles:    %8ld", numTris);
    if (numOther != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Unknown commands:   %8ld", numOther);

    return node;
}
