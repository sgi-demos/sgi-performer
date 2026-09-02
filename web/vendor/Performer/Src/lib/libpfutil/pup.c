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
 */

/*
 * file: pup.c
 * -----------
 * menu code with a sordid history.
 */



#include	<stdlib.h>
#ifdef mips
#include	<bstring.h>
#endif  /* mips */
#include	<string.h>
#include	<stdarg.h>
#include	<ctype.h>
#include	<assert.h>
#ifndef WIN32
#include	<sys/time.h>
#include	<sys/times.h>
#include	<sys/param.h>
#endif

#include	<stdio.h>

#ifndef WIN32
#include	<X11/Xlib.h>	/* XXX do we need all of this ? */
#include	<X11/Xutil.h>
#include	<X11/Xmd.h>
#include	<X11/cursorfont.h>
#endif

#include 	<Performer/pr.h>
#include	"pup.h"
#include        <Performer/pfutil-DLL.h>
#ifdef WIN32
/* XXX Alex -- implement this stuff on win32 ... */
PFUDLLEXPORT long dopup(long menu) { return 0L; }
PFUDLLEXPORT void addtopup(long pup,char *str, ...) {}
PFUDLLEXPORT long defpup(pfWSConnection conn,int screen,char *str,...) {}
PFUDLLEXPORT void freepup(long menu) {}
#else

#define POST_TIMEOUT	160
#define HOG_TIMEOUT	1000
#define MOTION_HISTORY	4

static struct hbuf { int x, y; } history[MOTION_HISTORY];
static int mhead=0;
static int PostTimeout=0;
static int HogTimeout=0;
static int MenuState=0;

static void set_state(int);
#ifdef NEED_NEXT_EVENT
static unsigned int StateTimeout=0;
static void post_submenu(void);
static void next_event(Display*,XEvent*);
#endif

#define MS_NORMALSTATE		0
#define	MS_PULLRIGHTPENDING	1
#define MS_PULLRIGHTACTIVE	2
#define MS_HOGSTATE		3

static char* defaultMenuFont = "-*-helvetica-bold-o-normal--14-100-*";

#define ALLOC(x)  malloc(x)
#define FREE(x)   free(x)
#define STRDUP(x) strdup(x)

static char m_class[] = "Menu";
static char m_inst[] = "menu";

static void render_outline (Display* , Window , GC *, int , int , int , int );
static void render_fill (Display* , Window , GC *, GC, int , int , int , int );

typedef struct mFont {
    XFontStruct*	finfo;
    char*		fontName;
    Display		*dpy;
    struct mFont*	next;
} mFont;

#define Menu_itemTopInset 2
#define Menu_itemBottomInset 2
#define Menu_itemLeftInset 4
#define Menu_itemRightInset 4
#define Menu_itemOffset 2

#define Menu_topInset 4
#define Menu_bottomInset 4
#define Menu_leftInset 4
#define Menu_rightInset 4

#define Menu_arrowLeftInset 7
#define Menu_arrowWidth 8
#define Menu_arrowHeight 8
#define Menu_arrowRightInset 0

#define  Menu_checkLeftInset 4
#define  Menu_checkWidth 11
#define  Menu_checkHeight 11
#define  Menu_checkRightInset 3

#define Menu_subMenuXOffset -2
#define Menu_subMenuYOffset -3

#define Menu_shadowXOffset 5 + 2	/* +2 for border width */
#define Menu_shadowYOffset 5 + 2

#define Menu_titleYGap 6

#define Menu_arrowAdjust	\
	(Menu_arrowLeftInset+Menu_arrowWidth+Menu_arrowRightInset)

#define Menu_checkAdjust \
    (Menu_checkLeftInset + Menu_checkWidth + Menu_checkRightInset)

#define Menu_widthAdjust (Menu_leftInset+Menu_rightInset)

#define Menu_heightAdjust (Menu_topInset+Menu_bottomInset)

/*---------------------------------------------------------------------- */

typedef struct struct_PopupInfo {
    struct struct_PopupInfo	*next;
    Display*		dpy;
    int			screen;
    int			overFirst;
    GC			black,
			darkGrey,
			grey,
			lightGrey,
			lightWhiteGrey,
			white;
    int			whitePixel,
			blackPixel,
			lightGreyPixel;
    Colormap		colormap;
    Visual*		visual;
    int			depth;
    Pixmap		shadowPixmap;

    Pixmap		lightWhiteGreyPixmap;
    GC			titleColors[4];
    GC			regularColors[4];
    GC			selectedColors[4];
    GC			checkBoxOnColors[4];
    GC			checkBoxOffColors[4];
    Cursor		arrowCursor;
} PopupInfo;

typedef struct Menu {
    PopupInfo*			popupInfo;
    struct MenuItem*	head;
    struct MenuItem*	tail;
    unsigned char	popping;
    unsigned char	isTitle;
    unsigned char	mapped;
    unsigned char	button;
    unsigned char	layoutDone;
    unsigned char	overFirst;
    short		width, height;
    Window	contentWindow;
    Window	shadowWindow;
    int		initialX;
    struct Menu*	menuTitle;

    struct MenuItem*	currentItem;	/* current selected item in this menu */
    struct Menu*	currentSubMenu;	/* current active sub menu (if any) */
    struct Menu*	stackPrev;	/* previous menu in stack from this menu */

    struct mFont* menuFont;
    int		(*menuFunc)(int);
} Menu;
static void draw_check(PopupInfo* , Window , int , int , int , int );
static PopupInfo* create_PopupInfo(Display* , int );
static Menu* create_menu(Display* , int , int);
static void set_title_menu(Menu* , char *);
static void destroy_menu(Menu* );
static int popup_menu(Menu* , int, int , int );
static struct MenuItem* find_item(Menu*,int);

#define MenuItem_normal 0
#define MenuItem_hasCheckBox 1

/* MenuItem::visualState */
#define MenuItem_disabled 0
#define MenuItem_quiet 1
#define MenuItem_selected 2

/* Generic menu item type.  Subclasses provide a particular thing to */
/* image as the label part of the menu item. */

