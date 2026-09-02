/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
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
 *	main.c
*/

#include <stdlib.h>
#include <stdio.h>

/* proceess control includes */
#ifndef WIN32
#include <sys/statfs.h>
#endif
#include <sys/types.h>


/* Performer include files */
/*
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfui.h>
*/
#include <Performer/pfutil/pfuProcessManager.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfPipeWindow.h>

/* Common Code include file */
#include "generic.h"

/* 'custom.h' is provided by the individual applications. It must contain
    the typedef for the ViewState structure
*/
#include "custom.h"


/**************************************************************************
 *									  *
 *		    APPLICATION PROCESS ROUTINES			  *
 *									  * 
 **************************************************************************/


/* frees any restricted/isolated CPUs when app starts up */
int FreeInitCPUs = 1;
int AppCPU=-1, CullCPU=-1, DrawCPU=-1, LPointCPU=-1;

int
main(int argc, char *argv[])
{
    int doProcMngr = 0;

    /* 
     * Initialize OpenGL Performer: create shared memory, initialize clocks, etc.
     * This must be the first libpf call in a libpf application.
    */
    pfInit();

    /* 
     * Allocate and initialize global shared memory before pfConfig 
     * so that forked processes share global pointers. 
    */
    InitSharedMem(argc, argv);
    pfuInit();
    
    /* Set multiprocessing mode and other configuration values. */
    InitConfig();

    /* Setup default process Manager before pfConfig 
     * so that it is visible to all procs 
     */
    if (ViewState->procLock == ((uint)~0) || PrioritizeProcs)
    {
	doProcMngr = 1;
	pfuDefaultProcessManager::init();
    }
   
    /* 
     * Configure OpenGL Performer and fork extra processes if 
     * configured for multiprocessing. 
     */
    pfConfig();
   
    /* Tell ProcessManager to configure the system */
    if (doProcMngr)
    {
	pfuProcessManager::setMode(PFUPM_CONTINUOUS, 1);
	pfuProcessManager::setMode(PFUPM_LOCK_DOWN, 
			ViewState->procLock == ((uint)~0));
	pfuProcessManager::setMode(PFUPM_PRIORITIES, PrioritizeProcs);
	pfuProcessManager::reconfigure();
    }

    /* Initialize OpenGL Performer utility libraries. */
    pfiInit();

    /* Create the visual database. */
    InitScene();

    /* Initialize the rendering pipeline(s). */
    InitPipe();
    
    /* set off first frame of cull and draw processes for initialization 
     * in parallel with app
     */
    pfFrame();
    
    /* 
     * Initialize the graphical user interface (GUI). 
     * Do this before InitChannel so that the GUI is the first channel.
    */
    InitGUI();

    /* Initialize pfChannel(s). */
    InitChannel();

    /* Initialize input handling (X or GL) for mouse and event inputs. */
    if (MultiPipeInput && (pfGetMultipipe() > 1))
	pfuInitMultiChanInput(ViewState->chan, NumChans, ViewState->input);
    else
	pfuInitInput(ViewState->masterChan->getPWin(), ViewState->input);
 
    /* not using process manager - do commandline specified locking*/
    if ((ViewState->procLock != ((uint)~0)) && (AppCPU >= 0))
	pfuLockDownProc(AppCPU);
    
    /* Set the desired frame rate. */
    pfFrameRate(ViewState->frameRate);	

    /* Set the MP synchronization phase. */
    pfPhase(ViewState->phase);		

    /* Application main loop */
    while (!SimDone())
    {
	/* Sleep until next frame */
	pfSync();
	
	/* 
         * Should do all latency critical processing inbetween pfSync()
	 * and pfFrame(). Typically this is viewing position update.
	 */
	PreFrame();

	/* Trigger cull and draw processing for this frame. */
        pfFrame();

	/* Perform non-latency critical simulation updates. */
	PostFrame();
    }
   
    /* 
     * Cleanup and exit.  
     */
    pfuFreeAllCPUs();
    pfuExitInput();
    pfuExitUtil();

    pfExit();

    return(0);
}


/**************************************************************************
 *									  *
 *		    APP PROCESS ROUTINES				  *
 *									  * 
 **************************************************************************/

void
AppFunc(pfChannel *chan, void *data)
{
    PreApp(chan, data);
    pfApp();
    PostApp(chan, data);
}


/**************************************************************************
 *									  *
 *		    CULL PROCESS ROUTINES				  *
 *									  * 
 **************************************************************************/

/*
 * The cull function callback.  Any work that needs to be done
 * in the cull process should happen in this function.
*/
void
CullFunc(pfChannel * chan, void *data)
{
    PreCull(chan, data);

    pfCull();			/* Cull to the viewing frustum */

    PostCull(chan, data);
}


/**************************************************************************
 *									  *
 *		    DRAW PROCESS ROUTINES				  *
 *									  * 
 **************************************************************************/

/*
 * The draw function callback.  I/O with GL devices must happen here.
 * Any GL functionality outside Performer must be done here. 
 * All libpr routines which may be captured by a pfDispList, 
 * e.g.- pfEnable, pfApply*, pfCullFace, pfPushMatrix, should be called 
 * here since they send GL commands to the graphics hardware.
*/
void
DrawFunc(pfChannel *chan, void *data)
{
    PreDraw(chan, data);	/* e.g. - clear the viewport */

    pfDraw();			/* Render the frame */

    PostDraw(chan, data);	/* e.g. - draw HUD, read GL devices */

   {
   int err;
   if ((err = glGetError()) != GL_NO_ERROR)
	pfNotify(PFNFY_WARN,PFNFY_USAGE,"OpenGL Error 0x%x - %s",err, gluErrorString(err));
   }
}

/**************************************************************************
 *                                                                        *
 *                  LPOINT PROCESS ROUTINES                               *
 *                                                                        *
 **************************************************************************/

/* 
 * The lpoint process callback. Configuration of the light points processing
 * can be done here. NO CALL to the graphic or draw function can be made
 * from there.
*/
void
LpointFunc(pfChannel *chan, void *data)
{
	PreLpoint(chan, data);	/* configure the preprocessing */
	
	pfLPoint();		/* do the preprocessing */
	
	PostLpoint(chan, data);	/* after preprocessing */
}
