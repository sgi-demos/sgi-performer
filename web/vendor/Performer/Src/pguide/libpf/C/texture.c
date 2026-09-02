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
 * texture.c: simple Performer program to demonstrate textured poly's.
 *
 * $Revision: 1.30 $ $Date: 2000/10/06 21:26:39 $ 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <Performer/pf.h>

pfVec2          texcoords[]={   {0.0f, 0.0f},
                                {1.0f, 0.0f},
                                {1.0f, 1.0f},
                                {0.0f, 1.0f} };

ushort          texlist[] =     { 0, 1, 2, 3 };

pfVec3          coords[] ={     {-1.0f,  0.0f,  -1.0f },
                                { 1.0f,  0.0f,  -1.0f },
                                { 1.0f,  0.0f,   1.0f },
                                {-1.0f,  0.0f,   1.0f } };

ushort          vertexlist[] = { 0, 1, 2, 3 };

pfVec4          colors[] ={     {1.0f, 1.0f, 1.0f, 1.0f},
                                {0.0f, 0.0f, 1.0f, 1.0f},
                                {1.0f, 0.0f, 0.0f, 1.0f},
                                {0.0f, 1.0f, 0.0f, 1.0f} };

ushort          colorlist[] =   { 0, 1, 2, 3 };

/*
 *	Usage() -- print usage advice and exit. This
 *      procedure is executed in the application process.
 */
static void
Usage (void)
{
    fprintf(stderr, "Usage: texture texfile ...\n");
    exit(1);
}