typedef struct MenuItem {
    struct Menu*	parent;
    struct MenuItem*	nextItem;
    struct Menu*	subMenu;
    char	type;
    char	visualState;
    int		checkState;
    int		enabled;
    short	index;			/* item # for client */
    int		pickValue;
    int		inset;
    short	x, y, width, height;	/* position in parent */
    short	trueWidth, trueHeight;

    char*	label;			/* If it is a label, these are */
    int		len;			/* used.  If label is zero, it */
    struct mFont*	font;			/* is assumed to be a line */

    int useMenuFunc;
    int	(*itemFunc)(int);
} MenuItem;
static MenuItem* create_item(struct Menu* , int , char* );
static void reset_MenuItem(MenuItem*);
static void resize_item(MenuItem*, int, int);
static void move_item(MenuItem*, int, int);
static void destroy_item(MenuItem* );
static void render_background_item(MenuItem* );
static void enter_item(MenuItem *);
static void leave_item(MenuItem *);
static void render_item(MenuItem *);
static void set_enable_item(MenuItem* , int );
static void set_check_item(MenuItem* , int );

/*---------------------------------------------------------------------*/

typedef struct aFuncElement {
    int			(*func)(int);
    struct aFuncElement *next;
} aFuncElement;

typedef struct {
    aFuncElement	*head;
    aFuncElement	*tail;
} aFuncChain;

static aFuncChain *
create_aFuncChain(void) {
    aFuncChain	*ret;
    ret = ALLOC(sizeof *ret);
    bzero(ret, sizeof *ret);
    return ret;
}

static void
extend_aFuncChain(aFuncChain* this, int (*newfunc)()) {
    aFuncElement	*newElement;
    if (newfunc) {
	newElement = ALLOC(sizeof *newElement);
	newElement->next = 0;
	newElement->func = newfunc;
	if (this->head == 0)
	    this->head = this->tail = newElement;
	else {
	    this->tail->next = newElement;
	    this->tail = newElement;
	}
    }
}

static int
call_aFuncChain(aFuncChain* this, int ret) {
    aFuncElement	*who;

    for (who = this->head; who; who = who->next)
	ret = (who->func)(ret);
    return ret;
}

static void
destroy_aFuncChain(aFuncChain* this) {
    aFuncElement	*who, *youwho;
    for (who = this->head; who; who = youwho) {
	youwho = who->next;
	FREE(who);
    }
    FREE(this);
}
	
/*---------------------------------------------------------------------*/

mFont* allFontInfo;
static mFont*
create_font(Display* dpy, const char* fname) {
    mFont*	this = allFontInfo;
    XFontStruct	*font = 0;
    
    for ( ; this ; this = this->next)
	if (this->dpy == dpy) {
	    if (fname == 0 && this->fontName == 0)
		return this;
	    if (fname == 0 || this->fontName == 0)
		continue;
	    if (strcmp(fname, this->fontName) == 0)
		return this;
	}
    if (fname)
	font = XLoadQueryFont(dpy, fname);
    if (font == 0) {
	font = XLoadQueryFont(dpy, defaultMenuFont);
	if (font == 0)
	    font = XLoadQueryFont(dpy, "fixed");
    }
    this = ALLOC(sizeof *this);
    this->dpy = dpy;
    this->next = allFontInfo;
    allFontInfo = this;
    this->finfo = font;
    this->fontName = fname ? STRDUP(fname) : 0;
    return this;
}

static int
strwidth_font(mFont* this, char* str, int len) {
    int direction, ascent, descent;
    XCharStruct	size;

    XTextExtents(this->finfo, str, len, &direction, &ascent, &descent, &size);
    return size.lbearing + size.rbearing;
}

typedef int (*PFI)();
static void
do_addpup(Menu* pup,char *str,va_list args) {
    MenuItem* it;
    int lastItemNo = 0;
    char *cp;

    for (it = pup->head; it; it = it->nextItem)
	 if (it->index)
	    lastItemNo = it->index;
    for (cp = str;;cp++) {
	/*
	** First collect all the characters of the string, setting
	** pieces of flag information as we go ...
	*/
	PFI	entryFunc;
	int	ignoreMenuFunc;
	int	isTitle;
	int	hasUnderline;
	int	returnValue;
	Menu*	subMenu;
	char	collectName[512], *dp;

	entryFunc = 0;
	ignoreMenuFunc = isTitle = hasUnderline = 0;
	returnValue = lastItemNo+1;
	subMenu = 0;
	dp = collectName;

	while (*cp && *cp != '|') {
	    if (*cp != '%')
		*dp++ = *cp++;
	    else {
		switch (*++cp) {
		    case 't': isTitle = True; break;
		    case 'l': hasUnderline = True; break;
		    case 'n': ignoreMenuFunc = True; /* Fall through to next case */
		    case 'f': entryFunc = va_arg(args, PFI); break;
		    case 'F': pup->menuFunc = va_arg(args, PFI); break;
		    case 'm': subMenu = va_arg(args, Menu*); break;
		    case 'x':
		      returnValue = (int)strtol(++cp, &cp, 10);
		      cp--;
		      break;
		    case '%': *dp++ = '%'; break;
		}
		cp++;
	    }
	}

	if (dp == collectName)
	    break;
	*dp = 0;
	/*
	** Now we've collected all the info, add the item(s) to the menu
	*/
	if (isTitle)
	    set_title_menu((Menu*) pup, collectName);
	else {
	    it = create_item((Menu*) pup, ++lastItemNo, collectName);
	    it->pickValue = returnValue;
	    if (subMenu)
		it->subMenu = subMenu;
	    if (ignoreMenuFunc)
		it->useMenuFunc = 0;
	    it->itemFunc = entryFunc;
	    if (hasUnderline)
		create_item((Menu*) pup, 0, 0);
	}
	if (*cp == 0)
	    break;
    }
}

long
newpup(Display *dpy, int screen) {
    return (long) create_menu(dpy, screen, False);
}

void 
addtopup(long pup, char *str, ...) {
    va_list args;
    va_start(args, str);
    (void) do_addpup((Menu*) pup, str, args);
    va_end(args);
}

long
defpup(Display *dpy, int scrn, char *str, ...) {
    va_list args;
    long ret = newpup(dpy, scrn);
    va_start(args, str);
    (void) do_addpup((Menu*) ret, str, args);
    va_end(args);
    return ret;
}

