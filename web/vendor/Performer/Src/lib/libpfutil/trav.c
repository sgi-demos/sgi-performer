/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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
 * file: trav.c
 * ------------- 
 * 
 * $Revision: 1.118 $
 * $Date: 2002/07/30 21:21:22 $
 * 
 *  Sample implementation of general user traversal utility.
 * 
 *  Specific Traversals implemented here:
 *  o pfuTravPrintNodes() - a printing traversal
 *  o pfuTravGLProf() - a traversal for outputing glprof object tags
 *			(needs libflprof.a)
 *  o pfuTravCountDB() - accumulates static graphics and database statistics
 *			    for the tree under the given node.
 *  o pfuTravNodeHlight() - sets a given highlighting structure on all of the
 *			    pfGeoSet under the given node.
 *  o pfuTravNodeAttrBind() - sets a given attr to the given bind value on
 *			    every pfGeoSet under the given node.
 *  o pfuTravCalcBBox() - compute the bounding box
 *  o pfuTravCountNumVerts() - count the number of vertices
 *  o pfuTravSetDListMode() -
 *  o pfuTravCreatePackedAttrs() -
 *  o pfuFillGSetPackedAttrs() -
 *  o pfuDelGSetAttrs() -
 *  o pfuTravCachedCull() -
 *  o pfuCalcDepth() - calculate the depth of the scene graph rooted at node.
 *			a single root node with no children is counted
 *			as having depth 1.
 *  o pfuLowestCommonAncestor() - find the lowest common ancestor
 *	     of all nodes under node for which a given function returns true.
 *  o pfuLowestCommonAncestorOfGeoSets() - find the lowest common ancestor node
 *	     of all geosets under node for which a given function returns true.
 *  o pfuFindTexture() - find the n'th texture under a given node
 *			 for which a given function returns true.
 */

/******************************************************************************
 *			    NOTES
 ******************************************************************************
 */

/*
 *  Sample code to put in program for calling the GLProf Traversal
 
    if (doGLProfTraversal)
    {
	    if (glProfMode)
	    {
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
			"installing glprof object tags");
		pfuDoTravGLProf((pfNode *)scene, 1);
	    }
	    else
	    {
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
			"removing glprof object tags");
		pfuDoTravGLProf((pfNode *)scene, 1);
	    }
	   doGLProfTraversal = 0;
    }
*/

/* Traversals for SCSs and DCSs: 
 * need matrix stacks if callback wants to be able to transform from
 * object coordinates into world coordinates.  
 * 
    trav.mstack = pfNewMStack(32, pfGetSharedArena());
    
    ....
    
    pfuTraverse(trav);
    
    pfDelete(trav.mstack);
 */

#define _PFU_TRAV_C_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32 
#include <X11/X.h>
#endif

#include <Performer/pfutil.h>

/*extern void glpObject(unsigned int inNum, const char *inName);*/
void glprof_object (char *str)
{
(str, str);
}

/******************************************************************************
 *			    Static Variables
 ******************************************************************************
*/


/* tables for printing modes and types */

static char            primTable[9][32] ={
			       "POINTS",
			       "LINES",
			       "LINESTRIPS",
			       "TRIS",
			       "QUADS",
			       "TRISTRIPS",
			       "FLAT_LINESTRIPS",
			       "FLAT_TRISTRIPS",
			       "POLYS"
			       "TRIFANS"
			       "FLAT_TRIFANS"
};

#if	0
static char		*attrTable[4] = {
				"PFGS_COLOR4",
				"PFGS_NORMAL3",
				"PFGS_TEXCOORD2",
				"PFGS_COORD3"
};
#endif

static char            bindTable[4][32] ={
			       "OFF",
			       "OVERALL",
			       "PER_PRIM",
			       "PER_VERT"
};

static FILE *outFP;

/******************************************************************************
 *			    General traversal routine
 ******************************************************************************
 */

/* 
 * Set up pfuTraverser with reasonable defaults.
*/
PFUDLLEXPORT void
pfuInitTraverser(pfuTraverser *trav)
{
    trav->preFunc = NULL;
    trav->postFunc = NULL;
    trav->mode = PFUTRAV_SW_ALL | PFUTRAV_LOD_ALL | PFUTRAV_SEQ_ALL;
    trav->mstack = NULL;
    trav->data = NULL;
    trav->node = NULL;
    trav->depth = 0;
}
 
/*
 * Main traversal routine
 *	recursively traverses database under node, applying the
 *	the pre and post traversal functions specied in the
 *	trav structure to each node.
 */

#define PFU_DO_RET(_ret) \
    switch (_ret) \
    { \
    case PFTRAV_PRUNE: \
        trav->node = prevNode; \
        if (needPop)	       \
	    pfPopMStack(trav->mstack); \
        return PFTRAV_CONT; \
    case PFTRAV_TERM: \
        trav->node = prevNode; \
        if (needPop)	       \
	    pfPopMStack(trav->mstack); \
        return PFTRAV_TERM; \
    }

static int
travChild(pfGroup *group, int i, pfuTraverser *trav)
{
   int ret;

   trav->depth++;
   ret = pfuTraverse((pfNode*)pfGetChild(group, i), trav);	
   trav->depth--;

   return ret;
}

PFUDLLEXPORT int
pfuTraverse(pfNode *node, pfuTraverser *trav)
{
    int 	i;
    int 	numChild = 1;
    int 	val;
    float 	range0, range1;
    int  	ret = PFTRAV_CONT, needPop = 0;
    pfNode 	*prevNode = trav->node;
    
    if (node == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuTraverse() Null node");
	return PFTRAV_CONT;
    }
	
    trav->node =  (pfNode *)node;
    
    if (trav->mstack)
    {
	if (pfIsOfType(node, pfGetSCSClassType()))
	{
	    pfMatrix 	mat;

	    pfGetSCSMat((pfSCS *)trav->node, mat);

	    pfPushMStack(trav->mstack);
	    pfPreMultMStack(trav->mstack, mat);
	    needPop = 1;
	}
	else if (pfIsOfType(node, pfGetDoubleSCSClassType()))
	{
	    pfMatrix4d 	mat4d;
	    pfMatrix mat;

	    pfGetDoubleSCSMat((pfDoubleSCS *)trav->node, mat4d);

#ifdef DOUBLE_DEBUG
	    // XXX cop out and convert to single
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfuTraverse: converting pfDoubleSCS matrix to single");
#endif /* DOUBLE_DEBUG */
	    PFCOPY_MAT(mat, mat4d);

	    pfPushMStack(trav->mstack);
	    pfPreMultMStack(trav->mstack, mat);
	    needPop = 1;
	}
    }

    if (trav->preFunc)
	ret = (*trav->preFunc)(trav);

    PFU_DO_RET(ret);

    /* Reset variables in case preFunc replaced the current node */
    node = trav->node;
    if (node == NULL)
	return PFTRAV_CONT;

    /* after preFunc, in case topology changed */
    if (pfIsOfType(node, pfGetGroupClassType()))
	numChild = pfGetNumChildren(node);
    
    /* pfSwitch */
    if (pfIsOfType(node, pfGetSwitchClassType()))
	switch (trav->mode & PFUTRAV_SW_MASK)
	{
	case PFUTRAV_SW_CUR:
	    val = pfGetSwitchVal((pfSwitch *)node);
	    ret = travChild((pfGroup*)node, val, trav);
	    break;
	case PFUTRAV_SW_ALL:
	    for (i = 0 ; i < numChild && ret == PFTRAV_CONT ; i++)
	    {
		ret = travChild((pfGroup*)node, i, trav);
		if(numChild > pfGetNumChildren(node)) /* if the child is deleted */
		{
		    i--;
		    numChild--;
		}
	    }
	    break;
	case PFUTRAV_SW_NONE:
	    break;
	}
    /* pfLOD */
    else if (pfIsOfType(node, pfGetLODClassType()))
	switch (trav->mode & PFUTRAV_LOD_MASK)
	{
	case PFUTRAV_LOD_RANGE0:
	    range0 = pfGetLODRange((pfLOD *)node, 0);
	    range1 = pfGetLODRange((pfLOD *)node, 1);
	    if (range0 <= 0.0f && 0.0f < range1)
		if (numChild > 0)
		    ret = travChild((pfGroup*)node, 0, trav);
	    break;
	case PFUTRAV_LOD_ALL:
	    for (i = 0 ; i < numChild && ret == PFTRAV_CONT ; i++)
	    {
		ret = travChild((pfGroup*)node, i, trav);
		if(numChild > pfGetNumChildren(node)) /* if the child is deleted */
		{
		    i--;
		    numChild--;
		}
	    }
	    break;
	}
    /* pfSequence */
    else if (pfIsOfType(node, pfGetSeqClassType()))
	switch (trav->mode & PFUTRAV_SEQ_MASK)
	{
	case PFUTRAV_SEQ_CUR:
	    if ((val = pfGetSeqFrame((pfSequence *)node, NULL)) >= 0)
		ret = travChild((pfGroup*)node, val, trav);
	    else
		ret = PFTRAV_PRUNE;
	    break;
	case PFUTRAV_SEQ_ALL:
	    for (i = 0 ; i < numChild && ret == PFTRAV_CONT ; i++)
	    {
		ret = travChild((pfGroup*)node, i, trav);
		if(numChild > pfGetNumChildren(node)) /* if the child is deleted */
		{
		    i--;
		    numChild--;
		}
	    }
	    break;
	case PFUTRAV_SEQ_NONE:
	    break;
	}
    /* other pfGroups */
    else if (pfIsOfType(node, pfGetGroupClassType()))
	for (i = 0 ; i < numChild && ret == PFTRAV_CONT ; i++)
	{
	    ret = travChild((pfGroup*)node, i, trav);
	    if(numChild > pfGetNumChildren(node)) /* if the child is deleted */
	    {
		i--;
		numChild--;
	    }
	}
    /* other nodes, presume leaf */
    else if (pfIsOfType(node, pfGetNodeClassType()))
	;
    else
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfuTraverse() Unknown node type 0x%x - %s",
		 pfGetType(node), pfGetTypeName((pfObject *)node));
    
    PFU_DO_RET(ret);
    
    trav->node = (pfNode *)node;
    if (trav->postFunc)
	ret = (*trav->postFunc)(trav);
    
    PFU_DO_RET(ret);
    
    if (needPop)
	pfPopMStack(trav->mstack);
    
    return PFTRAV_CONT;
}

