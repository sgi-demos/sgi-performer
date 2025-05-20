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
// cullPgmSimple.C: simple Performer program to demonstrate cull programs
//
// $Revision: 1.5 $ 
// $Date: 2002/11/12 19:46:33 $
//



#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfCullProgram.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfDCS.h>
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
    int      testBin;
    int      boxBin;
} SharedData;

static SharedData *Shared;

static float xmin, xmax, ymin, ymax, hmin, hmax, xpos, ypos;


// user's draw function and open window function
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);


//
//	Usage() -- print usage advice and exit. This
//      procedure is executed in the application process.
//
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: cullPgmSimple simple file.ext (preferrable Performer town)\n");
    exit(1);
}


pfDCS *SetBox(pfChannel *chan) {
    // create the visible bounding box
    int *lengths = (int *)new (sizeof(int)*(3)) pfMemory;
    lengths[0] = 4;
    lengths[1] = 4;
    lengths[2] = 4 + 3*2;
	
    pfMaterial *mat = new pfMaterial;
    mat->setColor(PFMTL_EMISSION, 1, 1, 1);

    pfGeoState *gstate = new pfGeoState;
    gstate->setAttr(PFSTATE_FRONTMTL, (void *)mat);
    gstate->setAttr(PFSTATE_BACKMTL, (void *)mat);
    gstate->setMode(PFSTATE_ENWIREFRAME, PFTR_ON);
    gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);
    gstate->setMode(PFSTATE_ENLIGHTING, PFTR_ON);
    
    pfGeoSet *gset = new pfGeoSet;
    gset->setGState(gstate);
    gset->setPrimType(PFGS_TRISTRIPS);
    gset->setPrimLengths(lengths);
    gset->setNumPrims(3);

    pfVec3 *coords = (pfVec3 *)new (sizeof(pfVec3)*(4+4+4+6)) pfMemory; 

    xmin = ymin = -120;
    xmax = ymax = 120;
    hmin = -0.5;
    hmax = 80;
    
    int index = 0;
    //bottom
    coords[index++].set(xmin, ymin, hmin);
    coords[index++].set(xmin, ymax, hmin);
    coords[index++].set(xmax, ymin, hmin);
    coords[index++].set(xmax, ymax, hmin);

    // top
    coords[index++].set(xmin, ymax, hmax);
    coords[index++].set(xmin, ymin, hmax);
    coords[index++].set(xmax, ymax, hmax);
    coords[index++].set(xmax, ymin, hmax);

    // sides
    coords[index++].set(xmin, ymin, hmax);
    coords[index++].set(xmin, ymin, hmin);
	    
    coords[index++].set(xmax, ymin, hmax);
    coords[index++].set(xmax, ymin, hmin);

    coords[index++].set(xmax, ymax, hmax);
    coords[index++].set(xmax, ymax, hmin);

    coords[index++].set(xmin, ymax, hmax);
    coords[index++].set(xmin, ymax, hmin);

    coords[index++].set(xmin, ymin, hmax);
    coords[index++].set(xmin, ymin, hmin);

    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	
    pfGeode *bbox = new pfGeode;
    bbox->addGSet(gset);

    // add dcs to move the bbox around
    pfDCS *bboxdcs = new pfDCS;
    bboxdcs->addChild(bbox);

    // make a bin that would contain this box only
    // create a new bin
    Shared->boxBin = chan->getFreeBin();
    chan->setBinOrder(Shared->boxBin, Shared->boxBin);
    chan->setBinSort(Shared->boxBin, PFSORT_BY_STATE, NULL);
    // we don't want to run cull program on the box and we don't need
    // to draw it as a part of the scene
    chan->setBinFlags(Shared->boxBin, 
		      PFBIN_DONT_RUN_CULL_PROGRAM | PFBIN_DONT_DRAW_BY_DEFAULT);
    
    gset->setDrawBin(Shared->boxBin);

    return bboxdcs;
}

void SetCullPolytope(pfCullProgram *cullPg)
{
    pfPolytope *ptope;
    pfPlane facet;

    ptope = cullPg->getPolytope(0);
    
    ptope->getFacet(0, &facet);
    facet.offset = -(xpos + xmin);
    ptope->setFacet(0, &facet);
		    
    ptope->getFacet(1, &facet);
    facet.offset = xpos + xmax;
    ptope->setFacet(1, &facet);   

    ptope->getFacet(2, &facet);
    facet.offset = -(ypos + ymin);
    ptope->setFacet(2, &facet);
		    
    ptope->getFacet(3, &facet);
    facet.offset = ypos + ymax;
    ptope->setFacet(3, &facet);

    // the top and bottop does not change
}


