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
 *	generic.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#ifndef WIN32
#include <sys/sysmp.h>

/* X Window includes */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif


/* OpenGL Performer includes */
/*
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <Performer/pfui.h>
*/

#include <Performer/pf/pfPipeWindow.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pr/pfLight.h>
#include <Performer/pfdu.h>
#include <Performer/pfui/pfiXformer.h>

/* common includes */
#include "main.h"
#include "custom.h"

#define streq(a,b) (strcmp(a,b) == 0)
#define strsuffix(s, suf) \
	(strlen(s) >= strlen(suf) && streq(s + (strlen(s)-strlen(suf)), suf))

/******************************************************************************
 *			    Global Variables
 *****************************************************************************/

/* Data shared between processes */
SharedViewState	*ViewState;

/*
 * Command line optional settings for initialization. These variables are
 * set before the call to pfConfig and are only used for initialization.
 * So they don't need to be in the ViewState shared memory structure. They
 * are initialized in the function InitSharedMem().
*/
int	WinSizeX		= -1;
int	WinSizeY		= -1;
int	Multipipe		= -1;
int	NumPipes		= -1;
int	NumChans		= -1;
int	MultiPipeInput		= 1;
int	NotifyLevel		= PFNFY_INFO;
int	ProcSplit		= PFMP_DEFAULT;
int	PrioritizeProcs		= 0;
int	GangDraw		= 0;
int	InitHPR			= FALSE;
int	InitXYZ			= FALSE;
char*	WriteFileDbg		= NULL;
int	ChanOrder[MAX_CHANS]	= {-1};
int	InitFOV			= FALSE;


int 	Multisamples = -1;
int	ZBits	     = -1;
#define FBATTR_MSAMPLES	5
#define FBATTR_ZBITS	7
#ifdef WIN32
#define None 0
#endif
static int FBAttrs[] = {
    PFFB_RGBA, 
    PFFB_DOUBLEBUFFER, 
    PFFB_RED_SIZE, 8,
    PFFB_SAMPLES, 8,
    PFFB_DEPTH_SIZE, 15, 
    PFFB_STENCIL_SIZE, 1,
    None,
};

/*********************************************************************
*								     *
*		APPLICATION PROCESS ROUTINES			     *
*								     *
**********************************************************************/

/* The following globals are used by the Application process only */
int	    NumFiles	    = 0;
char	    **DatabaseFiles = NULL;

static int	numScreens;


/*---------------------------------------------------------------------*/


/* Determine if Simulation is finished */
int
SimDone(void)
{
    return(ViewState->exitFlag);
}


/* Initialize MP, system config */
void
InitConfig(void)
{
    int 	numCPUs = 0;
    int 	availCPUs;

    /* Set multiprocessing mode. */
    pfMultiprocess(ProcSplit);

    /* Set number of pfPipes desired. */
    pfMultipipe(NumPipes);

    /* Enable hyperpipe mode */
    if (Multipipe == HYPERPIPE)
    	pfHyperpipe(NumPipes);

    /* Set intersection callback. */
    pfIsectFunc(IsectFunc);

    /* Check for restricted CPUs */
#ifndef WIN32
    availCPUs = sysmp(MP_NAPROCS);
    numCPUs = sysmp(MP_NPROCS);
#else
    {
	SYSTEM_INFO sysInfo;

	GetSystemInfo(&sysInfo);
	availCPUs = numCPUs = sysInfo.dwNumberOfProcessors;
    }
#endif

    if (numCPUs != availCPUs)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE, 
	    "Only %d out of %d processors available on this machine !!!",
			availCPUs, numCPUs);

#ifndef WIN32
	if ((geteuid() == 0) && FreeInitCPUs)
#else
        if (FreeInitCPUs)
#endif
	    pfuFreeAllCPUs();
	else
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Not freeing CPUs.");
    } 
    else
	pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	    "All %d processors available on this machine.", numCPUs);

    /* call initConfig customisation in perfly */
    initConfig();
}


