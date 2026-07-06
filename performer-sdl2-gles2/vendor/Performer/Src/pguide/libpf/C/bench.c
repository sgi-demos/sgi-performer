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
 * file: bench.c	
 * ---------------
 *
 *	Example for benchmarking channel process callbacks
 *
 * $Revision: 1.63 $ $Date: 2000/10/06 21:26:29 $
 *
 * Command-line options:
 *  -b	<frames>    : set value for BenchLoop - num frames to put in benchmark,
 *			used with runtime interactive benchmark mode 
 *  -B 	    	    : auto benchmark mode - totally not interactive
 *			will continuously benchmark sets of _frames_ frames
 *  -c		    : turn off clear
 *  -C		    : cull mode
 *  -e h,p,r	    : initial viewing angles
 *  -E		    : turn on earth-sky
 *  -F filePath	    : set pfFilePath()
 *  -i <etime>	    : bench interval in elapsed time
 *  -I <frames>	    : set frame-stats interval in frames
 *  -q <frames>     : goes with the auto bench mode - exit prog after num frames
 *  -l p1,p2,p3	    : CPU to processor mapping
 *  -L		    : lock down CPUs - requires root id
 *  -m		    : multiprocess configuration
 *  -n notify	    : set pfNotifyLevel
 *  -o	1/2	    : use GL objects for scene=1 or gsets=2
 *  -p x,y,z	    : initial position
 *  -P		    : set phase - default is PFPHASE_FLOAT
 *  -q	val	    : optimization of gstates
 *  -Q	count	    : quite after count frames
 *  -r		    : frame rate
 *  -s	scale	    : set the LOD scale
 *  -S		    : run app in single buffer
 *  -t		    : force default texture override
 *  -T		    : use TAG clear
 *  -z		    : set framestats mode on 
 *  -Z		    : don't free CPus
 *
 * Run-time controls:
 *       ESC-key: exits
 *    Left-mouse: advance
 *  Middle-mouse: stop
 *   Right-mouse: retreat
 	'g': toggle display of channel stats
 *	'o': use 1 GL object for entire scene
 *	'O': use GL objects for all gsets in scene
 *	ctrl-o: back to immediate mode drawing
 *	'p': toggle phase
 *	'r': reset position and stop
 *	' ': stop
 *	'b': put into non-interactive bench mode for BenchLoop frames
 *	's': put into single buffer for benchmark mode
 *		(will defeat pfPhase settting but can give more accurate
 *		 drawing benchmarks over multiple frames)
 *	'z': toggle into statistics-gathering mode
 *	'B', 'Z': put into statistics gathering and benchmark mode permanently.
 *	'w': change window resolution: toggles through startup, full screen,
 *		VGA, and 100x100
 */

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <X11/X.h>
#include <X11/keysym.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

/* Performer Callback functions */
static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeline(int pipe, uint stage);
static void ConfigCull(int pipe, uint stage);
static void OpenPipeWin(pfPipeWindow *pw);
static void SwapFunc(pfPipe *p, pfPipeWindow *pwin);


static void Usage(void);

/* stats utility functions */
static void GetXInput(void);
static void UpdateView(void);
static void  queryFrameStats(pfFrameStats *fstats);
static void  initFrameStats(void);
static void  calcFrameStats(void);
static void  dumpBenchStats(void);
static void  dumpFrameStats(void);
static void  makeGSetObjs(pfNode *node, int enable);

#define WIN_SELECT_INIT 0
#define WIN_SELECT_TINEY 1
#define WIN_SELECT_VGA 2
#define WIN_SELECT_HIRES 3
#define NUM_WIN_SELECTS 4

#define HISPEED		0.1f
#define LOSPEED		0.001f

static int AppCPU=-1, CullCPU=-1, DrawCPU=-1;

/*
 * structure that resides in shared memory so that the
 * application, cull, and draw processes can access it.
 */
typedef struct
{
    pfChannel	*channel;
    pfPipeWindow *pw;
    pfCoord	initView, view;
    pfList	*texList;
    pfScene	*scene;
    int		exitFlag;
    float       mouseX;
    float	mouseY;
    float       speed;
    float	sceneSize;
    int		phase;
    int		graph;
    int		quitCount;
    int		reset;
    int		benchMode;
    int		benchLoop;
    int		setBuffer;
    int		frameStatsMode;
    int		initFrameStats;
    int		winSelect;
    int		drawMode;
    int		winSizeX, winSizeY;
    unsigned int	buttons;
    int		inWindow;
} SharedData;

/*
 * structures for keeping timing data
 */

typedef struct FrameTimingData {
    double startTime;
    int	   startFrame, endFrame;
	/* counts of frames for each process */
    pfFStatsValProc counts, last, misses;
	/* buffers for holding timing data */
    float *app, *cull, *draw, *total;
	/* frame stamps for cull and draw */
    int	 *appFrames, *cullFrames, *drawFrames, *totalFrames;
	/* calculated statistics */
    pfFStatsValProc cum, avg, min, minFrames, max, maxFrames, variance, stdDev;
    pfFStatsValPFTimesDraw drawhist[PFFSTATS_MAX_HIST_FRAMES];
}FrameTimingData;

static SharedData   *Shared;
static int  	    RestrictProcessors = 0, FreeProcessors = 1;
static int  	    ProcSplit = PFMP_APP_CULL_DRAW;

static int  	    Clear = 1;
static int  	    ESky = 0;
static int   	    SingleBuffer = 0;
static int  	    BenchLoop = 100;
static float	    FrameRate = 60.0f;
static int  	    InitXYZ=0, InitHPR=0;
static unsigned int	    WinType = PFPWIN_TYPE_X;
static FrameTimingData   *FrameStats=NULL;
static int  	    FrameStatsMaxFrames = 500;
static int  	    FrameStatsETime = 5; /* seconds */
static int	    WinOriginX=0, WinOriginY=0, WinSizeX=500, WinSizeY=500;
static float	    LODscale = 1.0f;
static int	    DftTexOverride = 0;  
static int	    CullMode = 0x0; 
static int	    OptimizeGStates = 0x0;
/* light source created and updated in draw-process */
static pfLight *Sun;

