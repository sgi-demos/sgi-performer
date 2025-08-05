/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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
 */


/*
 * file: xwin.c  
 * --------------
 * 
 * $Revision: 1.1 $
 *
*/

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#ifndef __linux__
#include <bstring.h>
#endif  /* __linux__ */
#include <math.h>
#include <sys/types.h>


#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include "xwin.h"

static pfuGLXWindowData *pfuInitXWinDataPool(void);
static pfuGLXWindow*	addGLXWin(pfPipe *_pipe, pfPipeWindow *pw, Display *_dsp, 
			    Window _xwin, pfGLContext _glwin,  Window _overWin);
static pfuGLXWindow*	getGLXWin(pfPipe *_pipe);
static int		getGLXIndex(pfPipe *_pipe);
static int *findFBAttr(int *buf, int attr);
static pfFBConfig makeSharedFB(pfFBConfig src, void *arena);
static XVisualInfo* makeVisualInfo(Display* D, int screen, int id, void *arena);
static pfuGLXWindow	*GLXWin = NULL;
static pfuGLXWindowData *GLXData = NULL;
static pfPipe		**GLXPipe = NULL;

static XVisibilityEvent *glWinVisibilityP = NULL;

	/*-------------------------------------------------------*/

static pfuGLXWindowData *
pfuInitXWinDataPool(void)
{
    if (!GLXData)
    
    while (!GLXData)
    {
	int nTrys = 0;

	GLXData = (pfuGLXWindowData *)pfuFindUtilDPData(PFU_XWIN_DATA_DPID);
	
	if (GLXData)
	    break;

	pfNotify(PFNFY_DEBUG, PFNFY_RESOURCE, 
	      	 "pfuInitXWinDataPool() Util DataPool does not exist - retrying...");

	sginap(1);

	/* Keep Trying for approx 5 seconds */
	if (nTrys++ > 500)
	{
	    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, "pfuInitXWinDataPool() Could not "
		     "find pfuGLXWindowData structure for approx 5 seconds - exiting");
	    exit(1);
	}
    }

    GLXWin = &(GLXData->GLXWin[0]);
    GLXPipe = &(GLXData->pipe[0]);
    
    if (!glWinVisibilityP)
    {
	glWinVisibilityP = (XVisibilityEvent*)pfCalloc(1, 
					    sizeof(XVisibilityEvent), 
					    pfGetSharedArena());
	glWinVisibilityP->type = VisibilityNotify;
	glWinVisibilityP->send_event = TRUE;
	glWinVisibilityP->state = VisibilityUnobscured;
    }
    return GLXData;
}

void
pfuGetGLXWin(pfPipe *p, pfuGLXWindow *win)
{
    if (p)
    {
	int id = pfGetId(p);
	
	if (GLXData || pfuInitXWinDataPool())
	{
	    pfDPoolLock(GLXData);
	    *win = GLXWin[id];
	    pfDPoolUnlock(GLXData);
	}
    } else {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfuGetGLXWin(pfPipe*) - NULL pipe");
    }
}

const char*
pfuGetGLXDisplayString(pfPipe *p)
{
    if (p)
    {
	int id = pfGetId(p);
	if (GLXData || pfuInitXWinDataPool())
	    return (GLXData->dspString[id]);
	else {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfuGetGLXDisplayString() - can't connect to data pool.");
    }
    } else {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfuGetGLXDisplayString(pfPipe*) - NULL pipe");
    }
    return NULL;
}

/*
 * addGLXWin() this is used to add new pipe windows and also to get the pointer
 * to pipes already initialized to change their windows.
 */
