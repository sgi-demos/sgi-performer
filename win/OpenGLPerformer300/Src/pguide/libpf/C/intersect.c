/*
 * Copyright 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * This source code ("Source Code") was originally derived from a
 * code base owned by Silicon Graphics, Inc. ("SGI")
 * 
 * LICENSE: SGI grants the user ("Licensee") permission to reproduce,
 * distribute, and create derivative works from this Source Code,
 * provided that: (1) the user reproduces this entire notice within
 * both source and binary format redistributions and any accompanying
 * materials such as documentation in printed or electronic format;
 * (2) the Source Code is not to be used, or ported or modified for
 * use, except in conjunction with OpenGL Performer; and (3) the
 * names of Silicon Graphics, Inc.  and SGI may not be used in any
 * advertising or publicity relating to the Source Code without the
 * prior written permission of SGI.  No further license or permission
 * may be inferred or deemed or construed to exist with regard to the
 * Source Code or the code base of which it forms a part. All rights
 * not expressly granted are reserved.
 * 
 * This Source Code is provided to Licensee AS IS, without any
 * warranty of any kind, either express, implied, or statutory,
 * including, but not limited to, any warranty that the Source Code
 * will conform to specifications, any implied warranties of
 * merchantability, fitness for a particular purpose, and freedom
 * from infringement, and any warranty that the documentation will
 * conform to the program, or any warranty that the Source Code will
 * be error free.
 * 
 * IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
 * LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
 * ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
 * SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
 * OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
 * PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
 * OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
 * USE, THE SOURCE CODE.
 * 
 * Contact information:  Silicon Graphics, Inc., 
 * 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
 * or:  http://www.sgi.com
 */
/* ////////////////////////////////////////////////////////////////////// */
/*                                                                        */
/* intersect.c   - Intersection testing sample                            */
/*                                                                        */
/* $Revision: 1.26 $ $Date: 2002/11/14 00:02:43 $                         */
/*                                                                        */
/* ////////////////////////////////////////////////////////////////////// */

#include <stdio.h>
#include <math.h>
#include <Performer/pf.h>
#ifndef WIN32
#include <X11/keysym.h>
#endif

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

static pfVec3  scoords[] ={	{ -1.0f, -1.0f, 1.0f }, 
				{  1.0f, -1.0f, 1.0f },
				{  1.0f,  1.0f, 1.0f }, 
				{ -1.0f,  1.0f, 1.0f },
				{ -2.0f, -2.0f, 0.0f }, 
				{  2.0f, -2.0f, 0.0f },
				{  2.0f,  2.0f, 0.0f }, 
				{ -2.0f,  2.0f, 0.0f }};

static ushort  svindex[] ={	0, 1, 2, 3, 
				0, 3, 7, 4,
				4, 7, 6, 5, 
				1, 5, 6, 2,
				3, 2, 6, 7, 
				0, 4, 5, 1};


static pfVec4  scolors[] ={	{1.0f, 0.0f, 0.0f, 0.5f},
				{0.0f, 1.0f, 0.0f, 0.5f},
				{0.0f, 0.0f, 1.0f, 0.5f},
				{1.0f, 1.0f, 1.0f, 0.5f}};

static ushort  scindex[] ={	0, 1, 2, 3,
				0, 1, 2, 3,
				0, 1, 2, 3,
				0, 1, 2, 3,
				0, 1, 2, 3,
				0, 1, 2, 3};


/* ////////////////////////////////////////////////////////////////////// */

