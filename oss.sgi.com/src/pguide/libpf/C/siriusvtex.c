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
 *
 * file: siriusvtex.c
 * ----------------
 * 
 * Example of using pfTextures different dynamic input sources
 *  for creating texture movies
 * o using Sirius video as input source control via VideoLibrary
 *	- needs HAVE_VL defined
 * o using images in memory as input source
 * o using framebufer as input source (not yet implemented)
 *
 * $Revision: 1.1 $
 *
 * Cmdline Options:
 * ----------------
 *	-i - use image texture source (can follow with list of textures
 *		or else we will make default ones)
 *	-v - use video texture source
 *	-f - use framebuffer texture source
 *	-t - default base texture
 *	-R - start up running
 *	-c - set number of components in texture image
 *
 * Runtime Controls:
 * -----------------
 *	r - start running movie
 *	space - stop and reset movie
 *	ESC - exit
 *	up/down arrows: step through image movie
 *
 */

/* Make this 1 to have Video Library calls */
#define HAVE_VL 0

/* general includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

/* X Window includes */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#if HAVE_VL
/* VL includes */
#include <vl/vl.h>
#include <vl/dev_sirius.h>
#endif

#include <Performer/pr.h>

#if HAVE_VL

#define SIRIUS_SCANLINE_SIZE	768     /* must be 32 bit word */

static int DoFields=0; /* Sirius - non-interleaved fields */
int vlTexWidth, vlTexHeight; 

/* Video controls */
static int vlInSrc = VL_ANY, vlDevice = VL_ANY;
static VLControlValue vlSize, vlTiming, vlTexPack;
static VLControlValue vlFormat;
static VLControlValue vlCaptureType;
static VLServer vlServer;
static VLPath  vlPath;
static VLNode  vlSrc, vlDrn;
#endif /* HAVE_VL */

#if HAVE_VL
static int VideoInit(void);
static void VideoExit(void);
static void setVideoTextureParameters(pfGeoSet *gset);
static int loadTextureMatrixCB(pfGeoState *gs, void *userData);
static int loadIDMatrixCB(pfGeoState *gs, void *userData);
#endif /* HAVE_VL */

static pfList *loadMovieFiles(pfTexture *movieTex, char **fileNames, int count);
static pfDispList *MakeScene(pfGeoSet *geom, pfTexture *tex);

static void DoEvents(void);
static void DrawScene(void);
static void StartMovie(void);
static void ResetMovie(void);
static void GetFrameData(void);


static float idmat[4][4] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

pfVec2          texcoords[]={   {0.0f, 0.0f},
                                {1.0f, 0.0f},
                                {1.0f, 1.0f},
                                {0.0f, 1.0f} };

pfVec3          coords[] ={     {-1.0f,  -1.0f,  0.0f },
                                { 1.0f,  -1.0f,  0.0f },
                                { 1.0f,  1.0f,   0.0f },
                                {-1.0f,  1.0f,   0.0f } };

pfVec4		colors[] ={ {1.0f, 1.0f, 1.0f, 1.0f} };

#define NUM_COLORS 10
static unsigned int texclrs[] = {
	0x108f8fff, /* base is teal */
	0xff0000ff, /* level 1 is red */
	0x00ff00ff, /* level 2 is green */
	0x0000ffff, /* level 3 is blue */
	0xff00ffff, /* level 4 is magenta */
	0xfff00fff, /* level 5 is yellow */
	0x00ffffff, /* level 6 is cyan */
	0xf0f0f0ff, /* level 7 is grey */
	0xf0f010ff, /* level 8 is ground */
	0xf010f0ff, /* level 9 is sky */
};
				
/* setup parameters */
static int TexSource=PFTEX_SOURCE_IMAGE;
static int glTexFormat = PFTEX_RGB_5; /* PFTEX_RGB_4, PFTEX_RGBA_8 */
static int TexComp = 3;

