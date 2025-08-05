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
 * drive.c
 * 
 * $Revision: 1.1 $
 * $Date: 2000/11/21 21:39:39 $
 *
 * Run-time controls:
 *       ESC-key: exits
 *        F1-key: profile
 *    Left-mouse: advance
 *  Middle-mouse: stop
 *   Right-mouse: retreat
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h> /* for sigset for forked X event handler process */
#include <getopt.h> /* for cmdline handler */
#include <X11/keysym.h>
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfui.h>
#include <Performer/pfdu.h>

/*
 * structure that resides in shared memory so that the
 * application, cull, and draw processes can access it.
 */
typedef struct
{
    pfPipeWindow    *pw;
    int		    exitFlag;
    int		    inWindow, reset, stop;
    float	    mouseX, mouseY;
    int		    winSizeX, winSizeY;
    int		    mouseButtons;
    pfCoord	    view, viewOrig;
    float	    sceneSize;
    int		    drawStats;
    pfiInputXformDrive *drive;
    pfiInputCoord   *driveInput;
    int		    XInputInited;
} SharedData;

static SharedData *Shared;

/*
 * APP process variables
 */
/* for configuring multi-process */
static int ProcSplit = PFMP_APPCULLDRAW;
/* write out scene upon read-in - uses pfDebugPrint */
static int WriteScene = 0;
static int FullScreen = 0;
static int WinType = PFWIN_TYPE_X;
static int ForkedXInput = 0;
char ProgName[PF_MAXSTRING];
 
/* light source created and updated in DRAW-process */
static pfLight *Sun;
  
static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static void GetGLInput(void);
static void InitXInput(pfWSConnection dsp);
static void DoXInput(void);
static void GetXInput(Display *dsp);
static void Usage(void);

/* Drive callback functions */
static int StartMotionCB(pfiInputXform *ix, void *data);
static int StopMotionCB(pfiInputXform *ix, void *data);


/*
 *	Usage() -- print usage advice and exit. This procedure
 *	is executed in the application process.
 */

