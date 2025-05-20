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
 * bins.c
 * 
 * This simple example add bin usage to complex.c. 
 * Two bins are created, and the database is traversed so 1/3 of the objects
 * are marked to go in the first bin, 1/3 go in the other bin. The other third
 * are not marked, so they will end up in the scene DList
 * By pressing Space during run time, the program toggle between pfDraw() and
 * pfDrawBin(). 
 *
 * $Revision: 1.14 $ 
 * $Date: 2000/10/06 21:26:29 $
 *
 * Command-line options:
 *  -b	: norborder window
 *  -f	: full screen
 *  -F	: put X input handling in a forked process
 *  -m procsplit : multiprocess mode
 *  -w	: write scene to file
 *
 * Run-time controls:
 *       ESC-key: exits
 *        F1-key: profile
 *        SPACE : toggle pfDrawBin / pfDraw
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
#include <Performer/pfdu.h>

/*
 * structure that resides in shared memory so that the
 * application, cull, and draw processes can access it.
 */
typedef struct
{
    pfPipeWindow    *pw;
    int		    exitFlag;
    int		    inWindow, reset;
    float	    mouseX, mouseY;
    int		    winSizeX, winSizeY;
    int		    mouseButtons;
    pfCoord	    view, viewOrig;
    float	    sceneSize;
    float	    accelRate;
    int		    drawStats;
    int		    XInputInited;
    int		    drawBins;
} SharedData;

static SharedData *Shared;
static pfDCS *objDCS;

static Atom	wm_protocols, wm_delete_window;

/*
 * APP process variables
 */
/* for configuring multi-process */
static int ProcSplit = PFMP_DEFAULT;
/* write out scene upon read-in - uses pfDebugPrint */
static int WriteScene = 0;
static int FullScreen = 0;
static int WinType = PFPWIN_TYPE_X;
static int NoBorder = 0;
static int ForkedXInput = 0;
char ProgName[PF_MAXSTRING];
 
/* light source created and updated in DRAW-process */
static pfLight *Sun;
static int *myBin;
  
static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static void UpdateView(void);
static void GetGLInput(void);
static void InitXInput(pfWSConnection dsp);
static void DoXInput(void);
static void GetXInput(Display *dsp);
static void Usage(void);
static void assignBins(pfNode *node);


/*
 *	Usage() -- print usage advice and exit. This procedure
 *	is executed in the application process.
 */

