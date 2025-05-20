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
 * file: gui.c
 * -----------
 *
 * Simple gui for Performer sample app.
 * Uses pfDataPools to store GUI data so that it is MP safe.
 *
 * $Revision: 1.227 $
 * $Date: 2002/11/07 01:49:06 $
 *
 */

#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#ifdef mips
#include <bstring.h>
#endif 
#include <math.h>
#ifndef WIN32
#include <X11/cursorfont.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef WIN32
#include <X11/X.h>
#endif

#ifdef WIN32
#define powf pow
#define log10f log10
#endif
 
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include "gui.h"
#include "pup.h"

/******************************************************************************
 *			    Defines and Constants
 ******************************************************************************
 */

#define MAP_XMOUSE(mx)	((((mx - GUI->pixOrgX) - GUI->transX) / GUI->scaleX) )
#define MAP_YMOUSE(my)	((((my - GUI->pixOrgY) - GUI->transY) / GUI->scaleY) )

#define PFUGUI_REFRESH	2	/* Draw into both front and back buffers */

/* Cursor constants */
#define PFU_CURSOR_IN_MAIN 	0
#define PFU_CURSOR_IN_GUI 	1

/* mode bitmasks */
#define STICKY_DFT 0x1

/******************************************************************************
 *			    Static Variables
 ******************************************************************************
 */

static pfuGUI 		*GUI = NULL;
static pfDataPool 	*GUIDP = NULL;

/* static data - for Draw Process */
static pfVec3   OnColor;
static pfVec3   OffColor;
static pfVec3   SliderColor;
static pfVec3   InactiveSliderColor;
static pfVec3   ValColor;
static pfVec3   BackColor;
static pfVec3   TextColor;
static pfVec3   MessageColor;
static pfVec3   InactiveTextColor;
static pfVec4   ClearColor;

/*
 * Font stuff
 *  XXX Currently the font stuff is kept in static arrays.
 *	This means that, when scaling fonts,  only the draw process knows about
 *	The current font sizes.
 *	The font arrays need to be put in the GUI data pool !!!
 *
 */


#define FONT_SIZE_TREE 	(PFU_FONT_BIG)
#define FONT_SIZE_TITLE	(PFU_FONT_BIG)
#define FONT_SIZE_VAL 	(PFU_FONT_MED)
#define FONT_SIZE_TEXT 	(PFU_FONT_SMALL)
#define NUM_FONT_SIZES (PFU_NUM_FONTS+1)

#define NUMBER_FONTS	0
#define TEXT_FONTS	0
#define TREE_FONTS	1
#define NUM_GUI_FONTS   2


/* Message fonts are for pfuDrawMessage and are scaled separately from GUI 
 * and Message fonts are ONLY maintained in the draw process !!!
 */
#define MESSAGE_FONTS	2
#define NUM_TOTAL_FONTS	3



/* Font Management Flags */
#define FONT_SCALE_DIRTY    0x1
#define FONT_DRAW_DIRTY	    0x2
#define GUI_FONT_DIRTY	    0x3
#define MESSAGE_FONT_SCALE_DIRTY 0x4
#define MESSAGE_FONT_DIRTY	    0x3
#define FONT_DIRTY	    0x7

#ifndef WIN32
#define NUM_FONT_NAME		"utopia-medium-r"
#define TEXT_FONT_NAME		"utopia-bold-r"
#define FANCY_FONT_NAME		"utopia-bold-i"
#else
#define NUM_FONT_NAME		"Courier New-medium-r"
#define TEXT_FONT_NAME		"Courier New-bold-r"
#define FANCY_FONT_NAME		"Courier New-bold-i"
#endif

static char FontNames[NUM_TOTAL_FONTS][PF_MAXSTRING] = {
  /* TextFonts */ TEXT_FONT_NAME,
  /* TreeFonts */ NUM_FONT_NAME,
  /* MessageFonts */ FANCY_FONT_NAME,
};

#ifndef WIN32
static int FontSizes[NUM_TOTAL_FONTS][NUM_FONT_SIZES] = {
  /* TextFonts */ {1.0f, 8.0f, 9.0f, 10.0f},
  /* TreeFonts */ {1.0f, 8.0f, 9.0f, 10.0f},
  /* MessageFonts */ {1.0f, 14.0f, 24.0f, 48.0f},
};
#else
static int FontSizes[NUM_TOTAL_FONTS][NUM_FONT_SIZES] = {
  /* TextFonts */ {1.0f, 10.0f, 12.0f, 14.0f},
  /* TreeFonts */ {1.0f, 10.0f, 12.0f, 14.0f},
  /* MessageFonts */ {1.0f, 18.0f, 30.0f, 60.0f},
};
#endif
/* Per-process variables necessary since we fork and each
 * process needs to maintain local FM or XFont structures
 */
static pfuGUIFontArray	Fonts[NUM_TOTAL_FONTS];
static int FontFlags = GUI_FONT_DIRTY;
static float GUIFontScale = 0;
static int MessageFontsInited = 0;


static GUIFont *CurFont = 0;

#if 0
static void fontstub(void) {}
#endif
#define	dspstring(_string)	    pfuDrawString(_string)
#define	stringwidth(_f, _string)    pfuGetXFontWidth(&(_f), _string)
#define fontheight(_f)		    pfuGetXFontHeight(&(_f))
#define mysetfont(_f)		    pfuSetXFont(&(_f))



	/*---------------------------------------------------------*/

PFUDLLEXPORT void pfuCullGUI(pfChannel * ch, void *data);
PFUDLLEXPORT void pfuDrawGUI(pfChannel * ch, void *data);

	/*---------------------------------------------------------*/

static pfuGUI*	getGUIDataPool(int okToCallBeforeInit) ;
static void	updateGUIViewport(int force);
static void	drawPanel(pfuPanel* p, int redraw);
static pfuWidget*	scanPanel(pfuPanel* p, int x, int y, int ymin);
static void	initNewWidget(pfuWidget* w, int type, int id);
static void	resetWidget(pfuWidget* w);
static void	positionWidgetAttrs(pfuWidget * w);
#if 0
static int	getWidgetSize(int type);
#endif
static void	widgetDim(pfuWidget* w, int xo, int yo, int xs, int ys);
static void	fitWidgets(void) ;
static void	setWidgetValue(pfuWidget* w, int dft, float newValue);
static void	setWidgetSelection(pfuWidget* w, int dft, int index);
static int	scanSelections(RadioButton* r, int x, int y, int ymin);
static void	drawWidget(pfuWidget* w, pfuPanel *p);
static void	drawString(float x, float y, pfVec3 clr, const char* str);
static void	drawBox(pfVec2 coord[4], pfVec3 clr, int elev);
static void	drawStateIndicator(pfVec2 icoord[4], int on);
static void	drawSwitchButton(pfVec2 coord[4], pfVec2 icoord[4], int on);
static void	drawBevel(pfVec2 coord[4], pfVec3 clr, float bevel);
static void	drawTriBevel(pfVec2 coord[4], pfVec3 clr, float bevel);

static void	scaleFonts(float _scale, int drawFlag);
static void	initGUIFonts(int drawFlag);
static void	initMessageFonts(void);
static void	dirtyWidget(pfuWidget * w);

static void drawMessage(pfChannel *chan, const char *msg,
		int rel, int just, float x, float y, int size, int cimode,
		unsigned long textClr, unsigned long shadowClr);

        /*-------------------------- GUI -----------------------------*/

PFUDLLEXPORT void
pfuInitGUI(pfPipeWindow *pw)
{
    pfPipe *p = pfGetPWinPipe(pw);
    pfFrameStats    *fs;

    getGUIDataPool(1);

    if (pfGetHyperpipe(p))
    {
	GUI->refresh = pfGetHyperpipe(p) * 4;
    }
    else
    {
	if (pfGetPWinType(pw) & PFPWIN_TYPE_X)
	    GUI->refresh = 4;
	else
	    GUI->refresh = 2;
    }
    GUI->pipe = p;
    GUI->pwin = pw;
    GUI->channel = pfNewChan(p);
    pfAddChan(pw, GUI->channel);
    pfChanTravFunc(GUI->channel, PFTRAV_CULL, pfuCullGUI);
    pfChanTravFunc(GUI->channel, PFTRAV_DRAW, pfuDrawGUI);
    pfMakeOrthoChan(GUI->channel, 0.0f, 280.0f, 0.0f, 1024.0f);
    pfChanNearFar(GUI->channel, -1.0f, 1.0f);

    /* have only the fast stats on for the GUI */
    fs = pfGetChanFStats(GUI->channel);
    pfFStatsClass (fs, PFSTATS_ALL, PFSTATS_OFF);
    /* set the fast process timing stats just in case someone turns
     * on stats for the gui for load managment
     */
    pfFStatsClassMode(fs, PFFSTATS_PFTIMES, PFFSTATS_PFTIMES_BASIC, PFSTATS_SET);

    pfuGUIViewport(0.0f, 1.0f, 0.0f, .2f);		/* Default viewport */
#ifdef	USE_FONT_MANAGER

    GUI->useFM = 1;
#endif

    updateGUIViewport(1);

    GUI->numPanels = 0;
    GUI->selectedWidget = 0;
    GUI->selectedPanel = 0;

    GUI->transX = GUI->transY = 0.0f;
    GUI->scaleX = GUI->scaleY = 1.0f;

    GUI->nGUIDraws = 0;
    GUI->guiDrawTime = 0.0f;

    pfuInitGUICursors();
    pfuUpdateGUICursor();
    
    GUI->inited = 1;

    GUI->pick->path = pfNewPath();
}

/*
 * All processes need to connect to this data pool.
*/
static pfuGUI*
getGUIDataPool(int okToCallBeforeInit)
{
    if (!GUI)
    {
	if (!GUIDP)
	    GUIDP = pfuGetUtilDPool();

	if (!GUIDP)
	{
	    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
		     "getGUIDataPool: Must call pfuInitUtil() first.");
	}

	if (!(GUI = (pfuGUI *) pfDPoolFind(GUIDP, PFUGUI_DATA_DPID)))
	{
	    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
		     "getGUIDataPool: could not find GUI Data Pool Data!");
	}
	else
	{
	    if (!(GUI->flags = (pfuGUIFlags*) pfDPoolFind(GUIDP, PFUGUI_FLAGS_DPID)))
	    {
		pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
		 "getGUIDataPool: could not find GUI Flags Data Pool Data!");
	    }
	    if (!(GUI->pick = (pfuGUIPicker*) pfDPoolFind(GUIDP, PFUGUI_PICK_DPID)))
	    {
		pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
		 "getGUIDataPool: could not find GUI Path Data Pool Data!");
	    }
	    if (!GUI->inited && !okToCallBeforeInit)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
		    "getGUIDataPool - pfuInitGUI() has not yet been called.");
	    }
	}
	initGUIFonts(0);
	GUI->fontScale = -1;
	GUI->hilightedNode = NULL;
	GUI->hlight = pfNewHlight(pfGetSharedArena());
	GUI->cursor = PFU_CURSOR_MAIN;
	if (!GUI->mainCursor)
	    GUI->mainCursor = PFU_CURSOR_MAIN_DEFAULT;
	if (!GUI->guiCursor)
	    GUI->guiCursor = PFU_CURSOR_GUI_DEFAULT;
	pfHlightColor(GUI->hlight, PFHL_FGCOLOR, 1.0f, 0.1f, 0.1f);
	pfHlightColor(GUI->hlight, PFHL_BGCOLOR, 0.1f, 1.0f, 0.1f);
	pfHlightPntSize(GUI->hlight, 3.0f);
	pfHlightLineWidth(GUI->hlight, 1.0f);
	pfHlightMode(GUI->hlight, PFHL_LINES);
    }
    return GUI;
}

static void
updateGUIViewport(int dirty)
{
    int         wxs, wys;
    int		sizeDirty;
    
    pfGetPWinSize(GUI->pwin, &wxs, &wys);

    sizeDirty = ((GUI->wxs != wxs) || (GUI->wys != wys));
    if (sizeDirty)
    {
	GUI->wxs = wxs;
	GUI->wys = wys;
	dirty = 1;
    }
    /* Compute pixel window coordinates of GUI */
    if (dirty)
    {
	GUI->orthoOrgY = 0.0f;
	GUI->pixOrgX = GUI->vpLeft * wxs;
	GUI->pixOrgY = GUI->vpBottom * wys;
	GUI->pixSizeX = (GUI->vpRight - GUI->vpLeft) * wxs;
	GUI->pixSizeY = (GUI->vpTop - GUI->vpBottom) * wys;
	GUI->orthoMaxY = GUI->pixSizeY;
	GUI->flags->dirty = GUI->flags->redrawAll = GUI->refresh;
	if (sizeDirty)
	{
	    GUI->flags->fitDirty = sizeDirty;
	    FontFlags |= FONT_SCALE_DIRTY;
	}
    }
}

PFUDLLEXPORT void
pfuGUIViewport(float l, float r, float b, float t)
{
    if (!GUI)
	getGUIDataPool(0);

    pfDPoolLock(GUI->flags);

    GUI->vpLeft = l;
    GUI->vpRight = r;
    GUI->vpBottom = b;
    GUI->vpTop = t;

    pfChanViewport(GUI->channel, GUI->vpLeft, GUI->vpRight,
		   GUI->vpBottom, GUI->vpTop);

    updateGUIViewport(1);

    pfDPoolUnlock(GUI->flags);
}

PFUDLLEXPORT pfChannel *
pfuGetGUIChan(void)
{
    if (!GUI)
	getGUIDataPool(0);
    return GUI->channel;
}

PFUDLLEXPORT void
pfuGetGUIViewport(float *l, float *r, float *b, float *t)
{
    if (!GUI)
	getGUIDataPool(0);

    *l = GUI->vpLeft;
    *r = GUI->vpRight;
    *b = GUI->vpBottom;
    *t = GUI->vpTop;
}

PFUDLLEXPORT void
pfuEnableGUI(int s)
{
    if (!GUI)
	getGUIDataPool(0);

    if (s)
    {
	pfChanTravMode(GUI->channel, PFTRAV_DRAW, PFDRAW_ON);

	pfDPoolLock(GUI->flags);
	GUI->flags->dirty = GUI->flags->redrawAll = GUI->refresh;
	pfDPoolUnlock(GUI->flags);
    }
    else
    {
	pfChanTravMode(GUI->channel, PFTRAV_DRAW, PFDRAW_OFF);
	GUI->cursor = PFU_CURSOR_MAIN;
    }
    GUI->active = s;
}

PFUDLLEXPORT void
pfuExitGUI(void)
{
    /* write out performance stats */
    if (GUI->nGUIDraws)
	pfNotify(PFNFY_INFO, PFNFY_PRINT,
	    "pfuExitGUI: %d frames at %.2f ms per draw",
	    GUI->nGUIDraws,
	    1000.0f * GUI->guiDrawTime / GUI->nGUIDraws);
}

/*
 * Automatically scale and translate widgets to fit into panel.
*/
/*ARGSUSED*/
PFUDLLEXPORT void
pfuFitWidgets(int val)
{
    if (!GUI)
	getGUIDataPool(0);

    pfDPoolLock(GUI->flags);

    GUI->flags->fitDirty = 1;

    pfDPoolUnlock(GUI->flags);
}


PFUDLLEXPORT int
pfuInGUI(int x, int y)
{
    int clickX, clickY, clickIn=1;
    if (!GUI)
	getGUIDataPool(1);

    if (!GUI->active)
	return 0;

    if (GUI->mouse.flags  & PFUDEV_MOUSE_DOWN_MASK)
    {
	clickX = GUI->mouse.clickPosLast[0];
	clickY = GUI->mouse.clickPosLast[1];
	clickIn = (clickX >= GUI->pixOrgX && 
			clickX <=(GUI->pixOrgX + GUI->pixSizeX) &&
	    	   clickY >= GUI->pixOrgY && 
			clickY <=(GUI->pixOrgY + GUI->pixSizeY));
    }
    if (GUI->selectedWidget || (clickIn &&
	(x >= GUI->pixOrgX && x <=(GUI->pixOrgX + GUI->pixSizeX) &&
	    y >= GUI->pixOrgY && y <=(GUI->pixOrgY + GUI->pixSizeY))))
    {
	return(1);
    }
    return (0);
}

/*
 * pfuResetGUI will reset all widgets in all active panels to their
 *  default values.  The Widget funcs will be called in all cases,
 *  regardless of whether its value was already the default, execpt for
 *  action buttons (type == BUTTON).
 *  This is especially useful for MESSAGE widgets that have no concept
 *  of a default value.
 *  There is a test so that this function won't recursively call itself
 *  just in case it has been used as a Widget func.
 */

PFUDLLEXPORT void
pfuResetGUI(void)
{
    static int	 inReset	= 0;
    int		 i;
    int		 j;
    pfuPanel	*p;
    pfuWidget	*w;

    if (inReset)
	return;

    inReset = 1;

    if (!GUI)
	getGUIDataPool(0);

    for (i = 0; i < GUI->numPanels; i++)
    {
	p = GUI->panels[i];
	if (!p || !(p->active))
	{
	    continue;
	}
	p->dirty = GUI->refresh;
	for (j = 0; j < p->numWidgets; j++)
	{
	    w = p->widgets[j];
	    if (w)
	    {
		resetWidget(w);
	    }
	}
    }

    pfDPoolLock(GUI->flags);
    GUI->flags->redrawAll = GUI->refresh;
    GUI->flags->dirty     = GUI->refresh;
    pfDPoolUnlock(GUI->flags);

    inReset = 0;
}

PFUDLLEXPORT void
pfuGetGUIScale(float *x, float *y)
{
    if (!GUI)
	getGUIDataPool(0);
    *x = GUI->scaleX;
    *y = GUI->scaleY;
}

PFUDLLEXPORT void
pfuGetGUITranslation(float *x, float *y)
{
    if (!GUI)
	getGUIDataPool(0);
    *x = GUI->transX;
    *y = GUI->transY;
}