static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, 
	    "Usage: %s [-m procSplit] [file.ext ...]\n", ProgName);
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
    while ((opt = getopt(argc, argv, "fFbm:wxp:?")) != -1)
    {
	switch (opt)
	{
	case 'f': 
	    FullScreen = 1;
	    break;
	case 'F': 
	    ForkedXInput = 1;
	    break;
	case 'm':
	case 'p':
	    ProcSplit = atoi(optarg);
	    break;
	case 'w': 
	    WriteScene = 1;
	    break;
	case 'x': 
	    WinType &= ~(PFWIN_TYPE_X);
	    break;
	case 'b': 
	    WinType |= (PFWIN_NOBORDER);
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
    pfPipeWindow   *pw;
    pfEarthSky     *eSky;
    pfBox           bbox;
    float	    far = 10000.0f;
    void	   *arena;
    pfWSConnection  dsp=NULL;
    
    arg = docmdline(argc, argv);
    
    pfInit();

    /* configure multi-process selection */
    pfMultiprocess(ProcSplit);
    
    /* allocate shared before fork()'ing parallel processes */
    arena = pfGetSharedArena();
    Shared = (SharedData*)pfMalloc(sizeof(SharedData), arena);
    Shared->inWindow = 0;
    Shared->stop = 0;
    Shared->reset = 0;
    Shared->exitFlag = 0;
    Shared->drawStats = 1;
    Shared->XInputInited = 0;
    Shared->mouseButtons = 0;

    /* Load all loader DSO's before pfConfig() forks */
    for (found =  arg; found < argc; found++)
	pfdInitConverter(argv[found]);

    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     */
    pfConfig();

    pfiInit();
    
    /* configure pipes and windows */
    p = pfGetPipe(0);
    Shared->pw = pfNewPWin(p);
    pfPWinName(Shared->pw, "OpenGL Performer");
    pfPWinType(Shared->pw, WinType);
    /* Open and configure the GL window. */
    pfPWinConfigFunc(Shared->pw, OpenPipeWin);
    pfConfigPWin(Shared->pw);

    if (FullScreen)
	pfPWinFullScreen(Shared->pw);
    else
	pfPWinOriginSize(Shared->pw, 0, 0, 300, 300);
    
    /* set off the draw process to open windows and call init callbacks */
    pfFrame();
   
    /* specify directories where geometry and textures exist */
    if (!(getenv("PFPATH")))
        pfFilePath(
                   "."
                   ":./data"
                   ":../data"
                   ":../../data"
                   ":/usr/share/Performer/data"
                   );
    pfNotify(PFNFY_INFO, PFNFY_PRINT,"FilePath: %s\n", pfGetFilePath());
    
    /* load files named by command line arguments */
    scene = pfNewScene();
    for (found = 0; arg < argc; arg++)
    {
	if ((root = pfdLoadFile(argv[arg])) != NULL)
	{
	    pfAddChild(scene, root);
	    found++;
	}
    }

    /* if no files successfully loaded, terminate program */
#if 0
    if (!found)
	Usage();
#endif

    /* Write out nodes in scene (for debugging) */
    if (WriteScene)
    {
	FILE *fp;
	if (fp = fopen("scene.out", "w"))
	{
	    pfPrint(scene, PFTRAV_SELF|PFTRAV_DESCEND, PFPRINT_VB_DEBUG, fp);
	    fclose(fp);
	}
	else
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		"Could not open scene.out for debug printing.");
    }

    /* determine extent of scene's geometry */
    pfuTravCalcBBox(scene, &bbox);
    
    pfFrameRate(30.0f);
    pfPhase(PFPHASE_FREE_RUN);
    
    chan = pfNewChan(p);
    pfAddChan(Shared->pw, chan);
    pfChanTravFunc(chan, PFTRAV_CULL, CullChannel);
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 0.1f, far);
    
    /* Create an earth/sky model that draws sky/ground/horizon */
    eSky = pfNewESky();
    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    pfESkyAttr(eSky, PFES_GRND_HT, -10.0f);
    pfChanESky(chan, eSky);

    pfChanTravMode(chan, PFTRAV_CULL, PFCULL_VIEW|PFCULL_GSET);
    
    /* vertical FOV is matched to window aspect ratio */
    pfChanFOV(chan, 45.0f, -1.0f);
    if (found)
    {
	float sceneSize;
	/* Set initial view to be "in front" of scene */
	
	/* view point at center of bbox */
	pfAddVec3(Shared->view.xyz, bbox.min, bbox.max);
	pfScaleVec3(Shared->view.xyz, 0.5f, Shared->view.xyz);
	
	/* find max dimension */
	sceneSize = bbox.max[PF_X] - bbox.min[PF_X];
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	sceneSize = PF_MIN2(sceneSize, 0.5f * far);
	Shared->sceneSize = sceneSize;
	
	/* offset so all is visible */
	Shared->view.xyz[PF_Y] -=      sceneSize;
	Shared->view.xyz[PF_Z] += 0.25f*sceneSize;	
    }  else
    {
	pfSetVec3(Shared->view.xyz, 0.0f, 0.0f, 100.0f);
	PFSET_VEC3(bbox.min, -5000.0, -5000.0, -1000000.0);
	PFSET_VEC3(bbox.max, 5000.0, 5000.0, 10000000.0);
	Shared->sceneSize = 10000.0f;
    }
    pfSetVec3(Shared->view.hpr, 0.0f, 0.0f, 0.0f);
    pfChanView(chan, Shared->view.xyz, Shared->view.hpr);
    PFCOPY_VEC3(Shared->viewOrig.xyz, Shared->view.xyz);
    PFCOPY_VEC3(Shared->viewOrig.hpr, Shared->view.hpr);
    
    Shared->drive = pfiCreate2DIXformDrive(arena);
    Shared->driveInput = pfiGetIXformInputCoordPtr(Shared->drive);
    
    pfiIXformMotionFuncs(Shared->drive, StartMotionCB, StopMotionCB, NULL);
    
    pfiIXformDBLimits(Shared->drive, &bbox);
    pfiXformerResetCoord(Shared->drive, &Shared->viewOrig);
    pfiIXformCoord(Shared->drive, &(Shared->viewOrig));
    
    /* create forked XInput handling process 
     * since the Shared pointer has already been initialized, that structure
     * will be visible to the XInput process. Nothing else created in the
     * application after this fork whose handles are not put in shared memory
     * (such as the database and channels) will be visible to the
     * XInput process.
     */
    if (WinType & PFWIN_TYPE_X)
    {
	if (ForkedXInput)
	{
	    pid_t	    fpid = 0;
	    if ((fpid = fork()) < 0)
		pfNotify(PFNFY_FATAL, PFNFY_SYSERR, "Fork of XInput process failed.");
	    else if (fpid)
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"XInput running in forked process %d",
		    fpid);
	    else if (!fpid)
		DoXInput();
	}
	else
	{
	    dsp = pfGetCurWSConnection();
	    InitXInput(dsp);
	}
    }
    
    /* main simulation loop */
    while (!Shared->exitFlag)
    {
	/* wait until next frame boundary */
	pfSync();
	/* Start of frame rendering */
	pfFrame();
	
	/* do drive */
	if (Shared->reset)
	{
	    pfiStopIXform(Shared->drive);
	    pfiResetIXformPosition(Shared->drive);
	    Shared->reset = 0x0;
	}
	else
	if (Shared->stop)
	{
	    pfiIXformDriveMode(Shared->drive, 
		PFIXDR_MODE_MOTION, PFIXDR_MOTION_STOP);
	    pfiIXformDriveMode(Shared->drive, 
		PFIXDR_MODE_ACCEL, PFIXDR_ACCEL_NONE);
	    Shared->stop = 0;
	} 
	if (!(Shared->mouseButtons))
	{
	    /* ignore mouse if no mouse buttons down */
	    pfiStopIXform(Shared->drive);
	}
	else 
	{
	    pfiIXformFocus(Shared->drive, 1);
	    switch(Shared->mouseButtons)
	    {
		case Button1Mask:
		    pfiIXformDriveMode(Shared->drive, 
			    PFIXDR_MODE_MOTION, PFIXDR_MOTION_FORWARD);
		    pfiIXformDriveMode(Shared->drive, 
			    PFIXDR_MODE_ACCEL, PFIXDR_ACCEL_NONE);
		    break;
		case Button2Mask:
		    pfiIXformDriveMode(Shared->drive, 
			    PFIXDR_MODE_MOTION, PFIXDR_MOTION_HEADING);
		    break;
		case Button3Mask:
		    pfiIXformDriveMode(Shared->drive, 
			    PFIXDR_MODE_MOTION, PFIXDR_MOTION_REVERSE);
		    pfiIXformDriveMode(Shared->drive, 
			    PFIXDR_MODE_ACCEL, PFIXDR_ACCEL_NONE);
		    break;
		case Button1Mask | Button3Mask:
		    pfiIXformDriveMode(Shared->drive, 
			    PFIXDR_MODE_ACCEL, PFIXDR_ACCEL_INCREASE);
		    break;
		case Button1Mask | Button2Mask:
		case Button3Mask | Button2Mask:
		    pfiIXformDriveMode(Shared->drive, 
			    PFIXDR_MODE_ACCEL, PFIXDR_ACCEL_DECREASE);
		    break;
	    }
	}
	/* let drive spin if no new input */
	if (Shared->mouseButtons) 
	{
	   pfVec2 pos;
	   pfuCalcNormalizedChanXY(&pos[0], &pos[1],
		chan, Shared->mouseX, Shared->mouseY);
           pfiInputCoordVec(Shared->driveInput, pos);
	}
	pfiUpdateIXform(Shared->drive);
	{
	    pfMatrix mat;
	    pfiGetIXformMat(Shared->drive, mat);
	    pfChanViewMat(chan, mat);
	}

	if (!ForkedXInput)
	{
	    if (!Shared->XInputInited)
		InitXInput(dsp);
	    if (Shared->XInputInited)
		GetXInput(dsp);
	}
    }
    
    /* terminate cull and draw processes (if they exist) */
    pfExit();
    
    /* exit to operating system */
    return 0;
}

