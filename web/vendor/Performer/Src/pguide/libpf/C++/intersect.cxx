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
///////////////////////////////////////////////////////////////////////// 
//                                                                        
// intersectC.C   - Intersection testing sample                            
//                                                                        
// $Revision: 1.18 $ 
// $Date: 2002/11/10 02:49:28 $                         
//                                                                        
///////////////////////////////////////////////////////////////////////// 

#include <stdio.h>
#include <math.h>
#ifndef WIN32
#include <X11/keysym.h>
#endif
#include <stdlib.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfEarthSky.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfSCS.h>
#include <Performer/pr/pfGeoSet.h>

typedef struct SharedData
{
    int exitFlag;
    int showStats;
    float	yval;
    float	zval;
} SharedData;


static void DrawChannel (pfChannel *chan, void *data);
static void OpenPipeWin (pfPipeWindow *pw);

SharedData	*shared;

int 
main(int argc, char *argv[])
{
    pfSCS		*scs[10];
    pfMatrix	mat;
    pfCoord		view;
    pfSegSet  	segset;
    int		loop;
    
    // Initialize Performer 
    pfInit();
    
    // Use default multiprocessing mode based on number of processors. 
    pfMultiprocess(PFMP_DEFAULT);
    
    // Malloc storage in shared memory region for shared data   
    shared = (SharedData *) pfMalloc(sizeof(SharedData),pfGetSharedArena());
    
    // Configure multiprocessing mode and start parallel processes 
    pfConfig();
    
    shared->exitFlag = 0; 
    shared->showStats = 0;
    
    //
    // Configure and open GL window
    //
    pfPipe *pipe = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(pipe);
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setName(argv[0]);
    pw->setOriginSize(0, 0, 500, 500);
    // Open and configure the GL window. 
    pw->setConfigFunc(OpenPipeWin);
    pw->config();
    
    pfFrameRate(30.0f);
    
    // Create & configure channel 
    pfChannel *chan = new pfChannel(pipe);
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    chan->setNearFar(0.1f, 10000.0f);
    chan->setFOV(45.0f, -1.0f);
    
    // Add an earth/sky effect 
    pfEarthSky *esky = new pfEarthSky();
    esky->setMode(PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    esky->setAttr(PFES_GRND_HT, 0.0f);
    esky->setColor(PFES_GRND_FAR, 0.3f, 0.1f, 0.0f, 1.0f);
    esky->setColor(PFES_GRND_NEAR, 0.5f, 0.3f, 0.1f, 1.0f);
    chan->setESky(esky);
    
    // 
    // Create pfScene node, attach it to channel
    //
    pfScene *scene = new pfScene();
    chan->setScene(scene);
    
    // Create group node to hold SCS's; attach it to scene 
    pfGroup *root = new pfGroup();
    scene->addChild(root);
    
    // Create 10 SCS nodes, spaced equally apart.  Make each one
    //   a child of 'root', and make 'geode' a child of each SCS    
    pfGeode *geode = new pfGeode();
    for (loop=0; loop < 10; loop++)
    {
	mat.makeTrans(0.0f, (float) loop * 10.0f + 10.0f, 0.0f);
	scs[loop] = new pfSCS(mat);
	scs[loop]->addChild(geode);
	root->addChild(scs[loop]);
	
	// set up SCS's (and their children) for intersections 
	scs[loop]->setTravMask(PFTRAV_ISECT, 1, PFTRAV_SELF,
			       PF_SET);
    }
    
    // Create a GeoSet to hold the ramp geometry 
    //        pfGeoSet *gset = new pfGeoSet(pfGetSharedArena());
    
    pfVec3 *verts = (pfVec3*) new(8*sizeof(pfVec3)) pfMemory;
    verts[0].set(-1.0f, -1.0f,  1.0f);
    verts[1].set( 1.0f, -1.0f,  1.0f);
    verts[2].set( 1.0f,  1.0f,  1.0f);
    verts[3].set(-1.0f,  1.0f,  1.0f);
    verts[4].set(-2.0f, -2.0f,  0.0f);
    verts[5].set( 2.0f, -2.0f,  0.0f);
    verts[6].set( 2.0f,  2.0f,  0.0f);
    verts[7].set(-2.0f,  2.0f,  0.0f);
    
    ushort *vindex = (ushort*) new(24*sizeof(ushort)) pfMemory;
    vindex[0]  = 0; vindex[1]  = 1; vindex[2]  = 2; vindex[3]  = 3; /* front */
    vindex[4]  = 0; vindex[5]  = 3; vindex[6]  = 7; vindex[7]  = 4; /* left */
    vindex[8]  = 4; vindex[9]  = 7; vindex[10] = 6; vindex[11] = 5; /* back */
    vindex[12] = 1; vindex[13] = 5; vindex[14] = 6; vindex[15] = 2; /* right */
    vindex[16] = 3; vindex[17] = 2; vindex[18] = 6; vindex[19] = 7; /* top */
    vindex[20] = 0; vindex[21] = 4; vindex[22] = 5; vindex[23] = 1; /* bottom */
    
    pfVec4 *colors = (pfVec4*) new(4*sizeof(pfVec4)) pfMemory;
    colors[0].set(1.0f, 0.0f, 0.0f, 0.5f);
    colors[1].set(0.0f, 1.0f, 0.0f, 0.5f);
    colors[2].set(0.0f, 0.0f, 1.0f, 0.5f);
    colors[3].set(1.0f, 1.0f, 1.0f, 0.5f);
    
    ushort *cindex = (ushort*) new(24*sizeof(ushort)) pfMemory;
    cindex[0]  = 0; cindex[1]  = 1; cindex[2]  = 2; cindex[3]  = 3;
    cindex[4]  = 0; cindex[5]  = 1; cindex[6]  = 2; cindex[7]  = 3;
    cindex[8]  = 0; cindex[9]  = 1; cindex[10] = 2; cindex[11] = 3;
    cindex[12] = 0; cindex[13] = 1; cindex[14] = 2; cindex[15] = 3;
    cindex[16] = 0; cindex[17] = 1; cindex[18] = 2; cindex[19] = 3;
    cindex[20] = 0; cindex[21] = 1; cindex[22] = 2; cindex[23] = 3;
    
    pfGeoSet *gset = new pfGeoSet;
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, verts, vindex);
    gset->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, colors, cindex);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(6);
    
    // attach it to the parent geode 
    geode->addGSet(gset);
    
    // 
    // set initial view position
    //
    shared->yval=0.0f;
    shared->zval=0.5f;
    
    // set up intersection segment, pointing down 
    //   for "terrain" following 
    segset.activeMask = 1;
    segset.isectMask = 0xFFFF;
    segset.discFunc = NULL;
    segset.bound = NULL;
    segset.mode = PFTRAV_IS_PRIM|PFTRAV_IS_NORM|PFTRAV_IS_CULL_BACK;
    segset.segs[0].dir.set(0.0f, 0.0f, -1.0f);
    segset.segs[0].length = 50000.0f;
    
    //
    // main loop
    //
    while (!shared->exitFlag)
    {
	pfHit	  **hits[32];
	int	  isect;
	
	view.xyz.set(0.0f, shared->yval, shared->zval);
	view.hpr.set(0.0f, -5.0f, 0.0f);
	
	chan->setView(view.xyz, view.hpr);
	pfFrame();
	
	// increment view position; if too far, reset 
	shared->yval += 0.1f;
	if (shared->yval > 110.0f) shared->yval = 0.0f;
	
	// update location of intersection segment 
	segset.segs[0].pos.set(0.0f, shared->yval,shared->zval);
	
	// do an intersection test against the scene graph 
	isect = root->isect(&segset, hits);
	
	// if successful, set our height to that of
	// the point of contact, plus a small offset  
	if (isect)
	{
	    pfVec3 pnt, xpnt;
	    pfMatrix xmat;
	    (*hits[0])->query(PFQHIT_POINT, pnt.vec);
	    // transform object point into scene coordinates 
	    (*hits[0])->query(PFQHIT_XFORM, (float*)xmat.mat);
	    xpnt.xformPt(pnt, xmat);
	    shared->zval = xpnt[PF_Z] + 0.5f;
	}
    }
    
    pfExit();
    return 0;
}


