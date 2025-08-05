/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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

/*
 *	Terrain tracking light points.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <malloc.h>
#include <assert.h>

#include "perfly.h"

#include "pfuLightPointsTypes.h"
#include "pfuLightPointsPrototypes.h"

static int  	pfuBillboardDrawCallback (pfTraverser *trav, void *data);
static int  	pfuBillboardPostDrawCallback (pfTraverser *trav, void *data);
static int 	pfuGStateCallback (pfGeoState *gstate, void *userData);

static int 	fluxed_gset_create_func (pfFluxMemory *fmem);
static void 	fluxed_gset_init_func (void *_data, void *_funcData);

typedef struct
{
	pfGeoState	*gstate;
	float		*c_attr;
        pfBox		*box;
} pfuFluxedGSetData;


#undef DEBUG_SHADOW
#undef DEBUG_FLUX

/*============================================================================*/
static void
buildGState(pfGeoState *gs)
/*============================================================================*/
{
    int width, height, depth;
    pfLPointState  *lps;

#if 0
    pfGStateMode(gs, PFSTATE_DECAL,
	PFDECAL_LAYER_DISPLACE | PFDECAL_LAYER_7 | PFDECAL_LAYER_OFFSET);
#endif

    if (! ViewState->lightPoints.useRealLightPoints)
    {
        pfGStateMode(gs, PFSTATE_ANTIALIAS, PFAA_ON);
	pfGStateMode(gs, PFSTATE_CULLFACE, PFCF_OFF);
	pfGStateMode(gs, PFSTATE_ENLIGHTING, PF_OFF);
	pfGStateMode(gs, PFSTATE_ENTEXTURE, PF_OFF);
    }
    else
    {
	lps = pfNewLPState(pfGetSharedArena());
	pfGStateMode(gs, PFSTATE_ENLPOINTSTATE, 1);
	pfGStateAttr(gs, PFSTATE_LPOINTSTATE, lps);

	/*
	 * Light point projected diameter is computed on CPU. Real world
	 * size is 0.07 database units and projected size is clamped be
	 * between 0.25 and 4 pixels.
	 */
	pfLPStateMode(lps, PFLPS_SIZE_MODE, PFLPS_SIZE_MODE_ON);
	pfLPStateVal(lps, PFLPS_SIZE_MIN_PIXEL, 0.75f);
	pfLPStateVal(lps, PFLPS_SIZE_ACTUAL, 0.1f);
	pfLPStateVal(lps, PFLPS_SIZE_MAX_PIXEL, 10.0f);
	pfLPStateMode(lps, PFLPS_RANGE_MODE, PFLPS_RANGE_MODE_TRUE);


	/*
	 * Light points become transparent when their projected diameter is
	 * < 2 pixels. The transparency falloff rate is linear with
	 * projected size with a scale factor of 0.6. The transparency
	 * multiplier, NOT the light point transparency, is clamped to 0.1.
	 */
	pfLPStateVal(lps, PFLPS_TRANSP_PIXEL_SIZE, 2.0f);
	pfLPStateVal(lps, PFLPS_TRANSP_EXPONENT, 1.0f);
	pfLPStateVal(lps, PFLPS_TRANSP_SCALE, 0.6f);
	pfLPStateVal(lps, PFLPS_TRANSP_CLAMP, 0.1f);

	/*
	 * Light points will be fogged as if they were 4 times
	 * nearer to the eye than actual to achieve punch-through.
	 */
	pfLPStateVal(lps, PFLPS_FOG_SCALE, 0.25f);
	pfLPStateMode(lps, PFLPS_DIR_MODE, PFLPS_DIR_MODE_OFF);
#if 0
	/*
	 * Turn on calligraphic light points
	 */
	pfLPStateMode(lps, PFLPS_DRAW_MODE, PFLPS_DRAW_MODE_CALLIGRAPHIC);
	pfLPStateMode(lps, PFLPS_QUALITY_MODE, PFLPS_QUALITY_MODE_HIGH);
#endif

	/*
	 * Disable pfFog effects since light points are fogged by
	 * the pfLPointState.
	 */
	pfGStateMode ( gs, PFSTATE_ENFOG, PF_OFF ) ;
	/*
	 * Disable lighting effects since light points are completely
	 * emissive.
	 */
        pfGStateMode ( gs, PFSTATE_ENHIGHLIGHTING, PF_OFF ) ;
	pfGStateMode ( gs, PFSTATE_CULLFACE, PFCF_OFF ) ;
	pfGStateVal  ( gs, PFSTATE_ALPHAREF, 0.0 ) ;
	pfGStateMode ( gs, PFSTATE_ALPHAFUNC, PFAF_GREATER ) ;

	pfGStateMode(gs, PFSTATE_ENLIGHTING, PF_OFF);
	pfGStateMode(gs, PFSTATE_ENTEXTURE, PF_OFF);
	pfGStateAttr(gs, PFSTATE_TEXENV, NULL);

    }
}

/*============================================================================*/
pfNode *
pfNewLightPointAlignNode(
			int			nofLightPoints,
			float			*vertexList,
			float			*colorList,
			float			*normalList,
			int			is_down_z,
			pfASD			*asd_hook)
