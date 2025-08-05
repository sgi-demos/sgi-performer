//
// Copyright 1999, 2000, Silicon Graphics, Inc.
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
// draw.C
//

// 
// Run-time controls:
//        F1-key: profile
//    Left-mouse: advance
//  Middle-mouse: stop
//   Right-mouse: retreat
//
// Command line options to try:
// patchy fog
//   $PFPATH/town_ogl_pfi.pfb -bf -bfi 1 -fd 3 -bfs 1
//   $PFPATH/town_ogl_pfi.pfb -bf -bfi 2 -fd 2 -bfr 1 -bfv 0.5
//   $PFPATH/town_ogl_pfi.pfb -bf -bfi 1 -fm exp -bfs 1 -fd 0.003
// animated patchy fog
//   $PFPATH/town_ogl_pfi.pfb -bf -bfi 3 -fd 0.6
// layered fog
//   $PFPATH/town_ogl_pfi.pfb -lf -fd 0.5
//   $PFPATH/town_ogl_pfi.pfb -lf -fd 0.001 -fm exp2 
//   $PFPATH/town_ogl_pfi.pfb -clf -fd 1
//
// NOTE: on a machine with no hw 3D texture support (such as 02) add 
//       option -f2d when a layered fog is defined
// 
// combined
//   $PFPATH/town_ogl_pfi.pfb -blf -fd 2 -bfi 1 -fdb 0.02 -bfs 0.5
//   $PFPATH/town_ogl_pfi.pfb -blf -fd 0.0015 -fm exp 
//
// Set PFPATH to /usr/share/Performer/data/town/
//

#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef __linux__
#include <pthread.h>
#define __USE_UNIX98
#endif
#include <signal.h> // for sigset for forked X event handler process 
#include <getopt.h> // for cmdline handler 
#include <X11/keysym.h>

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfPipe.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfVolFog.h>
#include <Performer/pr/pfMaterial.h>

#include <Performer/pr/pfLight.h>

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#include "draw.h"
#include "fog.h"
#include "smoke.h"

int sizex = 1024, sizey = 768; // XXX make this static and use Shared 

#define NO_FOG          0
#define OPENGL_FOG      1
#define LAYERED_FOG     2
#define LAYERED_MULTICOLORED_FOG  4
#define FOG_BOUNDARIES  8

#define MAX_FILES 50

static int fogType = NO_FOG;

///////////////////////////////////////////////////////////////////////////
//
// structure that resides in shared memory so that the
// application, cull, and draw processes can access it.

typedef struct
{
    pfPipeWindow    *pw;
    int		    exitFlag;
    int		    inWindow, reset;
    float	    mouseX, mouseY;
    int		    winSizeX, winSizeY;
    int		    mouseButtons;
    pfCoord	    view, viewOrig;
    float	    accelRate;
    float	    sceneSize;
    int		    drawStats;
    int		    XInputInited;
    pfVolFog        *volFog;
    int             animateSmoke;
    pfNode          *smokeNode;
} SharedData;

static SharedData *Shared;

//
// APP process variables

// write out scene upon read-in - uses pfDebugPrint 
static int WriteScene = 0;
static int FullScreen = 0;
static int WinType = PFPWIN_TYPE_X;
static int NoBorder = 0;
static int ForkedXInput = 0;

static int fogIndex = 1;
static int fogMode = PFVFOG_LINEAR;
static int force2d = 0;
static float fogDensity = 1, fogHeight = 0;
static float fogDensityBias = 0;
static float fogRandomness = 0;
static int fasterPatchy = 0;

float fogScale = 0.2; 
static float fogVariance = 0;

// light source created and updated in DRAW-process 
//static pfLight *Sun;

static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static void UpdateView(void);
static void InitXInput(pfWSConnection dsp);
static void DoXInput(void);
static void GetXInput(Display *dsp);


