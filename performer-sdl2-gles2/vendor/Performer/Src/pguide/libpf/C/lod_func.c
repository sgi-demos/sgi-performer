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
 *
 * lod_func.c: demonstrate the use of a pfLOD evaluation function.
 *
 * $Revision: 1.3 $ $Date: 2002/09/15 03:51:02 $ 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Performer/pf.h>

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#define		RADIUS 15.0

static pfNode 	*build_box(float _scale, float _rotation);
static void 	advance_lod(void);
static float 	lod_user_func(pfLOD *lod, pfChannel *chan, pfMatrix *m);

float		lod_alpha = 0.0;
float		lod_result = 0.0;

/*===========================================================================*/
void main (int argc, char *argv[])
/*===========================================================================*/
{
    float       	t = 0.0f;
    pfScene     	*scene;
    pfLOD		*lod;
    pfPipe      	*p;
    pfChannel   	*chan;
    pfNode		*box;
    pfPipeWindow	*pwin;
    int			i;

    /* Initialize Performer */
    pfInit();	

    /* 
     * Use different mode for spawning DBase thread.
     */
    pfMultiprocess(PFMP_APP_CULL_DRAW);			

    /* 
     * Configure multiprocessing mode and start parallel
     * processes. DBase thread doesn't start yet. 
     */
    pfConfig();			

    scene = pfNewScene();

    lod = pfNewLOD();
    pfLODUserEvalFunc (lod, lod_user_func);

    pfAddChild(scene, lod);

    box = build_box(6.0, 0.0);
    pfAddChild(lod, box);

    box = build_box(10.0, 90.0);
    pfAddChild(lod, box);

    /* Configure and open GL window */
    p = pfGetPipe(0);

    pwin = pfNewPWin (p);
    pfPWinType (pwin, PFWIN_TYPE_X);
    pfPWinOriginSize (pwin, 0, 0, 500, 500);
    pfOpenPWin (pwin);

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 1000.0f);
    pfChanFOV(chan, 45.0f, 0.0f);

    pfInitClock (0.0f);

    while (t < 120.0f)
    {
	float      s, c;
	pfCoord	   view;

	/* Go to sleep until next frame time. */
	pfSync();		

	/* Initiate cull/draw for this frame. */
	pfFrame();		

	/* DBase new view position. */
	t = pfGetTime();

        pfSinCos(45.0f, &s, &c);
        pfSetVec3(view.hpr, 45.0f, -23.0f, 0);
        pfSetVec3(view.xyz, 2.0f * RADIUS * 3.0 * s,
                -2.0f * RADIUS * 3.0 *c,
                 RADIUS * 3.0);

	pfChanView(chan, view.xyz, view.hpr);
    }

    /* Terminate parallel processes and exit. */
    pfExit();
}


static float	box[6][4*3] =
{
	{
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0,
	1.0, 1.0, 0.0,
	0.0, 1.0, 0.0
	},
	{
	0.0, 0.0, 1.0,
	1.0, 0.0, 1.0,
	1.0, 1.0, 1.0,
	0.0, 1.0, 1.0
	},
	{
	0.0, 0.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 1.0,
	0.0, 0.0, 1.0
	},
	{
	1.0, 0.0, 0.0,
	1.0, 1.0, 0.0,
	1.0, 1.0, 1.0,
	1.0, 0.0, 1.0
	},
	{
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0,
	1.0, 0.0, 1.0,
	0.0, 0.0, 1.0
	},
	{
	0.0, 1.0, 0.0,
	1.0, 1.0, 0.0,
	1.0, 1.0, 1.0,
	0.0, 1.0, 1.0
	},
};

static float	box_colors[6][4] =
	{
	0.0, 0.0, 1.0, 1.0,
	0.0, 1.0, 0.0, 1.0,
	0.0, 1.0, 1.0, 1.0,
	1.0, 0.0, 0.0, 1.0,
	1.0, 0.0, 1.0, 1.0,
	1.0, 1.0, 1.0, 1.0
	};


/*===========================================================================*/
static pfNode *build_box(float _scale, float _rotation)
/*===========================================================================*/
{
    int			i, face;
    float		*clist;
    float		*vlist;
    float		*vp, *cp;
    pfGeoSet		*gset;
    pfGeoState		*gstate;
    pfGeode		*geode;
    pfSCS		*scs;
    pfMatrix        	mat;

    gstate = pfNewGState (pfGetSharedArena());
    pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
    pfGStateMode (gstate, PFSTATE_ENLIGHTING, PF_OFF);

    vlist = pfMalloc (24 * 3 * sizeof (float), pfGetSharedArena());
    clist = pfMalloc (6 * 4 * sizeof (float), pfGetSharedArena());

    vp = vlist;
    cp = clist;

    for (face = 0; face < 6; face ++)
    {
	*cp ++ = box_colors[face][0] * 1.0;
	*cp ++ = box_colors[face][1] * 1.0;
	*cp ++ = box_colors[face][2] * 1.0;
	*cp ++ = box_colors[face][3] * 1.0;

	for (i = 0; i < 4 * 3; i ++)
	    *vp ++ = box[face][i];
    }

    gset = pfNewGSet (pfGetSharedArena());
    pfGSetPrimType (gset, PFGS_QUADS);

    pfGSetAttr (gset, PFGS_COORD3, PFGS_PER_VERTEX, (void *) vlist, NULL);
    pfGSetAttr (gset, PFGS_TEXCOORD2, PFGS_OFF, NULL, NULL);

    pfGSetAttr (gset, PFGS_COLOR4, PFGS_PER_PRIM, (void *) clist, NULL);
    pfGSetAttr (gset, PFGS_NORMAL3, PFGS_OFF, NULL, NULL);

    pfGSetNumPrims (gset, 6);

    pfGSetGState (gset, gstate);

    geode = pfNewGeode();
    pfAddGSet (geode, gset);

    pfMakeScaleMat (mat, _scale, _scale, _scale);
    pfPostRotMat (mat, mat, _rotation, 0.0, 0.0, 1.0);

    scs = pfNewSCS(mat);
    pfAddChild (scs, geode);

    return ((pfNode *) scs);
}



/*===========================================================================*/
static void advance_lod(void)
/*===========================================================================*/
{
    lod_alpha += 0.01;
    if (lod_alpha > 2.0 * M_PI)
	lod_alpha -= 2.0 * M_PI;

    lod_result = (0.5 * sin(lod_alpha) + 0.5);
}


/*===========================================================================*/
static float lod_user_func(pfLOD *lod, pfChannel *chan, pfMatrix *m)
/*===========================================================================*/
{
    advance_lod();
    return (lod_result);
}