int
main (int argc, char *argv[])
{
    float       t = 0.0f;
    pfScene     *scene;
    pfPipe      *p;
    pfPipeWindow *pw;
    pfChannel   *chan;
    pfSphere	bsphere;
    pfGroup	*root;
    pfGeoSet	*gset;
    pfGeode     *geode1,*geode2;
    pfGeoState  *gstate;
    pfTexture   *tex;
    pfTexEnv    *tev;
    pfDCS       *dcs1,*dcs2;
    pfMatrix	*mat;
    int		numTextures = 0;
    void	*arena;

    if (argc<2) Usage();

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

    /* Append to PFPATH files in /usr/share/Performer/data */
    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures/");

    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinName(pw, "OpenGL Performer");
    pfPWinType(pw, PFPWIN_TYPE_X);
    pfPWinOriginSize(pw, 0, 0, 300, 300);
    /* Open and configure the GL window. */
    pfOpenPWin(pw);
    pfFrame();

    /* Set up textures & gstates structures */
    arena = pfGetSharedArena();
    tex = pfNewTex (arena); 
    mat = (pfMatrix *) pfMalloc(sizeof(pfMatrix), arena);
    pfMakeScaleMat(*mat,4,4,4);
    pfTexFilter(tex, PFTEX_MINFILTER, PFTEX_BILINEAR);

    gstate = pfNewGState (arena);
    if (pfLoadTexFile (tex, argv[1]))
    {
	uint *i;
	int nc, sx, sy, sz;
	pfGetTexImage(tex, &i, &nc, &sx, &sy, &sz);
	/* if have alpha channel, enable transparency */
	if (nc != 3)
	    pfGStateMode (gstate, PFSTATE_TRANSPARENCY, PFTR_HIGH_QUALITY);
	/* set alpha function to block pixels of 0 alpha for 
	   transparent textures */
	pfGStateMode (gstate, PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
	pfGStateVal (gstate, PFSTATE_ALPHAREF, 0.0f);
	pfGStateAttr (gstate, PFSTATE_TEXTURE, tex);
	pfGStateAttr (gstate, PFSTATE_TEXMAT, mat);
	pfGStateMode (gstate, PFSTATE_ENTEXTURE, 1);
	pfGStateMode (gstate, PFSTATE_ENTEXMAT, 1);
	pfGStateMode (gstate, PFSTATE_ENLIGHTING, 0);
	pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
	tev = pfNewTEnv (arena);
	/*
	pfTEnvMode(tev, PFTE_BLEND);
	pfTEnvBlendColor(tev, 0.0f, 0.0f, 0.0f, 1.0f);
	*/
	pfGStateAttr (gstate, PFSTATE_TEXENV, tev);
	
	numTextures++;
    }
    /* Set up geosets */
    gset = pfNewGSet(arena);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, vertexlist);
    pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, texlist);
    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, colorlist);
    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 1);
    pfGSetGState (gset, gstate);
    /* set up scene graph */
    geode1 = pfNewGeode(); 
    pfAddGSet(geode1, gset);
    
    if (argc > 2)
    {
	tex = pfNewTex (arena); 
	if (pfLoadTexFile (tex, argv[2]))
	{
	    uint *i;
	    int nc, sx, sy, sz;
	    gstate = pfNewGState (arena);
	    pfGetTexImage(tex, &i, &nc, &sx, &sy, &sz);
	    /* if have alpha channel, enable transparency */
	    if (nc != 3)
		pfGStateMode (gstate, PFSTATE_TRANSPARENCY, PFTR_FAST);
	    /* set alpha function to block pixels of 0 alpha for 
	       transparent textures */
	    pfGStateMode (gstate, PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
	    pfGStateVal (gstate, PFSTATE_ALPHAREF, 0.0f);	
	    pfGStateAttr (gstate, PFSTATE_TEXTURE, tex);
	    tev = pfNewTEnv (arena);
	    pfGStateAttr (gstate, PFSTATE_TEXENV, tev);
	    pfGStateMode (gstate, PFSTATE_ENTEXTURE, 1);
	    pfGStateMode (gstate, PFSTATE_ENLIGHTING, 0);
	    pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
	    numTextures++;
	}
    }
    gset = pfNewGSet(arena);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, vertexlist);
    pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, texlist);
    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, colorlist);
    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 1);
    pfGSetGState (gset, gstate);
    geode2 = pfNewGeode(); 
    pfAddGSet(geode2, gset);

    dcs1 = pfNewDCS();
    pfDCSTrans (dcs1, -2.0f, 0.1f, 0.5f);
    pfAddChild(dcs1, geode1);

    dcs2 = pfNewDCS();
    pfAddChild(dcs2, geode2);
    pfDCSTrans (dcs2, 2.0f, 0.1f, 0.5f);

    root = pfNewGroup();
    pfAddChild(root, dcs1);        /* first child is base */
    pfAddChild(root, dcs2);        /* subsequent children are offset-layered */

    scene = pfNewScene();
    pfAddChild(scene, root);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene, &bsphere);

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere.radius);
    /* 45 degrees wide,  vertical=-1 to signal match window aspect */
    pfChanFOV(chan, 45.0f, -1.0f);

    /* Create an earth/sky model that draws sky/ground/horizon */
    {
	pfEarthSky *esky = pfNewESky();
#if 0
	pfESkyMode(esky, PFES_BUFFER_CLEAR, PFES_SKY_GRND );
	pfESkyAttr(esky, PFES_GRND_HT, -1.0f * bsphere.radius);
	pfESkyColor(esky, PFES_GRND_FAR, 0.3f, 0.1f, 0.0f, 1.0f);
	pfESkyColor(esky, PFES_GRND_NEAR, 0.5f, 0.3f, 0.1f, 1.0f);
#endif
	pfESkyMode(esky, PFES_BUFFER_CLEAR, PFES_FAST);
	pfESkyColor(esky, PFES_CLEAR, .3f, .3f, .7f, 0.5f);
    pfChanESky(chan, esky);
    }
    
    /* Simulate for twenty seconds. */
    while (1)
    {
	float      s, c, r=0;
	pfCoord	   view;

	/* Go to sleep until next frame time. */
	pfSync();		

	/* Initiate cull/draw for this frame. */
	pfFrame();

	t = pfGetTime();
	/* roll polygons around y axis */
	r += 5.0f;
	if (r >= 360.0f)
	    r = 0.0f;
	pfDCSRot (dcs1, 0.0f, 0.0f, r);
	pfDCSRot (dcs2, 0.0f, 0.0f, -r);
	
	/* Compute new view position for next frame. */
	pfSinCos(45.0f*t, &s, &c);
	pfSetVec3(view.hpr, 45.0f*t, -15.0f, 0);
	pfSetVec3(view.xyz, 2.0f * bsphere.radius * s, 
		-2.0f * bsphere.radius *c, 
		 0.5f * bsphere.radius);
	/* set view position for next frame */
	pfChanView(chan, view.xyz, view.hpr);
    }

    /* Terminate parallel processes and exit. */
    pfExit();

    return 0;
}

