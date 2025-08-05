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
 */

#include <stdlib.h>
#include <getopt.h> /* for cmdline handler */
#include <X11/keysym.h>
#include <vl/vl.h>
#include <getopt.h> /* for cmdline handler */
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

/* 
 * file: videotex.c
 * ----------------
 *
 * demonstrates use of video input for textures across SGI product line
 *
 * cmdline controls
 *  -m procsplit : multiprocess mode
 *  -v	: use fake video loop from memory - not real video input
 *
 * Run-time controls:
 *       ESC-key: exits
 *      g/s-keys: stats profile
 *       SPACE  : toggle spinning of video cube
 */

#define NUMANIMFRAMES 7
unsigned int animation[];
#define TEXW 8
#define TEXH 8


/* Performer config vars */
static int ProcSplit = PFMP_DEFAULT;
static int Video = 1;
static char ProgName[PF_MAXSTRING];

static pfMatrix texture_matrix = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

/*
** As usual, anything that the cull or draw processes need to access must be
** placed in shared memory.  I put these things in shared memory just in
** case an application might have a use for them there.
*/
typedef struct {
    /* video scene stuff */
    pfTexture	*vtex;
    pfGeoState  *gstate;
    
    /* video stuff */
    int		tvxsize, tvysize; /* size of texture */
    int		vXSize, vYSize; /* video frame size */
    int		fXSize, fYSize; /* memory field size */
    int		F1_is_first;   /* Which field is first (i.e., spacially higher) */
    int		packing;
    VLServer	svr;
    VLPath	path;
    VLNode	src, drn;
    GLXVideoSourceSGIX glxvidsrc;
    
    /* video config controls */
    int		interlace;
    int		vldmedia; /* use old style VL (RE and OCTANE) */
    int		divo;
    int		octane;
    
    /* program controls */
    pfPipeWindow *pw;
    pfWSConnection dsp;
    int		spin;
    int		stats;
    int		XInputInited;
    int		exitFlag;

} shareData;
shareData *Shared;


static void OpenPipeWin(pfPipeWindow *pw);
static pfGeoSet *cube( void );
static void InitXInput(pfWSConnection dsp);
static void GetXInput(Display *dsp);

/******************************************************************************
 *			    VL Variables and Routines
 ******************************************************************************
 */

int capType =  VL_CAPTURE_NONINTERLEAVED;


/* VL Draw Process Variables */
#ifndef IRIX6_2
DMbufferpool pool = 0; /* needed for cleanup */
#endif

void
UpdateTiming(void)
{
    VLControlValue timing, dominance;

    if (vlGetControl(Shared->svr, Shared->path, Shared->src, VL_TIMING, &timing) <0) {
	vlPerror("VlGetControl of timing");
	exit(EXIT_FAILURE);
    }
    /*
     * in 525 ("NTSC"), the odd field is drawn first (higher spacially).
     * in 625 ("PAL"), the even field is drawn first
     */
    Shared->F1_is_first = ( (timing.intVal == VL_TIMING_625_SQ_PIX) ||
		  (timing.intVal == VL_TIMING_625_CCIR601) );

    if (Shared->divo)
    {
#ifndef IRIX6_2
	if (vlGetControl(Shared->svr, Shared->path, Shared->src, 
				    VL_FIELD_DOMINANCE, &dominance) <0) 
	{
	    vlPerror("VlGetControl of dominance");
	    exit(EXIT_FAILURE);
	}
	Shared->F1_is_first ^= (dominance.intVal == VL_F2_IS_DOMINANT);
#endif
    }
}

