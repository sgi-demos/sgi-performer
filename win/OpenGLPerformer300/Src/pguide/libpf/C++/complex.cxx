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
 */

//
// complexC.C
// 
// OpenGL Performer example using cull and draw process callbacks.
// Mouse and keyboard go through GL which is simpler than mixed
// model (GLX), but does incur some overhead in the draw process.
// X input handling is done in a forked event handling process.
//
// $Revision: 1.24 $ 
// $Date: 2000/10/06 21:26:20 $
//
// Command-line options:
//  -b	: norborder window
//  -f	: full screen
//  -F	: put X input handling in a forked process
//  -m procsplit : multiprocess mode
//  -w	: write scene to file
// 
// Run-time controls:
//       ESC-key: exits
//        F1-key: profile
//    Left-mouse: advance
//  Middle-mouse: stop
//   Right-mouse: retreat

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h> // for sigset for forked X event handler process 
#include <getopt.h> // for cmdline handler 
#include <X11/keysym.h>

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfPipe.h>
#include <Performer/pf/pfChannel.h>

#include <Performer/pr/pfLight.h>

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

//
// structure that resides in shared memory so that the
// application, cull, and draw processes can access it.

typedef struct
{
    pfPipeWindow    *pw;
    int		    exitFlag;
    int		    inWindow, reset;
    float	    mouseX, mouseY;
    int		    winSizeX, winSizeY;
    int		    mouseButtons;
    pfCoord	    view, viewOrig;
    float	    accelRate;
    float	    sceneSize;
    int		    drawStats;
    int		    XInputInited;
} SharedData;

static SharedData *Shared;

//
// APP process variables

// for configuring multi-process 
static int ProcSplit = PFMP_DEFAULT;
// write out scene upon read-in - uses pfDebugPrint 
static int WriteScene = 0;
static int FullScreen = 0;
static int WinType = PFPWIN_TYPE_X;
static int NoBorder = 0;
static int ForkedXInput = 0;
char ProgName[PF_MAXSTRING];

// light source created and updated in DRAW-process 
static pfLight *Sun;

static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static void UpdateView(void);
static void GetGLInput(void);
static void InitXInput(pfWSConnection dsp);
static void DoXInput(void);
static void GetXInput(Display *dsp);
static void Usage(void);


//
//	Usage() -- print usage advice and exit. This procedure
//	is executed in the application process.


static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, 
	     "Usage: %s [-m procSplit] [file.ext ...]\n", ProgName);
    exit(1);
}

//
//	docmdline() -- use getopt to get command-line arguments, 
//	executed at the start of the application process.


static int
docmdline(int argc, char *argv[])
{
    int	    opt;
    
    strcpy(ProgName, argv[0]);
    
    // process command-line arguments 
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
	    WinType &= ~(PFPWIN_TYPE_X);
	    break;
	case 'b': 
	    NoBorder ^= 1;
	    break;
	case '?': 
	    Usage();
	}
    }
    return optind;
}


//
//	main() -- program entry point. this procedure
//	is executed in the application process.


