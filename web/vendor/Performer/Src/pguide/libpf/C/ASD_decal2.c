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
 *	ASD_decal [-A][-a][asd_filename]
 *
 * 	Demonstrate how to generate decals pfASD.
 *
 *	This program uses fluxed geosets as ASD query results. pfASD fills in 
 *	the resulting decal geometry directly into the geoset. There is no 
 *	pfEngine connecting pfASD to the final pfGeoSet therefore - APP 
 *	intervention is not possible.
 *
 *	Without flags: generate an animated triangular decal (move in a circle).
 *	    -a  move the decal triangle in a circle.
 *	    -A  Disable decal motion
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#define EYE_RADIUS	400.0
#define SIZE		100.0
#define	NOF_FRAGMENTS	200

static void 		init_globals(void);
static pfMaterial 	*defaultMaterial (void);
static pfGeoState 	*make_asd_gstate (void);
static void 		advance_motion(void);

static pfNode		*setup_ASD_decal(pfASD *asd, pfGeoSet *base_gset);
static int 		fluxed_gset_create_func (pfFluxMemory *fmem);
static pfFlux 		*make_fluxed_geoset(void);
static pfGeoSet		*make_base_gset (void);

float			decal_azimuth;

char			asd_filename[300];
char			texture_filename[300];

pfASD			*asd;
float			cx, cy;
float			dx, dy;
pfBox			asd_box;
pfBox			decal_box;
int			query_id;
pfGeoSet		*decal_gset;
pfGeoSet		*base_gset;
float			*global_v_attr;
int			animate_decal;


