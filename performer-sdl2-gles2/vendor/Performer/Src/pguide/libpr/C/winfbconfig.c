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
 * winfbconfig.c: simple Performer program to demonstrate 
 *	frambuffer configuration selection for windows
 *
 * $Revision: 1.18 $ $Date: 2002/11/13 00:23:21 $ 
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h>
#endif
#include <Performer/pr.h>

pfVec3          coords[] ={     {-1.0f,  -1.0f,  0.0f },
                                { 1.0f,  -1.0f,  0.0f },
                                { 1.0f,  1.0f,   0.0f },
                                {-1.0f,  1.0f,   0.0f } };

pfVec4          colors[] ={     {1.0f, 1.0f, 1.0f, 1.0f},
                                {0.0f, 0.0f, 1.0f, 1.0f},
                                {1.0f, 0.0f, 0.0f, 1.0f},
                                {0.0f, 1.0f, 0.0f, 1.0f} };


static int FBAttrs[] = {
    PFFB_RGBA, 
    PFFB_DOUBLEBUFFER, 
    PFFB_DEPTH_SIZE, 1, 
    PFFB_SAMPLES, 1,
    PFFB_RED_SIZE, 1,
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
	 case 'v':
	    sscanf(optarg,"0x%x",&VisualId);
	    break;
	 }
    }
}

void main (int argc, char **argv)
{
    float t = 0.f;
    pfGeoSet	*gset;
    pfGeoState  *gstate;
    pfWindow 	*win, *over;
    pfFrustum   *frust;
    static char str[] = "Overlay !!!";

    docmdline(argc, argv);

    if (MSamples > -1)
	FBAttrs[5] = MSamples;
    if (DepthBits > -1)
	FBAttrs[3] = DepthBits;
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
    win = pfNewWin(NULL);
    pfWinMode(win, PFWIN_AUTO_RESIZE, 1);
    pfWinMode(win, PFWIN_ORIGIN_LL, 1);
    pfWinMode(win, PFWIN_HAS_OVERLAY, 1);
    
    
    pfWinOriginSize(win, 100, 100, 500, 500);
    pfWinName(win, "OpenGL Performer window configuration test");
    pfWinType(win, WinType);
    
    if (VisualId > -1)
	pfWinFBConfigId(win, VisualId);
    else
	pfWinFBConfigAttrs(win, FBAttrs);

    over = pfNewWin(NULL);
    pfWinName(over, "OpenGL Performer Overlay");
    pfWinType(over, WinType | PFWIN_TYPE_OVERLAY);
    if (!(pfChooseWinFBConfig(over, OverlayAttrs)))
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		"pfChooseWinFBConfig failed for OVERLAY win");
    
    pfWinOverlayWin(win, over);
    
    pfOpenWin(win);
    pfPrint(win, 1, PFPRINT_VB_ON, 0);
    /*pfuPrintWinFBConfig(win, 0);*/
    pfPrint(over, 1, PFPRINT_VB_ON, 0);
    

    pfWinIndex(win, PFWIN_OVERLAY_WIN);
    pfSelectWin(win);


    pfWinIndex(win, PFWIN_GFX_WIN);
    pfSelectWin(win);

    /* set up the gfx window */
    frust = pfNewFrust(NULL);
    pfApplyFrust(frust);
    pfDelete(frust);

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
    while (t < 5.f)
    {
        static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};

	t = pfGetTime();
	pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
	pfRotate (PF_Y, 1.0f);
	pfDrawGSet(gset);

	pfSwapWinBuffers(win);
    }
}
