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
 *
 * file: eventq.c
 * --------------
 * 
 * $Revision: 1.12 $
 * $Date: 2002/05/15 00:31:09 $
 */

#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#endif  /* __linux__ */
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include "eventq.h"


/*****************************************************************************
 *				Static Declarations
 *****************************************************************************
 */

static pfDataPool   	*UtilDP=NULL;

static void initEventQ(pfuEventQueue *eq, pfDataPool *dp, int id);


/*****************************************************************************
 *				Exposed Functions
 *****************************************************************************
 */


PFUDLLEXPORT pfuEventQueue *
pfuNewEventQ(pfDataPool *dp, int id)
{
    pfuEventQueue *eq=NULL;
    
    if (!dp)
    {
	if (!UtilDP)
	    UtilDP = pfuGetUtilDPool();
	dp = UtilDP;
    }
	
    if (!(eq = (pfuEventQueue *) pfDPoolFind(dp, id)))
    {
	eq = (pfuEventQueue*) pfDPoolAlloc(dp, sizeof(pfuEventQueue), id);
    }
    initEventQ(eq, dp, id);
    return eq;
}


PFUDLLEXPORT void
pfuAppendEventQStream(pfuEventQueue *dst, pfuEventStream *esrc)
{
    int srcNumDevs;
    int dstNumDevs;
    int srcNumKeys;
    int dstNumKeys;
    int i, j;
    pfuEventStream *edst;
    
    if (!esrc)
    {
	return;
    }
    if (!dst)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_USAGE, 
	    "pfuAppendEventQStream: destination event queue is NULL.");
	return;
    } 
    
    pfDPoolLock(dst);
    
    /* get pointer to destination event stream */
    edst = (dst->eventP);

    edst->buttonFlags = esrc->buttonFlags;
    
    if (esrc->numDevs <= 0)
    {
	pfDPoolUnlock(dst); 
	return;
    }
    
    srcNumDevs = esrc->numDevs;
    dstNumDevs = edst->numDevs;
    srcNumKeys = esrc->numKeys;
    dstNumKeys = edst->numKeys;
    
    /* add new events onto end of dst queue */
    if ((srcNumDevs + dstNumDevs) > PFUDEV_MAX_DEVS)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,  
	    "pfuCollectEvents - overflowed device event queue - dropping %d events", 
	    (srcNumDevs + dstNumDevs) - PFUDEV_MAX_DEVS);
	srcNumDevs = PFUDEV_MAX_DEVS - dstNumDevs;
    }
    
    /* copy onto the end of the queues for devs and keys */
    bcopy(esrc->devQ, &edst->devQ[dstNumDevs], srcNumDevs * sizeof(int));
    bcopy(esrc->devVal, &edst->devVal[dstNumDevs], srcNumDevs * sizeof(int));
    if (srcNumKeys > 0)
	bcopy(esrc->keyQ, &edst->keyQ[dstNumKeys], srcNumKeys * sizeof(int));
    edst->numDevs += srcNumDevs;
    edst->numKeys += srcNumKeys;
    
    /* increment the individual device and key counts */
    for (j = 0; j < srcNumDevs; j++)
    {
	int key, dev;
	dev = esrc->devQ[j];
	if (esrc->devCount[dev] > 0)
	{
	    edst->devCount[dev] += esrc->devCount[dev];
	    if (dev == PFUDEV_KEYBD)
	    {
		for (i = 0; i < srcNumKeys; i++)
		{
		    key = esrc->keyQ[i];
		    if (esrc->keyCount[key] > 0)
		    {
			edst->keyCount[key] += esrc->keyCount[key];
			/* reset the key as done */
			esrc->keyCount[key] = 0;
		    }
		}
	    }
	}
	/* mark dev done in src */
	esrc->devCount[dev] = 0;
    } 
    
    pfDPoolUnlock(dst); 
}

PFUDLLEXPORT void
pfuAppendEventQ(pfuEventQueue *dst, pfuEventQueue *src)
{
    if (!src)
	return;
    if (!dst)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_USAGE, 
	    "pfuAppendEventQ: destination event queue is NULL.");
	return;
    }    
    pfDPoolLock(src);
    pfuAppendEventQStream(dst, (src->eventP));
    pfDPoolUnlock(src);
}

