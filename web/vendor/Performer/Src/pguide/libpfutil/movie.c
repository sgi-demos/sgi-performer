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
 * file: movie.c
 * -------------
 * 
 * $Revision: 1.41 $ 
 *
 * This is an example of using functionalities of libpr, libpf,  and utilities
 *  in libpfutil to achieve animated texture on scene objects.
 *  This program allows you to have multiple movie screens and movies, 
 *  and provides a GUI utility (using libpfutil GLX GUI and input handling
 *  utilities) to allow you to select screens and movies and control the
 *  animation.
 *
 * This example creates a projector for each movie that is loaded.
 * Polygons in the scene are screens - each using a pfTexture that plays
 * a movie by being added to the screen list of a given movie projector.
 * Note that the movie in a projector can also be changed at any time with
 * pfuProjectorMovie(),  however, this is a slightly more expensive operation than 
 * just adding or deleting screens.  When a movie is changed, all screens for that 
 * projector will then start showing the new movie.
 *
 * Command Line Options: (getopt() is used to parse command line)
 * ---------------------
 * -c num pipe channels - use with multipipe to force multiple pipes on a 
 *			single pipe machine.
 * -p procSplit	    - performer multiprocess mode
 * -m "moviename namefmt start end" - a new movie
 *			frames of the movie in the range [start,end] will
 *			be loaded into a movie.  Use a separate -m for each 
 *			movie to be loaded. basetex will be used when the
 *			movie is applied to the screen,  but not running.
 *                      Each frame of the movie should be in a separate 
 *                      image file.  namefmt provides the printf format 
 *			to generate the file names from the frame numbers,
 *			e.g. mymovie%02d.rgb.  start and end are the starting 
 *			and ending frame numbers, respectively. 
 *
 *			movieconvert(1) in dmedia_tools mcan be useful
 *			for generating series of images.  
 *			
 * -M <mode>	    - Multipipe mode\n"
 *           0 -> single pipe mode\n"
 *           1 -> multipipe mode\n"
 *           2 -> hyperpipe mode\n" 
 * -n notifyLevel   - controls verobsity of program (defualt=5=DEBUG)
 * -N		    - set non-degrading priority
 * -R		    - restrict processors - requires root ID
 * -s numScreens    - set the number of movie screens in the scene (default=2)
 * -t texfile	    - set a base texture on the screens for when they are
 *			not playing a movie
 *
 *
 * Run-time controls:
 * ------------------
 *       ESC-key: exits
 */

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h>

/* X Window includes */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif // WIN32

#include <Performer/pf.h>
#include <Performer/pfutil.h>


/***************************** Defines and Enums *****************************/

#define MAX_MOVIES 16
#define MAX_SCREENS 16
#define MAX_TEXTURES	256

/* Panel Widget Control Tokens */
enum PanelControlToken {
    CTL_QUIT=1, 
    CTL_SCREEN_SELECT, 
    CTL_MOVIE_SELECT, 
    CTL_ADD_SCREEN_TO_PROJECTOR, 
    CTL_RESET, 
    CTL_START, 
    CTL_STOP, 
    CTL_PAUSE, 
    CTL_RESUME, 
    CTL_STEP, 
    CTL_REVERSE, 
    CTL_SPIN, 
    CTL_SPEED, 
    CTL_STATS
};

#define HYPERPIPE 2

/******************************* Typedefs  ***********************************/

typedef struct MovieData
{
    pfSequence	*proj;	/* the "projector" */
    int	length;
    char	name[PF_MAXSTRING];
    pfTexture	*tex;  /* the handle texture for the movie */
    pfList	*tape; /* the tape of movie frames */
} MovieData;

typedef struct ScreenData
{
    int	movieIndex;
    pfTexture	*tex;
}ScreenData;

typedef struct SharedData
{
    int			exitFlag;
    int			movieFlag;
    pfuEventStream	events;		/* event structure */
    pfuPanel		*guiPanel;
    int			spin;
    int			showStats;
    
    pfGroup		*movieRoot;
    int			screenSel;
    int			movieSel;
    int			start, end, direction;
    float		speed;
    pfList		*texList;
    pfTexture		*dftTex; /* for screen with no movie */
    ScreenData		movieScreen[MAX_SCREENS];
    MovieData		movie[MAX_MOVIES];
    pfuMouse		mouse;
} SharedData;

/****************************** Global Vars **********************************/

/* data for geometry */

