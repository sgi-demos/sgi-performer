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
 * pfpts.c: $Revision: 1.5 $ $Date: 2002/11/07 04:02:08 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#ifdef	_POSIX_SOURCE
extern int strcasecmp (const char *s1, const char *s2);
#endif

#include <Performer/pf.h>

#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

/* length of longest input line in ".pts" file */
#define	BUFFER_SIZE	256

/* case insensitive string equality test */
#ifdef WIN32
#define SAME(_a, _b)	(_stricmp(_a, _b) == 0)
#else
#define	SAME(_a, _b)	(strcasecmp(_a,_b) == 0)
#endif

/*
 * pfdLoadFile_pts -- Load UWASH ".pts" files 
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_pts (char *fileName)
{
    FILE	*pointFile;
    char	 buffer[BUFFER_SIZE];
    char	 token[BUFFER_SIZE];
    char	*next		= NULL;
    pfNode	*node		= NULL;
    int		 width		= 0;
    int		 numPoints	= 0;
    pfdGeom	*prim		= NULL;
    float	 brightness	= 0.0f;
    pfMaterial  *m		= NULL;

    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;
    int		 t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open ".pts" file */
    if ((pointFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* allocate primitive buffer */
    prim = pfdNewGeom(2);

    /* disable lighting */
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_OFF);

    /* define a pfMaterial for the points */
    m = (pfMaterial*)
	pfdGetTemplateObject(pfGetMtlClassType());
    pfMtlColor(m, PFMTL_AMBIENT,  1.0f, 1.0f, 1.0f);
    pfMtlColor(m, PFMTL_DIFFUSE,  1.0f, 1.0f, 1.0f);
    pfMtlColor(m, PFMTL_SPECULAR, 0.0f, 0.0f, 0.0f);
    pfMtlShininess(m, 0.0f);
    pfMtlColorMode(m, PFMTL_FRONT, PFMTL_CMODE_OFF);
    pfdBldrStateAttr(PFSTATE_FRONTMTL, m);

    /* disable lighting */
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_OFF);
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_OFF);

    /* read ".pts" file */
    while (fgets(buffer, BUFFER_SIZE, pointFile) != NULL)
    {
	/* find first non-"space" character in line */
	for (next = buffer; *next != '\0' && isspace(*next); next++)
	    /* EMPTY */ {};
	
	/* skip blank lines and comments */
	if (*next == '\0' || *next == '#' || *next == '!' || *next == '$')
	    continue;
	
	/* extract token */
	sscanf(next, "%s%n", token, &width);
	next += width;

	/* identify token */
	if (SAME(token, "p"))
	{
	    /* read point definition from string */
	    sscanf(next, "%f %f %f %f", 
		&prim->coords[0][PF_X], 
		&prim->coords[0][PF_Y], 
		&prim->coords[0][PF_Z],
		&brightness);


	    /* specify point information */
	    prim->numVerts = 1;
	    prim->primtype = PFGS_POINTS;
	    for (t = 1 ; t < PF_MAX_TEXTURES ; t ++)
		prim->tbind[t] = PFGS_OFF;
	    prim->nbind = PFGS_OFF;
	    prim->cbind = PFGS_OFF;
	    prim->pixelsize = 1.0f;

	    /* add this facet to builder */
	    pfdAddBldrGeom(prim, 1);

	    /* keep total counts */
	    ++numPoints;
	}
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_pts: unrecognized: %s", buffer);
	}
    }

    /* close ".pts" file */
    fclose(pointFile);

    /* release primitive buffer */
    pfdDelGeom(prim);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();

    /* use file name for top-level pfNode */
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_pts: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input points:       %8ld", numPoints);

    return node;
}
