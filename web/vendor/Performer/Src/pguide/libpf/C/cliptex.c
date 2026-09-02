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
 * cliptex.c
 * 
 * OpenGL Performer example program overflying a clip-textured flat terrain.
 * This program is designed to be used as a simple example/test program for
 * those doing clip-textured terrain.
 *
 * $Revision: 1.21 $ 
 * $Date: 2002/11/14 00:02:43 $
 *
 * Command-line options:
 *  -b	: norborder window
 *  -f	: full screen
 *  -2  : two windows
 *  -c  : cliptexture configuration file
 *  -F	: put X input handling in a forked process
 *  -m procsplit : multiprocess mode
 *  -w	: write scene to file
 *
 * Run-time controls:
 *       ESC-key: exits
 *        F1-key: profile
 *    Left-mouse: advance
 *  Middle-mouse: stop
 *   Right-mouse: retreat
 *        d or D: delete cliptexture
 *        l or L: load cliptexture
 */


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h> /* for sigset for forked X event handler process */
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h> /* for cmdline handler */
#include <X11/keysym.h>
#endif
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_WINDOWS 3

#define POLYSIZE 1000.f

/*
 * structure that resides in shared memory so that the
 * application, cull, and draw processes can access it.
 */
typedef struct
{
    pfPipeWindow     *pw[MAX_WINDOWS];
    pfChannel        *chan[MAX_WINDOWS];
    int		     exitFlag;
    int              deleteClipTex;
    int		     inWindow, reset;
    float	     mouseX, mouseY;
    int		     winSizeX, winSizeY;
    int		     mouseButtons;
    pfCoord	     view, viewOrig;
    float	     sceneSize;
    float	     accelRate;
    int		     drawStats;
    int		     XInputInited;
    pfMPClipTexture  *mpcliptex;
    pfClipTexture    *ct;
    int		     ctWidth;
    int		     ctHeight;
    int		     ctDepth;
    pfTexture        *dummy; /* to disconnect cliptexture from global state */
    int clearFinished;
    int deleteFinished;
} SharedData;

static SharedData *Shared;

/*
 * APP process variables
 */
/* for configuring multi-process */
static int ProcSplit = PFMP_APP_CULL_DRAW; /*PFMP_DEFAULT;*/
/* write out scene upon read-in - uses pfDebugPrint */
static int WriteScene = 0;
static int FullScreen = 0;
static int WinType = PFPWIN_TYPE_X;
static int numWin = 1; /* number of windows */
static int NoBorder = 0;
static int ForkedXInput = 0;
static char *ClipTexFileName;
char ProgName[PF_MAXSTRING];
 
/* light source created and updated in DRAW-process */
static pfLight *Sun;
  