/* for movies of host texture images */
static char *dftTexName=NULL;
static char **texFileNames=NULL;
static int ListLength=0;
static pfList *TexList=NULL;

/* general movie controls */
static int DoExit = 0;	    /* flag to quit */
static int RunMode = 0;	    /* is movie running */
static int TexFrame = -1;   /* for IMAGE textures only */

/* general movie data */
static pfTexture *MovieTex=NULL;
static pfGeoSet *ScreenGeom=NULL;
static pfDispList *SceneDL=NULL;


static Display *Dsp=NULL;
static pfWindow *Win=NULL;

static char OptionStr[] = "c:ifvR?t:";

static int
docmdline(int argc, char **argv)
{
     extern char *optarg;
     extern int  optind;
     int opt;
        
     while ((opt = getopt(argc,argv,OptionStr)) != -1) {
	 switch(opt) {
	 /* custom options */
	 case 'c':
	    TexComp = atoi(optarg);
	    break;
	 case 'f':  /* use image in framebuffer - not implemented */
	    TexSource = PFTEX_SOURCE_FRAMEBUFFER;
	    break;
	 case 'i':  /* use specified files as movie textures */
	    TexSource = PFTEX_SOURCE_IMAGE;
	    break;
	 case 'v':  /* use video input */
	    TexSource = PFTEX_SOURCE_VIDEO;
	    break;
	 case 't':  /* default textue when no movie is running */
	    dftTexName = optarg;
	    break;
	 case 'R':  /* come up running */
	    RunMode ^= 1;
	    break;
	 case '?':
	 default:
	    pfNotify(PFNFY_FATAL,PFNFY_USAGE,
		"movietex [-irRf] [-t dftimage] [image0 image1 image2 ...]\n");
	    exit(0);
	    break;
	} /* switch */
    } /* while */
    
    texFileNames = &(argv[optind]);
    return argc - optind;
}


int
main(long argc, char *argv[])
{
    int		    i, j, k;
    int		    arg;
    char*	    str;
    pfFrustum	    *frust;
    pfTexture*       tex;
    Window	     xwin;
    void	    *arena=NULL;
    
     /* Initialize Performer */    
    pfInit();
    /* pfInitArenas(); */
    
    pfNotifyLevel(PFNFY_DEBUG);
    
    pfInitState(arena);
    
    arg = docmdline(argc, argv);
    
    /* Append to PFPATH standard data dirs */
    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures");
    
    /* Initialize window and viewing frustum */
    Win = pfNewWin(arena);
    pfWinOriginSize(Win, 0, 0, 400, 400);
    pfWinName(Win, "texlist");
    pfWinType(Win, PFWIN_TYPE_X);
    pfOpenWin(Win);
    Dsp = pfGetCurWSConnection();
    xwin = pfGetWinWSWindow(Win);
    XSelectInput(Dsp, xwin,  KeyPressMask);
    XMapWindow(Dsp, xwin);
    XSync(Dsp,FALSE);

    frust = pfNewFrust(NULL);
    pfApplyFrust(frust);
    pfDelete(frust);
    pfLoadMatrix(idmat);
    pfTranslate (0.0f, 0.0f, -4.0f);
    
    /* Create Movie Texture and Screen Geom */
    MovieTex = pfNewTex(arena);
    /* set some basic formats for doing dynamic textures */
    pfTexFormat(MovieTex, PFTEX_SUBLOAD_FORMAT, 1);
    pfTexFilter(MovieTex, PFTEX_MINFILTER, PFTEX_BILINEAR);
    if (dftTexName)
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		"\tLoading tex %s as base texture\n", dftTexName);
	if (!pfLoadTexFile(MovieTex, dftTexName))
	    dftTexName = 0;
    }
    
    ScreenGeom = pfNewGSet(arena);
    /* create scene */
    SceneDL = MakeScene(ScreenGeom, MovieTex);
        
    /* initialize movie */
    switch(TexSource)
    {
    case PFTEX_SOURCE_VIDEO:
#if HAVE_VL
	pfNotify(PFNFY_INFO, PFNFY_PRINT, "Starting Sirius Video....");
	if (VideoInit() < 0)
	    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, "Error in initializing VL and Sirius.");
	setVideoTextureParameters(ScreenGeom);

	/* video starts running HERE */
	vlBeginTransfer(vlServer, vlPath, 0, NULL);
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Done with video init");

