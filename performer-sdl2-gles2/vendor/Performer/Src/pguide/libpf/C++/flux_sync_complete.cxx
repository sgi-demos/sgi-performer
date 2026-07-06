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
// flux_sync_complete.C: This example is based on sum_engine.C.  It
//			 shows how to syncronize the completion of
//			 pfFluxes.  If the -u flag is used the triangeles
//			 will be out of sync.
//
// $Revision: 1.7 $
// $Date: 2002/12/03 03:20:08 $
//



#include <stdlib.h>
#include <string.h>

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
pfFlux *l_flux_control, *r_flux_control;
int do_sync = 1;

//
// This program assumes that it can successfully configure Performer for
// multiple processes (APP_CULL_DRAW). 
//
// Since the WIN32 version of Performer is single process-only, a pfFlux
// allocates only a single buffer. We must force pfFlux to allocate more 
// buffers in order to demonstrate the syncGroup functionality.
//
#ifdef WIN32
#define	add_flux_buffers  5
#else
#define	add_flux_buffers  0
#endif


//
// local prototypes
//
static pfGeode *make_geode(void);
static void shift_tri(void);
static void usage(void);


int
main (int argc, char *argv[])
{
    float t = 0.0f;

    if (argc == 2 && strcmp(argv[1], "-u") == 0)
	do_sync = 0;
    else if (argc != 1)
	usage();

    // Initialize Performer
    pfInit();

    // Use multiprocessing mode
    pfMultiprocess( PFMP_APP_CULL_DRAW );

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
	// shift the triangle
	shift_tri();

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
    pfVec3 *l_coords, *r_coords;
    pfGeode *geode;
    pfGeoSet *l_gset, *r_gset;
    pfGeoState *gstate;
    pfFlux *l_flux_coords, *r_flux_coords;
    pfEngine *l_sum_engine, *r_sum_engine;
    uint sync_group;
    float s, c;

    l_coords = (pfVec3 *)pfMemory::malloc(sizeof(pfVec3) * 3,
					  pfGetSharedArena());
    r_coords = (pfVec3 *)pfMemory::malloc(sizeof(pfVec3) * 3,
					  pfGetSharedArena());
    pfSinCos(30.0f, &s, &c);
    PFSET_VEC3(l_coords[0], 0.0f, 4.0f, -1.0f);
    PFSET_VEC3(l_coords[1],   -c, 4.0f,    -s);
    PFSET_VEC3(l_coords[2], 0.0f, 4.0f,  1.0f);
    PFSET_VEC3(r_coords[0], 0.0f, 4.0f,  1.0f);
    PFSET_VEC3(r_coords[1],    c, 4.0f,    -s);
    PFSET_VEC3(r_coords[2], 0.0f, 4.0f, -1.0f);

    sync_group = pfFlux::getNamedSyncGroup("flux_sync_complete");
    l_flux_coords = new pfFlux(sizeof(pfVec3) * 3, 
			(PFFLUX_DEFAULT_NUM_BUFFERS + add_flux_buffers));
    l_flux_coords->initData(l_coords);
    l_flux_coords->setSyncGroup(sync_group);
    r_flux_coords = new pfFlux(sizeof(pfVec3) * 3, 
			(PFFLUX_DEFAULT_NUM_BUFFERS + add_flux_buffers));
    r_flux_coords->initData(r_coords);
    r_flux_coords->setSyncGroup(sync_group);
    if (do_sync)
	pfFlux::enableSyncGroup(sync_group);

    l_flux_control = new pfFlux(sizeof(float), 
			(PFFLUX_DEFAULT_NUM_BUFFERS + add_flux_buffers));
    l_flux_control->setMode(PFFLUX_PUSH, PF_ON);
    r_flux_control = new pfFlux(sizeof(float), 
			(PFFLUX_DEFAULT_NUM_BUFFERS + add_flux_buffers));
    r_flux_control->setMode(PFFLUX_PUSH, PF_ON);

    l_sum_engine = new pfEngine(PFENG_SUM);
    l_sum_engine->setSrc(PFENG_SUM_SRC(0), l_coords, NULL, 0, 0, 3);
    l_sum_engine->setSrc(PFENG_SUM_SRC(1), l_flux_control, NULL, 0, 0, 0);
    l_sum_engine->setIterations(3, 1);
    l_sum_engine->setDst(l_flux_coords, NULL, 0, 3);
    r_sum_engine = new pfEngine(PFENG_SUM);
    r_sum_engine->setSrc(PFENG_SUM_SRC(0), r_coords, NULL, 0, 0, 3);
    r_sum_engine->setSrc(PFENG_SUM_SRC(1), r_flux_control, NULL, 0, 0, 0);
    r_sum_engine->setIterations(3, 1);
    r_sum_engine->setDst(r_flux_coords, NULL, 0, 3);

    gstate = new pfGeoState();
    gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    gstate->setMode(PFSTATE_ENTEXTURE, PF_OFF);
    gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);

    l_gset = new pfGeoSet();
    l_gset->setPrimType(PFGS_TRIS);
    l_gset->setNumPrims(1);
    l_gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, l_flux_coords, NULL);
    l_gset->setGState(gstate);
    r_gset = new pfGeoSet();
    r_gset->setPrimType(PFGS_TRIS);
    r_gset->setNumPrims(1);
    r_gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, r_flux_coords, NULL);
    r_gset->setGState(gstate);

    geode = new pfGeode();
    geode->addGSet(l_gset);
    geode->addGSet(r_gset);

    return geode;
}


static void shift_tri(void)
{
    static int count = 0;
    static int state = 0;
    float *control;
    float s, c;

    pfSinCos((float)((count*6*2)%360), &s, &c);

    switch (state)
    {
	case 0:
	    control = (float *)l_flux_control->getWritableData();
	    *control = s * 0.5f;
	    l_flux_control->writeComplete();
	    break;
	case 3:
	    control = (float *)r_flux_control->getWritableData();
	    *control = s * 0.5f;
	    r_flux_control->writeComplete();
	    break;
	case 5:
	    pfFlux::syncGroupReady(1);
	    state = -1;
	    count++;
	    break;
    }

    state++;
}


//
//	usage() -- print usage advice and exit. This
//      procedure is executed in the application process.
//
static void usage(void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: flux_sync_complete [-u]\n");
    exit(1);
}
