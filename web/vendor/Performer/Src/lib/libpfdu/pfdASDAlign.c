/*
 * Copyright 2000, Silicon Graphics, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>

static int normalizeVector (float *from, float *to);
static int isVectorInFront (float *v0, float *v1, float *d);

/*============================================================================*/
PFDUDLLEXPORT
void pfdAlignVerticesToASD (
			pfASD		*asd,
			pfVec3 		*_v,
			int		_nofVertices,
			float 		*_base,
			float 		*_down,
			float 		*_azimuth,
		        unsigned long	_opcode,
			pfVec3		*_v_aligned)
/*============================================================================*/
{
    int				i;
    pfMatrix			M;
    float			base_pos[3], base_normal[3];
    pfVec3 			norm, azim;
    pfVec3 			right_wing, nose;
    register float		x, y, z;
    float			down[3];

    /*
     *	======================================================
     * 	Normalize and resolve NULL input directions.
     *	======================================================
     */

    if (_down)
	normalizeVector (_down, down);
    else
    {
	down[0] = 0.0;
	down[1] = 0.0;
	down[2] = -1.0;
    }

    /*
     *	======================================================
     * 	Generate global transformation matrix for triangles.
     *	======================================================
     */

    pfASDProjectPointFinestPositionNormal 
		    (asd, _base, down, 0, base_pos, base_normal);
    
    if (_opcode & PR_ALIGN_TRANSLATE)
	pfMakeTransMat (M, base_pos[0], base_pos[1], base_pos[2]);
    else
	pfMakeIdentMat(M);

    if (_opcode & PR_ALIGN_NORMAL)
    {
	pfSetVec3(norm, base_normal[0], base_normal[1], base_normal[2]);

	if (_azimuth)
	    pfSetVec3(azim, _azimuth[0], _azimuth[1], _azimuth[2]);
	else
	    pfSetVec3(azim, 1.0, 0.0, 0.0);

	pfNormalizeVec3(norm);

	pfCrossVec3(right_wing,norm, azim);
	pfCrossVec3(nose,right_wing, norm);

	pfNormalizeVec3(right_wing);
	pfNormalizeVec3(nose);

	M[0][0] = nose[0];
	M[0][1] = nose[1];
	M[0][2] = nose[2];
	M[1][0] = right_wing[0];
	M[1][1] = right_wing[1];
	M[1][2] = right_wing[2];
	M[2][0] = norm[0];
	M[2][1] = norm[1];
	M[2][2] = norm[2];
    }
    else
    if (_opcode & PR_ALIGN_AZIMUTH)
    {
	pfSetVec3 (norm, -down[0], -down[1], -down[2]);

	if (_azimuth)
	    pfSetVec3(azim, _azimuth[0], _azimuth[1], _azimuth[2]);
	else
	    pfSetVec3(azim, 1.0, 0.0, 0.0);

	pfCrossVec3(right_wing, norm, azim);
	pfCrossVec3(nose, right_wing, norm);

	pfNormalizeVec3 (right_wing);
	pfNormalizeVec3 (nose);

	M[0][0] = nose[0];
	M[0][1] = nose[1];
	M[0][2] = nose[2];
	M[1][0] = right_wing[0];
	M[1][1] = right_wing[1];
	M[1][2] = right_wing[2];
	M[2][0] = norm[0];
	M[2][1] = norm[1];
	M[2][2] = norm[2];
    }
    else
    if (_opcode & PR_ALIGN_AXIS_BILLBOARD)
    {
	pfSetVec3 (norm, -down[0], -down[1], -down[2]);

	if (_azimuth)
	    pfSetVec3 (azim, -_azimuth[0], -_azimuth[1], -_azimuth[2]);
	else
	    pfSetVec3 (azim, 0.0, 1.0, 0.0);

	pfNormalizeVec3 (norm);

	pfCrossVec3 (right_wing, norm, azim);
	pfCrossVec3 (nose, right_wing, norm);

	pfNormalizeVec3 (right_wing);
	pfNormalizeVec3 (nose);
	M[0][0] = right_wing[0]; /* X axis along billboard face */
	M[0][1] = right_wing[1];
	M[0][2] = right_wing[2];
	M[1][0] = nose[0]; /* Y axis points at -projection */
	M[1][1] = nose[1];
	M[1][2] = nose[2];
	M[2][0] = norm[0]; /* Z axis points at -down. */
	M[2][1] = norm[1];
	M[2][2] = norm[2];
    }

    /*
     *	======================================================
     * 	Apply matrix on each vertex
     *	======================================================
     */

    for (i = 0; i < _nofVertices ; i++)
    {
	/*
	 *	============================================
	 * 	Transform to get ASD alignment.
	 *	Make sure _v and _v_aligned can be the same
	 *	memory. 
	 *	============================================
	 */
	x = _v[i][0];
	y = _v[i][1];
	z = _v[i][2];
	_v_aligned[i][0] = x * M[0][0] + y * M[1][0] + z * M[2][0] + M[3][0];
	_v_aligned[i][1] = x * M[0][1] + y * M[1][1] + z * M[2][1] + M[3][1];
	_v_aligned[i][2] = x * M[0][2] + y * M[1][2] + z * M[2][2] + M[3][2];
    }

}

