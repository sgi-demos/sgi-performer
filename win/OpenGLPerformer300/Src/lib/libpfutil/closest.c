/*
 * Copyright 1996, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
*/

/*
 * file: closest.c
 * ----------------
 *
 * Contains the function pfuGetClosestPoint(),
 * which traverses a node to find the closest point to a given
 * point.  See the comment above the function for more details
 * and limitations.
 *
 * $Revision: 1.16 $
 * $Date: 2002/05/15 00:31:09 $
 */

#include <Performer/pfutil.h>
#include <stdlib.h>
#include <math.h>
#ifndef WIN32
#include <values.h>
#else
#define MAXFLOAT FLT_MAX
#endif

/*
 * Similar to pfuGetClosestPointOnTriangle below...
 * Note that planes[0] is assumed to be the far clip plane,
 * and it gets adjusted to meet the new closest point, if any.
 */
static int
nearestVisiblePointToEyePlaneOnTriangle(
		       pfMatrix *travMat,
		       pfPlane planes[5], /* planes[0] is the far clip */
		       pfVec3 v0, pfVec3 v1, pfVec3 v2,
		       float *W0, float *W1, float *W2)
{
    int i, nearesti, nverts;
    pfVec3 verts[3 + 5];        /* clipping by 5 planes can add 5 sides */
    pfVec3 affinecoords[3 + 5];

    /*
     * Work in world space since that's where the planes are
     * and transforming planes is expensive (especially for small geosets)
     */
    if (travMat != NULL)
    {
	pfXformPt3(verts[0], v0, *travMat);
	pfXformPt3(verts[1], v1, *travMat);
	pfXformPt3(verts[2], v2, *travMat);
    }
    else
    {
	PFCOPY_VEC3(verts[0], v0);
	PFCOPY_VEC3(verts[1], v1);
	PFCOPY_VEC3(verts[2], v2);
    }

    PFSET_VEC3(affinecoords[0], 1,0,0);
    PFSET_VEC3(affinecoords[1], 0,1,0);
    PFSET_VEC3(affinecoords[2], 0,0,1);

    /*
     * Clip the triangle
     */
    nverts = 3; /* start with the three triangle vertices */
    for (i = 0; i < 5; ++i)
    {
	nverts = pfClipConvexPolygonPlane(&planes[i], nverts, verts,
					  3, &affinecoords[0][0]);
	if (nverts == 0)
	    return 0;
	if (nverts < 0)
	{
	    pfNotify(PFNFY_WARN, PFNFY_INTERNAL,
		     "pfuGetNearestPoint: non-convex clipped triangle??");
	    return 0;
	}
    }

    /*
     * Return the nearest vertex of the clipped result,
     * i.e. the vertex whose dot product with the view direction is smallest.
     * The view direction is the normal of the far clip plane (planes[0]).
     */
    nearesti = -1;
    for (i = 0; i < nverts; ++i)
    {
	float thisDot = PFDOT_VEC3(verts[i], planes[0].normal);
	if (thisDot < planes[0].offset)
	{
	    planes[0].offset = thisDot;
	    nearesti = i;
	}
    }
    if (nearesti >= 0)
    {
	*W0 = affinecoords[nearesti][0];
	*W1 = affinecoords[nearesti][1];
	*W2 = affinecoords[nearesti][2];
	return 1;
    }
    else
	return 0;
}

/*
 * Find closest point to p on the triangle with vertices v0,v1,v2,
 * but only if it's closer than the square root of *closest_distsqrd.
 * On success, set *W0,*W1,*W2 to be the barycentric coordinates
 * of the closest point, update *closest_distsqrd and return 1;
 * on failure, return 0 (in which case *w0,*w1,*w2
 * and *closest_distsqrd is left undisturbed).
 */
