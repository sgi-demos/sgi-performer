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
/* pfisect.c   - Old-style intersection compatibility sample             */
/*                                                                        */
/* ////////////////////////////////////////////////////////////////////// */

/*
  
IMPORTANT: DO NOT USE THIS CODE FOR NEW DEVELOPMENT.

The pfuSegsIsectNode functions in libpfutil emulate old-style
Performer 1.0/1.1 intersection functions on top of the new 1.2
pfNodeIsectSegs API.  They are slower than pfNodeIsectSegs.

pfuSegsIsectNode is provided strictly as a temporary fix when
quick source code porting is required and will go away in a
future release.

To aid in porting to the new pfNodeIsectSegs API, compare this code
with the version of this program in pguide/libpf/C/intersect.c

*/

/*

As the nodes (in the subtree passed to pfNodeIsectSegs) are traversed
for an intersection test, their "intersection traversal mask value"
(which is 0xFFFF by default, or you can set explicitly) is AND-ed
with the mask you pass to pfNodeIsectSegs.

If the result is non-zero, then the traversal continues by recursing
down into the children of the node (or geosets, if it's a geode).

So what does this give you?  You can select certain objects that you
don't want to bother intersecting with.  In the code I'm sending, the
scene hierarchy is set up with a single geode multiply referenced by
10 SCS's.  I've given consecutive intersection masks (0-9) for each
SCS, and have told pfNodeIsectSegs that I want to only intersect with
nodes whose mask AND's with 0x1 or 0x4.  So:

  Node     Mask      Mask AND (0x1 | 0x4)
  SCS 0    0000              0000   <--- fail
  SCS 1    0001              0001
  SCS 2    0010              0000   <--- fail
  SCS 3    0011              0001
  SCS 4    0100              0100
  SCS 5    0101              0101
  SCS 6    0110              0100
  SCS 7    0111              0101
  SCS 8    1000              0000   <--- fail
  SCS 9    1001              0001

You'll see as the program runs that we go "through" the first, 3rd,
and 9th pyramid but go "over" (successfully intersect) with all the
others.

*/

/* ////////////////////////////////////////////////////////////////////// */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <Performer/pf.h>
#include <Performer/pr.h>
#include "pfutil.h"

#define	X	0
#define	Y	1
#define	Z	2


typedef struct SharedData
{
	int exitFlag;
	int showStats;
	int fastTextFlag;
	float	xval;
	float	yval;
	float	zval;
} SharedData;

/* ////////////////////////////////////////////////////////////////////// */

static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeline(pfPipe *p);

void		*arena;
SharedData	*shared;

pfVec3  scoords[] ={    { -1.0f, -1.0f, 1.0f }, 
			{  1.0f, -1.0f, 1.0f },
                        {  1.0f,  1.0f, 1.0f }, 
			{ -1.0f,  1.0f, 1.0f },
                        { -2.0f, -2.0f, 0.0f }, 
			{  2.0f, -2.0f, 0.0f },
                        {  2.0f,  2.0f, 0.0f }, 
			{ -2.0f,  2.0f, 0.0f }};

ushort  svindex[] ={    0, 1, 2, 3, 0, 3, 7, 4,
                        4, 7, 6, 5, 1, 5, 6, 2,
                        3, 2, 6, 7, 0, 4, 5, 1};


pfVec4  scolors[] ={	{1.0f, 0.0f, 0.0f, 0.5f},
			{0.0f, 1.0f, 0.0f, 0.5f},
			{0.0f, 0.0f, 1.0f, 0.5f},
			{1.0f, 1.0f, 1.0f, 0.5f}};

ushort  scindex[] ={	0, 1, 2, 3,
			0, 1, 2, 3,
			0, 1, 2, 3,
			0, 1, 2, 3,
			0, 1, 2, 3,
			0, 1, 2, 3};

/* ////////////////////////////////////////////////////////////////////// */