static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static void DeleteClipTexture(pfPipeWindow *pw);
static void ClearGlobalState(pfPipeWindow *pw);
static void UpdateView(void);
#ifndef WIN32
static void InitXInput(pfWSConnection dsp);
static void DoXInput(void);
static void GetXInput(Display *dsp);
#endif
static void Usage(void);


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
    char *string;
    struct stat statbuf;
    strcpy(ProgName, argv[0]);
    
    /* process command-line arguments */
    while ((opt = getopt(argc, argv, "fFb23m:wxp:?")) != -1)
    {
	switch (opt)
	{
	case 'f': 
	    FullScreen = 1;
	    break;
	case 'F': 
	    ForkedXInput = 1;
	    break;
	case '2': 
	    numWin = 2;
	    break;
	case '3': 
	    numWin = 3;
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

    if(optind >= argc) {
      pfNotify(PFNFY_FATAL, PFNFY_PRINT,
	      "%s: No clip texture configuration file argument\n",
	       ProgName);
      exit(-1);
    }

    /* use argument as config file name */
    ClipTexFileName = strdup(argv[optind]);

    return(optind + 1); /* skip the cliptexture */
}


/*
 *	main() -- program entry point. this procedure
 *	is executed in the application process.
 */

char *winnames[] = { "Center View", "Left View", "Right View"};

int
main (int argc, char *argv[])
{
    int		    arg;
    int		    found;
    int 	    i;
    pfGroup	   *root;
    pfGeode	   *terrain;
    pfPipe	   *p;
    pfNode	   *obj;
    pfChannel      *chan[MAX_WINDOWS];
    pfScene        *scene;
    pfEarthSky     *eSky;
    pfBox           bbox;
    float	    myfar = POLYSIZE * 10.f;
    pfWSConnection  dsp=NULL;
    pfGeoSet	    *quad;
    pfGeoState	    *gstate;
    pfVec3	    *corners;
    pfVec2	    *texvals;
    pfTexEnv	    *tev;
    void 	    *arena;
    pfVec3	    *xyzoffset;
    pfCoord	    offset;
    int             cliptex_supported;
    pfTexture       *deltex = 0;
    uint image[4]; /* for dummy texture */

    char	    namestring[128];
    arg = docmdline(argc, argv);
    
    pfInit();

    /* configure multi-process selection */
    pfMultiprocess(ProcSplit);

    /* set number of pipes == number of pwins == number of channels */
    pfMultipipe(numWin);

    arena = pfGetSharedArena();
    
    /* allocate shared before fork()'ing parallel processes */
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), arena);
    Shared->ct = 0;
    Shared->inWindow = 0;
    Shared->reset = 0;
    Shared->exitFlag = 0;
    Shared->deleteClipTex = 0;
    Shared->drawStats = 0;
    Shared->XInputInited = 0;
    Shared->deleteFinished = 0;
    Shared->clearFinished = 0;
    /* create dummy texture so cliptexture can be disconnected */
    Shared->dummy = pfNewTex(arena);
    pfTexFilter(Shared->dummy, PFTEX_MINFILTER, PFTEX_BILINEAR);
    pfTexImage(Shared->dummy, image, 1, 1, 1, 1);

    /* Load all loader DSO's before pfConfig() forks */
    for (found = arg; found < argc; found++)
	pfdInitConverter(argv[found]);


    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     */
    pfConfig();
    
    /* look in reasonable places for clipmap data */
    pfFilePath(".:/usr/share/Performer/data/clipdata/hunter:"
	       "/usr/share/Performer/data/clipdata/moffett");

    /* create new mp clip texture */
    Shared->ct = pfdLoadClipTexture(ClipTexFileName);
    if(!Shared->ct)
      pfNotify(PFNFY_FATAL, PFNFY_PRINT, 
	       "%s: Invalid cliptexture configuration file: %s\n",
	       ProgName, ClipTexFileName);

    pfGetClipTextureVirtualSize(Shared->ct, 
				&Shared->ctWidth,
				&Shared->ctHeight,
				&Shared->ctDepth);

    
    Shared->mpcliptex = pfNewMPClipTexture();

    pfMPClipTextureClipTexture(Shared->mpcliptex, Shared->ct);
    (void)pfuAddMPClipTextureToPipes(Shared->mpcliptex, pfGetPipe(0), NULL);
    
    for(i = 0; i < numWin;i++) {
      p = pfGetPipe(i);
      pfPipeScreen(p, 0); /* all pipes use 1 screen */
      Shared->pw[i] = pfNewPWin(p);
      (void)sprintf(namestring, 
		    "Clip Texture Demo: %s%s", 
		    winnames[i + 1 - (numWin & 0x1)],
		    i == 0 ? "(control)" : "");
      pfPWinName(Shared->pw[i], namestring);
      pfPWinType(Shared->pw[i], WinType);
      if (NoBorder)
	pfPWinMode(Shared->pw[i], PFWIN_NOBORDER, 1);
      /* Open and configure the GL window. */
      pfPWinConfigFunc(Shared->pw[i], OpenPipeWin);
      pfConfigPWin(Shared->pw[i]);
      if (FullScreen)
	pfPWinFullScreen(Shared->pw[i]);
      else
	pfPWinOriginSize(Shared->pw[i], 0, 0, 512, 512);
      pfPipeIncrementalStateChanNum(p, 0);
    }
    
    
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

    /* build a scene with a terrain node */
    root = pfNewGroup();
    pfAddChild(scene, root);
    terrain = pfNewGeode();
    pfAddChild(root, terrain);
    quad = pfNewGSet(arena);


    texvals = (pfVec2 *)pfMalloc(sizeof(pfVec2) * 4, arena);

    pfSetVec2(texvals[0], 0., 0.);
    pfSetVec2(texvals[1], 1., 0.);
    pfSetVec2(texvals[2], 1., 1.);
    pfSetVec2(texvals[3], 0., 1.);

    corners = (pfVec3 *)pfMalloc(sizeof(pfVec3) * 4, arena);

    pfSetVec3(corners[0], 0.f,      0.f,      0.f);
    pfSetVec3(corners[1], POLYSIZE, 0.f,      0.f);
    pfSetVec3(corners[2], POLYSIZE, POLYSIZE, 0.f);
    pfSetVec3(corners[3], 0.f,      POLYSIZE, 0.f);

    pfGSetAttr(quad, PFGS_COORD3, PFGS_PER_VERTEX, corners, NULL);
    pfGSetAttr(quad, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texvals, NULL);
    pfGSetPrimType(quad, PFGS_QUADS);
    pfGSetNumPrims(quad, 1);
    pfAddGSet(terrain, quad);
    
    /* set up geo state */

    gstate = pfNewGState(arena);
    pfGStateMode(gstate, PFSTATE_ENTEXTURE, 1);
    pfGStateAttr(gstate, PFSTATE_TEXTURE, Shared->ct);
    pfGSetGState(quad, gstate);

    for (found = 0; arg < argc; arg++)
    {
	if ((obj = pfdLoadFile(argv[arg])) != NULL)
	{
	    pfAddChild(root,obj);
	    found++;
	}
    }

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
    


    eSky = pfNewESky();
    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    pfESkyAttr(eSky, PFES_GRND_HT, -POLYSIZE/500.f);
    pfESkyColor(eSky, PFES_GRND_FAR, 0.1f, 0.1f, 0.1f, 1.0f);
    pfESkyColor(eSky, PFES_GRND_NEAR, 0.2f, 0.4f, 0.2f, 1.0f);


    pfSetVec3(offset.xyz, 0.f, 0.f, 0.f);
    pfSetVec3(offset.hpr, 0.f, 0.f, 0.f);
    for(i = 0; i < numWin; i++) {
      chan[i] = pfNewChan(pfGetPipe(i));
      pfAddChan(Shared->pw[i], chan[i]);
      if(i > 0) {
	  pfAttachChan(chan[0], chan[i]);
	  offset.hpr[PF_H] = i * -90.f + (numWin - 1) * 90.f - 45.f;
	  pfChanViewOffsets(chan[i], offset.xyz, offset.hpr);
      } else {
	  /* vertical FOV is matched to window aspect ratio */
	  pfChanFOV(chan[i], 45.0f, -1.0f);
	  pfSetVec3(Shared->view.xyz, POLYSIZE/2.f, 
		    POLYSIZE/2.f, 
		    POLYSIZE/50.f);
	  pfSetVec3(Shared->view.hpr, 0.f, 0.f, 0.f);
	  Shared->view.hpr[PF_H] = -(numWin -1) * 22.5;
	  PFSET_VEC3(bbox.min, 0.0, 0.0, 0.0);
	  PFSET_VEC3(bbox.max, POLYSIZE, POLYSIZE, POLYSIZE);
	  Shared->sceneSize = POLYSIZE;
	  pfChanView(chan[i], Shared->view.xyz, Shared->view.hpr);
	  PFCOPY_VEC3(Shared->viewOrig.xyz, Shared->view.xyz);
	  PFCOPY_VEC3(Shared->viewOrig.hpr, Shared->view.hpr);
	  pfChanScene(chan[i], scene);
	  pfChanNearFar(chan[i], 0.1f, myfar);
	  pfChanESky(chan[i], eSky);
	  pfChanTravFunc(chan[i], PFTRAV_CULL, CullChannel);
	  pfChanTravFunc(chan[i], PFTRAV_DRAW, DrawChannel);
      }
      pfChanTravMode(chan[i], PFTRAV_CULL, PFCULL_ALL);
    }
