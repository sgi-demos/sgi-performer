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
 * motifxformer.c
 * 
 * OpenGL Performer example using the pfiXformer and motif.
 * 
 * This example uses the pfiXformer motion models and user-interface
 *  but uses Motif for all event collection (as opposed to libpfutil input
 *  collection) and hands the expected input information to the pfiXformer.
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
#include <signal.h>
#include <math.h>
#include <getopt.h>		    /* for cmdline handler */
#include <X11/X.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <Xm/MainW.h>		    /* various Motif widget headers */
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/FileSB.h>
#include <Xm/CascadeB.h>
#include <Xm/Frame.h>
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <Performer/pfui.h>
#ifndef MESA
#include <X11/GLw/GLwMDrawA.h>	    /* header for OpenGL Motif widget */
#else
#include <GL/GLwMDrawA.h>	    /* header for OpenGL Motif widget */
#endif

#define HISPEED		0.1f
#define LOSPEED		0.001f


/* used in input process for X mouse tracking */
static int the_X_y_max;
#define X_to_GL_Y(y)	(the_X_y_max - (y))

/*
 * structure that resides in shared memory so that the
 * application, cull, and draw processes can access it.
 */
typedef struct
{
    pfPipeWindow    *pw;
    pfChannel	    *chan;
    pfScene	    *scene;
    int		    exitFlag;
    int		    reset;
    pfuMouse        mouse; /* for communicating with Xformer */
    int		    xformerModel;
    pfiTDFXformer   *xformer;
    pfDCS	    *sceneDCS;
    pfCoord	    view, viewOrig;
    int		    drawStats;
    int		    newScene;
    int		    chanOriginX, chanOriginY;
    char	    loadFileName[PF_MAXSTRING];
    char	    loadFilePath[PF_MAXSTRING];
} SharedData;

typedef struct MotifData_t {
    XVisualInfo *vi;
    Window xWin, gXWin;
    int *config;
    XtWorkProcId work_id;
}MotifData_t;

/* Inter-process communication structures */
static SharedData *Shared;
MotifData_t *Motif;

/* light source created and visible in draw-process */
static pfLight *Sun=NULL;

static float FarClip = 10000.0f;

/* for configuring multi-process */
static int ProcSplit = PFMP_APPCULLDRAW;
static int WriteScene = 0; /* use pfDebugPrint to write out scene upon read */
static int ForkMotif = 0; /* put Motif in a forked process */
static int WinOrgX=0, WinOrgY=0, WinSizeX=300, WinSizeY=300;
char ProgName[PF_MAXSTRING];
   
static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static void InitPipe(void);
static int SimFrame(void);
static void UpdateView(void);
static pfScene * NewScene(char **filename, int num);
static void Usage(void);

/* Motif process routines */
static void DoMotif(int argc, char **argv, int forked);
static int WorkProc(void);
static void InitMotifWidget(Widget w, XtPointer data, 
		GLwDrawingAreaCallbackStruct * cb);
static void GetTopInput(Widget w, void *call, XEvent * event);
static void GetInput(Widget w, void *call, XEvent * eventp);
static void quitCB(Widget w, XtPointer data, XtPointer callData);
static void openCB(Widget foo, XtPointer data, XtPointer callData);


static void initXformer(void);
static void resetPosition(void);

/*
 *	Usage() -- print usage advice and exit. This procedure
 *	is executed in the application process.
 */