extern PFUDLLEXPORT int
pfuGetClosestPointOnTriangle(pfVec3 p,
		       pfVec3 v0, pfVec3 v1, pfVec3 v2,
		       float *closest_distsqrd,
		       float *W0, float *W1, float *W2)
{
    pfVec3 e01, e12, e20; /* edge vectors of the triangle */
    pfVec3 normal;	  /* vector perpendicular to plane of triangle */
    pfVec3 n01, n12, n20; /* perpendicular to resp. edge in plane of triangle */
    pfVec3 pv0, pv1, pv2; /* v0-p, v1-p, v2-p */
    float where_along_e01; /* where p is along the edge e01--
			      -1 means on the v0 side,
			      0 means strictly between v0 and v1,
			      1 means on the v1 side. */
    float where_along_e12; /* likewise */
    float where_along_e20; /* likewise */
    pfVec3 result;	   /* the closest point on the triangle */
    float distsqrd;	   /* square of distance to that point */
    float w0, w1, w2;	   /* barycentric coords of closest point in terms of
			      v0,v1,v2 */

    /*
     * Note, this function could be substantially improved
     * by first testing to see whether any part of the bounding
     * box of the triangle lies within closest_dist of p.
     */

    PFSUB_VEC3(e01, v1, v0);
    PFSUB_VEC3(e12, v2, v1);
    PFSUB_VEC3(e20, v0, v2);
    pfCrossVec3(normal, e01, e12);
    pfCrossVec3(n01, e01, normal);
    pfCrossVec3(n12, e12, normal);
    pfCrossVec3(n20, e20, normal);
    PFSUB_VEC3(pv0, v0, p);
    PFSUB_VEC3(pv1, v1, p);
    PFSUB_VEC3(pv2, v2, p);

    /*
     * Order the tests so that, in the case where
     * this function might be a bottleneck (i.e. where there
     * are many small triangles), the most common cases
     * are detected first.
     * When the triangle is small,
     * the portion of space closest to a vertex is much larger
     * than the portion closest to an edge, which is in turn much larger than
     * the portion closest to the triangle interior,
     * so we should test for vertex, edge, and triangle interior
     * in that order.
     */

    where_along_e01 = (PFDOT_VEC3(pv0,e01) >= 0 ? -1 :
		       PFDOT_VEC3(pv1,e01) <= 0 ? 1 : 0);
    where_along_e12 = (PFDOT_VEC3(pv1,e12) >= 0 ? -1 :
		       PFDOT_VEC3(pv2,e12) <= 0 ? 1 : 0);
    where_along_e20 = (PFDOT_VEC3(pv2,e20) >= 0 ? -1 :
		       PFDOT_VEC3(pv0,e20) <= 0 ? 1 : 0);
    /* XXX messes up if triangle is degenerate! */

    if (where_along_e01 == -1 && where_along_e20 == 1)
    {
	/*
	 * Closest point is v0
	 */
	w0 = 1;
	w1 = 0;
	w2 = 0;
	PFCOPY_VEC3(result, v0);
    }
    else if (where_along_e12 == -1 && where_along_e01 == 1)
    {
	/*
	 * Closest point is v1
	 */
	w0 = 0;
	w1 = 1;
	w2 = 0;
	PFCOPY_VEC3(result, v1);
    }
    else if (where_along_e20 == -1 && where_along_e12 == 1)
    {
	/*
	 * Closest point is v2
	 */
	w0 = 0;
	w1 = 0;
	w2 = 1;
	PFCOPY_VEC3(result, v2);
    }
    else if (PFDOT_VEC3(pv0, n01) <= 0 && where_along_e01 == 0)
    {
	/*
	 * Closest point is along the edge e01
	 */
	w1 = -PFDOT_VEC3(pv0, e01) / PFDOT_VEC3(e01, e01);
	w0 = 1 - w1;
	w2 = 0;
	PFSCALE_VEC3(result, w1, v1);
	PFADD_SCALED_VEC3(result, result, w0, v0);
    }
    else if (PFDOT_VEC3(pv1, n12) <= 0 && where_along_e12 == 0)
    {
	/*
	 * Closest point is along the edge e12
	 */
	w2 = -PFDOT_VEC3(pv1, e12) / PFDOT_VEC3(e12, e12);
	w1 = 1 - w2;
	w0 = 0;
	PFSCALE_VEC3(result, w2, v2);
	PFADD_SCALED_VEC3(result, result, w1, v1);
    }
    else if (PFDOT_VEC3(pv2, n20) <= 0 && where_along_e20 == 0)
    {
	/*
	 * Closest point is along the edge e20
	 */
	w0 = -PFDOT_VEC3(pv2, e20) / PFDOT_VEC3(e20, e20);
	w2 = 1 - w0;
	w1 = 0;
	PFSCALE_VEC3(result, w0, v0);
	PFADD_SCALED_VEC3(result, result, w2, v2);
    }
    else
    {
	/*
	 * Closest point is strictly in the interior of the triangle
	 */
	w0 = PFDOT_VEC3(pv1, n12) / PFDOT_VEC3(e01, n12);
	w1 = PFDOT_VEC3(pv2, n20) / PFDOT_VEC3(e12, n20);
	w2 = PFDOT_VEC3(pv0, n01) / PFDOT_VEC3(e20, n01);
	PFSCALE_VEC3(result, w0, v0);
	PFADD_SCALED_VEC3(result, result, w1, v1);
	PFADD_SCALED_VEC3(result, result, w2, v2);
    }
    /* weights should sum to 1... */
    if (fabs(w0+w1+w2 - 1.) > 1e-3)
    {
	pfNotify(PFNFY_WARN, PFNFY_INTERNAL,
		"pfuGetClosestPointOnTriangle: barycentric coords %g,%g,%g sum to %10.10g",
		w0,w1,w2, w0+w1+w2);
    }
    distsqrd = PFSQR_DISTANCE_PT3(p, result);
    if (distsqrd < *closest_distsqrd)
    {
	*closest_distsqrd = distsqrd;
	*W0 = w0;
	*W1 = w1;
	*W2 = w2;
	return 1;
    }
    else
	return 0;
}

