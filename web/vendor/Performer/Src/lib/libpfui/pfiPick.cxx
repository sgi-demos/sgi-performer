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
 * file: pfiPick.C
 * ----------------
 *
 * pfiPick Class Routines
 *
 * $Revision: 1.12 $
 * $Date: 2002/09/18 02:47:08 $
 *
 */
 
#define PF_C_API 1 // enable C API also 

#include <Performer/pf.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfMemory.h>
#include <Performer/prmath.h>
#include <Performer/pfui.h>

#include "pfiPick.h"

#define pfQueryHit(_hit, _which, _dst)  \
        pfQueryHit(_hit, _which, (float*)_dst)


PFUIDLLEXPORT pfType *pfiPick::classType = NULL;

void
pfiPick::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfiPick");
    }
}

pfiPick::pfiPick(void)
{
    void *arena		= pfMemory::getArena(this);
    
    setType(classType);
    pPickMode		= PFPK_M_ALL;
    pIsectMode		= PFTRAV_IS_PRIM | PFTRAV_IS_PATH;
    pCount		= 0;
    pChan		= NULL;
    pNode		= NULL;
    pPath		= pfNewPath();
    pChanList		= pfNewList(sizeof(void*), 1, arena);
    pNodeList		= pfNewList(sizeof(void*), 1, arena);
    pPathList		= pfNewList(sizeof(void*), 1, arena);
    pHitCBData		= NULL;
    pHitCB		= NULL;
}

pfiPick::~pfiPick(void)
{
    pfUnrefDelete(pChanList);
    pfUnrefDelete(pNodeList);
    pfUnrefDelete(pPathList);
}

void
pfiPick::setMode(int _mode, int _val)
{
    switch(_mode)
    {
	case PFIP_MODE_PICK:
	    pPickMode = _val;
	    break;
	case PFIP_MODE_ISECT:
	    pIsectMode = _val;
	    break;
        default:
	    pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfiPick::setMode() - unknown mode %d", _mode);
	    break;
    }
}

int
pfiPick::getMode(int _mode) const
{
    switch(_mode)
    {
	case PFIP_MODE_PICK:
	    return pPickMode;
	case PFIP_MODE_ISECT:
	    return pIsectMode;
        default:
	    pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"pfiPick::setMode() - unknown mode %d", _mode);
	    return -1;
    }
}

void
pfiPick::addChan(pfChannel *_chan)
{
    if (pfSearch(pChanList, _chan) < 0)
	pfAdd(pChanList, _chan);
}

void
pfiPick::insertChan(int _index, pfChannel *_chan)
{
    if (pfSearch(pChanList, _chan) < 0)
	pfInsert(pChanList, _index, _chan);
}

void
pfiPick::removeChan(pfChannel *_chan)
{
    pfRemove(pChanList, _chan);
}

void
pfiPick::setupChans(void)
{ 
    int i;
    int num;
    pfScene *scene;

    num = pfGetNum(pChanList);
    for (i=0; i < num; i++)
    {
	pfChannel *chan = (pfChannel*) pfGet(pChanList, i);
	if (chan)
	{
	    scene = pfGetChanScene(chan);
	    if (scene) 
		pfNodePickSetup(scene);
        }
    }
}

void
pfiPick::reset(void)
{
    pCount		= 0;
    pChan		= NULL;
    pNode		= NULL;
    pPath		= pfNewPath();
    pfResetList(pNodeList);
    pfResetList(pPathList);
    pfResetList(pPathList);
}

int
pfiPick::doPick(int _x, int _y)
{ 
    int i;
    int num;
    pfChannel *chan;
    
    // for each chan in selection list, do pick against scene
    pCount = 0;
    num = pfGetNum(pChanList);
    for (i=0; i < num; i++)
    {
	chan = (pfChannel*) pfGet(pChanList, i);
	if (chan)
	    if (_doPick(chan, _x, _y) > 0)
	    {
		pChan = chan;
		// the pHitCB can force a continue and pick on the next channel
		if (pHitCB && ((pHitCB(this, pHitCBData))))
		    continue;
		else
		    return pCount;
	    }
    }
    return pCount;
}

int
pfiPick::_doPick(pfChannel *_chan, int _x, int _y)
{ 
    float	    nx,  ny;
    int		    i, j, pathLen;
    pfHit	    **picklistp=NULL;
    pfNode	    *pathElt, *node;
    pfPath	    *path=NULL;
    int		    count;

    pfuCalcNormalizedChanXY(&nx, &ny, _chan, _x, _y);
    count = pfChanPick(_chan, 
	    pPickMode | pIsectMode, 
	    nx,  ny, 0.0f, &picklistp);

    if (count > 0)
    {
	pCount = count;
	pfResetList(pNodeList);
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT," Pick %d Hits", pCount);
	for (i = 0 ; i < pCount ; i++)
        {
	    pfQueryHit(picklistp[i], PFQHIT_NODE, &node);
	    if (i==0)
		pNode = node;
	    if (node)
		pfAdd(pNodeList, node);
	    pfQueryHit(picklistp[i], PFQHIT_GSET, &pGSet);
	    pfQueryHit(picklistp[i], PFQHIT_PATH, &path);
	
            if (i==0)
                pPath = path;
	    if (path)
	    {
                pfAdd(pPathList, path);
		pathLen = pfGetNum(path);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
			 "Have path for %dth pfHit of %d children",
			 i, pathLen);
		for (j=0; j < pathLen; j++)
		{
		    pathElt = (pfNode*)pfGet(path, j);
		    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			     "\tchild %d 0x%x of type %s",
			     j, pathElt, pfGetTypeName(pathElt));
		}
	    }
	}
    }
    else
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "No Pick: %d %d.", _x, _y);
    return pCount;
    
}