long
dopup(long menu) {
    long ret;
    Menu* pup = (Menu*) menu;
    Window junkw;
    int junki, x, y;
    XQueryPointer(pup->popupInfo->dpy,
	RootWindow(pup->popupInfo->dpy, pup->popupInfo->screen),
	&junkw, &junkw, &x, &y, &junki, &junki, (unsigned *)&junki);
    ret = popup_menu((Menu*) menu, Button3, x, y);
    XFlush(((Menu*)menu)->popupInfo->dpy);
    return ret;
}

void
freepup(long menu) {
    if (menu)
	destroy_menu((Menu*) menu);
}

void
setpup(long menu, int itemNo, unsigned int arg) {
    MenuItem* it;

    if (it = find_item((Menu*) menu, itemNo)) {
	set_enable_item(it, ((arg & PUP_GREY) == 0));
	set_check_item(it, (arg & (PUP_CHECK|PUP_BOX)));
    }
}

static void realize_menu(Menu* , int , int );
static void map_menu(Menu* );
static void enter_menu(Window , int , int );
static void leave_item_menu(Menu* );
static void move_to(int , int );
static void enter_item_menu(Menu*,MenuItem* );
static void cancel_menu(Menu* );
static void render_menu(Menu* , int , int );
static void append_item_menu(Menu* , MenuItem* );
static void layout_menu(Menu* );
static Menu* find_menu(Menu* , Window );
static void resize_menu(Menu*, int, int);

static Menu* currentMenu;

static char shadowBits[16*2] = {
    0xAA, 0xAA, 0x55, 0x55,
    0xAA, 0xAA, 0x55, 0x55,
    0xAA, 0xAA, 0x55, 0x55,
    0xAA, 0xAA, 0x55, 0x55,
    0xAA, 0xAA, 0x55, 0x55,
    0xAA, 0xAA, 0x55, 0x55,
    0xAA, 0xAA, 0x55, 0x55,
    0xAA, 0xAA, 0x55, 0x55,
};
#define checkWidth 16
#define checkHeight 8
static char checkBits[] = {
   0x00, 0x0f,
   0x86, 0x03,
   0xcf, 0x01,
   0xec, 0x00,
   0x78, 0x00,
   0x38, 0x00,
   0x10, 0x00,
   0x10, 0x00,
};

/*ARGSUSED*/
static char const *
getresource(const char* class, const char* instance, ...) 
{
    return 0;
}

static void
grab_everything(XEvent* ev, PopupInfo* pInfo) 
{
    int press;
    static have_active_grab;

    switch (ev->type) {
      case KeyPress:
      case ButtonPress:
	press = 1;
	break;
      case KeyRelease:
      case ButtonRelease:
	press = 0;
	break;
      default:
	return;
    }

    if (press) {
	if (!have_active_grab) {
	    Cursor c = pInfo->arrowCursor;
	    Display *dpy = pInfo->dpy;
	    int gp;
	    Window window = RootWindow(dpy, pInfo->screen);
#define POINTER_GRAB_MASK (PointerMotionMask|ButtonPressMask|ButtonReleaseMask)

	    gp = XGrabPointer(dpy, window, True, POINTER_GRAB_MASK,
				   GrabModeAsync, GrabModeAsync,
				   None, c, CurrentTime);
	    if ((gp == GrabSuccess)) /* winner */
		have_active_grab = 1;
	}
    } else {
	Display *dpy = pInfo->dpy;
	if (have_active_grab) {
	    XUngrabPointer(dpy, CurrentTime);
	    have_active_grab = 0;
	}
    }
}

static Visual *
find_popup(Display *dpy, int screen, int *depth) {

    XVisualInfo *vinfo, template;
    int nvisuals;

    template.screen = screen;
    vinfo = XGetVisualInfo(dpy, VisualScreenMask, &template, &nvisuals);

    /* SGI convention is that first visual is PUP visual XXX */
    *depth = vinfo->depth;
    return vinfo->visual;
}


/*---------------------------------------------------------------------- */
/* "allPopupInfo" points to a list of structures, one per screen, which  */
/* contains information about how popup menus are rendered on that	 */
/* screen.  In the future, it may be per screen/visual pair, so that	 */
/* not all popups have to be in the same planes.			 */
/*---------------------------------------------------------------------- */

static PopupInfo* allPopupInfo;

