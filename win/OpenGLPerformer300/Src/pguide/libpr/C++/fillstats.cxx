//
// Copyright 1995, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// This source code ("Source Code") was originally derived from a
// code base owned by Silicon Graphics, Inc. ("SGI")
// 
// LICENSE: SGI grants the user ("Licensee") permission to reproduce,
// distribute, and create derivative works from this Source Code,
// provided that: (1) the user reproduces this entire notice within
// both source and binary format redistributions and any accompanying
// materials such as documentation in printed or electronic format;
// (2) the Source Code is not to be used, or ported or modified for
// use, except in conjunction with OpenGL Performer; and (3) the
// names of Silicon Graphics, Inc.  and SGI may not be used in any
// advertising or publicity relating to the Source Code without the
// prior written permission of SGI.  No further license or permission
// may be inferred or deemed or construed to exist with regard to the
// Source Code or the code base of which it forms a part. All rights
// not expressly granted are reserved.
// 
// This Source Code is provided to Licensee AS IS, without any
// warranty of any kind, either express, implied, or statutory,
// including, but not limited to, any warranty that the Source Code
// will conform to specifications, any implied warranties of
// merchantability, fitness for a particular purpose, and freedom
// from infringement, and any warranty that the documentation will
// conform to the program, or any warranty that the Source Code will
// be error free.
// 
// IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
// LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
// ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
// SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
// OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
// PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
// OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
// USE, THE SOURCE CODE.
// 
// Contact information:  Silicon Graphics, Inc., 
// 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
// or:  http://www.sgi.com
//
//
// file: fillstatsC.C:
// ------------------
// 
// Demonstration of enabling and querying fill statsistics in a libpr program.
//
// $Revision: 1.15 $ 
// $Date: 2000/10/06 21:26:42 $ 
//
//

#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfWindow.h>

#include <Performer/prstats.h>

int main (void)
{
    float       t = 0.0f;
    int i, bits, sten;
    int first = 1;
    float s;
    
    
    // Initialize Performer
    pfInit();
    pfInitState(NULL);
    
    // set up number of bits needed for stats before
    //  window is made so that it will have the right number
    pfQuerySys(PFQSYS_MAX_STENCIL_BITS,&sten);
    bits = PF_MIN2(4, sten);
    pfStats::setHwAttr(PFSTATSHW_FILL_DCBITS, bits);
    
    
    // Initialize GL
    pfWindow *win = new pfWindow;
    win->setOriginSize(100, 100, 500, 500);
    win->setName("OpenGL Performer");
    win->setWinType(PFWIN_TYPE_X | PFWIN_TYPE_STATS);
    win->open();
    
    // set up the window
    pfFrustum *frust = new pfFrustum;
    frust->apply();
    delete frust;
    
    pfAntialias(PFAA_ON);
    pfCullFace(PFCF_OFF);
    
    pfVec3 *coords = new pfVec3[4];
    coords[0].set(-1.0f,  -1.0f,  0.0f );
    coords[1].set( 1.0f,  -1.0f,  0.0f );
    coords[2].set( 1.0f,  1.0f,   0.0f );
    coords[3].set(-1.0f,  1.0f,   0.0f );
    
    pfVec4 *colors = new pfVec4[4];
    colors[0].set(1.0f, 1.0f, 1.0f, 1.0f);
    colors[1].set(0.0f, 0.0f, 1.0f, 1.0f);
    colors[2].set(1.0f, 0.0f, 0.0f, 1.0f);
    colors[3].set(0.0f, 1.0f, 0.0f, 1.0f);
    
    // Set up a geoset
    pfGeoSet *gset = new pfGeoSet;
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, colors, NULL);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(1);
    
    // Set up a geostate, backface removal turned off
    pfGeoState *gstate = new pfGeoState;
    gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);
    gset->setGState(gstate);
    
    pfTranslate (0.0f, 0.0f, -4.0f);
    
    // set up fill stats
    pfStats *stats = new pfStats;
    stats->setClass(PFSTATSHW_ENGFXPIPE_FILL, PFSTATS_ON);
    // set for up to 4 bits of stencil - will count up to 15 levels of depth
    // complexity.
    //
    
    pfStats::setHwAttr(PFSTATSHW_FILL_DCBITS, bits);
    pfStats::enableHw(PFSTATSHW_ENGFXPIPE_FILL);
    
    // Simulate for twenty seconds.
    pfInitClock (0.0f);
    while ( t < 5.0f )
    {
        static pfVec4 clr(0.1f, 0.0f, 0.4f, 1.0f);
	
	t = pfGetTime();
	
	stats->open(PFSTATS_ALL);
        pfClear(PFCL_COLOR | PFCL_DEPTH, &clr);
	pfRotate (PF_Y, 1.0f);
	pfPushMatrix();
	s = 1.0f;
	for (i=0; i < 15; i++)
	{
	    pfScale(s, s, s);
	    gset->draw();
	    s -= 0.04f;
	}
	pfPopMatrix();
	
	pfStats::close(PFSTATS_ALL);
	
	if (first)
	{
	    pfStatsValFill fillstats;
	    
	    // query fill stats from first frame
	    stats->query(PFSTATSVAL_GFXPIPE_FILL, &fillstats, sizeof(fillstats));
	    fprintf(stderr, "Bits of accuracy for depth complexity: %d\n", 
		    (int) pfStats::getHwAttr(PFSTATSHW_FILL_DCBITS));
	    fprintf(stderr, "MPixels written: %.3f, depthComplexity: %.3f\n", 
		    fillstats.pixels / 1e6, fillstats.depthComplexity);
	    first = 0;
	}
	
	win->swapBuffers();
    }
    return 0;
}
