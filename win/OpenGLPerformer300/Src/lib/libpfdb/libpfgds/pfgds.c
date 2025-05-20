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
 * pfgds.c: $Revision: 1.21 $ $Date: 2002/11/07 03:11:55 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>
//#include "wintypes.h"

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

/* length of longest input line in ".gds" file */
#define	BUFFER_SIZE	256

/* function type and argument declarations */
static pfGeoState* defaultGeoState(void);

/*
 * pfdLoadFile_gds -- Load X-Solid ".gds" files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_gds (char *fileName)
{
    FILE	*gdsFile;
    pfdGeom	*buffer;
    pfNode	*node		= NULL;
    pfVec3	*vertex		= NULL;
    pfMaterial  *m      	= NULL;

    int		 i		= 0;
    int		 numTris	= 0;
    int		 numVerts	= 0;
    int		 numPolys	= 0;

    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;

    int		 t 		= 0;

    /* open ".gds" file */
    if ((gdsFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

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

    /* initialize geoset builder */
    buffer = pfdNewGeom(2048);

    /***
     ***    Phase 1: read vertex definitions from file
     ***/

    /* skip initial "YIN" token in ".gds" file */
    fscanf(gdsFile, "%*s");

    /* read the number of vertices in ".gds" file */
    fscanf(gdsFile, "%d", &numVerts);

    vertex = (pfVec3 *)pfMalloc(numVerts*sizeof(pfVec3), NULL);

    for (i = 0; i < numVerts; i++)
	fscanf(gdsFile, "%f %f %f", 
	    &vertex[i][0], &vertex[i][1], &vertex[i][2]);

    /***
     ***    Phase 2: read polygon definitions from file
     ***/

    /* skip "0" token in ".gds" file */
    fscanf(gdsFile, "%*s");

    /* read the number of polygons in ".gds" file */
    fscanf(gdsFile, "%d", &numPolys);

    /* read polygons from ".gds" file */
    for (i = 0; i < numPolys; i++)
    {
	char	materialName[BUFFER_SIZE];
	int	v;
	int	vCount;
	int	vIndex;

	/* skip "1" token in ".gds" file */
	fscanf(gdsFile, "%*s");

	/* read the material name for this polygon */
	fscanf(gdsFile, "%s", materialName);

	/* read the number of vertices in this polygon */
	fscanf(gdsFile, "%d", &vCount);

	/* read and expand the polygon definition */
	for (v = 0; v < vCount; v++)
	{
	    /* read the index for this vertex */
	    fscanf(gdsFile, "%d", &vIndex);

	    /* copy indicated vertex to pfdGeomfer */
	    pfCopyVec3(buffer->coords[v], vertex[vIndex - 1]);
	}

	buffer->numVerts = vCount; 
	buffer->primtype = PFGS_POLYS;
	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    buffer->tbind[t] = PFGS_OFF;
	buffer->nbind = PFGS_OFF;

#if 0
	buffer->cbind = PFGS_OFF;
#else
	/* provide a default color */
	buffer->cbind = PFGS_OVERALL;
	pfuRandomize((materialName[0]<<8)|(materialName[1]));
	pfuRandomColor(buffer->colors[0], 0.2f, 0.7f);
#endif

	pfdAddBldrGeom(buffer, 1);
	numTris  += buffer->numVerts - 2;
    }

    /* close ".gds" file */
    fclose(gdsFile);

    /***
     ***    Phase 3: convert pfdGeoBuilder data to libpf structures
     ***/

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* release allocated storage */
    pfdDelGeom(buffer);
    pfFree(vertex);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_gds: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", numVerts);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input polygons:     %8ld", numTris);

    return node;
}
