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
 * file: benchgs.c 
 * --------------- 
 *
 * $Revision: 1.6 $ 
 *
 * benchmark program for basic drawing of geosets 
 */


#include <math.h>
#if !defined(__linux__) && !defined(WIN32)
#include <bstring.h>		/* for bzero() */
#endif
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h>
#endif

/* process control includes */
#ifndef WIN32
#include <sys/statfs.h>
#endif
#include <sys/types.h>
#if !defined(__linux__) && !defined(WIN32)
#include <sys/sysmp.h>
#include <sys/schedctl.h>
#endif

#include <Performer/pr.h>

typedef struct obj_t
{
    float               *vdata, *cdata, *ndata, *tdata;
    ushort              *vindex, *cindex, *nindex, *tindex;
    int                 num_verts;
} obj_t;


/******************************************************************************
 *                              Constants
 ******************************************************************************
 */

#define MKDC_PAGEALIGN -1
#define MKDC_RANDOMIZE -2
#define MKDC_PRIMITIVE -3
#define MKDC_INDEX     -4

void     mkdata_config(int prop, int val);
obj_t   *mkdata(int prims_per_obj, int strip_length, int num_objs);


/******************************************************************************* 
 *			Globals and Declarations
 *******************************************************************************
 */


/* function declarations */
static void printRates(double start, double stop, int frames, int num);


/* tables for printing modes */
static const char            primTable[9][32] ={
				   "POINTS",
				   "LINES",
				   "LINESTRIPS",
				   "TRIS",
				   "QUADS",
				   "TRISTRIPS", 
				   "FLAT_LINESTRIPS", 
				   "FLAT_TRISTRIPS",
				   "POLYS"
};

static const char            bindTable[4][32] ={
				   "OFF",
				   "OVERALL",
				   "PER_PRIM",
				   "PER_VERT"
};

/* window controls */
static int            winXorigin, winYorigin, winXsize=10, winYsize=10;

/* benchmark controls and default settings */
static int            obj = 0, dbl = 0;
static int		randomize = 0; /* for random data layout */
static int		compile = 0; /* compile geosets */
static int		oneFrame = 0;
static int		indexData = 0;
static int		RestrictProcessors = 0;
static int		AppCPU = 1;

static int            benchLoop = 10;
static double           benchTime = 3.;
static int            primitive = PFGS_TRISTRIPS;
static int		dispList = 1, useGeoStates = 0;
static int		colormode = PFMTL_CMODE_COLOR;
static int		numShademodel=0, numLMbinds=0, numTexbinds=0;
static int		freqShademodel=0, freqLMbinds=0, freqTexbinds=0;
static int            colorBind=PFGS_OFF, normalBind=PFGS_OFF, textureBind=PFGS_OFF;
static int		flatShade = 1;
static int		ZBuffer=1;
static int		Antialias = 0;
static pfGeoSet       *geos, **geos_list;	/* the geoset data object */
static obj_t          *obj_list;
static int            numPrimitivesPerGSet = 100;
static int            numPrimitivesTotal = 100;
static int		stripLength = 6;
static int		wireframe = 0, gsetwireframe = 0;
static int		polyMode = 0;
static int		Normalize = 1;


/* data storage */
static int      NumVertsPerObj = 0;
static int      NumPrimsPerObj = 0;
static int	 StripLength = 0;

/* object configuration */
static int      NumObjects = 1;
static int      Randomize = 0;	/* for randomizing indices for indexed objects */
static int      PrimitiveType = 5;
static int      vColor = 0, vNormal = 0, vTexture = 0;
static int      PageAlign = 1;
static int	 IndexData = 1;



/******************************************************************************* 
 *			Command Line Stuff
 *******************************************************************************
 */


char            OptionStr[] = "aAB:c:dDfgG:il:L:m:n:N:oO:Pp:rR:t:T:W:wzZ?";