pfVec2          texcoords[]={   {0.0f, 0.0f},
                                {1.0f, 0.0f},
                                {1.0f, 1.0f},
                                {0.0f, 1.0f} };

pfVec3          coords[] ={     {-1.0f,  0.0f,  -1.0f },
                                { 1.0f,  0.0f,  -1.0f },
                                { 1.0f,  0.0f,   1.0f },
                                {-1.0f,  0.0f,   1.0f } };

pfVec3          norms[] ={      { 0.0f,  -1.0f,   0.0f },
                                { 0.0f,  -1.0f,   0.0f },
                                { 0.0f,  -1.0f,   0.0f },
                                { 0.0f,  -1.0f,   0.0f } };
pfVec4		colors[] ={	{1.0f, 1.0f, 1.0f, 1.0f} };

/* initial window position */
static int WinXOrigin = 20, WinYOrigin = 20;
static int WinXSize = 800, WinYSize = 700;

/* global name strings */
static char 	    Name[PF_MAXSTRING] = "OpenGL Performer";
static char 	    ScreenStrs[MAX_SCREENS][PF_MAXSTRING];
/* these are used for holding movie info off the cmdline */
static char	    MovieName[MAX_MOVIES][PF_MAXSTRING] = {"None"};
static char	    MovieFmt[MAX_MOVIES][PF_MAXSTRING];
static char	    TexName[PF_MAXSTRING];

/* shared memory handle */
static SharedData *Shared=0;

static int	    HaveBaseTexture = 0;
static int	    HaveTextureList = 0;
static char	    TexFileNames[MAX_TEXTURES][PF_MAXSTRING];
static int	    NumMovies=0;
static int	    NumTextures = 0;
static int	    MovieStart[MAX_MOVIES], MovieEnd[MAX_MOVIES];

static int	    NumScreens=2;
/* DCSs for spinning screens */
static pfDCS	    *ScreenDCS[MAX_SCREENS];

/* Performer control vars */
int		    NotifyLevel = PFNFY_DEBUG;
unsigned int	    ProcSplit = PFMP_DEFAULT;
int		    RestrictCPUs = 0;
int		    PrioritizeProcs = 0;
int		    Multipipe = 0;
int		    NumPipes = -1;
int		    FastDefine = 0;
int		    ReplaceMode = 0;

/***************************** Static Decls **********************************/

/* GUI stuff */
static pfuPanel	    *InitPanel(void);
static void	    ProcessInputIO(void);
static void	    addScreenToProjector(int m, int s);
static void	    PanelControl(pfuWidget *w);

static void	    OpenPipeWin(pfPipeWindow *pw);
static void	    OpenPipeline (int pipe, uint stage);
static void	    DrawChannel(pfChannel *chan, void *data);
static void	    CullChannel(pfChannel *chan, void *data);
static pfNode	    *MakeScreens(void);
static void	    resetProjector(int _movie);

/********************************* Main **************************************/


static void
Usage (void)
{
    fprintf(stderr, "Usage: movie [-m movie,start,end] [texfile ...]\n");
    pfExit();
    exit(1);
}

static char OptionStr[] = "c:Fm:M:n:Np:rRs:t:?";

static int
docmdline(int argc, char **argv)
{
     extern char *optarg;
     extern int  optind;
     int opt, i;
        
     while ((opt = getopt(argc,argv,OptionStr)) != -1) {
	 switch(opt) {
	 /* custom options */

	 case 'c':
	    NumPipes = atoi(optarg);
	    break;
	 
	 case 'F':
	    FastDefine ^= 1;
	    break;
	    
	 case 'm': 
	    if (NumMovies < MAX_MOVIES)
	    {
		NumMovies += 1;
		sscanf(optarg, "%s %s %d %d",
			&(MovieName[NumMovies]), 
			&(MovieFmt[NumMovies]),
			&(MovieStart[NumMovies]), &(MovieEnd[NumMovies]));
	    }
	    break;
	 
	    /* Enable multipipe mode */
	case 'M':
	    Multipipe = atoi(optarg);
	    break;
	
	case 'n':
	    NotifyLevel =  atoi(optarg);
	    break;
	    
	case 'N':
	    PrioritizeProcs ^= 1;
	    break;
	    
	case 'p':
	    ProcSplit = atoi(optarg);
	    break;
	
	case 'r':
	    ReplaceMode ^= 1;
	    break;
	  
	case 'R':
	    RestrictCPUs ^= 1;
	    break;
	    
	case 's':
	    NumScreens = atoi(optarg);
	    if (NumScreens > MAX_SCREENS)
		NumScreens = MAX_SCREENS;
	    break;
	  
	 case 't':
	    HaveBaseTexture = 1;
	    strcpy(TexName,optarg);
	    break;

	 case '?':
	 default:
	    Usage();
	    exit(0);
	    break;
	} /* switch */
    } /* while */
    
    if (!NumMovies && (optind < argc))
    {
	NumTextures = (argc - optind);
	NumMovies = 1;
	HaveTextureList = 1;
	for (i=0; i < NumTextures && i < MAX_TEXTURES; i++)
	{
	    strcpy(TexFileNames[i], (argv[optind + i]));
	}
	TexFileNames[i][0] = '\0';
	strcpy(MovieName[1], TexFileNames[0]);

	return NumTextures;
    } else
	return 0;
}