static pfuGLXWindow*
addGLXWin(pfPipe *p, pfPipeWindow *pw, Display *dsp, 
	Window xWin, pfGLContext glWin, Window overWin)
{
    int n;
    
    n = getGLXIndex(p);
    if (n >= MAX_PIPES)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfuGLXWinopen() - can only open %d GLX windows.", MAX_PIPES);
	return NULL;
    }
    
    pfDPoolLock(GLXData);
    
    if (!GLXPipe[n])
    {
	GLXPipe[n] = p;
	strcpy(GLXData->dspString[n], DisplayString(dsp));
	GLXData->numPipes++;
    }
    GLXWin[n].pw = pw;
    GLXWin[n].glWin = (pfuXWindow) glWin;
    GLXWin[n].overWin = (pfuXWindow) overWin;
	    
    GLXWin[n].dsp = (pfuXDisplay *) dsp;
    GLXWin[n].xWin = (pfuXWindow) xWin;
    
    pfDPoolUnlock(GLXData);
    return (&GLXWin[n]);
}
static pfuGLXWindow*
getGLXWin(pfPipe *p)
{
    int n = getGLXIndex(p);
    return (&(GLXWin[n]));
}

static int
getGLXIndex(pfPipe *p)
{
    int n;
    
    if (!GLXData && !pfuInitXWinDataPool())
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfuGLXWinopen() - can't connect to data pool.");
	return NULL;
    }
    n  = pfGetId(p);
    return n;
}

pfuXDisplay *
pfuOpenXDisplay(int screen)
{
    Display *XDpy=NULL;

    if (!(XDpy = pfOpenScreen(screen, TRUE)))
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, 
	 "pfuOpenXDisplay() couldn't open display on screen %d", screen);
    return (pfuXDisplay *)XDpy;
}

pfuGLXWindow*
pfuGLXWinopen(pfPipe *p, pfPipeWindow* pw, const char *name)
{
    Display 			*XDpy;
    pfWindow			*pfOverWin;
    Window 			xWin, overWin;
    pfuGLXWindow 		*glxWin;
    int 			screen, winType;
    pfGLContext			glcxt;
    
    if (!pw)
	pw = pfNewPWin(p);
	
    XDpy = pfGetCurWSConnection();
    if (((screen = pfGetPipeScreen(p)) < 0) && 
	((screen = pfGetPWinScreen(pw) < 0)))
	screen = DefaultScreen(XDpy);
    if (screen < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "CreateGLXWins: could not connect to display.");
	    return NULL;
    }
    winType = pfGetPWinType(pw);
    pfPWinType(pw, winType | PFPWIN_TYPE_X);
    if (name)
	pfPWinName(pw, name);
    pfPWinMode(pw, PFWIN_HAS_OVERLAY, 1);
    pfPWinMode(pw, PFWIN_HAS_STATS, 1);
    pfPWinShare(pw,PFWIN_SHARE_GL_OBJS);
    if (!(pfOverWin = pfGetPWinOverlayWin(pw)))
    {
	char str[PF_MAXSTRING];
	pfOverWin = pfNewWin(pfGetSharedArena());
	sprintf(str, "%s - Overlay", pfGetPWinName(pw));
	pfWinName(pfOverWin, str);
	pfWinType(pfOverWin, PFWIN_TYPE_OVERLAY);
	pfPWinOverlayWin(pw, pfOverWin);
    }

    pfOpenPWin(pw);
    xWin = pfGetPWinWSWindow(pw);
    if (!pfOverWin)
	pfOverWin = pfGetPWinOverlayWin(pw);
    /* check that we were really able to get overlays */
    if (pfOverWin && (pfGetWinGLCxt(pfOverWin)))
    {
	overWin = pfGetWinWSDrawable(pfOverWin);
    }
    else
    {
	overWin = NULL;
	if (pfOverWin)
	    pfDelete((pfObject*)pfOverWin);
	pfPWinOverlayWin(pw, NULL);
	pfPWinMode(pw, PFWIN_HAS_OVERLAY, 0);
    }
    glcxt = pfGetPWinGLCxt(pw);
    
    /* this will put ptrs in shared mem that the input process can see
     * and will trigger the input process to start
     */
    if (!(glxWin = addGLXWin(p, pw, XDpy, xWin, glcxt, overWin)))
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfuGLXWinopen() - could not get pfuGLXWindow from data pool.");
    }
    return glxWin;
}