/*============================================================================*/
{
    pfFlux				*results;
    pfEngine				*eng;
    pfGeoState				*gs;
    pfGeoSet				*gset, **gset_array;
    pfGeode				*geode;
    static pfVec3			normal;
    static ushort			ncnt[1];
    float				*vlist, *nlist;
    float				*clist;
    int					i, j;
    int					is_pos;
    float				*down;
    pfFlux				*attr;
    pfLOD				*lod;
    pfSphere				sphere;
    pfBox				box, *box_p;
    pfBox				Vbox;
    int					src_index_1, src_index_2;
    int					query_id;
    unsigned long			is;


    /*
     *  =====================
     *	Final tree structure:
     *  =====================
     *
     *      <pfLOD>
     *	       |
     *	    <Geode>
     *	       |
     *      <Geoset>
     *
     */

    gs = pfNewGState(pfGetSharedArena());
    buildGState(gs);

    /*
     *	=====================================================================
     *	Copy points to chared arena.
     *	=====================================================================
     */

    if (vertexList)
    {
	attr = pfNewFlux(nofLightPoints * 3 * sizeof(float),
			 4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());
	pfFluxSyncGroup(attr, pfGetASDSyncGroup(asd_hook));

	vlist = pfMalloc(nofLightPoints * 3 * sizeof(float),
			 pfGetSharedArena());

	memcpy(vlist, vertexList, nofLightPoints * 3 * sizeof(float));

	pfFluxInitData(attr, vertexList);

	if (is_down_z)
	{
	    /*
	     *	 Fake displacement from ground.
	     */
	    for (i = 0; i < nofLightPoints; i++)
		vlist[i * 3 + 2] += 2.0;
	}
    }
    else
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "Problem: Null vertex list.\n");
	return(NULL);
    }

    if (colorList)
    {
	clist = pfMalloc(nofLightPoints * 4 * sizeof(float),
			 pfGetSharedArena());
	memcpy(clist, colorList, nofLightPoints * 4 * sizeof(float));
    }
    else
	clist = NULL;

    if (normalList)
    {
	nlist = pfMalloc(nofLightPoints * 3 * sizeof(float),
			 pfGetSharedArena());
	memcpy(nlist, normalList, nofLightPoints * 3 * sizeof(float));
    }
    else
	nlist = NULL;

    down = pfMalloc(nofLightPoints * 3 * sizeof(float), pfGetSharedArena());

    for (i = 0; i < nofLightPoints; i++)
    {
	if (is_down_z)
	{
	    down[3 * i + 0] = 0.0;
	    down[3 * i + 1] = 0.0;
	    down[3 * i + 2] = -1.0;
	}
	else
	{
	    down[3 * i + 0] = - vertexList[3*i];
	    down[3 * i + 1] = - vertexList[3*i + 1];
	    down[3 * i + 2] = - vertexList[3*i + 2];
	}
    }

    /*
     *	======================================================================
     *	Run a check to verify that this set of light points 
     *  falls within the given asd... 
     *	======================================================================
     */

    is = pfASDContainsQueryArray (asd_hook, vertexList, down, nofLightPoints);

    if (! (is & PFIS_ALL_IN))
    {
	if (is & PFIS_MAYBE)
	    printf ("Light points are MAYBE on pfASD\n");
	else
	if (is == PFIS_FALSE)
	    printf ("Light Point set is NOT on this pfASD node.\n");
    }

    /*
     *	=====================================================================
     *	Make a standard GeoSet with light points.
     *	=====================================================================
     */

    gset = pfNewGSet(pfGetSharedArena());
    pfGSetPrimType(gset, PFGS_POINTS);
    pfGSetPntSize(gset, 4.0);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, (void *)attr, NULL);

    if (clist)
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, (void *)clist, NULL);
    else
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "Warning: No color for strip of light points.\n");
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_OFF, NULL, NULL);
    }

    if (nlist)
	pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, (void *)nlist, NULL);
    else
	pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OFF, NULL, NULL);

    pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_OFF, NULL, NULL);

    pfGSetNumPrims(gset, nofLightPoints);
    pfGSetGState(gset, gs);
    pfGSetBBox(gset, NULL, PFBOUND_STATIC);

    geode = pfNewGeode();
    pfAddGSet(geode, gset);

    /*
     *	=====================================================================
     *	Setup alignment operation for this geoset.
     *	=====================================================================
     */

    results = pfNewFlux(nofLightPoints * sizeof(pfVec3),
			4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());
    pfFluxMode(results, PFFLUX_PUSH, PF_ON);

    query_id = pfASDAddQueryArray(asd_hook, vertexList, down, nofLightPoints,
				  PR_QUERY_POSITION, results);

    pfASDGetQueryArrayPositionSpan(asd_hook, query_id, &box);

    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
	     "asdfly: Light set bbox = (%f,%f) x (%f,%f) x (%f,%f)\n",
	     box.min[0], box.max[0],
	     box.min[1], box.max[1],
	     box.min[2], box.max[2]);


    if (is_down_z)
    {
	/*
	 *	 Update only the Z part.
	 */
#if 1
	eng = pfNewEngine(PFENG_SUM, pfGetSharedArena());
	pfEngineIterations(eng, nofLightPoints, 1);
	pfEngineSrc(eng, PFENG_SUM_SRC(0), vlist, NULL, 0, PF_Z, 3);
	pfEngineSrc(eng, PFENG_SUM_SRC(1), results, NULL, 0, PF_Z, 3);
	pfEngineDst(eng, attr, NULL, PF_Z, 3);

#else

	for (i = 0; i < nofLightPoints; i++)
	{
	    vlist[i * 3 + 0] = 0.0;
	    vlist[i * 3 + 1] = 0.0;
	}

	eng = pfNewEngine(PFENG_SUM, pfGetSharedArena());
	pfEngineIterations(eng, nofLightPoints, 3);
	pfEngineSrc(eng, PFENG_SUM_SRC(0), vlist, NULL, 0, 0, 3);
	pfEngineSrc(eng, PFENG_SUM_SRC(1), results, NULL, 0, 0, 3);
	pfEngineDst(eng, attr, NULL, 0, 3);
#endif
    }
    else
    {
	/*
	 * Update All X,Y,Z.
	 */
	eng = pfNewEngine(PFENG_SUM, pfGetSharedArena());	/* XXX _SET */
	pfEngineIterations(eng, nofLightPoints, 3);
	pfEngineSrc(eng, PFENG_SUM_SRC(0), results, NULL, 0, 0, 3);
	pfEngineDst(eng, attr, NULL, 0, 3);
    }

    /*
     *  Set static bounding boxes to reflect maximum position span of query
     *  points.
     */
    box_p = & box;
    pfSphereAroundBoxes(&sphere, (const pfBox **)&box_p, 1);

    lod = pfNewLOD();
    pfAddChild(lod, geode);

    pfLODRange(lod, 0, 0.0);
    pfLODRange(lod, 1, 40000.0);
    pfLODCenter(lod, sphere.center);

    pfNodeBSphere(lod, &sphere, PFBOUND_STATIC);
    pfNodeBSphere(geode, &sphere, PFBOUND_STATIC);
    pfGSetBBox(gset, &box, PFBOUND_STATIC);

    /*
     *  Evaluation Range
     */
    pfEngineEvaluationRange(eng, sphere.center, 0.0, 40000.0);
    pfEngineMode(eng, PFENG_RANGE_CHECK, PF_ON);

    return((pfNode *)lod);
}

