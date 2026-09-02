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
 * file: utilui.c
 * -----------------
 * 
 * OpenGL Performer example showing how to use
 * libpfutil to set up a user-interface with viewing model
 * and graphical control panel.
 * Input handling is done from GL input handling or mixed model (GLX) with 
 * Asynchronous XInput handling.
 * Routines from libpfutil are used for managing and drawing the GUI,
 * creating the GLX window, and for catching GL and X events 
 * and converting them to a single format (PFDEV_* events).
 *
 *
 * Command Line Options: (getopt() is used to parse command line)
 * ---------------------
 * -x	    - use GLX window and use asynchronous process to handle X events.
 *
 *
 * Run-time controls:
 * ------------------
 *       ESC-key: exits
 *        F1-key: display profile stats
 *     SPACE-key: stop
 *	   r-key: re-position
 *    Left-mouse:
 *  Middle-mouse:
 *   Right-mouse:
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

/* proecess control includes */
#include <sys/types.h>
#include <sys/sysmp.h>
#include <sys/prctl.h>
#include <sys/schedctl.h>

/* X Window includes */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
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

/* Panel Widget Control Tokens */
enum PanelControlToken {
    CTL_RESET_ALL=1, 
    CTL_REPOSITION, 
    CTL_QUIT, 
    CTL_VEHICLE, 
    CTL_VIEWPOINT, 
    CTL_MODEL, 
    CTL_DIAGS, 
    CTL_AA, 
    CTL_HSLIDER,
    CTL_HRADIO, 
    CTL_CURSOR
};

/* use of device not handled by libpfutil - handled in MyHandler() */
#define MY_HOMEKEY (PFUDEV_MAX + 1)

/* SharedData:
 * structure that resides in shared memory so that the
 * application, cull, and draw processes can access it.
 */
typedef struct
{
    /* main channel stuff */
    pfChannel		*chan;
    pfVec3		startXYZ;
    float		sceneSize;
    int		drawStats;
    int		antialias;
    int		exitFlag;
    pfLightSource	*sun;		/* sun light source */
	/* Input handling structures */
    int		input;		/* PFUINPUT_GL or PFUINPUT_X */
    pfuGLXWindow	xWinInfo;	/* GLX window info */
    pfuMouse		mouse;		/* mouse structure */
    pfuEvent		events;		/* event structure */
	/* flight model and collision detection sructures */
    pfScene		*scene;
    pfDCS		*sceneDCS;	/* DCS for trackball interface */ 
    pfGroup		*sceneGroup;	/* root of scene under DCS */
    pfCoord		viewCoord;	/* current view position, direction */ 
    pfCoord		initView;	/* initial view position, direction */ 
    float		near;		/* near clipping plane  */
    float		far;		/* far clipping plane  */
    pfuXformer		*xformer;	/* interactive movement model */
    int		xformerMode;	/* Fly, Drive, or Trackball   */
    int		collideMode;	/* collision & terrain following flag*/
	/* flags for GUI */
    int		updateChannels;
    int		gui, redraw, guiDirty;
    pfuPanel		*guiPanel;
    pfuWidget		*wResetAll, *wKillGUI, *wPosXYZ, *wStats, *wAA, *wFModel;
} SharedData;

/******************************************************************************
*				Static Declarations
******************************************************************************
*/

/* shared memory handle */
static SharedData *Shared=0;

static pfMatrix ident;

/* X control variables */
static int XInput = PFUINPUT_X;
static int ResizeMode = PFGLX_AUTO_RESIZE;
static int WinXOrigin = 20, WinYOrigin = 20;
static int WinXSize = 800, WinYSize = 500;
static int Overlay = 0;

/* process control variables */
static int ProcSplit = PFMP_APP_CULL_DRAW;
static int NumCPUs=0;

/* global name strings */
static char ProgName[PF_MAXSTRING];
static char Name[PF_MAXSTRING] = "OpenGL Performer";

/*
* X Display for opening window and input.  In MP mode, each
* process separately opens the display.
*/

static Display *XDpy = NULL;

static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeline(pfPipe *p);
static void OpenXPipeline(pfPipe *p);
static void InitGraphics(void);
static pfuPanel *InitPanel(void);
static void ProcessInputIO(void);
static void PanelControl(pfuWidget *w);
static void resetPosition(void);
static void KillGUI(pfuWidget *w);
static void Usage(void);
static void InitXformer(void);
static void XformerMode(int mode, int collideMode);
static void IsectFunc(void *data);
static void UpdateView(pfChannel *chan, pfScene *scene);
static void updateGUIViewMsg(pfCoord *viewpt);
static void drawOverlayName(void);

