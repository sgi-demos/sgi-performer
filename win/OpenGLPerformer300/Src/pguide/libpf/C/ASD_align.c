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
 *	ASD_align [asd_filename model_filename]
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#define EYE_RADIUS	100.0
#define RADIUS		40.0

#define	NOF_OBJECTS		100

static void 		init_globals(void);
static pfNode		*setup_ASD_align(pfNode *model, pfASD *asd, float *v);
static pfMaterial 	*defaultMaterial (void);
static pfGeoState 	*make_gstate (void);
static void 		advance_motion(void);

float		model_azimuth;

char		asd_filename[300];
char		model_filename[300];
char		texture_filename[300];

pfASD		*asd;
pfNode	 	*model;
float		cx, cy;
float		dx, dy;
pfBox		asd_box;
pfBox		model_box;
int		query_id;
pfFlux		*azimuth_flux;


/*===========================================================================*/
void main (int argc, char *argv[])
/*===========================================================================*/
{
    float       	t = 0.0f;
    pfScene     	*scene;
    pfPipe      	*p;
    pfChannel   	*chan;
    pfPipeWindow	*pwin;
    float		base_position[3];
    pfLight     	*lt;
    pfGeoState 		*gs, **gsa;


    init_globals();

    if (argc == 3)
    {
	strcpy (asd_filename, argv[1]);
	strcpy (model_filename, argv[2]);
    }

    /* Initialize Performer */
    pfInit();	

    pfuInit();

    /* 
     * Use different mode for spawning DBase thread.
     */
    pfMultiprocess(PFMP_APPCULLDRAW | PFMP_FORK_COMPUTE);			

    /* 
     * Configure multiprocessing mode and start parallel
     * processes. 
     */
    pfdInitConverter(asd_filename);
    pfdInitConverter(model_filename);
    pfConfig();			

    scene = pfNewScene();

    /* Load ASD. */
    pfFilePath(".:/usr/share/Performer/data:/usr/share/Performer/data/asddata");
    asd = (pfASD *) pfdLoadFile (asd_filename);
    if (! asd)
    {
	fprintf (stderr, "ASD file %s not loaded.\n", asd_filename);
	pfExit();
	exit(8);
    }

    pfASDSyncGroup(asd,1);
    pfAddChild(scene, asd);

    /* calculate center of model circular motion. */
    pfGetASDBBox (asd, &asd_box);
    cx = (asd_box . min[0] + asd_box . max[0]) * 0.5;
    cy = (asd_box . min[1] + asd_box . max[1]) * 0.5;

    dx = (asd_box . max[0] - asd_box . min[0]) * 0.5;
    dy = (asd_box . max[1] - asd_box . min[1]) * 0.5;

    /* setup gstate with some texture */
    gsa = pfMalloc(sizeof(pfGeoState*), pfGetSharedArena());
    gs = make_gstate();
    *gsa = gs;
    pfASDGStates(asd, gsa, 1);
    pfFree(gsa);


    /* Load model.  */
    model = pfdLoadFile (model_filename);
    if (! model)
    {
	fprintf (stderr, "Model file %s not loaded.\n", model_filename);
	pfExit();
	exit(8);
    }

    /*
     *	============================================================
     * 	Setup alignment
     *	============================================================
     */
    base_position[0] = cx;
    base_position[1] = cy;
    base_position[2] = 0;

    pfAddChild(scene, setup_ASD_align(model, asd, base_position));



    /* Configure and open GL window */
    p = pfGetPipe(0);

    pwin = pfNewPWin (p);
    pfPWinType (pwin, PFWIN_TYPE_X);
    pfPWinOriginSize (pwin, 0, 0, 500, 500);
    pfOpenPWin (pwin);

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 100000.0f);
    pfChanFOV(chan, 45.0f, 0.0f);

    pfInitClock (0.0f);

    /* Make some light */

    pfEnable(PFEN_LIGHTING);
    lt = pfNewLight(pfGetSharedArena());
    pfLightPos(lt, 0.0f, 0.0f, 1.0f, 0.0f);
    pfLightOn(lt);

    pfEnable(PFEN_TEXTURE);

    /* Simulate for twenty seconds. */
    while (t < 200.0f)
    {
	pfCoord	   	view;
	float		x, y, z, pitch, cz;

	advance_motion();

	/* Go to sleep until next frame time. */
	pfSync();		

	/* Initiate cull/draw for this frame. */
	pfFrame();		

	/* Compute new view position. */
	t = pfGetTime();

	cz = (model_box . min[2] + model_box . max[2]) * 0.5;

	x = cx + EYE_RADIUS;
	y = cy - EYE_RADIUS;
	z = cz + EYE_RADIUS;

	pitch = - atan2 (fabs(z - cz), sqrt ((x-cx)*(x-cx) + (y-cy)*(y-cy))) 
			* 180.0 / M_PI;

        pfSetVec3(view.hpr, 45.0f, pitch, 0);
        pfSetVec3(view.xyz, x, y, z);

	pfChanView(chan, view.xyz, view.hpr);
    }

    /* Terminate parallel processes and exit. */
    pfExit();
}