int
main (int argc, char *argv[])
{
    int		    arg;
    int		    found;
    pfPipe         *p;
    pfBox           bbox;
    float	    far = 10000.0f;
    pfWSConnection  dsp=NULL;
    
    arg = docmdline(argc, argv);
    
    pfInit();
    
    // configure multi-process selection 
    pfMultiprocess(ProcSplit);
    
    // allocate shared before fork()'ing parallel processes 
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    Shared->drawStats = 1;
    
    // Load all loader DSO's before pfConfig() forks 
    for (found = arg; found < argc; found++)
	pfdInitConverter(argv[found]);

    // initiate multi-processing mode set in pfMultiprocess call 
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    
    pfConfig();
    
    // configure pipes and windows 
    p = pfGetPipe(0);
    Shared->pw = new pfPipeWindow(p);
    Shared->pw->setName("OpenGL Performer");
    Shared->pw->setWinType(WinType);
    if (NoBorder)
	Shared->pw->setMode(PFWIN_NOBORDER, 1);
    // Open and configure the GL window. 
    Shared->pw->setConfigFunc(OpenPipeWin);
    Shared->pw->config();
    
    if (FullScreen)
	Shared->pw->setFullScreen();
    else
	Shared->pw->setOriginSize(0, 0, 300, 300);
    
    // set off the draw process to open windows and call init callbacks 
    pfFrame();
    
    // create forked XInput handling process 
    // since the Shared pointer has already been initialized, that structure
    // will be visible to the XInput process. Nothing else created in the
    // application after this fork whose handles are not put in shared memory
    // (such as the database and channels) will be visible to the
    // XInput process.
    
    if (WinType & PFPWIN_TYPE_X)
    {
	pid_t	    fpid = 0;
	if (ForkedXInput)
	{
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
	}
    }
    
    // specify directories where geometry and textures exist 
    if (!(getenv("PFPATH")))
        pfFilePath(
                   "."
                   ":./data"
                   ":../data"
                   ":../../data"
                   ":/usr/share/Performer/data"
                   );
    pfNotify(PFNFY_INFO, PFNFY_PRINT,"FilePath: %s\n", pfGetFilePath());
    
    // load files named by command line arguments 
    pfScene *scene = new pfScene();
    for (found = 0; arg < argc; arg++)
    {
        pfNode	   *root;
	if ((root = pfdLoadFile(argv[arg])) != NULL)
	{
	    scene->addChild(root);
	    found++;
	}
    }
    
    // if no files successfully loaded, terminate program 
#if 0
    if (!found)
	Usage();
#endif
    
    // Write out nodes in scene (for debugging) 
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
    
    // determine extent of scene's geometry 
    pfuTravCalcBBox(scene, &bbox);
    
    pfFrameRate(30.0f);
    pfPhase(PFPHASE_FREE_RUN);
    
    pfChannel *chan = new pfChannel(p);
    Shared->pw->addChan(chan);
    chan->setTravFunc(PFTRAV_CULL, CullChannel);
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    chan->setScene(scene);
    chan->setNearFar(0.1f, far);
    
    // Create an earth/sky model that draws sky/ground/horizon 
    pfEarthSky *eSky = new pfEarthSky();
    eSky->setMode(PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    eSky->setAttr(PFES_GRND_HT, -10.0f);
    chan->setESky(eSky);
    
    chan->setTravMode(PFTRAV_CULL, PFCULL_ALL);
    
    // vertical FOV is matched to window aspect ratio 
    chan->setFOV(45.0f, -1.0f);
    if (found)
    {
	float sceneSize;
	// Set initial view to be "in front" of scene 
	
	// view point at center of bbox 
	Shared->view.xyz.add(bbox.min, bbox.max);
	Shared->view.xyz.scale(0.5f, Shared->view.xyz);
	
	// find max dimension 
	sceneSize = bbox.max[PF_X] - bbox.min[PF_X];
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	sceneSize = PF_MIN2(sceneSize, 0.5f * far);
	Shared->sceneSize = sceneSize;
	
	// offset so all is visible 
	Shared->view.xyz[PF_Y] -=      sceneSize;
	Shared->view.xyz[PF_Z] += 0.25f*sceneSize;	
    }  else
    {
	Shared->view.xyz.set(0.0f, 0.0f, 100.0f);
	PFSET_VEC3(bbox.min, -5000.0f, -5000.0f, -1000000.0f);
	PFSET_VEC3(bbox.max, 5000.0f, 5000.0f, 10000000.0f);
	Shared->sceneSize = 10000.0f;
    }
    Shared->view.hpr.set(0.0f, 0.0f, 0.0f);
    chan->setView(Shared->view.xyz, Shared->view.hpr);
    PFCOPY_VEC3(Shared->viewOrig.xyz, Shared->view.xyz);
    PFCOPY_VEC3(Shared->viewOrig.hpr, Shared->view.hpr);

    // main simulation loop 
    while (!Shared->exitFlag)
    {
	// wait until next frame boundary 
	pfSync();
	
	pfFrame();
	
	// Set view parameters for next frame 
	UpdateView();
	chan->setView(Shared->view.xyz, Shared->view.hpr);
	// initiate traversal using current state 
    
	if (!ForkedXInput  && (WinType & PFPWIN_TYPE_X))
	{
	    if (!Shared->XInputInited)
		InitXInput(dsp);
	    if (Shared->XInputInited)
		GetXInput(dsp);
	}
    }
    
    // terminate cull and draw processes (if they exist) 
    pfExit();
    
    // exit to operating system 
    return 0;
}

static void 
InitXInput(pfWSConnection dsp)
{
    Window w;
    
    /* wait for X Window to exist in Performer shared memory */
   if (w = Shared->pw->getWSWindow())
   {
	XSelectInput(dsp, w, PointerMotionMask |
			ButtonPressMask | ButtonReleaseMask | 
			KeyPressMask | KeyReleaseMask);
	XMapWindow(dsp, w);
	XFlush(dsp);
	Shared->XInputInited = 1;
    }
}

//
// DoXInput() runs an asychronous forked even handling process.
//  Shared memory structures can be read from this process
//  but NO performer calls that set any structures should be 
//  issues by routines in this process.

void
DoXInput(void)
{
    // windows from draw should now exist so can attach X input handling
    // to the X window 
    
    Display *dsp = pfGetCurWSConnection();
#ifndef __linux__    
    prctl(PR_TERMCHILD);        // Exit when parent does 
    sigset(SIGHUP, SIG_DFL);    // Exit when sent SIGHUP by TERMCHILD 
#endif    
    InitXInput(dsp);
    
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

// 
//	UpdateView() updates the eyepoint based on the information
//	placed in shared memory by GetInput().

static void    
UpdateView(void)
{
    static float speed = 0.0f;
    pfCoord *view = &Shared->view;
    float cp;
    float mx, my;
    static double thisTime = -1.0f;
    double prevTime;
    float deltaTime;

    prevTime = thisTime;
    thisTime = pfGetTime();

    if (prevTime < 0.0f)
	return;

    if (!Shared->inWindow || Shared->reset)
    {
	speed = 0;
	Shared->reset = 0;
	Shared->accelRate = 0.1f * Shared->sceneSize;
	return;
    }

    deltaTime = thisTime - prevTime;
    switch (Shared->mouseButtons)
    {
    case Button1Mask: /* LEFTMOUSE: faster forward or slower backward*/
    case Button1Mask|Button2Mask:
	speed += Shared->accelRate * deltaTime;
	if (speed > Shared->sceneSize)
	    speed = Shared->sceneSize;
	break;
    case Button3Mask: /* RIGHTMOUSE: faster backward or slower foreward*/
    case Button3Mask|Button2Mask:
	speed -= Shared->accelRate * deltaTime;
	if (speed < -Shared->sceneSize)
	    speed = -Shared->sceneSize;
	break;
    }
    if (Shared->mouseButtons)
    {
	mx = 2.0f * (Shared->mouseX / (float)Shared->winSizeX) - 1.0f;
	my = 2.0f * (Shared->mouseY / (float)Shared->winSizeY) - 1.0f;
				     
	/* update view direction */
	view->hpr[PF_H] -= mx * PF_ABS(mx) * 30.0f * deltaTime;
	view->hpr[PF_P] += my * PF_ABS(my) * 30.0f * deltaTime;
	view->hpr[PF_R]  = 0.0f;
	
	/* update view position */
	cp = cosf(PF_DEG2RAD(view->hpr[PF_P]));
	view->xyz[PF_X] += speed*sinf(-PF_DEG2RAD(view->hpr[PF_H]))*cp;
	view->xyz[PF_Y] += speed*cosf(-PF_DEG2RAD(view->hpr[PF_H]))*cp;
	view->xyz[PF_Z] += speed*sinf( PF_DEG2RAD(view->hpr[PF_P]));
    }
    else
    {
	speed = 0.0f;
	Shared->accelRate = 0.1f * Shared->sceneSize;
    }
}

//
//	CullChannel() -- traverse the scene graph and generate a
// 	display list for the draw process.  This procedure is 
//	executed in the CULL process.


static void
CullChannel(pfChannel *, void *)
{
    // 
    // pfDrawGeoSet or other display listable Performer routines
    // could be invoked before or after pfCull()
    pfCull();
}

//
//	OpenPipeWin() -- create a win: setup the GL and OpenGL Performer.
//	This procedure is executed in the DRAW process 
//	(when there is a separate draw process).


static void
OpenPipeWin(pfPipeWindow *pw)
{
    pw->open();
    
    // create a light source in the "south-west" (QIII) 
    Sun = new pfLight();
    Sun->setPos(-0.3f, -0.3f, 1.0f, 0.0f);
}


//
//	DrawChannel() -- draw a channel and read input queue. this
//	procedure is executed in the draw process (when there is a
//	separate draw process).

static void
DrawChannel (pfChannel *channel, void *)
{
    // rebind light so it stays fixed in position 
    Sun->on();
    
    // erase framebuffer and draw Earth-Sky model 
    channel->clear();
    
    // invoke Performer draw-processing for this frame 
    pfDraw();
    
    // draw Performer throughput statistics 
    
    if (Shared->drawStats)
	channel->drawStats();
    
    // read window origin and size (it may have changed) 
    channel->getPWin()->getSize(&Shared->winSizeX, &Shared->winSizeY);
    
}


static void
GetXInput(pfWSConnection dsp)
{
    static int x=0, y=0;
    
    if (XEventsQueued(dsp, QueuedAfterFlush))
    while (XEventsQueued(dsp, QueuedAlready))
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
		    Shared->reset = 1;
		    PFCOPY_VEC3(Shared->view.xyz, Shared->viewOrig.xyz);
		    PFCOPY_VEC3(Shared->view.hpr, Shared->viewOrig.hpr);
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
	}// switch 
    }
    Shared->mouseX = x;
    Shared->mouseY = y;
}