/* event handlers for declaring and handling custom events not in libpfutil,
 * or for overriding libpfutil's handling of specific events
 */
static void MyGLHandler(unsigned int dev, unsigned int val, pfuCustomEvent *pfuevent);
static void MyXHandler(unsigned int display, unsigned int event, pfuCustomEvent *pfuevent);

/******************************************************************************
*			    Main and Commandline Processing
******************************************************************************
*/

/*
*	Usage() -- print usage advice and exit. This procedure
*	is executed in the application process.
*/

char OptionStr[] = "rm:p:x?";

static void
Usage(void)
{
    fprintf(stderr, "Usage: %s [m mpmode] [x] file.ext ...\n", ProgName);
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
	case 'r':
	    if (ResizeMode == PFGLX_AUTO_RESIZE)
		ResizeMode = 0;
	    else
		ResizeMode = PFGLX_AUTO_RESIZE;
	    break;
	case 'm':
	case 'p':
	    ProcSplit = atoi(optarg);
	    break;
	case 'x':
	    if (XInput == PFUINPUT_X)
		XInput = PFUINPUT_GL; 
	    else
		XInput = PFUINPUT_X; 
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
    pfGroup	   *root;
    pfChannel      *chan;
    pfScene        *scene;
    pfPipe         *p;
    pfEarthSky     *eSky;
    pfBox           bbox;
    float	    far = 10000.0f;
    pfFrameStats   *stats;
    
    Overlay = (getgdesc(GD_BITS_OVER_SNG_CMODE) ? OVERDRAW : 
		getgdesc(GD_BITS_PUP_SNG_CMODE) ? PUPDRAW : 0);
		
    arg = docmdline(argc, argv);
    
    pfInit();
    pfNotifyLevel(PFNFY_DEBUG);
    
    /* init shared mem for libpfutil */
    pfuInit();

    pfMakeIdentMat(ident);
    
    pfMultiprocess(ProcSplit);
    
    /* allocate shared before fork()'ing parallel processes 
     * so that all processes have will the correct Shared address
     */
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    /* init shared memory structures */
    Shared->xformerMode		= PFUXF_TRACKBALL;
    Shared->input		= XInput;
    Shared->collideMode		= PF_ON;
    Shared->gui			= 1; /* enable gui */
    Shared->updateChannels	= 1;
    Shared->guiDirty = Shared->redraw = 1;
    
    if (Shared->input == PFUINPUT_X)
    {
	strcpy(Name, "OpenGL Performer GLX");
    } 

    /* Load all loader DSO's before pfConfig() forks */
    for (found =  arg; found < argc; found++)
	pfdInitConverter(argv[found]);
    
    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKS HAPPEN HERE !!!
     */
    pfConfig();

    /*
     * App Process Setup
     */
    
    scene = pfNewScene();
    Shared->sceneDCS = pfNewDCS();
    Shared->sceneGroup = pfNewGroup();
    pfAddChild(scene, Shared->sceneDCS);
    pfAddChild(Shared->sceneDCS, Shared->sceneGroup);
    
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
	    if ((root = (pfGroup *)pfdLoadFile(argv[arg])) != NULL)
	    {
		pfAddChild(Shared->sceneGroup, root);
		found++;
	    }
	}
    }
    if (found) 
    {
	/* Set intersection callback. */
	pfIsectFunc(IsectFunc);
	/* do intersection setup */
	pfuCollideSetup(scene, PFUCOLLIDE_STATIC, 0xffffffff);
    }
    Shared->scene = scene;
    
    p = pfGetPipe(0);
     
    /* Initialize input structure (X or GL) for mouse and event inputs
     * then Open and configure full screen GL or GLX window. 
     */
    if (Shared->input == PFUINPUT_X)
    {
	pfInitPipe(NULL, OpenXPipeline);
    } 
    else 
    {
	pfInitPipe(NULL, OpenPipeline);
    }
    
    pfuInitGUI(pfGetPipe(0));
    pfuGUIViewport(0.0f, 1.0f, 0.0f, 0.3);
    /* init the control panel for perfly */
    Shared->guiPanel = InitPanel();
    pfuEnablePanel(Shared->guiPanel, 1);
    
    Shared->chan = chan = pfNewChan(p);
    pfChanViewport(Shared->chan, 0.0f, 1.0f, 0.3, 1.0f);
    pfChanTravFunc(chan, PFTRAV_CULL, CullChannel);
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 0.1f, far);

    /* Create an earth/sky model that draws sky/ground/horizon */
    eSky = pfNewESky();
    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    pfESkyAttr(eSky, PFES_GRND_HT, -10.0f);
    pfChanESky(chan, eSky);

    /* Initialize sun - create a light source in the "south-west" (QIII) */
    Shared->sun = pfNewLSource();
    pfLSourcePos(Shared->sun, -0.3f, -0.3f, 1.0f, 0.0f);
    pfAddChild(Shared->scene, Shared->sun);

    stats = pfGetChanFStats(chan);

    /* vertical FOV is matched to window aspect ratio */
    pfChanFOV(chan, 45.0f, -1.0f);
    /* set up scene bounding box and initial viewing position */
    {
	float sceneSize;
	/* Set initial view to be "in front" of scene */
	
	/* view point at center of bbox */
	/* determine extent of scene's geometry */
	if (found)
	{
	    pfVec3 view;
	    pfuTravCalcBBox(scene, &bbox);
	    /* find max dimension */
	    sceneSize = bbox.max[PF_X] - bbox.min[PF_X];
	    sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	    sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	    sceneSize = PF_MIN2(sceneSize, 0.5f * far);
	    Shared->sceneSize = sceneSize;
	    pfAddVec3(view, bbox.min, bbox.max);
	    pfScaleVec3(Shared->viewCoord.xyz, 0.5f, view);
	    /* back up so all is visible */
	    Shared->viewCoord.xyz[PF_Y] -=  2*sceneSize;
	}
	else
	{
	    pfSetVec3(Shared->viewCoord.xyz, 0.0f, 0.0f, 100.0f);
	    PFSET_VEC3(bbox.min, -5000.0, -5000.0, -1000000.0);
	    PFSET_VEC3(bbox.max, 5000.0, 5000.0, 10000000.0);
	}
    
	pfSetVec3(Shared->viewCoord.hpr, 0.0f, 0.0f, 0.0f);
	pfChanView(chan, Shared->viewCoord.xyz, 
			    Shared->viewCoord.hpr);
	Shared->initView = Shared->viewCoord;
    }
    pfCopyVec3(Shared->startXYZ, Shared->viewCoord.xyz);
    
    if (Shared->input == PFUINPUT_X)
    {
	pfuInitInput(p, PFUINPUT_X);
	pfuInputHandler(MyXHandler, PFUINPUT_CATCH_UNKNOWN);
    } 
    else 
    {
	pfuInitInput(p, PFUINPUT_GL);
	pfuInputHandler(MyGLHandler, PFUINPUT_CATCH_UNKNOWN);
    }
    
    /* Initialize channel viewing model for interactive motion */    
    InitXformer();
    
    /* main simulation loop */
    while (!Shared->exitFlag)
    {
	/* wait until next frame boundary */
	pfSync();
	
	/* 
	 * All latency critical processing should be done here. This is
	 * typically the viewing position. 
	 * One should also read latency critical devices here.
	 */
	pfuGetMouse(&Shared->mouse);
	UpdateView(Shared->chan, Shared->scene);
	
	/* initiate traversal using current state */
	pfFrame();
	
	/* Get snapshot of event/key queues */
	pfuGetEvents(&Shared->events);
	ProcessInputIO();
    
	if (Shared->updateChannels)
	{
	    if (Shared->gui)
	    {
		float	foo, top;
    
		Shared->redraw = Shared->guiDirty = 1;
		pfuEnableGUI(TRUE);
		pfuGetGUIViewport(&foo, &foo, &foo, &top); 
		pfChanViewport(Shared->chan, 0.0f, 1.0f, top, 1.0f);
	    }
	    else
	    {
		pfuEnableGUI(FALSE);
		pfChanViewport(Shared->chan, 0.0f, 1.0f, 0.0f, 1.0f);
	    }
	    Shared->guiDirty = Shared->updateChannels = 0;
	} else if (Shared->gui)
	{
	    updateGUIViewMsg(&(Shared->viewCoord));
	    pfuUpdateGUI();
	}
    }
    /* exit GUI and print avg time to draw GUI - optional */
    pfuExitGUI();
    pfuExitInput();
    /* Exit from libpfutil and remove shared mem arenas */
    pfuExitUtil();
    /* terminate cull and draw processes (if they exist) */
    pfExit();
    
    /* exit to operating system */
    return (0);
}

