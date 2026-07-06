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
 * vchan.c 
 *
 *  example run:
 *	vchan [-d]
 *	-d	- to disable dynamic resolution changing to see the
 *			viewport change
 *
 *  run-time controls:
 *  -------------------
 *  up/down arrows: scale video channel output up and down
 *  right/left arrows: move video channel output origin up/down
 *  p - draw points
 *  w,l - draw lines
 *  s - disable pixel scale
 *  S - enable pixel scale
 *  i/k - force pixel scale up/down
 *  space - reset
 * 
 *
 *
 * $Revision: 1.15 $
 * $Date: 2002/12/04 04:31:17 $
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h>
#include <string.h>
#include <X11/keysym.h>
#endif
#include <Performer/pfutil.h>

#define NUM_GSETS 16
#define NUM_GSETS_2 8

static void do_events(void);
static void draw_scene(void);
static void drawVChanBox(pfVideoChannel *vChan);


pfVec4          scolors[] ={{1., 1., 1., 1.}, 
			    {1., 0., 0., 1.},
			    {0., 1., 0., 1.},
			    {0., 0., 1., 1.}};

pfVec3          scoords[] ={{0, 0,0},
			    {1, 0,0},
			    {1,1,0},
			    {0,1,0},
			    {0, 0, 0},
			    {1, 0, 0},
			    {1,1, 0},
			    {0,1, 0}};


static pfVideoChannel *VChan;
static pfDispList* dlist;
static pfGeoSet	*geom;
static pfGeoState *gstate;

static Display *Dsp;
static pfWindow *Win;
static pfFrustum *frust;
static int winXSize=1280, winYSize=1024;
static int dfrXSize=1280, dfrYSize=1024;
static int ScreenWidth, ScreenHeight;

static float pixSize = 1.0f;

static float scaleFactor = 1.0f;
static int originTrans = 0;
static int DoDVR = 1;
static int XMinDelta, YMinDelta;
static int MouseX=0, MouseY=0;
static int Bind = 1;

static char OptionStr[] = "d?";

static int
docmdline(int argc, char **argv)
{
     extern char *optarg;
     extern int  optind;
     int opt;
        
     while ((opt = getopt(argc,argv,OptionStr)) != -1) {
	 switch(opt) {
	 /* custom options */
	 case 'd':
	    DoDVR = 0;
	    break;
	 case '?':
	 default:
	    exit(0);
	    break;
	} /* switch */
    } /* while */
    
}

#define XWID_NULL   0

