/* 
 * Copyright 1995, Silicon Graphics, Inc.
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
 * pfdGSet.c
 *
 * $Revision: 1.16 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * Construct implicit surfaces like spheres, cubes, cones, cylinders
 *
 */

#include <stdio.h>
#include <malloc.h>
#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#endif
#include <Performer/pfdu.h>

#define PFD_TEXTURED		0x100

static pfVec3	cubeCoords[6][4] = {
    {{-1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f}, 	/* +Z */
    {  1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}}, 	/* +Z */
    {{-1.0f, -1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f}, 	/* -Z */
    {  1.0f,  1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f}}, 	/* -Z */
    {{ 1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f, -1.0f}, 	/* +X */
    {  1.0f,  1.0f, -1.0f}, { 1.0f,  1.0f,  1.0f}}, 	/* +X */
    {{-1.0f, -1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}, 	/* -X */
    { -1.0f,  1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f}}, 	/* -X */
    {{-1.0f,  1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f}, 	/* +Y */
    {  1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f}}, 	/* +Y */
    {{-1.0f, -1.0f,  1.0f}, {-1.0f, -1.0f, -1.0f}, 	/* -Y */
    {  1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f,  1.0f}}}; 	/* -Y */

static pfVec3	cubeNorms[6] = {
    { 0.0f,  0.0f,  1.0f}, 	/* +Z */
    { 0.0f,  0.0f, -1.0f}, 	/* -Z */
    { 1.0f,  0.0f,  0.0f}, 	/* +X */
    {-1.0f,  0.0f,  0.0f}, 	/* -X */
    { 0.0f,  1.0f,  0.0f}, 	/* +Y */
    { 0.0f, -1.0f,  0.0f}}; 	/* -Y */


PFDUDLLEXPORT pfGeoSet *pfdNewCube(void *arena); 
PFDUDLLEXPORT pfGeoSet *pfdNewSphere(int ntris, void *arena); 
PFDUDLLEXPORT pfGeoSet *pfdNewCylinder(int ntris, void *arena); 
PFDUDLLEXPORT pfGeoSet *pfdNewPipe(float botRad, float topRad, int ntris, void *arena); 
PFDUDLLEXPORT pfGeoSet *pfdNewCone(int ntris, void *arena);
PFDUDLLEXPORT pfGeoSet *pfdNewCircle(int ntris, void *arena);
PFDUDLLEXPORT pfGeoSet *pfdNewRing(int ntris, void *arena);
PFDUDLLEXPORT pfGeoSet *pfdNewPyramid(void *arena);
PFDUDLLEXPORT pfGeoSet *pfdNewArrow(int ntris, void *arena);
PFDUDLLEXPORT pfGeoSet *pfdNewDoubleArrow(int ntris, void *arena);
PFDUDLLEXPORT pfGeoSet * pfdNewTorus(float rMajor, float rMinor, int nDivMajor, int nDivMinor, void *arena);

    /*-----------------------------------------------------------------*/

static void
xformCoords(pfVec3 *coords, pfMatrix mat, int n)
{
    int	i;

    for (i=0; i<n; i++)
	pfXformPt3(coords[i], coords[i], mat);
}

static void
xformNorms(pfVec3 *norms, pfMatrix mat, int n)
{
    pfMatrix	invTranspose;
    int		i;

    pfInvertFullMat(invTranspose, mat);
    pfTransposeMat(invTranspose, invTranspose); 
    
    for (i=0; i<n; i++)
    {
	pfXformVec3(norms[i], norms[i], invTranspose);
	pfNormalizeVec3(norms[i]);
    }
}


/* 
 * Transform gset's coordinates and normals by 'mat'
 * 
 * Only implemented for non-indexed, normal-strided geosets.
*/
PFDUDLLEXPORT void
pfdXformGSet(pfGeoSet *gset, pfMatrix mat)
{
    pfVec3	*coords, *norms;
    ushort	*vindex, *nindex;
    int		vstride, nstride;

    pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&coords, &vindex);
    pfGetGSetAttrLists(gset, PFGS_NORMAL3, (void**)&norms, &nindex);

    if (vindex || nindex)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, "pfdXformGSet() "
		 "Flatten not yet implemented for indexed pfGeoSets");
	return;
    }

    if (coords)
	xformCoords(coords, mat, pfGetSize(coords) / sizeof(pfVec3));

    if (norms)
	xformNorms(norms, mat, pfGetSize(norms) / sizeof(pfVec3));
}

static void
getBoxSize(int *nstrips, int *nc, int *nn, int nbind)
{
    *nstrips = 6;
    *nc = 24;
    if (nbind == PFGS_PER_VERTEX)
	*nn = 24;
    else if (nbind == PFGS_PER_PRIM)
	*nn = 6;
    else
	*nn = 0;
}

