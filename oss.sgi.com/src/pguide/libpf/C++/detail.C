//
// Copyright 1995, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// This source code ("Source Code") was originally derived from a
// code base owned by Silicon Graphics, Inc. ("SGI")
// 
// LICENSE: SGI grants the user ("Licensee") permission to reproduce,
// distribute, and create derivative works from this Source Code,
// provided that: (1) the user reproduces this entire notice within
// both source and binary format redistributions and any accompanying
// materials such as documentation in printed or electronic format;
// (2) the Source Code is not to be used, or ported or modified for
// use, except in conjunction with OpenGL Performer; and (3) the
// names of Silicon Graphics, Inc.  and SGI may not be used in any
// advertising or publicity relating to the Source Code without the
// prior written permission of SGI.  No further license or permission
// may be inferred or deemed or construed to exist with regard to the
// Source Code or the code base of which it forms a part. All rights
// not expressly granted are reserved.
// 
// This Source Code is provided to Licensee AS IS, without any
// warranty of any kind, either express, implied, or statutory,
// including, but not limited to, any warranty that the Source Code
// will conform to specifications, any implied warranties of
// merchantability, fitness for a particular purpose, and freedom
// from infringement, and any warranty that the documentation will
// conform to the program, or any warranty that the Source Code will
// be error free.
// 
// IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
// LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
// ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
// SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
// OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
// PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
// OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
// USE, THE SOURCE CODE.
// 
// Contact information:  Silicon Graphics, Inc., 
// 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
// or:  http://www.sgi.com
//
//
// file: detailC.C
// --------------
//
// OpenGL Performer tool allowing one to load a specific base texture
// and detail texture.  The textures are applied to a cube and displayed.
// The gui interface contains 9 horizontal sliders which corresponds
// to the control points for pfTexSpline.  The use can manipulate
// the control points and view results.
//
// A sample texture and matching detail texture can be found in the 
// data directory: road.rgb and road_detail.rgb
//
// dtail_tex <base texture> <detail texture>
//
// Run-time controls:
// ------------------
//       ESC-key: exits
//        F1-key: display profile stats
//     SPACE-key: stop
//	   r-key: re-position
//    Left-mouse: moves forward
//  Middle-mouse: stop
//   Right-mouse: moves backward
//

/******************************************************************************
*				Includes
******************************************************************************
*/

// general includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h> // for cmdline handler

// proecess control includes
#include <sys/types.h>
#include <sys/prctl.h>
#ifndef __linux__
#include <sys/sysmp.h>
#include <sys/schedctl.h>
#endif

// X Window includes
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

// GL includes

// Performer includes
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfGeoSet.h>


#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <Performer/pfui/pfiXformer.h>


/******************************************************************************
*				Defines and Typedefs
******************************************************************************
*/

// Default detail stuff
#define DEF_LOD1	0.0f
#define DEF_SCALE1	0.0f
#define DEF_LOD2	3.0f
#define DEF_SCALE2	1.5f
#define DEF_LOD3	4.0f
#define DEF_SCALE3	2.0f
#define DEF_LOD4	6.5f
#define DEF_SCALE4	3.25f

// Panel Widget Control Tokens
enum PanelControlToken {
    CTL_RESET_ALL=1,
    CTL_REPOSITION,
    CTL_QUIT,
    CTL_MODEL,
    CTL_DIAGS,
    CTL_LOD1_SLIDER,
    CTL_LOD2_SLIDER,
    CTL_LOD3_SLIDER,
    CTL_LOD4_SLIDER,
    CTL_SCALE1_SLID,
    CTL_SCALE2_SLID,
    CTL_SCALE3_SLID,
    CTL_SCALE4_SLID,
    CTL_CLAMP
    };

// SharedData:
// structure that resides in shared memory so that the
// application, cull, and draw processes can access it.
//
typedef struct SharedData
{
    // main channel stuff
    pfChannel		*chan;
    pfVec3		startXYZ;
    float		sceneSize;
    int			drawStats;
    int			exitFlag;
    pfLightSource	*sun;		// sun light source
    // Input handling structures
    int			input;		// PFUINPUT_GL or PFUINPUT_X
    pfuMouse		mouse;		// mouse structure
    pfuEventStream	events;		// event structure
    // flight model and collision detection sructures
    pfScene		*scene;
    pfDCS		*sceneDCS;	// DCS for trackball interface
    pfCoord		viewCoord;	// current view position, direction
    pfCoord		initView;	// initial view position, direction
    float		near;		// near clipping plane
    float		far;		// far clipping plane
    pfiXformer		*xformer;	// interactive movement model
    int			xformerModel;	// Fly, Drive, or Trackball
    int			collideMode;	// collision & terrain following flag
    // flags for GUI
    int			updateChannels;
    int			gui, redraw, guiDirty;
    pfuPanel		*guiPanel;
    pfuWidget		*wResetAll, *wKillGUI, *wPosXYZ, *wStats, *wFModel;
}SharedData;

