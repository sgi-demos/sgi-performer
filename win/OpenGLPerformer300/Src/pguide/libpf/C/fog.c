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
 * fog.c: Performer program to demonstrate use of fog.  
 *        Based on simple.c
 *
 * $Revision: 1.25 $ $Date: 2000/10/06 21:26:33 $ 
 *
 */

#include <stdlib.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>


static void OpenPipeWin (pfPipeWindow *pw);
static void DrawChannel (pfChannel *chan, void *data);

pfSphere *bsphere;



/*
 *	Usage() -- print usage advice and exit. This
 *      procedure is executed in the application process.
 */
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: fog file.ext ...\n");
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
    pfEarthSky	*esky;
    pfNode	*root;

    if (argc < 2)
	Usage();

    /* Initialize Performer */
    pfInit();	

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess(PFMP_DEFAULT);			
    bsphere = (pfSphere*) pfMalloc (sizeof(pfSphere), pfGetSharedArena());

    /* Load all loader DSO's before pfConfig() forks */
    pfdInitConverter(argv[1]);

    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    pfConfig();			

    /* Append to PFPATH additional standard directories where 
     * geometry and textures exist 
     */
    pfFilePath(".:/usr/share/Performer/data");

    /* Read a single file, of any known type. */
    if ((root = pfdLoadFile(argv[1])) == NULL) 
    {
	pfExit();
	exit(-1);
    }

    /* Attach loaded file to a pfScene. */
    scene = pfNewScene();
    pfAddChild(scene, root);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene, bsphere);

    /* Create a pfLightSource and attach it to scene. */
    pfAddChild(scene, pfNewLSource());

    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinName(pw, argv[0]);
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    pfPWinConfigFunc(pw, OpenPipeWin);	
    pfConfigPWin(pw);	

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere->radius);
    pfChanFOV(chan, 45.0f, 0.0f);
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);

    esky = pfNewESky();
    pfESkyMode(esky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    pfESkyAttr(esky, PFES_GRND_HT, -1.0f * bsphere->radius);
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
	pfSetVec3(view.xyz, 2.0f * bsphere->radius * s,
		-2.0f * bsphere->radius *c,
		 0.5f * bsphere->radius);

	pfChanView(chan, view.xyz, view.hpr);

	/* Initiate cull/draw for this frame. */
	pfFrame();		
    }

    /* Terminate parallel processes and exit. */
    pfExit();

    return 0;
}


/*
 *	OpenPipeline() -- create a GL window: set up the
 *      window system, OpenGL, and OpenGL Performer. This
 *      procedure is executed in the draw process (when
 *      there is a separate draw process).
 */
static void
OpenPipeWin (pfPipeWindow *pw)
{
    pfFog *fog;
    
    /* Open graphics window. */
    pfOpenPWin(pw);

    fog = pfNewFog(pfGetSharedArena());
    pfFogType (fog, PFFOG_VTX_LIN);
    pfFogColor (fog, 1.0, 1.0, 1.0);
    pfFogRange (fog, 0.5f * bsphere->radius, 3.5f * bsphere->radius);
    pfApplyFog (fog);
    pfEnable (PFEN_FOG);
    pfOverride(PFSTATE_FOG | PFSTATE_ENFOG, PF_ON);
}

/* Draw process callback */
static void DrawChannel (pfChannel *chan, void *data)
{
	pfClearChan (chan);
	pfDraw ();
}

