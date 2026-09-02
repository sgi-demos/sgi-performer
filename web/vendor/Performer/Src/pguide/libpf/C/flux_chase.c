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
 *
 *	flux_chase [-s]
 *
 *	Demonstrate synchronizing the results of a DBase process.
 *	The main program controls a single box - moving in a circle.
 *	A DBase process makes many other boxes follow the path of the 
 *	first box.
 *
 *	When the DBase boxes are not synchronized, they follow the 
 *	path individually - each at its own pace. 
 *	When using '-s' option, they follow the path - all together. 
 *	
 *	Command line options:
 *        no flags : No synchronization.
 *	  -s       : Run with syncGroup synchronization on the result fluxes. 
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

#ifdef WIN32
#define random rand
#endif

#define	EXTRA_BUFFERS	1

#define RADIUS	10.0

#define	NOF_OBJECTS		5

static pfNode 	*build_box(float color_scale);

static void 	startDbaseThread(void);
static void 	DBaseFunc(void *data);
static void     sendDbaseData(void);

static pfFCS    *setupDBaseConnection (int i, pfNode *box);

static void 	advance_motion(void);
static void 	update_box2(pfDCS *dcs);

float		box_azimuth;
pfFlux		*in_flux[NOF_OBJECTS];
pfFlux		*out_flux[NOF_OBJECTS];
float		radius[NOF_OBJECTS];

/*
 *	Data structure for DBase thread.
 */
typedef struct
{
    float	box_azimuth;
    pfFlux	*in_flux[NOF_OBJECTS];
    pfFlux	*out_flux[NOF_OBJECTS];
    float	radius[NOF_OBJECTS];
    uint	sync_group;
} userDataType;


userDataType	*userData;

static int	use_sync_group;
static uint	sync_group;

