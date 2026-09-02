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
 * starfield.c: Performer program given as an example for light points
 *        and lpoint process with a dynamic star field simulation.
 *        Based on simple.c from a source code given by Eric Heft
 *
 *
 * $Revision: 1.5 $ $Date: 2000/10/06 21:26:38 $
 *
 */


#include <stdlib.h>
#include <math.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

/*------------------------------------------------------------------*/


pfVec3      	*norms, *coords;  /* moved Global so we can move them */
int     foundLPB = 0;

/* Initialise Calligraphic boards */
int
StartCalligraphic(int _numpipes)
{
int     i;
int found=0;

    /* assume pipe starts a 0 to */
    for (i=0; i< _numpipes; i++)
        if (pfCalligInitBoard(i) == TRUE)
        {
            found ++;
            /* get the memory size */
            pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
                "StartCalligraphic: board(%d) has %d Bytes of memory",
                i, pfGetCalligBoardMemSize(i));
        }
    return found;
}


static pfGeode*
initLPoints(int NPOINTS)
{
    pfLPointState	*lps;
    pfTexGen            *tgen;
    pfTexture           *tex;
    pfGeoState          *gst;
    pfGeode             *gd;
    pfGeoSet            *gs;
    pfVec4		*colors;
    float       	phi, dphi, theta, dtheta;
    int                 i, j, k;
    void		*arena = pfGetSharedArena();

    /*------------------ Set up pfLPointState -------------------*/

    lps = pfNewLPState(arena);

    /* Enable perspective size computation */
    pfLPStateMode(lps, PFLPS_SIZE_MODE, PF_ON);

    /* Clamp point size between .25 and 4 pixels */
    pfLPStateVal(lps, PFLPS_SIZE_MIN_PIXEL, 0.25f);
    pfLPStateVal(lps, PFLPS_SIZE_MAX_PIXEL, 4.0f);

    /* Real-world point size is .15 meters */
    pfLPStateVal(lps, PFLPS_SIZE_ACTUAL, .45f);

    /* Fade points smaller than 2 pixels */
    pfLPStateVal(lps, PFLPS_TRANSP_PIXEL_SIZE, 2.0f);

    /* Linear fade, scaled by .6 and alpha clamped at .1 */
    pfLPStateVal(lps, PFLPS_TRANSP_EXPONENT, 1.0f);
    pfLPStateVal(lps, PFLPS_TRANSP_SCALE, .6f);
    pfLPStateVal(lps, PFLPS_TRANSP_CLAMP, .1f);

    pfLPStateMode ( lps, PFLPS_DRAW_MODE,    PFLPS_DRAW_MODE_CALLIGRAPHIC);
    pfLPStateMode ( lps, PFLPS_QUALITY_MODE, PFLPS_QUALITY_MODE_HIGH);

    pfLPStateVal ( lps, PFLPS_SIGNIFICANCE, 0.5);
    pfLPStateVal ( lps, PFLPS_MIN_DEFOCUS, 0.1);
    pfLPStateVal ( lps, PFLPS_MAX_DEFOCUS, 0.9);


    /* Points are fogged as if 4 times closer than they really are */
    pfLPStateVal(lps, PFLPS_FOG_SCALE, .25f);

    /* Compute true, slant range from eye to points */
    pfLPStateMode(lps, PFLPS_RANGE_MODE, PFLPS_RANGE_MODE_TRUE);

    /* Points are bidirectional with  purple back color */
    pfLPStateMode(lps, PFLPS_SHAPE_MODE, PFLPS_SHAPE_MODE_UNI);
    pfLPStateBackColor(lps, 1.f, 0.0f, 1.f, 1.0f);

    /* 
     * Point shape is 60 horiz and 90 degrees vertical with 
     * no roll, falloff of 1 and ambient intensity of .1 
    */ 
    pfLPStateShape(lps, 60.0f, 90.0f, 0.0f, 1, .9f);

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

    for (i=0; i<NPOINTS; i++)
    {
	pfSetVec3(norms[i],0,-1,0);
        pfSetVec3(coords[i],rand()%100-50,rand()%1000,rand()%100-50);
	pfSetVec4(colors[i],1,1,1,1);
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
    int		SPEED,NPOINTS,DEBUG;
    char	name[80];

    NPOINTS = 0;
    
    if (argc > 1)
    {
	NPOINTS = atoi(argv[1]);
	argv++; argc--;
    }

    SPEED = 0;
    if (argc > 1)
    {
	SPEED = atoi(argv[1]);
	argv++; argc--;
    }

    DEBUG = 0;
    if (argc > 1)
    {
	DEBUG = atoi(argv[1]);
	argv++; argc--;
    }
    
    if (!SPEED) SPEED = 5;
    if (!NPOINTS) NPOINTS = 100;

    
    /* Initialize Performer */
    pfInit();	

    /* Use default multiprocessing mode based on number of
     * processors.
     */
#ifndef __linux__
    pfMultiprocess(PFMP_FORK_LPOINT);
#else
    pfMultiprocess(PFMP_DEFAULT);
#endif
    
    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    foundLPB = StartCalligraphic(1);
    if (foundLPB)
    {
	printf("Found a calligraphic board \n");
    }
    pfConfig();			

    /* Create and attach morph to a pfScene. */
    scene = pfNewScene();
    dcs = pfNewDCS();
    pfAddChild(dcs, initLPoints(NPOINTS));
    pfAddChild(scene, dcs);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene, &bsphere);

    /* Configure and open GL window */
    pfPhase(PFPHASE_LOCK);
    
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    sprintf(name,"OpenGL Performer Star Fields : %d stars at %d",NPOINTS,SPEED);
    pfPWinName(pw,name);
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    pfOpenPWin(pw);
    pfConfigPWin(pw);

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 1000);
    pfChanFOV(chan, 45.0f, 0.0f);
    if (foundLPB)
	pfChanCalligEnable(chan,1);

    pfSetVec3(view.xyz, 0.0f, 0.0f, 0.0f); 
    pfSetVec3(view.hpr, 0.0f, 0.0f, 0.0f); 
    pfChanView(chan, view.xyz, view.hpr);

    /* Simulate for ever ! */
    while(1)
    {
	int i;

	/* Spin lightpoints */
	t = pfGetTime();

	i=NPOINTS;
	while (i)
	{
	    i--;
	    if (coords[i][1] < 0)
	    {
		pfSetVec3(coords[i],rand()%200-100,1000,rand()%200-100);
	    }
	    else	
		coords[i][1] -= SPEED;
	}

        if (DEBUG)  pfDrawChanStats(chan);  

	/* Initiate cull/draw for this frame. */
	pfFrame();		
    }

    /* Terminate parallel processes and exit. */
    pfExit();

    return 0;
}