static void
docmdline(int argc, char **argv)
{
    int             opt;

    while ((opt = getopt(argc, argv, OptionStr)) != -1)
    {
	switch (opt)
	{
	case 'a':	/* set antialias flag */
	    Antialias ^= 1;
	    break;
	case 'A':	/* compile geosets into assembly code */
	    compile ^= 1;
	    break;
	case 'B':	/* number of times to run the benchmark loop */
	    benchLoop = atoi(optarg);
	    break;
	case 'c':	/* Set color binding */
	    colorBind = atoi(optarg);
	    break;
	case 'd':	/* bench toggle pfDispList */
	    dispList ^= 1;
	    break;
	case 'f':
	    gsetwireframe = 1;
	    break;
	case 'D':	/* Set double buffer mode on */
	    dbl = 1;
	    break;
	case 'G':
	    useGeoStates = atoi(optarg);;
	    break;
	case 'i':
	    indexData ^= 1;
	    break;
	case 'o':	/* Use GL object for comparison */
	    obj = 1;
	    break;
	case 'O':	/* Set the number of geoset objects */
	    NumObjects = atoi(optarg);
	    break;
	case 'p':	/* Set primitive type */
	    pfGSetPrimType(geos, primitive = atoi(optarg));
	    break;
	case 'P':
	    polyMode = 1;
	    break;
	case 'l':	/* set the lengths of strips */
	    stripLength = atoi(optarg);
	    break;
	case 'g':
	    flatShade = 0;
	    break;
	case 'L':
	    colormode = atoi(optarg);
	    break;
	case 'm':	/* Set mode toggles - shademodel, lmbinds, texbinds */
	    sscanf(optarg, "%d,%d,%d\n", 
		     &numShademodel, &numLMbinds, &numTexbinds);
	    break;
	case 'N':	/* Set number of primitives */
	    numPrimitivesPerGSet = atoi(optarg);
	    pfGSetNumPrims(geos, numPrimitivesPerGSet);
	    break;
	case 'n':	/* Set normal binding */
	    normalBind = atoi(optarg);
	    break;
	case 'r':	/* randomize everywhere possible */
	    randomize = 1;
	    break;
	case 'R': /* lock down CPUs - requires root id */
	    RestrictProcessors = 1;
	    AppCPU = atoi(optarg);
	    break;
	case 't':	/* Set texture coord binding */
	    textureBind = atoi(optarg);
	    break;
	case 'T':	/* Benchmark time */
	    benchTime = atof(optarg);
	    break;
	case 'W':
	    if (!(4 == sscanf(optarg, "%ld,%ld,%ld,%ld",
			    &winXorigin, &winYorigin, &winXsize, &winYsize)))
		if (2 == sscanf(optarg, "%ld,%ld", &winXsize, &winYsize))
		{
		    winXorigin = winYorigin = 0;
		}
		else
		    if (1 == sscanf(optarg, "%ld", &winXsize))
		    {
			winYsize = winXsize;
			winXorigin = winYorigin = 0;
		    }
		    else
		    {
			winXsize = winYsize = 0;
		    }
	    break;
	case 'w':
	    wireframe = 1;
	    break;
	case 'z':	/* set zbuffer flag */
	    ZBuffer ^= 1;
	    break;
	case 'Z':
	    Normalize ^= 1;
	    break;
	case '?':
	default:
	    fprintf(stderr, "unknown cmdline option %c. exiting...\n", opt);
	    exit(0);
	}
    }

}


/******************************************************************************* 
 *			    Main
 *******************************************************************************
 */


/* visual attributes */
static int FBAttrs[] = {
    PFFB_RGBA,
    /*PFFB_DOUBLEBUFFER, */
    PFFB_DEPTH_SIZE, 24,
    PFFB_RED_SIZE, 8,
    PFFB_SAMPLES, 4,
    PFFB_STENCIL_SIZE, 1,
    NULL,
};

int
main(int argc, char *argv[])
{
    pfWindow	    *win;
    pfFrustum	    *frust;
    pfTexture*       tex;
    pfTexEnv*        tev;
    pfLightModel*    lm;
    pfMaterial	    *mt, *mt1;
    pfDispList	    *dl=0;
    pfGeoState	    *gs[2];
    int		    gscount=0;
    int		    *lengths=NULL;
    
    double           start, stop;
    int             frames = 0, nframes = 0, bl;
    int             i;
static    pfLight*         lt[8] = {
	    NULL, NULL, NULL, NULL, 
	    NULL, NULL, NULL, NULL
};
static	  pfVec4	clr={0.1, 0.1, 0.4, 1.0};    

    pfInit();

    /* Initialize Performer */
    pfInitState(NULL);
    pfInitClock(0.0);
    
    pfNotifyLevel(PFNFY_DEBUG);

    /* Defaults */
    gs[0] = pfNewGState(NULL);
    gs[1] = pfNewGState(NULL);
    geos = pfNewGSet(NULL);
    pfGSetPrimType(geos, primitive);
    pfGSetNumPrims(geos, numPrimitivesPerGSet);

    docmdline(argc, argv);
#if !defined(__linux__) && !defined(WIN32)
    if (RestrictProcessors)
    {
	int nprocs = sysmp(MP_NPROCS);
	if (schedctl(NDPRI, getpid(), NDPHIMAX) < 0)
	if ((sysmp(MP_RESTRICT, AppCPU) < 0))
	{
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"schedctl of process %d FAILED. "
		"Are you root?", getpid());
	} 
	if (AppCPU > nprocs)
	{
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"Have max of %d CPUs. ", nprocs);
	    AppCPU = nprocs - 1;
	}
	if (AppCPU) /* don't restrict CPU 0 */
	{
	    if ((sysmp(MP_RESTRICT, AppCPU) < 0))
	    {
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		    "RESTRICT of CPU %d for process %d FAILED. "
		    "Are you root?", AppCPU, getpid());
	    } else {
		sysmp(MP_ISOLATE, AppCPU);
		sysmp(MP_NONPREEMPTIVE, AppCPU);
	    }
	}
    } 
