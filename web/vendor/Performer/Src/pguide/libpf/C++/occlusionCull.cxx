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
// occlusionCull.C: 
//
// $Revision: 1.5 $ 
// $Date: 2002/12/07 02:48:00 $
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
    pfVec3      *coords;
} SharedData;

static SharedData *Shared;

static float xmin, xmax, ymin, ymax, hmin, hmax, xpos, ypos;


// user's draw function
static void DrawChannel(pfChannel *chan, void *data);


/*****************************************************************************/
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

/*****************************************************************************/
void GetNormal(pfVec3 *norm, pfVec3 v1, pfVec3 v2, pfVec3 v3)
{
    pfVec3 vec1, vec2;

    vec1 = v2 - v1;
    vec2 = v3 - v1;
    norm->cross(vec1, vec2);
    norm->normalize();
}

/*****************************************************************************/
void SetCullPolytope(pfCullProgram *cullPg, pfChannel *chan, 
		     pfVec3 *coords, pfMatrix mat)
{
    pfPolytope *ptope;
    pfPlane facet;
    pfVec3  eye, pt[4];
    int     i;

    chan->getEye(eye);

    for(i=0; i<4; i++)
	pt[i].xformPt(coords[i], mat);

    ptope = cullPg->getPolytope(0);

    // initialize 5 facets 
    GetNormal(&facet.normal, pt[0], pt[1], pt[2]);
    facet.offset = facet.normal.dot(pt[0]);
    ptope->setFacet(0, &facet);
		    
    GetNormal(&facet.normal, eye, pt[0], pt[1]);
    facet.offset = facet.normal.dot(eye);
    ptope->setFacet(1, &facet);

    GetNormal(&facet.normal, eye, pt[1], pt[2]);
    facet.offset = facet.normal.dot(eye);
    ptope->setFacet(2, &facet);
		    
    GetNormal(&facet.normal, eye, pt[2], pt[3]);
    facet.offset = facet.normal.dot(eye);
    ptope->setFacet(3, &facet);
		    
    GetNormal(&facet.normal, eye, pt[3], pt[0]);
    facet.offset = facet.normal.dot(eye);
    ptope->setFacet(4, &facet);

    cullPg->setPolytope(0, ptope);
}


pfDCS *SetRectangle(void) 
{
    // create the visible bounding box
    int *lengths = (int *)pfMalloc(sizeof(int));
    lengths[0] = 4;
	
    pfMaterial *mat = new pfMaterial;
    mat->setColor(PFMTL_EMISSION, 1, 1, 1);

    pfGeoState *gstate = new pfGeoState;
    gstate->setAttr(PFSTATE_FRONTMTL, (void *)mat);
    gstate->setAttr(PFSTATE_BACKMTL, (void *)mat);
    gstate->setMode(PFSTATE_ENWIREFRAME, PFTR_ON);
    gstate->setMode(PFSTATE_ENLIGHTING, PFTR_ON);
    
    pfGeoSet *gset = new pfGeoSet;
    gset->setGState(gstate);
    gset->setPrimType(PFGS_POLYS);
    gset->setPrimLengths(lengths);
    gset->setNumPrims(1);

    pfVec3 *coords = (pfVec3 *)pfMalloc(sizeof(pfVec3)*4); 

    xmin = ymin = -80;
    xmax = ymax = 80;
    hmin = 1;
    hmax = 80;
    
    int index = 0;

    coords[index++].set(xmin, ymin, hmin);
    coords[index++].set(xmax, ymin, hmin);
    coords[index++].set(xmax, ymin, hmax);
    coords[index++].set(xmin, ymin, hmax);

    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    Shared->coords = coords;

    pfGeode *rect = new pfGeode;
    rect->addGSet(gset);

    // add dcs to move the bbox around
    pfDCS *rectdcs = new pfDCS;
    rectdcs->addChild(rect);

    return rectdcs;
}


int
main (int argc, char *argv[])
{
    float t = 0.0f;
    int i;
    
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

	// set a default file path
	pfFilePath("/usr/share/Performer/data/:/usr/share/Performer/data/town/");

    // Load all loader DSO's before pfConfig() forks 
    for(i=1; i<argc; i++)
	pfdInitConverter(argv[i]);

    // initiate multi-processing mode set in pfMultiprocess call 
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    //
    pfConfig();			
    
    // Append to Performer search path, PFPATH, files in 
    //	    /usr/share/Performer/data */
     pfFilePath(".:/usr/share/Performer/data");
    
    pfNode *root;

    pfScene *scene = new pfScene;

    for(i=1; i<argc; i++)
    {
	root = pfdLoadFile(argv[i]);
	if (root == NULL)
	{
	    pfExit();
	    exit(-1);
	}
	else
	    // Attach loaded file to a new pfScene
	    //if(i==1)
	    scene->addChild(root);
    }
    
    // Create a pfLightSource and attach it to scene
    scene->addChild(new pfLightSource);
    
    
    // Configure and open GL window
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setName("OpenGL Performer");
    pw->setOriginSize(0,0,1024,768);
    pw->open();
    
    // Create and configure a pfChannel.
    pfChannel *chan = new pfChannel(p);
    chan->setScene(scene);
    chan->setFOV(45.0f, 0.0f);

    // determine extent of scene's geometry
    pfSphere bsphere;
    scene->getBound(&bsphere);
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

    // create the occluder (rectangle)
    pfDCS *rectdcs = SetRectangle();

    t = pfGetTime();
    pfSinCos(15.0f*t, &s, &c);
    xpos = 2600+c*140;
    ypos = 500+s*140;
    rectdcs->setTrans(xpos,ypos,0);

    // add rectangle to the scene
    scene->addChild(rectdcs);
 
    ///////////////////////////////////////////////////////////////

    // set cull program
    pfCullProgram *cullPg = chan->getCullProgram();
    cullPg->resetCullProgram(PFCULLPG_GEOSET_PROGRAM | PFCULLPG_NODE_PROGRAM);
    cullPg->addCullProgramOpcode(PFCULLPG_TEST_POLYTOPE, 0);
    cullPg->addCullProgramOpcode(PFCULLPG_RETURN, PFCULLPG_CULL_ON_ALL_IN);
    
    // set cull program polytope
    cullPg->setNumPolytopes(1);
    pfPolytope *ptope = new pfPolytope(5);    
    cullPg->setPolytope(0, ptope);
 
    // set the cull program bit
    chan->setTravMode(PFTRAV_CULL,  PFCULL_ALL | PFCULL_PROGRAM);

    t = 0;

    // Simulate for 60 seconds.
    while (t < 60.0f)
    {
	pfCoord	   view;
	float      s, c;
	
	// Go to sleep until next frame time.
	pfSync();		
 
	// Compute box position, rotate its center around a point
	t += 1.0/30.0; //pfGetTime();
	pfSinCos(15.0f*t, &s, &c);
	xpos = 2600+c*140;
	ypos = 2500+s*140;
	rectdcs->setTrans(xpos,ypos,0);

	// update cull program polytope
	pfMatrix mat;
	rectdcs->getMat(mat);
	SetCullPolytope(cullPg, chan, Shared->coords, mat);
	
	// Initiate cull/draw for this frame.
	pfFrame();
    }
    
    // Terminate parallel processes and exit
    pfExit();
    return 0;
}

static void
DrawChannel (pfChannel *channel, void *)
{
    // erase framebuffer and draw Earth-Sky model 
    channel->clear();
    
    // draws only geosets whose bounding box intersects the rotating box
    pfDraw();
}
      
  
