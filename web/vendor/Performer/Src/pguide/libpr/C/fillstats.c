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
 * file: fillstats.c: 
 * ------------------
 * 
 * Demonstration of enabling and querying fill statsistics in a libpr program.
 *
 * $Revision: 1.22 $ $Date: 2000/10/06 21:26:44 $ 
 *
 */

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

int main (void)
{
    float       t = 0.0f;
    int 	xs, ys;
    pfGeoSet	*gset;
    pfGeoState  *gstate;
    pfStats	*stats;
    pfStatsValFill fillstats;
    float s;
    pfWindow    *win;
    pfFrustum   *frust;
    int i, sten, bits;
    int first = 1;


    /* Initialize Performer */
    pfInit();
    pfInitState(NULL);

    /* set up number of bits needed for stats before
     * window is made so that it will have the right number
     */
    pfQuerySys(PFQSYS_MAX_STENCIL_BITS,&sten);
    bits = PF_MIN2(4, sten);
    pfStatsHwAttr(PFSTATSHW_FILL_DCBITS, bits);

    /* Initialize GL */
    win = pfNewWin(NULL);
    pfWinOriginSize(win, 100, 100, 500, 500);
    pfWinName(win, "OpenGL Performer fill stats test");
    pfWinType(win, PFWIN_TYPE_X | PFWIN_TYPE_STATS );
    pfOpenWin(win);

    /* set up the window */
    frust = pfNewFrust(NULL);
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
    
    /* set up fill stats */
    stats = pfNewStats(NULL);
    pfStatsClass(stats, PFSTATSHW_ENGFXPIPE_FILL, PFSTATS_ON);
    /* set for up to 4 bits of stencil - will count up to 15 levels of depth
     * complexity.
     */
    pfEnableStatsHw(PFSTATSHW_ENGFXPIPE_FILL);

    /* Simulate for twenty seconds. */
    pfInitClock (0.0f);
    while (t < 5.0f )
    {
        static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};

	t = pfGetTime();

	pfOpenStats(stats, PFSTATS_ALL);
        pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
	pfRotate (PF_Y, 1.0f);
	pfPushMatrix();
	s = 1.0f;
	for (i=0; i < 15; i++)
	{
	    pfScale(s, s, s);
	    pfDrawGSet(gset);
	    s -= .04;
	}
	pfPopMatrix();
	
	pfCloseStats(PFSTATS_ALL);
	
	if (first)
	{
	     /* query fill stats from first frame */
	    pfQueryStats(stats, PFSTATSVAL_GFXPIPE_FILL, &fillstats, 
			 sizeof(fillstats));
	    fprintf(stderr, "Bits of accuracy for depth complexity: %d\n", 
		    (int) pfGetStatsHwAttr(PFSTATSHW_FILL_DCBITS));
	    fprintf(stderr, "MPixels written: %.3f, depthComplexity: %.3f\n", 
	    fillstats.pixels / 1e6, fillstats.depthComplexity);
	    first = 0;
	}

	pfSwapWinBuffers(win);
    }
    return 0;
}
