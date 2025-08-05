/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 */

#define KON_XVC_PLATFORM_PARAM_LOAD_DAC_CURSOR		-3


/*
 * file: cursor.c  
 * --------------
 * 
 * Support for X cursors
 * 
 * $Revision: 1.1 $
 * $Date: 2000/11/21 21:39:36 $
 *
 */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#ifndef __linux__
#include <bstring.h>
#else
#include <string.h>
#endif  /* __linux__ */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>

#include <Performer/pfutil.h>

#include "cursor.h"


/******************************************************************************
 *			    Static Declarations
 ******************************************************************************
 */

static void initCursorMgmt(void);
static void draw2DCursor(int mx, int my, int chxo, int chyo, int wxs, int wys, 
			    float pixXScale, float pixYScale);

static pfDataPool *CursorDP = NULL;
static pfuCursorMgmt *CM = NULL;


/******************************************************************************
 *			    Global Routines
 ******************************************************************************
 */

void
pfuCursorType(int val)
{
    if (!CM)
	initCursorMgmt();
	
    if (CM->type == val)
	return;
    CM->type = val;
    /* if want 3D cursor, must set main X cursor to OFF */
    CM->curXCursor = PFU_CURSOR_OFF;
    CM->dirty |= DIRTY_TYPE;
}

int
pfuGetCursorType(void)
{
    if (!CM)
	initCursorMgmt();
	
    return CM->type;
}

void
pfuSelCursor(int c)
{
    if (!CM)
	initCursorMgmt();
	
    if (CM->curXCursor == c)
	return;
    CM->curXCursor = c;
    CM->dirty |= DIRTY_SEL;
}

Cursor
pfuCreateCursor(Cursor c)
{
    Display *dsp = pfGetCurWSConnection();
    
    if (!CM)
	initCursorMgmt();
	
    if (c > 154 && c != PFU_CURSOR_OFF)
	    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, 
		    "pfuLoadWinCursor - bad cursor index %d", c);
    if (c == PFU_CURSOR_OFF)
	    CM->XCursors[c] = pfuGetInvisibleCursor();
    else 
	CM->XCursors[c] = XCreateFontCursor(dsp, (unsigned int)c);
    XFlush(dsp);
    if (CM->dirty & DIRTY_COLOR)
    {
	XColor      c1, c2;
	c1.red = 0xffff * CM->fgColor[0];
	c1.green = 0xffff * CM->fgColor[1];
	c1.blue = 0xffff * CM->fgColor[2];
	c2.red = 0xffff * CM->bgColor[0];
	c2.green = 0xffff * CM->bgColor[1];
	c2.blue = 0xffff * CM->bgColor[2];
	XRecolorCursor( dsp, CM->XCursors[c], &c1, &c2 );
	CM->dirty &= ~DIRTY_COLOR;
    }
    return CM->XCursors[c];
}

void
pfuLoadWinCursor(pfWindow *w, int c)
{
    Window xwin;
    Display *dsp;
    
    if (!w)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		    "pfuLoadWinCursor - NULL pfWindow*");
	return;
    }
    xwin = pfGetWinWSWindow(w);
    if (!xwin)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		    "pfuLoadWinCursor - pfWindow has not been opened.");
	return;
    }
    if (!(pfGetWinType(w) & PFPWIN_TYPE_X))
	return;
    
    dsp = pfGetCurWSConnection();
    
    if (!CM)
	initCursorMgmt();
	
    if (!(CM->XCursors[c]) && ((c == PFU_CURSOR_OFF) || (c < XC_num_glyphs)))
    {
	pfuCreateCursor(c);
    }
    XDefineCursor(dsp, xwin, CM->XCursors[c]);
    XFlush(dsp);
}

void
pfuLoadPWinCursor(pfPipeWindow *w, int c)
{
    Window xwin;
    Display *dsp;
   
    if (!w)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		    "pfuLoadWinCursor - NULL pfWindow*");
	return;
    }
    xwin = pfGetPWinWSWindow(w);
    if (!xwin)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		    "pfuLoadWinCursor - pfWindow has not been opened.");
	return;
    }
    if (!(pfGetPWinType(w) & PFPWIN_TYPE_X))
	return;

    if (!CM)
	initCursorMgmt();
	
    dsp = pfGetCurWSConnection();
    if (!(CM->XCursors[c]) && ((c == PFU_CURSOR_OFF) || (c < XC_num_glyphs)))
    {
	pfuCreateCursor(c);
    }
    XDefineCursor(dsp, xwin, CM->XCursors[c]);
    XFlush(dsp);
}