/******************************************************************************
*				Static Declarations
******************************************************************************
*/

// shared memory handle
static SharedData *Shared=0;

// texture stuff
static pfTexture	*tex;
static pfTexture	*dtex;

static float LOD1 = DEF_LOD1;
static float SCALE1 = DEF_SCALE1;
static float LOD2 = DEF_LOD2;
static float SCALE2 = DEF_SCALE2;
static float LOD3 = DEF_LOD3;
static float SCALE3 = DEF_SCALE3;
static float LOD4 = DEF_LOD4;
static float SCALE4 = DEF_SCALE4;
static float TEX_CLAMP = -1;

// update pfTexSpline boolean
int Detail_mod = FALSE;

static pfVec2	ctl_pts[4];

// X control variables
#ifndef __linux__
static int XInput = PFUINPUT_X;
#else
static int XInput = PFUINPUT_NOFORK_X;
#endif
static int WinXOrigin = 10, WinYOrigin = 10;
static int WinXSize = 500, WinYSize = 500;
/*  static int WinXSize = 1280, WinYSize = 1024;  */

// process control variables
static int ProcSplit = PFMP_APPCULLDRAW;

// global name strings
static char TexName[PF_MAXSTRING];
static char DTexName[PF_MAXSTRING];
static char ProgName[PF_MAXSTRING];

static int HaveDetail = 0;

static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static pfuPanel *InitPanel(void);
static void ProcessInputIO(void);
static void PanelControl(pfuWidget *w);
static void resetPosition(void);
static void KillGUI(pfuWidget *w);
static void Usage(void);
static void InitXformer(void);
static void IsectFunc(void *data);
static void UpdateView(pfChannel *chan, pfScene *scene);
static void updateGUIViewMsg(pfCoord *viewpt);
static void xformerModel(int model, int collideMode);

// make the textured cube
static pfGroup	*MakeTexCube ();

/******************************************************************************
*			    Main and Commandline Processing
******************************************************************************
*/

//
//      Usage() -- print usage advice and exit. This procedure
//	is executed in the application process.
//

char OptionStr[] = "m:p:x?";

static void
Usage (void)
{
    fprintf(stderr, "Usage: %s <base texture> <detail texture> \n",
	    ProgName);
    exit(1);
}

//
//	docmdline() -- use getopt to get command-line arguments,
//	executed at the start of the application process.
//

static int
docmdline(int argc, char *argv[])
{
    int	    opt;
    
    strcpy(ProgName, argv[0]);
    
    // process command-line arguments
    while ((opt = getopt(argc, argv, OptionStr)) != -1)
    {
	switch (opt)
	{
	case 'm':
	case 'p':
	    ProcSplit = atoi(optarg);
	    break;
	case 'x':
#ifndef __linux__
	    XInput = PFUINPUT_X;
#else
	    XInput = PFUINPUT_NOFORK_X;
#endif
	    break;
	case '?':
	    Usage();
	    break;
	}
    }
    return optind;
}

//
//	main() -- program entry point. this procedure
//	is executed in the application process.
//