/*===========================================================================*/
void main (int argc, char *argv[])
/*===========================================================================*/
{
    float       	t = 0.0f;
    pfScene     	*scene;
    pfPipe      	*p;
    pfChannel   	*chan;
    pfPipeWindow	*pwin;
    int			i;
    pfLight     	*lt;
    pfGeoState 		*gs, **gsa;
    pfNode		*node;


    init_globals();

    for (i = 1 ; i < argc ; i ++)
    {
	if (argv[i][0] == '-')
	{
	    if (argv[i][1] == 'A')
		animate_decal = 0;
	    else
	    if (argv[i][1] == 'a')
		animate_decal = 1;
   	}
	else
	    strcpy (asd_filename, argv[1]);
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
    pfConfig();			

    scene = pfNewScene();

    /* 
     *	======================================================
     *	Load ASD. 
     *	======================================================
     */
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
    gs = make_asd_gstate();
    *gsa = gs;
    pfASDGStates(asd, gsa, 1);
    pfFree(gsa);

    /*
     *	============================================================
     * 	Setup alignment
     *	============================================================
     */
    base_gset = make_base_gset();
    node = setup_ASD_decal(asd, base_gset);

    pfAddChild(scene, node);

    /*
     *	============================================================
     * 	other common setup.
     *	============================================================
     */
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

	if (animate_decal)
	    advance_motion();

	/* Go to sleep until next frame time. */
	pfSync();		

	/* Initiate cull/draw for this frame. */
	pfFrame();		

	/* Compute new view position. */
	t = pfGetTime();

	cz = (decal_box . min[2] + decal_box . max[2]) * 0.5;

	x = cx + EYE_RADIUS;
	y = cy - EYE_RADIUS;
	z = cz + 2.0 * EYE_RADIUS;

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
    float	*v;
    ushort	*dummy_ushort;
    float	down[3];

    pfGetGSetAttrLists(base_gset, PFGS_COORD3, (void **) &v, &dummy_ushort);

    decal_azimuth += 0.01;
    if (decal_azimuth > 2.0 * M_PI)
	decal_azimuth -= 2.0 * M_PI;

    v[0] = cx;
    v[1] = cy;
    v[2] = 0.0;

    v[3] = cx + SIZE * cos(decal_azimuth);
    v[4] = cy + SIZE * sin(decal_azimuth);
    v[5] = 0.0;

    v[6] = cx + SIZE * cos(decal_azimuth + M_PI * 0.25);
    v[7] = cy + SIZE * sin(decal_azimuth + M_PI * 0.25);
    v[8] = 0.0;

    pfGSetAttr(base_gset, PFGS_COORD3, PFGS_PER_VERTEX, v, NULL);

    down[0] = 0.0;
    down[1] = 0.0;
    down[2] = -1.0;

    pfASDReplaceQueryGeoSet (asd, query_id, base_gset, down);
}


/*===========================================================================*/
static void init_globals(void)
/*===========================================================================*/
{
    strcpy (asd_filename, "tinyASD.pfb");
    strcpy (texture_filename, "marble.rgb");
    decal_azimuth = 0.0;
    animate_decal = 1;
}


/*===========================================================================*/
static pfNode	*setup_ASD_decal(pfASD *asd, pfGeoSet *base_gset)
/*===========================================================================*/
{
    /*
     *  =====================
     *  Final tree structure:
     *  =====================
     *
     *      <pfLOD>                               <asd> <-- base_gset
     *         |                                    |
     *      <Geode>                                 |
     *         |                                    |
     *   [Fluxed pfGeoSet] <-------------------------
     *        
     */

    pfLOD	*lod;
    pfGeoSet	*gset;
    pfFlux	*fluxed_gset;
    pfGeode 	*geode;
    pfSphere    big_sphere;
    pfBox       *box_p;
    float	*s_down;

    s_down = (float *) pfMalloc(3 * sizeof(float), pfGetSharedArena());

    s_down[0] = 0.0;
    s_down[1] = 0.0;
    s_down[2] = -1.0;

    /*
     *  =====================================================================
     *	Make a fluxed geoset. Add to a geode.
     *  =====================================================================
     */

    fluxed_gset = make_fluxed_geoset();
    gset = (pfGeoSet*) pfGetFluxCurData (fluxed_gset);

    pfFluxSyncGroup((pfFlux *) fluxed_gset, 1);
    pfFluxMode((pfFlux *) fluxed_gset, PFFLUX_PUSH, PF_ON);

    geode = pfNewGeode();
    pfAddGSet (geode, gset);


    query_id = pfASDAddQueryGeoSet (
			    asd,
                            base_gset,
                            s_down,
                            PR_QUERY_TRI_COORD,
                            (pfFlux *) fluxed_gset);

    pfASDGetQueryArrayPositionSpan(asd, query_id, &decal_box);

    /*
     *  ==================================================================
     *  Calculate the maximum bbox of the morphing object.
     *  ==================================================================
     */

    box_p = & decal_box;
    pfSphereAroundBoxes(&big_sphere, (const pfBox **)&box_p, 1);

    lod = pfNewLOD();
    pfAddChild(lod, geode);

    pfLODRange(lod, 0, 0.0);
    pfLODRange(lod, 1, 100000.0 + big_sphere.radius);
    pfLODCenter(lod, big_sphere.center);

    pfNodeBSphere(lod,   &big_sphere, PFBOUND_STATIC);
    pfNodeBSphere(geode, &big_sphere, PFBOUND_STATIC);

    return((pfNode *)lod);
}

/*===========================================================================*/
static pfGeoState *make_asd_gstate (void)
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

/*===========================================================================*/
static int fluxed_gset_create_func (pfFluxMemory *fmem)
/*===========================================================================*/
{
    float 	*c_attr;
    pfGeoState  *gstate;
    pfGeoSet 	*gset;

    if (fmem == NULL)
        return pfFluxedGSetInit(fmem);

    pfFluxedGSetInit(fmem);

    gset = (pfGeoSet*)pfGetData(fmem);

    c_attr = pfMalloc(4 * sizeof(float), pfGetSharedArena());
    c_attr[0] = 0.1;
    c_attr[1] = 0.1;
    c_attr[2] = 0.1;
    c_attr[3] = 0.6;
    pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, c_attr, NULL);

    /*
     *  ===========================================================
     *  Build state.
     *  ===========================================================
     */
    gstate = pfNewGState (pfGetSharedArena());
    pfMakeBasicGState (gstate);
    pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
    pfGStateMode (gstate, PFSTATE_ENLIGHTING, PF_OFF);
    pfGStateMode (gstate, PFSTATE_TRANSPARENCY, PFTR_ON | PFTR_NO_OCCLUDE);
    pfGStateMode(gstate, PFSTATE_DECAL,
        PFDECAL_LAYER_DISPLACE | PFDECAL_LAYER_7 | PFDECAL_LAYER_OFFSET);
    pfGStateMode (gstate, PFSTATE_ENTEXTURE, 0);


    /*
     *  ===========================================================
     *  Gset attributes.
     *  ===========================================================
     */
    pfGSetPrimType(gset, PFGS_TRIS);
    pfGSetNumPrims(gset, 0);
    pfGSetGState(gset, gstate);

    return 0;
}

/*===========================================================================*/
static pfFlux *make_fluxed_geoset(void)
/*===========================================================================*/
{
    pfFlux	*fluxed_gset;

    fluxed_gset = pfNewFluxInitFunc (
				fluxed_gset_create_func, 
				PFFLUX_DEFAULT_NUM_BUFFERS, 
				pfGetSharedArena());

    return fluxed_gset;
}

/*===========================================================================*/
static pfGeoSet	*make_base_gset (void)
/*===========================================================================*/
{
    pfGeoSet 	*gset;
    float	*v;

    gset = pfNewGSet (pfGetSharedArena());

    pfGSetPrimType(gset, PFGS_TRIS);
    pfGSetNumPrims(gset, 1);

    v = pfMalloc (9 * sizeof (float), pfGetSharedArena());
    v[0] = cx;
    v[1] = cy;
    v[2] = 0.0;

    v[3] = cx + SIZE;
    v[4] = cy;
    v[5] = 0.0;

    v[6] = cx;
    v[7] = cy + SIZE;
    v[8] = 0.0;

    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, v, NULL);

    return (gset);
}
