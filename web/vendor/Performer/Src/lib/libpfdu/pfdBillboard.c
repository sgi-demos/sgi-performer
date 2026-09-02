/*
 * pfdBillboard.c
 *
 * $Revision: 1.8 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * Gather sibling non-instanced billboard nodes into a single 
 * billboard node. Such a node will work well and run faster
 * at the expense of overly conservative culling.
 *
 * Copyright 1995, Silicon Graphics, Inc.
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

#include <stdlib.h>
#include <string.h>

#ifdef _POSIX_SOURCE
#ifndef __linux__
extern char *strdup(const char *);
#endif
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

/***
 ***	Utility to merge pfBillboard nodes
 ***/

typedef struct
{
    int		oldCount;
    int		newCount;
    int		sizeLimit;
} pfdBillboardCBData;

static int
billboardTrav(pfuTraverser *trav)
{
    int	 	i		= 0;
    int	 	numChildren	= 0;
    int	 	j		= 0;
    pfNode	*child		= NULL;
    pfNode	*similar	= NULL;
    pfdBillboardCBData *cbd 	= (pfdBillboardCBData*)trav->data;

    /* Punt if not group or if ordering of children matters to current node */
    if (!pfIsOfType(trav->node, pfGetGroupClassType()) ||
        pfIsOfType(trav->node, pfGetSwitchClassType()) || 
	pfIsOfType(trav->node, pfGetLODClassType()) || 
	pfIsOfType(trav->node, pfGetSeqClassType()))
	return PFTRAV_CONT;

    /* loop through child nodes looking for a billboard */
    numChildren = pfGetNumChildren(trav->node);
    for (i = numChildren - 1; i >= 0; i--)
    {
	/* skip NULL children */
	if ((child = pfGetChild(trav->node, i)) == NULL)
	    continue;

	/* skip non-pfBillboard children */
	if (!pfIsOfType(child, pfGetBboardClassType()))
	    continue;

	/* a pfBillboard node child has been found */
	++cbd->oldCount;

	/* skip instanced children */
	if (pfGetNumParents(child) > 1)
	    continue;

	/* combine this pfBillboard with left-most similar one */
	for (j = 0; j < i; j++)
	{
	    pfVec3	cAxis, sAxis;
	    pfVec3	position;
	    int	cMode, sMode;
	    int	cNum,  sNum;
	    int	k;
	    int	n;

	    /* skip NULL children */
	    if ((similar = pfGetChild(trav->node, j)) == NULL)
		continue;

	    /* skip non-pfBillboard children */
	    if (!pfIsOfType(similar, pfGetBboardClassType()))
		continue;

	    /* skip instanced children */
	    if (pfGetNumParents(similar) > 1)
		continue;

	    /* skip over-sized pfBillboards */
	    sNum = pfGetNumGSets((pfBillboard *)similar);
	    cNum = pfGetNumGSets((pfBillboard *)child);
	    if (cbd->sizeLimit > 0 && sNum >= cbd->sizeLimit)
		continue;

	    /* skip pfBillboards with different rotation mode */
	    sMode = pfGetBboardMode((pfBillboard *)similar, PFBB_ROT);
	    cMode = pfGetBboardMode((pfBillboard *)child,   PFBB_ROT);
	    if (sMode != cMode)
		continue;
	    
	    /* skip pfBillboards with different rotation axes */
	    pfGetBboardAxis((pfBillboard *)similar, sAxis);
	    pfGetBboardAxis((pfBillboard *)child,   cAxis);
	    if (!pfAlmostEqualVec3(cAxis, sAxis, 0.0001f))
		continue;

	    /* 
	     * success! now move as many geosets as allowed
	     */

	    /* reparent as many as allowed to similar node */
	    for (k = 0; k < cNum; k++)
	    {
		/* honor merged pfBillboard size limit */
		if (cbd->sizeLimit > 0 && (sNum+k+1) > cbd->sizeLimit)
		    break;

		pfAddGSet((pfBillboard *)similar, 
		    pfGetGSet((pfBillboard *)child, k));
		pfGetBboardPos((pfBillboard *)child, k, position);
		pfBboardPos((pfBillboard *)similar, sNum + k, position);
	    }

	    /* remove reparented nodes [0..k-1] from old parent */
	    for (n = 0; n < k; n++)
		pfRemoveGSet(child, pfGetGSet(child, 0));
	    
	    /* if job's done -- break out of loop */
	    if (n >= cNum)
		break;
	}
    }

    /* shuffle is complete. delete empty pfBillboard nodes */
    for (i = numChildren - 1; i >= 0; i--)
    {
	/* skip NULL children */
	if ((child = pfGetChild(trav->node, i)) == NULL)
	    continue;

	/* skip non-pfBillboard children */
	if (!pfIsOfType(child, pfGetBboardClassType()))
	    continue;

	/* delete empty pfBillboards */
	if (pfGetNumGSets((pfBillboard *)child) < 1)
	{
	    pfRemoveChild(trav->node, child);
	    pfDelete(child);
	    continue;
	}

	/* non-empty billboard. it's a survivor */
	++cbd->newCount;
    }

    /* no explicit action required for other node types */
    /* continue traversal */
    return PFTRAV_CONT;
}

/*
 * Gather sibling non-instanced billboard nodes into a single 
 * billboard node. Such a node will work well and run faster
 * at the expense of overly conservative culling. The number
 * of total agglomerated pfGeoSets in a pfBillboard is limited
 * to the value of the sizeLimit argument.
 */
extern PFDUDLLEXPORT void
pfdCombineBillboards(pfNode *node, int sizeLimit)
{
    int			oc;
    int			nc;
    pfuTraverser	trav;
    pfdBillboardCBData 	cbd;
    double		startTime;
    double		elapsedTime;

    /* check arguments */
    if (node == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfdCombineBillboards: NULL pfNode");
	return;
    }

    /* not much can happen in this case, so take early out */
    if (sizeLimit == 1)
	return;

    pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	"pfdCombineBillboards optimizing traversal");

    /* define state-sharing traversal */
    cbd.oldCount  = 0;
    cbd.newCount  = 0;
    cbd.sizeLimit = sizeLimit;

    pfuInitTraverser(&trav);
    trav.preFunc = billboardTrav;
    trav.data    = &cbd;

    /* perform billboard-combining traversal */
    startTime = pfGetTime();
    pfuTraverse(node, &trav);
    elapsedTime = pfGetTime() - startTime;

    /* report results of billboard-combining traversal */
    oc = cbd.oldCount;
    nc = cbd.newCount;
    if (oc != 0)
    {
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "  Scene-graph reorganization actions:");
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Combining sibling pfBillboard nodes");
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Combine size limit:%4d %s", sizeLimit,
	    (sizeLimit < 1) ? "(unlimited)" : "");
    }
    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Performance statistics:");
    if (oc != 0)
    {
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Input billboards:  %4d (%6.2f%%)", oc, 100.0f);
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Output billboards: %4d (%6.2f%%)", nc, (100.0f*nc)/oc);
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Deleted billboards:%4d (%6.2f%%)", oc-nc, (100.0f*(oc-nc))/oc);
    }
    pfNotify(PFNFY_INFO, PFNFY_MORE, 
	"    Elapsed time:        %.3f sec", elapsedTime);
    pfNotify(PFNFY_INFO, PFNFY_MORE, NULL);
}
