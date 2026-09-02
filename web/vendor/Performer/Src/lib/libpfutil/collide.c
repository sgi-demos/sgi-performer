/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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
 * collide.c
 *
 * $Revision: 1.36 $
 * $Date: 2002/05/15 00:31:09 $
 * 
 * simple collision routines for moving DCSes
 */

#define _PFU_COLLIDE_C_		/* don't want typdefing #defs */
#include <Performer/pfutil.h>

/* Channel used for evaluating LODs when intersecting */
static pfChannel	*isectChan = NULL;

	/*-----------------------------------------------------*/

PFUDLLEXPORT void
pfuCollisionChan(pfChannel *chan)
{
    isectChan = chan;
}

PFUDLLEXPORT pfChannel*
pfuGetCollisionChan(void)
{
    return isectChan;
}

/* 
 * Set up intersection masks for collision detection using 'mask' as the 
 * intersection mask. Cache normals and such inside geosets if the geometry 
 * is static, but not if it is dynamic, i.e., its vertices change.
 */
PFUDLLEXPORT void
pfuCollideSetup(pfNode *node, int mode, int mask)
{
    int		completeMode;
    double	startTime;
    double	elapsedTime;

    pfNotify(PFNFY_INFO, PFNFY_PRINT, "pfuCollideSetup collision traversal");

    if (mode == PFUCOLLIDE_DYNAMIC)
    {
	completeMode = PFTRAV_SELF|PFTRAV_DESCEND|PFTRAV_IS_UNCACHE;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "  Intersection data is uncached");
    }
    else
    {
	completeMode = PFTRAV_SELF|PFTRAV_DESCEND|PFTRAV_IS_CACHE;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "  Intersection data is cached");
    }

    startTime = pfGetTime();
    pfNodeTravMask(node, PFTRAV_ISECT, mask, completeMode, PF_OR);
    elapsedTime = pfGetTime() - startTime;

    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Elapsed time: %.3f sec", elapsedTime);
    pfNotify(PFNFY_INFO, PFNFY_MORE, NULL);
}

PFUDLLEXPORT int
pfuCollideGrnd(pfCoord *coord, pfNode *node, pfVec3 zpr)
{
    int	    isect;
    pfHit	    **hits[32];
    pfSegSet        segset;

    zpr[0] = -1.0f;
    zpr[1] = 0.0f;
    zpr[2] = 0.0f;

    /* Make a ray looking "down" at terrain */
    PFCOPY_VEC3(segset.segs[0].pos, coord->xyz);
    segset.segs[0].pos[2] += 25000.0f;	    /* Find terrain even if we're below it */
    PFSET_VEC3(segset.segs[0].dir, 0.0f, 0.0f, -1.0f);
    segset.segs[0].length = 50000.0f;

    segset.mode = PFTRAV_IS_PRIM|PFTRAV_IS_NORM|PFTRAV_IS_CULL_BACK;
    segset.userData = NULL;
    segset.isectMask = PFUCOLLIDE_GROUND;
    segset.activeMask = 0x1;
    segset.bound = NULL;
    segset.discFunc = NULL;

    if (isectChan)
	isect = pfChanNodeIsectSegs(isectChan, node, &segset, hits, NULL);
    else
	isect = pfNodeIsectSegs(node, &segset, hits);

    /* See if we collided with object. */
    if (isect > 0)
    {
	uint query[] = { PFQHIT_FLAGS, PFQHIT_POINT, PFQHIT_NORM, NULL };
	struct 
	{
	    int flags;
	    pfVec3 point;
	    pfVec3 normal;
	} result;
	pfMatrix xform;
	    
	pfMQueryHit(hits[0][0], query, &result);

	if ((result.flags & PFHIT_POINT) && (result.flags & PFHIT_NORM))
	{
	    if (result.flags & PFHIT_XFORM)
	    {
		pfQueryHit(hits[0][0], PFQHIT_XFORM, &xform);

		pfXformPt3(result.point, result.point, xform);
		pfXformVec3(result.normal, result.normal, xform);
		pfNormalizeVec3(result.normal);
	    }
	    
	    zpr[0] = result.point[PF_Z];
	    
	    {
		pfVec3  head, head90;
		float   sh, ch;
		float   dotp;
		
		pfSinCos(coord->hpr[0], &sh, &ch);
		
		head[0] = -sh;
		head[1] =  ch;
		head[2] = 0.0f;
		dotp = PFDOT_VEC3(head, result.normal);
		zpr[1]  = pfArcCos(dotp) - 90.0f;
		
		head90[0] =  ch;
		head90[1] =  sh;
		head90[2] = 0.0f;
		dotp = PFDOT_VEC3(head90, result.normal);
		zpr[2]  = 90.0f - pfArcCos(dotp);
	    }
	    return PFUCOLLIDE_GROUND;
	}
    }
    return FALSE;
}


