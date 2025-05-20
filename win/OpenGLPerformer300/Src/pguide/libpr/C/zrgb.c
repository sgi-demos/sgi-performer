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
 *   zrgb.c: basic libpr z-buffered polygons with mouse input.  
 *           based on ~4Dgifts/examples/grafix/misc/zrgb.c
 *
 */

#include <Performer/pr.h>
#include <X11/X.h>
#include <X11/keysym.h>


pfVec3 polygon1[] = {	{-10.0, -10.0,   0.0,},
			{ 10.0, -10.0,   0.0,},
			{-10.0,  10.0,   0.0,},
			{  0.0, -10.0, -10.0,},
			{  0.0, -10.0,  10.0,},
			{  0.0,   5.0, -10.0,} };

pfVec3 polygon2[] = {	{-10.0,   6.0,   4.0,},
			{-10.0,   3.0,   4.0,},
			{  4.0,  -9.0, -10.0,},
			{  4.0,  -6.0, -10.0,} };

pfVec4 colors1[] =  {	{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.5f, 0.5f, 0.5f, 0.0f },
			{ 1.0f, 1.0f, 1.0f, 0.0f },
			{ 1.0f, 1.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.5f, 0.0f },
			{ 0.0f, 0.0f, 1.0f, 0.0f } };

pfVec4 colors2[] =  {	{ 1.0f, 1.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f, 0.0f },
			{ 1.0f, 0.0f, 1.0f, 0.0f } };

int main(void)
{
    pfWindow *win;
    pfFrustum *frust;
    Display *dsp;
    Window xwin;
    pfGeoSet	*gset1,*gset2;
    int mx=0, my=0;
    int mousedown=0;
    int winSizeX, winSizeY;
    float scrnaspect;

    pfInit();
    pfInitState(NULL);
    
    /* Initialize GL */
    win = pfNewWin(NULL);
    pfWinOriginSize(win, 100, 100, 500, 500);
    pfWinName(win, "OpenGL Performer");
    pfWinType(win, PFWIN_TYPE_X);
    pfOpenWin(win);
    pfGetWinSize(win, &winSizeX, &winSizeY);

    /* set up the window */
    frust = pfNewFrust(NULL);
    pfFrustNearFar(frust, 30, 60);
    pfMakeSimpleFrust(frust, 40);
    pfApplyFrust(frust);
    pfDelete(frust);

    /* set up X input event handling */
    dsp = pfGetCurWSConnection();
    xwin = pfGetWinWSWindow(win);
    XSelectInput(dsp, xwin,  StructureNotifyMask | KeyPressMask | 
		    PointerMotionMask | ButtonPressMask | ButtonReleaseMask );
    XMapWindow(dsp, xwin);
    XSync(dsp,FALSE);

    gset1 = pfNewGSet(NULL);
    pfGSetAttr (gset1, PFGS_COORD3, PFGS_PER_VERTEX, polygon1, NULL);
    pfGSetAttr (gset1, PFGS_COLOR4, PFGS_PER_VERTEX, colors1, NULL);
    pfGSetPrimType (gset1, PFGS_TRIS);
    pfGSetNumPrims (gset1, 2);

    gset2 = pfNewGSet(NULL);
    pfGSetAttr (gset2, PFGS_COORD3, PFGS_PER_VERTEX, polygon2, NULL);
    pfGSetAttr (gset2, PFGS_COLOR4, PFGS_PER_VERTEX, colors2, NULL);
    pfGSetPrimType (gset2, PFGS_QUADS);
    pfGSetNumPrims (gset2, 1);

    while (TRUE) 
    {
	int dev;
	short val;
	static pfVec4 clr = {0.15f, 0.39f, 0.78f, 1.0f};
	pfClear(PFCL_COLOR | PFCL_DEPTH, clr);

	pfPushMatrix();
		pfTranslate(0.0, 0.0, -40.0);
		pfRotate(PF_X, (float) mx);
		pfRotate(PF_Y, (float) my - 60.0f);

		pfDrawGSet (gset1);
		pfDrawGSet (gset2);
	pfPopMatrix();

	pfSwapWinBuffers(win);
	
	while (XPending(dsp))
	{
	    XEvent event;
	    XNextEvent(dsp, &event);
	    switch (event.type) 
	    {
	    case ConfigureNotify: /* Have a move/resize event */
	    {
		XConfigureEvent *config_event = (XConfigureEvent *) &event;
		pfWinSize(win, config_event->width, config_event->height);
		pfSelectWin(win);
		break;
	    }
	    case KeyPress:
	    {
		char buf[100];
		int rv;
		KeySym ks;
		rv = XLookupString(&event.xkey, buf, sizeof(buf), &ks, 0);
		switch(ks) 
		{
		case XK_Escape: 
		    exit(0);
		    break;
		}
	    }
	    case ButtonPress: /* Have a mouse button push event */
	    {
		XButtonEvent *button_event = (XButtonEvent *) &event;
		if (button_event->button == Button1)
		{
		    mousedown = 1;
		    mx = event.xbutton.x;
		    my = winSizeY - event.xbutton.y;
		}
		break;
	    }
	    case ButtonRelease: /* Have a mouse button release event */
	    {
		XButtonEvent *button_event = (XButtonEvent *) &event;
		if (button_event->button == Button1)
			mousedown = 0;
		break;
	    }
	    case MotionNotify: /* Have a mouse motion event */
		if (mousedown)
		{
		    XMotionEvent *motion_event = (XMotionEvent *) &event;
		    /* get window relative mouse coordinates */
		    mx =  motion_event->x;
		    /* reverse Y coord so 0 is at bottom of window */
		    my =  winSizeY - motion_event->y;
		}
		break;
	    } /* switch */
	} /* while have event */
    }
    return 0;
}
