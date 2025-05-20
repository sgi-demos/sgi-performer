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
// morph_LOD.C: Example of a pfLOD controling a pfEngine(PFENG_MORPH) to
//		achieve a smooth morph LOD transition.
//
// $Revision: 1.5 $
// $Date: 2000/10/06 21:26:23 $
//



#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfLOD.h>
#include <Performer/pf/pfDCS.h>
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
static pfLOD *make_LOD(void);


int
main (int argc, char *argv[])
{
    float t = 0.0f;

    // Initialize Performer
    pfInit();

    // Use multiprocessing mode
    pfMultiprocess( PFMP_APP_CULL_DRAW );

    // initiate multi-processing mode set in pfMultiprocess call
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    //
    pfConfig();	

    pfLOD *lod = make_LOD();
    pfDCS *dcs = new pfDCS();
    dcs->addChild(lod);

    // Attach lod to a new pfScene
    pfScene *scene = new pfScene;
    scene->addChild(dcs);

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
    chan->setNearFar(1.0f, 100.0f);

    // Simulate for twenty seconds.
    while (t < 20.0f)
    {
	if (t <= 7.0f)
	    dcs->setTrans(0.0f, 2.5f + t * 0.5f, 0.0f);
	else if (t <= 14.0f)
	    dcs->setTrans(0.0f, 6.0f - (t-7.0f) * 0.5f, 0.0f);
	else
	    dcs->setTrans(0.0f, 2.5f + (t-14.0f) * 0.5f, 0.0f);

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


#define NUM_LOD0_COORDS	32

static pfLOD *make_LOD(void)
{
    pfVec3 *coords;
    pfVec3 *last_coords;
    int num_coords;
    int *lengths;
    pfLOD *lod;
    pfGeode *geode;
    pfGeoSet *gset;
    pfGeoState *gstate;
    pfFlux *flux_coords;
    pfFlux *range_flux;
    pfEngine *morph_engine;
    pfVec3 v;
    float scale;
    float s, c;
    int i, j, levels;

    /*
     *  make nearest LOD
     */
    coords = (pfVec3 *)pfMemory::malloc(sizeof(pfVec3) * NUM_LOD0_COORDS,
					     pfGetSharedArena());
    pfSinCos(0.0f, &s, &c);
    PFSET_VEC3(coords[0], s, 0.0f, c);
    for (i = 1; i < (NUM_LOD0_COORDS-1); i+=2)
    {
	pfSinCos(((i+1)/2) * (360.0f / NUM_LOD0_COORDS), &s, &c);
	PFSET_VEC3(coords[i],    s, 0.0f, c);
	PFSET_VEC3(coords[i+1], -s, 0.0f, c);
    }
    pfSinCos(180.0f, &s, &c);
    PFSET_VEC3(coords[NUM_LOD0_COORDS-1], s, 0.0f, c);

    lengths = (int *)pfMemory::malloc(sizeof(int) * 1, pfGetSharedArena());
    lengths[0] = NUM_LOD0_COORDS;

    gstate = new pfGeoState();
    gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    gstate->setMode(PFSTATE_ENTEXTURE, PF_OFF);
    gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);

    gset = new pfGeoSet();
    gset->setPrimType(PFGS_TRISTRIPS);
    gset->setNumPrims(1);
    gset->setPrimLengths(lengths);
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setGState(gstate);

    geode = new pfGeode();
    geode->addGSet(gset);

    lod = new pfLOD();
    PFSET_VEC3(v, 0.0f, 0.0f, 0.0f);
    lod->setCenter(v);
    lod->addChild(geode);

    /*
     *  Make other LODs and morphing transitions.
     */
    for (levels = 1, num_coords = NUM_LOD0_COORDS; num_coords >= 8;
	 num_coords >>= 1, levels += 2)
    {
	last_coords = coords;

	/*
	 *  lod[i] morphing level
	 */
	v.combine(0.5f, last_coords[0], 0.5f, last_coords[3]);
	scale = v.length();
	coords = (pfVec3 *)pfMemory::malloc(sizeof(pfVec3) * num_coords,
					    pfGetSharedArena());
	PFCOPY_VEC3(coords[0], last_coords[0]);
	for (i = 1, j = 3; i < (num_coords-1); i += 2, j += 4)
	{
	    PFCOPY_VEC3(coords[i],   last_coords[i]);
	    PFCOPY_VEC3(coords[i+1], last_coords[i+1]);
	    if (i % 4 == 3)
	    {
		PFCOPY_VEC3(coords[i],   last_coords[i]);
		PFCOPY_VEC3(coords[i+1], last_coords[i+1]);
	    }
	    else
	    {
		PFSCALE_VEC3(coords[i],   scale, last_coords[i]);
		PFSCALE_VEC3(coords[i+1], scale, last_coords[i+1]);
	    }
	}
	PFCOPY_VEC3(coords[num_coords-1], last_coords[num_coords-1]);

	flux_coords = new pfFlux(sizeof(pfVec3) * num_coords,
				 PFFLUX_DEFAULT_NUM_BUFFERS);
	flux_coords->initData(last_coords);

	range_flux = new pfFlux(sizeof(float), PFFLUX_DEFAULT_NUM_BUFFERS);
	range_flux->setMode(PFFLUX_PUSH, PF_ON);
	lod->setRangeFlux(levels, range_flux);

	morph_engine = new pfEngine(PFENG_MORPH);
	morph_engine->setSrc(PFENG_MORPH_FRAME, range_flux, NULL, 0, 0, 0);
	morph_engine->setSrc(PFENG_MORPH_SRC(0), last_coords, NULL, 0, 0, 3);
	morph_engine->setSrc(PFENG_MORPH_SRC(1), coords, NULL, 0, 0, 3);
	morph_engine->setIterations(num_coords, 3);
	morph_engine->setDst(flux_coords, NULL, 0, 3);

	lengths = (int *)pfMemory::malloc(sizeof(int) * 1, pfGetSharedArena());
	lengths[0] = num_coords;

	gset = new pfGeoSet();
	gset->setPrimType(PFGS_TRISTRIPS);
	gset->setNumPrims(1);
	gset->setPrimLengths(lengths);
	gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, flux_coords, NULL);
	gset->setGState(gstate);

	geode = new pfGeode();
	geode->addGSet(gset);
	lod->addChild(geode);

	/*
	 *  make lod[i+1] non morphing level
	 */
	coords = (pfVec3 *)pfMemory::malloc(sizeof(pfVec3) * num_coords / 2,
					    pfGetSharedArena());
	PFCOPY_VEC3(coords[0], last_coords[0]);
	for (i = 1, j = 3; i < (num_coords/2-1); i += 2, j += 4)
	{
	    PFCOPY_VEC3(coords[i],   last_coords[j]);
	    PFCOPY_VEC3(coords[i+1], last_coords[j+1]);
	}
	PFCOPY_VEC3(coords[num_coords/2-1], last_coords[num_coords-1]);

	lengths = (int *)pfMemory::malloc(sizeof(int) * 1, pfGetSharedArena());
	lengths[0] = num_coords / 2;

	gset = new pfGeoSet();
	gset->setPrimType(PFGS_TRISTRIPS);
	gset->setNumPrims(1);
	gset->setPrimLengths(lengths);
	gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	gset->setGState(gstate);

	geode = new pfGeode();
	geode->addGSet(gset);
	lod->addChild(geode);
    }

    lod->setRange(0, 0.0f);
    for (i = 1; i < levels; i++)
    {
	lod->setRange(i, 5.0f + i);
    }
    lod->setRange(levels, 1000.0f);

    return lod;
}