/*============================================================================*/
pfNode *
pfNewObjectAlignNode(
			char			*filename,
			float			x,
			float			y,
			float			z,
			float			*down,
			int			is_normal,
			int			is_azimuth,
			float			*azimuth,
			pfASD			*asd_hook)
/*============================================================================*/
{
    pfFlux				*results;
    pfEngine				*eng;
    pfGeoState				*gs;
    pfGeoSet				*gset;
    pfGeode				*geode;
    static pfVec3			normal;
    static ushort			ncnt[1];
    float				*vlist, *nlist;
    float				*clist;
    int					i;
    pfFlux				*flux_matrix;
    pfFCS				*fcs;
    float				*s_position, *s_down;
    float				*s_azimuth, *s_normal;
    pfLOD				*lod;
    pfSphere				sphere;
    pfSphere				min_sphere, max_sphere, big_sphere;
    pfSphere				*sphere_list[10];
    pfBox				Vbox, *box_p;
    int					query_id;
    int					src_index_1, src_index_2, src_index_3;
    pfMatrix				M;

    /*
     *  =====================
     *	Final tree structure:
     *  =====================
     *
     *	    <pfLOD>
     *	       |
     *	     <FCS>
     *	       |
     *      <pfNode>
     *      /      \
     *       Object
     */

    /*
     *	==================================================================
     *	Copy the position information into shared arena memory. We may not
     *	need them all (dependeing on is_azimuth/is_normal).
     *	==================================================================
     */

    s_position = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());
    s_down = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());
    s_normal = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());
    s_azimuth = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());

    s_position[0] = x;
    s_position[1] = y;
    s_position[2] = z;


    if (down)
    {
	s_down[0] = down[0];
	s_down[1] = down[1];
	s_down[2] = down[2];
	s_normal[0] = - down[0];
	s_normal[1] = - down[1];
	s_normal[2] = - down[2];
    }
    else
    {
	s_down[0] = 0.0;
	s_down[1] = 0.0;
	s_down[2] = -1.0;
	s_normal[0] = 0.0;
	s_normal[1] = 0.0;
	s_normal[2] = 1.0;
    }

    if (azimuth)
    {
	s_azimuth[0] = azimuth[0];
	s_azimuth[1] = azimuth[1];
	s_azimuth[2] = azimuth[2];
    }
    else
    {
	s_azimuth[0] = 1.0;
	s_azimuth[1] = 0.0;
	s_azimuth[2] = 0.0;
    }

    /*
     *	==================================================================
     *	Make a FCS. Put the object under it.
     *	==================================================================
     */

    flux_matrix = pfNewFlux(sizeof(pfMatrix),
			    4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());
    pfFluxSyncGroup(flux_matrix, pfGetASDSyncGroup(asd_hook));

    pfMakeTransMat (M, x, y, 300.0);
    pfFluxInitData (flux_matrix, M);

    fcs = pfNewFCS(flux_matrix);

    pfAddChild(fcs, pfdLoadFile(filename));

    pfGetNodeBSphere(fcs, &sphere);

    /*
     *	=====================================================================
     *	Distinguish four different possible modes:
     *
     *		1. Align object to terrain normal.
     *		   Use azimuth vector as the nose direction vector (projected
     *		   onto wing plane).
     *
     *		2. No normal but rotate object around it's Z for azimuth.
     *
     *		3. No normal, no azimuth and down direction is -Z.
     *		   We only need to translate the object.
     *
     *		4. no normal, no azimuth but down direction != (-Z).
     *		   Use normal = (- down).
     *	=====================================================================
     */
    if (is_normal)
    {
	results = pfNewFlux(2 * sizeof(pfVec3),
			    4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());
	pfFluxMode(results, PFFLUX_PUSH, PF_ON);

	query_id = pfASDAddQueryArray(asd_hook, s_position, s_down, 1,
				      PR_QUERY_POSITION | PR_QUERY_NORMAL,
				      results);

	eng = pfNewEngine(PFENG_ALIGN, pfGetSharedArena());
	pfEngineSrc(eng, PFENG_ALIGN_POSITION, results, NULL, 0, 0, 6);
	pfEngineSrc(eng, PFENG_ALIGN_NORMAL, results, NULL, 0, 3, 6);
	pfEngineSrc(eng, PFENG_ALIGN_AZIMUTH, s_azimuth, NULL, 0, 0, 3);
	pfEngineDst(eng, flux_matrix, NULL, 0, 0);
    }
    else
    {
	results = pfNewFlux(sizeof(pfVec3),
			    4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());
	pfFluxMode(results, PFFLUX_PUSH, PF_ON);

	query_id = pfASDAddQueryArray(asd_hook, s_position, s_down, 1,
				      PR_QUERY_POSITION, results);

	if (is_azimuth)
	{
	    eng = pfNewEngine(PFENG_ALIGN, pfGetSharedArena());
	    pfEngineSrc(eng, PFENG_ALIGN_POSITION, results, NULL, 0, 0, 3);
	    pfEngineSrc(eng, PFENG_ALIGN_NORMAL, s_normal, NULL, 0, 0, 3);
	    pfEngineSrc(eng, PFENG_ALIGN_AZIMUTH, s_azimuth, NULL, 0, 0, 3);
	    pfEngineDst(eng, flux_matrix, NULL, 0, 0);
	}
	else
	{
	    if ((down[0] == 0.0) && (down[1] == 0.0))
	    {
		eng = pfNewEngine(PFENG_ALIGN, pfGetSharedArena());
		pfEngineSrc(eng, PFENG_ALIGN_POSITION, results, NULL, 0, 0, 3);
		pfEngineDst(eng, flux_matrix, NULL, 0, 0);
	    }
	    else
	    {
		s_azimuth[0] = 1.0;
		s_azimuth[1] = 0.0;
		s_azimuth[2] = 0.0;

		eng = pfNewEngine(PFENG_ALIGN, pfGetSharedArena());
		pfEngineSrc(eng, PFENG_ALIGN_POSITION, results, NULL, 0, 0, 3);
		pfEngineSrc(eng, PFENG_ALIGN_NORMAL, s_normal, NULL, 0, 0, 3);
		pfEngineSrc(eng, PFENG_ALIGN_AZIMUTH, s_azimuth, NULL, 0, 0, 3);
		pfEngineDst(eng, flux_matrix, NULL, 0, 0);
	    }
	}
    }

    pfASDGetQueryArrayPositionSpan(asd_hook, query_id, &Vbox);

    /*
     *	==================================================================
     *	Calculate the maximum bbox of the morphing object.
     *	==================================================================
     */
    for (i = 0; i < 3; i++)
    {
	Vbox.min[i] = Vbox.min[i] - sphere.radius;
	Vbox.max[i] = Vbox.max[i] + sphere.radius;
    }

    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
	     "Object <%s> bbox = (%f,%f) x (%f,%f) x (%f,%f)\n",
	     filename,
	     Vbox.min[0], Vbox.max[0],
	     Vbox.min[1], Vbox.max[1],
	     Vbox.min[2], Vbox.max[2]);

    box_p = & Vbox;
    pfSphereAroundBoxes(&big_sphere, (const pfBox **)&box_p, 1);

    lod = pfNewLOD();
    pfAddChild(lod, fcs);

    pfLODRange(lod, 0, 0.0);
    pfLODRange(lod, 1, 10000.0 + big_sphere.radius);
    pfLODCenter(lod, big_sphere.center);

    pfNodeBSphere(lod, &big_sphere, PFBOUND_STATIC);
    pfNodeBSphere(fcs, &big_sphere, PFBOUND_STATIC);

    /*
     *  Evaluation Range
     */
    pfEngineEvaluationRange(eng, sphere.center, 0.0, 10000.0);
    pfEngineMode(eng, PFENG_RANGE_CHECK, PF_ON);

    return((pfNode *)lod);
}

