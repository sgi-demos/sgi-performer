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
 * pfpegg.c: $Revision: 1.13 $ $Date: 2002/11/07 23:44:16 $
 */

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include "wintypes.h"

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pf.h>
#include <Performer/pfdu.h>

/*
 * globals for shaun's parser
 */
typedef int PAT[4];
static int nv, np;
static float xmin, xmax, ymin, ymax, zmin, zmax;
static float xc, yc, zc;
static pfVec3 *ver;
static pfVec3 *rgb;
static PAT *pat;
static char *rec;

static void readEgg(FILE *fp);

/*
 * pfdLoadFile_pegg -- Load ".pegg" data files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_pegg (char *fileName)
{
    FILE	*peggFile	= NULL;
    pfNode	*node		= NULL;
    pfdGeom	*prim		= NULL;
    int		 v		= 0;
    int 	 i		= 0;
    int		 numTris	= 0;
    int		 numQuads	= 0;
    int		 t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open ".pegg" file */
    if ((peggFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* allocate primitive buffer */
    prim = pfdNewGeom(4);

    /* disable lighting (that's what radiosity is all about ;-) */
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_OFF);
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);

    /* disable undesired automatic builder actions */
    pfdBldrMode(PFDBLDR_AUTO_NORMALS, PF_OFF);
    pfdBldrMode(PFDBLDR_AUTO_ORIENT,  PF_OFF);

    /* specify control data */
    prim->primtype = PFGS_POLYS;
    prim->cbind = PFGS_PER_VERTEX;
    prim->nbind = PFGS_OFF;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	prim->tbind[t] = PFGS_OFF;

    /* read vertices and patches from ".pegg" file */
    readEgg(peggFile);

    /* generate polygons */
    for (v = 0; v < np; v++)
    {
	/* determine number of vertices in polygon */
	prim->numVerts = (rec[v]==1) ? 4 : 3;

	for (i=0; i<prim->numVerts; i++)
	{
	    /* copy postion value */
	    pfCopyVec3(prim->coords[i], ver[pat[v][i]]);

	    /* copy color. set alpha value to opaque */
	    pfCopyVec3(prim->colors[i], rgb[pat[v][i]]);
	    prim->colors[i][3] = 1.0f;
	}

	/* add polygon to builder */
	pfdAddBldrGeom(prim, 1);

	/* keep running count of primitives processed */
	if (prim->numVerts == 3)
	    ++numTris;
	else
	    ++numQuads;
    }

    /* free dynamic storage */
    pfFree(ver);
    pfFree(rgb);
    pfFree(pat);
    pfFree(rec);

    /* release primitive buffer */
    pfdDelGeom(prim);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();

    /* release storage allocated by the builder */
    pfdResetBldrGeometry();

    /* use indicated name for top-level pfNode */
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_pegg: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", nv);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input tris:         %8ld", numTris);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input quads:        %8ld", numQuads);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Total triangles:    %8ld", numTris + 2*numQuads);

    return node;
}

void readEgg(FILE *fp)
{
    int c;
    int i;

    /* count number of vertices */
    while ((c = getc(fp)) != '[');
    getc(fp);
    nv = 0;
    while ((c = getc(fp)) != ']')
    {
	while ((c = getc(fp)) != '\n');
	nv++;
    }

    /* count number of polygons */
    while ((c = getc(fp)) != '[');
	getc(fp);
    np = 0;
    while ((c = getc(fp)) != ']')
    {
	while ((c = getc(fp)) != '\n');
	np++;
    }

    ver = (pfVec3 *)pfMalloc(nv * sizeof(pfVec3), NULL);
    rgb = (pfVec3 *)pfMalloc(nv * sizeof(pfVec3), NULL);
    pat = (PAT    *)pfMalloc(np * sizeof(PAT),    NULL);
    rec = (char   *)pfMalloc(np * sizeof(char),   NULL);
    memset(rec, 1, np * sizeof(char));

    xmin = ymin = zmin =  10000.0;
    xmax = ymax = zmax = -10000.0;

    /* read vertex definitions from file */
    rewind(fp);
    while ((c = getc(fp)) != '[');
    getc(fp);
    for (i = 0; i < nv; i++)
    {
	/* read next vertex */
	getc(fp);
	fscanf(fp, "%f%f%f%f%f%f",
	    &ver[i][0], &ver[i][1], &ver[i][2],
	    &rgb[i][0], &rgb[i][1], &rgb[i][2]);
	getc(fp); getc(fp);

	/* keep track of minimum and maximum coordinate values */
	if (ver[i][0] < xmin)
	    xmin = ver[i][0];
	if (ver[i][0] > xmax)
	    xmax = ver[i][0];

	if (ver[i][1] < ymin)
	    ymin = ver[i][1];
	if (ver[i][1] > ymax)
	    ymax = ver[i][1];

	if (ver[i][2] < zmin)
	    zmin = ver[i][2];
	if (ver[i][2] > zmax)
	    zmax = ver[i][2];

	/* make color values range from 0 to 1 */
	rgb[i][0] = PF_CLAMP(rgb[i][0], 0.0f, 1.0f);
	rgb[i][1] = PF_CLAMP(rgb[i][1], 0.0f, 1.0f);
	rgb[i][2] = PF_CLAMP(rgb[i][2], 0.0f, 1.0f);
    }

    xc = (xmin + xmax) / 2.0;
    yc = (ymin + ymax) / 2.0;
    zc = (zmin + zmax) / 2.0;

    /* read polygon definitions from file */
    while ((c = getc(fp)) != '[');
    getc(fp);
    for (i = 0; i < np; i++)
    {
	getc(fp);
	fscanf(fp, "%d%d%d", &pat[i][0], &pat[i][1], &pat[i][2]);
	if ((c = getc(fp)) == ']')
	{
	    rec[i] = 0;
	    getc(fp);
	}
	else
	{
	    ungetc(c, fp);
	    fscanf(fp, "%d", &pat[i][3]);
	    getc(fp); getc(fp);
	}
    }

    fclose(fp);

    /* center object at origin */
    for (i = 0; i < nv; i++)
    {
	ver[i][0] -= xc;
	ver[i][1] -= yc;
	ver[i][2] -= zc;
    }
}