PFUDLLEXPORT void
pfuRedrawGUI(void)
{
    if (!GUI)
	getGUIDataPool(0);
    pfDPoolLock(GUI->flags);
    GUI->flags->dirty = GUI->flags->redrawAll = GUI->refresh;
    pfDPoolUnlock(GUI->flags);
}

PFUDLLEXPORT void
pfuGUIHlight(pfHighlight *hlight)
{
    if (!GUI)
	getGUIDataPool(0);

    pfDPoolLock(GUI->flags);
    GUI->hlight = hlight;
    pfDPoolUnlock(GUI->flags);
}

PFUDLLEXPORT pfHighlight *
pfuGetGUIHlight(void)
{
    if (!GUI)
	getGUIDataPool(0);

    return GUI->hlight;
}

/*ARGSUSED*/
int
_pfuGUIPreHlight(pfTraverser *trav, void *data)
{
    pfPushState();

    pfOverride(
        PFSTATE_ENLIGHTING |
        PFSTATE_ENWIREFRAME |
        PFSTATE_ENHIGHLIGHTING |
        PFSTATE_LIGHTMODEL |
        PFSTATE_FRONTMTL |
        PFSTATE_BACKMTL, 0);

    pfEnable(PFEN_HIGHLIGHTING);
    pfApplyHlight(GUI->hlight);

    return 0;
}

/*ARGSUSED*/
int
_pfuGUIPostHlight(pfTraverser *trav, void *data)
{
    pfPopState();
    return 0;
}


static void
drawBevelBox(float xorg, float yorg, float xsize, float ysize, 
	     uint clr, const char *str)
{
    pfVec2	coords[4];
    pfVec3	clr3;
    static pfVec3 strClr = {.1f, .1f, .1f};

    pfSetVec3(clr3, (clr&0xff)/255.0f, 
	      ((clr&0xff00) >> 8)/255.0f, 
	      ((clr&0xff0000) >> 16)/255.0f); 

    pfSetVec2(coords[0], xorg, yorg);
    pfSetVec2(coords[1], xorg+xsize, yorg);
    pfSetVec2(coords[2], xorg+xsize, yorg+ysize);
    pfSetVec2(coords[3], xorg, yorg+ysize);

    drawBevel(coords, clr3, 2);
    drawBox(coords, clr3, -1);
    drawBevel(coords, clr3, -1);

    if (str)
    {
	mysetfont(Fonts[TREE_FONTS][FONT_SIZE_TITLE]);
	drawString(xorg+4, yorg+4, strClr, str);
	mysetfont((*CurFont));
    }
}

typedef struct 
{
    pfVec2	bbox[2];
    int		x, y, depth, skip;
    int		parent, instling;
    pfNode	*node;

} pfuGUINode;

#define CALCY(d) 	( (ySize - (((d) + 1) * 2.0f) * YS ) )
#define CALCW(w) 	( (w) * 1.5f*XS - .5f*XS)

#define XS		60
#define YS		20
#define MAX_NODES	(1<<14)

static int		xSize, ySize, xOrigin, yOrigin, xMouse, yMouse;
static int		xChanSize, yChanSize, xChanOrigin, yChanOrigin;
static int		vpLeft, vpRight, vpBottom, vpTop, vpWidth;
static int		indentLevel;
static pfuGUINode	*nodeList = NULL;
static pfPath		*pickPath = NULL;

static void
drawSwitch(pfSwitch *sw, int x, int y)
{
    static pfVec3 	strClr = {.1f, .1f, .1f};
    int val;
    int			ht = fontheight(Fonts[TREE_FONTS][FONT_SIZE_TEXT]);
    char		row[64];

    val = pfGetSwitchVal(sw);

    mysetfont(Fonts[TREE_FONTS][FONT_SIZE_TEXT]);
    sprintf(row, "switch value %d", val);
    drawBevelBox(x-10, y-10, stringwidth(Fonts[TREE_FONTS][FONT_SIZE_TEXT], row) + 15,
	ht+15, 0xff80a0c0, NULL);
    drawString(x, y, strClr, row);
    mysetfont((*CurFont));
}

static void
drawSCS(pfSCS *scs, int x, int y)
{
    static pfVec3 	strClr = {.1f, .1f, .1f};
    char		row[64];
    pfMatrix 		mat;
    int			ht = fontheight(Fonts[TREE_FONTS][FONT_SIZE_TEXT]);

    int			r;
    int			c;
    int			widest = 0;

    pfGetSCSMat(scs, mat);

    /* determine widest matrix element */
    for (r = 0; r < 4; r++)
    {
	for (c = 0; c < 4; c++)
	{
	    char buffer[64];
	    int  width;
	    sprintf(buffer, "%+.2f", mat[r][c]);
	    width = (int)strlen(buffer);
	    if (widest < width)
		widest = width;
	}
    }
    widest+=2;

    mysetfont(Fonts[TREE_FONTS][FONT_SIZE_TEXT]);
    sprintf(row, "%+*.2f %+*.2f %+*.2f %+*.2f", 
	widest, mat[0][0], 
	widest, mat[0][1], 
	widest, mat[0][2], 
	widest, mat[0][3]); 

    drawBevelBox(x-10, y-10, stringwidth(Fonts[TREE_FONTS][FONT_SIZE_TEXT], row) + 15,
	4 * ht+15, 0xff80a0c0, NULL);

    drawString(x, y+3*ht, strClr, row);

    sprintf(row, "%+*.2f %+*.2f %+*.2f %+*.2f", 
	widest, mat[1][0], 
	widest, mat[1][1], 
	widest, mat[1][2], 
	widest, mat[1][3]); 
    drawString(x, y+2*ht, strClr, row);

    sprintf(row, "%+*.2f %+*.2f %+*.2f %+*.2f", 
	widest, mat[2][0], 
	widest, mat[2][1], 
	widest, mat[2][2], 
	widest, mat[2][3]); 
    drawString(x, y+ht, strClr, row);

    sprintf(row, "%+*.2f %+*.2f %+*.2f %+*.2f", 
	widest, mat[3][0], 
	widest, mat[3][1], 
	widest, mat[3][2], 
	widest, mat[3][3]);
    drawString(x, y, strClr, row);

    mysetfont((*CurFont));
}

typedef struct { int hcol; int col; char name[256]; } colTable;
#define COL_ENTRIES 13
static colTable boxColors[COL_ENTRIES] =
{
    0xff89a0ff, 0xff8989ff, "SCS",	  /*salmon*/
    0xff3c9700, 0xff3c8200, "DCS",    /*green*/
    0xffffa000, 0xffff7e00, "Group",  /*blue*/
    0xffa0ffff, 0xff68ffff, "Switch", /*powder blue*/
    0xff60ffff, 0xff60e0ff, "LOD",    /*yellow*/
    0xff0050a7, 0xff003ba7, "Billboard", /*brown*/
    0xff00c0ff, 0xff00b0ff, "LightSource", /*gold*/
    0xffc7b7ff, 0xffc7a7ff, "LightPoint", /*pink*/
    0xffff20ff, 0xffff10ff, "Morph", /*purple*/
    0xff6510ff, 0xff6500ff, "ASD", /*ruby*/
    0xff72ff3b, 0xff72f13b, "Sequence", /*light green*/
    0xffff8bbb, 0xffff7bbb, "Layer", /*violet*/
    0xff1b53ff, 0xff1b43ff, "Text", /*orange*/
};

static void
drawNode(pfNode *node, float x, float y, int ishlight)
{
    const char *typeName;
    char        str[512];
    int found = 0, i;

    /* Highlight node if it is on the pick path */
    if (pfSearch(pickPath, node) >= 0)
        ishlight = 1;

    if(ishlight == 0)
    {
        typeName = pfGetTypeName((pfObject*) node);
        if (strncmp(typeName, "pf", 2) == 0)
            typeName +=2;

        for (i = 0; i < COL_ENTRIES; i++)
        {
            if (!strcmp(typeName,boxColors[i].name))
                {
                    found = 1;
                    drawBevelBox(x, y, XS-1, YS-1, boxColors[i].col,
typeName);
                }
        }
        if (!found)
            drawBevelBox(x, y, XS-1, YS-1, 0xff808080, typeName);
    }
    else
    {
        typeName = pfGetTypeName((pfObject*) node);
        if (strncmp(typeName, "pf", 2) == 0)
            typeName +=2;

        strcpy(str, typeName);

        if (pfGetNodeName(node) != NULL)
        {
            strcat(str, "\"");
            strcat(str, pfGetNodeName(node));
            strcat(str, "\"");
        }

        /* indicate number of geosets referenced by geode */
        if (pfIsOfType(node, pfGetGeodeClassType()))
        {
            char children[32];
            sprintf(children, "(%d)", pfGetNumGSets((pfGeode *)node));
            strcat(str, children);
        }

        for (i = 0; i < COL_ENTRIES; i++)
        {
            if (!strcmp(typeName,boxColors[i].name))
            {
                found = 1;
        	drawBevelBox(x, y, XS-1, YS-1, boxColors[i].hcol, str);
    	    }
    	}
    	if (!found)
            drawBevelBox(x, y, XS-1, YS-1, 0xff80a0c0, str);
    }
}


#if 0
static void
drawNode(pfNode *node, float x, float y, int ishlight)
{
    /* Highlight node if it is on the pick path */
    if (pfSearch(pickPath, node) >= 0)
	ishlight = 1;

    if (ishlight == 0)
    {
	const char *typeName;
	typeName = pfGetTypeName((pfObject*) node);
	if (strncmp(typeName, "pf", 2) == 0)
	    typeName +=2;
	drawBevelBox(x, y, XS-1, YS-1, 0xff808080, typeName);
    }
    else
    {
	const char *typeName;
	char	str[512];

	typeName = pfGetTypeName((pfObject*) node);
	if (strncmp(typeName, "pf", 2) == 0)
	    typeName +=2;
	
	strcpy(str, typeName);

	if (pfGetNodeName(node) != NULL)
	{
	    strcat(str, "\"");
	    strcat(str, pfGetNodeName(node));
	    strcat(str, "\"");
	}

	/* indicate number of geosets referenced by geode */
	if (pfIsOfType(node, pfGetGeodeClassType()))
	{
	    char children[32];
	    sprintf(children, "(%d)", pfGetNumGSets((pfGeode *)node));
	    strcat(str, children);
	}

	drawBevelBox(x, y, XS-1, YS-1, 0xff80a0c0, str);
    }
}
#endif

#define pfcpack(c)	glColor3ub((c)&0xff, ((c)&0xff00) >> 8, \
				   ((c)&0xff0000) >> 16);
#define pfbgnline()	glBegin(GL_LINE_STRIP)
#define pfendline()	glEnd()
#define pfortho2(l, r, b, t)	glOrtho(l, r, b, t, -1.0f, 1000.0f)
#define pfv2f(v)	glVertex2fv(v)
#define pflinewidth(l)	glLineWidth(l)



static void
drawLink(int x0, int y0, int x1, int y1, int type) 
{
    pfVec2	from, across, down;

    pfSetVec2(from, x0, y0);

    if (type == 0)	/* Draw XY-aligned link */
    {
	/* Don't draw horizontal portion if link is strictly vertical */
	pfcpack(0xff0000);
	if (x0 == x1)
	{
	    pfSetVec2(down, x1, y1);
	    pfbgnline();
	    pfv2f(from);
	    pfv2f(down);
	    pfendline();
	}
	else if (x1 - x0 > vpWidth)
	{
	    /* Draw across, then down */
	    pfSetVec2(across, x1, y0);
	    pfSetVec2(down, x1, y1);
	    pfbgnline();
	    pfv2f(from);
	    pfv2f(across);
	    pfv2f(down);
	    pfendline();
	}
	else
	{
	    /* Draw slanted link */
	    pfSetVec2(across, x1, y1);
	    pfbgnline();
	    pfv2f(from);
	    pfv2f(across);
	    pfendline();
	}
    }
    else	/* Draw slanted link for instance links */
    {
	pfcpack(0xffff00);
	/* If instance link is horizontal, make it not so for clarity */
	if (y0 == y1)
	{
	    pfSetVec2(across, x1+XS, y1-YS);
	    pfSetVec2(down, x0-XS, y0-YS);
	    pfbgnline();
	    pfv2f(from);
	    pfv2f(down);
	    pfv2f(across);
	    pfSetVec2(across, x1, y1);
	    pfv2f(across);
	    pfendline();
	}	
	else
	{
	    pfSetVec2(across, x1, y1);
	    pfbgnline();
	    pfv2f(from);
	    pfv2f(across);
	    pfendline();
	}
    }
}

static int maxDepth, nodeCount;

static void
drawTree(void)
{
    int		i, hilight, hilightDepth;
    pfuGUINode	*node;

    if (pickPath == NULL)
	pickPath = pfNewPath();

    pfDPoolLock(GUI->pick);
    pfCopy(pickPath, GUI->pick->path);
    pfDPoolUnlock(GUI->pick);

    pfPushMatrix();

    /* Translate line endpoint to center of box */
    pfTranslate(XS/2.0f, YS/2.0f, 0.0f);

    for (i=0; i<nodeCount; i++)
    {
	int	p;
	node = nodeList + i;

	p = node->parent;
	if (p >= 0)
	{
	    if (node->instling < 0)
		drawLink(nodeList[p].x, nodeList[p].y, 
			       node->x, node->y, 0); 
	    else
	    {
		drawLink(nodeList[p].x, nodeList[p].y, 
			       nodeList[node->instling].x, 
			       nodeList[node->instling].y, 1); 
		continue;	/* Do not cull phantom instling */
	    }

	    /* Cull box to viewport. This may incorrectly cull instance link */
	    if ((node->bbox[1][0] < vpLeft  && node->bbox[0][0] < vpLeft) ||
		(node->bbox[0][0] > vpRight && node->bbox[1][0] > vpRight) ||
		(node->bbox[1][1] < vpBottom && node->bbox[0][1] < vpBottom) ||
		(node->bbox[0][1] > vpTop && node->bbox[1][1] > vpTop))
	    {
		i = node->skip - 1;
		continue;
	    }
	}
    }

    pfPopMatrix();

    hilight = 0;
    for (i=0; i<nodeCount; i++)
    {
	node = nodeList + i;

	/* Do not draw instanced node unless it's highlighted */
	if (node->instling >= 0)
	{
	    if (hilight == 1)
		drawNode(nodeList[node->instling].node, 
			 nodeList[node->instling].x,
			 nodeList[node->instling].y, 1);
	    continue;
	}

	/* Cull box to viewport */
	if ((node->bbox[1][0] < vpLeft  && node->bbox[0][0] < vpLeft) ||
	    (node->bbox[0][0] > vpRight && node->bbox[1][0] > vpRight) ||
	    (node->bbox[1][1] < vpBottom && node->bbox[0][1] < vpBottom) ||
	    (node->bbox[0][1] > vpTop && node->bbox[1][1] > vpTop))
	{
	    i = node->skip - 1;
	    continue;
	}

	/* Highlight subtree corresponding to box which has mouse focus */
	if (hilight == 0)
	{
	    if (xMouse >= node->x && xMouse <= node->x + XS && 
		yMouse >= node->y && yMouse <= node->y + YS)
	    {
		hilight = 1;
		hilightDepth = node->depth;
		GUI->hilightedNode = node->node;
	    }
	}
	else if (hilight == 1)
	{
	    /* Stop highlighting once we finish subtree */
	    if (node->depth <= hilightDepth)
		hilight = 2;
	}

	switch (hilight) {
	case 0:
	case 2:
	    drawNode(node->node, node->x, node->y, 0);
	    break;
	case 1:
	    drawNode(node->node, node->x, node->y, 1);
	    break;
	}
    }
    if (hilight == 0)
    {
	if (GUI->pick->node == NULL)
	    GUI->hilightedNode = NULL;
    }
    else if (GUI->mouse.flags & PFUDEV_MOUSE_LEFT_DOWN)
    {
	if (pfIsOfType(GUI->hilightedNode, pfGetSCSClassType()))
	    drawSCS((pfSCS*)GUI->hilightedNode, xMouse, yMouse);
	if (pfIsOfType(GUI->hilightedNode, pfGetSwitchClassType()))
	    drawSwitch((pfSwitch*)GUI->hilightedNode, xMouse, yMouse);
    }
}

static pfuGUINode	*pickGUINode = NULL;

/* 
 * Traverse tree and calculate dimensions of graph.
*/
static int
travTree(int parent, pfNode *node, pfuTraverser *trav)
{
    int		i, nodeId;
    pfuGUINode	*gnode;

    if (trav->depth > maxDepth)
	maxDepth = trav->depth;

    if (nodeCount >= MAX_NODES)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
		 "pfuDrawTree() Too many nodes, max of %d\n", MAX_NODES);
	return 0;
    }

    nodeId = nodeCount++;
    gnode = nodeList + nodeId;
    gnode->parent = parent;
    gnode->depth = trav->depth;
    gnode->node = node;
    gnode->instling = -1;

    /* If node is instanced, find the first instance of it on nodeList */
    if (pfGetNumParents(node) > 1 && 
	pfGetParent(node, 0) != (pfGroup*)nodeList[parent].node)
    {
	/* Linear search through previous nodes to find instance sibling */
	for (i=0; i<nodeCount-1; i++)
	     if (nodeList[i].node == node)
		 break;

	/* Found instance sibling so patch up reference and prune traversal */
	if (i != nodeCount-1)
	{
	    gnode->instling = i;

	    indentLevel += 1.5f * XS;
	    gnode->skip = nodeCount;

	    /* Prune traversal here */
	    return 1;
	}
	/* If instance sibling not found, continue normally since 
	 first parent is not in tree. */
    }

    if (pfIsOfType(node, pfGetGroupClassType()))
    {
	int	width = 0, n, maxDepthSave, pwidth;
	n = pfGetNumChildren(node);

	/* Find maxDepth under this group */
	maxDepthSave = maxDepth;
	maxDepth = -1;

	for (i=0; i<n; i++)
	{
	    trav->depth++;
	    width += travTree(nodeId, pfGetChild(node, i), trav);
	    trav->depth--;
	}

	gnode->skip = nodeCount;

	/* Place parent at center of its children */
	gnode->x = indentLevel - XS - CALCW(width)/2.0f;
	gnode->y = CALCY(trav->depth);

	pwidth = (width * 1.5f * XS - .5f*XS) * .5f;
	
	/* Set group's bounding box */
	if (n == 0)
	{
	    PFSET_VEC2(gnode->bbox[0], gnode->x, gnode->y);
	    PFSET_VEC2(gnode->bbox[1], gnode->x + XS, gnode->y + YS);
	    indentLevel += 1.5f * XS;
	}
	else
	{
	    PFSET_VEC2(gnode->bbox[0], gnode->x - pwidth, CALCY(maxDepth));
	    PFSET_VEC2(gnode->bbox[1], gnode->x + pwidth, gnode->y + YS);
	}

	if (node == GUI->pick->node)
	    pickGUINode = gnode;

	/* Restore maxDepth */
	maxDepth = PF_MAX2(maxDepth, maxDepthSave); 

	return width;
    }
    else
    {
	gnode->x = indentLevel;
	gnode->y = CALCY(trav->depth);

	/* Set node's bounding box */
        PFSET_VEC2(gnode->bbox[0], gnode->x, gnode->y);
        PFSET_VEC2(gnode->bbox[1], gnode->x + XS, gnode->y + YS);

	if (node == GUI->pick->node)
	    pickGUINode = gnode;

	indentLevel += 1.5f * XS;
	gnode->skip = nodeCount;
	return 1;
    }
}