/*============================================================================*/
pfNode *
pfNewBillboardAlignNode(
			char			*filename,
			float			x,
			float			y,
			float			z,
			float			*down,
			float			width,
			float			height,
			pfASD			*asd_hook)
/*============================================================================*/
{
    pfGeoState				*gstate;
    pfGeoSet				*gset;
    int					i;
    pfBillboard				*bill;
    float				*s_position, *s_down, *n_down;
    pfLOD				*lod;
    pfSphere				sphere;
    pfSphere				min_sphere, max_sphere, big_sphere;
    pfSphere				*sphere_list[10];
    pfBox				Vbox, *box_p;
    int					query_id;
    int					src_index_1, src_index_2, src_index_3;
    float				TexCoord[4][2];
    float				VertexCoord[4][3];
    float				*vlist, *tlist, *clist;
    float				*len;
    pfTexture       			*tex;
    pfTexEnv        			*tev;
    pfFlux				*flux_position;
    float				span;


    TexCoord[0][0] = 0.0;
    TexCoord[0][1] = 0.0;

    TexCoord[1][0] = 1.0;
    TexCoord[1][1] = 0.0;

    TexCoord[2][0] = 1.0;
    TexCoord[2][1] = 1.0;

    TexCoord[3][0] = 0.0;
    TexCoord[3][1] = 1.0;

    VertexCoord[0][0] = - width * 0.5;
    VertexCoord[0][1] = 0.0;
    VertexCoord[0][2] = 0.0;

    VertexCoord[1][0] = width * 0.5;
    VertexCoord[1][1] = 0.0;
    VertexCoord[1][2] = 0.0;

    VertexCoord[2][0] = width * 0.5;
    VertexCoord[2][1] = 0.0;
    VertexCoord[2][2] = height;

    VertexCoord[3][0] = -width * 0.5;
    VertexCoord[3][1] = 0.0;
    VertexCoord[3][2] = height;

    /*
     *	=========================================
     *	Generate attributes for geoset.
     *	=========================================
     */


    vlist = pfMalloc (4 * 3 * sizeof (float), pfGetSharedArena());
    clist = pfMalloc (4 * sizeof (float), pfGetSharedArena());
    tlist = pfMalloc (4 * 2 * sizeof (float), pfGetSharedArena());
    len =   pfMalloc (sizeof (int), pfGetSharedArena());

    memcpy (vlist, VertexCoord, 3 * 4 * sizeof (float));
    memcpy (tlist, TexCoord, 2 * 4 * sizeof (float));

    clist[0] = 1.0;
    clist[1] = 1.0;
    clist[2] = 1.0;
    clist[3] = 1.0;

    len[0] = 4;

    /*
     *	=========================================
     *	Load texture for billboard
     *	=========================================
     */
    printf ("Load Billboard texture file %s\n", filename);

    tex = pfNewTex (pfGetSharedArena());
    if (! pfLoadTexFile (tex, filename))
	fprintf (stderr, "Texture %s: Not loaded (Error).\n", filename);;

    pfTexRepeat(tex, PFTEX_WRAP, PFTEX_CLAMP);

    tev = pfNewTEnv (pfGetSharedArena());

    gstate = pfNewGState (pfGetSharedArena());
    pfMakeBasicGState (gstate);

    pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
    pfGStateMode (gstate, PFSTATE_ENLIGHTING, PF_OFF);

    pfGStateMode (gstate, PFSTATE_ENTEXTURE, 1);
    pfGStateAttr (gstate, PFSTATE_TEXTURE, tex);
    pfGStateAttr (gstate, PFSTATE_TEXENV, tev);

    pfGStateMode (gstate, PFSTATE_TRANSPARENCY, PFTR_ON);
    pfGStateMode (gstate, PFSTATE_ALPHAFUNC, PFAF_GEQUAL);
    pfGStateVal  (gstate, PFSTATE_ALPHAREF, 0.4);

    /*
     *  =====================
     *	Final tree structure:
     *  =====================
     *
     *	    <pfLOD>
     *	       |
     *	   <Billboard>
     *	       |
     *     <pfGeoSet>
     *
     */

    /*
     *	==================================================================
     *	Copy the position information into shared arena memory. We may not
     *	need them all (dependeing on is_azimuth/is_normal).
     *	==================================================================
     */

    s_position = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());
    s_down = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());
    n_down = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());

    s_position[0] = x;
    s_position[1] = y;
    s_position[2] = z;


    if (down)
    {
	s_down[0] = down[0];
	s_down[1] = down[1];
	s_down[2] = down[2];
    }
    else
    {
	s_down[0] = 0.0;
	s_down[1] = 0.0;
	s_down[2] = -1.0;
    }

    n_down[0] = - s_down[0];
    n_down[1] = - s_down[1];
    n_down[2] = - s_down[2];

    /*
     *	==================================================================
     *	Make a Billboard node. Put the object under it.
     *	==================================================================
     */

    flux_position = pfNewFlux(sizeof(pfVec3),
			    4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());

    pfFluxSyncGroup(flux_position, pfGetASDSyncGroup(asd_hook));

    bill = pfNewBboard();
    pfBboardPosFlux (bill, flux_position);

    pfBboardAxis(bill, n_down);
    pfBboardMode(bill, PFBB_ROT, PFBB_AXIAL_ROT);


    /*
     *	==================================================================
     *	Make Geoset for billboard geometry
     *	==================================================================
     */
    gset = pfNewGSet(pfGetSharedArena());

    pfGSetGState (gset, gstate);

    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, vlist, NULL);
    pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, tlist, NULL);
    pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, clist, NULL);
    pfGSetAttr (gset, PFGS_NORMAL3, PFGS_OFF, NULL, NULL);

    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 1);

    pfAddGSet(bill, gset);

    /*
     *	==================================================================
     *	Setup pfASD query.
     *	==================================================================
     */
    pfFluxMode(flux_position, PFFLUX_PUSH, PF_ON);

    query_id = pfASDAddQueryArray(asd_hook, s_position, s_down, 1,
				      PR_QUERY_POSITION, flux_position);

    pfASDGetQueryArrayPositionSpan(asd_hook, query_id, &Vbox);

    /*
     *	==================================================================
     *	Calculate the maximum bbox of the morphing object.
     *	==================================================================
     */
    span = sqrt (height * height + width * width * 0.25);
    for (i = 0; i < 3; i++)
    {
	Vbox.min[i] = Vbox.min[i] - span;
	Vbox.max[i] = Vbox.max[i] + span;
    }

    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
	     "Object <%s> bbox = (%f,%f) x (%f,%f) x (%f,%f)\n",
	     filename,
	     Vbox.min[0], Vbox.max[0],
	     Vbox.min[1], Vbox.max[1],
	     Vbox.min[2], Vbox.max[2]);

    box_p = & Vbox;
    pfSphereAroundBoxes(&big_sphere, (const pfBox **)&box_p, 1);

    lod = pfNewLOD();
    pfAddChild(lod, bill);

    pfLODRange(lod, 0, 0.0);
    pfLODRange(lod, 1, 40000.0 + big_sphere.radius);
    pfLODCenter(lod, big_sphere.center);

    pfNodeBSphere(lod, &big_sphere, PFBOUND_STATIC);
    pfNodeBSphere(bill, &big_sphere, PFBOUND_STATIC);