#else
	pfNotify(PFNFY_FATAL, PFNFY_PRINT, 
	    "HAVE_VL is not defined - no video texture possible.");
#endif /* HAVE_VL */
	break;
    case PFTEX_SOURCE_IMAGE:
	TexList = loadMovieFiles(MovieTex, texFileNames, arg);
	ListLength = pfGetNum(TexList);
	pfNotify(PFNFY_INFO, PFNFY_PRINT, "%s has %d frames\n", 
	    pfGetTexName(MovieTex), ListLength);
	break;
    case PFTEX_SOURCE_FRAMEBUFFER:
	pfNotify(PFNFY_FATAL, PFNFY_PRINT, 
	    "Framebuffer movies are not implemented.");
	break;
    }
    

    pfEnable(PFEN_TEXTURE);
    pfApplyTEnv(pfNewTEnv(arena));
    pfApplyTex(MovieTex);

    while(!DoExit)
    {
	/* draw textured scene */
	DrawScene();
	/* advance running movie */
	if (RunMode)
	{
	    /* get data for next movie frame */
	    GetFrameData();
	    /* load texture for next frame */
	    pfLoadTex(MovieTex); 
	}
	/* handle user events */
	DoEvents();
    }
#if HAVE_VL
    VideoExit();
#endif /* HAVE_VL */
    return 0;
}


static void 
DoEvents(void)
{
    long do_draw = 0;
    long hlmode = 0;
    long ov = 0;
    int dev;
    short val;
    
    while (XPending(Dsp))
    {
	XEvent event;
	
	XNextEvent(Dsp, &event);
	switch (event.type) 
	{
	case KeyPress:
	{
	    char buf[100];
	    int rv;
	    KeySym ks;

	    rv = XLookupString(&event.xkey, buf, sizeof(buf), &ks, 0);
	       
	    switch(ks) 
	    {
	    case XK_Escape:
		DoExit = 1;
		break;
	    case XK_space:
		ResetMovie();
		break;
	    case XK_r:
		StartMovie();
		break;
	    case XK_Up:
		if ((TexSource == PFTEX_SOURCE_IMAGE) && val)
		{
		    TexFrame++;
		    if (TexFrame < ListLength)
		    {
			pfTexFrame(MovieTex, TexFrame);
			TexFrame = pfGetTexFrame(MovieTex);
			fprintf(stderr, "Tex frame %d: \"%s\"\n",
			     TexFrame, pfGetTexName(pfGet(TexList, TexFrame)));
			pfApplyTex(MovieTex);
		    } else {
			fprintf(stderr, "frame %d too high. max is %d.\n", 
				TexFrame, ListLength-1);
			TexFrame = ListLength-1;
		    }
		}
		break;   
	    case XK_Down:
		if ((TexSource == PFTEX_SOURCE_IMAGE) && val && (TexFrame > -1))
		{
		    TexFrame--;
		    if (TexFrame < -1)
			TexFrame = -1;
		    pfTexFrame(MovieTex, TexFrame);
		    TexFrame = pfGetTexFrame(MovieTex);
		    fprintf(stderr, "Tex frame %d: \"%s\"\n",
			 TexFrame, pfGetTexName(pfGet(TexList, TexFrame)));
		    pfApplyTex(MovieTex);
		}
		break;    
	    default:
		break;
	    }/* key switch */
	    }
	}/* dev switch */
    } /* while events */
}

