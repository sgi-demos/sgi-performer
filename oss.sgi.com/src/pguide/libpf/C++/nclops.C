//
// Copyright (c) 2000, Silicon Graphics, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <alloca.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/sysmp.h>
#include <sys/sysinfo.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <Performer/pr.h>
#include <Performer/pf.h>
#include <Performer/pfui.h>
#include <Performer/pfdu.h>

#include <Performer/pr/pfLinMath.h>

#include <Performer/pf/pfPipe.h>
#include <Performer/pf/pfPipeWindow.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfFrameStats.h>

#include <Performer/pfui/pfiXformer.h>


// global command name parsed at startup
static char* gCmd;

// shared data region for communicating with draw
struct SharedData {
    int statsEnabled;
};

static SharedData* gShared;

// error message printing
static void
verrmsg(const char* fmt, va_list ap)
{
    char buf[2048];
    vsprintf(buf, fmt, ap);
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, buf);
}


// error exit
static void
errexit(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verrmsg(fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}


static void
usage()
{
    errexit("%s [-sf] [-n level] [-r framerate] [-p phase] [-m mpmode]\n"
	    "   -f  use small window (512x512)\n"
	    "   -m  a-cd|a-c-d|a-cod|a-cld\n"
	    "   -n  notification level\n"
	    "   -p  float|lock|free|limit\n"
	    "   -s  show stats\n",
	    gCmd);
}


// PipeWindow config func
static void
openPipeWindow(pfPipeWindow* pw)
{
    pw->open();
}


// Draw callback
static void
drawFunc(pfChannel* chan, void*)
{
    chan->clear();
    pfDraw();
    if (gShared->statsEnabled)
	chan->drawStats();
}


void
main(int argc, char** argv)
{
#ifndef SGIX_hyperpipe
    errexit("Hyperpipe extension not available on this system.\n");
#else
    if ((gCmd = strrchr(argv[0], '/')) == NULL) gCmd = argv[0];

    int hyperpipePipes = 2;
    int fullscreen = 1;
    int c, i;
    char *cmpmode = NULL, *cphase = NULL;
    size_t slen;
    int numHyper = 1;
    int nLevel = -1;
    int nFrameRate = -1;

    // init Performer and allocate shared data
    pfInit();

    gShared = (SharedData*)pfMalloc(sizeof(SharedData));
    gShared->statsEnabled = 0;

    while ((c = getopt(argc, argv, "fm:n:p:r:s")) != -1) {
	switch (c) {
	    case 'f':
		fullscreen = 0;
		break;
	    case 'm':
		slen = strlen(optarg);
		cmpmode = (char*)alloca(slen+1);
		for (i = 0; i < slen; i++)
		    cmpmode[i] = tolower(optarg[i]);
		cmpmode[slen] = '\0';
		break;
	    case 'n':
		nLevel = atoi(optarg);
		break;
	    case 'p':
		slen = strlen(optarg);
		cphase = (char*)alloca(slen+1);
		for (i = 0; i < slen; i++)
		    cphase[i] = tolower(optarg[i]);
		cphase[slen] = '\0';
		break;
	    case 'r':
		nFrameRate = atoi(optarg);
		break;
	    case 's':
		gShared->statsEnabled = 1;
		break;
	    case '?':
		usage();
	}
    }

    // determine multiprocess mode
    int mpmode = PFMP_APP_CULL_DRAW;
    if (cmpmode) {
	if (strcmp(cmpmode, "a-cd") == 0)
	    mpmode = PFMP_APP_CULLDRAW;
	else if (strcmp(cmpmode, "ac-d") == 0)
	    mpmode = PFMP_APPCULL_DRAW;
	else if (strcmp(cmpmode, "a-c-d") == 0)
	    mpmode = PFMP_APP_CULL_DRAW;
	else if (strcmp(cmpmode, "a-cod") == 0)
	    mpmode = PFMP_APP_CULLoDRAW;
	else if (strcmp(cmpmode, "a-cld") == 0)
	    mpmode = PFMP_APP_CULL_DL_DRAW;
	else
	    usage();
    }
    pfMultiprocess(mpmode);

    // determine phase
    int phase = PFPHASE_LOCK;
    if (cphase) {
	if (strncmp(cphase, "flo", 3) == 0)
	    phase = PFPHASE_FLOAT;
	else if (strncmp(cphase, "loc", 3) == 0)
	    phase = PFPHASE_LOCK;
	else if (strncmp(cphase, "fre", 3) == 0)
	    phase = PFPHASE_FREE_RUN;
	else if (strncmp(cphase, "lim", 3) == 0)
	    phase = PFPHASE_LIMIT;
    }
    pfPhase(phase);

    // set pfPath to something reasonable
    pfFilePath("."
	       ":./moffett"
	       ":/usr/share/Performer/data"
	       ":/usr/share/Performer/data/clipdata/hunter"
	       ":/usr/share/Performer/data/clipdata/moffett"
	       ":../data"
	       ":../../data"
	       ":../../data/polyhedra"
	       ":../../../data"
	       ":/usr/demos/models"
	       ":/usr/demos/data/flt");

    if (nLevel >= 0)
	pfNotifyLevel(nLevel);

    GLXHyperpipeNetworkSGIX* net = NULL;
    int netPipes;

    Display* dsp;
    dsp = pfGetCurWSConnection();

    // Does this machine support hyperpipe
    int hasHyperpipe;
    pfQueryFeature(PFQFTR_HYPERPIPE, &hasHyperpipe);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "hasHyperpipe = %d\n", hasHyperpipe);
    if (hasHyperpipe) {
	net = glXQueryHyperpipeNetworkSGIX(dsp, &netPipes);
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "netpipes = %d\n", netPipes);

	if (netPipes == 0)
	    errexit("%s: no hyperpipes associated with display %s\n",
		    gCmd, getenv("DISPLAY"));
	
	// determine number of distinct hyperpipe groups
	numHyper = net[netPipes-1].networkId + 1;

	// setup the hyperpipes (they'll be mapped to screens below)
	int hpipe = net[0].networkId, i = 0;
	int pipeCount = 0;
	while (i < netPipes) {
	    if (net[i].networkId != hpipe || i == netPipes-1) {
		// Tell Performer to aggregate pfPipes for each
		// hyperpipe group
		pfHyperpipe(pipeCount);
		pipeCount = 1;
		hpipe = net[i].networkId;
	    } else
		pipeCount++;

	    i++;
	}
    } else
	errexit("%s: no hyperpipe feature on %s\n", gCmd, getenv("DISPLAY"));

    // Init the loaders before Config
    for (i = optind; i < argc; i++)
	pfdInitConverter(argv[i]);

    // Init the utilities
    pfuInit();

    // Config the rendering pipeline
    pfConfig();

    // initialize frame rate if supplied
    if (nFrameRate > -1)
	pfFrameRate(nFrameRate);

    float frameRate = -1, frameRateDiv = 1;

    // Map the hyperpipe group pipes to pfPipes
    for (i = 0; i < netPipes; i++)
	pfGetPipe(i)->setWSConnectionName(net[i].pipeName);

    // Create the pfPipeWindows and pfChannels for the master
    // pfPipes (lowest numbered pfPipe of the hyperpipe group) of
    // each hyperpipe group.
    pfChannel* masterChan;	// master channel
    pfPipeWindow* masterPw;	// master pipewindow

    i = 0;
    while (i < netPipes) {
	pfPipe* p = pfGetPipe(i);

	// Construct the PipeWindow
	pfPipeWindow* pw = new pfPipeWindow(p);
	pw->setName("Hyperpipe Window");
	pw->setConfigFunc(openPipeWindow);
	pw->setWinType(PFPWIN_TYPE_X);
	if (fullscreen)
	    pw->setFullScreen();
	else
	    pw->setOriginSize(0, 0, 512, 512);
	pw->setMode(PFWIN_NOBORDER, 1);
	pw->config();
	if (i == 0)
	    masterPw = pw;

	// Construct the Channel
	pfChannel* chan = new pfChannel(p);
	pw->addChan(chan);

	chan->setShare(chan->getShare() | PFCHAN_VIEWPORT |
		PFCHAN_SWAPBUFFERS | PFCHAN_SWAPBUFFERS_HW);
	chan->makeSimple(45);
	chan->setAutoAspect(PFFRUST_CALC_VERT);

	// Lay channels out vertically
	pfVec3 xyz(0, 0, 0);
	pfVec3 hpr((((numHyper-1)*.5f)-(i/hyperpipePipes))*45.f,0,0);
	chan->setViewOffsets(xyz, hpr);
	if (i > 0)
	    masterChan->attach(chan);
	else 
	    masterChan = chan;

	chan->setNearFar(.000001, 100000);
	chan->setTravFunc(PFTRAV_DRAW, drawFunc);

	i += pfGetHyperpipe(p);
    }

    // Init the input library
    pfiInit();

    // Setup the transformer
    pfiTDFXformer* xformer = new pfiTDFXformer;
    pfuMouse mouse;

    pfuEventStream events;
    memset(&events, 0, sizeof(events));

    xformer->setAutoInput(masterChan, &mouse, &events);

    // Construct the scene
    pfScene* scene = new pfScene;

    pfLightSource* sun = new pfLightSource;
    sun->setPos(0, -1, 0, 0);
    sun->setColor(PFLT_AMBIENT, 0, 0, 0);
    sun->setColor(PFLT_DIFFUSE, .8, .8, .8);
    sun->setColor(PFLT_SPECULAR, .8, .8, .8);

    pfDCS* sunDCS = new pfDCS;
    sunDCS->addChild(sun);

    scene->addChild(sunDCS);

    pfDCS* sceneDCS = new pfDCS;
    scene->addChild(sceneDCS);

    pfGroup* sceneGroup = new pfGroup;
    sceneDCS->addChild(sceneGroup);

    // load the input files
    for (i = optind; i < argc; i++) {
	pfNode* root = pfdLoadFile(argv[i]);
	if (root != NULL)
	    sceneGroup->addChild(root);
    }

    // set the scene on the master Channel
    masterChan->setScene(scene);

    // Init any cliptextures
    {
	pfList* mpct = new pfList(sizeof(pfMPClipTexture*), 1);
	pfuProcessClipCentersWithChannel((pfNode*)sceneGroup, mpct, masterChan);
	delete mpct;
	mpct = new pfList(sizeof(pfMPClipTexture*), 1);
	pfuProcessClipCenters((pfNode*)scene, mpct);
	pfuAddMPClipTexturesToPipes(mpct, pfGetPipe(0), NULL);
	delete mpct;
    }

    // Position the eye point
    pfSphere bsphere;
    scene->getBound(&bsphere);

    {
	pfList* mpct = new pfList(sizeof(pfMPClipTexture*), 1);
	pfuProcessClipCentersWithChannel((pfNode*)sceneGroup, mpct, masterChan);
	delete mpct;
	mpct = new pfList(sizeof(pfMPClipTexture*), 1);
	pfuProcessClipCenters((pfNode*)scene, mpct);
	pfuAddMPClipTexturesToPipes(mpct, pfGetPipe(0), NULL);
	delete mpct;
    }

    pfCoord initPos;
    PFCOPY_VEC3(initPos.xyz, bsphere.center);
    initPos.xyz[1] -= PF_MIN2(2.5f*bsphere.radius, 80000.0f);
    initPos.hpr[0] = initPos.hpr[1] = initPos.hpr[2] = 0;

    // Finish transformer setup
    xformer->setNode(sceneGroup);
    xformer->setAutoPosition(NULL, sceneDCS);
    xformer->setResetCoord(&initPos);
    xformer->selectModel(PFITDF_TRACKBALL);
    xformer->stop();
    xformer->setCoord(&initPos);

    pfMatrix mat;
    xformer->getMat(mat);
    masterChan->setViewMat(mat);

    // Place the Sun
    sunDCS->setMat(mat);

    pfuInitInput(masterPw, PFUINPUT_X);

    // Setup statistics
    pfFrameStats* fstats = masterChan->getFStats();
    fstats->setAttr(PFFSTATS_UPDATE_SECS, 2);
    fstats->setClass(PFFSTATS_ENPFTIMES, PFSTATS_ON);
    fstats->setClass(PFFSTATS_ENRTMON, PFSTATS_ON);
    fstats->setClassMode(PFFSTATS_CLASSES, PFFSTATS_PFTIMES_MASK, PFSTATS_ON);

    // Kick off the first frame
    pfFrame();

    pfNotify(PFNFY_DEBUG, PFNFY_INTERNAL, "field rate = %d, frame rate = %f "
	    "video rate = %f\n", pfGetFieldRate(), pfGetFrameRate(),
	    pfGetVideoRate());

    int exitFlag = 0;
    events.numDevs = 0;
    while (!exitFlag) {
	pfSync();


	pfuGetEvents(&events);
	pfuGetMouse(&mouse);
	pfiUpdateXformer(xformer);

	for (i = 0; i < events.numDevs; i++) {
	    int dev = events.devQ[i];

	    if (events.devCount[dev] > 0) {
		switch (dev) {
		    case PFUDEV_WINQUIT:
			exitFlag = 1;
			break;
		    case PFUDEV_KEYBD:
			for (int j = 0; j < events.numKeys; j++) {
			    int key = events.keyQ[j];
			    if (events.keyCount[key]) {
				switch (key) {
				    case 27:
					exitFlag = 1;
					break;
				    case 'r':
					xformer->stop();
					xformer->setCoord(&initPos);
					break;
				    case 's':
					gShared->statsEnabled ^= 1;
					break;
				    case 'p':
					pfPhase(phase = (phase + 1) % 4);
					break;
				    case 'f':
					if (frameRateDiv < 10)
					    frameRateDiv += 1;
					else
					    frameRateDiv = 1;
					if (frameRate < 0)
					    frameRate = pfGetFrameRate();
					pfNotify(PFNFY_DEBUG, PFNFY_INTERNAL,
						"video rate %f, old frame "
						"rate %f, new frame rate %f\n",
						pfGetVideoRate(), frameRate,
						frameRate/frameRateDiv);
					pfFrameRate(frameRate/frameRateDiv);
					break;
				    case 'F':
					if (frameRateDiv > 1)
					    frameRateDiv -= 1;
					else
					    frameRateDiv = 10;
					if (frameRate < 0)
					    frameRate = pfGetFrameRate();
					pfNotify(PFNFY_DEBUG, PFNFY_INTERNAL,
						"video rate %f, old frame "
						"rate %f, new frame rate %f\n",
						pfGetVideoRate(), frameRate,
						frameRate/frameRateDiv);
					pfFrameRate(frameRate/frameRateDiv);
					break;
				    case 'i':
					{
					    int fields, vrate;
					    vrate = pfGetVideoRate();
					    fields = pfGetFieldRate();
					    fields++;
					    if (fields > vrate)
						fields = vrate;
					    pfFieldRate(fields);
					    frameRate = pfGetFrameRate();
					    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "field rate %d frame rate %f\n",
						    fields, frameRate);
					}
					break;
				    case 'I':
					{
					    int fields;
					    fields = pfGetFieldRate();
					    fields--;
					    if (fields <= 0)
						fields = 1;
					    pfFieldRate(fields);
					    frameRate = pfGetFrameRate();
					    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "field rate %d frame rate %f\n",
						    fields, frameRate);
					}
					break;
				}
			    }
			}
		}
	    }
	}

	events.numDevs = 0;

	pfFrame();
    }

    pfuExitInput();
    pfuExitUtil();
    pfExit();

    exit(EXIT_SUCCESS);
#endif
}