#ifndef WIN32
    /* create forked XInput handling process 
     * since the Shared pointer has already been initialized, that structure
     * will be visible to the XInput process. Nothing else created in the
     * application after this fork whose handles are not put in shared memory
     * (such as the database and channels) will be visible to the
     * XInput process.
     */
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
	else
	{
	    dsp = pfGetCurWSConnection();
	}
    }
#endif
    
    /* main simulation loop */
    while (!Shared->exitFlag)
    {
	
	/* wait until next frame boundary */
	pfSync();
	
        /* delete the current cliptexture, if one exists */
	if(Shared->deleteClipTex && Shared->mpcliptex) 
	{
	    int framecount;
	    int notclear = 1;

	    /* Only need to delete cliptex GLhandle 1x on multiple pipes (?) */ 
	    Shared->ct = pfGetMPClipTextureClipTexture(Shared->mpcliptex);
	    pfRef(Shared->ct);
	    pfDelete(Shared->mpcliptex);

	    framecount = pfGetFrameCount(); /* what frame am I deleting in? */
	    while(notclear)
	    {
		/* wait until all pipes have drawn this frame */
		pfFrame();
		notclear = 0;
		for(i = 0; i < numWin; i++)
		    if(framecount+4 >  pfGetPipeDrawCount(i))
			notclear = 1;
	    }

	    /* disconnect cliptexture from scenegraph */
	    pfGStateAttr(gstate, PFSTATE_TEXTURE, NULL); 
	    Shared->deleteFinished = 0;
	    for(i = 0; i < numWin;  i++)
	    {
		pfUserData(Shared->pw[i], (void *)Shared->ct);
		pfPWinConfigFunc(Shared->pw[i], DeleteClipTexture);
		pfConfigPWin(Shared->pw[i]);
	    }
	    while(Shared->deleteFinished < numWin)
	        pfFrame();

	    /* wait for frame to go by if in lock mode. this ensures
	       that all downloads to texture memory and cliptexture are done */
	    for (i = 0; i < numWin; i++)
	    {
		pfUserData(Shared->pw[i], NULL); /* stop referencing cliptexture */
	    }
	    
	    framecount = pfGetFrameCount(); /* what frame am I deleting in? */
	    notclear = 1;
	    while(notclear)
	    {
		/* wait until all pipes have drawn this frame */
		pfFrame();
		notclear = 0;
		for(i = 0; i < numWin; i++)
		    if(framecount+4 >  pfGetPipeDrawCount(i))
			notclear = 1;
	    }

            /* undo ref from attaching mpcliptex to cliptex and delete */
		pfNotify(PFNFY_WARN,PFNFY_PRINT,"Starting Unref Delete (%d)",pfGetRef(Shared->ct));
	    pfUnrefDelete(Shared->ct); 
		pfNotify(PFNFY_WARN,PFNFY_PRINT,"Finished Unref Delete");
	    Shared->mpcliptex = NULL;
	    Shared->ct = NULL;
	}
	/* if there is no cliptexture, and it's no longer supposed to be
	 * deleted, create a new one
	 */
	if(!Shared->mpcliptex && !Shared->deleteClipTex)
	{
	    int framecount;
	    int notclear = 1;

	    /* create new mp clip texture */
	    Shared->ct = pfdLoadClipTexture(ClipTexFileName);
	    if(!Shared->ct)
		pfNotify(PFNFY_FATAL, PFNFY_PRINT, 
			 "%s: Invalid cliptexture configuration file: %s\n",
			 ProgName, ClipTexFileName);

	    pfGetClipTextureVirtualSize(Shared->ct, 
					&Shared->ctWidth,
					&Shared->ctHeight,
					&Shared->ctDepth);

	    Shared->mpcliptex = pfNewMPClipTexture();
	    
	    pfMPClipTextureClipTexture(Shared->mpcliptex, Shared->ct);
	    (void)pfuAddMPClipTextureToPipes(Shared->mpcliptex, pfGetPipe(0),
					     NULL);
    
	    framecount = pfGetFrameCount(); /* what frame am I deleting in? */
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"Created New Ct 0x%x in frame %d",
		Shared->ct, framecount);
	    while(notclear)
	    {
		/* wait until all pipes have drawn this frame */
		pfFrame();
		notclear = 0;
		for(i = 0; i < numWin; i++)
		    if(framecount+4 >  pfGetPipeDrawCount(i))
			notclear = 1;
	    }
    
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"Attaching Ct 0x%x to gstate in frame %d",
		Shared->ct, pfGetFrameCount());
	    pfGStateAttr(gstate, PFSTATE_TEXTURE, Shared->ct);
	}

	/* initiate traversal using current state */
	pfFrame();
	
	/* Set view parameters for next frame */
	UpdateView();
	/* update center based on (x, y) position */
	if(Shared->mpcliptex)
	    pfMPClipTextureCenter(Shared->mpcliptex,
				  Shared->view.xyz[PF_X] * 
				  Shared->ctWidth/POLYSIZE,
				  Shared->view.xyz[PF_Y] * 
				  Shared->ctHeight/POLYSIZE,
				  0);
	pfChanView(chan[0], Shared->view.xyz, Shared->view.hpr);
	