void 
pfuLoadChanCursor(pfChannel *chan, int c)
{
    if (!CM)
	initCursorMgmt();
	
    switch(CM->type)
    {
    case PFU_CURSOR_X:
	{
	    pfPipeWindow *pw = pfGetChanPWin(chan);
	    pfuLoadPWinCursor(pw, c);
	    break;
	}
    }
}

Cursor
pfuGetInvisibleCursor(void)
{
    Cursor curs;
    /* cursor mask for invisible cursor */
    XColor xclr = {0,0,0,0,0,0};
    char *cursoffdata = calloc(1, 16*16*(sizeof(char)));
    Pixmap pix;
    Display		*dsp;
    
    if (!CM)
	initCursorMgmt();
	
    if (CM->invisibleCursor)
	return CM->invisibleCursor;

    dsp = pfGetCurWSConnection();
    pix = XCreateBitmapFromData(dsp, 
		RootWindow(dsp, DefaultScreen(dsp)),
		cursoffdata, 16, 16);
    curs = XCreatePixmapCursor(dsp, pix, pix, 
		    &xclr, &xclr, 0, 0);
    XFreePixmap(dsp, pix);
    free(cursoffdata);
    CM->invisibleCursor = curs;
    return curs;
}

void
pfuCursor(Cursor c, int index)
{
    if (!CM)
	initCursorMgmt();
	
    CM->XCursors[index] = c;
}

Cursor
pfuGetCursor(int index)
{
    if (!CM)
	initCursorMgmt();
	
    return CM->XCursors[index];
}

void	
pfuDrawPWin2DCursor(pfPipeWindow *pw, int mx, int my)
{
    pfPipeVideoChannel *vc = pfGetPWinPosPVChan(pw, mx, my);

    if (!vc)
	vc = pfGetPWinPVChan(pw, 0);
    
    pfuDrawPVChan2DCursor(vc, mx, my);
}

void	
pfuDrawPVChan2DCursor(pfPipeVideoChannel *pVChan, int mx, int my)
{
    float xs, ys;
    int xo, yo, wxs, wys;
    pfPipeWindow *pwin =  pfGetPVChanPWin(pVChan);

    if (!CM)
	initCursorMgmt();
	
    pfGetPVChanScale(pVChan, &xs, &ys);
    pfGetPVChanOrigin(pVChan, &xo, &yo);
    pfGetPWinSize(pwin, &wxs, &wys);
    draw2DCursor(mx, my, xo, yo, wxs, wys, xs, ys);
}

void	
pfuDrawVChan2DCursor(pfVideoChannel *vChan, int mx, int my)
{
    float xs, ys;
    int xo, yo, wxs, wys;
    pfWindow *win;

    if (!CM)
	initCursorMgmt();
	
    pfGetVChanScale(vChan, &xs, &ys);
    pfGetVChanOrigin(vChan, &xo, &yo);
    if ((win = pfGetCurWin()) && pfIsWinOpen(win))
	pfGetWinSize(win, &wxs, &wys);
    else
	pfGetVChanSize(vChan, &wxs, &wys);
    draw2DCursor(mx, my, xo, yo, wxs, wys, xs, ys);
}

void	
pfuDrawChan2DCursor(pfChannel *chan, int mx, int my)
{
    float xs, ys;
    int wxs, wys;
    int   chxo, chyo, chxs, chys;
    pfPipeVideoChannel *vc = pfGetChanPVChan(chan);
    pfPipeWindow *pwin =  pfGetChanPWin(chan);
    
    if (!CM)
	initCursorMgmt();
    
    /* get channel origin and size to test mouse against */
    pfGetChanOutputOrigin(chan,  &chxo,  &chyo);
    pfGetChanOutputSize(chan,  &chxs,  &chys);
    pfGetPWinSize(pwin, &wxs, &wys);
    /* if mouse is not in channel, don't draw it */
    if (!((mx > chxo) && (my > chyo) && 
	(mx < chxo + chxs) && (my < chyo + chys)))
	return;
    
    pfGetPVChanScale(vc, &xs, &ys);
    draw2DCursor(mx, my, chxo, chyo, wxs, wys, xs, ys);
}