#endif

    if(flatShade)
	pfGSetDrawMode(geos, PFGS_FLATSHADE, 1);

    if (gsetwireframe)
	pfGSetDrawMode(geos, PFGS_WIREFRAME, 1);

    if (compile)
	pfGSetDrawMode(geos, PFGS_COMPILE_GL, 1);

    if(primitive == PFGS_TRISTRIPS || primitive == PFGS_LINESTRIPS ||
       primitive == PFGS_FLAT_TRISTRIPS || primitive == PFGS_FLAT_LINESTRIPS ||
       primitive == PFGS_POLYS)
    {
     	lengths = calloc(numPrimitivesPerGSet, sizeof(int));

	for (i=0; i < numPrimitivesPerGSet; i++)
	{
	    lengths[i] = stripLength;
	}

    	pfGSetNumPrims(geos, numPrimitivesPerGSet);
    	pfGSetPrimLengths(geos, lengths);

	if(primitive == PFGS_LINESTRIPS)
    	    numPrimitivesTotal = NumObjects * numPrimitivesPerGSet * (stripLength - 1);
	else
    	    numPrimitivesTotal = NumObjects * numPrimitivesPerGSet * (stripLength - 2);
    } 
    else
    {
    	pfGSetNumPrims(geos, numPrimitivesPerGSet);
    	numPrimitivesTotal = NumObjects * numPrimitivesPerGSet;
    }


    /* print options */
    fprintf(stderr, "shademodel(FLAT)=%d, DispList=%d, Compile=%d Normalize=%d", 
	    flatShade, dispList, compile, Normalize);
    if (!indexData) fprintf(stderr, "NonIndexed.\n");
    else  fprintf(stderr, "Indexed.\n");	    
    fprintf(stderr, "primType = %s\tc = %s, n = %s, t = %s\n",
	    primTable[primitive], bindTable[colorBind],
	    bindTable[normalBind], bindTable[textureBind]);
    

    if(primitive == PFGS_TRISTRIPS || primitive == PFGS_LINESTRIPS ||
       primitive == PFGS_FLAT_TRISTRIPS || primitive == PFGS_FLAT_LINESTRIPS ||
       primitive == PFGS_POLYS)
    {
	fprintf(stderr, "NumObjects=%ld, PrimitivesPerGSet=%ld, StripLength=%d, TotalPrimitives=%ld\n",
	    NumObjects, numPrimitivesPerGSet, stripLength, numPrimitivesTotal);
    } else
	fprintf(stderr, "NumGSets=%ld, PrimitivesPerGSet=%ld, TotalPrimitives=%ld\n",
	    NumObjects, numPrimitivesPerGSet, numPrimitivesTotal);

    if (useGeoStates) fprintf(stderr, "GeoStates=%d.\n", useGeoStates);
    if (numShademodel) freqShademodel = NumObjects / numShademodel;
    if (numLMbinds) freqLMbinds = NumObjects / numLMbinds;
    if (numTexbinds) freqTexbinds = NumObjects / numTexbinds;
    if (numShademodel || numLMbinds || numTexbinds)
    {
	fprintf(stderr, "numShademodel=%d, numLMbinds=%d, numTexbinds=%d\n", 
		numShademodel, numLMbinds, numTexbinds);
    }
	
    win = pfNewWin(NULL);
    pfWinOriginSize(win, winXorigin, winYorigin, 
		winXorigin + winXsize-1, winYorigin + winYsize-1);
    pfWinName(win, "benchgs");
    pfWinFBConfigAttrs(win, FBAttrs);
    pfOpenWin(win);
    pfInitGfx();
    pfPushIdentMatrix();
    
    if (!ZBuffer)
	glDisable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);
    frust = pfNewFrust(NULL);
    pfApplyFrust(frust);
    pfDelete(frust);
    if (Antialias)
	pfAntialias(PFAA_ON);
    
    pfCullFace(PFCF_OFF);
    pfClear(PFCL_DEPTH | PFCL_COLOR, clr);
 
    if (wireframe)
	pfEnable(PFEN_WIREFRAME);

    if (polyMode)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glColor3f(0.0f, 0.0f, 0.0f);

    mkdata_config(PFGS_COLOR4, colorBind);
    mkdata_config(PFGS_NORMAL3, normalBind);
    mkdata_config(PFGS_TEXCOORD2, textureBind);
    mkdata_config(MKDC_PRIMITIVE, primitive);
    mkdata_config(MKDC_PAGEALIGN, 1);
    mkdata_config(MKDC_INDEX, indexData);
    mkdata_config(MKDC_RANDOMIZE, randomize);
    obj_list = mkdata(numPrimitivesPerGSet, stripLength, NumObjects);

    /* set up Global State */    

    /* Set up texturing if needed */
    if (textureBind == PFGS_PER_VERTEX)
    {
	tex = pfNewTex(NULL);
	pfLoadTexFile(tex, "/usr/demos/data/textures/terrain.bw");
	tev = pfNewTEnv(NULL);
	if (useGeoStates)
	{
	    pfGStateMode(gs[0], PFSTATE_ENTEXTURE, 1);
	    pfGStateAttr(gs[0], PFSTATE_TEXTURE, tex);
	    pfGStateAttr(gs[0], PFSTATE_TEXENV, tev);
	    if (useGeoStates > 1)
	    {
		pfGStateMode(gs[1], PFSTATE_ENTEXTURE, 1);
		pfGStateAttr(gs[1], PFSTATE_TEXTURE, tex);
		pfGStateAttr(gs[1], PFSTATE_TEXENV, tev);
	    }
	} else
	{
	    pfApplyTex(tex);
	    pfApplyTEnv(tev);
	    pfEnable(PFEN_TEXTURE);
	}
    }

    /* Set up lighting if needed */
    if (normalBind != PF_OFF)
    {
	lt[0] = pfNewLight(NULL); lt[1] = 0;
	pfLightOn(lt[0]);
	lm = pfNewLModel(NULL);
	mt = pfNewMtl(NULL);
	pfMtlColorMode(mt, PFMTL_FRONT, colormode);
	mt1 = pfNewMtl(NULL);
	if (useGeoStates)
	{
	    pfGStateMode(gs[0], PFSTATE_ENLIGHTING, 1);
	    pfGStateAttr(gs[0], PFSTATE_FRONTMTL, mt);
	    pfGStateAttr(gs[0], PFSTATE_LIGHTS, lt);
	    pfGStateAttr(gs[0], PFSTATE_LIGHTMODEL, lm);
	    if (useGeoStates > 1)
	    {
		pfGStateMode(gs[1], PFSTATE_ENLIGHTING, 1);
		pfGStateAttr(gs[1], PFSTATE_FRONTMTL, mt);
		pfGStateAttr(gs[1], PFSTATE_LIGHTS, lt);
		pfGStateAttr(gs[1], PFSTATE_LIGHTMODEL, lm);
	    }
	} else
	{
	    pfApplyLModel(lm);
	    pfApplyMtl(mt);
	}
    }
    
    /* Set up special Geo States */

    geos_list = (pfGeoSet* *) malloc(NumObjects * sizeof(pfGeoSet*));
    /* Set attribute list base pointers */
    for (i = 0; i < NumObjects; i++)
    {
	geos_list[i] = pfNewGSet(NULL);
	pfCopy(geos_list[i], geos);
	if (freqShademodel) 
	{
	    if (colorBind == PFGS_PER_VERTEX || normalBind == PFGS_PER_VERTEX)
	    {
		colorBind = (colorBind ? PFGS_OVERALL : colorBind);
		normalBind = (normalBind ? PFGS_OVERALL : normalBind);
	    } else
	    {
		colorBind = (colorBind ? PFGS_PER_VERTEX : colorBind);
		normalBind = (normalBind ? PFGS_PER_VERTEX : normalBind);
	    }
	}
	pfGSetAttr(geos_list[i], PFGS_COLOR4, colorBind, 
				obj_list[i].cdata, obj_list[i].cindex);
	pfGSetAttr(geos_list[i], PFGS_NORMAL3, normalBind, 
				obj_list[i].ndata, obj_list[i].nindex);
	pfGSetAttr(geos_list[i], PFGS_TEXCOORD2, textureBind,
				obj_list[i].tdata, obj_list[i].tindex); 
	pfGSetAttr(geos_list[i], PFGS_COORD3, PFGS_PER_VERTEX, 
				obj_list[i].vdata, obj_list[i].vindex);
	if (lengths)
	    pfGSetPrimLengths(geos_list[i], lengths);

	if (compile)
	    pfGSetDrawMode(geos_list[i], PFGS_COMPILE_GL, 1);

	if (useGeoStates) {
	    pfGSetGState(geos_list[i], gs[gscount]);
	    /* fprintf(stderr,"geostate: 0x%x\n", gs[gscount]); */
	    if (useGeoStates > 1)
		gscount = 1 - gscount;
	}

    }

    if (dispList)
    {
    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Making Performer DispList.");
	dl = pfNewDList(PFDL_FLAT, 100000, NULL);
	pfOpenDList(dl);
	for (i = 0; i < NumObjects; i++)
	    pfDrawGSet(geos_list[i]);
	pfCloseDList();
    }
    
    /* Draw one to set shademodel */
    pfDrawGSet(geos_list[0]);
	
    if (oneFrame)
	exit(0);

    if (!Normalize)
	glDisable(GL_NORMALIZE);

    /* New GL object to compare */
    if (obj)
    {
	glNewList(1, GL_COMPILE);
	for (i = 0; i < NumObjects; i++)
	    pfDrawGSet(geos_list[i]);
	glEndList();
    }
	glFinish();
    fprintf(stderr, "calculate timing\n");

    start = pfGetTime();

    /* Compute number of frames it takes roughly benchTime/2 seconds to draw */
    nframes = 0;
    while ((pfGetTime() - start) < ((benchTime + 1.0) / 2.0))
    {
	if (obj)
	{
	    glCallList(1);
	} else if (dl)
	{
		pfDrawDList(dl);
	} else
	{
	    for (i = 0; i < NumObjects; i++)
		pfDrawGSet(geos_list[i]);
	}
	nframes++;
    }

    nframes *= 2;	/* Benchmark roughly every 'benchTime' seconds */
    bl = benchLoop;
    fprintf(stderr, "Benchmarking...\n");
    /* Benchmark in 'benchTime' intervals for 'benchLoop' times */
    if (!dl)
    {
	glFinish();
	start = pfGetTime();
	while (bl)
	{
	    
	    for (i = 0; i < NumObjects; i++)
	    {
		pfDrawGSet(geos_list[i]);
	    }
	    frames++;
	    if (!(frames == nframes))
		continue;
	    
	    {
		glFinish();
		stop = pfGetTime();
		printRates(start, stop, nframes, numPrimitivesTotal);
		frames = 0;
		bl--;
		start = pfGetTime();
	    }
	}
    } else {
	glFinish();
	start = pfGetTime();
	while (bl)
	{
	    pfDrawDList(dl);
	    frames++;
	    if (!(frames == nframes))
		continue;
	    
	    {
		glFinish();
		stop = pfGetTime();
		printRates(start, stop, nframes, numPrimitivesTotal);
		frames = 0;
		bl--;
		start = pfGetTime();
	    }
	}
    }
    if (obj)
    {
	bl = benchLoop;
	fprintf(stderr, "Rendering comparison GL object\n");
	/* Benchmark in 'benchTime' intervals for 'benchLoop' times */
	glFinish();
	start = pfGetTime();
	while (bl)
	{
	    glCallList(1);
	    frames++;
	    if (!(frames == nframes))
		continue;
	    
	    {
		glFinish();
		stop = pfGetTime();
		printRates(start, stop, nframes, numPrimitivesTotal);
		frames = 0;
		bl--;
		start = pfGetTime();
	    }
	}
    }
    exit(0);
    return 0;
}