/*
 * Allocate Global pointers and set Global variables-
 * This must be called before pfConfig() if forked processes
 * must share globals and/or pointers to shared memory.
 * Initialize shared memory structures
 * 
 * This queries X for machine config info and currently assumes the 
 * default display and screen.
 * 
 * XXX we really need to do config info after we have the desired gl context
 * so that we can do the necessary OpenGL queries on extensions.
*/
void
InitSharedMem(int &argc, char **&argv)
{
    void 	*arena;
    int		i;
    int 	maxScreenX, maxScreenY;
#ifndef WIN32
    Display	*dsp;
#endif
    int		screen; 

    /* Get pointer to shared memory malloc arena created by pfInit()*/
    arena = pfGetSharedArena();
#ifndef WIN32
    dsp = pfGetCurWSConnection(); /* get display for machine config queries */
    screen = DefaultScreen( dsp );
#else
    screen = 0; /* XXX Alex */
#endif
    /*
     * Malloc structures now, before pfConfig(), so forked processes have same
     * pointers.
    */
    ViewState = (SharedViewState *) pfCalloc(1, sizeof(SharedViewState), arena);

    /* Find out how big the window can be */
#ifndef WIN32
    maxScreenX = DisplayWidth(dsp, screen);
    maxScreenY = DisplayHeight(dsp, screen);
#else
    maxScreenX = GetSystemMetrics(SM_CXSCREEN);
    maxScreenY = GetSystemMetrics(SM_CYSCREEN);
#endif
    /* get number of screens */
#ifdef WIN32
#ifdef SM_CMONITORS
    numScreens = GetSystemMetrics(SM_CMONITORS);
#else
    numScreens = 1; /* XXX Alex -- SM_CMONITORS only for win98/Me ??? */
#endif
#else
    numScreens = ScreenCount(dsp);
#endif

    /*
     * Set window size - this is set here, before the call to pfConfig,
     * so that all processes will get the same copy. These do not need
     * to be in shared memory, because they are initialization values
     * which are not changed during the course of the application
    */
    if ((WinSizeX < 0) || (WinSizeY < 0))
    {	
	/* If no window size specified, set to be max possible */
	WinSizeX   = maxScreenX;
	WinSizeY   = maxScreenY;
    }
    else
    {	
	/* If the window size was set by command line arguments */
	WinSizeX = PF_MIN2(WinSizeX,maxScreenX);
	WinSizeY = PF_MIN2(WinSizeY,maxScreenY);
    }

    /* Initialize the channel-pipe assignment table: 0->0, 1->1, ... */
    for (i = 0; i < MAX_CHANS; i++)
	ChanOrder[i] = i;

    /* Initialize Keyboard and GUI ViewState data */
    initViewState();

    /* Set default multipipe mode */
    Multipipe = (numScreens > 1);

    /* Process the command line args */
    NumFiles = processCmdLine(argc, argv, &DatabaseFiles);

    /* set a path to everywhere we want to look for data.
     * append to any path set from the command line.
     */
    {
    size_t oldLength = 0;
    size_t newLength = 0;
    size_t fullLength = 0;
    const char *oldPath = pfGetFilePath();
    char *fullPath = NULL;
    char *newPath = 
       "."
       ":/usr/share/Performer/data"
       ":/usr/share/Performer/data/clipdata/hunter"
       ":/usr/share/Performer/data/clipdata/moffett"
       ":../data"
       ":../../data"
       ":../../data/polyhedra"
       ":../../../data"
       ":/usr/demos/models"
       ":/usr/demos/data/flt";

    if (oldPath != NULL)
	oldLength = strlen(oldPath);
    if (newPath != NULL)
	newLength = strlen(newPath);
    fullLength = oldLength + newLength;

    if (fullLength > 0)
    {
	/* allocate space for old, ":", new, and ZERO */
	fullPath = (char *)pfMalloc(fullLength + 2, NULL);
	fullPath[0] = '\0';
	if (oldPath != NULL)
	    strcat(fullPath, oldPath);
	if (oldPath != NULL && newPath != NULL)
	    strcat(fullPath, ":");
	if (newPath != NULL)
	    strcat(fullPath, newPath);
	pfFilePath(fullPath);
	pfFree(fullPath);
    }
    }

    /* Set Debug Level */
    pfNotifyLevel(NotifyLevel);

    /* Open loader DSOs before pfConfig() */
    for (i = 0 ; i < NumFiles ; i++)
	if (strsuffix(DatabaseFiles[i], ".perfly"))
	    initConfigFile(DatabaseFiles[i]);
	else
	    pfdInitConverter(DatabaseFiles[i]);

    if (!ViewState->guiFormat)
	ViewState->guiFormat = GUI_VERTICAL;

    /* Figure out the number of channels and pipelines required */
    switch (Multipipe) 
    {
    case TRUE:
	if (NumChans < 0)
	    NumChans = numScreens;
	NumPipes = NumChans;
	if (!ViewState->guiFormat)
	    ViewState->guiFormat = GUI_HORIZONTAL;
	break;
    case FALSE:
	if (NumChans < 0)
	    NumChans = 1;
	else if ((NumChans > 1) && (!ViewState->guiFormat))
		ViewState->guiFormat = GUI_HORIZONTAL;
	NumPipes = 1;
	break;
    }
    ViewState->numChans = NumChans;
}


