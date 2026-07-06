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
 * winfbconfig.C: simple Performer program to demonstrate 
 *	frambuffer configuration selection for windows
 *
 * $Revision: 1.7 $ $Date: 2002/11/13 01:04:54 $ 
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h>
#endif

#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfWindow.h>


static int FBAttrs[] = {
    PFFB_RGBA, 
    PFFB_DOUBLEBUFFER, 
    PFFB_DEPTH_SIZE, 1, 
    PFFB_SAMPLES, 1,
    PFFB_RED_SIZE, 1,
    PFFB_ALPHA_SIZE, 1,
    PFFB_STENCIL_SIZE, 1, 
    PFFB_STEREO, 
    NULL,
};

static int OverlayAttrs[] = {
    PFFB_LEVEL, 1,
    PFFB_BUFFER_SIZE, 8, 
    NULL,
};

int MSamples = -1;
int DepthBits = -1;
int StencilBits = -1;
int StereoReq = -1;
int RGBbits = -1;
int VisualId = -1;

int WinType = PFWIN_TYPE_X;

static char OptionStr[] = "s:S:z:d:m:r:xv:";

static void
docmdline(int argc, char **argv)
{
     extern char *optarg;
     extern int  optind;
     int opt;
        
     while ((opt = getopt(argc,argv,OptionStr)) != -1) {
	 switch(opt) 
	 {
	 /* custom options */
	 case 'm':
	    MSamples = atoi(optarg);
	    break;
	 case 'z':
	 case 'd':
	    DepthBits = atoi(optarg);
	    break;
	 case 'r':
	    RGBbits = atoi(optarg);
	    break;
	 case 's':
	    StencilBits = atoi(optarg);
	    break;
	 case 'S':
	    StereoReq = (atoi(optarg) ? PFFB_STEREO : 0);
	    break;
	 case 'x':
	    WinType ^= PFWIN_TYPE_X;
	    break;
/* ROBJ
Changed this from atoi which was just giving nonsense visId 
*/
	 case 'v':
	    sscanf(optarg,"0x%x",&VisualId);
	    break;
	 }
    }
}

void main (int argc, char **argv)
{
    pfGeoSet	*gset;
    pfGeoState  *gstate;
    pfWindow 	*win, *over;
    pfFrustum   *frust;

    docmdline(argc, argv);


    if (MSamples > -1)
	FBAttrs[5] = MSamples;
    if (DepthBits > -1)
	FBAttrs[4] = DepthBits;
    if (RGBbits > -1)
	FBAttrs[7] = RGBbits;
    if (StencilBits > -1)
	FBAttrs[9] = StencilBits;
    if (StereoReq > -1)
	FBAttrs[10] = StereoReq;
    
    /* Initialize Performer */
    pfInit();
    pfInitState(NULL);
    pfNotifyLevel(5);

    /* Initialize GL */
    win = new pfWindow;
    win->setMode(PFWIN_AUTO_RESIZE, 1);
    win->setMode(PFWIN_ORIGIN_LL, 1);
    win->setMode(PFWIN_HAS_OVERLAY, 1);
    
    
    win->setOriginSize(100, 100, 500, 500);
    win->setName("OpenGL Performer");
    win->setWinType(WinType);
    
    if (VisualId > -1)
	win->setFBConfigId(VisualId);
    else
	win->setFBConfigAttrs(FBAttrs);

    over = new pfWindow;
    over->setName("OpenGL Performer Overlay");
    over->setWinType(WinType | PFWIN_TYPE_OVERLAY);
    if (!(over->chooseFBConfig(OverlayAttrs)))
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		"pfWindow::chooseFBConfig failed for OVERLAY win");
    
    win->setOverlayWin(over);
    
    win->open();
    win->print(PFTRAV_SELF, PFPRINT_VB_ON, NULL, NULL);
    /*pfuPrintWinFBConfig(win, 0);*/
    over->print(PFTRAV_SELF, PFPRINT_VB_ON, NULL, NULL);
    
    over->setIndex(PFWIN_OVERLAY_WIN);
    over->select();


    win->setIndex(PFWIN_GFX_WIN);
    win->select();

    /* set up the gfx window */
    frust = new pfFrustum;
    frust->apply();
    delete frust;

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

    /* Set up a geoset */
    gset = new pfGeoSet;
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, colors, NULL);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(1);

    /* Set up a geostate, backface removal turned off */
    gstate = new pfGeoState;
    gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);
    gstate->apply();

    pfTranslate(0.0f, 0.0f, -4.0f);

    /* Simulate for twenty seconds. */
    pfInitClock(0.0f);
    while (1 /* t < 20.0f */)
    {
        static pfVec4 clr(0.1f, 0.0f, 0.4f, 1.0f);

	pfClear(PFCL_COLOR | PFCL_DEPTH, &clr);
	pfRotate (PF_Y, 1.0f);
	gset->draw();

	win->swapBuffers();
    }
}
