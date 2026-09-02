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
// torusLOD.C
//	OpenGL Performer example for creating GeoSets and LODs
//	    usage:
//		    lod		-runs sample program
//		    F1		-toggle stats display
//		    w		-toggle wireframe/Gouraud
//		    LEFTMOUSE:	- zoom in
//		    RIGHTMOUSE	- zoom out
// $Revision: 1.17 $ 
// $Date: 2000/10/06 21:26:28 $
//
#include <stdlib.h>
#include <X11/keysym.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfLOD.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfLight.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfMaterial.h>

static void drawChannel(pfChannel *chan, void *data);
static void processInput(void);

static int winOriginX = 20, winOriginY = 20;
static int winSizeX = 500, winSizeY = 500;
static int done = FALSE;
static pfCoord view;
static void openPipeWin(pfPipeWindow *);
static void initView(pfChannel *);
static void processInput(void);
static pfLOD *generateTorusLODs(void *);

static float *offset = NULL;

//
//	main() -- program entry point.
//
int
main (int argc, char *argv[])
{
    // initialize Performer
    pfInit();
    pfMultiprocess(PFMP_DEFAULT);
 
    // place "offset" in shared memory location before fork() call
    offset = (float*)pfMalloc(sizeof(float), pfGetSharedArena());
    if (offset != NULL)
	*offset = 0.0f;

    // initiate multi-processing mode
    pfConfig();			    
    
    // Set the graphics pipeline
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setWinType(PFPWIN_TYPE_X);
    pw->setName(argv[0]);
    pw->setOriginSize(winOriginX, winOriginY, winSizeX ,winSizeY);
    pw->setConfigFunc(openPipeWin);
    pw->config();
    
    pfPhase(PFPHASE_FLOAT);
    pfFrameRate(60.0f);
    
    // get the malloc arena
    void *arena = pfGetSharedArena();
    
    // create the root scene node */
    pfScene *scene = new pfScene;
    
    // Create a lit scene pfGeoState for the scene
    pfGeoState *gstate = new pfGeoState;
    gstate->setMode(PFSTATE_ENLIGHTING, PF_ON);

    // attach the pfGeoState to the scene
    scene->setGState(gstate);
    
    // create Level of Detail node
    pfLOD *lod = generateTorusLODs(arena);

    /* attach the Level of Detail node to the scene */
    pfDCS *dcs = new pfDCS();
    dcs->addChild(lod);
    scene->addChild(dcs);
    
    // initialize the view
    pfChannel *chan = new pfChannel(p);
    initView(chan);
    
    // attach the scene to the channel
    chan->setScene(scene);
    
    while(!done)
    {
	// update view position and direction   
	chan->setView(view.xyz, view.hpr);

	// update the torus' position
	dcs->setTrans(0.0f, *offset, 0.0f);
	
	// initiate traversal using current state
	pfFrame();
    }
    // terminate cull and draw processes
    pfExit();
    
    // exit to operating system
    return 0;
}

//
//    initView() --sets up the initial viewing parameters 
//
static void
initView(pfChannel *chan)
{
    chan->setNearFar(0.0001f, 1000.0f);
    chan->setFOV(45.0f, -1.0f);

    // set callback function for custom drawing
    chan->setTravFunc(PFTRAV_DRAW, drawChannel);
    
    // create an Earth/Sky model that draws sky only
    pfEarthSky	*eSky = new pfEarthSky;
    eSky->setMode(PFES_BUFFER_CLEAR, PFES_FAST);
    eSky->setColor(PFES_CLEAR, 0.3f, 0.3f, 0.7f, 1.0f);
    chan->setESky(eSky);
    
    // specify origin(X,Y,Z) and view direction(heading, pitch and roll)
    view.xyz.set(0.0f, -8.0f, 8.0f);
    view.hpr.set(0.0f,-40.0f, 0.0f);
    chan->setView(view.xyz, view.hpr);
    chan->setViewport(0.0f, 1.0f, 0.0f, 1.0f);
}