static void
printRates (double start, double stop, int frames, int num)
{
    double	elapsed	= stop - start;
    double	prim_rate;

    if (elapsed <= 0.0f)
	return;

    prim_rate = ((double)num*(double)frames)/elapsed;

    fprintf(stderr, "frames=%ld, sec=%.3f, frames/sec = %.3f ",
	frames, elapsed, frames/elapsed);

    if (prim_rate >= 1000000.0f)
	fprintf(stderr, " prims/sec = %.3fM\n", prim_rate / 1000000.0f);
    else
    if (prim_rate >= 1000.0)
	fprintf(stderr, " prims/sec = %.3fK\n", prim_rate / 1000.0f);
    else
	fprintf(stderr, " prims/sec = %.3f\n", prim_rate);
}

/*
 * file: mkdata.c 
 * --------------
 *
 * $Revision: 1.6 $
 *
 * Create data structures for use in geosets
 */


/*******************************************************************************
 *				Global Static Variables
 *******************************************************************************
 */


/*
 * Geometry Data 
 */
static const pfVec4   scolors[] ={{1., 0., 0., 0.},
			    {0., 1., 0., 0.},
			    {0., 0., 1., 0.},
			    {1., 1., 1., 0.}};

static const pfVec3   snorms[] ={{0., 0., 1.},
			   {0., 0., -1.},
			   {0., 0., 1.},
			   {0., 0., -1.}};

