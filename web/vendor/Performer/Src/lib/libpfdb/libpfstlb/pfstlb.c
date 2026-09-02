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
 * pfstlb.c: $Revision: 1.32 $ $Date: 2002/11/07 19:03:11 $
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

#ifdef __linux__
#include <asm/byteorder.h>
#endif

typedef struct StlHeader
{
    char    text[80];
    int	    count;
} StlHeader;

/* NOTE: sadly, the compiler pads this out to 52 bytes */
typedef struct StlFacet
{
    float   n[3];
    float   v[3][3];
    char    empty[2];
} StlFacet;

/*
 * pfdLoadFile_stlb -- Load 3D-lithography ".stlb" files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_stlb (char *fileName)
{
    int		 i		= 0;
    int		 numTris	= 0;
    int		 numVerts	= 0;
    FILE	*stlbFile	= NULL;
    pfdGeom	*buffer		= NULL;
    pfNode	*node		= NULL;
    StlHeader	 header;
    StlFacet	 facet;
    pfMaterial  *m      	= NULL;
    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;
    int		 t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open ".stlb" file */
    if ((stlbFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* initialize utility library triangle/geoset builder */
    buffer = pfdNewGeom(3);

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

    /* read ".stlb" header */
    fread(&header, sizeof(StlHeader), 1, stlbFile);

    /* trim trailing whitespace from header string */
    header.text[79] = '\0';
    for (i = 78; i >= 0 && isspace(header.text[i]); i--)
	header.text[i] = '\0';

    /* read ".stlb" facets */
    while (fread(&facet, 50 /*sizeof(StlFacet)*/, 1, stlbFile) == 1)
    {
#ifndef __LITTLE_ENDIAN
	/* reverse byte order of data (intel->mips) */
	for (i = 0; i < 12; i++)
	{
	    float *f = &facet.n[i];
	    unsigned char *b = (unsigned char *)f;
	    unsigned char t0, t1, t2, t3;
	    t0 = b[0]; t1 = b[1]; t2 = b[2]; t3 = b[3];
	    b[0] = t3; b[1] = t2; b[2] = t1; b[3] = t0;
	}
#endif

	/* define facet normal */
	pfCopyVec3(buffer->norms[0], facet.n);
	buffer->nbind = PFGS_PER_PRIM;

	/* define facet vertices */
        pfCopyVec3(buffer->coords[0], facet.v[0]);
        pfCopyVec3(buffer->coords[1], facet.v[1]);
        pfCopyVec3(buffer->coords[2], facet.v[2]);
	buffer->cbind = PFGS_PER_PRIM;
	buffer->numVerts = 3; 

	/* add the polygon to the builder */
	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    buffer->tbind[t] = PFGS_OFF;
	buffer->primtype = PFGS_POLYS;
	pfdAddBldrGeom(buffer, 1);
	numTris  += buffer->numVerts - 2;
	numVerts += buffer->numVerts;
    }

    /* close ".stlb" file */
    fclose(stlbFile);
    pfdDelGeom(buffer);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_stlb: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    File header:        %s", header.text);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", numVerts);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input triangles:    %8ld", numTris);

    return node;
}