#ifdef DEBUG_SHADOW
    pfNodeTravFuncs (bill, PFTRAV_DRAW, pfuBillboardDrawCallback, 
					pfuBillboardPostDrawCallback);
    pfNodeTravData (bill, PFTRAV_DRAW, (void *) ((int) y));

    pfGStateFuncs (gstate, pfuGStateCallback, NULL, (void *) ((int) y));
#endif

    return((pfNode *)lod);
}
/*============================================================================*/
pfNode *
pfuNewGSetShadowNode (pfGeoSet *_gset, 
			float		*_base, 
			float		*_down, 
			float		*_azimuth, 
			float		*_projection, 
			char		*_texture_filename, 
			unsigned long	_mask,
			unsigned long	_transform_mask,
			float		*_c,
			pfASD		*asd_hook)
/*============================================================================*/
{
    pfGeode		*geode;
    pfGeoSet		*gset;
    pfGeoState		*gstate;
    pfTexture      	*tex;
    pfTexEnv        	*tev;
    unsigned long	query_mask;
    pfuAppCallbackData	*userData;
    int			query_id;
    pfBox		box;
    pfEngine 		*engine;
    pfuShadowFluxData	shadow_data;
    pfSwitch 		*sw;
    pfVec3		center;
    int			i, do_wrap;
    int			nof_geosets;
    int			dummy;
    int			size;
    float		fdummy;
    int			_nof_triangles;
    ushort		*dummy_ushort;
    float		*_v, *_t;
    pfFlux      	*fluxed_gset;
    pfuFluxedGSetData	*fluxed_gset_data;
    float		*c_attr;

    _nof_triangles = pfGetGSetNumPrims (_gset);

    /*
     *	===========================================================
     *	Build output Geoset
     *	===========================================================
     */
    query_mask = 0;

    if (_mask & PR_QUERY_TRI_COORD)
	query_mask |= PR_QUERY_TRI_COORD;

    if (_mask & PR_QUERY_TRI_TEXTURE)
	query_mask |= PR_QUERY_TRI_TEXTURE;

    if (_mask & PR_QUERY_TRI_COLOR)
	query_mask |= PR_QUERY_TRI_COLOR;

    if (_mask & PR_QUERY_TRI_NORMAL)
	query_mask |= PR_QUERY_TRI_NORMAL;


    fluxed_gset = pfNewFluxInitFunc (
                                fluxed_gset_create_func,
                                4 + PFFLUX_DEFAULT_NUM_BUFFERS,
                                pfGetSharedArena());

    pfFluxSyncGroup((pfFlux *) fluxed_gset, pfGetASDSyncGroup(asd_hook));
    pfFluxMode((pfFlux *) fluxed_gset, PFFLUX_PUSH, PF_ON);


    /*
     *	===========================================================
     *	connect to asd.
     *	===========================================================
     */

    pfGetGSetAttrLists (_gset, PFGS_COORD3, (void **) &_v, &dummy_ushort);

    pfdAlignVerticesToASD (asd_hook, 
			    (pfVec3 *) _v, 
			    _nof_triangles * 3, 
			    _base, 
			    _down, 
			    (_transform_mask & PR_ALIGN_AXIS_BILLBOARD) ? 
			    _projection : _azimuth,
			    _transform_mask, 
			    (pfVec3 *) _v);

    pfdProjectVerticesOnASD (asd_hook, 
			    (pfVec3 *) _v, 
			    _nof_triangles * 3, 
			    _projection, 
			    _down, 
			    (pfVec3 *) _v);

    query_id = pfASDAddQueryGeoSet (asd_hook, 
			    _gset, 
			    _down, 
			    query_mask, 
			    fluxed_gset);

    pfASDGetQueryArrayPositionSpan(asd_hook, query_id, &box);

    /*
     *	===========================================================
     *	Build state.
     *	===========================================================
     */
    gstate = pfNewGState (pfGetSharedArena());
    pfMakeBasicGState (gstate);
    pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
    pfGStateMode (gstate, PFSTATE_ENLIGHTING, PF_OFF);
    pfGStateMode (gstate, PFSTATE_TRANSPARENCY, PFTR_ON | PFTR_NO_OCCLUDE);
    pfGStateMode(gstate, PFSTATE_DECAL,
	PFDECAL_LAYER_DISPLACE | PFDECAL_LAYER_7 | PFDECAL_LAYER_OFFSET);

    /*
     *	===========================================================
     *	If needed - add texture info.
     *	===========================================================
     */
    if (_mask & PR_QUERY_TRI_TEXTURE)
    {
	tex = pfNewTex (pfGetSharedArena());
	pfLoadTexFile (tex, _texture_filename);

	do_wrap = 0;
	pfGetGSetAttrLists (_gset, PFGS_TEXCOORD2, 
				(void **) &_t, &dummy_ushort);
	for (i = 0 ; i < 2 * 3 * _nof_triangles ; i ++)
	    if ((_t[i] < -0.001) || (_t[i] > 1.001))
		do_wrap = 1;

	if (do_wrap)
	    pfTexRepeat(tex, PFTEX_WRAP, PFTEX_WRAP);
	else
	    pfTexRepeat(tex, PFTEX_WRAP, PFTEX_CLAMP);

	tev = pfNewTEnv (pfGetSharedArena());

	pfGStateMode (gstate, PFSTATE_ENTEXTURE, 1);
	pfGStateAttr (gstate, PFSTATE_TEXTURE, tex);
	pfGStateAttr (gstate, PFSTATE_TEXENV, tev);
    }
    else
	pfGStateMode (gstate, PFSTATE_ENTEXTURE, 0);


    /*
     *	===========================================================
     *	If needed - add overall color
     *	===========================================================
     */

    c_attr = NULL;
    if (! (_mask & PR_QUERY_TRI_COLOR))
    {
        c_attr = pfMalloc(4 * sizeof(float), pfGetSharedArena());

        if (_c)
        {
            /*
             *  If we have any color specified, use it as overall color.
             */
            c_attr[0] = _c[0];
            c_attr[1] = _c[1];
            c_attr[2] = _c[2];
            c_attr[3] = _c[3];
        }
        else
        {
            c_attr[0] = 0.1;
            c_attr[1] = 0.1;
            c_attr[2] = 0.1;
            c_attr[3] = 0.6;
        }
    }

    /*
     *	====================================================================
     *	Init other attributes of the geoset (not taken care of by the init 
     *  function or the pfASD query.
     *	====================================================================
     */

    fluxed_gset_data = (pfuFluxedGSetData *) 
		    pfMalloc(sizeof(pfuFluxedGSetData), pfGetSharedArena());

    fluxed_gset_data -> c_attr = c_attr;
    fluxed_gset_data -> gstate = gstate;
    fluxed_gset_data -> box = & box;

    pfFluxCallDataFunc (fluxed_gset, fluxed_gset_init_func, 
					(void *) fluxed_gset_data);

    /* 
     *  ====================================================================
     *	Set user data on flux, so that future flux-generated buffers will 
     *  initialize correctly.
     *  ====================================================================
     */
    pfUserData ((pfObject *) fluxed_gset, (void *) fluxed_gset_data);

    geode = pfNewGeode();

    gset = (pfGeoSet*) pfGetFluxCurData (fluxed_gset);
    pfAddGSet(geode, gset);

    return ((pfNode *) geode);
}