int
main (int argc, char *argv[])
{
    float t = 0.0f;
    
    if (argc < 2)
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

    // initiate multi-processing mode set in pfMultiprocess call 
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    //
    pfConfig();			
    
    // Append to Performer search path, PFPATH, files in 
    //	    /usr/share/Performer/data */
     pfFilePath(".:/usr/share/Performer/data");
    
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
		 0.3f * bsphere.radius, 
		 0.02f * bsphere.radius);
    chan->setView(view.xyz, view.hpr);

    // set user's draw function
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    
    // set light source
    pfLightSource *ls = new pfLightSource;
    ls->setPos(-0.3f, -0.3f, 1.0f, 0.0f);
    ls->setColor(PFLT_AMBIENT, 0.6, 0.6, 0.6);
    ls->setColor(PFLT_DIFFUSE, 0.8, 0.8, 0.6);
    scene->addChild(ls);

    // create the box
    pfDCS *bboxdcs = SetBox(chan);

    t = pfGetTime();
    pfSinCos(15.0f*t, &s, &c);
    xpos = 2600+c*140;
    ypos = 500+s*140;
    bboxdcs->setTrans(xpos,ypos,0);

    // add bbox to the scene
    scene->addChild(bboxdcs);
 
    ///////////////////////////////////////////////////////////////
    // create a new bin
    Shared->testBin = chan->getFreeBin();
    chan->setBinOrder(Shared->testBin, Shared->testBin);
    chan->setBinSort(Shared->testBin, PFSORT_BY_STATE, NULL);
    chan->setBinFlags(Shared->testBin, 
		      PFBIN_DONT_DRAW_BY_DEFAULT);

    // set cull program
    pfCullProgram *cullPg = chan->getCullProgram();
    cullPg->resetCullProgram(PFCULLPG_GEOSET_PROGRAM);
    cullPg->addCullProgramOpcode(PFCULLPG_TEST_POLYTOPE, 0);
    cullPg->addCullProgramOpcode(PFCULLPG_ADD_BIN_MAYBE, Shared->testBin);
    cullPg->addCullProgramOpcode(PFCULLPG_RETURN, 0);
    
    // set cull program polytope
    cullPg->setNumPolytopes(1);
    pfPolytope *ptope = new pfPolytope(6);
    pfPlane facet;

    // initialize 6 facets (set only normals)
    facet.normal.vec[0] = -1;
    facet.normal.vec[1] = 0.0;
    facet.normal.vec[2] = 0.0;
    ptope->setFacet(0, &facet);
		    
    facet.normal.vec[0] = 1;
    ptope->setFacet(1, &facet);

    facet.normal.vec[0] = 0;
    facet.normal.vec[1] = -1;
    ptope->setFacet(2, &facet);

    facet.normal.vec[1] = 1;
    ptope->setFacet(3, &facet);
		    		    		    
    facet.normal.vec[1] = 0;
    facet.normal.vec[2] = -1;
    facet.offset = -hmin;
    ptope->setFacet(4, &facet);
    
    facet.normal.vec[2] = 1;
    facet.offset = hmax;
    ptope->setFacet(5, &facet);
    
    cullPg->setPolytope(0, ptope);
 
    // set the cull program bit
    chan->setTravMode(PFTRAV_CULL,  PFCULL_ALL | PFCULL_PROGRAM);
    
    // Simulate for twenty seconds.
    while (t < 20.0f)
    {
	pfCoord	   view;
	float      s, c;
	
	// Go to sleep until next frame time.
	pfSync();		
 
	// Compute box position, rotate its center around a point
	t = pfGetTime();
	pfSinCos(15.0f*t, &s, &c);
	xpos = 2600+c*140;
	ypos = 2500+s*140;
	bboxdcs->setTrans(xpos,ypos,0);

	// update cull program polytope
	SetCullPolytope(cullPg);
	
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
	NULL};
    
    pw->chooseFBConfig(constraints);

    pw->open();

    int rgb_bits;
    pw->query(PFQWIN_RGB_BITS, &rgb_bits);
    printf("RGB bits: %d\n", rgb_bits);

    // make sure we have stencil buffer
    int stencil_bits = -1;
    pw->query(PFQWIN_STENCIL_BITS, &stencil_bits);
    printf("Stencil bits: %d\n", stencil_bits);

    if(stencil_bits <= 0) {
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, 
		 "Couldn't get visual with stencil buffer. Exiting.\n");
	exit(1);
    }
}