static void 
InitXInput(pfWSConnection dsp)
{
    Window w;
    
    /* wait for X Window to exist in Performer shared memory */
    if (w = pfGetPWinWSWindow(Shared->pw))
    {
	XSelectInput(dsp, w, PointerMotionMask |
			ButtonPressMask | ButtonReleaseMask | KeyPressMask);
	XMapWindow(dsp, w);
	XFlush(dsp);
	Shared->XInputInited = 1;
    }
}

/*
 * DoXInput() runs an asychronous forked even handling process.
 *  Shared memory structures can be read from this process
 *  but NO performer calls that set any structures should be 
 *  issues by routines in this process.
 */
void
DoXInput(void)
{
    /* windows from draw should now exist so can attach X input handling
     * to the X window 
     */
    Display *dsp = pfGetCurWSConnection();
    Window w;
#ifndef __linux__    
    prctl(PR_TERMCHILD);        /* Exit when parent does */
    sigset(SIGHUP, SIG_DFL);    /* Exit when sent SIGHUP by TERMCHILD */
#endif    
    while (1)
    {
	XEvent          event;
	if (!Shared->XInputInited)
	    InitXInput(dsp);
	if (Shared->XInputInited)
	{
	    XPeekEvent(dsp, &event);
	    GetXInput(dsp);
	}		
    }
}