/************************************************************
 * Various and Sundry Sample Traversals using pfuTraverse
 ************************************************************/

/************************************************************************
 * Traversal and callback for printing info on pfGeoSets inside pfGeodes
 ***********************************************************************/

/*
 * traversal callback for printing out database information
 */

static int 
cbPrintNodeInfo(pfuTraverser *trav)
{
    pfNode *node = trav->node;
    
    if (node == NULL) 
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbPrintNodeInfo: node null!");
	return PFTRAV_CONT;
    } 
    
    fprintf(outFP, "\nNode %s: type: %s address 0x%p\n",
	    pfGetNodeName(node), pfGetTypeName((pfObject *)node), node);
    
    if (pfIsOfType(node, pfGetGeodeClassType()))
    {
	int nGSets = pfGetNumGSets((pfGeode *)node);
	int i;
	
	fprintf(outFP, "\tnum GeoSets: %d\n", nGSets);
	
	for (i = 0 ; i < nGSets ; i++)
	{
	    pfGeoSet *gd = pfGetGSet((pfGeode *)node, i);
	    int nPrims = pfGetGSetNumPrims(gd);
	    int  ptype = pfGetGSetPrimType(gd);
	    int cbind = pfGetGSetAttrBind(gd, PFGS_COLOR4);
	    int nbind = pfGetGSetAttrBind(gd, PFGS_NORMAL3);
	    int tbind = pfGetGSetAttrBind(gd, PFGS_TEXCOORD2);
	    pfGeoState *gs = pfGetGSetGState((pfGeoSet *)node);
	    
	    int nVerts = 0;
	    int k;
	    
	    fprintf(outFP, 
		    "\t    GeoSet %d: %s - %d prims, cbind=%s, "
		    "nbind=%s, tbind=%s flatshade=%d\n", 
		    i, primTable[ptype], nPrims, 
		    bindTable[cbind], bindTable[nbind], bindTable[tbind], 
		    pfGetGSetDrawMode(gd, PFGS_FLATSHADE));
	    
	    switch (ptype)
	    {
	    case PFGS_LINES:
		nVerts = 2 *nPrims;
		break;
	    case PFGS_TRIS:
		nVerts = 3 * nPrims; 
		break;
	    case PFGS_QUADS:
		nVerts = 4 * nPrims;
		break;
	    case PFGS_LINESTRIPS:
	    case PFGS_FLAT_LINESTRIPS:
		{
		    for (k = 0 ; k < nPrims ; k++)
			nVerts += pfGetGSetPrimLength(gd, k);
		}
		break;
	    case PFGS_TRISTRIPS:
	    case PFGS_FLAT_TRISTRIPS:
	    case PFGS_POLYS:
	    case PFGS_TRIFANS:
	    case PFGS_FLAT_TRIFANS:
		{
		    int len;
		    fprintf(outFP, "\t\tlengths:");
		    for (k = 0 ; k < nPrims ; k++) {
			len = pfGetGSetPrimLength(gd, k);
			nVerts += len;
			fprintf(outFP, " %d",len); 
		    }
		    fprintf(outFP, "\n");
		}
		break;
	    }
	    fprintf(outFP, "\tGeoState: 0x%x\n", gs);
	}
    }
    return PFTRAV_CONT;
}

/* Printing Traversal */
PFUDLLEXPORT void
pfuTravPrintNodes(pfNode *node, const char *fname)
{
    pfuTraverser trav;
    pfuInitTraverser(&trav);
    
    if (!fname) 
	outFP = stdout;
    else if (!(outFP = fopen(fname,  "w"))) {
        pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, 
		 "pfuTravPrintNodes: can't open %s for write", fname);
        exit(0);
    }
    
    trav.preFunc = cbPrintNodeInfo;
    
    pfuTraverse(node, &trav);
}


/*****************************************************************
 * Traversal and callback to accumulate counts into a pfFrameStats
 *****************************************************************/

static int
cbCountNodes(pfuTraverser *trav)
{
    pfNode *node = trav->node;
    pfFrameStats *fstats = (pfFrameStats *)(trav->data);
    int nGSets, i;
    
    if (node == NULL) 
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbCountNodes: node null!");
    
    /* pfBillboard */
    else if (pfIsOfType(node, pfGetBboardClassType()))
    {
	pfFStatsCountNode(fstats, PFFSTATS_DB, PFFSTATS_DB_VIS, node);
	nGSets = pfGetNumGSets((pfBillboard *)node);
	for (i = 0 ; i < nGSets ; i++)
	{
	    pfGeoSet *gs = pfGetGSet((pfGeode *)node, i);
	    pfFStatsCountGSet(fstats, gs);
	}
    }
    /* pfGeode */
    else if (pfIsOfType(node, pfGetGeodeClassType()))
    {
	pfFStatsCountNode(fstats, PFFSTATS_DB, PFFSTATS_DB_VIS, node);
	nGSets = pfGetNumGSets((pfGeode *)node);
	for (i = 0 ; i < nGSets ; i++)
	{
	    pfGeoSet *gs = pfGetGSet((pfGeode *)node, i);
	    pfFStatsCountGSet(fstats, gs);
	}
    }
    else
	pfFStatsCountNode(fstats, PFFSTATS_DB, PFFSTATS_DB_VIS, node);
    
    return PFTRAV_CONT;
}

PFUDLLEXPORT void
pfuTravCountDB(pfNode *node, pfFrameStats *fstats)
{
    pfuTraverser trav;
    
    pfFStatsClass(fstats, PFSTATS_ENGFX | PFFSTATS_ENDB, PFSTATS_ON);
    pfFStatsClassMode(fstats, PFSTATS_GFX, 
		      PFSTATS_GFX_GEOM | PFSTATS_GFX_TSTRIP_LENGTHS, 
		      PFSTATS_SET);
    pfOpenFStats(fstats, PFSTATS_ENGFX | PFFSTATS_ENDB);
    
    pfuInitTraverser(&trav);
    trav.preFunc = cbCountNodes;
    trav.data = fstats;
    
    pfuTraverse(node, &trav);
    
    pfCloseFStats(PFSTATS_ENGFX | PFFSTATS_ENDB);
}