static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE,
	"Usage: %s [-p procSplit] [file.ext ...]\n", ProgName);
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
    while ((opt = getopt(argc, argv, "fFm:p:wW:?")) != -1)
    {
	switch (opt)
	{
	case 'f':
	    ForkMotif ^= 1;
	    break;
	case 'm':
	case 'p':
	    ProcSplit = atoi(optarg);
	    break;
	case 'w': 
	    WriteScene = 1;
	    break;
	/* Set the window size */
	case 'W':
	    if (sscanf(optarg, "%d,%d,%d,%d", 
		    &WinOrgX, &WinOrgY, &WinSizeX, &WinSizeY) != 4)
		if (sscanf(optarg, "%d,%d", &WinSizeX, &WinSizeY) != 2)
		    if (sscanf(optarg, "%d", &WinSizeX) == 1)
			WinSizeY = WinSizeX;
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
    pfNode	   *root;
    pfPipe         *p;
    pfEarthSky     *eSky;
    pid_t	    fpid = 0;
    
    arg = docmdline(argc, argv);
    
    /* Performer Initialization */
    pfInit();
    
    /*------------------------------------------------ 
     * Allocate shared before fork()'ing motif so that all processes 
     * see the Shared pointer.
     *------------------------------------------------ 
     */
    Shared = (SharedData*)pfMalloc(sizeof(SharedData), pfGetSharedArena());
    Shared->scene = NULL;
    Shared->mouse.inWin = 0;
    Shared->reset = 1;
    Shared->exitFlag = 0;
    Shared->drawStats = 1;
    Shared->newScene = 0;
    Shared->xformerModel = PFITDF_TRACKBALL;
    
    
   
   /*------------------------------------------------
    * Set global application configuration parameters
    *------------------------------------------------
    */
    
    /* configure multi-process selection */
    pfMultiprocess(ProcSplit);

    {
	/* Load in the DSOs before pfConfig() forking */
	int i;
	for (i = arg ; i < argc ; i++)
	    pfdInitConverter(argv[i]);
    }
    
    /* initiate multi-processing mode set in pfMultiprocess call */
    /* FORK happens HERE !!! */
    pfConfig();	


    pfuInitUtil();
    pfiInit();
       
    pfFrameRate(20.0f);
    pfPhase(PFPHASE_FREE_RUN);
   
   
   /*------------------------------------
    * Load and Create the Database
    *------------------------------------
    */
    
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
    if (arg < argc)
	Shared->scene = NewScene(&(argv[arg]), argc - arg);
    
    /* if no files successfully loaded, terminate program */
    if (!Shared->scene)
	Usage();

    /* Write out nodes in scene (for debugging) */
    if (WriteScene)
    {
	FILE *fp;
	if (fp = fopen("scene.out", "w"))
	{
	    pfPrint(Shared->scene, PFTRAV_SELF|PFTRAV_DESCEND, PFPRINT_VB_DEBUG, fp);
	    fclose(fp);
	}
	else
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		"Could not open scene.out for debug printing.");
    }
 
   /*------------------------------------
    * Create Pipes, Windows, and Channels
    *------------------------------------
    */
    
    /* Create Performer Window */
    p = pfGetPipe(0);
    Shared->pw = pfNewPWin(p);
    pfPWinType(Shared->pw, PFWIN_TYPE_X);
    pfPWinName(Shared->pw, "OpenGL Performer");
    /* set the window initialization callback */
    pfPWinConfigFunc(Shared->pw, OpenPipeWin);
    /* set the request for the configuration of the window */
    pfConfigPWin(Shared->pw);
    
    Shared->chan = pfNewChan(p);
    pfAddChan(Shared->pw, Shared->chan);
    pfChanTravFunc(Shared->chan, PFTRAV_CULL, CullChannel);
    pfChanTravFunc(Shared->chan, PFTRAV_DRAW, DrawChannel);
    pfChanScene(Shared->chan, Shared->scene);
    pfChanNearFar(Shared->chan, 0.1f, FarClip);
    
    initXformer();
    
    /* Set off motif AFTER pfConfig so that the Motif process can
     * have query access to libpf objects
     */
    Motif = (MotifData_t*) pfCalloc(1, sizeof(MotifData_t), pfGetSharedArena());
    if (ForkMotif)
    {
	if ((fpid = fork()) < 0)
	    pfNotify(PFNFY_FATAL, PFNFY_SYSERR, "Fork of motif process failed.");
	else if (fpid)
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Motif running in forked process %d",
		fpid);
	if (!fpid)
	    DoMotif(argc, argv, ForkMotif);
    }
    	
    
    /* Create an earth/sky model that draws sky/ground/horizon */
    eSky = pfNewESky();
    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_FAST);
    pfESkyColor(eSky, PFES_CLEAR, .3f, .3f, .7f, 1.0f);
    pfESkyAttr(eSky, PFES_GRND_HT, -10.0f);
    pfChanESky(Shared->chan, eSky);

    pfChanTravMode(Shared->chan, PFTRAV_CULL, PFCULL_VIEW|PFCULL_GSET);
    
    /* vertical FOV is matched to window aspect ratio */
    pfChanFOV(Shared->chan, 45.0f, -1.0f);
        
   /*------------------------------------
    * Do Main Loop
    *------------------------------------
    */
    
    /* if Motif is forked, we control the app main loop */
    if (!ForkMotif)
	DoMotif(argc, argv, ForkMotif);
    else if (ForkMotif)
    {
	/* wait for Motif process to initialize the pfPipeWindow parameters */
	while (!Motif->xWin)
	{
	    sginap(1);
	}
	InitPipe();
	
	/* main simulation loop */
	while (!Shared->exitFlag)
	{
	    SimFrame();
	}
    }
    /* terminate cull and draw processes (if they exist) */
    pfExit();
    
    /* exit to operating system */
    exit(0);
}

