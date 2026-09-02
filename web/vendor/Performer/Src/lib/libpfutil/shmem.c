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
 * file: shmem.c 	
 * -------------
 * 
 * Shared memory for libpfutil - GLX Input handling,  GUI data, 
 * flight model, etc.
 *
 * $Revision: 1.42 $
 * $Date: 2002/07/26 00:22:42 $
 *
 */


#include <stdlib.h>
#include <string.h>
#ifdef mips
#include <bstring.h>
#endif 

#ifndef WIN32
#include <X11/X.h>
#include <X11/Xlib.h>
#endif

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include "xwin.h"
#include "gui.h"
#include "input.h"
#include "cursor.h"
#include "pfuAutoList.h"
#include "pfuClipCenterNode.h"
#include "pfuProcessManager.h"
#include "pfuTextureManager.h"


	/*---------------------------------------------------------*/


static pfDataPool 	*UtilDP = NULL;
static pfDataPool*	initUtilDPool(void);


	/*---------------------------------------------------------*/

static long DPoolSize = 1 << 17; /* 128K bytes */
			 
PFUDLLEXPORT void
pfuDPoolSize(long size)
{
    DPoolSize = size;
}

PFUDLLEXPORT long
pfuGetDPoolSize(void)
{
    return DPoolSize;
}

static pfDataPool*
initUtilDPool(void)
{
    volatile void  *p;
    int             appid;
    char     	    dpoolName[PF_MAXSTRING];

    /* create the data pool of protected gui data */
    if (!UtilDP)
    {
	appid = (int)pfGetPID(0, PFPROC_APP);
	sprintf(dpoolName, "pfutil%d", appid);

	if (!(UtilDP = pfAttachDPool(dpoolName)))
	{
	    /* Note possible race condition, could cause multiple creators */
	    UtilDP = pfCreateDPool(DPoolSize, dpoolName);
	    if (!UtilDP)
	    {
		pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
			 "pfuInit(): could not make pfUtil Data Pool!");
	    }
	    else
	    {
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
			 "pfuInit(): pid %d created pfUtil data pool: %s", 
			 getpid(), dpoolName);
	    }

	    /* Allocate and zero all of the pfutil regions in the Util data pool */
	    p = pfDPoolAlloc(UtilDP, sizeof(pfList*), PFU_TEXLIST_DATA_DPID);
	    bzero((void *) p, sizeof(pfList*));

	    p = pfDPoolAlloc(UtilDP, sizeof(pfuGLXWindowData), PFU_XWIN_DATA_DPID);
	    bzero((void *) p, sizeof(pfuGLXWindowData));
	    
	    p = pfDPoolAlloc(UtilDP, sizeof(pfuGUI), PFUGUI_DATA_DPID);
	    bzero((void *) p, sizeof(pfuGUI));
	
	    p = pfDPoolAlloc(UtilDP, sizeof(pfuGUIFlags), PFUGUI_FLAGS_DPID);
	    bzero((void *) p, sizeof(pfuGUIFlags));
	
	    p = pfDPoolAlloc(UtilDP, sizeof(pfuGUIPicker), 
			     PFUGUI_PICK_DPID);
	    bzero((void *) p, sizeof(pfuGUIPicker));
	
	    p = pfDPoolAlloc(UtilDP, sizeof(pfuDeviceInput), PFU_XIO_DATA_DPID);
	    bzero((void *) p, sizeof(pfuDeviceInput));
	
	    p = pfDPoolAlloc(UtilDP, sizeof(pfuMouse), PFU_MOUSE_DATA_DPID);
	    bzero((void *) p, sizeof(pfuMouse));
	
	    p = pfDPoolAlloc(UtilDP, sizeof(pfuCursorMgmt), PFU_CURSOR_DATA_DPID);
	    bzero((void *) p, sizeof(pfuCursorMgmt));
	}
    }
    return UtilDP;
}

volatile PFUDLLEXPORT void*
pfuFindUtilDPData(int id)
{
    if (!UtilDP)
	initUtilDPool();
    return pfDPoolFind(UtilDP, id);
}

/* init libpfutil classes */
PFUDLLEXPORT void
pfuInitClasses(void)
{
    pfuInitAutoListClass();
    pfuInitClipCenterNodeClass();
    pfuInitProcessManagerClasses();
    pfuInitTextureManagerClass();
}

PFUDLLEXPORT void
pfuInit(void)
{
    pfuInitClasses();
    
    if (!UtilDP)
	initUtilDPool();
}

/* compatibility entry point */
#ifdef pfuInitUtil
#undef pfuInitUtil
#endif
PFUDLLEXPORT void pfuInitUtil(void)
{
    pfuInit();
}

PFUDLLEXPORT pfDataPool     *
pfuGetUtilDPool(void)
{
    if (UtilDP)
	return UtilDP;
    else
	return initUtilDPool();
}

PFUDLLEXPORT void
pfuExitUtil(void)
{
    /* unlink the datapool */
    pfReleaseDPool(UtilDP);
    UtilDP = NULL;
}