static void
pickTree(void)
{
    pfHit	    	**hits[32];
    int			nhits;

    if (!GUI->pick->requestFlag)
	return;

    if (GUI->pick->x < 0.0f || GUI->pick->x > 1.0f || 
	GUI->pick->y < 0.0f || GUI->pick->y > 1.0f)
	return;

    nhits = pfChanPick(GUI->pick->chan,
		       PFPK_M_NEAREST|PFTRAV_IS_PRIM|PFTRAV_IS_PATH|
		       PFTRAV_LOD_CUR|PFTRAV_SW_CUR|PFTRAV_SEQ_CUR,
		       GUI->pick->x, GUI->pick->y, 
		       0.0f, hits);

    pfDPoolLock(GUI->pick);
    if (nhits)
    {
	pfPath		*path;
	int		n;

	pfQueryHit(hits[0][0], PFQHIT_PATH, &path);
	pfCopy(GUI->pick->path, path);
	n = pfGetNum(GUI->pick->path) - 1;
	GUI->pick->node = NULL;
	while (n >= 0 && 
	       !pfIsOfType(pfGet(GUI->pick->path, n), 
			   pfGetNodeClassType()))
	    n--;
	if (n >= 0)
	    GUI->pick->node = GUI->hilightedNode = 
		pfGet(GUI->pick->path, n);
    }
    else
    {
	pfResetList(GUI->pick->path);
	GUI->pick->node = NULL;
    }
    GUI->pick->requestFlag = 0;
    pfDPoolUnlock(GUI->pick);
}


PFUDLLEXPORT void
pfuDrawTree(pfChannel *chan, pfNode *tree, pfVec3 panScale)
{
    pfuTraverser	trav;
    float		xScale, xOffset, yScale, yOffset;

    if (nodeList == NULL)
	nodeList = pfMalloc(sizeof(pfuGUINode) * MAX_NODES, NULL);

    if (!GUI)
    {
	getGUIDataPool(0);
    }
    if (FontFlags & GUI_FONT_DIRTY)
	scaleFonts(1.0f, 1);
	
    

    pfGetChanSize(chan, &xSize, &ySize);
    pfGetChanOrigin(chan, &xOrigin, &yOrigin);
    pfGetChanOutputSize(chan, &xChanSize, &yChanSize);
    pfGetChanOutputOrigin(chan, &xChanOrigin, &yChanOrigin);


    pfPushIdentMatrix();
    pfPushState();
    pfBasicState();
    pflinewidth(2);

    glViewport(xChanOrigin, yChanOrigin, xChanSize, yChanSize);
    glDepthFunc(GL_ALWAYS);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, xSize, 0.0f, ySize, -1.0f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    

    pfuInitTraverser(&trav);
    indentLevel = .5f * XS;
    maxDepth = nodeCount = 0;
    pickGUINode = NULL;
    travTree(-1, tree, &trav);


    /* Pan and scale scene graph */
    if (panScale)
    {
	int 		width, height, xc, yc, dx, dy, middle;
#ifdef USE_FONT_MANAGER
	float		fontScale;
	static float 	prevScale = -1.0f;
#endif

	middle = nodeList[0].x;
	width = indentLevel;
	height = (ySize - CALCY(maxDepth));

	if (pickGUINode)
	    dx = middle - 
		.5f * (pickGUINode->bbox[1][0] + pickGUINode->bbox[0][0]);
	else
	    dx = 0.0f;

	/* Map (-1, 1) to pan completely to left and right */
	if (panScale[0] <= 0)
	    dx += -middle * panScale[0];
	else
	    dx += (middle - width) * panScale[0];

	dy = PF_ABS(height - ySize) * panScale[1];

	/* Figure out center of scene graph in order to scale it properly */
	xc = (PF_MIN2(xChanSize, width)) / 2.0f;
	if (ySize < height)
	    yc = ySize / 2.0f;
	else
	    yc = ySize - height / 2.0f;

#ifdef USE_FONT_MANAGER
	fontScale = PF_MAX2(.5f, panScale[2]);
	if (fontScale != prevScale)
	{
	    int	i;
	    prevScale = fontScale;
	    for (i=1; i<PFU_NUM_FONTS; i++)
	    {
		if (Fonts[TREE_FONTS][0].handle && (Fonts[TREE_FONTS][i].handle =
				    fmscalefont(Fonts[TREE_FONTS][0].handle, 
						prevScale*Fonts[TREE_FONTS][i].size)))
		{
		    fmgetfontinfo(Fonts[TREE_FONTS][i].handle, 
				  &(Fonts[TREE_FONTS][i].info));
		}
	    }
	}
#endif

	/* Translate back to graph center */
	pfTranslate(xc, yc, 0.0f);

	/* Scale graph */

	if (panScale[2] < 1.0f && height < ySize)
	{
	    pfScale(panScale[2], 1.0f, 1.0f);
	    yScale = 1.0f;

	}
	else
	{
	    pfScale(panScale[2], panScale[2], 1.0f);
	    yScale = panScale[2];
	}
	/* Translate graph center to origin for nice scale*/
	pfTranslate(-xc, -yc, 0.0f);
	pfTranslate(dx-middle+xSize/2.0f, dy, 0.0f);

	/* Compute scale and offsets to map from scene graph into viewport */
	xScale = panScale[2];
	xOffset = (xScale * (dx-middle+xSize/2.0f - xc))  + xc;
	yOffset = (yScale * (dy - yc))  + yc;
    }
    else
    {
	xScale = yScale = 1;
	xOffset = yOffset = 0;
    }

    xMouse = GUI->mouse.xpos - (xOrigin);
    xMouse = (xMouse - xOffset) / xScale;
    yMouse = GUI->mouse.ypos - (yOrigin);
    yMouse = (yMouse - yOffset) / yScale;

    if (GUI->mouse.modifiers & PFUDEV_MOD_ALT)
    {
	pfDPoolLock(GUI->pick);
	GUI->pick->requestFlag = 1;
	GUI->pick->x = (float)(GUI->mouse.xpos - xOrigin) / (float)xSize;
	GUI->pick->y = (float)(GUI->mouse.ypos - yOrigin) / (float)ySize; 
	GUI->pick->chan = chan;
	pfDPoolUnlock(GUI->pick);
    }
    else
    {
	/* highlight checking is after this in drawTree() so this will
	 * not clobber its result.
	 */
	GUI->hilightedNode = NULL;
    }
    
    /* Compute viewport coordinates in tree-space by inverse panning/scaling */
    vpLeft = -xOffset / xScale;
    vpBottom = -yOffset / yScale;
    vpRight = vpLeft + xSize / xScale;
    vpTop = vpBottom + ySize / yScale;
    vpWidth = vpRight - vpLeft;
	
    drawTree();

    pfPopMatrix();
    pfPopState();
    glDepthFunc(GL_LEQUAL);
}

int
preWrapper(pfTraverser *trav, void *data)
{
    pfList		*list = (pfList*)data;
    pfNodeTravFuncType	pre;
    
    pre = (pfNodeTravFuncType)pfGet(list, 0);

    (*_pfuGUIPreHlight)(trav, NULL);

    if (pre)
	(*pre)(trav, pfGet(list, 2));

    return PFTRAV_CONT;
}

int
postWrapper(pfTraverser *trav, void *data)
{
    pfList		*list = (pfList*)data;
    pfNodeTravFuncType	post;
    
    post = (pfNodeTravFuncType)pfGet(list, 1);

    (*_pfuGUIPostHlight)(trav, NULL);

    if (post)
	(*post)(trav, pfGet(list, 2));

    return PFTRAV_CONT;
}


PFUDLLEXPORT void
pfuUpdateGUI(pfuMouse *mouse)
{
    int            j;
    int            in;
    int            x, y;
    pfuWidget       *w = 0;
    int             haveWidget = 0, havePanel = 0;
    static pfNode   *oldHilightedNode = NULL;

    if (GUI == NULL)
	return;

    if (!GUI)
	getGUIDataPool(0);

    pickTree();

    /* De-hilight old node and hilight current one, if any */
    if (GUI->hilightedNode != oldHilightedNode)
    {
	pfNodeTravFuncType	pre, post;
	static pfList	        *list = NULL;
	    
	/* De-highlight old node */
	if (oldHilightedNode != NULL)
	{
	    /* Restore original callbacks if necessary */
	    pfGetNodeTravFuncs(oldHilightedNode, PFTRAV_DRAW, &pre, &post);
	    if (pre == preWrapper)
	    {
		list = (pfList*) 
		    pfGetNodeTravData(oldHilightedNode, PFTRAV_DRAW);

		pfNodeTravFuncs(oldHilightedNode, PFTRAV_DRAW, 
				(pfNodeTravFuncType) pfGet(list, 0),
				(pfNodeTravFuncType) pfGet(list, 1));
		pfNodeTravData(oldHilightedNode, PFTRAV_DRAW, pfGet(list, 2));
	    }
	    else
		pfNodeTravFuncs(oldHilightedNode, PFTRAV_DRAW, NULL, NULL);
	}

	/* Highlight current node, if any */
	if (GUI->hilightedNode != NULL)
	{
	    pfGetNodeTravFuncs(GUI->hilightedNode, PFTRAV_DRAW, &pre, &post);

	    /* Don't clobber pre-existing DRAW callbacks */
	    if (pre || post)
	    {
		pfNodeTravFuncs(GUI->hilightedNode, PFTRAV_DRAW, 
				preWrapper, postWrapper);
		if (!list)
		    list = pfNewList(sizeof(void*), 3, pfGetSharedArena());

		pfResetList(list);
		pfAdd(list, (void *)pre);
		pfAdd(list, (void *)post);
		pfAdd(list, 
		      pfGetNodeTravData(GUI->hilightedNode, PFTRAV_DRAW)); 

		pfNodeTravData(GUI->hilightedNode, PFTRAV_DRAW, list); 
	    }
	    else
	    {
		pfNodeTravFuncs(GUI->hilightedNode, PFTRAV_DRAW, 
				_pfuGUIPreHlight, _pfuGUIPostHlight);
	    }
	}

	oldHilightedNode = GUI->hilightedNode;
    }

    /* Highlighting code needs mouse */
    GUI->mouse = *mouse;

    if (!GUI->active && ! GUI->selectedWidget)
	return;

    /* make sure we have up-to-date info in win size and pos */
    updateGUIViewport(0);

    in = (mouse->inWin && 
	((mouse->inWin == 1) || (mouse->inWin == pfGetPWinWSWindow(GUI->pwin))) &&
	pfuInGUI(mouse->xpos, mouse->ypos));
    
    if (in || GUI->selectedWidget)
	GUI->cursor = PFU_CURSOR_GUI;
    else
	GUI->cursor = PFU_CURSOR_MAIN;
    pfuUpdateGUICursor();

    /* If there is no selected widget and no mouse down, leave */
    if (!GUI->selectedWidget &&
        (!(in) || !(mouse->flags & PFUDEV_MOUSE_DOWN_MASK) ||
	 !pfuInGUI(mouse->clickPosLast[0], mouse->clickPosLast[1])))
    {
	return;
    }
    else
    {
	/* Map mouse from window coords into panel coords */
	x = MAP_XMOUSE(mouse->xpos);
	y = MAP_YMOUSE(mouse->ypos);

	/*
	 * Only sliders keep active focus outside the widget, but non sliders
	 * still need to be cleared if they still have focus but aren't
	 * active.
	*/
	if (GUI->selectedPanel)
	{
	    j = GUI->selectedPanel->id;
	}
	else
	    j = 0;

	while (j < GUI->numPanels)
	{
	    if (!GUI->panels[j]->active)
	    {
		GUI->selectedWidget = 0;
		GUI->selectedPanel = 0;
		j++;
		continue;
	    }

	    /* See if mouse focus is still in selected widget */
	    if (!GUI->selectedWidget)
	    {
		pfuWidget         *wclick;
		wclick = scanPanel((pfuPanel *) GUI->panels[j],
				   MAP_XMOUSE(mouse->clickPosLast[0]),
				   MAP_YMOUSE(mouse->clickPosLast[1]),
				   GUI->orthoOrgY);
		w = scanPanel((pfuPanel *) GUI->panels[j], x, y, GUI->orthoOrgY);
		if (w != wclick)
		{
		    w = 0;
		}
		else if (w && w->selectFunc)
		{
		    w = (w->selectFunc)(w,  GUI->panels[j]);
		}
	    }
	    else
	    {
		w = GUI->selectedWidget;
	    }
	    if (w == NULL)
	    {
		j++;
		continue;
	    }

	    haveWidget = 0;

#define CALLFUNC(w)	{					\
	if ((w)->actionFunc) 	{					\
	    /* Unlock widget so callback doesn't double-trip */	\
	    pfDPoolUnlock(w); 					\
	    (*((w)->actionFunc))(w);					\
	    pfDPoolLock(w); 					\
	}}

	    /* Process selected widget. */

	    /*
	     * Lock widget. Must take care not to double-trip on this lock
	     * in the widget callback.
	    */
	    pfDPoolLock(w);
	    switch (w->type & PFUGUI_TYPE_MASK)
	    {
	    case PFUGUI_SLIDER:
		{
		    Slider         *s =(Slider *) w;
		    if (mouse->flags & PFUDEV_MOUSE_LEFT_DOWN)
		    {
			if (s->hvFlag)	    /* horizontal */
			{
			    s->sval = (((float) x - s->start) *
				   s->scale) + s->smin;
			}
			else		    /* vertical */
			{
			    s->sval = (((float) s->start - y) *
				   s->scale) + s->smin;
			}
			if (s->sval < s->smin)
			    s->sval = s->smin;
			if (s->sval > s->smax)
			    s->sval = s->smax;

			/* Make val stick at default */
			if ((w->mode & STICKY_DFT) &&
				PF_ABSLT(s->sval - s->sDefaultVal, 10 * (s->scale)))
			    s->sval = s->sDefaultVal;

			if (s->type == PFUGUI_SLIDER)
			    s->val = s->sval;
			else if (s->type == PFUGUI_SLIDER_LOG)
			    s->val = powf(10.0f, s->sval);

			CALLFUNC(w);

			s->dirty = GUI->refresh;
			GUI->selectedWidget = (pfuWidget *) s;
			GUI->selectedPanel = (pfuPanel *) GUI->panels[j];
			haveWidget = 1;
		    }
		    else 
		    if (!(mouse->flags & PFUDEV_MOUSE_LEFT_DOWN))
		    {
			s->oldVal = s->val;
			s->sOldVal = s->sval;
			s->dirty = GUI->refresh;
			haveWidget = 1;
			if (GUI->selectedWidget)
			{
			    GUI->selectedWidget = 0;
			    GUI->selectedPanel = 0;
			}
		    }
		    break;
		}
	    case PFUGUI_BUTTON:
		if (mouse->flags & PFUDEV_MOUSE_LEFT_DOWN)
		{
		    if (!w->state)
		    {
			w->style = PFUGUI_HILIGHT;
			w->val = 1.0f;
			w->state = 1;
			w->dirty = GUI->refresh;
			GUI->selectedWidget = w;
			GUI->selectedPanel = (pfuPanel *) GUI->panels[j];
		    }
		    haveWidget = 1;
		}
		else 
		if (w->state == 2)
		{
		    w->val = 0.0f;
		    CALLFUNC(w);
		    w->state = 0;
		    w->style = PFUGUI_NORMAL;
		    w->dirty = GUI->refresh;
		    if (GUI->selectedWidget)
		    {
			GUI->selectedWidget = 0;
			GUI->selectedPanel = 0;
		    }
		    haveWidget = 1;
		}
		break;

	    case PFUGUI_SWITCH:
		if (mouse->flags & PFUDEV_MOUSE_LEFT_DOWN)
		{
		    if (!w->state)
		    {
			if (w->val != w->min)
			    w->val = w->min;
			else
			    w->val = w->max;
			CALLFUNC(w);
			w->state = 1;
			w->dirty = GUI->refresh;
			GUI->selectedWidget = w;
			GUI->selectedPanel = (pfuPanel *) GUI->panels[j];
		    }
		    haveWidget = 1;
		}
		else 
		if (w->state)
		{
		    w->state = 0;
		    w->dirty = GUI->refresh;
		    if (GUI->selectedWidget)
		    {
			GUI->selectedWidget = 0;
			GUI->selectedPanel = 0;
		    }
		    haveWidget = 1;
		}
		break;

	    case PFUGUI_MENU_BUTTON:
		{
		    MenuButton     *m =(MenuButton *) w;
		    if (mouse->flags & PFUDEV_MOUSE_LEFT_DOWN)
		    {
			if (!m->state)
			{
			    m->sel = (m->sel + 1) % m->numSelections;
			    m->val = m->selectionList[m->sel].val;

			    if (m->selectionList[m->sel].actionFunc != NULL)
			    {
				/* Unlock widget so callback doesn't double-trip */
			    	pfDPoolUnlock(w);
				(*(m->selectionList[m->sel].actionFunc)) ((pfuWidget *) m);
			    	pfDPoolLock(w);
			    }
			    else 
			    if (m->actionFunc != NULL)
			    {
				/* Unlock widget so callback doesn't double-trip */
			    	pfDPoolUnlock(w);
				(*(m->actionFunc)) ((pfuWidget *) m);
			    	pfDPoolLock(w);
			    }
			    m->state = 1;
			    m->dirty = GUI->refresh;
			    GUI->selectedWidget = (pfuWidget *) m;
			    GUI->selectedPanel = (pfuPanel *) GUI->panels[j];
			}
			haveWidget = 1;
		    }
		    else 
		    if (mouse->flags & PFUDEV_MOUSE_RIGHT_DOWN)
		    {	/* do menu - happens in draw process */
			if (!(GUI->dopup) && !(m->state))
			{
			    GUI->dopup = m;
			    GUI->selectedWidget = (pfuWidget *) m;
			    GUI->selectedPanel = (pfuPanel *) GUI->panels[j];
			}
			haveWidget = 1;
		    }
		    else 
		    if (m->state == 1)
		    { /* menu was selected in draw - call widget function */
			if (m->selectionList[m->sel].actionFunc != NULL)
			    (*(m->selectionList[m->sel].actionFunc)) ((pfuWidget *) m);
			else 
			if (m->actionFunc != NULL)
			    (*(m->actionFunc)) ((pfuWidget *) m);
			m->state = 0;
			m->dirty = GUI->refresh;
			haveWidget = 1;
			GUI->dopup = NULL;
		    }
		    else 
		    if (!(GUI->dopup))
		    { /* was a left-mouse menu select or a finished menu
		       * select - drew it pushed, now pop it back out and
		       * release widget.
		       */
			m->state = 0;
			m->dirty = GUI->refresh;
			if (GUI->selectedWidget)
			{
			    GUI->selectedWidget = 0;
			    GUI->selectedPanel = 0;
			}
			haveWidget = 1;
		    }
		    break;
		}
	    case PFUGUI_RADIO_BUTTON:
		{
		    RadioButton    *r =(RadioButton *) w;
		    if (mouse->flags & PFUDEV_MOUSE_LEFT_DOWN)
		    {
			int             index;

			/* find out where the mouse is */
			index = (int)scanSelections(r,
					  MAP_XMOUSE(mouse->clickPosLast[0]),
					  MAP_YMOUSE(mouse->clickPosLast[1]),
					  GUI->orthoOrgY);
			if (!r->state)
			{
			    if (index == -1)
			    {	/* have title - do switch */
				if (!r->on)
				{
				    r->on = 1;
				    r->val = r->selectionList[r->sel].val;
				}
				else
				    r->on = 0;
				r->state = 1;
			    }
			    else 
			    if (index > -1)
			    {	/* changing selection selection - also turn on */
				r->on = 1;
				r->sel = index;
				r->val = r->selectionList[index].val;
				r->state = 1;
			    }
			    if (r->selectionList[r->sel].actionFunc != NULL)
			    {
				/* Unlock widget so callback doesn't double-trip */
			    	pfDPoolUnlock(w);
				(*(r->selectionList[r->sel].actionFunc)) ((pfuWidget *) r);
			    	pfDPoolLock(w);
			    }
			    else 
			    if (r->actionFunc != NULL)
			    {
				/* Unlock widget so callback doesn't double-trip */
			    	pfDPoolUnlock(w);
				(*(r->actionFunc)) ((pfuWidget *) r);
			    	pfDPoolLock(w);
			    }
			    r->dirty = GUI->refresh;
			    GUI->selectedWidget = (pfuWidget *) r;
			    GUI->selectedPanel = (pfuPanel *) GUI->panels[j];
			}
			haveWidget = 1;
		    }
		    else 
		    if (r->state || (r->sel != r->oldSel))
		    {
			r->state = 0;
			r->oldSel = r->sel;
			r->dirty = GUI->refresh;
			if (GUI->selectedWidget)
			{
			    GUI->selectedWidget = 0;
			    GUI->selectedPanel = 0;
			}
			haveWidget = 1;
		    }
		    break;
		}
	    case PFUGUI_MESSAGE:
		break;
	    default:
		break;
	    }
	    pfDPoolUnlock(w);
	    /* can only have one active widget */
	    if (haveWidget)
	    {
		GUI->panels[j]->dirty = GUI->refresh;
		havePanel = 1;
		break;
	    }

	    j++;	/* go to next panel */
	}
	if (havePanel)
	{
	    pfDPoolLock(GUI->flags);
	    GUI->flags->dirty = GUI->refresh;
	    pfDPoolUnlock(GUI->flags);
	}

	return;
    }
}