static void
InitializeVL(void)
{
#ifndef IRIX6_2
    DMparams* plist;
#endif
    VLControlValue size;
    VLDevList	devlist;
    VLTransferDescriptor xferDesc;
    VLControlValue val;
    VLDev	deviceId;
    int		devicenum = -1;
    int		mask;
    int		frameSize;

    
    
    /* open connection to VL server */
    if (!(Shared->svr = vlOpenVideo(""))) {
	printf("couldn't open connection to VL server\n");
	exit(EXIT_FAILURE);
    }

    /* Get the list of devices that the daemon supports */
    if (vlGetDeviceList(Shared->svr, &devlist) < 0) {
	fprintf(stderr, "%s: getting device list: %s\n",
		ProgName, vlStrError(vlErrno));
	exit(1);
    }

    /* Set up a source node from video */
    Shared->src = vlGetNode(Shared->svr, VL_SRC, VL_VIDEO, VL_ANY);

    /* Set up a drain node in memory */
    if (Shared->vldmedia)
	Shared->drn = vlGetNode(Shared->svr, VL_DRN, VL_MEM, VL_ANY);
    else
	Shared->drn = vlGetNode(Shared->svr, VL_DRN, VL_TEXTURE, VL_ANY);


    /*
     * If the user didn't specify a device create a path using the first
     * device that will support it
     */
    if (devicenum == -1) {
	if ((Shared->path = vlCreatePath(Shared->svr, VL_ANY, Shared->src, Shared->drn)) < 0) {
	    vlPerror(ProgName);
	    exit(1);
	}
	/* Get the device number and name */
	devicenum = vlGetDevice(Shared->svr, Shared->path);
    }
    else 
    {
	/* Create a path using the user specified device */
	deviceId   = devlist.devices[devicenum].dev;
	if ((Shared->path = vlCreatePath(Shared->svr, deviceId, Shared->src, Shared->drn)) < 0)
	    {
		vlPerror(ProgName);
		exit(1);
	    }
    }


    /* Set up the hardware for and define the usage of the path */
    if (vlSetupPaths(Shared->svr, (VLPathList)&Shared->path, 1, VL_SHARE, VL_SHARE) < 0) {
	vlPerror(ProgName);
	exit(1);
    }

    if (Shared->vldmedia)
    {
	/*
	 * Specify what path-related events we want to receive.
	 * In this example we want transfer complete and transfer failed
	 * events only.
	 */
	mask= VLStreamStartedMask | VLTransferCompleteMask | VLTransferFailedMask
	    | VLStreamStoppedMask | VLSequenceLostMask;
	vlSelectEvents(Shared->svr, Shared->path, mask);
    }

    /* VL Congif vars and defaults */
    if (Shared->vldmedia)
    {
#ifndef IRIX6_2    
	if (Shared->divo)
	    Shared->packing = VL_PACKING_4444_8; /* DIVO only */
	else
	    Shared->packing = VL_PACKING_ABGR_8; /* O2  */
#endif
    }
    else
	Shared->packing = VL_PACKING_RGB_8;

    val.intVal = Shared->packing;
    if (vlSetControl(Shared->svr, Shared->path, Shared->drn, VL_PACKING, &val) < 0) {
	vlPerror("vlSetControl(VL_PACKING)");
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "PACKING: %d", Shared->packing);
	exit(1);
    }

    if (Shared->divo)
    {
#ifndef IRIX6_2
	int cspace = VL_COLORSPACE_RGB;
	val.intVal = cspace;
	if (vlSetControl(Shared->svr, Shared->path, Shared->drn, VL_COLORSPACE, &val) < 0) {
	    vlPerror("vlSetControl(VL_COLORSPACE_RGB)");
	    exit(1);
	}
#endif
    }

    /*
     * we want individual fields, so we can demonstrate manual interleaving
     */
    val.intVal = capType;
    if (vlSetControl(Shared->svr, Shared->path, Shared->drn, VL_CAP_TYPE, &val) < 0) {
	vlPerror("vlSetControl(VL_CAP_TYPE)");
	exit(1);
    }
    
    /* get input mem frame size */
    vlGetControl(Shared->svr, Shared->path, Shared->src, VL_SIZE, &size);\
    Shared->fXSize = size.xyVal.x;
    Shared->fYSize = size.xyVal.y;
    fprintf(stderr,"frame: xsize is %d, ysize is %d\n",size.xyVal.x, size.xyVal.y);
    /* get video field size for texture which is based on field size */
    vlGetControl(Shared->svr, Shared->path, Shared->drn, VL_SIZE, &size);
    fprintf(stderr,"field: xsize is %d, ysize is %d\n",size.xyVal.x, size.xyVal.y);
    Shared->vXSize = size.xyVal.x;
    Shared->vYSize = size.xyVal.y;

    frameSize = vlGetTransferSize(Shared->svr, Shared->path); /**/