static const pfVec2   stexCoords[] ={{0., 0.},
			       {1., 0.},
			       {1., 1.},
			       {0., 1.}};

static const pfVec3   scoords[] ={{-.1, -.1, -2},
			    {.1, -.1, -2},
			    {0.,.1, -2},
			    {-.2,.1, -2}};

/*******************************************************************************
 *				Data Init Routines
 *******************************************************************************
 */


#define PAGE_ALIGN(datap, type)                                         \
    datap = (type *) (((int) datap) + 0xfff);                           \
    datap = (type *) (((int) datap) & 0xfffff000);


static void 
alloc_data(void)
{
    int             index_size = 0;
    int             totalv,  numv = NumVertsPerObj;
    int             i;
    float          *vp = 0, *np = 0, *cp = 0, *tp = 0;
    ushort          *vip = 0, *nip = 0, *cip = 0, *tip = 0;

    obj_list = (obj_t *) calloc(NumObjects, sizeof(obj_t));
    for (i = 0; i < NumObjects; i++)
    {
	obj_list[i].num_verts = numv;
    }
    
    totalv = NumObjects * NumVertsPerObj;
    index_size = (totalv * sizeof(ushort)) + 4096;

    if (IndexData) {
	vip = (ushort *) malloc(totalv * index_size);
	if (PageAlign)
	{
	    PAGE_ALIGN(vip, ushort);
	}
    }
    vp = (float *) malloc(totalv * 3 * sizeof(float) + 4096);
    if (PageAlign)
    {
	PAGE_ALIGN(vp, float);
    }

    if (vColor)
    {
	if (IndexData) {
	    cip = (ushort *) malloc(totalv * index_size);
	    if (PageAlign)
	    {
		PAGE_ALIGN(cip, ushort);
	    }
	}
	cp = (float *) malloc(totalv * 4 * sizeof(float) + 4096);
	if (PageAlign)
	{
	    PAGE_ALIGN(cp, float);
	}
    }
    if (vNormal)
    {
	if (IndexData) {
	    nip = (ushort *) malloc(totalv * index_size);
	    if (PageAlign)
	    {
		PAGE_ALIGN(nip, ushort);
	    }
	}
	np = (float *) malloc(totalv * 3 * sizeof(float) + 4096);
	if (PageAlign)
	{
	    PAGE_ALIGN(np, float);
	}
    }
    if (vTexture)
    {
	if (IndexData) {
	    tip = (ushort *) malloc(totalv * index_size);
	    if (PageAlign)
	    {
		PAGE_ALIGN(tip, ushort);
	    }
	}
	tp = (float *) malloc(totalv * 2 * sizeof(float) + 4096);
	if (PageAlign)
	{
	    PAGE_ALIGN(tp, float);
	}
    }


    /* assign data ptrs */
    for (i = 0; i < NumObjects; i++)
    {
	numv = obj_list[i].num_verts;
	obj_list[i].vdata = vp;
	vp += numv * 3;
	if (IndexData) {
	    obj_list[i].vindex = vip;
	    vip += numv;
	}
	if (vNormal)
	{
	    obj_list[i].ndata = np;
	    np += numv * 3;
	    if (IndexData) {
		obj_list[i].nindex = nip;
		nip += numv;
	    }
	}
	if (vColor)
	{
	    obj_list[i].cdata = cp;
	    cp += numv * 4;
	    if (IndexData) {
		obj_list[i].cindex = cip;
		cip += numv;
	    }
	}
	if (vTexture)
	{
	    obj_list[i].tdata = tp;
	    tp += numv * 2;
	    if (IndexData) {
		obj_list[i].tindex = tip;
		tip += numv;
	    }
	}
    }
}

