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
 * pvchan.c: Performer program to demonstrate multiple pfPipeVideo channels
 *              in one pipe.  Derived from multichan.c
 *
 * $Revision: 1.7 $ $Date: 2001/10/13 00:29:34 $ 
 *
 */

#include <stdlib.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>

#define CHAN_ARRAY 2
#define NUM_CHANS (CHAN_ARRAY*CHAN_ARRAY)

static void OpenPipeWin(pfPipeWindow *pw);
static void DrawChannel(pfChannel *chan, void *data);


/*
 *	Usage() -- print usage advice and exit. This
 *      procedure is executed in the application process.
 */
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: pvchan file.ext ...\n");
    exit(1);
}

int
main (int argc, char *argv[])
{
    float       t = 0.0f;
    pfScene     *scene;
    pfPipe      *p;
    pfPipeWindow *pw;
    pfChannel   *chan[NUM_CHANS];
    pfNode	*root=NULL;
    pfSphere	bsphere;
    int 	loop;
    char	str[PF_MAXSTRING];
    pfFrameStats *fstats;
    pfPipeVideoChannel *pvc;
    float	vpl, vpr, vpb, vpt, vps;
    int		i, j;
    int		nVChans, pvcIndex;

    if (argc < 2)
	Usage();

    /* Initialize Performer */
    pfInit();	

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess(PFMP_DEFAULT);

    /* Load all loader DSO's before pfConfig() forks */
    pfdInitConverter(argv[1]);

    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    pfConfig();		
    
    pfFrameRate(60.0f / NUM_CHANS);	
    pfPhase(PFPHASE_LOCK);

    /* Attach loaded file to a pfScene. */
    scene = pfNewScene();

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

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (root, &bsphere);
    pfAddChild(scene, root);

    /* Create a pfLightSource and attach it to scene. */
    pfAddChild(scene, pfNewLSource());

    /* Configure and open GL window */
    p = pfGetPipe(0);
    /* set the pipe screen so we know what vchans to use */
    pfPipeScreen(p, 0);
    pw = pfNewPWin(p);
    sprintf(str, "OpenGL Performer");
    pfPWinName(pw, str);
    pfPWinOriginSize(pw, 0, 0, 1000, 1000);
    pfPWinConfigFunc(pw, OpenPipeWin);
    pfConfigPWin(pw);

    /* Create and configure a pfChannel with a pfPipeVideoChannel  */
    nVChans = pfGetNumScreenVChans(0);
    pvc = NULL;
    for (loop=0; loop < NUM_CHANS; loop++)
    {
	chan[loop] = pfNewChan(p);	
	pfChanTravFunc(chan[loop], PFTRAV_DRAW, DrawChannel);
	pfChanNearFar(chan[loop], 1.0f, 10.0f * bsphere.radius);
	pfChanFOV(chan[loop], 45.0f, 0.0f);
	pfChanScene(chan[loop], scene);
	fstats = pfGetChanFStats(chan[loop]);
	if (loop < (nVChans-1))
	    pvc = NULL;
	if (!loop)
	{ /* get default window pipe video channel */
	    pvc = pfGetPWinPVChan(pw, 0);
	    pvcIndex = 0;
	}
	if (!pvc)
	{
	    pvc = pfNewPVChan(p);
	    pvcIndex = pfPWinAddPVChan(pw, pvc);
	}
	/* need to set manual mode or pfPipeVideoChannel will force a full
	 * channel initialization
	 */
	pfPVChanDVRMode(pvc, PFPVC_DVR_MANUAL);
	pfPVChanAreaScale(pvc, 0.8);
	pfChanPWinPVChanIndex(chan[loop], pvcIndex);
    }

    vps = 1.0f / CHAN_ARRAY;
    vpt = vps;
    vpb = 0.0f;
    for (i=0, loop=0; i < CHAN_ARRAY; i++)
    {
	vpl = 0.0f;
	vpr = vps;
	for (j=0; j < CHAN_ARRAY; j++, loop++)
	{
	    pfChanViewport (chan[loop], vpl, vpr, vpb, vpt);
	    vpl += vps;
	    vpr += vps;
	}
	vpb += vps;
	vpt += vps;
    }
    
    /* Simulate for twenty seconds. */
    while (pfGetPipeDrawCount(0) < 60)
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

	for (loop=0; loop < NUM_CHANS; loop++)
	{
	    pfChanView(chan[loop], view.xyz, view.hpr);
	    pfDrawChanStats(chan[loop]);
	}
	/* Initiate cull/draw for this frame. */
	pfFrame();		
    }

    /* Terminate parallel processes and exit. */
    pfExit();

    return 0;
}


/*
 *	OpenPipeWin() -- create a GL window: set up the
 *      window system, OpenGL, and OpenGL Performer. This
 *      procedure is executed for each window in the draw process 
 *	for that pfPipe.
 */

static void
OpenPipeWin(pfPipeWindow *pw)
{
    pfLight *Sun;
    
    pfOpenPWin(pw);

    /* create a light source in the "south-west" (QIII) */
    Sun = pfNewLight(NULL);
    pfLightPos(Sun, -0.3f, -0.3f, 1.0f, 0.0f);
}

static void
DrawChannel (pfChannel *channel, void *data)
{
	
    static pfVec4 clr[] = {{1.0f, 0.0f, 0.0f, 1.0f},
			    {0.0f, 0.0f, 1.0f, 1.0f}};
    static int i=0;

#if 0
    /* erase framebuffer and draw Earth-Sky model */
    pfClear(PFCL_COLOR | PFCL_DEPTH, clr[i]);
    i ^= 1;
#endif    
    pfClearChan(channel);

    /* invoke Performer draw-processing for this frame */
    pfDraw();

}
