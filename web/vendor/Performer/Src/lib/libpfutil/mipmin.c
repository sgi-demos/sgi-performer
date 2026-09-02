/*
 * Copyright 1996, Silicon Graphics, Inc.
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
 * file: mipmin.c
 *
 * $Revision: 1.6 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * -----------------
 *
	Inputs:
		View frustum
		Viewport size in pixels
		Distance from eye to closest point (or to bounding volume)
			in object space
		Min change in s,t per change in x,y,z in object space
		Max texture size (used only for clamping the result,
		which can otherwise be unbounded).
	Output:
		The floating-point
		size of the biggest (finest) square mipmap level
		that can be needed for rendering textured geometry
		subject to the above constraints,
		clamped to the range 1..maxtexsize.

		Thus, for a square mipmap texture,
		the min (finest) lod for rendering can be clamped to
			log2(texturesize / result),
		without any visible change.

	Algorithm:
		We want a method that does not change
		if the eye's heading,pitch,roll changes,
		so it suffices to do the calculation for
		the "worst" possible euler angle,
		i.e. a viewing rotation that takes c (= the point on the
		the sphere closest to the eye)
		and puts it in the corner of the screen
		farthest from the view direction.

		At that "worst" viewing rotation, the distance
		from c to the eye plane (in eye space) is
		   d = dist(eye,c) * cos(angle between view dir
				         and farthest frust corner)
		     = dist(eye,c) * neardist
				  / dist(eye, farthest point on near frust face)
		The magnification of any point
		at distance d from the eye plane is:
		   neardist/d * max(viewportwidth/nearwidth,
				    viewportheight/nearheight)
		So the magnification of the worst point at the
		worst rotation is:
		    dist(eye,farthest point on near frust face)
		    * max(viewportwidth/nearwidth, viewportheight/nearheight)
		    / dist(eye,c)

		Magnification is easier to think about than minification
		(its inverse) but unfortunately magnification
		can be infinite, so invert our thinking...

		So the above says that the min possible point minification
		(i.e. change in world x,y,z per change in screen x,y) is:
		    dist(eye,c) * min(nearwidth/viewportwidth,
				      nearheight/viewportheight)
		    / dist(eye, farthest point on near frust face)

		So the size of the min possible pixel in texture coords
		is the above times min_dst_dxyz.

		Divide by sqrt(2) (to account for the worst
		case of 45 degrees rotation of a square textured in the
		canonical way, in which case all the minifications
		du/dx, du/dy, dv/dx, dv/dy decrease
		(i.e. calculated magnification factor increases)
		Take the inverse, clamp to 1..maxtexsize,
		and that's the answer.
*/

#include <Performer/pfutil.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#define log2(x) (log(x)*M_LOG2E)

#ifdef WIN32 /* XXX Alex -- put this in win32stubs.h */
#ifndef M_SQRT1_2
#define M_SQRT1_2       0.70710678118654752440
#endif
#endif

extern PFUDLLEXPORT float
pfuCalcSizeFinestMipLOD(pfFrustum *cullFrust,	/* in eye space */
		       int viewportWidth, int viewportHeight,
		       float heightAboveTerrain,/* lower bound, in obj space */
		       float min_dst_dxyz, /* min change in tex coords per position in obj space */
		       int maxtexsize) /* result will be clamped to 1..maxtexsize */
{
    float min_possible_texture_minification;
    float max_possible_miplevel_size;
    pfVec3 ll, lr, ul, ur; /* vertices of front frust face */
    float nearwidth, nearheight; /* size of near frust face */
    float dist_eye_to_farthest_point_on_near_frust_face;

    /*
     * If we converted heightAboveTerrain and min_dst_dxyz
     * to eye space, it would mean multiplying heightAboveTerrain
     * by a scale factor and dividing min_dst_dxyz by the same
     * scale factor; since we end up multiplying
     * heightAboveTerrain by min_dst_dxyz,
     * this would cancel out, so it's unneccessary to
     * even think about it.
     */

    /*
     * Find the corner of the near frust plane
     * that's farthest from the view direction.
     */
    pfGetFrustNear(cullFrust, ll, lr, ul, ur);
    {
	float lldistsqrd = PFDOT_VEC3(ll, ll);
	float lrdistsqrd = PFDOT_VEC3(lr, lr);
	float uldistsqrd = PFDOT_VEC3(ul, ul);
	float urdistsqrd = PFDOT_VEC3(ur, ur);
	dist_eye_to_farthest_point_on_near_frust_face =
	    pfSqrt(PF_MAX4(lldistsqrd, lrdistsqrd, uldistsqrd, urdistsqrd));
    }

    {
	static int trivial = -1;
	if (trivial == -1)
	{
	    char *e = getenv("_PFUMIPMIN_TRIVIAL");
	    trivial = (e ? *e ? atoi(e) : 1 : 0);
	}
	if (trivial)
	{
	    /*
	     * Use the nearest instead of farthest
	     * point on the near frustum face,
	     * since that's easier to visually
	     * verify correct.
	     */
	    pfVec3 side0, side1; /* of near clip face */
	    pfVec3 viewDir;
	    PFSUB_VEC3(side0, lr, ll);
	    PFSUB_VEC3(side1, ur, lr);
	    pfCrossVec3(viewDir, side1, side0);
	    pfNormalizeVec3(viewDir);
	    dist_eye_to_farthest_point_on_near_frust_face =
		PFDOT_VEC2(viewDir, ll);
	}
    }

    /*
     * Find the width and height of the near frustum face
     */
    nearwidth = PF_ABS(lr[PF_X] - ll[PF_X]);
    nearheight = PF_ABS(ul[PF_Z] - ll[PF_Z]);

    /*
     * Calculate the min possible texture minification
     * (change in s,t per change in screen x,y).
     */
    min_possible_texture_minification = min_dst_dxyz
	      * heightAboveTerrain
	      * PF_MIN2(nearwidth/viewportWidth, nearheight/viewportHeight)
	      / dist_eye_to_farthest_point_on_near_frust_face
	      * (float)M_SQRT1_2;


    /*
     * Size of the finest mip lod is
     * the inverse of min_possible_texture_minification,
     * clamped to 1..maxtexsize.
     * Clamp before inverting, to avoid possible division by 0.
     */
    if (min_possible_texture_minification <= 1.0f/maxtexsize)
	max_possible_miplevel_size = maxtexsize;
    else if (min_possible_texture_minification >= 1)
	max_possible_miplevel_size = 1;
    else
	max_possible_miplevel_size =
		    1.0f/min_possible_texture_minification;

    return max_possible_miplevel_size;
}