/*
 * Callback data and function for traversal to find closest textured point
 */
#define FIND_FRUSTUM_INTERSECT_TRIANGLES 0
#define FIND_CLOSEST_POINT_TO_EYE 1
struct find_closest_textured_point_args {
    int mode;	      	      /* input, one of the above choices */
    pfPlane clipplanes[5];    /* input, if mode==FRUSTUM_INTERSECT_TRIANGLES */
    pfVec3 eye;		      /* input, if mode==FIND_CLOSEST_POINT_TO_EYE */
    const pfTexture *tex;     /* input */
    float closest_distsqrd;   /* output, caller must initialize to MAXFLOAT */
    float x, y, z;	      /* output */
    float s, t, r;	      /* output */
};

static int find_closest_textured_point_fun(pfuTraverser *trav)
{
/* XXX don't bother messing with eye if mode isn't closest_to_eye! */
    struct find_closest_textured_point_args *args =
	(struct find_closest_textured_point_args *)trav->data;
#ifdef DEBUG
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "traversing node %p (mat stack depth %d)\n",
		trav->node, pfGetMStackDepth(trav->mstack));
#endif

#ifdef NOTYET
    if (closest point in bounding sphere is farther than closest)
	return PFTRAV_PRUNE;
#endif

    if (pfIsOfType(trav->node, pfGetGeodeClassType()))
    {
	pfMatrix *xform;
	float xformscalesqrd;
	pfVec3 eye_in_geode_space;
	pfVec3 closest_in_geode_space;
	float closest_distsqrd_in_geode_space;
	int found_closer_one_in_this_geode = 0;
	int nGSets, i;

	if (pfGetMStackDepth(trav->mstack) > 1)
	{
	    pfMatrix invxform;
	    xform = pfGetMStackTop(trav->mstack);
	    xformscalesqrd = PFDOT_VEC3((*xform)[0], (*xform)[0]);
		/* XXX assumes uniform scaling! */
	    pfInvertFullMat(invxform, *xform);
	    if (args->mode == FIND_CLOSEST_POINT_TO_EYE)
	    {
		pfXformPt3(eye_in_geode_space, args->eye, invxform);
		if (args->closest_distsqrd == MAXFLOAT)
		    closest_distsqrd_in_geode_space = MAXFLOAT;
		else
		    closest_distsqrd_in_geode_space = args->closest_distsqrd
						    / xformscalesqrd;
	    }
	}
	else
	{
	    xform = NULL;
	    if (args->mode == FIND_CLOSEST_POINT_TO_EYE)
	    {
		PFCOPY_VEC3(eye_in_geode_space, args->eye);
		closest_distsqrd_in_geode_space = args->closest_distsqrd;
	    }
	}
#ifdef DEBUG
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "\tgeode matrix:");
	if (xform)
	{
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t\t%g %g %g %g",
		    (*xform)[0][0],(*xform)[0][1],(*xform)[0][2],(*xform)[0][3]);
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t\t%g %g %g %g",
		    (*xform)[1][0],(*xform)[1][1],(*xform)[1][2],(*xform)[1][3]);
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t\t%g %g %g %g",
		    (*xform)[2][0],(*xform)[2][1],(*xform)[2][2],(*xform)[2][3]);
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t\t%g %g %g %g",
		    (*xform)[3][0],(*xform)[3][1],(*xform)[3][2],(*xform)[3][3]);
	}
	else
	{
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t\t(NULL)");
	}
	if (args->mode == FIND_CLOSEST_POINT_TO_EYE)
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "\teye in geode space: (%g %g %g)",
		     eye_in_geode_space[0],
		     eye_in_geode_space[1],
		     eye_in_geode_space[2]);

	{
	pfSphere bsph;
	(void)pfGetNodeBSphere(trav->node, &bsph);
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		"Bounding sphere of geode: center=(%g %g %g), radius=%g",
		bsph.center[0], bsph.center[1], bsph.center[2],
		bsph.radius);
	}

