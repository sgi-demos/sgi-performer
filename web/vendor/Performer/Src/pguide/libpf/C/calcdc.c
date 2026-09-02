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
 * calcd.c: Performer program to compute depth complexity
 *
 *
 * Commandline options:
 * --------------------
 *
 * -e h,p,r		- set viewing direction
 * -f filename		- read file of positions: x y z h p r
 * -H fovh,fovv		- set horizontal and vertical FOV
 * -F filepath		- set pfFilePath
 * -p x,y,z		- set view position
 * -v n			- leave scene up for viewing for n secs
 * -W wsize[,wsize]	- set window dimensions
 * -Z near,far		- set distances to near and far clip planes
 *
 *
 * $Revision: 1.7 $ $Date: 2002/11/14 00:02:43 $ 
 *
 */

#include <stdlib.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h> /* for cmdline handler */
#endif
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#define MAX_VIEWS 100

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
    pfVec2	fov;
    float	mynear, myfar;
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
    float	LODscale;
} SharedData;

static void InitViewTable(void);

static SharedData *Shared;


static int	    WinSizeX=640, WinSizeY=480;
static int	    ViewPosInited=0, NearFarInited=0;
static int	    HaveViewTable = 0;
static int	    Wait=0;
static int	    viewnum = 1;
static int	    curviewnum = 0;

static pfVec3	    ptable[MAX_VIEWS], otable[MAX_VIEWS];
static char	    vname[PF_MAXSTRING];

/*
 *	Usage() -- print usage advice and exit. This
 *      procedure is executed in the application process.
 */
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: calcdc file.ext ...\n");
    exit(1);
}

static char progName[80];

/*
 *	main() -- program entry point. this procedure
 *	is executed in the application process.
 */

char OptionStr[] = "e:H:F:p:s:v:W:z:";

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
	case 'e':
            if ((sscanf(optarg, "%f,%f,%f",
                    &Shared->initView.hpr[PF_H],
                    &Shared->initView.hpr[PF_P],
                    &Shared->initView.hpr[PF_R]) == 3))
	    {
		PFCOPY_VEC3(Shared->view.hpr, Shared->initView.hpr);
	    }
	    else
		Usage();
            break;
	case 'f':
	    if (!sscanf(optarg, "%s", vname) == 1)
		fprintf(stderr, "Warning: unable to load view file\n");
	    else
		InitViewTable();
	    break;
	case 'H': /* set Field of View' */
	     if(!(sscanf(optarg, "%f,%f", 
		      &Shared->fov[0],
		      &Shared->fov[1]) == 2))
		Usage();
	    break;
	case 'F':
	    pfFilePath(optarg);
	    break;
	case 'p': /* initial position */
	    if (sscanf(optarg, "%f,%f,%f",
                    &Shared->initView.xyz[PF_X],
                    &Shared->initView.xyz[PF_Y],
                    &Shared->initView.xyz[PF_Z]) == 3)
	    {
		PFCOPY_VEC3(Shared->view.xyz, Shared->initView.xyz);
		ViewPosInited = TRUE;
	    }
	    break;
	case 'W': /* Set the window size */
	    if (sscanf(optarg, "%ld,%ld", &WinSizeX, &WinSizeY) != 2)
	    {
		if (sscanf(optarg, "%ld", &WinSizeX) == 1)
		{
		    WinSizeY = WinSizeX;
		}
		else
		    WinSizeX = -1;
	    } 
	case 's': /* run app in single buffer */
	    Shared->LODscale = atof(optarg);
	    break;
	case 'v':
	    Wait = atoi(optarg);
	    break;
	case 'z':
	    {
		float mynear, myfar;
		if (sscanf(optarg, "%f,%f", &mynear, &myfar) == 2)
		{
		    Shared->mynear = mynear;
		    Shared->myfar = myfar;
		    NearFarInited = 1;
		}
		else
		    Usage();
	    }
	    
	    break;
	}
    }
    return optind;
}

void
initShared(void)
{
    
    /* allocate shared before fork()'ing parallel processes */
    Shared = (SharedData*)pfCalloc(1,  sizeof(SharedData), pfGetSharedArena());
    Shared->fov[0]	= 45.0f;
    Shared->fov[1]	= -1.0f;
    Shared->mynear	= 0.1f;
    Shared->myfar	= 1000.0f;
    Shared->sceneSize = 10000.0f;
}

