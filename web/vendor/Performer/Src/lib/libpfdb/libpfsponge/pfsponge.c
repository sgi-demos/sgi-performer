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
 * pfsponge.c: $Revision: 1.19 $ $Date: 2002/11/07 18:06:03 $
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
#include <Performer/pfdu.h>

#define	EAST	0x01
#define	WEST	0x02
#define	NORTH	0x04
#define	SOUTH	0x08
#define	TOP	0x10
#define	BOTTOM	0x20

#define	ALL	(EAST|WEST|NORTH|SOUTH|TOP|BOTTOM)

#define	XMIN	(-256.0f)
#define	XMAX	( 256.0f)
#define	YMIN	(-256.0f)
#define	YMAX	( 256.0f)
#define	ZMIN	(-256.0f)
#define	ZMAX	( 256.0f)

#define	RMIN	0.1f
#define	RMAX	0.7f
#define	GMIN	0.1f
#define	GMAX	0.7f
#define	BMIN	0.1f
#define	BMAX	0.7f

static int first = 0;
static pfGroup *group = NULL;

static void
spongeQuad (
    pfdGeom *buffer,
    pfVec3 a, pfVec3 b, pfVec3 c, pfVec3 d)
{
    int v;
    int t;

    buffer->numVerts = 4; 
    buffer->primtype = PFGS_POLYS;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	buffer->tbind[t] = PFGS_OFF;
    buffer->nbind = PFGS_OFF;
    buffer->cbind = PFGS_PER_VERTEX;

    pfCopyVec3(buffer->coords[0], a);
    pfCopyVec3(buffer->coords[1], b);
    pfCopyVec3(buffer->coords[2], c);
    pfCopyVec3(buffer->coords[3], d);

    for (v = 0; v < 4; v++)
	pfSetVec4(buffer->colors[v],
	    RMIN + (RMAX-RMIN)*(buffer->coords[v][0]-XMIN)/(XMAX-XMIN),
	    GMIN + (GMAX-GMIN)*(buffer->coords[v][1]-YMIN)/(YMAX-YMIN),
	    BMIN + (BMAX-BMIN)*(buffer->coords[v][2]-ZMIN)/(ZMAX-ZMIN),
	    1.0);

    pfdAddBldrGeom(buffer, 1);
}

