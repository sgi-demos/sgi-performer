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
 * file: procsetup.c
 * -----------------
 * 
 * $Revision: 1.35 $
 * $Date: 2000/10/06 21:26:41 $
 *
 * OpenGL Performer example showing how to setup different
 * MP configureations for GL or mixed model (GLX) with 
 * Asynchronous XInput handling, and unix process control.
 * 
 * Routines from libpfutil are used for creating the GLX window
 * and for catching GL and X events
 * and converting them to a single format (PFDEV_* events).
 * 
 * For process control, seperate processes are assigned to specified CPUs
 * via utilities in libpfutil/lockcpu.c.
 *
 * For more information on unix process control,  refer to the sysmp manual
 * page and to the SGI REACT Technical Report.
 *
 *
 * Command Line Options: (getopt() is used to parse command line)
 * ---------------------
 * -p mode  - Set the MP mode
 *		0 = PFMP_APPCULLDRAW
 *		2 = PFMP_APP_CULLDRAW
 *		4 = PFMP_APPCULL_DRAW
 *		6 = PFMP_APP_CULL_DRAW
 * -P priority - Set the base priority for the App process
 *		Cull will have priority+1 and Draw will have
 *		priority+2. This ordering of priorities minimizes
 *		context switching amoung them and prevents deadlocks.
 *		if (priority == -1) pfuPrioritizeProcs() is called - the default.
 * -L	    - use our default policty for locking down processes to processor
 * -R app,cull,draw - Here the spacing counts!!!
 *		specifiy the processors for the app, cull, and draw processes.
 *		these processors will be restricted and isolated.
 *		A (-1) will cause no action to be taken for that process.
 * -x	    - use GLX window
 * -X	    - use GLX window and use asynchronous process to handle X events.
 *
 *
 *
 * Run-time controls:
 * ------------------
 *       ESC-key: exits
 *        F1-key: profile
 *    Left-mouse: advance
 *  Middle-mouse: stop
 *   Right-mouse: retreat
 */
 
 

/******************************************************************************
 *				Includes
 ******************************************************************************
 */


/* general includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h> /* for cmdline handler */
#include <sys/schedctl.h> /* for schedctl() */

/* X Window includes */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <gl/glws.h>  

/* GL includes */
#include <gl/device.h>
#include <gl/gl.h>

/* Performer includes */
#include <Performer/pf.h>
#include <Performer/pfutil.h>

/******************************************************************************
 *				Defines and Typedefs
 ******************************************************************************
 */

/* defines for flight model */
#define HISPEED		0.1f
#define LOSPEED		0.001f

/* SharedData:
 * structure that resides in shared memory so that the
 * application, cull, and draw processes can access it.
 */
typedef struct
{
    int	    exitFlag;
    pfCoord	    view;
    float	    sceneSize;
    int		    drawStats;
    /* stuff for mix mode GLX window */
    pfuGLXWindow    *xWinInfo;
     /* from pfutil.h */
    pfuMouse	    mouse;			
    pfuEvent	    events;
    int	    redrawOverlay;
} SharedData;

/******************************************************************************
 *				Static Declarations
 ******************************************************************************
 */

/* shared memory handle */
static SharedData *Shared=0;

/* X control variables */
static int XInput = 0, XIOProc = 0;
static int WinXOrigin = 20, WinYOrigin = 20;
static int WinXSize = 800, WinYSize = 500;
static int Overlay = 0;

/* process control variables */
static int Priority = -1; /* use libpfutil pfuPrioritizeProcs() */
static int RestrictProcessors = 0;
static int AppProc = -1, CullProc = -1, DrawProc = -1;
static int ProcSplit = PFMP_DEFAULT;
static int NumCPUs=0;

/* global name strings */
static char ProgName[PF_MAXSTRING];
static char Name[PF_MAXSTRING] = "OpenGL Performer";
static char XWinName[] =  "Performer Perfly-GLX";


/*
 * X Display for opening window and input.  In MP mode, each
 * process separately opens the display.
 */