#ifndef WIN32
	if (!ForkedXInput)
	{
	    if (!Shared->XInputInited)
		InitXInput(dsp);
	    if (Shared->XInputInited)
		GetXInput(dsp);
	}
#endif
    }
    
    /* terminate cull and draw processes (if they exist) */
    pfExit();
    
    /* exit to operating system */
    exit(0);
}

#ifndef WIN32
static void 
InitXInput(pfWSConnection dsp)
{
    Window w;
    
    /* wait for X Window to exist in Performer shared memory */
    /* only take x events from window 0 */
    if (w = pfGetPWinWSWindow(Shared->pw[0]))
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
#endif

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
	Shared->accelRate = 0.05f * Shared->sceneSize;
	return;
    }

    deltaTime = thisTime - prevTime;
#ifndef WIN32
    switch (Shared->mouseButtons)
    {
    case Button1Mask: /* LEFTMOUSE: faster forward or slower backward*/
    case Button1Mask|Button2Mask:
	speed += Shared->accelRate * deltaTime;
	if (speed > Shared->sceneSize * .5f)
	    speed = Shared->sceneSize * .5f;
	break;
    case Button3Mask: /* RIGHTMOUSE: faster backward or slower foreward*/
    case Button3Mask|Button2Mask:
	speed -= Shared->accelRate * deltaTime;
	if (speed < -Shared->sceneSize * .5f)
	    speed = -Shared->sceneSize * .5f;
	break;
    }