PFUDLLEXPORT int
pfuCollideObj(pfSeg *seg, pfNode *objNode, pfVec3 hitPos, pfVec3 hitNorm)
{
    int	    	isect;
    pfHit	**hits[32];
    pfSegSet    segset;

    segset.mode = PFTRAV_IS_PRIM|PFTRAV_IS_NORM|PFTRAV_IS_CULL_BACK;
    segset.userData = NULL;
    segset.isectMask = PFUCOLLIDE_OBJECT;
    segset.activeMask = 0x1;
    segset.bound = NULL;
    segset.discFunc = NULL;
    segset.segs[0] = *seg;

    if (isectChan)
	isect = pfChanNodeIsectSegs(isectChan, objNode, &segset, hits, NULL);
    else
	isect = pfNodeIsectSegs(objNode, &segset, hits);

    /* see if we collided with object */
    if (isect > 0)
    {
	uint query[] = { PFQHIT_FLAGS, PFQHIT_POINT, PFQHIT_NORM, NULL };
	struct 
	{
	    int flags;
	    pfVec3 point;
	    pfVec3 normal;
	} result;
	pfMatrix xform;
	    
	pfMQueryHit(hits[0][0], query, &result);

	/* transform intersection */
	if (result.flags & PFHIT_XFORM)
	{
	    /* get matrix */
	    pfQueryHit(hits[0][0], PFQHIT_XFORM, &xform);

	    /* transform point */
	    if (result.flags & PFHIT_POINT)
		pfXformPt3(result.point, result.point, xform);

	    /* transform normal */
	    if (result.flags & PFHIT_NORM)
	    {
		pfXformVec3(result.normal, result.normal, xform);
		pfNormalizeVec3(result.normal);
	    }
	}

	/* copy intersection data to user's arguments */
	if ((result.flags & PFHIT_POINT) && (hitPos  != NULL))
	    PFCOPY_VEC3(hitPos, result.point);
	if ((result.flags & PFHIT_NORM)  && (hitNorm != NULL))
	    PFCOPY_VEC3(hitNorm, result.normal);

	/* indicate that intersection occurred */
	return PFUCOLLIDE_OBJECT;
    }
    else
	return FALSE;
}

PFUDLLEXPORT int
pfuCollideGrndObj(pfCoord *coord, pfNode *grndNode, pfVec3 zpr, 
		  pfSeg *seg, pfNode *objNode, pfVec3 hitPos, pfVec3 hitNorm)
{
    int 	    retval = 0;
    int	    isect;
    pfHit	    **hits[32];
    pfSegSet        segset;

    /* Collide separately if ground and object nodes are different */
    if (grndNode != objNode)
    {
	return pfuCollideGrnd(coord, grndNode, zpr) | 
	       pfuCollideObj(seg, objNode, hitPos, hitNorm);
    }

    /* Else, we can intersect both segments at once for better performance */
    zpr[0] = -1.0f;
    zpr[1] = 0.0f;
    zpr[2] = 0.0f;

    /* Make a ray looking "down" at terrain */
    PFCOPY_VEC3(segset.segs[0].pos, coord->xyz);
    segset.segs[0].pos[2] += 25000.0f;	    /* Find terrain even if we're below it */
    PFSET_VEC3(segset.segs[0].dir, 0.0f, 0.0f, -1.0f);
    segset.segs[0].length = 50000.0f;

    /* Copy user supplied segment */
    segset.segs[1] = *seg;

    segset.mode = PFTRAV_IS_PRIM|PFTRAV_IS_NORM|PFTRAV_IS_CULL_BACK;
    segset.userData = NULL;
    segset.isectMask = PFUCOLLIDE_GROUND;
    segset.activeMask = 0x3;
    segset.bound = NULL;
    segset.discFunc = NULL;

    if (isectChan)
	isect = pfChanNodeIsectSegs(isectChan, grndNode, &segset, hits, NULL);
    else
	isect = pfNodeIsectSegs(grndNode, &segset, hits);

    /* See if we collided with anything. */
    if (isect > 0)
    {
	uint query[] = { PFQHIT_FLAGS, PFQHIT_POINT, PFQHIT_NORM, NULL };
	struct 
	{
	    int flags;
	    pfVec3 point;
	    pfVec3 normal;
	} result;
	pfMatrix xform;
	    
	pfMQueryHit(hits[0][0], query, &result);

	if ((result.flags & PFHIT_POINT) && (result.flags & PFHIT_NORM))
	{
	    if (result.flags & PFHIT_XFORM)
	    {
		pfQueryHit(hits[0][0], PFQHIT_XFORM, &xform);

		pfXformPt3(result.point, result.point, xform);
		pfXformVec3(result.normal, result.normal, xform);
		pfNormalizeVec3(result.normal);
	    }
	    
	    zpr[0] = result.point[PF_Z];
	    
	    {
		pfVec3  head, head90;
		float   sh, ch;
		float   dotp;
		
		pfSinCos(coord->hpr[0], &sh, &ch);
		
		head[0] = -sh;
		head[1] =  ch;
		head[2] = 0.0f;
		dotp = PFDOT_VEC3(head, result.normal);
		zpr[1]  = pfArcCos(dotp) - 90.0f;
		
		head90[0] =  ch;
		head90[1] =  sh;
		head90[2] = 0.0f;
		dotp = PFDOT_VEC3(head90, result.normal);
		zpr[2]  = 90.0f - pfArcCos(dotp);
	    }
	    retval |= PFUCOLLIDE_GROUND;
	}
	    
	pfMQueryHit(hits[1][0], query, &result);

	if ((result.flags & PFHIT_POINT) && (result.flags & PFHIT_NORM))
	{
	    if (result.flags & PFHIT_XFORM)
	    {
		pfQueryHit(hits[0][0], PFQHIT_XFORM, &xform);
		pfXformPt3(result.point, result.point, xform);
		pfXformVec3(result.normal, result.normal, xform);
		pfNormalizeVec3(result.normal);
	    }
	    PFCOPY_VEC3(hitPos, result.point);
	    PFCOPY_VEC3(hitNorm, result.normal);

	    retval |= PFUCOLLIDE_OBJECT;
	}
    }
    return retval;
}