/***********************************************************************
 * Traversal for highlighting all pfGeodes and pfBillboards under a node
 ************************************************************************/

static void
fluxed_gset_hlight(void *gset, void *hl)
{
    pfGSetHlight((pfGeoSet*)gset, (pfHighlight*)hl);
}


/* callback sets on the given hlight structure in 
 * trav->data on all the gsets under the node 
 */
static int 
cbNodeHlight(pfuTraverser *trav)
{
    pfNode *node = trav->node;
    pfHighlight *hl = (pfHighlight *)trav->data;
    int nGSets, i;
    static pfList *geom = NULL;

    if(geom == NULL)
	geom = pfNewList(sizeof(pfGroup*), 1, pfGetSharedArena());
    
    if (node == NULL) 
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbNodeHlight: node null!");
    else if (pfIsOfType(node, pfGetASDClassType()))
    {
/* we only get geometry in the first channel because trav doesn't contain a channel pointer */
  	pfGetASDActiveGeom((pfASD *)node, NULL, geom);
	for(i = 0; i < pfGetNum(geom); i++)
	    pfuTraverse((pfNode *)pfGet(geom, i), trav);
    }
    else if (pfIsOfType(node, pfGetGeodeClassType()))
    {
	nGSets = pfGetNumGSets((pfGeode *)node);
	for (i = 0 ; i < nGSets ; i++)
	{
	    pfGeoSet *gd = pfGetGSet((pfGeode *)node, i);
	    if (pfIsFluxed(gd))
	        pfFluxCallDataFunc(pfGetFlux(gd), fluxed_gset_hlight, hl);
	    else
		pfGSetHlight(gd, hl);
	}
    }
    return PFTRAV_CONT;
}

PFUDLLEXPORT void 
pfuTravNodeHlight(pfNode *node, pfHighlight *hl)
{
    pfuTraverser trav;
    
    pfuInitTraverser(&trav);
    
    trav.preFunc = cbNodeHlight;
    trav.data = hl;
    
    pfuTraverse(node, &trav);
}

/****************************************************
 * Traversal and callback for setting an attribute 
 * in all pfGeoSets to the have the specified binding
 ****************************************************/

static int 
cbNodeAttrBind(pfuTraverser *trav)
{
    pfNode *node = trav->node;
    pfHighlight *hl = (pfHighlight *)trav->data;
    int nGSets, i;
    unsigned int attr = *((unsigned int*)trav->data);
    unsigned int bind = *(((unsigned int*)trav->data)+1);
    void *alist;
    unsigned short *ilist;
    
    if (node == NULL) 
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbNodeHlight: node null!");
    else if (pfIsOfType(node, pfGetGeodeClassType()))
    {
	nGSets = pfGetNumGSets((pfGeode *)node);
	for (i = 0 ; i < nGSets ; i++)
	{
	    pfGeoSet *gd = pfGetGSet((pfGeode *)node, i);
	    pfGetGSetAttrLists(gd, attr, &alist, &ilist);
	    pfGSetAttr(gd, attr, bind, alist, ilist);
	}
    }
    return PFTRAV_CONT;
}

PFUDLLEXPORT void
pfuTravNodeAttrBind(pfNode *node, unsigned int attr, unsigned int bind)
{
    unsigned int data[2];
    pfuTraverser trav;
    pfuInitTraverser(&trav);
    
    trav.preFunc = cbNodeAttrBind;
    data[0] = attr;
    data[1] = bind;
    trav.data = &data;
    
    pfuTraverse(node, &trav);
}

/*******************************************************************
 * Traversals and callbacks for using cull callbacks for re-using 
 * cull results with static viewpoints.
 * WARNING: removes any existing pre or post-cull callbacks
 * WARNING: requires special matching cached-cull channel data
 ********************************************************************/

static int 
cbNodeCachedCullPreCull(pfTraverser *trav, void *data)
{
    pfChannel *chan = pfGetTravChan(trav);
    pfuChanCullCache  *chancc = 
	(pfuChanCullCache *) pfGetChanData(chan);
    pfNode *n = pfGetTravNode(trav);
    pfuNodeCullCache  *nodecc;
    
    nodecc = ((pfuNodeCullCache *) pfGetUserData(n)) + chancc->chanId;
    
    if (nodecc->updateFrame == chancc->updateFrame)
    {  /* Cached cull result is valid */
	pfCullResult(nodecc->cullResult);
    }
    
    return PFTRAV_CONT;
}


static int 
cbNodeCachedCullPostCull(pfTraverser *trav, void *data)
{
    pfChannel	   *chan = pfGetTravChan(trav);
    pfuChanCullCache  *chancc = (pfuChanCullCache *) pfGetChanData(chan);
    pfuNodeCullCache  *nodecc = 
	((pfuNodeCullCache *) pfGetUserData(pfGetTravNode(trav))) + 
	    chancc->chanId;
    
    if (nodecc->updateFrame != chancc->updateFrame)
    {  /* Need to update node cull result for this channel */
	nodecc->cullResult = pfGetCullResult();
	nodecc->updateFrame = chancc->updateFrame;
    }
    
    return PFTRAV_CONT;
}

static int
cbInitCachedCull(
		 pfuTraverser *trav)
{
    pfNode     *pfnThis = trav->node;
    int	numChans = (int) trav->data;
    int		i;
    
    pfuNodeCullCache *nodecc = 
	pfMalloc(numChans * sizeof(pfuNodeCullCache), pfGetSharedArena());
    
    if (nodecc)
    {
	pfNodeTravFuncs(pfnThis, PFTRAV_CULL,
			cbNodeCachedCullPreCull, cbNodeCachedCullPostCull);
	pfUserData(pfnThis, (void *) nodecc);
	
	/* Init each chan result to a stale value */
	for (i = 0; i < numChans; i++)
	    nodecc[i].updateFrame = 0;
	
    }
    else
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, "Couldn't allocate node data");
    }
    
    return PFTRAV_CONT;
}

PFUDLLEXPORT void	
pfuTravCachedCull(pfNode* node, int numChans)
{  /* Install pre- and post- cull callbacks, node data */
    pfuTraverser trav;
    
    pfuInitTraverser(&trav);
    trav.preFunc = cbInitCachedCull;
    trav.data = (void*) numChans;
    pfuTraverse(node, &trav);
}

/*******************************************************************
 * Traversals and callbacks for installing and removing pre-draw 
 * callbacks (pfNodeFuncs) for generating GL Prof tags during drawing
 * WARNING: removes any existing pre or post-draw callbacks
 ********************************************************************/

/*
 * glprof pre-draw traversal callback  issues glprof_object calls
 */
static int 
cbGLProf(pfTraverser *trav, void *data)
{
    const pfNode	*node = pfGetTravNode(trav);
    const char		*nn;
    static char		name[80];
    
    if (node == NULL) 
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbGLProf: node null!");
    else if (nn = pfGetNodeName(node))
    { 
	/* if it exists, use the node name for the tag */
	strncpy(name, nn, 79);
    } 
    else 
    { 
	/* otherwise, use the node type string for the tag */
	strncpy(name, pfGetTypeName((pfObject *)node), 79);
    }
    glprof_object(name);
    
    return 0;
}

/*
 * traversal callback for placing glprof callbacks as pre-draw
 * callbacks on nodes
 */
static int 
cbPutGLProfTag(pfuTraverser *trav)
{
    pfNode *node = trav->node;
    
    if (node == NULL) 
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbPutGLProfTag: node null!");
    else if (pfIsOfType(node, pfGetGeodeClassType()))
	pfNodeTravFuncs(node, PFTRAV_DRAW, cbGLProf, 
			(int (*) (pfTraverser *, void *))NULL);
    
    return PFTRAV_CONT;
}
/*
 * Traversal to remove the pre-draw glprof tag callbacks
 */