/*============================================================================*/
static int  	pfuBillboardDrawCallback (pfTraverser *trav, void *data)
/*============================================================================*/
{
    pfGeoSet			*gset;
    pfBillboard			*bill;
    pfTexture			*tex, *active;
    pfGeoState			*gstate;
    int				mode;

    fprintf (stderr, "================================== DRAW  %d\n", 
								(int) data);

    bill = (pfBillboard *) pfGetTravNode (trav);

    fprintf (stderr, "DRAW: billboard = %x\n", (int) bill);

    gset = pfGetGSet (bill, 0);

    fprintf (stderr, "DRAW: gset = %x\n", (int) gset);

    gstate = pfGetGSetGState (gset);

    fprintf (stderr, "DRAW: gstate = %x\n", (int) gstate);

    tex = pfGetGStateAttr(gstate, PFSTATE_TEXTURE);

    if (tex == NULL)
	fprintf (stderr, "DRAW: No texture\n");
    else
	fprintf (stderr, "DRAW: texture name = <%s>\n", pfGetTexName (tex));

    mode = pfGetGStateMode (gstate, PFSTATE_ENTEXTURE);

    fprintf (stderr, "DRAW: texture mode == %d\n", mode);

    active = pfGetCurTex();

    fprintf (stderr, "DRAW: texture = %x, active = %x\n", 
			(int) tex, (int) active);
    fprintf (stderr, "DRAW: active texture name = <%s>\n", 
					pfGetTexName (active));

    return (PFTRAV_CONT);
}