int
pfuGLXAllocColormap(pfuXDisplay *dsp, pfuXWindow w)
{
    int max, result;
    XWindowAttributes wa;
    unsigned long *buf;
    
    XGetWindowAttributes((Display*)dsp, (Window)w, &wa);
    max = powf(2, wa.depth);
    buf = (unsigned long *)calloc(max, sizeof(unsigned long));
    if (!(result = XAllocColorCells((Display*)dsp,  wa.colormap, 
				True, NULL, 0, buf, max-1)))
	result = XAllocColorCells((Display*)dsp,  wa.colormap, 
		True, NULL, 0, buf, max-1);
    if (result > 0)
	return max;
    else
	return result;
}

void
pfuGLXMapcolors(pfuXDisplay *dsp, pfuXWindow w, pfVec3 *clrs, int start, int num)
{
    Display *xdsp = (Display*)dsp;
    XWindowAttributes wa;
    XColor clr;
    int i,c;
    
    XGetWindowAttributes(xdsp, (Window)w, &wa);
    for (i=0, c=start; i < num; i++, c++)
    {
	clr.pixel = c;
	clr.red = (short)(clrs[i][0]*65535); 
	clr.green = (short)(clrs[i][1]*65535); 
	clr.blue = (short)(clrs[i][2]*65535); 
	clr.flags = DoRed | DoGreen | DoBlue;
	XStoreColor((Display*)dsp, wa.colormap, &clr);
    }
    XMapWindow(xdsp, w);
    XSync(xdsp, FALSE);
}

/*ARGSUSED*/
void
pfuGLMapcolors(pfVec3 *clrs, int start, int num)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfuGLMapcolors is only for IRIS GL.  "
					"Use pfuMapWinColors");
}

void
pfuMapWinColors(pfWindow *w,  pfVec3 *clrs, int start, int num)
{
    if (!w) 
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuMapWinColors - NULL pfWindow");
	return;
    }
    {
	pfWSWindow xwin = pfGetWinWSDrawable(w);
	pfuGLXMapcolors((pfuXDisplay *)pfGetCurWSConnection(), xwin, clrs, start, num);
    }
}

void
pfuMapPWinColors(pfPipeWindow *pwin,  pfVec3 *clrs, int start, int num)
{
    if (!pwin) 
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuMapWinColors - NULL pfPipeWindow");
	return;
    }
    {
	pfWSWindow xwin = pfGetPWinWSDrawable(pwin);
	pfuGLXMapcolors((pfuXDisplay *)pfGetCurWSConnection(), xwin, clrs, start, num);
    }
}

#define NUM_QWIN_TOKS 10
static int toks[NUM_QWIN_TOKS+1] = {
    PFQWIN_RGB_BITS,
    PFQWIN_ALPHA_BITS,
    PFQWIN_CI_BITS,
    PFQWIN_DEPTH_BITS,
    PFQWIN_MIN_DEPTH_VAL, 
    PFQWIN_MAX_DEPTH_VAL, 
    PFQWIN_MS_SAMPLES,
    PFQWIN_STENCIL_BITS,
    PFQWIN_DOUBLEBUFFER,
    PFQWIN_STEREO,
    NULL
};
static char strs[NUM_QWIN_TOKS+1][32] = {
    "RGB_BITS",
    "ALPHA_BITS",
    "CI_BITS",
    "DEPTH_BITS",
    "MIN_DEPTH_VAL", 
    "MAX_DEPTH_VAL", 
    "MS_SAMPLES",
    "STENCIL_BITS",
    "DOUBLEBUFFER",
    "STEREO",
    NULL
};


void
pfuPrintWinFBConfig(pfWindow *win, FILE *_file)
{
    static int dst[NUM_QWIN_TOKS];
    int i;

    if (_file == NULL)
	_file = stderr;
    
    pfMQueryWin(win, toks, dst);
    fprintf(_file, 
	    "pfWindow Framebuffer Configuration: %s\n", 
		pfGetWinName(win));
    for (i=0; i < (NUM_QWIN_TOKS); i++)
	fprintf(_file, "\t%s: %d\n", strs[i], dst[i]);
}   


