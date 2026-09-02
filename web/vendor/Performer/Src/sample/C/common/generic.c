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
#endif

#ifndef WIN32
/* X Window includes */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif


/* OpenGL Performer includes */
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <Performer/pfui.h>

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
int	ChanPipes[MAX_CHANS]	= {-1};
int	InitFOV			= FALSE;
int     ChooseShaderVisual      = 0;

int NumHyperpipes		= -1;
int NumHyperpipeSingles		= 0;
int TotHyperpipePipes		= -1;
pfCompositor *compositor 	= NULL;

int NumHyperpipePipes[MAX_HYPERPIPES];
int HyperpipeNetIds[MAX_HYPERPIPES];
char* HyperpipeSingles[MAX_PIPES];

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
    None
};

#ifndef WIN32
static int ShaderFBAttrs[] = {
  PFFB_RGBA, 
  PFFB_DOUBLEBUFFER, 
  PFFB_RED_SIZE, 8,
  PFFB_SAMPLES, 0,
  PFFB_DEPTH_SIZE, 15, 
  PFFB_STENCIL_SIZE, 8, 
  PFFB_ALPHA_SIZE, 8,
  None
};
#else
static int ShaderFBAttrs[] = {
  PFFB_RGBA, 
  PFFB_DOUBLEBUFFER, 
  PFFB_RED_SIZE, 1,
  PFFB_DEPTH_SIZE, 1, 
  PFFB_STENCIL_SIZE, 1, 
  PFFB_ALPHA_SIZE, 1,
  None
};
#endif

#if !defined(IRISGL) && defined(GLX_SGIX_hyperpipe)
static GLXHyperpipeNetworkSGIX *HyperpipeNet = NULL;
static int NumHyperpipeNet = 0;
#endif

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
    if (Multipipe == HYPERPIPE) {
	int i;
	for (i = 0; i < NumHyperpipes; i++)
	    pfHyperpipe(NumHyperpipePipes[i]);
    }

    if (Multipipe == COMPOSITOR) {
	int i;
	compositor = pfNewCompositor();
	for (i = 0; i < 4; i++)
	{
		pfCompositorAddChild(compositor, i);
	}

	pfCompositorVal(compositor, PFLOAD_COEFF, 0.1);
	pfHyperpipe2D(compositor);
    }

    /* Set intersection callback. */
    pfIsectFunc(IsectFunc);

    /* Check for restricted CPUs */
#ifndef WIN32
    availCPUs = (int)sysmp(MP_NAPROCS);
    numCPUs = (int)sysmp(MP_NPROCS);
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
        if(FreeInitCPUs)
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
InitSharedMem(int argc, char *argv[])
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
#ifdef WIN32
    maxScreenX = GetSystemMetrics(SM_CXSCREEN);
    maxScreenY = GetSystemMetrics(SM_CYSCREEN);   
