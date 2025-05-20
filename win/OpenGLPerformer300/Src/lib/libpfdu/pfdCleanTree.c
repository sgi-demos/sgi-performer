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
 * pfdCleanTree
 *
 * $Revision: 1.11 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * pfdCleanTree optimizes scene graph by removing empty groups 
 *
 * pfdFreezeTransforms optimizes scene graph by replacing most DCSes with 
 * SCSes (unless the name includes "dcs" or a callback otherwise 
 * prevents it).  This is usually done before calling pfFlatten() 
 * to reduce the number of transformations sent to the graphics pipe.
 *
 */
 
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <string.h>
#include <Performer/pfdu-DLL.h>

/* #define CLEANDEBUG 1 */

/*
 * Replace "oldn" with "newn"
 */

static int
checkTrav(pfNode *node, int trav)
{
    pfNodeTravFuncType	pre, post;
    void *data;
    
    data = pfGetNodeTravData(node, trav);
    pfGetNodeTravFuncs(node, trav, &pre, &post);
    
    return (data != NULL || pre != NULL || post != NULL);
}

static void
copyTrav(pfNode *dst, pfNode *src, int trav)
{
    pfNodeTravFuncType	pre, post;
    void *data;
    
    data = pfGetNodeTravData(src, trav);
    if (data != NULL)
	pfNodeTravData(src, trav, data);

    pfGetNodeTravFuncs(src, trav, &pre, &post);
    if (pre != NULL || post != NULL)
	pfNodeTravFuncs(src, trav, pre, post);

    pfNodeTravMask(dst, trav, pfGetNodeTravMask(src, trav), 
		   PFTRAV_SELF, PF_SET);
}

void
copyNode(pfNode* newn, pfNode* oldn)
{
    if (newn != NULL)
    {

	copyTrav(newn, oldn, PFTRAV_APP);
	copyTrav(newn, oldn, PFTRAV_CULL);
	copyTrav(newn, oldn, PFTRAV_DRAW);
	copyTrav(newn, oldn, PFTRAV_ISECT);
	
	{
	    const char *name = pfGetNodeName(oldn);
	    if (name != NULL && name[0] != '\0')
		pfNodeName(newn, name);
	}
	
	{
	    int i, n = pfGetNumUserData(oldn);
	    for (i = 0; i < n; ++i)
	    {
		void *ud = pfGetUserDataSlot(oldn, i);
		if  (ud != NULL)
		    pfUserDataSlot(newn, i, ud);
	    }
	}
    }    
}

/*
 * pfdReplaceNode - replace oldn with newn in the scene graph,
 * under all of oldn's parents and copying callbacks and masks
 * from oldn to newn.
 */

PFDUDLLEXPORT void
pfdReplaceNode(pfNode *oldn, pfNode *newn)
{
    int		i, num;
    
#ifdef CLEANDEBUG
    printf("\treplacing %s(0x%x) with %s(0x%x)\n",
	   pfGetTypeName(oldn), oldn, 
	   pfGetTypeName(newn), newn);
#endif

    /* just in case newn was a child of oldn */
    pfRemoveChild(oldn, newn);

    /* Reparent old node's children to new node */
    if (pfIsOfType(oldn, pfGetGroupClassType()))
    {
	pfGroup *newgrp;

	if (newn != NULL && pfIsOfType(newn, pfGetGroupClassType()))
	    newgrp = (pfGroup*)newn;
	else
	    newgrp = NULL;

	num = pfGetNumChildren(oldn);
	for (i = 0; i < num; i++)
	{
	    pfNode *child = pfGetChild(oldn, 0);
	    
	    pfRemoveChild(oldn, child);
	    if (newn != NULL)
	    {
		if (newgrp == NULL)
		    pfNotify(PFNFY_WARN, PFNFY_USAGE,
			     "pfdReplaceNode() WARNING: replacing a %s with "
			     " children by a %s\n", 
			     pfGetTypeName(oldn), pfGetTypeName(newn));
		else if (newn != child)
		    pfAddChild(newn, child);
	    }
	}
    }
    
    /* Substitute new node for old */
    for (i = pfGetNumParents(oldn)-1; i >= 0 ; i--)
	if (newn != NULL)
	    pfReplaceChild(pfGetParent(oldn, i), oldn, newn);
	else
	    pfRemoveChild(pfGetParent(oldn, i), oldn);
    
    copyNode(newn, oldn);
}


/*
 * pfdInsertGroup - insert grp above oldn in the the scene graph 
 * parenting grp to all parents of oldn.
 */

PFDUDLLEXPORT void
pfdInsertGroup(pfNode *oldn, pfGroup *grp)
{
    int i;
    int num;
    
    num = pfGetNumParents(oldn);
    /* replace oldn with grp as child in all parents */
    for (i=0; i < num ; i++)
    {
	pfGroup *parent = pfGetParent(oldn, i);
	pfReplaceChild(parent, oldn, grp);
    }
    
    /* add oldn as child to grp */
    pfAddChild(grp, oldn);
}