static int
sponge (
    pfdGeom *buffer,
    int levels, 
    int sides, 
    float xMin, float xMax, 
    float yMin, float yMax, 
    float zMin, float zMax)
{
    long	polygons = 0;
    pfVec3	a, b, c, d;
    
    if (sides == 0)
        return 0;
    
    if (levels < 1)
    {
        if (sides & NORTH)
	{
	    pfSetVec3(a, xMax, yMax, zMin);
	    pfSetVec3(b, xMin, yMax, zMin);
	    pfSetVec3(c, xMin, yMax, zMax);
	    pfSetVec3(d, xMax, yMax, zMax);
	    spongeQuad(buffer, a, b, c, d);
	    polygons += 2;
    	}
        if (sides & SOUTH)
	{
	    pfSetVec3(a, xMin, yMin, zMin);
	    pfSetVec3(b, xMax, yMin, zMin);
	    pfSetVec3(c, xMax, yMin, zMax);
	    pfSetVec3(d, xMin, yMin, zMax);
	    spongeQuad(buffer, a, b, c, d);
	    polygons += 2;
    	}
        if (sides & EAST)
	{
	    pfSetVec3(a, xMax, yMin, zMin);
	    pfSetVec3(b, xMax, yMax, zMin);
	    pfSetVec3(c, xMax, yMax, zMax);
	    pfSetVec3(d, xMax, yMin, zMax);
	    spongeQuad(buffer, a, b, c, d);
	    polygons += 2;
    	}
        if (sides & WEST)
	{
	    pfSetVec3(a, xMin, yMax, zMin);
	    pfSetVec3(b, xMin, yMin, zMin);
	    pfSetVec3(c, xMin, yMin, zMax);
	    pfSetVec3(d, xMin, yMax, zMax);
	    spongeQuad(buffer, a, b, c, d);
	    polygons += 2;
    	}
        if (sides & TOP)
	{
	    pfSetVec3(a, xMin, yMin, zMax);
	    pfSetVec3(b, xMax, yMin, zMax);
	    pfSetVec3(c, xMax, yMax, zMax);
	    pfSetVec3(d, xMin, yMax, zMax);
	    spongeQuad(buffer, a, b, c, d);
	    polygons += 2;
    	}
        if (sides & BOTTOM)
	{
	    pfSetVec3(a, xMin, yMax, zMin);
	    pfSetVec3(b, xMax, yMax, zMin);
	    pfSetVec3(c, xMax, yMin, zMin);
	    pfSetVec3(d, xMin, yMin, zMin);
	    spongeQuad(buffer, a, b, c, d);
	    polygons += 2;
    	}
    }
    else
    {
        float	xThird = (xMax - xMin)/3.0;
        float	xLess  =  xMin + xThird;
        float	xMore  =  xMax - xThird;
    
        float	yThird = (yMax - yMin)/3.0;
        float	yLess  =  yMin + yThird;
        float	yMore  =  yMax - yThird;
    
        float	zThird = (zMax - zMin)/3.0;
        float	zLess  =  zMin + zThird;
        float	zMore  =  zMax - zThird;
    
        --levels;
        
        /* 
         * 6 7 8
         * 4   5	Top layer (eight sub-squares)
         * 1 2 3
         */
        polygons += sponge(buffer, levels,  sides & (SOUTH|WEST|TOP), 
            xMin,  xLess, yMin, yLess, zMore, zMax);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels, (sides & (SOUTH|TOP))|NORTH|BOTTOM, 
            xLess, xMore, yMin, yLess, zMore, zMax);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels,  sides & (SOUTH|EAST|TOP), 
            xMore, xMax,  yMin, yLess, zMore, zMax);
	if (levels == first) pfAddChild(group, pfdBuild());
        
        polygons += sponge(buffer, levels, (sides & (WEST|TOP))|EAST|BOTTOM, 
            xMin,  xLess, yLess, yMore, zMore, zMax);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels, (sides & (EAST|TOP))|WEST|BOTTOM, 
            xMore, xMax,  yLess, yMore, zMore, zMax);
	if (levels == first) pfAddChild(group, pfdBuild());
        
        polygons += sponge(buffer, levels,  sides & (NORTH|WEST|TOP), 
            xMin,  xLess, yMore, yMax, zMore, zMax);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels, (sides & (NORTH|TOP))|SOUTH|BOTTOM, 
            xLess, xMore, yMore, yMax, zMore, zMax);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels,  sides & (NORTH|EAST|TOP), 
            xMore, xMax,  yMore, yMax, zMore, zMax);
	if (levels == first) pfAddChild(group, pfdBuild());
    	
        /* 
         * 3   4
         *      	Middle layer (four sub-squares)
         * 1   2
         */
        polygons += sponge(buffer, levels, (sides & (SOUTH|WEST))|NORTH|EAST, 
            xMin,  xLess, yMin, yLess, zLess, zMore);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels, (sides & (SOUTH|EAST))|NORTH|WEST, 
            xMore, xMax,  yMin, yLess, zLess, zMore);
	if (levels == first) pfAddChild(group, pfdBuild());
        
        polygons += sponge(buffer, levels, (sides & (NORTH|WEST))|SOUTH|EAST, 
            xMin,  xLess, yMore, yMax, zLess, zMore);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels, (sides & (NORTH|EAST))|SOUTH|WEST, 
            xMore, xMax,  yMore, yMax, zLess, zMore);
	if (levels == first) pfAddChild(group, pfdBuild());
        
        /* 
         * 6 7 8
         * 4   5	Bottom layer (eight sub-squares)
         * 1 2 3
         */
        polygons += sponge(buffer, levels,  sides & (SOUTH|WEST|BOTTOM), 
            xMin,  xLess, yMin, yLess, zMin, zLess);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels, (sides & (SOUTH|BOTTOM))|NORTH|TOP, 
            xLess, xMore, yMin, yLess, zMin, zLess);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels,  sides & (SOUTH|EAST|BOTTOM), 
            xMore, xMax,  yMin, yLess, zMin, zLess);
	if (levels == first) pfAddChild(group, pfdBuild());
        
        polygons += sponge(buffer, levels, (sides & (WEST|BOTTOM))|EAST|TOP, 
            xMin,  xLess, yLess, yMore, zMin, zLess);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels, (sides & (EAST|BOTTOM))|WEST|TOP, 
            xMore, xMax,  yLess, yMore, zMin, zLess);
	if (levels == first) pfAddChild(group, pfdBuild());
        
        polygons += sponge(buffer, levels,  sides & (NORTH|WEST|BOTTOM), 
            xMin,  xLess, yMore, yMax, zMin, zLess);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels, (sides & (NORTH|BOTTOM))|SOUTH|TOP, 
            xLess, xMore, yMore, yMax, zMin, zLess);
	if (levels == first) pfAddChild(group, pfdBuild());
        polygons += sponge(buffer, levels,  sides & (NORTH|EAST|BOTTOM), 
            xMore, xMax,  yMore, yMax, zMin, zLess);
	if (levels == first) pfAddChild(group, pfdBuild());
    }
    
    return polygons;
}

/*
 * pfdLoadFile_sponge -- build a Sponge for IRIS Performer
 */
PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_sponge (char *fileName)
{
    pfdGeom	*buffer		= NULL;
    pfNode	*node		= NULL;
    pfMaterial	*material	= NULL;
    int		 i		= 0;
    int		 levels		= 0;
    int	 	 numTris	= 0;
    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* extract recursive depth from "file name" */
    if (sscanf(fileName, "%d", &levels) != 1)
	return NULL;

    /* initialize utility library triangle/geoset builder */
    buffer = pfdNewGeom(4);

    /* enable lighting for sponge object */
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_ON);

    /* specify a material for the sponge */
    material = (pfMaterial*)pfdGetTemplateObject(pfGetMtlClassType());
    pfMtlColor(material, PFMTL_AMBIENT,  0.2f, 0.2f, 0.2f);
    pfMtlColor(material, PFMTL_DIFFUSE,  0.8f, 0.8f, 0.8f);
    pfMtlColor(material, PFMTL_SPECULAR, 0.4f, 0.4f, 0.4f);
    pfMtlShininess(material, 8.0f);
    pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_AD);
    pfdBldrStateAttr(PFSTATE_FRONTMTL, material);
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_ON);
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);

    /* add geometry to builder */
    first = levels - 1;
    group = pfNewGroup();
    numTris += sponge(buffer, levels, ALL, 
	XMIN, XMAX, YMIN, YMAX, ZMIN, ZMAX);

    /* get a complete scene graph representing file's primitives */
#if 0
    node = pfdBuild();
#else
    node = (pfNode *)group;
#endif
    if (node != NULL)
	pfNodeName(node, fileName);

    /* delete allocations */
    pfdDelGeom(buffer);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_sponge: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Recursive levels:   %8ld", levels);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Triangles:          %8ld", numTris);

    return node;
}