void	
pfuDraw2DCursor(int mx, int my)
{
    if (!CM)
	initCursorMgmt();
	
    draw2DCursor(mx, my, 0, 0, 0, 0, 1.0f, 1.0f);
}


void	
pfuCursorColor(pfVec3 fg, pfVec3 bg)
{
    if (!CM)
	initCursorMgmt();
	
    PFCOPY_VEC3(CM->fgColor, fg);
    PFCOPY_VEC3(CM->bgColor, bg);
    CM->dirty |= DIRTY_COLOR;
}


/******************************************************************************
 *			    Static Routines
 ******************************************************************************
 */


static void
initCursorMgmt(void)
{
    if (!CursorDP)
	CursorDP = pfuGetUtilDPool();

    if (!(CM = (pfuCursorMgmt *)  pfDPoolFind(CursorDP, PFU_CURSOR_DATA_DPID)))
    {
	pfNotify(PFNFY_FATAL, PFNFY_SYSERR, 
	    "initCursorMgmt - didn't find pfuCursorMnger in Util Data Pool.");
	return;
    }
    PFSET_VEC3(CM->bgColor, 1.0f, 0.0f, 0.0f);
    PFSET_VEC3(CM->fgColor, 1.0f, 1.0f, 1.0f);

}

#define CURSOR_3D_SIZE 12
#define CURSOR_3D_HALFSIZE (CURSOR_3D_SIZE >> 1)
static void
draw2DCursor(int mx, int my, int chxo, int chyo, int wxs, int wys, 
	float pixXScale, float pixYScale)
{
    int pixXSize2 = (CURSOR_3D_HALFSIZE * pixXScale) + 0.5f;
    int pixYSize2 = (CURSOR_3D_HALFSIZE * pixYScale) + 0.5f;
    int pixXSize = pixXSize2 * 2;
    int pixYSize = pixYSize2 * 2;
    float pixScale = PF_MAX2(pixXScale, pixYScale);
    pfMatrix	 proj;
    static int lp[4][2] = { {-1, 0}, {1, 0}, 
			    {0, -1}, {0, 1} };

    mx = chxo + ((mx - chxo) * pixXScale + 0.5f);
    my = chyo + ((my - chyo) * pixYScale + 0.5f);

    pfPushIdentMatrix();
    pfPushState();
    pfBasicState();

    glDepthFunc(GL_ALWAYS);
    glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
    /* make tiny viewport around mouse pos */
    glMatrixMode(GL_PROJECTION);
    glGetFloatv(GL_PROJECTION_MATRIX, (float*) proj);
    glLoadIdentity();

    if (wxs && wys)
    {
	/* cursor position will be more stable under video resize if we always fix the
	 * viewport to the window boundaries instead of trying arbitrary pixel centers 
	 */
	glViewport(0, 0, wxs, wys);
	glScissor(mx - pixXSize2,  my - pixYSize2, pixXSize, pixYSize); 
	glEnable(GL_SCISSOR_TEST);
	glOrtho(0,wxs,0,wys,-1,1);
	pfTranslate(mx, my, 0);
	pfScale((float)CURSOR_3D_HALFSIZE,(float)CURSOR_3D_HALFSIZE,1.0f);
    } 
    else
    {
	glViewport(mx - pixXSize2,  my - pixYSize2, pixXSize, pixYSize);
	glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    }
    glMatrixMode(GL_MODELVIEW);


    glColor3fv(CM->bgColor);
    glLineWidth(3.0f * pixScale);
    glBegin(GL_LINES);
    glVertex2iv(lp[0]);
    glVertex2iv(lp[1]);
    glVertex2iv(lp[2]);
    glVertex2iv(lp[3]);
    glEnd();
    glColor3fv(CM->fgColor);
    glLineWidth(1.0f * pixScale);
    glBegin(GL_LINES);
    glVertex2iv(lp[0]);
    glVertex2iv(lp[1]);
    glVertex2iv(lp[2]);
    glVertex2iv(lp[3]);
    glEnd();
    glDepthFunc(GL_LEQUAL);
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    pfLoadMatrix(proj);
    glMatrixMode(GL_MODELVIEW);
    pfPopMatrix();
    pfPopState();
}
