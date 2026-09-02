//
// Copyright 1997, Silicon Graphics, Inc.
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
// strobe_engine.C: example of a PFENG_STROBE pfEngine to simulate a group
//		    of strobing lights.
//
// $Revision: 1.4 $
// $Date: 2000/10/06 21:26:26 $
//



#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfFlux.h>
#include <Performer/pr/pfEngine.h>


#include <Performer/pfutil.h>
#include <Performer/pfdu.h>


//
// globals
//


//
// local prototypes
//
static pfGeode *make_geode(void);


int
main(int argc, char *argv[])
{
    float t = 0.0f;

    // Initialize Performer
    pfInit();

    // Use multiprocessing mode
    pfMultiprocess(PFMP_APP_CULL_DRAW);

    // initiate multi-processing mode set in pfMultiprocess call
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    //
    pfConfig();

    pfGeode *geode = make_geode();

    // Attach geode to a new pfScene
    pfScene *scene = new pfScene;
    scene->addChild(geode);

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
    chan->setNearFar(1.0f, 10.0f);

    // Simulate for twenty seconds.
    while (t < 20.0f)
    {
	// Go to sleep until next frame time.
	pfSync();

	// Initiate cull/draw for this frame.
	pfFrame();

	t = pfGetTime();
    }

    // Terminate parallel processes and exit
    pfExit();
    return 0;
}


static pfGeode *make_geode(void)
{
    pfVec3 *coords;
    pfVec4 *colors;
    pfVec3 *timing;
    pfSphere bsph;
    pfGeode *geode;
    pfGeoSet *gset;
    pfGeoState *gstate;
    pfEngine *strobe_engine;
    pfFlux *flux_colors;
    pfFlux *frame_time_flux;
    pfFlux *flux_frame;
    float *time_scale;

    coords = (pfVec3 *)pfMemory::malloc(sizeof(pfVec3) * 11,
					pfGetSharedArena());
    PFSET_VEC3(coords[0], -0.8f, 4.0f, -1.0f);
    PFSET_VEC3(coords[1], -0.8f, 4.0f, -0.8f);
    PFSET_VEC3(coords[2], -0.8f, 4.0f, -0.6f);
    PFSET_VEC3(coords[3], -0.8f, 4.0f, -0.4f);
    PFSET_VEC3(coords[4], -0.8f, 4.0f, -0.2f);
    PFSET_VEC3(coords[5], -0.4f, 4.0f,  0.0f);
    PFSET_VEC3(coords[6], -0.4f, 4.0f,  0.2f);
    PFSET_VEC3(coords[7],  0.0f, 4.0f,  0.4f);
    PFSET_VEC3(coords[8],  0.0f, 4.0f,  0.6f);
    PFSET_VEC3(coords[9],  0.4f, 4.0f,  0.8f);
    PFSET_VEC3(coords[10], 0.8f, 4.0f,  1.0f);

    colors = (pfVec4 *)pfMemory::malloc(sizeof(pfVec4) * 11,
					pfGetSharedArena());
    PFSET_VEC4(colors[0],  1.0f, 1.0f, 1.0f, 1.0f);
    PFSET_VEC4(colors[1],  1.0f, 1.0f, 1.0f, 1.0f);
    PFSET_VEC4(colors[2],  1.0f, 1.0f, 1.0f, 1.0f);
    PFSET_VEC4(colors[3],  1.0f, 1.0f, 1.0f, 1.0f);
    PFSET_VEC4(colors[4],  1.0f, 1.0f, 1.0f, 1.0f);
    PFSET_VEC4(colors[5],  1.0f, 1.0f, 1.0f, 1.0f);
    PFSET_VEC4(colors[6],  1.0f, 1.0f, 1.0f, 1.0f);
    PFSET_VEC4(colors[7],  1.0f, 1.0f, 1.0f, 1.0f);
    PFSET_VEC4(colors[8],  1.0f, 1.0f, 1.0f, 1.0f);
    PFSET_VEC4(colors[9],  1.0f, 1.0f, 1.0f, 1.0f);
    PFSET_VEC4(colors[10], 1.0f, 1.0f, 1.0f, 1.0f);

    timing = (pfVec3 *)pfMemory::malloc(sizeof(pfVec3) * 11,
					pfGetSharedArena());
    PFSET_VEC3(timing[0],  0.5f, 0.5f, 0.0f);
    PFSET_VEC3(timing[1],  0.5f, 0.5f, 0.1f);
    PFSET_VEC3(timing[2],  0.5f, 0.5f, 0.2f);
    PFSET_VEC3(timing[3],  0.5f, 0.5f, 0.3f);
    PFSET_VEC3(timing[4],  0.5f, 0.5f, 0.4f);
    PFSET_VEC3(timing[5],  0.1f, 0.1f, 0.0f);
    PFSET_VEC3(timing[6],  0.1f, 0.1f, 0.1f);
    PFSET_VEC3(timing[7],  1.5f, 0.5f, 0.0f);
    PFSET_VEC3(timing[8],  0.5f, 1.5f, 0.5f);
    PFSET_VEC3(timing[9],  1.0f, 1.0f, 0.0f);
    PFSET_VEC3(timing[10], 0.1f, 2.9f, 0.0f);

    flux_colors = new pfFlux(sizeof(pfVec4) * 11, PFFLUX_DEFAULT_NUM_BUFFERS);
    flux_colors->initData(colors);

    flux_frame = new pfFlux(sizeof(float), PFFLUX_DEFAULT_NUM_BUFFERS);
    flux_frame->setMode(PFFLUX_PUSH, PF_ON);

    frame_time_flux = pfGetFrameTimeFlux();
    time_scale = (float*)pfMemory::malloc(sizeof(float) * 4,
					  pfGetSharedArena());

    strobe_engine = new pfEngine(PFENG_STROBE);
    strobe_engine->setSrc(PFENG_STROBE_TIME, frame_time_flux, NULL, 0, 0, 0);
    strobe_engine->setSrc(PFENG_STROBE_TIMING, timing, NULL, 0, 0, 3);
    strobe_engine->setSrc(PFENG_STROBE_ON, colors, NULL, 0, 3, 4);
    strobe_engine->setDst(flux_colors, NULL, 3, 4);
    strobe_engine->setIterations(11, 1);

    gstate = new pfGeoState();
    gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    gstate->setMode(PFSTATE_ENTEXTURE, PF_OFF);
    gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);
    gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_ON);

    gset = new pfGeoSet();
    gset->setPrimType(PFGS_POINTS);
    gset->setNumPrims(11);
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, flux_colors, NULL);
    gset->setGState(gstate);
    gset->setPntSize(3.0f);

    geode = new pfGeode();
    geode->addGSet(gset);

    return geode;
}