static void
InitXformer(void)
{
    Shared->xformer = pfuNewXformer(pfGetSharedArena());

    pfuXformerAutoInput(Shared->xformer, Shared->chan, 
	&Shared->mouse, &Shared->events);

    XformerMode(Shared->xformerMode, Shared->collideMode);
}

static void 
IsectFunc(void *data)
{
    (data); /* avoid compiler warn about unused vars */
    pfuCollideXformer(Shared->xformer);
}


/******************************************************************************
*			    App Viewing Model Routines 
******************************************************************************
*/

/*
 * Thes routines use the viewing and tranformation utilities in 
 * libpfutil to implement several fiewing modes: drive, fly, and trackball.
 */

static void
XformerMode(int mode, int collideMode)
{
    pfMatrix		mat;
    static int		oldMode = -1, oldCollMode = -1;

    if ((mode == oldMode && oldCollMode == collideMode) || !Shared->xformer)
	return;

    pfuXformerMode(Shared->xformer, mode);

    /* If not in trackball mode, remove trackball DCS for better performance */
    if (oldMode == PFUXF_TRACKBALL)
    {
	pfRemoveChild(Shared->scene, Shared->sceneDCS); 
	pfAddChild(Shared->scene, Shared->sceneGroup); 
    }
    else if (mode == PFUXF_TRACKBALL) /* Restore trackball DCS to scene */
    {
	pfRemoveChild(Shared->scene, Shared->sceneGroup); 
	if (pfGetNumParents(Shared->sceneDCS) == 0)
	    pfAddChild(Shared->scene, Shared->sceneDCS); 
    }
      
    switch (mode) 
    {
    case PFUXF_DRIVE:
	/* Limit to ~60mph, 90 degrees/sec pan, .125G acceleration */
	pfuXformerLimits(Shared->xformer, 30.0f, 90.0f, 1.25f, NULL);

	/* Follow 2 meters above ground */
	if (collideMode == PF_ON)
	    pfuXformerCollision(Shared->xformer, PFUCOLLIDE_GROUND, 2.0f, 
		Shared->scene);
	else
	    pfuXformerCollision(Shared->xformer, PFUCOLLIDE_GROUND, 0.0f, 
		NULL);
	
	/* Collide with objects in scene */
	if (collideMode == PF_ON)
	    pfuXformerCollision(Shared->xformer, PFUCOLLIDE_OBJECT, 
		2.0f*Shared->near, Shared->scene);
	else
	    pfuXformerCollision(Shared->xformer, PFUCOLLIDE_OBJECT, 0.0f, 
		NULL);

	pfuXformerCoord(Shared->xformer, &Shared->viewCoord);
	break;

    case PFUXF_FLY:
	/* Limit to ~180mph, 90 degrees/sec pan, .25G acceleration */
	pfuXformerLimits(Shared->xformer, 90.0f, 90.0f, 2.5f, NULL);

	/* Do not terrain follow */
	pfuXformerCollision(Shared->xformer, PFUCOLLIDE_GROUND, 0.0f, NULL);

	/* Collide with objects in scene */
	if (collideMode == PF_ON)
	    pfuXformerCollision(Shared->xformer, PFUCOLLIDE_OBJECT, 
		2.0f*Shared->near, Shared->scene);
	else
	    pfuXformerCollision(Shared->xformer, PFUCOLLIDE_OBJECT, 0.0f, 
		NULL);

	pfuXformerCoord(Shared->xformer, &Shared->viewCoord);
	break;
     case PFUXF_TRACKBALL:

	pfuXformerLimits(Shared->xformer, Shared->sceneSize*.5f, 
		    180.0f, 0.0f, NULL);

	/* Do not terrain follow */
	pfuXformerCollision(Shared->xformer, PFUCOLLIDE_GROUND, 0.0f, NULL);

	/* Do not collide with objects in scene */
	pfuXformerCollision(Shared->xformer, PFUCOLLIDE_OBJECT, 0.0f, NULL);

	pfGetDCSMat(Shared->sceneDCS, mat);
	pfuXformerMat(Shared->xformer, mat);
	break;
    }

    oldMode = mode;
    oldCollMode = collideMode;
}