int
main (int argc, char *argv[])
{
    float	    t = 0.0f;
    pfScene	    *scene;
    pfNode	    *root;
    pfPipe	    *p;
    pfPipeWindow    *pw;
    pfChannel	    *chan;
    pfFrameStats    *fstats;
    pfBox           bbox;
    pfSphere	    bsphere;
    int		    arg, found;

    
    arg = docmdline(argc, argv);

    /* Initialize Performer */
    pfInit();	
    
    pfFilePath(".:/usr/share/Performer/data");
    initShared();

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess( 0 );
    
    /* specify number of hw stencil bits for depth complexity computation */
    pfStatsHwAttr(PFSTATSHW_FILL_DCBITS, 4);			

    /* Load all loader DSO's before pfConfig() forks */
    for (found = arg; found < argc; found++)
	pfdInitConverter(argv[found]);

    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     */
    pfConfig();			    

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

    if (!found)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "No database - exiting.");
	exit(0);
    }
        
    /* Attach loaded file to a new pfScene. */
    scene = pfNewScene();
    pfAddChild(scene, root);
    /* Create a pfLightSource and attach it to scene. */
    pfAddChild(scene, pfNewLSource());
    
    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinType(pw, PFPWIN_TYPE_X | PFWIN_TYPE_STATS);
    pfPWinName(pw, "OpenGL Performer");
    pfPWinOriginSize(pw, 0, 0, WinSizeX, WinSizeY);
    /* Open and configure the GL window. */
    pfOpenPWin(pw);
    pfFrame();
    
    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanFOV(chan, Shared->fov[0], Shared->fov[1]);
    pfChanLODAttr(chan, PFLOD_SCALE, Shared->LODscale);
    fstats = pfGetChanFStats(chan);
    
    /* determine extent of scene's geometry */
    pfuTravCalcBBox(scene, &bbox);
    pfGetNodeBSphere (root, &bsphere);
    
    if (!NearFarInited)
	Shared->myfar = 10.0f * bsphere.radius;
    
    if (!ViewPosInited)
    {
	float sceneSize;

	/* view point at center of bbox */
	pfAddVec3(Shared->view.xyz, bbox.min, bbox.max);
	pfScaleVec3(Shared->view.xyz, 0.5f, Shared->view.xyz);
	
	/* find max dimension */
	sceneSize = bbox.max[PF_X] - bbox.min[PF_X];
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	sceneSize = PF_MIN2(sceneSize, 0.5f * Shared->myfar);
	Shared->sceneSize = sceneSize;
	
	/* offset so all is visible */
	Shared->view.xyz[PF_Y] -=      sceneSize;
	Shared->view.xyz[PF_Z] += 0.25f*sceneSize;
	PFCOPY_VEC3(Shared->initView.xyz, Shared->view.xyz);
	PFCOPY_VEC3(Shared->initView.hpr, Shared->view.hpr);	
    } 
    pfChanView(chan, Shared->initView.xyz, Shared->initView.hpr);
    pfChanNearFar(chan, Shared->mynear, Shared->myfar);
    
    /* draw first frame to init */
    pfFrame(); 
    
    /* turn on stats */
    pfFStatsClass(fstats, PFSTATSHW_ENGFXPIPE_FILL, PFSTATS_ON);
    pfFStatsClassMode(fstats, PFSTATSHW_GFXPIPE_FILL,
                                     PFSTATSHW_GFXPIPE_FILL_DEPTHCMP |
                                     PFSTATSHW_GFXPIPE_FILL_TRANSPARENT,
                                     PFSTATS_ON);

    fprintf(stdout, "Bits of accuracy for depth complexity: %d\n", 
	    (int) pfGetStatsHwAttr(PFSTATSHW_FILL_DCBITS));

    for (; curviewnum < viewnum; curviewnum++)
    {
	if (HaveViewTable)
	{
	    PFCOPY_VEC3(Shared->view.xyz, ptable[curviewnum]);    
	    PFCOPY_VEC3(Shared->view.hpr, otable[curviewnum]);    
	    pfChanView(chan, Shared->view.xyz, Shared->view.hpr);
	}

	/* draw frame of fill stats */
	pfFrame();
	/* calc stats for previous frame */
	pfFrame(); 
	
	/* get stats */
	{
	    pfStatsValFill fillstats;
	    
	     /* query fill stats from first frame */
	    pfQueryFStats(fstats, PFFSTATS_BUF_PREV | PFSTATSVAL_GFXPIPE_FILL, &fillstats, 
			 sizeof(fillstats));
	    
	    fprintf(stdout, "Pos: %f %f %f  MPixels : %.3f, DC: %.3f\n\n", 
		Shared->initView.xyz[0], 
		Shared->initView.xyz[1], 
		Shared->initView.xyz[2],
		fillstats.pixels / 1e6, fillstats.depthComplexity);
	}
    }

    if (Wait)
#ifndef WIN32
	sleep(Wait);
#else
        _sleep(Wait * 1000);
#endif

    
    /* Terminate parallel processes and exit. */
    pfExit();
}


static void
InitViewTable(void)
{
    FILE *vfile;
    char buf[128];
    float x,y,z,h,p,r;

    if (vname == NULL)
    {
        fprintf(stderr, "Warning: no view file found\n");
        exit(1);
    }
    if ((vfile = fopen(vname, "r")) == NULL)
    {
        fprintf(stderr, "Warning: could not open view file <%s>\n", vname);
        exit(1);
    }
    fprintf(stderr, "Loading view file <%s>\n", vname);
    while (fgets(buf, 127, vfile) != NULL)
    { 
    	sscanf(&buf[15], "%f,%f,%f) HPR(%f,%f,%f", &x,&y,&z,&h,&p,&r);
	PFSET_VEC3(ptable[viewnum], x,y,z);
	PFSET_VEC3(otable[viewnum], h,p,r);
        viewnum++;
    } 
    Shared->quitCount = viewnum-1;

    if (curviewnum >= viewnum-1)
	return;
    PFCOPY_VEC3(Shared->view.xyz, ptable[curviewnum]);   
    PFCOPY_VEC3(Shared->view.hpr, otable[curviewnum]);   
    PFCOPY_VEC3(Shared->initView.xyz, ptable[curviewnum]);
    PFCOPY_VEC3(Shared->initView.hpr, otable[curviewnum]);
    curviewnum++;

}