// DrawChannel:
// Renders the part of the scene that intersects the box in wireframe.
// 1. First, the scene is drawn and the wireframe is off
// 2. The box is drawn in wireframe.
// 3. To determine what points are inside the box:
//   a) the front faces of the box are drawn incrementing stencil for pixels
//      in front of scene pixels
//   b) the back faces of the box are drawn decrementing stencil for pixels
//      in front of scene pixels
// 4. The pixels with stencil == 1 are set to black (the box is drawn again)
// 5. Last, the scene is drawn again, this time wireframe is on.
// Cull programs allow us to reduce draw time, in this case we do not have
// to render the whole scene in the last step, but only the part of the
// scene that intersects the box.
static void
DrawChannel (pfChannel *channel, void *)
{
    // erase framebuffer and draw Earth-Sky model 
    channel->clear();
    glClear(GL_STENCIL_BUFFER_BIT);

    ////////////////////////////////////////////////////////////////////
    // draw scene 
    glDisable(GL_STENCIL_TEST);
    pfDraw();

    // draw the box, disable updates in depth buffer
    glDepthMask(GL_FALSE);
    pfDrawBin(Shared->boxBin);

    // disable wireframe
    pfOverride(PFSTATE_ENWIREFRAME, PF_OFF);
    pfDisable(PFEN_WIREFRAME);
    pfOverride(PFSTATE_ENWIREFRAME, PF_ON);

    /////////////////////////////////////////////////////////////////////
    // draw front faces of the box, adding to stencil when z test passes
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glStencilFunc(GL_ALWAYS, 0x01, 0xff);
    // allow writing to stencil buffer only
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    // cull out back faces
    pfOverride(PFSTATE_CULLFACE, PF_OFF);
    pfCullFace(PFCF_BACK);
    pfOverride(PFSTATE_CULLFACE, PF_ON);
    pfDrawBin(Shared->boxBin);

    /////////////////////////////////////////////////////////////////////
    // draw back faces of the box, subtracting from stencil when z test 
    // passes
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    // cull out front faces
    pfOverride(PFSTATE_CULLFACE, PF_OFF);
    pfCullFace(PFCF_FRONT);
    pfOverride(PFSTATE_CULLFACE, PF_ON);
    pfDrawBin(Shared->boxBin);

    pfOverride(PFSTATE_CULLFACE, PF_OFF);
    pfCullFace(PFCF_OFF);

    /////////////////////////////////////////////////////////////////////
    // set color of pixels with stencil == 1 to black
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glColor3f(0,0,0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 0x01, 0xff);	    
    pfOverride(PFSTATE_ENLIGHTING | PFSTATE_FRONTMTL | PFSTATE_BACKMTL | 
	       PFSTATE_COLORTABLE, PF_ON);
    
    pfDrawBin(Shared->boxBin);
    pfOverride(PFSTATE_ENLIGHTING | PFSTATE_FRONTMTL | PFSTATE_BACKMTL | 
	       PFSTATE_COLORTABLE, PF_OFF);

    /////////////////////////////////////////////////////////////////////
    // draw the part of the scene that intersects the box, but only where
    // stencil is set to 1
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    // disable dept test
    glDepthFunc(GL_ALWAYS);
    glStencilFunc(GL_EQUAL, 0x01, 0xff);	    
    // enable wireframe
    pfOverride(PFSTATE_ENWIREFRAME, PF_OFF);
    pfEnable(PFEN_WIREFRAME);
    pfOverride(PFSTATE_ENWIREFRAME, PF_ON);

    pfDrawBin(Shared->testBin);

    glDisable(GL_STENCIL_TEST);
    glDepthFunc(GL_LEQUAL);

    pfOverride(PFSTATE_ENWIREFRAME, PF_OFF);
    pfDisable(PFEN_WIREFRAME);
    
}
      
  