int
main (int argc, char *argv[])
{
    pfScene     *scene;
    pfChannel   *chan;
    pfSphere	bsphere;
    pfNode	*root;
    pfEarthSky  *esky;
    pfTexture	*tex;
    pfPipe	*pipe;
    pfPipeWindow	*pipewin;
    int		guiDirty;
    
    float       t = 0.0f;
    int	i, j;
    
    pfNotifyLevel(NotifyLevel);
    
    /* Initialize Performer */
    pfInit();	

    /* init shared mem for libpfutil */
    pfuInit();
  		
    /* Append to PFPATH standard data dirs */
    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures");
    
    /* cmdline options */
    docmdline(argc, argv);
    for (i=0; i < NumScreens; i++)
	sprintf(ScreenStrs[i], "Screen%d", i);

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess(ProcSplit);	
    
    /* Figure out the number of channels and pipelines required */
    switch (Multipipe) {
    case TRUE:
	if (NumPipes == (-1))
#ifndef WIN32
	    NumPipes = ScreenCount(pfGetCurWSConnection());
#else
	    NumPipes = 1;
#endif
	break;
    case FALSE:
	NumPipes = 1;
	break;
#ifdef IRISGL
    case HYPERPIPE:
	/* MUXPIPES not implemented for RealityEngine */
	if (0 && !getgdesc(GD_MUXPIPES))
	{
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
			"This machine not is not capable of Hyperpipe mode");

	    Multipipe = TRUE;
	}
	else
	{
	    NumPipes = getgdesc(GD_NSCRNS);
	}
	NumPipes = 1;
	break;
#endif /* IRISGL */
    }
    
    /* Set number of pfPipes desired. */
    pfMultipipe(NumPipes);
    /* Enable hyperpipe mode */
    if (Multipipe == HYPERPIPE)
    	pfHyperpipe(NumPipes);

    
    /* allocate shared before fork()'ing parallel processes 
     * so that all processes have will the correct Shared address
     */
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
	
    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    pfConfig();	
    
    /*
     * Assign a non-degrading priority to all Performer processes.
     * User processes spawned from this one will inherit the same priority.
    */
    if (PrioritizeProcs)
        pfuPrioritizeProcs(TRUE);

    if (RestrictCPUs)
	pfuLockDownApp();
    
    scene = pfNewScene();
    
    /* Create a pfLightSource and attach it to scene. */
    {
	pfLightSource *ls = pfNewLSource();
	pfLSourcePos(ls, -0.2f, -0.3f, 1.0f, 0.0f);
	pfAddChild(scene, ls);
    }
    /* make root for movies at start of scene */
    Shared->movieRoot = pfNewGroup();
    /* Turn off culling - this node should always be drawn */
    pfNodeTravMask(Shared->movieRoot, PFTRAV_CULL, 0x0,
                        PFTRAV_SELF | PFTRAV_DESCEND, PF_SET);

    pfAddChild(scene, Shared->movieRoot);
    
    /* set up geomtry and Shared->movieScreen */
    if (HaveBaseTexture) /* base texture for screens for when */
    			 /* no movie is playing */
    {
	Shared->dftTex = pfNewTex(pfGetSharedArena()); 
        pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "base tex file: %s", TexName);	
	pfLoadTexFile(Shared->dftTex, TexName);
    }
    root = MakeScreens();
    pfAddChild(scene, root);
    
    /***** MAKE AND LOAD MOVIE *****/
    {
	/* load Movie */
	if (HaveTextureList)
	{
	    tex = Shared->movie[0].tex = pfNewTex(pfGetSharedArena()); 
	    Shared->movie[0].tape = pfuLoadTexListFiles(NULL, TexFileNames, NumTextures);
	    strcpy(Shared->movie[0].name, TexFileNames[0]);
	} else if (NumMovies) {
	    for (i=0; i < NumMovies; i++)
	    {
		tex = Shared->movie[i].tex = pfNewTex (pfGetSharedArena()); 
		strcpy(Shared->movie[i].name, MovieName[i+1]);
		pfTexName(tex, MovieName[i+1]);
		Shared->movie[i].tape = pfuLoadTexListFmt(NULL, MovieFmt[i+1], MovieStart[i+1], MovieEnd[i+1]);
	    }
	}
	/* Make the projector node (a pfSequence) and add to scene. */
	if (NumMovies)
	{
	    for (i=0; i < NumMovies; i++)
	    {
		Shared->movie[i].proj = pfuNewProjector(Shared->movie[i].tex);
		pfNodeName(Shared->movie[i].proj, Shared->movie[i].name);
		if (!Shared->movie[i].tape)
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
			"No frames for movie \"%s\"", Shared->movie[i].name);
		else
		{
		    /* this will basically just do a pfTextureList(tex, tape); */
		    pfuProjectorMovie(Shared->movie[i].proj, Shared->movie[i].tape);
		    Shared->movie[i].length = pfGetNum(Shared->movie[i].tape);
		}
		if (ReplaceMode)
		    pfTexLoadMode(Shared->movie[i].tex, PFTEX_LOAD_LIST, PFTEX_LIST_AUTO_SUBLOAD);
		else
		    pfTexLoadMode(Shared->movie[i].tex, PFTEX_LOAD_LIST, PFTEX_LIST_AUTO_IDLE);
	    }
	    
	}
	/* Add all projectors to projection root */
	for (i=0; i < NumMovies; i++)
	    pfAddChild(Shared->movieRoot, Shared->movie[i].proj);
	
	/* start with first movie ready to play on all screens */
	Shared->screenSel = 0;
	Shared->movieSel = 0;
	if (NumMovies) for (i=0; i < NumScreens; i++)
	{
	    addScreenToProjector(Shared->movieSel, i);
	}
	resetProjector(Shared->movieSel);
    }
    /* get list of all textures to pre-def and load */
    Shared->texList = pfuGetSharedTexList();
    
    /* Make all textures Fast-Define */
    if (ReplaceMode)
    {
	if (Shared->dftTex)
	{
	    pfTexFormat(Shared->dftTex,  PFTEX_SUBLOAD_FORMAT, TRUE);
	    if (ReplaceMode)
		pfTexFormat(Shared->dftTex,  PFTEX_SUBLOAD_FORMAT, TRUE);
	    pfTexFilter(Shared->dftTex,  PFTEX_MINFILTER, PFTEX_BILINEAR);
	}
	for (i=0; i < NumMovies; i++)
	{
	    for (j=0; j < Shared->movie[i].length; j++)
	    {
		tex = pfGet(Shared->movie[i].tape, j);
		pfTexFormat(tex,  PFTEX_SUBLOAD_FORMAT, TRUE);
		if (ReplaceMode)
		    pfTexFormat(tex,  PFTEX_SUBLOAD_FORMAT, TRUE);
		pfTexFilter(tex, PFTEX_MINFILTER, PFTEX_BILINEAR);
	    }
	}
    }
    
    /* initialize all pipes */
    pfStageConfigFunc(NULL, PFPROC_DRAW, OpenPipeline);
    pfConfigStage(0, PFPROC_DRAW);

    /* Open window */
    pipe = pfGetPipe(0);
    pipewin = pfNewPWin(pipe);
    pfPWinType(pipewin, PFWIN_TYPE_X);
    pfPWinName(pipewin, "Movie");
    pfPWinOriginSize(pipewin, WinXOrigin, WinYOrigin, WinXSize, WinYSize);

    pfPWinConfigFunc(pipewin, OpenPipeWin);
    pfConfigPWin(pipewin);

    pfFrame();
    
    pfuInitGUI(pipewin);