void main()
{
	pfPipe		*pipe;
	pfChannel	*chan;
	pfEarthSky	*eSky;
	pfScene		*scene;
	pfGroup		*root;
	pfGeoSet	*gset;
	pfSCS		*scs[10];
	pfGeode		*geode;
	pfMatrix	mat;
	pfCoord		view;
	int		loop;

	pfInit();
	pfMultiprocess(PFMP_APPCULLDRAW);
	pfConfig();

	arena = pfGetSharedArena();
	shared = (SharedData *) pfMalloc(sizeof(SharedData), arena);
	shared->exitFlag = 0; 
	shared->showStats = 0;

	pipe = pfGetPipe(0);

	pfInitPipe(pipe, OpenPipeline);
	pfFrameRate(30.0f);

	pfInitClock(0.0);


	chan = pfNewChan(pipe);
	scene = pfNewScene();
	root = pfNewGroup();
	geode = pfNewGeode();
	gset = pfNewGSet(arena);


	eSky = pfNewESky();
	pfESkyMode(eSky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
	pfESkyAttr(eSky, PFES_GRND_HT, 0.0f);
	pfESkyColor(eSky, PFES_GRND_FAR, 0.3f, 0.1f, 0.0f, 1.0f);
	pfESkyColor(eSky, PFES_GRND_NEAR, 0.5f, 0.3f, 0.1f, 1.0f);
	pfChanESky(chan, eSky);

        pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, scoords, svindex);
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, scolors,scindex);
        pfGSetPrimType(gset, PFGS_QUADS);
        pfGSetNumPrims(gset, 6);

	pfAddGSet(geode, gset);
	for (loop=0; loop < 10; loop++)
	{
		pfMakeTransMat(mat, 0.0f, (float) loop * 10.0f + 10.0f, 0.0f);
		scs[loop] = pfNewSCS(mat);
		pfAddChild(scs[loop], geode);
		pfAddChild(root, scs[loop]);

		/* set up SCS's (and therefore, their children) for 
                   intersections */
		pfNodeTravMask(scs[loop], PFTRAV_ISECT, loop, 
			PFTRAV_SELF, PF_SET);
	}
	pfAddChild(scene, root);
	pfChanScene(chan, scene);

	pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);

	pfChanNearFar(chan, 0.1f, 10000.0f);
	pfChanFOV(chan, 45.0f, -1.0f);

	shared->yval=0.0f;
	shared->zval=0.5f;
	pfSetVec3(view.xyz, 0.0f, shared->yval, shared->zval);
	pfSetVec3(view.hpr, 0.0f, -5.0f, 0.0f);

	while (!shared->exitFlag)
	{
		pfSeg	segment;
		pfSeg	*sp = &segment;
		pfuIsect	result;
		int	isect;

		shared->yval += 0.1f;
		if (shared->yval > 110.0f) shared->yval = 0.0f;

		PFSET_VEC3 (segment.pos, 0.0f, shared->yval, shared->zval);
		PFSET_VEC3 (segment.dir, 0.0f, 0.0f, -1.0f);
		segment.length = 50000.0f;

		isect = pfuSegsIsectNode(root, &sp, 1,
			PFTRAV_IS_PRIM|PFTRAV_IS_NORM|PFTRAV_IS_CULL_BACK,
			0x1 | 0x4, NULL, &result, NULL);

		if (isect)
		{
			shared->zval = result.point[PF_Z] + 0.5f;
		}

		pfSetVec3(view.xyz, 0.0f, shared->yval, shared->zval);
		pfChanView(chan, view.xyz, view.hpr);
		pfFrame();
	}

	pfExit();
}

static void OpenPipeline (pfPipe *p)
{
	winopen ("Terrain");
	pfInitGfx(p);

	qdevice (ESCKEY);
	qdevice (F1KEY);
	qdevice (GKEY);
}

static void DrawChannel (pfChannel *chan, void *data)
{
	short dev,val;

	pfClearChan(chan);
	pfDraw();
	if (shared->showStats) pfDrawChanStats(chan);

	while (qtest()) 
	{
		dev = qread(&val);
		if (val) switch (dev)
		{
			case ESCKEY:
				shared->exitFlag=1;
				break;
			case F1KEY:
			case GKEY:
				shared->showStats = !shared->showStats;
				break;
		}
	}

}

