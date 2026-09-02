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
 * smoke.c:   simple Performer program to demonstrate using 
 *            pfutil smoke
 *
 * $Revision: 1.20 $ $Date: 2000/10/06 21:26:41 $ 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>


static void OpenPipeline (pfPipe *p);
static void DrawChannel(pfChannel *chan, void *data);

pfSphere *bsphere;
Display *Dsp=NULL;
int Win=NULL; 

static void DrawChannel (pfChannel *channel, void *data)
{
	pfVec3 vec;
        static pfMatrix tempmat;

        pfClearChan (channel);
        pfDraw();

	pfSetVec3 (vec, 0.0f, -1.0f, 0.0f);
	pfuDrawSmokes (vec);
}

/*
 *	Usage() -- print usage advice and exit. This
 *      procedure is executed in the application process.
 */
static void
Usage (void)
{
    fprintf(stderr, "Usage: smoke file.ext ...\n");
    exit(1);
}


int
main (int argc, char *argv[])
{
    float       t = 0.0f;
    pfScene     *scene;
    pfPipe      *p;
    pfChannel   *chan;
    pfNode	*root;
    pfEarthSky	*esky;
    pfVec3	vec;
    pfuSmoke	*smoke;
    pfuSmoke	*fire;

    if (argc < 2)
	Usage();

    /* Initialize Performer */
    pfInit();	

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess(PFMP_DEFAULT);
    bsphere = (pfSphere*) pfMalloc (sizeof(pfSphere), pfGetSharedArena());

    /* Append to PFPATH additional standard directories where 
     * geometry and textures exist 
     */
    pfFilePath(".:/usr/share/Performer/data");

    pfuInitSmokes();

    /* Load all loader DSO's before pfConfig() forks */
    pfdInitConverter(argv[1]);

    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    pfConfig();			

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
    pfInitPipe(p, OpenPipeline);	

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere->radius);
    pfChanFOV(chan, 45.0f, 0.0f);

    esky = pfNewESky();
    pfESkyMode(esky, PFES_BUFFER_CLEAR, PFES_SKY_GRND);
    pfESkyAttr(esky, PFES_GRND_HT, -1.0f * bsphere->radius);
    pfESkyColor(esky, PFES_GRND_FAR, 0.3f, 0.1f, 0.0f, 1.0f);
    pfESkyColor(esky, PFES_GRND_NEAR, 0.5f, 0.3f, 0.1f, 1.0f);
    pfChanESky(chan, esky);

    pfSetVec3 (vec, 0.0f, 0.0f, 0.0f);

    fire = pfuNewSmoke();
    pfuSmokeType (fire, PFUSMOKE_FIRE);
    pfuSmokeOrigin (fire, vec, 0.25f * bsphere->radius);
    pfuSmokeMode (fire, PFUSMOKE_START);

    smoke = pfuNewSmoke();
    pfuSmokeType (smoke, PFUSMOKE_SMOKE);
    pfuSmokeOrigin (smoke, vec, 0.25f * bsphere->radius);
    pfuSmokeVelocity (smoke, 1.0f, 0.25f * bsphere->radius);
    pfuSmokeDensity (smoke, 0.33f, 2.42f, 1.83f);
    pfuSmokeMode (smoke, PFUSMOKE_START);

    /* Simulate for twenty seconds. */
    while (t < 20.0f)
    {
	pfCoord    view;

	/* Go to sleep until next frame time. */
	pfSync();		

	/* Compute new view position. */
	t = pfGetTime();
	pfSetVec3(view.hpr, 0.0f, -10.0f, 0.0f);
	pfSetVec3(view.xyz, 0.0f, 
		-2.0f * bsphere->radius, 
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
OpenPipeline (pfPipe *p)
{
    /* Open graphics window. */
    pfWinopen("OpenGL Performer", 100, 500, 100, 500);

    /* Configure window with reasonable defaults. */
    pfInitGLXGfx(p, NULL, Win, pfGetCurGLXContext(), NULL, PFGLX_AUTO_RESIZE);

    /* Create and apply a default material for those models
     * without one.
     */
    pfApplyMtl(pfNewMtl(pfGetSharedArena()));

    /* Create a default lighting model. */
    pfApplyLModel(pfNewLModel(pfGetSharedArena()));
}