#else
    maxScreenX = DisplayWidth(dsp, screen);
    maxScreenY = DisplayHeight(dsp, screen);
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
	ChanOrder[i] = ChanPipes[i] = i;

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
     case HYPERPIPE:
     case COMPOSITOR:
	 {
#if defined(GLX_SGIX_hyperpipe)
	    int i, lPipe;
	    int hasHyperpipe, numNets;

	    pfQueryFeature(PFQFTR_HYPERPIPE, &hasHyperpipe);
	    if (!hasHyperpipe) {
		pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
			"This machine does not support hyperpipe extension");
		exit(1);
	    }

	    TotHyperpipePipes = 0;
	    if (NumHyperpipes != 0) {
		HyperpipeNet = glXQueryHyperpipeNetworkSGIX(dsp,
			&NumHyperpipeNet);
		if (NumHyperpipeNet == 0) {
		    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
			    "This machine has no hyperpipes");
		    exit(1);
		}

		/* check the requested configuration */
		for (i = 0, numNets = -1; i < NumHyperpipeNet; i++)
		    if (numNets < HyperpipeNet[i].networkId)
			numNets = HyperpipeNet[i].networkId;
		numNets += 1;

		if (NumHyperpipes == -1) {
		    NumHyperpipes = numNets;
		    for (i = 0; i < NumHyperpipes; i++)
			HyperpipeNetIds[i] = i;
		} else {
		    for (i = 0; i < NumHyperpipes; i++)
			if (HyperpipeNetIds[i] < 0 ||
				HyperpipeNetIds[i] >= numNets) {
			    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
				"Hyperpipe net %d not available",
				HyperpipeNetIds[i]);
			    exit(1);
			}
		}

		/* count the pipes */
		for (i = 0; i < NumHyperpipes; i++) {
		    int j;
		    int netid = HyperpipeNetIds[i];
		    int hpipeCount = 0;
		    for (j = 0; j < NumHyperpipeNet; j++)
			if (netid == HyperpipeNet[j].networkId)
			    hpipeCount++;

		    TotHyperpipePipes += NumHyperpipePipes[i] = hpipeCount;
		}
	    }

	    NumPipes = TotHyperpipePipes + NumHyperpipeSingles;
            if(Multipipe == COMPOSITOR)
            {
	        NumPipes = 4;
	        NumChans = NumPipes;
            }
            else
            {
	        NumChans = NumHyperpipes + NumHyperpipeSingles;
            }

	    /* correct the ChanPipes mapping to channels for hyperpipe */
	    for (i = lPipe = 0; i < NumPipes; lPipe++) {
		ChanPipes[lPipe] = i;
		i += lPipe < NumHyperpipes ? NumHyperpipePipes[lPipe] : 1;
	    }

	    if (NumPipes <= 0) {
		pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
			    "No pipes configured for hyperpipe mode");
		exit(1);
	    }
#else
	    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
		    "Application not built with hyperpipe support\n");
	    exit(1);
#endif
	 }
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
    ViewState->scene = scene = pfNewScene();

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
	pfSetVec3(bsphere.center, 0.0f, 0.0f, 0.0f);
	bsphere.radius = BOUND;
	pfNodeBSphere(scene, &bsphere, PFBOUND_DYNAMIC);
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
    int i, lPipe;
    char str[PF_MAXSTRING];
    
    /* init the pipe screens before pipe initialization for hyperpipe */
#if defined(GLX_SGIX_hyperpipe)
    if (Multipipe == HYPERPIPE || Multipipe == COMPOSITOR) {
	int j;
	lPipe = 0;

	/* map the hyperpipe pipes to pfPipes */
	for (i=0; i < NumHyperpipes; i++) {
	    int netid = HyperpipeNetIds[i];
	    for (j=0; j < NumHyperpipeNet; j++)
		if (netid == HyperpipeNet[j].networkId)
		    pfPipeWSConnectionName(pfGetPipe(lPipe++),
			    HyperpipeNet[j].pipeName);
	}

	/* map the single pipes to pfPipes */
	for (j=0; lPipe < NumPipes; lPipe++, j++)
	    pfPipeWSConnectionName(pfGetPipe(lPipe), HyperpipeSingles[j]);
    } else