#if defined(__linux__) || defined(WIN32)
    pfuInitInput(pipewin, PFUINPUT_NOFORK_X);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Forcing Input model to NOFORK_X [%s:%d]\n", __FILE__, __LINE__);
#else
    pfuInitInput(pipewin, PFUINPUT_X);
#endif
    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene, &bsphere);
    
    /* Create and configure a pfChannel. */
    /* Configure and open GLX window */
    chan = pfNewChan(pipe);
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);
    pfChanTravFunc(chan, PFTRAV_CULL, CullChannel);
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 20.0f * bsphere.radius);
    /* 45 degrees wide,  vertical=-1 to signal match window aspect */
    pfChanFOV(chan, 45.0f, -1.0f);
    pfFStatsClass(pfGetChanFStats(chan), PFSTATS_ENGFX, PFSTATS_ON);

    /* Create an earth/sky model that draws sky/ground/horizon */
    esky = pfNewESky();
    pfESkyMode(esky, PFES_BUFFER_CLEAR, PFES_SKY_GRND );
    pfESkyAttr(esky, PFES_GRND_HT, -1.0f * bsphere.radius);
    pfESkyColor(esky, PFES_GRND_FAR, 0.3f, 0.1f, 0.0f, 1.0f);
    pfESkyColor(esky, PFES_GRND_NEAR, 0.5f, 0.3f, 0.1f, 1.0f);
    
    /* If multipipe, then we need channels on each pipe */
    if (Multipipe)
    {
	unsigned int share;
	pfChannel *c;
    
	/* share everything */
	share = pfGetChanShare(chan);
	pfChanShare(chan, share | PFCHAN_VIEWPORT | PFCHAN_VIEW_OFFSETS);
	for (i=1; i < NumPipes; i++)
	{
	    c = pfNewChan(pfGetPipe(i));
	    pfAttachChan(chan, c);
	}
    }

    pfChanESky(chan, esky);
    
    pfChanViewport(chan, 0.0f, 0.8f, 0.0f, 1.0);
    pfuGUIViewport(0.8f, 1.0f, 0.0f, 1.0);
    guiDirty = 1;
    
    Shared->guiPanel = InitPanel();
    pfuEnablePanel(Shared->guiPanel);
    
    /* Main simulation loop */
    while (!(Shared->exitFlag))
    {
	float      s, c, r;
	pfCoord	   view;

	/* Go to sleep until next frame time. */
	pfSync();
	
	pfuGetMouse(&Shared->mouse);

	/* Initiate cull/draw for this frame. */
	pfFrame();
	
	if(guiDirty)
	{
    	    pfuEnableGUI(TRUE);
	    guiDirty = 0;
	}
	/* Compute new view position for next frame. */
	if (Shared->spin > 0)
	{
	    t = pfGetTime();
	    pfSinCos(45.0f*t, &s, &c);
	    pfSetVec3(view.hpr, 45.0f*t, -10.0f, 0);
	    pfSetVec3(view.xyz, 2.0f * bsphere.radius * s, 
		    -2.1f * bsphere.radius *c, 
		     0.2f * bsphere.radius);
	} else if (Shared->spin == 0)
	{
	    pfSetVec3(view.xyz, 0.0f, -2.1f * bsphere.radius, 0.2f *
	    	bsphere.radius);
	    pfSetVec3(view.hpr, 0.0f, 0.0f, 0.0f);
	    Shared->spin = -1;
	}
	/* set view position for next frame */
	pfChanView(chan, view.xyz, view.hpr);
	
	/* Get snapshot of event/key queues */
	pfuGetEvents(&Shared->events);
	ProcessInputIO();
	
	/* spin (roll) the screens around y axis */
	r = ((int) (t*100.0f)) % 360;
	for (i=0; i < NumScreens; i++)
	{
	    float ri = r + i*20.0f;
	    if (i & 0x1)
		pfDCSRot(ScreenDCS[i], 0.0f, 0.0f, ri);
	    else
		pfDCSRot(ScreenDCS[i], 0.0f, 0.0f, -ri);
	}
	/*** MOVIE CONTROL HERE ***/
	if (Shared->movieFlag)
	{
	    switch(Shared->movieFlag)
	    {
		case CTL_ADD_SCREEN_TO_PROJECTOR:
		    addScreenToProjector(Shared->movieSel, Shared->screenSel);
		    resetProjector(Shared->movieSel);
		    break;
		case CTL_RESET:
		    if (Shared->movieSel > -1)
		    {
			pfNotify(PFNFY_INFO, PFNFY_PRINT, 
				"Reseting movie %s.", 
				Shared->movie[Shared->movieSel].name);
			resetProjector(Shared->movieSel);
		    }
		    break;
		case CTL_START:
		    if (Shared->movieSel > -1)
		    {
			pfNotify(PFNFY_INFO, PFNFY_PRINT, 
			    "Starting movie %s.", 
			    Shared->movie[Shared->movieSel].name);
			pfSeqInterval(Shared->movie[Shared->movieSel].proj,
				    PFSEQ_CYCLE, Shared->start, Shared->end);
			pfSeqMode(Shared->movie[Shared->movieSel].proj,
				    PFSEQ_START);
		    }
		    break;
		case CTL_STOP:
		    if (Shared->movieSel > -1)
		    {
			pfNotify(PFNFY_INFO, PFNFY_PRINT, 
			    "Stoping movie %s.", 
			    Shared->movie[Shared->movieSel].name);
			pfSeqMode(Shared->movie[Shared->movieSel].proj,
				    PFSEQ_STOP);
		    }
		    break;
		case CTL_PAUSE:
		    if (Shared->movieSel > -1)
		    {
			pfNotify(PFNFY_INFO, PFNFY_PRINT, 
			    "Pause movie %s.", 
			    Shared->movie[Shared->movieSel].name);
			pfSeqMode(Shared->movie[Shared->movieSel].proj,
				    PFSEQ_PAUSE);
		    }
		    break;
		case CTL_RESUME:
		    if (Shared->movieSel > -1)
		    {
			pfNotify(PFNFY_INFO, PFNFY_PRINT, 
			    "Resume movie %s.", 
			    Shared->movie[Shared->movieSel].name);
			pfSeqMode(Shared->movie[Shared->movieSel].proj,
				    PFSEQ_RESUME);
			pfSeqInterval(Shared->movie[Shared->movieSel].proj,
				    PFSEQ_CYCLE, Shared->start, Shared->end);
		    }
		    break;
		case CTL_STEP:
		    if (Shared->movieSel > -1)
		    {
			pfSequence *seq = Shared->movie[Shared->movieSel].proj;
			int repeat;
			int frame = pfGetSeqFrame(seq, &repeat);
			
			frame += Shared->direction;
			if (frame > (Shared->movie[Shared->movieSel].length - 1))
			    frame = 0;
			pfSeqInterval(Shared->movie[Shared->movieSel].proj,
				    PFSEQ_CYCLE, 
				    frame, frame);
		    }
		    break;
		case CTL_REVERSE:
		    if (Shared->movieSel > -1)
		    {
			int tmp;
			pfNotify(PFNFY_INFO, PFNFY_PRINT, 
			    "Reverse movie %s.", 
			    Shared->movie[Shared->movieSel].name);
			
			Shared->direction *= -1;
			tmp = Shared->start;
			Shared->start = Shared->end;
			Shared->end = tmp;
		    }
		    break;
		case CTL_SPEED:
		    if (Shared->movieSel > -1)
		    {
			pfSeqDuration(Shared->movie[Shared->movieSel].proj, 
				    Shared->speed, -1.0f);
		    }
		    break;
	    }
	    Shared->movieFlag = 0;
	}
	pfuUpdateGUI(&Shared->mouse);
    }

     /* Exit from libpfutil and remove shared mem arenas */
    pfuExitUtil();
    pfuExitInput();
    /* Terminate parallel processes and exit. */
    pfExit();

    return 0;
}