#ifndef IRIX6_2
    if (dmParamsCreate(&plist)) {
        printf("dmParamsCreate failed!\n");
        exit(1);
    }
#endif

    if (Shared->vldmedia) /* O2 and ONYX2 */
    {
#ifndef IRIX6_2
	{
	    int videoBufCt;
	    dmBufferSetPoolDefaults(plist, 4, frameSize, DM_FALSE, DM_FALSE);
	    dmParamsSetInt(plist, DM_BUFFER_COUNT, 0);
	    vlDMGetParams(Shared->svr, Shared->path, Shared->drn, plist);
	    videoBufCt = dmParamsGetInt(plist, DM_BUFFER_COUNT);
	    dmParamsSetInt(plist, DM_BUFFER_COUNT, videoBufCt + 1);
	    if (dmBufferCreatePool(plist, &pool) != DM_SUCCESS) 
	    {
		printf("dmBufferCreatePool failed %s\n", dmGetError(0, 0));
	    }
	    dmParamsDestroy(plist);
	}
    
	if (!pool) {
	    vlPerror(ProgName);
	    exit(1);
	}
    
	/* Associate the pool with the path */
	if (vlDMPoolRegister(Shared->svr, Shared->path, Shared->drn, pool)) 
	{
	    vlPerror(ProgName);
	    dmBufferDestroyPool(pool);
	    exit(1);
	}
#endif
    }
    UpdateTiming();

    /* On Octane and Sirius, use GLXVideoSource to get video data */
    if (!Shared->vldmedia) {
      Shared->dsp =  pfGetCurWSConnection();
      Shared->glxvidsrc = 
	glXCreateGLXVideoSourceSGIX(Shared->dsp, DefaultScreen(Shared->dsp),
				    Shared->svr,Shared->path,
				    VL_TEXTURE,Shared->drn);
    }

    if (Shared->divo)
    {
	xferDesc.mode = VL_TRANSFER_MODE_CONTINUOUS;
	xferDesc.count = 0;
	xferDesc.delay = 0;
	xferDesc.trigger = VLTriggerImmediate;
    
	/* Do not need gl cxt or win for this.  */
	vlBeginTransfer(Shared->svr, Shared->path, 1, &xferDesc);
    }
    else
	vlBeginTransfer(Shared->svr, Shared->path, 0, NULL );
}

void
cleanup(void)
{
    vlEndTransfer(Shared->svr, Shared->path);
#ifndef IRIX6_2
    if (pool)
    {
	vlDMPoolDeregister(Shared->svr, Shared->path, Shared->drn, pool);
	dmBufferDestroyPool(pool);
    }
#endif
    vlDestroyPath(Shared->svr, Shared->path);
    vlCloseVideo(Shared->svr);
    exit(EXIT_SUCCESS);
}

void
ProcessVideoEvents(void)
{
    VLEvent ev;

    if (vlCheckEvent(Shared->svr,
		     VLSequenceLostMask|VLStreamPreemptedMask|VLTransferCompleteMask|VLTransferFailedMask |
			VLStreamStartedMask | VLStreamStoppedMask,
		     &ev) == -1) {
	return;
    }
    switch(ev.reason) {
      case VLSequenceLost:
	pfNotify(PFNFY_INFO,PFNFY_PRINT,"VLSequenceLost....\n");
	break;
      case VLTransferComplete:
	pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"VLTransferComplete....\n");
	break;
      case VLTransferFailed:
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"VLTransfer FAILED ....\n");
	break;
      case VLStreamPreempted:
        cleanup();
        exit(EXIT_SUCCESS);
	/*NOTREACHED*/
      default:
	break;
    }
}



/******************************************************************************
 *			    Performer Structural Routines
 ******************************************************************************
 */

static void 
ConfigDraw( int pipe, uint stage )
{
    /* Ininialize VL in draw stage config function */
    if (Video)
	InitializeVL();
    
}

