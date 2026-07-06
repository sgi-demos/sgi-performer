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
 * pfunc.c: $Revision: 1.14 $ $Date: 2002/11/07 18:06:03 $
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
#include <Performer/pfdu.h>

/* length of longest input line in ".unc" file */
#define	BUFFER_SIZE	256

/* case insensitive string equality test */
#ifdef WIN32
#define SAME(_a, _b)	(_stricmp(_a, _b) == 0)
#else
#define	SAME(_a, _b)	(strcasecmp(_a,_b) == 0)
#endif

/*
 * pfdLoadFile_unc -- Load ".unc" data files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_unc (char *fileName)
{
    FILE	*uncFile;
    char	 buffer[BUFFER_SIZE];
    pfNode	*node		= NULL;
    int		 numVerts	= 0;
    int		 maxVerts	= 256;
    int		 totalTris	= 0;
    int		 totalVerts	= 0;
    pfdGeom	*prim		= NULL;
    pfMaterial  *m      	= NULL;

    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;

    int		 t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open ".unc" file */
    if ((uncFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* allocate primitive buffer */
    prim = pfdNewGeom(maxVerts);

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

    /* read ".unc" file */
    while (fgets(buffer, BUFFER_SIZE, uncFile) != NULL)
    {
	int count;
	int v;
	int hasNorms;
	int c[8];

	/* parse control information */
	count = sscanf(buffer, "%d %d %d %d %d %d %d %d", 
	    &c[0], &c[1], &c[2], &c[3], 
	    &c[4], &c[5], &c[6], &c[7]);

	/* get number of vertices */
	numVerts = (count == 4) ? c[3] : c[6];

	/* read vertices from file */
	for (v = 0, hasNorms = 1; 
	    v < numVerts && fgets(buffer, BUFFER_SIZE, uncFile) != NULL; 
	    v++)
	{
	    if (sscanf(buffer, "%f %f %f %f %f %f", 
		&prim->coords[v][PF_X], 
		&prim->coords[v][PF_Y], 
		&prim->coords[v][PF_Z],
		&prim->norms[v][PF_X], 
		&prim->norms[v][PF_Y], 
		&prim->norms[v][PF_Z]
		) != 6)
		hasNorms = 0;
	}

	/* specify polygon color */
	prim->colors[0][0] = c[0]/255.0f;
	prim->colors[0][1] = c[1]/255.0f;
	prim->colors[0][2] = c[2]/255.0f;
	prim->colors[0][3] = 1.0f;

	/* specify polygon control data */
	prim->numVerts = numVerts;
	prim->primtype = PFGS_POLYS;
        for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    prim->tbind[t] = PFGS_OFF;
	prim->nbind = hasNorms ? PFGS_PER_VERTEX : PFGS_OFF;
	prim->cbind = PFGS_PER_PRIM;

/* #define	REVERSE_POLYGON_ORDER */
#ifdef	REVERSE_POLYGON_ORDER
	/* these files seem to be CW. reverse to CCW */
	pfdReverseGeom(prim);
#endif

	/* add this facet to builder */
	pfdAddBldrGeom(prim, 1);

	/* keep total counts */
	totalTris += prim->numVerts - 2;
	totalVerts += prim->numVerts;
    }

    /* close ".unc" file */
    fclose(uncFile);

    /* release primitive buffer */
    pfdDelGeom(prim);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();

    /* use indicated name for top-level pfNode */
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_unc: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", totalVerts);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input polygons:     %8ld", totalTris);

    return node;
}
