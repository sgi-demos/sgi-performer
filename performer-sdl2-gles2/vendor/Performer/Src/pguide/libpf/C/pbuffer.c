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
 * pbuffer.c
 * 
 * OpenGL Performer pfPipeWindow example showing use of pbuffers.
 *
 * $Revision: 1.15 $ 
 * $Date: 2000/10/06 21:26:36 $
 *
 * Command line options:
 *	'b'	: noborder window
 *	'F'	: run X input in forked process
 *	'm procsplit' : multiprocess mode
 *
 *
 * Run-time controls:
 *       ESC-key: exits
 *        F1-key: profile
 *    Left-mouse: advance
 *  Middle-mouse: stop
 *   Right-mouse: retreat
 *	C-key	: close window (will auto-reopen in 3 secs).
 *	s-key	: toggle window between gfx and fill-stats windows
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

#define WIN_SIZE 400

/*
 * structure that resides in shared memory so that the
 * application, cull, and draw processes can access it.
 */
typedef struct
{
    pfPipeWindow    *pwCopy;
    pfPipeWindow    *pwPbuf;
    int		    exitFlag;
    int		    inWindow, reset;
    float	    mouseX, mouseY;
    int		    winSizeX, winSizeY;
    int		    mouseButtons;
    pfCoord	    view, viewOrig;
    float	    sceneSize;
    int		    drawStats;
} SharedData;


static int FBAttrs[] = {
    GLX_DRAWABLE_TYPE_SGIX, GLX_WINDOW_BIT_SGIX|GLX_PBUFFER_BIT_SGIX,
    GLX_RENDER_TYPE_SGIX, GLX_RGBA_BIT_SGIX,
    PFFB_RED_SIZE, 5,
    PFFB_BLUE_SIZE, 5,
    PFFB_GREEN_SIZE, 5,
    None,
};

static SharedData *Shared;

/*
 * APP process variables
 */
/* for configuring multi-process */
static int ProcSplit = PFMP_DEFAULT;
/* write out scene upon read-in - uses pfDebugPrint */
static int WinType = PFPWIN_TYPE_X;
char ProgName[PF_MAXSTRING];
 
/* light source created and updated in DRAW-process */
static pfLight *Sun=NULL;
  
static void CullChannel(pfChannel *chan, void *data);
static void DrawCopy(pfChannel *chan, void *data);
static void DrawPbuf(pfChannel *chan, void *data);
static void OpenPbuf(pfPipeWindow *pw);
static void OpenCopy(pfPipeWindow *pw);
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

    strcpy(ProgName, argv[0]);
    
    /* process command-line arguments */
    while ((opt = getopt(argc, argv, "Fm:Rp:?")) != -1)
    {
	switch (opt)
	{
	case 'm':
	case 'p':
	    ProcSplit = atoi(optarg);
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
    pfChannel      *chanPbuf, *chanCopy;
    pfScene        *scene;
    pfPipe         *p;
    pfPipeWindow   *pw;
    pfEarthSky     *eSky;
    pfFrameStats   *fstats;
    pfSphere        bsphere;
    float	    far = 10000.0f;
    double	    ct;
    pfWSConnection  dsp=NULL;


    arg = docmdline(argc, argv);

    pfInit();

    /* configure multi-process selection */
    pfMultiprocess(ProcSplit);
    
    /* allocate shared before fork()'ing parallel processes */
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    Shared->inWindow = 0;
    Shared->reset = 0;
    Shared->exitFlag = 0;
    Shared->drawStats = 0;

    /* Load all loader DSO's before pfConfig() forks */
    for (found = arg; found < argc; found++)
	pfdInitConverter(argv[found]);

    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     */
    pfConfig();
    
    /* configure pipes and windows */
    p = pfGetPipe(0);
    pfPipeScreen(p, 0);
    
    Shared->pwPbuf = pfNewPWin(p);
    Shared->pwCopy = pfNewPWin(p); /* make copy 2nd so pbuf is drawn first */
    pfPWinShare(Shared->pwPbuf,  PFWIN_SHARE_FBCONFIG | PFWIN_SHARE_STATE | PFWIN_SHARE_GL_OBJS);
    pfPWinType(Shared->pwPbuf, PFPWIN_TYPE_SHARE);
    pfPWinType(Shared->pwCopy, PFPWIN_TYPE_SHARE);


    pfPWinOriginSize(Shared->pwPbuf, 0, 0, WIN_SIZE, WIN_SIZE);
    pfPWinOriginSize(Shared->pwCopy, 0, 0, WIN_SIZE, WIN_SIZE);
    
    pfPWinName(Shared->pwPbuf, "PBuffer");
    pfPWinType(Shared->pwPbuf, WinType | PFWIN_TYPE_PBUFFER);
    /* Open and configure the GL window. */
    pfPWinConfigFunc(Shared->pwPbuf, OpenPbuf);
    pfConfigPWin(Shared->pwPbuf);
    
    pfPWinName(Shared->pwCopy, "Copy");
    pfPWinType(Shared->pwCopy, WinType);
    /* Open and configure the GL window. */
    pfPWinConfigFunc(Shared->pwCopy, OpenCopy);
    pfConfigPWin(Shared->pwCopy);

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
	    pfAddChild(scene, root);
	    found++;
	}
    }

    /* if no files successfully loaded, terminate program */