/*============================================================================*/
PFDUDLLEXPORT
void pfdProjectVerticesOnASD (
			pfASD		*asd,
			pfVec3 		*_v,
			int		_nofVertices,
			float 		*_projection,
			float 		*_down,
			pfVec3		*_v_projected)
/*============================================================================*/
{
    int				i;
    float			fv[3], pv[3];
    pfVec3 			norm, azim;
    pfVec3 			right_wing, nose;
    float			x, y, z;
    float			down[3];
    int				hit_floor;
    float			proj[3];

    if (_down)
	normalizeVector (_down, down);
    else
    {
	down[0] = 0.0;
	down[1] = 0.0;
	down[2] = -1.0;
    }

    for (i = 0 ; i < _nofVertices ; i++)
    {
	/*
	 *	============================================
	 * 	Project down onto pfASD. If we are under
	 *	the surface - clamp to surface.
	 *	============================================
	 */
	fv[0] = _v[i][0];
	fv[1] = _v[i][1];
	fv[2] = _v[i][2];

	pfASDProjectPointFinestPosition (asd, fv, down, 0, proj);

	if (isVectorInFront (proj, fv, down))
	{
	    /*
	     *	 Vertex is underground. Lift it up.
	     */
	    fv[0] = proj[0];
	    fv[1] = proj[1];
	    fv[2] = proj[2];
	    hit_floor = 1;
	}
	else
	    hit_floor = 0;

	/*
	 *	============================================
	 * 	Project onto pfASD at projection direction.
	 *	============================================
	 */
	if (hit_floor)
	{
	    _v_projected[i][0] = fv[0];
	    _v_projected[i][1] = fv[1];
	    _v_projected[i][2] = fv[2];
	}
	else
	{
	    pfASDProjectPointFinestPosition (asd, fv, _projection,
					PR_QUERY_DIRECTIONAL, pv);
	    _v_projected[i][0] = pv[0];
	    _v_projected[i][1] = pv[1];
	    _v_projected[i][2] = pv[2];
	}
    }

}

/*===========================================================================*/
static int normalizeVector (float *from, float *to)
/*===========================================================================*/
{
    float	mag_2, mag;

    mag_2 = from[0] * from[0] +
	    from[1] * from[1] +
	    from[2] * from[2];

    if (mag_2 < 0.01)
    {
	to[0] = 0.0;
	to[1] = 0.0;
	to[2] = 1.0;
	return (0);
    }
    else
    {
	if (fabs (mag_2 - 1.0) > 0.001)
	{
	    mag = 1.0 / sqrtf(mag_2);
	    to[0] = from[0] * mag;
	    to[1] = from[1] * mag;
	    to[2] = from[2] * mag;
	}
	else
	{
	    to[0] = from[0];
	    to[1] = from[1];
	    to[2] = from[2];
	}
	return (1);
    }
}

/*==========================================================================*/
static int isVectorInFront (float *v0, float *v1, float *d)
/*==========================================================================*/
{
    float       fa, fb, fc;
    int         use;
    float       p0, p1, dp;

    /* 
     *	Pick largest direction component.
     */

    fa = fabs (d[0]);
    fb = fabs (d[1]);
    fc = fabs (d[2]);

    if (fa > fb)
    {
        if (fa > fc)
            use = 0;
        else
            use = 2;
    }
    else
    {
        if (fb > fc)
            use = 1;
        else
            use = 2;
    }

    p0 = v0[use];
    p1 = v1[use];
    dp = d[use];

    if (p1 > p0)
    {
        if (dp > 0)
            return (1);
        else
            return (0);
    }
    else
    if (p1 < p0)
    {
        if (dp < 0)
            return (1);
        else
            return (0);
    }
    else
    {
        /*
	 *	 Points are the same - result is arbitrary.
	 */
        return (1);
    }
}