static void Usage(void)
{
    printf("The program draws a scene with layered or patchy fog.\n\n"
	   "USAGE: volfog [options] <database files>\n"
	   "  -glf     OpenGL fog\n"
	   "  -bf      fog specified by its boundaries\n"
	   "  -lf      layered fog\n"
	   "  -clf     multicolor layered fog\n"
	   "\n"
	   "  -fd F    fog density \n"
	   "  -fm m    fog mode (m=linear,exp,exp2)\n"
	   "\n"
	   "  -s x y   window size\n"
	   "  -fs      fullscreen\n"
	   "\n"
	   "  -f2d     force the use of 2d textures (layered fog)\n"
	   "  -fdb F   fog density bias\n"
	   "  -bfi N   fog index (different boundaries: 0(layer),1(box),2(ground))\n"
	   "  -bfi F   fog variance (irregularity of ground fog 0-1)\n"
	   "  -bfh F   fog height (ground fog only)\n"
	   "  -bfr F   fog randomness 0-1 (ground fog only)\n"
	   "  -bfs F   fog resolution (to avoid overflows in color buffer)\n"
	   "  -bff     faster patchy fog\n"
	   "\nCommand line options to try:\n"
	   "patchy fog\n"
	   "  $PFPATH/town_ogl_pfi.pfb -bf -bfi 1 -fd 3 -bfs 1\n"
	   "  $PFPATH/town_ogl_pfi.pfb -bf -bfi 2 -fd 2 -bfr 1 -bfv 0.5\n"
	   "  $PFPATH/town_ogl_pfi.pfb -bf -bfi 1 -fm exp -bfs 1 -fd 0.003\n"
	   "animated patchy fog\n"
	   "  $PFPATH/town_ogl_pfi.pfb -bf -bfi 3 -fd 0.6\n"
	   "layered fog\n"
	   "  $PFPATH/town_ogl_pfi.pfb -lf -fd 0.5\n"
	   "  $PFPATH/town_ogl_pfi.pfb -lf -fd 0.001 -fm exp2 \n"
	   "  $PFPATH/town_ogl_pfi.pfb -clf -fd 1\n\n"
	   "NOTE: on a machine with no hw 3D texture support (such as 02) add \n"
	   "      option -f2d when a layered fog is defined\n"
	   "\ncombined\n"
	   "  $PFPATH/town_ogl_pfi.pfb -blf -fd 2 -bfi 1 -fdb 0.02 -bfs 0.5\n"
	   "  $PFPATH/town_ogl_pfi.pfb -blf -fd 0.0015 -fm exp \n"
	   "\nSet PFPATH to /usr/share/Performer/data/town/\n"
	   "\nRun-time controls:\n"
	   "       F1-key: profile\n"
	   "   Left-mouse: advance\n"
	   " Middle-mouse: stop\n"
	   "  Right-mouse: retreat\n"
	   "        t-key: faster/slower patchy fog\n"
	   );
}

//
//	docmdline() -- use getopt to get command-line arguments, 
//	executed at the start of the application process.


