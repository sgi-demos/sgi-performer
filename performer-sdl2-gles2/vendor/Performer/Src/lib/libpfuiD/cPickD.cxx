/*
 * WARNING! DO NOT EDIT!
 * cPickD.C automatically generated from (../libpfui/cPick.C)
 */
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
 * pfiPickD Class C-API Wrappers
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
#include <Performer/pfuiD.h>

#include "pfiPickD.h"



PFUIDLLEXPORT pfType*
pfiPickDClassType(void)
{
    return pfiPickD::getClassType();    
}


PFUIDLLEXPORT pfiPickD*		 
pfiNewPickD(void *arena)
{
    return new(arena) pfiPickD;
}

PFUIDLLEXPORT void
pfiPickDMode(pfiPickD *pick, int _mode, int _val)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiPickDMode -- null pfiPickD*");
	return;
    }
    pick->setMode(_mode, _val);
}

PFUIDLLEXPORT int
pfiGetPickDMode(pfiPickD *pick, int _mode)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetPickDMode -- null pfiPickD*");
	return -1;
    }
    return pick->getMode(_mode);
}

PFUIDLLEXPORT void
pfiPickDHitFunc(pfiPickD *pick, pfiPickDFuncType _func, void *_data)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiPickDHitFunc -- null pfiPickD*");
	return;
    }
    pick->setHitFunc(_func, _data);
}

PFUIDLLEXPORT void
pfiGetPickDtHitFunc(pfiPickD *pick, pfiPickDFuncType *_func, void **_data)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetPickDtHitFunc -- null pfiPickD*");
	return;
    }
    pick->getHitFunc(_func, _data);
}

PFUIDLLEXPORT void
pfiAddPickDChan(pfiPickD *pick, pfChannel *_chan)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiAddPickDChan -- null pfiPickD*");
	return;
    }
    pick->addChan(_chan);
}

PFUIDLLEXPORT void
pfiInsertPickDChan(pfiPickD *pick, int _index, pfChannel *_chan)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInsertPickDChan -- null pfiPickD*");
	return;
    }
    pick->insertChan(_index, _chan);
}

PFUIDLLEXPORT void
pfiRemovePickDChan(pfiPickD *pick, pfChannel *_chan)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiRemovePickDChan -- null pfiPickD*");
	return;
    }
    pick->removeChan(_chan);
}

PFUIDLLEXPORT int
pfiGetPickDNumHits(pfiPickD *pick)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetPickDNumHits -- null pfiPickD*");
	return -1;
    }
    return pick->getCount();
}

PFUIDLLEXPORT pfNode*
pfiGetPickDNode(pfiPickD *pick)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetPickDNode -- null pfiPickD*");
	return NULL;
    }
    return pick->getNode();
}


PFUIDLLEXPORT pfGeoSet*
pfiGetPickDGSet(pfiPickD *pick)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetPickDGSet -- null pfiPickD*");
	return NULL;
    }
    return pick->getGSet();
}

PFUIDLLEXPORT void
pfiSetupPickDChans(pfiPickD *pick)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiSetupPickDChans -- null pfiPickD*");
	return;
    }
    pick->setupChans();
}

PFUIDLLEXPORT int
pfiDoPickD(pfiPickD *pick, int _x, int _y)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiDoPickD -- null pfiPickD*");
	return -1;
    }
    return pick->doPick(_x, _y);
}

PFUIDLLEXPORT void
pfiResetPickD(pfiPickD *pick)
{ 
    if (pick == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetPickD -- null pfiPickD*");
	return;
    }
    pick->reset();
}