static PopupInfo*
create_PopupInfo(Display* dpy, int screen) {
    PopupInfo* ret = allPopupInfo;
    XGCValues	gcv;
    int		isOverlay = 1;
    XColor	light_def, medium_def, dark_def;
    Window	root = RootWindow(dpy, screen);
    const char*	cname;

    while (ret) {
	if (ret->dpy == dpy && ret->screen == screen)
	    return ret;
	ret = ret->next;
    }
    ret = ALLOC(sizeof(*ret));
    ret->dpy = dpy;
    ret->screen = screen;
    ret->visual = find_popup(dpy, screen, &ret->depth);
    ret->colormap = XCreateColormap(dpy, root, ret->visual, AllocNone);
    ret->arrowCursor = XCreateFontCursor(dpy, XC_arrow);
    ret->overFirst = False;
    if (cname = getresource(m_class, m_inst, "Timeout", "hogTimeout", 0)) {
	HogTimeout = atoi(cname);
	FREE((char *)cname);
    } else
	HogTimeout = HOG_TIMEOUT;
    if (cname = getresource(m_class, m_inst, "Timeout", "postTimeout", 0)) {
	PostTimeout = atoi(cname);
	FREE((char *)cname);
    } else
	PostTimeout = POST_TIMEOUT;
    if (cname = getresource(m_class, m_inst, "OverFirst", "overFirst", 0)) {
	if (strcasecmp(cname, "true") == 0)
	    ret->overFirst = True;
	FREE((char *)cname);
    }
    
    /* First color in the menu colormap is medium (default grey) */
    if (cname = getresource(m_class, m_inst, "Background", "mediumColor", 0)) {
	XParseColor(dpy, ret->colormap, cname, &medium_def);
	FREE((char *)cname);
    } else
	medium_def.red = medium_def.green = medium_def.blue = 0xaa00;
    XAllocColor(dpy, ret->colormap, &medium_def);

    /* 2nd color in the menu colormap is dark (default black) */
    if (cname = getresource(m_class, m_inst, "Foreground", "darkColor", 0)) {
	XParseColor(dpy, ret->colormap, cname, &dark_def);
	FREE((char *)cname);
    } else
	dark_def.red = dark_def.green = dark_def.blue = 0;
    XAllocColor(dpy, ret->colormap, &dark_def);

    /* 3nd color in the menu colormap is light (default white) */
    if (cname = getresource(m_class, m_inst, "Background", "lightColor", 0)) {
	XParseColor(dpy, ret->colormap, cname, &light_def);
	FREE((char *)cname);
    } else
	light_def.red = light_def.green = light_def.blue = 0xff00;
    XAllocColor(dpy, ret->colormap, &light_def);

    ret->shadowPixmap =
	    XCreatePixmapFromBitmapData(dpy, root, shadowBits, 16, 16,
					isOverlay ? 0 : light_def.pixel,
					dark_def.pixel,
					ret->depth);
    
    gcv.foreground = dark_def.pixel;
    gcv.stipple = XCreatePixmapFromBitmapData(dpy, root, checkBits,
					     checkWidth, checkHeight, 1, 0, 1);
    ret->black = XCreateGC(dpy, ret->shadowPixmap, 
			   GCForeground|GCStipple, &gcv);
    XFreePixmap(dpy, gcv.stipple);
    gcv.fill_style = FillOpaqueStippled;
    gcv.stipple = XCreatePixmapFromBitmapData(dpy, root,
					      shadowBits, 16, 16, 0, 1, 1);
    gcv.foreground = medium_def.pixel;
    ret->lightGrey = XCreateGC(dpy, ret->shadowPixmap, GCForeground, &gcv);
    gcv.background = dark_def.pixel;
    ret->darkGrey = XCreateGC(dpy, ret->shadowPixmap,
			GCForeground|GCBackground|GCFillStyle|GCStipple, &gcv);
    gcv.foreground = light_def.pixel;
    ret->grey = XCreateGC(dpy, ret->shadowPixmap,
			GCForeground|GCBackground|GCFillStyle|GCStipple, &gcv);
    ret->white = XCreateGC(dpy, ret->shadowPixmap, GCForeground, &gcv);
    gcv.background = medium_def.pixel;
    ret->lightWhiteGrey = XCreateGC(dpy, ret->shadowPixmap,
			GCForeground|GCBackground|GCFillStyle|GCStipple, &gcv);

    ret->whitePixel     = (int)light_def.pixel;
    ret->blackPixel     = (int)dark_def.pixel;
    ret->lightGreyPixel = (int)medium_def.pixel;

    XFreePixmap(dpy, gcv.stipple);
#ifdef DEBUG_RENDER
    {	/* Pick the four wild colors to see how things get framed! */
	int i;
	for (i = 0; i < 4; i++) {
	    gcv.foreground = i+1;
	    ret->regularColors[i]=
		XCreateGC(dpy, ret->shadowPixmap, GCForeground, &gcv);
	}
    }
#else
    ret->regularColors[0] = ret->white;
    ret->regularColors[1] = ret->darkGrey;
    ret->regularColors[2] = ret->white;
    ret->regularColors[3] = ret->grey;
#endif
    ret->titleColors[0] = ret->white;
    ret->titleColors[1] = ret->darkGrey;
    ret->titleColors[2] = ret->lightWhiteGrey;
    ret->titleColors[3] = ret->grey;
    ret->selectedColors[0] = ret->white;
    ret->selectedColors[1] = ret->darkGrey;
    ret->selectedColors[2] = ret->white;
    ret->selectedColors[3] = ret->white;
    ret->checkBoxOnColors[0] = ret->black;
    ret->checkBoxOnColors[1] = ret->white;
    ret->checkBoxOnColors[2] = ret->lightGrey;
    ret->checkBoxOnColors[3] = ret->white;
    ret->checkBoxOffColors[0] = ret->darkGrey;
    ret->checkBoxOffColors[1] = ret->darkGrey;
    ret->checkBoxOffColors[2] = ret->white;
    ret->checkBoxOffColors[3] = ret->darkGrey;

    ret->lightWhiteGreyPixmap =
	    XCreatePixmapFromBitmapData(dpy, root, shadowBits, 16, 16,
					     medium_def.pixel,
					     light_def.pixel,
					     ret->depth);

    ret->next = allPopupInfo;
    allPopupInfo = ret;
    return ret;
}

/*---------------------------------------------------------------------- */

static Menu*
create_menu(Display* dpy, int screen, int isTitle) {
    Menu*			ret;
    XSetWindowAttributes	init;
    int			mask;

    ret = (Menu*) ALLOC(sizeof *ret);
    bzero(ret, sizeof *ret);

    ret->popupInfo = create_PopupInfo(dpy, screen);
    if (ret->isTitle = isTitle) {
	ret->menuFont = create_font(dpy,
		getresource("MenuTitle", "menuTitle", "Font", "font", 0));
	init.background_pixel = ret->popupInfo->lightGreyPixel;
	mask = CWBackPixel;
    } else {
	ret->menuFont = create_font(dpy,
			    getresource(m_class, m_inst, "Font", "font", 0));
	init.background_pixmap = ret->popupInfo->lightWhiteGreyPixmap;
	mask = CWBackPixmap;
    }
    init.border_pixel = ret->popupInfo->blackPixel;
    init.override_redirect = True;
    init.cursor = ret->popupInfo->arrowCursor;
    init.colormap = ret->popupInfo->colormap;
    init.event_mask = EnterWindowMask|LeaveWindowMask|FocusChangeMask|ButtonPressMask
			|ButtonReleaseMask|ExposureMask|PointerMotionMask
			|ButtonMotionMask;

    ret->contentWindow = XCreateWindow(dpy, RootWindow(dpy, screen),
	0, 0,
	100, 100,
	1,
	ret->popupInfo->depth,
	InputOutput,
	ret->popupInfo->visual,
	mask|CWColormap|CWEventMask|CWBorderPixel
	    |CWOverrideRedirect|CWCursor,
	&init
    );

    init.background_pixmap = ret->popupInfo->shadowPixmap;
    init.event_mask = ButtonPressMask | ButtonReleaseMask;
    ret->shadowWindow = XCreateWindow(dpy, RootWindow(dpy, screen),
	    0, 0,
	    100, 100,
	    0, ret->popupInfo->depth,
	    InputOutput, ret->popupInfo->visual,
	    CWEventMask|CWColormap|CWBackPixmap
		|CWBorderPixel|CWOverrideRedirect|CWCursor,
	    &init
	);
    return ret;
}