static char progName[80];

/*
 *	main() -- program entry point. this procedure
 *	is executed in the application process.
 */

char OptionStr[] = "b:BcC:de:EF:i:I:l:Lm:n:o:p:P:q:Q:r:s:StTW:xzZ?";

static int  
docmdline(int argc, char *argv[])
{
    int		    opt;
extern char *optarg;
extern int optind;
    
    strcpy(progName, argv[0]);
    
    /* process command-line arguments */
    while ((opt = getopt(argc, argv, OptionStr)) != -1)
    {
	switch (opt)
	{
	case 'b': /* set benchloop */
	    Shared->benchLoop = BenchLoop = atoi(optarg);
	    break;
	case 'B': /* auto benchmark mode - totally not interactive */
	    Shared->benchMode = -1;
	    break;
	case 'c': /* turn off clear */
	    Clear ^= 1;
	    break;
	case 'C':
	    CullMode = atoi(optarg);
	    break;
	case 'e':
            if (sscanf(optarg, "%f,%f,%f",
                    &Shared->initView.hpr[PF_H],
                    &Shared->initView.hpr[PF_P],
                    &Shared->initView.hpr[PF_R]) == 3)
                InitHPR = TRUE;
            break;
	case 'E': /* turn on earth-sky */
	    ESky ^= 1;
	    break;
	case 'F':
	    pfFilePath(optarg);
	    break;
	case 'i': /* bench interval in elapsed time */
	    FrameStatsETime = atof(optarg);
	    break;
	case 'I': /* set bench interval in frames */
	    FrameStatsMaxFrames = atoi(optarg);
	    break;
	case 'l':
	{
	    int i1, i2, i3;
	    if (sscanf(optarg, "%d,%d,%d", &i1, &i2, &i3) == 3)
	    {
		AppCPU = i1;
		CullCPU = i2;
		DrawCPU = i3;
	    }
	}
	break;
	case 'L': /* lock down CPUs - requires root id */
	    RestrictProcessors = 1;
	    break;
	case 'm': /* multiprocess configuration */
	    ProcSplit = atoi(optarg);
	    break;
	case 'n':
	    pfNotifyLevel(atoi(optarg));
	    break;
	case 'o':
	    Shared->drawMode = atoi(optarg);
	    break;
	case 'p': /* initial position */
	    if (sscanf(optarg, "%f,%f,%f",
                    &Shared->initView.xyz[PF_X],
                    &Shared->initView.xyz[PF_Y],
                    &Shared->initView.xyz[PF_Z]) == 3)
                InitXYZ = TRUE;
	    break;
	case 'P': /* set phase - default is PFPHASE_FLOAT */
	    Shared->phase = atoi(optarg);
	    break;
	case 'q': /* gstate optimization */
	    OptimizeGStates = atoi(optarg);
	    break;
	case 'Q': /* goes with the auto bench mode - exit prog after num benchmarks */
	    Shared->quitCount = atoi(optarg);
	    break;
	case 'r':
	    FrameRate = atof(optarg);
	    break;
	case 's': /* run app in single buffer */
	    LODscale = atof(optarg);
	    break;
	case 'S': /* run app in single buffer */
	    SingleBuffer ^= 1;
	    break;
	case 't':
	    DftTexOverride ^= 1;
	    break;
	case 'T':
	    ESky = PFES_TAG; 
	    break;  
	case 'W': /* Set the window size */
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
	    break;
	case 'z': /* start up in frame-stats mode */
	    Shared->frameStatsMode = 1;
	    break;
	case 'Z': /* start up in frame-stats mode */
	    FreeProcessors ^= 1;
	    break;
	default:
	    Usage();
	}
    }
    return optind;
}

/*
 *	Usage() -- print usage advice and exit. This procedure
 *	is executed in the application process.
 */

static void
Usage (void)
{
    fprintf(stderr, "Usage: %s [-c] [-B benchLoop] [-e bool] [-q count] file.ext ...\n", progName);
    exit(1);
}