/* cull callback stub */
/*ARGSUSED*/
PFUDLLEXPORT void pfuCullGUI(pfChannel * ch, void *data)
{
}

/*
 * Dirty Bit System for updating the GUI Disply
 * --------------------------------------------
 *  GUI->redrawAll: The big ugly dirty bit - everthing is redrawn from scratch.
 *  GUI->dirty:	    There is at least one dirty widget somewhere....
 *  panel->dirty:   There is at least one dirty widget somewhere....
 *  widget->dirty:  This widget needs to be redrawn
 *
*/
/*ARGSUSED*/
PFUDLLEXPORT void
pfuDrawGUI(pfChannel * ch, void *data)
{
    static int     drawInited = 0;
    static int     oldxs=-1, oldys=-1;

    pfPipe	    *p;
    int            i, redraw;
    double          startTime, endTime;

    /* Initialize stuff the first time through */
    if (!drawInited)
    {
	int depth;
	pfQuerySys(PFQSYS_MAX_DBL_RGB_BITS, &depth);

	if (depth < 4)
	{
	    PFSET_VEC4(ClearColor, 0.05f, 0.2f, 0.3f, 1.0f);
	    PFSET_VEC3(BackColor, 0.22f, 0.33f, 0.6f);
	    PFSET_VEC3(OnColor, 1.0f, 0.0f, 0.0f);
	    PFSET_VEC3(OffColor, 0.4f, 0.1f, 0.1f);
	    PFSET_VEC3(SliderColor, 0.4f, 0.1f, 0.1f);
	    PFSET_VEC3(ValColor, 0.9f, 0.9f, 0.8f);
	    PFSET_VEC3(TextColor, 0.1f, 0.05f, 0.1f);
	    PFSET_VEC3(MessageColor, 0.6f, 0.2f, 0.2f);
	    PFSET_VEC3(InactiveSliderColor, 0.45f, 0.4f, 0.4f);
	    PFSET_VEC3(InactiveTextColor, 0.5f, 0.5f, 0.4f);
	}
	else
	{
	    PFSET_VEC4(ClearColor, 0.15f, 0.15f, 0.2f, 1.0f);
	    PFSET_VEC3(BackColor, 0.36f, 0.36f, 0.46f);
	    PFSET_VEC3(OnColor, 1.0f, 0.0f, 0.0f);
	    PFSET_VEC3(OffColor, 0.4f, 0.1f, 0.1f);
	    PFSET_VEC3(SliderColor, 0.4f, 0.1f, 0.1f);
	    PFSET_VEC3(ValColor, 0.9f, 0.9f, 0.85f);
	    PFSET_VEC3(TextColor, 0.1f, 0.05f, 0.1f);
	    PFSET_VEC3(MessageColor, 0.6f, 0.2f, 0.2f);
	    PFSET_VEC3(InactiveSliderColor, 0.45f, 0.4f, 0.4f);
	    PFSET_VEC3(InactiveTextColor, 0.45f, 0.5f, 0.5f);
	}
	
	if (!GUI)
	{
	    /* Attatch to the gui data pool created by pfuInitGUI */
	    GUI = getGUIDataPool(0);
	}
	if (GUI == NULL)
	    return;
	

	drawInited = 1;
    }

    p = pfGetChanPipe(ch);

    /* Make sure we draw GUI only on the proper pfPipe. */
    if (p != GUI->pipe && !pfGetHyperpipe(p))
	return;

    /* check states that require redrawing the GUI:
     * window size, position
     */
    if (GUI->wxs != oldxs || GUI->wys != oldys)
    {
	updateGUIViewport(0);

	oldxs = GUI->wxs;
	oldys = GUI->wys;
    }

    /* Compute scale and translation needed to fit widgets into GUI */
    if (GUI->flags->fitDirty)
	fitWidgets();
    if ((FontFlags & GUI_FONT_DIRTY) || 
	    (GUI->fontScale < 0) ||
	    (PF_ABSGE(GUIFontScale - GUI->fontScale, 0.05f)))
	scaleFonts(PF_MIN2(GUI->scaleX, GUI->scaleY), 1);

    /* Return if nothing needs to be redrawn */
    pfDPoolLock(GUI->flags);
    if ((GUI->flags->dirty <= 0) && (GUI->flags->redrawAll <= 0))
    {
	pfDPoolUnlock(GUI->flags);
	return;
    }

    startTime = pfGetTime();


    /* Deal with popup menus in draw process
     * NOTE: this is asynchronous from the handling of the menu
     * values in the case of X input handling!
     */
    if (GUI->dopup && pfGetHyperpipe(GUI->pipe) == 0)
    {
	long		t;
	MenuButton 	*m = (MenuButton *)GUI->dopup;
    	pfDPoolLock(m);
	if (!m->state)
	{ /* menu has not yet been popped/selected */
	    t = dopup(m->menu);
	    if (t > -1)
	    {
		m->sel = (int)t - 1;
		m->val = m->selectionList[m->sel].val;
		m->state = 1;
	    }
	}
	GUI->dopup = NULL;
	pfDPoolUnlock(m);
    }

    redraw = GUI->flags->redrawAll;

    /* Decrement dirty and redraw values */
    if (GUI->flags->redrawAll > 0)
	GUI->flags->redrawAll--;
    if (GUI->flags->dirty > 0)
	GUI->flags->dirty--;

    pfDPoolUnlock(GUI->flags);

    if (redraw > 0)
	/* clear before setting up state, since pfClear
	 * may muck with dither
	 */
	pfClear(PFCL_COLOR, ClearColor);

    /* Draw GUI */
    pfPushState();
    pfBasicState();
    glDepthFunc(GL_ALWAYS);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, GUI->pixSizeX, GUI->orthoOrgY, GUI->orthoMaxY,
		-1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_DITHER);
    pfPushIdentMatrix();

    /* Draw border of GUI only if we're redrawing everything */
    if (redraw > 0)
    {
	static pfVec2   coord[4];
	float	b = PFUGUI_PANEL_BORDER * PF_MIN2(GUI->scaleX, GUI->scaleY);
	PFSET_VEC2(coord[0], b, GUI->orthoOrgY + b - 0.5f);
	PFSET_VEC2(coord[1], GUI->pixSizeX - 5.0f, GUI->orthoOrgY + b - 0.5f);
	PFSET_VEC2(coord[2], GUI->pixSizeX - 5.0f, GUI->orthoMaxY - b - 0.5f);
	PFSET_VEC2(coord[3], b, GUI->orthoMaxY - b - 0.5f);
	drawBevel(coord, ClearColor, b);
	drawBevel(coord, ClearColor, -b * 0.5f);
    }
    /* Scale, then translate widgets so they all fit in panel */
    pfTranslate(GUI->transX, GUI->transY, 0.0f);
    pfScale(GUI->scaleX, GUI->scaleY, 1.0f);


    for (i = 0; i < GUI->numPanels; i++)
	drawPanel(GUI->panels[i], (redraw > 0));
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DITHER);
    pfPopMatrix();
    pfPopState();

    endTime = pfGetTime();
    GUI->guiDrawTime += (endTime - startTime);
    GUI->nGUIDraws++;

}

    /*----------------------------- pfuPanel ------------------------------*/

PFUDLLEXPORT pfuPanel*
pfuNewPanel(void)
{
    pfuPanel          *p;

    if (!GUI)
	getGUIDataPool(0);

    if (GUI->numPanels >= PFUGUI_MAXPANELS)
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
		 "NewpfuPanel: too many panels.");
	return (NULL);
    }
    p = (pfuPanel *) pfDPoolAlloc(GUIDP, sizeof(pfuPanel),
			       (PFUGUI_PANELS_DPID + GUI->numPanels));

    memset( p, 0, sizeof( pfuPanel ) );

    if (!p)
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
		 "NewPanel: failed to allocate panel in gui data pool.");
    }
    p->numWidgets = 0;
    p->listSize = 16;
    p->active = 1;
    p->id = GUI->numPanels;
    GUI->panels[GUI->numPanels] = p;
    GUI->numPanels++;
    p->dirty = GUI->refresh;

    return (p);
}

/*ARGSUSED*/
PFUDLLEXPORT void
pfuGetPanelOriginSize(pfuPanel * p, float *xo, float *yo, float *xs, float *ys)
{
    float           l, r, t, b;
    /* for now, the panel size is fixed, so this api is for later */
    /* XXX - GUI->panWIN X and Y are in screen coords for the in-out tests so
     * they need to be converted to channel coords where. probably should
     * store channel coords
     */
    /*(p, p);*/

    if (!GUI)
	getGUIDataPool(0);

    pfGetChanViewport(GUI->channel, &l, &r, &b, &t);
    *xo = l * GUI->pixSizeX + PFUGUI_PANEL_FRAME;
    *yo = b * GUI->pixSizeY + PFUGUI_PANEL_FRAME;
    *xs = GUI->pixSizeX - 2*PFUGUI_PANEL_FRAME;
    *ys = GUI->pixSizeY - 2*PFUGUI_PANEL_FRAME;
}

PFUDLLEXPORT void
pfuEnablePanel(pfuPanel * p)
{
    if (!GUI)
	getGUIDataPool(0);
  
    if (p != NULL)
    {
	pfDPoolLock(p);
	p->dirty = GUI->refresh;
	p->active = 1;
	pfDPoolUnlock(p);
	
	pfDPoolLock(GUI->flags);
	GUI->flags->dirty = GUI->flags->redrawAll = GUI->refresh;
	pfDPoolUnlock(GUI->flags);
  
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuEnablePanel() NULL panel.");
}


PFUDLLEXPORT void
pfuDisablePanel(pfuPanel * p)
{
    if (!GUI)
	getGUIDataPool(0);

    if (p != NULL)
    {
	pfDPoolLock(p);
	p->dirty = GUI->refresh;
	p->active = 0;
	pfDPoolUnlock(p);
	
	pfDPoolLock(GUI->flags);
	GUI->flags->dirty = GUI->flags->redrawAll = GUI->refresh;
	pfDPoolUnlock(GUI->flags);
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuDisablePanel() NULL panel.");
}

PFUDLLEXPORT void
pfuResetPanel(pfuPanel *p)
{
    static int	 inReset = 0;
    int		 j;
    pfuWidget	*w;

    if (inReset)
	return;

    if (!GUI)
	getGUIDataPool(0);

    inReset = 1;

    p->dirty = GUI->refresh;
    for (j = 0; j < p->numWidgets; j++)
    {
	w = p->widgets[j];
	if (w)
	    resetWidget(w);
    }
    pfDPoolLock(GUI->flags);
    GUI->flags->dirty = GUI->refresh;
    pfDPoolUnlock(GUI->flags);
    
    inReset = 0;
}

static void
drawPanel(pfuPanel *p, int redraw)
{
    pfuWidget      *w;
    int            i;

    if (GUI == NULL)
	return;

    pfDPoolLock(p);

    /* Return if panel is not dirty and GUI does not need redrawing */
    if ((!p->active) || (p->dirty <= 0 && !redraw))
    {
	pfDPoolUnlock(p);
	return;
    }
    p->dirty--;
    pfDPoolUnlock(p);

    /* Draw only dirty widgets in panel or all widgets is GUI needs redraw */
    for (i = 0; i < p->numWidgets; i++)
    {
	w = p->widgets[i];

	pfDPoolLock(w);
	if (redraw || (w->dirty > 0))
	{
	    drawWidget(w, p);
	    w->dirty--;
	}
	pfDPoolUnlock(w);
    }
}

static pfuWidget*
scanPanel(pfuPanel *p, int x, int y, int ymin)
{
    int            i;
    pfuWidget      *w;

    if (p == NULL)
	return (NULL);
    for (i = 0; i < p->numWidgets; i++)
    {
	w = (pfuWidget *) p->widgets[i];
	if (!(w->hidden) && (w->active) && x >= w->xorg &&
	    x <= (w->xorg + w->xsize))
	    if (y >= (w->yorg - ymin)
		&& y <= ((w->yorg - ymin) + w->ysize))
	    {
		return (w);
	    }
    }
    return (NULL);
}

	/*-------------------------- pfuWidget -------------------------*/

PFUDLLEXPORT pfuWidget*
pfuNewWidget(pfuPanel * p, int type, int id)
{
    pfuWidget         *w;
    int            dpid;

    if (!GUI)
	getGUIDataPool(0);

    if (p)
    {
	pfDPoolLock(p);
	if (p->numWidgets >= PFUGUI_MAXWIDGETS)
	{
	    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
		     "NewpfuWidget: too many widgets.");
	    pfDPoolUnlock(p);
	    return (NULL);
	}
	dpid = (PFUGUI_WIDGETS_DPID | (p->numWidgets));
    }
    else
    {
	dpid = id;
    }
    w = (pfuWidget *) pfDPoolAlloc(GUIDP, MAX_WIDGET_SIZE, dpid);

    memset( w, 0, MAX_WIDGET_SIZE );
    
    if (!w)
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
    "NewpfuWidget: failed to allocate widget %d of type 0x%x in gui data pool.",
		 id, type);
	return NULL;
    }

    if (p)
    {
	p->widgets[p->numWidgets] = w;
	p->numWidgets++;
	pfDPoolUnlock(p);

	w->panel = p;
    }
    initNewWidget(w, type, id);

    dirtyWidget(w);

    return (w);
}

