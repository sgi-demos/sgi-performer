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
 * motif.c
 * 
 * OpenGL Performer example using X and motif.

 * $Revision: 1.57 $ 
 * $Date: 2002/12/08 00:12:14 $
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
#ifndef __linux__
#include <X11/GLw/GLwMDrawA.h>	    /* header for OpenGL Motif widget */
#else
#include <GL/GLwMDrawA.h>	    /* header for OpenGL Motif widget */
#endif

#define HISPEED		0.1f
#define LOSPEED		0.001f
#define	FARCLIP 	10000.0f

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
    pfNode	    *model;
    int		    exitFlag;
    int		    inWindow, reset;
    float	    mouseX;
    float	    mouseY;
    int		    winSizeX, winSizeY;
    int		    mouseButtons;
    pfCoord	    view, viewOrig;
    float	    sceneSize;
    int		    drawStats;
    int		    loadNewModel;
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
static void LoadModel(char *filename);
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

/*
 *	Usage() -- print usage advice and exit. This procedure
 *	is executed in the application process.
 */

static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE,
	"Usage: %s [-f][-p procSplit][-w][-W ox,oy,sx,sy] [file.ext]\n", ProgName);
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
    while ((opt = getopt(argc, argv, "Fm:p:wW:?")) != -1)
    {
	switch (opt)
	{
	case 'F':
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
    int		i;
    int		arg;
    pfNode	*root;
    pfPipe      *p;
    pfEarthSky  *eSky;
    pfLightSource *sun;
    pfVec3	where;
    pid_t	fpid = 0;
    
    /* process command line arguments */
    arg = docmdline(argc, argv);
    
    /* Performer Initialization */
    pfInit();
    
    /*
     * Allocate Shared before fork()'ing motif so that all 
     * processes see the Shared pointer.
     */
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());

    Shared->inWindow = 0;
    Shared->reset = 1;
    Shared->exitFlag = 0;
    Shared->drawStats = 1;
    Shared->loadNewModel = 0;
    pfSetVec3(Shared->view.xyz, 0.0f, -100.0f, 50.0f);
    pfSetVec3(Shared->view.hpr, 0.0f, 0.0f, 0.0f);
    
   /*
    * Set global application configuration parameters
    */
    
    /* configure multi-process selection */
    pfMultiprocess(ProcSplit);

    /* initialize the file-loader DSO before forking in pfConfig() */
    if (arg < argc)
	pfdInitConverter(argv[arg]);
    
    /* initiate multi-processing */
    pfConfig();	
    
    /* 
     * Set off motif AFTER pfConfig so that we will have query 
     * access to Performer objects in shared memory
     */
    Motif = (MotifData_t*) pfCalloc(1, sizeof(MotifData_t), pfGetSharedArena());
    if (ForkMotif)
    {
	if ((fpid = fork()) < 0)
	    pfNotify(PFNFY_FATAL, PFNFY_SYSERR, 
		"Fork of motif process failed.");
	else 
	if (fpid)
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		"Motif running in forked process %d", fpid);

	if (!fpid)
	    DoMotif(argc, argv, ForkMotif);
    }
    	   
    pfFrameRate(20.0f);
    pfPhase(PFPHASE_FREE_RUN);
   
   /*
    * Load the initial database
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
    pfNotify(PFNFY_INFO, PFNFY_PRINT,
	"FilePath: %s", pfGetFilePath());
    
    /* initialize the scene graph */
    Shared->scene = pfNewScene();

    /* add infinite light source to scene */
    sun = pfNewLSource();
    pfSetVec3(where, -1.0f, -1.0f, 2.0f);
    pfNormalizeVec3(where);
    pfLSourcePos(sun, where[0], where[1], where[2], 0.0f);
    pfLSourceColor(sun, PFLT_DIFFUSE, 1.0f, 1.0f, 1.0f);
    pfAddChild(Shared->scene, sun);

    /* add place holder "model" to scene */
    Shared->model = (pfNode *)pfNewGroup();
    pfAddChild(Shared->scene, Shared->model);

    /* load file named on command line */
    if (arg < argc)
	LoadModel(argv[arg]);
    
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
 
   /*
    * Create Pipes, Windows, and Channels
    */
    
    /* Create Performer Window */
    p = pfGetPipe(0);
    Shared->pw = pfNewPWin(p);
    pfPWinType(Shared->pw, PFPWIN_TYPE_X);
    pfPWinName(Shared->pw, "OpenGL Performer");

    /* set the window initialization callback */
    pfPWinConfigFunc(Shared->pw, OpenPipeWin);

    /* set the request for the configuration of the window */
    pfConfigPWin(Shared->pw);
    
    Shared->chan = pfNewChan(p);
    pfAddChan(Shared->pw, Shared->chan);
    pfChanTravMode(Shared->chan, PFTRAV_CULL, PFCULL_ALL);
    pfChanTravFunc(Shared->chan, PFTRAV_CULL, CullChannel);
    pfChanTravFunc(Shared->chan, PFTRAV_DRAW, DrawChannel);
    pfChanScene(Shared->chan, Shared->scene);
    pfChanNearFar(Shared->chan, 0.1f, FARCLIP);
    
    /* Create an earth/sky model that draws sky/ground/horizon */
    eSky = pfNewESky();
    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    pfESkyAttr(eSky, PFES_GRND_HT, -100.0f);
    pfChanESky(Shared->chan, eSky);

    /* horizontal FOV is matched to window aspect ratio */
    pfChanFOV(Shared->chan, 0.0f, 45.0f);
    
    /* initialize viewpoint */
    pfChanView(Shared->chan, Shared->view.xyz, Shared->view.hpr);
    PFCOPY_VEC3(Shared->viewOrig.xyz, Shared->view.xyz);
    PFCOPY_VEC3(Shared->viewOrig.hpr, Shared->view.hpr);
    
   /*
    * Main simulation loop
    */
    
    /* if Motif is forked, we control the app main loop */
    if (ForkMotif)
    {
	/* wait for Motif process to initialize the pfPipeWindow parameters */
	while (!Motif->xWin)
	    sginap(1);

	InitPipe();
	
	/* main simulation loop */
	while (!Shared->exitFlag)
	    SimFrame();
    }
    else
	DoMotif(argc, argv, ForkMotif);

    /* terminate cull and draw processes (if they exist) */
    pfExit();
    
    /* exit to operating system */
    exit(0);
}