/* Initialize the visual database */
void
InitScene(void)
{
    int		 loaded		= 0;
    pfScene	*scene		= NULL;
    double	 startTime	= 0.0f;
    double	 elapsedTime	= 0.0f;

    /* Read in any databases specified on cmd line */
    ViewState->scene = scene = new pfScene();

    startTime = pfGetTime();
    
    /* Read all files mentioned on command line */
    loaded = initSceneGraph(scene);

    /* Set up scene graph for collisions */
    if (loaded)
	pfuCollideSetup(scene, PFUCOLLIDE_STATIC, 0xffffffff);

    if (loaded)
	ViewState->texList = pfuMakeSceneTexList(scene);

    /* Initialize EarthSky, fog, and sun model */
    initEnvironment();
    elapsedTime = pfGetTime() - startTime;

    if (loaded)
    {
	pfNotify(PFNFY_INFO, PFNFY_PRINT, "Total scene-graph statistics");
	pfdPrintSceneGraphStats((pfNode *)scene, elapsedTime);
    }
    else
    {
    	pfSphere	bsphere;

	pfNotify(PFNFY_INFO, PFNFY_PRINT, "No Databases loaded");
	pfNotify(PFNFY_INFO, PFNFY_MORE, NULL);

	/* Set a default bounding sphere */
	bsphere.center.set(0.0f, 0.0f, 0.0f);
	bsphere.radius = BOUND;
	scene->setBound(&bsphere, PFBOUND_DYNAMIC);
    }

    /* Write out nodes in scene (for debugging) */
    if (WriteFileDbg)
    {
	if (!strstr(WriteFileDbg, ".out") && pfdInitConverter(WriteFileDbg))
	{
	    /* it's a valid extension, go for it */
	    if (!pfdStoreFile((pfNode *)scene, WriteFileDbg))
		pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
			 "No store function available for %s\n", WriteFileDbg);
	}
	else 
	{
	    /* write the file as ASCII */
	    FILE *fp;
	    if (fp = fopen(WriteFileDbg, "w"))
	    {
		pfPrint(scene, PFTRAV_SELF|PFTRAV_DESCEND, PFPRINT_VB_OFF, fp);
		fclose(fp);
	    }
	    else
		pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
			 "Could not open scene.out for debug printing.");
	}
    }

    /* Set initial view position and angles */
    initView(scene);
}


