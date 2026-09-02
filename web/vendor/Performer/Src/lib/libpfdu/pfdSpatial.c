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
 * pfdSpatial.c
 *
 * $Revision: 1.12 $
 * $Date: 2002/05/15 00:31:09 $
 *
 */

#include <math.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

static pfNode*
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
     * XXX would be better to use the average center of bounding
     * XXX boxes rather than the bounding box of the union;
     * XXX that would guarantee separation unless all the centers
     * XXX really are the same
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
     * if fewer than geodeChild children and smaller than specified
     * geode dimension, return a single geode
     */
    if (ngset <= 1 || ngset <= geodeChild && size2 < geodeSize*geodeSize)
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
	for (i = 0 ; i < 8 ; i++)
	    pfResetList(lists[i]);
	
	for (i = 0 ; i < ngset ; i++)
	{
	    pfGeoSet *gset = (pfGeoSet *)pfGet(gsets, i);
	    pfAdd(lists[i%4], gset);
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


/************************************************************************
 * Traversal and callback for computing a bounding box around pfGeoSets
 ***********************************************************************/

typedef struct 
{
    int	depth;
    pfList	*gsets;
} pfuGSetter;

static int
cbPreGSet(pfuTraverser *trav)
{
    pfuGSetter *gsetter = trav->data;
    int ngsets, i;
    
    if (pfIsOfType(trav->node, pfGetGeodeClassType()))
    {
	if (gsetter->depth == 0)
	{
	    ngsets = pfGetNumGSets(trav->node);
	    for (i = 0 ; i < ngsets ; i++)
	    {
		pfGeoSet *gset = pfGetGSet(trav->node, i);
		pfAdd(gsetter->gsets, gset);
	    }
	    for (i = ngsets-1 ; i >= 0 ; i--)
	    {
		pfGeoSet *gset = pfGetGSet(trav->node, i);
		pfRemoveGSet(trav->node, gset); 
	    }
	}
	return PFTRAV_CONT;
    }
    else if (pfIsOfType(trav->node, pfGetSCSClassType()))
    {
	gsetter->depth++;
	return PFTRAV_CONT;
    }
    else if (pfIsOfType(trav->node, pfGetGroupClassType()))
	return PFTRAV_CONT;
    else
	return PFTRAV_PRUNE;
}

static int
cbPostGSet(pfuTraverser *trav)
{
    int nChild;
    pfuGSetter *gsetter = trav->data;
    int i;
    
    if (pfIsOfType(trav->node, pfGetSCSClassType()))
    {
	gsetter->depth--;
    }
    else if (pfIsOfType(trav->node, pfGetGroupClassType()))
    {
	nChild = pfGetNumChildren(trav->node);
	for (i = nChild -1 ; i >= 0 ; i--)
	{
	    int numGrandKids = -1;
	    pfNode *child = pfGetChild(trav->node, i);
	    if (pfIsOfType(child, pfGetGeodeClassType()))
		numGrandKids = 0; /* pfGetNumGSets((pfGeode *)child); */
	    else if (pfIsOfType(child, pfGetGroupClassType()))
		numGrandKids = pfGetNumChildren(child);
	    
	    if (numGrandKids == 0)
	    {
		pfRemoveChild((pfGroup*)trav->node, child);
		pfDelete(child);
	    }
	}
    }
    return PFTRAV_CONT;
}

PFDUDLLEXPORT pfList *
pfdTravGetGSets(pfNode *node)
{
    pfuTraverser trav;
    pfuGSetter gsetter;
    gsetter.gsets = pfNewList(sizeof(pfGeoSet*), 100, pfGetSharedArena());
    gsetter.depth = 0;

    pfuInitTraverser(&trav);
    trav.data = &gsetter;

    trav.mode = PFUTRAV_LOD_NONE|PFUTRAV_SW_NONE|PFUTRAV_SEQ_NONE;
    trav.preFunc = cbPreGSet;
    trav.postFunc = cbPostGSet;
    trav.mstack = pfNewMStack(32, pfGetSharedArena());
    
    pfuTraverse(node, &trav);

    pfDelete(trav.mstack);
    return gsetter.gsets;
}

PFDUDLLEXPORT pfGroup *
pfdSpatialize(pfGroup *grp, float maxGeodeSize, int maxGeoSets)
{
    pfList *gsets;
    pfNode *newTree;
    pfGroup *newGrp;

    gsets = pfdTravGetGSets((pfNode *)grp);
    printf("%d geosets found\n", pfGetNum(gsets));

    newTree = subdivide(gsets, maxGeodeSize, maxGeoSets);
    newGrp = pfNewGroup();
    pfAddChild(newGrp, newTree);

    pfDelete(gsets);
    return newGrp;
}
