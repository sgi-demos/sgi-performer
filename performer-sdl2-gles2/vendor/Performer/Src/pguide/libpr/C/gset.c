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
 * gset.c: simple Performer program to demonstrate an easy geoset.
 *
 * $Revision: 1.18 $ $Date: 2000/10/06 21:26:44 $ 
 *
 */

#include <Performer/pr.h>

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
    pfWindow 	*win;
    pfFrustum   *frust;

    /* Initialize Performer */
    pfInit();
    pfInitState(NULL);

    /* Initialize GL */
    win = pfNewWin(NULL);
    pfWinOriginSize(win, 100, 100, 500, 500);
    pfWinName(win, "OpenGL Performer geoset test");
    pfWinType(win, PFWIN_TYPE_X);
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

    /* Simulate for twenty seconds. */
    pfInitClock (0.0f);
    while (t < 5.0f)
    {
        static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};

	t = pfGetTime();
	pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
	pfRotate (PF_Y, 1.0f);
	pfDrawGSet(gset);

	pfSwapWinBuffers(win);
    }
    return 0;
}
