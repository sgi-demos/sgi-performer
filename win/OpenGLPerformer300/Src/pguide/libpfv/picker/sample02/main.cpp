//
// Copyright 2002, Silicon Graphics, Inc.
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


///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>


#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pf/pfDCS.h>

 #include <Performer/pr/pfLinMath.h>   

#include <Performer/pr/pfHighlight.h>

#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#include <Performer/pfv/pfvXml.h>

#include <Performer/pfv/pfvPicker.h>
#include <Performer/pfv/pfvMousePicker.h>

#include "myInteractor.h"

///////////////////////////////////////////////////////////////////////////////

int
main (int argc, char *argv[])
{
    float t = 0.0f;
    int i,n;

    pfInit();
    pfuInit();

    for(i=1;i<argc;i++)	
	pfdInitConverter(argv[i]);

    pfMultiprocess( PFMP_DEFAULT );			
    pfConfig();			
    
    pfFilePath(".:/usr/share/Performer/data");



    pfScene *scene = new pfScene;
  
    scene->addChild(new pfLightSource);


	
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setName("OpenGL Performer");
    pw->setOriginSize(0,0,500,500);
    pw->open();
    
    pfChannel *chan = new pfChannel(p);
    chan->setScene(scene);
    chan->setFOV(45.0f, 0.0f);

    pfuInitInput( pw, PFUINPUT_X );



    

    // interaction:
    
    pfvMousePicker* picker = new pfvMousePicker;
    picker->setNodeDataSlot("PICKER");
    picker->setChannel(chan);
    picker->setState(PFPICKER_ALLOW_HLITE, NULL, NULL);

#if 0
    picker->setScene(scene);
    picker->chan = chan;
    pfvPickerRequest req;
    req.state = PFPICKER_ALLOW_HLITE;
    req.selector = NULL;
    req.interactor = NULL;
    picker->grantRequest(&req);
#endif





    float separation = 5.0f;
    char*env = getenv("SEPARATION");
    if(env)
	separation = atof(env);

    printf("OBJECT SEPARATION: %f\n", separation );
    float halfDist = (separation*((argc-2)))/2.0f;
    
    pfNode *root;
    pfDCS* dcs;
    
    for(i=1;i<argc;i++)	
    {
	root = pfdLoadFile(argv[i]);
	PFASSERTALWAYS(root);
	dcs = new pfDCS;
	PFASSERTALWAYS(dcs);
	dcs->addChild(root);	
	
	char n[32];
	sprintf(n,"obj%d",i);
	root->setName(n);

	dcs->setTrans( -halfDist + ((i-1)*separation), 0.0f, 0.0f ); 

	scene->addChild(dcs);

	myInteractor* ia = new myInteractor();
	ia->node = dcs;
	ia->nodeSetup( dcs, picker );
    }
	
    pfSphere bsphere;
    scene->getBound(&bsphere);
    chan->setNearFar(1.0f, 10.0f * bsphere.radius);
    
    while (t < 2000000.0f)
    {
	pfCoord	view;
	float s, c;

	pfSync();		
	pfFrame();

	//t = pfGetTime();
	pfSinCos( 45.0f*t, &s, &c );
	view.hpr.set( 45.0f*t, -10.0f, 0 );
	view.xyz.set(  2.0f * bsphere.radius * s, 
			  -2.0f * bsphere.radius *c, 
		       0.5f * bsphere.radius);
	
	static int first=1;
	if(first)
	{
	    first=0;
	    chan->setView(view.xyz, view.hpr);
	}

	// interaction:
	picker->update();
    }
    
    pfExit();
    return 0;
}