int 
main (int argc, char *argv[])
{
    int		    arg;
    int		    found;
    pfScene	   *scene;
    pfNode	   *root;
    pfChannel      *chan;
    pfPipe         *p;
    pfFrameStats   *fstats;
    pfEarthSky     *eSky;
    pfBox           bbox;
    float	    far = 10000.0f;
    double	    dbstart, dbend, start, end;

    pfNotifyLevel(PFNFY_NOTICE);
    pfInit();
    
    start = pfGetTime();

    /* allocate shared before fork()'ing parallel processes */
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    Shared->speed = 0.0f;
    Shared->phase = PFPHASE_FLOAT;
    Shared->mouseX = Shared->mouseY = -1;
    Shared->winSelect = WIN_SELECT_INIT;
    Shared->inWindow = 0;
    
    arg = docmdline(argc, argv);
    
    Shared->winSizeX = WinSizeX;
    Shared->winSizeY = WinSizeY;
    
    if (Shared->frameStatsMode || Shared->benchMode)
	Shared->initFrameStats = 1;
    
    /* force MP mode to time each process task individually */
    pfMultiprocess(ProcSplit);

    /* Load all loader DSO's before pfConfig() forks */
    for (found = arg; found < argc; found++)
	pfdInitConverter(argv[found]);

    /* initiate multi-processing mode set in pfMultiprocess call */
    pfConfig();
    
    /* Process and schedual control: requires ROOT id.
     *	Assign a non-degrading priority to all Performer processes.
     *	User processes spawned from this one will inherit the same priority.
     *	Additionally, all processes are  restricted and isolated on
     *	separate processors.
     */
    if (RestrictProcessors)
    {
	pfuPrioritizeProcs(TRUE);
	if (AppCPU > -1)
	    pfuLockDownProc(AppCPU);
	else
	    pfuLockDownApp();
    } 
    
    Shared->scene = scene = pfNewScene();

    /* specify directories where geometry and textures exist */
    if (pfGetFilePath() == NULL)
        pfFilePath(
                   "."
                   ":./data"
                   ":../data"
                   ":../../data"
                   ":/usr/share/Performer/data"
                   );

    /* load files named by command line arguments */
    dbstart = pfGetTime();
    for (found = 0; arg < argc; arg++)
    {
	if ((root = pfdLoadFile(argv[arg])) != NULL)
	{
	    pfAddChild(scene, root);
	    found++;
	}
    }
    if (found)
    {
	pfdMakeShared((pfNode *)scene);
	pfdMakeSharedScene(scene);
	Shared->texList = pfuMakeTexList((pfNode *)scene);
    }
    dbend = pfGetTime();

    p = pfGetPipe(0);
    pfPipeSwapFunc(p, SwapFunc);
    pfFrameRate(FrameRate);
    pfPhase(Shared->phase);

    
    Shared->pw = pfNewPWin(p);
    pfPWinName(Shared->pw, "OpenGL Performer");
    pfPWinType(Shared->pw, WinType);
    pfPWinOriginSize(Shared->pw, WinOriginX, WinOriginY, WinSizeX, WinSizeY);
    /* Open and configure the GL window. */
    pfPWinConfigFunc(Shared->pw, OpenPipeWin);
    pfConfigPWin(Shared->pw);
    pfStageConfigFunc(0, PFPROC_DRAW, OpenPipeline);
    pfConfigStage(0, PFPROC_DRAW);
    pfStageConfigFunc(0, PFPROC_CULL, ConfigCull);
    pfConfigStage(0, PFPROC_CULL);

    chan = Shared->channel = pfNewChan(p);
    pfChanTravFunc(chan, PFTRAV_CULL, CullChannel);
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);
    pfChanScene(chan, scene);
    
    /* enable only the most minimal stats - tracking of process frame times */
    fstats = pfGetChanFStats(chan);
    pfFStatsClass(fstats, PFFSTATS_ENPFTIMES, PFSTATS_SET);
    /*pfFStatsClassMode(fstats, PFFSTATS_PFTIMES, PFFSTATS_PFTIMES_BASIC, PFSTATS_SET);*/
    /* turn off accumulation and averaging of stats */
    pfFStatsAttr(fstats, PFFSTATS_UPDATE_FRAMES, 0.0f);

    /* Create an earth/sky model that draws sky/ground/horizon */
    eSky = pfNewESky();
    switch(ESky)
    {
	case 0:
	    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_FAST);
	    break;
	case 1:
	    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
	    break;
	case PFES_TAG:
	    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_TAG);
	    break;
    }
    pfESkyAttr(eSky, PFES_GRND_HT, -10.0f);
    pfESkyColor(eSky, PFES_CLEAR, .3f, .3f, .7f, 1.0f);
    pfChanESky(chan, eSky);
    
    pfChanTravMode(chan, PFTRAV_CULL, CullMode);
    pfChanLODAttr(chan, PFLOD_SCALE, LODscale);

    
    /* vertical FOV is matched to window aspect ratio */
    pfChanFOV(chan, 45.0f, -1.0f);

    if (found) 
    { /* Set initial view to be "in front" of scene */
	
	/* determine extent of scene's geometry */
	pfuTravCalcBBox(scene, &bbox);

	/* view point at center of bbox */
	pfAddVec3(Shared->view.xyz, bbox.min, bbox.max);
	pfScaleVec3(Shared->view.xyz, 0.5f, Shared->view.xyz);
	
	/* find max dimension */
	Shared->sceneSize = bbox.max[PF_X] - bbox.min[PF_X];
	Shared->sceneSize = PF_MAX2(Shared->sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	Shared->sceneSize = PF_MAX2(Shared->sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	
	far = PF_MAX2(far, Shared->sceneSize * 2.5f);
	pfChanNearFar(chan, 0.1f, far);
	
	/* offset so all is visible */
	if (!InitXYZ)
	{
	    Shared->view.xyz[PF_Y] -=      Shared->sceneSize;
	    Shared->view.xyz[PF_Z] += 0.25*Shared->sceneSize;
	} else
	    pfCopyVec3(Shared->view.xyz, Shared->initView.xyz);
	    
	if (!InitHPR)
	    pfSetVec3(Shared->view.hpr, 0.0f, 0.0f, 0.0f);
	else
	    pfCopyVec3(Shared->view.hpr,Shared->initView.hpr);
	    
    }  else
    {
	if (!InitXYZ)
	    pfSetVec3(Shared->view.xyz, 0.0f, 0.0f, 100.0f);
	PFSET_VEC3(bbox.min, -5000.0, -5000.0, -1000000.0);
	PFSET_VEC3(bbox.max, 5000.0, 5000.0, 10000000.0);
    }
    
    /* save initial viewpoint for reset */
    if (!InitXYZ)
	pfCopyVec3(Shared->initView.xyz, Shared->view.xyz);
    if (!InitHPR)
	pfCopyVec3(Shared->initView.hpr, Shared->view.hpr);

    pfChanView(chan, Shared->view.xyz, Shared->view.hpr);	
    
    end = pfGetTime();
    
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Database load time: %g secs.", 
		    dbend-dbstart);
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Application startup time: %g secs.", 
		    end-start);

    /* main simulation loop */
    while (!Shared->exitFlag)
    {
	float           cp;

	/* wait until next frame boundary */
	pfSync();
	
	/*** Critical Section ***/

	/* compute new view */
	UpdateView();
	
	/* Set view parameters. */
	pfChanView(chan, Shared->view.xyz, Shared->view.hpr);
	
	/* initiate next frame traversal using current state */
	pfFrame();
	
	/*** End Critical Section ***/
	
	if (Shared->initFrameStats == 1)
	    initFrameStats();

	/* query timing stats */
	if (Shared->benchMode || Shared->frameStatsMode)
	    queryFrameStats(fstats);
	
	if (!Shared->benchMode)
	{
	    static int	 prevGraph = 0;
	    if (Shared->graph) 
	    {
		/* if display stats - turn averaging back on */
		if (!prevGraph)
		    pfFStatsAttr(fstats, PFFSTATS_UPDATE_SECS, 2.0f);
		pfDrawChanStats(chan);
	    } 
	    else if (prevGraph)
		pfFStatsAttr(fstats, PFFSTATS_UPDATE_SECS, 0.0f);
	    prevGraph = Shared->graph;
	} 
	else if (Shared->benchMode)
	{
	    if ((Shared->benchLoop <= 0))
	    {
		if (!Shared->frameStatsMode)
		{
		    fprintf(stderr,"-------------\n");
		    dumpBenchStats();
		    initFrameStats();
		    fprintf(stderr,"-------------\n");
		}
		if ((Shared->benchMode < 0)) /* app is running in permanent benchmark mode */
		{
		    Shared->benchLoop = BenchLoop;
		    if (Shared->quitCount && --Shared->quitCount == 0)
			Shared->exitFlag = 1;
		}
		else 
		{
		    Shared->benchMode = Shared->benchLoop = 0;
		    Shared->setBuffer = 1;
		}
	    }
	}
    }

    if (FreeProcessors)
	pfuFreeCPUs();
    pfExit();
    return 0;
}