/*
 * pfdRemoveGroup - remove oldn from the scene graph reparenting 
 * any children to each of oldn's parents
 */

PFDUDLLEXPORT void
pfdRemoveGroup(pfGroup *oldn)
{
    int i, j, k;
    int numPar, numChild;
    
    numPar = pfGetNumParents(oldn);
    numChild = pfGetNumChildren(oldn);

    /* For each parent:
     * 	1) insert all children of oldn in parent's 
     *	   child list just before oldn. 
     *  2) remove chidren from oldn
     *	3) remove oldn from parent list.
     */
    for (i=0; i < numPar ; i++)
    {
	pfGroup *parent = pfGetParent(oldn, i);
	k = pfSearchChild(parent, oldn);
	for (j=0 ; j < numChild ; j++)
	{
	    pfNode *child = pfGetChild(oldn, 0);
	    pfInsertChild(parent, k++, child);
	    pfRemoveChild(oldn, child);
	}
	pfRemoveChild(parent, oldn);
    }
}

/*
 * Traverse scene graph and remove useless nodes:
 * 	1. empty pfGroups
 * 	2. identity pfSCSs
 * 	3. pfGroups with a single child
 */

static pfuTravFuncType CleanFunc = NULL;
static pfuTraverser CleanTrav;

static pfNode* cleanTree(pfNode *node, pfuTravFuncType doitFunc);

PFDUDLLEXPORT pfNode*
pfdCleanTree(pfNode *node, pfuTravFuncType doitFunc)
{
    pfGroup *grp = pfNewGroup();
    CleanFunc = doitFunc;
    pfuInitTraverser(&CleanTrav);

    pfAddChild(grp, node);
    cleanTree((pfNode*)grp, doitFunc);
    node = pfGetChild(grp, 0);
    pfRemoveChild(grp, node);
    pfDelete(grp);
    return node;
}

static pfNode*
cleanTree(pfNode *node, pfuTravFuncType doitFunc)
{
    int i;
    static pfMatrix identMat = {1.0f, 0.0f, 0.0f, 0.0f,
				    0.0f, 1.0f, 0.0f, 0.0f,
				    0.0f, 0.0f, 1.0f, 0.0f,
				    0.0f, 0.0f, 0.0f, 1.0f};
    
#ifdef CLEANDEBUG
    static int depth = 0;
    const char* name = pfGetTypeName(node);

    depth++;
    printf("%d %s(0x%x) children:%d\n",
	   depth, name, node,
	   pfIsOfType(node, pfGetGroupClassType())?pfGetNumChildren(node):-1);
#endif
    
    if (pfIsOfType(node, pfGetGroupClassType()))
    {
        int nc;
	int mergeSCSes = FALSE;
	
        /*
	 * Recurse on children. Act on return value. 
	 */
        for (i = 0; i < (nc = pfGetNumChildren(node)); i++)
        {
            pfNode     *child = pfGetChild(node, i), *cleanChild;
	    
	    /* Replace identity vanilla pfSCS with pfGroup */
	    if (pfIsExactType(child, pfGetSCSClassType()))
	    {
		pfMatrix        scsMat;
		
		pfGetSCSMat((pfSCS*)child, scsMat);
		if (PFEQUAL_MAT(scsMat, identMat))
		{
		    pfGroup *grp = pfNewGroup();
		    pfdReplaceNode(child, (pfNode*)grp);
		    
		    if (pfDelete(child) <= 0)
		    {
			pfNotify(PFNFY_WARN, PFNFY_ASSERT,
				 "pfdCleanTree() unsuccessful pfDelete().");
		    }
		    child = (pfNode*)grp;
		}
	    }
	    
	    /* Recurse on child */
            cleanChild = cleanTree(child, doitFunc);

	    if (cleanChild != child)
	    {
		int doit;

		/* don't doit if merging child and cleanChild's
		 * trav funcs/data or user data would collide
		 */
		/* XXX None of this is legal if child or cleanChild is NULL! */
		doit = !((checkTrav(child, PFTRAV_APP) &&
			  checkTrav(cleanChild, PFTRAV_APP)) ||
			 (checkTrav(child, PFTRAV_CULL) &&
			  checkTrav(cleanChild, PFTRAV_CULL)) ||
			 (checkTrav(child, PFTRAV_DRAW) &&
			  checkTrav(cleanChild, PFTRAV_DRAW)) ||
			 (checkTrav(child, PFTRAV_ISECT) &&
			  checkTrav(cleanChild, PFTRAV_ISECT)));
		if (doit)
		{
		    int i, n = pfGetNumNamedUserDataSlots();
		    for (i = 0; i < n; ++i)
			if (pfGetUserDataSlot(child, i) &&
			    pfGetUserDataSlot(cleanChild, i))
			{
			    doit = 0;
			    break;
			}
		}

		if (doit && cleanChild == NULL)
		{
		    /* only nuke children of real pfGroups and pfSCSes
		     * since most others rely on child ordering and may
		     * have empty nodes as place holders
		     */
		    doit = (pfIsExactType(node, pfGetGroupClassType()) ||
			    pfIsOfType(node, pfGetSCSClassType()));
		}

		if (doitFunc)
		{
		    CleanTrav.node = child;
		    CleanTrav.data = &doit;
		    doit = (*doitFunc)(&CleanTrav);
		}

		if (doit)
                {
		    if (cleanChild == NULL)
		    {
			pfRemoveChild(node, child);
			pfDelete(child);
			i--;
		    }
		    else if (pfIsExactType(child, pfGetSCSClassType()) &&
			     pfIsExactType(cleanChild, pfGetSCSClassType()))
		    {
			/* if both are SCSes merge them into one */
			pfMatrix m1, m2;
			pfSCS *scs;
			int j, num;

			pfGetSCSMat((pfSCS*)child, m1);	
			pfGetSCSMat((pfSCS*)cleanChild, m2);
			pfPreMultMat(m1, m2);
			scs = pfNewSCS(m1);
			pfdReplaceNode(cleanChild, (pfNode *)scs);
			pfRemoveChild(child, scs);
			pfdReplaceNode(child, (pfNode *)scs);
			pfDelete(child);
			pfDelete(cleanChild);
		    }
		    else	
		    {
			pfRemoveChild(child, cleanChild);
			pfdReplaceNode(child, cleanChild);
			pfDelete(child);
		    }
                }
            }
	}

	/* nuke childless groups */
        if (nc == 0)
            return NULL;
        /* Reparent only child; Can do for identity (flattened) SCSes */
        else if (nc == 1)
        {
            int ok = 0;
	    
	    /* only nuke true groups */
	    if (pfIsExactType(node, pfGetGroupClassType()))
                ok = 1;

	    /* nuke true pfSCSes with identity transforms */
            else if (pfIsExactType(node, pfGetSCSClassType()))
            {
                pfMatrix        scsMat;
		pfNode* child = pfGetChild(node, 0);
		
                pfGetSCSMat((pfSCS*)node, scsMat);
                if (PFEQUAL_MAT(scsMat, identMat))
                    ok = 1;
		/* back to back SCSes can be merged into one */
		else if (pfGetNumParents(child) == 1 &&
			 pfIsExactType(child, pfGetSCSClassType()))
		    ok = 1;
            }
	    
            if (ok && pfGetNumParents(node) == 1)
                node = pfGetChild(node, 0);
        }
    }

#ifdef CLEANDEBUG
    depth--;
#endif
    return node;
}