static void
resetProjector(int m)
{    
    MovieData *movie;
    
    if (m < 0)
	return;
    
    movie = &(Shared->movie[m]);
    Shared->direction = 1;
    Shared->start = 0;
    Shared->end = movie->length - 1;
    pfSeqInterval(movie->proj, PFSEQ_CYCLE, Shared->start, Shared->end);
    Shared->speed = 1.0f;
    pfSeqDuration(movie->proj, Shared->speed, -1.0f);
    pfSeqMode(movie->proj, PFSEQ_STOP);
}

static void 
addScreenToProjector(int m, int s)
{

    if (m > -1)
	pfNotify(PFNFY_INFO, PFNFY_PRINT, "Putting movie %s on screen %s.", 
		    Shared->movie[m].name, ScreenStrs[s]);
    else
	pfNotify(PFNFY_INFO, PFNFY_PRINT, "Now no movie on screen %s.", 
					    ScreenStrs[s]);
    
    if (Shared->movieScreen[s].movieIndex > -1)
	pfuRemoveProjectorScreen(
	    Shared->movie[Shared->movieScreen[s].movieIndex].proj, 
	    Shared->movieScreen[s].tex);
    if (m > -1)
    { /* put movie on screen and put up first frame if movie isn't started */
	pfuAddProjectorScreen(Shared->movie[m].proj, 
		  Shared->movieScreen[s].tex);
	if (pfGetTexFrame(Shared->movieScreen[s].tex) == -1)
	    pfTexFrame(Shared->movieScreen[s].tex, 0);
    } else {
	/* no movie - put up the screens own default image */
	pfTexFrame(Shared->movieScreen[s].tex, -1);
	pfTexList(Shared->movieScreen[s].tex, NULL);
    }
    Shared->movieScreen[s].movieIndex = m;
}

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
		pfuRedrawGUI(); /* GUI redraws lazily so must force it here */
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
			case 'g':	/* stats */
			    Shared->showStats ^= 1;
			    break;
			}
		    }
		}
		/* XXX this is very important or else future keybd events
		 * will be lost !!!
		 */
		pfevents->devCount[dev] = 0; /* mark device done */
		break;
	    } /* switch on device */
	} /* if have device */
    } /* for each device */
    pfevents->numDevs = 0;
    pfevents->numKeys = 0;
}