/*
 *	OpenPipeline() -- create a pipeline: setup the window system,
 *	the GL, and OpenGL Performer. this procedure is executed in
 *	the draw process (when there is a separate draw process).
 */

Display *Dsp=NULL;

static void
OpenPipeline(int pipe, uint stage)
{
    if (RestrictProcessors)
    {
	if (DrawCPU > -1)
	    pfuLockDownProc(DrawCPU);
	else	    
	    pfuLockDownDraw(pfGetPipe(pipe));
    }
}

static void
OpenPipeWin(pfPipeWindow *pw)
{
    int		xmax, ymax;
    Window	xwin;

    pfOpenPWin(pw);
    
    {
	Dsp = pfGetCurWSConnection();
	xwin = pfGetPWinWSWindow(pw);
	XSelectInput(Dsp, xwin, StructureNotifyMask | PointerMotionMask |
		    FocusChangeMask | ButtonPressMask | ButtonReleaseMask | 
		    KeyPressMask);
	XMapWindow(Dsp, xwin);
	XFlush(Dsp);
    }

   
    if (SingleBuffer)
	glDrawBuffer(GL_FRONT);

    if (DftTexOverride)
    {
	void *arena =  pfGetSharedArena();
	uint *image = pfMalloc(sizeof(int*), arena);
	pfTexture *tex = pfNewTex(arena);
	*image = 0xffff00ff;
	pfTexImage(tex, image, 4,  1,  1,  1);
	pfApplyTex(tex);
	pfApplyTEnv(pfNewTEnv(arena));
	pfOverride(PFSTATE_TEXTURE | PFSTATE_TEXENV, 1);
    }
    
    /* create a light source in the "south-west" (QIII) */
    Sun = pfNewLight(NULL);
    pfLightPos(Sun, -0.3f, -0.3f, 1.0f, 0.0f);
}

/*
 *	CullChannel() -- traverse the scene graph and generate a
 * 	display list for the draw process.  This procedure is 
 *	executed in the cull process.
 */


static void
ConfigCull(int pipe, uint stage)
{

    if (RestrictProcessors)
    {
	if (CullCPU > -1)
	    pfuLockDownProc(CullCPU);
	else
	    pfuLockDownCull(pfGetPipe(pipe));
    }
}

static void
CullChannel(pfChannel *channel, void *data)
{
    (channel, data); /* remove compiler warn about unused var */
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
static int first=1;
static int owinselect;

    (data); /* remove compiler warn about unused var */
    
    if (first)
    {
	pfuDownloadTexList(Shared->texList, PFUTEX_SHOW);
	owinselect = Shared->winSelect;
	first=0;
    }
    if (Shared->setBuffer == 1)
    {
	if (Shared->benchMode)
	{
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Single-Buffered benchmark");
	    glDrawBuffer(GL_FRONT);
	} 
	else
	{
	    glDrawBuffer(GL_BACK);
	}
	Shared->setBuffer = 0;
   
    }
    /* rebind light so it stays fixed in position */
    pfLightOn(Sun);

    /* erase framebuffer and draw Earth-Sky model */
    if (Clear)
	pfClearChan(channel);

    {
	static have_obj = 0;
	switch (Shared->drawMode)
	{
	case -1: 
	    glCallList(1);
	    break;
	case 1:
	    {
		fprintf(stderr, "Making 1 GL object\n");
		glNewList(1, GL_COMPILE_AND_EXECUTE);
		pfDraw();
		glEndList();
		Shared->drawMode = -1;
		have_obj = 1;
	    }
	    break;
	case 2:
	    fprintf(stderr, "Making GSet objects\n");
	    makeGSetObjs((pfNode*)Shared->scene, 1);
	    Shared->drawMode = -2;
	    break;
	default:
	    /* invoke Performer draw-processing for this frame */
	    pfDraw();
	    break;
	}
    }
    
    
    /* examine the event queue for keyboard events */
    if (!Shared->benchMode)
    {
	/* read window origin and size (it may have changed) */
	pfGetPWinSize(Shared->pw, &Shared->winSizeX, &Shared->winSizeY);
	    GetXInput();

	if (Shared->winSelect != owinselect)
	{
	    int xo, yo;
	    owinselect = Shared->winSelect;
	    switch(owinselect)
	    {
		case WIN_SELECT_INIT:
		    xo = WinOriginX;
		    yo = WinOriginY;
		    Shared->winSizeX = WinSizeX;
		    Shared->winSizeY = WinSizeY;
		    break;
		case WIN_SELECT_TINEY:
		    xo = yo = 0;
		    Shared->winSizeX = Shared->winSizeY = 100;
		    break;
		case WIN_SELECT_VGA:
		    xo = yo = 0;
		    Shared->winSizeX = 640;
		    Shared->winSizeY = 480;
		    break;
		case WIN_SELECT_HIRES:
		    xo = yo = 0;
		    Shared->winSizeX = 1280;
		    Shared->winSizeY = 1024;
		    break;
	    }
	    pfPWinOriginSize(Shared->pw, xo, yo, Shared->winSizeX, Shared->winSizeY); 
	}
    } /* if !(Shared->benchMode) */
    else
	Shared->benchLoop--;
    if (Shared->benchMode >= 0)
    { /* if not in auto-bench mode, count draw frames */
	
	if (Shared->quitCount && --Shared->quitCount == 0)
		Shared->exitFlag = 1;
    }
}