static void
set_title_menu(Menu* this, char *title) {
    if (this->isTitle)
	return;
    if (this->menuTitle)
	destroy_menu(this->menuTitle);
    this->menuTitle = create_menu(this->popupInfo->dpy,
				    this->popupInfo->screen, True);
    create_item(this->menuTitle, 0, title);
    this->layoutDone = 0;
}

static void
destroy_menu(Menu* this) {
    if (this->head) destroy_item(this->head);
    if (this->menuTitle) destroy_menu(this->menuTitle);
    bzero(this, sizeof *this);
    FREE(this);
}

static void
map_menu(Menu* this) {
    Display *dpy = this->popupInfo->dpy;
    if (this->popping) {
	XRaiseWindow(dpy, this->shadowWindow);
	XResizeWindow(dpy, this->shadowWindow, this->width, this->height);
	XResizeWindow(dpy, this->contentWindow, this->width, this->height);
	XRaiseWindow(dpy, this->contentWindow);
	/*
	XMapWindow(dpy, this->shadowWindow); XXXbly
	*/
	XMapWindow(dpy, this->contentWindow);
	XMapWindow(dpy, this->shadowWindow);
	this->mapped = True;
    }
}

/* XXX try to center menu (title?) under cursor */
/* XXX need a resource for auto-select top item */
/* XXX need a resource for auto-select last item (YEAH) */

static void
realize_menu(Menu* this, int rx, int ry) {
    Display *dpy;
    int screenWidth;
    int screenHeight;
    int w, h;

    dpy = this->popupInfo->dpy;
    screenWidth = DisplayWidth(dpy, this->popupInfo->screen);
    screenHeight = DisplayHeight(dpy, this->popupInfo->screen);

    if (!this->isTitle) {
	layout_menu(this);	/* Title layout happens from menu layout */
	if (this->overFirst && this->head) {
	    rx -= this->head->width/2;
	    ry -= this->head->height/2;
	}
    }
    if (rx < 0)
	rx = 0;
    w = this->width + 2;			/* add in border width */
    if (rx + this->width > screenWidth) {
	rx = screenWidth - w;
    }

    h = this->height + 2;			/* add in border width */
    if (this->menuTitle) {
	int menuTitleHeight = this->menuTitle->height + Menu_titleYGap;
	if (ry < menuTitleHeight) {
	    ry = menuTitleHeight;
	}
    } else if (ry < 0)
	ry = 0;
    if (ry + h > screenHeight) {
	ry = screenHeight - h;
    }
    if (this->menuTitle) {
	realize_menu(this->menuTitle, rx, ry - this->menuTitle->height
					- Menu_titleYGap);
    }
    XMoveWindow(dpy, this->contentWindow, rx, ry);
    XMoveWindow(dpy, this->shadowWindow, rx + Menu_shadowXOffset,
		     ry + Menu_shadowYOffset);
    this->popping = True;
    this->currentItem = 0;
    this->currentSubMenu = 0;
    this->stackPrev = 0;
    map_menu(this);
}

static Menu*
find_menu(Menu* this, Window lookfor) {
    MenuItem*	it;

    if (lookfor == this->contentWindow)
	return this;
    if (this->menuTitle && this->menuTitle->contentWindow == lookfor)
	return this->menuTitle;
    for (it = this->head; it; it = it->nextItem) {
	Menu* foundBelow;
	if (it->subMenu && (foundBelow = find_menu(it->subMenu, lookfor)))
	    return foundBelow;
    }
    return False;
}

static int
popup_menu(Menu* this, int b, int rx, int ry) {
    static int alreadyUp = 0;
    PopupInfo* pInfo = this->popupInfo;
    Display	*dpy = pInfo->dpy;
    this->button = b;

    if (alreadyUp) {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "popup_menu: can't call dopup from a menu function");
	return -1;
    }
    XInstallColormap(dpy, pInfo->colormap);
    this->overFirst = pInfo->overFirst;
    realize_menu(this, rx, ry);

    {
    /* should check that the button is still down ... */
    XEvent ev;
    ev.type = ButtonPress;
    grab_everything(&ev, pInfo); /* XXX */
    }

    alreadyUp = 1;
    /* Spin, waiting for event that dismisses the menu */
    currentMenu = this;
    set_state(MS_NORMALSTATE);
    for (;;) {
	XEvent ev;
#ifdef NEED_NEXT_EVENT
	next_event(dpy, &ev);
#endif
	XNextEvent(dpy, &ev);
	switch (ev.type) {
	  case KeyPress:
	  case KeyRelease:
	  case ButtonPress:
		grab_everything(&ev, pInfo);
	    break;
	  case ButtonRelease:
	    {	
	    grab_everything(&ev, pInfo);
	    if (ev.xbutton.button == b) 
	    {
		MenuItem* result = currentMenu->currentItem;
		aFuncChain	*chain;
		int		pick = -1;

		chain = create_aFuncChain();
		if (result) {
		    extend_aFuncChain(chain, result->itemFunc);
		    if (result->useMenuFunc) {
			Menu*	amen;
			for (amen = result->parent; amen != NULL; amen = amen->stackPrev)
			    extend_aFuncChain(chain, amen->menuFunc);
		    }
		}
		cancel_menu(this);
		if (result && result->enabled && !result->subMenu)
		    pick = call_aFuncChain(chain, result->pickValue);
		alreadyUp = 0;
		destroy_aFuncChain(chain);
		return pick;
	    }
	    }
	    break;
	  case MotionNotify:
	    {
		int haveXY = False;
		int lx, ly;
		Window ww = currentMenu->contentWindow;
		do {
		    if (ev.xmotion.window == ww) {
			history[mhead].x = lx = ev.xmotion.x;
			history[mhead].y = ly = ev.xmotion.y;
			mhead = (mhead + 1) % MOTION_HISTORY;
			haveXY = True;
		    }
		} while (XCheckMaskEvent(dpy, ButtonMotionMask, &ev));
		if (haveXY)
		    move_to(lx, ly);
	    }
	    break;
	  case EnterNotify:
	    {
		Window window;
		int x, y, type;

		do {
		    x = ev.xcrossing.x;
		    y = ev.xcrossing.y;
		    type = ev.type;
		    window = ev.xcrossing.window;
		} while (XCheckMaskEvent(dpy,
			    EnterWindowMask|LeaveWindowMask, &ev));
		if (type == EnterNotify)
		{
		    enter_menu(window, x, y);
		}
	    }
	    break;
	  case FocusOut:
		if (MenuState != MS_HOGSTATE)
		{
		    leave_item_menu(currentMenu);
		}
	    break;
	  case Expose:
	    {
		Window theWindow = ev.xexpose.window;
		Menu* theMenu = find_menu(this, theWindow);
		if (theMenu) {
		    int topY = theMenu->height;
		    int bottomY = 0;
		    do {
			if (ev.xexpose.y < topY) topY = ev.xexpose.y;
			if (ev.xexpose.y+ev.xexpose.height > bottomY)
			    bottomY = ev.xexpose.y+ev.xexpose.height;
			if (ev.xexpose.count == 0)
			  render_menu(theMenu, topY, bottomY);
		    } while 
			(XCheckWindowEvent(dpy, theWindow, ExposureMask, &ev));
		}
		break;
	    }
	}
    }
}