/*
 *	NewScene() -- Load in new databases -
 *	is executed in the application process.
 */

static pfScene *
NewScene(char **filename, int num)
{
    pfScene	    *scene=NULL;
    pfNode	    *root;
    int		    found=0;
    pfBox           bbox;
    int		    i;
    char *c;
    
    for (i = 0; i < num; i++)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT,"NewScene - loading file '%s'\n",filename[i]);
	if (pfFindFile(filename[i], Shared->loadFilePath, R_OK))
	{
	    if ((root = pfdLoadFile(Shared->loadFilePath)) != NULL)
	    {
		if (!scene)
		{
		    scene = pfNewScene();
		}
		Shared->sceneDCS = pfNewDCS();
		pfAddChild(scene, Shared->sceneDCS);
		pfAddChild(Shared->sceneDCS, root);
		found++;
	    }
	    strcpy(Shared->loadFileName, filename[i]);
	    c = strrchr(Shared->loadFilePath, '/');
	    if (c)
		*c = '\0';
	}
    }
    if (found) /* set appropriate viewing position and near/far clipplanes */
    {
	
	/* optimize sharing of geostate structures and components */
	pfdMakeShared((pfNode *)scene);
	pfdMakeSharedScene(scene);

	/* optimize pfLayer nodes via "DISPLACE POLYGON" */
	pfdCombineLayers((pfNode *)scene);

    }  
    if (Shared->xformer)
    {
	pfiXformerNode(Shared->xformer, Shared->scene);
	pfiXformerAutoPosition(Shared->xformer, Shared->chan, Shared->sceneDCS);
    }

    return scene;
}

static void
DBaseFunc(void)
{
    pfDBase();
}

/******************************************************************************
 *			    Motif Routines
 ******************************************************************************
 */

static XtAppContext appcontext;