static Display *XDpy = NULL;

/* light source created and updated in draw-process */

static pfLight *Sun;


static void CullChannel(pfChannel *_chan, void *_data);
static void DrawChannel(pfChannel *_chan, void *_data);
static void OpenPipeline(pfPipe *_p);
static void OpenXPipeline(pfPipe *_p);
static void InitGraphics(pfPipe * _p);
static void ProcessInputIO(void);
static void DrawOverlay(pfChannel *_chan);
static void UpdateView(void);
static void Usage(void);


/******************************************************************************
 *			    Main and Commandline Processing
 ******************************************************************************
 */

/*
 *	Usage() -- print usage advice and exit. This procedure
 *	is executed in the application process.
 */

char OptionStr[] = "p:P:R:LxX?";

static void
Usage(void)
{
    fprintf(stderr, "Usage: %s file.flt ...\n", ProgName);
    exit(1);
}


/*
 *	docmdline() -- use getopt to get command-line arguments, 
 *	executed at the start of the application process.
 */

static int
docmdline(int argc, char *argv[])
{
    int	    opt;
    
    strcpy(ProgName, argv[0]);
    
    /* process command-line arguments */
    while ((opt = getopt(argc, argv, OptionStr)) != -1)
    {
	switch (opt)
	{
	case 'p':
	    ProcSplit = atoi(optarg);
	    break;
	case 'P':
	    Priority = atoi(optarg);
	    break;
	case 'L':
	    RestrictProcessors = 1;
	    break;
	case 'R':
	    RestrictProcessors = 2;
	    if (!((optind <= argc) && 
		(3 == sscanf(argv[optind-1], "%ld,%ld,%ld", 
		    &AppProc, &CullProc, &DrawProc))))
	    {
		AppProc = CullProc = DrawProc = -1;
	    }
	    break;
	case 'x': 
	    XInput = 1; 
	    break;
	case 'X': 
	    XInput = 1;  
	    XIOProc = 1;
	    break;    
	case '?': 
	    Usage();
	}
    }
    return optind;
}

/*
 *	main() -- program entry point. this procedure
 *	is executed in the application process.
 */