/* 
 * Update the current view 
 */
static void
UpdateView(pfChannel *chan, pfScene *scene)
{
    pfMatrix 	mat;

    (scene); /* avoid compiler warn about unused vars */
    
    pfuUpdateXformer(Shared->xformer);

    switch (pfuGetXformerMode(Shared->xformer))
    {
    case PFUXF_FLY:
    case PFUXF_DRIVE:
	pfuGetXformerCoord(Shared->xformer, &Shared->viewCoord);
	pfuGetXformerMat(Shared->xformer, mat);
	pfChanViewMat(chan, mat);
	break;
    case PFUXF_TRACKBALL:
	pfuGetXformerMat(Shared->xformer, mat);
	pfDCSMat(Shared->sceneDCS, mat);
	break;
    }
}


/* 
 * Reset view params to original XYZ position and HPR Euler angles.
 * This is called from the input handling process so it should not
 * call pfChanView() or pfChanViewMat() and initView needs to be
 * in shared memory.
 */
static void 
resetPosition(void)
{
    pfMatrix	mat;

    Shared->viewCoord = Shared->initView;

    switch (pfuGetXformerMode(Shared->xformer))
    {
    case PFUXF_FLY:
    case PFUXF_DRIVE:
	pfuXformerCoord(Shared->xformer, &Shared->viewCoord);
	break;	

    case PFUXF_TRACKBALL:
	pfuXformerCoord(Shared->xformer, &Shared->viewCoord);
	pfuGetXformerMat(Shared->xformer, mat);
	pfuXformerMat(Shared->xformer, ident);
	pfDCSMat(Shared->sceneDCS, ident);
	break;
    }

    pfuStopXformer(Shared->xformer);
}