#endif

	nGSets = pfGetNumGSets((pfGeode *)trav->node);
	for (i = 0; i < nGSets; ++i)
	{
	    pfGeoSet *gset;
	    pfGeoState *gstate; /* only used if args->tex != NULL */
	    int tgenmode_s, tgenmode_t, tgenmode_r;
	    pfVec3 tgenplane_s_abc, tgenplane_t_abc, tgenplane_r_abc;
	    float  tgenplane_s_d,   tgenplane_t_d,   tgenplane_r_d;
	    int primtype;
	    pfVec3 *vcoords;
	    pfVec2 *tcoords;
	    ushort *vinds, *tinds;
	    int nPrims, *primLens, primLen;
	    int incr0[2], incr1[2], incr2[2]; /* strides for the three
						 triangle vertices; use [0]
						 for even loop iterations
						 and [1] for odd ones */
	    int primStart;
	    int j;

	    gset = pfGetGSet(trav->node, i);
	    primtype = pfGetGSetPrimType(gset);
	    if (primtype != PFGS_TRIS
	     && primtype != PFGS_QUADS
	     && primtype != PFGS_TRISTRIPS
	     && primtype != PFGS_FLAT_TRISTRIPS
	     && primtype != PFGS_POLYS)
		continue; /* it's not decomposable into triangles */

	    if (args->tex != NULL)
	    {
		/* NOTE gstate is only initialized if args->tex != NULL */
		gstate = pfGetGSetGState(gset);
		if (gstate == NULL)
		    continue;

		if (!pfGetGStateMode(gstate, PFSTATE_ENTEXTURE))
		    continue;

		if (pfGetGStateAttr(gstate, PFSTATE_TEXTURE) != args->tex)
		    continue;
	    }

#ifdef NOTYET
	    if (closest point in bounding box is farther than closest)
		continue;
#endif

	    if (args->tex != NULL)
	    {
		pfTexGen *tgen = pfGetGStateAttr(gstate, PFSTATE_TEXGEN);
		if (tgen != NULL && pfGetGStateMode(gstate, PFSTATE_ENTEXGEN)) {
		    tgenmode_s = pfGetTGenMode(tgen, PF_S);
		    tgenmode_t = pfGetTGenMode(tgen, PF_T);
		    tgenmode_r = pfGetTGenMode(tgen, PF_R);

		    if (tgenmode_s == PFTG_OBJECT_LINEAR)
			pfGetTGenPlane(tgen, PF_S, &tgenplane_s_abc[0],
						   &tgenplane_s_abc[1],
						   &tgenplane_s_abc[2],
						   &tgenplane_s_d);
		    else if (tgenmode_s != PFTG_OFF)
			continue; /* can't handle the mode :-( */

		    if (tgenmode_t == PFTG_OBJECT_LINEAR)
			pfGetTGenPlane(tgen, PF_T, &tgenplane_t_abc[0],
						   &tgenplane_t_abc[1],
						   &tgenplane_t_abc[2],
						   &tgenplane_t_d);
		    else if (tgenmode_t != PFTG_OFF)
			continue; /* can't handle the mode :-( */

		    if (tgenmode_r == PFTG_OBJECT_LINEAR)
			pfGetTGenPlane(tgen, PF_R, &tgenplane_r_abc[0],
						   &tgenplane_r_abc[1],
						   &tgenplane_r_abc[2],
						   &tgenplane_r_d);
		    else if (tgenmode_r != PFTG_OFF)
			continue; /* can't handle the mode :-( */
		} else {
		    tgenmode_s = PFTG_OFF;
		    tgenmode_t = PFTG_OFF;
		    tgenmode_r = PFTG_OFF;
		}

		if (pfGetGSetAttrBind(gset, PFGS_TEXCOORD2) == PFGS_PER_VERTEX)
		    pfGetGSetAttrLists(gset, PFGS_TEXCOORD2,
				       (void**)&tcoords, &tinds);
		else
		{
		    tcoords = NULL;
		    if (tgenmode_t == PFTG_OFF
		     || tgenmode_s == PFTG_OFF)
		    {
			/*
			 * XXX This can happen if texgen parameters are
			 * being set in the preDraw callback,
			 * which is what happens when the im loader
			 * is being used.  This function
			 * is useless for that case-- should
			 * fix the builder so that it puts a pfTexGen in
			 * the geostate so that we know what's going on here!
			 */
			static int notified = 0;
			if (!notified)
			    pfNotify(notified ? PFNFY_DEBUG : PFNFY_WARN,
			             PFNFY_INTERNAL,
				     "closest: there's a texture(%s), texgen mode is PFTG_OFF and no tex coords???", pfGetTexName(args->tex));
			notified = 1;
		    }
		}
	    }
	    pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&vcoords, &vinds);

	    nPrims = pfGetGSetNumPrims(gset);

	    switch (primtype)
	    {
		case PFGS_TRIS:
		    incr0[0] = 3;
		    incr1[0] = 3;
		    incr2[0] = 3;
		    incr0[1] = 3;
		    incr1[1] = 3;
		    incr2[1] = 3;
		    primLens = NULL;
		    /* hack to make one big inner loop instead of
		       tons of little ones... */
		    primLen = nPrims+2; /* since we assume ntris = primLen-2 */
		    nPrims = 1;
		    break;
		case PFGS_QUADS:
		    incr0[0] = 0;
		    incr1[0] = 1;
		    incr2[0] = 1;
		    incr0[1] = 4;
		    incr1[1] = 3;
		    incr2[1] = 3;
		    primLens = NULL;
		    /* hack to make one big inner loop instead of
		       tons of little ones... */
		    primLen = 2*nPrims+2; /* since we assume ntris = primLen-2*/
		    nPrims = 1;
		    break;
		case PFGS_FLAT_TRISTRIPS:
		case PFGS_TRISTRIPS:
		    incr0[0] = 2;
		    incr1[0] = 0;
		    incr2[0] = 1;
		    incr0[1] = 0;
		    incr1[1] = 2;
		    incr2[1] = 1;
		    primLens = pfGetGSetPrimLengths(gset);
		    break;
		case PFGS_POLYS:
		    incr0[0] = 0;
		    incr1[0] = 1;
		    incr2[0] = 1;
		    incr0[1] = 0;
		    incr1[1] = 1;
		    incr2[1] = 1;
		    primLens = pfGetGSetPrimLengths(gset);
		    break;
		default:
		    /* this can't happen, but to catch mistakes
		       if future types are added... */
		    pfNotify(PFNFY_FATAL, PFNFY_INTERNAL,
			"find_closest_textured_point_fun: bad geoset prim type %d", primtype);
	    }
	    primStart = 0;
	    for (j = 0; j < nPrims; ++j)
	    {
		int found_better;
		int i0 = primStart;
		int i1 = i0+1;
		int i2 = i1+1;
		int k;
		if (primLens != NULL)
		    primLen = primLens[j];
		for (k = 2; k < primLen; ++k)    /* always primLen-2 triangles */
		{
		    float w0, w1, w2;
		    int vi0=i0, vi1=i1, vi2=i2;
		    if (vinds != NULL)
		    {
			vi0 = vinds[vi0];
			vi1 = vinds[vi1];
			vi2 = vinds[vi2];
		    }
		    if (args->mode == FIND_FRUSTUM_INTERSECT_TRIANGLES)
		        found_better = nearestVisiblePointToEyePlaneOnTriangle(
			    xform,
			    args->clipplanes,
			    vcoords[vi0], vcoords[vi1], vcoords[vi2],
			    &w0,&w1,&w2);
		    else
			found_better = pfuGetClosestPointOnTriangle(
			    eye_in_geode_space,
			    vcoords[vi0], vcoords[vi1], vcoords[vi2],
			    &closest_distsqrd_in_geode_space,
			    &w0,&w1,&w2);
		    if (found_better)
		    {
			closest_in_geode_space[0] = w0*vcoords[vi0][0]
						  + w1*vcoords[vi1][0]
						  + w2*vcoords[vi2][0];
			closest_in_geode_space[1] = w0*vcoords[vi0][1]
						  + w1*vcoords[vi1][1]
						  + w2*vcoords[vi2][1];
			closest_in_geode_space[2] = w0*vcoords[vi0][2]
						  + w1*vcoords[vi1][2]
						  + w2*vcoords[vi2][2];
			if (args->tex != NULL)
			{
			    args->s = 0;
			    args->t = 0;
			    args->r = 0;
			    if (tcoords != NULL)
			    {
				int ti0=i0, ti1=i1, ti2=i2;
				if (tinds != NULL)
				{
				    ti0 = tinds[ti0];
				    ti1 = tinds[ti1];
				    ti2 = tinds[ti2];
				}
				args->s = w0*tcoords[ti0][0]
					+ w1*tcoords[ti1][0]
					+ w2*tcoords[ti2][0];
				args->t = w0*tcoords[ti0][1]
					+ w1*tcoords[ti1][1]
					+ w2*tcoords[ti2][1];
			    }
			    if (tgenmode_s == PFTG_OBJECT_LINEAR)
				args->s = PFDOT_VEC3(closest_in_geode_space,
						     tgenplane_s_abc)
				        + tgenplane_s_d;
			    if (tgenmode_t == PFTG_OBJECT_LINEAR)
				args->t = PFDOT_VEC3(closest_in_geode_space,
						     tgenplane_t_abc)
				        + tgenplane_t_d;
			    if (tgenmode_r == PFTG_OBJECT_LINEAR)
				args->r = PFDOT_VEC3(closest_in_geode_space,
						     tgenplane_r_abc)
				        + tgenplane_r_d;
			}
#ifdef DEBUG
			pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
				"   found closer point %g %g %g",
				closest_in_geode_space[0],
				closest_in_geode_space[1],
				closest_in_geode_space[2]);
#endif
			found_closer_one_in_this_geode = 1;
		    }
		    i0 += incr0[k&1];
		    i1 += incr1[k&1];
		    i2 += incr2[k&1];
		}
		primStart += primLen;
	    }
	}

	if (found_closer_one_in_this_geode)
	{
	    if (xform != NULL)
	    {
		pfVec3 closest;
		pfXformPt3(closest, closest_in_geode_space, *xform);
		args->x = closest[0];
		args->y = closest[1];
		args->z = closest[2];
		if (args->mode == FIND_CLOSEST_POINT_TO_EYE)
		    args->closest_distsqrd = closest_distsqrd_in_geode_space
				     * xformscalesqrd;
		else
		    args->closest_distsqrd = 0; /* anything but MAXFLOAT */
	    }
	    else
	    {
		args->x = closest_in_geode_space[0];
		args->y = closest_in_geode_space[1];
		args->z = closest_in_geode_space[2];
		if (args->mode == FIND_CLOSEST_POINT_TO_EYE)
		    args->closest_distsqrd = closest_distsqrd_in_geode_space;
		else
		    args->closest_distsqrd = 0; /* anything but MAXFLOAT */
	    }
	}
    }
    return PFTRAV_CONT;
}