/*===========================================================================*/
void main (int argc, char *argv[])
/*===========================================================================*/
{
    float       	t = 0.0f;
    pfScene     	*scene;
    pfPipe      	*p;
    pfChannel   	*chan;
    pfNode		*box, *box2;
    pfFCS		*fcs;
    pfDCS		*dcs2;
    pfPipeWindow	*pwin;
    int			i;

    use_sync_group = 0;
    if (argc > 1)
    {
	if (strcmp (argv[1], "-s") == 0)
	{
	    use_sync_group = 1;
	    printf ("Using Sync Group\n");
	}
    }

    /* Initialize Performer */
    pfInit();	

    /* 
     * Use different mode for spawning DBase thread.
     */
    pfMultiprocess(PFMP_APPCULLDRAW | PFMP_FORK_DBASE);			

#ifdef WIN32
    pfNotify (PFNFY_WARN, PFNFY_PRINT, 
	    "This program uses a DBASE process: A slow asynchronous process");
    pfNotify (PFNFY_WARN, PFNFY_PRINT, 
	    "Performer for Windows doesn't support multiprocessing. The slow");
    pfNotify (PFNFY_WARN, PFNFY_PRINT, 
	    "DBASE function runs as a part of the main APP process and slows"); 
    pfNotify (PFNFY_WARN, PFNFY_PRINT, 
	    "it down considerably.");
#endif

    /* 
     * Configure multiprocessing mode and start parallel
     * processes. DBase thread doesn't start yet. 
     */
    pfConfig();			

    if (use_sync_group)
	sync_group = pfGetFluxNamedSyncGroup("flux_chase");

    scene = pfNewScene();

    /*
     * 	Build boxes with FCS control.
     */
    for (i = 0 ; i < NOF_OBJECTS ; i ++)
    {
	box = build_box(1.0);
	fcs = setupDBaseConnection (i, box);

	pfAddChild(scene, fcs);
    }

    /*
     *	Setup leading box - controled by the APP.
     */
    box2 = build_box(0.5);
    dcs2 = pfNewDCS();
    pfAddChild (dcs2, box2);
    pfAddChild(scene, dcs2);

    /* Configure and open GL window */
    p = pfGetPipe(0);

    pwin = pfNewPWin (p);
    pfPWinType (pwin, PFWIN_TYPE_X);
    pfPWinName (pwin, "OpenGL Performer");
    pfPWinOriginSize (pwin, 0, 0, 500, 500);
    pfOpenPWin (pwin);

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 1000.0f);
    pfChanFOV(chan, 45.0f, 0.0f);

    if (use_sync_group)
	pfFluxEnableSyncGroup(sync_group);

    startDbaseThread();

    pfInitClock (0.0f);

    /* Simulate for twenty seconds. */
    while (t < 120.0f)
    {
	float      s, c;
	pfCoord	   view;

	advance_motion();

        update_box2(dcs2);

	/*
	 *    Send the new angle data to DBase task.
         */
	sendDbaseData();

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
static pfNode *build_box(float color_scale)
/*===========================================================================*/
{
    int			i, face;
    float		*clist;
    float		*vlist;
    float		*vp, *cp;
    pfGeoSet		*gset;
    pfGeoState		*gstate;
    pfGeode		*geode;

    gstate = pfNewGState (pfGetSharedArena());
    pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
    pfGStateMode (gstate, PFSTATE_ENLIGHTING, PF_OFF);

    vlist = pfMalloc (24 * 3 * sizeof (float), pfGetSharedArena());
    clist = pfMalloc (6 * 4 * sizeof (float), pfGetSharedArena());

    vp = vlist;
    cp = clist;

    for (face = 0; face < 6; face ++)
    {
	*cp ++ = box_colors[face][0] * color_scale;
	*cp ++ = box_colors[face][1] * color_scale;
	*cp ++ = box_colors[face][2] * color_scale;
	*cp ++ = box_colors[face][3] * color_scale;

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

    return ((pfNode *) geode);
}

/*===========================================================================*/
static pfFCS     *setupDBaseConnection (int i, pfNode *box)
/*===========================================================================*/
{
    pfFlux	*flux_matrix, *results;
    pfEngine	*eng;
    float	*s_normal, *s_azimuth;
    pfMatrix	M;
    pfFCS	*fcs;
    pfSphere	sphere;

    /*
     *	Allocate come constant data for the static Engine sources.
     */

    s_normal = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());
    s_azimuth = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());

    s_azimuth[0] = 1.0;
    s_azimuth[1] = 0.0;
    s_azimuth[2] = 0.0;

    s_normal[0] = 0.0;
    s_normal[1] = 0.0;
    s_normal[2] = 1.0;

    /* ===================================================================== */

    /*
     *	Make the final flux - this is what the FCS will use.
     */

    flux_matrix = pfNewFlux(sizeof(pfMatrix), 
			    PFFLUX_DEFAULT_NUM_BUFFERS + EXTRA_BUFFERS,
			    pfGetSharedArena());

    if (use_sync_group)
	pfFluxSyncGroup(flux_matrix, sync_group);

    pfMakeTransMat (M, 0.0, 0.0, 0.0);
    pfFluxInitData (flux_matrix, M);

    fcs = pfNewFCS(flux_matrix);

    pfAddChild(fcs, box); 

    /* ===================================================================== */

    /*
     *	Set large enough sphere to avoid culling.
     */

    sphere . center[0] = 0.0;
    sphere . center[1] = 0.0;
    sphere . center[2] = 0.0;
    sphere . radius = 100000.0;

    pfNodeBSphere(fcs, &sphere, PFBOUND_STATIC);

    /* ===================================================================== */

    /*
     *	DBase task will write into the 'results' flux. A change to 'results'
     *	will trigger engine evaluation, and a write into 'flux_matrix'
     */

    results = pfNewFlux(sizeof(pfVec3), 
			PFFLUX_DEFAULT_NUM_BUFFERS + EXTRA_BUFFERS,
			pfGetSharedArena());

    pfFluxMode(results, PFFLUX_PUSH, PF_ON);

    eng = pfNewEngine(PFENG_ALIGN, pfGetSharedArena());
    pfEngineSrc(eng, PFENG_ALIGN_POSITION, results, NULL, 0, 0, 3);
    pfEngineSrc(eng, PFENG_ALIGN_NORMAL, s_normal, NULL, 0, 0, 3);
    pfEngineSrc(eng, PFENG_ALIGN_AZIMUTH, s_azimuth, NULL, 0, 0, 3);
    pfEngineDst(eng, flux_matrix, NULL, 0, 0);

    pfEngineEvaluationRange(eng, sphere.center, 0.0, 10000.0);
    pfEngineMode(eng, PFENG_RANGE_CHECK, PF_ON);

    /* ===================================================================== */

    /*
     *	in_flux will be used later by the DBase thread. It will write
     *	the new position information here.
     */

    in_flux[i] = results;
    out_flux[i] = flux_matrix;
    radius[i] = RADIUS + ((float) (random() % 1000)) * RADIUS / 1000.0;

    return (fcs);

}