PFUDLLEXPORT pfuWidget*
pfuNewWidgetBlock(pfuPanel* p, int id, int type, int num)
{
    pfuWidget         *w;
    int            dpid, i;

    if (!GUI)
	getGUIDataPool(0);

    if (p)
    {
	pfDPoolLock(p);
	if (p->numWidgets >= PFUGUI_MAXWIDGETS)
	{
	    pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
		     "pfuNewWidgetBlock() Too many widgets.");
	    pfDPoolUnlock(p);
	    return (NULL);
	}
	dpid = (PFUGUI_WIDGETS_DPID | (p->numWidgets));
    }
    else
    {
	dpid = id;
    }
    w = (pfuWidget *) pfDPoolAlloc(GUIDP, num * MAX_WIDGET_SIZE, dpid);

    if (!w)
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,
		 "pfuNewWidgetBlock() failed to allocate %d widgets %d "
		 "of type 0x%x in gui data pool.", num, id, type);

	return NULL;
    }

    memset( w, 0, num * MAX_WIDGET_SIZE );

    if (p)
    {
	for (i = 0; i < num; i++)
	{
	    p->widgets[p->numWidgets] = w;
	    p->numWidgets++;
	    w[i].panel = p;
	}
	pfDPoolUnlock(p);
    }

    for (i = 0; i < num; i++)
    {
	initNewWidget(&w[i], type, i);
    	dirtyWidget(w);
    }

    return (w);
}


static void
initNewWidget(pfuWidget * w, int type, int id)
{
    w->id = id;
    w->mode = 0;
    w->active = 1;
    w->hidden = 0;
    w->style = PFUGUI_NORMAL;
    w->type = type & PFUGUI_TYPE_VALS;
    w->min = 0.0f;
    w->max = 1.0f;
    w->xsize = 0.0f;
    w->ysize = 0.0f;
    w->drawFunc = NULL;
    w->selectFunc = NULL;
    w->actionFunc = NULL;
    
    switch (w->type & PFUGUI_TYPE_MASK)
    {
    case PFUGUI_SLIDER:
	{
	    Slider    *s =(Slider *) w;
	    s->hvFlag = !(type & PFUGUI_VERTICAL);
	    w->mode = STICKY_DFT;
	}
	break;
    case PFUGUI_SWITCH:
	w->min = 0.0f;
	w->val = w->defaultVal = 0.0f;
	w->max = 1.0f;
	break;
    case PFUGUI_MENU_BUTTON:
	{
	    MenuButton *m = (MenuButton *)w;
	    m->defaultSel = 0;
	}
	break;
    case PFUGUI_RADIO_BUTTON:
	{
	    RadioButton    *r =(RadioButton *) w;
	    r->hvFlag = !(type & PFUGUI_VERTICAL);
	    r->toggle = (type & PFUGUI_TYPE_VALS) == PFUGUI_RADIO_BUTTON_TOGGLE;
	    r->defaultSel = 0;
	    if (!(r->toggle))
	    {
		r->on = r->defaultOn = 1;
	    }
	    else
	    {
		r->on = r->defaultOn = 0;
	    }
	    break;
	}
    default:
	break;
    }
}

PFUDLLEXPORT void
pfuEnableWidget(pfuWidget *w)
{
    pfDPoolLock(w);
    w->active = 1;
    pfDPoolUnlock(w);

    dirtyWidget(w);
}

PFUDLLEXPORT void
pfuDisableWidget(pfuWidget *w)
{
    pfDPoolLock(w);
    w->active = 0;
    pfDPoolUnlock(w);

    dirtyWidget(w);
}

PFUDLLEXPORT void
pfuHideWidget(pfuWidget *w)
{
    pfDPoolLock(w);
    w->hidden = 1;
    pfDPoolUnlock(w);
    
    pfDPoolLock(GUI->flags);
    GUI->flags->fitDirty = 1;
    pfDPoolUnlock(GUI->flags);

    dirtyWidget(w);
}

PFUDLLEXPORT void
pfuUnhideWidget(pfuWidget *w)
{
    pfDPoolLock(w);
    w->hidden = 0;
    pfDPoolUnlock(w);

    pfDPoolLock(GUI->flags);
    GUI->flags->fitDirty = 1;
    pfDPoolUnlock(GUI->flags);

    dirtyWidget(w);
}

PFUDLLEXPORT void
pfuResetWidget(pfuWidget *w)
{
    resetWidget(w);
}

static void
resetWidget(pfuWidget *w)
{
    switch (w->type & PFUGUI_TYPE_MASK)
    {
    case PFUGUI_SLIDER:
    case PFUGUI_SWITCH:
	pfuWidgetValue(w, w->defaultVal);
	break;
    case PFUGUI_MENU_BUTTON:
	pfuWidgetSelection(w, ((MenuButton *)w)->defaultSel);
	break;
    case PFUGUI_RADIO_BUTTON:
	pfuWidgetSelection(w, ((RadioButton *)w)->defaultSel);
	pfuWidgetOnOff(w, ((RadioButton *)w)->defaultOn);
	break;
    default: break;
    }
    if (((w->type & PFUGUI_TYPE_MASK) != PFUGUI_BUTTON) && w->actionFunc)
    {
	(*(w->actionFunc)) (w);
    }
}

#if 0
static int
getWidgetSize(int type)
{
    switch(type & PFUGUI_TYPE_MASK)
    {
    case PFUGUI_SLIDER:
	return sizeof(Slider);
    case PFUGUI_RADIO_BUTTON:
	return sizeof(RadioButton);
    case PFUGUI_MENU_BUTTON:
	return sizeof(MenuButton);
    default:
	return sizeof(pfuWidget);
    }
}
#endif

PFUDLLEXPORT void
pfuWidgetDim(pfuWidget * w, int xo, int yo, int xs, int ys)
{
    pfDPoolLock(w);
    widgetDim(w, xo, yo, xs, ys);
    positionWidgetAttrs(w);
    pfDPoolUnlock(w);
    dirtyWidget(w);
}

PFUDLLEXPORT void
pfuGetWidgetDim(pfuWidget * w, int *xo, int *yo, int *xs, int *ys)
{
    *xo = w->xorg;
    *yo = w->yorg;
    *xs = w->xsize;
    *ys = w->ysize;
}

static void
widgetDim(pfuWidget * w, int xo, int yo, int xs, int ys)
{
    if ((xs != w->xsize) || (ys != w->ysize))
    {
	pfDPoolLock(GUI->flags);
	GUI->flags->fitDirty = 1;
	pfDPoolUnlock(GUI->flags);
    }
    
    w->xorg = xo;
    w->yorg = yo;
    w->xsize = xs;
    w->ysize = ys;

    /* set the widget coords */
    PFSET_VEC2(w->coord[0], xo, yo);
    PFSET_VEC2(w->coord[1], xo + xs, yo);
    PFSET_VEC2(w->coord[2], xo + xs, yo + ys);
    PFSET_VEC2(w->coord[3], xo, yo + ys);

    /* set the state indicator coords */
    if (w->type & PFUGUI_BUTTON)
    {
	PFSET_VEC2(w->icoord[0], xo + xs - 16, yo + 8);
	PFSET_VEC2(w->icoord[1], xo + xs - 8, yo + 8);
	PFSET_VEC2(w->icoord[2], xo + xs - 8, yo + 16);
	PFSET_VEC2(w->icoord[3], xo + xs - 16, yo + 16);
    }
    w->tx = (xo + 8);
    w->ty = ((yo + ys) - 16);

    switch (w->type & PFUGUI_TYPE_VALS)
    {
    case PFUGUI_SLIDER:
    case PFUGUI_SLIDER_LOG:
	{
	    Slider         *s =(Slider *) w;

	    if (s->hvFlag)  /* horizontal */
	    {
		/* slider bar */
		s->lt[0] = s->xorg + 16;
		s->lt[1] = s->ty - 4;
		s->lb[0] = s->xorg + 16;
		s->lb[1] = s->yorg + 4;
		s->rt[0] = (s->xorg + s->xsize) - 16;
		s->rt[1] = s->lt[1];
		s->rb[0] = (s->xorg + s->xsize) - 16;
		s->rb[1] = s->lb[1];

		s->start = s->lt[0];
		if (s->rt[0] - s->lt[0] > 0.0f)
		    s->scale = (s->smax - s->smin) / (s->rt[0] - s->lt[0]);
	    }
	    else    /* vertical */
	    {
		int width = s->xsize / 3;
		if (s->nameWidth)
		    s->tx = s->xorg + (s->xsize - s->nameWidth) * 0.5f;
		if (s->minStrWidth)
		    s->minx = s->xorg + (s->xsize - s->minStrWidth) * 0.5f;
		if (s->maxStrWidth)
		    s->maxx = s->xorg + (s->xsize - s->maxStrWidth) * 0.5f;
		s->ty = ((yo + ys) - 16); /* min string below title */
		/* slider coords */
		s->lt[0] = s->xorg + width;
		s->rt[0] = (s->xorg + s->xsize) - width;
		s->lb[0] = s->lt[0];
		s->rb[0] = s->rt[0];
		s->lt[1] = s->ty - 32;
		s->lb[1] = s->yorg + 20; /* leave room for max string at bottom */

		s->rt[1] = s->lt[1];

		s->rb[1] = s->lb[1];
		s->start = s->lt[1];
		if (s->rt[1] - s->rb[1] > 0.0f)
		    s->scale = (s->smax - s->smin) / (s->rt[1] - s->rb[1]);
	    }
	    break;
	}

    case PFUGUI_RADIO_BUTTON_TOGGLE:
	{
	    RadioButton    *r =(RadioButton *) w;
	    r->toggle = 1;
	    /* setup for indicator button */
	    PFSET_VEC2(r->icoord[0], r->xorg + 8, r->ty);
	    PFSET_VEC2(r->icoord[1], r->xorg + 16, r->ty);
	    PFSET_VEC2(r->icoord[2], r->xorg + 16, r->ty + 8);
	    PFSET_VEC2(r->icoord[3], r->xorg + 8, r->ty + 8);
	    r->tx = r->xorg + 24;
	    break;
	}

    case PFUGUI_RADIO_BUTTON_ATTR:
	{
	    PFSET_VEC2(w->icoord[0], w->xorg + 8, w->ty);
	    PFSET_VEC2(w->icoord[1], w->xorg + 16, w->ty);
	    PFSET_VEC2(w->icoord[2], w->xorg + 16, w->ty + 8);
	    PFSET_VEC2(w->icoord[3], w->xorg + 8, w->ty + 8);
	    w->tx = (w->xorg + 24);

	    break;
	}

    case PFUGUI_MENU_BUTTON:
	{
	    MenuButton     *m =(MenuButton *) w;
	    /* setup for menu arrow */
	    PFSET_VEC2(m->icoord[0], m->xorg + m->xsize - 16, m->yorg + 8);
	    PFSET_VEC2(m->icoord[1], m->xorg + m->xsize - 8, m->yorg + 12);
	    PFSET_VEC2(m->icoord[2], m->xorg + m->xsize - 16, m->yorg + 16);
	    break;
	}

    default:
	break;
    }
}

static void
fitWidgets(void)
{
    int            i, j, xmax, ymax, xmin, ymin, xsize, ysize;
    pfuPanel          *p;
    pfuWidget         *w;
    int	    foundWidget = 0, start;

    if (GUI->numPanels <= 0)
	return;

    pfDPoolLock(GUI->flags);
    GUI->flags->fitDirty = 0;
    pfDPoolUnlock(GUI->flags);
    FontFlags |= FONT_SCALE_DIRTY;

    /* find the max/max widget coords over all the widgets */
    for (j = 0; j < GUI->numPanels; j++)
    {
	p = GUI->panels[j];

	if (p->numWidgets)
	{

	    w = p->widgets[0];

	    if (!foundWidget)
	    {
		xmax = w->xorg + w->xsize - 1;
		xmin = w->xorg;
		ymax = w->yorg + w->ysize - 1;
		ymin = w->yorg;
		foundWidget = 1;
		start = 1;
	    }
	    else
		start = 0;

	    for (i = start; i < p->numWidgets; i++)
	    {
		w = p->widgets[i];
		if (!w->hidden)
		{
		    xmax = PF_MAX2(xmax, w->xorg + w->xsize - 1);
		    ymax = PF_MAX2(ymax, w->yorg + w->ysize - 1);
		    xmin = PF_MIN2(xmin, w->xorg);
		    ymin = PF_MIN2(ymin, w->yorg);
		}
	    }
	}
    }
    /*  XXX - padding seems necesary */
    ymin -= 8;

    if (!foundWidget || (xmax <= xmin) || (ymax <= ymin))
	return;

    xsize = xmax - xmin;
    ysize = ymax - ymin;

    if (xsize > 0.0f)
	    GUI->scaleX = (float) (GUI->pixSizeX - 2*PFUGUI_PANEL_FRAME) / (float) xsize;
    else
	GUI->scaleX = 1.0f;

    if (ysize > 0.0f)
	    GUI->scaleY = (float) (GUI->pixSizeY - PFUGUI_PANEL_FRAME) / (float) ysize;
    else
	GUI->scaleX = 1.0f;

    xmin *= GUI->scaleX;
    ymin *= GUI->scaleY;
    xsize *= GUI->scaleX;
    ysize *= GUI->scaleY;

    /* Left justify x */
    GUI->transX = -xmin + PFUGUI_PANEL_FRAME;

    /* Top justify y */
    GUI->transY = -(ymin + ysize) + (GUI->pixSizeY - PFUGUI_PANEL_FRAME);
}

PFUDLLEXPORT void
pfuWidgetLabel(pfuWidget * w, const char *name)
{
    pfDPoolLock(w);

    strncpy(w->name, name, (PF_MAXSTRING - 1));
    w->name[(PF_MAXSTRING - 1)] = '\0';
    w->nameWidth = stringwidth(Fonts[TEXT_FONTS][PFU_FONT_MED], w->name);

    if ((w->type & PFUGUI_SLIDER) && (w->type & PFUGUI_VERTICAL) &&  w->xsize)
	w->tx = w->xorg + (w->xsize - w->nameWidth) * 0.5f;

    pfDPoolUnlock(w);

    dirtyWidget(w);
}

PFUDLLEXPORT const char *
pfuGetWidgetLabel(pfuWidget * w)
{
    return(w->name);
}

PFUDLLEXPORT int
pfuGetWidgetLabelWidth(pfuWidget * w)
{
    int findex=FONT_SIZE_TEXT;

    switch (w->type & PFUGUI_TYPE_MASK)
    {
	case PFUGUI_MESSAGE:
	    findex = FONT_SIZE_VAL;
	    break;
	default:
	    break;
    }
    return stringwidth(Fonts[TEXT_FONTS][findex], w->name);
}

PFUDLLEXPORT int
pfuGetWidgetId(pfuWidget * w)
{
    return(w->id);
}

PFUDLLEXPORT int
pfuGetWidgetType(pfuWidget * w)
{
    return(w->type);
}


PFUDLLEXPORT void	
pfuWidgetMode(pfuWidget * w, int mode, int val)
{
    switch (mode)
    {
    case PFUGUI_STICKY_DEFAULT:
	PFFLAG_BOOL_SET(w->mode, STICKY_DFT, val);
	break;
    }
}

PFUDLLEXPORT int	
pfuGetWidgetMode(pfuWidget * w, int mode)
{
    switch (mode)
    {
    case PFUGUI_STICKY_DEFAULT:
	return PFFLAG_BOOL_GET(w->mode, STICKY_DFT);
    }
    return -1;
}

PFUDLLEXPORT float
pfuGetWidgetValue(pfuWidget * w)
{
    return(w->val);
}

PFUDLLEXPORT float
pfuGetWidgetDefaultValue(pfuWidget * w)
{
    return(w->defaultVal);
}

PFUDLLEXPORT int
pfuGetWidgetDefaultSelection(pfuWidget * w)
{
    switch(w->type & PFUGUI_TYPE_MASK)
    {
    case PFUGUI_RADIO_BUTTON:
	return ((RadioButton *) w)->defaultSel;
    case PFUGUI_MENU_BUTTON:
	return ((MenuButton *) w)->defaultSel;
    default:
	return 0;
    }
}

PFUDLLEXPORT int
pfuGetWidgetSelection(pfuWidget * w)
{
    switch(w->type & PFUGUI_TYPE_MASK)
    {
    case PFUGUI_SLIDER:
	return((Slider *) w)->val;
    case PFUGUI_RADIO_BUTTON:
	return ((RadioButton *) w)->sel;
    case PFUGUI_MENU_BUTTON:
	return ((MenuButton *) w)->sel;
    default:
	return -1;
    }
}

PFUDLLEXPORT void
pfuWidgetRange(pfuWidget * w, int mode, float min, float max, float initial)
{
    pfDPoolLock(w);
    w->dv = mode;
    w->min = min;
    w->max = max;
    w->val = initial;
    w->defaultVal = w->oldVal = w->val;

    if (w->type & PFUGUI_SLIDER)
    {
	Slider         *s =(Slider *) w;

	/* set slider relative values */
	switch (s->type & PFUGUI_TYPE_VALS)
	{
	case PFUGUI_SLIDER:
	    s->sval = s->sDefaultVal = s->sOldVal = s->val;
	    s->smin = min;
	    s->smax = max;
	    break;
	case PFUGUI_SLIDER_LOG:
	    if (s->val <0.0001f)
		s->val = 0.0001f;
	    s->sval = s->sDefaultVal = s->sOldVal = log10f(s->val);
	    if (min <0.001f)
		s->min = 0.001f;
	    s->smin = log10f(s->min);
	    s->smax = log10f(s->max);
	    break;
	}
	if (s->hvFlag)
	{
	    if (s->rt[0] - s->lt[0] > 0.0f)
		s->scale = (s->smax - s->smin) / (s->rt[0] - s->lt[0]);
	}
	else
	if (s->rt[1] - s->rb[1] > 0.0f)
	    s->scale = (s->smax - s->smin) / (s->rt[1] - s->rb[1]);

	sprintf(s->minstr, "%3.2f", min);
	sprintf(s->maxstr, "%3.2f", max);
	s->minStrWidth = stringwidth(Fonts[TEXT_FONTS][FONT_SIZE_VAL], s->minstr) + 4;
	s->maxStrWidth = stringwidth(Fonts[TEXT_FONTS][FONT_SIZE_VAL], s->maxstr);
	if (s->minStrWidth)
	    s->minx = s->xorg + (s->xsize - s->minStrWidth) * 0.5f;
	if (s->maxStrWidth)
	    s->maxx = s->xorg + (s->xsize - s->maxStrWidth) * 0.5f;
    }

    pfDPoolUnlock(w);

    dirtyWidget(w);
}