static int 
cbRmGLProfTag(pfuTraverser *trav)
{
    pfNode *node = trav->node;
    
    if (node == NULL) 
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbGLProf: node null!");
    else 
    if (pfIsOfType(node, pfGetGeodeClassType()) ||
	pfIsExactType(node, pfGetGroupClassType()))
	pfNodeTravFuncs(node, PFTRAV_DRAW, 
	    (int (*) (pfTraverser *, void *))NULL, 
	    (int (*) (pfTraverser *, void *))NULL);
    return PFTRAV_CONT;
}

/* glprof object tag traversal */
PFUDLLEXPORT void 
pfuTravGLProf(pfNode *node, int mode)
{
    pfuTraverser trav;
    pfuInitTraverser(&trav);
    
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
	     "doing travGLProf: mode = %d", mode);
    
    if (mode)	/* place gprof tag callbacks */
	trav.preFunc = cbPutGLProfTag;
    else	/* remove the callbacks */
	trav.preFunc = cbRmGLProfTag;
    
    pfuTraverse(node, &trav);
}


/************************************************************************
 * Traversal and callback for computing the maximum depth of a subtree
 ***********************************************************************/

typedef struct
{
    int depth;
    int maxDepth;
    
} Depther;

/*--------------------------------------------------------------*/

static int
cbPreDepth(pfuTraverser *trav)
{
    Depther* dp = (Depther*)trav->data;
    
    dp->depth++;
    if (dp->depth > dp->maxDepth)
	dp->maxDepth = dp->depth;
    return PFTRAV_CONT;
}

static int
cbPostDepth(pfuTraverser *trav)
{
    Depther* dp = (Depther*)trav->data;
    
    dp->depth--;
    return PFTRAV_CONT;
}

PFUDLLEXPORT int 
pfuCalcDepth(pfNode *node)
{
    Depther 		dp;
    pfuTraverser 	trav;
    pfuInitTraverser(&trav);
    
    dp.depth = 0;
    dp.maxDepth = 0;
    
    trav.preFunc = cbPreDepth;
    trav.postFunc = cbPostDepth;
    trav.data = &dp;
    
    pfuTraverse(node, &trav);
    
    return dp.maxDepth;
}


/************************************************************************
 * Traversal and callback for computing a bounding box around pfGeoSets
 ***********************************************************************/

static int
cbPreBbox(pfuTraverser *trav)
{
    pfBox *box = (pfBox *)trav->data;
    int i;
    if (pfIsOfType(trav->node, pfGetASDClassType()))
    {
	pfGetASDBBox((pfASD *)trav->node, box);
    }
    else if (!pfIsOfType(trav->node, pfGetBboardClassType()) &&
	(pfIsOfType(trav->node, pfGetGeodeClassType())))
    {
	int ngsets, depth;
	pfMatrix *xform = NULL;
	pfGeode *geode = (pfGeode *)trav->node;
	depth = pfGetMStackDepth(trav->mstack);
	if (depth > 1)
	    xform = pfGetMStackTop(trav->mstack);
	
	ngsets = pfGetNumGSets(geode);
	for (i = 0 ; i < ngsets ; i++)
	{
	    pfBox gsetbox;
	    pfGeoSet *gset = pfGetGSet(geode, i);
	    pfGetGSetBBox(gset, &gsetbox);
	    if (xform)
		pfXformBox(&gsetbox, &gsetbox, *xform);
	    pfBoxExtendByBox(box, &gsetbox);
	}
    }
    else if (pfIsOfType(trav->node, pfGetBboardClassType()))
 	/* this applies for billboards that rotate about Z-axis */
    {
        int ngsets, depth;
        pfMatrix *xform = NULL;
        pfBillboard *bboard = (pfBillboard *)trav->node;
        pfVec3  pos;

        depth = pfGetMStackDepth(trav->mstack);
        if (depth > 1)
            xform = pfGetMStackTop(trav->mstack);
        
        ngsets = pfGetNumGSets(bboard);

	if (pfIsOfType(trav->node, pfGetIBRnodeClassType()) &&
	    pfGetIBRnodeFlags((pfIBRnode *)trav->node, PFIBRN_USE_PROXY))
	{
	    for (i = 0 ; i < ngsets ; i++)
	    {
	        pfBox gsetbox;
		pfGeoSet *gset = pfGetGSet(bboard, i);
		pfGetGSetBBox(gset, &gsetbox);
		pfGetBboardPos (bboard, i, pos);
		
		if (xform)
		  pfXformBox(&gsetbox, &gsetbox, *xform);
		pfBoxExtendByBox(box, &gsetbox);
	    }

	}
	else
	    for (i = 0 ; i < ngsets ; i++)
	    {
	        pfBox gsetbox;
		pfGeoSet *gset = pfGetGSet(bboard, i);
		pfGetGSetBBox(gset, &gsetbox);
		gsetbox.min[1] = gsetbox.min[0];
		gsetbox.max[1] = gsetbox.max[0];
		pfGetBboardPos (bboard, i, pos);
		pfAddVec3 (gsetbox.min, gsetbox.min, pos);
		pfAddVec3 (gsetbox.max, gsetbox.max, pos);
		
		if (xform)
		  pfXformBox(&gsetbox, &gsetbox, *xform);
		pfBoxExtendByBox(box, &gsetbox);
	    }
    }
    return PFTRAV_CONT;
}

PFUDLLEXPORT void
pfuTravCalcBBox(pfNode *node, pfBox *box)
{
    pfuTraverser trav;
    
    pfuInitTraverser(&trav);
    pfMakeEmptyBox(box);
    
    trav.preFunc = cbPreBbox;
    trav.data = box;
    trav.mstack = pfNewMStack(32, pfGetSharedArena());
    
    pfuTraverse(node, &trav);
    
    pfDelete(trav.mstack);
}


/************************************************************************
 * Traversal and callback for computing a the number of vertices in pfGeoSets
 ***********************************************************************/

static int
cbPreNumVerts(pfuTraverser *trav)
{
    int i;
    int *numVerts = (int *)trav->data;
    
    if (pfIsOfType(trav->node, pfGetGeodeClassType()))
    {
	int ngsets;
	pfGeode *geode = (pfGeode *)trav->node;
	ngsets = pfGetNumGSets(geode);
	for (i = 0 ; i < ngsets ; i++)
	{
	    int j;
	    pfGeoSet *gset = pfGetGSet(geode, i);
	    int pType = pfGetGSetPrimType(gset);
	    int numPrims = pfGetGSetNumPrims(gset);
	    
	    switch (pType)
	    {
	    case PFGS_POINTS:
		*numVerts += numPrims;
		break;
	    case PFGS_LINES:
		*numVerts += 2 * numPrims;
		break;
	    case PFGS_TRIS:
		*numVerts += 3 * numPrims;
		break;
	    case PFGS_QUADS:
		*numVerts += 4 * numPrims;
		break;
	    case PFGS_LINESTRIPS:
	    case PFGS_TRISTRIPS:
	    case PFGS_FLAT_LINESTRIPS:
	    case PFGS_FLAT_TRISTRIPS:
	    case PFGS_POLYS:
	    case PFGS_TRIFANS:
	    case PFGS_FLAT_TRIFANS:
		for (j = 0 ; j < numPrims ; j++)
		    *numVerts += pfGetGSetPrimLength(gset, j);
		break;
	    }
	}
    }
    return PFTRAV_CONT;
}

PFUDLLEXPORT int
pfuTravCountNumVerts(pfNode *node)
{
    int 		numVerts = 0;
    pfuTraverser 	trav;
    
    pfuInitTraverser(&trav);
    
    trav.preFunc = cbPreNumVerts;
    trav.data = &numVerts;
    
    pfuTraverse(node, &trav);
    
    return numVerts;
}

/************************************************************************
 * Traversal and callback for creating vertex arrays for pfGeoSets
 ***********************************************************************/

#define MAX_SHORT 32767

static  int
calcVAStride(int mask)
{
    int stride = 0;
    
    /* compute vertex size in shorts */
    if (mask & PFU_ATTRS_COLORS)
	stride += 4;
    if (mask & PFU_ATTRS_NORMALS)
	    stride += 8;
    if (mask & PFU_ATTRS_TEXCOORDS)
	stride += 8;
    if (mask & PFU_ATTRS_COORDS)
	stride += 12;

    return stride;
}


