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
 * pfproc.c - ".proc" pseudo-loader
 *
 * Makes a scene consisting of a single node
 * with callbacks that spin for a number of seconds
 * as specified in the fake "filename" argument,
 * and print output to stderr at the beginning and end of each callback.
 *
 * This can be useful in order to artificially time-extend
 * stages of the pipeline, to simulate various load environments.
 * For example, to show the frame-dropping behavior of LOCK phase,
 * try the following:
 *	perfly -P1 -W600,150 .1,.2,.4.proc <optional other scene files>
 * Notice that the CULL process drops frames rather than making the APP block,
 * and the DRAW drops culled frames rather than making the CULL block.
 */

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <process.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pfdu.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pr/pfGeoSet.h> // for PFBOUND_STATIC

#ifdef __linux__
#define get_pid getpid
#endif
#ifdef WIN32
#define get_pid _getpid
#endif

struct CallbackData { const char *string; float spinTime; };

static int
spinFunc(pfTraverser *, void *data)
{
    const struct CallbackData *cbData = (struct CallbackData *)data;

    pfNotifyLock();
    fprintf(stderr, "%s(%d):%d start spin %f\n", cbData->string, get_pid(), pfGetFrameCount(), cbData->spinTime);
    pfNotifyUnlock();

    double t0 = pfGetTime();
    while (pfGetTime()-t0 < cbData->spinTime)
	;

    pfNotifyLock();
    fprintf(stderr, "%s(%d):%d end spin %f\n", cbData->string, get_pid(), pfGetFrameCount(), cbData->spinTime);
    pfNotifyUnlock();

    return PFTRAV_CONT;
}

extern "C" PFPFB_DLLEXPORT pfNode *
pfdLoadFile_proc(char *fileName)
{
    float appTime, cullTime, drawTime;
    if (sscanf(fileName, "%f,%f,%f", &appTime, &cullTime, &drawTime) != 3)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		"pfdLoadFile_proc: bad file name %s", fileName);
	return NULL;
    }

    pfNode *node = new pfGroup();
    struct CallbackData *buf = (struct CallbackData *)
	    pfMalloc(3*sizeof(struct CallbackData)); // from shared arena
    PFASSERTALWAYS(node != NULL && buf != NULL);

    buf[0].string = "APP";			buf[0].spinTime = appTime;
    buf[1].string = "\t\t\t\tCULL";		buf[1].spinTime = cullTime;
    buf[2].string = "\t\t\t\t\t\t\t\tDRAW";	buf[2].spinTime = drawTime;

    node->setTravFuncs(PFTRAV_APP, spinFunc, NULL);
    node->setTravFuncs(PFTRAV_CULL, spinFunc, NULL);
    node->setTravFuncs(PFTRAV_DRAW, spinFunc, NULL);
    node->setTravData(PFTRAV_APP, &buf[0]);
    node->setTravData(PFTRAV_CULL, &buf[1]);
    node->setTravData(PFTRAV_DRAW, &buf[2]);
    node->setUserData(buf); // so it will get deleted with the node

    pfNotify(PFNFY_INFO, PFNFY_PRINT, "pfdLoadFile_proc: appTime=%f", appTime);
    pfNotify(PFNFY_INFO, PFNFY_MORE,  "                 cullTime=%f", cullTime);
    pfNotify(PFNFY_INFO, PFNFY_MORE,  "                 drawTime=%f", drawTime);

    // Give it physical dimensions
    // so it doesn't get culled away
    pfSphere bsph;
    bsph.center = pfVec3(0,0,0);
    bsph.radius = 1;
    node->setBound(&bsph, PFBOUND_STATIC);

    return node;
}