#endif

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
	/* clamp (x, y, z) range */

	if(view->xyz[PF_X] < 0.f)
	    view->xyz[PF_X] = 0.f;
	if(view->xyz[PF_X] > POLYSIZE)
	    view->xyz[PF_X] = POLYSIZE;

	if(view->xyz[PF_Y] < 0.f)
	    view->xyz[PF_Y] = 0.f;
	if(view->xyz[PF_Y] > POLYSIZE)
	    view->xyz[PF_Y] = POLYSIZE;

	if(view->xyz[PF_Z] < POLYSIZE / 200.f)
	    view->xyz[PF_Z] = POLYSIZE / 200.f;
	if(view->xyz[PF_Z] > POLYSIZE / 2.f)
	    view->xyz[PF_Z] = POLYSIZE / 2.f;


    }
    else
    {
	speed = 0.0f;
	Shared->accelRate = 0.001f * Shared->sceneSize;
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

    pfApplyTEnv(pfNewTEnv(pfGetSharedArena()));
    pfOverride(PFSTATE_TEXENV, PF_ON);
}

static void
DeleteClipTexture(pfPipeWindow *pw)
{
    pfClipTexture *clip;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Start DeleteClipTex");
		
    pfApplyTex(Shared->dummy); /* stop global state from using ct */

    /* get libpr cliptexture ptr */
    clip = (pfClipTexture*)pfGetUserData(pw); /* ptr can be passed other ways */
    PFASSERTALWAYS(pfIsOfType(clip, pfGetClipTextureClassType()));
    pfDeleteGLHandle((pfTexture *)clip);
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Finish DeleteClipTex");
    Shared->deleteFinished++;
}

static void
ClearGlobalState(pfPipeWindow *pw)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Start Clear Global");
		
    pfApplyTex(Shared->dummy); /* stop global state from using ct */

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Finish Clear Global");
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

#ifndef WIN32
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
	    case XK_D: /* MP deletion of cliptextures */
	    case XK_d:
		if(!Shared->deleteClipTex)
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "deleting cliptexture");
		Shared->deleteClipTex = 1;
		break;
	    case XK_L: /* MP load of cliptextures */
	    case XK_l:
		if(Shared->deleteClipTex)
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "creating cliptexture");
		Shared->deleteClipTex = 0;

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
#endif
