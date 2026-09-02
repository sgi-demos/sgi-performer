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
 * gui.h
 *
 * $Revision: 1.42 $
 * $Date: 2000/10/06 21:25:57 $
 *
 */

#ifndef __PFUGUI_H__
#define __PFUGUI_H__

#include <Performer/pfutil.h>

	/*--------------------------------------------------------*/
struct _pfuPanel 
{
    int	    id;
    int	    active;
    int	    numWidgets;
    pfuWidget	    *widgets[PFUGUI_MAXWIDGETS];
    int	    dirty;
    int	    listSize;
};

#define WIDGET								\
    pfuPanel	    *panel;						\
    int	    id;    							\
    pfuGUIString    name;						\
    int	    type;							\
    int	    mode;							\
    int	    hidden;							\
    int	    active;							\
    int	    style;		/* normal, hilighted */			\
    int	    xorg, yorg;							\
    int	    xsize, ysize;						\
    int	    nameWidth;							\
    int	    state;							\
    float	    min, max, val, oldVal, defaultVal;			\
    float	    tx, ty;	/* location to draw name */		\
    int	    dv;		/* display button value, 1 for int, */		\
				/* 2 for float, 0 = nothing */		\
    int	    dirty;	/* >0, widget needs redrawing */		\
    									\
    /* User-supplied function is called when widget is activated */	\
    pfuWidgetActionFuncType	    actionFunc;					\
    /* User-supplied function is called when widget is to be drawn */	\
    pfuWidgetDrawFuncType	    drawFunc;					\
    /* User-supplied function is called when widget is to be selected */ \
    pfuWidgetSelectFuncType	    selectFunc;					\
									\
    pfVec2	coord[4];	/* widget boundaries */			\
    pfVec2	icoord[4];	/* to draw state indicator */
    
    
struct _pfuWidget 
{
    WIDGET
};

typedef struct _radioButton 
{  
    WIDGET 
    int	hvFlag;		/* Widget is horizontal if true */ 
    int	toggle;		/* Let title serve as an on/off toggle */
    int	on, defaultOn;
    int	numSelections;
    int	sel, oldSel, defaultSel;
    int	selectionSpace;
    pfuWidget	*selectionList;

} RadioButton;

typedef struct _menuButton 
{  
    WIDGET 
    int	numSelections;
    int	sel, oldSel, defaultSel;
    int	menu;
    int	menu_dirty;
    int	selectionSpace;
    pfuWidget	*selectionList;

} MenuButton;

typedef struct _slider 
{
    WIDGET
    int	hvFlag;			/* widget is horizontal if true */ 
    float	start, scale;

    /* Slider-relative values */
    float	smin, smax, sval, sOldVal, sDefaultVal;
    float	minx, maxx;

    pfVec2	rt,rb,lt,lb;
    int	minStrWidth, maxStrWidth;
    char	maxstr[PFUGUI_BUTTON_STRING_SIZE];
    char	minstr[PFUGUI_BUTTON_STRING_SIZE];

} Slider;    

#define MAX_WIDGET_SIZE ((int)sizeof(Slider))


typedef struct _pfuGUIFlags
{
    int	redrawAll;  /* > 0, need to redraw all GUI */
    int	dirty;	    /* > 0, need to redraw some portion of GUI */
    int	fitDirty;   /* > 0, scale/trans needs to be recomputed */

} pfuGUIFlags;

typedef struct _pfuGUIPicker
{
    int		requestFlag;
    float	x, y;
    pfPath	*path;
    pfNode	*node;
    pfChannel	*chan;

} pfuGUIPicker;


#ifdef	USE_FONT_MANAGER
#ifndef	mips
#define mips			/* XXX - get prototype from fmclient.h */
#endif
#include <fmclient.h>
typedef struct _FMFont
{
    float 		size;
    fmfonthandle 	handle;
    fmfontinfo 		info;

} FMFont;
typedef FMFont GUIFont;
#else /* OPENGL */
typedef pfuXFont GUIFont;
#endif /* GL type */

/* pfuGUIFontArray holds standard GUI font sizes of each font */
typedef GUIFont pfuGUIFontArray[PFU_NUM_FONTS]; 

typedef struct _pfuGUI
{
    int		active;	    /* != 0 ->  gui is active and displayed */
    pfuGUIFlags *flags;	    /* Flags are in pfDataPool for MP access */

    pfChannel	*channel;
    pfPipe	*pipe;
    pfPipeWindow *pwin;
    
    int		refresh;	

    int   	numPanels;
    pfuPanel	*panels[PFUGUI_MAXPANELS];

    int		wxs, wys;
    float 	pixOrgX, pixOrgY;
    float	orthoOrgY, orthoMaxY;
    float 	pixSizeX, pixSizeY;
    float 	vpLeft, vpRight, vpBottom, vpTop;

    float	scaleX, scaleY;
    float	transX, transY;

    pfuWidget	*selectedWidget;
    pfuPanel	*selectedPanel;

    pfuMouse	mouse;
    Cursor	cursor;
    Cursor	guiCursor;
    Cursor	mainCursor;
    int		useFM;		    /* For disabling font manager */

    /* For performance statistics */
    int 	nGUIDraws;
    float 	guiDrawTime;
    
    float	fontScale;

    MenuButton	*dopup;		/* Active popup menu widget */
    
    int		inited;	/* marks that pfuInitGUI() has been called */

    pfHighlight	*hlight;
    pfNode	*hilightedNode; /* Node that is currently being highlighted */

    pfuGUIPicker *pick;

} pfuGUI;

#endif	/* __PFUGUI_H__ */