static void
enter_menu(Window evw, int x, int y) {
    Menu* m;
    if ((m = currentMenu->currentSubMenu) && (evw == m->contentWindow)) {
	/* Entered the current menus sub menu.  Make the sub menu the new */
	/* current menu. */
	set_state(MS_NORMALSTATE);
	m->stackPrev = currentMenu;
	currentMenu = m;
	assert(currentMenu->currentItem == 0);
	assert(currentMenu->currentSubMenu == 0);
	move_to(x, y);
	return;
    }

    if (MenuState == MS_HOGSTATE
	&& evw != currentMenu->currentSubMenu->contentWindow)
	return;
    set_state(MS_NORMALSTATE);
    if ((m = currentMenu->stackPrev) && (evw == m->contentWindow)) {
	/* Entered the current menus parent.  Don't cancel the sub menu */
	/* automatically.  Instead, just revert the currentMenu. */
	leave_item_menu(currentMenu);
	currentMenu->stackPrev = 0;
	currentMenu = m;
	assert(currentMenu->currentItem != 0);
	assert(currentMenu->currentSubMenu != 0);
	move_to(x, y);
	return;
    }

    m = currentMenu;
    while (m) {
	if (m->contentWindow == evw) {
	    while (currentMenu != m) {
		leave_item_menu(currentMenu);
		currentMenu = currentMenu->stackPrev;
	    }
	    move_to(x, y);
	    return;
	}
	m = m->stackPrev;
    }
    leave_item_menu(currentMenu);
}

static void
leave_item_menu(Menu* this) {
    set_state(MS_NORMALSTATE);
    if (this->currentItem) {
	leave_item(this->currentItem);
	this->currentItem = 0;
	if (this->currentSubMenu) {
	    cancel_menu(this->currentSubMenu);
	    this->currentSubMenu = 0;
	}
    }
}

/*
** Look at the motion history buffer and see if the delta in x is
** greater than or equal to the delta in y
*/

static int
history_allows(void) {
    struct hbuf *oldest, *newest;
    int dx, dy;

    newest = history + (mhead+MOTION_HISTORY-1)%MOTION_HISTORY;
    oldest = history + mhead;
    dx = newest->x - oldest->x;
    dy = newest->y - oldest->y;
    return dx > 0 && dy > 0 && dx*2 > dy;
}

static void
move_to(int evx, int evy)
{
    MenuItem* it = currentMenu->head;
    if (MenuState == MS_HOGSTATE)
	return;
    while (it) {
	/* See if evx & evyy are inside an item */
	if ((evx >= it->x) && (evx < it->x + it->width) &&
	    (evy >= it->y) && (evy < it->y + it->height)) {
	    if (it != currentMenu->currentItem) {
		if (MenuState == MS_PULLRIGHTACTIVE && history_allows())  {
		    set_state(MS_HOGSTATE);
		    return;
		}
		set_state(MS_NORMALSTATE);
		leave_item_menu(currentMenu);
		enter_item_menu(currentMenu, it);
		currentMenu->initialX = evx;
	    }
	    return;
	}
	it = it->nextItem;
    }
    if (MenuState == MS_PULLRIGHTACTIVE && history_allows())
	set_state(MS_HOGSTATE);
    if (MenuState != MS_HOGSTATE)
	leave_item_menu(currentMenu);
}

static void
enter_item_menu(Menu* this,MenuItem* it) {
    Menu* sub = it->subMenu;
    this->currentItem = it;
    enter_item(it);

    if (it->enabled && sub) {
	this->currentSubMenu = sub;
	set_state(MS_PULLRIGHTPENDING);
    }
}

#ifdef NEED_NEXT_EVENT
static void
post_submenu(void) {
    Menu* this = currentMenu;
    MenuItem* it = currentMenu->currentItem;
    Display*	dpy = currentMenu->popupInfo->dpy;
    int rootx, rooty;
    Window child;

    XTranslateCoordinates(dpy, this->contentWindow,
	  RootWindow(dpy, this->popupInfo->screen),
	  it->x + it->width + Menu_subMenuXOffset,
	  it->y + Menu_subMenuYOffset,
	  &rootx, &rooty, &child);
    this->currentSubMenu->overFirst = False;
    realize_menu(this->currentSubMenu, rootx, rooty);
    XFlush(dpy);
    set_state(MS_PULLRIGHTACTIVE);
}
#endif

static void
cancel_menu(Menu* this) {
    Display* dpy = this->popupInfo->dpy;

    XUnmapWindow(dpy, this->shadowWindow);
    XUnmapWindow(dpy, this->contentWindow);
    set_state(MS_NORMALSTATE);
    this->mapped = False;
    if (this->menuTitle)
	cancel_menu(this->menuTitle);
    this->popping = False;
    leave_item_menu(this);
    this->stackPrev = 0;
}

static void
render_menu(Menu* this, int startY, int endY) {
    PopupInfo* pi = this->popupInfo;
    MenuItem* it = this->head;
    if (!this->mapped)
	return;
    render_outline(pi->dpy, this->contentWindow,
		    this->isTitle ? pi->titleColors : pi->regularColors,
		    0, 0, this->width, this->height);
    while (it) {
	if (it->y + it->height >= startY && it->y < endY)
	    render_item(it);
	it = it->nextItem;
    }
}

static void
append_item_menu(Menu* this, MenuItem* it) {
    if (this->head)
	this->tail->nextItem = it;
    else
	this->head = it;
    this->tail = it;
}

