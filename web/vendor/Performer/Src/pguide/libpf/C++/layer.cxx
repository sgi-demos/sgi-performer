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
// layerC.C: simple Performer program to demonstrate layered polygons
//
// $Revision: 1.10 $ 
// $Date: 2000/10/06 21:26:23 $ 
//
//

#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfEarthSky.h>
#include <Performer/pf/pfLayer.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfGeoSet.h>

#include <Performer/pfdu.h>

int
main (int argc, char *argv[])
{
    float       t = 0.0f;
    
    // Initialize Performer
    pfInit();	
    
    // Use default multiprocessing mode based on number of
    // processors.
    //
    pfMultiprocess( PFMP_DEFAULT );
    
    // Configure multiprocessing mode and start parallel
    // processes.
    //
    pfConfig();			
    
    // Append to PFPATH additional standard directories where 
    // geometry and textures exist 
    //
    pfFilePath(".:/usr/share/Performer/data");
    
    // Configure and open GL window
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setName(argv[0]);
    pw->setOriginSize(0, 0, 500, 500);
    pw->open();
    
    pfGeoState *gstate = new pfGeoState;
    gstate->setMode(PFSTATE_CULLFACE, 0);
    
    if (argc>1)
    {
        // Set up textures & gstate
	
        pfTexEnv *tev = new pfTexEnv;
        pfTexture *tex = new pfTexture;
	
        if (tex->loadFile(argv[1]))
	{
	    gstate->setMode(PFSTATE_ENTEXTURE, 1);
	    gstate->setMode(PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
	    gstate->setVal(PFSTATE_ALPHAREF, 0.0f);
	    gstate->setAttr(PFSTATE_TEXENV, tev);
	    gstate->setAttr(PFSTATE_TEXTURE, tex);
	}
    }
    
    // Set up geosets
    pfVec3 *coords = (pfVec3*) new(4*sizeof(pfVec3)) pfMemory;
    
    coords[0].set(-1.0f,  0.0f,  -1.0f);
    coords[1].set( 1.0f,  0.0f,  -1.0f );
    coords[2].set( 1.0f,  0.0f,   1.0f );
    coords[3].set(-1.0f,  0.0f,   1.0f );
    
    ushort *vertexlist = (ushort*) new(4*sizeof(ushort)) pfMemory;
    vertexlist[0] = 0;
    vertexlist[1] = 1;
    vertexlist[2] = 2;
    vertexlist[3] = 3;
    
    pfVec4 *colors = (pfVec4*) new(4*sizeof(pfVec4)) pfMemory;
    
    colors[0].set(1.0f, 1.0f, 1.0f, 1.0f);
    colors[1].set(0.0f, 0.0f, 1.0f, 1.0f);
    colors[2].set(1.0f, 0.0f, 0.0f, 1.0f);
    colors[3].set(0.0f, 1.0f, 0.0f, 1.0f);
    
    ushort *colorlist = (ushort*) new(4*sizeof(ushort)) pfMemory;
    
    colorlist[0] = 0;
    colorlist[1] = 1;
    colorlist[2] = 2;
    colorlist[3] = 3;
    
    pfVec2 *texcoords = (pfVec2*) new(4*sizeof(pfVec2)) pfMemory;
    texcoords[0].set(0.0f, 0.0f);
    texcoords[1].set(1.0f, 0.0f);
    texcoords[2].set(1.0f, 1.0f);
    texcoords[3].set(0.0f, 1.0f);;
    
    ushort *texlist = (ushort*) new(4*sizeof(ushort)) pfMemory;
    
    texlist[0] = 0;
    texlist[1] = 1;
    texlist[2] = 2;
    texlist[3] = 3;
    
    pfGeoSet *gset = new pfGeoSet;
    
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, vertexlist);
    gset->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, colors, colorlist);
    gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, texlist);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(1);
    
    gset->setGState(gstate);
    
    // set up scene graph
    pfGeode *geode1 = new pfGeode; geode1->addGSet(gset);
    pfGeode *geode2 = new pfGeode; geode2->addGSet(gset);
    pfGeode *geode3 = new pfGeode; geode3->addGSet(gset);
    
    pfDCS *dcs1 = new pfDCS;
    dcs1->addChild(geode1);
    
    pfDCS *dcs2 = new pfDCS;
    dcs2->addChild(geode2);
    dcs2->setScale(0.5f);
    dcs2->setRot(0.0f, 0.0f, 180.0f);
    
    pfDCS *dcs3 = new pfDCS;
    dcs3->addChild(geode3);
    dcs3->setScale(0.25f);
    
    pfLayer *layer = new pfLayer;
    
    layer->addChild(dcs1);        // first child is base
    layer->addChild(dcs2);        // subsequent children layered
    layer->addChild(dcs3);
    
    pfScene *scene = new pfScene;
    scene->addChild(layer);
    
    // determine extent of scene's geometry
    pfSphere bsphere;
    scene->getBound(&bsphere);
    
    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(p);
    chan->setScene(scene);
    chan->setNearFar(1.0f, 10.0f * bsphere.radius);
    chan->setFOV(45.0f, 0.0f);
    
    // Create an earth/sky model that draws sky/ground/horizon
    pfEarthSky *esky = new pfEarthSky;
    esky->setMode(PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    esky->setAttr(PFES_GRND_HT, -1.0f * bsphere.radius);
    esky->setColor(PFES_GRND_FAR, 0.3f, 0.1f, 0.0f, 1.0f);
    esky->setColor(PFES_GRND_NEAR, 0.5f, 0.3f, 0.1f, 1.0f);
    chan->setESky(esky);
    
    
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
	chan->setView(view.xyz, view.hpr);
	
	// Initiate cull/draw for this frame.
	pfFrame();		
    }
    
    // Terminate parallel processes and exit.
    pfExit();
    
    return 0;
}