#if 0
    if (!found)
	Usage();
#endif


    pfFrameRate(30.0f);
    pfPhase(PFPHASE_FREE_RUN);
    
    chanPbuf = pfNewChan(p);
    pfAddChan(Shared->pwPbuf, chanPbuf);
    pfChanTravFunc(chanPbuf, PFTRAV_CULL, CullChannel);
    pfChanTravFunc(chanPbuf, PFTRAV_DRAW, DrawPbuf);
    pfChanScene(chanPbuf, scene);
    pfChanNearFar(chanPbuf, 0.1f, far);
    
    /* Create an earth/sky model that draws sky/ground/horizon */
    eSky = pfNewESky();
    pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    pfESkyAttr(eSky, PFES_GRND_HT, -10.0f);
    pfChanESky(chanPbuf, eSky);
    
    /* vertical FOV is matched to window aspect ratio */
    pfChanFOV(chanPbuf, 45.0f, -1.0f);
    if (found)
    {
	pfGetNodeBSphere (scene, &bsphere);
	Shared->sceneSize = bsphere.radius;
    }  else
    {
	pfSetVec3(Shared->view.xyz, 0.0f, 0.0f, 100.0f);
	PFSET_VEC3(bsphere.center, 0.0, 0.0, 0.0);
	bsphere.radius = Shared->sceneSize = 10000.0f;
    }
    pfSetVec3(Shared->view.hpr, 0.0f, 0.0f, 0.0f);
    pfChanView(chanPbuf, Shared->view.xyz, Shared->view.hpr);
    PFCOPY_VEC3(Shared->viewOrig.xyz, Shared->view.xyz);
    PFCOPY_VEC3(Shared->viewOrig.hpr, Shared->view.hpr);
    
    chanCopy = pfNewChan(p);
    pfMakeOrthoChan(chanCopy, 0, WIN_SIZE, 0, WIN_SIZE);
    pfChanNearFar(chanCopy, -1, 100);
    pfChanTravFunc(chanCopy, PFTRAV_DRAW, DrawCopy);
    pfAddChan(Shared->pwCopy, chanCopy);
    pfChanScene(chanCopy, pfNewScene());
    
    pfFrame();
    
    /* main simulation loop */
    while (!Shared->exitFlag)
    {
	double t;
	float s, c;
	/* wait until next frame boundary */
	pfSync();
	
	pfFrame();
	
	/* Compute new view position. */
	t = pfGetTime();
	pfSinCos(45.0f*t, &s, &c);
	pfSetVec3(Shared->view.hpr, 45.0f*t, -10.0f, 0);
	pfSetVec3(Shared->view.xyz, 2.0f * bsphere.radius * s, 
		-2.0f * bsphere.radius *c, 0.5f * bsphere.radius);
	
	pfChanView(chanPbuf, Shared->view.xyz, Shared->view.hpr);
	/* initiate traversal using current state */
    }
    
    /* terminate cull and draw processes (if they exist) */
    pfExit();
    
    /* exit to operating system */
    return 0;
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
OpenPbuf(pfPipeWindow *pw)
{
    pfPipe *p;
    pfWSConnection dsp;
    int xs=0, ys=0, wxs, wys, num, screen, ret;
    Window pbuf;
    GLXFBConfigSGIX *fbc;

    dsp = pfGetCurWSConnection();
    screen = pfGetPWinScreen(pw);
    if (screen < 0)
    {
	screen = DefaultScreen(dsp);
	pfPWinScreen(pw, screen);
    }
    {
    fbc = glXChooseFBConfigSGIX(dsp, 0, FBAttrs, &num);
pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Num matching FBConfigs on display 0x%p=%s screen %d is %d", 
	dsp, XDisplayString(dsp), screen, num);

    /* just use first FBConfig found */
#if 0
    glXGetFBConfigAttribSGIX(dsp, *fbc, GLX_OPTIMAL_PBUFFER_WIDTH_SGIX, &xs);
    glXGetFBConfigAttribSGIX(dsp, *fbc, GLX_OPTIMAL_PBUFFER_HEIGHT_SGIX, &ys);
pfNotify(PFNFY_NOTICE,PFNFY_PRINT, "Optimal pbuffer size: %d %d", xs, ys);
#endif

    /* note that since we are drawing to the pbuffer, if the pbuffer
     * is bigger, the visual window will only show the lower-left
     * corner of the scene 
     */
    pfGetPWinSize(pw, &wxs, &wys);
    if (!xs)
    {
	xs = wxs;
	ys = wys;
    }
    else if (xs < wxs || ys < wys)
	pfPWinSize(Shared->pwCopy, xs, ys);

    pfPWinSize(pw, xs, ys);
    pfPWinFBConfig(pw, *fbc);
    }
    pfOpenPWin(pw);

    pbuf = pfGetPWinWSDrawable(pw);
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"PBuf is 0x%x", pbuf);

    /* create a light source in the "south-west" (QIII) */
    Sun = pfNewLight(NULL);
    pfLightPos(Sun, -0.3f, -0.3f, 1.0f, 0.0f);
    
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"PBuf is OPEN");
}
 