static int
docmdline(int argc, char *argv[], int *fargc, char *fargv[])
{
    int i, found;

    if(argc <= 1) {
	Usage();
	exit(-1);
    }

    found = 0;

    /* process commmand line args */
    for (i =1 ; i < argc; ++i) {
	if(argv[i][0] != '-') {
	    if(found == MAX_FILES) {
		fprintf(stderr,
			"Cannot read more than %d files!\n", MAX_FILES);
		continue;
	    }
	    fargv[found++] = argv[i];
	    continue; //skip over databases
	}

        if (!strcmp("-s", argv[i])) {
	    sizex = atoi(argv[++i]);
	    sizey = atoi(argv[++i]);
	}
	else if (!strcmp("-fs", argv[i])) {
	    FullScreen = 1;
	}
	else if (!strcmp("-f2d", argv[i])) {
	    force2d = 1;
	}
	else if (!strcmp("-glf", argv[i])) {
	    fogType = OPENGL_FOG;
	}
	else if (!strcmp("-bf", argv[i])) {
	    fogType = FOG_BOUNDARIES;
	}
	else if (!strcmp("-lf", argv[i])) {
	    fogType = LAYERED_FOG;
	}
	else if (!strcmp("-blf", argv[i])) {
	    fogType = LAYERED_FOG | FOG_BOUNDARIES;
	}
	else if (!strcmp("-clf", argv[i])) {
	    fogType = LAYERED_MULTICOLORED_FOG;
	}
	else if (!strcmp("-bfi", argv[i])) {
	    fogIndex = atoi(argv[++i]);
	}
	else if (!strcmp("-bfv", argv[i])) {
	    fogVariance = atof(argv[++i]);
	}
	else if (!strcmp("-bff", argv[i])) {
	    fasterPatchy = 1;
	}
	else if (!strcmp("-fd", argv[i])) {
	    fogDensity = atof(argv[++i]);
	}
	else if (!strcmp("-fdb", argv[i])) {
	    fogDensityBias = atof(argv[++i]);
	}
	else if (!strcmp("-bfs", argv[i])) {
	    fogScale = atof(argv[++i]);
	}
	else if (!strcmp("-bfh", argv[i])) {
	    fogHeight = atof(argv[++i]);
	}
	else if (!strcmp("-bfr", argv[i])) {
	    fogRandomness = atof(argv[++i]);
	}
	else if (!strcmp("-fm", argv[i])) {
	    i++;
	    if(!strcmp("linear", argv[i]))
		fogMode = PFVFOG_LINEAR;
	    else if(!strcmp("exp", argv[i]))
		fogMode = PFVFOG_EXP;
	    else if(!strcmp("exp2", argv[i]))
		fogMode = PFVFOG_EXP2;
	}
	else {
	    printf("\nUnknown option: %s\n\n", argv[i]);
 	    Usage();
	    exit(1);
	}
    }

    *fargc = found;
    if(found == 0) {
	Usage();
	exit(-1);
    }
	    
    return found;
}


//
//	main() -- program entry point. this procedure
//	is executed in the application process.


int
main (int argc, char *argv[])
{
    int		    arg;
    pfPipe         *p;
    pfBox           bbox;
    float	    far = 10000.0f;
    pfWSConnection  dsp=NULL;
    int             fargc;
    char            *fargv[MAX_FILES];
    
    docmdline(argc, argv, &fargc, fargv);


    pfInit();
    
    // configure multi-process selection 
    pfMultiprocess(PFMP_APPCULLDRAW);
    //pfMultiprocess(PFMP_DEFAULT);
    
    // allocate shared before fork()'ing parallel processes 
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    Shared->drawStats = 1;
    
    // Load all loader DSO's before pfConfig() forks 
    for (arg = 0; arg < fargc; arg++)
	if(fargv[arg][0] != '-') //is not an option, must be db file
	    pfdInitConverter(fargv[arg]);

    // initiate multi-processing mode set in pfMultiprocess call 
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    
    pfConfig();
 
    //iR = (strncasecmp(pfGetMachString(), "IR", 2) == 0);
   
    // configure pipes and windows 
    p = pfGetPipe(0);
    Shared->pw = new pfPipeWindow(p);
    Shared->pw->setName("OpenGL Performer");
    Shared->pw->setWinType(WinType);
    if (NoBorder)
	Shared->pw->setMode(PFWIN_NOBORDER, 1);
    // Open and configure the GL window. 
    Shared->pw->setConfigFunc(OpenPipeWin);

    int constraints[] = {
	PFFB_DOUBLEBUFFER,
	PFFB_RGBA,
	PFFB_RED_SIZE, 8, 
	PFFB_GREEN_SIZE, 8, 
	PFFB_BLUE_SIZE, 8, 
	PFFB_ALPHA_SIZE, 8, 
	PFFB_STENCIL_SIZE, 8, 
	PFFB_DEPTH_SIZE, 15, 
#ifndef __linux__
	PFFB_SAMPLES, 0,
	GLX_SAMPLE_BUFFERS_SGIS, 0,
#endif
	(int)NULL};

    Shared->pw->chooseFBConfig(constraints);

    Shared->pw->config();

    if (FullScreen)
	Shared->pw->setFullScreen();
    else
	Shared->pw->setOriginSize(0, 0, sizex, sizey);

    // set off the draw process to open windows and call init callbacks 
    pfFrame();
    
    int rgb_bits;
    Shared->pw->query(PFQWIN_RGB_BITS, &rgb_bits);
    printf("RGB bits: %d\n", rgb_bits);
    Shared->pw->query(PFQWIN_ALPHA_BITS, &rgb_bits);
    printf("Alpha bits: %d\n", rgb_bits);

    int stencil_bits = -1;
    Shared->pw->query(PFQWIN_STENCIL_BITS, &stencil_bits);
    printf("Stencil bits: %d\n", stencil_bits);

    // create forked XInput handling process 
    // since the Shared pointer has already been initialized, that structure
    // will be visible to the XInput process. Nothing else created in the
    // application after this fork whose handles are not put in shared memory
    // (such as the database and channels) will be visible to the
    // XInput process.
    
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
	    else if (!fpid)
		DoXInput();
	}
	else
	{
	    dsp = pfGetCurWSConnection();
	}
    }
    
    // specify directories where geometry and textures exist 
    if (!(getenv("PFPATH")))
        pfFilePath(
                   "."
                   ":./data"
                   ":../data"
                   ":../../data"
                   ":/usr/share/Performer/data"
                   );
    pfNotify(PFNFY_INFO, PFNFY_PRINT,"FilePath: %s\n", pfGetFilePath());
    
    pfScene *scene = new pfScene();

    int i;
    
    // load files named by command line arguments 
    for (i=0 ; i<fargc; i++) {
	pfNode *root;
	if ((root = pfdLoadFile(fargv[i])) != NULL)
	{
	    scene->addChild(root);
	}
    }
    
    // Write out nodes in scene (for debugging) 
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
    
    // determine extent of scene's geometry 
    pfuTravCalcBBox(scene, &bbox);
    
    //pfFrameRate(30.0f);
    pfPhase(PFPHASE_FREE_RUN);
    
    pfChannel *chan = new pfChannel(p);
    Shared->pw->addChan(chan);
    chan->setTravFunc(PFTRAV_CULL, CullChannel);
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    chan->setScene(scene);
    chan->setNearFar(10.0f, far);
    
    chan->setTravMode(PFTRAV_CULL,  PFCULL_ALL);

    Shared->animateSmoke = 0;

    // force the use of 2d textures on O2 or on linux
