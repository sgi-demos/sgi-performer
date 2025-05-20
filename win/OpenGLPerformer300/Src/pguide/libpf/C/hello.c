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
 * hello.c: Welcome Performer program for programmer's guide
 *
 * $Revision: 1.15 $ $Date: 2000/10/06 21:26:34 $ 
 *
 */

#include <Performer/pf.h>
#include <Performer/pfdu.h>

int
main (int argc, char *argv[])
{
    pfPipe      *pipe;
    pfChannel   *chan;
    pfScene     *scene;
    pfGeoState  *gstate;
    pfText	*text;
    pfFont 	*fnt;
    pfString	*str;
    void	*arena;
    pfSphere	bsphere;
    float       t = 0.0f;
    char        path[PF_MAXSTRING];
    int xs, ys;

    /* Initialize Performer */
    pfInit();	
    
    /* Get shared memory arena */
    arena = pfGetSharedArena();

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess( PFMP_DEFAULT );			

    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     */
    pfConfig();			

    /* Append standard directories to Performer search path, PFPATH */
    pfFilePath(".:/usr/share/Performer/data:../../../../data");

    /* Attach loaded file to a new pfScene. */
    scene = pfNewScene();
    
    /* Create a scene pfGeoState with lighting enabled */
    gstate = pfNewGState(arena);
    pfGStateMode(gstate, PFSTATE_ENLIGHTING, PF_ON);
    /* attach pfGeoState to the scene */
    pfSceneGState(scene, gstate);
    
    /* Create a pfLightSource and attach it to scene. */
    pfAddChild(scene, pfNewLSource());
    
    /* Create 3D message and place in scene. */
    text = pfNewText();
    pfAddChild(scene, text);
    if ((fnt = pfdLoadFont_type1("Times-Elfin", PFDFONT_EXTRUDED))!= NULL)
    {
	str = pfNewString(arena);

        pfStringFont(str, fnt);	
/*	pfStringMode(str, PFSTR_DRAWSTYLE, PFSTR_EXTRUDED);*/
	pfStringMode(str, PFSTR_JUSTIFY, PFSTR_MIDDLE);
	pfStringColor(str, 1.0f, 0.0f, 0.8f, 1.0f);
	pfStringString(str, "Welcome to OpenGL Performer");
	pfFlattenString(str);
	pfAddString(text, str);
    }
    else
    {
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"Couldn't find font file.");
	exit(0);
    }

    pipe = pfGetPipe(0);

    /* Create and configure a pfChannel. */
    chan = pfNewChan(pipe);	
    pfChanScene(chan, scene);
    pfChanFOV(chan, 60.0f, 0.0f);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (text, &bsphere);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere.radius);

    /* Spin text for 10 seconds. */
    while (t < 10.0f)
    {
	pfCoord	   view;
	float      s, c;

	/* Go to sleep until next frame time. */
	pfSync();		

	/* Initiate cull/draw for this frame. */
	pfFrame();
	
	/* Compute new view position, rotating around text. */
	t = pfGetTime();
	pfSinCos(45.0f*t, &s, &c);
	pfSetVec3(view.hpr, 45.0f*t, -5.0f, 0);
	pfSetVec3(view.xyz, 2.0f * bsphere.radius * s, 
		-2.0f * bsphere.radius *c, 
		 0.3f * bsphere.radius);
	pfChanView(chan, view.xyz, view.hpr);
    }

    /* Terminate parallel processes and exit. */
    pfExit();
}