int
main (int argc, char *argv[])
{
    int		    arg;
    int		    found;
    pfNode	   *root;
    pfChannel      *chan;
    pfScene        *scene;
    pfPipe         *p;
    pfEarthSky     *eSky;
    pfBox           bbox;
    pfCoord         view;
    float	    far = 10000.0f;
    int	    np, proc;

    Overlay = (getgdesc(GD_BITS_OVER_SNG_CMODE) ? OVERDRAW : 
		(getgdesc(GD_BITS_PUP_SNG_CMODE) ? PUPDRAW : 0));
		
    arg = docmdline(argc, argv);
    
    pfInit();
    pfNotifyLevel(PFNFY_DEBUG);
    
    /* Initialize OpenGL Performer utility library. */
    pfuInit();
    
    NumCPUs = sysmp(MP_NAPROCS);
    np = sysmp(MP_NPROCS);
    if (NumCPUs != np)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "Only %d out of %d processors available on this machine.",
			NumCPUs, np);
    } else
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
	    "All %d processors available on this machine.\n", NumCPUs);
    
    pfMultiprocess(ProcSplit);
    
    /* allocate shared before fork()'ing parallel processes 
     * so that all processes have will the correct Shared address
     */
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    Shared->redrawOverlay = 1;

    /* Load all loader DSO's before pfConfig() forks */
    for (found =  arg; found < argc; found++)
	pfdInitConverter(argv[found]);
    
    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKS HAPPEN HERE !!!
     */
    pfConfig();
    
    /* Initialize input structure for mouse and event inputs. */
    if (XInput)
	pfuInitInput(pfGetPipe(0), PFUINPUT_X);
    else
	pfuInitInput(pfGetPipe(0), PFUINPUT_GL);
    
    /*
     * App Process Setup
     */
    
    /* get back the actual mp configuration used */
    ProcSplit = pfGetMultiprocess();
    if (RestrictProcessors)
    {
	if (RestrictProcessors == 1)
	    pfuLockDownApp();
	else if ((RestrictProcessors == 2) && (AppProc > -1))
	{
	    if (AppProc == 0)
	    {
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
		    "%s - Forcing APP to run on CPU %d", 
		    ProgName, AppProc);
		pfuRunProcOn(AppProc);
	    } else {
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
		    "%s - Locking down APP on CPU %d", 
		    ProgName, AppProc);
		pfuLockDownProc(AppProc);
	    }
	}
    }
    
    /* set given priority - requires root uid.
     * note, Performer does do this by default 
     */
    if (Priority > 0) /* set specified priorities */
	schedctl(NDPRI,getpid(), Priority);
    else if (Priority < 0) /* use libpfutil priorities */
	pfuPrioritizeProcs(TRUE);
	
    scene = pfNewScene();
    
    /* specify directories where geometry and textures exist */
    if (arg < argc)
    {
	/* Append to PFPATH additional standard directories where 
	 * geometry and textures exist 
	 */
	pfFilePath(
		   "."
		   ":./data"
		   ":../data"
		   ":../../data"
		   ":/usr/share/Performer/data"
		   );
	
	/* load FLIGHT files named by command line arguments */
	for (found = 0; arg < argc; arg++)
	{
	    if ((root = pfdLoadFile(argv[arg])) != NULL)
	    {
		pfAddChild(scene, root);
		found++;
	    }
	}
    }
    
    pfPhase(PFPHASE_FLOAT);
    
    /* init shared mem for libpfutil */
    pfuInitUtil();
    
    /* Open and configure full screen GL window. */
    if (XInput)
	pfInitPipe(NULL, OpenXPipeline);
    else
	pfInitPipe(NULL, OpenPipeline);
    
    pfFrameRate(20.0f);
    
    p = pfGetPipe(0);
    chan = pfNewChan(p);
    pfChanTravFunc(chan, PFTRAV_CULL, CullChannel);
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 0.1f, far);
    
    /* vertical FOV is matched to window aspect ratio */
    pfChanFOV(chan, 45.0f, -1.0f);
    
    {
	float sceneSize;
	/* Set initial view to be "in front" of scene */
	
	/* view point at center of bbox */
	/* determine extent of scene's geometry */
	if (found)
	{
	    pfuTravCalcBBox(scene, &bbox);
	    /* find max dimension */
	    sceneSize = bbox.max[PF_X] - bbox.min[PF_X];
	    sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	    sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	    sceneSize = PF_MIN2(sceneSize, 0.5f * far);
	    Shared->sceneSize = sceneSize;
	    pfAddVec3(view.xyz, bbox.min, bbox.max);
	    pfScaleVec3(Shared->view.xyz, 0.5f, view.xyz);
	    /* offset so all is visible */
	    Shared->view.xyz[PF_Y] -=      sceneSize;
	    Shared->view.xyz[PF_Z] += 0.25f*sceneSize;
	}
	else
	{
	    pfSetVec3(Shared->view.xyz, 0.0f, 0.0f, 100.0f);
	    PFSET_VEC3(bbox.min, -5000.0, -5000.0, -1000000.0);
	    PFSET_VEC3(bbox.max, 5000.0, 5000.0, 10000000.0);
	}
	pfSetVec3(Shared->view.hpr, 0.0f, 0.0f, 0.0f);
	pfChanView(chan, Shared->view.xyz, Shared->view.hpr);
    }
    
    /* Create an earth/sky model that draws sky/ground/horizon */
    eSky = pfNewESky();
    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    pfESkyAttr(eSky, PFES_GRND_HT, -10.0f);
    pfChanESky(chan, eSky);
    
    /* main simulation loop */
    while (!Shared->exitFlag)
    {
	/* wait until next frame boundary */
	pfSync();
	
	/* Poll the mouse. */
	pfuGetMouse(&Shared->mouse);
	
	/* Set view parameters. */
	UpdateView();
	pfChanView(chan, Shared->view.xyz, Shared->view.hpr);
	
	/* initiate traversal using current state */
	pfFrame();
	
	/* Get snapshot of event/key queues */
	pfuGetEvents(&Shared->events);
	ProcessInputIO();
  
    }
    
    /*
     * Release restricted processes for clean exit
     */
    if (RestrictProcessors && (NumCPUs > 1))
    {
	pfuFreeCPUs();
    }
    
    /* exit from libpfutil */
    pfuExitUtil();
    
    pfuExitInput();

    /* terminate cull and draw processes (if they exist) */
    pfExit();
    
    /* exit to operating system */
    exit(0);
}