static  int
calcVABind(pfGeoSet *gset, int format)
{
    int mask = 0;
    int cbind = pfGetGSetAttrBind(gset, PFGS_COLOR4);
    int nbind = pfGetGSetAttrBind(gset, PFGS_NORMAL3);
    int numPrims = pfGetGSetNumPrims(gset);
    int maxVerts, numVerts, length, i;
    int primType;

    pfQueryGSet(gset, PFQGSET_NUM_VERTS, &numVerts);
    if (numVerts < 6)
	return 0;

    primType = pfGetGSetPrimType(gset);
    
    /* we can do per-prim attributes only for stripped primitives */
    if ((cbind == PFGS_PER_PRIM) || (nbind == PFGS_PER_PRIM))
    {
	if ((primType != PFGS_TRISTRIPS) && (primType != PFGS_TRIFANS) &&
	    (primType != PFGS_LINESTRIPS) && (primType != PFGS_FLAT_LINESTRIPS) &&
	    (primType != PFGS_FLAT_TRISTRIPS)) 
	{
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		"pfuFillGSetPackedAttrs - non-stripped gset with per-prim attrs");
	    return 0;
	}
    }
#if 1
    if (nbind == PFGS_OVERALL) /* InfiniteReality bug GL Work-Around for 1st release */
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
	    "pfuFillGSetPackedAttrs - skipping gset with overall normal"
	    " for InfiniteReality bug Work-Around.");
	return 0;
    }
#endif
    maxVerts = numVerts;
    switch (pfGetGSetPrimType(gset))
    {
    case PFGS_LINESTRIPS:
    case PFGS_TRISTRIPS:
    case PFGS_POLYS:
	for (i=0; i < numPrims; i++)
	{
	    length = pfGetGSetPrimLength(gset, i);
	    if (length > maxVerts)
		maxVerts = length;
	}
	break;
    }
    if (maxVerts < 6)
    {
	return 0;
    }
    
    if (cbind == PFGS_PER_VERTEX)
	mask |= PFU_ATTRS_COLORS;
    if ((nbind == PFGS_PER_VERTEX) &&
	    ((format == PFGS_PA_C4UBN3ST2FV3F) || 
	    (format == PFGS_PA_C4UBN3ST2F)))
	mask |= PFU_ATTRS_NORMALS;
    if (pfGetGSetAttrBind(gset, PFGS_TEXCOORD2) == PFGS_PER_VERTEX)
	mask |= PFU_ATTRS_TEXCOORDS;
    if (format == PFGS_PA_C4UBN3ST2FV3F)
	mask |= PFU_ATTRS_COORDS;
    
    return mask;
}


PFUDLLEXPORT int
pfuFillGSetPackedAttrs(pfGeoSet *gset, int mask)
{
    int numPrims = pfGetGSetNumPrims(gset);
    int pType = pfGetGSetPrimType(gset);
    int *lengths = pfGetGSetPrimLengths(gset), length;
    int numVerts;
    ushort *coordsi=0, *normsi=0, *colorsi=0, *texcoordsi=0, *vai=0;
    pfVec3 *coords=0, *norms=0;
    pfVec4 *colors=0;
    pfVec2 *texcoords=0;
    char *va;
    int loop, flat=0, i, j;
    int attrStride=0;
    

    pfQueryGSet(gset, PFQGSET_NUM_VERTS, &numVerts);
    if (numVerts < 6)
    {
	return 0;
    }

    pfGetGSetAttrLists(gset, PFGS_PACKED_ATTRS, (void**)&va, &vai);
    if (mask & PFU_ATTRS_COLORS)
	pfGetGSetAttrLists(gset, PFGS_COLOR4, (void**)&colors, &colorsi);
    if (mask & PFU_ATTRS_NORMALS)
	pfGetGSetAttrLists(gset, PFGS_NORMAL3, (void**)&norms, &normsi);
    if (mask & PFU_ATTRS_TEXCOORDS)
	pfGetGSetAttrLists(gset, PFGS_TEXCOORD2, (void**)&texcoords, &texcoordsi);
    if (mask & PFU_ATTRS_COORDS)
	pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&coords, &coordsi);
    
    /* set up basic vertex loop control */
    switch (pType)
    {
    case PFGS_POINTS:
    case PFGS_LINES:
    case PFGS_TRIS:
    case PFGS_QUADS:
	loop = 1;
	break;
    case PFGS_FLAT_LINESTRIPS:
    case PFGS_FLAT_TRISTRIPS:
    case PFGS_FLAT_TRIFANS:
	flat = 1;
	attrStride = calcVAStride(mask & PFU_ATTRS_CN);
	/* no break here - fall through for normal strip stuff */
    case PFGS_LINESTRIPS:
    case PFGS_TRISTRIPS:
    case PFGS_POLYS:
    case PFGS_TRIFANS:
	loop = numPrims; 
	numVerts = pfGetGSetPrimLength(gset, 0);
	break;
    }

    for (j=1; loop--; j++)
    {
	for (i=0; i < numVerts; i++)
	{
	    if (flat && (i < 2))
	    {
		va += attrStride;
	    }
	    else
	    {
		if (mask & PFU_ATTRS_COLORS)
		{
		    pfVec4 *c;
		    if (colorsi)
		    {
			c = &colors[*colorsi];
			colorsi++;
		    }
		    else
		    {
			c = colors;
			colors++;
		    }
		    *((uint*)va) = PFU_PACK_COLOR4(*c);
		    va += 4;
		}
		if (mask & PFU_ATTRS_NORMALS)
		{
		    pfVec3 *n;
		    if (normsi)
		    {
			n = &norms[*normsi];
			normsi++;
		    }
		    else
		    {
			n = norms;
			norms++;
		    }
		    ((short*)va)[0] = (short) (MAX_SHORT * ((*n)[0]));
		    ((short*)va)[1] = (short) (MAX_SHORT * ((*n)[1]));
		    ((short*)va)[2] = (short) (MAX_SHORT * ((*n)[2]));
		    va += 8;
		}
	    }
	    if (mask & PFU_ATTRS_TEXCOORDS)
	    {
		pfVec2 *t;
		if (texcoordsi)
		{
		    t = &texcoords[*texcoordsi];
		    texcoordsi++;
		}
		else
		{
		    t = texcoords;
		    texcoords++;
		}
		((float*)va)[0] = (*t)[0];
		((float*)va)[1] = (*t)[1];
		va += 8;
	    }
	    if (mask & PFU_ATTRS_COORDS)
	    {
		pfVec3 *v;
		if (coordsi)
		{
		    v = &coords[*coordsi];
		    coordsi++;
		}
		else
		{
		    v = coords;
		    coords++;
		}
		((float*)va)[0] = (*v)[0];
		((float*)va)[1] = (*v)[1];
		((float*)va)[2] = (*v)[2];
		va += 12;
	    }
	}
	if (loop && lengths) /* have strip primitive */
	{
	    numVerts = pfGetGSetPrimLength(gset, j);
	}
    }
    return 1;
}

PFUDLLEXPORT void
pfuDelGSetAttrs(pfGeoSet *gset, int delMask)
{
    void *alist, *ilist;
    if (delMask & PFU_ATTRS_COORDS)
    {
	pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&alist, (ushort**)&ilist);
	pfDelete(alist);
	pfDelete(ilist);
	pfGetGSetAttrLists(gset, PFGS_COORD3, NULL, NULL);
    }
    if (delMask & PFU_ATTRS_TEXCOORDS)
    {
	pfGetGSetAttrLists(gset, PFGS_TEXCOORD2, (void**)&alist, (ushort**)&ilist);
	pfDelete(alist);
	pfDelete(ilist);
	pfGetGSetAttrLists(gset, PFGS_TEXCOORD2, NULL, NULL);
    }
    if (delMask & PFU_ATTRS_NORMALS)
    {
	pfGetGSetAttrLists(gset, PFGS_NORMAL3, (void**)&alist, (ushort**)&ilist);
	pfDelete(alist);
	pfDelete(ilist);
	pfGetGSetAttrLists(gset, PFGS_NORMAL3, NULL, NULL);
    }
    if (delMask & PFU_ATTRS_COLORS)
    {
	pfGetGSetAttrLists(gset, PFGS_COLOR4, (void**)&alist, (ushort**)&ilist);
	pfDelete(alist);
	pfDelete(ilist);
	pfGetGSetAttrLists(gset, PFGS_COLOR4, NULL, NULL);
    }
}