static void DrawScene(void) 
{
    static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};
    pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
    
    pfRotate (PF_Z, 1.0f);
    pfDrawDList(SceneDL);
    pfSwapWinBuffers(Win);  
   
}

static void
StartMovie(void)
{
    switch(TexSource)
    {
    case PFTEX_SOURCE_IMAGE:
	TexFrame = 0;
    case PFTEX_SOURCE_VIDEO:
	break;
    case PFTEX_SOURCE_FRAMEBUFFER:
	break;
    }
    RunMode = 1;
}

static void
GetFrameData(void)
{
    switch(TexSource)
    {
    case PFTEX_SOURCE_IMAGE:
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT," MOVIE FRAME %d", TexFrame);
	pfTexFrame(MovieTex, TexFrame);
	pfApplyTex(MovieTex);
	TexFrame++;
	if (TexFrame == ListLength)
	    TexFrame = 0;
    case PFTEX_SOURCE_VIDEO:
	break;
    case PFTEX_SOURCE_FRAMEBUFFER:
	break;
    }
}

static void
ResetMovie(void)
{
    switch(TexSource)
    {
    case PFTEX_SOURCE_IMAGE:
	TexFrame = -1;
	RunMode = 0;
	pfTexFrame(MovieTex, TexFrame);
	TexFrame = pfGetTexFrame(MovieTex);
	fprintf(stderr, "Tex frame %d: \"%s\"\n",
	     TexFrame, pfGetTexName(MovieTex));
	pfApplyTex(MovieTex);
	break;
    case PFTEX_SOURCE_VIDEO:
	break;
    case PFTEX_SOURCE_FRAMEBUFFER:
	break;
    }
}