/* 
 *	UpdateView() updates the eyepoint based on the information
 *	placed in shared memory by GetXInput().
 */
static void    
UpdateView(void)
{
    static float speed = 0.0f;
    static float speedLimit = 4.0f;
    pfCoord *view = &Shared->view;
    float mx, my;
    float cp;

    if (!Shared->mouse.inWin)
    {
	speed = 0;
	return;
    }
    mx =  2.0f*(Shared->mouse.xpos) / (float)Shared->mouse.winSizeX - 1.0f;
    my = -2.0f*(Shared->mouse.ypos) / (float)Shared->mouse.winSizeY + 1.0f;
    switch (Shared->mouse.flags)
    {
    case PFUDEV_MOUSE_LEFT_DOWN: /* LEFTMOUSE: faster forward or slower backward*/
	if (speed > 0.0f)
	    speed *= 1.2f;
	else
	    speed /= 1.2f;
	
	if (PF_ABSLT(speed, LOSPEED * Shared->sceneSize))
	    speed = LOSPEED * Shared->sceneSize;
	else if (speed >=  HISPEED * Shared->sceneSize)
	    speed = HISPEED * Shared->sceneSize;
	break;
    case PFUDEV_MOUSE_MIDDLE_DOWN:	/* MIDDLEMOUSE: stop moving and pick */
	speed = 0.0f;
	break;
    case PFUDEV_MOUSE_RIGHT_DOWN: /* RIGHTMOUSE: faster backward or slower foreward*/
	if (speed < 0.0f)
	    speed *= 1.2f;
	else
	    speed /= 1.2f;
	
	if (PF_ABSLT(speed, LOSPEED * Shared->sceneSize))
	    speed = -LOSPEED * Shared->sceneSize;
	else if (speed <=  -HISPEED * Shared->sceneSize)
	    speed = -HISPEED * Shared->sceneSize;
	break;
    }

    /* update view direction */
    view->hpr[PF_H] -= mx * PF_ABS(mx);
    view->hpr[PF_P]  = my * PF_ABS(my) * 90.0f;
    view->hpr[PF_R]  = 0.0f;
    
    /* update view position */
    cp = cosf(PF_DEG2RAD(view->hpr[PF_P]));
    view->xyz[PF_X] += speed*sinf(-PF_DEG2RAD(view->hpr[PF_H])*cp);
    view->xyz[PF_Y] += speed*cosf(-PF_DEG2RAD(view->hpr[PF_H])*cp);
    view->xyz[PF_Z] += speed*sinf( PF_DEG2RAD(view->hpr[PF_P]));
}


/******************************************************************************
 *			    Draw Process: Graphics Initialization
 ******************************************************************************
 */


/*
 *	OpenPipeline() -- create a pipeline: setup the window system,
 *	the OpenGL, and OpenGL Performer. this procedure is executed in
 *	the draw process (when there is a separate draw process).
 */
 
  
static void
OpenPipeline(pfPipe * p)
{   
    prefposition(WinXOrigin, WinXOrigin +  WinXSize, 
		    WinYOrigin, WinYOrigin +  WinYSize);
       
    foreground();
    winopen(ProgName);
    winconstraints();
    subpixel(TRUE);
    
    qdevice(WINQUIT);
    qdevice(REDRAW);
    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    qdevice(RIGHTMOUSE);
    qdevice(KEYBD);
    qdevice(F1KEY);

    pfInitGfx(p);
    
    InitGraphics(p);
      
}


