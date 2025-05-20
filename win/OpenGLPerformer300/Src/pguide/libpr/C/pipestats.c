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
 * file: pipestats.c: 
 * ------------------
 * 
 * Demonstration of enabling and querying InfiniteReality 
 * pipe statsistics in a libpr program.
 *
 * $Revision: 1.6 $ $Date: 2002/11/13 00:23:21 $ 
 *
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h>
#endif
#include <Performer/pr.h>
#include <Performer/prstats.h>


pfVec3          coords[] ={     {-1.0f,  -1.0f,  0.0f },
                                { 1.0f,  -1.0f,  0.0f },
                                { 1.0f,  1.0f,   0.0f },
                                {-1.0f,  1.0f,   0.0f } };

pfVec4          colors[] ={     {1.0f, 1.0f, 1.0f, 1.0f},
                                {0.0f, 0.0f, 1.0f, 1.0f},
                                {1.0f, 0.0f, 0.0f, 1.0f},
                                {0.0f, 1.0f, 0.0f, 1.0f} };

static /*const*/ int attrs[] = {
    PFFB_RGBA, 
    PFFB_DOUBLEBUFFER, 
    PFFB_DEPTH_SIZE, 1, 
    PFFB_STENCIL_SIZE, 4, 
    PFFB_SAMPLES, 1,
    NULL,
};

pfStatsValGfxPipeTimes piptimes;
int NumQuads = 1;

int main (int argc, char *argv[])
{
    int         qret, t = 0;
    int 	xs, ys;
    pfGeoSet	*gset;
    pfGeoState  *gstate;
    pfStats	*stats;
    pfFrustum	*frust;
    pfWindow	*win;
    int i, bits, loop=1;
    int count = 100;
    float s;
    double start, end;

    if (argc > 1)
	NumQuads = atoi(argv[1]);

    pfInit();
    pfInitState(NULL);   

    pfQueryFeature(PFQFTR_PIPE_STATS, &qret);
    if (!qret)
    {
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Pipe stats are not supported on this hardware");
    }
    
    win = pfNewWin(NULL);
    pfWinOriginSize(win, 0, 0, 100, 100);
    pfWinName(win, "Performer FillStats");
    pfWinFBConfig(win, pfChooseFBConfig(pfGetCurWSConnection(), -1, attrs));
    pfOpenWin(win);
    pfWinType(win,PFWIN_TYPE_X | PFWIN_TYPE_STATS);
    pfInitGfx();
    
    frust = pfNewFrust(NULL);
    pfMakeOrthoFrust(frust, -1.0, 1.0, -1.0, 1.0);
    pfApplyFrust(frust);
    pfDelete(frust);
    pfAntialias(PFAA_ON);
    pfCullFace(PFCF_OFF);

    /* Set up a geoset */
    gset = pfNewGSet(NULL);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, NULL);
    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 1);

    /* Set up a geostate, backface removal turned off */
    gstate = pfNewGState (NULL);
    pfGStateMode(gstate, PFSTATE_CULLFACE, PFCF_OFF);
    pfGSetGState (gset, gstate);

    pfTranslate (0.0f, 0.0f, -4.0f);
    
    stats = pfNewStats(NULL);
    pfStatsClass(stats, PFSTATSHW_ENGFXPIPE_TIMES, PFSTATS_ON);
    pfStatsClassMode(stats, PFSTATSHW_GFXPIPE_TIMES, 
		PFSTATSHW_GFXPIPE_TIMES_MASK, PFSTATS_SET);
    pfEnableStatsHw(PFSTATSHW_ENGFXPIPE_TIMES);

    /* set loop for number of levels of depth complexity that can
     * be displayed - beyond this the colors will wrap.
     */
pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Loop = %d", loop);  
    /* Simulate for twenty seconds. */
    pfInitClock (0.0f);
    t = 0;
    s = 1.0f;
    pfScale(s, s, s);
    while ( t < count )
    {
	static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};
	int q;
	
	t++;

	glColor3f(1,1,1); /* get to start of video field */

	start = pfGetTime();

	pfOpenStats(stats, PFSTATSHW_ENGFXPIPE_TIMES);

	pfClear(PFCL_COLOR | PFCL_DEPTH, clr); 


        pfPushMatrix();
	pfRotate (PF_Y, 1.0f);
	for (i=0; i < loop; i++)
	{
	    for(q=0; q <NumQuads; q++)
		pfDrawGSet(gset);
	    s -= .04;
	}
        pfPopMatrix();

	
	pfGfxPipeTimestamp(PFSTATSHW_TIMESTAMP_FRAME_END);
	pfCloseStats(PFSTATSHW_ENGFXPIPE_TIMES);

	end = pfGetTime();

	pfSwapWinBuffers(win);
	
	 /* query fill stats from first frame */
	pfQueryStats(stats, PFSTATSVAL_GFXPIPE_TIMES, &piptimes, 0);
	fprintf(stderr, "Tris: %d Elapsed msecs: host=%.3f, pipe=%.3f\n", 
		loop * NumQuads, (end - start) * 1000.0f, piptimes.time.total);
    }
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Bye....");
    /*sleep(1);*/
}