#ifndef __linux__
    if(!strncmp(pfGetMachString(), "CRM", 3))
#endif
	force2d = 1;


    switch(fogType) {
    case OPENGL_FOG: {
	float color[4] = {0.9,0.9,1,1};
	
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, color);
	
	glFogf(GL_FOG_START, 100);
	glFogf(GL_FOG_END, 550);
	glFogf(GL_FOG_DENSITY, 0.002);
	
	glHint(GL_FOG_HINT, GL_NICEST);
	break;
    }
    
    case LAYERED_MULTICOLORED_FOG:
	Shared->volFog = new pfVolFog;

	Shared->volFog->addColoredPoint(20,0, 0.9, 0.9, 1);
	Shared->volFog->addColoredPoint(40, fogDensity,
					   0.9 ,0.9 , 1);
	Shared->volFog->addColoredPoint(120, fogDensity,
					   0.67,0.45, 0.1);
	Shared->volFog->addColoredPoint(140, 0, 0.67,0.45, 0.1);

	Shared->volFog->addColoredPoint(180, 0, 0.6, 0.7, 1);
	Shared->volFog->addColoredPoint(200, fogDensity*0.2, 
					   0.6, 0.7, 1);
	Shared->volFog->addColoredPoint(250, fogDensity*0.2, 
					   0.6,0.7, 1);
	Shared->volFog->addColoredPoint(270, 0, 0.6,0.7, 1);

	Shared->volFog->setVal(PFVFOG_MODE, (float)fogMode);

	if(force2d)
	    Shared->volFog->setFlags(PFVFOG_FLAG_FORCE_2D_TEXTURE, 1);

	Shared->volFog->addChannel(chan);
	Shared->volFog->apply(scene);
	break;

    case LAYERED_FOG:
    case FOG_BOUNDARIES | LAYERED_FOG:
	Shared->volFog = new pfVolFog;

	Shared->volFog->addPoint(100,0);
	Shared->volFog->addPoint(100,fogDensity);
	Shared->volFog->addPoint(200,fogDensity);
	Shared->volFog->addPoint(200,0);

	Shared->volFog->setColor(0.6, 0.6, 1);

	Shared->volFog->setVal(PFVFOG_MODE, (float)fogMode);

	if(force2d)
	    Shared->volFog->setFlags(PFVFOG_FLAG_FORCE_2D_TEXTURE, 1);

	if(fogType == LAYERED_FOG) {
	    // only layered fog

	    Shared->volFog->addChannel(chan);
	    Shared->volFog->apply(scene);
	    break;
	}

    case FOG_BOUNDARIES:
	pfNode *root;

	if(fogType == FOG_BOUNDARIES)
	    Shared->volFog = new pfVolFog;

	switch(fogIndex) {
	case 0:
	    root = MakeOneLayerFog(&bbox, 100, 20);
	    break;
	case 1:
            {
	    pfBox bbox2;
	    float vec[3];
	    
	    vec[0] = 0.45 * (bbox.max[0] - bbox.min[0]);
	    vec[1] = 0.45 * (bbox.max[1] - bbox.min[1]);

	    bbox2.min[0] = bbox.min[0] + vec[0];
	    bbox2.min[1] = bbox.min[1] + vec[1];
	    bbox2.min[2] = bbox.min[2];
	    bbox2.max[0] = bbox.max[0] - vec[0];
	    bbox2.max[1] = bbox.max[1] - vec[1];
	    bbox2.max[2] = bbox.max[2];

	    root = MakeOneLayerFog(&bbox2, 80, -10);
            }
	    break;

	case 2:
	    root = MakeGroundFog(fogHeight, 
				 fogRandomness, fogVariance, &bbox);
	    break;

	case 3: {
	    float pos[3] = {2500,3000,1};
	    float speed[3] = {0,0,1};
	    float wind[3] = {0.002,0.000,0}; //{0.003,0.000,0};
	    int texSize[3];

	    root = MakeSmoke(pos, 10, 4, speed, wind);

	    Shared->smokeNode = root;
	    Shared->animateSmoke = 1;
	    Shared->volFog->setColor(0.1, 0.05, 0);
	    //Shared->volFog->setColor(1, 1, 1); // test

#if 1
	    // set max distance, as low as possible, but still bigger than
	    // size of the fog object
	    Shared->volFog->setVal(PFVFOG_MAX_DISTANCE, 200);

	    // define density of smoke using layered fog
	    Shared->volFog->setFlags(PFVFOG_FLAG_LAYERED_PATCHY_FOG, 1);
	    
	    // Note, the fog area should not contain any scene objects for now

#define SCALE 2
	    // too high densities cause overflows
	    Shared->volFog->addPoint(-10,0.1*SCALE); 
	    Shared->volFog->addPoint(30,0.05*SCALE); 
	    Shared->volFog->addPoint(60,0.025*SCALE); 
	    Shared->volFog->addPoint(80,0.008*SCALE); 
	    Shared->volFog->addPoint(95,0.0*SCALE);
	    Shared->volFog->addPoint(200,0.0*SCALE);

	    // it doesn't work with multicolored layered fog

	    texSize[0] = 64;
	    texSize[1] = 64;
	    texSize[2] = 64;

	    Shared->volFog->setAttr(PFVFOG_3D_TEX_SIZE, (void *)texSize);

	    if(force2d)
	      Shared->volFog->setFlags(PFVFOG_FLAG_FORCE_2D_TEXTURE, 1);
#endif
	    break;
	}
	}
	
	scene->addChild(root);

	Shared->volFog->addNode(root);
	Shared->volFog->setVal(PFVFOG_RESOLUTION, fogScale);
	Shared->volFog->setDensity(fogDensity);
	Shared->volFog->setVal(PFVFOG_DENSITY_BIAS, fogDensityBias);

	Shared->volFog->setVal(PFVFOG_MODE, (float)fogMode);

	if(fasterPatchy)
	    /* flag not exposed yet, use its value */
	    Shared->volFog->setFlags(PFVFOG_FASTER_PATCHY_FOG, 1);
	    
	Shared->volFog->addChannel(chan);
	Shared->volFog->apply(scene);	    

	break;
    }
    
    // vertical FOV is matched to window aspect ratio 
    chan->setFOV(45.0f, -1.0f);
    if (1)
    {
	float sceneSize;
	// Set initial view to be "in front" of scene 
	
	// view point at center of bbox 
	Shared->view.xyz.add(bbox.min, bbox.max);
	Shared->view.xyz.scale(0.5f, Shared->view.xyz);
	Shared->view.xyz[PF_Z] -= 0.15*(bbox.max[PF_Z] - bbox.min[PF_Z]);
	
	// find max dimension 
	sceneSize = bbox.max[PF_X] - bbox.min[PF_X];
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Y] - bbox.min[PF_Y]);
	sceneSize = PF_MAX2(sceneSize, bbox.max[PF_Z] - bbox.min[PF_Z]);
	sceneSize = PF_MIN2(sceneSize, 0.5f * far);
	Shared->sceneSize = sceneSize;
	
	// offset so all is visible 
	//Shared->view.xyz[PF_Y] -=  sceneSize;
	//Shared->view.xyz[PF_Z] += 0.25f*sceneSize;	
    }  else
    {
	Shared->view.xyz.set(0.0f, 0.0f, 100.0f);
	PFSET_VEC3(bbox.min, -5000.0f, -5000.0f, -1000000.0f);
	PFSET_VEC3(bbox.max, 5000.0f, 5000.0f, 10000000.0f);
	Shared->sceneSize = 10000.0f;
    }
    Shared->view.hpr.set(0.0f, 0.0f, 0.0f);
    chan->setView(Shared->view.xyz, Shared->view.hpr);
    PFCOPY_VEC3(Shared->viewOrig.xyz, Shared->view.xyz);
    PFCOPY_VEC3(Shared->viewOrig.hpr, Shared->view.hpr);

    // light
    pfLight *light = new pfLight;

    light->setColor(PFLT_AMBIENT, 0.8, 0.8, 0.8);
    light->setColor(PFLT_DIFFUSE, 1.2, 1.2, 1.2);
    light->setColor(PFLT_SPECULAR, 0.0, 0.0, 0.0);
    light->setPos(0.25, 0.5, 1, 0);

    light->on();

    // enable lighting

    //glEnable(GL_COLOR_MATERIAL);
    //glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

    // light model
    pfLightModel *lm = new pfLightModel;
    lm->setAmbient(0.0f, 0.0f, 0.0f);
    lm->apply();

    // material
    //pfMaterial *mat = new pfMaterial;
    //mat->setColor(PFMTL_AMBIENT, 0.0, 0.0, 0.0);
    //mat->setColor(PFMTL_DIFFUSE, 0.64, 0.64, 0.64);
    //mat->setColor(PFMTL_SPECULAR, 0.0, 0.0, 0.0);
    //mat->setShininess(10.0f);
    //mat->setColorMode(PFMTL_BOTH, PFMTL_CMODE_AD);
    //mat->apply();

    // main simulation loop 
    while (!Shared->exitFlag)
    {
	// wait until next frame boundary 
	pfSync();
	
	pfFrame();
	
	// Set view parameters for next frame 
	UpdateView();

	if(Shared->animateSmoke == 1) {
	    UpdateSmoke();

	    // to update the bounding box
	    Shared->volFog->addNode(Shared->smokeNode);
	}

	chan->setView(Shared->view.xyz, Shared->view.hpr);
	if(Shared->volFog)
	    Shared->volFog->updateView();

	// initiate traversal using current state 
    
	if (!ForkedXInput  && (WinType & PFPWIN_TYPE_X))
	{
	    if (!Shared->XInputInited)
		InitXInput(dsp);
	    if (Shared->XInputInited)
		GetXInput(dsp);
	}
    }
    
    // terminate cull and draw processes (if they exist) 
    pfExit();
    
    // exit to operating system 
    return 0;
}

