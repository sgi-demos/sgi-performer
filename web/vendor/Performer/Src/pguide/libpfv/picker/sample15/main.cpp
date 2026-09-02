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

#include "myNavigator.h"

///////////////////////////////////////////////////////////////////////////////

class myInteractor : public pfvInteractor
{
public:
    myInteractor( pfvXmlNode* xml=NULL ){;}
    ~myInteractor() {;}
    int specialFocus( pfvPicker* p,int ev,int prmsn,pfvPickerRequest*r ){

	if(ev==LEFT_DOWN)
	{
	    pfNotify( PFNFY_ALWAYS,PFNFY_PRINT,"myInteractor::specialFocus called");
	    pfHit ** hits = p->getPickList();
	    if(hits[0])
	    {
	        pfGeoSet* gset;
	        if(hits[0]->query(PFQHIT_GSET,&gset))
	        {
		    pfPrint(gset, 0, PFPRINT_VB_DEBUG, NULL);
	        }
	    }
        }
        return 0;
    }
};

///////////////////////////////////////////////////////////////////////////////

int
main (int argc, char *argv[])
{
    int i;

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

    myInteractor* interactor = new myInteractor();
    interactor->nodeSetup( scene, picker );

    picker->setState( PFPICKER_FOCUS_EVENT, NULL, NULL);


    /* NAVIGATION */
    myNavigator* nav = new myNavigator(NULL);
    PFASSERTALWAYS(nav);

	
    float separation = 5.0f;
    char*env = getenv("SEPARATION");
    if(env)
        separation = atof(env);
    /*printf("OBJECT SEPARATION: %f\n", separation );*/
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
    }
	    
    {
        pfSphere bsphere;
        scene->getBound(&bsphere);
        chan->setNearFar(1.0f, 10.0f * bsphere.radius);

        pfVec3 xyz, hpr;
        xyz.set(0.0f, -2.0f * bsphere.radius, 0.5f * bsphere.radius);
        hpr.set(0.0f, 0.0f, 0.0f);
	
        nav->setCoord(&xyz,&hpr);
        nav->setDefaultCoord(&xyz,&hpr);
    }

    while (TRUE)
    {

	pfSync();		
	pfFrame();

	// interaction:
	picker->update();
	
	// navigation:
	nav->update(picker->getMouse(),picker->getEventStream(),chan);
    }
    
    pfExit();
    return 0;
}