void 
DoMotif(int argc, char **argv, int forked)
{
    static String app_defaults[] = {
	"*useSchemes: all",
	NULL,
    };
    Widget toplevel;
    Widget mainw, menubar, menupane, btn, cascade, frame;
    Widget gfxwin, glwidget;
    Arg    args[8];
    XVisualInfo *vi;
    Display *dsp;

#ifndef __linux__
    prctl(PR_TERMCHILD);        /* Exit when parent does */
    sigset(SIGHUP, SIG_DFL);    /* Exit when sent SIGHUP by TERMCHILD */
#endif

    toplevel = XtVaAppInitialize(&appcontext, ProgName, NULL, 0,
				 &argc, argv, app_defaults, NULL, NULL);
    dsp = XtDisplay(toplevel);
    /* Create main window and motif UI widgets */
    mainw = XmCreateMainWindow(toplevel, "mainw", NULL, 0);
    XtManageChild(mainw);
    /* Menu Bar */
    menubar = XmCreateMenuBar(mainw, "menubar", NULL, 0);
    XtManageChild(menubar);
    
    /* File Menu -> Open File Button */
    menupane = XmCreatePulldownMenu(menubar, "menupane", NULL, 0);
    btn = XmCreatePushButton(menupane, "Open", NULL, 0);
    XtAddCallback(btn, XmNactivateCallback, openCB, NULL);
    XtManageChild(btn);
    /* File Menu -> Quit Button */
    btn = XmCreatePushButton(menupane, "Quit", NULL, 0);
    XtAddCallback(btn, XmNactivateCallback, quitCB, NULL);
    XtManageChild(btn);
    /* Menu Bar -> File Menu  */
    XtSetArg(args[0], XmNsubMenuId, menupane);
    cascade = XmCreateCascadeButton(menubar, "File", args, 1);
    XtManageChild(cascade);
    
    /* create framed drawing area for OpenGL rendering */
    frame = XmCreateFrame(mainw, "frame", NULL, 0);
    XtManageChild(frame);
    /* Create the Performer rendering widget with default visual */
     Motif->vi = vi = pfChooseFBConfigData((void**)&(Motif->config), 
			dsp, -1, NULL, pfGetSharedArena());
    glwidget = XtVaCreateManagedWidget("glwidget",
		    glwMDrawingAreaWidgetClass, frame, GLwNvisualInfo, vi, 
		    XmNwidth, WinSizeX, XmNheight, WinSizeY,
		    NULL);    
    /**/
    XtAddEventHandler(mainw, StructureNotifyMask, 
		    FALSE, (XtEventHandler)GetTopInput, 0);
    
    XtAddEventHandler(glwidget, 
		    (StructureNotifyMask | PointerMotionMask | FocusChangeMask |
		    ButtonPressMask | ButtonReleaseMask | KeyPressMask), 
		    FALSE, (XtEventHandler)GetInput, 0);
    
    XtAddCallback(glwidget,
		    GLwNginitCallback,
		  (XtCallbackProc)InitMotifWidget, 0);
    XmMainWindowSetAreas(mainw, menubar, NULL, NULL, NULL, frame);
    XtRealizeWidget(toplevel);
    XSync(dsp, FALSE);
    
    /* set PipeWindow X parameters from the widget */
    Motif->gXWin =  XtWindow(glwidget);
    if (!Motif->gXWin)
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "Motif - No GFX Window!!");
    /* this will release the waiting application and input processes */
pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Motif init done - setting XWIN 0x%x", 
	     XtWindow(frame));
    Motif->xWin =  XtWindow(frame); 
    if (!forked)
    {
	Motif->work_id = XtAppAddWorkProc(appcontext, (XtWorkProc)SimFrame, 0);
	InitPipe();
    }
    XtAppMainLoop(appcontext);
}

static void
InitMotifWidget(Widget w, XtPointer data, 
	    GLwDrawingAreaCallbackStruct * cb)
{
    Screen *scr = XtScreen(w);
    the_X_y_max = DisplayHeight(XtDisplay(w), XScreenNumberOfScreen(scr));
}

static void
initXformer(void)
{
    Shared->xformer = pfiNewTDFXformer(pfGetSharedArena());

    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "xformer = 0x%x\n",Shared->xformer);
    
    /* Xformer will look at this mouse structure for input info */
    pfiXformerAutoInput(Shared->xformer, Shared->chan,
		&Shared->mouse, NULL);
    pfiSelectXformerModel(Shared->xformer, Shared->xformerModel);
    pfiXformerNode(Shared->xformer, Shared->scene);
    pfiXformerAutoPosition(Shared->xformer, Shared->chan, Shared->sceneDCS);

    resetPosition();
}

void resetPosition(void)
{
    pfMatrix    mat;
    pfSphere    bsphere;

    pfiStopXformer(Shared->xformer);
    pfiResetXformerPosition(Shared->xformer);
}

static void
GetTopInput(Widget w, void *call, XEvent * event)
{
    static pid_t draw_pid = 0;
    switch (event->type) 
    {
    case MapNotify:
	if (Motif->work_id)
	    Motif->work_id = XtAppAddWorkProc(appcontext, (XtWorkProc)SimFrame, 0);
	/* Resume any stopped draw process (see below) */
	if (draw_pid > 0)
	    kill(draw_pid, SIGCONT);
	break;
    case UnmapNotify:
	/* If there is a separate draw process, it must be temporarily
	* stopped, otherwise it busy-waits.
	*/
	if (Motif->work_id)
	    XtRemoveWorkProc(Motif->work_id);
	if (draw_pid == 0)
	{
	    int mode = pfGetMultiprocess();
	    if (mode & PFMP_FORK_DRAW)
		draw_pid = pfGetPID(0, PFPROC_DRAW);
	    else
		draw_pid = -1;
	}
	if (draw_pid > 0)
	    kill(draw_pid, SIGSTOP);
	break;
    }
}