int
main(int argc, char *argv[])
{
    int			i, j, k, haveDetail, ret=0;
    int			arg;
    char*		str;
    Window		xwin;
    int			haveDVR;

    pfInit();

    pfQueryFeature(PFQFTR_DVR, &haveDVR);
    if (!haveDVR)
    {
	pfNotify(PFNFY_WARN,PFNFY_RESOURCE,
	"Dynamic resolution is NOT supported on this platform");
    }
    
    /* cmdline options */
    arg = docmdline(argc, argv);

    /* Initialize Performer */
    pfNotifyLevel(PFNFY_INFO);
    pfInitState(NULL);

    /* initialize X and video display */
    Dsp = pfGetCurWSConnection();
    pfGetScreenSize(0, &ScreenWidth, &ScreenHeight);

    VChan = pfNewVChan(NULL);
    pfGetVChanSize(VChan, &winXSize, &winYSize);
    pfGetVChanMinDeltas(VChan, &XMinDelta, &YMinDelta);
    
    /* Initialize GL */
    Win = pfNewWin(NULL);
    pfWinOriginSize(Win, 0, 0, winXSize, winYSize);
    pfWinName(Win, "video resize");
    pfWinType(Win, PFWIN_TYPE_X);
    pfOpenWin(Win);
    
    xwin = pfGetWinWSDrawable(Win);
#ifndef WIN32
    XSelectInput(Dsp, xwin,  StructureNotifyMask | PointerMotionMask |
			ButtonPressMask | ButtonReleaseMask | KeyPressMask);
    XMapWindow(Dsp, xwin);
    XSync(Dsp,FALSE);
#endif


    pfVChanWSWindow(VChan, xwin);
    pfBindVChan(VChan);
    
    /* turn off X cursor since it doesn't DR well - we will draw our own */
    pfuLoadWinCursor(Win, PFU_CURSOR_OFF);
    
    frust = pfNewFrust(NULL);
    /* pfMakeSimpleFrust(frust, 60.0f); */
    pfMakeOrthoFrust(frust, 0, winXSize, 0, winYSize);
    pfFrustNearFar(frust, -1, 1);
    pfApplyFrust(frust);
    pfAntialias(PFAA_ON);
    pfCullFace(PFCF_OFF);

    gstate = pfNewGState(NULL);
    geom = pfNewGSet(NULL);
    pfGSetAttr(geom, PFGS_COORD3, PFGS_PER_VERTEX, scoords, NULL);
    pfGSetAttr(geom, PFGS_COLOR4, PFGS_OVERALL, scolors, NULL);
    pfGSetPrimType(geom, PFGS_QUADS);
    pfGSetPntSize(geom,  2.0f);
    pfGSetLineWidth(geom,  2.0f);
    pfGSetNumPrims(geom, 1);
    pfGSetGState(geom, gstate);

    pfPushMatrix();
    
    do_events();

    return 0;
}

static void draw_scene(void) 
{
    int i, j, ret=0;
    static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};
    static int oxs = -1,  oys = -1;
    static int org = -1;
    
    if (XMinDelta > 0)
	dfrXSize = winXSize * scaleFactor;
    if (YMinDelta > 0)
	dfrYSize = winYSize * scaleFactor;
    
    if (oxs != dfrXSize || oys != dfrYSize || org != originTrans)
    {
	if (DoDVR)
	{
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Requested Rect: %d,%d %dx%d",
		originTrans, originTrans, dfrXSize,  dfrYSize);
	    pfVChanOutputSize(VChan, dfrXSize, dfrYSize);
	    pfVChanOutputOrigin(VChan, originTrans, originTrans);
	    pfApplyVChan(VChan);
	    /* may not get exactly what you ask for */
	    pfGetVChanOutputSize(VChan, &dfrXSize, &dfrYSize);
	    pfGetVChanOutputOrigin(VChan, &ret, &ret);
	    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "    Got rect: %d,%d %dx%d", 
		ret, ret, dfrXSize,  dfrYSize);
	    scaleFactor = dfrYSize/(float)winYSize;
	}

	if (pixSize > 0.0f)
	    pfPixScale(scaleFactor);
	else
	    pfPixScale(1);
	glViewport(0, 0, dfrXSize, dfrYSize);
	oxs = dfrXSize;
	oys = dfrYSize;
	org = originTrans;
    }
    
    pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
    
    pfPushMatrix();
    pfTranslate(10.0, 0.0, 0.0);
    pfScale(64.0f, 64.0f, 1.0f);
    for (i = 0; i < NUM_GSETS_2; i++)
    {	
	pfPushMatrix();
	for (j = 0; j < (NUM_GSETS_2); j++)
	{
	    pfPushMatrix();
	    pfDrawGSet(geom);
	    pfPopMatrix();
	    pfTranslate(0.0, 2.5, 0.0);
	}
	pfPopMatrix();
	pfTranslate(3.0, 0.0, 0.0);
	
    }    
    pfPopMatrix();
    
    /* draw 2D cursor */
    pfuDrawVChan2DCursor(VChan, MouseX, MouseY);

    drawVChanBox(VChan);
    
    pfSwapWinBuffers(Win);
    
   {
   int err;
   if ((err = glGetError()) != GL_NO_ERROR)
	pfNotify(PFNFY_NOTICE,PFNFY_USAGE,"OpenGL Error 0x%x - %s",err, gluErrorString(err));
   }
}