static void 
init_geom_data(void)
{
    int             i, j;
    float          *vptr, *cptr, *nptr, *tptr, stheta, ctheta;

    if (primitive == PFGS_FLAT_TRISTRIPS || primitive == PFGS_TRISTRIPS)
    {
	for (i = 0; i < NumObjects; i++)
	{
	    int 	nv = NumVertsPerObj - 1, k;

	    vptr = obj_list[i].vdata;
	    nptr = obj_list[i].ndata;
	    cptr = obj_list[i].cdata;
	    tptr = obj_list[i].tdata;

	    for (j = 0; j < nv; j++, nv--)
	    {
		for (k = 0; k<2; k++)
		{
		    if (k)
			pfSinCos( (float)j / (float)NumVertsPerObj * 360.0f, &stheta, &ctheta);
		    else
			pfSinCos( (float)nv / (float)NumVertsPerObj * 360.0f, &stheta, &ctheta);

		    if (vColor)
		    {
			pfSetVec4(cptr, PF_ABS(stheta), PF_ABS(ctheta), 0.0f, 1.0f);
			cptr += 4;
		    }
		    if (vNormal)
		    {
			pfSetVec3(nptr, stheta, ctheta, 0.0f);
			nptr += 3;
		    }
		    if (vTexture)
		    {
			pfSetVec2(tptr, stheta, ctheta);
			tptr += 2;
		    }
		    pfSetVec3(vptr, stheta, ctheta, -3.0f);
		    vptr += 3;
		}
	    }
	    if (j == nv)
	    {

		pfSinCos( (float)j / (float)NumVertsPerObj * 360.0f, &stheta, &ctheta);
		if (vColor)
		{
		    pfSetVec4(cptr, PF_ABS(stheta), PF_ABS(ctheta), 0.0f, 1.0f);
		    cptr += 4;
		}
		if (vNormal)
		{
		    pfSetVec3(nptr, stheta, ctheta, 0.0f);
		    nptr += 3;
		}
		if (vTexture)
		{
		    pfSetVec2(tptr, stheta, ctheta);
		    tptr += 2;
		}
		pfSetVec3(vptr, stheta, ctheta, -3.0f);
		vptr += 3;
	    }
	}
    }
    else if (primitive == PFGS_QUADS)
    {
	for (i = 0; i < NumObjects; i++)
	{
	    vptr = obj_list[i].vdata;
	    nptr = obj_list[i].ndata;
	    cptr = obj_list[i].cdata;
	    tptr = obj_list[i].tdata;

	    for (j = 0; j < NumVertsPerObj; j++)
	    {
		pfSinCos( (float)j / (float)NumVertsPerObj * 360.0f, &stheta, &ctheta);		    
		if (vColor)
		{
		    pfSetVec4(cptr, PF_ABS(stheta), PF_ABS(ctheta), 0.0f, 1.0f);
		    cptr += 4;
		}
		if (vNormal)
		{
		    pfSetVec3(nptr, stheta, ctheta, 0.0f);
		    nptr += 3;
		}
		if (vTexture)
		{
		    pfSetVec2(tptr, stheta, ctheta);
		    tptr += 2;
		}
		if ((j % 4) == 0)
		    pfSetVec3(vptr, 0.0f, 0.0f, -3.0f);
		else
		    pfSetVec3(vptr, stheta, ctheta, -3.0f);
		vptr += 3;
	    }
	}
    }
    else if (primitive == PFGS_TRIS)
    {
	for (i = 0; i < NumObjects; i++)
	{
	    vptr = obj_list[i].vdata;
	    nptr = obj_list[i].ndata;
	    cptr = obj_list[i].cdata;
	    tptr = obj_list[i].tdata;

	    for (j = 0; j < NumVertsPerObj; j++)
	    {
		pfSinCos( (float)j / (float)NumVertsPerObj * 360.0f, &stheta, &ctheta);		    
		if (vColor)
		{
		    pfSetVec4(cptr, PF_ABS(stheta), PF_ABS(ctheta), 0.0f, 1.0f);
		    cptr += 4;
		}
		if (vNormal)
		{
		    pfSetVec3(nptr, stheta, ctheta, 0.0f);
		    nptr += 3;
		}
		if (vTexture)
		{
		    pfSetVec2(tptr, stheta, ctheta);
		    tptr += 2;
		}
		if ((j % 3) == 0)
		    pfSetVec3(vptr, 0.0f, 0.0f, -3.0f);
		else
		    pfSetVec3(vptr, stheta, ctheta, -3.0f);
		vptr += 3;
	    }
	}
    }
    else if (primitive == PFGS_POLYS)
    {
	for (i = 0; i < NumObjects; i++)
	{
	    vptr = obj_list[i].vdata;
	    nptr = obj_list[i].ndata;
	    cptr = obj_list[i].cdata;
	    tptr = obj_list[i].tdata;

	    for (j = 0; j < NumVertsPerObj; j++)
	    {
		pfSinCos( (float)j / (float)NumVertsPerObj * 360.0f, &stheta, &ctheta);		    
		if (vColor)
		{
		    pfSetVec4(cptr, PF_ABS(stheta), PF_ABS(ctheta), 0.0f, 1.0f);
		    cptr += 4;
		}
		if (vNormal)
		{
		    pfSetVec3(nptr, stheta, ctheta, 0.0f);
		    nptr += 3;
		}
		if (vTexture)
		{
		    pfSetVec2(tptr, stheta, ctheta);
		    tptr += 2;
		}
/*		if ((j % stripLength) == 0)
		    pfSetVec3(vptr, 0.0f, 0.0f, -3.0f);
		else
*/
		    pfSetVec3(vptr, stheta, ctheta, -3.0f);
		vptr += 3;
	    }
	}
    }
    else 
    {
	for (i = 0; i < NumObjects; i++)
	{
	    vptr = obj_list[i].vdata;
	    nptr = obj_list[i].ndata;
	    cptr = obj_list[i].cdata;
	    tptr = obj_list[i].tdata;

	    for (j = 0; j < NumVertsPerObj; j++)
	    {
		pfSinCos( (float)j / (float)NumVertsPerObj * 360.0f, &stheta, &ctheta);
		if (vColor)
		{
		    pfSetVec4(cptr, PF_ABS(stheta), PF_ABS(ctheta), 0.0f, 1.0f);
		    cptr += 4;
		}
		if (vNormal)
		{
		    pfSetVec3(nptr, stheta, ctheta, 0.0f);
		    nptr += 3;
		}
		if (vTexture)
		{
		    pfSetVec2(tptr, stheta, ctheta);
		    tptr += 2;
		}
		pfSetVec3(vptr, stheta, ctheta, -3.0f);
		vptr += 3;
	    }
	}
    }
}