void
computeNormals(pfVec3 *verts, ushort *vl, pfVec3 *norms, ushort *nl, int numTris)
{
    int i, j;
    float *v0, *v1, *v2;
    pfVec3 vec0, vec1;
    
    for( j = 0, i = 0; i < numTris ; i++, j+=3)
    {
	v0 = verts[ vl[j]].vec;
	v1 = verts[ vl[j+1]].vec;
	v2 = verts[ vl[j+2]].vec;
	PFSUB_VEC3(vec0, v1, v0);
	PFSUB_VEC3(vec1, v2, v1);
	norms[i].cross(vec0, vec1);
	norms[i].normalize();
	nl[i] = i;
    }
}

//
//    createTorusGSet()--Sweeps out a torus using an approximation
//	     to a circle with numPoints. 
//
static pfGeoSet *
createTorusGSet(void *arena,ushort numPieces, ushort numPoints, float radius)
{
    pfVec4	*clr;
    pfVec3	*verts, *norms;
    ushort	*v_ilist, *c_ilist, *n_ilist;
    int		numVerts, numTris,index;
    ushort	i, base, limit, piece;
    float	phi, theta, thetaAngInc, phiAngInc;
    float	sTheta, cTheta, sPhi, cPhi;
    
    // create a new pfGeoSet
    pfGeoSet *gset = new pfGeoSet;
    
    // GeoSet will be comprised of independent triangles
    gset->setPrimType(PFGS_TRIS);
    numVerts	= numPieces*numPoints;
    numTris	= numPieces*2*numPoints;
    
    // create a lists for vertex and color values
    verts = (pfVec3 *)pfMalloc(sizeof(pfVec3)*numVerts,arena);
    norms = (pfVec3 *)pfMalloc(sizeof(pfVec3)*numTris, arena);
    clr   = (pfVec4 *)pfMalloc(sizeof(pfVec4), arena);
    // create indices lists for colors and triangle vertices
    v_ilist = (ushort *)pfMalloc(sizeof(ushort)*numTris*3, arena);
    c_ilist = (ushort *)pfMalloc(sizeof(ushort), arena);
    n_ilist = (ushort *)pfMalloc(sizeof(ushort)*numTris, arena);
    
    // set the color the entire GeoSet will be one color
    clr->set(0.5f, 0.1f, 0.1f, 1.0f);
    c_ilist[0] = 0;		    
    
    // set attributes for the GeoSet
    gset->setNumPrims(numTris);
    gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, clr, c_ilist);
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, verts, v_ilist);
    gset->setAttr(PFGS_NORMAL3, PFGS_PER_PRIM, norms, n_ilist);
    
    thetaAngInc = 360.0f/numPieces;
    phiAngInc   = 360.0f/numPoints;
    for(theta =0.0f,piece = 0,i=0; piece < numPieces; piece++)
    { 
	pfSinCos(theta, &sTheta, &cTheta);
	for( phi = 0.0f; phi < 360.0f; phi += phiAngInc )
	{// this loop generates each n-gonal piece.
	    pfSinCos(phi, &sPhi, &cPhi);
	    verts[i++].set((3*radius + radius*cPhi)*cTheta, 
			   (3*radius + radius*cPhi)*sTheta, radius*sPhi);
	}
	theta += thetaAngInc;
    }
    // now establish connectivity with the indexlist
    piece = index = 0;
    for(base = piece*numPoints; piece < numPieces; piece++)
    {
	limit = base + numPoints;
	for( i = base; i < (numPoints+base); i++)
	{// this loop connects each n-gonal piece to the next w/triangles
	    v_ilist[index++] = i;
	    v_ilist[index++] = (i + numPoints)%numVerts;
	    v_ilist[index++] = (((i + 1) < limit)?(i+1):base);
	    v_ilist[index++] = (((i + 1) < limit)?(i+1):base);
	    v_ilist[index++] = (i + numPoints)%numVerts;
	    v_ilist[index++] = ((((i + 1)<limit)?i+1:base)+numPoints)%numVerts;
	}
	base = limit ;
    }
    computeNormals(verts, v_ilist, norms,n_ilist, numTris);
    /*pfPrint(gset, 1, NULL);*/
    
    pfMaterial *mtl = new pfMaterial;
    mtl->setColor(PFMTL_SPECULAR, 0.6f, 0.0f, 0.6f); 
    mtl->setColorMode(PFMTL_FRONT, PFMTL_CMODE_AD);
    pfGeoState *gSt = new pfGeoState;
    gSt->setAttr(PFSTATE_FRONTMTL, (void *)mtl);
    gset->setGState(gSt);
    
    return(gset);
}