static void
OpenCopy(pfPipeWindow *pw)
{
    
    pfOpenPWin(pw);

    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Main window is OPEN");
}

/*
 *	DrawPbuf() -- draw the scene graph into the pbuffer window
 *	procedure is executed in the draw process (when there is a
 *	separate draw process).
 */
static void
DrawPbuf (pfChannel *channel, void *data)
{
    /* rebind light so it stays fixed in position */
    pfLightOn(Sun);

    /* erase framebuffer and draw Earth-Sky model */
    pfClearChan(channel);
    
    /* invoke Performer draw-processing for this frame */
    pfDraw();

    /* draw Performer throughput statistics */
    pfDrawChanStats(channel);
}

/*
 *	DrawCopy() -- copy the pbuffer window the visible window
 */
static void
DrawCopy (pfChannel *channel, void *data)
{
    pfWSDrawable draw, read;
    pfGLContext ctx;
    int width,  height;
    pfWSConnection dsp;
    
    pfGetPWinSize(Shared->pwCopy, &width, &height);
    dsp = pfGetCurWSConnection();
    read = pfGetPWinWSDrawable(Shared->pwPbuf);
    draw = pfGetPWinWSDrawable(Shared->pwCopy);
    ctx = pfGetPWinGLCxt(Shared->pwCopy);

#if 0
    if (!read)
	return;
#endif

    glXMakeCurrentReadSGI(dsp, draw, read, ctx);
    pfPushIdentMatrix();
    glRasterPos2i(1, 1);
    glCopyPixels(0, 0, width, height, GL_COLOR);

    pfPopMatrix();
   {
   int err;
   if ((err = glGetError()) != GL_NO_ERROR)
        pfNotify(PFNFY_NOTICE,PFNFY_USAGE,"OpenGL Error 0x%x - %s",err, gluErrorString(err));
   }
}