PFUDLLEXPORT void
pfuWidgetValue(pfuWidget * w, float newValue)
{
    setWidgetValue(w, 0, newValue);
}

PFUDLLEXPORT void
pfuWidgetDefaultValue(pfuWidget * w, float newValue)
{
    setWidgetValue(w, 1, newValue);
}

/*
 * setWidgetValue - tries to do the right thing for each kind
 *	of widget with the new value.
 *	Its straighforward for sliders.
 *	Labels are not affected.
 *	Menu buttons and Radio buttons look through their attrs
 *	    for a matching value and then select that index.
 *	Switches and buttons treat it as a TRUE(!=0) FALSE(=0) toggle.
 */
static void
setWidgetValue(pfuWidget * w, int dft, float newValue)
{
    pfDPoolLock(w);
    switch (w->type & PFUGUI_TYPE_VALS)
    {
    case PFUGUI_SLIDER:
	{
	    Slider         *s =(Slider *) w;
	    if (!dft)
	    {
		s->val = s->oldVal = newValue;
		s->sval = s->sOldVal = newValue;
	    }
	    else
	    {
		s->defaultVal = s->sDefaultVal = newValue;
	    }
	}
	break;
    case PFUGUI_SLIDER_LOG:
	{
	    Slider         *s =(Slider *) w;
	    if (!dft)
	    {
		s->val = s->oldVal = newValue;
		s->sval = s->sOldVal = log10f(newValue);
	    }
	    else
	    {
		s->defaultVal = s->sDefaultVal = log10f(newValue);
	    }
	}
	break;
    case PFUGUI_SWITCH:
	if (!dft)
	{
	    if (PF_ABSLT(w->min - newValue, 0.01f))
	    {
		w->val = w->min;
	    }
	    else
	    {
		w->val = w->max;
	    }
	}
	else
	{
	    w->defaultVal = newValue;
	}
	break;
    case PFUGUI_BUTTON:
	if (!dft)
	{
	    if (newValue != 0.0f)
	    {
		w->style = PFUGUI_HILIGHT;
		w->val = newValue;
		w->state = 1;
	    }
	    else
	    {
		w->val = 0.0f;
		w->state = 0;
		w->style = PFUGUI_NORMAL;
	    }
	}
	break;
    case PFUGUI_MENU_BUTTON:
	{
	    MenuButton *m =(MenuButton *) w;
	    int sel=(-1), i=0, l=(m->numSelections);
	    for (i=0; i < l; i++)
	    {
		if (PF_ABSLT(m->selectionList[i].val - newValue, 0.00001f))
		{
		    sel = i;
		    break;
		}
	    }
	    if (!dft)
	    {
		if (sel > -1)
		{
		    m->sel = m->oldSel = sel;
		    m->val = m->oldVal = m->selectionList[m->sel].val;
		}
		else
		{
		    pfNotify(PFNFY_WARN, PFNFY_PRINT,
			"pfuWidgetValue of PFUGUI_MENU_BUTTON: "
			"could find index for value of %f", newValue);
		}
	    }
	    else
	    if (m->defaultSel != sel) {
		m->defaultSel = sel;
		m->defaultVal = m->selectionList[sel].val;
		m->menu_dirty = 1;
	    }
	}
	break;
    case PFUGUI_RADIO_BUTTON: case PFUGUI_RADIO_BUTTON_TOGGLE:
	{
	    RadioButton *r =(RadioButton *) w;
	    int sel=(-1), i=0, l=(r->numSelections);
	    for (i=0; i < l; i++)
	    {
		if (PF_ABSLT(r->selectionList[i].val - newValue, 0.00001f))
		{
		    sel = i;
		    break;
		}
	    }
	    if (sel == -1)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
		    "pfuWidgetValue of PFUGUI_RADIO_BUTTON: "
		    "could find index for value of %f", newValue);
	    }
	    else
	    {	/* changing selection selection  */
		if (!dft)
		{
		    r->sel = r->oldSel = sel;
		    r->val = r->oldVal =  r->selectionList[sel].val;
		}
		else
		{
		    r->defaultSel = sel;
		    r->defaultVal = r->selectionList[sel].val;
		}
	    }
	}
	break;
    }
    pfDPoolUnlock(w);

    dirtyWidget(w);
}

PFUDLLEXPORT void
pfuWidgetSelection(pfuWidget * w, int index)
{
    setWidgetSelection( w, 0,  index);
}

PFUDLLEXPORT void
pfuWidgetDefaultSelection(pfuWidget * w, int index)
{
    setWidgetSelection( w, 1,  index);
}

/*
 *	Tries to do the right thing for each kind
 *	of widget with the new index.
 *	Menu buttons and Radio buttons use the index to select an selection.
 *	An index of (-1) can be used to turn off a Radio button.
 *	Sliders and labels are not affected.
 *	Switches and buttons treat it as a TRUE(!=0) FALSE(=0) toggle.
 */
static void
setWidgetSelection(pfuWidget * w, int dft, int index)
{
    pfDPoolLock(w);
    switch (w->type & PFUGUI_TYPE_MASK)
    {
    case PFUGUI_SLIDER:
	break;
    case PFUGUI_SWITCH:
	if (!dft)
	{
	    if (index == 0)
	    {
		w->val = w->min;
	    }
	    else
	    {
		w->val = w->max;
	    }
	}
	else
	{
	    if (index == 0)
	    {
		w->defaultVal = w->min;
	    }
	    else
	    {
		w->defaultVal = w->max;
	    }
	}
	break;
    case PFUGUI_BUTTON:
	if (!dft)
	{
	    if (index == 1)
	    {
		w->style = PFUGUI_HILIGHT;
		w->val = 1.0f;
		w->state = 1;
	    }
	    else
	    {
		w->val = 0.0f;
		w->state = 0;
		w->style = PFUGUI_NORMAL;
	    }
	}
	break;
    case PFUGUI_MENU_BUTTON:
	{
	    MenuButton *m =(MenuButton *) w;
	    if ((index > -1) && index < m->numSelections)
	    {
		if (!dft)
		{
		    m->sel = m->oldSel = m->sel = index;
		    m->val = m->selectionList[index].val;
		}
		else
		{
		    m->defaultSel = index;
		    m->defaultVal = m->selectionList[index].val;
		}
	    }
	    else
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
		    "pfuWidgetSelection of PFUGUI_MENU_BUTTON: "
		    "bad index of %d", index);
	    }
	}
	break;
    case PFUGUI_RADIO_BUTTON:
	{
	    RadioButton *r =(RadioButton *) w;
	    if (!dft && index == -1 &&
		((w->type & PFUGUI_TYPE_VALS) == PFUGUI_RADIO_BUTTON_TOGGLE))
	    {	/* have title - do switch */
		if (!r->on)
		{
		    r->on = 1;
		    r->val = r->selectionList[r->sel].val;
		}
		else
		    r->on = 0;
		r->state = 0;
	    }
	    else 
	    if (index > -1)
	    {	/* changing selection selection */
		if (!dft)
		{
		    r->sel = index;
		    r->val = r->selectionList[index].val;
		    r->state = 0;
		}
		else
		{
		    r->defaultSel = index;
		    r->defaultVal = r->selectionList[index].val;
		}
	    }

	}
	break;
    }

    pfDPoolUnlock(w);

    dirtyWidget(w);
}

/*
 * pfuWidgetOnOff - tries to do the right thing for each kind
 *	of widget with an on/off select.
 *	It can be used to turn off a Radio button.
 *	Sliders and labels are not affected.
 *	Switches and buttons treat it as a TRUE(!=0) FALSE(=0) toggle.
 */
PFUDLLEXPORT void
pfuWidgetOnOff(pfuWidget * w, int on)
{
    pfDPoolLock(w);

    switch(w->type & PFUGUI_TYPE_MASK)
    {
	case PFUGUI_SWITCH:
	    if (on == 0)
	    {
		w->val = w->min;
	    }
	    else
	    {
		w->val = w->max;
	    }
	    break;
	case PFUGUI_BUTTON:
	    if (on == 1)
	    {
		w->style = PFUGUI_HILIGHT;
		w->val = 1.0f;
		w->state = 1;
	    }
	    else
	    {
		w->val = 0.0f;
		w->state = 0;
		w->style = PFUGUI_NORMAL;
	    }
	    break;
	case PFUGUI_RADIO_BUTTON:
	    ((RadioButton *) w)->on = (on == 1);
	    break;
	default:
	    break;
    }

    pfDPoolUnlock(w);

    dirtyWidget(w);
}

PFUDLLEXPORT void
pfuWidgetDefaultOnOff(pfuWidget * w, int on)
{
    switch(w->type & PFUGUI_TYPE_MASK)
    {
	case PFUGUI_SWITCH:
	    if (on == PF_OFF)
	    {
		w->defaultVal = w->min;
	    }
	    else
	    {
		w->defaultVal = w->max;
	    }
	    break;
	case PFUGUI_RADIO_BUTTON:
	    ((RadioButton *) w)->defaultOn = (on == 1);
	    break;
	default:
	    break;
    }
}

PFUDLLEXPORT int
pfuIsWidgetOn(pfuWidget * w)
{
    switch(w->type & PFUGUI_TYPE_MASK)
    {
	case PFUGUI_RADIO_BUTTON:
	    return((RadioButton *) w)->on;
	case PFUGUI_SWITCH:
	    return (!(w->val == w->min));
	case PFUGUI_BUTTON:
	    return (!(w->val == 0.0f));
    default:
	return -1;
    }
}

PFUDLLEXPORT void
pfuWidgetDrawFunc(pfuWidget * w, pfuWidgetDrawFuncType func)
{
    pfDPoolLock(w);
    w->drawFunc = func;
    pfDPoolUnlock(w);
    dirtyWidget(w);
}

PFUDLLEXPORT void
pfuWidgetSelectFunc(pfuWidget * w, pfuWidgetSelectFuncType func)
{
    pfDPoolLock(w);
    w->selectFunc = func;
    pfDPoolUnlock(w);
}

PFUDLLEXPORT void
pfuWidgetActionFunc(pfuWidget * w, pfuWidgetActionFuncType func)
{
    pfDPoolLock(w);
    w->actionFunc = func;
    pfDPoolUnlock(w);
}

PFUDLLEXPORT pfuWidgetActionFuncType
pfuGetWidgetActionFunc(pfuWidget * w)
{
    if (w)
	return w->actionFunc;
    else
	return NULL;
}

PFUDLLEXPORT pfuWidgetSelectFuncType
pfuGetWidgetSelectFunc(pfuWidget * w)
{
    if (w)
	return w->selectFunc;
    else
	return NULL;
}

PFUDLLEXPORT pfuWidgetDrawFuncType
pfuGetWidgetDrawFunc(pfuWidget * w)
{
    if (w)
	return w->drawFunc;
    else
	return NULL;
}

static void
positionWidgetAttrs(pfuWidget * w)
{
    int             i;
    int            xp, yp;
    RadioButton	    *r = 0;
    MenuButton	    *m = 0;
    
    if (!(w->type & PFUGUI_RADIO_BUTTON))
	return;

    switch (w->type & PFUGUI_TYPE_MASK)
    {
    case PFUGUI_RADIO_BUTTON:

	r = (RadioButton *) w;
	if (r->numSelections == 0)
	    break;
    	    
	/* Setup start of selection positioning */
	if (r->hvFlag)
	{	/* radio selections go horizontally */
	    xp = r->xorg + r->nameWidth + (34);
	    yp = r->yorg;
	}
	else	/* Radio selections go down vertically */
	{
	    xp = r->xorg;
	    yp = r->yorg + r->ysize - (2*PFUGUI_RADIO_BUTTON_YSIZE + 4);
	}
	break;
    default:
	break;
    }
    
    if (r && r->selectionList && (r->numSelections > 0))
    {
	for (i = 0; i < r->numSelections; i++)
	{
	    switch (w->type & PFUGUI_TYPE_MASK)
	    {
	    case PFUGUI_RADIO_BUTTON:
		/* do successive selection positioning for radio buttons */
		if (r->hvFlag)
		{
		    /* is horizontal */
		    widgetDim(&(r->selectionList[i]), xp, yp,
		      r->selectionList[i].nameWidth + 28, PFUGUI_RADIO_BUTTON_YSIZE);
		    xp += r->selectionList[i].xsize;
		}
		else
		{
		    /* is vertical */
		    widgetDim(&(r->selectionList[i]), xp, yp,
		      PFUGUI_RADIO_BUTTON_XSIZE, PFUGUI_RADIO_BUTTON_YSIZE);
		    yp -= r->selectionList[i].ysize;
		}
		break;
	    default:
		break;
	    }
	}
    }
}

PFUDLLEXPORT void
pfuWidgetSelections(pfuWidget * w, pfuGUIString * selectionList,
		    int *valList, void(**funcList)(pfuWidget *),
		    int numSelections)
{
    int             i;
    int		    type, atype;
    pfuWidget       **selection;
    RadioButton	    *r = 0;
    MenuButton	    *m = 0;

    if (!((w->type & PFUGUI_RADIO_BUTTON) || (w->type & PFUGUI_MENU_BUTTON)))
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfuWidgetSelections() Bad widget type 0x%x.", w->type);

    pfDPoolLock(w);
    
    type = w->type;
    switch (type & PFUGUI_TYPE_MASK)
    {
    case PFUGUI_RADIO_BUTTON:

	r = (RadioButton *) w;
	r->numSelections = numSelections;
	selection = (pfuWidget **) &(r->selectionList);
	atype = PFUGUI_RADIO_BUTTON_ATTR;
	break;
    case PFUGUI_MENU_BUTTON:
	m = (MenuButton *) w;
	atype = PFUGUI_MENU_BUTTON_ATTR;
	m->numSelections = numSelections;
	selection = (pfuWidget **) &(m->selectionList);
	m->menu_dirty = 1;
	break;
    }

    if (selectionList && (numSelections > 0))
    {
	if (*selection != NULL)
	{
	    /* free old selection list */
	    pfDPoolFree(GUIDP, *selection);
	}
	*selection = pfuNewWidgetBlock(NULL, PFUGUI_ATTRSBLOCK_DPID + (w->id),
			       atype, numSelections);
	for (i = 0; i < numSelections; i++)
	{
	    strcpy((*selection)[i].name, selectionList[i]);
	    (*selection)[i].nameWidth =
		stringwidth(Fonts[TEXT_FONTS][FONT_SIZE_VAL], (*selection)[i].name);
	    if (valList)
		(*selection)[i].val = valList[i];
	    else
		(*selection)[i].val = i;

	    if (funcList)
		(*selection)[i].actionFunc = funcList[i];
	    else
		(*selection)[i].actionFunc = w->actionFunc;
	}
    }
    positionWidgetAttrs(w);
    pfDPoolUnlock(w);
    dirtyWidget(w);
}

/*
 * Warning: Scanning is fragile - isn't using for speed,
 * but is only looking at things that _never_ change
 */
static int
scanSelections(RadioButton * r, int x, int y, int ymin)
{
    int             i;

    /* first check title */
    if (((r->hvFlag) && x <= (r->selectionList[0].xorg)) ||
	(!(r->hvFlag) &&
    (y >= ((r->selectionList[0].yorg + r->selectionList[0].ysize) - ymin))))
	return (-1);

    for (i = 0; i < r->numSelections; i++)
    {
	if ((y >= (r->selectionList[i].yorg - ymin)) &&
	    (y <= ((r->selectionList[i].yorg - ymin) +
		   r->selectionList[i].ysize)))
	    if ((x >= r->selectionList[i].xorg) &&
	     (x <= (r->selectionList[i].xorg + r->selectionList[i].xsize)))
		return (i);
    }
    return (-2);
}

/*
 * Propogate dirty flag from widget to panel and GUI so the widget gets redrawn.
*/
static void
dirtyWidget(pfuWidget * w)
{
    pfDPoolLock(w);
    w->dirty = GUI->refresh;
    pfDPoolUnlock(w);

    if (w->panel)
    {
    	pfDPoolLock(w->panel);
    	w->panel->dirty = GUI->refresh;
    	pfDPoolUnlock(w->panel);
    }

    pfDPoolLock(GUI->flags);
    GUI->flags->dirty = GUI->refresh;
    pfDPoolUnlock(GUI->flags);
}

/*
 *  drawWidget is fragile - doesn't lock the widget
 *  so it should always be called within a lock
 */