static void
OpenXPipeline(pfPipe * p)
{
    pfuGLXWindow *win;

    XDpy = (Display *) pfuOpenXDisplay(0);
        
    if (!(Shared->xWinInfo = pfuGLXWinopen((pfuXDisplay*) XDpy, p, XWinName, 	
			    WinXOrigin, WinXOrigin + WinXSize, 
			    WinYOrigin, WinYOrigin + WinYSize)))
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, 
	    "OpenXPipeline: Could not create GLX Window.\n");
    }
    
    /* initialize Performer for GLX rendering */
    win = Shared->xWinInfo;
    pfInitGLXGfx(p, XDpy, win->xWin, win->glWin, win->overWin,
				PFGLX_AUTO_RESIZE);
         
    InitGraphics(p);    
    
    XFlush(XDpy);

}

/* Draw Process Setup */
static void
InitGraphics(pfPipe * p)
{
    if (RestrictProcessors)
    {
	if (RestrictProcessors == 1)
	    pfuLockDownDraw(p);
	else if ((RestrictProcessors == 2) && (DrawProc > -1))
	{
	    if (DrawProc == 0)
	    {
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
		    "%s - Forcing DRAW to run on CPU %d", 
		    ProgName, DrawProc);
		pfuRunProcOn(DrawProc);
	    } else {
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
		    "%s - Locking down DRAW on CPU %d", 
		    ProgName, DrawProc);
		pfuLockDownProc(DrawProc);
	    }
	}
    } 
    if ((Priority > 0)&& (NumCPUs > 1) && 
	    (ProcSplit & (PFMP_FORK_DRAW | PFMP_FORK_CULL)))
	schedctl(NDPRI,getpid(), Priority+2);
    
    /*
     * Graphics Initialization
     */
    /* create a light source in the "south-west" (QIII) */
    Sun = pfNewLight(NULL);
    pfLightPos(Sun, -0.3f, -0.3f, 1.0f, 0.0f);
    
    /* create a default texture environment */
    pfApplyTEnv(pfNewTEnv(NULL));
    
    /* create a default lighting model */
    pfApplyLModel(pfNewLModel(NULL));
    pfApplyMtl(pfNewMtl(NULL));
    
    /* enable culling of back-facing polygons */
    pfCullFace(PFCF_BACK);
    
    /*
     * These enables should be set to reflect the majority of the
     * database. If most geometry is not textured, then texture
     * should be disabled. However, you then need to change the
     * FLIGHT-format file reader. (pfflt.c)
     */
    pfEnable(PFEN_TEXTURE);
    pfEnable(PFEN_LIGHTING);
    pfEnable(PFEN_FOG);
    
    Shared->mouse.winOriginX = WinXOrigin;
    Shared->mouse.winOriginY = WinYOrigin;
    Shared->mouse.winSizeX = WinXSize;
    Shared->mouse.winSizeY = WinYSize;
}


/******************************************************************************
 *			    Draw Process Routines
 ******************************************************************************
 */


/*
 *	DrawChannel() -- draw a channel and read input queue. this
 *	procedure is executed in the draw process (when there is a
 *	separate draw process).
 */
static void
DrawChannel (pfChannel *channel, void *data)
{
    /* rebind light so it stays fixed in position */
    pfLightOn(Sun);
    
    /* erase framebuffer and draw Earth-Sky model */
    pfClearChan(channel);
    
    	
    if (Shared->redrawOverlay)
    {
	Shared->redrawOverlay = 0;
	DrawOverlay(channel);
    }
    
    /* invoke Performer draw-processing for this frame */
    pfDraw();
    
    /* draw Performer throughput statistics */
    if (Shared->drawStats)
	pfDrawChanStats(channel);
	
    pfuCollectInput();
    
}

