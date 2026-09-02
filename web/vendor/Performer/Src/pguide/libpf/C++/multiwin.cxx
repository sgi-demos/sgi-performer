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
// multiwin.C: Performer program to demonstrate multiple windows
//              in one pipe.  Derived from simple.c
//
// $Revision: 1.12 $ 
// $Date: 2000/10/06 21:26:24 $ 
//
//

#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pr/pfLight.h>

#include <Performer/pfdu.h>


static void OpenPipeWin(pfPipeWindow *pw);


//
//	Usage() -- print usage advice and exit. This
//      procedure is executed in the application process.
//
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: multiwinC file.ext ...\n");
    exit(1);
}


int
main (int argc, char *argv[])
{
    float       t = 0.0f;
    pfPipeWindow *pwin[16];
    pfChannel   *chan[16];
    int 	loop;
    int		NumWins = 4;
    char	str[PF_MAXSTRING];
    
    if (argc < 2)
	Usage();
    
    // Initialize Performer
    pfInit();	
    
    // Use default multiprocessing mode based on number of
    // processors.
    //
    pfMultiprocess( PFMP_DEFAULT );
    
    // Load all loader DSO's before pfConfig() forks 
    pfdInitConverter(argv[1]);

    // Configure multiprocessing mode and start parallel
    // processes.
    //
    pfConfig();			
    
    // Append to PFPATH additional standard directories where 
    // geometry and textures exist 
    //
    pfFilePath(".:/usr/share/Performer/data");
    
    // Read a single file, of any known type.
    pfNode *root = pfdLoadFile(argv[1]);
    if (root == NULL) 
    {
	pfExit();
	exit(-1);
    }
    
    
    // Attach loaded file to a pfScene.
    pfScene *scene = new pfScene;
    scene->addChild(root);
    
    // determine extent of scene's geometry
    pfSphere bsphere;
    scene->getBound(&bsphere);
    
    // Create a pfLightSource and attach it to scene.
    scene->addChild(new pfLightSource);
    
    // Configure and open GL window
    pfPipe *p = pfGetPipe(0);
    for (loop=0; loop < NumWins; loop++)
    {
	pwin[loop] = new pfPipeWindow(p);
	sprintf(str, "OpenGL Performer - Win %d", loop);
	pwin[loop]->setName(str);
	pwin[loop]->setOriginSize((loop&0x1)*315, ((loop&0x2)>>1)*340,
				  300, 300);
	pwin[loop]->setConfigFunc(OpenPipeWin);
	pwin[loop]->config();
    }
    
    // Create and configure a pfChannel.
    for (loop=0; loop < NumWins; loop++)
    {
	chan[loop] = new pfChannel(p);
	pwin[loop]->addChan(chan[loop]);
	chan[loop]->setScene(scene);
	chan[loop]->setNearFar(1.0f, 10.0f * bsphere.radius);
	chan[loop]->setFOV(45.0f, 0.0f);
    }
    
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
	
	for (loop=0; loop < NumWins; loop++)
	    chan[loop]->setView(view.xyz, view.hpr);
	
	// Initiate cull/draw for this frame.
	pfFrame();		
    }
    
    // Terminate parallel processes and exit.
    pfExit();
    
    return 0;
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
    pfPipe *p = pw->getPipe();
    
    // share GL objects with all the windows on the pipe
    pw->open();
    
    // create a light source in the "south-west" (QIII)
    pfLight *Sun = new pfLight;
    Sun->setPos(-0.3f, -0.3f, 1.0f, 0.0f);
}
