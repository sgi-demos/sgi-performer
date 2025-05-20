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
// file: statsC.C
// -------------
// 
// OpenGL Performer example using cull and draw process callbacks
// and demonstrating Performer Statistics and Debugging utilities.
// 
// $Revision: 1.23 $ 
// $Date: 2001/06/04 16:26:50 $
// 
// Commandline options:
// --------------------
//  'e': use an earth-sky model instead of just clearing screen
//  'D': set on PFMP_CULL_DL_DRAW
//  'o': turn on PFMP_CULLoDRAW - needs forked draw
//  'p': arg to pfMultiprocess
//  'S': do sleeps in APP, CULL, and DRAW so can see mp history timing lines
//  'w': write out scene to scene.out at start of prog
//  '?': get usage
//
// Run-time controls:
// -------------------
//    Left-mouse: advance
//  Middle-mouse: stop
//   Right-mouse: retreat
// PRINTSCREEN-key: snap picture of window to a stats.%d.rgb file
//       ESC-key: exits
//        F1-key: profile
//	  F2-key: toggle use of second channel for stats
//    UPARROWKEY: increase number of frames of stats PFTIMES history 
//  DOWNARROWKEY: decrease number of frames of stats PFTIMES history 
//	      p/P: print currently enabled statistics to stderr
//	      a/A: toggle entire statistics class enable
//              b: toggle bottleneck statistics on IR
//              B: print bottleneck statistics on IR
//	      s/S: toggle system CPU statistics class enable
//	      d/D: toggle database statistics class enable
//	      c/C: toggle cull statistics class enable
//	      f/F: toggle fill stats - depth complexity
//	        g: toggle graphics statistics class enable
//	        G: toggle geometry attribute statistics - counts for 
//				normals, colors, texcoords)
//	      h/F: toggle proc frame time history stats - the timing graph
//	      m/M: toggle mp process frame time stats - all frame time stats
//	      q/Q: do a pfQuery and print results to stderr
//	      r/R: reset view
//	        t: toggle tmesh lengths graph
//		T: toggle transparency fill stats -  is part of fill stats
//	      v/V: toggle Database Node Visibility stats
//	      z/Z: toggle z-compare check in fill stats


#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <getopt.h> // for cmdline handler 

#include <X11/X.h>
#include <X11/keysym.h>

#include <Performer/pf/pfPipeWindow.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfEarthSky.h>
#include <Performer/pf/pfFrameStats.h>
#include <Performer/pf/pfScene.h>

#include <Performer/pr/pfLight.h>

