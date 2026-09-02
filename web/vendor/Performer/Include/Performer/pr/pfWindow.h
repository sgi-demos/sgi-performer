//
// Copyright 1995, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfWindow.h
//
// $Revision: 1.137 $
// $Date: 2002/12/08 23:02:33 $
//

#ifndef __PF_WINDOW_H__
#define __PF_WINDOW_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>


// !!! pfXDisplayPID is in the prda so share group prcesses
// can have separate X Displays and reuse them
//

// Modified bits are in pfinternal.h
typedef struct _pfGfxType _pfGfxType;

#define XErrorEvent void
#define XWindowAttributes int
#include <windows.h>
#include <GL/gl.h>
#include <wglext.h>

typedef int (*xErrorHandler)(pfWSConnection, XErrorEvent *);

typedef struct _pfWSThread
{
    pid_t pid;
    pfWindow *curWin;
    pfWSConnection dsp;
    pfWSWindow errorWindow;
    ulong firstProtectedXRequest, xerror;
} _pfWSThread;


// internal bitmasks for mode field used in in-lined query routines

#define _PFWIN_STATE_OPEN		0x00100
#define _PFWIN_STATE_CLOSE_GL		0x00200
#define _PFWIN_STATE_CLOSE_ALL		0x00600
#define _PFWIN_STATE_OPEN_MASK	(_PFWIN_STATE_OPEN | _PFWIN_STATE_CLOSE_GL | _PFWIN_STATE_CLOSE_ALL)

#define _PFWIN_STATE_RAISE_DIRTY	0x00020000
#define _PFWIN_STATE_CONFIG_DIRTY	0x00040000
#define _PFWIN_STATE_DRAWMODE_DIRTY	0x00080000
#define _PFWIN_STATE_DIRTY		0x001fC000
#define _PFWIN_STATE_HASBORDER		0x00200000
#define _PFWIN_USER_SET_WSNAME		0x01000000
#define _PFWIN_USER_SET_GLDESC		0x02000000
#define _PFWIN_MODE_MOD_MASK		0x032f00ff

#define _PFWIN_STATE_WINDOW		0x10000000
#define _PFWIN_STATE_PIXMAP		0x20000000
#define _PFWIN_STATE_PBUFFER		0x40000000
#define _PFWIN_STATE_DRAWABLE_MASK	0x70000000


#define _PFWIN_MODES_SHARE_MASK	(_PFWIN_STATE_DRAWABLE_MASK | WIN_MODE_SHARE_MASK)

/* these are in swapState flag */
#define _PFWIN_STATE_SWAP_GROUP_DIRTY	0x01
#define _PFWIN_STATE_SWAP_BARRIER_DIRTY	0x02
#define _PFWIN_STATE_SWAP_DIRTY_MASK	0x03
#define _PFWIN_STATE_IN_SWAP_GROUP	0x04
#define _PFWIN_STATE_IN_SWAP_BARRIER	0x08
#define _PFWIN_STATE_SWAP_MASK		0x0f

/* internal sizeDirty bit fields (short) */
#define _PFWIN_STATE_SIZE_DIRTY		0x00000001
#define _PFWIN_STATE_ORG_DIRTY		0x00000002
#define _PFWIN_STATE_ORGSIZE_DIRTY	0x00000003
#define _PFWIN_STATE_ORGSIZE_XWIN_DIRTY	0x00000004
#define _PFWIN_STATE_ORGSIZE_DIRTY_MASK	0x00000007
#define _PFWIN_STATE_SCREENORG_DIRTY	0x00000008
#define _PFWIN_STATE_VIEWPORT_DIRTY	0x00000010
#define _PFWIN_STATE_ORGSIZE_ALL_DIRTY	0x0000001f

