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
 * pfdLayer.c
 *
 * $Revision: 1.18 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * Gather sibling layer nodes into a single layer with all the 
 * bases together and all of the layers together. Such a node
 * will work well and run faster.
 *
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
 ***	Utility to merge pfLayer nodes
 ***/

typedef struct
{
    int		oldCount;
    int		newCount;
} pfdShareCBData;

static int
layerTrav(pfuTraverser *trav)
{
    int	 i		= 0;
    int	 numChildren	= 0;
    int	 j		= 0;
    int	 numGrandChildren = 0;
    int	 numLayers	= 0;
    pfNode	*child		= NULL;
    pfdShareCBData *cbd 	= (pfdShareCBData*)trav->data;
    pfGroup	*newBase	= NULL;
    pfGroup	*newDecal	= NULL;
    pfLayer	*newLayer	= NULL;

    /* Punt if not group or if ordering of children matters to current node */
    if (!pfIsOfType(trav->node, pfGetGroupClassType()) ||
        pfIsOfType(trav->node, pfGetSwitchClassType()) || 
	pfIsOfType(trav->node, pfGetLODClassType()) || 
	pfIsOfType(trav->node, pfGetSeqClassType()))
	return PFTRAV_CONT;

    /* loop through child nodes looking for uninstanced layers */
    numChildren = pfGetNumChildren(trav->node);
    for (i = numChildren - 1; i >= 0; i--)
    {
	/* skip NULL children */
	if ((child = pfGetChild(trav->node, i)) == NULL)
	    continue;

	/* skip non-pfLayer children */
	if (!pfIsOfType(child, pfGetLayerClassType()))
	    continue;

	if (pfGetLayerMode((pfLayer *)child) != PFDECAL_BASE_DISPLACE &&
	    pfGetLayerMode((pfLayer *)child) != PFDECAL_BASE_FAST)
	    continue;

	/* a pfLayer node child has been found */
	++cbd->oldCount;

	/* skip instanced children (conservative choice) */
	if (pfGetNumParents(child) > 1)
	{
	    /* can't combine, but at least force displace mode */
	    pfLayerMode((pfLayer *)child, PFDECAL_BASE_DISPLACE);
	    ++cbd->newCount;
	    continue;
	}

	/* move GrandChildren from Child to new groups */
	numGrandChildren = pfGetNumChildren(child);

	/* DISPLACE supports maximum of 8 stacked layers */
	if (numGrandChildren > 8)
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "pfdCombineLayers() Layer 0x%x "
		     "has more than 8 children - can't combine and use "
		     "PFDECAL_BASE_DISPLACE", child);
	    continue;
	}

	if (numGrandChildren > 0)
	{
	    /* prepare new layer group and base child */
	    if (newLayer == NULL)
	    {
		newLayer = pfNewLayer();
		pfLayerMode(newLayer, PFDECAL_BASE_DISPLACE);
		pfAddChild(newLayer, (newBase = pfNewGroup()));
	    }
	    else
		newBase = (pfGroup*) pfGetChild(newLayer, 0);

	    /* add base node to newBase */
	    pfAddChild(newBase, pfGetChild(child, 0));

	    /* add layer nodes to newDecal group IN ORDER */
	    for (j = 1; j < numGrandChildren; j++)
	    {
		/* prepare new decal group */
		if (pfGetNumChildren(newLayer) <= j)
		    pfAddChild(newLayer, (newDecal = pfNewGroup()));
		else
		    newDecal = (pfGroup*) pfGetChild(newLayer, j);

		/* add decal node to newDecal */
		pfAddChild(newDecal, pfGetChild(child, j));
	    }
	    /* remove all children from child */
	    for (j = 0; j < numGrandChildren; j++)
		pfRemoveChild(child, pfGetChild(child, 0));

	}
	pfRemoveChild(trav->node, child);
	pfDelete(child);

    }
    if (!newLayer)
	return PFTRAV_CONT;
    
    /* deal with the new base and decal lists */
    if (pfGetNumChildren(newLayer) == 1)
    {
	/* 
	 * in the unlikely case that all the old pfLayer nodes
	 * consisted of only bases and no decals, then there's
         * no need for a pfLayer node at all.
	*/
	pfAddChild(trav->node, pfGetChild(newLayer, 0));
	pfDelete(newLayer);
    }
    else
    if (pfGetNumChildren(newLayer) > 1)
    {
	int		i;

	/*
	 * the expected case: all non-instanced pfLayer nodes
	 * have been "merged" -- now we need to build a new
         * pfLayer node to contain them.
	*/
	pfAddChild(trav->node, newLayer);

        /* Remove all useless groups with only 1 child */
        for (i=0; i<pfGetNumChildren(newLayer); i++)
        {
            pfNode      *child;
            
            child = pfGetChild(newLayer, i);
            if (pfGetNumChildren(child) == 1)
            {
                pfReplaceChild(newLayer, child, pfGetChild(child, 0));
                pfDelete(child);
            }
        }

	++cbd->newCount;
    }
    else
	pfDelete(newLayer);

    /* no explicit action required for other node types */
    /* continue traversal */
    return PFTRAV_CONT;
}

/*
 *	the idea is to gather up sibling layer nodes into
 *	a gang layer with all the bases together and all
 *	of the layers after that.
 */
extern PFDUDLLEXPORT void
pfdCombineLayers(pfNode *node)
{
    int			oc;
    int			nc;
    int			qret;
    float		total;
    pfuTraverser	trav;
    pfdShareCBData 	cbd;
    double		startTime;
    double		elapsedTime;

    /* check argument */
    if (node == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdCombineLayers: NULL pfNode");
	return;
    }

    pfNotify(PFNFY_INFO, PFNFY_PRINT, "pfdCombineLayers optimizing traversal");

    pfQueryFeature(PFQFTR_DISPLACE_POLYGON, &qret);
    if (qret != PFQFTR_FAST)
    {
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "DISPLACE_POLYGON not optimal on this platform - "
	    "abandoning pfdCombineLayers traversal.");
    }

    /* define state-sharing traversal */
    cbd.oldCount = 0;
    cbd.newCount = 0;
    
    pfuInitTraverser(&trav);
    trav.preFunc = layerTrav;
    trav.data    = &cbd;

    /* perform state-sharing traversal */
    startTime = pfGetTime();
    pfuTraverse(node, &trav);
    elapsedTime = pfGetTime() - startTime;

    /* report results of layer-combining traversal */
    oc = cbd.oldCount;
    nc = cbd.newCount;
    if (oc != 0)
    {
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "  Scene-graph reorganization actions:");
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Combining sibling pfLayer nodes");
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    pfLayers use PFDECAL_BASE_DISPLACE");
    }
    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Performance statistics:");
    if (oc != 0)
    {
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Input layers:    %5d (%6.2f%%)", oc, 100.0f);
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Output layers:   %5d (%6.2f%%)", nc, (100.0f*nc)/oc);
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Deleted layers:  %5d (%6.2f%%)", oc-nc, (100.0f*(oc-nc))/oc);
    }
    pfNotify(PFNFY_INFO, PFNFY_MORE, 
	"    Elapsed time:        %.3f sec", elapsedTime);
    pfNotify(PFNFY_INFO, PFNFY_MORE, NULL);
}