static void
GetInput(Widget w, void *call, XEvent * event)
{
    static int x=0, y=0;
    
    Shared->mouse.click = 0;
    Shared->mouse.release = 0;
    switch (event->type) 
    {
    case ConfigureNotify:
	break;
    case MotionNotify: 
	if (Shared->mouse.flags)
	{
	    XMotionEvent *motion_event = (XMotionEvent *) event;
	    x =  motion_event->x;
	    y = Shared->mouse.winSizeY - motion_event->y;
	}
	break;
    case ButtonPress: 
    {
	XButtonEvent *button_event = (XButtonEvent *) event;
	x = button_event->x;
	y = Shared->mouse.winSizeY - button_event->y;
	Shared->mouse.inWin = 1;
	switch (button_event->button) {
	    case Button1:
		pfuMouseButtonClick(&Shared->mouse, PFUDEV_MOUSE_LEFT_DOWN, 
		    x, y, pfuMapXTime(button_event->time));
		break;
	    case Button2:
		pfuMouseButtonClick(&Shared->mouse, PFUDEV_MOUSE_MIDDLE_DOWN, 
		    x, y, pfuMapXTime(button_event->time));
		break;
	    case Button3:
		pfuMouseButtonClick(&Shared->mouse, PFUDEV_MOUSE_RIGHT_DOWN, 
		    x, y, pfuMapXTime(button_event->time));
		break;
	}
	break;
    }
    case ButtonRelease:
    {
	XButtonEvent *button_event = (XButtonEvent *) event;
	x = button_event->x;
	y = Shared->mouse.winSizeY - button_event->y;
	switch (button_event->button) {
	    case Button1:
		pfuMouseButtonRelease(&Shared->mouse, PFUDEV_MOUSE_LEFT_DOWN, 
		    x, y, pfuMapXTime(button_event->time));
		break;
	    case Button2:
		pfuMouseButtonRelease(&Shared->mouse, PFUDEV_MOUSE_MIDDLE_DOWN, 
		    x, y, pfuMapXTime(button_event->time));
		break;
	    case Button3:
		pfuMouseButtonRelease(&Shared->mouse, PFUDEV_MOUSE_RIGHT_DOWN, 
		    x, y, pfuMapXTime(button_event->time));
		break;
	}
	Shared->mouse.flags &= ~(Shared->mouse.release);
	break;
    }
    case MapNotify:
	break;
    case UnmapNotify:
	break;
    case FocusIn:
	Shared->mouse.inWin = 1;
	break;
    case FocusOut:
	Shared->mouse.inWin = 0;
	break;
    case KeyPress:
    {
	char buf[100];
	int rv;
	KeySym ks;
	rv = XLookupString(&event->xkey, buf, sizeof(buf), &ks, 0);
	switch(ks) {
	case XK_Escape: 
	    Shared->exitFlag = 1;
	    exit(0);
	    break;
	case XK_space:
	    Shared->reset = 1;
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Reset");
	    break;
	case XK_g:
	case XK_F1:
	    Shared->drawStats = !Shared->drawStats;
	    break;
	case XK_f:
	    Shared->xformerModel = PFITDF_FLY;
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Fly");
	    break;
	case XK_d:
	    Shared->xformerModel = PFITDF_DRIVE;
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Drive");
	    break;
	case XK_t:
	    Shared->xformerModel = PFITDF_TRACKBALL;
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Trackball");
	    break;
	}
	break;
    }
    default:
	break;
    }/* switch */

    /* update cursor virtual position when cursor inside window */
    Shared->mouse.xpos = x;
    Shared->mouse.ypos = y;
    
    pfuMapMouseToChan(&Shared->mouse,  Shared->chan);
}