/*
 * Replace unnamed DCSs with SCSs so we can use pfFlatten. 
 */

static pfuTravFuncType FreezeFunc = NULL;
static int staticize(pfuTraverser *trav);

PFDUDLLEXPORT pfNode*
pfdFreezeTransforms(pfNode *node, pfuTravFuncType doitFunc)
{
    pfuTraverser trav;
    FreezeFunc = doitFunc;
    pfuInitTraverser(&trav);
    trav.postFunc = staticize;

    if (!pfIsExactType(node, pfGetDCSClassType()))
	pfuTraverse(node, &trav);
    else
    {
	/* create a dummy group in case node itself is replaced */
	pfGroup *grp = pfNewGroup();
	pfAddChild(grp, node);
	
	pfuTraverse(node, &trav);
	
	node = pfGetChild(grp, 0);
	pfRemoveChild(grp, node);
	
	pfDelete(grp);
    }

    return node;
}

static int
staticize(pfuTraverser *trav)
{
    int 	i, num;
    pfNode 	*node = trav->node;
    pfNode 	*tmp, *parent;
    const char	*name = pfGetNodeName(node);
    
    /* Replace DCS with SCS unless the callback says otherwise */
    if (pfIsExactType(node, pfGetDCSClassType()))
    {
	int hastrav, doit;

	hastrav = (checkTrav(node, PFTRAV_APP) ||
		   checkTrav(node, PFTRAV_CULL) ||
		   checkTrav(node, PFTRAV_DRAW) ||
		   checkTrav(node, PFTRAV_ISECT));

	if (hastrav)
	    doit = 0;
	else if (name == NULL)
	    doit = 1;
	else
	    doit = !strstr(name, "dcs") && !strstr(name, "DCS");

	/* if there's a user supplied callback invoke that for the 
	 * final word, provide our opinion in the data pointer
	 */
	if (FreezeFunc)
	{
	    trav->data = &doit;
	    doit = (*FreezeFunc)(trav);
	}

	if (doit)
	{
	    pfMatrix 	mat;
	    
	    pfGetDCSMat((pfDCS *)node, mat);
	    trav->node = (pfNode*)pfNewSCS(mat);
	    pfdReplaceNode(node, trav->node);
	    
	    if (pfDelete(node) <= 0)
	    {
		pfNotify(PFNFY_WARN, PFNFY_ASSERT,
			 "pfdFreezeTransforms() unsuccessful pfDelete().");
	    }
	    trav->node = NULL;
	    return PFTRAV_PRUNE;
	}
    }
    return PFTRAV_CONT;
}