static void
OpenPipeWin(pfPipeWindow *pw)
{
    void *arena = pfGetSharedArena();
    pfOpenPWin(pw);

    /* Create and apply a default material for those models
     * without one.
     */
    pfApplyMtl(pfNewMtl(arena));
    /* Create a default lighting model. */
    pfApplyLModel(pfNewLModel(arena));
    /* enable lighting */
    pfEnable(PFEN_LIGHTING);
    /* enable texture */
    pfApplyTEnv(pfNewTEnv(arena));
    pfEnable(PFEN_TEXTURE);
    /* set alpha function to block pixels of 0 alpha for transparent textures */
    pfAlphaFunc(0, PFAF_NOTEQUAL);
    /* disable backface culling (default is PFCF_BACK) */
    pfCullFace (PFCF_OFF);
    
    pfuDownloadTexList(Shared->texList, PFUTEX_SHOW);
} 


/*
 *	OpenPipeline() -- create a GLX window: set up the
 *      window system, OpenGL, X, and OpenGL Performer. This
 *      procedure is executed in the draw process (when
 *      there is a separate draw process).
 */
static void
OpenPipeline (int pipe, uint stage)
{
    if (RestrictCPUs)
	pfuLockDownDraw(pfGetPipe(pipe));
}


static void
CullChannel(pfChannel *chan, void *data)
{
static first = 1;

    (data, data);
    
    if (first)
    {
	if (RestrictCPUs)
	    pfuLockDownCull(pfGetChanPipe(chan));
	first = 0;
    }    
    pfCull();
}

