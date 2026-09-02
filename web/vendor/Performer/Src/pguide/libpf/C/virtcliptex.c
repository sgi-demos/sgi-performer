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
 * $Revision: 1.16 $ 
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
 *  -t <tess> : make a <tess>x<tess> grid (default 2x2)
 *  -v  : verbose mode (print virtual params every frame)
 *
 * Run-time controls:
 *       ESC-key: exits
 *        F1-key: profile
 *    Left-mouse: advance
 *  Middle-mouse: stop
 *   Right-mouse: retreat
 *	   s-key: toggle stats
 *	   g-key: toggle gridification
 *	   v-key: toggle verbosity
 *	   w-key: toggle scribed drawing mode
 */


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h> /* for sigset for forked X event handler process */
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#define M_SQRT2 (fsqrt(2.0))
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
#define log2(x) (log(x)*M_LOG2E)
#define round(x) floor((x)+.5)
#define log2int(x) ((int)round(log2(x)))/* could make this faster if critical */

float TerrSize = 1000.f; /* size of surface */
int Tess = 2; /* amount of surface tesselation, can set using -t */
int Verbose = 0; /* if set using -v, print params for every tile every frame */

/*
 * structure that resides in shared memory so that the
 * application, cull, and draw processes can access it.
 */
typedef struct
{
    pfPipeWindow     *pw[MAX_WINDOWS];
    pfChannel        *chan[MAX_WINDOWS];
    int		     exitFlag;
    int		     inWindow, reset;
    float	     mouseX, mouseY;
    int		     winSizeX, winSizeY;
    int		     mouseButtons;
    pfCoord	     view, viewOrig;
    float	     sceneSize;
    float	     accelRate;
    int		     drawStats;
    int		     XInputInited;
    pfMPClipTexture *mpcliptex;
    int		     ctWidth;
    int		     ctHeight;
    int		     ctDepth;
    float            tSize;
    int              tess;
    int		     drawStyle;
    int		     gridify, gridified;
    int		     verbose;
} SharedData;

static SharedData *Shared;

/* data used by geode callbacks */
typedef struct
{
    int i, j; /* location of quad in terrain grid */
    pfFrustum *frust; /* frustum copied from channel */
    pfFrustum *xformfrust; /* scratch frustum (xformed into obj space) */
    pfGeode *quad;
    pfClipTexture *cliptex;
    int levels;
    float minS, maxS, minT, maxT;
    /*
     * When Verbose is set, store the results at each node
     * so we can print it out in a table each frame...
     * (XXX this can only work single-pipe, i.e. when only
     * one pipe is writing to the structure at once)
     */
    int LODOffset, numEffectiveLevels;
    float minLOD, maxLOD;
} CallbackData;

/* XXX the following is wrong if there are additional scene files
   specified on the command line */
#define GETCALLBACKDATA(i,j) ((CallbackData *)pfGetNodeTravData(pfGetChild(Root, (i)+(j)*Tess), PFTRAV_CULL))

/*
 * APP process variables
 */
/* for configuring multi-process */
static int ProcSplit = PFMP_DEFAULT;
/* write out scene upon read-in - uses pfDebugPrint */
static int WriteScene = 0;
static int FullScreen = 0;
static int WinType = PFPWIN_TYPE_X;
static int numWin = 1; /* number of windows */
static int NoBorder = 0;
static int ForkedXInput = 0;
static char *ClipTexFileName;
static pfGroup *Root;
char ProgName[PF_MAXSTRING];
 
/* light source created and updated in DRAW-process */
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
    while ((opt = getopt(argc, argv, "fFb23m:wxp:t:v?")) != -1)
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
	case 't':
	    Tess = atoi(optarg);
	    break;
	case 'v':
	    Verbose = 1;
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

    return(optind + 1);
}