static void
Usage (void)
{
    fprintf(stderr,"Usage: %s [-m procSplit] [-f] [-b] file.ext [file.ext...]\n", ProgName);
    fprintf(stderr, "  Then press Space to toggle from pfDraw to pfDrawBin\n");
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
    while ((opt = getopt(argc, argv, "fFbm:wxXp:?")) != -1)
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
	    break;
	case 'X': 
	    WinType |= PFPWIN_TYPE_NOXEVENTS;
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
    pfWSConnection  dsp=NULL;
    pfMatrix	    mat,rotMat;
    
    arg = docmdline(argc, argv);
    
    pfInit();

    /* configure multi-process selection */
    pfMultiprocess(ProcSplit); 
    
    /* allocate shared before fork()'ing parallel processes */
    Shared = (SharedData*)pfCalloc(1,sizeof(SharedData), pfGetSharedArena());
    Shared->drawStats = 0;
    Shared->drawBins = -2;

    /* Load all loader DSO's before pfConfig() forks */
    for (found = arg; found < argc; found++)
	pfdInitConverter(argv[found]);

    /* allocate stuff in shared memory */
    {
        void *arena =  pfGetSharedArena();
        myBin = (int*) pfMalloc(sizeof(int),arena);
    }

    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     */

    pfConfig();

    /* configure pipes and windows */
    p = pfGetPipe(0);
    Shared->pw = pfNewPWin(p);
    pfPWinName(Shared->pw, "OpenGL Performer");
    pfPWinType(Shared->pw, WinType);
    if (NoBorder)
	pfPWinMode(Shared->pw, PFWIN_NOBORDER, 1);
    pfPWinMode(Shared->pw, PFWIN_EXIT, 1);
    /* Open and configure the GL window. */
    pfPWinConfigFunc(Shared->pw, OpenPipeWin);
    pfConfigPWin(Shared->pw);

    if (FullScreen)
	pfPWinFullScreen(Shared->pw);
    else
	pfPWinOriginSize(Shared->pw, 0, 0, 512, 512);
    
    /* set off the draw process to open windows and call init callbacks */
    pfFrame();
   
    /* specify directories where geometry and textures exist */
    if (!(getenv("PFPATH")))
        pfFilePath(
                   "."
                   ":./data"
                   ":../data"
                   ":../../data"
                   ":../../../../data"
                   ":/usr/share/Performer/data"
                   );
    pfNotify(PFNFY_INFO, PFNFY_PRINT,"FilePath: %s\n", pfGetFilePath());
    
    /* load files named by command line arguments */
    scene = pfNewScene();
    for (found = 0; arg < argc; arg++)
    {
	if ((root = pfdLoadFile(argv[arg])) != NULL)
	{
	    objDCS = pfNewDCS();
	    pfAddChild(scene, objDCS);
	    pfAddChild(objDCS, root);
	    found++;
	}
    }

    /* if no files successfully loaded, terminate program */
    if (!found)
	Usage();

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

    pfChanTravMode(chan, PFTRAV_CULL, PFCULL_ALL);
    
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
	Shared->view.xyz[PF_Y] -= 1.5*sceneSize;
	Shared->view.xyz[PF_Z] += 0.1f*sceneSize;	
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
   

    /* set up bin order for user bins */
    *myBin = pfGetChanFreeBin(chan);

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Free bin id = %d",*myBin);

    /* mark the bin in used, and insert my bins in between the Opaque
       and the Transparent bin. 
       We do not touch the LPSTATE_BIN as it is created with an order
       of 9999, and touching it will create it even if not needed (no
       light point process)
    */


    pfChanBinOrder(chan, PFSORT_OPAQUE_BIN, 0);
    pfChanBinOrder(chan, *myBin, 1);
    pfChanBinOrder(chan, *myBin+1, 2);
    pfChanBinOrder(chan, PFSORT_TRANSP_BIN, 3);

    /* No sorting inside the two new bins */

    pfChanBinSort(chan, *myBin, PFSORT_DRAW_ORDER, NULL);
    pfChanBinSort(chan, *myBin+1, PFSORT_DRAW_ORDER, NULL);
        
    assignBins ((pfNode *)scene);

    
    /* create forked XInput handling process 
     * since the Shared pointer has already been initialized, that structure
     * will be visible to the XInput process. Nothing else created in the
     * application after this fork whose handles are not put in shared memory
     * (such as the database and channels) will be visible to the
     * XInput process.
     */
    dsp = pfGetCurWSConnection();
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
	    if (!fpid)
		DoXInput();
	}
    }
    else
	ForkedXInput = 0;
    
    /* set the frame/frame rotation */
    pfMakeRotMat(rotMat, 0.7f, 0.0f, 0.0f, 1.0f);

    /* main simulation loop */
    while (!Shared->exitFlag)
    {
	
	/* wait until next frame boundary */
	pfSync();
	
	/* initiate traversal using current state */
	pfFrame();
	
	/* Set view parameters for next frame */
	UpdateView();
	pfChanView(chan, Shared->view.xyz, Shared->view.hpr);
	
	if (!ForkedXInput)
	{
	    if (!Shared->XInputInited)
		InitXInput(dsp);
	    if (Shared->XInputInited)
		GetXInput(dsp);
	}

	/* rotate the object */
	pfMultMat(mat, *pfGetDCSMatPtr(objDCS), rotMat);
	pfDCSMat(objDCS,mat);


    }
    
    /* terminate cull and draw processes (if they exist) */
    pfExit();
    
    /* exit to operating system */
    exit(0);
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
	wm_protocols = XInternAtom(dsp, "WM_PROTOCOLS", 1);
	wm_delete_window = XInternAtom(dsp, "WM_DELETE_WINDOW", 1);
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
static void
DoXInput(void)
{
    /* windows from draw should now exist so can attach X input handling
     * to the X window 
     */
    pfWSConnection dsp = pfGetCurWSConnection();


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
 *	UpdateView() updates the eyepoint based on the information
 *	placed in shared memory by GetInput().
 */
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
    if (Shared->drawBins != -2)
    {
	pfDrawBin(Shared->drawBins);
    } else
        pfDraw();

    /* draw Performer throughput statistics */
    
    if (Shared->drawStats)
	pfDrawChanStats(channel);
	
    /* read window origin and size (it may have changed) */
    pfGetPWinSize(pfGetChanPWin(channel), &Shared->winSizeX, &Shared->winSizeY);

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
	case ClientMessage:
	    if ((event.xclient.message_type == wm_protocols) &&
		(event.xclient.data.l[0] == wm_delete_window)) 
	    {
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Window exit !!");
		pfExit();
	    } 
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
		if (Shared->drawBins == -2)
	          Shared->drawBins = -1;
                else if (Shared->drawBins == -1)
		  Shared->drawBins = PFSORT_OPAQUE_BIN;
		else if (Shared->drawBins == PFSORT_OPAQUE_BIN)
		  Shared->drawBins = PFSORT_TRANSP_BIN;
		else if (Shared->drawBins == PFSORT_TRANSP_BIN)
                  Shared->drawBins = *myBin;
                else if (Shared->drawBins ==*myBin)
                  Shared->drawBins = *myBin+1;
                else
                  Shared->drawBins = -2;
		if (Shared->drawBins != -2)
		    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Calling pfDrawBin(%d)",
				Shared->drawBins);
		else
		    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Calling pfDraw()");
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


static int	flip = -1;
static unsigned int	order  = 12345;

static void
assignBins (pfNode *node)
{
	int		i, n;
	pfGeoSet        *gset;
	pfGeoState	*gstate;

	if(pfIsOfType(node, pfGetGeodeClassType()))
	{
	    n = pfGetNumGSets(node);
	    for(i=0; i<n; i++)
	    {
		   gset = pfGetGSet(node,i);
		   gstate = pfGetGSetGState(gset);
		   /* do not change automatic behavior on transparent objects */
		   if (pfGetGStateMode(gstate,PFSTATE_TRANSPARENCY))
		   {
		     /* set it to blend so it will go in the transparent bin */
		     pfGStateMode(gstate,PFSTATE_TRANSPARENCY, PFTR_BLEND_ALPHA);
		     continue;
		   }
		   if (flip == -1)
		   {
			flip = *myBin;
		   }
		   else if (flip == *myBin)
		   {
			pfGSetDrawBin (gset, flip);
			pfGSetDrawOrder(gset, --order);
			flip = *myBin+1;
		   }
		   else
		   {
			pfGSetDrawBin (gset, flip);
			pfGSetDrawOrder (gset, --order);
			flip = -1;
                   }
	     }
	}
	else
	if(pfIsOfType(node, pfGetGroupClassType()))
	{
	    n = pfGetNumChildren(node);
	    for(i=0; i<n; i++)
	    assignBins(pfGetChild(node, i));
	}
}