/*
 * Traverse a graph looking for the closest point (on any triangle)
 * to a given point.
 * Return the x,y,z coords of the point.
 * If tex != NULL, restrict the search to triangles that are textured
 * by tex.
 * On success, return 1 and set x,y,z to be the coords of the point,
 * and if tex != NULL, set s,t,r to be the texture coords.
 * On failure (there are no triangles, or, if tex != NULL,
 * there are no triangles textured by tex) return 0.
 *
 * WARNING: This function assumes all coordinate scales in the
 * scene are uniform.
 */
extern PFUDLLEXPORT int
pfuGetClosestPoint(pfNode *node,
		   float eyex, float eyey, float eyez,
		   const pfTexture *tex,
		   int travmode,
		   float *x, float *y, float *z,
		   float *s, float *t, float *r)
{
    pfuTraverser traverser;
    static pfMatStack *ms = NULL; /* XXX not reentrant! */
    static int msSize = 0;
    struct find_closest_textured_point_args args;
    int depth;

    {
	/* XXX super hack to get the quick answer if it's a unit sphere */
	static int is_unit_sphere = -1;
	if (is_unit_sphere == -1)
	{
	    char *e = getenv("_PFUCLOSEST_IS_UNIT_SPHERE");
	    is_unit_sphere = (e ? *e ? atoi(e) : 1 : 0);
	}
	if (is_unit_sphere)
	{
	    pfVec3 eye;
	    PFSET_VEC3(eye, eyex, eyey, eyez);
	    pfNormalizeVec3(eye);
	    if (x != NULL) *x = eye[PF_X];
	    if (y != NULL) *y = eye[PF_Y];
	    if (z != NULL) *z = eye[PF_Z];
	    if (s != NULL) *s = pfArcSin(*z)/180.0f + .5;
	    if (t != NULL) *t = pfArcTan2(*x,-*y)/360.0f + .5;
	    if (r != NULL) *r = 0;
	    return 1;
	}
    }

    args.mode = FIND_CLOSEST_POINT_TO_EYE;
    pfSetVec3(args.eye, eyex, eyey, eyez);
    args.tex = tex;
    args.closest_distsqrd = MAXFLOAT;

    depth = pfuCalcDepth(node);
    if (depth > msSize)
    {
	if (ms != NULL)
	    pfDelete(ms);
	ms = pfNewMStack(depth, pfGetSharedArena());
	msSize = depth;
    }

    pfuInitTraverser(&traverser);

    traverser.preFunc = find_closest_textured_point_fun;
    traverser.data = (void *)&args;
    traverser.mstack = ms;
    traverser.mode = travmode;

    pfuTraverse(node, &traverser);

    if (args.closest_distsqrd < MAXFLOAT)
    {
	if (x != NULL) *x = args.x;
	if (y != NULL) *y = args.y;
	if (z != NULL) *z = args.z;
	if (tex != NULL)
	{
	    if (s != NULL) *s = args.s;
	    if (t != NULL) *t = args.t;
	    if (r != NULL) *r = args.r;
	}
	return 1;
    }
    else
	return 0;
}


