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
///////////////////////////////////////////////////////////////////////// 
//                                                                        
// inheritC.C   - transform inheritance example
//                                                                        
// $Revision: 1.14 $ 
// $Date: 2002/11/10 02:29:57 $                         
//                                                                        
///////////////////////////////////////////////////////////////////////// 

#include <math.h>
#include <stdlib.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfLightSource.h>

#include <Performer/pfdu.h>

int
main(int argc, char *argv[])
{
    pfCoord         view;
    float           z, s, c;
    pfSphere sphere;
    char	*file1, *file2;

    file1 = (argc > 1) ? argv[1] : "blob.nff";
    file2 = (argc > 1) ? argv[1] : "torus.nff";

    pfInit();				// Initialize Performer
    pfFilePath("."
	":./data:../data:../../data:../../../data:../../../../data"
	":/usr/share/Performer/data");
    pfMultiprocess(PFMP_DEFAULT);	// Single thread for simplicity

    // Load all loader DSO's before pfConfig() forks 
    pfdInitConverter(file1);
    pfdInitConverter(file2);

    pfConfig();				// Configure

    // Load the files
    pfNode *model1 = pfdLoadFile(file1);
    if (model1 == NULL)
    {
	pfExit();
	exit(-1);
    }
    pfNode *model2 = pfdLoadFile(file2);
    if (model2 == NULL)
    {
	pfExit();
	exit(-1);
    }

    // scale models to unit size
    pfDCS *node1 = new pfDCS;
    node1->addChild(model1);
    model1->getBound(&sphere);
    if (sphere.radius > 0.0f)
	node1->setScale(1.0f/sphere.radius);

    pfDCS *node2 = new pfDCS;
    node2->addChild(model2);
    model2->getBound(&sphere);
    if (sphere.radius > 0.0f)
	node2->setScale(1.0f/sphere.radius);

    // Create the hierarchy
    pfDCS *dcs4 = new pfDCS;
    dcs4->addChild(node1);
    dcs4->setScale(0.5f);

    pfDCS *dcs3 = new pfDCS;
    dcs3->addChild(node1);
    dcs3->addChild(dcs4);

    pfDCS *dcs1 = new pfDCS;
    dcs1->addChild(node2);

    pfDCS *dcs2 = new pfDCS;
    dcs2->addChild(node2);
    dcs2->setScale(0.5f);
    dcs1->addChild(dcs2);

    pfScene *scene = new pfScene;
    scene->addChild(dcs1);
    scene->addChild(dcs3);
    scene->addChild(new pfLightSource);

    // Configure and open GL window 
    pfPipe *pipe = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(pipe);
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setName("OpenGL Performer");
    pw->setOriginSize(0, 0, 500, 500);
    pw->open();

    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(pipe);
    chan->setScene(scene);

    view.xyz.set(0.0f, 0.0f, 15.0f);
    view.hpr.set(0.0f, -90.0f, 0.0f);
    chan->setView(view.xyz, view.hpr);

    // Loop through various transformations of the DCS's
    for (z = 0.0f; z < 1084; z += 4.0f)
    {
	dcs1->setRot((z < 360) ? (int) z % 360 : 0.0f,
		     (z > 360 && z < 720) ? (int) z % 360 : 0.0f,
		     (z > 720) ? (int) z % 360 : 0.0f);

	pfSinCos(z, &s, &c);
	dcs2->setTrans(1.0f * c, 1.0f * s, 0.0f);

	dcs3->setRot(z, 0, 0);
	dcs3->setTrans(4.0f * c, 4.0f * s, 4.0f * s);
	dcs4->setRot(0, 0, z);
	dcs4->setTrans(1.0f * c, 1.0f * s, 0.0f);

	pfFrame();
    }
#ifdef WIN32
    Sleep(3000);
#else
    sleep(3);
#endif

    pfExit();

    return 0;
}
