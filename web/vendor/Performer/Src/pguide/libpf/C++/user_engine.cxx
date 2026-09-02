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
// user_engine.C: example of a simple user defined pfEngine and pfFlux chain
//		  being used by a pfGeoSet
//
// $Revision: 1.4 $
// $Date: 2000/10/06 21:26:28 $
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
pfFlux *flux_control;


//
// local prototypes
//
static pfGeode *make_geode(void);
static void scale_tri(void);
void my_mult_engine(pfEngine *eng);


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
	// scale the triangle
	scale_tri();

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
    pfGeode *geode;
    pfGeoSet *gset;
    pfGeoState *gstate;
    pfFlux *flux_coords;
    pfEngine *user_engine;
    float s, c;

    coords = (pfVec3 *)pfMemory::malloc(sizeof(pfVec3) * 3, pfGetSharedArena());
    pfSinCos(30.0f, &s, &c);
    PFSET_VEC3(coords[0], 0.0f, 4.0f, 1.0f);
    PFSET_VEC3(coords[1],    c, 4.0f,   -s);
    PFSET_VEC3(coords[2],   -c, 4.0f,   -s);

    flux_coords = new pfFlux(sizeof(pfVec3) * 3, PFFLUX_DEFAULT_NUM_BUFFERS);
    flux_coords->initData(coords);

    flux_control = new pfFlux(sizeof(float) * 3, PFFLUX_DEFAULT_NUM_BUFFERS);
    flux_control->setMode(PFFLUX_PUSH, PF_ON);

    user_engine = new pfEngine(PFENG_USER_FUNCTION);
    user_engine->setUserFunction(my_mult_engine);
    user_engine->setSrc(0, coords, NULL, 0, 0, 3);
    user_engine->setSrc(1, flux_control, NULL, 0, 0, 0);
    user_engine->setIterations(3, 3);
    user_engine->setDst(flux_coords, NULL, 0, 3);

    gstate = new pfGeoState();
    gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    gstate->setMode(PFSTATE_ENTEXTURE, PF_OFF);
    gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);

    gset = new pfGeoSet();
    gset->setPrimType(PFGS_TRIS);
    gset->setNumPrims(1);
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, flux_coords, NULL);
    gset->setGState(gstate);

    geode = new pfGeode();
    geode->addGSet(gset);

    return geode;
}


/*
 *  Since c goes from 1.0 to -1.0, this causes the triangle to scale and
 *  invert.
 */
static void scale_tri(void)
{
    static int count = 0;
    float *control;
    float s, c;

    pfSinCos((float)((count*2)%360), &s, &c);

    control = (float *)flux_control->getWritableData();

    control[0] = control[2] = c * 1.5f;
    control[1] = 1.0f;

    flux_control->writeComplete();

    count++;
}


/*
 *  User defined engine function to multiply 2 sources
 *
 *  This engine does not handle indexed sources or destination for simplicity.
 */
void my_mult_engine(pfEngine *eng)
{
    int i, j;
    void *dst, *src0, *src1;
    float *d, *s0, *s1;
    int d_stride, s_stride0, s_stride1;
    int d_offset, s_offset0, s_offset1;
    ushort *d_ilist, *s_ilist0, *s_ilist1;
    int s_icount0, s_icount1;
    int iterations, items;

    eng->getDst(&dst, &d_ilist, &d_offset, &d_stride);
    eng->getSrc(0, &src0, &s_ilist0, &s_icount0, &s_offset0, &s_stride0);
    eng->getSrc(1, &src1, &s_ilist1, &s_icount1, &s_offset1, &s_stride1);

    if (eng->getNumSrcs() != 2 || dst == NULL || src0 == NULL || src1 == NULL)
    {
	fprintf(stderr, "my_mult_engine(0x%08x) not configured correctly.\n",
		eng);
	return;
    }

    eng->getIterations(&iterations, &items);

    d = (float*)((pfFlux*)dst)->getWritableData();
    if (d == NULL)
	return;
    d += d_offset;

    if (pfMemory::isOfType(src0, pfFlux::getClassType()))
	s0 = (float *)((pfFlux *)src0)->getCurData();
    else
	s0 = (float *)src0;
    s0 += s_offset0;

    if (pfMemory::isOfType(src1, pfFlux::getClassType()))
	s1 = (float *)((pfFlux *)src1)->getCurData();
    else
	s1 = (float *)src1;
    s1 += s_offset0;

    for (i = 0; i < iterations; i++)
    {
	for (j = 0; j < items; j++)
	    d[j] = s0[j] * s1[j];
	d += d_stride;
	s0 += s_stride0;
	s1 += s_stride1;
    }
}

