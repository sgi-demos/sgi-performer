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
 * fly.c
 * 
 * $Revision: 1.1 $
 * $Date: 2002/04/19 16:31:08 $
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
#include <Performer/pfutil/pfuJoystick.h>

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
    int		    pitchMode;
    pfiInputXformFly *fly;
    pfiInputCoord   *flyInput;
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
static int flybox = 0;
char ProgName[PF_MAXSTRING];

static pfuJoystick *js;
 
/* light source created and updated in DRAW-process */
static pfLight *Sun;
  
void show_buttons(float *analog, int dig);

static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static void GetGLInput(void);
static void InitXInput(pfWSConnection dsp);
static void DoXInput(void);
static void GetXInput(Display *dsp);
static void Usage(void);

/* Fly callback functions */
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
    while ((opt = getopt(argc, argv, "jfFbm:wxp:?")) != -1)
    {
	switch (opt)
	{
	case 'j': 
	    flybox = 1;
	    break;
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
    float analog[8];
    int dig;
    float oldanalog=0;

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
    Shared->drawStats = 0;
    Shared->XInputInited = 0;
    Shared->mouseButtons = 0;
    Shared->pitchMode = PFIXFLY_PITCH_FLIGHT;

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
    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_SKY);
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
    
    Shared->fly = pfiCreate2DIXformFly(arena);
    Shared->flyInput = pfiGetIXformInputCoordPtr(Shared->fly);
  
    js = pfuNewJoystick();
    if(flybox)
    {
	pfuJoystickType(js, PFU_FLYBOX);
    }
    else
    {
	pfuJoystickType(js, PFU_JOYSTICK);
    }
    pfuInitJoystick(js, NULL);

    pfiIXformMotionFuncs(Shared->fly, StartMotionCB, StopMotionCB, NULL);
    pfiIXformDBLimits(Shared->fly, &bbox);
    pfiXformerResetCoord(Shared->fly, &Shared->viewOrig);
    pfiIXformCoord(Shared->fly, &(Shared->viewOrig));
    
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

printf("\033[1;1H\033[2J\n");
/*    
*/    
    /* main simulation loop */
    while (!Shared->exitFlag)
    {
	/* wait until next frame boundary */
	pfSync();
	
	/* initiate traversal using current state */
	pfFrame();
	
	if (Shared->reset)
	{
	    pfiStopIXform(Shared->fly);
	    pfiResetIXformPosition(Shared->fly);
	    Shared->reset = 0x0;
	}
	else
	if (Shared->stop)
	{
	    pfiIXformFlyMode(Shared->fly, 
		PFIXFLY_MODE_MOTION, PFIXFLY_MOTION_STOP);
	    Shared->stop = 0;
	}
#if USE_MOUSE
	if (!Shared->mouseButtons)
	{
	    /* ignore mouse if no buttons down */
	    pfiStopIXform(Shared->fly);
	}
	else
#endif
	{
	    /* do fly */
            pfuGetJoystick(js, analog, &dig);
	    pfiIXformFocus(Shared->fly, 1);
	    pfiIXformFlyMode(Shared->fly, PFIXFLY_MODE_PITCH, Shared->pitchMode);
            if(fabs(analog[3] - oldanalog) < 0.04)
            {
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_ACCEL, PFIXFLY_ACCEL_NONE);
            }
            if(analog[3] > oldanalog)
	    {
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_MOTION, PFIXFLY_MOTION_FORWARD);
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_ACCEL, PFIXFLY_ACCEL_INCREASE);
            }
            else if(analog[3] < oldanalog)
            {
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_MOTION, PFIXFLY_MOTION_REVERSE);
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_ACCEL, PFIXFLY_ACCEL_DECREASE);
            }
	    oldanalog = analog[3];
            pfuPrintJoystick(js);

#if USE_MOUSE
	    switch(Shared->mouseButtons)
	    {
		case Button1Mask:
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_MOTION, PFIXFLY_MOTION_FORWARD);
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_ACCEL, PFIXFLY_ACCEL_NONE);
		    break;
		case Button2Mask:
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_MOTION, PFIXFLY_MOTION_HP);
		    break;
		case Button3Mask:
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_MOTION, PFIXFLY_MOTION_REVERSE);
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_ACCEL, PFIXFLY_ACCEL_NONE);
		    break;
		case Button1Mask | Button3Mask:
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_ACCEL, PFIXFLY_ACCEL_INCREASE);
		    break;
		case Button1Mask | Button2Mask:
		    pfiIXformFlyMode(Shared->fly, 
			    PFIXFLY_MODE_ACCEL, PFIXFLY_ACCEL_DECREASE);
		    break;
	    }
