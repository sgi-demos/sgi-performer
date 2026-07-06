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
 * layer.c: simple Performer program to demonstrate layered polygons
 *
 * $Revision: 1.23 $ $Date: 2000/10/06 21:26:34 $ 
 *
 */

#include <stdlib.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>

pfVec4          colors[] ={     {1.0f, 1.0f, 1.0f, 1.0f},
                                {0.0f, 0.0f, 1.0f, 1.0f},
                                {1.0f, 0.0f, 0.0f, 1.0f},
                                {0.0f, 1.0f, 0.0f, 1.0f} };

ushort          colorlist[] =   { 0, 1, 2, 3 };

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

int
main (int argc, char *argv[])
{
    float       t = 0.0f;
    pfScene     *scene;
    pfPipe      *p;
    pfPipeWindow *pw;
    pfChannel   *chan;
    pfSphere	bsphere;
    pfLayer	*layer;
    pfGeoSet	*gset;
    pfGeode     *geode1,*geode2,*geode3;
    pfGeoState  *gstate;
    pfTexture   *tex;
    pfTexEnv    *tev;
    pfEarthSky  *esky;
    pfDCS       *dcs1,*dcs2,*dcs3;

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

    /* Append to PFPATH additional standard directories where 
     * geometry and textures exist 
     */
    pfFilePath(".:/usr/share/Performer/data");

    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinName(pw, argv[0]);
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    pfOpenPWin(pw);	

    gstate = pfNewGState (pfGetSharedArena());
    pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);

    if (argc>1)
    {
        /* Set up textures & gstate */

        tev = pfNewTEnv (pfGetSharedArena());
        tex = pfNewTex (pfGetSharedArena()); 

        if (pfLoadTexFile (tex, argv[1]))
	{
	    pfGStateMode (gstate, PFSTATE_ENTEXTURE, 1);
	    pfGStateMode (gstate, PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
	    pfGStateMode (gstate, PFSTATE_ENLIGHTING, 0);
	    pfGStateVal (gstate, PFSTATE_ALPHAREF, 0.0f);
	    pfGStateAttr (gstate, PFSTATE_TEXENV, tev);
	    pfGStateAttr (gstate, PFSTATE_TEXTURE, tex);
	}
    }

    /* Set up geosets */

    gset = pfNewGSet(pfGetSharedArena());

    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, vertexlist);
    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, colorlist);
    pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, texlist);
    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 1);

    pfGSetGState (gset, gstate);

    /* set up scene graph */
    geode1 = pfNewGeode(); pfAddGSet(geode1, gset);
    geode2 = pfNewGeode(); pfAddGSet(geode2, gset);
    geode3 = pfNewGeode(); pfAddGSet(geode3, gset);

    dcs1 = pfNewDCS();
    pfAddChild(dcs1, geode1);

    dcs2 = pfNewDCS();
    pfAddChild(dcs2, geode2);
    pfDCSScale(dcs2, 0.5f);
    pfDCSRot (dcs2, 0.0f, 0.0f, 180.0f);

    dcs3 = pfNewDCS();
    pfAddChild(dcs3, geode3);
    pfDCSScale(dcs3, 0.25f);

    layer = pfNewLayer();
    pfAddChild(layer, dcs1);        /* first child is base */
    pfAddChild(layer, dcs2);        /* subsequent children layered */
    pfAddChild(layer, dcs3);

    scene = pfNewScene();
    pfAddChild(scene, layer);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene, &bsphere);

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere.radius);
    pfChanFOV(chan, 45.0f, 0.0f);

    /* Create an earth/sky model that draws sky/ground/horizon */
    esky = pfNewESky();
    pfESkyMode(esky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    pfESkyAttr(esky, PFES_GRND_HT, -1.0f * bsphere.radius);
    pfESkyColor(esky, PFES_GRND_FAR, 0.3f, 0.1f, 0.0f, 1.0f);
    pfESkyColor(esky, PFES_GRND_NEAR, 0.5f, 0.3f, 0.1f, 1.0f);
    pfChanESky(chan, esky);


    /* Simulate for twenty seconds. */
    while (t < 20.0f)
    {
	float      s, c;
	pfCoord	   view;

	/* Go to sleep until next frame time. */
	pfSync();		

	/* Compute new view position. */
	t = pfGetTime();
	pfSinCos(45.0f*t, &s, &c);
	pfSetVec3(view.hpr, 45.0f*t, -10.0f, 0);
	pfSetVec3(view.xyz, 2.0f * bsphere.radius * s, 
		-2.0f * bsphere.radius *c, 
		 0.5f * bsphere.radius);
	pfChanView(chan, view.xyz, view.hpr);

	/* Initiate cull/draw for this frame. */
	pfFrame();		
    }

    /* Terminate parallel processes and exit. */
    pfExit();

    return 0;
}
