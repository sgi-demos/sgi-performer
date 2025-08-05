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
 *  The functions included in this file demonstrate the use of a
 *  user-defined callback function for doing terrain level-of-detail
 *  evaluation in the context of asdfly (as opposed to the distance
 *  and distance-squared pre-defined ASD functions).  This particular
 *  evaluation implementation attempts to place an upper bound on
 *  the screen-space error of the rendered terrain geometry.  The
 *  maximum distance between a vertex being considered and the
 *  next coarser level of detail is projected to screen space, and
 *  considered against a "pixel accuracy threshold" value, which
 *  can be varied with a slider while running asdfly.
 *  
 *  Due to the top-down nature of pfASD level of detail consideration,
 *  edges will not be considered for refvert unless all their "parent
 *  edges" have split.  This policy creates a situation in which
 *  vertices with large errors (as compared to highest resolution mesh)
 *  may not necessarily even be considered, if their ancestors do not
 *  have similarly large errors.  To combat this and attempt to satisfy
 *  a bounded error metric, this implementation considers the error
 *  value (the "delta" value in the code below) of a vertex to be
 *  the maximum error value of itself and all its children vertices.
 *  Similarly, at evaluation time, the eyepoint to vertex distance used
 *  in the projection equation is considered as the distance from the
 *  eyepoint to the closest point on the bounding box surrounding all
 *  the children of the vertex being considered.
 *  
 *  The resulting effect of this evaluation policy may thus include a
 *  significant amount of "popping", as a vertex deep in the mesh
 *  hierarchy may suddenly trigger the evaluation and inclusion of all of
 *  its ancestor vertices.  This could be minimized with a more
 *  sophisticated morphing weight calculation.
 *  
 *  Although the algorithm generally satisfies the goal of meeting
 *  an explicit projected pixel error metric, a couple of exceptional
 *  cases may occur.  For example, the error value for a vertex is
 *  the difference between the vertex's position and its parent edge,
 *  rather than the difference between the vertex and the highest
 *  resolution terrain representation.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <assert.h>

#include "perfly.h"


/*
 *  This is the actual vertex evalation callback function, called
 *  once for each vertex/edge being considered for inclusion in
 *  the active surface.  The distance to the nearest point on the
 *  vertex's bounding box (see above) is computed, and along with
 *  the precomputed error value (delta) at the vertex, the maximum
 *  projected error is computed and compared against the error
 *  threshold value.
 */

float lodEvalFunc(pfASD *asd, pfVec3 eyept, int faceid, int refvertid)
{
    float dist, prod;
    evalData_t *dat = &(ViewState->evalData[refvertid]);
    pfVec3 nearpt;

    nearpt[PF_X] = PF_CLAMP(ViewState->evalEye[PF_X], dat->boxmin[PF_X],
                            dat->boxmax[PF_X]);
    nearpt[PF_Y] = PF_CLAMP(ViewState->evalEye[PF_Y], dat->boxmin[PF_Y],
                            dat->boxmax[PF_Y]);
    nearpt[PF_Z] = PF_CLAMP(ViewState->evalEye[PF_Z], dat->boxmin[PF_Z],
                            dat->boxmax[PF_Z]);

    dist = PFDISTANCE_PT3(eyept, nearpt);

    prod = ViewState->evalKappa * dist;

    if (dat->delta > prod)
        return(0.0f);
    else if (dat->delta < 0.75 * prod)   /* Vertices begin to morph at */
        return(1.0f);                    /* 75% of the pixel thresold. */
    else
        return((prod - dat->delta) / (prod - 0.75*prod));
}


/*
 *  This function should be called before each pass of level
 *  of detail evaluation for the asd.  It sets the common
 *  values which are used in the projected pixel error comparison
 *  for each vertex.
 */