/*===========================================================================*/
static void advance_motion(void)
/*===========================================================================*/
{
    float	v[3], d[3];
    float	*azimuth;

    model_azimuth += 0.01;
    if (model_azimuth > 2.0 * M_PI)
	model_azimuth -= 2.0 * M_PI;

    v[0] = cx + RADIUS * cos (model_azimuth);
    v[1] = cy + RADIUS * sin (model_azimuth);
    v[2] = 0.0;

    d[0] = 0.0;
    d[1] = 0.0;
    d[2] = -1.0;

    pfASDQueryArrayElement (asd, query_id, 0, v, d);

    azimuth = (float *) pfGetFluxWritableData (azimuth_flux);

    azimuth[0] = cos (model_azimuth + M_PI * 0.5);
    azimuth[1] = sin (model_azimuth + M_PI * 0.5);

    pfFluxWriteComplete (azimuth_flux);

}


/*===========================================================================*/
static void init_globals(void)
/*===========================================================================*/
{
    strcpy (asd_filename, "tinyASD.pfb");
    strcpy (model_filename, "esprit.flt");
    strcpy (texture_filename, "marble.rgb");
    model_azimuth = 0.0;
}


/*===========================================================================*/
static pfNode	*setup_ASD_align(pfNode *model, pfASD *asd, float *v)
/*===========================================================================*/
{
    /*
     *  =====================
     *  Final tree structure:
     *  =====================
     *
     *      <pfLOD>                                 <asd>
     *         |                                      |
     *       <FCS> <----[matrix_flux]<--[engine]<--[query_flux]    in pfCompute 
     *         |                           |
     *      <pfNode>                       <-------[azimuth_flux]  in APP
     *      /      \
     *        Model 
     */

    /*
     *	Note: Multiple writers into matrix_flux. Both the pfCompute process 
     *  and the APP process cause engine source changes. We make azimuth_flux
     *	a NO-PUSH flux and query_flux a PUSH flux in order to avoid conflicts.
     */

    pfFlux	*matrix_flux, *query_flux;
    pfFCS	*fcs;
    pfEngine	*engine;
    pfLOD	*lod;
    pfSphere	sphere;
    pfSphere    big_sphere;
    pfBox       *box_p;
    float	*s_position, *s_down;
    pfMatrix	M;
    float	azimuth[3];

    /*
     *  ==================================================================
     *  Copy position to shared memory.
     *  ==================================================================
     */
    s_position = (float *) pfMalloc(3 * sizeof(float), pfGetSharedArena());

    s_position[0] = v[0];
    s_position[1] = v[1];
    s_position[2] = v[2];

    s_down = (float *) pfMalloc(3 * sizeof(float), pfGetSharedArena());

    s_down[0] = 0.0;
    s_down[1] = 0.0;
    s_down[2] = -1.0;

    /*
     *  ==================================================================
     *  Make a FCS. Put the object under it.
     *  ==================================================================
     */

    matrix_flux = pfNewFlux(sizeof(pfMatrix),
                            4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());
    pfFluxSyncGroup(matrix_flux, 1);

    pfMakeTransMat (M, v[0], v[1], v[2]);
    pfFluxInitData (matrix_flux, M);

    fcs = pfNewFCS(matrix_flux);

    pfAddChild(fcs, model);

    pfGetNodeBSphere(fcs, &sphere);

    /*
     *  =====================================================================
     *  Make flux for APP control over azimuth.
     *
     *  The azimuth Flux is NOT in push mode. When the ASD Compute task is 
     *	very busy, and azimuth_flux is in push mode, we miss some of the 
     * 	pfASD updates. in Push mode, the azimuth_flux competes with pfASD on 
     *	writing to matrix_flux and it sometimes overwrites the ASD results. 
     *  =====================================================================
     */

    azimuth_flux = pfNewFlux(3 * sizeof(float),
                            4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());
    azimuth[0] = 0.0;
    azimuth[1] = 1.0;
    azimuth[2] = 0.0;

    pfFluxInitData(azimuth_flux, azimuth);
    pfFluxMode(azimuth_flux, PFFLUX_PUSH, PF_OFF);

    /*
     *  =====================================================================
     *  Align object to terrain 
     *  =====================================================================
     */
    query_flux = pfNewFlux(2 * 3 * sizeof(float),
                            4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());
    pfFluxMode(query_flux, PFFLUX_PUSH, PF_ON);

    query_id = pfASDAddQueryArray(asd, s_position, s_down, 1,
                                  PR_QUERY_POSITION | PR_QUERY_NORMAL,
                                  query_flux);

    engine = pfNewEngine(PFENG_ALIGN, pfGetSharedArena());
    pfEngineSrc(engine, PFENG_ALIGN_POSITION, query_flux, NULL, 0, 0, 0);
    pfEngineSrc(engine, PFENG_ALIGN_NORMAL, query_flux, NULL, 0, 3, 0);
    pfEngineSrc(engine, PFENG_ALIGN_AZIMUTH, azimuth_flux, NULL, 0, 0, 0);

    pfEngineDst(engine, matrix_flux, NULL, 0, 0);
    /*
     *  Evaluation Range
     */
    pfEngineEvaluationRange(engine, sphere.center, 0.0, 100000.0);
    pfEngineMode(engine, PFENG_RANGE_CHECK, PF_ON);

    pfASDGetQueryArrayPositionSpan(asd, query_id, &model_box);

    /*
     *  ==================================================================
     *  Calculate the maximum bbox of the morphing object.
     *  ==================================================================
     */
    box_p = & model_box;
    pfSphereAroundBoxes(&big_sphere, (const pfBox **)&box_p, 1);

    lod = pfNewLOD();
    pfAddChild(lod, fcs);

    pfLODRange(lod, 0, 0.0);
    pfLODRange(lod, 1, 100000.0 + big_sphere.radius);
    pfLODCenter(lod, big_sphere.center);

    pfNodeBSphere(lod, &big_sphere, PFBOUND_STATIC);
    pfNodeBSphere(fcs, &big_sphere, PFBOUND_STATIC);

    return((pfNode *)lod);
}