static void
ProcessInputIO(void)
{
    int i,j,key, count;
    int dev, val, numDevs;
    pfuEvent *pfevents;
    
    pfevents = &(Shared->events);
    
    numDevs = pfevents->numDevs;
    for (j=0; j < numDevs; j++)
    {
	dev = pfevents->devQ[j];
	val = pfevents->devVal[j];
	if (pfevents->devCount[dev] > 0)
	{
	    switch(dev)
	    {
	    case PFUDEV_REDRAW:
		Shared->redrawOverlay = 1;
		pfevents->devCount[dev] = 0; /* mark device done */
		break;
		
	    case PFUDEV_WINQUIT:
		Shared->exitFlag = 1;
		pfevents->devCount[dev] = 0; /* mark device done */
		break;
		
		/* Main Keyboard */
	    case PFUDEV_KEYBD:
		for(i=0; i<pfevents->numKeys; i++)
		{
		    key = pfevents->keyQ[i];
		    if (count = pfevents->keyCount[key])
		    {	/* key was pressed count times */
			if (count > 1) /* scale count by relative frame rate */
			    count = PF_MAX2(
				    (int) (count * (pfGetFrameRate() / 60.0f)),
				     1);
			switch(key)
			{
			case 27:	/* ESC */
			    Shared->exitFlag = 1;
			    break;
			}
		    }
		}
		/* XXX this is very important or else future keybd events
		 * will be lost !!!
		 */
		pfevents->devCount[dev] = 0; /* mark device done */
		break;
	    case PFUDEV_F1KEY:
		Shared->drawStats = !Shared->drawStats;
		pfevents->devCount[dev] = 0; /* mark device done */
		break;
	    } /* switch on device */
	} /* if have device */
    } /* for each device */
    pfevents->numDevs = 0;
}

/* note: this uses the font library to print the 
 * Performer name in fancy text
 */
static void
DrawOverlay(pfChannel *chan)
{
    if (!Overlay)
	return;

    if (XInput)
    {
	if (GLXwinset(XDpy, Shared->xWinInfo->overWin) < 0)
	{
	    pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
		     "GLXwinset for overlay failed\n");
	    return;
	}
    } else
	drawmode(Overlay);
	
    /* print a friendly greeting */
    pfuDrawMessage(chan, Name, PFU_MSG_PIPE, PFU_RIGHT_JUSTIFIED, 
		    1.0f, 1.0f, PFU_FONT_MED, PFU_RGB); 
   
    if (XInput)
    {
	if (GLXwinset(XDpy, Shared->xWinInfo->glWin) < 0)
	{
	    pfNotify(PFNFY_WARN, PFNFY_SYSERR, 
		     "GLXwinset for NORMAL window failed\n");
	}
	XFlush(XDpy);
    } else
	drawmode(NORMALDRAW);
}


/******************************************************************************
 *			    Cull Process Routines
 ******************************************************************************
 */


/*
 *	CullChannel() -- traverse the scene graph and generate a
 * 	display list for the draw process.  This procedure is 
 *	executed in the cull process.
 */

static void
CullChannel(pfChannel *channel, void *data)
{
    static int first = 1;
    /*
     * Cull Process Setup
     */
     
    if (first)
    {
	if (RestrictProcessors)
	{ 
	    if (RestrictProcessors == 1)
		pfuLockDownCull(pfGetChanPipe(channel));
	    else if ((RestrictProcessors == 2) && (CullProc > -1))
	    {
		if (CullProc == 0)
		{
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
			"%s - Forcing CULL to run on CPU %d", 
			ProgName, CullProc);
		    pfuRunProcOn(CullProc);
		} else {
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
			"%s - Locking down CULL on CPU %d", 
			ProgName, CullProc);
		    pfuLockDownProc(CullProc);
		}
	    }
	} 
	if ((NumCPUs > 1) && (Priority > 0) && (ProcSplit == PFMP_APP_CULL_DRAW))
		schedctl(NDPRI,getpid(), Priority+1);
	first=0;
    }
    /* 
     * pfDrawGeoSet or other display listable Performer routines
     * could be invoked before or after pfCull()
     */
    
    pfCull();
}