static void
DrawChannel (pfChannel *channel, void *data)
{
static first = 1;

    (data, data);
    
    pfClearChan(channel);
    pfDraw();
    
    /* draw channel statistics */
    if (Shared->showStats)
	pfDrawChanStats(channel);
}


static pfNode *
MakeScreens(void)
{
    pfGroup	*root;
    pfGeoSet	*gset;
    pfGeode     *geode;
    pfGeoState  *gstate;
    pfDCS	*dcs;
    void	*arena;
    int		comp, sx, sy, sz;
    unsigned int	*image;
    int	i;
    int row	= 1;
    int col	= 0;
    
    arena = pfGetSharedArena();
    
    root = pfNewGroup();
    
    for (i=0; i < NumScreens; i++)
    {
	/* unique for each screen */
	gset = pfNewGSet(arena);
	gstate = pfNewGState (arena);
	pfGSetGState (gset, gstate);
	Shared->movieScreen[i].tex = pfNewTex(arena);
	Shared->movieScreen[i].movieIndex = -1; /* no movie */
	pfTexName(Shared->movieScreen[i].tex, ScreenStrs[i]);
	if (Shared->dftTex)
	{ /* all screens start off with the same default image */
	    pfGetTexImage(Shared->dftTex, &image, &comp, &sx, &sy, &sz);
	    pfTexImage(Shared->movieScreen[i].tex, image, comp, sx, sy, sz);
	}
	pfGStateAttr (gstate, PFSTATE_TEXTURE, Shared->movieScreen[i].tex);
	
	/* Set up geosets */
	pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
	pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, NULL);
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
	pfGSetPrimType(gset, PFGS_QUADS);
	pfGSetNumPrims(gset, 1);
    
	/* set up scene graph */
	geode = pfNewGeode(); 
	dcs = ScreenDCS[i] = pfNewDCS();
	pfDCSTrans(dcs,  row * (2.0f*col+2.0f), col*2.0f, 0.0f);
	
	pfAddGSet(geode, gset);
	pfAddChild(dcs, geode);
	pfAddChild(root, dcs);
    
	row *= -1;
	col += (i & 0x1);
    }
    
    return ((pfNode*)root);
}