// 
// Pipe Initialization Callback to open and configure 
// GL window, and set up GL event handling 
//

static void 
OpenPipeWin(pfPipeWindow *pw)
{
#ifndef WIN32
    Display *dsp;
    Window win;
#endif
    
    pw->open();
    
#ifndef WIN32
    dsp = pfGetCurWSConnection();
    win = pw->getWSWindow();
    XSelectInput(dsp, win, KeyPressMask);
    XMapWindow(dsp, win);
#endif
}


// 
// Draw Process Callback to draw the channel and handle GL events 
//
static void DrawChannel (pfChannel *chan, void *)
{
#ifndef WIN32
    Display *dsp;
#endif

    // clear and draw the channel 
    chan->clear();
    pfDraw();
    
#ifndef WIN32
    dsp = pfGetCurWSConnection();
    if (XEventsQueued(dsp, QueuedAfterFlush))
    while (XEventsQueued(dsp, QueuedAlready))
    {
	XEvent event;
	XNextEvent(dsp, &event);
	switch (event.type)
	{
	case KeyPress:
            {
                char buf[100];
                int rv;
                KeySym ks;
                rv = XLookupString(&event.xkey, buf, sizeof(buf), &ks, 0);
                switch(ks) 
		{
                case XK_Escape:
                    exit(0);
                    break;
		case XK_F1:
		case XK_g:
		    shared->showStats = !shared->showStats;
		    break;
		default:
		    break;
		}
	    }
	}
    }
#endif
    
    if (shared->showStats) chan->drawStats();
}

///////////////////////////////////////////////////////////////////////// 