static void 
InitXInput(pfWSConnection dsp)
{
    Window w;
    
    /* wait for X Window to exist in Performer shared memory */
   if (w = Shared->pw->getWSWindow())
   {
	XSelectInput(dsp, w, PointerMotionMask |
			ButtonPressMask | ButtonReleaseMask | 
			KeyPressMask | KeyReleaseMask);
	XMapWindow(dsp, w);
	XFlush(dsp);
	Shared->XInputInited = 1;
    }
}

//
// DoXInput() runs an asychronous forked even handling process.
//  Shared memory structures can be read from this process
//  but NO performer calls that set any structures should be 
//  issues by routines in this process.

void
DoXInput(void)
{
    // windows from draw should now exist so can attach X input handling
    // to the X window 
    
    Display *dsp = pfGetCurWSConnection();
#ifndef __linux__    
    prctl(PR_TERMCHILD);        // Exit when parent does 
#else
    prctl(PR_SET_PDEATHSIG, SIGHUP, 0, 0, 0);
#endif    
    sigset(SIGHUP, SIG_DFL);    // Exit when sent SIGHUP by TERMCHILD 
    InitXInput(dsp);
    
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

// 
//	UpdateView() updates the eyepoint based on the information
//	placed in shared memory by GetInput().

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
	Shared->accelRate = 0.003f * Shared->sceneSize;
	return;
    }

    deltaTime = thisTime - prevTime;
    switch (Shared->mouseButtons)
    {
    case Button1Mask: /* LEFTMOUSE: faster forward or slower backward*/
    case Button1Mask|Button2Mask:
	speed += Shared->accelRate * deltaTime;
	if (speed > Shared->sceneSize)
	    speed = Shared->sceneSize;
	break;
    case Button3Mask: /* RIGHTMOUSE: faster backward or slower foreward*/
    case Button3Mask|Button2Mask:
	speed -= Shared->accelRate * deltaTime;
	if (speed < -Shared->sceneSize)
	    speed = -Shared->sceneSize;
	break;
    }
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

    }
    else
    {
	speed = 0.0f;
	Shared->accelRate = 0.003f * Shared->sceneSize;
    }
}