static void
InitVideoTexture(void)
{
    int texFormat;
    unsigned int    *img=NULL;
    
    Shared->tvxsize = TEXW;
    Shared->tvysize = TEXH;
    
    if (!Shared->vtex)
    {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Video Texture not initialized!!!!");
	return;
    }
    
    {
	float vheight = Shared->vYSize * 2.0; /* XXX assume draw config is done */
	float s_scale, t_scale;

	/* Establish offsets that will align right side of the video texture on
	 * a 32 texel boundary.  This is a requirement of Impact graphics.
	 */
	int fieldWidth = Shared->vXSize;
	int xOffset = ((fieldWidth + 0x001f) & ~0x001f) - fieldWidth;
	
	pfTexLoadOrigin(Shared->vtex, PFTEX_ORIGIN_DEST, xOffset, 0);
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Tex Load x offset: %d", xOffset);

	/* compute texture size - must be power of 2 in each dimension */
	Shared->tvxsize = 1;
	Shared->tvysize = 1;
	while (Shared->tvxsize < Shared->vXSize) Shared->tvxsize <<= 1;
 	while (Shared->tvysize < vheight) Shared->tvysize <<= 1;
	s_scale = (float)Shared->vXSize / (float)Shared->tvxsize;
	t_scale = Shared->vYSize / (float)Shared->tvysize;
	texture_matrix[0][0] =  s_scale;
	texture_matrix[1][1] =  -t_scale; 
	texture_matrix[3][1] =  t_scale;
    }

    if (Video) {
    /* configure the pfTexture for video */
    pfTexImage(Shared->vtex, img, 4, Shared->tvxsize, Shared->tvysize, 0);
    pfTexLoadSize(Shared->vtex, Shared->vXSize, Shared->vYSize);
    /* these modes are important */
    pfTexFilter(Shared->vtex, PFTEX_MINFILTER, PFTEX_LINEAR);
    pfTexFilter(Shared->vtex, PFTEX_MAGFILTER, PFTEX_LINEAR);
    pfTexFormat(Shared->vtex, PFTEX_SUBLOAD_FORMAT, PF_ON);
    pfTexFormat(Shared->vtex, PFTEX_INTERNAL_FORMAT, GL_RGBA8_EXT);
    
    /* Texture external format should match the VL packing format */
    switch(Shared->packing)
    {
#ifndef IRIX6_2
	case VL_PACKING_X4444_5551:
	    texFormat = PFTEX_UNSIGNED_SHORT_5_5_5_1;
	    break;
	case VL_PACKING_444_332:
	    texFormat = PFTEX_UNSIGNED_BYTE_3_3_2;
	    break;
	case VL_PACKING_4444_8:
#endif
	case VL_PACKING_ABGR_8:
	default:
	    texFormat = PFTEX_PACK_8;
    }
    pfTexFormat(Shared->vtex, PFTEX_EXTERNAL_FORMAT, texFormat);
   
    pfTexLoadMode(Shared->vtex, PFTEX_LOAD_BASE, PFTEX_BASE_AUTO_SUBLOAD);

    pfTexLoadMode(Shared->vtex, PFTEX_LOAD_SOURCE, PFTEX_SOURCE_VIDEO);
      

    if (Shared->vldmedia)
      {
	/* these settings come from InitializeVL called in the draw process */
	pfTexLoadVal(Shared->vtex, PFTEX_LOAD_VIDEO_VLSERVER, (void*)Shared->svr);
	pfTexLoadVal(Shared->vtex, PFTEX_LOAD_VIDEO_VLPATH, (void*)Shared->path);
	pfTexLoadVal(Shared->vtex, PFTEX_LOAD_VIDEO_VLDRAIN, (void*)Shared->drn);
      }
    
    if (Shared->interlace)
      {
	pfTexLoadMode(Shared->vtex, PFTEX_LOAD_VIDEO_INTERLACED, 	
		      (Shared->F1_is_first ? PFTEX_VIDEO_INTERLACED_ODD : PFTEX_VIDEO_INTERLACED_EVEN));
      }
    }
}
 
static void
OpenPipeWin(pfPipeWindow *pw)
{
    /* a pfState is created for the pfPipeWindow upon open */
    pfOpenPWin(pw);
    
    /* set the per-draw process video params on the pfTexture */
      InitVideoTexture();

}