static pfDispList *
MakeScene(pfGeoSet *geom, pfTexture *tex)
{
    pfDispList *dl;
    pfGeoState *gstate;
    void *arena=NULL;
    
    dl = pfNewDList(PFDL_FLAT, 0, arena);
    
    pfGSetAttr(geom, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(geom, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, NULL);
    pfGSetAttr(geom, PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
    pfGSetPrimType(geom, PFGS_QUADS);
    pfGSetNumPrims(geom, 1);
    
    gstate = pfNewGState (arena);
    pfGSetGState (geom, gstate);
    
    pfGStateMode (gstate, PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
    pfGStateVal (gstate, PFSTATE_ALPHAREF, 0.0f);	
    pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
    pfGStateAttr (gstate, PFSTATE_TEXENV, pfNewTEnv (arena));
    pfGStateAttr (gstate, PFSTATE_TEXTURE, tex);
    
    pfOpenDList(dl);
    pfDrawGSet(geom);
    pfCloseDList();
    
    return dl;
}

static pfList *
loadMovieFiles(pfTexture *movieTex, char **fileNames, int count)
{
    pfList *texList;
    pfTexture *tex;
    unsigned int *image;
    void *arena=NULL;
    int i, j;
    
    texList = pfNewList(sizeof(pfTexture*), 16, arena);
    
    pfTexLoadMode(movieTex, PFTEX_LOAD_LIST, PFTEX_LIST_AUTO_SUBLOAD);
    if (count)
    {
	char *strp;
	
	pfTexName(movieTex, "File Image Movie");
	if (!dftTexName)
	{
	    dftTexName = texFileNames[0];
	    pfLoadTexFile(movieTex, dftTexName);
	}
	for (i=0; i < count; i++)
	{
	    strp = texFileNames[i];
	    tex = pfNewTex(arena);
	    pfTexName(tex, strp);
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		"\tLoading tex %s as frame %d\n", strp, i-1);
	    pfLoadTexFile(tex, strp);
	    pfTexFormat(tex, PFTEX_FAST_DEFINE, 1);
	    pfTexFilter(tex, PFTEX_MINFILTER, PFTEX_BILINEAR);
	    pfAdd(texList, tex);
	}
    } else { /* no files specified - create some colored textures */
	char tstr[PF_MAXSTRING];
	
	pfTexName(movieTex, "Color Image Movie");
	if (!dftTexName)
	{
	    image = pfMalloc(sizeof(int)*32*32, NULL);
	    for (j=0; j < 32*32; j++)
		image[j] = 0xff808000;
	    pfTexImage(movieTex, image, 4, 32, 32, 1);
	}
	
	for (i=0; i < NUM_COLORS; i++)
	{
	    image = pfMalloc(sizeof(int)*32*32, NULL);
	    for (j=0; j < 32*32; j++)
		image[j] = texclrs[i];
	    tex = pfNewTex(arena);
	    sprintf(tstr, "frame=%d", i);
	    pfTexName(tex, tstr);
	    pfTexFormat(tex, PFTEX_FAST_DEFINE, 1);
	    pfTexFilter(tex, PFTEX_MINFILTER, PFTEX_BILINEAR);
	    pfTexImage(tex, image, 4, 32, 32, 1);
	    pfTexLoadSize(tex, 32, 32);
	    pfAdd(texList, tex);
	}
    }
    
    /* set list of texture frame on the base texture */
    pfTexList(movieTex, texList);
    return texList;
}

#if HAVE_VL

static int
VideoInit(void)
{
    /* open the server */
    if (!(vlServer = vlOpenVideo(""))) {
        vlPerror("vlOpenVideo");
        fprintf(stderr, "couldn't open video server!\n");
        return 1;
    }

    /* Get the Video source */
    vlSrc = vlGetNode(vlServer, VL_SRC, VL_VIDEO, vlInSrc);
    /* Get the Texture drain */
    vlDrn = vlGetNode(vlServer, VL_DRN, VL_TEXTURE, 0);
    /* Create path   */
    vlPath = vlCreatePath(vlServer, vlDevice, vlSrc, vlDrn);
    if (vlPath < 0) 
    {
        vlPerror("vlCreatePath");
        return -1;
    }

    /* setup path */
    if (vlSetupPaths(vlServer, (VLPathList)&vlPath,1,VL_SHARE, VL_SHARE) < 0) {
        vlPerror("vlSetupPaths");
        return -1;
    }

    /* select the appropriate events */
    if (vlSelectEvents(vlServer, vlPath, VLStreamPreemptedMask |
                            VLControlChangedMask ) < 0) 
    {
        vlPerror("Select Events");
        return -1;
    }
 
    if (DoFields)
	vlCaptureType.intVal = VL_CAPTURE_NONINTERLEAVED;
    else
	vlCaptureType.intVal = VL_CAPTURE_INTERLEAVED;

    vlSetControl(vlServer, vlPath, vlDrn, VL_CAP_TYPE, &vlCaptureType);
    
    /* Get the timing from input source */
    vlGetControl(vlServer, vlPath, vlSrc, VL_TIMING, &vlTiming);
    /* Set texture drain's timing to input source */
    vlSetControl(vlServer, vlPath, vlDrn, VL_TIMING, &vlTiming);

    /* Get corresponding size */
    vlGetControl(vlServer, vlPath, vlSrc, VL_SIZE, &vlSize);

#if 0
    {
    GLXVideoSourceSGIX videosource;
    videosource = glXCreateGLXVideoSourceSGIX(Dsp, 0 /* screen */, 
	    vlServer, vlPath, VL_TEXTURE, vlDrn)) == None);
    glXMakeCurrentReadSGI(Dsp, 
	    pfGetWinWSDrawable(Win), pfGetWinWSDrawable(Win), /* ??? */
	    pfGetWinGLCxt(Win));
    }
#endif

    return 0;  
}


