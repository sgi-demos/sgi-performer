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
// flux_switch.C: example of contoling a pfSwitch with a pfFlux.
//
// $Revision: 1.4 $
// $Date: 2000/10/06 21:26:22 $
//



#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfSwitch.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfSwitch.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfFlux.h>


#include <Performer/pfutil.h>
#include <Performer/pfdu.h>


//
// globals
//
pfFlux *switch_val_flux;


//
// local prototypes
//
static pfSwitch *make_switch(void);


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

    pfSwitch *sw = make_switch();

    // Attach sw to a new pfScene
    pfScene *scene = new pfScene;
    scene->addChild(sw);

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
	float *switch_val;

	switch_val = (float*)switch_val_flux->getWritableData();
	switch ((int)(t * 2.0f) % 4)
	{
	    case 0:
		*switch_val = PFSWITCH_ON;
		break;
	    case 1:
		*switch_val = PFSWITCH_OFF;
		break;
	    case 2:
		*switch_val = 0.0f;
		break;
	    case 3:
		*switch_val = 1.0f;
		break;
	}
	switch_val_flux->writeComplete();

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


static pfSwitch *make_switch(void)
{
    pfSwitch *sw;
    pfGeode *geode;
    pfGeoSet *gset;
    pfGeoState *gstate;
    pfVec3 *coords;
    float switch_val;

    switch_val_flux = new pfFlux(sizeof(float), PFFLUX_DEFAULT_NUM_BUFFERS);
    switch_val = 0.0f;
    switch_val_flux->initData(&switch_val);

    gstate = new pfGeoState();
    gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    gstate->setMode(PFSTATE_ENTEXTURE, PF_OFF);
    gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);

    sw = new pfSwitch();

    coords = (pfVec3 *)pfMemory::malloc(sizeof(pfVec3) * 3, pfGetSharedArena());
    PFSET_VEC3(coords[0], -0.5f, 4.0f,  1.0f);
    PFSET_VEC3(coords[1], -0.8f, 4.0f, -1.0f);
    PFSET_VEC3(coords[2], -0.2f, 4.0f, -1.0f);
    gset = new pfGeoSet();
    gset->setPrimType(PFGS_TRIS);
    gset->setNumPrims(1);
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setGState(gstate);
    geode = new pfGeode();
    geode->addGSet(gset);
    sw->addChild(geode);

    coords = (pfVec3 *)pfMemory::malloc(sizeof(pfVec3) * 3, pfGetSharedArena());
    PFSET_VEC3(coords[0],  0.5f, 4.0f,  1.0f);
    PFSET_VEC3(coords[1],  0.2f, 4.0f, -1.0f);
    PFSET_VEC3(coords[2],  0.8f, 4.0f, -1.0f);
    gset = new pfGeoSet();
    gset->setPrimType(PFGS_TRIS);
    gset->setNumPrims(1);
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setGState(gstate);
    geode = new pfGeode();
    geode->addGSet(gset);
    sw->addChild(geode);

    sw->setValFlux(switch_val_flux);

    return sw;
}

