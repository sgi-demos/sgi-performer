//
// Copyright 1995-2001, Silicon Graphics, Inc.
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
// shadowsNew.C: simple Performer program to demonstrate shadows
//



#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfShadow.h>
#include <Performer/pr/pfLight.h>
#include <Performer/pr/pfMaterial.h>

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

///////////////////////////////////////////////////////////////////////////
//
// structure that resides in shared memory so that the
// application, cull, and draw processes can access it.
typedef struct
{
    pfShadow *shadow;
} SharedData;

static SharedData *Shared;

static float xpos, ypos;


// user's draw function and open window function
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);

#define XPOS  2600
#define YPOS  2500
#define ZPOS    30
#define RADIUS  40

//
//	Usage() -- print usage advice and exit. This
//      procedure is executed in the application process.
//
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, 
	     "Usage: shadowsNew scene_file caster_file\n"
	     "recomended: shadowsNew town_ogl_pfi.pfb ch53.pfb\n");
    exit(1);
}


int
main (int argc, char *argv[])
{
    float t = 0.0f;
    
    if (argc != 3)
	Usage();

    // Initialize Performer
    pfInit();
    
    // allocate shared before fork()'ing parallel processes 
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    
    // Use default multiprocessing mode based on number of
    // processors.
    //
    pfMultiprocess(PFMP_DEFAULT); //PFMP_DEFAULT or PFMP_APPCULLDRAW
    
    // Load all loader DSO's before pfConfig() forks 
    pfdInitConverter(argv[1]);
    pfdInitConverter(argv[2]);

    // initiate multi-processing mode set in pfMultiprocess call 
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    //
    pfConfig();			
    
    // Append to Performer search path, PFPATH, files in 
    //	    /usr/share/Performer/data */
     pfFilePath(".:/usr/share/Performer/data:/usr/share/Performer/data/town");
    
    pfNode *root = pfdLoadFile(argv[1]);

    if (root == NULL)
    {
	pfExit();
	exit(-1);
    }

    // Attach loaded file to a new pfScene
    pfScene *scene = new pfScene;

    if(root)
	scene->addChild(root);
    
    // Create a pfLightSource and attach it to scene
    scene->addChild(new pfLightSource);

    // Open and configure the GL window. 
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setWinType(PFPWIN_TYPE_X);

    pw->setConfigFunc(OpenPipeWin);
    pw->config();

    pw->setName("OpenGL Performer");
    pw->setOriginSize(0,0,1024,768);

    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(p);
    chan->setScene(scene);
    chan->setFOV(45.0f, 0.0f);

    // determine extent of scene's geometry
    pfSphere bsphere;
    root->getBound(&bsphere);
    chan->setNearFar(1.0f, 10.0f * bsphere.radius);
    
    // set view
    pfCoord	   view;
    float          s, c;
    pfSinCos(45.0f, &s, &c);
    view.hpr.set(0.0f, -10.0f, 0);
    view.xyz.set(0.4f * bsphere.radius, 
		 0.36f * bsphere.radius, 
		 0.007f * bsphere.radius);
    chan->setView(view.xyz, view.hpr);

    // set user's draw function
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    // read in the shadow caster
    pfNode *caster = pfdLoadFile(argv[2]);
    
    pfMatrix casterMat;
    t = pfGetTime();
    pfSinCos(15.0f*t, &s, &c);
    xpos = XPOS + c*RADIUS;
    ypos = YPOS + s*RADIUS;
    casterMat.makeRot(180+15.0*t, 0,0,1);
    casterMat.postTrans(casterMat,xpos,ypos,ZPOS);

    pfDCS *casterDCS = new pfDCS();
    casterDCS->setMat(casterMat);

    casterDCS->addChild(caster);
    scene->addChild(casterDCS);
	    
    // initialize shadow
    Shared->shadow = new pfShadow;
	
    Shared->shadow->addChannel(chan);
	
    Shared->shadow->setNumCasters(1);

    Shared->shadow->setShadowCaster(0, caster, casterMat);

    // you can adjust the caster center to achieve smoother transitions
    // of some parts of the caster (usualy the transitions are smoothest
    // at the plane containg the center)
    pfVec3 v;
    v.set(0,0,-3); // a good value for the chopper (ch53.pfb)
    Shared->shadow->adjustCasterCenter(0,v);

    Shared->shadow->setNumSources(1);
    // point light source
    //Shared->shadow->setSourcePos(0, XPOS, YPOS, 2*ZPOS, 1);
    // or directional light source
    Shared->shadow->setSourcePos(0, -0.2, 0.2, -0.2, 0);
    Shared->shadow->setAmbientFactor(0, 0.6);

    Shared->shadow->setVal(0, 0, PFSHD_PARAM_TEXTURE_SIZE, 512);
    Shared->shadow->setVal(0, 0, PFSHD_PARAM_NUM_TEXTURES, 32);

    Shared->shadow->setFlags(PFSHD_BLEND_TEXTURES, 1);

    Shared->shadow->apply();
    
    
    // set light source
    pfLightSource *ls = new pfLightSource;
    ls->setPos(-0.3f, -0.3f, 1.0f, 0.0f);
    ls->setColor(PFLT_AMBIENT, 0.6, 0.6, 0.6);
    ls->setColor(PFLT_DIFFUSE, 0.8, 0.8, 0.6);
    scene->addChild(ls);
 
    // DO NOT SET TRAV MODE TO PFCULL_ALL!! 
    // Shadows use cull programs and this would disable them!
    // chan->setTravMode(PFTRAV_CULL,  PFCULL_ALL);

    // Simulate for twenty seconds.
    while (t < 60.0f)
    {
	float      s, c;
	
	// Go to sleep until next frame time.
	pfSync();		
 
	// necessary to update view 
	Shared->shadow->updateView();
    
	// Compute caster position
	t = pfGetTime();
	pfSinCos(15.0f*t, &s, &c);
	xpos = XPOS + c*RADIUS;
	ypos = YPOS + s*RADIUS;
	casterMat.makeRot(180+15.0*t, 0,0,1);
	casterMat.postTrans(casterMat,xpos,ypos,ZPOS);

	casterDCS->setMat(casterMat);
	Shared->shadow->updateCaster(0, casterMat);

	// Initiate cull/draw for this frame.
	pfFrame();
    }
    
    // Terminate parallel processes and exit
    pfExit();
    return 0;
}

static void
OpenPipeWin(pfPipeWindow *pw)
{
    // Configure and open GL window
    int constraints[] = {
	PFFB_DOUBLEBUFFER,
	PFFB_RGBA,
	PFFB_RED_SIZE, 8, 
	PFFB_GREEN_SIZE, 8, 
	PFFB_BLUE_SIZE, 8, 
	PFFB_ALPHA_SIZE, 8, 
	PFFB_STENCIL_SIZE, 8, 
#ifndef __linux__
	PFFB_DEPTH_SIZE, 15, 
	PFFB_SAMPLES, 0,     // DO NOT REMOVE! or change code below
#endif
	NULL};
    
    pw->chooseFBConfig(constraints);

    pw->open();

    int alpha_bits;
    pw->query(PFQWIN_ALPHA_BITS, &alpha_bits);
    if(alpha_bits == 0)
      pfNotify(PFNFY_WARN, PFNFY_PRINT,
	       "No alpha bits present. Shadows will not be rendered correctly!\n");

}

// DrawChannel:
static void
DrawChannel (pfChannel *channel, void *)
{
    // erase framebuffer and draw Earth-Sky model 
    channel->clear();

    // call the draw fuction of the shadow class
    Shared->shadow->draw(channel);
}
      
  