static void
SwapFunc(pfPipe *p, pfPipeWindow *pwin)
{
    if (Shared->benchMode)
	return;
    pfSwapPWinBuffers(pwin);
}

static void
UpdateView(void)
{
    pfCoord *view = &Shared->view;
    float mx, my; 
    float speed;
    float cp;
    
    /* update cursor virtual position when cursor inside window */
    if (!Shared->inWindow || Shared->reset)
    {
	Shared->speed = 0;
	if (Shared->reset)
	{
	    pfCopyVec3(Shared->view.xyz, Shared->initView.xyz);
	    pfCopyVec3(Shared->view.hpr,Shared->initView.hpr);
	    Shared->reset = 0;
	}
	return;
    }
    
    switch (Shared->buttons)
    {
    case Button1Mask: /* LEFTMOUSE: faster forward or slower backward*/
	if (Shared->speed > 0.0f)
	    Shared->speed *= 1.2f;
	else
	    Shared->speed /= 1.2f;
	
	if (PF_ABSLT(Shared->speed, LOSPEED * Shared->sceneSize))
	    Shared->speed = LOSPEED * Shared->sceneSize;
	else if (Shared->speed >=  HISPEED * Shared->sceneSize)
	    Shared->speed = HISPEED * Shared->sceneSize;
	break;
    case Button2Mask:	/* MIDDLEMOUSE: stop moving */
	Shared->speed = 0.0f;
	break;
    case Button3Mask: /* RIGHTMOUSE: faster backward or slower foreward*/
	if (Shared->speed < 0.0f)
	    Shared->speed *= 1.2f;
	else
	    Shared->speed /= 1.2f;
	
	if (PF_ABSLT(Shared->speed, LOSPEED * Shared->sceneSize))
	    Shared->speed = -LOSPEED * Shared->sceneSize;
	else if (Shared->speed <=  -HISPEED * Shared->sceneSize)
	    Shared->speed = -HISPEED * Shared->sceneSize;
	break;
    }
    mx = 2.0f * (Shared->mouseX / (float)Shared->winSizeX) - 1.0f;
    my = 2.0f * (Shared->mouseY / (float)Shared->winSizeY) - 1.0f;
	
    /* update view direction */
    view->hpr[PF_H] -= mx * PF_ABS(mx);
    view->hpr[PF_P]  = my * PF_ABS(my) * 90.0f;
    view->hpr[PF_R]  = 0.0f;
    
    /* update view position */
    cp = cosf(PF_DEG2RAD(view->hpr[PF_P]));
    speed = Shared->speed;
    view->xyz[PF_X] += speed*sinf(-PF_DEG2RAD(view->hpr[PF_H]))*cp;
    view->xyz[PF_Y] += speed*cosf(-PF_DEG2RAD(view->hpr[PF_H]))*cp;
    view->xyz[PF_Z] += speed*sinf( PF_DEG2RAD(view->hpr[PF_P]));

}

/*
 * Event handling
 */