/*ARGSUSED*/
PFUDLLEXPORT void
pfuEventQEvent(pfuEventQueue *eq, pfuEvent *ev)
{
    
}

/*ARGSUSED*/
PFUDLLEXPORT void 
pfuGetNextEventQEvent(pfuEventQueue *eq, pfuEvent *ev)
{
    
}

PFUDLLEXPORT void
pfuEventQStream( pfuEventQueue *eq, pfuEventStream *_es)
{
    eq->eventP = _es;
}

PFUDLLEXPORT pfuEventStream *
pfuGetEventQStream( pfuEventQueue *src)
{
    return (src->eventP);
}

PFUDLLEXPORT void
pfuGetEventQEvents(pfuEventStream *dst, pfuEventQueue *src)
{
    pfuEventStream *events;
    if (!src)
	return;
	
    pfDPoolLock(src);
    
    events = src->eventP;
    /* Get a snapshot of events*/
    dst->numKeys = events->numKeys;
    dst->numDevs = events->numDevs;
    dst->buttonFlags = events->buttonFlags;
    bcopy(events->devCount, dst->devCount,  sizeof(dst->devCount));
    bcopy(events->devVal, dst->devVal, sizeof(dst->devVal));
    bcopy(events->keyCount, dst->keyCount, sizeof(dst->keyCount));
    bcopy(events->devQ, dst->devQ, sizeof(dst->devQ));
    bcopy(events->keyQ, dst->keyQ, sizeof(dst->keyQ));
    pfDPoolUnlock(src);
}

PFUDLLEXPORT void
pfuResetEventStream(pfuEventStream *es)
{
    es->numKeys = es->numDevs = 0;
    es->buttonFlags = 0;
    bzero(es->keyCount, PFUDEV_KEY_MAP_SIZE * sizeof(int));
    bzero(es->devCount, PFUDEV_MAX * sizeof(int));
}

PFUDLLEXPORT void
pfuResetEventQ(pfuEventQueue *eq)
{
    pfDPoolLock(eq);
    pfuResetEventStream(eq->eventP);
    pfDPoolUnlock(eq);
}

PFUDLLEXPORT void
pfuIncEventQFrame(pfuEventQueue *eq)
{
    pfuEventStream *events;
    if (!eq)
	return;

    events = eq->eventP;
    events->frameStamp++;
    if (events->frameStamp < 0)
        events->frameStamp = 0;
}

PFUDLLEXPORT void
pfuEventQFrame(pfuEventQueue *eq, int val)
{
     if (!eq || !(eq->eventP))
     {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
		"pfuEventQFrame - NULL event stream ptr eventP.");
	return;
     }
     eq->eventP->frameStamp = val;
}

PFUDLLEXPORT int
pfuGetEventQFrame(pfuEventQueue *eq)
{
    if (!eq || !(eq->eventP))
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfuEventQFrame - NULL event stream ptr eventP.");
	return -1;
    }
    return eq->eventP->frameStamp;
}


PFUDLLEXPORT void
pfuIncEventStreamFrame(pfuEventStream *events)
{
     events->frameStamp++;
     if (events->frameStamp < 0)
	events->frameStamp = 0;
}

PFUDLLEXPORT void
pfuEventStreamFrame(pfuEventStream *events, int val)
{
     events->frameStamp = val;
}

PFUDLLEXPORT int
pfuGetEventStreamFrame(pfuEventStream *events)
{
      return events->frameStamp;
}


/*****************************************************************************
 *				Static Utility Functions
 *****************************************************************************
 */

static void
initEventQ(pfuEventQueue *eq, pfDataPool *dp, int id)
{
    pfDPoolLock(eq);

    eq->id = id;
    eq->dpool = dp;
    eq->eventP = (&eq->eventStream);
    bzero((eq->eventP), sizeof(eq->eventStream));

    pfDPoolUnlock(eq);
}
