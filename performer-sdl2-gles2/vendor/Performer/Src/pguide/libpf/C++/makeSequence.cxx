//
// Copyright 2002, Silicon Graphics, Inc.
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
// makeSequence.C: example of making a movie on a billboard.
//



#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfSwitch.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pf/pfSequence.h>
#include <Performer/pf/pfBillboard.h>
#include <Performer/pr/pfTexture.h>


#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

static float width = 1, height = 1;
static float speed = 1.0;

static pfSequence *makeSequence(int num, char *imageName)
{
    pfSequence  *seq;
    pfBillboard *billb;
    pfGeoSet    *gset;
    pfGeoState  *gstate;
    pfTexture   *tex;
    int         i;
    char        name[2048];

    seq = new pfSequence;

    // common values
    pfTexEnv *tenv = new pfTexEnv;
    tenv->setMode(PFTE_MODULATE);

    // use width and height
    pfVec3 *coords = (pfVec3 *)pfMalloc(sizeof(pfVec3)*4, 
					_pfSharedArena);
    coords[0].set(-0.5*width, 0.0, 0);
    coords[1].set( 0.5*width, 0.0, 0);
    coords[2].set( 0.5*width, 0.0, height);
    coords[3].set(-0.5*width, 0.0, height);

    pfVec4 *color = (pfVec4 *)pfMalloc(sizeof(pfVec4), _pfSharedArena);
    color->set(1.0, 1.0, 1.0, 1.0);

    pfVec2 *tcoords = (pfVec2 *)pfMalloc(sizeof(pfVec2)*4, _pfSharedArena);
    tcoords[0].set( 1, 0);
    tcoords[1].set( 0, 0);
    tcoords[2].set( 0, 1);
    tcoords[3].set( 1, 1);

    int *lengths = (int *)pfMalloc(sizeof(int), _pfSharedArena);
    lengths[0] = 4;
    

    for(i=0 ;i<num; i++)
    {
	// load the texture
	tex = new pfTexture();
	sprintf(name, imageName, i);
	tex->setFormat(PFTEX_INTERNAL_FORMAT, PFTEX_RGBA_8);
	tex->loadFile(name);

	// create a new billboard

	gstate = new pfGeoState;
	gstate->setMode(PFSTATE_ENTEXTURE, PF_ON);
	
	gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
	gstate->setMode(PFSTATE_ALPHAFUNC, PFAF_GREATER);
	gstate->setVal(PFSTATE_ALPHAREF, 0.03);
	gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_HIGH_QUALITY);
	
	gstate->setAttr(PFSTATE_TEXTURE, tex);
	
	gstate->setAttr(PFSTATE_TEXENV, (void *)tenv);
	
	gset = new pfGeoSet();
	
	gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, color, NULL);
	gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
	
	gset->setPrimLengths(lengths);
	gset->setPrimType(PFGS_QUADS);
	gset->setNumPrims(1);
	
	gset->setGState(gstate);
	
	// create a new pfIBRnode (a child of pfBillboard)
	billb = new pfBillboard;
	
	billb->setMode(PFBB_ROT, PFBB_AXIAL_ROT);
	billb->addGSet(gset);
	
	pfVec3 *pos = (pfVec3 *)pfMalloc(sizeof(pfVec3), _pfSharedArena);
	pos->set(0,0,0);
	billb->setPos(0, *pos);

	seq->addChild(billb);
	
	seq->setTime(i, 1.0 / 60.0);
    }

    seq->setDuration(speed, -1);

    seq->setInterval(PFSEQ_CYCLE, 0, num-1);

    seq->setMode(PFSEQ_START);

    return seq;
}

int
main(int argc, char *argv[])
{
    float t = 0.0f;
    int   i, numTex;
    char  *fileName;
    char  *outputName = NULL;

    numTex = 0;

    if(argc < 2)
    {
	printf("USAGE: makeSequence -n num_tex -s xsize ysize -o outputFile -sp speed texFileNameFormat\n");
	exit(1);
    }

    /* process commmand line args */
    for (i = 1; i < argc; ++i) {
	if (!strcmp("-n", argv[i])) {
	    numTex = atoi(argv[++i]);
	}
	else if (!strcmp("-sp", argv[i])) {
	     speed = atof(argv[++i]);
	}
	else if (!strcmp("-s", argv[i])) {
	     width = atof(argv[++i]);
	     height = atof(argv[++i]);
	}
	else if (!strcmp("-o", argv[i])) {
	     outputName = argv[++i];
	}
	else {
	    break;
	}
    }
    fileName = argv[i];

    // Initialize Performer
    pfInit();

    // Use multiprocessing mode
    pfMultiprocess(PFMP_APP_CULL_DRAW);

    if(outputName)
	pfdInitConverter(outputName);

    // initiate multi-processing mode set in pfMultiprocess call
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    //
    pfConfig();

    pfSequence *seq = makeSequence(numTex, fileName);

    // Attach sw to a new pfScene
    pfScene *scene = new pfScene;
    scene->addChild(seq);

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
    chan->makeOrtho(-width*0.5, width*0.5, -height*0.5, height*0.5);
    chan->setNearFar(1, -1);
    pfVec3 xyz, hpr;
    xyz.set(0,-0.5,0.5*height);
    hpr.set(0,0,0);
    chan->setViewOffsets(xyz, hpr);

    if(outputName)
	pfdStoreFile(seq, outputName);

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
