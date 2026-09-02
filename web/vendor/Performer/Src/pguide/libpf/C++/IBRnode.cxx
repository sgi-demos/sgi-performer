//
// Copyright 2001, Silicon Graphics, Inc.
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
// IBRnode.C: illustrates the use of pfIBRnode and pfIBRtexture
//



#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfIBRnode.h>

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

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
	NULL};
    
    pw->chooseFBConfig(constraints);

    pw->open();

    int alpha_bits;
    pw->query(PFQWIN_ALPHA_BITS, &alpha_bits);
    printf("Alpha bits: %d\n", alpha_bits);
}


int main (int argc, char *argv[])
{
    float t = 0.0f;
    
    
    // Initialize Performer
    pfInit();
    
    
    // Use default multiprocessing mode based on number of
    // processors.
    //
    pfMultiprocess( PFMP_DEFAULT );			
    
    // initiate multi-processing mode set in pfMultiprocess call 
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    //
    pfConfig();			
    
    // Append to Performer search path, PFPATH, files in 
    //	    /usr/share/Performer/data */
    pfFilePath(".:/usr/share/Performer/data");
    
    // create a new pfScene
    pfScene *scene = new pfScene;

    // create a new pfIBRtexture
    pfIBRtexture *IBRtex = new pfIBRtexture;
    //IBRtex->setFlags(PFIBR_USE_2D_TEXTURES, 1);
    IBRtex->loadIBRTexture("ibr/tree/view%d.rgb", 128, 1);

    // create a new billboard
    pfGeoState *gstate = new pfGeoState;
    gstate->setMode(PFSTATE_ENTEXTURE, PF_ON);
 
    if(IBRtex->getFlags(PFIBR_USE_REG_COMBINERS))
    {
	gstate->setMultiMode(PFSTATE_ENTEXTURE, 1, PF_ON);

	if(IBRtex->getFlags(PFIBR_3D_VIEWS))
	{
	    gstate->setMultiMode(PFSTATE_ENTEXTURE, 2, PF_ON);
	    gstate->setMultiMode(PFSTATE_ENTEXTURE, 3, PF_ON);
	}
    }   
    gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    gstate->setMode(PFSTATE_ALPHAFUNC, PFAF_GREATER);
    gstate->setVal(PFSTATE_ALPHAREF, 0.03);
    gstate->setMode(PFSTATE_TRANSPARENCY, PF_ON);

    gstate->setAttr(PFSTATE_TEXTURE, IBRtex->getDefaultTexture());

	    
    pfTexEnv *tenv = new(_pfSharedArena) pfTexEnv;
    tenv->setMode(PFTE_MODULATE);
    gstate->setAttr(PFSTATE_TEXENV, (void *)tenv);
	    
    pfGeoSet *gset = new pfGeoSet();

    pfVec3 *coords = (pfVec3 *)pfMalloc(sizeof(pfVec3)*4, _pfSharedArena);
    coords[0].set(-0.5, 0.0, 0);
    coords[1].set( 0.5, 0.0, 0);
    coords[2].set( 0.5, 0.0, 1);
    coords[3].set(-0.5, 0.0, 1);
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);

    pfVec4 *color = (pfVec4 *)pfMalloc(sizeof(pfVec4), _pfSharedArena);
    color->set(1.0, 1.0, 1.0, 1.0);
    gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, color, NULL);

    pfVec2 *tcoords = (pfVec2 *)pfMalloc(sizeof(pfVec2)*4, _pfSharedArena);
    tcoords[0].set( 0, 0);
    tcoords[1].set( 1, 0);
    tcoords[2].set( 1, 1);
    tcoords[3].set( 0, 1);
    gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
    if(IBRtex->getFlags(PFIBR_USE_REG_COMBINERS))
    {
	gset->setMultiAttr(PFGS_TEXCOORD2, 1, PFGS_PER_VERTEX, tcoords, NULL);
	if(IBRtex->getFlags(PFIBR_3D_VIEWS))
	{
	    gset->setMultiAttr(PFGS_TEXCOORD2, 2, PFGS_PER_VERTEX, tcoords, NULL);
	    gset->setMultiAttr(PFGS_TEXCOORD2, 3, PFGS_PER_VERTEX, tcoords, NULL);
	}
    }       

    int *lengths = (int *)pfMalloc(sizeof(int));
    lengths[0] = 4;
    gset->setPrimLengths(lengths);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(1);
    
    gset->setGState(gstate);

    // create a new pfIBRnode (a child of pfBillboard)
    pfIBRnode *node = new pfIBRnode;
    node->setIBRtexture(IBRtex);
    node->setAngles(0, 0, 0.0);
    if(IBRtex->getFlags(PFIBR_3D_VIEWS))
	node->setMode(PFBB_ROT, PFBB_POINT_ROT_WORLD);
    else
	node->setMode(PFBB_ROT, PFBB_AXIAL_ROT);
    
    node->addGSet(gset);

    pfVec3 *pos = new pfVec3;
    pos->set(0,0,0);
    node->setPos(0, *pos);
	    
    scene->addChild(node);


    // Create a pfLightSource and attach it to scene
    scene->addChild(new pfLightSource);
    
    
    // Configure and open GL window
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setName("OpenGL Performer");
    pw->setOriginSize(0,0,500,500);
    pw->setConfigFunc(OpenPipeWin);
    pw->config();
    
    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(p);
    chan->setScene(scene);
    chan->setFOV(45.0f, 0.0f);
    
    // create an Earth/Sky model that draws sky only
    pfEarthSky	*eSky = new pfEarthSky;
    eSky->setMode(PFES_BUFFER_CLEAR, PFES_FAST);
    eSky->setColor(PFES_CLEAR, 0.3f, 0.3f, 0.7f, 1.0f);
    chan->setESky(eSky);
    
    // determine extent of scene's geometry
    pfSphere bsphere;
    scene->getBound(&bsphere);
    chan->setNearFar(1.0f, 10.0f * bsphere.radius);
    
    
    // Simulate for 30 seconds.
    while (t < 30.0f)
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
	view.xyz.set(0.8f * bsphere.radius * s, 
		     -0.8f * bsphere.radius *c, 
		     0.45f * bsphere.radius);
	chan->setView(view.xyz, view.hpr);
    }
    
    // Terminate parallel processes and exit
    pfExit();
    return 0;
}