void
pfuPrintPWinFBConfig(pfPipeWindow *pwin, FILE *_file)
{
    static int dst[NUM_QWIN_TOKS];
    int i;

    if (_file == NULL)
	_file = stderr;
    
    pfMQueryPWin(pwin, toks, dst);
    fprintf(_file, 
	    "pfPipeWindow Framebuffer Configuration: %s\n", 
		pfGetPWinName(pwin));
    for (i=0; i < (NUM_QWIN_TOKS); i++)
	fprintf(_file, "\t%s: %d\n", strs[i], dst[i]);
}   

pfFBConfig
pfuChooseFBConfig(Display *dsp,  int screen, int *constraints, void *arena)
{
    int *c;
    int n, i, t, r, numv, nextVis;
    /* these get limited */
    int rgb= -8, z= -24, samples= -8, stencil= -4; 
    int violation = 0;
    /* these we maximize */
    int alpha=0, ci=0, level=0, sbuf=0, aux=0;
    int accumRGB=0, accumAlpha=0; 
    /* default is single buffered */
    int dbl = 0, stereo=0; 
    int priorities[32], nump=0;
    XVisualInfo	tmplt, *all, *ret=NULL, *vis[256];

    if (!dsp)
	dsp = pfGetCurWSConnection();
    if (screen < 0)
	screen = DefaultScreen(dsp);

    tmplt.screen = screen;
    if (!constraints || findFBAttr(constraints, PFFB_RGBA))
	tmplt.class = TrueColor;
    else
	tmplt.class = PseudoColor;
    all =  XGetVisualInfo (dsp, 
	    VisualScreenMask | VisualClassMask, 
	    &tmplt, &n);

    numv = 0;
    /* loop through all of the visuals and find all those that meet constraints */
    for (i=0; i < n; i++)
    {
	glXGetConfig(dsp, &all[i], GLX_USE_GL, &t);
	if (!t)
	    continue;
	    
	nextVis = 0;
	c = constraints;
	while (c && *c && !nextVis)
	{
	    glXGetConfig(dsp, &all[i], *c, &t);
	    switch (*c)
	    {
	    /* Booleans */
	    case PFFB_USE_GL:
	    case PFFB_DOUBLEBUFFER:
	    case PFFB_STEREO:
	    case PFFB_RGBA:
		if (!t)
		    nextVis = 1;
		else
		    c++;
		break;
	    /* Level must be exact */
	    case PFFB_LEVEL:
		if (t != *(c+1))
		    nextVis = 1;
		else
		    c+=2 ;
		break;
	    /* values to meet or maximize */
	    case PFFB_BUFFER_SIZE:
	    case PFFB_AUX_BUFFERS:
	    case PFFB_RED_SIZE:
	    case PFFB_GREEN_SIZE:
	    case PFFB_BLUE_SIZE:
	    case PFFB_ALPHA_SIZE:
	    case PFFB_DEPTH_SIZE:
	    case PFFB_STENCIL_SIZE:
	    case PFFB_ACCUM_RED_SIZE:
	    case PFFB_ACCUM_GREEN_SIZE:
	    case PFFB_ACCUM_BLUE_SIZE:
	    case PFFB_ACCUM_ALPHA_SIZE:
	    case PFFB_SAMPLES:
	    case PFFB_SAMPLE_BUFFER:
		if (t < *(c+1))
		    nextVis = 1;
		else
		    c+=2 ;
		break;
	    default: 
		pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		"_pfChooseXVisual: unknown token 0x%x. Use PFFB_ token.", *c);
		return NULL;
	    }
	}
	/* if the visual works, at it to the list */
	if (!nextVis && (!c || !(*c)))
	{
	    vis[numv] = &(all[i]);
	    numv++;
	}
    }
    /* nothing worked - return NULL */
    if (!numv)
	return NULL;
    ret = vis[0];
    numv--;
    /* one worked so return it */
    if (!numv)
	return ret;
    /* go through contraints and calculate limits and set priorities */
    c = constraints;
    priorities[0] = 0;
    while (c && *c)
    {
	switch (*c)
	{
	/* Booleans */
	case PFFB_DOUBLEBUFFER:
	    dbl = 1;
	    c++;
	    break;
	case PFFB_STEREO:
	    stereo = 1;
	    c++;
	    break;
	case PFFB_USE_GL:
	case PFFB_RGBA:
	    c++;
	    break;
	/* values */
	case PFFB_BUFFER_SIZE:
	    if (*(c+1) > ci)
		ci = *(c+1);
	    priorities[nump++] = (*c);
	    c += 2;
	    break;
	case PFFB_LEVEL:
	    level = *(c+1);
	    c += 2;
	case PFFB_AUX_BUFFERS:
	    if (*(c+1) > aux)
		aux = *(c+1);
	    priorities[nump++] = (*c);
	    c += 2;
	    break;
	case PFFB_RED_SIZE:
	case PFFB_GREEN_SIZE:
	case PFFB_BLUE_SIZE:
	    if (*(c+1) > rgb)
		rgb = *(c+1);
	    priorities[nump++] = (*c);
	    c += 2;
	    break;
	case PFFB_ALPHA_SIZE:
	    if (*(c+1) > alpha)
		alpha = *(c+1);
	    priorities[nump++] = (*c);
	    c += 2;
	    break;
	case PFFB_DEPTH_SIZE:
	    if (*(c+1) > z)
		z = *(c+1);
	    priorities[nump++] = (*c);
	    c += 2;
	    break;
	case PFFB_STENCIL_SIZE:
	    if (*(c+1) > z)
		z = *(c+1);
	    priorities[nump++] = (*c);
	    c += 2;
	    break;
	case PFFB_ACCUM_RED_SIZE:
	case PFFB_ACCUM_GREEN_SIZE:
	case PFFB_ACCUM_BLUE_SIZE:
	    if (*(c+1) > accumRGB)
		accumRGB = *(c+1);
	    priorities[nump++] = (*c);
	    c += 2;
	    break;
	case PFFB_ACCUM_ALPHA_SIZE:
	    if (*(c+1) > accumAlpha)
		accumAlpha = *(c+1);
	    priorities[nump++] = (*c);
	    c += 2;
	    break;
	case PFFB_SAMPLES:
	    if (*(c+1) > samples)
		samples = *(c+1);
	    priorities[nump++] = (*c);
	    c += 2;
	    break;
	case PFFB_SAMPLE_BUFFER:
	    if (*(c+1) > sbuf)
		sbuf = *(c+1);
	    priorities[nump++] = (*c);
	    c += 2;
	    break;
	default: 
	    c = 0;
	    break;
	}
    }
    /* mark end of array */
    priorities[nump] = 0; 
    
    /* only do samples and limiting for level 0 (normal drawing) visuals
     * not an issues for overlay and underlay visuals
     */
    if (level == 0)
    {
	{
	    int maxs;
	    pfQuerySys(PFQSYS_MAX_MS_SAMPLES, &maxs);
	    if (maxs == 0)
		samples = 0;
	}
	if (samples)
	{
	    
	    /* check for default limts */
	    {
		if (z < 0)
		    z = -z;
		if (rgb < 0)
		    rgb = -rgb;
		if (stencil < 0)
		    stencil = -stencil;
		if (samples < 0)
		    samples = -samples;
	    }
	}	
    }
    /* of the visuals that work, find the best one while limiting 
     * Z, RGB, samples, Z and stencil
     */
    ret = 0;
    violation = 1;
    for (i=0; i < numv; i++)
    {
	nextVis = 0;
	/* normal drawing visual */
	if (level == 0) 
	{
	    /* check limits is doing multisampling */
	    if (samples)
	    {
		glXGetConfig(dsp, vis[i], PFFB_DEPTH_SIZE, &t);
		if (t > z)
		    continue;
		if (ret)
		{
		    glXGetConfig(dsp, ret, PFFB_DEPTH_SIZE, &r);
		    if (t > r)
			continue;
		}
		glXGetConfig(dsp, vis[i], PFFB_STENCIL_SIZE, &t);
		if (t > stencil)
		    continue;
		if (ret)
		{
		    glXGetConfig(dsp, ret, PFFB_STENCIL_SIZE, &r);
		    if (t > r)
			continue;
		}
		if (!ci)
		{
		    glXGetConfig(dsp, vis[i], PFFB_RED_SIZE, &t);
		    if (t > rgb)
			continue;
		    if (ret)
		    {
			glXGetConfig(dsp, ret, PFFB_RED_SIZE, &r);
			if (t > r)
			    continue;
		    }
		    glXGetConfig(dsp, vis[i], PFFB_SAMPLES, &t);
		    if (t > samples)
			continue;
		    if (ret)
		    {
			glXGetConfig(dsp, ret, PFFB_SAMPLES, &r);
			if (t > r)
			    continue;
		    }
		}
	    }
	}
	glXGetConfig(dsp, vis[i], PFFB_DOUBLEBUFFER, &r);
	if (r != dbl)
	    continue;
	
	violation = 0;
	
	if (!ret)
	{
	    ret = vis[i];
	    nextVis = 1;
	}
	else
	{
	    /* maximize in priority order established by order in their array */
	    c = priorities;
	    while (c && (*c) && !nextVis)
	    {
		glXGetConfig(dsp, vis[i], *c, &t);
		glXGetConfig(dsp, ret, *c, &r);
		if (t > r)
		{
		    ret = vis[i];
		    nextVis = 1;
		}
		c++;
	    }
	}
    }
    /* if they all violate the limits, minimize in priority order */
    if (violation)
    {

	for (i=1; i < numv; i++)
	{
	    nextVis = 0;
	    /* check limits */
	    glXGetConfig(dsp, vis[i], PFFB_DEPTH_SIZE, &t);
	    glXGetConfig(dsp, ret, PFFB_DEPTH_SIZE, &r);
	    if (t < r)
	    {
		ret = vis[i];
		continue;
	    }
	    /* Should RGB be minimized before samples ???? */
	    glXGetConfig(dsp, vis[i], PFFB_SAMPLES, &t);
	    glXGetConfig(dsp, ret, PFFB_SAMPLES, &r);
	    if (t > r || t > samples)
	    if (t < r)
	    {
		ret = vis[i];
		continue;
	    }
	    glXGetConfig(dsp, vis[i], PFFB_RED_SIZE, &t);
	    glXGetConfig(dsp, ret, PFFB_RED_SIZE, &r);
	    if (t > r || t > rgb)
	    if (t < r)
	    {
		ret = vis[i];
		continue;
	    }
	    glXGetConfig(dsp, vis[i], PFFB_STENCIL_SIZE, &t);
	    glXGetConfig(dsp, ret, PFFB_STENCIL_SIZE, &r);
	    if (t > r || t > stencil)
	    if (t < r)
	    {
		ret = vis[i];
		continue;
	    }
	}
    }
    if (ret)
    { 
	/* copy off our return visual and then free all the X data */
	ret =  makeVisualInfo(dsp, screen, (int)ret->visualid, arena);
    }    
    if (all)
    	XFree(all);
    return ret;
}

/******************************************************************************
 *		Some of our own GLX utilities
 ******************************************************************************
 */
 

static int *
findFBAttr(int* buf, int attr)
{
    int	i;
    if (buf)
	for (i = 0; buf[i]; i++)
	    if (buf[i] == attr)
		return &(buf[i]);
    return NULL;
}

static XVisualInfo*
makeVisualInfo(Display* D, int screen, int id, void *arena)
{
    XVisualInfo tmplt, *ret;
    int n;
    
    tmplt.screen = screen;
    tmplt.visualid =id;
    ret = XGetVisualInfo (D, VisualScreenMask|VisualIDMask, &tmplt, &n);
    if (arena)
	return makeSharedFB(ret, arena);
    else
	return ret;
}
static pfFBConfig
makeSharedFB(pfFBConfig src, void *arena)
{
    pfFBConfig ret=NULL;
    if (src)
    {
	ret = (pfFBConfig) pfCalloc(1, sizeof(XVisualInfo), arena);
	*ret = *src;
	XFree(src);
    }
    return ret;
}