/*
 *	CullChannel() -- traverse the scene graph and generate a
 * 	display list for the draw process.  This procedure is 
 *	executed in the CULL process.
 */

static void
CullChannel(pfChannel *channel, void *data)
{
    /* 
     * pfDrawGeoSet or other display listable Performer routines
     * could be invoked before or after pfCull()
     */
    
    (channel, data);
    
    pfCull();
}

/*
 *	OpenPipeWin() -- create a win: setup the GL and OpenGL Performer.
 *	This procedure is executed in the DRAW process 
 *	(when there is a separate draw process).
 */
 
static void
OpenPipeWin(pfPipeWindow *pw)
{
    pfPipe *p;
    Window w;
    Display *dsp;
    
    pfOpenPWin(pw);
    /* create a light source in the "south-west" (QIII) */
    Sun = pfNewLight(NULL);
    pfLightPos(Sun, -0.3f, -0.3f, 1.0f, 0.0f);
}


/*
 *	DrawChannel() -- draw a channel and read input queue. this
 *	procedure is executed in the draw process (when there is a
 *	separate draw process).
 */
static void
DrawChannel (pfChannel *channel, void *data)
{
    (channel, data);

    /* rebind light so it stays fixed in position */
    pfLightOn(Sun);

    /* erase framebuffer and draw Earth-Sky model */
    pfClearChan(channel);
    
    /* invoke Performer draw-processing for this frame */
    pfDraw();

    /* draw Performer throughput statistics */
    
    if (Shared->drawStats)
	pfDrawChanStats(channel);
	
    /* read window origin and size (it may have changed) */
    pfGetPWinSize(pfGetChanPWin(channel), &Shared->winSizeX, &Shared->winSizeY);


}

static void
GetXInput(Display *dsp)
{
    static int x=0, y=0;
    
    while (XPending(dsp))
    {
	XEvent event;
	    
	XNextEvent(dsp, &event);

	switch (event.type) 
	{
	case ConfigureNotify:
	    break;
	case FocusIn:
	    Shared->inWindow = 1;
	    break;
	case FocusOut:
	    Shared->inWindow = 0;
	    break;
	case MotionNotify: 
	    {
		XMotionEvent *motion_event = (XMotionEvent *) &event;
		x =  motion_event->x;
		y = Shared->winSizeY - motion_event->y;
	    }
	    break;
	case ButtonPress: 
	{
	    XButtonEvent *button_event = (XButtonEvent *) &event;
	    x = event.xbutton.x;
	    y = Shared->winSizeY - event.xbutton.y;
	    Shared->inWindow = 1;
	    switch (button_event->button) {
		case Button1:
		    Shared->mouseButtons |= Button1Mask;
		    break;
		case Button2:
		    Shared->mouseButtons |= Button2Mask;
		    break;
		case Button3:
		    Shared->mouseButtons |= Button3Mask;
		    break;
	    }
	}
	break;
	case ButtonRelease:
	{
	    XButtonEvent *button_event = (XButtonEvent *) &event;
	    switch (button_event->button) {
		case Button1:
		    Shared->mouseButtons &= ~Button1Mask;
		    break;
		case Button2:
		    Shared->mouseButtons &= ~Button2Mask;
		    break;
		case Button3:
		    Shared->mouseButtons &= ~Button3Mask;
		    break;
	    }
	}
	break;
	case KeyPress:
	{
	    char buf[100];
	    int rv;
	    KeySym ks;
	    rv = XLookupString(&event.xkey, buf, sizeof(buf), &ks, 0);
	    switch(ks) {
	    case XK_Escape: 
		Shared->exitFlag = 1;
		exit(0);
		break;
	    case XK_space:
		Shared->stop = 1;
		break;
	    case XK_r:
		Shared->reset = 1;
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Reset");
		break;
	    case XK_g:
		Shared->drawStats = !Shared->drawStats;
		break;
	    default:
		break;
	    }
	}
	break;
	default:
	    break;
	}/* switch */
    }
    Shared->mouseX = x;
    Shared->mouseY = y;
}

static int 
StartMotionCB(pfiInputXform *ix, void *data)
{ 
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"StartMotionCB - drive motion starting.");
}

static int 
StopMotionCB(pfiInputXform *ix, void *data)
{ 
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"StopMotionCB - drive motion stopping.");
}

