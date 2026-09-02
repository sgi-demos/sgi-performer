/*
 * Copyright 1996, Silicon Graphics, Inc.
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
 * pflodfix.c - ".lodfix" pseudo-loader.
 *
 * This is to "fix" pfLODs that occur under scaling SCSs or DoubleSCSs
 * (or anything derived from these types, like DCSs and FCSs).
 * The LOD range values stored in the LOD are assumed
 * to be intended for a scale of 1; therefore if the pfLOD
 * is under a scaling matrix node in the scene graph, we scale the range values
 * by that amount (actually changing the values of each pfLOD).
 * NOTE, this will fail if the pfLODs or scales change at runtime!
 *
 * Example (using with the .scale pseudo-loader):
 *      % perfly esprit.flt
 *          (zoom out using the right mouse button, notice it changes LODs)
 *	% perfly esprit.flt.0.0000001.scale
 *          (zoom out, notice you always get the finest LOD)
 *	% perfly esprit.flt.0.0000001.scale.lodfix
 *	    (zoom out, notice the LODs act properly again)
 */

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
//#include "wintypes.h"

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pfdu.h>
#include <Performer/pf/pfLOD.h>
#include <Performer/pf/pfSCS.h>
#ifdef __linux__
#include <limits.h>
#endif

#if defined(PFMATRIX4D) && !defined (WIN32)
    #include <Performer/pf/pfDoubleSCS.h>
    #include <Performer/pr/pfGenericMatrix.h>
#else
    #define pfDoubleSCS pfSCS
    #define pfMatrix4d pfMatrix
    #define pfGenericMatStack pfMatStack
    #define get4f get
    #define preMult4f preMult
    #define preMult4d preMult
#endif

#define FOR(i,n) for ((i) = 0; (i) < (n); ++(i))
#define DET3(m) \
	   ((m)[0][0]*((m)[1][1]*(m)[2][2]-(m)[1][2]*(m)[2][1]) \
	   +(m)[0][1]*((m)[1][2]*(m)[2][0]-(m)[1][0]*(m)[2][2]) \
	   +(m)[0][2]*((m)[1][0]*(m)[2][1]-(m)[1][1]*(m)[2][0]))

static void _fixLODtraverse(pfNode *node,
			    pfGenericMatStack *mstack,
			    pfList *doneList)
{
    if (node->isOfType(pfLOD::getClassType())
     && doneList->search(node) == -1)
    {
	pfLOD *lod = (pfLOD *)node;
	pfMatrix mat;
	mstack->get4f(mat);
#ifdef WIN32
	float scale = powf(fabs(DET3(mat)), 1.0/3.0);
#else
	float scale = cbrt(fabs(DET3(mat)));
#endif

	pfNotify(PFNFY_INFO, PFNFY_PRINT,
		 "Fixing LOD 0x%p: scaling by %.8g\n", lod, scale);

	int numRanges = lod->getNumRanges();
	pfNotify(PFNFY_INFO, PFNFY_MORE,
		 "  %d Range%s:",
		 numRanges, numRanges==1 ? "" : "s");
	int i;
	FOR (i, numRanges)
	{
	    float range = lod->getRange(i);
	    lod->setRange(i, range * scale);
	    pfNotify(PFNFY_INFO, PFNFY_MORE,
		     "    %.7g -> %.7g\n", range, range*scale);
	}
	int numTransitions = lod->getNumTransitions();
	pfNotify(PFNFY_INFO, PFNFY_MORE,
		 "  %d Transition%s:",
		 numTransitions, numTransitions==1 ? "" : "s");
	FOR (i, numTransitions)
	{
	    float transition = lod->getTransition(i);
	    lod->setTransition(i, transition * scale);
	    pfNotify(PFNFY_INFO, PFNFY_MORE,
		     "    %.7g -> %.7g\n", transition, transition*scale);
	}
	// Unspecified transitions are treated by Performer as 1;
	// have to adjust them too...
	for (; i < numRanges; ++i)
	{
	    float transition = 1.f;
	    lod->setTransition(i, transition * scale);
	    pfNotify(PFNFY_INFO, PFNFY_MORE,
		     "    (%.7g) -> %.7g\n", transition, transition*scale);
	}

	doneList->add(node);
    } // end if it's a pfLOD

    int pushed = 0;
    if (node->isOfType(pfSCS::getClassType()))
    {
	pfSCS *scs = (pfSCS *)node;
	pfMatrix mat;
	scs->getMat(mat);
	mstack->push();
	mstack->preMult4f(mat);
	pushed = 1;
    }
    if (node->isOfType(pfDoubleSCS::getClassType()))
    {
	pfDoubleSCS *scs = (pfDoubleSCS *)node;
	pfMatrix4d mat;
	scs->getMat(mat);
	mstack->push();
	mstack->preMult4d(mat);
	pushed = 1;
    }
	
    if (node->isOfType(pfGroup::getClassType()))
    {
	pfGroup *group = (pfGroup *)node;
	int n = group->getNumChildren();
	int i;
	for (i = 0; i < n; ++i)
	    _fixLODtraverse(group->getChild(i), mstack, doneList);
    }
    if (pushed)
	mstack->pop();
} // end _fixLODtraverse()

void pfuFixLODs(pfNode *node)
{
    // SP is really sufficient for this matrix stack
    // since we're only interested in the scales,
    // but let's exercise pfGenericMatStack anyway...
    pfGenericMatStack *mstack = new((void *)NULL) pfGenericMatStack(32);
    pfList *doneList = new((void *)NULL) pfList;

    _fixLODtraverse(node, mstack, doneList);

    pfDelete(doneList);
    pfDelete(mstack);
}


char *strrnchr(char *s, int n, int c)
{
    char *p = NULL;
    int i;
    for (i = 0; i < n && s[i] != '\0'; ++i)
	if (s[i] == c)
	    p = s+i;
    return p;
}

extern "C" PFPFB_DLLEXPORT pfNode *
pfdLoadFile_lodfix(char *fileName)
{
    char *lastdot;
    if ((lastdot = strrchr(fileName, '.')) == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		"pfdLoadFile_lodfix: bad file name %s", fileName);
	return NULL;
    }

    char realName[PF_MAXSTRING];
    strncpy(realName, fileName, lastdot-fileName);
    realName[lastdot-fileName] = '\0';

    pfNode *node = pfdLoadFile(realName);
    if (node == NULL)
	return NULL; /* error message printed already */

    pfuFixLODs(node);

    return node;
}