//
//	CullChannel() -- traverse the scene graph and generate a
// 	display list for the draw process.  This procedure is 
//	executed in the CULL process.


static void
CullChannel(pfChannel *, void *)
{
    // 
    // pfDrawGeoSet or other display listable Performer routines
    // could be invoked before or after pfCull()
    pfCull();
}

//
//	OpenPipeWin() -- create a win: setup the GL and OpenGL Performer.
//	This procedure is executed in the DRAW process 
//	(when there is a separate draw process).


static void
OpenPipeWin(pfPipeWindow *pw)
{
    pw->open();
    
    // create a light source in the "south-west" (QIII) 
    //Sun = new pfLight();
    //Sun->setPos(-0.3f, -0.3f, 1.0f, 0.0f);
}


//
//	DrawChannel() -- draw a channel and read input queue. this
//	procedure is executed in the draw process (when there is a
//	separate draw process).

static void
DrawChannel (pfChannel *channel, void *)
{
    // rebind light so it stays fixed in position 
    //Sun->on();
    
    // erase framebuffer and draw Earth-Sky model 
    channel->clear();
    
    switch(fogType) {
    case FOG_BOUNDARIES:
    case LAYERED_FOG:
    case FOG_BOUNDARIES | LAYERED_FOG:
    case LAYERED_MULTICOLORED_FOG:
	Shared->volFog->draw(channel);
	break;

    default:
	// invoke Performer draw-processing for this frame 
	pfDraw();
    }
    
    // draw Performer throughput statistics 
    
    if (Shared->drawStats)
	channel->drawStats();
    
    // read window origin and size (it may have changed) 
    channel->getPWin()->getSize(&Shared->winSizeX, &Shared->winSizeY);
    
}


static void
GetXInput(pfWSConnection dsp)
{
    static int x=0, y=0;
    
    if (XEventsQueued(dsp, QueuedAfterFlush))
#ifdef __linux__
    while (XPending(dsp))
#else
    while (XEventsQueued(dsp, QueuedAlready))
#endif    
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
		case XK_s:
		    Shared->animateSmoke ^= 0x02;
		    break;
		case XK_t:
		    fasterPatchy = !fasterPatchy;
		    Shared->volFog->setFlags(PFVFOG_FASTER_PATCHY_FOG, fasterPatchy);
		    break;
		default:
		    break;
		}
	    }
	    break;
	default:
	    break;
	}// switch 
    }
    Shared->mouseX = x;
    Shared->mouseY = y;
}