static void
drawWidget(pfuWidget *w, pfuPanel *p)
{
    int            i;
    char            str[80];
    pfVec3	   textColor, valColor;

    if (w == NULL)
	return;

    if (w->hidden)
	return;
	
    if (w->drawFunc)
    { /* use user-specified draw function */
	(w->drawFunc)(w, p);
	return;
    }

    if (w->active)
    {
	PFCOPY_VEC3(textColor, TextColor);
	PFCOPY_VEC3(valColor, ValColor);
    }
    else
    {
	PFCOPY_VEC3(textColor, InactiveTextColor);
	PFCOPY_VEC3(valColor, InactiveTextColor);
    }
    switch (w->type & PFUGUI_TYPE_MASK)
    {
    case PFUGUI_MESSAGE:
	mysetfont(Fonts[NUMBER_FONTS][FONT_SIZE_VAL]);
	drawBevel(w->coord, MessageColor, 3);
	drawBox(w->coord, MessageColor, -1);
	drawBevel(w->coord, MessageColor, -2);
	drawString(w->tx, w->ty, valColor, w->name);
	mysetfont((*CurFont));
	break;

    case PFUGUI_BUTTON:
	mysetfont(Fonts[TEXT_FONTS][FONT_SIZE_TITLE]);
	if (w->style == PFUGUI_HILIGHT)
	{
	    drawBevel(w->coord, BackColor, 3);
	    drawBox(w->coord, BackColor, -1);
	    drawBevel(w->coord, BackColor, -3);
	}
	else
	{
	    drawBevel(w->coord, BackColor, 3);
	    drawBox(w->coord, BackColor, 1);
	    drawBevel(w->coord, BackColor, -1);
	}

	drawString(w->tx, w->ty, textColor, w->name);
	if (w->dv)
	{
	    if (w->dv == PFUGUI_INT)
	    {
		i = (int) w->val;
		sprintf(str, "%ld", i);
	    }
	    else
	    {
		sprintf(str, "  %3.2f", w->val);
	    }
	    drawString(w->tx, w->ty - 16, textColor, str);
	}
	break;

    case PFUGUI_SWITCH:
	mysetfont(Fonts[TEXT_FONTS][FONT_SIZE_TITLE]);
	drawSwitchButton(w->coord, w->icoord, w->val != w->min);
	drawString(w->tx, w->ty, textColor, w->name);
	if (w->dv)
	{
	    if (w->dv == PFUGUI_INT)
	    {
		i = (int) w->val;
		sprintf(str, "%ld", i);
	    }
	    else
	    {
		sprintf(str, "  %3.2f", w->val);
	    }
	    drawString(w->tx, w->ty - 16, textColor, str);
	}
	break;

    case PFUGUI_SLIDER:
	{
	    Slider         *s =(Slider *) w;
	    int            hvflag = s->hvFlag;
	    pfVec2          coord[4];
	    float           gxl;
	    float           stmp = 1.0f /(s->scale);
	    int            minWidth = s->minStrWidth;
	    int            maxWidth = s->maxStrWidth;
	    int            width;
	    int            tx;

	    if ((s->scale) == 0.0f)
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "warning: slider has range scale of 0!");
	    drawBevel(s->coord, BackColor, 3);
	    drawBox(s->coord, BackColor, -1);
	    drawBevel(s->coord, BackColor, -2);

	    /* print current value after name */
	    sprintf(str, "  %5.2f", w->val);

	    /* print current, min and max values */
	    if (hvflag)
	    {			/* horizontal */
		/* name location */
		tx = s->tx + minWidth + ((s->xsize - minWidth - maxWidth) / 2)
		    - ((s->nameWidth + 80) / 2.0f);
		drawString(tx, s->ty, textColor, s->name);
		mysetfont(Fonts[NUMBER_FONTS][FONT_SIZE_VAL]);
		glColor3fv(valColor);
		dspstring(str);
		drawString(s->tx, s->ty, valColor, s->minstr);
		drawString((s->tx + s->xsize) - (maxWidth + 16),
			   s->ty, valColor, s->maxstr);
	    }
	    else
	    {
		/* vertical */
		drawString(s->tx, s->ty, textColor, s->name);
		mysetfont(Fonts[NUMBER_FONTS][FONT_SIZE_VAL]);
		width = stringwidth(Fonts[TEXT_FONTS][FONT_SIZE_VAL], str);
		tx = s->maxx + (maxWidth - width);
		drawString(tx, s->ty - 12, valColor, str);
		drawString(s->minx, s->ty - 30, valColor, s->minstr);
		drawString(s->maxx, s->yorg + 4, valColor, s->maxstr);
	    }

	    /* XXX these coords should be cached */
	    if (hvflag)
	    {			/* horizontal */
		/* draw the range bar as raised bevel */
		PFSET_VEC2(coord[0], s->lb[0] - 1, s->lb[1] + 4);
		PFSET_VEC2(coord[1], s->rt[0] + 1, s->lb[1] + 4);
		PFSET_VEC2(coord[2], s->rt[0] + 1, s->rt[1] - 4);
		PFSET_VEC2(coord[3], s->lb[0] - 1, s->rt[1] - 4);
		drawBox(coord, BackColor, 1);
		drawBevel(coord, BackColor, 2);

		/* draw the slider grip as raised bevel */
		gxl = (stmp * (s->sval - s->smin)) + s->lt[0];
		PFSET_VEC2(coord[0], gxl, s->lb[1] + 2);
		PFSET_VEC2(coord[1], gxl + 1, s->lb[1] + 2);
		PFSET_VEC2(coord[2], gxl + 1, s->rt[1] - 2);
		PFSET_VEC2(coord[3], gxl, s->rt[1] - 2);
	    }
	    else
	    {			/* vertical */
		/* draw the range bar as raised bevel */
		PFSET_VEC2(coord[0], s->lb[0] + 4, s->lb[1] + 1);
		PFSET_VEC2(coord[1], s->rt[0] - 4, s->lb[1] + 1);
		PFSET_VEC2(coord[2], s->rt[0] - 4, s->rt[1] - 1);
		PFSET_VEC2(coord[3], s->lb[0] + 4, s->rt[1] - 1);
		drawBox(coord, BackColor, 1);
		drawBevel(coord, BackColor, 2);

		/* draw the slider grip as raised bevel */
		gxl = s->lt[1] - (stmp * (s->sval - s->smin));
		PFSET_VEC2(coord[0], s->lb[0] + 2, gxl);
		PFSET_VEC2(coord[1], s->rt[0] - 2, gxl);
		PFSET_VEC2(coord[2], s->rt[0] - 2, gxl + 1);
		PFSET_VEC2(coord[3], s->lb[0] + 2, gxl + 1);
	    }
	    if (s->active)
	    {
		drawBevel(coord, SliderColor, 3);
		drawBevel(coord, SliderColor, -2);
	    }
	    else
	    {
		drawBevel(coord, InactiveSliderColor, 3);
		drawBevel(coord, InactiveSliderColor, -2);
	    }

	    /* draw inset bevel at default location */
	    if ((PF_ABSGT(s->sDefaultVal - s->sval, 10 * (s->scale))))
	    {
		if (hvflag)
		{		/* horizontal */
		    gxl = (stmp * (s->sDefaultVal - s->smin)) + s->lt[0];
		    coord[0][0] = gxl - 2;
		    coord[1][0] = gxl + 2;
		    coord[2][0] = gxl + 2;
		    coord[3][0] = gxl - 2;
		}
		else
		{		/* vertical */
		    gxl = s->lt[1] - (stmp * (s->sDefaultVal - s->smin));
		    coord[0][1] = gxl - 2;
		    coord[1][1] = gxl - 2;
		    coord[2][1] = gxl + 2;
		    coord[3][1] = gxl + 2;

		}

		drawBox(coord, BackColor, -1);
		drawBevel(coord, BackColor, -1);
	    }
	    if (s->sOldVal != s->sval)
	    {
		if (hvflag)
		{		/* horizontal */
		    gxl = (stmp * (s->sOldVal - s->smin)) + s->lt[0];
		    /* draw inset bevel where old slider grip was */
		    coord[0][0] = gxl;
		    coord[1][0] = gxl;
		    coord[2][0] = gxl;
		    coord[3][0] = gxl;
		    coord[0][1] += 1;
		    coord[1][1] += 1;
		    coord[2][1] -= 1;
		    coord[3][1] -= 1;
		}
		else
		{		/* vertical */
		    gxl = s->lt[1] - (stmp * (s->sOldVal - s->smin));
		    /* draw inset bevel where old slider grip was */
		    coord[0][1] = gxl;
		    coord[1][1] = gxl;
		    coord[2][1] = gxl;
		    coord[3][1] = gxl;
		    coord[0][0] += 1;
		    coord[1][0] += 1;
		    coord[2][0] -= 1;
		    coord[3][0] -= 1;
		}

		drawBevel(coord, BackColor, -1);
	    }
	    mysetfont((*CurFont));
	}
	break;

    case PFUGUI_RADIO_BUTTON:
	{
	    pfVec2          coord[4];
	    pfuWidget      *selection;
	    RadioButton    *r =(RadioButton *) w;
	    int            t;

	    drawSwitchButton(r->coord, r->icoord, r->on);
	    drawString(r->tx, r->ty, textColor, r->name);

	    if (r->selectionList)
	    {			/* draw selection widgets */
		int             on;

		/* put little inset bevel between name and selections */
		if (r->hvFlag)
		{		/* horizontal */
		    t = r->selectionList[0].xorg - 2;
		    PFSET_VEC2(coord[0], t - 1, r->yorg + 1);
		    PFSET_VEC2(coord[1], t + 2, r->yorg + 1);
		    PFSET_VEC2(coord[2], t + 2, r->yorg + r->ysize);
		    PFSET_VEC2(coord[3], t - 1, r->yorg + r->ysize);
		}
		else
		{		/* vertical */
		    t = r->selectionList[0].yorg + r->selectionList[0].ysize + 2;
		    PFSET_VEC2(coord[0], r->xorg + 1, t - 1);
		    PFSET_VEC2(coord[1], r->xorg + r->xsize, t - 1);
		    PFSET_VEC2(coord[2], r->xorg + r->xsize, t + 2);
		    PFSET_VEC2(coord[3], r->xorg + 1, t + 2);
		}
		drawBevel(coord, BackColor, -1);

		mysetfont(Fonts[NUMBER_FONTS][FONT_SIZE_VAL]);
		/* draw selectionibutes */
		for (i = 0; i < r->numSelections; i++)
		{
		    selection = &(r->selectionList[i]);
		    /* draw state indicator for selection */
		    if (r->sel != i)
			on = 0;
		    else
		    {
			if (r->on)
			    on = 1;
			else
			    on = -1;
		    }
		    if (i == r->defaultSel)
		    {
			PFSET_VEC2(coord[0], selection->icoord[0][0] - 1, selection->icoord[0][1] - 1);
			PFSET_VEC2(coord[1], selection->icoord[1][0] + 1, selection->icoord[1][1] - 1);
			PFSET_VEC2(coord[2], selection->icoord[2][0] + 1, selection->icoord[2][1] + 1);
			PFSET_VEC2(coord[3], selection->icoord[3][0] - 1, selection->icoord[3][1] + 1);
			drawStateIndicator(coord, on);
		    }
		    else
			drawStateIndicator(selection->icoord, on);
		    /* draw selection name after its state indicator */
		    drawString(selection->tx, selection->ty, textColor, selection->name);
		}
		mysetfont((*CurFont));;
	    }
	}
	break;

    case PFUGUI_MENU_BUTTON:
	{
	    MenuButton     *m =(MenuButton *) w;
#ifndef WIN32 /* XXX Alex -- remove the following and fi pup.h */
	    Display *dsp = pfGetCurWSConnection();
#else
	    pfWSConnection conn = pfGetCurWSConnection();
#endif
	    int screen = pfGetPWinScreen(GUI->pwin);
	    if (m->menu_dirty)
	    {
		/* setup selection menu */
		sprintf(str, "Select %s %%t", w->name);
		if (m->menu)
		    freepup(m->menu);
#ifndef WIN32
		m->menu = (int)defpup(dsp, screen, str);
#else
		m->menu = (int)defpup(conn, screen, str);
#endif
		for (i = 0; i < m->numSelections; i++)
		{
		    if (i == m->defaultSel)
		    {
			sprintf(str, "*%s*", m->selectionList[i].name);
			addtopup(m->menu, str, 0);
		    }
		    else
			addtopup(m->menu, m->selectionList[i].name, 0);
		}
		m->menu_dirty = 0;
	    }
	    drawBevel(m->coord, BackColor, 3);
	    if (m->state)
	    { /* draw it pushed */
		drawBox(m->coord, BackColor, -1);
		drawBevel(m->coord, BackColor, -3);
	    }
	    else
	    {
		drawBox(m->coord, BackColor, 1);
	    }
	    /*
	    drawBevel(m->coord, BackColor, 3);
	    drawBox(m->coord, BackColor, 1);
	    drawBevel(m->coord, BackColor, -1);
	    */
	    sprintf(str, "%s: %s", m->name, m->selectionList[m->sel].name);
	    drawString(w->tx, w->ty, textColor, str);
	    drawTriBevel(m->icoord, BackColor, 3);
	    drawTriBevel(m->icoord, BackColor, -1);
	    break;
	}
    default:
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "Unknown widget type %d",
		 w->type & PFUGUI_TYPE_MASK);
	break;
    }
    if (w->type == PFUGUI_BUTTON)
    {				/* release push-button to un-pushed for next
				 * draw */
	if (w->state == 1)
	{
	    w->state = 2;
	}
    }
}

/******************************************************************************
 *			    Low level drawing utilities
 ******************************************************************************
 */

#ifdef WIN32
void SCALE_CLR(pfVec2 s,pfVec3 c,float f) {
  s[0]=f*c[0]; s[1]=f*c[1]; s[2]=f*c[2];
  if(f > 1.0) { /* clamp colors */
    if(s[0] > 1.0) s[0] = 1.0;
    if(s[1] > 1.0) s[1] = 1.0;
    if(s[2] > 1.0) s[2] = 1.0;
  }
}
#else
#define SCALE_CLR(s, c, f)	{\
	s[0]=f*c[0]; s[1]=f*c[1]; s[2]=f*c[2]; \
	if (f > 1.0f) { /* clamp colors */\
	    if (s[0] > 1.0f) s[0] = 1.0f; \
	    if (s[1] > 1.0f) s[1] = 1.0f; \
	    if (s[2] > 1.0f) s[2] = 1.0f; \
	}}
#endif

static void
drawString(float x, float y, pfVec3 clr, const char *str)
{
    /* pfVec3  sh; SCALE_CLR(sh, clr, 0.2f); c3f(sh); cmov2(x + 1, y - 1);
     * dspstring(str);
     */

    if (str[0])
    {
	glColor3fv(clr);
	pfuDrawStringPos(str, x, y, 0.0f);
    }
}

static void
drawBox(pfVec2 coord[4], pfVec3 clr, int elev)
{
    /* Draw box */
    if (elev)
    {
	pfVec3          c;
	if (elev < 0)
	{
	    SCALE_CLR(c, clr,0.95f);
	}
	else
	{
	    SCALE_CLR(c, clr, 1.05f);
	}
	glColor3fv(c);
    }
    else
    {
	glColor3fv(clr);
    }
    glBegin(GL_QUADS);
    glVertex2fv(coord[0]);
    glVertex2fv(coord[1]);
    glVertex2fv(coord[2]);
    glVertex2fv(coord[3]);
    glEnd();
}

static void
drawBevel(pfVec2 coord[4], pfVec3 clr, float bevel)
{
    pfVec3          sh;
    pfVec2          bcoord[4];
    float           shLeft, shRight, shBot, shTop;

    PFSET_VEC2(bcoord[0], coord[0][0] - bevel, coord[0][1] - bevel);
    PFSET_VEC2(bcoord[1], coord[1][0] + bevel, coord[1][1] - bevel);
    PFSET_VEC2(bcoord[2], coord[2][0] + bevel, coord[2][1] + bevel);
    PFSET_VEC2(bcoord[3], coord[3][0] - bevel, coord[3][1] + bevel);

    if (bevel < 0)	/* 'in' bevel */
    {	/* in bevels rely on backface(FALSE) */
	shLeft = 0.6f;
	shBot = 1.6f;
	shRight = 1.2f;
	shTop = 0.4f;
    }
    else	/* 'out' bevel */
    {
	shLeft = 1.2f;
	shBot = 0.4f;
	shRight = 0.6f;
	shTop = 1.6f;
    }

    /* left */
    SCALE_CLR(sh, clr, shLeft);
    glColor3fv(sh);
    glBegin(GL_QUADS);
    glVertex2fv(coord[3]);
    glVertex2fv(coord[0]);
    glVertex2fv(bcoord[0]);
    glVertex2fv(bcoord[3]);
    /* glEnd(); */

    /* bottom */
    SCALE_CLR(sh, clr, shBot);
    glColor3fv(sh);
    /* glBegin(GL_QUADS); */
    glVertex2fv(coord[0]);
    glVertex2fv(coord[1]);
    glVertex2fv(bcoord[1]);
    glVertex2fv(bcoord[0]);
    /* glEnd(); */

    /* right */
    SCALE_CLR(sh, clr, shRight);
    glColor3fv(sh);
    /* glBegin(GL_QUADS); */
    glVertex2fv(coord[1]);
    glVertex2fv(coord[2]);
    glVertex2fv(bcoord[2]);
    glVertex2fv(bcoord[1]);
    /* glEnd(); */

    /* top */
    SCALE_CLR(sh, clr, shTop);
    glColor3fv(sh);
    /* glBegin(GL_QUADS); */
    glVertex2fv(coord[2]);
    glVertex2fv(coord[3]);
    glVertex2fv(bcoord[3]);
    glVertex2fv(bcoord[2]);
    glEnd();

}

static void
drawTriBevel(pfVec2 coord[4], pfVec3 clr, float bevel)
{
    pfVec3          sh;
    pfVec2          bcoord[3];
    float           shLeft, shBot, shTop;

    PFSET_VEC2(bcoord[0], coord[0][0] - bevel, coord[0][1] - bevel);
    PFSET_VEC2(bcoord[1], coord[1][0] + bevel, coord[1][1] - bevel);
    PFSET_VEC2(bcoord[2], coord[2][0] - bevel, coord[2][1] + bevel);

    if (bevel < 0)	/* 'in' bevel */
    {	/* in bevels rely on backface(FALSE) */
	shLeft = 0.6f;
	shBot = 1.6f;
	shTop = 0.4f;
    }
    else	/* 'out' bevel */
    {
	shLeft = 1.2f;
	shBot = 0.4f;
	shTop = 1.6f;
    }

    /* left */
    SCALE_CLR(sh, clr, shLeft);
    glColor3fv(sh);
    glBegin(GL_QUADS);
    glVertex2fv(coord[2]);
    glVertex2fv(coord[0]);
    glVertex2fv(bcoord[0]);
    glVertex2fv(bcoord[2]);
    /* glEnd(); */

    /* bottom */
    SCALE_CLR(sh, clr, shBot);
    glColor3fv(sh);
    /* glBegin(GL_QUADS); */
    glVertex2fv(coord[0]);
    glVertex2fv(coord[1]);
    glVertex2fv(bcoord[1]);
    glVertex2fv(bcoord[0]);
    /* glEnd(); */

    /* top */
    SCALE_CLR(sh, clr, shTop);
    glColor3fv(sh);
    /* glBegin(GL_QUADS); */
    glVertex2fv(coord[1]);
    glVertex2fv(coord[2]);
    glVertex2fv(bcoord[2]);
    glVertex2fv(bcoord[1]);
    glEnd();

}