#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void IsectFunc(void *data);
static void DrawStatsChannel (pfChannel *channel, void *data);
static void CullStatsChannel(pfChannel *ch, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static void UpdateView(void);
static void GetXInput(void);
static void Usage(void);
static void EnableStatsChan(int flag);


#define HISPEED		0.1f
#define LOSPEED		0.001f


//
// structure that resides in shared memory so that the
// application, cull, and draw processes can access it.

typedef struct
{
    pfPipeWindow *pw;
    int        exitFlag;
    int        reset;
    int        inWindow;
    float       mouseX;
    float	mouseY;
    int		mouseButtons;
    int		winSizeX;
    int		winSizeY;
    pfCoord	view;
    float	sceneSize;
    int		drawStats;
    pfFrameStats *frameStats;
    int		statsInterval;
    int		statsEnable, statsClass, statsClassMode;
    int		statsHistFrames;
    int		printStats;
    int		doStatsQuery;
    pfChannel	*chan, *statsChan;
    int	haveStatsChan;
    float	speed;
    int		snapImage;
    int         Sleep;
    float	SleepApp;
    float	SleepDraw;
    float	SleepCull;
    float	SleepIsect;
    int         doBottleneck;
} SharedData;

static SharedData *Shared;

// light source created and updated in draw-process 

static pfLight *Sun;
static int ProcSplit = PFMP_DEFAULT;
static int CulloDraw = 0;
static int CullDLDraw = 0;
static int WriteScene = 0;
static int varESky = 0;
static int CachedCull = 0;
static int WinOriginX=0, WinOriginY=0, WinSizeX=800, WinSizeY=600;
static int WinType = PFPWIN_TYPE_X;

static void
Nap(double t)
{
    static int	firstTime = 1, nSpins = 1;
    int		i;
    double	start;
    
    if (firstTime)
    {
	char	*spin;
	
	firstTime = 0;
	if (spin = getenv("PFSPINCOUNT"))
	    nSpins = atoi(spin);
    }
    
    if (nSpins < 0)
    {
	sginap(1);
	return;
    }
    
    t *= 0.001f;
    
    start = pfGetTime();
    
    do
    {    
    	for (i=0; i<nSpins; i++)
	    sginap(0);
	
    } while (pfGetTime() - start < t);
}


//
//			    Main and Commandline Processing
//

char ProgName[PF_MAXSTRING];
char OptionStr[] = "CDeop:Swx?";


//
//	Usage() -- print usage advice and exit. This procedure
//	is executed in the application process.


static void
Usage (void)
{
    fprintf(stderr, 
	    "Usage: %s [-ewS] [-p procSplit] [file.ext ...]\n", ProgName);
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
    while ((opt = getopt(argc, argv, OptionStr)) != -1)
    {
	switch (opt)
	{
	case 'e': // use an earth-sky model instead of just clearing screen 
	    varESky = 1;
	    break;
	case 'D':  // set on PFMP_CULL_DL_DRAW 
	    CullDLDraw = PFMP_CULL_DL_DRAW;
	    break;
	case 'C': // Cache cull results 
	    CachedCull = 1;
	    break;
	case 'o':  // turn on PFMP_CULLoDRAW - needs forked draw 
	    CulloDraw = PFMP_CULLoDRAW;
	    break;
	case 'p': // arg to pfMultiprocess 
	    ProcSplit = atoi(optarg);
	    break;
	case 'S': // do sleeps in APP,CULL, and DRAW so can see app timing lines 
	    Shared->Sleep ^= 1;
	    break;
	case 'w': // write out scene to scene.out at start of prog 
	    WriteScene = 1;
	    break;
	case 'W': // Set the window size 
	    if (sscanf(optarg, "%ld,%ld,%ld,%ld", 
		       &WinOriginX, &WinOriginY,
		       &WinSizeX, &WinSizeY) != 4)
	    {
		if (sscanf(optarg, "%ld,%ld", &WinSizeX, &WinSizeY) != 2)
		{
		    if (sscanf(optarg, "%ld", &WinSizeX) == 1)
		    {
			WinSizeY = WinSizeX;
			WinOriginX = -1;
		    }
		    else
			WinSizeX = -1;
		} 
		else
		    WinOriginX = -1;
	    }
	    break;
	case 'x':
	    WinType ^= PFPWIN_TYPE_X;
	    break;
	case '?': // get usage 
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
    pfNode *root;
    pfBox           bbox;
    float	    far = 10000.0f;
    FILE	   *fp=0;
    pfCoord	    reset;
    
#if 0
    if (argc < 2)
	Usage();
#endif    
    
    arg = docmdline(argc, argv);
    
    pfInit();
    pfNotifyLevel(PFNFY_DEBUG);
    
    pfMultiprocess(ProcSplit | CulloDraw | CullDLDraw);
    
    // allocate and init shared before fork()'ing parallel processes 
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    Shared->haveStatsChan = 1;
    Shared->statsEnable = 0;
    Shared->statsClass = 0;
    Shared->statsHistFrames = 4;
    Shared->statsClassMode = 0;
    Shared->speed = 0.0f;
    
    Shared->winSizeX = WinSizeX;
    Shared->winSizeY = WinSizeY;

    // Load all loader DSO's before pfConfig() forks 
    for (found = arg; found < argc; found++)
	pfdInitConverter(argv[found]);
    
    // initiate multi-processing mode set in pfMultiprocess call 
    pfConfig();
    
    pfScene *scene = new pfScene();
    
    if (arg < argc)
    {
	// specify directories where geometry and textures exist 
	if (!(getenv("PFPATH")))
	    pfFilePath(
		       "."
		       ":./data"
		       ":../data"
		       ":../../../../data"
		       ":/usr/share/Performer/data"
		       );
	
	// load files named by command line arguments 
	for (found = 0; arg < argc; arg++)
	{
	    if ((root = pfdLoadFile(argv[arg])) != NULL)
	    {
		scene->addChild(root);
		found++;
	    }
	}
    }
    
    pfPipe *p = pfGetPipe(0);
    pfPhase(PFPHASE_FLOAT);
    
    Shared->pw = new pfPipeWindow(p);
    Shared->pw->setName("OpenGL Performer");
    Shared->pw->setWinType(WinType);
    Shared->pw->setOriginSize(WinOriginX, WinOriginY, WinSizeX, WinSizeY);
    // Open and configure the GL window. 
    Shared->pw->setConfigFunc(OpenPipeWin);
    Shared->pw->config();
    
    //pfFrameRate(15.0f);
    
    pfChannel *chan = new pfChannel(p);
    chan->setTravFunc(PFTRAV_CULL, CullChannel);
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    pfIsectFunc(IsectFunc);
    chan->setScene(scene);
    chan->setNearFar(0.1f, far);
    // vertical FOV is matched to window aspect ratio 
    chan->setFOV(45.0f, -1.0f);
    Shared->chan = chan;
    
    // Create an earth/sky model that draws sky/ground/horizon 
    pfEarthSky *eSky = new pfEarthSky();
    if (varESky)
    {
	eSky->setMode(PFES_BUFFER_CLEAR, PFES_SKY_GRND);
	eSky->setAttr(PFES_GRND_HT, -10.0f);
    }
    else 
    {
	eSky->setMode(PFES_BUFFER_CLEAR, PFES_FAST);
	eSky->setColor(PFES_CLEAR, .3f, .3f, .7f, 1.0f);
    }
    
    chan->setESky(eSky);
    
    // create stats channel 
    pfChannel *statsChan = new pfChannel(p);
    statsChan->setTravFunc(PFTRAV_CULL, CullStatsChannel);
    statsChan->setTravFunc(PFTRAV_DRAW, DrawStatsChannel);
    statsChan->setViewport(0.5f, 1.0f, 0.0f, 1.0f);
    // don't do any statistics collection for the stats-display chan 
    statsChan->getFStats()->setClass(PFSTATS_ALL, PFSTATS_OFF );
    Shared->statsChan = statsChan;
    
    statsChan->setScene(scene);
    statsChan->setTravMode(PFTRAV_DRAW, PFDRAW_OFF);
    statsChan->setStatsMode(PFCSTATS_DRAW, PFSTATS_ALL );
    
    // Set initial view to be "in front" of scene 
    if (found)
    {	
	pfVec3 view;
	float sceneSize;
	// determine extent of scene's geometry 
	pfuTravCalcBBox(scene, &bbox);
	// view point at center of bbox 
	view.add(bbox.min, bbox.max);
	Shared->view.xyz.scale(0.5f, view);
	
	// find max dimension 
	sceneSize = bbox.max[PF_X] - bbox.min[PF_X];
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	sceneSize = PF_MIN2(sceneSize, 0.5f*far);
	Shared->sceneSize = sceneSize;
	
	// offset so all is visible 
	Shared->view.xyz[PF_Y] -=      sceneSize;
	Shared->view.xyz[PF_Z] += 0.25f*sceneSize;
	Shared->view.hpr.set(0.0f, 0.0f, 0.0f);
	chan->setView(Shared->view.xyz, Shared->view.hpr);
    }
    else
    {
	Shared->view.xyz.set(0.0f, 0.0f, 100.0f);
	PFSET_VEC3(bbox.min, -5000.0f, -5000.0f, -1000000.0f);
	PFSET_VEC3(bbox.max, 5000.0f, 5000.0f, 10000000.0f);
    }
    reset.xyz.copy(Shared->view.xyz);
    reset.hpr.copy(Shared->view.hpr);
    
    chan->setTravMode(PFTRAV_CULL, PFCULL_ALL);
    
    // initialize channel statistics for mail channel
    Shared->frameStats = chan->getFStats();
    // this would enable only proc frame times stats to be
    // displayed in the main channel
    //
    // pfChanStatsMode(chan, PFCSTATS_DRAW, PFFSTATS_ENPFTIMES);
    
    Shared->frameStats->setAttr(PFFSTATS_UPDATE_FRAMES, 30.0f);
    
    if (found && WriteScene)
    {
	fp = fopen ("scene.out","w");
	pfPrint(scene, PFTRAV_SELF|PFTRAV_DESCEND, PFPRINT_VB_DEBUG, fp);
	fclose(fp);
    }
    {
	pfFrameStats *fstats = new pfFrameStats();
	// pfuTravCountDB enables GFX and DB stats and
	// counts the scene stats into the CUR framestats 
	
	pfuTravCountDB((pfNode*)scene, fstats);
	// print the enabled and counted stats 
	pfPrint(fstats, PFFSTATS_BUF_CUR | fstats->getClass(PFSTATS_ALL),
		PFPRINT_VB_ON, fp);
	fstats->checkDelete();
    }
    
    if (CachedCull)
	pfuTravCachedCull((pfNode *) scene, 1);
    
    // main simulation loop 
    while (!Shared->exitFlag)
    {
	// sleep so can see line 
	if (Shared->Sleep)
	    Nap(Shared->SleepApp);
	
	// wait until next frame boundary 
	pfSync();
	
	// sleep so can see line 
	if (Shared->Sleep)
	    Nap(Shared->SleepApp);
	
	// Set view parameters. 
	if (Shared->reset)
	{
	    Shared->view.xyz.copy(reset.xyz);
	    Shared->view.hpr.copy(reset.hpr);
	    Shared->reset = 0;
	}
	else
	    UpdateView();
	chan->setView(Shared->view.xyz, Shared->view.hpr);
	if (CachedCull)
	{ // Flag all channel viewpoints as dirty 
	    pfuChanCullCache  *ccc = (pfuChanCullCache *)chan->getChanData();
	    ccc->chanId = 0;
	    ccc->updateFrame = pfGetFrameCount();
	    chan->passChanData();
	}
	
	// initiate traversal using current state 
	pfFrame();
	
	{ // set stats channel 
	    static int haveStatsChan = -1;
	    if (haveStatsChan != Shared->haveStatsChan)
		EnableStatsChan(Shared->haveStatsChan);
	    haveStatsChan = Shared->haveStatsChan;
	}
	
	// process stats selections 
	if (Shared->statsEnable)
	{
	    if (Shared->frameStats->getClass(Shared->statsEnable))
		Shared->frameStats->setClass(Shared->statsEnable, PFSTATS_OFF);
	    else
		Shared->frameStats->setClass(Shared->statsEnable, PFSTATS_ON);
	    Shared->statsEnable = 0;
	}
	if (Shared->statsClassMode)
	{
	    Shared->frameStats->setClassMode(Shared->statsClass,
                                             Shared->statsClassMode, 
					     !(Shared->frameStats->getClassMode(Shared->statsClass) &
					       Shared->statsClassMode));
	    Shared->statsClassMode = 0;
	}
	if (Shared->printStats)
	{ // print all enabled statistics 
	    pfPrint(Shared->frameStats, 
		    PFFSTATS_BUF_PREV | PFFSTATS_BUF_AVG |
		    Shared->frameStats->getClass(PFSTATS_ALL), 
		    PFPRINT_VB_INFO, NULL);
	    Shared->printStats = 0;
	}
	if (Shared->doStatsQuery)
	{ // show how to query a few individual statistics 
	    // note: if the corresponding stats class and mode is not enabled
	    // 	then the query will simply return 0 for that value.
	  unsigned int  qtmp[5];
	  float ftmp[5];
	  int ret;
	  
	  qtmp[0] = PFFSTATS_BUF_PREV | PFSTATSVAL_GFX_GEOM_TRIS;
	  qtmp[1] = PFFSTATS_BUF_PREV | PFFSTATSVAL_PFTIMES_PROC_TOTAL;
	  qtmp[2] = PFFSTATS_BUF_PREV | PFSTATSVAL_CPU_SYS_BUSY;
	  qtmp[4] = NULL;
	  ret = Shared->frameStats->mQuery(qtmp, ftmp, 0);
	  fprintf(stderr, "ret = %d bytes\n", ret);
	  fprintf(stderr, "Query num tris: %.0f\n", ftmp[0]);
	  fprintf(stderr, "Query frame time: %.0f msecs\n", ftmp[1]*1000.0f);	
	  fprintf(stderr, "Query  sys busy: %.0f%%\n", ftmp[2]);	
	  Shared->doStatsQuery = 0;

	  
	}
	
	if(Shared->doBottleneck) {
	  pfStatsValGfxPipeTimes times;
	  int ret;
	  ret = Shared->frameStats->query(PFFSTATS_BUF_PREV | PFSTATSVAL_GFXPIPE_TIMES, 
					  (float *)(&times), 
					  sizeof(times));
	  fprintf(stderr, "Times:\n");
	  fprintf(stderr, "  host (ms):  %f\n", times.time.host * 1000.0f);
	  fprintf(stderr, "  xform (ms): %f\n", times.time.xform * 1000.0f);
	  fprintf(stderr, "  fill (ms):  %f\n", times.time.fill * 1000.0f);
	  fprintf(stderr, "  total (ms): %f\n", times.time.total * 1000.0f);
	  fprintf(stderr, "Percentages:\n");
	  fprintf(stderr, "  host:  %f\n", times.percentage.host);
	  fprintf(stderr, "  xform: %f\n", times.percentage.xform);
	  fprintf(stderr, "  fill:  %f\n", times.percentage.fill);
	  fprintf(stderr, "  total: %f\n", times.percentage.total);
	  Shared->doBottleneck = 0;
	}

	if (Shared->frameStats->getClass(PFFSTATS_ENPFTIMES) &&
	    (Shared->frameStats->getClassMode(PFFSTATS_PFTIMES) &
	     PFFSTATS_PFTIMES_HIST) &&
	    (((int) Shared->frameStats->getAttr(PFFSTATS_PFTIMES_HIST_FRAMES))
	     != Shared->statsHistFrames))
	{
	    Shared->frameStats->setAttr(PFFSTATS_PFTIMES_HIST_FRAMES, 
					Shared->statsHistFrames);
	    Shared->statsHistFrames =
		Shared->frameStats->getAttr(PFFSTATS_PFTIMES_HIST_FRAMES);
	    // fprintf(stderr, "Set stats history frames to %d\n", 
	    //	    Shared->statsHistFrames); 
	}
    }
    
    // terminate cull and draw processes (if they exist) 
    pfExit();
    
    // exit to operating system 
    return 0;
}

static void 
EnableStatsChan(int flag)
{
    if (!Shared->statsChan)
    {
	Shared->chan->setViewport(0.0f, 1.0f, 0.0f, 1.0f);
	return;
    }
    if (flag)
    {
	// put view on the left,stats on the right 
	Shared->chan->setViewport(0.0f, 0.5f, 0.0f, 1.0f);
	Shared->statsChan->setTravMode(PFTRAV_DRAW, PFDRAW_ON);
    }
    else 
    {
	// put view on the left,stats on the right 
	Shared->chan->setViewport(0.0f, 1.0f, 0.0f, 1.0f);
	Shared->statsChan->setTravMode(PFTRAV_DRAW, PFDRAW_OFF);
    }
}

// 
//	UpdateView() updates the eyepoint based on the information
//	placed in shared memory by GetInput().

static void    
UpdateView(void)
{
    pfCoord *view = &Shared->view;
    float mx, my;
    float cp;
    
    if (!Shared->inWindow)
    {
	Shared->speed = 0.0f;
	return;
    }
    switch (Shared->mouseButtons)
    {
    case Button1Mask: // LEFTMOUSE: faster forward or slower backward
	if (Shared->speed > 0.0f)
	    Shared->speed *= 1.2f;
	else
	    Shared->speed /= 1.2f;
	
	if (PF_ABSLT(Shared->speed, LOSPEED*Shared->sceneSize))
	    Shared->speed = LOSPEED*Shared->sceneSize;
	else if (Shared->speed >=  HISPEED*Shared->sceneSize)
	    Shared->speed = HISPEED*Shared->sceneSize;
	break;
    case Button2Mask:	// MIDDLEMOUSE: stop moving and pick 
	Shared->speed = 0.0f;
	break;
    case Button3Mask: // RIGHTMOUSE: faster backward or slower foreward
	if (Shared->speed < 0.0f)
	    Shared->speed *= 1.2f;
	else
	    Shared->speed /= 1.2f;
	
	if (PF_ABSLT(Shared->speed, LOSPEED*Shared->sceneSize))
	    Shared->speed = -LOSPEED*Shared->sceneSize;
	else if (Shared->speed <=  -HISPEED*Shared->sceneSize)
	    Shared->speed = -HISPEED*Shared->sceneSize;
	break;
    }
    
    if (Shared->mouseButtons & (Button1Mask | Button2Mask | Button3Mask))
    {
	// update view direction 
	mx = 2.0f*(Shared->mouseX / (float)Shared->winSizeX) - 1.0f;
	my = 2.0f*(Shared->mouseY / (float)Shared->winSizeY) - 1.0f;
	view->hpr[PF_H] -= mx*PF_ABS(mx);
	view->hpr[PF_P]  = my*PF_ABS(my)*90.0f;
	view->hpr[PF_R]  = 0.0f;
    }
    
    // update view position 
    cp = cosf(PF_DEG2RAD(view->hpr[PF_P]));
    view->xyz[PF_X] += Shared->speed*sinf(-PF_DEG2RAD(view->hpr[PF_H]))*cp;
    view->xyz[PF_Y] += Shared->speed*cosf(-PF_DEG2RAD(view->hpr[PF_H]))*cp;
    view->xyz[PF_Z] += Shared->speed*sinf( PF_DEG2RAD(view->hpr[PF_P]));
}

//
//	OpenPipeWin() -- create a pipeline: setup the window system,
//	the OpenGL, and OpenGL Performer. this procedure is executed in
//	the draw process (when there is a separate draw process).


Display *Dsp=NULL;

static void
OpenPipeWin(pfPipeWindow *pw)
{
    Window win;
    
    pw->open();
    
    
    {  
	Dsp = pfGetCurWSConnection();
	win = pw->select()->getWSWindow();
	XSelectInput(Dsp, win, StructureNotifyMask | PointerMotionMask |
		     FocusChangeMask | ButtonPressMask | ButtonReleaseMask | 
		     KeyPressMask);
	XMapWindow(Dsp, win);
    }
    // create a light source in the "south-west" (QIII) 
    Sun = new pfLight();
    Sun->setPos(-0.3f, -0.3f, 1.0f, 0.0f);
}

static void
IsectFunc(void *)
{
    // sleep to show time in timing graph 
    Nap(2 * Shared->SleepIsect);
}

//
//	CullChannel() -- traverse the scene graph and generate a
// 	display list for the draw process.  This procedure is 
//	executed in the cull process.


static void
CullChannel(pfChannel *, void *)
{
    // 
    // pfDrawGeoSet or other display listable Performer routines
    // could be invoked before or after pfCull()
    
    
    pfCull();
    
    if (Shared->Sleep)
	Nap(Shared->SleepCull);
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
    
    // spin loop to show time before pfDraw() in timing graph 
    if (Shared->Sleep)
	Nap(Shared->SleepDraw);
    
    // snap picture of screen 
    
    
    // take snapshot of image 
    if (Shared->snapImage)
    {
	static char str[80];
	static int count = 0;
	pfPipe *cpipe = channel->getPipe();
        int id = pfGetId(cpipe)*10 + count++;
	sprintf(str, "stats.%d.rgb", id);
        fprintf(stderr, "Saving pipe %d image in file %s\n",
		pfGetId(cpipe), str);
	
	// read from FRONT-buffer to get channel stats info 
        glReadBuffer(GL_FRONT);
	pfuSaveImage(str, 0, 0, 
		     Shared->winSizeX, 
		     Shared->winSizeY, 0);
	
	Shared->snapImage = 0;
        fprintf(stderr, "Done\n");
    }
    
    
    // invoke Performer draw-processing for this frame 
    pfDraw();
    
    // draw Performer throughput statistics 
    if (Shared->drawStats)
	channel->drawStats();
    
    Shared->pw->getSize(&Shared->winSizeX, &Shared->winSizeY);
    
	GetXInput();
    
    // spin loop to show time after pfDraw() in timing graph 
    if (Shared->Sleep)
	Nap(Shared->SleepDraw);
    
}


static void
GetXInput(void)
{
    static int x, y;
    int haveMotion = 0;
    pfWSConnection dsp = pfGetCurWSConnection();
    
    if (XEventsQueued(dsp, QueuedAfterFlush))
    while (XEventsQueued(dsp, QueuedAlready))
    {
	XEvent event;
	
	XNextEvent(Dsp, &event);
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
		y =  Shared->winSizeY - motion_event->y;;
		haveMotion = 1;
		Shared->inWindow = 1;
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
		x = event.xbutton.x;
		y = Shared->winSizeY - event.xbutton.y;
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
		    exit(0);
		    break;
		case XK_space:  // stop 
		    Shared->speed = 0.0f;
		    break;
		case XK_r:
		case XK_R:
		    Shared->reset = 1;
		    break;
		case XK_F1:
		    Shared->drawStats = !Shared->drawStats;
		    break;
		    // F2-key toggles separate channel for stats display 
		case XK_F2:
		    Shared->haveStatsChan = !Shared->haveStatsChan;
		    break;
		case XK_Print:
		    Shared->snapImage = 1;
		    break;
		case XK_Up:
		    Shared->statsHistFrames++;
		    break;
		case XK_Down:
		    Shared->statsHistFrames--;
		    break;
		case XK_p: // print currently enabled statistics to stderr 
		    Shared->printStats = 1;
		    break;
		case XK_a: // toggle entire statistics class enable 
		case XK_A:
		    Shared->statsEnable = PFSTATS_ALL;
		    break;
		case XK_s: // toggle system CPU statistics class enable 
		case XK_S:
		    Shared->statsEnable = PFSTATSHW_ENCPU;
		    break;
		case XK_d: // toggle database statistics class enable 
		case XK_D:
		    Shared->statsEnable = PFFSTATS_ENDB;
		    break;
		case XK_c: // toggle cull statistics class enable 
		case XK_C:
		    Shared->statsEnable = PFFSTATS_ENCULL;
		    break;
		case XK_f: // toggle fill stats - depth complexity 
		case XK_F:
		    Shared->statsEnable = PFSTATSHW_ENGFXPIPE_FILL;
		    break;
		case XK_g:
		    Shared->statsEnable =  PFSTATS_ENGFX;
		    break;
		case XK_G: // toggle geometry attribute statistics 
		    // (normals, colors, texcoords)
		    
		    Shared->statsClass = PFSTATS_GFX;
		    Shared->statsClassMode = PFSTATS_GFX_ATTR_COUNTS;
		    break;
		case XK_h: // toggle proc frame time history stats 
		case XK_H:
		    Shared->statsClass = PFFSTATS_PFTIMES;
		    Shared->statsClassMode = PFFSTATS_PFTIMES_HIST;
		    break;
		case XK_n: // raise nap values 
		    Shared->Sleep = 1;
		    Shared->SleepApp += 1.0f;
		    Shared->SleepIsect = Shared->SleepApp;
		    Shared->SleepCull = Shared->SleepApp*3.0f;
		    Shared->SleepDraw = Shared->SleepApp;
		    break;
		case XK_N:
		    Shared->Sleep = 1;
		    Shared->SleepApp -= 1.0f; // lower nap values 
		    Shared->SleepIsect = Shared->SleepApp;
		    Shared->SleepCull = Shared->SleepApp*3.0f;
		    Shared->SleepDraw = Shared->SleepApp;
		    break;
		case XK_Pause: // toggle naps 
		    Shared->Sleep = 0;
		    if (!Shared->Sleep)
			break;
		case XK_m: // toggle mp process frame time stats 
		case XK_M:
		    Shared->statsEnable = PFFSTATS_ENPFTIMES;
		    break;
		case XK_q:
		case XK_Q:
		    Shared->doStatsQuery = 1;
		    break;
		case XK_t: // toggle tmesh lengths graph 
		    Shared->statsClass = PFSTATS_GFX;
		    Shared->statsClassMode = PFSTATS_GFX_TSTRIP_LENGTHS;
		    break;
		case XK_T: // toggle transparency fill stats 
		    Shared->statsClass = PFSTATSHW_GFXPIPE_FILL;
		    Shared->statsClassMode = PFSTATSHW_GFXPIPE_FILL_TRANSPARENT;
		    break;
		case XK_v: // toggle database visiblity stats 
		case XK_V:
		    Shared->statsClass = PFFSTATS_DB;
		    Shared->statsClassMode = PFFSTATS_DB_VIS;
		    break;
		case XK_z: // toggle depth-complexity z-compare stats 
		case XK_Z:
		    Shared->statsClass = PFSTATSHW_GFXPIPE_FILL;
		    Shared->statsClassMode = PFSTATSHW_GFXPIPE_FILL_DEPTHCMP;
		    break;
		case XK_B:
		    Shared->doBottleneck = 1;
		  break;
		case XK_b:
		    Shared->statsEnable = PFSTATSHW_ENGFXPIPE_TIMES;
		    Shared->statsClass = PFSTATSHW_GFXPIPE_TIMES;
		    Shared->statsClassMode = 
		    PFSTATSHW_GFXPIPE_TIMES_AUTO_COLLECT |  //enable automatic timestamps
		    PFSTATSHW_GFXPIPE_TIMES_STAGE; //collect bottleneck times from the various pipeline stages
		  break;
		}
	    }
	}// switch 
    }
    XFlush(dsp);
    Shared->mouseX = x;
    Shared->mouseY = y;
}


// Stats channel Callbacks 

static void 
CullStatsChannel(pfChannel *, void *) 
{ 
    // empty
}

static pfVec4  statsbclr(0.3f, 0.3f, 0.3f, 1.0f);
static void
DrawStatsChannel (pfChannel *channel, void *)
{
    pfClear(PFCL_COLOR, &statsbclr);
    // draw Performer throughput statistics 
    Shared->frameStats->draw(channel);
}