int
main (int argc, char *argv[])
{
    int		    arg, detail;
    float	    far = 10000.0f;
    
    
    pfNotifyLevel(PFNFY_NOTICE);
    pfInit();
    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures");

    pfQueryFeature(PFQFTR_TEXTURE_DETAIL, &detail);
    if (!detail)
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"Detail texture not supported on this platform.");
    
    arg = docmdline(argc, argv);
    
    if ((argc - arg) < 2)
	Usage();
    
    pfMultiprocess(ProcSplit);
    
    // grab base texture name
    strcpy(TexName, argv[arg]);
    
    // grab detail texture name
    strcpy(DTexName, argv[arg+1]);
    
    // Initialize ctl_pts
    ctl_pts[0].set(-LOD1, SCALE1);
    ctl_pts[1].set(-LOD2, SCALE2);
    ctl_pts[2].set(-LOD3, SCALE3);
    ctl_pts[3].set(-LOD4, SCALE4);
    
    
    // allocate shared before fork()'ing parallel processes
    // so that all processes have will the correct Shared address
    //
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    
    // init shared memory structures
    Shared->xformerModel	= PFITDF_FLY;
    Shared->input		= XInput;
    Shared->collideMode		= PF_ON;
    Shared->gui			= 1; // enable gui
    Shared->updateChannels	= 1;
    Shared->guiDirty = Shared->redraw = 1;
    
    // initiate multi-processing mode set in pfMultiprocess call
    // FORKS HAPPEN HERE !!!
    //
    pfConfig();
    
    // init shared mem for utility libraries
    pfuInit();
    pfiInit();

    
    // Initialize input structure (X or GL) for mouse and event inputs
    // then Open and configure full screen GL or GLX window.
    //
    
    // Configure window for the pipe
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setName(ProgName);
    pw->setOriginSize(WinXOrigin, WinYOrigin,
		      WinXOrigin +  WinXSize, WinYOrigin +  WinYSize);
    if (Shared->input == PFUINPUT_X)
	pw->setWinType(PFPWIN_TYPE_X);
    
    pw->setConfigFunc(OpenPipeWin);
    pw->config();
    
    pfFrame();
    
    pfuInitGUI(pw); 
    pfuInitInput(pw, Shared->input);
    
    //
    // App Process Setup
    //
    
    pfScene *scene = new pfScene;
    Shared->sceneDCS = new pfDCS;
    scene->addChild(Shared->sceneDCS);
    
    pfGroup *a_tex_cube = MakeTexCube();
    Shared->sceneDCS->addChild(a_tex_cube);
    
    // Set intersection callback.
    pfIsectFunc(IsectFunc);
    // do intersection setup
    pfuCollideSetup(scene, PFUCOLLIDE_STATIC, 0xffffffff);
    
    Shared->scene = scene;
    
    // Create pfChannels and assign draw callback functions.
    // Channels will automatically be assigned to the first created window
    // on the pfPipe.
    //
    
    pfChannel *chan = new pfChannel(p);
    Shared->chan = chan;
    
    chan->setTravFunc(PFTRAV_CULL, CullChannel);
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    chan->setScene(scene);
    chan->setNearFar(0.1f, far);
    // vertical FOV is matched to window aspect ratio
    chan->setFOV(45.0f, -1.0f);
    
    
    // Initialize sun - create a light source in the "south-west" (QIII)
    Shared->sun = new pfLightSource;
    Shared->sun->setPos(-0.3f, -0.3f, 1.0f, 0.0f);
    Shared->scene->addChild(Shared->sun);
    
    pfFrameStats *stats = chan->getFStats();
    stats->setClass(PFSTATS_ENGFX, PF_ON);
    
    // set up scene bounding box and initial viewing position
    {
	float sceneSize;
	pfVec3 view;
	// Set initial view to be "in front" of scene
	
	// view point at center of bbox
	// determine extent of scene's geometry
	pfBox bbox;
	pfuTravCalcBBox(scene, &bbox);
	
	// find max dimension
	sceneSize = bbox.max[PF_X] - bbox.min[PF_X];
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] -
			    bbox.min[PF_Y]);
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] -
			    bbox.min[PF_Z]);
	sceneSize = PF_MIN2(sceneSize, 0.5f * far);
	Shared->sceneSize = sceneSize;
	view.add(bbox.min, bbox.max);
	Shared->viewCoord.xyz.scale(0.5f, view);
	
	// offset so all is visible
	Shared->viewCoord.xyz[PF_Y] -=      sceneSize;
	Shared->viewCoord.xyz[PF_Z] += 0.25f*sceneSize;
	
	Shared->viewCoord.hpr.set(0.0f, 0.0f, 0.0f);
	chan->setView(Shared->viewCoord.xyz,
		      Shared->viewCoord.hpr);
	Shared->initView = Shared->viewCoord;
    }
    Shared->startXYZ.copy(Shared->viewCoord.xyz);
    
    Shared->chan->setViewport(0.0f, 1.0f, 0.3f, 1.0f);
    pfuGUIViewport(0.0f, 1.0f, 0.0f, 0.3f);
    // init the control panel for perfly
    Shared->guiPanel = InitPanel();
    pfuEnablePanel(Shared->guiPanel);
    
    // Initialize channel viewing model for interactive motion
    InitXformer();
    
    // main simulation loop
    while (!Shared->exitFlag)
    {
	// wait until next frame boundary
	pfSync();
	
	//
	// All latency critical processing should be done here. This is
	// typically the viewing position.
	// One should also read latency critical devices here.
	//
	pfuGetMouse(&Shared->mouse);
	UpdateView(Shared->chan, Shared->scene);
	
	// initiate traversal using current state
	pfFrame();
	
	// Get snapshot of event/key queues
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
		Shared->chan->setViewport(0.0f, 1.0f, top, 1.0f);
	    }
	    else
	    {
		pfuEnableGUI(FALSE);
		Shared->chan->setViewport(0.0f, 1.0f, 0.0f, 1.0f);
	    }
	    Shared->guiDirty = Shared->updateChannels = 0;
	} else if (Shared->gui)
	{
	    updateGUIViewMsg(&(Shared->viewCoord));
	    pfuUpdateGUI(&(Shared->mouse));
	}
    }
    // exit GUI and print avg time to draw GUI - optional
    pfuExitGUI();
    // Exit from libpfutil and remove shared mem arenas
    pfuExitUtil();
    pfuExitInput();
    
    // terminate cull and draw processes (if they exist)
    pfExit();
    
    // exit to operating system
    return 0;
}