static void
GetXInput(void)
{
    static int x=0, y=0;
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
	    case MotionNotify: 
	    {
		XMotionEvent *motion_event = (XMotionEvent *) &event;
		x =  motion_event->x;
		y = Shared->winSizeY - motion_event->y;
		haveMotion = 1;
		Shared->inWindow = 1;
	    }
	    case FocusIn:
                Shared->inWindow = 1;
                break;
           case FocusOut:
                Shared->inWindow = 0;
		break;
	    case ButtonPress: 
	    {
		XButtonEvent *button_event = (XButtonEvent *) &event;
		x = event.xbutton.x;
		y = Shared->winSizeY - event.xbutton.y;
		switch (button_event->button) {
		    case Button1:
			Shared->buttons |= Button1Mask;
			break;
		    case Button2:
			Shared->buttons |= Button2Mask;
			break;
		    case Button3:
			Shared->buttons |= Button3Mask;
			break;
		}
	    }
	    break;
	    case ButtonRelease:
	    {
		XButtonEvent *button_event = (XButtonEvent *) &event;
		x = event.xbutton.x_root;
		y = Shared->winSizeY - event.xbutton.y;
		switch (button_event->button) {
		    case Button1:
			Shared->buttons &= ~Button1Mask;
			break;
		    case Button2:
			Shared->buttons &= ~Button2Mask;
			break;
		    case Button3:
			Shared->buttons &= ~Button3Mask;
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
		switch(ks) 
		{
		/* ESC-key signals end of simulation */
		case XK_Escape:
		    Shared->exitFlag = 1;
		    break;
		case XK_space:
		    Shared->speed = 0.0f;
		    break;
		case XK_F1:
		    Shared->speed = 0.0f;
		    Shared->reset = 1;
		    break;
		case XK_F2:
		    Shared->graph ^= 1;
		    break;
		case XK_F12:
		    fprintf(stderr, "Viewpoint: %f,%f,%f  HPR: %f,%f,%f\n", 
		    Shared->view.xyz[PF_X], Shared->view.xyz[PF_Y], Shared->view.xyz[PF_Z],
		    Shared->view.hpr[PF_H], Shared->view.hpr[PF_P], Shared->view.hpr[PF_R]);
		break;
		case XK_p:		/* change the phase locking method */
		    if (Shared->phase == PFPHASE_FREE_RUN) 
			Shared->phase = PFPHASE_FLOAT;
		    else if (Shared->phase == PFPHASE_FLOAT) 
			Shared->phase = PFPHASE_LOCK;
		    else if (Shared->phase == PFPHASE_LOCK) 
			Shared->phase = PFPHASE_FREE_RUN;
		    break; 
		case XK_b:		/* put into bench mode */
		    if (!Shared->benchMode)
		    {
			Shared->initFrameStats = 1;
			Shared->benchMode = 1;
			Shared->benchLoop = BenchLoop;
			if (Shared->setBuffer)
			    Shared->setBuffer = 1;
		    } 
		    else
		    {
			Shared->benchMode = 0;
			Shared->benchLoop = 0;
			Shared->setBuffer = 1;
		    }
		    break;
		case XK_g:
		    Shared->graph ^= 1;
		    break;
		case XK_o:		/* use GL object for entire scene */
		    Shared->drawMode = 1;
		    break;
		case XK_O:		/* use GL objects for gsets */
		    Shared->drawMode = 2;
		    break;
		case 15: /* XXXX ctrl-o - back toimm mode */
		    if (Shared->drawMode == -2)
			makeGSetObjs((pfNode*)Shared->scene, 0);
		    Shared->drawMode = 0;
		    fprintf(stderr, "Back to immediate mode.\n");
		    break;
		case XK_r:
		    Shared->speed = 0.0f;
		    Shared->reset = 1;
		    break;
		case XK_s:
		    Shared->setBuffer = 2;
		    break;
		case XK_w:
		    Shared->winSelect = (Shared->winSelect + 1) % NUM_WIN_SELECTS;
		    break;
		case XK_W:
		    Shared->winSelect = (Shared->winSelect + (NUM_WIN_SELECTS - 1))
					     % NUM_WIN_SELECTS;
		    break;  
		case XK_z:		/* put into frame-stats mode */
		    Shared->frameStatsMode = !Shared->frameStatsMode;
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,"FrameStatsMode: %d",
					Shared->frameStatsMode);
		    Shared->initFrameStats = 1;
		    break;
		    
		case XK_Z: /* put PERMANENTLY into frame-stats bench mode */
		    Shared->frameStatsMode = 1;
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,"FrameStatsMode: %d",
					Shared->frameStatsMode);
		    /* No break here on purpose !!! */
		case XK_B:	
		    Shared->initFrameStats = 1;
		    Shared->benchLoop = BenchLoop;
		    Shared->benchMode = -1;
		    if (Shared->setBuffer)
			Shared->setBuffer = 1;
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
			"Now in permanent non-interactive benchmark mode.");			    
		    break;
		}    
	    }
	} /* switch event */
    }
    Shared->mouseX = x;
    Shared->mouseY = y;
}

/*
 *  Timing and statistics utilities
 */

static void 
initFrameStats(void)
{
static pfFStatsValProc zero_pfFStatsValProc = {0.0f, 0.0f, 0.0f, 0.0f};
static pfFStatsValProc neg_pfFStatsValProc = {-1.0f, -1.0f, -1.0f, -1.0f};
    
    Shared->initFrameStats = 0;
    if (!FrameStats)
    {
	FrameStats = (FrameTimingData *) 
	    pfCalloc(1, sizeof(FrameTimingData), pfGetSharedArena());
	if (FrameStats)
	{
	    FrameStats->app = (float *) 
		pfCalloc(1, sizeof(float)*FrameStatsMaxFrames*2, pfGetSharedArena());
	    FrameStats->cull = (float *) 
		pfCalloc(1, sizeof(float)*FrameStatsMaxFrames*2, pfGetSharedArena());
	    FrameStats->draw = (float *) 
		pfCalloc(1,sizeof(float)*FrameStatsMaxFrames*2, pfGetSharedArena());
	    FrameStats->appFrames = (int *) 
		pfCalloc(1, sizeof(float)*FrameStatsMaxFrames*2, pfGetSharedArena());
	    FrameStats->cullFrames = (int *) 
		pfCalloc(1, sizeof(float)*FrameStatsMaxFrames*2, pfGetSharedArena());
	    FrameStats->drawFrames = (int *) 
		pfCalloc(1, sizeof(float)*FrameStatsMaxFrames*2, pfGetSharedArena());
	    FrameStats->totalFrames = (int *) 
		pfCalloc(1, sizeof(float)*FrameStatsMaxFrames*2, pfGetSharedArena());
	    FrameStats->total = (float *) 
		pfCalloc(1, sizeof(float)*FrameStatsMaxFrames*2, pfGetSharedArena());
	}
	if (!FrameStats || !FrameStats->app || !FrameStats->cull
		    || !FrameStats->draw || !FrameStats->total)
	{
	    if (FreeProcessors)
		pfuFreeCPUs();
	    pfNotify(PFNFY_FATAL,  PFNFY_RESOURCE, 
		"initFrameStats: failed to alloc FrameStats!");
	}
    }
    FrameStats->counts = zero_pfFStatsValProc;
    FrameStats->misses = zero_pfFStatsValProc;
    FrameStats->cum = zero_pfFStatsValProc;
    FrameStats->avg = zero_pfFStatsValProc;
    FrameStats->max = neg_pfFStatsValProc;
    FrameStats->min = neg_pfFStatsValProc;
    FrameStats->startFrame = pfGetFrameCount();
    FrameStats->endFrame = 0;
}

typedef struct StatsResult {
/* CAREFUL - doubles must be double word alinged so put
 * types that have doubles in them first or you will see
 * garbage numbers come back!
 */
    double start;
    pfFStatsValPFTimesDraw drawhist[PFFSTATS_MAX_HIST_FRAMES];
    pfFStatsValProc appstamp;
    pfFStatsValProc time;
    pfFStatsValProc misses;
} StatsResult;