#endif
	}

	/* let fly spin if no new input */
	{
	   pfVec2 pos;
           pos[0] = analog[0] + 0.5f;	
           pos[1] = analog[1] + 0.5f;	
           pfiInputCoordVec(Shared->flyInput, pos);
	}

	pfiUpdateIXform(Shared->fly);
	{
	    pfMatrix mat;
	    pfiGetIXformMat(Shared->fly, mat);
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
		pfExit();
		exit(0);
		break;
	    case XK_space:
		Shared->stop = 1;
		break;
	    case XK_r:
		Shared->reset = 1;
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Reset");
		break;
	    case XK_p:
		if (Shared->pitchMode == PFIXFLY_PITCH_FLIGHT)
		    Shared->pitchMode = PFIXFLY_PITCH_FOLLOW;
		else
		    Shared->pitchMode = PFIXFLY_PITCH_FLIGHT;
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
#if USE_MOUSE
    Shared->mouseX = x;
    Shared->mouseY = y;
#endif
}

static int 
StartMotionCB(pfiInputXform *ix, void *data)
{ 
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"StartMotionCB - fly motion starting.");
}

static int 
StopMotionCB(pfiInputXform *ix, void *data)
{ 
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"StopMotionCB - fly motion stopping.");
}


void show_buttons(float *analog, int dig)
{
/*
"\033[y;xH" - goto position x,y
"\033[2J"   - clear screen
*/
#define THROTTLE_ROW    8
#define THROTTLE_COLUMN 8
#define BUTTON_ROW      8
#define BUTTON_COLUMN   26
#define TRIGGER_ROW     12
#define TRIGGER_COLUMN  45
#define HAT_ROW         8
#define HAT_COLUMN      45
#define AXIS_ROW        8
#define AXIS_COLUMN     60
	int i, r, c;

	/* Throttle */
	printf("\033[%u;%uH   Throttle\n", THROTTLE_ROW, THROTTLE_COLUMN);
	printf("\033[%u;%uH%.2f\n", THROTTLE_ROW+2, THROTTLE_COLUMN, analog[3]);
	printf("\033[%u;%uH%.2f\n", THROTTLE_ROW+2, THROTTLE_COLUMN+8, analog[4]);

	/* Buttons */
	printf("\033[%u;%uH  Buttons\n", BUTTON_ROW, BUTTON_COLUMN);
	for (i=0; i<8; i++) 
	{
		printf("\033[%u;%uH [%c]\n", (i%2)?BUTTON_ROW+i:BUTTON_ROW+1+i, (i%2)?BUTTON_COLUMN+6:BUTTON_COLUMN, (dig>>i&1)?'X':' ');
	}

	/* Trigger */
	printf("\033[%u;%uH Trigger", TRIGGER_ROW, TRIGGER_COLUMN);
	printf("\033[%u;%uH [%c]\n", TRIGGER_ROW+1, TRIGGER_COLUMN+2, (dig>>8&1)?'X':' ');

	/* Hat */
	/* see man iso_8859_1 for the characters used */
	printf("\033[%u;%uH   Hat", HAT_ROW, HAT_COLUMN);
	printf("\033[%u;%uH   \267  ", HAT_ROW+1, HAT_COLUMN+1);  /* middle dot, top */
	printf("\033[%u;%uH  \267  ", HAT_ROW+2, HAT_COLUMN);  /* middle dot, left */
	printf("\033[%u;%uH  \267  ", HAT_ROW+2, HAT_COLUMN+4);  /* middle dot, right */
	printf("\033[%u;%uH   \267   ", HAT_ROW+3, HAT_COLUMN+1);  /* middle dot, bottom */
	r = (HAT_ROW + 2) + (dig>>11&1) - (dig>>12&1);
        c = (HAT_COLUMN + 4) - (dig>>9&1)*2 + (dig>>10&1)*2;
	printf("\033[%u;%uH\244", r, c);  /* currency sign, center */

	printf("\033[%u;%uH[%c]", HAT_ROW+2, HAT_COLUMN-2, (dig>>13&1)?'X':' ');  /* left button */
	printf("\033[%u;%uH[%c]", HAT_ROW+2, HAT_COLUMN+8, (dig>>14&1)?'X':' ');  /* right button */

	/* Axis */
	printf("\033[%u;%uH      Axis", AXIS_ROW, AXIS_COLUMN);
	printf("\033[%u;%uH     X: %.2f", AXIS_ROW+1, AXIS_COLUMN, analog[0]);
	printf("\033[%u;%uH     Y: %.2f", AXIS_ROW+2, AXIS_COLUMN, analog[1]);
	printf("\033[%u;%uH Twist: %.2f", AXIS_ROW+3, AXIS_COLUMN, analog[2]);

	printf("\033[1;1H");
	fflush(stdout);
}