static void
layout_menu(Menu* this) {
    /* Find maximum width and total height.  Position each menu-item at */
    /* its final x & y relative to the menu window. */
    short maxWidth = 0;
    int y = Menu_topInset;
    int checkAdjust = 0;
    MenuItem* it = this->head;
    int arrowAdjust = 0;
    if (this->layoutDone)
	return;
    this->layoutDone = 1;
    while (it) {
	short iw, ih;
	MenuItem* next = it->nextItem;
	reset_MenuItem(it);
	iw = it->width;
	ih = it->height;
	if (it->subMenu) {
	    arrowAdjust = Menu_arrowAdjust;
	    if (ih < Menu_arrowHeight) {
		ih = Menu_arrowHeight;
		resize_item(it, iw, ih);
	    }
	}
	if (it->type == MenuItem_hasCheckBox)
	    checkAdjust = Menu_checkAdjust;
	if (it->label && ih < Menu_checkHeight) ih = Menu_checkHeight;
	if (iw > maxWidth) maxWidth = iw;
	move_item(it, Menu_leftInset, y);
	y += ih;
	if (it->label && next)
	    y += Menu_itemOffset;
	it = next;
    }
    maxWidth +=
	checkAdjust + arrowAdjust + Menu_itemLeftInset + Menu_itemRightInset;
    this->width = maxWidth + Menu_widthAdjust;
    if (this->menuTitle) {
	int tw;
	layout_menu(this->menuTitle);
	tw = this->menuTitle->width;
	if (this->width <= tw) {
	    int adjust = tw - this->width;
	    this->width = tw;
	    maxWidth += adjust;
	} else
	    resize_menu(this->menuTitle,this->width,this->menuTitle->height);
    }
    this->height = y + Menu_bottomInset;

    /* Now fix the widths of the menu items */
    for (it = this->head; (it) ; it = it->nextItem) {
	it->inset = checkAdjust;
	resize_item(it, maxWidth, it->height);
    }
}

static void
resize_menu(Menu* this, int w, int h) {
    this->width = w;
    this->height = h;
}

static MenuItem*
find_item(Menu* this, int itemNo) {
    MenuItem* it;

    for (it = this->head; it; it = it->nextItem)
	if (it->index == itemNo)
	    return it;
    return 0;
}

/*----------------------------------------------------------------------*/

static void
draw_arrow (Display* dpy, Window win, GC in, GC edge, int x, int y, int w, int h) {
    XPoint p[5];
    p[0].x = p[4].x = x;
    p[0].y = p[4].y = y;
    p[1].x = x + w - 1;
    p[1].y = y + (h / 2) - 1;
    p[2].x = p[1].x;
    p[2].y = p[1].y + 1;
    p[3].x = x;
    p[3].y = y + h - 1;
    XFillPolygon(dpy, win, in, p, 4, Convex, CoordModeOrigin);
    XDrawLines(dpy, win, edge, p, 5, CoordModeOrigin);
}

/*----------------------------------------------------------------------*/

static MenuItem*
create_item(Menu* m, int ix, char* label) {
    MenuItem* this = ALLOC(sizeof *this);
    this->parent = m;
    append_item_menu(m, this);
    this->nextItem = 0;
    this->type = MenuItem_normal;
    this->enabled = True;
    this->visualState = MenuItem_quiet;
    this->checkState = 0;
    this->inset = 0;
    this->subMenu = 0;
    this->index = this->pickValue = ix;
    this->useMenuFunc = True;
    if (label) {
	this->label = STRDUP(label);
	this->len = (int)strlen(label);
	this->trueWidth = this->width =
		strwidth_font(m->menuFont, label, this->len);
	this->trueHeight = this->height = Menu_itemTopInset
		+ m->menuFont->finfo->ascent + m->menuFont->finfo->descent
		+ Menu_itemBottomInset;
    } else {
	this->label = 0;
	this->trueWidth = this->width = 0;
	this->trueHeight = this->height = 3;
	this->enabled = False;
    }
    return this;
}

static void
destroy_item(MenuItem* this) {
    if (this->nextItem) destroy_item(this->nextItem);
    if (this->label) FREE(this->label);
    bzero(this, sizeof *this);
    FREE(this);
}

static void
set_enable_item(MenuItem* this, int on) {
    this->visualState = on ? MenuItem_quiet : MenuItem_disabled;
    this->enabled = on;
}

static void
set_check_item(MenuItem* this, int on) {
    this->type = (on ? MenuItem_hasCheckBox : MenuItem_normal);
    this->checkState = on;
    this->parent->layoutDone = False;
}

static void
render_background_item(MenuItem* this) {
    PopupInfo *pi = this->parent->popupInfo;
    Display* dpy = pi->dpy;
    Window win = this->parent->contentWindow;

    if (this->visualState == MenuItem_selected) {
	render_fill(dpy, win, pi->selectedColors, pi->white,
		    this->x, this->y, this->width, this->height);
    }
    if (this->subMenu) {
	int xx = this->x + this->width
		-Menu_itemRightInset-Menu_arrowRightInset-Menu_arrowWidth;
	int yy = this->y + (this->height - Menu_arrowHeight) / 2;
	GC fg, bg;
	if (this->visualState == MenuItem_disabled)
	    fg = pi->lightWhiteGrey, bg = pi->darkGrey;
	else
	    fg = pi->white, bg = pi->black;
	draw_arrow(dpy, win, fg, bg, xx, yy,
		     Menu_arrowWidth, Menu_arrowHeight);
	XDrawLine(dpy, win, bg, xx, yy + Menu_arrowHeight,
		       xx + Menu_arrowWidth - 1, yy + Menu_arrowHeight/2+1);
    }
    if (this->type == MenuItem_hasCheckBox)
	draw_check(pi, win, this->checkState, this->x, this->y, this->height);
}

static void
enter_item(MenuItem *this) {
    if (this->enabled) {
	this->visualState = MenuItem_selected;
	render_item(this);
    }
}

static void
leave_item(MenuItem *this) {
    if (this->enabled) {
	this->visualState = MenuItem_quiet;
	XClearArea(this->parent->popupInfo->dpy, this->parent->contentWindow,
		    this->x, this->y,
		    this->width, this->height, False);
	if (this->parent->mapped)
	    render_item(this);
    }
}