/******************************************************************************
 *			    App Input Handling
 ******************************************************************************
 */
 
static void
MyGLHandler(unsigned int dev, unsigned int val, pfuCustomEvent *pfuevent)
{
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
	"MyGLHandler: have dev %d with val %d", dev, val);
    
    /* fill event for libpfutil to add to its internal event queue */
    switch(dev)
    {
    case HOMEKEY:
	if (val)
	{
	    pfuevent->dev = MY_HOMEKEY;
	    pfuevent->val = val;
	} else
	    pfuevent->dev = PFUDEV_NULL;
	break;
    default:
	pfuevent->dev = PFUDEV_NULL;
    }
}

static void
MyXHandler(unsigned int dsp, unsigned int ev, pfuCustomEvent *pfuevent)
{
    Display *display = (Display *)dsp;
    XEvent *event = (XEvent *)ev;
    
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
	"MyXHandler: have display %d with event type %d", 
		display, event->type);
    switch(event->type)
    {
    case KeyPress:
	{
	    XKeyEvent *key_event = (XKeyEvent *) event;
	    short val = key_event->keycode;
	    int ccount;
	    KeySym	keysym;
	    char	charbuf[10];
	    
	    ccount = XLookupString(key_event, charbuf, 10, &keysym, 0);
	    if (keysym == XK_Home)
	    {
		if (val)
		{
		    pfuevent->dev = MY_HOMEKEY;
		    pfuevent->val = val;
		} else
		    pfuevent->dev = PFUDEV_NULL;
	    }   
	}
    }
}

static void
ProcessInputIO(void)
{
    int i,j,key;
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
		/* worst case in APP_CULL_DRAW mode it takes 3 frames to 
		 * propagate new window size back down to draw process.
		 */
		Shared->redraw = 3;
		pfuRedrawGUI();
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
		    if (pfevents->keyCount[key])
		    {	/* key was pressed count times */
			switch(key)
			{
			case 27:	/* ESC */
			    Shared->exitFlag = 1;
			    break;
			case 'g':
			case 'G':
			    Shared->drawStats = !Shared->drawStats;
			    pfuWidgetValue(Shared->wStats, Shared->drawStats);
			    break;
			case 'r':
			case 'R':
			    resetPosition();
			    break;
			case ' ':
			    pfuStopXformer(Shared->xformer);
			    break;
			default:
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
		Shared->gui = !Shared->gui;
		pfuWidgetValue(Shared->wKillGUI, Shared->gui);
		Shared->updateChannels = 1;
		pfevents->devCount[dev] = 0; /* mark device done */
		break;
		
	    case MY_HOMEKEY:
		Shared->updateChannels = 1;
		pfevents->devCount[dev] = 0; /* mark device done */
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		    "ProcessInputIO: I caught MY_HOMEKEY!");
		break;
	    } /* switch on device */
	} /* if have device */
    } /* for each device */
    pfevents->numDevs = 0;
    pfevents->numKeys = 0;
}


/******************************************************************************
*			    Panel Creation and Management
******************************************************************************
*/


static pfuGUIString viewModel_strs[] = {"Trackball", "Drive", "Fly"};
static int viewModel_vals[] = {PFUXF_TRACKBALL, PFUXF_DRIVE, PFUXF_FLY};
#define NUM_CURSORS 14		    
static pfuGUIString cursor_strs[NUM_CURSORS] = {
	    "Circle", "Fleur", "Man", "Gobbler", "Gumby", "Heart", 
	    "Pirate", "Sailboat", "Shuttle", "Umbrella", 
	    "Star", "target", "Dot", "Leftptr"
};

static int cursor_vals[] = {
		    PFU_CURSOR_circle, PFU_CURSOR_fleur, PFU_CURSOR_man, 
		    PFU_CURSOR_gobbler, PFU_CURSOR_gumby, PFU_CURSOR_heart, 
		    PFU_CURSOR_pirate, PFU_CURSOR_sailboat, 
		    PFU_CURSOR_shuttle, PFU_CURSOR_umbrella, 
		    PFU_CURSOR_star, PFU_CURSOR_target, 
		    PFU_CURSOR_dot, PFU_CURSOR_left_ptr};
