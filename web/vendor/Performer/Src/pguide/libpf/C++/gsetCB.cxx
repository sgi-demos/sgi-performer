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
// gsetCB.C: Demonstrate the use of a pfGeoSetCB node.
//
// $Revision: 1.1 $ 
// $Date: 2001/09/28 23:22:56 $
//



#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSetCB.h>

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#include <GL/gl.h>

#define	SIZE	20.0

void
drawCB(pfGeoSet *gset, void *data)
{
    // This function MUST leave OpenGL state untouched. 
    // Performer does not restore state after this callback.

    // Draw a transparent quad - crossing the origin. 

    glBegin (GL_QUADS);

	glColor4f (1.0, 1.0, 1.0, 0.4);
	glVertex3f (-SIZE, -SIZE, -SIZE);

	glColor4f (0.0, 1.0, 1.0, 0.1);
	glVertex3f ( SIZE,  SIZE, -SIZE);

	glColor4f (1.0, 0.0, 1.0, 1.0);
	glVertex3f ( SIZE,  SIZE,  SIZE);

	glColor4f (1.0, 1.0, 0.0, 0.7);
	glVertex3f (-SIZE, -SIZE,  SIZE);

    glEnd();
}

pfGeode *
createGeoSetCB()
{
    pfGeode	*geode = new pfGeode;
    pfGeoSetCB	*gsetCB = new pfGeoSetCB;
    pfBox	box;
    pfGeoState	*gstate = new pfGeoState;

    box . min[0] = -SIZE;
    box . min[1] = -SIZE;
    box . min[2] = -SIZE;
    box . max[0] = SIZE;
    box . max[1] = SIZE;
    box . max[2] = SIZE;

    // Must set a static bounding box on the geoset. This geoset has no 
    // explicit geometry so Performer has no way to compute its bounding
    // box.

    gsetCB -> setBound (&box, PFBOUND_STATIC);

    gsetCB -> setDrawCB(drawCB, NULL);

    gstate -> makeBasic();

    // Request high-quality transparency. This geoset will be 
    // drawn in the transparent bin - following all opaque geometry.
    // In other words, the draw callback will be called after drawing all 
    // opaque geometry. 

    gstate -> setMode(PFSTATE_TRANSPARENCY, PFTR_HIGH_QUALITY);

    gsetCB -> setGState(gstate);

    geode -> addGSet(gsetCB);

    return (geode);
}

int
main (int argc, char *argv[])
{
    float t = 0.0f;
    int	  i;
    
    // Initialize Performer

    pfInit();
    
    // Use default multiprocessing mode based on number of
    // processors.

    pfMultiprocess( PFMP_DEFAULT );			
    
    // Load all loader DSO's before pfConfig() forks 

    for (i = 1 ; i < argc ; i ++)
        pfdInitConverter(argv[i]);

    // initiate multi-processing mode set in pfMultiprocess call 
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    
    pfConfig();			

    // Append to Performer search path, PFPATH, files in 
    //	    /usr/share/Performer/data */

    pfFilePath(".:/usr/share/Performer/data");
    
    pfScene *scene = new pfScene;

    // Add pfGeode with pfGeoSetCB 

    pfGeode	*geode = createGeoSetCB();
    scene -> addChild(geode);

    // Load all models specified on the command line and add them to the 
    // scene graph.

    for (i = 1 ; i < argc ; i ++)
    {
	pfNode *root = pfdLoadFile(argv[i]);
	if (root == NULL)
	{
	    pfExit();
	    exit(-1);
	}
    
	// Attach loaded file to a new pfScene
	scene->addChild(root);
    }
    
    // Create a pfLightSource and attach it to scene
    scene->addChild(new pfLightSource);
    
    
    // Configure and open GL window
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setName("OpenGL Performer");
    pw->setOriginSize(0,0,500,500);
    pw->open();
    
    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(p);
    chan->setScene(scene);
    chan->setFOV(45.0f, 0.0f);
    
    
    chan->setNearFar(1.0f, SIZE * 10.0f );
    
    // Use a radius to make drawCB geometry visible. Ignore size of 
    // command line models. 

    float	radius = SIZE * 2.0;
    
    // Simulate for twenty seconds.
    while (t < 20.0f)
    {
	pfCoord	   view;
	float      s, c;

	// Go to sleep until next frame time.
	pfSync();		
	
	// Initiate cull/draw for this frame.
	pfFrame();
	
	// Compute new view position.
	t = pfGetTime();
	pfSinCos(45.0f*t, &s, &c);
	view.hpr.set(45.0f*t, -10.0f, 0);
	view.xyz.set(2.0f * radius * s, 
		     -2.0f * radius *c, 
		     0.5f * radius);
	chan->setView(view.xyz, view.hpr);
    }
    
    // Terminate parallel processes and exit
    pfExit();
    return 0;
}
