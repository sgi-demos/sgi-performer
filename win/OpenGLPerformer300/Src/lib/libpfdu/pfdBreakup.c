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
 * pfdBreakup.c
 *
 * $Revision: 1.22 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * Input: A pfGeoSet that:
 * 	1. Is non-indexed.
 *	2. Is PFGS_STRIPS
 *	Input is NOT deleted, this is left to the caller
 * 
 * Output: 
 *      1: The root of a subgraph
 *      2: The subgraph is octree-like, with geodes
 *         of no larger than geodeSize and strips no longer 
 *         than stripLength.
 */

#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#endif

#include <Performer/pf.h>
#include <Performer/pfdu-DLL.h>

#define INDEX ushort

static pfNode*
subdivide(pfList *gsets, float geodeSize, int geodeChild);

extern PFDUDLLEXPORT pfNode *
pfdBreakup(pfGeode *geode, float geodeSize, int stripLength, int geodeChild)
{
    pfNode *node;
    pfList *gsets;
    int igs;
    int numGSet = pfGetNumGSets(geode);
    
    /* striplength must be even */
    if (stripLength &1)
	stripLength++;
    
    gsets = pfNewList(sizeof(pfGeoSet*), 100, pfGetSharedArena());
    
    for (igs = 0 ; igs < numGSet ; igs++)
    {
	pfGeoSet *gs = pfGetGSet(geode, igs);
	/* 
	 * break single geoset with many strips into multiple geosets
	 * each of stripLength 
	 */
	
	int numPrims = pfGetGSetNumPrims(gs);
	pfVec3 *coords;
	pfVec3 *norms;
	pfVec4 *colors;
	pfVec2 *tcoords;
	INDEX  *verti, *normi, *colori, *tcoordi;
	int   *plens;
	int	nbind, cbind, tbind;
	int	nextVert, nextColor, nextNorm;
	int 	i;
	int	primType;
	pfGeoState *gstate;

	gstate = pfGetGSetGState(gs);
	pfGetGSetAttrLists(gs, PFGS_COORD3, (void **)&coords, &verti);
	pfGetGSetAttrLists(gs, PFGS_NORMAL3, (void **)&norms, &normi);
	pfGetGSetAttrLists(gs, PFGS_COLOR4, (void **)&colors, &colori);
	pfGetGSetAttrLists(gs, PFGS_TEXCOORD2, (void **)&tcoords, &tcoordi);
	
	primType = pfGetGSetPrimType(gs);
	if ((primType != PFGS_TRISTRIPS &&
	    primType != PFGS_FLAT_TRISTRIPS) ||
	    verti != NULL ||
	    normi != NULL || 
	    colori != NULL ||
	    tcoordi  != NULL)
	{
	    pfGeoSet *newgs = pfNewGSet(pfGetSharedArena());
	    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, 
		     "pfdBreakup: not a set of non-indexed tstrips");
	    pfCopy(newgs, gs);
	    pfAdd(gsets, newgs);
	    continue;
	}
	
	plens = pfGetGSetPrimLengths(gs);
	
	nbind = pfGetGSetAttrBind(gs, PFGS_NORMAL3);
	cbind = pfGetGSetAttrBind(gs, PFGS_COLOR4);
	tbind = pfGetGSetAttrBind(gs, PFGS_TEXCOORD2);
	
	nextVert = nextColor = nextNorm = 0;
	for (i = 0 ; i < numPrims ; i++)
	{
	    int nextPrimVert = nextVert + plens[i];
	    /* starting tri for this geoset */
	    while (nextVert < nextPrimVert - 2)
	    {
		pfGeoSet *newGSet;
		int startVert, remainVert;
		int startColor, startNorm;
		int numVerts, numColors, numNorms;
		pfVec3 *newCoords, *newNorms;
		pfVec4 *newColors;
		pfVec2 *newTCoords;

		startVert = nextVert;

		/* 
		 * when near the end, split in half so we don't 
		 * end up with a silly small geoset on the end
		 */
		remainVert = nextPrimVert - nextVert;
		if (remainVert < 2*stripLength && remainVert > stripLength)
		{
		    nextVert += remainVert/2;
		    if ((nextVert - startVert) & 1)
			nextVert++;
		}
		else
		    nextVert += (stripLength-2);

		if (nextVert > (nextPrimVert-2))
		    nextVert = nextPrimVert - 2;
		
		numVerts = nextVert - startVert + 2;
		
		newGSet = pfNewGSet(pfGetSharedArena());	
		pfAdd(gsets, newGSet);
		pfGSetPrimType(newGSet, primType);
		/* XXX - for better geoset size, should group multiple 
		 * strips up to striplength verts*/
		pfGSetNumPrims(newGSet, 1);
		pfGSetGState(newGSet, gstate);
		{
		    int *pl = (int *)pfMalloc(sizeof(int), 
						pfGetSharedArena());
		    pl[0] = numVerts;
		    pfGSetPrimLengths(newGSet, pl);
		}
		/* 
		 * copy coords
		 */
		newCoords = (pfVec3*)pfMalloc((uint)(numVerts*sizeof(pfVec3)),
			pfGetSharedArena());
		bcopy(&coords[startVert], newCoords, 
		      (uint)(numVerts * sizeof(pfVec3)));
		pfGSetAttr(newGSet, PFGS_COORD3, PFGS_PER_VERTEX,
			   newCoords, NULL);
		
		/* 
		 * copy normals 
		 */
		switch (nbind)
		{
		case PFGS_PER_VERTEX:
		    startNorm = nextNorm;
		    if (primType == PFGS_FLAT_TRISTRIPS)
		    {
			numNorms = numVerts - 2;
			nextNorm += numNorms;
		    }
		    else
		    {
			numNorms = numVerts;
			nextNorm += (numNorms-2);
		    }
		    break;
		case PFGS_PER_PRIM:
		    startNorm = nextNorm = i;
		    numNorms = 1;
		    break;
		case PFGS_OVERALL:
		    startNorm = nextNorm = 0;
		    numNorms = 1;
		    break;
		case PFGS_OFF:
		default:
		    numNorms = 0;
		    break;
		}
		newNorms = NULL;
		if (numNorms > 0)
		{
		    newNorms = (pfVec3 *)pfMalloc((uint)(numNorms * sizeof(pfVec3)), 
			  pfGetSharedArena());
		    bcopy(&norms[startNorm], newNorms, 
			  (uint)(numNorms*sizeof(pfVec3)));
		}
		pfGSetAttr(newGSet, PFGS_NORMAL3, nbind, newNorms, NULL);
		
		/* 
		 * copy colors
		 */
		switch (cbind)
		{
		case PFGS_PER_VERTEX:
		    startColor = nextColor;
		    if (primType == PFGS_FLAT_TRISTRIPS)
		    {
			numColors = numVerts - 2;
			nextColor += numColors;
		    }
		    else
		    {
			numColors = numVerts;
			nextColor += (numColors-2);
		    }
		    break;
		case PFGS_PER_PRIM:
		    startColor = nextColor = i;
		    numColors = 1;
		    break;
		case PFGS_OVERALL:
		    startColor = nextColor = 0;
		    numColors = 1;
		    break;
		case PFGS_OFF:
		default:
		    numColors = 0;
		    break;
		}
		newColors = NULL;
		if (numColors > 0)
		{
		    newColors = (pfVec4 *)pfMalloc((uint)(numColors*sizeof(pfVec4)),
						   pfGetSharedArena());
		    bcopy(&colors[startColor], newColors,
			  (uint)(numColors*sizeof(pfVec4)));
		}
		pfGSetAttr(newGSet, PFGS_COLOR4, cbind, newColors, NULL);
		/*
		 * copy tcoords 
		 */
		switch (tbind)
		{
		case PFGS_PER_VERTEX:
		    newTCoords = (pfVec2 *)pfMalloc((uint)(numVerts*sizeof(pfVec2)), 
						    pfGetSharedArena());
		    bcopy(&tcoords[startVert], newTCoords, 
			(uint)(numVerts*sizeof(pfVec2)));
		    pfGSetAttr(newGSet, PFGS_TEXCOORD2, PFGS_PER_VERTEX,
			       newTCoords, NULL);
		    break;
		case PFGS_OFF:
		default:
		    pfGSetAttr(newGSet, PFGS_TEXCOORD2, PFGS_OFF,
			       NULL, NULL);
		    break;
		}
		{
		    /* XXX debug */
		    pfBox box;
		    pfGetGSetBBox(newGSet, &box);
		}
	    }
	    nextVert = nextPrimVert;
	    if (nbind == PFGS_PER_VERTEX &&
		primType != PFGS_FLAT_TRISTRIPS)
		nextNorm += 2;
	    if (cbind == PFGS_PER_VERTEX &&
		primType != PFGS_FLAT_TRISTRIPS)
		nextColor += 2;
	}
    }
    /* 
     * Take array of geosets and place into a rough octree 
     * based upon bbox centers 
     */
    node = subdivide(gsets, geodeSize, geodeChild);
    pfDelete(gsets);
    return node;
}

