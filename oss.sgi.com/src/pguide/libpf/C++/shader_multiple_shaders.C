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
// shader_multiple_shaders.C:  Multiple shaders example
//
// $Revision: 1.1 $ 
// $Date: 2000/11/21 21:39:38 $
//

#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>

#include <Performer/pr/pfMaterial.h>
#include <Performer/pr/pfFBState.h>

#include <Performer/pf/pfShader.h>
#include <Performer/pf/pfShaderManager.h>

#include <Performer/pfdu.h>

#include <Performer/pfutil.h>

#include <assert.h>

///////////////////////////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////////////////////////

int
main (int argc, char *argv[])
{
    // Initialize and configure OpenGL Performer
    pfInit();
    
    pfMultiprocess( PFMP_DEFAULT );			
    
    pfConfig();
    
    // Look for files in PFPATH, ".", and  "/usr/share/Performer/data"
    pfFilePath(".:/usr/share/Performer/data:/usr/share/Performer/data/shaders");
    
    // Create a scene
    pfScene *scene = new pfScene;
    
    // Create a lit scene pfGeoState for the scene
    pfGeoState *gstate = new pfGeoState;
    gstate->setMode(PFSTATE_ENLIGHTING, PF_ON);
    // attach the pfGeoState to the scene
    scene->setGState(gstate);
    
    // put a default light source in the scene
    scene->addChild(new pfLightSource);


    // create a root
    pfDCS *root = new pfDCS;
    root->setName ("root");

    // add the root to the scene
    scene->addChild (root);

    // create a new local dcs
    pfDCS *local_dcs1 = new pfDCS;
    local_dcs1->setTrans (0.3f, 0.0f, -0.6f);
    local_dcs1->setName ("local_dcs1");
    root->addChild (local_dcs1);
    
    // create a new local dcs
    pfDCS *local_dcs2 = new pfDCS;
    local_dcs2->setTrans (-0.3f, 0.0f, -0.6f);
    local_dcs2->setName ("local_dcs2");
    root->addChild (local_dcs2);
    
    // create a new local dcs
    pfDCS *local_dcs3 = new pfDCS;
    local_dcs3->setTrans (-0.6f, 0.0f, -0.6f);
    local_dcs3->setName ("local_dcs3");
    local_dcs2->addChild (local_dcs3);


    // create a sphere in the root dcs
    pfGeode *sphere1 = new pfGeode;
    sphere1->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    sphere1->setName ("sphere1");
    pfDCS *dcs = new pfDCS;
    dcs->setName ("sphere1_dcs");
    dcs->setScale (0.2f, 0.2f, 0.2f);
    dcs->addChild (sphere1);
    root->addChild (dcs);
    sphere1->getGSet(0)->setGState (new pfGeoState);



    // create two spheres in the first local dcs
    pfGeode *sphere2 = new pfGeode;
    sphere2->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    sphere2->setName ("sphere2");
    dcs = new pfDCS;
    dcs->setName ("sphere2_dcs");
    dcs->setScale (0.2f, 0.2f, 0.2f);
    dcs->setTrans (0.0f, 0.0f, 0.0f);
    dcs->addChild (sphere2);
    local_dcs1->addChild (dcs);
    sphere2->getGSet(0)->setGState (new pfGeoState);

    pfGeode *sphere3 = new pfGeode;
    sphere3->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    sphere3->setName ("sphere3");
    dcs = new pfDCS;
    dcs->setName ("sphere3_dcs");
    dcs->setScale (0.2f, 0.2f, 0.2f);
    dcs->setTrans (0.4f, 0.0f, 0.0f);
    dcs->addChild (sphere3);
    local_dcs1->addChild (dcs);
    sphere3->getGSet(0)->setGState (new pfGeoState);


    // create three spheres in the second local dcs
    pfGeode *sphere4 = new pfGeode;
    sphere4->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    sphere4->setName ("sphere4");
    dcs = new pfDCS;
    dcs->setName ("sphere4_dcs");
    dcs->setScale (0.2f, 0.2f, 0.2f);
    dcs->setTrans (0.0f, 0.0f, 0.0f);
    dcs->addChild (sphere4);
    local_dcs2->addChild (dcs);
    sphere4->getGSet(0)->setGState (new pfGeoState);

    pfGeode *sphere5 = new pfGeode;
    sphere5->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    sphere5->setName ("sphere5");
    dcs = new pfDCS;
    dcs->setName ("sphere5_dcs");
    dcs->setScale (0.2f, 0.2f, 0.2f);
    dcs->setTrans (-0.4f, 0.0f, 0.0f);
    dcs->addChild (sphere5);
    local_dcs2->addChild (dcs);
    sphere5->getGSet(0)->setGState (new pfGeoState);

    pfGeode *sphere6 = new pfGeode;
    sphere6->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    sphere6->setName ("sphere6");
    dcs = new pfDCS;
    dcs->setName ("sphere6_dcs");
    dcs->setScale (0.2f, 0.2f, 0.2f);
    dcs->setTrans (-0.8f, 0.0f, 0.0f);
    dcs->addChild (sphere6);
    local_dcs2->addChild (dcs);
    sphere6->getGSet(0)->setGState (new pfGeoState);


    // create four spheres in the third local dcs
    pfGeode *sphere7 = new pfGeode;
    sphere7->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    sphere7->setName ("sphere7");
    dcs = new pfDCS;
    dcs->setName ("sphere7_dcs");
    dcs->setScale (0.2f, 0.2f, 0.2f);
    dcs->setTrans (0.0f, 0.0f, 0.0f);
    dcs->addChild (sphere7);
    local_dcs3->addChild (dcs);
    sphere7->getGSet(0)->setGState (new pfGeoState);

    pfGeode *sphere8 = new pfGeode;
    sphere8->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    sphere8->setName ("sphere8");
    dcs = new pfDCS;
    dcs->setName ("sphere8_dcs");
    dcs->setScale (0.2f, 0.2f, 0.2f);
    dcs->setTrans (-0.4f, 0.0f, 0.0f);
    dcs->addChild (sphere8);
    local_dcs3->addChild (dcs);
    sphere8->getGSet(0)->setGState (new pfGeoState);

    pfGeode *sphere9 = new pfGeode;
    sphere9->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    sphere9->setName ("sphere9");
    dcs = new pfDCS;
    dcs->setName ("sphere9_dcs");
    dcs->setScale (0.2f, 0.2f, 0.2f);
    dcs->setTrans (-0.8f, 0.0f, 0.0f);
    dcs->addChild (sphere9);
    local_dcs3->addChild (dcs);
    sphere9->getGSet(0)->setGState (new pfGeoState);

    pfGeode *sphere10 = new pfGeode;
    sphere10->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    sphere10->setName ("sphere10");
    dcs = new pfDCS;
    dcs->setName ("sphere10_dcs");
    dcs->setScale (0.2f, 0.2f, 0.2f);
    dcs->setTrans (-1.2f, 0.0f, 0.0f);
    dcs->addChild (sphere10);
    local_dcs3->addChild (dcs);
    sphere10->getGSet(0)->setGState (new pfGeoState);

    // create some shaders and apply them
    pfShader  *red_shader, *green_shader, *blue_shader, *purple_shader,
      *wood_shader, *logo_shader;

    
    red_shader =    pfdLoadShader ("red_material.shader");
    green_shader =  pfdLoadShader ("green_material.shader");
    blue_shader =   pfdLoadShader ("blue_material.shader");
    purple_shader = pfdLoadShader ("two_pass_purple.shader");
    wood_shader =   pfdLoadShader ("wood.shader");
    logo_shader =   pfdLoadShader ("texture_matrix.shader");

    pfRef (red_shader);
    pfRef (green_shader);
    pfRef (blue_shader);
    pfRef (purple_shader);
    pfRef (wood_shader);
    pfRef (logo_shader);

    pfShaderManager *shaderManager = pfGetShaderManager ();

    // apply some shaders and resolve them
    shaderManager->applyShader (root,       red_shader);
    shaderManager->applyShader (local_dcs1, green_shader);
    shaderManager->applyShader (sphere2,    wood_shader);
    shaderManager->applyShader (local_dcs2, blue_shader);
    shaderManager->applyShader (sphere5,    purple_shader);
    shaderManager->applyShader (local_dcs3, purple_shader);
    shaderManager->applyShader (sphere10,   red_shader);
    shaderManager->applyShader (sphere10,   logo_shader);
    shaderManager->resolveShaders (root);

    // remove a shader and resolve again
    shaderManager->removeShader (root, 0);
    shaderManager->resolveShaders (root);

    // add a different shader in place of the one we just removed,
    // remove another (and one illegal one) and resolve again.
    shaderManager->applyShader (root, logo_shader);
    shaderManager->removeShader (sphere10, 1);
    shaderManager->resolveShaders (root);
    
    // Create and configure a pfPipe and pfChannel.
    pfPipe *pipe = pfGetPipe(0);
    pfChannel *chan = new pfChannel(pipe);
    chan->setFOV(60.0f, 0.0f);
    chan->setScene(scene);
    
    // Determine extent of scene's geometry.
    pfSphere bsphere;
    root->getBound(&bsphere);
    chan->setNearFar(1.0f, 10.0f*bsphere.radius);
    
    // Spin text for 15 seconds.
    double startTime = pfGetTime();
    double t;
    while ((t = pfGetTime() - startTime) < 20.0f)
    {
	pfCoord	   view;
	float      s, c;
	
	// Compute new view position, rotating around text.
	pfSinCos(45.0f*t, &s, &c);
	view.hpr.set(45.0f*t, -5.0f, 0.0f);
	view.xyz.set(
		     2.0f * bsphere.radius * s,
		     -2.0f * bsphere.radius * c,
		     0.0f);
	chan->setView(view.xyz, view.hpr);
	
	// Initiate cull/draw processing for this frame.
	pfFrame();
    }
    
    // Terminate parallel processes and exit.
    pfExit();
    return 0;
}