static void queryFrameStats(pfFrameStats *fstats)
{
/* 
 * query a pfFrameStats structure for process frame times
 *  - call from application process
 */
static unsigned int StatsQuery[] = {
    PFFSTATS_BUF_PREV | PFFSTATSVAL_PFTIMES_START, /* double */
    PFFSTATS_BUF_PREV | PFFSTATSVAL_PFTIMES_HIST_ALL_DRAW, /* pfFStatsValPFTimesDraw */
    PFFSTATS_BUF_PREV | PFFSTATSVAL_PFTIMES_APPSTAMP, /* pfFStatsValProc */
    PFFSTATS_BUF_PREV | PFFSTATSVAL_PFTIMES_PROC,  /* pfFStatsValProc */
    PFFSTATS_BUF_PREV | PFFSTATSVAL_PFTIMES_MISSES, /* pfFStatsValProc */
    NULL
};
static StatsResult dst;

    int i;
    
    if (!FrameStats)
	initFrameStats();
	
    /* get the prev frame times and corresponding app frame stamps */
    pfMQueryFStats(fstats, StatsQuery, &dst, 0);
    
    /* record new frame data */
    if (Shared->frameStatsMode)
    {    
    
	if ((FrameStats->counts.total > 0.0f) && 
	    (((FrameStats->counts.app >= FrameStatsMaxFrames) ||
	    ((pfGetTime() - FrameStats->startTime) >= FrameStatsETime))))
	{
	    FrameStats->endFrame = dst.appstamp.app;
	    fprintf(stderr,"-------------\n");
	    calcFrameStats();
	    dumpFrameStats();
	    fprintf(stderr,"-------------\n");
	    initFrameStats();
	    FrameStats->startTime = dst.start;
	} 
	else 
	{
	    i = (int)FrameStats->counts.app;
	    FrameStats->app[i] = dst.time.app * 1000.0f;
	    FrameStats->appFrames[i] =  dst.appstamp.app;
	    FrameStats->counts.app++;
	    if (FrameStats->last.cull != dst.appstamp.cull)
	    {
		FrameStats->cull[(int)FrameStats->counts.cull] = 
			dst.time.cull * 1000.0f;
		FrameStats->cullFrames[(int)FrameStats->counts.cull] = dst.appstamp.cull;
		FrameStats->counts.cull++;
	    }
	    if (FrameStats->last.draw != dst.appstamp.draw)
	    {
		FrameStats->draw[(int)FrameStats->counts.draw] = 
				dst.time.draw * 1000.0f;
		FrameStats->drawFrames[(int)FrameStats->counts.draw] = dst.appstamp.draw;
		FrameStats->total[(int)FrameStats->counts.total] = 
				dst.time.total * 1000.0f;
		FrameStats->totalFrames[(int)FrameStats->counts.total] = dst.appstamp.total;
		FrameStats->counts.draw++;
		FrameStats->counts.total++;
	    }
	    FrameStats->misses.cull += dst.misses.cull;
	    FrameStats->misses.draw += dst.misses.draw;
	    FrameStats->misses.total += dst.misses.total;
	    for (i=0; i < PFFSTATS_MAX_HIST_FRAMES; i++)
		FrameStats->drawhist[i] = dst.drawhist[i];
	}
    }
    else if (Shared->benchMode)
    {
	FrameStats->cum.app += dst.time.app;
	FrameStats->counts.app++;
	if (FrameStats->last.cull != dst.appstamp.cull)
	{
	    FrameStats->cum.cull += dst.time.cull;
	    FrameStats->counts.cull++;
	}
	if (FrameStats->last.draw != dst.appstamp.draw)
	{
	    FrameStats->cum.draw += dst.time.draw;
	    FrameStats->cum.total += dst.time.total;
	    FrameStats->counts.draw++;
	    FrameStats->counts.total++;
	}
    }
    FrameStats->last = dst.appstamp;
}

static void
dumpBenchStats(void)
{
    if (!FrameStats)
	initFrameStats();
    fprintf(stderr, 
	"bench frame counts: app=%.0f, cull=%.0f, draw=%.0f, frame=%.0f \n",
	FrameStats->counts.app, FrameStats->counts.cull,
	FrameStats->counts.draw, FrameStats->counts.total);
    fprintf(stderr, 
	"\tbench msecs: app=%6.2f, cull=%6.2f, draw=%6.2f, frame=%6.2f \n",
	1000.0f * FrameStats->cum.app/FrameStats->counts.app,
	1000.0f * FrameStats->cum.cull/FrameStats->counts.cull,
	1000.0f * FrameStats->cum.draw/FrameStats->counts.draw,
	1000.0f * FrameStats->cum.total/FrameStats->counts.total);
}