/*
** This program doesn't need to have a draw callback, but I put one in just
** in case DIVO apps need to do something here (such as initiate the video
** load or do some sort of synchronization).
*/
static void
drawfunc(pfChannel *chan, void *data)
{
    pfClearChan(chan);

    if (Video) {
      if (!Shared->vldmedia) {
	glXMakeCurrentReadSGI(Shared->dsp,pfGetPWinCurWSDrawable(Shared->pw),
			      Shared->glxvidsrc,
			      pfGetPWinGLCxt(Shared->pw));
      }
    }

    pfDraw();
    if (Shared->stats)
	pfDrawChanStats(chan);
    
    /* XXXX Yuck - we'd like this to be in the application process!
     * Also,  putting it here means it can hold off swapbuffers
     * so we will need to had a post-swap-func callback to Performer
     */
    if (Video)
	ProcessVideoEvents();

   {
   int err;
   if ((err = glGetError()) != GL_NO_ERROR)
        pfNotify(PFNFY_NOTICE,PFNFY_USAGE,"OpenGL Error 0x%x - %s",err, gluErrorString(err));
   }
}


static int
docmdline(int argc, char *argv[])
{
    int	    opt;

    strcpy(ProgName, argv[0]);
    
    /* process command-line arguments */
    while ((opt = getopt(argc, argv, "m:p:sv?")) != -1)
    {
	switch (opt)
	{
	case 's':
	    Shared->spin ^= 1;
	    break;
	case 'm':
	case 'p':
	    ProcSplit = atoi(optarg);
	    break;
	case 'v':
	    Video ^= 1;
	}
    }
    return optind;
}