extern "C" {     // EXPORT to CAPI
/* ----------------------- pfWindow Tokens ----------------------- */

/* FBConfig attribute array tokens */
#define PFFB_USE_GL              1       /* support GLX rendering */
#define PFFB_BUFFER_SIZE         2       /* depth of the color buffer */
#define PFFB_LEVEL               3       /* level in plane stacking */
#define PFFB_RGBA                4       /* true if RGBA mode */
#define PFFB_DOUBLEBUFFER        5       /* double buffering supported */
#define PFFB_STEREO              6       /* stereo buffering supported */
#define PFFB_AUX_BUFFERS         7       /* number of aux buffers */
#define PFFB_RED_SIZE            8       /* number of red component bits */
#define PFFB_GREEN_SIZE          9       /* number of green component bits */
#define PFFB_BLUE_SIZE           10      /* number of blue component bits */
#define PFFB_ALPHA_SIZE          11      /* number of alpha component bits */
#define PFFB_DEPTH_SIZE          12      /* number of depth bits */
#define PFFB_STENCIL_SIZE        13      /* number of stencil bits */
#define PFFB_ACCUM_RED_SIZE      14      /* number of red accum bits */
#define PFFB_ACCUM_GREEN_SIZE    15      /* number of green accum bits */
#define PFFB_ACCUM_BLUE_SIZE     16      /* number of blue accum bits */
#define PFFB_ACCUM_ALPHA_SIZE    17      /* number of alpha accum bits */

#ifdef __linux__
#define PFFB_SAMPLE_BUFFER	100000  /* the number of multisample buffers */
#define PFFB_SAMPLES		100001  /* number of samples per pixel */
#else
#define PFFB_SAMPLES		100000  /* number of samples per pixel */
#define PFFB_SAMPLE_BUFFER	100001  /* the number of multisample buffers */
#endif

/* pfWinMode */
#define PFWIN_AUTO_RESIZE	1
#define PFWIN_EXIT  		2
#define PFWIN_ORIGIN_LL		3
#define PFWIN_NOBORDER		4
#define PFWIN_HAS_OVERLAY	5
#define PFWIN_HAS_STATS		6

/* pfWinType */
#define PFWIN_TYPE_NOPORT	0x0100
#define PFWIN_TYPE_X		0x0200
#define PFWIN_TYPE_OVERLAY	0x0400
#define PFWIN_TYPE_STATS	0x0800
#define PFWIN_TYPE_UNMANAGED	0x1000
#define PFWIN_TYPE_PBUFFER	0x2000
#define PFWIN_TYPE_MASK		0x3f00


/* pfWinShare */
#define PFWIN_SHARE_TYPE	 0x0001
#define PFWIN_SHARE_MODE	 0x0002
#define PFWIN_SHARE_WSWINDOW	 0x0010
#define PFWIN_SHARE_FBCONFIG	 0x0020
#define PFWIN_SHARE_WSDRAWABLE_BIT  0x0040
#define PFWIN_SHARE_GL_CXT	 0x0080
#define PFWIN_SHARE_STATE_BIT	 0x0100
#define PFWIN_SHARE_GL_OBJS	 0x0200
#define PFWIN_SHARE_VISUALID	 0x0400
#define PFWIN_SHARE_OVERLAY_WIN	 0x1000
#define PFWIN_SHARE_STATS_WIN	 0x2000
#define PFWIN_SHARE_WSDRAWABLE	(PFWIN_SHARE_WSDRAWABLE_BIT | PFWIN_SHARE_FBCONFIG | PFWIN_SHARE_VISUALID | PFWIN_SHARE_WSWINDOW)
#define PFWIN_SHARE_STATE	(PFWIN_SHARE_STATE_BIT | PFWIN_SHARE_GL_CXT | PFWIN_SHARE_FBCONFIG | PFWIN_SHARE_VISUALID)

/* pfWinSelect */
#define PFWIN_GFX_WIN		-1
#define PFWIN_OVERLAY_WIN	-2
#define PFWIN_STATS_WIN		-3


/* 
 * pfQueryWin - framebuffer config queries - returns 1 value 
 */
#define PFQWIN_BIT		    0x100000	/* int */
#define PFQWIN_RGB_BITS	    	    0x100101	/* int */
#define PFQWIN_ALPHA_BITS	    0x100102	/* int */
#define PFQWIN_CI_BITS		    0x100103	/* int */
#define PFQWIN_DEPTH_BITS	    0x100104	/* int */
#define PFQWIN_MIN_DEPTH_VAL	    0x100105	/* int */
#define PFQWIN_MAX_DEPTH_VAL	    0x100106	/* int */
#define PFQWIN_MS_SAMPLES	    0x100107	/* int */
#define PFQWIN_STENCIL_BITS	    0x100108	/* int */
#define PFQWIN_DOUBLEBUFFER	    0x100109	/* int */ 
#define PFQWIN_STEREO		    0x10010a	/* int */
#define PFQWIN_NUM_AUX_BUFFERS	    0x10010b	/* int */  
#define PFQWIN_LEVEL		    0x10010c	/* int */
#define PFQWIN_ACCUM_RGB_BITS	    0x10010d	/* int */
#define PFQWIN_ACCUM_ALPHA_BITS	    0x10010e	/* int */

} // extern "C" END of C include export