static void 
loadFileCB(Widget w, XtPointer data, XtPointer callData)
{
	XmFileSelectionBoxCallbackStruct *cbs =
	    (XmFileSelectionBoxCallbackStruct*) callData;
	if (cbs) 
	{
	    char *loadfile;
	    if (!XmStringGetLtoR(cbs->value,XmFONTLIST_DEFAULT_TAG,&loadfile))
		    return;
	    strcpy(Shared->loadFileName, loadfile);
	    XtFree(loadfile);
	    Shared->newScene = 1;
	}
}

static void 
openCB(Widget w, XtPointer data, XtPointer callData)
{
    static Widget fsDialog = NULL;
    static XmString xStr;    
    
    if (!fsDialog)
    {
	fsDialog = XmCreateFileSelectionDialog(w,"w",NULL,0);
	XtAddCallback(fsDialog,XmNokCallback, loadFileCB, NULL);
	XtAddCallback(fsDialog,XmNcancelCallback, (XtCallbackProc) XtUnmanageChild, NULL);	
    }
    xStr = XmStringCreateLocalized(Shared->loadFilePath);
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Set Directory to %s", Shared->loadFilePath);
    /* load up file selector with director of previously choosen file  */
    XtVaSetValues(fsDialog, XmNdirectory, xStr);
    XmStringFree(xStr);
    xStr = XmStringCreateLocalized(Shared->loadFileName);
    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Set Filename to %s", Shared->loadFileName);
    XtVaSetValues(fsDialog, XmNdirSpec, xStr);
    XmStringFree(xStr);
    XtManageChild(fsDialog);
}


static void 
quitCB(Widget w, XtPointer data, XtPointer callData)
{
    Shared->exitFlag = 1;
    exit(0);
}


/******************************************************************************
 *			    Application Routines
 ******************************************************************************
 */

static void
InitPipe(void)
{
    int dsp_xsize;
    pfPipe *p;
    
    /* set pfPipeWindow parameters from the Motif widget */
    pfPWinWSDrawable(Shared->pw, NULL, Motif->gXWin);
    pfPWinWSWindow(Shared->pw, NULL, Motif->xWin);
}

static int
SimFrame(void)
{
    /* wait until next frame boundary */
    pfSync();
    pfFrame();
    
    if (Shared->newScene) 
    {
	pfScene *newScene;
	char *fileName = Shared->loadFileName;
	if (newScene = NewScene(&(fileName), 1))
	{
	    pfDelete(Shared->scene);
	    Shared->scene = newScene;
	    pfChanScene(Shared->chan, Shared->scene);
	}
	Shared->newScene = 0;
    }
      
    /* read window origin and size (it may have changed) */
    pfGetPWinSize(pfGetChanPWin(Shared->chan), 
		    &Shared->mouse.winSizeX, &Shared->mouse.winSizeY);
    
    /* Set view parameters. */
    UpdateView();
    /* initiate traversal using current state */
     
    return FALSE; /* Motif: don't remove this callback */
}

/* 
 *	UpdateView() updates the eyepoint based on the information
 *	placed in shared memory by GetInput().
 */
static void    
UpdateView(void)
{
    pfMatrix mat;
    
    pfiSelectXformerModel(Shared->xformer, Shared->xformerModel);

    if (Shared->reset)
    {
	resetPosition();
	Shared->reset = 0;
	Shared->mouse.flags = 0x0;
	Shared->mouse.xpos = Shared->mouse.ypos = 0;
    }
    else
    {
	pfiUpdateXformer(Shared->xformer);
    }
}


/******************************************************************************
 *			    Draw Process Routines
 ******************************************************************************
 */

/*
 *	OpenPipeWin() -- create a win: setup the GL and OpenGL Performer.
 *	This procedure is executed in the draw process 
 *	(when there is a separate draw process).
 */

Display *Dsp=NULL;

static void
OpenPipeWin(pfPipeWindow *pw)
{
    pfPipe *p;
    Window w;

    pfOpenPWin(pw);
    Dsp = pfGetCurWSConnection();
    
    /* create a light source in the "south-west" (QIII) */
    Sun = pfNewLight(NULL);
    pfLightPos(Sun, -0.3f, -0.3f, 1.0f, 0.0f);
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
    /* 
     * pfDrawGeoSet or other display listable Performer routines
     * could be invoked before or after pfCull()
     */
    
    (channel, data);
    
    pfCull();
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
}