int
main (int argc, char *argv[])
{
    float	    t = 0;
    pfScene	    *scene;
    pfGroup	    *root;
    pfPipe	    *p;
    pfChannel	    *chan;
    pfSphere	    bsphere;
    pfGeode         *geode;
    pfTexture	    *vtex;
    pfGeoState      *gstate;
    pfGeoSet	    *gset;
    pfCoord	    view;
    void	    *arena;
    int		    arg;
    unsigned int    *img=NULL;
    int		    queryvals[2];
static int querytoks[] = {PFQFTR_VLDMEDIA, PFQFTR_INTERLACE, 0};


    pfInit();	
    arena = pfGetSharedArena();
    Shared = pfCalloc(1, sizeof(shareData), arena);
    Shared->stats = 1;
    
    /* query machine dependent parameters */
    {
	pfMQueryFeature(querytoks, queryvals);
	Shared->vldmedia =  (queryvals[0] != 0);
	Shared->interlace = 0; 
	/* check for DIVO vs Sirius */
	Shared->divo = ((strncasecmp(pfGetMachString(), "IRL", 3)== 0) ||
			(strncasecmp(pfGetMachString(), "IRE",3)== 0));
	/* another check for Octane because the video is flipped vertically */
	if (!Shared->divo)
	{
	    Shared->octane = (strncasecmp(pfGetMachString(), "MGRAS", 5)== 0);
	}
	if (!Shared->divo)
	    Shared->interlace = 0;
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Config %s: vl=%d divo=%d octane=%d", 
	    pfGetMachString(), Shared->vldmedia, Shared->divo, Shared->octane);
    }
    
    arg = docmdline(argc, argv);
    
    pfMultiprocess(ProcSplit);
    pfConfig();	

    /* Shared->put these in shared memory so draw process can access them */

    /* set up scene graph */
    geode = pfNewGeode(); 
    gset = cube();
    pfAddGSet(geode, gset);
    root = pfNewGroup();
    pfAddChild(root, geode);
    scene = pfNewScene();
    pfAddChild(scene, root);
    pfAddChild(scene, pfNewLSource());

    /* make video texture */
    Shared->vtex = vtex = pfNewTex(arena);

    /* make a window */
    p = pfGetPipe(0);
    pfStageConfigFunc(0, PFPROC_DRAW, ConfigDraw);
    
    Shared->pw = pfNewPWin(p);
    pfPWinType(Shared->pw, PFPWIN_TYPE_X);
    pfPWinName(Shared->pw, "videotex");
    pfPWinOriginSize(Shared->pw, 0, 0, 640, 480);
    pfPWinConfigFunc(Shared->pw, OpenPipeWin);

    /* OpenWindow and Init Draw */
    pfConfigStage(0, PFPROC_DRAW);
    pfConfigPWin(Shared->pw);
    pfFrame();
    
    /* make a channel */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanFOV(chan, 45, 0);
    pfChanTravFunc(chan, PFTRAV_DRAW, drawfunc); /* per-frame draw func */

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (root, &bsphere);
    pfChanNearFar(chan, 1, 10 * bsphere.radius);
    pfSetVec3(view.hpr, 0, -20, 0);
    pfSetVec3(view.xyz, 0, -3.0f * bsphere.radius, 0);

    pfTexRepeat(vtex, PFTEX_WRAP_S, PFTEX_CLAMP);
    pfTexRepeat(vtex, PFTEX_WRAP_T, PFTEX_CLAMP);
    
    /* make a geostate */
    gstate = pfNewGState (arena);
    pfGSetGState(gset, gstate);
    pfGStateMode(gstate, PFSTATE_ENTEXTURE, 1);
    pfGStateAttr(gstate, PFSTATE_TEXTURE, vtex);
    pfGStateMode(gstate, PFSTATE_ENTEXMAT, 1);
    pfGStateMode(gstate, PFSTATE_CULLFACE, PFCF_BACK);
    pfGStateAttr(gstate, PFSTATE_TEXMAT, texture_matrix);

    if (!Video) {
	img = (unsigned int *) pfCalloc(Shared->tvxsize * Shared->tvysize * sizeof(int), 1, NULL);
    }

    pfFrame();

    while (!Shared->exitFlag)
    {
	static int i=0;
	float      s, c;
	unsigned int *pb;
	
	/* set view position for next frame */
	if (Shared->spin)
	{
	t = pfGetTime();
	/* Compute new view position for next frame. */
	pfSinCos(45.0f*t, &s, &c);
	pfSetVec3(view.hpr, 45.0f*t, -20.0f, 0);
	pfSetVec3(view.xyz, 3.0f * bsphere.radius * s, 
		-3.0f * bsphere.radius *c, 
		 1.1f * bsphere.radius);
	}
	else
	{
	    pfSetVec3(view.hpr, 0.0f, 0.0f, 0);
	    pfSetVec3(view.xyz, 0.0f, -2.0f * bsphere.radius, 0.0f);
	}
	
	pfChanView(chan, view.xyz, view.hpr);
	
	pfSync();

	/* Update the buffer for the next frame.  Here we simply copy an
	** image from the animation array into the buffer, but DIVO would
	** initiate a video load into the buffer instead.
	*/
	if (!Video)
	{
	    /* Divide i by 10 to slow down the speed of the animation. */
	    pb = animation + Shared->tvxsize * Shared->tvysize * ((i/10) % NUMANIMFRAMES);
	    memcpy(img, pb, Shared->tvxsize * Shared->tvysize * sizeof(int));
	    pfTexLoadImage(Shared->vtex, img);
	    i++;
	}

	pfFrame();
	
	{
	    static pfWSConnection dsp = NULL;
	    if (!dsp)
		dsp = pfGetCurWSConnection();
	    if (!Shared->XInputInited)
		InitXInput(dsp);
	    if (Shared->XInputInited)
		GetXInput(dsp);
	}

    }
    pfExit();
    return 0;
}


/******************************************************************************
 *				XInput Handler
 ******************************************************************************
 */

static void 
InitXInput(pfWSConnection dsp)
{
    Window w;
    
    /* wait for X Window to exist in Performer shared memory */
    if (w = pfGetPWinWSWindow(Shared->pw))
    {
	XSelectInput(dsp, w, PointerMotionMask |
			ButtonPressMask | ButtonReleaseMask | KeyPressMask);
	XMapWindow(dsp, w);
	XFlush(dsp);
	Shared->XInputInited = 1;
    }
}