#endif
	for (i=0; i < NumPipes; i++)
	    if ((NumPipes > 1) || (numScreens == 1))
		pfPipeScreen(pfGetPipe(i), PF_MIN2(i, numScreens-1));

    i = lPipe = 0;
    while (i < NumPipes)
    {
	int pipeCount;
	if(Multipipe == COMPOSITOR)
	{
	    pipeCount = 1;
	}
	else
	{
	    pipeCount = lPipe < NumHyperpipes ? NumHyperpipePipes[lPipe] : 1;
	}

	ViewState->pw[i] = pw = pfNewPWin(pfGetPipe(i));
	sprintf(str,"OpenGL Performer [pipe %d]", i);
	pfPWinName(pw,str);
	pfPWinOriginSize(pw, 0, 0, WinSizeX, WinSizeY);
	if (ViewState->input != PFUINPUT_GL)
	    pfPWinConfigFunc(pw, OpenXWin);
	else
	    pfPWinConfigFunc(pw, OpenWin);

	{
	    /* init pipe specific visual information in pipewindows */
	    int j;
	    for (j = 0; j < pipeCount; j++)
		if (ViewState->visualIDs[i+j] > -1) {
		    pfPipeWindow* tpw = pfGetPipePWin(pfGetPipe(i+j), 0);
		    pfPWinFBConfigId(tpw, ViewState->visualIDs[i+j]);
		}
	}

	pfConfigPWin(pw);

	i += pipeCount;
	lPipe++;
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

    if (Multipipe && Multipipe != COMPOSITOR)
	masterPipe = ChanPipes[ChanOrder[(NumChans - 1) >> 1]];
    else 
    	masterPipe = 0;

    /* Init GUI and internal data for pipe of master channel */
    if(Multipipe == COMPOSITOR)
    {   /* open a GUI on each composited pipe */
        int i = 1;

        p = pfGetPipe(masterPipe);
        pfuInitGUI(pfGetPipePWin(p, 0));
        for(i=1; i<4; i++)
        {
            pfChannel *chan;
            p = pfGetPipe(i);
            chan = pfNewChan(p);
            pfMakeOrthoChan(chan, 0, 280, 0, 1024);
            pfChanNearFar(chan, -1, 1);
        }
	pfCompositorChannelClipped(compositor, 0, PF_OFF);
    }
    else
    {
        p = pfGetPipe(masterPipe);
        pfuInitGUI(pfGetPipePWin(p, 0));
    }

    /* Set the initial enable for the GUI */
    pfuEnableGUI(ViewState->gui);
}