static int
cbPreCreatePackedAttrs(pfuTraverser *trav)
{
    int *vaData = (int*) trav->data;
    void *arena;
    char *va;
    int stride;
    int vaBind, format, delMask;
    int i;
    
    format = vaData[0];
    delMask = vaData[1];

    
    if (pfIsOfType(trav->node, pfGetGeodeClassType()))
    {
	int ngsets;
	pfGeode *geode = (pfGeode *)trav->node;
	ngsets = pfGetNumGSets(geode);
	arena = pfGetSharedArena();
	for (i = 0 ; i < ngsets ; i++)
	{
	    pfGeoSet *gset = pfGetGSet(geode, i);
	    pfGeoState *gstate;
	    int numVerts;

	    /* see if gset is already a vert array! */
	    if (pfGetGSetDrawMode(gset, PFGS_PACKED_ATTRS))
		return PFTRAV_CONT;
	    else
	    {
		void *alist;
		ushort *ilist;
		pfGetGSetAttrLists(gset, PFGS_PACKED_ATTRS, &alist, &ilist);
		if (alist)
		    return PFTRAV_CONT;
	    }
	    
	    /* don't make vertex arrays for light-points */
	    gstate = pfGetGSetGState(gset);
	    if (gstate && pfGetGStateAttr(gstate, PFSTATE_LPOINTSTATE))
	    {
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		"cbPreCreatePackedAttrs - skipping light-point gset.");
		return PFTRAV_CONT;
	    }
	    
	    /* determine vert array binding mask and vertex stride */
	    vaBind = calcVABind(gset, format);
	    if (vaBind == 0)
	    {
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		"cbPreCreatePackedAttrs - skipping gset with problem vertex attribute binding.");
		return PFTRAV_CONT;
	    }
	    stride = calcVAStride(vaBind);

	    pfQueryGSet(gset, PFQGSET_NUM_VERTS, &numVerts);
	    
	    /* allocate an array for the packed verts */
	    va = (char *) pfMalloc(stride * numVerts * sizeof(char), arena);
	    /* place array on the pfGeoSet */
	    pfGSetAttr(gset, PFGS_PACKED_ATTRS, format, va, NULL);
	    
	    /* copy vertex data into packed array */
	    if (pfuFillGSetPackedAttrs(gset, vaBind))
	    {
		pfGSetDrawMode(gset, PFGS_PACKED_ATTRS, 1);
	    
		/* auto pfDelete attrs no longer needed */
		if (delMask)
		    pfuDelGSetAttrs(gset, delMask);
	    }
	    else
	    {
		pfGSetAttr(gset, PFGS_PACKED_ATTRS, 0, NULL, NULL);
		pfDelete(va);
		va = NULL;
	    }
	}
    }
    return PFTRAV_CONT;
}

PFUDLLEXPORT void 
pfuTravCreatePackedAttrs(pfNode *_node, int format, int delMask)
{
    pfuTraverser 	trav;
    int			data[2];

    /* make sure this is a format we can handle */
    if (format > PFGS_PA_C4UBN3ST2F)
    {
       pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
       "pfuTravCreatePackedAttrs - sorry, format %d is not implemented.", format);
       return;
    }
    
    pfuInitTraverser(&trav);
    
    trav.preFunc = cbPreCreatePackedAttrs;
    data[0] = format;
    data[1] = delMask;
    trav.data = (void*) &data;
    
    pfuTraverse(_node, &trav);
}

/************************************************************************
 * Traversal and callback for turning quad-tmeshes into quads
 ***********************************************************************/

PFUDLLEXPORT void
pfuMeshToTri(pfGeoSet *gset)
{
    int bind, prim, num, numT, *len;
    pfVec3 *verts, *overts, *ovp, *vp, *v0, *v1;
    pfVec3 *norms=NULL, *onorms, *onp, *np, *n0, *n1;
    pfVec4 *clrs=NULL, *oclrs, *ocp, *cp, *c0, *c1;
    pfVec2 *tex=NULL, *otex, *otp, *tp, *t0, *t1;
    ushort *index;

    if (!gset)
	return;

    /* do quick check for valid prim, lengths array and non-indexed */
    prim = pfGetGSetPrimType(gset);
    if (!((prim == PFGS_TRISTRIPS) || (prim == PFGS_FLAT_TRISTRIPS)))
	return;
    pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&overts, &index);
    if (index)
	return;
    num = pfGetGSetNumPrims(gset);
    pfQueryGSet(gset,  PFQGSET_NUM_TRIS, &numT);
    
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfuMeshToTri !!!!");
}

PFUDLLEXPORT void
pfuMeshToQuad(pfGeoSet *gset)
{
    int bind, prim, num, numT, numQ, len;
    pfVec3 *verts, *overts, *ovp, *vp, *v0, *v1;
    pfVec3 *norms=NULL, *onorms, *onp, *np, *n0, *n1;
    pfVec4 *clrs=NULL, *oclrs, *ocp, *cp, *c0, *c1;
    pfVec2 *tex=NULL, *otex, *otp, *tp, *t0, *t1;
    ushort *index;
    int i, j;

    if (!gset)
	return;

    /* do quick check for valid prim, lengths array and non-indexed */
    prim = pfGetGSetPrimType(gset);
    if (!((prim == PFGS_TRISTRIPS) || (prim == PFGS_FLAT_TRISTRIPS)))
	return;
    pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&overts, &index);
    if (index)
	return;
    num = pfGetGSetNumPrims(gset);
    if (num & 0x1)
	return;
    if (!pfGetGSetPrimLengths(gset))
	return;

    for (i=0; i < num; i++)
    {
	if (pfGetGSetPrimLength(gset, i) & 0x1)
	    return;
    }
    pfQueryGSet(gset,  PFQGSET_NUM_TRIS, &numT);
    numQ = (numT + 1) >> 1;
   
    pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&ovp, &index);
    vp = verts = pfMalloc(sizeof(pfVec3)*4*numQ, pfGetArena(gset));
    bind = pfGetGSetAttrBind(gset, PFGS_NORMAL3);
    if (bind == PFGS_PER_VERTEX)
    {
	pfGetGSetAttrLists(gset, PFGS_NORMAL3, (void**)&onorms, &index);
	onp = onorms;
	np = norms = pfMalloc(sizeof(pfVec3)*4*numQ, pfGetArena(gset));
    }
    bind = pfGetGSetAttrBind(gset, PFGS_COLOR4);
    if (bind == PFGS_PER_VERTEX)
    {
	pfGetGSetAttrLists(gset, PFGS_COLOR4, (void**)&oclrs, &index);
	ocp = oclrs;
	cp = clrs = pfMalloc(sizeof(pfVec4)*4*numQ, pfGetArena(gset));
    }
    bind = pfGetGSetAttrBind(gset, PFGS_TEXCOORD2);
    if (bind == PFGS_PER_VERTEX)
    {
	pfGetGSetAttrLists(gset, PFGS_TEXCOORD2, (void**)&otex, &index);
	otp = otex;
	tp = tex = pfMalloc(sizeof(pfVec2)*4*numQ, pfGetArena(gset));
    }

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfuMeshToQuad !!!!");
    for (i=0; i < num; i++);
    {
	int nv;
	nv = pfGetGSetPrimLength(gset, i);
	v0 = ovp;
	v1 = ovp + 1;
	ovp += 2;
	if (prim == PFGS_FLAT_LINESTRIPS)
	{
	    if (norms)
	    {
		n0 = n1 = onp;
	    }
	    if (clrs)
	    {
		c0 = c1 = ocp;
	    }
	    if (tex)
	    {
		t0 = t1 = otp;
	    }
	}
	else
	{
	    if (norms)
	    {
		n0 = onp;
		n1 = onp + 1;
		onp += 2;
	    }
	    if (clrs)
	    {
		c0 = ocp;
		c1 = ocp + 1;
		ocp += 2;
	    }
	}
	if (tex)
	{
	    t0 = otp;
	    t1 = otp + 1;
	    otp += 2;
	}
	for (j=0; j < nv; j+=2);
	{
	    PFCOPY_VEC3(vp[0], *v0);
	    PFCOPY_VEC3(vp[1], *v1);
	    PFCOPY_VEC3(vp[2], ovp[1]);
	    PFCOPY_VEC3(vp[3], ovp[0]);
	    v0 = ovp;
	    v1 = ovp + 1;
	    ovp += 2;
	    vp += 4;
	    if (norms)
	    {
		PFCOPY_VEC3(np[0], *n0);
		PFCOPY_VEC3(np[1], *n1);
		PFCOPY_VEC3(np[2], onp[1]);
		PFCOPY_VEC3(np[3], onp[0]);
		n0 = onp;
		n1 = onp+1;
		onp += 2;
		np += 4;
	    }
	    if (clrs)
	    {
		PFCOPY_VEC4(cp[0], *c0);
		PFCOPY_VEC4(cp[1], *c1);
		PFCOPY_VEC4(cp[2], ocp[1]);
		PFCOPY_VEC4(cp[3], ocp[0]);
		c0 = ocp;
		c1 = ocp+1;
		ocp += 2;
		cp += 4;
	    }
	    if (tex)
	    {
		PFCOPY_VEC2(tp[0], *t0);
		PFCOPY_VEC2(tp[1], *t1);
		PFCOPY_VEC2(tp[2], otp[1]);
		PFCOPY_VEC2(tp[3], otp[0]);
		t0 = otp;
		t1 = otp+1;
		otp += 2;
		tp += 4;
	    }
	}
    }
    pfGSetNumPrims(gset,numQ);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, verts, NULL);
    pfDelete(overts);
    if (norms)
    {
	pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
	pfDelete(onorms);
    }
    if (clrs)
    {
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, clrs, NULL);
	pfDelete(oclrs);
    }
    if (tex)
    {
	pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, tex, NULL);
	pfDelete(otex);
    }
}