static void do_events(void)
{
#ifndef WIN32
    while (1)
    {
    if (XEventsQueued(Dsp, QueuedAfterFlush))
    while (XEventsQueued(Dsp, QueuedAlready))
    {
	XEvent event;
	
	XNextEvent(Dsp, &event);
	switch (event.type) 
	{
	case ConfigureNotify:
	    break;
	case MotionNotify:
            {
                XMotionEvent *motion_event = (XMotionEvent *) &event;
                MouseX =  motion_event->x;
                MouseY = ScreenHeight - motion_event->y;
            }
            break;

	case KeyPress:
	{
	    char buf[100];
	    int rv;
	    KeySym ks;

	    rv = XLookupString(&event.xkey, buf, sizeof(buf), &ks, 0);
	       
	    switch(ks) {
	    case XK_Escape: 
		exit(0);
		break;
	    case XK_space:
		scaleFactor = 1.0f;
		pfGSetDrawMode(geom, PFGS_WIREFRAME, 0);
		pfGSetPrimType(geom, PFGS_QUADS);
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Reset");
		break;
	    case XK_b:
		Bind ^= 1;
		if (Bind)
		    pfBindVChan(VChan);
		else
		    pfUnbindVChan(VChan);
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"BIND: %d", Bind);
		break;
	    case XK_Up:
		scaleFactor += 0.01f;
		if (scaleFactor >= 1.0f)
		    scaleFactor = 1.0f;
		break;
	    case XK_Down:
		if (scaleFactor >= 0.1f)
		    scaleFactor -= 0.01f;
		break;
	    case XK_Right:
		originTrans += 1;
		if (originTrans >= winYSize)
		    originTrans = winYSize-1;
		break;
	    case XK_Left:
		originTrans -= 1;
		if (originTrans < 0)
		    originTrans = 0;
		break;
	    case XK_i:
		pixSize += 0.1f;
		if (pixSize >= 1.0f)
		    pixSize = 1.0f;
		break;
	    case XK_k:
		if (pixSize >= 0.1f)
		    pixSize -= 0.1f;
		break;
	    case XK_w:
	    case XK_l:
		pfGSetDrawMode(geom, PFGS_WIREFRAME, 1);
		pfGSetPrimType(geom, PFGS_QUADS);
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Lines");
		break;
	    case XK_p:
		pfGSetPrimType(geom, PFGS_POINTS);
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Points");
		break;
	    case XK_s:
		pixSize = -1;
		break;
	    case XK_S:
		pixSize = 1;
		break;
	    case XK_f:
	    case XK_q:
		pfGSetDrawMode(geom, PFGS_WIREFRAME, 0);
		pfGSetPrimType(geom, PFGS_QUADS);
		break;
	    default:
		break;
	    }
	}
	break;
	default:
	    break;
	}/* switch */
	}

	draw_scene();
    } /* while have events */
#endif
}

static void
drawVChanBox(pfVideoChannel *vChan)
{
    int xo, yo, xs, ys;
    float xscale, yscale;
    pfVec2 coords[4];

    pfPushState();
    pfBasicState();
    glDepthFunc(GL_ALWAYS);

    /* Draw output channel box to show DVR */
    pfGetVChanScale(vChan, &xscale, &yscale);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    coords[0][0] = -xscale; coords[0][1] = -yscale;
    coords[1][0] = xscale; coords[1][1] = -yscale;
    coords[2][0] = xscale; coords[2][1] = yscale;
    coords[3][0] = -xscale; coords[3][1] = yscale;
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2fv(coords[0]);
    glVertex2fv(coords[1]);
    glVertex2fv(coords[2]);
    glVertex2fv(coords[3]);
    glEnd();

    pfPopState();
    glDepthFunc(GL_LEQUAL);
    pfApplyFrust(frust);
}

