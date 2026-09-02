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
 * file: cPick.C
 * ----------------
 *
 * pfiPick Class C-API Wrappers
 *
 * $Revision: 1.7 $
 * $Date: 2002/07/24 01:09:01 $
 *
 */
 


#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pr/pfMemory.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pfui.h>

#include "pfiPick.h"



PFUIDLLEXPORT pfType*
pfiPickClassType(void)
{
    return pfiPick::getClassType();    
}


PFUIDLLEXPORT pfiPick*		 
pfiNewPick(void *arena)
{
    return new(arena) pfiPick;
}

PFUIDLLEXPORT void
pfiPickMode(pfiPick *pick, int _mode, int _val)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiPickMode -- null pfiPick*");
	return;
    }
    pick->setMode(_mode, _val);
}

PFUIDLLEXPORT int
pfiGetPickMode(pfiPick *pick, int _mode)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetPickMode -- null pfiPick*");
	return -1;
    }
    return pick->getMode(_mode);
}

PFUIDLLEXPORT void
pfiPickHitFunc(pfiPick *pick, pfiPickFuncType _func, void *_data)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiPickHitFunc -- null pfiPick*");
	return;
    }
    pick->setHitFunc(_func, _data);
}

PFUIDLLEXPORT void
pfiGetPicktHitFunc(pfiPick *pick, pfiPickFuncType *_func, void **_data)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetPicktHitFunc -- null pfiPick*");
	return;
    }
    pick->getHitFunc(_func, _data);
}

PFUIDLLEXPORT void
pfiAddPickChan(pfiPick *pick, pfChannel *_chan)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiAddPickChan -- null pfiPick*");
	return;
    }
    pick->addChan(_chan);
}

PFUIDLLEXPORT void
pfiInsertPickChan(pfiPick *pick, int _index, pfChannel *_chan)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInsertPickChan -- null pfiPick*");
	return;
    }
    pick->insertChan(_index, _chan);
}

PFUIDLLEXPORT void
pfiRemovePickChan(pfiPick *pick, pfChannel *_chan)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiRemovePickChan -- null pfiPick*");
	return;
    }
    pick->removeChan(_chan);
}

PFUIDLLEXPORT int
pfiGetPickNumHits(pfiPick *pick)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetPickNumHits -- null pfiPick*");
	return -1;
    }
    return pick->getCount();
}

PFUIDLLEXPORT pfNode*
pfiGetPickNode(pfiPick *pick)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetPickNode -- null pfiPick*");
	return NULL;
    }
    return pick->getNode();
}


PFUIDLLEXPORT pfGeoSet*
pfiGetPickGSet(pfiPick *pick)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetPickGSet -- null pfiPick*");
	return NULL;
    }
    return pick->getGSet();
}

PFUIDLLEXPORT void
pfiSetupPickChans(pfiPick *pick)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiSetupPickChans -- null pfiPick*");
	return;
    }
    pick->setupChans();
}

PFUIDLLEXPORT int
pfiDoPick(pfiPick *pick, int _x, int _y)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiDoPick -- null pfiPick*");
	return -1;
    }
    return pick->doPick(_x, _y);
}

PFUIDLLEXPORT void
pfiResetPick(pfiPick *pick)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetPick -- null pfiPick*");
	return;
    }
    pick->reset();
}


