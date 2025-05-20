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
 * $Revision: 1.24 $
 * $Date: 2002/08/07 22:31:06 $
 *
 */

#include <stdlib.h>
#include <stdarg.h>
#ifndef WIN32
#include <unistd.h>
#endif
#ifdef sgi
#include <bstring.h>
#else
#include <string.h>
#endif  /* __linux__ */

#ifndef WIN32
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>
#else
#define XC_num_glyphs 154
#endif

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

PFUDLLEXPORT void
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

PFUDLLEXPORT int
pfuGetCursorType(void)
{
    if (!CM)
	initCursorMgmt();
	
    return CM->type;
}

PFUDLLEXPORT void
pfuSelCursor(int c)
{
    if (!CM)
	initCursorMgmt();
	
    if (CM->curXCursor == c)
	return;
    CM->curXCursor = c;
    CM->dirty |= DIRTY_SEL;
}

/* the parameter 'c' to this function really should be an int, not a Cursor. */
PFUDLLEXPORT Cursor
pfuCreateCursor(Cursor c)
{
#ifndef WIN32
    Display *dsp = pfGetCurWSConnection();
#endif

#ifdef WIN32
  LPCTSTR cursorName = NULL;

  switch((int)c) {
    // XXX Alex -- making circle look like plain arrow ... not quite what we want ...
  case PFU_CURSOR_circle: cursorName = IDC_ARROW; break;
  case PFU_CURSOR_clock: cursorName = IDC_APPSTARTING; break;
  case PFU_CURSOR_cross: cursorName = IDC_CROSS; break;
  case PFU_CURSOR_arrow:
  case PFU_CURSOR_mouse: cursorName = IDC_ARROW; break;
    //case PFU_CURSOR_hand2: cursorName = IDC_HAND; break;
  case PFU_CURSOR_question_arrow: cursorName = IDC_HELP; break;
  default:
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"unable to load cursor %d, using arrow",c);
    cursorName = IDC_ARROW;
    break;
  }
#endif /* WIN32 */
    
    if (!CM)
	initCursorMgmt();
	
    if ((int)c > XC_num_glyphs && (int)c != PFU_CURSOR_OFF)
	    pfNotify(PFNFY_NOTICE, PFNFY_USAGE, 
		    "pfuLoadWinCursor - bad cursor index %d", (int)c);
    if ((int)c == PFU_CURSOR_OFF)
	    CM->XCursors[(int)c] = pfuGetInvisibleCursor();
    else 
#ifndef WIN32
	CM->XCursors[c] = XCreateFontCursor(dsp, (unsigned int)c);
#else
        CM->XCursors[(int)c] = LoadCursor( NULL, cursorName);
#endif

#ifndef WIN32
    XFlush(dsp);
#endif

#ifndef WIN32
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
#endif

    return CM->XCursors[(int)c];
}

PFUDLLEXPORT void
pfuLoadWinCursor(pfWindow *w, int c)
{
    Window xwin;
#ifndef WIN32
    Display *dsp;
#endif
    
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

#ifndef WIN32
    dsp = pfGetCurWSConnection();
#endif
    
    if (!CM)
	initCursorMgmt();
	
    if (!(CM->XCursors[(int)c]) && (((int)c == PFU_CURSOR_OFF) || ((int)c < XC_num_glyphs)))
	pfuCreateCursor((int)c);
#ifdef WIN32
    pfWin32Cursor(w,CM->XCursors[c]);
#else
    XDefineCursor(dsp, xwin, CM->XCursors[c]);
    XFlush(dsp);
#endif
}

PFUDLLEXPORT void
pfuLoadPWinCursor(pfPipeWindow *w, int c)
{
    Window xwin;
#ifndef WIN32
    Display *dsp;
#endif
   
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
	
#ifndef WIN32
    dsp = pfGetCurWSConnection();
#endif

    if (!(CM->XCursors[c]) && ((c == PFU_CURSOR_OFF) || (c < XC_num_glyphs)))
	pfuCreateCursor((Cursor)c);

#ifdef WIN32
    pfWin32Cursor(pfGetPWinSelect(w),CM->XCursors[c]);
#else
    XDefineCursor(dsp, xwin, CM->XCursors[c]);
    XFlush(dsp);
#endif /* WIN32 */
}

PFUDLLEXPORT void 
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
#ifdef WIN32
    pfNotify(PFNFY_NOTICE,PFNFY_WARN,"XXX Alex -- pfuGetInvisibleCursor() NYI on win32");
    return 0;
#else
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
#endif
}

PFUDLLEXPORT void
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

PFUDLLEXPORT void	
pfuDrawPWin2DCursor(pfPipeWindow *pw, int mx, int my)
{
    pfPipeVideoChannel *vc = pfGetPWinPosPVChan(pw, mx, my);

    if (!vc)
	vc = pfGetPWinPVChan(pw, 0);
    
    pfuDrawPVChan2DCursor(vc, mx, my);
}

PFUDLLEXPORT void	
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

PFUDLLEXPORT void	
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

PFUDLLEXPORT void	
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

PFUDLLEXPORT void	
pfuDraw2DCursor(int mx, int my)
{
    if (!CM)
	initCursorMgmt();
	
    draw2DCursor(mx, my, 0, 0, 0, 0, 1.0f, 1.0f);
}


PFUDLLEXPORT void	
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