static pfuGUIString num_strs[] = {"One", "Two", "Three"};
static pfuPanel *
InitPanel(void)
{
    float  xo, yo, ys, xs;
    int x, y, yTop;
    pfuWidget *wid;
    pfuPanel *pan;
    
    pan = pfuNewPanel();
    
    pfuGetPanelSize(pan, &xo, &xs, &yo, &ys);

    x = (int)xo;
    yTop = (int)(yo + ys);
    y = yTop - PFUGUI_BUTTON_YINC;

	/* action  buton - disable the gui (F1KEY to get it back) */
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, 0); 
    pfuWidgetDim(wid,  x,  y,  24,  24);
    pfuWidgetValue(wid, 1.0f);
    pfuWidgetDefaultValue(wid, 1.0f);
    pfuWidgetFunc(wid, KillGUI);
    Shared->wKillGUI = wid;
    x += 24 + PFUGUI_PANEL_FRAME;
    
	/* action  buton - reset to origin button */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_REPOSITION);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_Vint_XSIZE,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Reposition");
    pfuWidgetFunc(wid, PanelControl);
    x += PFUGUI_BUTTON_Vint_XINC;
    
	/* the message bar with positions */
    wid = pfuNewWidget(pan, PFUGUI_MESSAGE, 0);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_MESSAGE_XSIZE,  PFUGUI_MESSAGE_YSIZE);
    pfuWidgetFunc(wid, PanelControl);
    x += PFUGUI_MESSAGE_XINC;
    Shared->wPosXYZ = wid;
    
    
	/* action button - reset all */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_RESET_ALL); 
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_Vint_XSIZE,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Reset All");
    pfuWidgetFunc(wid, PanelControl);
    Shared->wResetAll = wid;
    
    x = (int)xo;
    y = yTop - 2*PFUGUI_BUTTON_YINC;
    
	/* action button - quit */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_QUIT); 
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_XSIZE,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Quit");
    x += PFUGUI_BUTTON_XINC;
    
	/* menu button - view model */
    wid = pfuNewWidget(pan, PFUGUI_MENU_BUTTON, CTL_MODEL); 
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_Vint_XSIZE,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Motion");
    pfuWidgetFunc(wid, PanelControl);
    pfuWidgetSelections(wid, viewModel_strs, viewModel_vals, NULL, 3);
    pfuWidgetValue(wid, Shared->xformerMode);
    Shared->wFModel = wid;
    x += PFUGUI_BUTTON_Vint_XINC;
    
	/* switch button - stats */
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_DIAGS); 
    pfuWidgetDim(wid, x, y, PFUGUI_BUTTON_XSIZE, PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Stats");
    pfuWidgetFunc(wid, PanelControl);
    Shared->wStats = wid;

    x += PFUGUI_BUTTON_XINC;
    
	/* switch button - aa */
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_AA); 
    pfuWidgetDim(wid, x, y, PFUGUI_BUTTON_XSIZE, PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "AA");
    pfuWidgetValue(wid, Shared->antialias);
    pfuWidgetDefaultValue(wid, Shared->antialias);
    pfuWidgetFunc(wid, PanelControl);
    Shared->wAA = wid;
    
    x += PFUGUI_BUTTON_XINC;
    
    if (Shared->input == PFUINPUT_X)
    {
	/* menu button - cursor selection */
	wid = pfuNewWidget(pan, PFUGUI_MENU_BUTTON, CTL_CURSOR); 
	pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_Vint_XSIZE,  PFUGUI_BUTTON_YSIZE);
	pfuWidgetLabel(wid, "Cursors");
	pfuWidgetFunc(wid, PanelControl);
	pfuWidgetSelections(wid, cursor_strs, cursor_vals, NULL, NUM_CURSORS);
	Shared->wFModel = wid;
	x += PFUGUI_BUTTON_Vint_XINC;
    }
    
    x = (int)xo;
    y -= PFUGUI_SLIDER_YINC;
	
	/* horizontal slider */
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_HSLIDER);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "Horiz Slider");
    pfuWidgetRange(wid, 1, 1000.0f, 10000.0f, 5000.0f);
    pfuWidgetFunc(wid, PanelControl);
    
     y -= PFUGUI_SLIDER_YINC;
	
	/* horizontal logrithmic slider */
    wid = pfuNewWidget(pan, PFUGUI_SLIDER_LOG, CTL_HSLIDER);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "HLogSlider");
    pfuWidgetRange(wid, 1, 1.0f, 100000.0f, 5000.0f);
    pfuWidgetFunc(wid, PanelControl);
    
    y -= PFUGUI_BUTTON_YINC;
    
        /* radio  button - horizonal */
    wid = pfuNewWidget(pan, PFUGUI_RADIO_BUTTON_TOGGLE | PFUGUI_HORIZONTAL, CTL_HRADIO);
    pfuWidgetLabel(wid, "HRadio");
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetFunc(wid, PanelControl);
    pfuWidgetSelections(wid, num_strs, NULL, NULL, 3);
    
    x += PFUGUI_SLIDER_XINC;
    
	/* Vertical slider */
    wid = pfuNewWidget(pan, PFUGUI_SLIDER | PFUGUI_VERTICAL, CTL_HSLIDER);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_int_XSIZE,  5*PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Vert Slider");
    pfuWidgetRange(wid, 1, 1000.0f, 10000.0f, 5000.0f);
    pfuWidgetFunc(wid, PanelControl);
    
    x += PFUGUI_BUTTON_int_XINC;
    
        /* radio  button - vertical */
    wid = pfuNewWidget(pan, PFUGUI_RADIO_BUTTON_TOGGLE | PFUGUI_VERTICAL, 
				CTL_HRADIO);
    pfuWidgetLabel(wid, "Vert Radio");
    pfuWidgetDim(wid, x, y, PFUGUI_BUTTON_int_XSIZE, 4*PFUGUI_BUTTON_YSIZE);
    pfuWidgetFunc(wid, PanelControl);
    pfuWidgetSelections(wid, num_strs, NULL, NULL, 2);
    
      
    return(pan);
}