/* Initialize Viewing Channel(s) */
void
InitChannel(void)
{
    int		    	i;
    pfPipe		*pipe;
    pfFrameStats    	*frameStats;
    pfChannel	    	*chan;
    pfVec3   		xyz, hpr;
    float		fov;

    if (Multipipe)
    {
	/* Distribute channels across pipes as defined by ChanOrder table */
	if (Multipipe != HYPERPIPE ) {
	    for (i=0; i< NumChans; i++)
		ViewState->chan[i] = pfNewChan(pfGetPipe(i));
	}
	else
	{
	    int pipeIndex = 0;
	    for (i=0; i< NumChans; i++) {
		ViewState->chan[i] = pfNewChan(pfGetPipe(pipeIndex));
		pipeIndex += i < NumHyperpipes ? NumHyperpipePipes[i] : 1;
	    }
	}
    }
    else 
    {
	pipe = pfGetPipe(0);
	for (i=0; i< NumChans; i++)
	    ViewState->chan[i] = pfNewChan(pipe);
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
	    pfChanBinOrder(ViewState->chan[i], PFSORT_OPAQUE_BIN, PFSORT_NO_ORDER);
    }

    /* Keep user-provided channel ordering within bounds */
    for (i=0 ; i < NumChans ; i++)
	ChanOrder[i] = PF_MIN2(NumChans-1, ChanOrder[i]);

    /* Master channel is source for keyboard and mouse input */
    ViewState->masterChan = ViewState->chan[ChanOrder[(NumChans - 1) >> 1]];
    if(compositor)
    {
        ViewState->masterChan = ViewState->chan[0];
    	pfCompositorMasterPipe(compositor, pfGetChanPipe(ViewState->masterChan));
    }
    
    /* If multipipe, then we want channels to share their viewports */
    if (Multipipe)
    {
   	int		share;

	/* Get the default */
	share = pfGetChanShare(ViewState->masterChan);

	/* Add in the viewport share bit */
	share |= PFCHAN_VIEWPORT;

	if(Multipipe == COMPOSITOR)
	{
		GangDraw = 1; 
		share = share & ~PFCHAN_FOV;
	    	share |= PFCHAN_NEARFAR;
	}

	if (GangDraw)
	{
	    /* Add GangDraw to share mask */
	    share |= PFCHAN_SWAPBUFFERS_HW;
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
	    "!!!!! Asking for gang draw !!!!!!!");
	}
	pfChanShare(ViewState->masterChan, share);
    }

    /* Attach channels to master channel to form a channel group. */
    for (i=0; i< NumChans; i++)
	if (ViewState->chan[i] != ViewState->masterChan)
	    pfAttachChan(ViewState->masterChan, ViewState->chan[i]);

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
    if (Multipipe != HYPERPIPE && Multipipe != COMPOSITOR)
    {
    	/* Set each channel's viewing offset */
    	pfSetVec3(xyz, 0.0f, 0.0f, 0.0f);

	/* Horizontally tile the channels from left to right */
    	for (i=0; i< NumChans; i++)
    	{
	    pfSetVec3(hpr, (((NumChans - 1)*0.5f) - i)*fov, 0.0f, 0.0f);
	    pfChanViewOffsets(ViewState->chan[ChanOrder[i]], xyz, hpr);
    	}
    }

    chan = ViewState->masterChan;

    {
	int             share;

        /* Get the default */
        share = pfGetChanShare(ViewState->masterChan);
	share &= ~PFCHAN_APPFUNC;
        pfChanShare(ViewState->masterChan, share);
    }
    /* Set the callback routines for the pfChannel */
    pfChanTravFunc(chan, PFTRAV_APP, AppFunc);
    pfChanTravFunc(chan, PFTRAV_CULL, CullFunc);
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawFunc);
    pfChanTravFunc(chan, PFTRAV_LPOINT, LpointFunc);

    /* Attach the visual database to the channel */
    pfChanScene(chan, ViewState->scene);

    /* Attach the EarthSky model to the channel */
    pfChanESky(chan, ViewState->eSky);

    /* Initialize the near and far clipping planes */
    pfChanNearFar(chan, ViewState->nearPlane, ViewState->farPlane);

    /* Vertical FOV is matched to window aspect ratio. */
    pfChanAutoAspect(chan, PFFRUST_CALC_VERT);
    pfMakeSimpleChan(chan, fov);

    /* Initialize the viewing position and direction */
    pfChanView(chan, ViewState->initViews[0].xyz, ViewState->initViews[0].hpr);

    /* custom intiailization */
    initChannel(chan);

    /* Set up Stats structure */
    frameStats = pfGetChanFStats(chan);
    pfFStatsAttr(frameStats, PFFSTATS_UPDATE_SECS, 2.0f);
    pfFStatsClass(frameStats, PFSTATS_ENGFX, PFSTATS_ON);
    pfFStatsClass(frameStats, PFFSTATS_ENDB, PFSTATS_ON);

    /* For GLX windows and X input, get the window info on the master chan */
    if (ViewState->input == PFUINPUT_X)
    {
	pfuGetGLXWin(pfGetChanPipe(ViewState->masterChan), 
		&(ViewState->xWinInfo));
    }
    /* Call custom initGUI routine with the pipe of the master chan */
    initGUI(ViewState->masterChan);

    /* Initialize model for interactive motion */
    initXformer();

    if(compositor)
    {
    	pfCompositorReconfig(compositor);
    }

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
/* ARGSUSED (suppress compiler warn) */
void
IsectFunc(void *data)
{
    if (ViewState->xformer)
#ifdef  __PFUI_D_H__
	pfiCollideXformerD(ViewState->xformer);
#else
	pfiCollideXformer(ViewState->xformer);
#endif
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
ConfigCull(int pipe, unsigned int stage)
{
    if ((pfGetMPBitmask() & PFMP_FORK_CULL) &&
	(ViewState->procLock & PFMP_FORK_CULL) &&
	(CullCPU > -1))
	    pfuLockDownProc(CullCPU);
}

/* ARGSUSED (suppress compiler warn) */
void
ConfigLPoint(int pipe, unsigned int stage)
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
/* ARGSUSED (suppress compiler warn) */
void
OpenPipeline(int pipe, unsigned int stage )
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
    if(ChooseShaderVisual) {
      pfNotify(PFNFY_INFO,PFNFY_PRINT, "Doing shader attrs\n");
      
      pfPWinFBConfigAttrs(pw, ShaderFBAttrs);
    } 
    else {
      if (Multisamples > -1)
	FBAttrs[FBATTR_MSAMPLES] = Multisamples;
      if (ZBits > -1)
	FBAttrs[FBATTR_ZBITS] = ZBits;
      fbc = pfuChooseFBConfig(NULL, pfGetPWinScreen(pw), FBAttrs, pfGetSharedArena());
      if (fbc)
	{
#ifndef WIN32
	  pfPWinFBConfig(pw, fbc);
#else
	  pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfPWinFBConfig() stubbed out on win32 ... ");
#endif
	}
    }
    
    p = pfGetPWinPipe(pw);
    if (!(win = pfuGLXWinopen(p, pw, NULL)))
        pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
	    "OpenXPipeline: Could not create GLX Window.");

    win = win; /* suppress compiler warn */

    /* set initial modes and welcome screen */
    InitGfx(pw);

}