static void
makeBox(pfVec3 **coords, pfVec3 **norms, int **lens, 
	float xmin, float ymin, float zmin, 
	float xmax, float ymax, float zmax, int nbind) 
{
    int		i, j, k;
    pfMatrix	mat;
    static int  stripOrder[] = {0, 1, 3, 2};

    for (k=0, i=0; i<6; i++)
    {
	if (nbind == PFGS_PER_PRIM)
	{
	    pfCopyVec3((*norms)[0], cubeNorms[i]);
	    *norms += 1;
	}
	for (j=0; j<4; j++, k++)
	{
	    pfCopyVec3((*coords)[k], cubeCoords[i][stripOrder[j]]);
	    if (nbind == PFGS_PER_VERTEX)
	    {
		pfCopyVec3((*norms)[0], cubeNorms[i]);
		*norms += 1;
	    }
	}
	if (lens)
	{
	    (*lens)[0] = 4;
	    *lens += 1;
	}
    }
    pfMakeTransMat(mat, 1.0f, 1.0f, 1.0f);
    pfPostScaleMat(mat, mat, 
		   (xmax - xmin)/2, (ymax - ymin)/2, (zmax - zmin)/2);
    pfPostTransMat(mat, mat, xmin, ymin, zmin);

    for (i=0; i<24; i++)
	pfXformPt3((*coords)[i], (*coords)[i], mat);

    *coords += 24;
}

/* 
 * Make cube centered at the origin with sides of length 1
 */
PFDUDLLEXPORT pfGeoSet*
pfdNewCube(void *arena) 
{
    pfGeoSet  	*g;
    pfVec3	*coords, *norms;
    pfVec2	*texCoords;
    int		*lens;
    int 	ns, nc, nn;

    g = pfNewGSet(arena);

    getBoxSize(&ns, &nc, &nn, PFGS_PER_PRIM);

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * nc, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * nn, arena);
    lens = (int*) pfMalloc(sizeof(int) * ns, arena);
    
    pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(g, PFGS_NORMAL3, PFGS_PER_PRIM, norms, NULL);
    pfGSetPrimType(g, PFGS_TRISTRIPS);

    pfGSetNumPrims(g, ns);
    pfGSetPrimLengths(g, lens);

    makeBox(&coords, &norms, &lens, -.5f, -.5f, -.5f, .5f, .5f, .5f,
	    PFGS_PER_PRIM);

    return g;
}

/* 
 * Start with a cube. Chop faces into uniform 2D grid then bloat grid points
 * to surface of unit sphere. A nonuniform grid would generate a more 
 * uniform sphere. 
 */