static pfNode *
subdivide(pfList *gsets, float geodeSize, int geodeChild)
{
    int i, j;
    int active;
    pfList *lists[8];
    int ngset;
    pfBox bigbox;
    pfVec3 center;
    float size2;
    pfPlane planes[3];
    pfGroup *grp;
    pfBox gsbbox;
    
    ngset = pfGetNum(gsets);
    pfMakeEmptyBox(&bigbox);
    
    /* 
     * compute center 
     */
    for (i = 0 ; i < ngset ; i++)
    {
	pfGeoSet *gset;
	gset = (pfGeoSet *)pfGet(gsets, i);
	pfGetGSetBBox(gset, &gsbbox);
	pfBoxExtendByBox(&bigbox, &gsbbox);
    }
    pfCombineVec3(center, 0.5f, bigbox.min, 0.5f, bigbox.max);
    
    size2 = PFSQR_DISTANCE_PT3(bigbox.min, bigbox.max);
    
    /* 
     * if fewer than 4 children or smaller than specified
     * geode dimension, return a single geode
     */
    if (ngset <= geodeChild || size2 < geodeSize*geodeSize)
    {
	pfGeode *geode = pfNewGeode();
	for (i = 0 ; i < ngset ; i++)
	{
	    pfGeoSet *gset;
	    gset = (pfGeoSet *)pfGet(gsets, i);
	    pfAddGSet(geode, gset);
	}
	return (pfNode*)geode;
    }
    
    for (i = 0 ; i < 8 ; i++)
	lists[i] = pfNewList(sizeof(pfGeoSet*), 10, pfGetSharedArena());
    
    {
	pfVec3  norms[3] = { {1.0, 0.0, 0.0}, 
			     {0.0, 1.0, 0.0}, 
			     {0.0, 0.0, 1.0}};
	for (j = 0 ; j < 3 ; j++)
	    pfMakeNormPtPlane(&planes[j], norms[j], center);
    }
    /* 
     * place geosets on one of eight lists based on center
     */
    for (i = 0 ; i < ngset ; i++)
    {
	pfBox bbox;
	pfVec3 gscenter;
	pfGeoSet *gset;
	int which = 0;
	
	gset = (pfGeoSet *)pfGet(gsets, i);
	pfGetGSetBBox(gset, &bbox);
	pfCombineVec3(gscenter, 0.5f, bbox.min, 0.5f, bbox.max);
	
	for (j = 0 ; j < 3 ; j++)
	{
	    if (pfHalfSpaceContainsPt(&planes[j], gscenter))
		which |= (1<<j);
	}
	pfAdd(lists[which], gset);
    }
    /* 
     * check for number of active octants
     */
    active = 0;
    for (i = 0 ; i < 8 ; i++)
    {
	if (pfGetNum(lists[i]) > 0)
	    active++;
    }
    /*
     * if only one octant active, arbitrarily subdivide
     */
    grp = pfNewGroup();
    
    if (active < 2)
    {
	int nper = PF_MAX2(ngset/4, 3);
	int which = 0;
	for (i = 0 ; i < 8 ; i++)
	    pfResetList(lists[i]);
	
	for (i = 0 ; i < ngset ; i++)
	{
	    pfGeoSet *gset = (pfGeoSet *)pfGet(gsets, i);
	    if (i > (which+1)*nper)
		which++;
	    pfAdd(lists[which], gset);
	}
    }
    /* 
     * add recursion result to current group &  cleanup
     */
    for (i = 0 ; i < 8 ; i++)
    {
	if (pfGetNum(lists[i]) > 0)
	{
	    pfNode *child = subdivide(lists[i], geodeSize, geodeChild);
	    pfAddChild(grp, child);
	}
	pfDelete(lists[i]);
    }
    return (pfNode *)grp;
}