/*===========================================================================*/
static pfGeoState *make_gstate (void)
/*===========================================================================*/
{
    pfGeoState 	*gs;
    pfTexture  	*tex;
    pfTexGen	*tgen;
    float	s;

    s = 1.0 / (asd_box . max[0] - asd_box . min[0]);

    gs = pfNewGState (pfGetSharedArena());
    tex = pfNewTex( pfGetSharedArena());
    tgen = pfNewTGen( pfGetSharedArena());

    pfMakeBasicGState(gs);
    pfGStateMode(gs, PFSTATE_CULLFACE, PFCF_BACK);
    pfGStateMode(gs, PFSTATE_ENLIGHTING, PF_ON);
    pfGStateMode(gs, PFSTATE_ENTEXTURE, PF_ON);
    pfGStateAttr(gs, PFSTATE_FRONTMTL, defaultMaterial());

    pfLoadTexFile( tex, texture_filename );
    pfGStateAttr( gs, PFSTATE_TEXTURE, tex );

    pfGStateMode(gs, PFSTATE_ENTEXGEN, PF_ON);

    pfGStateAttr( gs, PFSTATE_TEXGEN, tgen );
    pfGStateAttr( gs, PFSTATE_TEXENV, pfNewTEnv( pfGetSharedArena() ) );

    pfTGenPlane(tgen, PF_S,   s, 0.0, 0.0, 0.0);
    pfTGenPlane(tgen, PF_T, 0.0,   s, 0.0, 0.0);
    pfTGenPlane(tgen, PF_R, 0.0, 0.0,   s, 0.0);
    pfTGenPlane(tgen, PF_Q, 0.0, 0.0, 0.0, 1.0);

    pfTGenMode(tgen, PF_S, PFTG_OBJECT_LINEAR);
    pfTGenMode(tgen, PF_T, PFTG_OBJECT_LINEAR);
    pfTGenMode(tgen, PF_R, PFTG_OBJECT_LINEAR);
    pfTGenMode(tgen, PF_Q, PFTG_OBJECT_LINEAR);

    return (gs);
}

/*===========================================================================*/
static pfMaterial *defaultMaterial (void)
/*===========================================================================*/
{
    pfMaterial	*material;

    material = pfNewMtl(pfGetSharedArena());
    pfMtlColor(material, PFMTL_AMBIENT,  0.8f, 0.8f, 0.8f);
    pfMtlColor(material, PFMTL_DIFFUSE,  0.8f, 0.8f, 0.8f);
    pfMtlColor(material, PFMTL_SPECULAR, 0.0f, 0.0f, 0.0f);
    pfMtlShininess(material, 0.0f);

    pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_OFF);
    return material;
}

