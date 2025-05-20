/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 */

/*
 * boxlod.c 
 *
 * $Revision: 1.22 $ 
 * $Date: 2002/05/15 00:31:09 $
 */

#include <math.h>
#include <stdlib.h>

#ifdef WIN32
double drand48() { return (double)rand()/(double)RAND_MAX; }
#else
#ifdef	_POSIX_SOURCE
extern double drand48 (void);
#endif
#endif

#include <Performer/pfutil.h>

/*
 * Input: pfBox, color, flag indicating whether boxes
 *	  should be flat shaded or blob like (gouraud 
 *	  with vertex average normals).
 *
 * Output: A pfGeoSet depicting the box
 */
PFUDLLEXPORT pfGeoSet*
pfuMakeBoxGSet(pfBox *box, pfVec4 clr, int flat)
{
    int *primLengths;
    pfVec4 *clrs;
    pfVec3 *verts, *norms;
    pfGeoSet *gset = pfNewGSet(pfGetSharedArena());

    pfGSetNumPrims(gset, 3);
    primLengths = (int *)pfMalloc(sizeof(int)*3, pfGetSharedArena());
    primLengths[0] = 10;
    primLengths[1] = 4;
    primLengths[2] = 4;
    pfGSetPrimLengths(gset, primLengths);

    if (flat)
	pfGSetPrimType(gset, PFGS_FLAT_TRISTRIPS);
    else
	pfGSetPrimType(gset, PFGS_TRISTRIPS);

    clrs = (pfVec4 *)pfMalloc(sizeof(pfVec4), pfGetSharedArena());
    pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, clrs, NULL);
    pfCopyVec4(clrs[0], clr);

    verts = (pfVec3 *)pfMalloc(18*sizeof(pfVec3), pfGetSharedArena());
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, verts, NULL);
    /* strip around */
    pfSetVec3(verts[0], box->min[0], box->min[1], box->min[2]);
    pfSetVec3(verts[1], box->max[0], box->min[1], box->min[2]);
    pfSetVec3(verts[2], box->min[0], box->min[1], box->max[2]);
    pfSetVec3(verts[3], box->max[0], box->min[1], box->max[2]);
    pfSetVec3(verts[4], box->min[0], box->max[1], box->max[2]);
    pfSetVec3(verts[5], box->max[0], box->max[1], box->max[2]);
    pfSetVec3(verts[6], box->min[0], box->max[1], box->min[2]);
    pfSetVec3(verts[7], box->max[0], box->max[1], box->min[2]);
    pfSetVec3(verts[8], box->min[0], box->min[1], box->min[2]);
    pfSetVec3(verts[9], box->max[0], box->min[1], box->min[2]);
    /* min x end */
    pfSetVec3(verts[10], box->min[0], box->max[1], box->min[2]);
    pfSetVec3(verts[11], box->min[0], box->min[1], box->min[2]);
    pfSetVec3(verts[12], box->min[0], box->max[1], box->max[2]);
    pfSetVec3(verts[13], box->min[0], box->min[1], box->max[2]);
    /* max x end */
    pfSetVec3(verts[14], box->max[0], box->min[1], box->min[2]);
    pfSetVec3(verts[15], box->max[0], box->max[1], box->min[2]);
    pfSetVec3(verts[16], box->max[0], box->min[1], box->max[2]);
    pfSetVec3(verts[17], box->max[0], box->max[1], box->max[2]);

    norms = (pfVec3 *)pfMalloc(18*sizeof(pfVec3), pfGetSharedArena());
    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
    if (flat)
    {
	pfSetVec3(norms[0],  0.0f, -1.0f,  0.0f);
	pfSetVec3(norms[1],  0.0f, -1.0f,  0.0f);
	pfSetVec3(norms[2],  0.0f,  0.0f,  1.0f);
	pfSetVec3(norms[3],  0.0f,  0.0f,  1.0f);
	pfSetVec3(norms[4],  0.0f,  1.0f,  0.0f);
	pfSetVec3(norms[5],  0.0f,  1.0f,  0.0f);
	pfSetVec3(norms[6],  0.0f,  0.0f, -1.0f);
	pfSetVec3(norms[7],  0.0f,  0.0f, -1.0f);
	/* min x end */
	pfSetVec3(norms[8], -1.0f, 0.0f,  0.0f);
	pfSetVec3(norms[9], -1.0f, 0.0f,  0.0f);
	/* max x end */
	pfSetVec3(norms[10],  1.0f, 0.0f,  0.0f);
	pfSetVec3(norms[11],  1.0f, 0.0f,  0.0f);
    }
    else
    {
	int i;

	pfSetVec3(norms[0], -1.0f, -1.0f, -1.0f);
	pfSetVec3(norms[1],  1.0f, -1.0f, -1.0f);
	pfSetVec3(norms[2], -1.0f, -1.0f,  1.0f);
	pfSetVec3(norms[3],  1.0f, -1.0f,  1.0f);
	pfSetVec3(norms[4], -1.0f,  1.0f,  1.0f);
	pfSetVec3(norms[5],  1.0f,  1.0f,  1.0f);
	pfSetVec3(norms[6], -1.0f,  1.0f, -1.0f);
	pfSetVec3(norms[7],  1.0f,  1.0f, -1.0f);
	pfSetVec3(norms[8], -1.0f, -1.0f, -1.0f);
	pfSetVec3(norms[9],  1.0f, -1.0f, -1.0f);
	/* min x end */
	pfSetVec3(norms[10], -1.0f, 1.0f, -1.0f);
	pfSetVec3(norms[11], -1.0f,-1.0f, -1.0f);
	pfSetVec3(norms[12], -1.0f, 1.0f,  1.0f);
	pfSetVec3(norms[13], -1.0f,-1.0f,  1.0f);
	/* max x end */
	pfSetVec3(norms[14],  1.0f,-1.0f, -1.0f);
	pfSetVec3(norms[15],  1.0f, 1.0f, -1.0f);
	pfSetVec3(norms[16],  1.0f,-1.0f,  1.0f);
	pfSetVec3(norms[17],  1.0f, 1.0f,  1.0f);

	for (i = 0 ; i < 18 ; i++)
	    pfNormalizeVec3(norms[i]);
    }
    return gset;
}