PFDUDLLEXPORT pfGeoSet*
pfdNewSphere(int ntris, void *arena)
{
    pfGeoSet  	*g;
    pfVec3	*coords, *norms;
    pfVec2	*texCoords;
    int		*lens, nrows, i, j, k, l, ncoords;
    float	x, y, z, d;

    nrows = pfSqrt(ntris / 6);
    nrows = PF_MAX2(nrows, 1);

    ncoords = 2*(3 * nrows + 1) * nrows * 2;

    g = pfNewGSet(arena);

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * ncoords, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * ncoords, arena);
    lens = (int*) pfMalloc(sizeof(int) * 2 * nrows, arena);

    pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(g, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
    pfGSetPrimType(g, PFGS_TRISTRIPS);
    pfGSetNumPrims(g, 2 * nrows);
    pfGSetPrimLengths(g, lens);

    k = l = 0;
    d = 2.0f / (float)nrows;

    x = -1.0f;
    for (i=0; i<nrows; i++)
    {
	y = -1.0f;
	z = -1.0f;
	lens[l++] = 2*(3 * nrows + 1);
	for (j=0; j<nrows; j++)
	{
	    pfSetVec3(coords[k], x, y, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    pfSetVec3(coords[k], x+d, y, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    z += d;
	}
	for (j=0; j<nrows; j++)
	{
	    pfSetVec3(coords[k], x, y, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    pfSetVec3(coords[k], x+d, y, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    y += d;
	}
	for (j=0; j<=nrows; j++)
	{
	    pfSetVec3(coords[k], x, y, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    pfSetVec3(coords[k], x+d, y, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    z -= d;
	}
	x += d;
    }
    y = -1.0f;
    for (i=0; i<nrows; i++)
    {
	x = -1.0f;
	z =  1.0f;
	lens[l++] = 2*(3 * nrows + 1);
	for (j=0; j<nrows; j++)
	{
	    pfSetVec3(coords[k], x, y, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    pfSetVec3(coords[k], x, y+d, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    z -= d;
	}
	for (j=0; j<nrows; j++)
	{
	    pfSetVec3(coords[k], x, y, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    pfSetVec3(coords[k], x, y+d, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    x += d;
	}
	for (j=0; j<=nrows; j++)
	{
	    pfSetVec3(coords[k], x, y, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    pfSetVec3(coords[k], x, y+d, z);
	    pfNormalizeVec3(coords[k]);
	    pfCopyVec3(norms[k], coords[k]);
	    k++;
	    z += d;
	}
	y += d;
    }
    return g;
}


#define DOWN	-PF_Z
#define UP	PF_Z

static void
getCircleSize(int nsides, int *nstrips, int *nc, int *nn)
{
    *nstrips = 1;
    *nc = nsides;

    /* !!!!!!!!!! */
    if (nn)
	*nn = nsides;
}


static int
makeCircle(pfVec3 **coords, pfVec3 **norms, int **lens, 
	   int nsides, float radius, int axis, float dist)
{
    int		i, j, k, nv, odd;
    float	theta, dt, x, y, z;

    if (norms)
    {
	for (i=0; i<nsides; i++)
	    pfSetVec3((*norms)[i], 0.0f, 0.0f, 
		      (axis > 0) ? 1.0f : -1.0f);

	*norms += nsides;
    }

    dt = 360.0f / nsides;
    k = 0;

    if (axis > 0)
	dt = -dt;

    j = nsides - 1;
    i = 0;
    pfSinCos(dt * i, &x, &y);
    pfSetVec3((*coords)[k], x * radius, y * radius, dist);
    k++;
    i++;
    pfSinCos(dt * i, &x, &y);
    pfSetVec3((*coords)[k], x * radius, y * radius, dist);
    k++;
    i++;
    pfSinCos(dt * j, &x, &y);
    pfSetVec3((*coords)[k], x * radius, y * radius, dist);
    k++;
    j--;

    odd = 0;
    while (i <= j)
    {
	float	x, y;

	if (odd)
	{
	    pfSinCos(dt * j, &x, &y);
	    pfSetVec3((*coords)[k], x * radius, y * radius, dist);
	    k++;
	    j--;
	}
	else
	{
	    pfSinCos(dt * i, &x, &y);
	    pfSetVec3((*coords)[k], x * radius, y * radius, dist);
	    k++;
	    i++;
	}
	odd = !odd;
    }
    *coords += k;

    if (lens)
    {
	(*lens)[0] = nsides;
	*lens += 1;
    }

    return k;
}


static void
getRingSize(int nsides, int *nstrips, int *nc)
{
    *nstrips = 1;
    *nc = nsides;
}


static int
makeRing(pfVec3 **coords, int **lens, 
	 int nsides, float radius, int axis, float dist)
{
    int		i, j, k, nv, odd;
    float	theta, dt, x, y, z;

    dt = 360.0f / (nsides-1);
    k = 0;

    for (i=0; i<nsides; i++)
    {
	pfSinCos(dt*i, &x, &y);
	pfSetVec3((*coords)[i], x * radius, y * radius, dist);
    }
    *coords += nsides;
    if (lens)
    {
	(*lens)[0] = nsides;
	*lens++;
    }

    return k;
}


/*
 * Make wireframe circle of radius 1 in the Z=0 plane
 */
PFDUDLLEXPORT pfGeoSet*
pfdNewRing(int nlines, void *arena) 
{
    pfGeoSet  	*g;
    pfVec3	*coords;
    pfVec2	*texCoords;
    int		*lens;

    g = pfNewGSet(arena);

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * nlines, arena);
    lens = (int*) pfMalloc(sizeof(int), arena);
    
    pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetPrimType(g, PFGS_LINESTRIPS);
    pfGSetNumPrims(g, 1);
    pfGSetPrimLengths(g, lens);

    makeRing(&coords, &lens, nlines, 1.0f, PF_Z, 0.0f);

    return g;
}

static void
getPipeSize(int nsides, int *nstrips, int *nc, int *nn)
{
    *nstrips = 1;
    *nc = 2 * (nsides + 1);
    *nn = 2 * (nsides + 1);
}

/*
 * Make cylinder without end caps. zmin must be < zmax
*/
static int
makePipe(pfVec3 **coords, pfVec3 **norms, int **lens, 
	 int nsides, float bottomRadius, float topRadius, 
	 float zmin, float zmax)
{
    int		i, k;
    float	theta, dt, x, y, z, slope, znorm;

    slope = (bottomRadius - topRadius) / (zmax - zmin);

    if (slope != 0.0f)
	znorm = 1.0f / slope;
    else
	znorm = 0.0f;

    dt = 360.0f / nsides;
    theta = 0.0f;
    k = 0;

    /* Make the pipe */
    for (i=0; i<=nsides; i++)
    {
	pfSinCos(theta, &x, &y);

	pfSetVec3((*coords)[k], x * bottomRadius, y * bottomRadius, zmin);
	pfSetVec3((*norms)[k], x, y, znorm);
	if (znorm)
	    pfNormalizeVec3((*norms)[k]);
	k++;
	pfSetVec3((*coords)[k], x * topRadius, y * topRadius, zmax);
	pfCopyVec3((*norms)[k], (*norms)[k-1]); 
	k++;

	theta += dt;
    }
    *coords += k;
    *norms += k;

    if (lens)
    {
	(*lens)[0] = k;
	*lens += 1;
    }

    return k;
}

/*
 * Make cylinder of radius 1 and length 2 centered along the Z axis
 */
PFDUDLLEXPORT pfGeoSet*
pfdNewCylinder(int ntris, void *arena) 
{
    pfGeoSet  	*g;
    pfVec3	*coords, *norms;
    pfVec2	*texCoords;
    int		*lens, nsides, i, j, k, s, odd, nv;
    float	theta, dt, x, y, z;
    int		tns, tnc, tnn, ns, nc, nn;

    nsides = ntris / 4;
    nsides = PF_MAX2(nsides, 3);

    g = pfNewGSet(arena);

    getPipeSize(nsides, &tns, &tnc, &tnn);
    getCircleSize(nsides, &ns, &nc, &nn);

    tns += 2 * ns;
    tnc += 2 * nc;
    tnn += 2 * nn;

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnc, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnn, arena);
    lens = (int*) pfMalloc(sizeof(int) * tns, arena);
    
    pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(g, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
    pfGSetPrimType(g, PFGS_TRISTRIPS);
    pfGSetNumPrims(g, tns);
    pfGSetPrimLengths(g, lens);

    makePipe(&coords, &norms, &lens, nsides, 1.0f, 1.0f, -1.0f, 1.0f);
    makeCircle(&coords, &norms, &lens, nsides, 1.0f, DOWN, -1.0f);
    makeCircle(&coords, &norms, &lens, nsides, 1.0f, UP, 1.0f);

    return g;
}

PFDUDLLEXPORT pfGeoSet*
pfdNewPipe(float botRad, float topRad, int ntris, void *arena) 
{
    pfGeoSet  	*g;
    pfVec3	*coords, *norms;
    pfVec2	*texCoords;
    int		*lens, nsides, i, j, k, s, odd, nv;
    float	theta, dt, x, y, z;
    int		tns, tnc, tnn, ns, nc, nn;

    nsides = ntris / 2;
    nsides = PF_MAX2(nsides, 3);

    g = pfNewGSet(arena);

    getPipeSize(nsides, &tns, &tnc, &tnn);

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnc, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnn, arena);
    lens = (int*) pfMalloc(sizeof(int) * tns, arena);
    
    pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(g, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
    pfGSetPrimType(g, PFGS_TRISTRIPS);
    pfGSetNumPrims(g, tns);
    pfGSetPrimLengths(g, lens);

    makePipe(&coords, &norms, &lens, nsides, botRad, topRad, -1.0f, 1.0f);

    return g;
}

void
getConeTopSize(int nsides, int *nstrips, int *nc, int *nn)
{
    getPipeSize(nsides, nstrips, nc, nn);
}

static int
makeConeTop(pfVec3 **coords, pfVec3 **norms, int **lens, 
	    int nsides, float radius, float height, float base)
{

    if (height < 0.0f)
	makePipe(coords, norms, lens, nsides, 0.0f, radius, base+height, base);
    else
	makePipe(coords, norms, lens, nsides, radius, 0.0f, base, base+height);

}

void
getConeSize(int nsides, int *nstrips, int *nc, int *nn)
{
    int	ns, nnc, nnn;

    getConeTopSize(nsides, nstrips, nc, nn);
    getCircleSize(nsides, &ns, &nnc, &nnn);

    *nstrips += ns;
    *nc += nnc;
    *nn += nnn;
}

static void
makeCone(pfVec3 **coords, pfVec3 **norms, int **lens, 
	 int nsides, float radius, float height, float base)
{
    makeConeTop(coords, norms, lens, nsides, radius, height, base);
    if (height < 0.0f)
	makeCircle(coords, norms, lens, nsides, radius, UP, base);
    else
	makeCircle(coords, norms, lens, nsides, radius, DOWN, base);
}

/*
 * Make cone of radius 1 and height 1 centered along the Z axis with
 * base at Z = 0.
 */
PFDUDLLEXPORT pfGeoSet*
pfdNewCone(int ntris, void *arena) 
{
    pfGeoSet  	*g;
    pfVec3	*coords, *norms;
    pfVec2	*texCoords;
    int		nsides, ncoords, i, j, k, *lens, odd;
    float	dt, theta, x, y;
    int		tns, tnc, tnn, ns, nc, nn;

    nsides = ntris / 2;
    ncoords = nsides * 4;

    getConeTopSize(nsides, &tns, &tnc, &tnn);
    getCircleSize(nsides, &ns, &nc, &nn);
    tns += ns;
    tnc += nc;
    tnn += nn;

    g = pfNewGSet(arena);

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnc, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnn, arena);
    lens = (int*) pfMalloc(sizeof(int) * tns, arena);
    
    pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(g, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
    pfGSetPrimType(g, PFGS_TRISTRIPS);
    pfGSetNumPrims(g, tns);
    pfGSetPrimLengths(g, lens);

    makeConeTop(&coords, &norms, &lens, nsides, 1.0f, 1.0f, 0.0f);
    makeCircle(&coords, &norms, &lens, nsides, 1.0f, DOWN, 0.0f);

    return g;
}


/*
 * Make circle of radius 1 facing +Z
 */
PFDUDLLEXPORT pfGeoSet*
pfdNewCircle(int ntris, void *arena) 
{
    pfGeoSet  	*g;
    pfVec3	*coords, *norms;
    pfVec2	*texCoords;
    int		*lens, i, j, k, odd, nsides = ntris+2;
    float	dt, x, y;

    g = pfNewGSet(arena);

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * nsides, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3), arena);
    lens = (int*) pfMalloc(sizeof(int), arena);
    
    pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(g, PFGS_NORMAL3, PFGS_OVERALL, norms, NULL);
    pfGSetPrimType(g, PFGS_TRISTRIPS);
    pfGSetNumPrims(g, 1);
    pfGSetPrimLengths(g, lens);

    pfSetVec3(norms[0], 0.0f, 0.0f, 1.0f);

    makeCircle(&coords, NULL, &lens, nsides, 1.0f, UP, 0.0f);

    return g;
}

static void
getPyramidSize(int *nstrips, int *nc, int *nn, int nbind)
{
    *nstrips = 5;
    *nc = 16;

    if (nbind == PFGS_PER_VERTEX)
	*nn = *nc;
    else if (nbind == PFGS_PER_PRIM)
	*nn = 5;
    else
	*nn = 0;
}

static void
makePyramid(pfVec3 **coords, pfVec3 **norms, int **lens, 
	    float width, float height, float base, int nbind)
{
    int		i, j, k;
    float	w = width / 2.0f, out, up;
    pfVec3	norm;

    static pfVec3	pyrCoords[] = {
    {-1.0f, -1.0f, 0.0f}, 
    {-1.0f,  1.0f, 0.0f}, 
    { 1.0f,  1.0f, 0.0f}, 
    { 1.0f, -1.0f, 0.0f}, 
    { 0.0f,  0.0f, 1.0f}};	/* top */

    static pfVec3	pyrNorms[] = {
    { 0.0f,  0.0f,-1.0f},	/* base */
    { 0.0f, -1.0f, 1.0f}, 	/* sides */
    { 1.0f,  0.0f, 1.0f}, 
    { 0.0f,  1.0f, 1.0f}, 
    {-1.0f,  0.0f, 1.0f}};

    static int		pyrIndices[] = {
	0, 1, 3, 2, -1,	/* base */
	0, 3, 4, -1, 	/* sides */
	3, 2, 4, -1,
	2, 1, 4, -1,
	1, 0, 4, -1};

    pfSetVec3(norm, PF_ABS(height), 0.0f, w);
    pfNormalizeVec3(norm);
    out = norm[0];
    up = norm[2];

    for (k=0, j=0, i=0; i<5; i++, j++)
    {
	int		n;

	n = 0;
	while (pyrIndices[j] >= 0)
	{
	    pfCopyVec3((*coords)[k], pyrCoords[pyrIndices[j]]);
	    (*coords)[k][2] = (*coords)[k][2] * height + base;
	    (*coords)[k][0] *= w;
	    (*coords)[k][1] *= w;

	    if (nbind == PFGS_PER_VERTEX)
	    {
		pfCopyVec3((*norms)[0], pyrNorms[i]);
		(*norms)[0][0] *= out;
		(*norms)[0][1] *= out;
		if (height < 0.0f)
		    (*norms)[0][2] *= -up;
		else
		    (*norms)[0][2] *= up;
		pfNormalizeVec3((*norms)[0]);
		*norms += 1;
	    }
	    j++;
	    k++;
	    n++;

	}
	if (height < 0.0f)
	{
	    pfVec3	tmp;

	    pfCopyVec3(tmp, (*coords)[k-n+1]);
	    pfCopyVec3((*coords)[k-n+1], (*coords)[k-n+2]);
	    pfCopyVec3((*coords)[k-n+2], tmp);
	}
	if (nbind == PFGS_PER_PRIM)
	{
	    pfCopyVec3((*norms)[0], pyrNorms[i]);
	    (*norms)[0][0] *= out;
	    (*norms)[0][1] *= out;
	    if (height < 0.0f)
		(*norms)[0][2] *= -up;
	    else
		(*norms)[0][2] *= up;
	    pfNormalizeVec3((*norms)[0]);
	    *norms += 1;
	}
	if (lens)
	{
	    (*lens)[0] = n;
	    *lens += 1;
	}
    }
    *coords += k;

}

/*
 * Make pyramid with unit square base at Z=0 and height 1.
 */
PFDUDLLEXPORT pfGeoSet*
pfdNewPyramid(void *arena) 
{
    pfGeoSet  	*g;
    pfVec3	*coords, *norms;
    pfVec2	*texCoords;
    int		*lens, nstrips, nc, nn;

    g = pfNewGSet(arena);

    getPyramidSize(&nstrips, &nc, &nn, PFGS_PER_PRIM);

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * nc, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * nn, arena);
    lens = (int*) pfMalloc(sizeof(int) * nstrips, arena);
    
    pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(g, PFGS_NORMAL3, PFGS_PER_PRIM, norms, NULL);
    pfGSetPrimType(g, PFGS_TRISTRIPS);
    pfGSetNumPrims(g, nstrips);
    pfGSetPrimLengths(g, lens);

    makePyramid(&coords, &norms, &lens, 1.0f, 1.0f, 0.0f, PFGS_PER_PRIM);

    return g;
}

#define ARROW_LEN	.25f
#define ARROW_WIDTH	.125f 
#define SHAFT_WIDTH	.0625f


/*
 * Make arrow with base at Z=0, total height = 1, shaft width of 
 * SHAFT_WIDTH and arrowhead length ARROW_LEN and width ARROW_WIDTH
 */
PFDUDLLEXPORT pfGeoSet*
pfdNewArrow(int ntris, void *arena) 
{
    pfGeoSet  	*g;
    pfVec3	*coords, *norms;
    pfVec2	*texCoords;
    int		*lens, nsides;
    int		tns, tnc, tnn, ns, nc, nn;

    /* side ratios base:pipe:cone = 1 : 1 : 2 (cone is twice as fine) */
    /* tri ratios  base:pipe:cone = 1 : 2 : 2 */
    nsides = ntris / 7;	/* nsides is sides in pipe/base, cone is 2x */
    
    nsides = PF_MAX2(nsides, 4);

    g = pfNewGSet(arena);

    /* '4' is coarsest arrow which has pyramid for end and cube for shaft */
    if (nsides == 4 && ntris < 28)
    {
	getBoxSize(&tns, &tnc, &tnn, PFGS_PER_PRIM);
	getPyramidSize(&ns, &nc, &nn, PFGS_PER_PRIM);
	tns += ns;
	tnc += nc;
	tnn += nn;

	coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnc, arena);
	norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnn, arena);
	lens = (int*) pfMalloc(sizeof(int) * tns, arena);
	
	pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	pfGSetAttr(g, PFGS_NORMAL3, PFGS_PER_PRIM, norms, NULL);
	pfGSetPrimType(g, PFGS_TRISTRIPS);
	pfGSetNumPrims(g, tns);
	pfGSetPrimLengths(g, lens);

	makeBox(&coords, &norms, &lens, 
		-SHAFT_WIDTH*.5f, -SHAFT_WIDTH*.5f, 0.0f, SHAFT_WIDTH*.5f, SHAFT_WIDTH*.5f, 
		1.0f - ARROW_LEN, PFGS_PER_PRIM);
	makePyramid(&coords, &norms, &lens, ARROW_WIDTH, ARROW_LEN, 1.0f-ARROW_LEN,
		    PFGS_PER_PRIM);

	return g;
    }

    getPipeSize(nsides, &tns, &tnc, &tnn);
    getCircleSize(nsides, &ns, &nc, &nn);
    tns += ns;
    tnc += nc;
    tnn += nc;	/* Need PER_VERTEX here */
    getConeSize(2*nsides, &ns, &nc, &nn);
    tns += ns;
    tnc += nc;
    tnn += nn;

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnc, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnn, arena);
    lens = (int*) pfMalloc(sizeof(int) * tns, arena);
    
    pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(g, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
    pfGSetPrimType(g, PFGS_TRISTRIPS);
    pfGSetNumPrims(g, tns);
    pfGSetPrimLengths(g, lens);

    makePipe(&coords, &norms, &lens, nsides, SHAFT_WIDTH*.5f, SHAFT_WIDTH*.5f, 
	     0.0f, 1.0f-ARROW_LEN);
    makeCircle(&coords, &norms, &lens, nsides, SHAFT_WIDTH*.5f, DOWN, 0.0f);
    makeCone(&coords, &norms, &lens, 2*nsides, ARROW_WIDTH, 
	     ARROW_LEN, 1.0f-ARROW_LEN);

    return g;
}

/* Make a torus with specified axes and number of slices */
PFDUDLLEXPORT
pfGeoSet * pfdNewTorus(float rMajor, 
		       float rMinor, 
		       int nDivMajor, 
		       int nDivMinor,
		       void *arena) {
  pfGeoSet *gs;
  pfVec3 *coords;
  pfVec3 *normals;
  pfVec2 *texCoords;
  int *primLengths;
  int divMaj, divMin;

  float x1, y1, z1;
  float x2, y2, z2;
  float i1, j1, k1;
  float i2, j2, k2;
  float s1, t1;
  float s2, t2;

  float theta;
  float thetaMin = 0.0;
  float thetaMax = 360.0;
  float dTheta = (thetaMax - thetaMin) / (float) nDivMajor;
  float sinTheta, cosTheta;

  float omega;
  float omegaMin = 0.0;
  float omegaMax = 360.0;
  float dOmega = (omegaMax - omegaMin) / (float) nDivMinor;
  float sinOmega, cosOmega;

  int numStrips = nDivMajor;
  int numVertsPerStrip = 2 * (nDivMinor + 1);

  gs = pfNewGSet(arena);
  coords = (pfVec3*) pfMalloc (sizeof(pfVec3) * 
			       numStrips * numVertsPerStrip,
			       arena);
  normals = (pfVec3*) pfMalloc (sizeof(pfVec3) * 
				numStrips * numVertsPerStrip,
				arena);
  texCoords = (pfVec2*) pfMalloc (sizeof(pfVec3) * 
				  numStrips * numVertsPerStrip,
				  arena);
  primLengths = (int*) pfMalloc (sizeof (int) * numStrips, 
				 arena);

  pfGSetPrimType (gs, PFGS_TRISTRIPS);
  pfGSetNumPrims (gs, numStrips);
  pfGSetPrimLengths(gs, primLengths);
  pfGSetAttr(gs, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
  pfGSetAttr(gs, PFGS_NORMAL3, PFGS_PER_VERTEX, normals, NULL);
  pfGSetAttr(gs, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texCoords, NULL);
  theta = thetaMin;
  for (divMaj = 0; divMaj < nDivMajor; divMaj++) {
    omega = omegaMin;
    primLengths [divMaj] = numVertsPerStrip;
    for (divMin = 0; divMin <= nDivMinor; divMin++) {
      /* compute all params for first vertex */
      pfSinCos (theta, &sinTheta, &cosTheta);	    
      pfSinCos (omega, &sinOmega, &cosOmega);
	    
      x1 = cosTheta * (rMajor - rMinor * cosOmega);
      y1 = rMinor * sinOmega;
      z1 = sinTheta * (rMajor - rMinor * cosOmega);

      i1 = x1 - cosTheta * rMajor;
      j1 = y1;
      k1 = z1 - sinTheta * rMajor;

      s1 = (theta - thetaMin) / (thetaMax - thetaMin);
      t1 = (omega - omegaMin) / (omegaMax - omegaMin);

      /* compute all params for second vertex.  it's
       one step away in the theta direction. */
      pfSinCos (theta + dTheta, &sinTheta, &cosTheta);	    
	    
      x2 = cosTheta * (rMajor - rMinor * cosOmega);
      y2 = y1;
      z2 = sinTheta * (rMajor - rMinor * cosOmega);

      i2 = x2 - cosTheta * rMajor;
      j2 = y2;
      k2 = z2 - sinTheta * rMajor;

      s2 = (theta + dTheta - thetaMin) / (thetaMax - thetaMin);
      t2 = t1;

      pfSetVec3(coords[divMaj * numVertsPerStrip + 2 * divMin + 0], x1, y1, z1);
      pfSetVec3(coords[divMaj * numVertsPerStrip + 2 * divMin + 1], x2, y2, z2);

      pfSetVec3(normals[divMaj * numVertsPerStrip + 2 * divMin + 0], i1, j1, k1);
      pfSetVec3(normals[divMaj * numVertsPerStrip + 2 * divMin + 1], i2, j2, k2);

      pfSetVec2(texCoords[divMaj * numVertsPerStrip + 2 * divMin + 0], s1, t1);
      pfSetVec2(texCoords[divMaj * numVertsPerStrip + 2 * divMin + 1], s2, t2);

      omega += dOmega;
    }
    theta += dTheta;
  }
    
  return (gs);
}


/*
 * Make double-headed arrow ranging from Z=-1 to Z=1 
 */
PFDUDLLEXPORT pfGeoSet*
pfdNewDoubleArrow(int ntris, void *arena) 
{
    pfGeoSet  	*g;
    pfVec3	*coords, *norms;
    pfVec2	*texCoords;
    int		*lens, nsides;
    int		tns, tnc, tnn, ns, nc, nn;

    /* side ratios cone:pipe:cone = 2 : 1 : 2 (cone is twice as fine) */
    /* tri ratios  cone:pipe:cone = 2 : 2 : 2 */
    nsides = ntris / 10; /* nsides is sides in pipe, cone is 2x */
    
    nsides = PF_MAX2(nsides, 4);

    g = pfNewGSet(arena);

    /* '4' is coarsest arrow which has pyramid for end and cube for shaft */
    if (nsides == 4 && ntris < 40)
    {
	getBoxSize(&tns, &tnc, &tnn, PFGS_PER_PRIM);
	getPyramidSize(&ns, &nc, &nn, PFGS_PER_PRIM);
	tns += 2*ns;
	tnc += 2*nc;
	tnn += 2*nn;

	coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnc, arena);
	norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnn, arena);
	lens = (int*) pfMalloc(sizeof(int) * tns, arena);
	
	pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	pfGSetAttr(g, PFGS_NORMAL3, PFGS_PER_PRIM, norms, NULL);
	pfGSetPrimType(g, PFGS_TRISTRIPS);
	pfGSetNumPrims(g, tns);
	pfGSetPrimLengths(g, lens);

	makeBox(&coords, &norms, &lens, 
		-SHAFT_WIDTH*.5f, -SHAFT_WIDTH*.5f, -1.0f+ARROW_LEN, 
		SHAFT_WIDTH*.5f, SHAFT_WIDTH*.5f, 1.0f-ARROW_LEN, PFGS_PER_PRIM);

	makePyramid(&coords, &norms, &lens, ARROW_WIDTH, 
		    ARROW_LEN, 1.0f-ARROW_LEN, PFGS_PER_PRIM);
	makePyramid(&coords, &norms, &lens, ARROW_WIDTH, 
		    -ARROW_LEN, -(1.0f-ARROW_LEN), PFGS_PER_PRIM);
	return g;
    }

    getPipeSize(nsides, &tns, &tnc, &tnn);
    getConeSize(2*nsides, &ns, &nc, &nn);
    tns += 2*ns;
    tnc += 2*nc;
    tnn += 2*nn;

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnc, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * tnn, arena);
    lens = (int*) pfMalloc(sizeof(int) * tns, arena);
    
    pfGSetAttr(g, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(g, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
    pfGSetPrimType(g, PFGS_TRISTRIPS);
    pfGSetNumPrims(g, tns);
    pfGSetPrimLengths(g, lens);

    makePipe(&coords, &norms, &lens, nsides, SHAFT_WIDTH*.5f, SHAFT_WIDTH*.5f, 
	     -1.0f+ARROW_LEN, 1.0f-ARROW_LEN);
    makeCone(&coords, &norms, &lens, 2*nsides, ARROW_WIDTH, 
	     ARROW_LEN, 1.0f-ARROW_LEN);
    makeCone(&coords, &norms, &lens, 2*nsides, ARROW_WIDTH, 
	     -ARROW_LEN, -(1.0f-ARROW_LEN));

    return g;
}

/*
 * Convenience routine for setting global gset color
 * 
 * Only works if color binding is PFGS_OVERALL
 */
PFDUDLLEXPORT void
pfdGSetColor(pfGeoSet *gset, float r, float g, float b, float a)
{
    ushort	*iclrs;
    pfVec3	*clrs;

    if (!gset)
	return;

    if (pfGetGSetAttrBind(gset, PFGS_COLOR4) != PFGS_OVERALL)
	return;

    pfGetGSetAttrLists(gset, PFGS_COLOR4, (void**)&clrs, &iclrs);

    if (!clrs)
	return;

    if (iclrs)
	pfSetVec4(clrs[iclrs[0]], r, g, b, a);
    else
	pfSetVec4(clrs[0], r, g, b, a);
}


        /*-----------------------------------------------------------*/

#ifdef STRIP_BOX
static void
getBoxSize(int *nstrips, int *nc, int *nn)
{
    *nstrips = 2;
    *nc = 16;
    *nn = 12;
}

static void
makeBox(pfVec3 **coords, pfVec3 **norms, int **lens, 
	float xmin, float ymin, float zmin, 
	float xmax, float ymax, float zmax, int nbind) 
{
    pfSetVec3((*coords)[0], xmin, ymin, zmin);
    pfSetVec3((*coords)[1], xmax, ymin, zmin);
    pfSetVec3( (*norms)[0],  0.0f, -1.0f,  0.0f);
    pfSetVec3((*coords)[2], xmin, ymin, zmax);
    pfSetVec3( (*norms)[1],  0.0f, -1.0f,  0.0f);
    pfSetVec3((*coords)[3], xmax, ymin, zmax);
    pfSetVec3( (*norms)[2],  0.0f,  0.0f,  1.0f);
    pfSetVec3((*coords)[4], xmin, ymax, zmax);
    pfSetVec3( (*norms)[3],  0.0f,  0.0f,  1.0f);
    pfSetVec3((*coords)[5], xmax, ymax, zmax);
    pfSetVec3( (*norms)[4],  0.0f,  1.0f,  0.0f);
    pfSetVec3((*coords)[6], xmin, ymax, zmin);
    pfSetVec3( (*norms)[5],  0.0f,  1.0f,  0.0f);
    pfSetVec3((*coords)[7], xmax, ymax, zmin);

    pfSetVec3((*coords)[8],  xmin, ymin, zmax);
    pfSetVec3((*coords)[9],  xmin, ymax, zmax);
    pfSetVec3( (*norms)[6],  -1.0f,  0.0f,  0.0f);
    pfSetVec3((*coords)[10], xmin, ymin, zmin);
    pfSetVec3( (*norms)[7],  -1.0f,  0.0f,  0.0f);
    pfSetVec3((*coords)[11], xmin, ymax, zmin);
    pfSetVec3( (*norms)[8],   0.0f,  0.0f, -1.0f);
    pfSetVec3((*coords)[12], xmax, ymin, zmin);
    pfSetVec3( (*norms)[9],   0.0f,  0.0f, -1.0f);
    pfSetVec3((*coords)[13], xmax, ymax, zmin);
    pfSetVec3( (*norms)[10],  1.0f,  0.0f,  0.0f);
    pfSetVec3((*coords)[14], xmax, ymin, zmax);
    pfSetVec3( (*norms)[11],  1.0f,  0.0f,  0.0f);
    pfSetVec3((*coords)[15], xmax, ymax, zmax);

    (*lens)[0] = 8;
    (*lens)[1] = 8;
    (*lens) += 2;

    *coords += 16;
    *norms += 12;
}
#endif