static int 
cbMeshToIndep(pfuTraverser *trav)
{
    pfNode *node = trav->node;
	
    if (node == NULL) 
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbMeshToQuad: node null!");
    else 
    if (pfIsOfType(node, pfGetGeodeClassType()))
    {
	pfGeode *geode = (pfGeode *)node;
        int numGSets = pfGetNumGSets(geode);
	int prim, num, len;
	pfGeoSet *gset;
	int g, i, n;

	for (g=0; g<numGSets; g++)
	{
	    gset = pfGetGSet(geode, g);
	    /* turn gsets that are all meshes of length 2 into quads */
	    prim = pfGetGSetPrimType(gset);
	    num = pfGetGSetNumPrims(gset);
	    n = pfGetGSetPrimLength(gset, 0);
	    if ((n != 3) && (n != 4))
		continue;
	    if (prim == PFGS_TRISTRIPS || prim == PFGS_FLAT_TRISTRIPS)
	    {
		for (i=1; i < num; i++)
		{
		    len = pfGetGSetPrimLength(gset, i);
		    if (len != n)
		    {
			 i = -1;
			 break;
		    }
		}
		if (i == num)
		{
		    if (n == 4)
			pfuMeshToQuad(gset);
		    else if (n == 3)
			pfuMeshToTri(gset);
		}
	    }
	}
    }

    return PFTRAV_CONT;
}

PFUDLLEXPORT void 
pfuTravMeshToIndep(pfNode *node)
{
    pfuTraverser    trav;
    
    pfuInitTraverser(&trav);
    
    trav.preFunc = cbMeshToIndep;
    trav.data = 0;
    
    pfuTraverse(node, &trav);
}

/************************************************************************
 * Traversal and callback for setting display-list geoset status
 ***********************************************************************/

static int 
cbSetDListMode(pfuTraverser *trav)
{
    pfNode *node = trav->node;
    int enable = ((int)(trav->data));
    pfGeoState *gs;
    void *lpstate;
    int numVerts;
    
    if (node == NULL) 
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbSetDListMode: node null!");
    else 
    if (pfIsOfType(node, pfGetGeodeClassType()))
    {
	pfGeode *geode = (pfGeode *)node;
        int n = pfGetNumGSets(geode);
	pfGeoSet *gset;
	int i;

        for (i=0; i<n; i++)
	{
	    gset = pfGetGSet(geode, i);

	    if (!gset)
		continue;

	    pfQueryGSet(gset, PFQGSET_NUM_VERTS, &numVerts);
	    if (numVerts < 6)
	    {
		continue;
	    }

	    /* do not compile gset with a lpstate */
	    gs = pfGetGSetGState(gset);
     	    if (gs)
		lpstate = pfGetGStateAttr(gs,PFSTATE_LPOINTSTATE);
 	    else
		lpstate = NULL;

	    if (lpstate == NULL)
	        pfGSetDrawMode(gset, PFGS_COMPILE_GL, enable);
	    
	}
    }

    return PFTRAV_CONT;
}

PFUDLLEXPORT void 
pfuTravSetDListMode(pfNode *node, int enable)
{
    pfuTraverser    trav;
    
    pfuInitTraverser(&trav);
    trav.preFunc = cbSetDListMode;
    trav.data = (void*)enable;
    
    pfuTraverse(node, &trav);
}

static int
cbCompileDLists(pfuTraverser *trav)
{
    pfNode *node = trav->node;
    int delMask = ((int)(trav->data));
    pfGeoState *gs;
    void *lpstate;
    int numVerts;

    if (node == NULL)
        pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbSetDListMode: node null!");
    else
    if (pfIsOfType(node, pfGetGeodeClassType()))
    {
        pfGeode *geode = (pfGeode *)node;
        int n = pfGetNumGSets(geode);
        pfGeoSet *gset;
        int i;

        for (i=0; i<n; i++)
        {
            gset = pfGetGSet(geode, i);
            if (gset)
	    {
		pfQueryGSet(gset, PFQGSET_NUM_VERTS, &numVerts);
		if (numVerts < 6)
		{
		    continue;
		}
		/* do not compile gset with a lpstate */
		gs = pfGetGSetGState(gset);
		if (gs)
			lpstate = pfGetGStateAttr(gs,PFSTATE_LPOINTSTATE);
		else
			lpstate = NULL;

		if (lpstate == NULL)
		{
			pfCompileGSet(gset);

			/* auto pfDelete attrs no longer needed */
			if (delMask)
		    		pfuDelGSetAttrs(gset, delMask);
		}
	    }
        }
    }

    return PFTRAV_CONT;
}


PFUDLLEXPORT void
pfuTravCompileDLists(pfNode *node, int delMask)
{
    pfuTraverser    trav;
    
    pfuInitTraverser(&trav);
    trav.data = (void*)delMask;
    trav.preFunc = cbCompileDLists;
    pfuTraverse(node, &trav);
}


static int
cbDrawDLists(pfuTraverser *trav)
{
    pfNode *node = trav->node;

    if (node == NULL)
        pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "cbSetDListMode: node null!");
    else
    if (pfIsOfType(node, pfGetGeodeClassType()))
    {
        pfGeode *geode = (pfGeode *)node;
        int n = pfGetNumGSets(geode);
        pfGeoSet *gset;
        int i;

        for (i=0; i<n; i++)
        {
            gset = pfGetGSet(geode, i);
            if (gset && (pfGetGSetDrawMode(gset, PFGS_DRAW_GLOBJ)))
	    {
		pfDrawGSet(gset);
	    }
        }
    }

    return PFTRAV_CONT;
}


PFUDLLEXPORT void
pfuTravDrawDLists(pfNode *node)
{
    pfuTraverser    trav;
    
    pfuInitTraverser(&trav);
    trav.preFunc = cbDrawDLists;
    pfuTraverse(node, &trav);
}


/************************************************************************
 * Traversal and callback for computing the least common ancestor
 * of all nodes in a scene for which a given function returns true.
 ***********************************************************************/

struct LCAinfo
{
    int (*fun)(pfNode*,void*);  /* the testing function */
    void *arg;		  /* closure arg for the testing function */
    pfNode *lca;	  /* output, always best approx. so far */
    int possibleLCAdepth; /*depth of least common ancestor of lca with current*/
    int possibleLCAIsIt;  /* set when lca!=NULL and another node returns true */
};