#define PFWINDOW ((pfWindow*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFWINDOWBUFFER ((pfWindow*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfWindow : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename Win
    pfWindow();
    virtual ~pfWindow();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions
    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);
    virtual void printSwapGroup(void);
    virtual int getGLHandle() const { return index; }

public:
    // sets and gets
    void		setName(const char *_name);
    const char*		getName() const { return name; }
    void		setMode(int _mode, int _val);
    int			getMode(int _mode) const;
    // CAPI:verb WinType
    void		setWinType(unsigned int _type);
    // CAPI:verb GetWinType
    unsigned int	getWinType() const { return winType; }
    // CAPI:noverb
    pfState		*getCurState() const { return state; }
    void		setAspect(int _x, int _y);
    void		getAspect(int *_x, int *_y) const {
	*_x = xAspect; *_y = yAspect; 
    }
    void		setOriginSize(int _xo, int _yo, int _xs, int _ys);
    void		setOrigin(int _xo, int _yo);
    void		getOrigin(int *_xo, int *_yo) const {
	*_xo = xOrg; *_yo = yOrg;
    }
    void		setSize(int _xs, int _ys);
    void		getSize(int *_xs, int *_ys) const {
	*_xs = xSize; *_ys = ySize;
    }
    void		getScreenOrigin(int *_xo, int *_yo);
    void		setFullScreen();

    void		getCurOriginSize(int *_xo, int *_yo, int *_xs, int *_ys);
    void		getCurScreenOriginSize(int *_xo, int *_yo, int *_xs, int *_ys);
    void		setOverlayWin(pfWindow *_ow);
    pfWindow		*getOverlayWin() const {
	return overlay;
    }
    void		setStatsWin(pfWindow *_ow);
    pfWindow		*getStatsWin() const { return stats; }

    void		setScreen(int s);
    int			getScreen() const { return screen; }
    void		setShare(uint _mode);
    uint		getShare() const;
    void		setSwapBarrier(int _barrierName);
    int			getSwapBarrier() { return barrier; }
    int			inSwapGroup() { return (swapState & _PFWIN_STATE_IN_SWAP_GROUP); }
    
    //  X window 
    void		setWSWindow(pfWSConnection _dsp, pfWSWindow _wsWin);
    pfWSWindow		getWSWindow() const { return wsWin; }
    void		setWSDrawable(pfWSConnection _dsp, pfWSDrawable _wsWin);
    pfWSDrawable	getWSDrawable() const { return gXWin; }
    pfWSDrawable	getCurWSDrawable() const { 
	    return (selWin ? selWin->gXWin : gXWin);
    }
    void		setWSConnectionName(const char *_name);
    const char *	getWSConnectionName(void) const { return displayString; }
    void		setFBConfigData(void *_data);
    void*		getFBConfigData();
    void		setFBConfigAttrs(int *_attr);
    int*		getFBConfigAttrs() const;
    void                setPixelFormatAttrs(GLint *iattrs, GLfloat *fattrs);
    void                getPixelFormatAttrs(GLint *iattrs, GLfloat *fattrs);

    pfFBConfig          getFBConfig() const { return visualId; }
    void		setFBConfigId(int _vId);
    int			getFBConfigId(void) const { return (int)visualId; }
    
    void		setIndex(int _index);
    int			getIndex() const { return selectIndex; }
    pfWindow*		getSelect();

    void		setGLCxt(pfGLContext _gCxt);
    pfGLContext		getGLCxt() const { return gCxt; }
    
    // CAPI:verb WinList
    void		setWinList(pfList *_wl);
    // CAPI:verb GetWinList
    pfList*		getWinList() const { return winList; }