/*============================================================================*/
static int  	pfuBillboardPostDrawCallback (pfTraverser *trav, void *data)
/*============================================================================*/
{
    pfGeoSet			*gset;
    pfBillboard			*bill;
    pfTexture			*tex, *active;
    pfGeoState			*gstate;
    int				mode;

    fprintf (stderr, "============================ POST DRAW == %d\n", 
								(int) data);

    bill = (pfBillboard *) pfGetTravNode (trav);

    fprintf (stderr, "DRAW: billboard = %x\n", (int) bill);

    gset = pfGetGSet (bill, 0);

    fprintf (stderr, "DRAW: gset = %x\n", (int) gset);

    gstate = pfGetGSetGState (gset);

    fprintf (stderr, "DRAW: gstate = %x\n", (int) gstate);

    tex = pfGetGStateAttr(gstate, PFSTATE_TEXTURE);

    if (tex == NULL)
	fprintf (stderr, "DRAW: No texture\n");
    else
	fprintf (stderr, "DRAW: texture name = <%s>\n", pfGetTexName (tex));

    mode = pfGetGStateMode (gstate, PFSTATE_ENTEXTURE);

    fprintf (stderr, "DRAW: texture mode == %d\n", mode);

    active = pfGetCurTex();

    fprintf (stderr, "DRAW: texture = %x, active = %x\n", 
			(int) tex, (int) active);
    fprintf (stderr, "DRAW: active texture name = <%s>\n", 
					pfGetTexName (active));

    return (PFTRAV_CONT);
}

/*============================================================================*/
static int pfuGStateCallback (pfGeoState *gstate, void *userData)
/*============================================================================*/
{
    pfTexture   *tex, *active;
    int		mode;
    pfGeoState	*scene_gstate;
    pfGeoState	*cur_gstate;

    fprintf (stderr, "================= GSTATE %d\n", (int) userData);

    active = pfGetCurTex();
    fprintf (stderr, "GSTATE: active texture name = <%s>\n", 
					pfGetTexName (active));
    fprintf (stderr, "============\n");

/*==================================================================*/

    tex = pfGetGStateAttr(gstate, PFSTATE_TEXTURE);

    if (tex == NULL)
	fprintf (stderr, "GSTATE: No texture\n");
    else
	fprintf (stderr, "GSTATE: texture name = <%s>\n", pfGetTexName (tex));

    mode = pfGetGStateMode (gstate, PFSTATE_ENTEXTURE);

    fprintf (stderr, "GSTATE: texture mode == %d\n", mode);

    pfPrint (gstate, 0, 0, 0);

/*==================================================================*/

    fprintf (stderr, "SCENE GSTATE:\n");

    fprintf (stderr, "============\n");

    scene_gstate = pfGetSceneGState(ViewState->scene);

    tex = pfGetGStateAttr(scene_gstate, PFSTATE_TEXTURE);

    if (tex == NULL)
	fprintf (stderr, "SCENE GSTATE: No texture\n");
    else
	fprintf (stderr, "SCENE GSTATE: texture name = <%s>\n", 
						pfGetTexName (tex));

    mode = pfGetGStateMode (scene_gstate, PFSTATE_ENTEXTURE);
    fprintf (stderr, "SCENE GSTATE: texture mode == %d\n", mode);

    pfPrint (scene_gstate, 0, 0, 0);

/*==================================================================*/

    fprintf (stderr, "CUR   GSTATE:\n");

    fprintf (stderr, "============\n");

    cur_gstate = pfGetCurGState();

    tex = pfGetGStateAttr(cur_gstate, PFSTATE_TEXTURE);

    if (tex == NULL)
	fprintf (stderr, "CUR   GSTATE: No texture\n");
    else
	fprintf (stderr, "CUR   GSTATE: texture name = <%s>\n", 
						pfGetTexName (tex));

    mode = pfGetGStateMode (cur_gstate, PFSTATE_ENTEXTURE);
    fprintf (stderr, "CUR   GSTATE: texture mode == %d\n", mode);

    pfPrint (cur_gstate, 0, 0, 0);


    fprintf (stderr, "=================\n");

    return (PFTRAV_CONT);
}


/*============================================================================*/
pfNode *
pfNewAnimationAlignNode(
			char			*filename,
			float			x,
			float			y,
			float			z,
			float			*down,
			float			trip_radius,
			int			*o_query_id,
			pfFlux			**o_azimuth_flux,
			pfASD			*asd_hook)