/* callback to set lodoffset and effective levels */
int
virtCallback(pfTraverser *trav, void *userData)
{
    pfVec3 eye, eyevec, eyex, eyey, eyedist;
    pfVec3 angles, eyedir;
    float distance, tilt;
    pfVec3 normal; /* always (0, 0, 1) */
    CallbackData *data = (CallbackData *)userData;
    pfChannel *chan = pfGetTravChan(trav);
    float levelsize, level;
    int nlevels;
    pfMatrix viewmat, travmat;
    pfMatrix invtravmat, invviewmat;
    pfMatrix eyeobjmat, objeyemat, eyescreenmat, objscreenmat;
    float x, y, z, s, t, r;
    float ss, tt;
    float ds_dx, dt_dx, ds_dy, dt_dy;
    float log2_ds_dxy, log2_dt_dxy;

    pfGetChanViewMat(chan, viewmat);
    pfGetTravMat(trav, travmat);

    pfInvertOrthoMat(invtravmat, travmat);
    pfInvertOrthoMat(invviewmat, viewmat);

    pfMultMat(eyeobjmat, viewmat, invtravmat);

    pfMultMat(objeyemat, travmat, invviewmat);

    pfOrthoXformFrust(data->xformfrust, data->frust, eyeobjmat);

    /* add more elements to PFUTRAV mask as needed */
    if (pfuGetNearestVisiblePointToEyePlane((pfNode *)data->quad,
					    data->xformfrust,
					    (pfTexture *)data->cliptex,
					    PFUTRAV_SW_CUR, 
					    &x, &y, &z, &s, &t, &r))
    {
	float bboxMinDist, bboxMaxDist;
	int LODOffset, numEffectiveLevels;
	int centerS, centerT;
	float centCoordS, centCoordT;
	pfVec4 p3d;
	pfVec4 verts3d[3];
	pfVec4 verts2d[3], p2d;
	pfVec3 ll, lr, ul, ur; /* vertices of front frust face */
	float nearwidth, nearheight;
	float minLODTexPix, minLOD, maxLOD;
	int viewportWidth, viewportHeight;
	int i;

	/* triangle defining terrain plane */
	pfSetVec4(verts3d[0], 
		  data->i * Shared->tSize/Shared->tess, 
		  data->j * Shared->tSize/Shared->tess,
		  0.f, 1.f);
	pfSetVec4(verts3d[1], 
		  (data->i + 1) * Shared->tSize/Shared->tess, 
		  data->j * Shared->tSize/Shared->tess,
		  0.f, 1.f);
	pfSetVec4(verts3d[2], 
		  data->i * Shared->tSize/Shared->tess, 
		  (data->j + 1) * Shared->tSize/Shared->tess,
		  0.f, 1.f);

 	pfSetVec4(p3d, x, y, z, 1);

	/* Transform the problem into homogeneous screen space... */

	pfGetFrustNear(data->frust, ll, lr, ul, ur);

	nearwidth = PF_ABS(lr[PF_X] - ll[PF_X]);
	nearheight = PF_ABS(ul[PF_Z] - ll[PF_Z]);

	pfGetChanSize(chan, &viewportWidth, &viewportHeight);

	eyescreenmat[0][0] = viewportWidth/nearwidth * ll[PF_Y];
	eyescreenmat[0][1] = 0.f;
	eyescreenmat[0][2] = 0.f;
	eyescreenmat[0][3] = 0.f;

	eyescreenmat[1][0] = viewportWidth/2.f;
	eyescreenmat[1][1] = viewportHeight/2.f;
	eyescreenmat[1][2] = 0.f;
	eyescreenmat[1][3] = 1.f;

	eyescreenmat[2][0] = 0.f;
	eyescreenmat[2][1] = viewportHeight/nearheight * ll[PF_Y];
	eyescreenmat[2][2] = 0.f;
	eyescreenmat[2][3] = 0.f;

	eyescreenmat[3][0] = 0.f;
	eyescreenmat[3][1] = 0.f;
	eyescreenmat[3][2] = 0.f;
	eyescreenmat[3][3] = 0.f;

	pfMultMat(objscreenmat, objeyemat, eyescreenmat);

	for(i =0; i < 3; i++)
	    pfXformVec4(verts2d[i], verts3d[i], objscreenmat);
	
	pfXformVec4(p2d, p3d, objscreenmat);

	if (pfuCalcTexDerivs(
		      verts2d[0][0]/verts2d[0][3], verts2d[0][1]/verts2d[0][3],
		      verts2d[0][3], data->minS, data->minT,
		      verts2d[1][0]/verts2d[1][3], verts2d[1][1]/verts2d[1][3],
		      verts2d[1][3], data->maxS, data->minT,
		      verts2d[2][0]/verts2d[2][3], verts2d[2][1]/verts2d[2][3],
		      verts2d[2][3], data->minS, data->maxT,
		      p2d[0]/p2d[3], p2d[1]/p2d[3],
		      &ss, &tt,
		      &ds_dx, &dt_dx, &ds_dy, &dt_dy))
	{
	    log2_ds_dxy = log2(PF_MAX2(PF_ABS(ds_dx), PF_ABS(ds_dy)));
	    log2_dt_dxy = log2(PF_MAX2(PF_ABS(dt_dx), PF_ABS(dt_dy)));
	    minLODTexPix = data->levels - 1 + PF_MAX2(log2_ds_dxy, log2_dt_dxy);
	    minLODTexPix -= 1.f;
	}
	else
	{
	    /*
	    ** One or more of the partial derivatives is infinite--
	    ** use the coarsest LOD
	    */
	    minLODTexPix = data->levels - 1;	/* coarsest */
	}

	pfGetClipTextureCenter(data->cliptex, &centerS, &centerT, NULL);

	centCoordS = centerS/(float)Shared->ctWidth;
	centCoordT = centerT/(float)Shared->ctHeight;
	bboxMinDist = PF_MAX4(data->minS - centCoordS, centCoordS - data->maxS,
			      data->minT - centCoordT, centCoordT - data->maxT);
	bboxMaxDist = PF_MAX4(data->maxS - centCoordS, centCoordS - data->minS,
			      data->maxT - centCoordT, centCoordT - data->minT);

	pfuCalcVirtualClipTexParams(
        data->levels,
	pfGetClipTextureClipSize(data->cliptex),
	pfGetClipTextureInvalidBorder(data->cliptex),
	minLODTexPix, /* from calcTexDerivs */
	pfGetClipTextureMinDTRLOD(data->cliptex),
	pfGetClipTextureNumAllocatedLevels(data->cliptex),
/* maxs/mins (and t) are bounds for tex coords on current tile */

         bboxMinDist, /* from libpfdb/libpfspherpatrch/pfspherepatch.C */
         bboxMaxDist, 
	1.f,
        NULL, /* limits */
        /* returned values */
	&LODOffset, 
	&numEffectiveLevels,
	&minLOD,
	&maxLOD);

	pfApplyClipTextureVirtualParams(data->cliptex, 
					LODOffset, 
					numEffectiveLevels);
	pfApplyTexMinLOD((pfTexture *)data->cliptex, minLOD);
	pfApplyTexMaxLOD((pfTexture *)data->cliptex, maxLOD);

	if (Shared->verbose)
	{
	    /* store the results so we can print it out in a table */
	    data->LODOffset = LODOffset;
	    data->numEffectiveLevels = numEffectiveLevels;
	    data->minLOD = minLOD;
	    data->maxLOD = maxLOD;
	}
    }
    return PFTRAV_CONT;
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
    int 	    i, j;
    pfGeode	   *terrain;
    pfPipe	   *p;
    pfNode	   *obj;
    pfChannel      *chan[MAX_WINDOWS];
    pfScene        *scene;
    pfEarthSky     *eSky;
    pfBox           bbox;
    float	    myfar = TerrSize * 10.f;
    pfWSConnection  dsp=NULL;
    pfClipTexture   *ct;
    pfGeoSet	    *quad;
    pfGeoState	    *gstate;
    pfVec3	    *verts;
    pfVec2	    *texvals;
    pfTexEnv	    *tev;
    void 	    *arena;
    pfVec3	    *xyzoffset;
    pfCoord	    offset;
    int             cliptex_supported;
    CallbackData    *quadinfo;
    char	    namestring[128];
    pfFrustum       *frust;
    float           dsdx, radius;
    int             virtsize;
    int             levels, maxlevels, effectivelevels, neweffectivelevels;

    arg = docmdline(argc, argv);

    pfInit();

    /* configure multi-process selection */
    pfMultiprocess(ProcSplit);

    /* set number of pipes == number of pwins == number of channels */
    pfMultipipe(numWin);

    arena = pfGetSharedArena();
    
    /* allocate shared before fork()'ing parallel processes */
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), arena);
    Shared->inWindow = 0;
    Shared->reset = 0;
    Shared->exitFlag = 0;
    Shared->drawStats = 0;
    Shared->XInputInited = 0;
    Shared->tSize = TerrSize;
    Shared->tess = Tess;
    Shared->drawStyle = PFUSTYLE_FILLED;
    Shared->gridify = 0;
    Shared->gridified = 0;
    Shared->verbose = Verbose;

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
    ct = pfdLoadClipTexture(ClipTexFileName);
    if(!ct)
      pfNotify(PFNFY_FATAL, PFNFY_PRINT, 
	       "%s: Invalid cliptexture configuration file: %s\n",
	       ProgName, ClipTexFileName);

    pfGetClipTextureVirtualSize(ct, 
				&Shared->ctWidth,
				&Shared->ctHeight,
				&Shared->ctDepth);

    printf("Num tiles = %dx%d, virt size = %dx%d\n",
	    Tess,Tess, Shared->ctWidth, Shared->ctHeight);
    printf("Tile size in finest-level texels = %dx%d (optimal is 16Kx16K)\n",
	    Shared->ctWidth/Tess, Shared->ctHeight/Tess);

    virtsize = PF_MAX3(Shared->ctWidth, Shared->ctHeight, Shared->ctDepth);

    /* make sure cliptexture is virtual */

    levels = log2int(virtsize) + 1;
    pfQuerySys(PFQSYS_MAX_CLIPTEXTURE_SIZE, &maxlevels);
    if(maxlevels) /* cliptexturing is supported on this system */
	maxlevels = log2int(maxlevels) + 1;
    neweffectivelevels = PF_MIN2(levels - 1, maxlevels);
    /* force cliptexture to be virtual if it's not already */
    if(neweffectivelevels < pfGetClipTextureNumEffectiveLevels(ct))
	pfClipTextureNumEffectiveLevels(ct, neweffectivelevels);

    
    Shared->mpcliptex = pfNewMPClipTexture();

    pfMPClipTextureClipTexture(Shared->mpcliptex, ct);
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
    Root = pfNewGroup();
    pfAddChild(scene, Root);

    /* set up geo state */
    gstate = pfNewGState(arena);
    pfGStateMode(gstate, PFSTATE_ENTEXTURE, 1);
    pfGStateAttr(gstate, PFSTATE_TEXTURE, ct);

    


    for (found = 0; arg < argc; arg++)
    {
	if ((obj = pfdLoadFile(argv[arg])) != NULL)
	{
	    pfAddChild(Root,obj);
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
    pfESkyAttr(eSky, PFES_GRND_HT, -TerrSize/150.f);
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
	  pfSetVec3(Shared->view.xyz, TerrSize/2.f, 
		    TerrSize/2.f, 
		    TerrSize/50.f);
	  pfSetVec3(Shared->view.hpr, 0.f, 0.f, 0.f);
	  Shared->view.hpr[PF_H] = -(numWin -1) * 22.5;
	  PFSET_VEC3(bbox.min, 0.0, 0.0, 0.0);
	  PFSET_VEC3(bbox.max, TerrSize, TerrSize, TerrSize);
	  Shared->sceneSize = TerrSize;
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
    
    /* compute info for traversal data */

    frust = pfNewFrust(arena);
    pfGetChanBaseFrust(chan[0], frust);

    dsdx = 1/TerrSize;
    radius = TerrSize/(Tess * 2.f) * M_SQRT2;

    /* make a geode for each quad in the terrain */
    for(j = 0; j < Tess; j++)
	for(i = 0; i < Tess; i++)
	{
	    terrain = pfNewGeode();
	    pfAddChild(Root, terrain);
	    quad = pfNewGSet(arena);


	    quadinfo = (CallbackData *)pfMalloc(sizeof(CallbackData), arena);
	    texvals = (pfVec2 *)pfMalloc(sizeof(pfVec2) * 4, arena);
	    verts = (pfVec3 *)pfMalloc(sizeof(pfVec3) * 4, arena);


	    pfSetVec2(texvals[0], i/(float)Tess, j/(float)Tess);
	    pfSetVec2(texvals[1], (i + 1)/(float)Tess, j/(float)Tess);
	    pfSetVec2(texvals[2], (i + 1)/(float)Tess, (j + 1)/(float)Tess);
	    pfSetVec2(texvals[3], i/(float)Tess, (j + 1)/(float)Tess);


	    pfSetVec3(verts[0], 
		      i * TerrSize/Tess,
		      j * TerrSize/Tess,
		      0.f);

	    pfSetVec3(verts[1], 
		      (i + 1) * TerrSize/Tess,
		      j * TerrSize/Tess,
		      0.f);

	    pfSetVec3(verts[2],
		      (i + 1) * TerrSize/Tess,
		      (j + 1) * TerrSize/Tess,
		      0.f);

	    pfSetVec3(verts[3],
		      i * TerrSize/Tess,
		      (j + 1) * TerrSize/Tess, 0.f);

	    /* set callback info */
	    quadinfo->quad = terrain;
	    quadinfo->cliptex = ct;
	    quadinfo->i = i;
	    quadinfo->j = j;
	    quadinfo->frust = frust;
	    quadinfo->xformfrust = pfNewFrust(arena);
	    quadinfo->levels = log2int(virtsize) + 1;
	    quadinfo->minS = texvals[0][PF_S];
	    quadinfo->maxS = texvals[2][PF_S];
	    quadinfo->minT = texvals[0][PF_T];
	    quadinfo->maxT = texvals[2][PF_T];
	    if (Shared->verbose)
	    {
		/* set to obviously bogus values */
		quadinfo->LODOffset = -1;
		quadinfo->numEffectiveLevels = -1;
		quadinfo->minLOD = -1.f;
		quadinfo->maxLOD = -1.f;
	    }

	    pfNodeTravFuncs(terrain, PFTRAV_CULL, virtCallback, NULL);
	    pfNodeTravData(terrain, PFTRAV_CULL, quadinfo);
	    /*
	     * Very important!  If we don't set PFN_CULL_SORT_CONTAINED
	     * on each tile, the effect of the node pre-cull callbacks
	     * will not be drawn in the proper order with respect to
	     * the contents!
	     */
	    pfNodeTravMode((pfNode *)terrain,
			   PFTRAV_CULL, PFN_CULL_SORT, PFN_CULL_SORT_CONTAINED);
	    pfGSetAttr(quad, PFGS_COORD3, PFGS_PER_VERTEX, verts, NULL);
	    pfGSetAttr(quad, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texvals, NULL);
	    pfGSetPrimType(quad, PFGS_QUADS);
	    pfGSetNumPrims(quad, 1);
	    pfAddGSet(terrain, quad);
	    pfGSetGState(quad, gstate);
       }


    /* main simulation loop */
    while (!Shared->exitFlag)
    {
	
	/* wait until next frame boundary */
	pfSync();
	
	/* initiate traversal using current state */
	pfFrame();
	
	/* Set view parameters for next frame */
	UpdateView();
	/* update center based on (x, y) position */
	pfMPClipTextureCenter(Shared->mpcliptex,
			      Shared->view.xyz[PF_X] * 
			      Shared->ctWidth/TerrSize,
			      Shared->view.xyz[PF_Y] * 
			      Shared->ctHeight/TerrSize,
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
	if(view->xyz[PF_X] > TerrSize)
	    view->xyz[PF_X] = TerrSize;

	if(view->xyz[PF_Y] < 0.f)
	    view->xyz[PF_Y] = 0.f;
	if(view->xyz[PF_Y] > TerrSize)
	    view->xyz[PF_Y] = TerrSize;

	if(view->xyz[PF_Z] < 0.f)
	    view->xyz[PF_Z] = 0.f;
	if(view->xyz[PF_Z] > TerrSize *  2.f)
	    view->xyz[PF_Z] = TerrSize * 2.f;


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
/* ARGSUSED */
static void
CullChannel(pfChannel *channel, void *data)
{
    /* 
     * pfDrawGeoSet or other display listable Performer routines
     * could be invoked before or after pfCull()
     */
    
    if (Shared->gridified != Shared->gridify)
    {
	pfClipTexture *cliptex =
		pfGetMPClipTextureClipTexture(Shared->mpcliptex);
	if (Shared->gridified)
	    pfuUnGridifyClipTexture(cliptex);
	else
	    pfuGridifyClipTexture(cliptex);
	Shared->gridified = !Shared->gridified;
    }

    pfCull();

    if (Shared->verbose)
    {
	/*
	 * Print out the table of results
	 */
	int i, j;
	printf("Frame %d:\n", pfGetFrameCount());
	for (j = Tess-1; j >= 0; j--)
	{
	    for (i = 0; i < Tess; ++i)
		printf("off=%-7d%c",
		       GETCALLBACKDATA(i,j)->LODOffset,
		       i==Tess-1 ? '\n' : ' ');
	    for (i = 0; i < Tess; ++i)
		printf("num=%-7d%c",
		       GETCALLBACKDATA(i,j)->numEffectiveLevels,
		       i==Tess-1 ? '\n' : ' ');
	    for (i = 0; i < Tess; ++i)
		printf("min=%-7.4f%c",
		       GETCALLBACKDATA(i,j)->minLOD,
		       i==Tess-1 ? '\n' : ' ');
	    for (i = 0; i < Tess; ++i)
		printf("max=%-7.4f%c",
		       GETCALLBACKDATA(i,j)->maxLOD,
		       i==Tess-1 ? '\n' : ' ');
	    printf("\n");
	}
	printf("=============================================================================\n");
	/*
	 * Clear for next frame (set to obviously bogus values)
	 */
	for (j = 0; j < Tess; ++j)
	    for (i = 0; i < Tess; ++i)
	    {
		CallbackData *data = GETCALLBACKDATA(i,j);
		data->LODOffset = -1;
		data->numEffectiveLevels = -1;
		data->minLOD = -1;
		data->maxLOD = -1;
	    }
    }
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

    pfOpenPWin(pw);

    /* create a light source in the "south-west" (QIII) */
    Sun = pfNewLight(NULL);
    pfLightPos(Sun, -0.3f, -0.3f, 1.0f, 0.0f);

    pfApplyTEnv(pfNewTEnv(pfGetSharedArena()));
    pfOverride(PFSTATE_TEXENV, PF_ON);
}


/*
 *	DrawChannel() -- draw a channel and read input queue. this
 *	procedure is executed in the draw process (when there is a
 *	separate draw process).
 */
/* ARGSUSED */
static void
DrawChannel (pfChannel *channel, void *data)
{
    int drawStyle = Shared->drawStyle;
    static pfVec4 white = {1,1,1,1};

    /* rebind light so it stays fixed in position */
    pfLightOn(Sun);

    /* erase framebuffer and draw Earth-Sky model */
    pfClearChan(channel);

    pfuPreDrawStyle(drawStyle, white);
    
    /* invoke Performer draw-processing for this frame */
    pfDraw();

    pfuPostDrawStyle(drawStyle);

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
	    case XK_s:
		Shared->drawStats = !Shared->drawStats;
		break;
	    case XK_g:
		Shared->gridify = !Shared->gridify;
		break;
	    case XK_w:
		if (Shared->drawStyle != PFUSTYLE_FILLED)
		    Shared->drawStyle = PFUSTYLE_FILLED;
		else
		    Shared->drawStyle = PFUSTYLE_SCRIBED;
		break;
	    case XK_v:
		Shared->verbose = !Shared->verbose;
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