static void
setVideoTextureParameters(pfGeoSet *gset)
{
    pfGeoState *gstate;
    pfTexture *tex;
    pfMatrix *tmat;
    void *arena=NULL;
    float s_scale, t_scale;
    int loadYsize;
    
    gstate = pfGetGSetGState(gset);
    tex = pfGetGStateAttr(gstate, PFSTATE_TEXTURE);
    
    switch( glTexFormat )
    {
        case PFTEX_RGBA_4:
            vlTexPack.intVal = SIR_TEX_PACK_RGBA_4;
            break;
        case PFTEX_RGB_5:
            vlTexPack.intVal = SIR_TEX_PACK_RGB_5;
            break;
        case PFTEX_RGBA_8:
            vlTexPack.intVal = SIR_TEX_PACK_RGBA_8;
            break;
        default:
            vlTexPack.intVal = SIR_TEX_PACK_RGB_5;
            glTexFormat = PFTEX_RGB_5;
            break;
    }
    /* Set the Texture packing mode */
    vlSetControl(vlServer, vlPath, vlDrn, VL_PACKING, &vlTexPack);
    
    if ((vlTiming.intVal == VL_TIMING_525_SQ_PIX)
     || (vlTiming.intVal == VL_TIMING_525_CCIR601)) {
        vlTexWidth = 1024;
        vlTexHeight = 512;
    } else {
        vlTexWidth = 1024;
        vlTexHeight = 1024;
    }
    /* if we load only fields then reduce height by .5 */
    loadYsize = vlSize.xyVal.y;
    if (DoFields)  /* only 1 field in mem - half height */
    { 
	vlTexHeight >>=1;
 	loadYsize >>1;
    }

    /* create texture matrix for scaling texcoords to valid part of texture */
     
    tmat = (pfMatrix*) pfCalloc(sizeof(pfMatrix), 1, arena);
    pfMakeIdentMat(*tmat);
    s_scale = (vlSize.xyVal.x-1) / (float) vlTexWidth;
    t_scale = vlSize.xyVal.y / (float) vlTexHeight;
    (*tmat)[0][0] =  s_scale;
    (*tmat)[1][1] = -t_scale;
    (*tmat)[3][1] =  t_scale;
    /* gset callbacks to manage texture matrix - put matrix in userData */
    pfGStateFuncs(gstate, loadTextureMatrixCB, loadIDMatrixCB, tmat);
    
    /* define the video texture */
    pfTexImage(tex, NULL, TexComp, vlTexWidth, vlTexHeight, 1);
    /* specify the texture format, this is the default */
    pfTexFormat(tex, PFTEX_INTERNAL_FORMAT, glTexFormat);
    pfTexLoadMode(tex, PFTEX_LOAD_SOURCE, PFTEX_SOURCE_VIDEO);
    /* specify the size to be downloaded from Sirius */
    pfTexLoadSize(tex, SIRIUS_SCANLINE_SIZE, loadYsize); 
}


static void
VideoExit(void)
{
    vlEndTransfer(vlServer, vlPath);
    vlDestroyPath(vlServer, vlPath);
    vlCloseVideo(vlServer);
    exit(0);
}

/* callback to load texture matrix to invert the video image */
static int
loadTextureMatrixCB(pfGeoState *gs, void *userData)
{
    pfMatrix *tmat = (pfMatrix *)(userData);
    mmode(MTEXTURE);
    pfLoadMatrix(*tmat);
    mmode(MVIEWING);
    return 0;
}

static int 
loadIDMatrixCB(pfGeoState *gs, void *userData)
{
    mmode(MTEXTURE);
    pfLoadMatrix(idmat);
    mmode(MVIEWING);
    return 0;
}

#endif

#if 0

void
subtexload( long l1, long l2, float f1, float f2, float f3, float f4, 
		long l3, const unsigned long *p, unsigned long flags)
{
pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
"subtexload: %d %d %f %f %f %f %d 0x%x %d",
	l1, l2, f1,f2,f3,f4,l3,p,flags);
	
}
#endif