/*
 * Input: A subgraph and a depth
 * 
 * Output: 
 *        A new graph with all subgraphs below the specified
 *	  depth replaced by geodes with geometry representing
 *	  the corresponding bounding box.
 */

typedef struct 
{
    int targCount;
    int flat;
    pfVec4 *clr;
} Cloner;


static int
cbPreBoxClone(pfuTraverser *trav)
{
    Cloner *cl = (Cloner *)trav->data;
    pfNode *node = trav->node;
    pfBox box;
    int nChild, i;
    pfGeode *newGeode = NULL;

    if (!pfIsOfType(node, pfGetGroupClassType()))
	return PFTRAV_PRUNE;

    nChild = pfGetNumChildren(node);

    /* 
     * check if number of vertices under any children
     * is less than target, if so replace with bbox - SLOW
     */

    for (i = 0 ; i < nChild ; i++)
    {
	int numVerts;
	pfNode *child = pfGetChild(node, i);
	numVerts = pfuTravCountNumVerts(child);

	if (numVerts <= cl->targCount && numVerts > 18)
	{
	    pfGeoSet *boxGSet;
	    pfVec4 clr;

	    if (cl->clr == NULL)
		pfSetVec4(clr, drand48(), drand48(), drand48(), 1.0f);
	    else
		pfCopyVec4(clr, *(cl->clr));
	    pfuTravCalcBBox(child, &box);
	    boxGSet = pfuMakeBoxGSet(&box, clr, cl->flat);
	    if (newGeode == NULL)
		newGeode = pfNewGeode();
	    pfAddGSet(newGeode, boxGSet);
	    pfRemoveChild(node, child);
	    i--;
	    nChild--;
	}
    }
    if (newGeode)
	pfAddChild(node, newGeode);
    return PFTRAV_CONT;
}

PFUDLLEXPORT pfNode* 
pfuBoxClone(pfNode *node, int targCount, int flat, pfVec4 *clrp)
{
    Cloner cl;
    pfuTraverser trav;
    pfNode *newGraph;
    int numVerts;

    pfuInitTraverser(&trav);

    cl.targCount = targCount;
    cl.flat = flat;
    cl.clr = clrp;

    numVerts = pfuTravCountNumVerts(node);

    if (numVerts <= targCount && numVerts > 18)
    {
	pfBox box;
	pfVec4 clr;
	pfGeoSet *boxGSet;
	pfGeode *geode;
	pfuTravCalcBBox(node, &box);
	if (clrp == NULL)
	    pfSetVec4(clr, drand48(), drand48(), drand48(), 1.0f);
	else
	    PFCOPY_VEC4(clr, *clrp);
	    
	boxGSet = pfuMakeBoxGSet(&box, clr, flat);
	geode = pfNewGeode();
	pfAddGSet(geode, boxGSet);
	return (pfNode *)geode;
    }

    newGraph = pfClone(node, 0);

    trav.preFunc = cbPreBoxClone;
    trav.data = &cl;

    pfuTraverse(newGraph, &trav);

    return newGraph;
}

/*
 * Input: A subgraph
 * 
 * Output: 
 *        An LOD which draws low-LOD geometry as bounding boxes
 */

PFUDLLEXPORT pfLOD*
pfuBoxLOD(pfGroup *grp, int flat, pfVec4 *clr)
{
    pfLOD *lod;
    int numVerts;
    pfBox box;
    pfVec3 center;
    float size;
    int i;
    int lastVerts;
    int il = 0;

    pfuTravCalcBBox((pfNode *)grp, &box);
    size = 2 * PFDISTANCE_PT3(box.min, box.max);
    pfCombineVec3(center, 0.5f, box.min, 0.5f, box.max);

    lod = pfNewLOD();
    pfLODCenter(lod, center);

    numVerts = lastVerts = pfuTravCountNumVerts((pfNode *)grp);
    pfAddChild(lod, grp);
    pfLODRange(lod, il, size*il);
    il++;

    /*
     * try for a factor of two each time 36 is 2X the 
     * number of vertices in a tstripped box 
     */
    for (i = 36 ; i < 2 * numVerts ; i *= 2)
    {
	pfNode *boxy = pfuBoxClone((pfNode *)grp, i, flat, clr);
	if (boxy)
	{
	    int newVerts = pfuTravCountNumVerts(boxy);
	    if (newVerts < lastVerts)
	    {
		pfAddChild(lod, boxy);
		pfLODRange(lod, il, size*il);
		il++;
	    }
	    else
	    {
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
			 "BoxLOD throwing away LOD with %d vertices, higher than previous with %d", 
			 newVerts, lastVerts);
		pfDelete(boxy);
	    }
	}
    }
    pfLODRange(lod, il, size*il);
    return lod;
}