static void
GetXInput(pfWSConnection dsp)
{
    if (XEventsQueued(dsp, QueuedAfterFlush))
    while (XEventsQueued(dsp, QueuedAlready))
    {
	XEvent event;
	    
	XNextEvent(dsp, &event);

	switch (event.type) 
	{
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
		Shared->spin ^= 1;
		break;
	    case XK_g:
	    case XK_s:
		Shared->stats = !Shared->stats;
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
}



/******************************************************************************
 *				data for faked memory video loop
 ******************************************************************************
 */


#define B 0x000000ff
#define W 0xffffffff

unsigned int animation[] = {
    /* frame 0 */
    B,B,W,W,W,W,W,W,
    B,B,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,

    /* frame 1 */
    W,W,W,W,W,W,W,W,
    W,B,B,W,W,W,W,W,
    W,B,B,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,

    /* frame 2 */
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,B,B,W,W,W,W,
    W,W,B,B,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,

    /* frame 3 */
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,B,B,W,W,W,
    W,W,W,B,B,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,

    /* frame 4 */
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,B,B,W,W,
    W,W,W,W,B,B,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,

    /* frame 5 */
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,B,B,W,
    W,W,W,W,W,B,B,W,
    W,W,W,W,W,W,W,W,

    /* frame 6 */
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,B,B,
    W,W,W,W,W,W,B,B,
};


/******************************************************************************
 *				cube
 ******************************************************************************
 */

static pfGeoSet *cube( void )
{
    pfVec3		*scoords;
    ushort 		*sindex;
    pfVec4	        *colors;
    pfVec2		*tcoords;
    ushort		*tindex;

    pfGeoSet	*gset;
    void 		*arena;

    int i, j,  c;

static float v[][3] = {
    { -1., -1., -1. },
    {  1., -1., -1. },
    {  1.,  1., -1. },
    { -1.,  1., -1. },
    { -1., -1.,  1. },
    {  1., -1.,  1. },
    {  1.,  1.,  1. },
    { -1.,  1.,  1. }
};

static int con[][4] = {
    { 0, 1, 2, 3 },
    { 2, 6, 5, 1 },
    { 4, 5, 6, 7 },
    { 7, 3, 0, 4 },
    { 7, 6, 2, 3 },
    { 0, 1, 5, 4 }
};

    arena = pfGetSharedArena();

    scoords = (pfVec3 *)pfMalloc( 8 * sizeof( pfVec3 ), arena  );
    sindex  = (ushort *)pfMalloc( 24 * sizeof( ushort ), arena );
    colors  = (pfVec4 *)pfMalloc( 4 * sizeof( pfVec4 ), arena  );
    tcoords = (pfVec2 *)pfMalloc( 4 * sizeof( pfVec2 ), arena ); 
    tindex  = (ushort *)pfMalloc( 24 * sizeof( ushort ), arena );

    for( i = 0; i < 8; i++ )
	    PFSET_VEC3(scoords[i], v[i][0], v[i][1], v[i][2] );

    c = 0;
    for( i = 0; i < 6; i++ )
	    for( j = 0; j < 4; j++ )
	    {
		    sindex[c++] = con[i][j];
	    }
    
    PFSET_VEC4(colors[0], 1, 1, 1, 1 ); 
    PFSET_VEC4(colors[1], 1, 0, 0, 1 ); 
    PFSET_VEC4(colors[2], 0, 1, 0, 1 ); 
    PFSET_VEC4(colors[3], 0, 0, 1, 0 ); 
    
    PFSET_VEC2(tcoords[0], 0.0, 0.0 ); 
    PFSET_VEC2(tcoords[1], 1.0, 0.0 ); 
    PFSET_VEC2(tcoords[2], 1.0, 1.0 ); 
    PFSET_VEC2(tcoords[3], 0.0, 1.0 ); 
    
    c = 0;
    for( i = 0; i < 6; i++ )
	    for( j = 0; j < 4; j++ )
		    tindex[c++] = (ushort)j;

    gset = pfNewGSet(arena);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, scoords, sindex);
    pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, tindex);
    pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, colors, tindex);
    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 6);
    
    return gset;
}