static void
InitXformer(void)
{
    Shared->xformer = (pfiXformer *)pfiNewTDFXformer(pfGetSharedArena());
    Shared->xformer->setAutoInput(Shared->chan, &Shared->mouse, &Shared->events);
    Shared->xformer->setNode(Shared->scene);
    Shared->xformer->setAutoPosition(Shared->chan, Shared->sceneDCS);
    Shared->xformer->setResetCoord(&Shared->initView);
    xformerModel(Shared->xformerModel, Shared->collideMode);

}

static void
IsectFunc(void *)
{
    pfiCollideXformer(Shared->xformer);
}


/******************************************************************************
*			    App Viewing Model Routines
******************************************************************************
*/

//
// Thes routines use the viewing and tranformation utilities in
// libpfutil to implement several fiewing modes: drive, fly, and trackball.
//

static void
xformerModel(int model, int collideMode)
{
    static int		oldModel = -1, oldCollMode = -1;

    if ((model == oldModel && oldCollMode == collideMode) || !Shared->xformer)
	return;

    if (oldModel != model)
	Shared->xformer->selectModel(model);
    
    /* Collide with objects in scene */
    if (oldCollMode != collideMode)
    {
	if (collideMode == PF_ON)
	    Shared->xformer->enableCollision();
	else
	    Shared->xformer->disableCollision();
	oldCollMode = collideMode;
    }
}

//
// Update the current view
//
static void
UpdateView(pfChannel *, pfScene *)
{    
    pfiUpdateXformer(Shared->xformer);
    Shared->xformerModel = Shared->xformer->getCurModelIndex();
    
    /* if have moving-eyepoint motion model, update eyepoint */
    if (Shared->xformer->getCurModel()->isOfType(
	    pfiInputXformTravel::getClassType()))
    {
	Shared->xformer->getCoord(&Shared->viewCoord);
    }
}


//
// Reset view to original XYZ position and HPR Euler angles
//
static void
resetPosition(void)
{
    Shared->xformer->stop();
    Shared->xformer->resetPosition();

    // reset all detail texture gui to defaults
    LOD1 = DEF_LOD1;
    SCALE1 = DEF_SCALE1;
    LOD2 = DEF_LOD2;
    SCALE2 = DEF_SCALE2;
    LOD3 = DEF_LOD3;
    SCALE3 = DEF_SCALE3;
    LOD4 = DEF_LOD4;
    SCALE4 = DEF_SCALE4;
    TEX_CLAMP = -1;
}


/******************************************************************************
*			    App Input Handling
******************************************************************************
*/

static void
ProcessInputIO(void)
{
    int i,j,key;
    int dev, val, numDevs;
    pfuEventStream *pfevents;
    
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
		// worst case in APP_CULL_DRAW mode it takes 3 frames to
		// propagate new window size back down to draw process.
		//
		Shared->redraw = 3;
		pfuRedrawGUI();
		pfevents->devCount[dev] = 0; // mark device done
		break;
		
	    case PFUDEV_WINQUIT:
		Shared->exitFlag = 1;
		pfevents->devCount[dev] = 0; // mark device done
		break;
		
		// Main Keyboard
	    case PFUDEV_KEYBD:
		for(i=0; i<pfevents->numKeys; i++)
		{
		    key = pfevents->keyQ[i];
		    if (pfevents->keyCount[key])
		    {	// key was pressed count times
			switch(key)
			{
			case 27:	// ESC
			    Shared->exitFlag = 1;
			    break;
			case 'g':
			case 'G':
			    Shared->drawStats = !Shared->drawStats;
			    pfuWidgetValue(Shared->wStats,
					   Shared->drawStats);
			    break;
			case 'r':
			case 'R':
			    resetPosition();
			    break;
			case ' ':
			    Shared->xformer->stop();
			    break;
			default:
			    break;
			}
		    }
		}
		// XXX this is very important or else future keybd events
                // will be lost !!!
                //
                pfevents->devCount[dev] = 0; // mark device done
		break;
	    case PFUDEV_F1KEY:
		Shared->gui = !Shared->gui;
		pfuWidgetValue(Shared->wKillGUI, Shared->gui);
		Shared->updateChannels = 1;
		pfevents->devCount[dev] = 0; // mark device done
		break;
	    } // switch on device
	} // if have device
    } // for each device
    pfevents->numDevs = 0;
    pfevents->numKeys = 0;
}