static void
KillGUI(pfuWidget *w)
{
    if (pfuGetWidgetValue(w) == 0.0f)
    {
	Shared->gui = 0;
	Shared->updateChannels = 1;
    }
}

/*
* PanelControl
* -------------
* This function is called from the draw process from the currently
* active widget. 
*/

static void
PanelControl(pfuWidget *w)
{
    float val;
    switch(pfuGetWidgetId(w))
    {
    case CTL_RESET_ALL:
	pfuResetGUI();
	resetPosition();
	break;
    case CTL_REPOSITION:
	resetPosition();
	break;
    case CTL_QUIT:
	Shared->exitFlag = 1;
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		" Have Quit button - bye...\n");
	break;
    case CTL_MODEL:
	val = pfuGetWidgetValue(w);
	Shared->xformerMode = val;
	XformerMode(val, Shared->collideMode);
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		" Have Viewing Model: %.0f\n", val);
	break;
    case CTL_DIAGS:
	Shared->drawStats = pfuGetWidgetValue(w);
	break;
    case CTL_AA:
	Shared->antialias = pfuGetWidgetValue(w);
	break;
    case CTL_CURSOR:
	pfuCursor(PFU_CURSOR_MAIN, (int) pfuGetWidgetValue(w));
	break;
    case CTL_HRADIO:	
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
	    "Have GUI Widget \"%s\",  on=%d id=%d, val = %f\n", 
	    pfuGetWidgetLabel(w), pfuIsWidgetOn(w), 
	    pfuGetWidgetId(w), pfuGetWidgetValue(w));
	break;
    case CTL_HSLIDER:
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
	    "Have GUI Widget \"%s\",  id=%d, val = %f\n", 
	    pfuGetWidgetLabel(w),
	    pfuGetWidgetId(w), pfuGetWidgetValue(w));
	break;
    default:
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
	    "Have GUI Widget \"%s\": id=%d, val = %d\n", 
	    pfuGetWidgetLabel(w), pfuGetWidgetId(w), pfuGetWidgetValue(w));
	break;
    }
}

static void
updateGUIViewMsg(pfCoord *viewpt)
{
    static char     posBuf[256];

    sprintf(posBuf," X%5.1f Y%5.1f Z%5.1f H%5.1f P%5.1f R%5.1f",
	    viewpt->xyz[PF_X],viewpt->xyz[PF_Y],viewpt->xyz[PF_Z],
	    viewpt->hpr[PF_H],viewpt->hpr[PF_P],viewpt->hpr[PF_R]);
    pfuWidgetLabel(Shared->wPosXYZ, posBuf);
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
    
    qdevice(HOMEKEY);
    
    pfInitGfx(p);
    
    InitGraphics();
}


static void
OpenXPipeline(pfPipe * p)
{
    pfuGLXWindow *win;

    XDpy = (Display *) pfuOpenXDisplay(0);
    
    if (!(win = pfuGLXWinopen((pfuXDisplay*) XDpy, p, Name, 	
			    WinXOrigin, WinXOrigin + WinXSize, 
			    WinYOrigin, WinYOrigin + WinYSize)))
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, 
	    "OpenXPipeline: Could not create GLX Window.\n");
    }
    
    /* initialize Performer for GLX rendering */
    Shared->xWinInfo = *win;
    pfInitGLXGfx(p, XDpy, win->xWin, win->glWin, win->overWin,
				ResizeMode);
    InitGraphics();
}