/* Initialize graphics pipeline(s) */
void
InitPipe(void)
{
    pfPipeWindow *pw;
    int i;
    char str[PF_MAXSTRING];
    
    for (i=0; i < NumPipes; i++)
    {
	if ((NumPipes > 1) || (numScreens == 1))
	    pfGetPipe(i)->setScreen(PF_MIN2(i, numScreens-1));	
	ViewState->pw[i] = pw = new pfPipeWindow(pfGetPipe(i));
	sprintf(str,"OpenGL Performer [pipe %d]", i);
	pw->setName(str);
	pw->setOriginSize(0, 0, WinSizeX, WinSizeY);
	if (ViewState->input != PFUINPUT_GL)
	    pw->setConfigFunc(OpenXWin);
	else
	    pw->setConfigFunc(OpenWin);
	if (ViewState->visualIDs[i] > -1)
	    pw->setFBConfigId(ViewState->visualIDs[i]);
        pw->config();
    }
    
    pfStageConfigFunc(-1, PFPROC_DRAW, OpenPipeline);
    pfStageConfigFunc(-1, PFPROC_CULL, ConfigCull);
    pfStageConfigFunc(-1, PFPROC_LPOINT, ConfigLPoint);
    pfConfigStage(-1, PFPROC_CULL | PFPROC_DRAW | PFPROC_LPOINT);

    pfNotify(PFNFY_INFO,  PFNFY_PRINT,
		"Initialized %d Pipe%s", NumPipes, (NumPipes < 2) ? "" : "s");
    pfNotify(PFNFY_INFO,  PFNFY_MORE, NULL);
}


/* Initialize GUI Panel and widget callback routines */
void
InitGUI(void)
{
    pfPipe		*p;
    int		   	masterPipe;
    
    if (Multipipe)
    	masterPipe = ChanOrder[(NumChans - 1) >> 1];
    else
    	masterPipe = 0;

    /* Init GUI and internal data for pipe of master channel */
    p = pfGetPipe(masterPipe);
    pfuInitGUI(p->getPWin(0));

    /* Set the initial enable for the GUI */
    pfuEnableGUI(ViewState->gui);
}