static void 
dumpFrameStats(void)
{
    fprintf(stderr, "FrameStats - num app frames:%d[%d-%d] time: %f\n", 
	    (int)FrameStats->counts.total,
	    (int)FrameStats->startFrame,(int)FrameStats->endFrame, 
	    pfGetTime());
    fprintf(stderr, "\tNum Frames:\tapp=%d cull=%d draw=%d\n", 
	(int)FrameStats->counts.app, 
	(int)FrameStats->counts.cull, 
	(int)FrameStats->counts.draw);
    fprintf(stderr, "\tFrames Misses (%.0fHz):\tcull=%d draw=%d frame=%d\n", 
	pfGetFrameRate(), 
	(int)FrameStats->misses.cull, 
	(int)FrameStats->misses.draw, 
	(int)FrameStats->misses.total);
    fprintf(stderr, "\tAvg Times(msecs):    app=%3.1f cull=%3.1f draw=%3.1f frame=%3.1f\n", 
	FrameStats->avg.app, FrameStats->avg.cull, 
	FrameStats->avg.draw, FrameStats->avg.total); 
    fprintf(stderr, "\tMin Times:\t%5d:%3.1f %5d:%3.1f %5d:%3.1f %5d:%3.1f\n", 
	(int)FrameStats->minFrames.app, FrameStats->min.app, 
	(int)FrameStats->minFrames.cull, FrameStats->min.cull, 
	(int)FrameStats->minFrames.draw, FrameStats->min.draw, 
	(int)FrameStats->minFrames.total, FrameStats->min.total); 
    fprintf(stderr, "\tMax Times:\t%5d:%3.1f %5d:%3.1f %5d:%3.1f %5d:%3.1f\n", 
	(int)FrameStats->maxFrames.app, FrameStats->max.app, 
	(int)FrameStats->maxFrames.cull, FrameStats->max.cull, 
	(int)FrameStats->maxFrames.draw, FrameStats->max.draw, 
	(int)FrameStats->maxFrames.total, FrameStats->max.total); 
    fprintf(stderr, "\tMax-Min:\t%3.1f %3.1f %3.1f %3.1f\n", 
	FrameStats->max.app - FrameStats->min.app, 
	FrameStats->max.cull - FrameStats->min.cull, 
	FrameStats->max.draw - FrameStats->min.draw, 
	FrameStats->max.total - FrameStats->min.total);   	  
    fprintf(stderr, "\tStd Devation:\t"
	    "(%.3f,%.3f%%) (%.3f,%.3f%%) (%.3f,%.3f%%) (%.3f,%.3f%%)\n", 
	FrameStats->stdDev.app, 
	FrameStats->stdDev.app/FrameStats->avg.total,
	FrameStats->stdDev.cull, 
	FrameStats->stdDev.cull/FrameStats->avg.total,
	FrameStats->stdDev.draw, 
	FrameStats->stdDev.draw/FrameStats->avg.total,
	FrameStats->stdDev.total,
	FrameStats->stdDev.total/FrameStats->avg.total);
   fprintf(stderr, "\tFrame start: %.3g\n", FrameStats->startTime);
   fprintf(stderr, "\tDraw history: chan=%.3g-%.3g pfDraw=%.3g-%.3g swap=%.3g\n",
	FrameStats->drawhist[1].start, FrameStats->drawhist[1].end,
	FrameStats->drawhist[1].pfDrawStart, FrameStats->drawhist[1].pfDrawEnd,
	FrameStats->drawhist[1].afterSwap);
}


#define CACL_PROC_SD(sd, proc, s, e) {				    \
    float tsd, dev;						    \
    int t;							    \
    tsd = 0.0f;							    \
    for (t = s; t <= e; t++)					    \
    {								    \
	dev =  FrameStats->##proc[t] - FrameStats->avg.##proc;		    \
	tsd += (dev*dev);					    \
    }								    \
    tsd /= (float)((e-s)+1.0f);					    \
    sd = fsqrt(tsd);						    \
}

/* this uses ANSI parameter concatenation */
#define CALC_PROC_STATS(proc, skip) {				    \
    float sum, ftmp;						    \
    int i, total, num, end;					    \
    total = FrameStats->counts.##proc;				    \
    end = total-(skip);						    \
    num = end - skip;						    \
    sum = 0.0f;							    \
    for (i=skip; i < end ; i++)					    \
    {								    \
	ftmp = FrameStats->##proc[i];				    \
	sum += ftmp;						    \
	if ((FrameStats->min.##proc > ftmp) || (FrameStats->min.##proc < 0)) \
	{							    \
	    FrameStats->min.##proc = ftmp;			    \
	    FrameStats->minFrames.##proc = FrameStats->##proc##Frames[i]; \
	}							    \
	if (FrameStats->max.##proc < ftmp)			    \
	{							    \
	    FrameStats->max.##proc = ftmp;			    \
	    FrameStats->maxFrames.##proc = FrameStats->##proc##Frames[i]; \
	}							    \
    }								    \
    /* toss out min and max */					    \
    FrameStats->avg.##proc =					    \
	    (sum - (FrameStats->min.##proc + FrameStats->max.##proc)) \
		/ (float)(num - 2);				    \
    CACL_PROC_SD(FrameStats->stdDev.##proc, proc, skip, num);	    \
}

static void 
calcFrameStats(void)
{
    int last;
    /* skip the first & last 4 30 hz frames */
    last = PF_MAX2((int) (4*(30.0f/FrameRate)), 4);
    CALC_PROC_STATS(app, last);
    CALC_PROC_STATS(cull, last);
    CALC_PROC_STATS(draw, last);
    CALC_PROC_STATS(total, last);
}


static void
makeGSetObjs(pfNode *node, int enable)
{
    int	i, n;

    if(pfIsOfType(node, pfGetGeodeClassType()))
    {
	n = pfGetNumGSets(node);

	for(i=0; i<n; i++)
	    pfGSetDrawMode(pfGetGSet(node, i), PFGS_COMPILE_GL, enable);
    }
    else if(pfIsOfType(node, pfGetGroupClassType()))
    {
	n = pfGetNumChildren(node);

	for(i=0; i<n; i++)
	    makeGSetObjs(pfGetChild(node, i), enable);
    }
}


/* handy stubbs for stubbing out gfx calls */
#if 0
static float junk;
void bgntmesh(void) {}
void endtmesh(void) {}
void shademodel(int p) {}
void texdef2d(int t, int p, int w, int h,
		const unsigned int *i, int np, const float *pr) {}
void texbind(int t, int p) {}
void tevdef(int t, int p, const float *pr) {}
void tevbind(int t, int p) {}
void c3f(const float p[3]) {junk = p[0]; }
void c4f(const float p[4]) {junk = p[0];}
void n3f(const float p[3]) {junk = p[0];}
void t2f(const float p[2]) {junk = p[0];}
void v3f(const float p[3]) {junk = p[0];}
void clear(void) {}
void zclear(void) {}
void lmcolor(int m) {}
#endif


