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
// shader_blue_and_purple.C:  Shader demonstration program
//
// $Revision: 1.1 $ 
// $Date: 2000/11/21 21:39:38 $
//

#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pr/pfMaterial.h>

#include <Performer/pf/pfShader.h>
#include <Performer/pf/pfShaderManager.h>

#include <Performer/pfdu.h>

#include <Performer/pfutil.h>



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

    pfShaderManager *shaderManager = pfGetShaderManager ();
    
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

    pfDCS *dcs1 = new pfDCS;
    dcs1->setTrans (2.0, 0, 0);

    pfDCS *dcs2 = new pfDCS;
    dcs2->setTrans (-2.0, 0, 0);

    root->addChild (dcs1);
    root->addChild (dcs2);

    // create a sphere 
    pfGeode *geode1 = new pfGeode;
    geode1->addGSet (pfdNewSphere (200, pfGetSharedArena ()));

    // create a sphere 
    pfGeode *geode2 = new pfGeode;
    geode2->addGSet (pfdNewSphere (200, pfGetSharedArena ()));
    
    dcs1->addChild (geode1);
    dcs2->addChild (geode2);

    // load a shader and apply it
    pfShader *shader1 = pfdLoadShader ("two_pass_purple.shader");
    shaderManager->applyShader (geode1, shader1);

    // create a shader programmatically and apply it
    pfFBState *fbs = new pfFBState();
    fbs->setEnable(GL_BLEND, 0);
    fbs->setEnable(GL_DEPTH_TEST, 1);
    fbs->setDepthFunc(GL_LEQUAL);

    pfShader *shader2 = new pfShader;
    shader2->setDefaultFBState(fbs);
    shader2->openPass (PF_SHADERPASS_GEOMETRY);
    pfGeoState *gstate2 = new pfGeoState;
    pfMaterial *mtl = new pfMaterial;
    mtl->setColor (PFMTL_DIFFUSE, 0, 0, 1);
    gstate2->setAttr (PFSTATE_FRONTMTL, mtl);
    gstate2->setAttr (PFSTATE_BACKMTL, mtl);
    shader2->setPassAttr (PF_SHADERPASS_GSTATE, gstate2);
    shader2->closePass ();
    shaderManager->applyShader (geode2, shader2);

    shaderManager->resolveShaders (root);


    // add the root to the scene
    scene->addChild (root);
    
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
    while ((t = pfGetTime() - startTime) < 10.0f)
    {
	pfCoord	   view;
	float      s, c;
	
	// Compute new view position, rotating around text.
	pfSinCos(45.0f*t, &s, &c);
	view.hpr.set(45.0f*t, -5.0f, 0.0f);
	view.xyz.set(
		     2.0f * bsphere.radius * s,
		     -2.0f * bsphere.radius * c,
		     0.3f * bsphere.radius);
	chan->setView(view.xyz, view.hpr);
	
	// Initiate cull/draw processing for this frame.
	pfFrame();
    }
    
    // Terminate parallel processes and exit.
    pfExit();
    return 0;
}
