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
 * pfspf.c: $Revision: 1.12 $ $Date: 2002/11/07 18:06:03 $
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

#define	BUFFER_SIZE	1024

/*
 * pfdLoadFile_spf -- Load ".spf" data files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_spf (char *fileName)
{
    FILE	*spfFile	= NULL;
    pfNode	*node		= NULL;
    pfVec3	*vertData	= NULL;
    pfdGeom	*prim		= NULL;
    int 	 i		= 0;
    int		 numVerts	= 0;
    int		 numPolys	= 0;
    int		 maxVerts	= 0;
    int		 inputPolys	= 0;
    int		 totalTris	= 0;
    int		 totalVerts	= 0;
    pfVec4	 rgba;
    char	 buffer[BUFFER_SIZE];
    pfMaterial  *m      	= NULL;
    double	 startTime	= pfGetTime();
    int		 t 		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open ".spf" file */
    if ((spfFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* skip "identifying string" at top of file */
    if (fgets(buffer, BUFFER_SIZE, spfFile) == NULL)
    {
	fclose(spfFile);
	return NULL;
    }

    /* read count information from ".spf" file */
    if (fscanf(spfFile, "%d,%d\n", &numVerts, &numPolys) != 2)
    {
	fclose(spfFile);
	return NULL;
    }
    
    /* read vertex information from ".spf" file */
    vertData = (pfVec3 *)calloc(numVerts, sizeof(pfVec3));
    for (i = 0; i < numVerts; i++)
    {
	/* get next line */
	if (fgets(buffer, BUFFER_SIZE, spfFile) == NULL)
	{
	    free(vertData);
	    fclose(spfFile);
	    return NULL;
	}

	/* unspecified values default to zero */
	pfSetVec3(vertData[i], 0.0f, 0.0f, 0.0f);

	/* note that data is comma separated */
	sscanf(buffer, "%f,%f,%f", 
	    &vertData[i][PF_X], 
	    &vertData[i][PF_Y], 
	    &vertData[i][PF_Z]);
    }

    /* allocate primitive buffer */
    prim = pfdNewGeom(maxVerts = 256);

    /* pick a different set of colors each time */
    pfuRandomize(*(int *)&startTime);

    /* pick a random not-too-dark color */
    pfuRandomColor(rgba, 0.4f, 0.8f);

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

    /* read polygon information from ".spf" file */
    for (i = 0; i < numPolys; i++)
    {
	int	 k	= 0;
	int	 v	= 0;
	int	 numV	= 0;
	int	 width	= 0;
	int	 index	= 0;
	char 	*next	= buffer;

	/* get next line */
	if (fgets(buffer, BUFFER_SIZE, spfFile) == NULL)
	{
	    pfdDelGeom(prim);
	    free(vertData);
	    fclose(spfFile);
	    return NULL;
	}

	/* parse polygon's vertex count */
	sscanf(next, "%d%n", &numV, &width);
	next += width;

	/* enlarge primitive buffer if needed */
	if (numV >= maxVerts)
	    pfdResizeGeom(prim, maxVerts = 2*numV);

	/* read polygon indices and build primitive */
	for (v = 0; v < numV; v++)
	{
	    /* read index from file */
	    sscanf(next, ",%d%n", &index, &width);
	    next += width;

	    /* adjust for 1-based indexing */
	    index--;

	    /* check vertex */
	    if (index < 0 || index >= numVerts)
		pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_spf: bad vertex index");
	    else
	    {
		/* dereference vertex array */
		pfCopyVec3(prim->coords[k], vertData[index]);

		/* advance vertex count */
		k++;
	    }
	}

	/* k is the number of valid vertices in this polygon */
	if (k >= 3)
	{
	    /* set polygon color */
	    pfCopyVec4(prim->colors[0], rgba);

	    /* specify polygon control data */
	    prim->numVerts = k;
	    prim->primtype = PFGS_POLYS;
	    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
		prim->tbind[t] = PFGS_OFF;
	    prim->nbind = PFGS_OFF;
	    prim->cbind = PFGS_PER_PRIM;

	    /* add this facet to builder */
	    pfdAddBldrGeom(prim, 1);

	    /* keep total counts */
	    ++inputPolys;
	    totalTris += prim->numVerts - 2;
	    totalVerts += prim->numVerts;
	}
    }

    /* close ".spf" file */
    fclose(spfFile);

    /* release primitive buffer */
    pfdDelGeom(prim);

    /* release dynamic data */
    free(vertData);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();

    /* use indicated name for top-level pfNode */
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_spf: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", numVerts);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input polygons:     %8ld", inputPolys);

    return node;
}