/* Initialize Viewing Channel(s) */
void
InitChannel(void)
{
    int		    	i;
    pfFrameStats    	*frameStats;
    pfChannel	    	*chan;
    pfVec3   		xyz, hpr;
    float		fov;

    if (Multipipe)
    {
	/* Distribute channels across pipes as defined by ChanOrder table */
	for (i=0; i< NumChans; i++)
	    ViewState->chan[i] = new pfChannel(pfGetPipe(i));
    }
    else
    {
	/* Create all channels on the same pipe */
	for (i=0; i< NumChans; i++)
	    ViewState->chan[i] = new pfChannel(pfGetPipe(0));
	/* Config channel viewports for the Multichannel Option board */
	if ((NumChans > 1) || ViewState->MCO)
	{
	    if (!ViewState->MCO)
		pfuConfigMCO(ViewState->chan, NumChans);
	    else
		pfuConfigMCO(ViewState->chan, -NumChans);
	}
    }
	
    /* 
     * Disable sorting for OPAQUE bin in CULLoDRAW mode to 
     * improve overlap and parallelism between CULL and DRAW.
     * Still draw transparent geometry last though.
    */
    if (ProcSplit != PFMP_DEFAULT &&
	(ProcSplit & (PFMP_CULLoDRAW | PFMP_FORK_DRAW)) == 
		     (PFMP_CULLoDRAW | PFMP_FORK_DRAW))
    {
	for (i=0 ; i < NumChans ; i++)
	    ViewState->chan[i]->setBinOrder(PFSORT_OPAQUE_BIN, PFSORT_NO_ORDER);
    }

    /* Keep user-provided channel ordering within bounds */
    for (i=0 ; i < NumChans ; i++)
	ChanOrder[i] = PF_MIN2(NumChans-1, ChanOrder[i]);

    /* Master channel is source for keyboard and mouse input */
    ViewState->masterChan = ViewState->chan[ChanOrder[(NumChans - 1) >> 1]];
    
    /* If multipipe, then we want channels to share their viewports */
    if (Multipipe)
    {
   	int		share;

	/* Get the default */
	share = ViewState->masterChan->getShare();

	/* Add in the viewport share bit */
	share |= PFCHAN_VIEWPORT;

	if (GangDraw)
	{
	    /* Add GangDraw to share mask */
	    share |= PFCHAN_SWAPBUFFERS_HW;
	}
	ViewState->masterChan->setShare(share);
    }

    /* Attach channels to master channel to form a channel group. */
    for (i=0; i< NumChans; i++)
	if (ViewState->chan[i] != ViewState->masterChan)
	    ViewState->masterChan->attach(ViewState->chan[i]);

    /* Make sure we don't wrap field-of-view */
    if (NumChans * 45.0f > 360.0f)
	fov = 360.0f / NumChans;
    else
    	fov = 45.0f;

    /* if channel offset is supplied, use it instead of default */
    if(InitFOV) {
	fov = ViewState->fov[2];
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		 "Using user-supplied offset of %f\n",
		 fov);
    }
    if (Multipipe != HYPERPIPE)
    {
    	/* Set each channel's viewing offset */
    	xyz.set(0.0f, 0.0f, 0.0f);

	/* Horizontally tile the channels from left to right */
    	for (i=0; i< NumChans; i++)
    	{
	    hpr.set((((NumChans - 1)*0.5f) - i)*fov, 0.0f, 0.0f);
	    ViewState->chan[ChanOrder[i]]->setViewOffsets(xyz, hpr);
    	}
    }

    chan = ViewState->masterChan;

    {
	int             share;

        /* Get the default */
        share = ViewState->masterChan->getShare();
	share &= ~PFCHAN_APPFUNC;
        ViewState->masterChan->setShare(share);
    }
    /* Set the callback routines for the pfChannel */
    chan->setTravFunc(PFTRAV_APP, AppFunc);
    chan->setTravFunc(PFTRAV_CULL, CullFunc);
    chan->setTravFunc(PFTRAV_DRAW, DrawFunc);
    chan->setTravFunc(PFTRAV_LPOINT, LpointFunc);

    /* Attach the visual database to the channel */
    chan->setScene(ViewState->scene);

    /* Attach the EarthSky model to the channel */
    chan->setESky(ViewState->eSky);

    /* Initialize the nearPlane and farPlane clipping planes */
    chan->setNearFar(ViewState->nearPlane, ViewState->farPlane);

    /* Vertical FOV is matched to window aspect ratio. */
    chan->setAutoAspect(PFFRUST_CALC_VERT);
    chan->makeSimple(fov);

    /* Initialize the viewing position and direction */
    chan->setView(ViewState->initViews[0].xyz, ViewState->initViews[0].hpr);

    /* custom intiailization */
    initChannel(chan);

    /* Set up Stats structure */
    frameStats = chan->getFStats();
    frameStats->setAttr(PFFSTATS_UPDATE_SECS, 2.0f);
    frameStats->setClass(PFSTATS_ENGFX, PFSTATS_ON);
    frameStats->setClass(PFFSTATS_ENDB, PFSTATS_ON);

    /* For GLX windows and X input, get the window info on the master chan */
    if (ViewState->input == PFUINPUT_X)
    {
	pfuGetGLXWin(ViewState->masterChan->getPipe(), 
		&(ViewState->xWinInfo));
    }
    /* Call custom initGUI routine with the pipe of the master chan */
    initGUI(ViewState->masterChan);

    /* Initialize model for interactive motion */
    initXformer();

    pfNotify(PFNFY_INFO,  PFNFY_PRINT,
	"Initialized %d Channel%s", NumChans, (NumChans < 2) ? "" : "s");
    pfNotify(PFNFY_INFO,  PFNFY_MORE, NULL);
}


/*
 * All latency critical processing should be done here. This is
 * typically the viewing position. One should also read latency critical
 * devices here.
*/
void
PreFrame(void)
{
    /* Poll the mouse buttons and position */
    pfuGetMouse(&ViewState->mouse);

    /* Update the simulation */
    localPreFrame(ViewState->masterChan);

    /* if using real-time input device for view, update view here */
    if (ViewState->rtView)
	updateView(ViewState->masterChan);
}


/* Application simulation update is done here */
void
PostFrame(void)
{
    /* Get snapshot of event/key queues */
    pfuGetEvents(&ViewState->events);

    /* if doing app traversal using view, want to update view here */
    if (!ViewState->rtView)
	updateView(ViewState->masterChan);

    /* Update the simulation state */
    updateSim(ViewState->masterChan);
}