/*===========================================================================*/
static void startDbaseThread(void)
/*===========================================================================*/
{
    int		i;

    userData = (userDataType *) pfAllocDBaseData(sizeof (userDataType)); 

    userData -> box_azimuth = box_azimuth;
    for (i = 0 ; i < NOF_OBJECTS ; i ++)
    {
	userData -> in_flux[i] = in_flux[i];
	userData -> out_flux[i] = out_flux[i];
	userData -> radius[i] = radius[i];
    }

    pfDBaseFunc(DBaseFunc);

    pfPassDBaseData();
}

/*===========================================================================*/
static void sendDbaseData(void)
/*===========================================================================*/
{
    int		i;

    userData -> box_azimuth = box_azimuth;
    for (i = 0 ; i < NOF_OBJECTS ; i ++)
    {
	userData -> in_flux[i] = in_flux[i];
	userData -> out_flux[i] = out_flux[i];
	userData -> radius[i] = radius[i];
    }

    userData -> sync_group = sync_group;

    pfPassDBaseData();
}


/*===========================================================================*/
static void DBaseFunc(void *data)
/*===========================================================================*/
{
    int				i;
    userDataType		*info;
    float			x, y, z;
    float			*out;

    pfDBase();

    if (data == NULL)
	printf ("DBaseFunc: Null data\n");

    info = (userDataType *) data;

    for (i = 0 ; i < NOF_OBJECTS ; i ++)
    {
	x = info -> radius[i] * cosf (info -> box_azimuth);
	y = info -> radius[i] * sinf (info -> box_azimuth);
	z = 0.0;

	out = (float *) pfGetFluxWritableData (info -> in_flux[i]);

	out[0] = x;
	out[1] = y;
	out[2] = z;

	pfFluxWriteComplete(info -> in_flux[i]);
	sginap(40);
    }

    /*
     *	Arbitrary choice of nap. This emulates Dbase thread frames that 
     *	take a lot longer than one draw frame.
     */
    sginap (30);

    /*
     *	Only when we are done updating all in_flux'es we make all the 
     *	results available (in '-s' mode)
     */

    if (use_sync_group)
	pfFluxSyncGroupReady(info -> sync_group);
}

/*===========================================================================*/
static void advance_motion(void)
/*===========================================================================*/
{
    box_azimuth += 0.01;
    if (box_azimuth > 2.0 * M_PI)
	box_azimuth -= 2.0 * M_PI;
}

/*===========================================================================*/
static void update_box2(pfDCS *dcs)
/*===========================================================================*/
{
    float	x, y, z;

    /*
     *	This is a direct update of the DCS. 
     */

    x = 1.2 * RADIUS * cosf (box_azimuth);
    y = 1.2 * RADIUS * sinf (box_azimuth);
    z = 10.0 * sinf (box_azimuth * 5.0);

    pfDCSTrans (dcs, x, y, z); 
}