/*============================================================================*/
{
    pfFlux				*results;
    pfEngine				*eng;
    pfGeoState				*gs;
    pfGeoSet				*gset;
    pfGeode				*geode;
    static pfVec3			normal;
    static ushort			ncnt[1];
    float				*vlist, *nlist;
    float				*clist;
    int					i;
    pfFlux				*flux_matrix;
    pfFCS				*fcs;
    float				*s_position, *s_down;
    pfLOD				*lod;
    pfSphere				sphere;
    pfSphere				min_sphere, max_sphere, big_sphere;
    pfSphere				*sphere_list[10];
    pfBox				Vbox, *box_p;
    int					query_id;
    int					src_index_1, src_index_2, src_index_3;
    pfFlux				*azimuth_flux;
    float				s_azimuth[3];
    pfMatrix				M;

    /*
     *  =====================
     *	Final tree structure:
     *  =====================
     *
     *	    <pfLOD>
     *	       |
     *	     <FCS>
     *	       |
     *      <pfNode>
     *      /      \
     *       Object
     */

    /*
     *	==================================================================
     *	Copy the position information into shared arena memory. We may not
     *	need them all (dependeing on is_azimuth/is_normal).
     *	==================================================================
     */

    s_position = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());
    s_down = (float *)pfMalloc(3 * sizeof(float), pfGetSharedArena());

    s_position[0] = x;
    s_position[1] = y;
    s_position[2] = z;


    if (down)
    {
	s_down[0] = down[0];
	s_down[1] = down[1];
	s_down[2] = down[2];
    }
    else
    {
	s_down[0] = 0.0;
	s_down[1] = 0.0;
	s_down[2] = -1.0;
    }

    /*
     *	==================================================================
     *	Make a FCS. Put the object under it.
     *	==================================================================
     */

    flux_matrix = pfNewFlux(sizeof(pfMatrix),
			    4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());
    pfFluxSyncGroup(flux_matrix, pfGetASDSyncGroup(asd_hook));

    pfMakeTransMat (M, x, y, 300.0);
    pfFluxInitData (flux_matrix, M);

    fcs = pfNewFCS(flux_matrix);

    pfAddChild(fcs, pfdLoadFile(filename));

    pfGetNodeBSphere(fcs, &sphere);

    /*	
     *	=====================================================================
     *	Make flux for APP control over azimuth.
     *	=====================================================================
     */

    azimuth_flux = pfNewFlux(3 * sizeof(float),
			    4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());

    s_azimuth[0] = 0.0;
    s_azimuth[1] = 1.0;
    s_azimuth[2] = 0.0;

    pfFluxInitData(azimuth_flux, s_azimuth);

    *o_azimuth_flux = azimuth_flux;

    /*
     *	=====================================================================
     *	Align object to terrain normal.
     *	Use azimuth vector as the nose direction vector (projected
     *	onto wing plane).
     *	=====================================================================
     */
    results = pfNewFlux(2 * 3 * sizeof(float),
			    4 + PFFLUX_DEFAULT_NUM_BUFFERS, pfGetSharedArena());
    pfFluxMode(results, PFFLUX_PUSH, PF_ON);

    query_id = pfASDAddQueryArray(asd_hook, s_position, s_down, 1,
				  PR_QUERY_POSITION | PR_QUERY_NORMAL,
				  results);

    eng = pfNewEngine(PFENG_ALIGN, pfGetSharedArena());
    pfEngineSrc(eng, PFENG_ALIGN_POSITION, results, NULL, 0, 0, 0);
    pfEngineSrc(eng, PFENG_ALIGN_NORMAL, results, NULL, 0, 3, 0);
    pfEngineSrc(eng, PFENG_ALIGN_AZIMUTH, azimuth_flux, NULL, 0, 0, 0);
    pfEngineDst(eng, flux_matrix, NULL, 0, 0);
    /*
     *  Evaluation Range
     */
    pfEngineEvaluationRange(eng, sphere.center, 0.0, 10000.0);
    pfEngineMode(eng, PFENG_RANGE_CHECK, PF_ON);

    pfASDGetQueryArrayPositionSpan(asd_hook, query_id, &Vbox);

    /*
     *	==================================================================
     *	Calculate the maximum bbox of the morphing object.
     *	==================================================================
     */
    for (i = 0; i < 3; i++)
    {
	Vbox.min[i] = Vbox.min[i] - sphere.radius - trip_radius;
	Vbox.max[i] = Vbox.max[i] + sphere.radius + trip_radius;
    }

    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
	     "Animation <%s> bbox = (%f,%f) x (%f,%f) x (%f,%f)\n",
	     filename,
	     Vbox.min[0], Vbox.max[0],
	     Vbox.min[1], Vbox.max[1],
	     Vbox.min[2], Vbox.max[2]);

    box_p = & Vbox;
    pfSphereAroundBoxes(&big_sphere, (const pfBox **)&box_p, 1);

    lod = pfNewLOD();
    pfAddChild(lod, fcs);

    pfLODRange(lod, 0, 0.0);
    pfLODRange(lod, 1, 10000.0 + big_sphere.radius);
    pfLODCenter(lod, big_sphere.center);

    pfNodeBSphere(lod, &big_sphere, PFBOUND_STATIC);
    pfNodeBSphere(fcs, &big_sphere, PFBOUND_STATIC);


    *o_query_id = query_id;

    return((pfNode *)lod);
}

/*===========================================================================*/
static int fluxed_gset_create_func (pfFluxMemory *fmem)
/*===========================================================================*/
{
    float       *v_attr, *c_attr;
    pfGeoState  *gstate;
    pfGeoSet    *gset;
    pfFlux	*flux;
    void	*flux_data;

    if (fmem == NULL)
        return pfFluxedGSetInit(fmem);

    pfFluxedGSetInit(fmem);

    gset = (pfGeoSet*)pfGetData(fmem);

    /*
     *  ===========================================================
     *  Gset attributes.
     *  ===========================================================
     */
    pfGSetPrimType(gset, PFGS_TRIS);
    pfGSetNumPrims(gset, 0);

    /*
     *  =======================================================================
     *	If we have user-data on the flux, use it to initialize the flux geoset.
     * 	This should only happen for newly generated (on-the-fly) flux buffers.
     *  =======================================================================
     */
    flux = pfGetFluxMemoryFlux(fmem);

    flux_data = pfGetUserData ((pfObject *) flux);
    if (flux_data)
	fluxed_gset_init_func ((void *) gset, flux_data);

    return 0;
}

/*===========================================================================*/
static void fluxed_gset_init_func (void *_data, void *_funcData)
/*===========================================================================*/
{
    pfuFluxedGSetData 	*ShadowData;
    pfGeoSet		*gset;

    ShadowData = (pfuFluxedGSetData *) _funcData;
    gset = (pfGeoSet *) _data;

    pfGSetGState(gset, ShadowData -> gstate);

    if (ShadowData -> c_attr)
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, ShadowData -> c_attr, NULL);

    pfGSetBBox(gset, ShadowData -> box, PFBOUND_STATIC);
}