static void 
init_indices(void)
{
    ushort             	i;
    int			j;

    for (j = 0; j < NumObjects; j++)
    {
	for (i = 0; i < obj_list[j].num_verts; i++)
	{
	    obj_list[j].vindex[i] = i;
	    if (vNormal)
		obj_list[j].nindex[i] = i;
	    if (vColor)
		obj_list[j].cindex[i] = i;
	    if (vTexture)
		obj_list[j].tindex[i] = i;
	}
    }
}

static void 
init_data(void)
{
    init_geom_data();
    if (IndexData)
	init_indices();
}


/*******************************************************************************
 *			Entry Routines
 *******************************************************************************
 */

void mkdata_config(int prop, int val)
{
    switch(prop)
    {
	case MKDC_PAGEALIGN: PageAlign = val; break;
	case MKDC_RANDOMIZE: Randomize = val; break;
	case MKDC_PRIMITIVE: PrimitiveType = val; break;
	case MKDC_INDEX: IndexData = val; break;
	case PFGS_COLOR4: vColor = val; break;
	case PFGS_NORMAL3: vNormal = val; break;
	case PFGS_TEXCOORD2: vTexture = val; break;
	default: break;
    }
}

obj_t *
mkdata(int prims_per_obj, int strip_length, int num_objs)
{
#ifndef WIN32
    srandom(1);
#endif

    NumPrimsPerObj = prims_per_obj;
    NumObjects = num_objs;
    StripLength = strip_length;
    switch (PrimitiveType)
    {
    case PFGS_QUADS:
	NumVertsPerObj = NumPrimsPerObj * 4;
	break;
    case PFGS_TRIS:
	NumVertsPerObj = NumPrimsPerObj * 3;
	break;
    case PFGS_TRISTRIPS:
    case PFGS_LINESTRIPS:
    case PFGS_FLAT_LINESTRIPS:
    case PFGS_FLAT_TRISTRIPS:
    case PFGS_POLYS:
	NumVertsPerObj = NumPrimsPerObj * (strip_length + 2);
	break;
    case PFGS_LINES:
	NumVertsPerObj = NumPrimsPerObj * 2;
	break;
    case PFGS_POINTS:
	NumVertsPerObj = NumPrimsPerObj;
	break;

    }
    alloc_data();
    init_data();

    return (obj_list);

}

#define STUBS

#ifdef STUBS
#define STUBS

void bgntmesh(void) {}
void endtmesh(void) {}
void texdef2d(long t, long p, long w, long h,
		const ulong *i, long np, const float *pr) {}
void texbind(long t, long p) {}
void tevdef(long t, long p, const float *pr) {}
void tevbind(long t, long p) {}
void c3f(const float p[3]) {}
void c4f(const float p[4]) {}
void n3f(const float p[3]) {}
void t2f(const float p[2]) {}
void v3f(const float p[3]) {}
#endif