/*
 *	LoadModel() -- Load in new databases -
 *	is executed in the application process.
 */

static void
LoadModel(char *filename)
{
    int		    found=0;
    pfBox           bbox;
    
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
	"LoadModel - loading file '%s'", filename);

    if (pfFindFile(filename, Shared->loadFilePath, R_OK))
    {
	pfNode	*node;
	char	*c;

	/* load named file */
	if ((node = pfdLoadFile(Shared->loadFilePath)) != NULL)
	{
	    /* install new model into scene */
	    pfReplaceChild(Shared->scene, Shared->model, node);
	    pfDelete(Shared->model);
	    Shared->model = node;
	    found = 1;

	    /* remove unsed objects from pfdBuilder share cache */
	    pfdCleanBldrShare();

	    /* optimize model (optional) */
#define OPTIMIZE_MODEL
#ifdef  OPTIMIZE_MODEL
	    /* optimize sharing of geostate structures within scene */
	    pfdMakeShared(Shared->model);

	    /* factor common geostate elements to scene geostate */
	    pfdMakeSharedScene(Shared->scene);

	    /* optimize pfLayer nodes via "DISPLACE POLYGON" */
	    pfdCombineLayers(Shared->model);

	    /* optimize pfBillboard nodes via */
	    pfdCombineBillboards(Shared->model, 32);
#endif

	    /* update file path */
	    strcpy(Shared->loadFileName, filename);
	    c = strrchr(Shared->loadFilePath, '/');
	    if (c)
		*c = '\0';
	}
    }

    /* set appropriate viewing position and near/far clipplanes */
    if (found) 
    {
	float sceneSize;
	
	/* Set initial view to be "in front" of model */
	/* determine extent of scene's geometry */
	pfuTravCalcBBox(Shared->model, &bbox);
    
	/* place view point at center of bbox */
	pfAddVec3(Shared->view.xyz, bbox.min, bbox.max);
	pfScaleVec3(Shared->view.xyz, 0.5f, Shared->view.xyz);
	
	/* find max dimension */
	sceneSize =                    bbox.max[PF_X] - bbox.min[PF_X];
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	sceneSize = PF_MIN2(sceneSize, 0.5f * FARCLIP);
	Shared->sceneSize = sceneSize;
	
	/* offset so all is visible */
	Shared->view.xyz[PF_Y] -= 1.50f*sceneSize;
	Shared->view.xyz[PF_Z] += 0.25f*sceneSize;	
    }
#ifdef  RESET_VIEWPOINT_ON_FAILED_LOAD
    else
    {

	pfSetVec3(Shared->view.xyz, 0.0f, -100.0f, 50.0f);
	PFSET_VEC3(bbox.min, -5000.0, -5000.0, -5000.0);
	PFSET_VEC3(bbox.max,  5000.0,  5000.0,  5000.0);
	Shared->sceneSize = 10000.0f;
    }
#endif
}

/******************************************************************************
 *			    Motif Routines
 ******************************************************************************
 */

static XtAppContext appcontext;