/* 
 * Create and configure a GL window.
*/
void
OpenWin(pfPipeWindow *pw)
{
    pfOpenPWin(pw);
    /* set initial modes and welcome screen */
    InitGfx(pw);
}


/* Initialize graphics state */
void
ConfigPWin(pfPipeWindow *pw)
{
    /* set up default lighting parameters */
    pfLightModel	*lm;
    pfMaterial 		*mtl;

    /* 
     * Set up default material in case the database does not
     * have any. Use PFMTL_CMODE_AD so pfGeoSet colors supply the
     * ambient and diffuse material color.
    */
    mtl = pfNewMtl(NULL);
    pfMtlColorMode(mtl, PFMTL_FRONT, PFMTL_CMODE_AD);
    pfApplyMtl(mtl);

    /* Set up default lighting model. */
    lm = pfNewLModel(NULL);
    pfLModelAmbient(lm, 0.0f, 0.0f, 0.0f);

    /* 
     * Local viewer is required so that lighting matches across adjacent
     * channels since rotational offsets are encoded in the viewing 
     * matrix because GL will not fog channels properly if the offsets
     * are encoded in the projection matrix. 
    */
    if (NumChans > 1)
	pfLModelLocal(lm, 1);

    pfApplyLModel(lm);
    if (!ViewState->lighting)
	pfDisable(PFEN_LIGHTING);
    
    /* Prebind textures to be used in simulation */
    if (!(pfGetPWinType(pw) & PFPWIN_TYPE_STATS))
    {
	pfEnable(PFEN_TEXTURE);
	/* Do not show textures in hyperpipe mode */
	pfuDownloadTexList(ViewState->texList,
		Multipipe != HYPERPIPE ? PFUTEX_SHOW : 0);
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
    pfPipe 	*pipe = pfGetChanPipe(chan);
    int 	pipeId = pfGetId(pipe);
#if 0
    pfPipeVideoChannel *pvc;
    int		id=0;
#endif /* 0 */

    /*
     * If this pipe does not display the GUI, then clear the space where
     * the GUI would ordinarily be.
     *
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

	pvchan = pfGetChanPVChan(chan);
	pfGetPVChanScale(pvchan, &xscale, &yscale);
	
	gchan = pfuGetGUIChan();
	if (gchan)
	{
	    pfGetChanOrigin(gchan, &xo, &yo);
	    pfGetChanSize(gchan, &xs, &ys);
	    xo = xo*xscale + 0.5;
	    xs = xs*xscale + 0.5;
	    yo = yo*yscale + 0.5;
	    ys = ys*yscale + 0.5;
	}
	else
	{
	    xo = yo = 0;
	    pfGetPipeSize(pipe, &xs, &ys);
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
    pfClearChan(chan);

    /* Update Draw modes that have been changed in ViewState structure. */
    localPreDraw(chan, data);
}


/* In Draw callback routine, post-pfDraw() processing */
/* ARGSUSED (suppress compiler warn) */
void
PostDraw(pfChannel *chan, void *data)
{
    /* Display Stats, if active */
    if (ViewState->stats)
	pfDrawChanStats(chan);

    /*
     * Read GL keyboard, mouse. Make sure we only collect input on
     * master channel. If hyperpiping we must collect on all pipes.
    */
    if (chan == ViewState->masterChan)
    	pfuCollectInput();

    /* Call local post draw routine */
    localPostDraw(chan, data);
}
