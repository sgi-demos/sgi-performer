//
// Copyright 1995, Silicon Graphics, Inc.
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
//
// multipipe.C: simple Performer program to demonstrate use of 
//               multiple pfPipe's.  based on simple.c
//
// $Revision: 1.14 $
// $Date: 2000/10/06 21:26:24 $ 
//
//

#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pr/pfLight.h>

#include <Performer/pfdu.h>

static int NumScreens=1;
static int NumPipes=3;
static int ProcSplit = PFMP_DEFAULT;
static int GangSwap = 0;
static int PipeStats = 0;
char ProgName[PF_MAXSTRING];

static void ConfigPipeDraw(int pipe, uint stage);
static void OpenPipeWin(pfPipeWindow *pw);


//
//	Usage() -- print usage advice and exit. This
//      procedure is executed in the application process.
//
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: multipipeC file.ext ...\n");
    exit(1);
}

//
//	docmdline() -- use getopt to get command-line arguments, 
//	executed at the start of the application process.
//

static int
docmdline(int argc, char *argv[])
{
    int	    opt;

    strcpy(ProgName, argv[0]);
    
    /* process command-line arguments */
    while ((opt = getopt(argc, argv, "gs:mp:?")) != -1)
    {
	switch (opt)
	{
	case 'g': 
	    GangSwap = 1;
	    break;
	case 's': 
	    PipeStats = 100;
	    break;
	case 'S': 
	    PipeStats = atoi(optarg);
	    break;
	case 'm':
	case 'p':
	    ProcSplit = atoi(optarg);
	    break;
	}
    }
    return optind;
}

int
main (int argc, char *argv[])
{
    float       t = 0.0f;
    int		arg, found;
    
    if (argc < 2)
	Usage();
    
    arg = docmdline(argc, argv);
    
    // Initialize Performer
    pfInit();	
    
    // specify the number of pfPipes
    pfMultipipe (NumPipes);
    
    // Use default multiprocessing mode based on number of
    // processors.
    //
    pfMultiprocess(ProcSplit);
    
    // Load all loader DSO's before pfConfig() forks 
    for (found = arg; found < argc; found++)
	pfdInitConverter(argv[found]);

    // Configure multiprocessing mode and start parallel
    // processes.
    //
    pfConfig();			

    // Append to PFPATH additional standard directories where 
    // geometry and textures exist 
    //
    if (!(getenv("PFPATH")))
        pfFilePath(
                   "."
                   ":./data"
                   ":../data"
                   ":../../data"
                   ":/usr/share/Performer/data"
                   );
    
    // Read a single file, of any known type.
    pfNode *root = pfdLoadFile(argv[1]);
    if (root == NULL) 
    {
	pfExit();
	exit(-1);
    }
    
    // Attach loaded file to a pfScene.
    pfScene *scene = new pfScene;
    for (found = 0; arg < argc; arg++)
    {
	if ((root = pfdLoadFile(argv[arg])) != NULL)
	{
	    scene->addChild(root);
	    found++;
	}
    }
    
    // determine extent of scene's geometry
    pfSphere bsphere;
    scene->getBound(&bsphere);
    
    // Create a pfLightSource and attach it to scene.
    scene->addChild(new pfLightSource);
    
    // Configure and open GL windows
    int max;
    
    if ((NumScreens = ScreenCount(pfGetCurWSConnection())) == 1)
	max = NumPipes;
    else
	max = NumScreens;
    
    
    int loop;
    pfPipe      *pipe[4];
    pfChannel   *chan[4];
    
    for (loop=0; loop < max; loop++)
    {
	char str[PF_MAXSTRING];
	
	pipe[loop] = pfGetPipe(loop);

	pipe[loop]->setScreen(loop);
	
	pfPipeWindow *pw = new pfPipeWindow(pipe[loop]);
	sprintf(str, "OpenGL Performer - Pipe %d", loop);
	pw->setName(str);
	if (NumScreens > 1)
	{
	    pw->setOriginSize(0, 0, 300, 300);
	} else
	    pw->setOriginSize((loop&0x1)*315, ((loop&0x2)>>1)*340, 300, 300);
	
	pw->setConfigFunc(OpenPipeWin);
	pw->config();
    }
    
    // Create and configure pfChannels - by default, channels are
    // placed in the first window on their pipe
    //
    for (loop=0; loop < max; loop++)
    {
	chan[loop] = new pfChannel(pipe[loop]);
	if (!loop)
	{
	    chan[loop]->setScene(scene);
	    chan[loop]->setNearFar(1.0f, 10.0f * bsphere.radius);
	    chan[loop]->setFOV(45.0f, 0.0f);
	    chan[loop]->setShare(PFCHAN_FOV | PFCHAN_SCENE 
		| (GangSwap * PFCHAN_SWAPBUFFERS_HW));
	}
	else
	    chan[0]->attach(chan[loop]);
    }
    
    // set up optional DRAW pipe stage config callback
    pfStageConfigFunc(-1 /* selects all pipes */, 
			PFPROC_DRAW /* stage bitmask */, 
			ConfigPipeDraw);
    pfConfigStage(-1, PFPROC_DRAW);

    // Simulate for twenty seconds.
    while (t < 20.0f)
    {
	float      s, c;
	pfCoord	   view;
	
	// Go to sleep until next frame time.
	pfSync();		
	
	// Compute new view position.
	t = pfGetTime();
	pfSinCos(45.0f*t, &s, &c);
	view.hpr.set(45.0f*t, -10.0f, 0);
	view.xyz.set(2.0f * bsphere.radius * s, 
		     -2.0f * bsphere.radius *c, 
		     0.5f * bsphere.radius);
	
	for (loop=0; loop < max; loop++)
	    chan[loop]->setView(view.xyz, view.hpr);
	
	// Initiate cull/draw for this frame.
	pfFrame();		

	if (PipeStats)
	    pfuManageMPipeStats(PipeStats, NumPipes);
    }
    
    // Terminate parallel processes and exit.
    pfExit();
    
    return 0;
}

// ConfigPipeDraw() --
// Application state for the draw process can be initialized
// here. This is also a good place to do real-time
// configuration for the drawing process, if there is one.
// There is no graphics state or pfState at this point so no
// rendering calls or pfApply*() calls can be made.
//
static void
ConfigPipeDraw(int pipe, uint stage)
{
    
    pfPipe *p = pfGetPipe(pipe);
    int x, y;

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"Initializing stage 0x%x of pipe %d", stage, pipe);
    p->getSize(&x, &y);
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Pipe %d size: %dx%d", pipe, x,y);
}

//
//	OpenPipeWin() -- create a GL window: set up the
//      window system, OpenGL, and OpenGL Performer. This
//      procedure is executed for each window in the draw process 
//	for that pfPipe.
//

static void
OpenPipeWin(pfPipeWindow *pw)
{
    // open the window on the specified screen. By default,
    // if a screen has not yet been set and we are in multipipe mode,
    // a window of pfPipeID i will now open on screen i
    //
    pw->open();
    
    // create a light source in the "south-west" (QIII)
    pfLight *Sun = new pfLight;
    Sun->setPos(-0.3f, -0.3f, 1.0f, 0.0f);
}