static int
cbPostLCA(pfuTraverser *trav)
{
    struct LCAinfo *info = (struct LCAinfo *)trav->data;
    if (info->possibleLCAIsIt && info->possibleLCAdepth == trav->depth)
    {
	info->lca = trav->node;
	info->possibleLCAIsIt = 0;
    }
    if (info->lca != NULL)
	info->possibleLCAdepth = PF_MIN2(info->possibleLCAdepth, trav->depth-1);

    return PFTRAV_CONT;
}

static int
cbPreLCA(pfuTraverser *trav)
{
    struct LCAinfo *info = (struct LCAinfo *)trav->data;
    if (info->possibleLCAIsIt)
    {
	/*
	 * possibleLCAIsIt is a message
	 * sent to the post-function of one of my ancestors
	 * from one of that ancestor's other descendents
	 * for which fun() returned TRUE...
	 * if that message is in the process of being sent,
	 * then I am irrelevant.
	 */
	return PFTRAV_PRUNE;
    }
    if (info->fun(trav->node, info->arg))
    {
	if (info->lca == NULL)
	    info->possibleLCAdepth = trav->depth;
	info->possibleLCAIsIt = 1;
	/* pruning keeps the post callback from being called, so force it */
	(void)cbPostLCA(trav);
	return PFTRAV_PRUNE; /* my children are irrelevant */
    }
    return PFTRAV_CONT;
}

extern PFUDLLEXPORT pfNode *
pfuLowestCommonAncestor(pfNode *scene,
			int (*fun)(pfNode*,void*), void *arg,
			int mode)
{
    struct LCAinfo info;
    pfuTraverser trav;

    info.fun = fun;
    info.arg = arg;
    info.lca = NULL;
    info.possibleLCAIsIt = 0;
    /* possibleLCAdepth is irrelevant until some fun(node,arg) returns true */

    pfuInitTraverser(&trav);
    trav.mode = mode;
    trav.data = (void *)&info;
    trav.preFunc = cbPreLCA;
    trav.postFunc = cbPostLCA;
    pfuTraverse(scene, &trav);
    return info.lca;
}

/************************************************************************
 * pfuLowestCommonAncestorOfGeoSets is just like
 * pfuLowestCommonAncestor except that the condition
 * function is appied to the leaf geosets rather than nodes
 ***********************************************************************/

struct GeoSetFunAndArg
{
    int (*fun)(pfGeoSet*,void*);
    void *arg;
};
static int isGeodeContainingGSetForWhichFunReturnsTrue(pfNode *node, void *arg)
{
    struct GeoSetFunAndArg *fun_and_arg = (struct GeoSetFunAndArg *)arg;
    if (pfIsOfType(node, pfGetGeodeClassType()))
    {
	pfGeode *geode = (pfGeode *)node;
	int nGSets = pfGetNumGSets(geode);
	int i;
	for (i = 0; i < nGSets; ++i)
	    if (fun_and_arg->fun(pfGetGSet(geode, i), fun_and_arg->arg))
		return TRUE;
    }
    return FALSE;
}

extern PFUDLLEXPORT pfNode *
pfuLowestCommonAncestorOfGeoSets(pfNode *scene,
				 int (*fun)(pfGeoSet*,void*), void *arg,
				 int mode)
{
    struct GeoSetFunAndArg fun_and_arg;
    fun_and_arg.fun = fun;
    fun_and_arg.arg = arg;
    return pfuLowestCommonAncestor(scene,
				   isGeodeContainingGSetForWhichFunReturnsTrue,
				   (void *)&fun_and_arg,
				   mode);
}


/************************************************************************
 * pfuFindTexture finds the n'th texture in the scene
 * for which fun(gstate,tex,arg) returns true.
 * XXX textures occuring multiple times will be counted multiple times.
 * See the libpfsubstclip source for a more sophisticated algorithm.
 ***********************************************************************/

struct TexFunAndArg
{
    int (*fun)(pfGeoState*,pfTexture*,void*);
    void *arg;
    pfTexture *tex; /* return value */
    int n, i;
};

static int _find_texture(pfuTraverser *trav)
{
    struct TexFunAndArg *fun_and_arg = (struct TexFunAndArg *)trav->data;
    int i;
    if (pfIsOfType(trav->node, pfGetGeodeClassType()))
    {
        pfGeode *geode = (pfGeode *)trav->node;
	int nGSets = pfGetNumGSets(geode);
        for (i = 0; i < nGSets; ++i)
        {
            pfGeoSet *gset = pfGetGSet(geode, i);
            pfGeoState *gstate = pfGetGSetGState(gset);
            if (gstate != NULL)
	    {
                pfTexture *tex = (pfTexture *) pfGetGStateAttr(gstate,
							       PFSTATE_TEXTURE);
                if (tex != NULL
		 && (fun_and_arg->fun == NULL ||
		     fun_and_arg->fun(gstate, tex, fun_and_arg->arg)))
		    if ((fun_and_arg->n < 0) || (fun_and_arg->i == fun_and_arg->n))
		    {
			fun_and_arg->tex = tex;
			return PFTRAV_TERM;
		    }
		    else
			fun_and_arg->i++;
            }
        }
    }
    else if (pfIsOfType(trav->node, pfGetASDClassType()))
    {
	pfASD *asd = (pfASD *) trav->node;
	int numGStates;
	pfGeoState **gstates;
	pfGetASDGStates(asd, &gstates, &numGStates);
	for (i = 0; i < numGStates; ++i)
	{
            pfGeoState *gstate = gstates[i];
            if (gstate != NULL)
	    {
                pfTexture *tex = (pfTexture *) pfGetGStateAttr(gstate,
							       PFSTATE_TEXTURE);
                if (tex != NULL
		 && (fun_and_arg->fun == NULL ||
		     fun_and_arg->fun(gstate, tex, fun_and_arg->arg)))
		    if ((fun_and_arg->n < 0) || (fun_and_arg->i == fun_and_arg->n))
		    {
			fun_and_arg->tex = tex;
			return PFTRAV_TERM;
		    }
		    else
			fun_and_arg->i++;
            }
	}
    }
    return PFTRAV_CONT;
}

extern PFUDLLEXPORT pfTexture *
pfuFindTexture(pfNode *scene,
	       int n,
	       int (*fun)(pfGeoState*,pfTexture*,void*), void *arg,
	       int mode)
{
    pfuTraverser trav;
    struct TexFunAndArg fun_and_arg;
    fun_and_arg.fun = fun;
    fun_and_arg.arg = arg;
    fun_and_arg.tex = NULL;
    fun_and_arg.n = n;
    fun_and_arg.i = 0;
    pfuInitTraverser(&trav);
    trav.preFunc = _find_texture;
    trav.data = (void *)&fun_and_arg;
    trav.mode = mode;
    pfuTraverse(scene, &trav);
    return fun_and_arg.tex;
}



/*
** Find all cliptextures in the scene graph. If the
** _promote argument is nonzero, all cliptextures found will be promoted
** to mpcliptextures and added to the _tex list. Otherwise the cliptextures
** will be returned to the list unchanged.
*/

typedef struct {
    uint   promote; /* promote cliptextures to mpcliptextures */
    pfList *cliptextures;
} _pfuFindClipTexData;


int
_pfuClipTexCallback(pfGeoState *_gstate, pfTexture *_tex, void *_data)
{
    _pfuFindClipTexData *info = (_pfuFindClipTexData *)_data;

    if(pfIsOfType(_tex, pfGetClipTextureClassType()))
    {
	/* don't do it to the same cliptexture twice */
	if(pfSearch(info->cliptextures, _tex) == -1)
	    if(info->promote)
	    {
		pfMPClipTexture *mpct = pfNewMPClipTexture();
		pfMPClipTextureClipTexture(mpct, (pfClipTexture *)_tex);
		pfAdd(info->cliptextures, mpct);
	    }   
	    else
		pfAdd(info->cliptextures, _tex);
    }  
    return PFTRAV_CONT; /* find all cliptextures and mpcliptextures */
}

extern PFUDLLEXPORT void
pfuFindClipTextures(pfList *_tex, pfNode *_scene, uint _promote)
{
    _pfuFindClipTexData data;

    data.cliptextures = _tex;
    data.promote = _promote;

    pfuFindTexture(_scene, -1, _pfuClipTexCallback, (void*)&data, 0);
}