static pfuPanel *
InitPanel(void)
{
    float  xo, yo, ys, xs;
    int x, y, yTop, xsize, ysize;
    pfuWidget *wid;
    pfuPanel *pan;
    
    xsize = 170;
    ysize = PFUGUI_BUTTON_YSIZE;
    
    pan = pfuNewPanel();
    pfuGetPanelOriginSize(pan, &xo, &yo, &xs, &ys);
    
    x = (int)xo + 4;
    yTop = (int)(yo + ys - 4);
    y = yTop - PFUGUI_BUTTON_YINC;
       
	/* action button - quit */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_QUIT); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Quit");
    
    y -= PFUGUI_BUTTON_YINC;
	
	/* menu button - select */
    wid = pfuNewWidget(pan, PFUGUI_MENU_BUTTON, CTL_SCREEN_SELECT); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Screen");
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetSelections(wid, ScreenStrs, NULL, NULL, NumScreens);
  	
    y -= PFUGUI_BUTTON_YINC;
	
	/* menu button - select */
    wid = pfuNewWidget(pan, PFUGUI_MENU_BUTTON, CTL_MOVIE_SELECT); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Movie");
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetSelections(wid, MovieName, NULL, NULL, NumMovies+1);
    pfuWidgetDefaultSelection(wid, 0);
    pfuWidgetSelection(wid, 0);
  	
    y -= PFUGUI_BUTTON_YINC;
	
	/* action button - put */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_ADD_SCREEN_TO_PROJECTOR); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "MovieOnScreen");
  	
    y -= PFUGUI_BUTTON_YINC;

	/* action button - start */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_START); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Start");
  	
    y -= PFUGUI_BUTTON_YINC;
	
	/* action button - stop */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_STOP); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Stop");
    	
    y -= PFUGUI_BUTTON_YINC;
	
	/* action button - pause */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_PAUSE); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Pause");
    	
    y -= PFUGUI_BUTTON_YINC;
	
	/* action button - resume */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_RESUME); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Resume");
    	
    y -= PFUGUI_BUTTON_YINC;
	
	/* action button - step */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_STEP); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Step");
 	
    y -= PFUGUI_BUTTON_YINC;
	
	/* switch button - reverse */
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_REVERSE); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetValue(wid, 0.0f);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Reverse");
 	
    y -= PFUGUI_BUTTON_YINC;
		
	/* action button - reset */
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_RESET); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Reset");
  	
    y -= PFUGUI_BUTTON_YINC;
	
	/* action button - spin */
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_SPIN); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetValue(wid, 0.0f);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Spin");
 
    y -= PFUGUI_SLIDER_YINC;
    
	/* horizontal slider */
    wid = pfuNewWidget(pan, PFUGUI_SLIDER_LOG, CTL_SPEED);
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "Speed");
    pfuWidgetRange(wid, 1, 0.01f, 10.0f, 1.0f);
    pfuWidgetActionFunc(wid, PanelControl);
    
    y -= PFUGUI_BUTTON_YINC;
	
	/* switch button - show stats */
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_STATS); 
    pfuWidgetDim(wid,  x,  y,  xsize,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetValue(wid, 0.0f);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Stats");

    return pan;
}

/*
 * This is called by the asynch input handling process
 */
static void
PanelControl(pfuWidget *w)
{
    int id;

    switch(id = pfuGetWidgetId(w))
    {
    case CTL_QUIT:
	Shared->exitFlag = 1;
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Have Quit button - bye...\n");
	break;
    case CTL_SCREEN_SELECT:
	Shared->screenSel = (int) pfuGetWidgetValue(w);
	break;
    case CTL_MOVIE_SELECT:
	Shared->movieSel = ((int) pfuGetWidgetValue(w)) - 1;
	break;
    case CTL_SPEED:
	Shared->speed = pfuGetWidgetValue(w);
	Shared->movieFlag = id;
	break;
    case CTL_SPIN:
	Shared->spin = (int) pfuGetWidgetValue(w);
	break;
    case CTL_STATS:
	Shared->showStats ^= 1;
	break;
    case CTL_ADD_SCREEN_TO_PROJECTOR:
    case CTL_RESET:
    case CTL_START:
    case CTL_STOP:
    case CTL_PAUSE:
    case CTL_RESUME:
    case CTL_STEP:
    case CTL_REVERSE:
	Shared->movieFlag = id;
	break;
    }
}