static void
InitGraphics(void)
{
    static const pfVec3  clrt = { 0.05f, 0.3f, 0.3f};
    static char msg[] = "Welcome to OpenGL Performer";
    
     /* Draw introductory message */
    c3f(clrt);
    clear();
    /* Message will be draw in in purple - the pfuDrawMessage() color */
    pfuDrawMessage(NULL, msg, PFU_MSG_PIPE, PFU_CENTER_JUSTIFIED, 
		   	0.5f, 0.5f, PFU_FONT_BIG, PFU_RGB);
    swapbuffers();

    /*
     * Graphics Initialization
     */
    
    /* create a default texture environment */
    pfApplyTEnv(pfNewTEnv(NULL));
    
    { /* create a default lighting model and material*/
	pfMaterial *mat;
	pfApplyLModel(pfNewLModel(NULL));
	mat = pfNewMtl(NULL);
	pfApplyMtl(mat);
	pfMtlSide(mat, PFMTL_BACK);
	pfApplyMtl(mat);
    }
    
    /* enable culling of back-facing polygons */
    pfCullFace(PFCF_BACK);
    
    /*
     * These enables should be set to reflect the majority of the
     * database. If most geometry is not textured, then texture
     * should be disabled. However, you then need to change the
     * FLIGHT-format file reader. (pfflt.c)
     */
    pfEnable(PFEN_LIGHTING);
    pfEnable(PFEN_FOG);
    
    /* query the antialias default */
    Shared->antialias = pfGetAntialias();
    
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
    (data); /* avoid compiler warn about unused vars */
    
    { /* put this first because it may destroy the old win and
       * create a new one (in GLX mode)
       */
	static aa=-1;
	if (aa == -1)
	    aa = Shared->antialias;
	else if (aa != Shared->antialias)
	{
	    if (Shared->input == PFUINPUT_X)
	    {
		pfPipe *p = pfGetChanPipe(channel);
		pfuPipeGLXMSConfig(p, Shared->antialias);
		pfuGetGLXWin(p, &(Shared->xWinInfo));
	    }
	    pfAntialias(Shared->antialias);
	    aa = Shared->antialias;
	}
    }
    
    /* erase framebuffer and draw Earth-Sky model */
    pfClearChan(channel);
    
    /* invoke Performer draw-processing for this frame */
    pfDraw();
    
    
    
    /* Enable drawing of Performer throughput statistics */
    if (Shared->drawStats)
	pfDrawChanStats(channel);
    
    if (Shared->redraw && Overlay)
    {
	drawOverlayName();
	Shared->redraw--;
    }

    /* Check the mouse and keyboard = only needed for GL input 
     * This is done after the draw becuase it can happen in parallel
     * with the pipe finishing the rendering of the frame.
     */
    pfuCollectInput();     
}

static void
drawOverlayName(void)
{
    static char msg[] = "OpenGL Performer";
    static int mapped = 0;
	    
    if (Shared->input == PFUINPUT_GL)
    {
	drawmode(Overlay);
	if (!mapped) 
	{
	    mapcolor(1, 71, 9, 82);
	    mapcolor(2, 0, 0, 0);
	    gflush();
	    mapped = 1;
	}
    }
    else
    {		    
	if (!mapped) /* store in X */
	{
	    /* text color - purple */
	    pfuGLXMapcolor(Shared->xWinInfo.dsp, 
		    Shared->xWinInfo.overWin, 
		    1, 0.9f, 0.0f, 0.9f);
	    /* shadow color - drk grey */
	    pfuGLXMapcolor(Shared->xWinInfo.dsp, 
		    Shared->xWinInfo.overWin, 
		    2, 0.125f, 0.06f, 00.125f);
	    mapped = 1;
	}
	GLXwinset(Shared->xWinInfo.dsp, 
		Shared->xWinInfo.overWin);
    }
    /* Clear overlay planes in entire window */
    color(0);
    clear();
    pfuDrawMessageCI(Shared->chan, 
	    msg, PFU_MSG_CHAN, PFU_RIGHT_JUSTIFIED, 
	    1.0f, 1.0f, PFU_FONT_MED, 1, 2);
    if (Shared->input == PFUINPUT_GL)
	drawmode(NORMALDRAW);
    else
	GLXwinset(Shared->xWinInfo.dsp, 
	    Shared->xWinInfo.glWin);
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
    (channel, data); /* avoid compiler warn about unused vars */
    
    /* 
     * pfDrawGeoSet or other display listable Performer routines
     * could be invoked before or after pfCull()
     */
    
    pfCull();
}

