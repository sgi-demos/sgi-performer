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
 * pftri.c: $Revision: 1.12 $ $Date: 2002/11/07 18:06:03 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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

/*
 * pfdLoadFile_tri -- Load ".tri" data files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_tri (char *fileName)
{
    FILE	*triFile	= NULL;
    pfNode	*node		= NULL;
    pfdGeom	*prim		= NULL;
    int		 v		= 0;
    int		 numTris	= 0;
    pfMaterial  *m      	= NULL;
    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;
    int		 t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open ".tri" file */
    if ((triFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* allocate primitive buffer */
    prim = pfdNewGeom(3);

    /* pick a different set of colors each time */
    pfuRandomize(*(int *)&startTime);

    /* pick a random not-too-dark color */
    pfuRandomColor(prim->colors[0], 0.4f, 0.8f);

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

    /* specify control data */
    prim->numVerts = 3;
    prim->primtype = PFGS_POLYS;
    prim->nbind = PFGS_PER_VERTEX;
    prim->cbind = PFGS_PER_PRIM;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	prim->tbind[t] = PFGS_OFF;

    /* read triangles from ".tri" file */
    while (!feof(triFile))
    {
	/* read vertices from ".tri" file */
	for (v = 0; v < 3; v++)
	{
	    /* read vertex data */
	    fscanf(triFile, "%f %f %f", 
		&prim->coords[v][PF_X], 
		&prim->coords[v][PF_Y], 
		&prim->coords[v][PF_Z]);
	    fscanf(triFile, "%f %f %f", 
		&prim->norms[v][PF_X], 
		&prim->norms[v][PF_Y], 
		&prim->norms[v][PF_Z]);
	}

	/* add this line to builder */
	pfdAddBldrGeom(prim, 1);
	++numTris;
    }

    /* close ".tri" file */
    fclose(triFile);

    /* release primitive buffer */
    pfdDelGeom(prim);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();

    /* use indicated name for top-level pfNode */
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_tri: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input triangles:    %8ld", numTris);

    return node;
}