/*************************************************************************
 *									 *
 *		    ISECT PROCESS ROUTINES				 *
 *									 *
 *************************************************************************/

/*
 * Collide the viewing position represented by the pfiXformer with the
 * scene.
*/
void
IsectFunc(void *)
{
    if (ViewState->xformer)
	ViewState->xformer->collide();
}

/*************************************************************************
 *									 *
 *		    APP PROCESS ROUTINES				 *
 *									 *
 *************************************************************************/

/* In App callback routine, pre-pfApp() processing */
void
PreApp(pfChannel *chan, void *data)
{
    preApp(chan, data);
    return;
}


/* In App callback routine, post-pfApp() processing */
void
PostApp(pfChannel *chan, void *data)
{
    postApp(chan, data);
    return;
}

/*************************************************************************
 *									 *
 *		    CULL PROCESS ROUTINES				 *
 *									 *
 *************************************************************************/

/* ARGSUSED (suppress compiler warn) */
void
ConfigCull(int , unsigned int )
{
    if ((pfGetMPBitmask() & PFMP_FORK_CULL) &&
	(ViewState->procLock & PFMP_FORK_CULL) &&
	(CullCPU > -1))
	    pfuLockDownProc(CullCPU);
}

void
ConfigLPoint(int , unsigned int )
{
    if ((pfGetMPBitmask() & PFMP_FORK_LPOINT) &&
	(ViewState->procLock & PFMP_FORK_LPOINT) &&
	(LPointCPU > -1))
	    pfuLockDownProc(LPointCPU);
}

/* In Lpoint callback routine, pre-pfLpoint processing */
void
PreLpoint(pfChannel *chan, void *data)
{
    preLpoint(chan, data);
    return;
}

/* In Lpoint callback routine, post-pfLpoint processing */
void
PostLpoint(pfChannel *chan, void *data)
{
    postLpoint(chan, data);
    return;
}

/* In Cull callback routine, pre-pfCull() processing */
void
PreCull(pfChannel *chan, void *data)
{
    preCull(chan, data);
    return;
}


/* In Cull callback routine, post-pfCull() processing */
void
PostCull(pfChannel *chan, void *data)
{
    postCull(chan, data);
    return;
}


/*****************************************************************************
 *									     *
 *		DRAW PROCESS ROUTINES					     *
 *									     *
 *****************************************************************************/

/* Initialize window */

char const WinName[80] = "OpenGL Performer";

/*
 * Initialize Draw Process and Pipe
 */
void
OpenPipeline(int, unsigned int)
{
	/* Not using pfu-Process manager - do cmdline specified locking */
    if ((ViewState->procLock & PFMP_FORK_DRAW) && (DrawCPU > -1))
	pfuLockDownProc(DrawCPU);
}

/* 
 * Create and configure an X window.
 */

void
OpenXWin(pfPipeWindow *pw)
{
    /* -1 -> use default screen or that specified by shell DISPLAY variable */
    pfuGLXWindow 	*win;
    pfPipe  		*p;
    pfFBConfig 		fbc;
    
    /* use pfuChooseFBConfig to limit config to that of FBAttrs */
    if (Multisamples > -1)
	FBAttrs[FBATTR_MSAMPLES] = Multisamples;
    if (ZBits > -1)
	FBAttrs[FBATTR_ZBITS] = ZBits;
    fbc = pfuChooseFBConfig(NULL, pw->getScreen(), FBAttrs, pfGetSharedArena());
    if (fbc)
#ifndef WIN32
	pw->setFBConfig(fbc);
#else
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfPipeWindow::setFBConfig() stubbed out on win32 ...");
#endif

    p = pw->getPipe();
    if (!(win = pfuGLXWinopen(p, pw, WinName)))
        pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
	    "OpenXPipeline: Could not create GLX Window.");

    /* set initial modes and welcome screen */
    InitGfx(pw);

}