int 
main(int argc, char *argv[])
{
	pfPipe		*pipe;
	pfPipeWindow    *pw;
	pfChannel	*chan;
	pfScene		*scene;
	pfEarthSky	*eSky;
	pfGroup		*root;
	pfGeoSet	*gset;
	pfSCS		*scs[10];
	pfGeode		*geode;
	pfMatrix	mat;
	pfCoord		view;
	pfSegSet  	segset;
	int		loop;

	/* Initialize Performer */
	pfInit();

	/* Use default multiprocessing mode based on number of processors. */
	pfMultiprocess(PFMP_DEFAULT);

	/* Malloc storage in shared memory region for shared data   */
	shared = (SharedData *) pfMalloc(sizeof(SharedData),pfGetSharedArena());

	/* Configure multiprocessing mode and start parallel processes */
	pfConfig();

	shared->exitFlag = 0; 
	shared->showStats = 0;

	/* ////////////////////////////////////////////////////////////// */
	/* Configure and open GL window */
	pipe = pfGetPipe(0);
	pw = pfNewPWin(pipe);
	pfPWinType(pw, PFWIN_TYPE_X);
	pfPWinName(pw, argv[0]);
	pfPWinOriginSize(pw, 0, 0, 500, 500);
	/* Open and configure the GL window. */
	pfPWinConfigFunc(pw, OpenPipeWin);
	pfConfigPWin(pw);

	pfFrameRate(30.0f);

	/* Create & configure channel */
	chan = pfNewChan(pipe);
	pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);
	pfChanNearFar(chan, 0.1f, 10000.0f);
	pfChanFOV(chan, 45.0f, -1.0f);

	/* Add an earth/sky effect */
	eSky = pfNewESky();
	pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
	pfESkyAttr(eSky, PFES_GRND_HT, 0.0f);
	pfESkyColor(eSky, PFES_GRND_FAR, 0.3f, 0.1f, 0.0f, 1.0f);
	pfESkyColor(eSky, PFES_GRND_NEAR, 0.5f, 0.3f, 0.1f, 1.0f);
	pfChanESky(chan, eSky);

	/* ////////////////////////////////////////////////////////////// */
	/* Create pfScene node, attach it to channel */
	scene = pfNewScene();
	pfChanScene(chan, scene);

	/* Create group node to hold SCS's; attach it to scene */
	root = pfNewGroup();
	pfAddChild(scene, root);

	/* Create 10 SCS nodes, spaced equally apart.  Make each one
	   a child of 'root', and make 'geode' a child of each SCS    */
	geode = pfNewGeode();
	for (loop=0; loop < 10; loop++)
	{
		pfMakeTransMat(mat, 0.0f, (float) loop * 10.0f + 10.0f, 0.0f);
		scs[loop] = pfNewSCS(mat);
		pfAddChild(scs[loop], geode);
		pfAddChild(root, scs[loop]);

		/* set up SCS's (and their children) for intersections */
		pfNodeTravMask(scs[loop], PFTRAV_ISECT, 1, 
			PFTRAV_SELF, PF_SET);

	}

	/* Create a GeoSet to hold the ramp geometry */
	gset = pfNewGSet(pfGetSharedArena());
        pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, scoords, svindex);
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, scolors,scindex);
        pfGSetPrimType(gset, PFGS_QUADS);
        pfGSetNumPrims(gset, 6);

	/* attach it to the parent geode */
	pfAddGSet(geode, gset);

	/* ////////////////////////////////////////////////////////////// */
	/* set initial view position */
	shared->yval=0.0f;
	shared->zval=0.5f;

	/* set up intersection segment, pointing down 
	   for "terrain" following */
	segset.activeMask = 1;
	segset.isectMask = 0xFFFF;
	segset.discFunc = NULL;
	segset.bound = NULL;
	segset.mode = PFTRAV_IS_PRIM|PFTRAV_IS_NORM|PFTRAV_IS_CULL_BACK;
	pfSetVec3(segset.segs[0].dir, 0.0f, 0.0f, -1.0f);
	segset.segs[0].length = 50000.0f;

	/* ////////////////////////////////////////////////////////////// */
	/* main loop */
	while (!shared->exitFlag)
	{
		pfHit	  **hits[32];
		int	  isect;

		pfSetVec3(view.xyz, 0.0f, shared->yval, shared->zval);
		pfSetVec3(view.hpr, 0.0f, -5.0f, 0.0f);

		pfChanView(chan, view.xyz, view.hpr);
		pfFrame();

		/* increment view position; if too far, reset */
		shared->yval += 0.1f;
		if (shared->yval > 110.0f) shared->yval = 0.0f;

		/* update location of intersection segment */
		pfSetVec3(segset.segs[0].pos, 0.0f, shared->yval,shared->zval);

		/* do an intersection test against the scene graph */
		isect = pfNodeIsectSegs(root, &segset, hits);

		/* if successful, set our height to that of
		   the point of contact, plus a small offset  */
		if (isect)
		{
			pfVec3 pnt, xpnt;
			pfMatrix xmat;
			pfQueryHit(*hits[0], PFQHIT_POINT, &pnt);
			/* transform object point into scene coordinates */
			pfQueryHit(*hits[0], PFQHIT_XFORM, xmat);
			pfXformPt3(xpnt, pnt, xmat);
			shared->zval = xpnt[PF_Z] + 0.5;
		}
	}

	pfExit();
	return 0;
}


/* ////////////////////////////////////////////////////////////////////// */
/* Pipe Initialization Callback to open and configure 
 * GL window, and set up GL event handling */

   
static void 
OpenPipeWin(pfPipeWindow *pw)
{
#ifndef WIN32
    Display *dsp;
#endif
    Window win;
    
    pfOpenPWin(pw);

#ifndef WIN32
    dsp = pfGetCurWSConnection();
    win = pfGetPWinWSWindow(pw);
    XSelectInput(dsp, win, KeyPressMask);
    XMapWindow(dsp, win);
#endif
    pfDisable(PFEN_LIGHTING);
}


/* /////////////////////////////////////////////////////////////////////// */
/* Draw Process Callback to draw the channel and handle GL events */

static void DrawChannel (pfChannel *chan, void *data)
{
#ifndef WIN32
    Display *dsp;
#endif

    /* clear and draw the channel */
    pfClearChan(chan);
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
    if (shared->showStats) pfDrawChanStats(chan);
}

/* /////////////////////////////////////////////////////////////////////// */