/******************************************************************************
*			    Panel Creation and Management
******************************************************************************
*/

static pfuGUIString viewModel_strs[] = {"Drive", "Fly", "Trackball"};
static int viewModel_vals[] = {
    PFITDF_DRIVE, PFITDF_FLY, PFITDF_TRACKBALL};


static pfuPanel *
InitPanel(void)
{
    float  xo, yo, ys, xs;
    int x, y, yTop;
    pfuWidget *wid;
    pfuPanel *pan;
    
    pan = pfuNewPanel();
    
    pfuGetPanelOriginSize(pan, &xo, &yo, &xs, &ys);
    
    x = xo;
    yTop = (yo + ys);
    y = yTop - PFUGUI_BUTTON_YINC;
    
    // action  buton - disable the gui (F1KEY to get it back)
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, 0);
    pfuWidgetDim(wid,  x,  y,  24,  24);
    pfuWidgetValue(wid, 1.0f);
    pfuWidgetDefaultValue(wid, 1.0f);
    pfuWidgetActionFunc(wid, KillGUI);
    Shared->wKillGUI = wid;
    x += 24 + PFUGUI_PANEL_FRAME;
    
    // action  buton - reset to origin button
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_REPOSITION);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_VLONG_XSIZE,
		 PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Reposition");
    pfuWidgetActionFunc(wid, PanelControl);
    x += PFUGUI_BUTTON_VLONG_XINC;
    
    // the message bar with positions
    wid = pfuNewWidget(pan, PFUGUI_MESSAGE, 0);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_MESSAGE_XSIZE,
		 PFUGUI_MESSAGE_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    x += PFUGUI_MESSAGE_XINC;
    Shared->wPosXYZ = wid;
    
    
    // action button - reset all
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_RESET_ALL);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_VLONG_XSIZE,
		 PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Reset All");
    pfuWidgetActionFunc(wid, PanelControl);
    Shared->wResetAll = wid;
    
    x = xo;
    y = yTop - 2*PFUGUI_BUTTON_YINC;
    
    // action button - quit
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_QUIT);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_XSIZE,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Quit");
    x += PFUGUI_BUTTON_XINC;
    
    // menu button - view model
    wid = pfuNewWidget(pan, PFUGUI_MENU_BUTTON, CTL_MODEL);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_VLONG_XSIZE,
		 PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Motion");
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetSelections(wid, viewModel_strs, viewModel_vals, NULL, 3);
    Shared->wFModel = wid;
    x += PFUGUI_BUTTON_VLONG_XINC;
    
    // switch button - stats
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_DIAGS);
    pfuWidgetDim(wid, x, y, PFUGUI_SCALE_XSIZE(80), PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Stats");
    pfuWidgetActionFunc(wid, PanelControl);
    Shared->wStats = wid;
    
    x = xo;
    y -= PFUGUI_SLIDER_YINC;
    
    // horizontal slider LOD Ctrl Pt 1
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_LOD1_SLIDER);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "LOD Ctrl Pt 1");
    pfuWidgetRange(wid, 1, 0.0f, 6.5f, LOD1);
    pfuWidgetActionFunc(wid, PanelControl);
    
    x += PFUGUI_SLIDER_XINC;
    
    // horizontal slider SCALE Ctrl Pt 1
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_SCALE1_SLID);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "SCALE Ctrl Pt 1");
    pfuWidgetRange(wid, 1, 0.0f, 6.5f, SCALE1);
    pfuWidgetActionFunc(wid, PanelControl);
    
    x = xo;
    y -= PFUGUI_SLIDER_YINC;
    
    // horizontal slider LOD Ctrl Pt 2
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_LOD2_SLIDER);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "LOD Ctrl Pt 2");
    pfuWidgetRange(wid, 1, 0.0f, 6.5f, LOD2);
    pfuWidgetActionFunc(wid, PanelControl);
    
    x += PFUGUI_SLIDER_XINC;
    
    // horizontal slider SCALE Ctrl Pt 2
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_SCALE2_SLID);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "SCALE Ctrl Pt 2");
    pfuWidgetRange(wid, 1, 0.0f, 6.5f, SCALE2);
    pfuWidgetActionFunc(wid, PanelControl);
    
    x = xo;
    y -= PFUGUI_SLIDER_YINC;
    
    // horizontal slider LOD Ctrl Pt 3
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_LOD3_SLIDER);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "LOD Ctrl Pt 3");
    pfuWidgetRange(wid, 1, 0.0f, 6.5f, LOD3);
    pfuWidgetActionFunc(wid, PanelControl);
    
    x += PFUGUI_SLIDER_XINC;
    
    // horizontal slider SCALE Ctrl Pt 3
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_SCALE3_SLID);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "SCALE Ctrl Pt 3");
    pfuWidgetRange(wid, 1, 0.0f, 6.5f, SCALE3);
    pfuWidgetActionFunc(wid, PanelControl);
    
    x = xo;
    y -= PFUGUI_SLIDER_YINC;
    
    // horizontal slider LOD Ctrl Pt 4
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_LOD4_SLIDER);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "LOD Ctrl Pt 4");
    pfuWidgetRange(wid, 1, 0.0f, 6.5f, LOD4);
    pfuWidgetActionFunc(wid, PanelControl);
    
    x += PFUGUI_SLIDER_XINC;
    
    // horizontal slider SCALE Ctrl Pt 4
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_SCALE4_SLID);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_XSIZE,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "SCALE Ctrl Pt 4");
    pfuWidgetRange(wid, 1, 0.0f, 6.5f, SCALE4);
    pfuWidgetActionFunc(wid, PanelControl);
    
    return pan;
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