static void
render_item(MenuItem *this) {
    PopupInfo *pi = this->parent->popupInfo;
    Window	w = this->parent->contentWindow;
    if (this->label) {
	int xx = this->x + Menu_itemLeftInset;
	int yy = this->y + Menu_itemTopInset;
	GC gc;

	render_background_item(this);
	if (this->visualState == MenuItem_disabled)
	    gc = pi->darkGrey;
	else
	    gc = pi->black;
	XSetFont(pi->dpy, gc, this->parent->menuFont->finfo->fid);
	XDrawString(pi->dpy, w, gc,
		    xx+this->inset, yy + this->parent->menuFont->finfo->ascent,
		    this->label, this->len);
    } else {
	int yy = this->y;
	int xr = this->x + this->width;
	XDrawLine(pi->dpy, w, pi->darkGrey, this->x, yy, xr, yy);
	yy++;
	XDrawLine(pi->dpy, w, pi->white, this->x, yy, xr, yy);
    }
}

static void
resize_item(MenuItem* this, int w, int h) {
    this->width = w;
    this->height = h;
}

static void
reset_MenuItem(MenuItem* this) {
    this->width = this->trueWidth;
    this->height = this->trueHeight;
}

static void
move_item(MenuItem* this, int x, int y) {
    this->x = x;
    this->y = y;
}

static void
render_edges (Display* dpy, Window win, GC *colors, int x, int y, int w, int h) {
    XPoint	p[3];

    p[0].x = x;			/* Start in lower left */
    p[0].y = y + h - 1;
    p[1].x = x;			/* Go to top left */
    p[1].y = y;
    p[2].x = x + w - 2;		/* Finish in upper right */
    p[2].y = y;
    XDrawLines(dpy, win, colors[0], p, 3, CoordModeOrigin);

    p[0].x = x + 1;		/* Start in lower left */
    p[0].y = y + h - 1;
    p[1].x = x + w - 1;		/* Go to lower right */
    p[1].y = y + h - 1;
    p[2].x = x + w - 1;		/* Finish in upper right */
    p[2].y = y;
    XDrawLines(dpy, win, colors[1], p, 3, CoordModeOrigin);
}

static void
render_outline (Display* dpy, Window win, GC *colors, int x, int y, int w, int h) {
    render_edges(dpy, win, colors, x, y, w, h);
    render_edges(dpy, win, colors+2, x+1, y+1, w-2, h-2);
}

static void
render_fill(Display* dpy, Window win, GC *colors, GC fillColor,
	int x, int y, int w, int h) {
    render_outline(dpy, win, colors, x, y, w, h);
    XFillRectangle(dpy, win, fillColor, x+2, y+2, w-4, h-4);
}

static void
draw_check(PopupInfo* pi, Window w, int on, int x, int y, int ih) {
    GC *gcs;

    gcs = (on ? pi->checkBoxOnColors : pi->checkBoxOffColors);
    x += Menu_checkLeftInset;
    y += (ih - Menu_checkHeight) / 2;
    if (on)
	render_fill(pi->dpy, w, gcs, pi->lightGrey,
		x, y, Menu_checkWidth, Menu_checkHeight);
    if (on & PUP_CHECK) {
	int xo = (x + 2) % checkWidth;
	int yo = (y + 2) % checkHeight;
	XSetFillStyle(pi->dpy, pi->black, FillStippled);
	XSetTSOrigin(pi->dpy, pi->black, xo, yo);
	XFillRectangle(pi->dpy, w, pi->black,
			    x + 2, y + 2, checkWidth, checkHeight);
	XSetFillStyle(pi->dpy, pi->black, FillSolid);
    }
}

static void
set_state(int newState) {
    int delay;

    switch (MenuState = newState) {
	case MS_PULLRIGHTACTIVE:
	case MS_NORMALSTATE: 
#ifdef NEED_NEXT_EVENT
	    StateTimeout = 0; 
#endif
	    return;

	case MS_HOGSTATE: delay = HogTimeout; break;
	case MS_PULLRIGHTPENDING: delay = PostTimeout; break;
    }
    delay = (delay * HZ) / 1000;
#ifdef NEED_NEXT_EVENT
    struct tms t;
    StateTimeout = times(&t) + delay;
#endif
}

#ifdef NEED_NEXT_EVENT
static void
timeout_happened(void) {
    switch (MenuState) {
	case MS_PULLRIGHTPENDING:
	    post_submenu();
	    break;
	case MS_HOGSTATE: {
	    struct hbuf *last;

	    cancel_menu(currentMenu->currentSubMenu);
	    currentMenu->currentSubMenu = 0;
	    set_state(MS_NORMALSTATE);
	    last = history + (mhead+MOTION_HISTORY-1)%MOTION_HISTORY;
	    move_to(last->x, last->y);
	    break;
	}
	default:
	    set_state(MS_NORMALSTATE);
	    break;
    }
}
#endif

#ifdef NEED_NEXT_EVENT
static void
next_event(Display* dpy, XEvent* pEv) {
    struct tms tms;
#if 1
    if (StateTimeout) {
	unsigned int now = times(&tms);

	if (now > StateTimeout)
	    timeout_happened();
	if (StateTimeout) {
	    int	fd;

	    if (XEventsQueued(dpy, QueuedAfterFlush) == 0) {
		fd = ConnectionNumber(dpy);
		do {
		    struct timeval t;
		    fd_set fds;
		    unsigned int delta;

		    FD_ZERO(&fds);
		    FD_SET(fd, &fds);
		    delta = StateTimeout - now;
		    t.tv_sec = delta / HZ;
		    t.tv_usec = (delta % HZ) * 1000000 / HZ;
		    switch (select(fd+1, &fds, 0, 0, &t)) {
			case 0:		/* timeout happened */
			    timeout_happened();
			    if (StateTimeout == 0) {
				fd = -1;
				break;
			    }
			    /* else do it again, fall through to .. */
			case -1:	/* Error, should call select again */
			    now = times(&tms);
			    break;
			default:	/* Data is ready */
			    if (XEventsQueued(dpy, QueuedAfterReading) == 0)
			    {
				pfNotify(PFNFY_FATAL, PFNFY_USAGE, "next_event: botch");
				abort();
			    }
			    fd = -1;
			    break;
		    }
		} while (fd >= 0);
	    }
	}
    }
#endif
    XNextEvent(dpy, pEv);
}
#endif

#endif /* !WIN32 */