static void
drawStateIndicator(pfVec2 icoord[4], int on)
{
    switch(on)
    {
    case 1:
	drawBevel(icoord, BackColor, 2);
	drawBox(icoord, OnColor, 1);
	break;
    case -1:
	drawBox(icoord, OffColor, -1);
	drawBevel(icoord, OnColor, -1);
	break;
    case 0:
	drawBox(icoord, OffColor, -1);
	drawBevel(icoord, BackColor, -1);
	break;
    }
}

static void
drawSwitchButton(pfVec2 coord[4], pfVec2 icoord[4], int on)
{
    drawBevel(coord, BackColor, 3);
    if (on)	/* draw pushded-in bevel */
    {
	drawBox(coord, BackColor, -1);
	drawBevel(coord, BackColor, -3);
    }
    else
    {
	drawBox(coord, BackColor, 1);
    }
    drawStateIndicator(icoord, on);
}

static void
scaleFonts(float _scale, int drawFlag)
{
    int loadCursor = 0;
    int i, j, size, doScale=0;
    double start,  end;

    if ((_scale > 0.0f) && 
	(PF_ABSGE(_scale - GUI->fontScale, 0.05f) || 
		(GUI->fontScale < 0.0f)))
    {
	FontFlags |= FONT_DIRTY;
	GUI->fontScale = _scale;
	doScale = 1;
    }
    else
    {
	FontFlags &= ~(FONT_SCALE_DIRTY);
    }
    if ((FontFlags & FONT_DIRTY) &&
	drawFlag && GUI->pwin && pfGetPWinWSWindow(GUI->pwin))
    {
	pfuLoadPWinCursor(GUI->pwin, PFU_CURSOR_watch);
	loadCursor = 1;
    }
    if (doScale)
    {
	char str[PF_MAXSTRING];
	
	GUIFontScale = GUI->fontScale;
	if (GUI && GUI->flags)
	{
	    pfDPoolLock(GUI->flags);
	    GUI->flags->dirty = GUI->flags->redrawAll = GUI->refresh;
	    pfDPoolUnlock(GUI->flags);
	}
	start = pfGetTime();
	for (j = 0; j < NUM_GUI_FONTS; j++)
	{
	    for (i = 1; i < PFU_NUM_FONTS; i++)
	    {
#ifndef WIN32
	      size = (GUIFontScale * FontSizes[j][i] * 1.6f); /* XXX X11 Fudge Factor */
#else
	      size = (GUIFontScale * FontSizes[j][i]);
#endif
		if (size > 0)
		    Fonts[j][i].size = size;
		sprintf(str, "-*-%s-normal--%i-*-*-*-*-*-iso8859-1", 
		    FontNames[j], Fonts[j][i].size);
		if (Fonts[j][0].info)
		{
#ifndef WIN32
		  if (Fonts[j][i].info)
		    XFreeFont(pfGetCurWSConnection(), Fonts[j][i].info);
#else
		  DeleteObject(Fonts[j][i].info);
#endif
		  pfuLoadXFont(str, &(Fonts[j][i]));
		}
		else
		    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
			"scaleFonts: font \"%s\" not scalable to size %i",
		       FontNames[j], Fonts[j][i].size);
	    }
	}
	end = pfGetTime();
	pfNotify(PFNFY_INFO, PFNFY_PRINT, "scaleFonts %.2f - Load: %.2f secs", 
		 _scale,  (float) (end - start));
	FontFlags &= ~(FONT_SCALE_DIRTY);
	FontFlags |= FONT_DRAW_DIRTY;
    }
    if (drawFlag && (FontFlags & FONT_DRAW_DIRTY))
      {
	start = pfGetTime();
	for (j = 0; j < NUM_GUI_FONTS; j++)
	  {
	    if (Fonts[j][0].info)
	      for (i = 1; i < PFU_NUM_FONTS; i++)
		{
		  pfuMakeXFontBitmaps(&(Fonts[j][i]));
		}
	  }
	FontFlags &= ~(FONT_DRAW_DIRTY);
	glFinish();
	end = pfGetTime();
	pfNotify(PFNFY_INFO, PFNFY_PRINT, "scaleFonts - Make Font Bitmaps: %.2f secs", 
		 (float) (end - start));
	initMessageFonts();
      }
    if (loadCursor) /* replace cursor */
      {
	pfuLoadPWinCursor(GUI->pwin, pfuGetGUICursor(pfuGetGUICursorSel()));
      }
}

static void
initMessageFonts(void)
{
    int i, size;
    char str[PF_MAXSTRING];
    
    MessageFontsInited = 1;
    for (i = 1; i < PFU_NUM_FONTS; i++)
    {
#ifndef WIN32
      size = (FontSizes[MESSAGE_FONTS][i] * 1.5f); /* XXX X11 Fudge Factor */
#else
      size = (FontSizes[MESSAGE_FONTS][i]); 
#endif
	if (size > 0)
	    Fonts[MESSAGE_FONTS][i].size = size;
	sprintf(str, "-*-%s-normal--%i-*-*-*-*-*-iso8859-1", 
	    FontNames[MESSAGE_FONTS], Fonts[MESSAGE_FONTS][i].size);
	if (Fonts[MESSAGE_FONTS][0].info)
	{
	    if (Fonts[MESSAGE_FONTS][i].info)
#ifndef WIN32
	      XFreeFont(pfGetCurWSConnection(), Fonts[MESSAGE_FONTS][i].info);
#else
	    DeleteObject(Fonts[MESSAGE_FONTS][i].info);
#endif
	    pfuLoadXFont(str, &(Fonts[MESSAGE_FONTS][i]));
	}
	else
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		"initMessageFonts: font \"%s\" not scalable to size %i",
	       FontNames[MESSAGE_FONTS], Fonts[MESSAGE_FONTS][i].size);
    }   
    for (i = 1; i < PFU_NUM_FONTS; i++)
    {
	if (Fonts[MESSAGE_FONTS][0].info)
	    pfuMakeXFontBitmaps(&(Fonts[MESSAGE_FONTS][i]));
    }
}

static void
initGUIFonts(int drawFlag)
{
#ifdef USE_FONT_MANAGER
static int      FMInited = 0;
    fmfonthandle    fontHandle;

    if (!FMInited)
    {
	fminit();
	FMInited = 1;
    }
    bzero(Fonts, sizeof(Fonts));
#endif	/* USE_FONT_MANAGER */
    {
	int i, j;
	char str[256];
	for (i=0; i < NUM_TOTAL_FONTS; i++)
	{
	    int c;
	    char **ts;
	    sprintf(str, "-*-%s-normal--*-*-*-*-*-*-iso8859-1", FontNames[i]);
#ifndef WIN32
	    ts = XListFonts(pfGetCurWSConnection(),  str, 1, &c);
#else
	    c = 1;
#endif
	    if (c)
	    {
		Fonts[i][0].info = (void*)1; /* mark font found */
#ifndef WIN32
		XFreeFontNames(ts);
#endif
		for (j=0; j < NUM_FONT_SIZES; j++)
		{
		    Fonts[i][j].size = FontSizes[i][j];
		}
	    }
	    else
		pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		    "initGUIFonts: font \"%s\" not found", FontNames[i]);
	}
	FontFlags |= FONT_DIRTY;
	GUIFontScale = 0.0f;
	scaleFonts(1.0f, drawFlag);
	CurFont = &Fonts[TEXT_FONTS][PFU_FONT_BIG];
	if (drawFlag)
	    mysetfont(*(CurFont));
    }
}
/*******************************************************************************
 *				Draw Message Utilities
 *******************************************************************************
 */


/* Draw a message justified wrt to x,y in font of size 'size'.
 * 'just' may be CENTER, LEFT, RIGHT. x, y should range from 0 to 1
 * and 'size' should be SMALL, MED, or BIG.
 * The colors of the message are fixed: purple text with black shadow.
 * This is ageneral and  expensive routine and is meant to be used for
 * fairly static messages that only need updating upon a REDRAW event.
 * For messages that need to be
 * updated frequently, a more streamlined routine should be used.
 */
PFUDLLEXPORT void
pfuDrawMessage(pfChannel *chan, const char *msg,
		int rel, int just, float x, float y, int size, int cimode)
{

static pfVec4	    textRGB = {1.0f, 0.0f, 1.0f, 1.0f};
static pfVec4	    shadowRGB = {0.125f, 0.06f, 0.125f, 1.0f};

    if (cimode)
    {
    }
    else
    {
	drawMessage(chan, msg, rel, just, x, y, size, cimode,
			(unsigned long) textRGB, (unsigned long) shadowRGB);
    }

}

PFUDLLEXPORT void
pfuDrawMessageCI(pfChannel *chan,
		    const char *msg, int rel, int just,
		    float x, float y, int size,
		    int textClr, int shadowClr)
{
    drawMessage(chan, msg, rel, just, x, y, size, PFU_CI, textClr, shadowClr);
}

PFUDLLEXPORT void
pfuDrawMessageRGB(pfChannel *chan,
		    const char *msg, int rel, int just,
		    float x, float y, int size,
		    pfVec4 textClr, pfVec4 shadowClr)
{
    drawMessage(chan, msg, rel, just, x, y, size, PFU_RGB,
		(unsigned long) textClr, (unsigned long) shadowClr);
}

static void
drawMessage(pfChannel *chan, const char *msg,
		int rel, int just, float x, float y, int size, int cimode,
		unsigned long textClr, unsigned long shadowClr)
{
    float	 width;
    float	 xstart;
    float	 height;
    float	 ystart;
    int		 xo, yo, xs, ys;
    int		 vp[4];
    pfMatrix	 proj;


    if (!chan)
	rel = PFU_MSG_PIPE;

    if (!GUI)
    {
	getGUIDataPool(1);
    }
    if (!MessageFontsInited)
	initMessageFonts();;
	
    if (GUI && GUI->flags && GUI->flags->fitDirty)
	fitWidgets();

    if (size < 0 || size > PFU_NUM_FONTS)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfuDrawMessage: unknown font size %d,  using PFU_FONT_MED.",
	    size);
	size = PFU_FONT_MED;
    }
    
    glGetIntegerv(GL_VIEWPORT, vp);

    if (rel == PFU_MSG_CHAN)
    {
	/* convert to channel viewport coords */
	pfGetChanOutputOrigin(chan, &xo, &yo);
	pfGetChanOutputSize(chan, &xs, &ys);
	if (!(xs > 0) || !(ys > 0))
	{
	    pfWindow *w = pfGetCurWin();
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		"pfuDrawMessage - bad pfChannel size: %d %d, trying current window.", xs, ys);
	    pfGetWinSize(w, &xs, &ys);
	    xo = yo = 0;
	    if (!(xs > 0) || !(ys > 0))
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
		    "pfuDrawMessage - bad window size: %d %d", xs, ys);
		return;
	    }
	}
	glViewport(xo, yo, xs, ys);
	glScissor(xo, yo, xs, ys);
    }
    else /* Pipe Window relative message position */
    {
	if (chan)
	{
	    pfPipeWindow *pw = pfGetChanPWin(chan);
	    pfGetPWinSize(pw, &xs, &ys);
	}
	else
	{
	    xs = vp[2];
	    ys = vp[3];
	}

	if (!(xs > 0) || !(ys > 0))
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		"pfuDrawMessage bad Pipe Window size: %d %d", xs, ys);
	}

	/* set the viewport to be able to draw to the entire window */
	glViewport(0, 0, xs, ys);
	glScissor(0, 0, xs, ys);
    }
    {
	GLint mm;
	glGetIntegerv(GL_MATRIX_MODE, &mm);
	if (mm != GL_PROJECTION)
	    glMatrixMode(GL_PROJECTION);
	glGetFloatv(GL_PROJECTION_MATRIX, (float*) proj);
	glLoadIdentity(); 
	glOrtho(0.0f, xs, 0.0f, ys, -1.0f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
    }

    pfPushIdentMatrix();

    /* convert x and y to pixel coords */
    x *= xs;
    y *= ys;

    mysetfont(Fonts[MESSAGE_FONTS][size]);
    width = (stringwidth(Fonts[MESSAGE_FONTS][size], msg)) + 10.0f;
    height = fontheight(Fonts[MESSAGE_FONTS][size]) + 10.0f;

    switch (just) {
    case PFU_LEFT_JUSTIFIED:
	xstart = x;
	break;
    case PFU_RIGHT_JUSTIFIED:
	xstart = x - width - 2.0f;
	break;
    case PFU_CENTER_JUSTIFIED:
    default:
	xstart = x - width*0.5f;
	break;
    }

    /* Don't let text go off the right side of the screen */
    if (just != PFU_LEFT_JUSTIFIED)
    {
	if ((xstart + width) >= (xs - 1.0f))
	    xstart = (xs) - width - 2.0f;
    }

    /* Don't let text go off the top of the screen */
    if ((y + height) >= (ys - 1.0f))
	ystart = PF_MAX2(0.0f, (ys) - height - 2.0f);
    else
	ystart = y;

    pfPushState();
    pfBasicState();
    glDepthFunc(GL_ALWAYS);

    /* Draw shadow */
    if (cimode)
	glIndexi((int)shadowClr);
    else
	glColor4fv((float*)shadowClr);
    pfuDrawStringPos(msg, xstart + 2.0f, ystart - 2.0f, 0.0f);

    /* Draw top */
    if (cimode)
	glIndexi((int)textClr);
    else
	glColor4fv((float*)textClr);
    pfuDrawStringPos(msg, xstart, ystart, 0.0f);
    
    pfPopMatrix();
    glDepthFunc(GL_LEQUAL);
    pfPopState();

    { /* restore viewpoort */
	glViewport(vp[0], vp[1], vp[2], vp[3]);
    }
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((float*) proj);
    glMatrixMode(GL_MODELVIEW);
    mysetfont(Fonts[TEXT_FONTS][FONT_SIZE_TITLE]);
}

/******************************************************************************
 *				Cursor Routines
 ******************************************************************************
 */

#if 0
static int	simCursorName=0, guiCursorName=0;
static Cursor	simCursor=0, guiCursor=0;
#endif

PFUDLLEXPORT void
pfuGUICursor(int target, int c)
{
    if (!GUI)
	getGUIDataPool(1);

    switch(target)
    {
	case PFU_CURSOR_GUI:
	    if (c == PFU_CURSOR_DEFAULT)
		c = PFU_CURSOR_GUI_DEFAULT;
	    GUI->guiCursor = c;
	    break;
	case PFU_CURSOR_MAIN:
	    if (c == PFU_CURSOR_DEFAULT)
		c = PFU_CURSOR_MAIN_DEFAULT;
	    GUI->mainCursor = c;
	    break;
	default:
	    pfNotify(PFNFY_WARN, PFNFY_USAGE,
		"pfuGUICursor() - unknown target %d", target);
	    break;
    }

}

PFUDLLEXPORT Cursor
pfuGetGUICursor(int target)
{
    if (!GUI)
	getGUIDataPool(1);
    switch(target)
    {
	case PFU_CURSOR_GUI:
	    return (GUI->guiCursor);
	case PFU_CURSOR_MAIN:
	    return (GUI->mainCursor);
	default:
	    pfNotify(PFNFY_WARN, PFNFY_USAGE,
		"pfuGetGUICursor() - unknown target %d", target);
	    return 0;
    }
}

PFUDLLEXPORT void
pfuGUICursorSel(int c)
{
    if (!GUI)
	getGUIDataPool(1);
    if (c == PFU_CURSOR_DEFAULT)
	c = PFU_CURSOR_GUI_DEFAULT;
    GUI->cursor = c;
}

PFUDLLEXPORT Cursor
pfuGetGUICursorSel(void)
{
    if (!GUI)
	getGUIDataPool(1);
    return GUI->cursor;
}


PFUDLLEXPORT void
pfuInitGUICursors(void)
{
    pfuCreateCursor(pfuGetGUICursor(PFU_CURSOR_GUI));
    pfuCreateCursor(pfuGetGUICursor(PFU_CURSOR_MAIN));
}

PFUDLLEXPORT void
pfuUpdateGUICursor(void)
{
    Cursor		cursorVal;
    static int		oldCursorVal = -1;
    
    cursorVal = pfuGetGUICursor(pfuGetGUICursorSel());
    if ((cursorVal != oldCursorVal) && (GUI->pwin != NULL) &&
		(pfGetPWinWSWindow(GUI->pwin) != NULL))
    {
	pfuLoadPWinCursor(GUI->pwin, cursorVal);
	oldCursorVal = (int)cursorVal;
    }
}

PFUDLLEXPORT void
pfuDrawChanDVRBox(pfChannel *chan)
{
    pfPipeVideoChannel *pvc;
    int xo, yo, xs, ys;
    float xscale, yscale;
    pfVec2 coords[4];

    pfPushState();
    pfBasicState();
    glDepthFunc(GL_ALWAYS);

    /* Draw output channel box to show DVR */
    pvc = pfGetChanPVChan(chan);
    pfGetChanOutputOrigin(chan, &xo, &yo);
    pfGetChanOutputSize(chan, &xs, &ys);
    pfGetPVChanScale(pvc, &xscale, &yscale);
    glScissor(xo, yo, xs, ys);
    glViewport(xo, yo, xs, ys);
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
}