/* XXX assume knowledge of frustum internals */
#define PFFRUST_LEFT	0
#define PFFRUST_RIGHT	1
#define PFFRUST_NEAR	2
#define PFFRUST_FAR	3
#define PFFRUST_BOTTOM	4    
#define PFFRUST_TOP	5
static const int frust_clip_order[5] = {
    PFFRUST_FAR, /* far must be first! */
    PFFRUST_LEFT, PFFRUST_RIGHT, PFFRUST_BOTTOM, PFFRUST_TOP
};


extern PFUDLLEXPORT int
pfuGetNearestVisiblePointToEyePlane(pfNode *node,
				    pfFrustum *frust,
				    const pfTexture *tex,
				    int travmode,
				    float *x, float *y, float *z,
				    float *s, float *t, float *r)
{
    pfuTraverser traverser;
    static pfMatStack *ms = NULL; /* XXX not reentrant! */
    static int msSize = 0;
    struct find_closest_textured_point_args args;
    int i, depth;

    args.mode = FIND_FRUSTUM_INTERSECT_TRIANGLES;
    args.tex = tex;
    args.closest_distsqrd = MAXFLOAT; /* actually used for min dot prod */
    for (i = 0; i < 5; ++i)
	(void) pfGetPtopeFacet((pfPolytope *)frust, frust_clip_order[i],
			       &args.clipplanes[i]);

    depth = pfuCalcDepth(node);
    if (depth > msSize)
    {
	if (ms != NULL)
	    pfDelete(ms);
	ms = pfNewMStack(depth, pfGetSharedArena());
	msSize = depth;
    }

    pfuInitTraverser(&traverser);

    traverser.preFunc = find_closest_textured_point_fun;
    traverser.data = (void *)&args;
    traverser.mstack = ms;
    traverser.mode = travmode;

    pfuTraverse(node, &traverser);

    if (args.closest_distsqrd < MAXFLOAT) /* actually min dot prod */
    {
	*x = args.x;
	*y = args.y;
	*z = args.z;
	if (tex != NULL)
	{
	    *s = args.s;
	    *t = args.t;
	    *r = args.r;
	}
	return 1;
    }
    else
	return 0;
}
