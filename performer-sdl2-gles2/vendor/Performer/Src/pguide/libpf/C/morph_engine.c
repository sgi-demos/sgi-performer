/*
 * Copyright 1997, Silicon Graphics, Inc.
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
 * morph_engine.c: Performer program to demonstrate use of morph pfEngine.
 *        Based on simple.c
 *
 * $Revision: 1.2 $ $Date: 2000/10/06 21:26:35 $ 
 *
 */

#include <stdlib.h>
#include <math.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>

int	nSph;
pfFlux	*coord_flux;
pfFlux	*color_flux;
pfFlux	*norm_flux;
pfFlux	*weight_flux;


/*------------------------------------------------------------------*/

static void
breatheMorph(double t)
{
    float	s = (sinf(t) + 1.0f) / 2.0f;
    float	*weights;
    int		i;
    pfVec4	*colors;

    colors = (pfVec4*)pfGetFluxWritableData(color_flux);
    for (i = 0; i < nSph; i++)
        colors[i][0] = s;
    pfFluxWriteComplete(color_flux);

    weights = (float*)pfGetFluxWritableData(weight_flux);
    weights[0] = s;
    weights[1] = 1.0f - s;
    pfFluxWriteComplete(weight_flux);
}


static pfGeode*
initMorph(void)
{
    pfGeoSet	*gset;
    pfGeode	*geode;
    pfGeoState	*gstate;
    pfMaterial	*mtl;
    pfEngine	*engine;
    ushort	*icoords, *inorms;
    pfVec3	*coords, *ncoords, *norms, *nnorms;
    pfVec4	*colors;
    int		i;
    void	*arena = pfGetSharedArena();

    geode = pfNewGeode();
    gset = pfdNewSphere(400, arena);
    gstate = pfNewGState(arena);

    mtl = pfNewMtl(arena);
    pfMtlColor(mtl, PFMTL_DIFFUSE, 1.0f, 0.0f, 0.0f); 
    pfMtlColor(mtl, PFMTL_SPECULAR, 1.0f, 1.0f, 1.0f); 
    pfMtlColorMode(mtl, PFMTL_BOTH, PFMTL_CMODE_AD);
    pfMtlShininess(mtl, 32);

    pfGStateAttr(gstate, PFSTATE_FRONTMTL, mtl);
    pfGStateMode(gstate, PFSTATE_ENLIGHTING, 1);
    pfGSetGState(gset, gstate);

    pfAddGSet(geode, gset);

    /*
     * NULL forces recomputation of bound. Force it to be static 
     * to avoid expensive recomputation. Static bound should encompass
     * the extent of all morph possibilities. 
     */
    pfGSetBBox(gset, NULL, PFBOUND_STATIC);
    pfNodeBSphere(geode, NULL, PFBOUND_STATIC);

    pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&coords, &icoords);
    pfGetGSetAttrLists(gset, PFGS_NORMAL3, (void**)&norms, &inorms);
    nSph = pfGetSize(coords) / sizeof(pfVec3);

    color_flux = pfNewFlux(nSph * sizeof(pfVec4), PFFLUX_DEFAULT_NUM_BUFFERS,
			   arena);
    coord_flux = pfNewFlux(pfGetSize(coords), PFFLUX_DEFAULT_NUM_BUFFERS,
			   arena);
    pfFluxInitData(coord_flux, coords);
    norm_flux = pfNewFlux(pfGetSize(norms), PFFLUX_DEFAULT_NUM_BUFFERS, arena);
    pfFluxInitData(norm_flux, norms);
    weight_flux = pfNewFlux(2 * sizeof(float), PFFLUX_DEFAULT_NUM_BUFFERS,
			    arena);
    pfFluxMode(weight_flux, PFFLUX_PUSH, PF_ON);

    ncoords = pfMalloc(pfGetSize(coords), arena);
    nnorms = pfMalloc(pfGetSize(norms), arena);

    colors = (pfVec4*)pfGetFluxCurData(color_flux);
    for (i=0; i<nSph; i++)
    {
	int	max;
	float	t;

	/* Find which face of the cube this vertex maps to */
	if (PF_ABS(coords[i][PF_X]) > PF_ABS(coords[i][PF_Y]))
	{
	    if (PF_ABS(coords[i][PF_X]) > PF_ABS(coords[i][PF_Z]))
		max = PF_X;
	    else
		max = PF_Z;
	}
	else 
	{
	    if (PF_ABS(coords[i][PF_Y]) > PF_ABS(coords[i][PF_Z]))
		max = PF_Y;
	    else
		max = PF_Z;
	}

	pfSetVec4(colors[i], 0.0f, 0.0f, 0.0f, 1.0f);
	colors[i][max] = 1.0f;

	/* Compute cube normals and coordinates */
	pfSetVec3(nnorms[i], 0.0f, 0.0f, 0.0f); 
	if (coords[i][max] < 0.0f)
	{
	    t = -1.0f / coords[i][max];
	    pfScaleVec3(ncoords[i], t, coords[i]);
	    nnorms[i][max] = -1.0f;
	}
	else
	{
	    t = 1.0f / coords[i][max];
	    pfScaleVec3(ncoords[i], t, coords[i]);
	    nnorms[i][max] = 1.0f;
	}
    }
    /* Set all pfFlux buffers to the same colors */
    pfFluxInitData(color_flux, colors);

    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, (void*)color_flux, NULL);

    engine = pfNewEngine(PFENG_MORPH, arena);
    pfEngineSrc(engine, PFENG_MORPH_WEIGHTS, weight_flux, NULL, 0, 0, 1);
    pfEngineSrc(engine, PFENG_MORPH_SRC(0), coords, NULL, 0, 0, 3);
    pfEngineSrc(engine, PFENG_MORPH_SRC(1), ncoords, NULL, 0, 0, 3);
    pfEngineDst(engine, coord_flux, NULL, 0, 3);
    pfEngineIterations(engine, nSph, 3);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coord_flux, icoords);

    engine = pfNewEngine(PFENG_MORPH, arena);
    pfEngineSrc(engine, PFENG_MORPH_WEIGHTS, weight_flux, NULL, 0, 0, 1);
    pfEngineSrc(engine, PFENG_MORPH_SRC(0), norms, NULL, 0, 0, 3);
    pfEngineSrc(engine, PFENG_MORPH_SRC(1), nnorms, NULL, 0, 0, 3);
    pfEngineDst(engine, norm_flux, NULL, 0, 3);
    pfEngineIterations(engine, nSph, 3);
    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, norm_flux, inorms);

    return geode;
}