#define pfGLXFBConfig int 
    // CAPI:private
    void		setFBConfig(pfGLXFBConfig _fbconfig);


public:
    // other functions
    // CAPI:verb
    void	open();
    void	close();
    // CAPI:verb CloseWinGL
    void	closeGL();

    /* window share operations */
    int		attach(pfWindow *_w1);
    int		detach(pfWindow *_w1);
    
    /* swap group */
    // CAPI:verb AttachWinSwapGroup
    void	attachSwapGroup(pfWindow *_w1);
    // CAPI:verb DetachWinSwapGroup
    void	detachSwapGroup();
    
    /* make this the current window */
    pfWindow*	select();

    // CAPI:verb SwapWinBuffers
    void	swapBuffers();
    // CAPI:verb ChooseWinFBConfig
    pfFBConfig	chooseFBConfig(int *_attr);
    // CAPI:verb IsWinOpen
    int		isOpen() const { return ((mode & _PFWIN_STATE_OPEN_MASK) == _PFWIN_STATE_OPEN); }
    int		isManaged() const { 
	return (!((winType & PFWIN_TYPE_UNMANAGED) || (selWin->winType & PFWIN_TYPE_UNMANAGED))); 
    }

    int		query(int _which, int *_dst);
    int		mQuery(int *_which, int *_dst);

    static pfWindow* openNewNoPort(const char *name, int screen);
    
    //CAPI:verb WinWndClassName
    void        setWndClassName(char *name);

    // name these methods in such a fashio that it's clear
    // that they only exist on win32
    //CAPI:verb GetWin32Cursor
    HCURSOR     getWin32Cursor() const { return cursor; }
    //CAPI:verb Win32Cursor
    void        setWin32Cursor(HCURSOR _cursor) { cursor = _cursor; }

PFINTERNAL:  // XXX should make protected???

    int		    index;
    int		    selectIndex;
    int		    winType;
    int		    mode;
//--------------------------------------------------
    int		    xOrg,yOrg, xSize,ySize;
    int		    xScreenOrg, yScreenOrg;
//--------------------------------------------------
    short	    xAspect, yAspect;
    short	    screen;
    short	    sizeDirty;
//--------------------------------------------------
    int		    displayWidth, displayHeight;
    int		    shareMode;
    int		    modified;
    short	    barrier;
    short	    swapState;
//--------------------------------------------------
    // current framebuffer configuration
#define VisualID int
    VisualID	    visualId; // X visual Id
    int		    ms, depth, sten; // cache ms config info
    int		    zmin, zmax;
    float	    displaceBias; 
//--------------------------------------------------
    _pfGfxType	    *gfxType; // gfx hw capabilities and specs
    pfGLContext	    gCxt;
    pfState	    *state;
    pfList	    *winList;
    pfWSWindow	    wsWin;
    pfWSDrawable    gXWin;
   
    
    pfWindow	    *overlay;
    pfWindow	    *stats;
    pfWindow	    *selWin;
    pfList	    *shareGroup;
    pfList	    *swapGroup;
    static WNDCLASS       dftWndClass; //performer window class
    static WNDCLASS       dftChildWndClass; //Window class for child windows
    char *wndClassName;
    /* WGL Extended pixel format configuration attributes */
    /* Current configuration */
    GLint           *WGLIntAttribList;
    GLfloat         *WGLFloatAttribList;
    /* What the user requested */
    GLint           *WGLReqIntAttribList;
    GLfloat         *WGLReqFloatAttribList;
    HCURSOR         cursor; 
    /* For now, we need to cache the functions for WGL extensions we use */
    PFNWGLGETEXTENSIONSSTRINGARBPROC    wglGetExtensionsStringARB;
    PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB;
    PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfvARB;
    PFNWGLCHOOSEPIXELFORMATARBPROC      wglChoosePixelFormatARB;
    char	    name[PF_MAXSTRING];
    char	    displayString[PF_MAXSTRING];

private:
    static pfType *classType;
public:
    void   *_pfWindowExtraData;
};

#endif // !__PF_WINDOW_H__
