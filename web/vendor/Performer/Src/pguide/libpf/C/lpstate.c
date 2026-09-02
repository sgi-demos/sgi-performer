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
 * lpstate.c: Performer program to demonstrate use of pfLPointState
 *        Based on simple.c
 *
 * $Revision: 1.9 $ $Date: 2000/10/06 21:26:34 $ 
 *
 */

#include <stdlib.h>
#include <math.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

/*------------------------------------------------------------------*/

#define NPOINTS2        50
#define NPOINTS         (NPOINTS2 * (NPOINTS2 - 2))

static pfGeode*
initLPoints(void)
{
    pfLPointState	*lps;
    pfTexGen            *tgen;
    pfTexture           *tex;
    pfGeoState          *gst;
    pfGeode             *gd;
    pfGeoSet            *gs;
    pfVec4		*colors;
    pfVec3      	*norms, *coords;
    pfMatrix		squash, squashInvTransp;
    float       	phi, dphi, theta, dtheta;
    int                 i, j, k;
    void		*arena = pfGetSharedArena();

    /*------------------ Set up pfLPointState -------------------*/

    lps = pfNewLPState(arena);

    /* Enable perspective size computation */
    pfLPStateMode(lps, PFLPS_SIZE_MODE, PF_ON);

    /* Clamp point size between .25 and 4 pixels */
    pfLPStateVal(lps, PFLPS_SIZE_MIN_PIXEL, .25f);
    pfLPStateVal(lps, PFLPS_SIZE_MAX_PIXEL, 4.0f);

    /* Real-world point size is .15 meters */
    pfLPStateVal(lps, PFLPS_SIZE_ACTUAL, .15f);

    /* Fade points smaller than 2 pixels */
    pfLPStateVal(lps, PFLPS_TRANSP_PIXEL_SIZE, 2.0f);

    /* Linear fade, scaled by .6 and alpha clamped at .1 */
    pfLPStateVal(lps, PFLPS_TRANSP_EXPONENT, 1.0f);
    pfLPStateVal(lps, PFLPS_TRANSP_SCALE, .6f);
    pfLPStateVal(lps, PFLPS_TRANSP_CLAMP, .1f);

    /* Points are fogged as if 4 times closer than they really are */
    pfLPStateVal(lps, PFLPS_FOG_SCALE, .25f);

    /* Compute true, slant range from eye to points */
    pfLPStateMode(lps, PFLPS_RANGE_MODE, PFLPS_RANGE_MODE_TRUE);

    /* Points are bidirectional with  purple back color */
    pfLPStateMode(lps, PFLPS_SHAPE_MODE, PFLPS_SHAPE_MODE_BI_COLOR);
    pfLPStateBackColor(lps, 1.f, 0.0f, 1.f, 1.0f);

    /* 
     * Point shape is 60 horiz and 90 degrees vertical with 
     * no roll, falloff of 1 and ambient intensity of .1 
    */ 
    pfLPStateShape(lps, 60.0f, 90.0f, 0.0f, 1, .1f);

    /*------------------ Set up pfGeoState -------------------*/

    gst = pfNewGState(arena);

    /* Specify high-quality transparency */
    pfGStateMode(gst, PFSTATE_TRANSPARENCY, 
		 PFTR_BLEND_ALPHA | PFTR_NO_OCCLUDE);
    pfGStateVal(gst, PFSTATE_ALPHAREF, 0.0f);
    pfGStateMode(gst, PFSTATE_ALPHAFUNC, PFAF_GREATER);
    pfGStateMode(gst, PFSTATE_ANTIALIAS, PFAA_ON);
    
    pfGStateMode(gst, PFSTATE_ENFOG, 0);
    pfGStateMode(gst, PFSTATE_ENLIGHTING, 0);
    pfGStateMode(gst, PFSTATE_ENTEXTURE, 1);
    pfGStateMode(gst, PFSTATE_ENLPOINTSTATE, 1);
    pfGStateAttr(gst, PFSTATE_LPOINTSTATE, lps);

    /*------------------ Configure texturing -------------------*/

    tgen = pfNewTGen(arena);
    tex = pfNewTex(arena);

/*#define USE_TEXTURE_MAPPING_FOR_DIRECTIONALITY*/
#ifdef USE_TEXTURE_MAPPING_FOR_DIRECTIONALITY
    /* 
     * Use texture mapping for directionality. CPU computes size
     * and range and fog attenuation 
    */
    pfLPStateMode(lps, PFLPS_DIR_MODE, PFLPS_DIR_MODE_TEX);
    pfLPStateMode(lps, PFLPS_TRANSP_MODE, PFLPS_TRANSP_MODE_ALPHA);
    pfLPStateMode(lps, PFLPS_FOG_MODE, PFLPS_FOG_MODE_ALPHA);
    pfuMakeLPStateShapeTex(lps, tex, 256);
    pfGStateAttr(gst, PFSTATE_TEXTURE, tex);
    pfTGenMode(tgen, PF_S, PFTG_SPHERE_MAP);
    pfTGenMode(tgen, PF_T, PFTG_SPHERE_MAP);
#else 
    /* 
     * Use texture mapping for range and fog attenuation. CPU 
     * computes size and directionality.
    */
    pfLPStateMode(lps, PFLPS_DIR_MODE, PFLPS_DIR_MODE_ALPHA);
    pfLPStateMode(lps, PFLPS_TRANSP_MODE, PFLPS_TRANSP_MODE_TEX);
    pfLPStateMode(lps, PFLPS_FOG_MODE, PFLPS_FOG_MODE_TEX);
    pfuMakeLPStateRangeTex(lps, tex, 256, pfNewFog(NULL));
    pfGStateAttr(gst, PFSTATE_TEXTURE, tex);
    pfTGenPlane(tgen, PF_S, 0.0f, 0.0f, 1.0f, 0.0f);
    pfTGenPlane(tgen, PF_T, 0.0f, 0.0f, 1.0f, 0.0f);
    pfTGenMode(tgen, PF_S, PFTG_EYE_LINEAR_IDENT);
    pfTGenMode(tgen, PF_T, PFTG_EYE_LINEAR_IDENT);
#endif
    pfGStateAttr(gst, PFSTATE_TEXGEN, tgen);
    pfGStateMode(gst, PFSTATE_ENTEXGEN, 1);

    /* Make PFGS_POINTS pfGeoSet arranged in a sphere */

    gd = pfNewGeode();
    gs = pfNewGSet(arena);
    pfGSetPrimType(gs, PFGS_POINTS);
    pfGSetNumPrims(gs, NPOINTS);

    colors = pfMalloc(sizeof(pfVec4) * NPOINTS, arena);
    coords = pfMalloc(sizeof(pfVec3) * NPOINTS, arena);
    norms = pfMalloc(sizeof(pfVec3) * NPOINTS, arena);
    pfGSetAttr(gs, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
    pfGSetAttr(gs, PFGS_COLOR4, PFGS_PER_VERTEX, colors, NULL);
    pfGSetAttr(gs, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);

    pfGSetGState(gs, gst);
    pfAddGSet(gd, gs);

    /* Squash sphere into an ellipse so perspective point size 
	is more easily seen
    */
    pfMakeRotMat(squash, 90.0f, 1.0f, 0.0f, 0.0f);
    pfPostScaleMat(squash, squash, 1.0f, 2.0f, .5f);
    pfInvertAffMat(squashInvTransp, squash);
    pfTransposeMat(squashInvTransp, squashInvTransp);

    dphi = 180.0f / (NPOINTS2-1);
    dtheta = 360.0f / NPOINTS2;

    phi = dphi;
    for (k=0, i=0; i<NPOINTS2 - 2; i++)
    {
	float	ct, st, sp, cp;

        theta = 0.0f;
        pfSinCos(phi, &sp, &cp);

        for (j=0; j<NPOINTS2; j++, k++)
        {
	    pfSetVec4(colors[k], 1.0f,  1.0f,  1.0f, 1.0f);  

            pfSinCos(theta, &st, &ct);
            pfSetVec3(norms[k], ct * sp, st * sp, cp);
            pfScaleVec3(coords[k], 10.0f, norms[k]);

	    pfXformPt3(coords[k], coords[k], squash);
	    pfXformVec3(norms[k], norms[k], squashInvTransp);
	    pfNormalizeVec3(norms[k]);

            theta += dtheta;
        }

        phi += dphi;
    }
    return gd;
}


int
main (int argc, char *argv[])
{
    double     t = 0.;
    pfScene     *scene;
    pfDCS	*dcs;
    pfPipe      *p;
    pfPipeWindow *pw;
    pfChannel   *chan;
    pfSphere 	bsphere;
    pfCoord	view;


    /* Initialize Performer */
    pfInit();	

    /* Use default multiprocessing mode based on number of
     * processors and add the lpoint process to it.
     */

    pfMultiprocess(PFMP_DEFAULT | PFMP_FORK_LPOINT);

    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    pfConfig();			

    /* Create and attach morph to a pfScene. */
    scene = pfNewScene();
    dcs = pfNewDCS();
    pfAddChild(dcs, initLPoints());
    pfAddChild(scene, dcs);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene, &bsphere);

    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinName(pw, argv[0]);
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    pfOpenPWin(pw);
    pfConfigPWin(pw);

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere.radius);
    pfChanFOV(chan, 45.0f, 0.0f);
    pfCopyVec3(view.xyz, bsphere.center);
    view.xyz[PF_Y] -= 2.0f * bsphere.radius;
    pfSetVec3(view.hpr, 0.0f, 0.0f, 0.0f); 
    pfChanView(chan, view.xyz, view.hpr);

    /* Simulate for twenty seconds. */
    while (t < 20.0f)
    {
	pfMatrix	mat;

	/* Spin lightpoints */
	t = pfGetTime();
	pfMakeRotMat(mat, t*17.0f, 0.0f, 0.0f, 1.0f);
	pfDCSMat(dcs, mat);

	/* Initiate cull/draw for this frame. */
	pfFrame();		
    }

    /* Terminate parallel processes and exit. */
    pfExit();

    return 0;
}