/* 
 * Create and configure a GL window.
*/
void
OpenWin(pfPipeWindow *pw)
{
    pw->open();
    /* set initial modes and welcome screen */
    InitGfx(pw);
}


/* Initialize graphics state */
void
ConfigPWin(pfPipeWindow *pw)
{
    
    /* set up default lighting parameters */
    pfLightModel	*lm;
    pfMaterial 	*mtl;

    /* 
     * Set up default material in case the database does not
     * have any.  Use PFMTL_CMODE_AD so pfGeoSet colors supply the     
     * ambient and diffuse material color. 
    */
    mtl = new pfMaterial();
    mtl->setColorMode(PFMTL_FRONT, PFMTL_CMODE_AD);
    mtl->apply();

    /* Set up default lighting model. */
    lm = new pfLightModel();
    lm->setAmbient(0.0f, 0.0f, 0.0f);

    /* 
     * Local viewer is required so that lighting matches across adjacent
     * channels since rotational offsets are encoded in the viewing 
     * matrix because GL will not fog channels properly if the offsets
     * are encoded in the projection matrix. 
    */
    if (NumChans > 1)
	lm->setLocal(1);

    lm->apply();
    if (!ViewState->lighting)
	pfDisable(PFEN_LIGHTING);
    
    
    /* Prebind textures to be used in simulation */
    if (!(pw->getWinType() & PFPWIN_TYPE_STATS))
    {
	pfEnable(PFEN_TEXTURE);
	pfuDownloadTexList(ViewState->texList, PFUTEX_SHOW);
    }
}

void
InitGfx(pfPipeWindow *pw)
{

    /* Set up default state values.  */
    ConfigPWin(pw);

    /* call custom routine to initialize pipe */
    initPipe(pw);
    
}


/* In Draw callback routine, pre-pfDraw() processing */
void
PreDraw(pfChannel *chan, void *data)
{
    pfPipe *pipe = chan->getPipe();
    int pipeId = pfGetId(pipe);

    /*
     * If this pipe does not display the GUI, then clear the space where
     * the GUI would ordinarily be.
     * if we have DVR, we have to apply our resize by the GUI viewport size
    */
    /* XXX should really only do this once per pipe !!! */
    if ((Multipipe == 1) && ViewState->gui &&
	    (chan != ViewState->masterChan) && 
	    (ViewState->redrawOverlay || 
		(ViewState->drawFlags[pipeId] & REDRAW_WINDOW)))
    {
	pfChannel	*gchan;
	pfPipeVideoChannel *pvchan;
	int		xo, yo, xs, ys;
	float		xscale, yscale;

	pvchan = chan->getPVChan();
	pvchan->getScale(&xscale, &yscale);
	
	gchan = pfuGetGUIChan();
	if (gchan)
	{
	    gchan->getOrigin(&xo, &yo);
	    gchan->getSize(&xs, &ys);
	    xo = xo*xscale + 0.5;
	    xs = xs*xscale + 0.5;
	    yo = yo*yscale + 0.5;
	    ys = ys*yscale + 0.5;
	}
	else
	{
	    xo = yo = 0;
	    pipe->getSize(&xs, &ys);
	}
	if (xs > 0)
	{
            glPushAttrib(GL_SCISSOR_BIT);
	    glScissor(xo, yo, xs, ys);
	    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	    glClear(GL_COLOR_BUFFER_BIT);
	    glPopAttrib();
	}
    }

    /* Clear the frame buffer */
    chan->clear();

    /* Update Draw modes that have been changed in ViewState structure. */
    localPreDraw(chan, data);
}


/* In Draw callback routine, post-pfDraw() processing */
void
PostDraw(pfChannel *chan, void *data)
{
    /* Display Stats, if active */
    if (ViewState->stats)
	chan->drawStats();

    /*
     * Read GL keyboard, mouse. Make sure we only collect input on
     * master channel. If hyperpiping we must collect on all pipes.
    */
    if (chan == ViewState->masterChan)
    	pfuCollectInput();

    /* Call local post draw routine */
    localPostDraw(chan, data);
}