//  
//	generateTorusLODs()-- creates Torus LOD node
//
static pfLOD *
generateTorusLODs(void *arena)
{
    pfVec3   center;
    int	     i;
    ushort   numLOD = 4;
    ushort   numPoints = 2 << numLOD;
    ushort   numPieces = 2 << numLOD;
  
    pfLOD *lod = new pfLOD;
    for( i = 1; i <= numLOD; i++)
    {    
	// create a new Geode
	pfGeode *geode = new pfGeode;

    	// build a torus GeoSet
	pfGeoSet *gset = createTorusGSet(arena, numPieces,numPoints, 1.0f);

	// now add the GeoSet to the Geode
	geode->addGSet(gset);

	// add geode as another level of detail node
	lod->addChild(geode);

	// simplify the geometry
	numPoints >>= 1;
	numPieces >>= 1;
    }

    // set ranges for LODs, there should be (num LODs + 1) ranges
    lod->setRange(0,   0.0f);
    lod->setRange(1,  25.0f);
    lod->setRange(2,  32.0f);
    lod->setRange(3,  39.0f);
    lod->setRange(4, 100.0f);
    
    // set the center of the LOD
    center.set(0.0f, 0.0f, 0.0f);
    lod->setCenter(center);
    
    return(lod);
}

//
// Draw Process Routines
//

static int WinXSize, WinYSize, ShowStats=0;

//
//    openPipeWin()-- opens a GL window and initializes  
///
static void
openPipeWin(pfPipeWindow *pw)
{
    pw->open();
    
    pw->getSize(&WinXSize, &WinYSize);
    
    Display *dsp = pfGetCurWSConnection();
    Window win = pw->getWSWindow();
    XSelectInput(dsp, win, PointerMotionMask | KeyPressMask | 
		 ButtonPressMask | ButtonReleaseMask);
    XMapWindow(dsp, win);
    
    (new pfLight)->on();
}

//
//	drawChannel()-- Custom drawing routine callback
//
static void
drawChannel(pfChannel *chan, void *)
{
    // Must erase framebuffer and draw Earth-Sky model
    chan->clear();

    // get any input from GL
    processInput();	
		    
    // invoke Performer draw for this frame
    pfDraw();
    if (ShowStats)
	chan->drawStats();
}

//
//	processInput()-- handles X input queue events in draw process
//
static void 
processInput(void)
{
    int x,y;
    static int leftDown=0, rightDown=0;
    static float vy = 0.0f;
    
    Display *dsp = pfGetCurWSConnection();
    if (XEventsQueued(dsp, QueuedAfterFlush))
    while (XEventsQueued(dsp, QueuedAlready))
    {
	XEvent event;
	XNextEvent(dsp, &event);
	switch (event.type)
	{
	case MotionNotify: 
	    {
		XMotionEvent *motion_event = (XMotionEvent *) &event;
		x =  motion_event->x;
		y = WinYSize - motion_event->y;
	    }
	    break;
	case ButtonPress: 
	    {
		XButtonEvent *button_event = (XButtonEvent *) &event;
		x = event.xbutton.x;
		y = WinYSize - event.xbutton.y;
		switch (button_event->button) {
		case Button1:
		    leftDown = 1;
		    break;
		case Button3:
		    rightDown = 1;
		    break;
		}
	    }
	    break;
	case ButtonRelease:
	    {
		XButtonEvent *button_event = (XButtonEvent *) &event;
		x = event.xbutton.x;
		y = WinYSize - event.xbutton.y;
		switch (button_event->button) {
		case Button1:
		    leftDown = 0;
		    break;
		case Button3:
		    rightDown = 0;
		    break;
		}
	    }
	    break;
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
		    ShowStats = !ShowStats;
		    break;
		case XK_w:
		    if( pfGetEnable(PFEN_WIREFRAME))
			pfDisable(PFEN_WIREFRAME);
		    else
			pfEnable(PFEN_WIREFRAME);
		    break;
		default:
		    break;
		}
	    }
	}
    }
    
    if (leftDown && (vy < 20.0f)) // zoom in
	vy += 0.1f;
    if (rightDown && (vy > -2.0f)) // zoom out
	vy -= 0.1f;

    if (offset != NULL)
	*offset = vy;
}