//
// PanelControl
// -------------
// This function is called from the draw process from the currently
// active widget.
//

static void
PanelControl(pfuWidget *w)
{
    float val;
    switch(pfuGetWidgetId(w))
    {
    case CTL_RESET_ALL:
	pfuResetGUI();
	resetPosition();
	Detail_mod = TRUE;
	break;
    case CTL_REPOSITION:
	resetPosition();
	Detail_mod = TRUE;
	break;
    case CTL_QUIT:
	Shared->exitFlag = 1;
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     " Have Quit button - bye...\n");
	*/
	break;
    case CTL_MODEL:
	val = pfuGetWidgetValue(w);
	Shared->xformerModel = val;
	xformerModel(val, Shared->collideMode);
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     " Have Viewing Model: %.0f\n", val);
	*/
	break;
    case CTL_DIAGS:
	if (pfuGetWidgetValue(w) == 0.0f)
	    Shared->drawStats = 0;
	else
	    Shared->drawStats = 1;
	break;
    case CTL_LOD1_SLIDER:
	LOD1 = pfuGetWidgetValue(w);
	Detail_mod = TRUE;
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Have GUI Widget \"%s\",  id=%d, val = %f\n",
		     pfuGetWidgetLabel(w),
		     pfuGetWidgetId(w), pfuGetWidgetValue(w));
	*/
	break;
    case CTL_SCALE1_SLID:
	SCALE1 = pfuGetWidgetValue(w);
	Detail_mod = TRUE;
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Have GUI Widget \"%s\",  id=%d, val = %f\n",
		     pfuGetWidgetLabel(w),
		     pfuGetWidgetId(w), pfuGetWidgetValue(w));
	*/
	break;
    case CTL_LOD2_SLIDER:
	LOD2 = pfuGetWidgetValue(w);
	Detail_mod = TRUE;
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Have GUI Widget \"%s\",  id=%d, val = %f\n",
		     pfuGetWidgetLabel(w),
		     pfuGetWidgetId(w), pfuGetWidgetValue(w));
	*/
	break;
    case CTL_SCALE2_SLID:
	SCALE2 = pfuGetWidgetValue(w);
	Detail_mod = TRUE;
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Have GUI Widget \"%s\",  id=%d, val = %f\n",
		     pfuGetWidgetLabel(w),
		     pfuGetWidgetId(w), pfuGetWidgetValue(w));
	*/
	break;
    case CTL_LOD3_SLIDER:
	LOD3 = pfuGetWidgetValue(w);
	Detail_mod = TRUE;
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Have GUI Widget \"%s\",  id=%d, val = %f\n",
		     pfuGetWidgetLabel(w),
		     pfuGetWidgetId(w), pfuGetWidgetValue(w));
	*/
	break;
    case CTL_SCALE3_SLID:
	SCALE3 = pfuGetWidgetValue(w);
	Detail_mod = TRUE;
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Have GUI Widget \"%s\",  id=%d, val = %f\n",
		     pfuGetWidgetLabel(w),
		     pfuGetWidgetId(w), pfuGetWidgetValue(w));
	*/
	break;
    case CTL_LOD4_SLIDER:
	LOD4 = pfuGetWidgetValue(w);
	Detail_mod = TRUE;
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Have GUI Widget \"%s\",  id=%d, val = %f\n",
		     pfuGetWidgetLabel(w),
		     pfuGetWidgetId(w), pfuGetWidgetValue(w));
	*/
	break;
    case CTL_SCALE4_SLID:
	SCALE4 = pfuGetWidgetValue(w);
	Detail_mod = TRUE;
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Have GUI Widget \"%s\",  id=%d, val = %f\n",
		     pfuGetWidgetLabel(w),
		     pfuGetWidgetId(w), pfuGetWidgetValue(w));
	*/
	break;
    default:
	/*
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Have GUI Widget \"%s\": id=%d, val = %d\n",
		     pfuGetWidgetLabel(w), pfuGetWidgetId(w), pfuGetWidgetValue(w));
	*/
	break;
    }
    // Check if detail texture sliders changed, if so
    // modify pfTexSpline
    //
    if (Detail_mod)
    {
	ctl_pts[0].set(-LOD1, SCALE1);
	ctl_pts[1].set(-LOD2, SCALE2);
	ctl_pts[2].set(-LOD3, SCALE3);
	ctl_pts[3].set(-LOD4, SCALE4);
	tex->setSpline(PFTEX_DETAIL_SPLINE, ctl_pts, TEX_CLAMP);
	Detail_mod = FALSE;
	
	/* pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"Updating pfTexSpline\n"); */
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

//
//	OpenPipeWin() -- create a pipeline: setup the window system,
//	the OpenGL, and OpenGL Performer. this procedure is executed in
//	the draw process (when there is a separate draw process).
//

static void
OpenPipeWin(pfPipeWindow *pw)
{
    static const pfVec4  clrt(0.15f, 0.15f, 0.15f, 1.0f);
    static char msg[] = "Welcome to OpenGL Performer";
    
    // use pfu utility to open the window so that the pfu input
    // handler will be able to attach to it
    //
    pw->open();
    
    // Draw introductory message
    pfClear(PFCL_COLOR, &clrt);
    // Message will be draw in in purple - the pfuDrawMessage() color
    pfuDrawMessage(NULL, msg, PFU_MSG_PIPE, PFU_CENTER_JUSTIFIED, 
		   0.5f, 0.5f, PFU_FONT_BIG, PFU_RGB);
    pw->swapBuffers();
}

/******************************************************************************
*			    Draw Process Routines
******************************************************************************
*/

//
//	DrawChannel() -- draw a channel and read input queue. this
//	procedure is executed in the draw process (when there is a
//	separate draw process).
//
static void
DrawChannel (pfChannel *channel, void *)
{
    static pfVec4 bgclr(0.10f, 0.10f, 0.10f, 1.0f);
    
    // erase framebuffer and draw Earth-Sky model
    pfClear(PFCL_COLOR | PFCL_DEPTH, &bgclr);
    
    // invoke Performer draw-processing for this frame
    pfDraw();
    
    {
	int err = glGetError();
	if (err != GL_NO_ERROR)
	    pfNotify(PFNFY_NOTICE,PFNFY_USAGE,
		     "OpenGL Error 0x%x - %s",err, gluErrorString(err));
    }
    
    
    // Enable drawing of Performer throughput statistics
    if (Shared->drawStats)
	channel->drawStats();
    
    // Check the mouse and keyboard = only needed for GL input
    // This is done after the draw becuase it can happen in parallel
    // with the pipe finishing the rendering of the frame.
    //
    pfuCollectInput();
}

/******************************************************************************
*			    Cull Process Routines
******************************************************************************
*/

//
//	CullChannel() -- traverse the scene graph and generate a
// 	display list for the draw process.  This procedure is
//	executed in the cull process.
//
static void
CullChannel(pfChannel *, void *)
{
    //
    // pfDrawGeoSet or other display listable Performer routines
    // could be invoked before or after pfCull()
    // 
    
    pfCull();
}

//
// texcube.c		routine for constructing a textured cube geoset
//
//

static pfGroup*
MakeTexCube()
{
    pfGeode *thiscube = new pfGeode;
    pfGroup *cube = new pfGroup;
    pfGeoSet *gset = new pfGeoSet;
    
    pfVec3 *verts = (pfVec3*) new(8*sizeof(pfVec3)) pfMemory;
    verts[0].set(-3.0f, -3.0f,  3.0f);
    verts[1].set( 3.0f, -3.0f,  3.0f);
    verts[2].set( 3.0f,  3.0f,  3.0f);
    verts[3].set(-3.0f,  3.0f,  3.0f);
    verts[4].set(-3.0f, -3.0f, -3.0f);
    verts[5].set( 3.0f, -3.0f, -3.0f);
    verts[6].set( 3.0f,  3.0f, -3.0f);
    verts[7].set(-3.0f,  3.0f, -3.0f);
    
    ushort *vindex = (ushort*) new(24*sizeof(ushort)) pfMemory;
    vindex[0]  = 0; vindex[1]  = 1; vindex[2]  = 2; vindex[3]  = 3; /* front */
    vindex[4]  = 0; vindex[5]  = 3; vindex[6]  = 7; vindex[7]  = 4; /* left */
    vindex[8]  = 4; vindex[9]  = 7; vindex[10] = 6; vindex[11] = 5; /* back */
    vindex[12] = 1; vindex[13] = 5; vindex[14] = 6; vindex[15] = 2; /* right */
    vindex[16] = 3; vindex[17] = 2; vindex[18] = 6; vindex[19] = 7; /* top */
    vindex[20] = 0; vindex[21] = 4; vindex[22] = 5; vindex[23] = 1; /* bottom */
    
    pfVec3 *norms = (pfVec3*) new(6*sizeof(pfVec3)) pfMemory;
    norms[0].set( 0.0f,  0.0f,  1.0f);
    norms[1].set( 0.0f,  0.0f, -1.0f);
    norms[2].set( 0.0f,  1.0f,  0.0f);
    norms[3].set( 0.0f, -1.0f,  0.0f);
    norms[4].set( 1.0f,  0.0f,  0.0f);
    norms[5].set(-1.0f,  0.0f,  0.0f);
    
    ushort *nindex = (ushort*) new(24*sizeof(ushort)) pfMemory;
    nindex[0]  = nindex[1]  = nindex[2]  = nindex[3]  = 0;
    nindex[4]  = nindex[5]  = nindex[6]  = nindex[7]  = 5;
    nindex[8]  = nindex[9]  = nindex[10] = nindex[11] = 1;
    nindex[12] = nindex[13] = nindex[14] = nindex[15] = 4;
    nindex[16] = nindex[17] = nindex[18] = nindex[19] = 2;
    nindex[20] = nindex[21] = nindex[22] = nindex[23] = 3;
    
    pfVec2 *tcoords = (pfVec2*) new(4*sizeof(pfVec2)) pfMemory;
    tcoords[0].set(0.0f, 0.0f);
    tcoords[1].set(1.0f, 0.0f);
    tcoords[2].set(1.0f, 1.0f);
    tcoords[3].set(0.0f, 1.0f);
    
    ushort *tindex = (ushort*) new(24*sizeof(ushort)) pfMemory;
    tindex[0]  = 0; tindex[1]  = 1; tindex[2]  = 2; tindex[3]  = 3;
    tindex[4]  = 0; tindex[5]  = 1; tindex[6]  = 2; tindex[7]  = 3;
    tindex[8]  = 0; tindex[9]  = 1; tindex[10] = 2; tindex[11] = 3;
    tindex[12] = 0; tindex[13] = 1; tindex[14] = 2; tindex[15] = 3;
    tindex[16] = 0; tindex[17] = 1; tindex[18] = 2; tindex[19] = 3;
    tindex[20] = 0; tindex[21] = 1; tindex[22] = 2; tindex[23] = 3;
    
    //
    // set the coordinate, normal and color arrays
    // and their corresponding index arrays
    //
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, verts, vindex);
    gset->setAttr(PFGS_NORMAL3, PFGS_PER_PRIM, norms, nindex);
    gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, tindex);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(6);
    
    //
    // create a geostate from shared memory, enable
    // texturing (in case that's not the default), and
    // set the geostate for this geoset
    //
    pfGeoState *gstate = new pfGeoState;
    
    //
    // create a new texture from shared memory,
    // load a texture file and add texture to geostate
    //
    tex = new pfTexture;
    if( !(tex->loadFile(TexName)) )
    {
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
		 "Detail: could not load base texture %s. Exiting",TexName);
    }
    dtex = new pfTexture;
    if( !(dtex->loadFile(DTexName)) )
    {
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
		 "Detail: could not load detail texture %s. Exiting",DTexName);
    }
    else
    {
	HaveDetail = 1;
	tex->setDetail(PFTEX_DEFAULT, dtex);
    }
    gstate->setMode(PFSTATE_ENTEXTURE,1);
    gstate->setAttr(PFSTATE_TEXTURE, tex);
    
    // defining the detail texture spline
    tex->setSpline(PFTEX_DETAIL_SPLINE, ctl_pts, TEX_CLAMP);
    // if texture has alpha component, enable transparency  and alpha func
    {
	uint *i;
	int nc, sx, sy, sz;
	tex->getImage(&i, &nc, &sx, &sy, &sz);
	// if have alpha channel, enable transparency
	if (nc != 3)
	{
	    gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_FAST);
	    // set alpha function to block pixels of 0 alpha for 
	    // transparent textures
	    gstate->setMode(PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
	    gstate->setVal(PFSTATE_ALPHAREF, 0.0f);
	}
    }
    
    //
    // create a new texture environment from shared memory,
    // decal the texture since the geoset has no color to
    //
    pfTexEnv *tenv = new pfTexEnv;
    tenv->setMode(PFTE_DECAL);
    gstate->setAttr(PFSTATE_TEXENV, tenv);
    
    gset->setGState(gstate);
    
    thiscube->addGSet(gset);
    
    cube->addChild(thiscube);
    
    return cube;
}