void lodEvalFunc_pass(void)
{
    pfCopyVec3(ViewState->evalEye, ViewState->viewCoord.xyz);

    ViewState->evalKappa = (logf(ViewState->switchin) / M_LN2) *
        pfTan(ViewState->fov[0] / 2.0f) / (ViewState->winXSize / 2.0f);
}


/*
 *  This function should be called before the callback function is
 *  ever used.  This function initializes the data structure used
 *  for the per-vertex comparisons, including computing the
 *  pixel errors and bounding boxes for each vertex, and combining
 *  them hierarchically to maintain correct "maximum" values.
 */

void lodEvalFunc_init(pfASD *asd)
{
    int numverts, numfaces;
    pfASDVert *v0;
    pfASDFace *faces;
    int maxfacelevel;
    int i, f, lev, sf, ssp;
    int refvertpt, subface, spt;
    evalData_t *t1, *t2;          /* parent and child vertex pointers */
    evalData_t *evalData;
    int bind;

    if(asd == NULL) return;
    pfGetASDAttr(asd, PFASD_COORDS, &bind, &numverts, (void**) &v0);
    pfGetASDAttr(asd, PFASD_FACES, &bind, &numfaces, (void**) &faces);

    ViewState->evalData = pfMalloc(sizeof(evalData_t) * numverts,
				   pfGetSharedArena());

    evalData = ViewState->evalData;

    for (i=0; i<numverts; ++i)
    {
	evalData[i].delta = PFLENGTH_VEC3(v0[i].vd);
	PFCOPY_VEC3(evalData[i].boxmin, v0[i].v0);
	PFCOPY_VEC3(evalData[i].boxmax, v0[i].v0);
    }

    maxfacelevel = faces[0].level;

    for (i=1; i<numfaces; ++i)
	if (faces[i].level > maxfacelevel)
	    maxfacelevel = faces[i].level;

    /*
     *  This mess of nesting "traverses" the hierarchy of refvertpoints,
     *  and assigns parents the maximum/minimum values of thier children.
     */

    for (lev=maxfacelevel-2; lev>=0; --lev)
	for (f=0; f<numfaces; ++f)
            if (faces[f].level == lev)
            {
                for (i=0; i<3; ++i)
                {
		    refvertpt = faces[f].refvert[i];
                    if (refvertpt != PFASD_NIL_ID)
			for (sf=0; sf<4; ++sf)
			{
			    subface = faces[f].child[sf];
			    if (subface != PFASD_NIL_ID)
			    {
				if (faces[subface].vert[0] == refvertpt ||
				    faces[subface].vert[1] == refvertpt ||
				    faces[subface].vert[2] == refvertpt)
                                {
				    for (ssp=0; ssp<3; ++ssp)
				    {
                                        spt = faces[subface].refvert[ssp];
					if (spt != PFASD_NIL_ID)
					{
					    t1 = &(evalData[refvertpt]);
					    t2 = &(evalData[spt]);

					    t1->delta = PF_MAX2(t1->delta,
							        t2->delta);

					    t1->boxmax[PF_X] = PF_MAX2(
						  t1->boxmax[PF_X],
						  t2->boxmax[PF_X]);

					    t1->boxmax[PF_Y] = PF_MAX2(
						  t1->boxmax[PF_Y],
						  t2->boxmax[PF_Y]);

					    t1->boxmax[PF_Z] = PF_MAX2(
						  t1->boxmax[PF_Z],
						  t2->boxmax[PF_Z]);

					    t1->boxmin[PF_X] = PF_MIN2(
						  t1->boxmin[PF_X],
						  t2->boxmin[PF_X]);

					    t1->boxmin[PF_Y] = PF_MIN2(
						  t1->boxmin[PF_Y],
						  t2->boxmin[PF_Y]);

					    t1->boxmin[PF_Z] = PF_MIN2(
						  t1->boxmin[PF_Z],
						  t2->boxmin[PF_Z]);
					}
				    }
				}
			    }
			}
                }
            }
}