void 
DoMotif(int argc, char **argv, int forked)
{
    static String app_defaults[] = 
    {
	"*sgiMode: true",
	"*useSchemes: all",
	NULL,
    };
    Widget toplevel;
    Widget mainw, menubar, menupane, btn, cascade, frame;
    Widget glwidget;
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
	XmNtopOffset, 8,
	XmNwidth, WinSizeX, XmNheight, WinSizeY,
	NULL);    
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
    /* this will release the waiting application process */
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
    
    switch (event->type) 
    {
    case ConfigureNotify:
	break;

    case MotionNotify: 
	if (Shared->mouseButtons)
	{
	    XMotionEvent *motion_event = (XMotionEvent *) event;
	    x =  motion_event->x;
	    y = Shared->winSizeY - motion_event->y;
	}
	break;

    case ButtonPress: 
    {
	XButtonEvent *button_event = (XButtonEvent *) event;
	x = button_event->x;
	y = Shared->winSizeY - button_event->y;
	Shared->inWindow = 1;
	switch (button_event->button) 
	{
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
	break;
    }	

    case ButtonRelease:
    {
	XButtonEvent *button_event = (XButtonEvent *) event;
	switch (button_event->button) 
	{
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
	break;
    }

    case MapNotify:
	break;

    case UnmapNotify:
	break;

    case FocusIn:
	Shared->inWindow = 1;
	break;

    case FocusOut:
	Shared->inWindow = 0;
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
	}
	break;
    }

    default:
	break;
    }/* switch */

    /* update cursor virtual position when cursor inside window */
    Shared->mouseX = x;
    Shared->mouseY = y;
}

static void 
loadFileCB(Widget w, XtPointer data, XtPointer callData)
{
    XmFileSelectionBoxCallbackStruct *cbs =
	(XmFileSelectionBoxCallbackStruct*) callData;
    if (cbs) 
    {
	char *loadfile;
	if (!XmStringGetLtoR(cbs->value, XmFONTLIST_DEFAULT_TAG, &loadfile))
	    return;
	strcpy(Shared->loadFileName, loadfile);
	XtFree(loadfile);
	Shared->loadNewModel = 1;
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
	XtAddCallback(fsDialog,XmNcancelCallback, 
	    (XtCallbackProc) XtUnmanageChild, NULL);	
    }
    xStr = XmStringCreateLocalized(Shared->loadFilePath);

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"Set Directory to %s", Shared->loadFilePath);

    /* load file selector with directory of previously choosen file */
    XtVaSetValues(fsDialog, XmNdirectory, xStr, NULL);
    XmStringFree(xStr);
    xStr = XmStringCreateLocalized(Shared->loadFileName);

    pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	"Set Filename to %s", Shared->loadFileName);

    XtVaSetValues(fsDialog, XmNdirSpec, xStr, NULL);
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

    /* Set view parameters. */
    UpdateView();
    pfChanView(Shared->chan, Shared->view.xyz, Shared->view.hpr);

    /* initiate traversal using current state */
    pfFrame();
    
    if (Shared->loadNewModel) 
    {
	/* load named database in place of current one */
	LoadModel(Shared->loadFileName);
	Shared->loadNewModel = 0;
    }
	    
    /* read window origin and size (it may have changed) */
    pfGetPWinSize(pfGetChanPWin(Shared->chan), 
	&Shared->winSizeX, &Shared->winSizeY);
		    
    return FALSE; /* Motif: don't remove this callback */
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
    float mx, my;
    float cp;

    if (!Shared->inWindow || Shared->reset)
    {
	speed = 0;
	if (Shared->reset)
	{
	    PFCOPY_VEC3(Shared->view.xyz, Shared->viewOrig.xyz);
	    PFCOPY_VEC3(Shared->view.hpr, Shared->viewOrig.hpr);
	    Shared->reset = 0;
	    Shared->mouseButtons = 0x0;
	    Shared->mouseX = Shared->mouseY = 0;
	}
	return;
    }
  
    switch (Shared->mouseButtons)
    {
    case Button1Mask: /* LEFTMOUSE: faster forward or slower backward*/
	if (speed > 0.0f)
	    speed *= 1.2f;
	else
	    speed /= 1.2f;
	
	if (PF_ABSLT(speed, LOSPEED * Shared->sceneSize))
	    speed = LOSPEED * Shared->sceneSize;
	else if (speed >=  HISPEED * Shared->sceneSize)
	    speed = HISPEED * Shared->sceneSize;
	break;

    case Button2Mask:	/* MIDDLEMOUSE: stop moving */
	speed = 0.0f;
	break;

    case Button3Mask: /* RIGHTMOUSE: faster backward or slower foreward*/
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
    
    if (Shared->mouseButtons)
    {
	mx = 2.0f * (Shared->mouseX / (float)Shared->winSizeX) - 1.0f;
	my = 2.0f * (Shared->mouseY / (float)Shared->winSizeY) - 1.0f;
	    
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
    /* erase framebuffer and draw Earth-Sky model */
    pfClearChan(channel);
    
    /* invoke Performer draw-processing for this frame */
    pfDraw();

    /* draw Performer throughput statistics */
    if (Shared->drawStats)
	pfDrawChanStats(channel);
}