void 
OpenPipeWin (pfPipeWindow *pw)
{
    pfOpenPWin(pw);
    pfApplyLModel(pfNewLModel(NULL));
}

void 
DrawChannel (pfChannel *chan, void *data)
{
    pfVec4	clr;

    pfSetVec4(clr, .2f, .2f, .2f, 1.0f); 
    pfClear(PFCL_COLOR|PFCL_DEPTH, clr);
    pfDraw();
}

int
main (int argc, char *argv[])
{
    double     t = 0.;
    pfScene     *scene;
    pfDCS	*morphDCS;
    pfGeode	*morph;
    pfPipe      *p;
    pfPipeWindow *pw;
    pfChannel   *chan;
    pfSphere 	bsphere;
    pfCoord	view;
    pfLightSource	*ls;

    /* Initialize Performer */
    pfInit();	

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess(PFMP_DEFAULT);

    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    pfConfig();			

    /* Create and attach morph to a pfScene. */
    scene = pfNewScene();
    morphDCS = pfNewDCS();
    morph = initMorph();
    pfAddChild(morphDCS, morph);
    pfAddChild(scene, morphDCS);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene, &bsphere);

    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinName(pw, argv[0]);
    pfPWinConfigFunc(pw, OpenPipeWin);	
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    pfConfigPWin(pw);	

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere.radius);
    pfChanFOV(chan, 45.0f, 0.0f);
    pfCopyVec3(view.xyz, bsphere.center);
    view.xyz[PF_Y] -= 3.0f * bsphere.radius;
    pfSetVec3(view.hpr, 0.0f, 0.0f, 0.0f); 
    pfChanView(chan, view.xyz, view.hpr);
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);

    /* Create a pfLightSource and attach it to scene. */
    ls = pfNewLSource();
    pfLSourcePos(ls, view.xyz[0], view.xyz[1], view.xyz[2], 1.0f); 
    pfAddChild(scene, ls);

    /* Simulate for twenty seconds. */
    while (t < 20.0f)
    {
	pfMatrix	mat;

	/* Spin morph object */
	t = pfGetTime();
	pfMakeRotMat(mat, t*57.0f, 1.0f, -0.3f, 1.0f);
	pfDCSMat(morphDCS, mat);

	/* Update morph */
	breatheMorph(t);

	/* Initiate cull/draw for this frame. */
	pfFrame();		
    }

    /* Terminate parallel processes and exit. */
    pfExit();

    return 0;
}