/* XXX OLD DEFUNCT UNNECCESSARILY COMPLICATED API-- THIS SHOULD GO AWAY */
/*
 * file: mipmin.c
 *
 * $Revision: 1.6 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * -----------------
 *
	Inputs:
		View frustum
		Window size in pixels
		Geometry bounding box
		Tex gen parameters in world space
			(so if there's a scale, they would get divided
			by the scale amount)
			(constant term doesn't matter)
			(this should define an upper bound on
			the texture magnification,
			i.e. the actual parameters are allowed
			to be >= (magnification <=) these params).
	Output:
		log base 2 of the size of the biggest (finest) mipmap level,
		in s and t, that can be needed for rendering textured geometry
		in the bounding box, subject to the above constraints.

		Thus the min (finest) lod for rendering can be clamped up to
			MIN(log2(texsizeS) - resultS,
			    log2(texsizeT) - resultT)
		without any visible change.
*/

#include <Performer/pfutil.h>
#include <assert.h>
#include <math.h>
#define log2(x) (log(x)*M_LOG2E)

/*
 * Bounding sphere must be in eye space! (Transform by
 * the traverser matrix and then the inverse of the viewing matrix,
 * if necessary).
 */
extern PFUDLLEXPORT float
pfuMipMinLOD(pfChannel *chan,
	     pfFrustum *cullfrust, /* region of interest */
	     pfSphere *bsph,	 /* bounding sphere of geometry, in eye space */
	     float min_dst_dxyz, /* min change in tex coords per position */
	     int logmaxtexdim,
	     int algorithm)
{
    /* Stuff to query from the channel */
    float fovX, fovY;
    int viewportWidth, viewportHeight;

    pfVec3 p;
    float min_possible_texture_minification;
    float max_possible_miplevel_size;
    float log_max_possible_miplevel_size;
    float minLOD;
    int use_cone;

    assert(algorithm == PFUMINMIPLOD_FRUSTUM_INTERSECT_SPHERE
	|| algorithm == PFUMINMIPLOD_CONE_INTERSECT_SPHERE); /* others one not implemented yet */
    use_cone = (algorithm == PFUMINMIPLOD_CONE_INTERSECT_SPHERE);

    pfRecommendNearFrustSphere(cullfrust, bsph, p, use_cone);

    /*
     * p is the worst-case point (in eye space)
     * for minLOD calculation.
     *
     * Figure out how big it is on the screen (w.r.t.
     * how big it is in eye space).
     */

    /*
     * Texel magnification is easier to think about than
     * minification (its inverse),
     * but unfortunately it can be infinite, so
     * we really need to do calculations in terms of minification.
     * We are looking for the "minimum possible minification"
     * (i.e. the maximum possible magnification).
     * min_possible_texture_minification is the size
     * of the smallest possible pixel in texture coordinates.
     */
    pfGetChanFOV(chan, &fovX, &fovY);
    pfGetChanSize(chan, &viewportWidth, &viewportHeight);

    min_possible_texture_minification = min_dst_dxyz
	      * p[PF_Y] 
	      * PF_MIN2(PF_ABS(pfTan(.5*fovX)*2/viewportWidth),
			PF_ABS(pfTan(.5*fovY)*2/viewportHeight))
	      * (float)M_SQRT1_2;
	 /* XXX EEK! macro expansion causes 6 pfTan calls
	    XXX Not sure fov thing is right when
	    XXX viewing off-axis.
	 */

	/* the "/ sqrt(2)" is to account for a worst case
	   of 45 degrees rotation of a square textured
	   in the canonical way,
	   in which case all the minifications
	   du/dx, du/dy, dv/dx, dv/dy decrease
	   (i.e. calculated magnification factor increases)
	   by a factor of sqrt(2). */

    /* clamp before taking log rather than after, to avoid log(0)
       (though it would probably work anyway) */
    if (min_possible_texture_minification <= 1.0f/(1<<logmaxtexdim))
	max_possible_miplevel_size = (1<<logmaxtexdim);
    else if (min_possible_texture_minification >= 1)
	max_possible_miplevel_size = 1;
    else
	max_possible_miplevel_size =
		    1.0f/min_possible_texture_minification;

    log_max_possible_miplevel_size=log2(max_possible_miplevel_size);

    minLOD = logmaxtexdim - log_max_possible_miplevel_size;
    return minLOD;
}
