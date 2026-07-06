/*
 * Copyright 1995, Silicon Graphics, Inc.
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

/*
 * file: pfutil.h
 * --------------
 *
 * $Revision: 1.343 $
 * $Date: 2002/12/08 23:02:33 $
 *
 * IRIS Performer utility library header.
 */

#ifndef __PFUTIL_H__
#define __PFUTIL_H__

#include <Performer/pfutil-DLL.h>
#include <Performer/pf.h>

#include <win32stubs.h>

#if PF_CPLUSPLUS_API
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#endif



#ifdef __cplusplus
extern "C" {
#endif

#if !PF_CPLUSPLUS_API
typedef struct _pfuClipCenterNode pfuClipCenterNode;
typedef struct _pfuTexGenClipCenterNode pfuTexGenClipCenterNode;
#else
class pfuClipCenterNode;
class pfuTexGenClipCenterNode;
#endif /* !PF_CPLUSPLUS_API */

/*-------------------- libpfutil Management --------------------*/

/*
 * Initialize, and exit libpfutil. The utility routines rely on a
 * pfDataPool for shared memory multiprocessed communication.
 */

/* Datapool identifiers */
#define PFUGUI_PICK_DPID	    0x0080
#define PFUGUI_FLAGS_DPID	    0x0100
#define PFUGUI_PANELS_DPID	    0x0200
#define PFUGUI_WIDGETS_DPID	    0x0300
#define PFUGUI_DATA_DPID	    0x0400
#define PFUGUI_ATTRS_DPID	    0x0500
#define PFUGUI_ATTRSBLOCK_DPID	    0x1000
#define PFU_CURSOR_DATA_DPID	    0x2000

#define PFU_TEXLIST_DATA_DPID       0x10000000

#define PFU_XIO_DATA_DPID	    0x40000000
#define PFU_XEVENT_DATA_DPID        0x80000001
#define PFU_XWIN_DATA_DPID	    0x80000002
#define PFU_MOUSE_DATA_DPID	    0x80000004

/* callback return Values */
#define PFU_CB_CONT	0
#define PFU_CB_TERM	1
#define PFU_CB_ABORT	2

/* compatibility define for old vdersions */
#define pfuInitUtil pfuInit
extern PFUDLLEXPORT void	    pfuInit(void);
extern PFUDLLEXPORT void	    pfuInitClasses(void);
extern PFUDLLEXPORT pfDataPool*  pfuGetUtilDPool(void);
extern PFUDLLEXPORT void	    pfuExitUtil(void);
extern PFUDLLEXPORT void	    pfuDPoolSize(long size);
extern PFUDLLEXPORT long	    pfuGetDPoolSize(void);
extern PFUDLLEXPORT volatile void* pfuFindUtilDPData(int id);

/*---------------- pfuAutoList CAPI ------------------------------*/

#if PF_CPLUSPLUS_API
class pfuAutoList;
#else
typedef struct _pfuAutoList pfuAutoList;
#endif /* PF_CPLUSPLUS_API */

#if PF_C_API

extern PFUDLLEXPORT void 	    	    pfuInit(void);  

extern PFUDLLEXPORT pfuAutoList*         pfuNewAutoList(int _listLength, void *arena);
extern PFUDLLEXPORT pfType*              pfuGetAutoListClassType(void);
extern PFUDLLEXPORT int                  pfuGetNum(const pfuAutoList* _list);
extern PFUDLLEXPORT void                 pfuSet(pfuAutoList* _list, int index, void *elt);
extern PFUDLLEXPORT void*                pfuGet(const pfuAutoList* _list, int index);
extern PFUDLLEXPORT void                 pfuResetList(pfuAutoList* _list);
extern PFUDLLEXPORT void                 pfuAdd(pfuAutoList* _list, void *elt);
extern PFUDLLEXPORT void                 pfuInsert(pfuAutoList* _list, int index, void *elt);
extern PFUDLLEXPORT int                  pfuSearch(const pfuAutoList* _list, void *elt);
extern PFUDLLEXPORT int                  pfuSearchForType(const pfuAutoList* _list, pfType *type);
extern PFUDLLEXPORT int                  pfuRemove(pfuAutoList* _list, void *elt);
extern PFUDLLEXPORT void                 pfuRemoveIndex(pfuAutoList* _list, int index);
extern PFUDLLEXPORT int                  pfuMove(pfuAutoList* _list, int index, void *elt);
extern PFUDLLEXPORT int                  pfuReplace(pfuAutoList* _list, void *oldElt, void *newElt);
#endif /* !PF_C_API */


	/*---------------- Processor Control -----------------------------*/

extern PFUDLLEXPORT int pfuRunProcOn(int cpu);
extern PFUDLLEXPORT int pfuLockDownProc(int cpu);
extern PFUDLLEXPORT int pfuLockDownApp(void);
extern PFUDLLEXPORT int pfuLockDownCull(pfPipe *);
extern PFUDLLEXPORT int pfuLockDownDraw(pfPipe *);
extern PFUDLLEXPORT int pfuLockDownLPoint(pfPipe *);
extern PFUDLLEXPORT int pfuPrioritizeProcs(int onOff);
extern PFUDLLEXPORT int pfuRunDiskProcsOn(int cpu);

	/*----------------- Process Manager C-API ------------------------*/


#if PF_CPLUSPLUS_API
class pfuBaseProcessManager;
class pfuDefaultProcessManager;
#else
typedef struct _pfuBaseProcessManager pfuBaseProcessManager;
typedef struct _pfuDefaultProcessManager pfuDefaultProcessManager;
#endif /* PF_CPLUSPLUS_API */

/* ProcessManager config modes */
#define PFUPM_LOCK_DOWN  1
#define PFUPM_PRIORITIES 2
#define PFUPM_CONTINUOUS 3
#define PFUPM_DEFAULT    (-1)

#define PFUPM_NOPRI	(-1000)

typedef int (*pfuProcessHandlerFuncType)(int _pipe, uint _stage, pid_t _pid);

/* some useful defines which make implementing user handlers more portable */
#define PointerMotionMask (1L<<6)
#define ButtonPressMask   (1L<<2)
#define ButtonReleaseMask (1L<<3)
#define KeyPressMask      (1L<<0)
#define ExposureMask      (1L<<15)
#define StructureNotifyMask (1L<<17)
#define FocusChangeMask   (1L<<21)

extern PFUDLLEXPORT void pfuInitDefaultProcessManager(void);
extern PFUDLLEXPORT void pfuReconfigureProcessManager(void);
extern PFUDLLEXPORT void pfuReleaseProcessManager(void);
extern PFUDLLEXPORT void pfuProcessManagerMode(int mode, int val);
extern PFUDLLEXPORT int pfuGetProcessManagerMode(int mode);
extern PFUDLLEXPORT void pfuSelectProcessManager(pfuBaseProcessManager *);
extern PFUDLLEXPORT pfuBaseProcessManager *pfuGetCurProcessManager(void);
extern PFUDLLEXPORT void pfuProcessManagerCreateFunc(pfuProcessHandlerFuncType func, uint procMask);
extern PFUDLLEXPORT void pfuGetProcessManagerCreateFunc(pfuProcessHandlerFuncType *func, uint *procMask);
extern PFUDLLEXPORT void pfuProcessManagerPlaceFunc(pfuProcessHandlerFuncType func, uint procMask);
extern PFUDLLEXPORT void pfuGetProcessManagerPlaceFunc(pfuProcessHandlerFuncType *func, uint *procMask);
extern PFUDLLEXPORT void pfuProcessManagerPrioritizeFunc(pfuProcessHandlerFuncType func, uint procMask);
extern PFUDLLEXPORT void pfuGetProcessManagerPrioritizeFunc(pfuProcessHandlerFuncType *func, uint *procMask);

#define pfuFreeCPUs pfuFreeAllCPUs
extern PFUDLLEXPORT int pfuFreeAllCPUs(void);


	/*----------------- Clip Center Node C-API ------------------------*/

typedef int pfuClipCenterPostAppCallbackType(pfTraverser *_trav);

extern PFUDLLEXPORT pfuClipCenterNode *pfuNewClipCenterNode(void);
extern PFUDLLEXPORT pfType *pfuGetClipCenterNodeClassType(void);
extern PFUDLLEXPORT void pfuClipCenterNodeCallback(pfuClipCenterNode *_ccn, pfuClipCenterPostAppCallbackType *cb);
extern PFUDLLEXPORT pfuClipCenterPostAppCallbackType *pfuGetClipCenterNodeCallback(pfuClipCenterNode *_ccn);
extern PFUDLLEXPORT void pfuClipCenterNodeRefNode(pfuClipCenterNode *_ccn, pfNode *_node);
extern PFUDLLEXPORT pfNode *pfuGetClipCenterNodeRefNode(pfuClipCenterNode *_ccn);
extern PFUDLLEXPORT void pfuClipCenterNodeClipTexture(pfuClipCenterNode *_ccn, pfClipTexture *_ct);
extern PFUDLLEXPORT pfClipTexture *pfuGetClipCenterNodeClipTexture(pfuClipCenterNode *_ccn);
extern PFUDLLEXPORT void pfuClipCenterNodeMPClipTexture(pfuClipCenterNode *_ccn, pfMPClipTexture *_mpct);
extern PFUDLLEXPORT pfMPClipTexture *pfuGetClipCenterNodeMPClipTexture(pfuClipCenterNode *_ccn);
extern PFUDLLEXPORT pfChannel *pfuGetClipCenterNodeChannel(pfuClipCenterNode *_ccn);
extern PFUDLLEXPORT void pfuClipCenterNodeChannel(pfuClipCenterNode *_ccn, pfChannel *_chan);

    	/*---------------- SubClass TexGen Clip Center Node C-API --------*/
extern PFUDLLEXPORT pfuTexGenClipCenterNode *pfuNewTexGenClipCenterNode(void);
extern PFUDLLEXPORT void pfuTexGenClipCenterNodeTexGen(pfuTexGenClipCenterNode *_ccn, pfTexGen *_tgen);
extern PFUDLLEXPORT pfTexGen *pfuGetTexGenClipCenterNodeTexGen(pfuTexGenClipCenterNode *_ccn);

	/*------------------ Multiprocess Rendezvous ---------------------*/


#define PFURV_MAXSLAVES         2

#define PFURV_GARBAGE           -1
#define PFURV_READY             10
#define PFURV_SYNC              11
#define PFURV_SYNCACK           12
#define PFURV_RESUME            13

typedef struct _pfuRendezvous
{
    int master;
    int numSlaves;
    int slaves[PFURV_MAXSLAVES];

} pfuRendezvous;

extern PFUDLLEXPORT void pfuInitRendezvous(pfuRendezvous *rvous, int numSlaves);
extern PFUDLLEXPORT void pfuMasterRendezvous(pfuRendezvous *rvous);
extern PFUDLLEXPORT void pfuSlaveRendezvous(pfuRendezvous *rvous, int id);

	/*------------------ Spin-free Barrier ---------------------------*/

typedef struct _pfuBarrier pfuBarrier;
extern PFUDLLEXPORT pfuBarrier *pfuBarrierCreate(PF_USPTR_T *usptr);
extern PFUDLLEXPORT void pfuBarrierDestroy(pfuBarrier *b);
extern PFUDLLEXPORT void pfuBarrierEnter(pfuBarrier *b, unsigned int n);

	/*----------------------- GLX Mixed Mode -------------------------*/

/* these are defined here for building on 4.0.5 */
#ifndef GLX_MSSAMPLE
#define GLX_MSSAMPLE	0x000a
#define GLX_MSZSIZE	0x000b
#define GLX_MSSSIZE	0x000c
#define GLX_STEREOBUF	0x000d
#endif

  typedef HWND pfuXWindow;
  typedef void pfuXDisplay;
  typedef void Display;

typedef struct _pfuGLXWindow
{
    pfPipeWindow *pw;
    pfuXDisplay *dsp;
    pfuXWindow	xWin;
    pfuXWindow	glWin;
    pfuXWindow	overWin;
} pfuGLXWindow;

extern PFUDLLEXPORT pfuXDisplay * 	pfuOpenXDisplay(int _screen);
extern PFUDLLEXPORT pfuGLXWindow * 	pfuGLXWinopen(pfPipe *p, pfPipeWindow *_pw, const char *_name);
extern PFUDLLEXPORT void		pfuGetGLXWin(pfPipe *_pipe, pfuGLXWindow *_glxWin);
extern PFUDLLEXPORT const char *	pfuGetGLXDisplayString(pfPipe *_pipe);
extern PFUDLLEXPORT void		pfuGLMapcolors(pfVec3 *clrs, int start, int num);
extern PFUDLLEXPORT int		pfuGLXAllocColormap(pfuXDisplay *dsp, pfuXWindow w);
extern PFUDLLEXPORT void		pfuGLXMapcolors(pfuXDisplay *dsp, pfuXWindow w, pfVec3 *clrs, int loc, int num);
extern PFUDLLEXPORT void		pfuMapWinColors(pfWindow *w,  pfVec3 *clrs, int start, int num);
extern PFUDLLEXPORT void		pfuMapPWinColors(pfPipeWindow *pwin,  pfVec3 *clrs, int start, int num);
extern PFUDLLEXPORT void		pfuPrintWinFBConfig(pfWindow *win, FILE *_file);
extern PFUDLLEXPORT void		pfuPrintPWinFBConfig(pfPipeWindow *pwin, FILE *_file);
extern PFUDLLEXPORT pfFBConfig       pfuChooseFBConfig(Display *dsp,  int screen, int *constraints, void *arena);
extern PFUDLLEXPORT pfFBConfig       pfuChooseWGLFBConfig(GLint *iConstraints, int layer, void *arena);

	/*--------------- Input Handling ----------------*/

#define PFUMOUSE_IN_CHAN(m)	((m)->xchan>=-1.0f && (m)->xchan<=1.0f && \
				 (m)->ychan>=-1.0f && (m)->ychan<=1.0f)

#define PFUDEV_NULL		0

/*
 * Special performer device mappings
 * Function keys MUST be numbers 1-12
*/
#define PFUDEV_F1KEY		1
#define PFUDEV_F2KEY		2
#define PFUDEV_F3KEY		3
#define PFUDEV_F4KEY		4
#define PFUDEV_F5KEY		5
#define PFUDEV_F6KEY		6
#define PFUDEV_F7KEY		7
#define PFUDEV_F8KEY		8
#define PFUDEV_F9KEY		9
#define PFUDEV_F10KEY		10
#define PFUDEV_F11KEY		11
#define PFUDEV_F12KEY		12
#define PFUDEV_KEYBD		13
#define PFUDEV_REDRAW		14
#define PFUDEV_WINQUIT		15
/*
 * ESC is offered as a convenience for when using GL events.
 * It generally should be acquired through PFUDEV_KEYBD (== 27)
*/
#define PFUDEV_ESCKEY		16
#define PFUDEV_LEFTARROWKEY	17
#define PFUDEV_DOWNARROWKEY	18
#define PFUDEV_RIGHTARROWKEY	19
#define PFUDEV_UPARROWKEY	20

#define PFUDEV_PRINTSCREENKEY	21

#define PFUDEV_MAX		22

#define PFUDEV_MAX_DEVS		256

/* Masks for 'flags' in pfuMouse */
#define PFUDEV_MOUSE_RIGHT_DOWN	    0x0001
#define PFUDEV_MOUSE_MIDDLE_DOWN    0x0002
#define PFUDEV_MOUSE_LEFT_DOWN	    0x0004
#define PFUDEV_MOUSE_DOWN_MASK	    0x0007

/* Special masks for pfuEventStream 'buttonFlags' and pfuMouse flags */
#define PFUDEV_MOD_SHIFT	     0x0010
#define PFUDEV_MOD_LEFT_SHIFT	     0x0020
#define PFUDEV_MOD_LEFT_SHIFT_SET   (PFUDEV_MOD_LEFT_SHIFT | PFUDEV_MOD_SHIFT)
#define PFUDEV_MOD_RIGHT_SHIFT	     0x0040
#define PFUDEV_MOD_RIGHT_SHIFT_SET  (PFUDEV_MOD_RIGHT_SHIFT | PFUDEV_MOD_SHIFT)
#define PFUDEV_MOD_SHIFT_MASK	     0x0070
#define PFUDEV_MOD_CTRL		     0x0080
#define PFUDEV_MOD_LEFT_CTRL	     0x0100
#define PFUDEV_MOD_LEFT_CTRL_SET    (PFUDEV_MOD_LEFT_CTRL | PFUDEV_MOD_CTRL)
#define PFUDEV_MOD_RIGHT_CTRL	     0x0200
#define PFUDEV_MOD_RIGHT_CTRL_SET   (PFUDEV_MOD_RIGHT_CTRL | PFUDEV_MOD_CTRL)
#define PFUDEV_MOD_CAPS_LOCK	    (0x0400 | PFUDEV_MOD_CTRL)
#define PFUDEV_MOD_CTRL_MASK	     0x0780
#define PFUDEV_MOD_ALT		     0x0800
#define PFUDEV_MOD_LEFT_ALT	     0x1000
#define PFUDEV_MOD_LEFT_ALT_SET	    (PFUDEV_MOD_LEFT_ALT | PFUDEV_MOD_ALT)
#define PFUDEV_MOD_RIGHT_ALT	     0x2000
#define PFUDEV_MOD_RIGHT_ALT_SET    (PFUDEV_MOD_RIGHT_ALT | PFUDEV_MOD_ALT)
#define PFUDEV_MOD_ALT_MASK	     0x3800
#define PFUDEV_MOD_MASK		     0x3ff0

#define IS_FUNCKEY(x) (((x) >= F1KEY) && ((x) <= F12KEY))
#define FUNCKEY_TO_PFUDEV(x)  (PFUDEV_F1KEY + ((x) - F1KEY))

#define PFUDEV_KEY_MAP_SIZE	128
#define PFUDEV_MAX_INKEYS	256
#define PFUDEV_MAX_INDEVS	256

typedef struct _pfuMouse
{
    int		flags;		/* for PDEV_MOUSE_*_DOWN and PFUDEV_MOD_* bitmasks */
    int		modifiers;	/* modifier keys only  */

    int 	xpos, ypos;	/* Screen coordinates of mouse */
    float	xchan, ychan;	/* Normalized coordinates of mouse */
    double	posTime;	/* msec timestamp on current mouse position */


    /* These are used by the GUI and pfiXformer
     * GUI needs Last click positional info
     * Xformers need last and middle click and release info
     */
		    /* Mask of clicks seen last frame */
    int		click;		
		    /* Last click position for each mouse button */      
    int 	clickPos[PFUDEV_MOUSE_DOWN_MASK][2];    
		    /* Screen coordinates where a mouse button was last clicked */
    int		clickPosLast[2];
		    /* mask of mouse releases seen last frame */
    int		release;
		    /* last release position for each mouse button */      
    int 	releasePos[PFUDEV_MOUSE_DOWN_MASK][2]; 
		    /* Screen coordinates where a mouse button was last released*/   
    int		releasePosLast[2];
		    /* Last click time for each mouse button */
    double	clickTime[PFUDEV_MOUSE_DOWN_MASK];
		    /* Time of last button click */
    double	clickTimeLast;
		    /* Last release time for each mouse button */
    double	releaseTime[PFUDEV_MOUSE_DOWN_MASK];
		    /* Time of last button reelase */
    double	releaseTimeLast;
		    /* Window Size */
    int      	winSizeX;	
    int      	winSizeY;
		    /* Window focus flag */
    ulong	inWin;		

} pfuMouse;

typedef struct _pfuEventQueue pfuEventQueue;

typedef struct _pfuEventStream
{
    int		buttonFlags;		/* For polled buttons */
    int		frameStamp;		/* frame stamp of events */
    int 	numDevs;		/* Length of dev queue */
    int 	numKeys;		/* Length of key-push queue */
    int		devQ[PFUDEV_MAX_INDEVS];	/* Queue of devices */
    int		devCount[PFUDEV_MAX_DEVS];	/* Counts for queued devices */
    int		devVal[PFUDEV_MAX_INDEVS];	/* Values of device input */
    int		keyQ[PFUDEV_MAX_INKEYS];	/* Queue of key pushes */
    int 	keyCount[PFUDEV_KEY_MAP_SIZE];	/* Counts for ascii keys */		/* num of repeats to record */
} pfuEventStream;

typedef struct _pfuEvent
{
    int dev;			/* dev to place in devQ - 0 means NO device returned */
    int val;			/* val of dev */
    int count;			/* num of repeats to record */
} pfuEvent;

extern PFUDLLEXPORT pfuEventQueue *pfuNewEventQ(pfDataPool *dp, int id);
extern PFUDLLEXPORT void pfuResetEventStream(pfuEventStream *es);
extern PFUDLLEXPORT void pfuResetEventQ(pfuEventQueue *eq);
extern PFUDLLEXPORT void pfuAppendEventQ(pfuEventQueue *eq0, pfuEventQueue *eq1);
extern PFUDLLEXPORT void pfuAppendEventQStream(pfuEventQueue *eq, pfuEventStream *es);
extern PFUDLLEXPORT void pfuEventQStream(pfuEventQueue *eq, pfuEventStream *es);
extern PFUDLLEXPORT pfuEventStream *pfuGetEventQStream(pfuEventQueue *eq);
extern PFUDLLEXPORT void pfuGetEventQEvents(pfuEventStream *events, pfuEventQueue *eq);
extern PFUDLLEXPORT void pfuIncEventQFrame(pfuEventQueue *eq);
extern PFUDLLEXPORT void pfuEventQFrame(pfuEventQueue *eq, int val);
extern PFUDLLEXPORT int pfuGetEventQFrame(pfuEventQueue *eq);
extern PFUDLLEXPORT void pfuIncEventStreamFrame(pfuEventStream *es);
extern PFUDLLEXPORT void pfuEventStreamFrame(pfuEventStream *es, int val);
extern PFUDLLEXPORT int pfuGetEventStreamFrame(pfuEventStream *es);

typedef struct _pfuCustomEvent
{
    int dev;			/* dev to place in devQ - 0 means NO device returned */
    int val;			/* val of dev */
} pfuCustomEvent;

typedef void (*pfuEventHandlerFuncType)(int _dev, void* _val, pfuCustomEvent *_pfuevent);

/* pfuInitInput() types */
#define PFUINPUT_X		0
#define PFUINPUT_GL		1  /* for IRIS GL input handling */
#define PFUINPUT_NOFORK_X	2  /* for debugging X input in application process */

/* pfuInputHandler() handler mode */
#define PFUINPUT_CATCH_UNKNOWN	0
#define PFUINPUT_CATCH_SIM	1 /* catch all non-GUI events */
#define PFUINPUT_CATCH_ALL	(~((unsigned int )0))

/*** routines to do auto-event collection for you ***/
extern PFUDLLEXPORT void pfuInitInput(pfPipeWindow *_pw, int _mode);
extern PFUDLLEXPORT void pfuInitMultiChanInput(pfChannel **_chanArray, int _numChans, int _mode);
extern PFUDLLEXPORT void pfuExitInput(void);
extern PFUDLLEXPORT int pfuMapMouseToChan(pfuMouse *mouse, pfChannel *chan);
extern PFUDLLEXPORT int pfuMouseInChan(pfuMouse *mouse, pfChannel *chan);
/* IRIS GL - trigger auto-event collection */
extern PFUDLLEXPORT void pfuCollectInput(void);
/* Explicit event collection for when not using pfuInitInput utilities */
extern PFUDLLEXPORT void pfuCollectGLEventStream(pfuEventStream *events, pfuMouse *mouse, int handlerMask, pfuEventHandlerFuncType handlerFunc);
extern PFUDLLEXPORT void	pfuCollectXEventStream(pfWSConnection dsp, pfuEventStream *events, pfuMouse *mouse, int handlerMask, pfuEventHandlerFuncType handlerFunc);

/* get back the auto-collected events */
extern PFUDLLEXPORT void	pfuGetMouse(pfuMouse *_mouse);
extern PFUDLLEXPORT void	pfuGetEvents(pfuEventStream *_events);

/* this routine lets you get/record events yourself */
extern PFUDLLEXPORT void	pfuInputHandler(pfuEventHandlerFuncType _userFunc, unsigned int mask);
extern PFUDLLEXPORT void	pfuMouseButtonClick(pfuMouse *mouse, 
		    int _button, int _x, int _y, double _time);
extern PFUDLLEXPORT void	pfuMouseButtonRelease(pfuMouse *mouse, 
		    int _button, int _x, int _y, double _time);
/* map X time stamps to pfGetTime() relative values */
extern PFUDLLEXPORT double pfuMapXTime(double xtime);
		    
/*------------------------- Cursor Control ---------------------------*/

/*
 * Cursor utilities are in cursor.c and require GLX
 */
#define Cursor void* 

/* PFU Cursor targets */
#define PFU_CURSOR_MAIN  1 /* cursor to be used when not in GUI */
#define PFU_CURSOR_GUI   2 /* cursor to be used in GUI */

/* Cursor values - taken from /usr/include/X11/cursorfont.h */
#define PFU_CURSOR_circle	24
#define PFU_CURSOR_hand1	58

/* #define XC_num_glyphs 154 */
#define PFU_NUM_CUSORS 256
#define PFU_CURSOR_OFF 255
#define PFU_CURSOR_arrow 2
#define PFU_CURSOR_based_arrow_down 4
#define PFU_CURSOR_based_arrow_up 6
#define PFU_CURSOR_boat 8
#define PFU_CURSOR_bogosity 10
#define PFU_CURSOR_bottom_left_corner 12
#define PFU_CURSOR_bottom_right_corner 14
#define PFU_CURSOR_bottom_side 16
#define PFU_CURSOR_bottom_tee 18
#define PFU_CURSOR_box_spiral 20
#define PFU_CURSOR_center_ptr 22
#define PFU_CURSOR_clock 26
#define PFU_CURSOR_coffee_mug 28
#define PFU_CURSOR_cross 30
#define PFU_CURSOR_cross_reverse 32
#define PFU_CURSOR_crosshair 34
#define PFU_CURSOR_diamond_cross 36
#define PFU_CURSOR_dot 38
#define PFU_CURSOR_dotbox 40
#define PFU_CURSOR_double_arrow 42
#define PFU_CURSOR_draft_large 44
#define PFU_CURSOR_draft_small 46
#define PFU_CURSOR_draped_box 48
#define PFU_CURSOR_exchange 50
#define PFU_CURSOR_fleur 52
#define PFU_CURSOR_gobbler 54
#define PFU_CURSOR_gumby 56
#define PFU_CURSOR_hand2 60
#define PFU_CURSOR_heart 62
#define PFU_CURSOR_icon 64
#define PFU_CURSOR_iron_cross 66
#define PFU_CURSOR_left_ptr 68
#define PFU_CURSOR_left_side 70
#define PFU_CURSOR_left_tee 72
#define PFU_CURSOR_leftbutton 74
#define PFU_CURSOR_ll_angle 76
#define PFU_CURSOR_lr_angle 78
#define PFU_CURSOR_man 80
#define PFU_CURSOR_middlebutton 82
#define PFU_CURSOR_mouse 84
#define PFU_CURSOR_pencil 86
#define PFU_CURSOR_pirate 88
#define PFU_CURSOR_plus 90
#define PFU_CURSOR_question_arrow 92
#define PFU_CURSOR_right_ptr 94
#define PFU_CURSOR_right_side 96
#define PFU_CURSOR_right_tee 98
#define PFU_CURSOR_rightbutton 100
#define PFU_CURSOR_rtl_logo 102
#define PFU_CURSOR_sailboat 104
#define PFU_CURSOR_sb_down_arrow 106
#define PFU_CURSOR_sb_h_double_arrow 108
#define PFU_CURSOR_sb_left_arrow 110
#define PFU_CURSOR_sb_right_arrow 112
#define PFU_CURSOR_sb_up_arrow 114
#define PFU_CURSOR_sb_v_double_arrow 116
#define PFU_CURSOR_shuttle 118
#define PFU_CURSOR_sizing 120
#define PFU_CURSOR_spider 122
#define PFU_CURSOR_spraycan 124
#define PFU_CURSOR_star 126
#define PFU_CURSOR_target 128
#define PFU_CURSOR_tcross 130
#define PFU_CURSOR_top_left_arrow 132
#define PFU_CURSOR_top_left_corner 134
#define PFU_CURSOR_top_right_corner 136
#define PFU_CURSOR_top_side 138
#define PFU_CURSOR_top_tee 140
#define PFU_CURSOR_trek 142
#define PFU_CURSOR_ul_angle 144
#define PFU_CURSOR_umbrella 146
#define PFU_CURSOR_ur_angle 148
#define PFU_CURSOR_watch 150
#define PFU_CURSOR_xterm 152

#define PFU_CURSOR_MAIN_DEFAULT PFU_CURSOR_circle
#define PFU_CURSOR_GUI_DEFAULT	PFU_CURSOR_hand2
#define PFU_CURSOR_DEFAULT	(-1)

/* pfuCursorType */
#define PFU_CURSOR_X	0
#define PFU_CURSOR_2D	1


extern PFUDLLEXPORT Cursor	pfuGetInvisibleCursor(void);
extern PFUDLLEXPORT void	pfuLoadPWinCursor(pfPipeWindow *_w, int _c);
extern PFUDLLEXPORT void 	pfuLoadWinCursor(pfWindow *_w, int _c);
extern PFUDLLEXPORT void	pfuLoadChanCursor(pfChannel *_chan, int _c);
extern PFUDLLEXPORT Cursor	pfuCreateCursor(Cursor index);

extern PFUDLLEXPORT void	pfuDraw2DCursor(int mx, int my);
extern PFUDLLEXPORT void	pfuDrawPWin2DCursor(pfPipeWindow *pw, int mx, int my);
extern PFUDLLEXPORT void	pfuDrawVChan2DCursor(pfVideoChannel *vChan, int mx, int my);
extern PFUDLLEXPORT void	pfuDrawPVChan2DCursor(pfPipeVideoChannel *vChan, int mx, int my);
extern PFUDLLEXPORT void	pfuDrawChan2DCursor(pfChannel *chan, int mx, int my);
extern PFUDLLEXPORT void	pfuCursorColor(pfVec3 fg, pfVec3 bg);

extern PFUDLLEXPORT void	pfuCursorType(int val);
extern PFUDLLEXPORT int	pfuGetCursorType(void);
extern PFUDLLEXPORT void	pfuSelCursor(int c);
extern PFUDLLEXPORT int	pfuGetCursorType(void);
extern PFUDLLEXPORT void	pfuCursor(Cursor c, int index);
extern PFUDLLEXPORT Cursor	pfuGetCursor(int index);

extern PFUDLLEXPORT void	pfuCursor(Cursor c, int index);
extern PFUDLLEXPORT Cursor	pfuGetCursor(int index);

/**** GUI maintains a cursor - in gui.c ****/

/* Cursor sel constants */
#define PFU_CURSOR_IN_MAIN 	0
#define PFU_CURSOR_IN_GUI 	1

extern PFUDLLEXPORT void	pfuInitGUICursors(void);

extern PFUDLLEXPORT void	pfuGUICursor(int target,  int c);
extern PFUDLLEXPORT Cursor	pfuGetGUICursor(int target);

extern PFUDLLEXPORT void	pfuGUICursorSel(int sel);
extern PFUDLLEXPORT Cursor	pfuGetGUICursorSel(void);

extern PFUDLLEXPORT void	pfuUpdateGUICursor(void);

/**** DVR utilities in gui.c ****/

extern PFUDLLEXPORT void     pfuDrawChanDVRBox(pfChannel *chan);

/*-------------------- OpenGL X Fonts -----------------------*/

typedef struct pfuXFont
{
    int   size;
    int    handle;
  HFONT info;
} pfuXFont;

extern PFUDLLEXPORT void pfuLoadXFont(char *fontName, pfuXFont *fnt);
extern PFUDLLEXPORT void pfuLoadWINFont(LOGFONT *lf, pfuXFont *fnt);
extern PFUDLLEXPORT void pfuMakeXFontBitmaps(pfuXFont *fnt);
extern PFUDLLEXPORT void pfuMakeRasterXFont(char *fontName, pfuXFont *font);
extern PFUDLLEXPORT void pfuSetXFont(pfuXFont *);
extern PFUDLLEXPORT void pfuGetCurXFont(pfuXFont *);
extern PFUDLLEXPORT int pfuGetXFontWidth(pfuXFont *, const char *);
extern PFUDLLEXPORT int pfuGetXFontHeight(pfuXFont *);
extern PFUDLLEXPORT void pfuCharPos(float x, float y, float z);
extern PFUDLLEXPORT void pfuDrawString(const char *s);
extern PFUDLLEXPORT void pfuDrawStringPos(const char *s,float x, float y, float z);

	/*------------------------- Simple GUI ---------------------------*/

#define PFUGUI_MAXPANELS	64
#define PFUGUI_MAXWIDGETS	512

#define PFUGUI_INT		1
#define PFUGUI_FLOAT		2

/* main GUI Widget types */
#define PFUGUI_UNDEFINED	0x00000000
#define PFUGUI_BUTTON		0x00000010
#define PFUGUI_SWITCH		0x00000011
#define PFUGUI_SLIDER		0x00000020
#define PFUGUI_MESSAGE		0x00000040
#define PFUGUI_ATTR		0x00000080
#define PFUGUI_RADIO_BUTTON	0x00000100
#define PFUGUI_MENU_BUTTON	0x00000200
#define PFUGUI_CUSTOM_WIDGET	0x80000000
#define	PFUGUI_TYPE_MASK	0x003ff

/* sub-type selects */
#define PFUGUI_SLIDER_LOG	    (0x10000 | PFUGUI_SLIDER)
#define PFUGUI_SLIDER_FUNC	    (0x20000 | PFUGUI_SLIDER)
#define PFUGUI_RADIO_BUTTON_TOGGLE  (0x10000 | PFUGUI_RADIO_BUTTON)
#define PFUGUI_RADIO_BUTTON_ATTR    (PFUGUI_RADIO_BUTTON | PFUGUI_ATTR)
#define PFUGUI_MENU_BUTTON_ATTR	    (PFUGUI_MENU_BUTTON | PFUGUI_ATTR)
#define PFUGUI_TYPE_VALS	    0xfffff

/* extra type modifiers */
#define PFUGUI_HORIZONTAL	0x0
#define PFUGUI_VERTICAL		0x8000000

/* Widget Modes */
#define PFUGUI_STICKY_DEFAULT   1

#define PFUGUI_SCALE_XSIZE(x)	((x))
#define PFUGUI_SCALE_YSIZE(y)	((y))

#define PFUGUI_PANEL_BORDER	4
#define PFUGUI_PANEL_FRAME	(2*PFUGUI_PANEL_BORDER)

#define PFUGUI_MESSAGE_XSIZE	(400)
#define PFUGUI_MESSAGE_YSIZE	(24)
#define PFUGUI_MESSAGE_XINC	(PFUGUI_MESSAGE_XSIZE + PFUGUI_PANEL_FRAME)
#define PFUGUI_MESSAGE_YINC	(PFUGUI_MESSAGE_YSIZE + PFUGUI_PANEL_FRAME)

#define PFUGUI_BUTTON_XSIZE	(70)
#define PFUGUI_BUTTON_YSIZE	(24)
#define PFUGUI_BUTTON_XINC		(PFUGUI_BUTTON_XSIZE + PFUGUI_PANEL_FRAME)
#define PFUGUI_BUTTON_YINC		(PFUGUI_BUTTON_YSIZE + PFUGUI_PANEL_FRAME)

#define PFUGUI_BUTTON_SHORT_XSIZE	(50)
#define PFUGUI_BUTTON_SHORT_XINC	(PFUGUI_BUTTON_SHORT_XSIZE + PFUGUI_PANEL_FRAME)
#define PFUGUI_BUTTON_MED_XSIZE		(80)
#define PFUGUI_BUTTON_MED_XINC		(PFUGUI_BUTTON_MED_XSIZE + PFUGUI_PANEL_FRAME)
#define PFUGUI_BUTTON_LONG_XSIZE	(100)
#define PFUGUI_BUTTON_LONG_XINC		(PFUGUI_BUTTON_LONG_XSIZE + PFUGUI_PANEL_FRAME)
#define PFUGUI_BUTTON_VLONG_XSIZE	(130)
#define PFUGUI_BUTTON_VLONG_XINC	(PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_PANEL_FRAME)
#define PFUGUI_BUTTON_SLONG_XSIZE	(180)
#define PFUGUI_BUTTON_SLONG_XINC    	(PFUGUI_BUTTON_SLONG_XSIZE + PFUGUI_PANEL_FRAME)

#define PFUGUI_SLIDER_XSIZE	    (310)
#define PFUGUI_SLIDER_TINY_XSIZE    (220)
#define PFUGUI_SLIDER_SMALL_XSIZE   (240)
#define PFUGUI_SLIDER_BIG_XSIZE	    (380)
#define PFUGUI_SLIDER_YSIZE	    (40)
#define PFUGUI_SLIDER_XINC	    (PFUGUI_SLIDER_XSIZE + PFUGUI_PANEL_FRAME)
#define PFUGUI_SLIDER_TINY_XINC	    (PFUGUI_SLIDER_TINY_XSIZE + PFUGUI_PANEL_FRAME)
#define PFUGUI_SLIDER_SMALL_XINC    (PFUGUI_SLIDER_SMALL_XSIZE + PFUGUI_PANEL_FRAME)
#define PFUGUI_SLIDER_BIG_XINC	    (PFUGUI_SLIDER_BIG_XSIZE + PFUGUI_PANEL_FRAME)
#define PFUGUI_SLIDER_YINC	    (PFUGUI_SLIDER_YSIZE + PFUGUI_PANEL_FRAME)

#define PFUGUI_RADIO_BUTTON_XSIZE   PFUGUI_BUTTON_XSIZE
#define PFUGUI_RADIO_BUTTON_YSIZE   PFUGUI_BUTTON_YSIZE
#define PFUGUI_RADIO_BUTTON_YINC    PFUGUI_RADIO_BUTTON_YSIZE

/* GUI Widget Draw Styles  */
#define PFUGUI_OFF		0
#define PFUGUI_ON		1
#define PFUGUI_HILIGHT		2
#define PFUGUI_NORMAL		3

#define PFUGUI_BUTTON_STRING_SIZE 40

/* 1.2 compatability defines */
#define pfuWidgetFunc(widget, func)	pfuWidgetActionFunc(widget, func)

typedef char 			pfuGUIString[PF_MAXSTRING];
typedef struct _pfuWidget 	pfuWidget;
typedef struct _pfuPanel 	pfuPanel;

typedef void (*pfuWidgetDrawFuncType)(pfuWidget *_widget, pfuPanel *_panel);
typedef pfuWidget* (*pfuWidgetSelectFuncType)(pfuWidget *_widget, pfuPanel *_panel);
typedef void (*pfuWidgetActionFuncType)(pfuWidget *_widget);

extern PFUDLLEXPORT void	pfuInitGUI(pfPipeWindow *_pw);
extern PFUDLLEXPORT void	pfuExitGUI(void);
extern PFUDLLEXPORT void 	pfuEnableGUI(int _en);
extern PFUDLLEXPORT void	pfuUpdateGUI(pfuMouse *mouse);
extern PFUDLLEXPORT pfChannel *pfuGetGUIChan(void);
extern PFUDLLEXPORT void	pfuRedrawGUI(void);
extern PFUDLLEXPORT void	pfuGUIViewport(float _l, float _r, float _b, float _t);
extern PFUDLLEXPORT void	pfuGetGUIViewport(float *_l, float *_r, float *_b, float *_t);
extern PFUDLLEXPORT int	pfuInGUI(int _x, int _y);
extern PFUDLLEXPORT void	pfuFitWidgets(int _val);
extern PFUDLLEXPORT void	pfuGetGUIScale(float *_x, float *_y);
extern PFUDLLEXPORT void	pfuGetGUITranslation(float *_x, float *_y);
extern PFUDLLEXPORT void 	pfuGUIHlight(pfHighlight *hlight);
extern PFUDLLEXPORT pfHighlight * pfuGetGUIHlight(void);

extern PFUDLLEXPORT pfuPanel* pfuNewPanel(void);
extern PFUDLLEXPORT void	pfuEnablePanel(pfuPanel *_p);
extern PFUDLLEXPORT void	pfuDisablePanel(pfuPanel *_p);
extern PFUDLLEXPORT void 	pfuGetPanelOriginSize(pfuPanel *_p, float *_xo, float *_yo, float *_xs, float *_ys);

extern PFUDLLEXPORT pfuWidget *pfuNewWidget(pfuPanel *_p, int _type, int _id);
extern PFUDLLEXPORT int	pfuGetWidgetType(pfuWidget *_w);
extern PFUDLLEXPORT void	pfuEnableWidget(pfuWidget *_w);
extern PFUDLLEXPORT void	pfuDisableWidget(pfuWidget *_w);
extern PFUDLLEXPORT void	pfuHideWidget(pfuWidget *_w);
extern PFUDLLEXPORT void	pfuUnhideWidget(pfuWidget *_w);
extern PFUDLLEXPORT int	pfuGetWidgetId(pfuWidget *_w);
extern PFUDLLEXPORT void 	pfuWidgetDim(pfuWidget *_w, int _xo, int _yo, int _xs, int _ys);
extern PFUDLLEXPORT void	pfuGetWidgetDim(pfuWidget *_w, int *_xo, int *_yo, int *_xs, int *_ys);
extern PFUDLLEXPORT void	pfuWidgetLabel(pfuWidget *_w, const char *_label);
extern PFUDLLEXPORT int	pfuGetWidgetLabelWidth(pfuWidget *_w);
extern PFUDLLEXPORT const char *pfuGetWidgetLabel(pfuWidget *_w);
extern PFUDLLEXPORT void     pfuWidgetRange(pfuWidget *_w, int _mode, float _min, float _max, float _val);
extern PFUDLLEXPORT void	pfuWidgetValue(pfuWidget *_w, float _val);
extern PFUDLLEXPORT float	pfuGetWidgetValue(pfuWidget *_w);
extern PFUDLLEXPORT void	pfuWidgetDefaultValue(pfuWidget *_w, float _val);
extern PFUDLLEXPORT void 	pfuWidgetDrawFunc(pfuWidget *w, pfuWidgetDrawFuncType func);
extern PFUDLLEXPORT void 	pfuWidgetSelectFunc(pfuWidget *w, pfuWidgetSelectFuncType func);
extern PFUDLLEXPORT void 	pfuWidgetActionFunc(pfuWidget *w, pfuWidgetActionFuncType func);
extern PFUDLLEXPORT pfuWidgetActionFuncType 	pfuGetWidgetActionFunc(pfuWidget *w);
extern PFUDLLEXPORT pfuWidgetSelectFuncType 	pfuGetWidgetSelectFunc(pfuWidget *w);
extern PFUDLLEXPORT pfuWidgetDrawFuncType 	pfuGetWidgetDrawFunc(pfuWidget *w);
extern PFUDLLEXPORT void	pfuWidgetValueScaleFunc(pfuWidget * w, float (*fun)(float, void*), float (*invfun)(float, void*), void *arg);

extern PFUDLLEXPORT void	pfuWidgetSelections(pfuWidget *_w, pfuGUIString *_attrList, int *_valList, void (**funcList)(pfuWidget *_w), int _numSelections);
extern PFUDLLEXPORT void	pfuWidgetSelection(pfuWidget *w, int _index);
extern PFUDLLEXPORT int	pfuGetWidgetSelection(pfuWidget *w);
extern PFUDLLEXPORT void	pfuWidgetDefaultSelection(pfuWidget *w, int _index);

extern PFUDLLEXPORT void	pfuWidgetDefaultOnOff(pfuWidget * w, int _on);
extern PFUDLLEXPORT void	pfuWidgetOnOff(pfuWidget *_w, int _on);
extern PFUDLLEXPORT int	pfuIsWidgetOn(pfuWidget *_w);
extern PFUDLLEXPORT void	pfuWidgetMode(pfuWidget * w, int mode, int val);
extern PFUDLLEXPORT int	pfuGetWidgetMode(pfuWidget * w, int mode);

extern PFUDLLEXPORT void	pfuResetGUI(void);
extern PFUDLLEXPORT void	pfuResetPanel(pfuPanel *_p);
extern PFUDLLEXPORT void	pfuResetWidget(pfuWidget *_w);

extern PFUDLLEXPORT void	pfuDrawTree(pfChannel *_chan, pfNode *_node, PFVEC3 panXYScale);

/* tokens for pfuDrawMessage: _rel - for chan-relative or pipe-relative */
#define PFU_MSG_CHAN		0
#define PFU_MSG_PIPE		1
/* tokens for pfuDrawMessage: _just */
#define PFU_LEFT_JUSTIFIED	0
#define PFU_CENTER_JUSTIFIED	1
#define PFU_RIGHT_JUSTIFIED	2
/* tokens for pfuDrawMessage: _size */
#define PFU_FONT_TINY		1
#define PFU_FONT_SMALL		1
#define PFU_FONT_MED		2
#define PFU_FONT_BIG		3
#define PFU_FONT_GIANT		3
#define PFU_NUM_FONTS 		4
/* tokens for pfuDrawMessage: _cmode */
#define PFU_RGB			0
#define PFU_CI			1

extern PFUDLLEXPORT void	pfuDrawMessage(pfChannel *_chan, const char *_msg, int _rel, int _just, float _x, float _y, int _size, int _cmode);

extern PFUDLLEXPORT void	pfuDrawMessageCI(pfChannel *_chan, const char *_msg, int _rel, int _just, float _x, float _y, int _size, int _textClr, int _shadowClr);

extern PFUDLLEXPORT void	pfuDrawMessageRGB(pfChannel *_chan, const char *_msg, int _rel, int _just, float _x, float _y, int _size, PFVEC4 _textClr, PFVEC4 _shadowClr);

	/*----------------- Scene Graph Traversal ------------------------*/

typedef struct _pfuTraverser	pfuTraverser;
typedef int (*pfuTravFuncType)(pfuTraverser *trav);

/* modes for traversals */
#define PFUTRAV_SW_CUR		0x0000
#define PFUTRAV_SW_ALL		0x0001
#define PFUTRAV_SW_NONE		0x0002
#define PFUTRAV_SW_MASK     	0x0003

#define PFUTRAV_SEQ_CUR		0x0000
#define PFUTRAV_SEQ_ALL		0x0010
#define PFUTRAV_SEQ_NONE     	0x0020
#define PFUTRAV_SEQ_MASK     	0x0030

#define PFUTRAV_LOD_RANGE0	0x0000
#define PFUTRAV_LOD_ALL		0x0100
#define PFUTRAV_LOD_NONE	0x0200
#define PFUTRAV_LOD_MASK     	0x0300

#define PFUTRAV_LAYER_BOTH	0x0000
#define PFUTRAV_LAYER_NONE	0x1000
#define PFUTRAV_LAYER_BASE	0x2000
#define PFUTRAV_LAYER_DECAL	0x3000
#define PFUTRAV_LAYER_MASK     	0x3000


/* bitmask selecting attrs for pfGeoSet operations */
#define PFU_ATTRS_NONE		0x0
#define PFU_ATTRS_COORDS	0x1
#define PFU_ATTRS_TEXCOORDS	0x2
#define PFU_ATTRS_NORMALS	0x4
#define PFU_ATTRS_COLORS	0x8
#define PFU_ATTRS_CN		(PFU_ATTRS_COLORS | PFU_ATTRS_NORMALS)
#define PFU_ATTRS_CT		(PFU_ATTRS_COLORS | PFU_ATTRS_TEXCOORDS)
#define PFU_ATTRS_ALL		0xf

/* algorithms for pfuMipMinLOD */
#define PFUMINMIPLOD_FRUSTUM_INTERSECT_SPHERE 0
#define PFUMINMIPLOD_CONE_INTERSECT_SPHERE 1
#define PFUMINMIPLOD_ARBITRARYROTATIONCONE_INTERSECT_SPHERE 2


struct _pfuTraverser
{
    pfuTravFuncType	preFunc, postFunc;
    int			mode;
    int			depth;
    pfNode		*node;
    pfMatStack		*mstack;
    void		*data;

};

extern PFUDLLEXPORT int pfuTravCountNumVerts(pfNode *node);
extern PFUDLLEXPORT int pfuTraverse(pfNode *_node, pfuTraverser *_trav);
extern PFUDLLEXPORT void pfuInitTraverser(pfuTraverser *_trav);
extern PFUDLLEXPORT void pfuTravCalcBBox(pfNode *node, pfBox *box);
extern PFUDLLEXPORT void pfuTravCountDB(pfNode *_node, pfFrameStats *_fstats);
extern PFUDLLEXPORT void pfuTravGLProf(pfNode *_node, int _mode);
extern PFUDLLEXPORT void pfuTravNodeAttrBind(pfNode *_node, unsigned int _attr, unsigned int _bind);
extern PFUDLLEXPORT void pfuTravNodeHlight(pfNode *_node, pfHighlight *_hl);
extern PFUDLLEXPORT void pfuTravPrintNodes(pfNode *_node, const char *_fname);
extern PFUDLLEXPORT int pfuFillGSetPackedAttrs(pfGeoSet *gset, int mask);
extern PFUDLLEXPORT void pfuDelGSetAttrs(pfGeoSet *gset, int delMask);
extern PFUDLLEXPORT void pfuTravCreatePackedAttrs(pfNode *_node, int format, int delMask);
extern PFUDLLEXPORT void pfuMeshToQuad(pfGeoSet *gset);
extern PFUDLLEXPORT void pfuMeshToTri(pfGeoSet *gset);
extern PFUDLLEXPORT void pfuTravMeshToIndep(pfNode *node);
extern PFUDLLEXPORT void pfuTravSetDListMode(pfNode *node, int enable);
extern PFUDLLEXPORT void pfuTravCompileDLists(pfNode *node, int delMask);
extern PFUDLLEXPORT void pfuTravDrawDLists(pfNode *node);
extern PFUDLLEXPORT int pfuCalcDepth(pfNode *node);
extern PFUDLLEXPORT pfNode *pfuLowestCommonAncestor(pfNode *scene, int (*fun)(pfNode*,void*), void *arg, int mode);
extern PFUDLLEXPORT pfNode *pfuLowestCommonAncestorOfGeoSets(pfNode *scene, int (*fun)(pfGeoSet*,void*), void *arg, int mode);
extern PFUDLLEXPORT pfTexture *pfuFindTexture(pfNode *scene, int i, int (*fun)(pfGeoState*,pfTexture*,void*), void *arg, int mode);
extern PFUDLLEXPORT void pfuFindClipTextures(pfList *_tex, pfNode *_scene, uint _promote);
extern PFUDLLEXPORT int pfuGetClosestPoint(pfNode *node, float eyex, float eyey, float eyez, const pfTexture *tex, int travmode, float *x, float *y, float *z, float *s, float *t, float *r);
extern PFUDLLEXPORT int pfuGetNearestVisiblePointToEyePlane(pfNode *node, pfFrustum *frust, const pfTexture *tex, int travmode, float *x, float *y, float *z, float *s, float *t, float *r);
extern PFUDLLEXPORT int pfuGetClosestPointOnTriangle(PFVEC3 p, PFVEC3 v0, PFVEC3 v1, PFVEC3 v2, float *closest_distsqrd, float *W0, float *W1, float *W2);

/* pfASD node collector */
extern PFUDLLEXPORT void pfuFindASDs(pfNode *_node, pfList *_ASDs);

/* clipcenter functionality */

extern PFUDLLEXPORT void pfuProcessClipCenters(pfNode *_node, pfList *_mpcliptextures);
extern PFUDLLEXPORT void pfuProcessClipCentersWithChannel(pfNode *_node, pfList *_mpcliptextures, pfChannel *chan);
extern PFUDLLEXPORT pfNode *pfuNewSphereFrontOld(int n, pfTexture *texture, pfVec2 lonlat[2], pfVec2 st[2], int update_frequency);
extern PFUDLLEXPORT pfNode *pfuNewSphereFront(float hightol, float lowtol, pfTexture *texture, pfVec2 lonlat[2], pfVec2 st[2], int update_frequency);


extern PFUDLLEXPORT int pfuGridifySaveSize(int width, int height, int pixeldepth, int borderwidth);
extern PFUDLLEXPORT void pfuGridifyMem(unsigned char *buf, int width, int height, int pixeldepth, int borderwidth, const unsigned char color[/* pixeldepth */], unsigned char savecolors[/* pfuGridifySaveSize() */]);
extern PFUDLLEXPORT void pfuUnGridifyMem(unsigned char *buf, int width, int height, int pixeldepth, int borderwidth, const unsigned char savedcolors[/* pfuGridifySaveSize() */]);
extern PFUDLLEXPORT int pfuGridifyFileP(const char *infilename_for_errors, FILE *infp, const char *outfilename_for_errors, FILE *outfp, int width, int height, int depth, int borderwidth, unsigned char color[/* depth */]);
extern PFUDLLEXPORT int pfuGridifyFile(const char *filename, int width, int height, int depth, int borderwidth, unsigned char color[/* depth */]);
extern PFUDLLEXPORT int pfuGridifyImageTile(pfImageTile *itile, int pixeldepth, const unsigned char color[/* pixeldepth */], int isproto);
extern PFUDLLEXPORT int pfuUnGridifyImageTile(pfImageTile *itile);
extern PFUDLLEXPORT int pfuGridifyImageCache(pfImageCache *icache, int pixeldepth, unsigned char color[/* pixeldepth */]);
extern PFUDLLEXPORT int pfuUnGridifyImageCache(pfImageCache *icache);
extern PFUDLLEXPORT int pfuGridifyClipTexture(pfClipTexture *cliptex);
extern PFUDLLEXPORT int pfuUnGridifyClipTexture(pfClipTexture *cliptex);
extern PFUDLLEXPORT int pfuGridifyMPClipTexture(pfMPClipTexture *mpcliptex);
extern PFUDLLEXPORT int pfuUnGridifyMPClipTexture(pfMPClipTexture *mpcliptex);
extern PFUDLLEXPORT int pfuGridifyAnyClipTexture(pfNode *scene, int i);
extern PFUDLLEXPORT int pfuUnGridifyAnyClipTexture(pfNode *scene, int i);
/* NOTE: pfuMipMinLOD is obsoleted by pfuCalcSizeFinestMipLOD */
extern PFUDLLEXPORT float pfuMipMinLOD(pfChannel *chan, pfFrustum *cullfrust, pfSphere *bsph, float min_dst_dxyz, int logmaxtexdim, int algorithm);
extern PFUDLLEXPORT float pfuCalcSizeFinestMipLOD(pfFrustum *cullFrust, int viewportWidth, int viewportHeight, float heightAboveTerrain, float min_dst_dxyz, int maxtexsize);





/*
 *  For cached culling -- cache results between frames
 */

typedef struct
{
    int        updateFrame;   /* Frame when view pos last changed */
    int        cullResult;    /* Previous cull result */
} pfuNodeCullCache;

typedef struct
{
    int        chanId;	/* Index of this channel */
    int        updateFrame;	/* Frame of current view setting */
} pfuChanCullCache;

extern PFUDLLEXPORT void	pfuTravCachedCull(pfNode* _node, int numChans);

	/*------------------------ MultiChannel Option ------------------------------*/

extern PFUDLLEXPORT void pfuTileChans(pfChannel **chn,  int nChans, int ntilesx, int ntilesy);
extern PFUDLLEXPORT void pfuConfigMCO(pfChannel **chn, int nChans);
extern PFUDLLEXPORT int pfuGetNumMCOChannels(pfPipe *p);
extern PFUDLLEXPORT void pfuTileChan(pfChannel **chn, int thisChan, int nChans, float l, float r, float b, float t);

	/*----------------- MultiPipe Statistics --------------------------------*/

extern PFUDLLEXPORT int pfuManageMPipeStats(int nFrames, int nSampledPipes);


	/*----------------------- Path Following -----------------------------*/
/*
 *	PathSeg -- an individual path-component definition
 */

typedef struct _segment pfuPathSeg;

#ifdef __cplusplus
extern "C++" {  /* give structs containing Performer classes C++ linkage */
#endif

struct _segment
{
    /* pointers to prior and next segments */
    pfuPathSeg	*prev;
    pfuPathSeg	*next;

    /* segment type */
    int		type;
#define		  PATYPE_LINE	1
#define		  PATYPE_ARC	2
#define		  PATYPE_FILLET	3
#define		  PATYPE_SPEED	4
#define		  PATYPE_DELAY	5

    /* line-segment information */
    pfVec3	start;
    pfVec3	final;
    pfVec3	orient;

    /* circular-arc information */
    pfVec3	center;
    float	radius;
    pfVec2	angles;

    /* speed-definition information */
    float	desired;
    float	rate;

    /* delay-definition information */
    float	delay;

    /* segment length (valid for all segment types) */
    float	length;
};

#ifdef __cplusplus
}   /* End of C++ linkage */
#endif

/*
 *	pfuPath -- overall path definition and control
 */

typedef struct _path
pfuPath;

struct _path
{
    /* start segment in path */
    pfuPathSeg	*head;

    /* vehicle parameters */
    float	speed;
    float	pitch;
    float	roll;
    float	desired;
    float	rate;
    float	delay;
    float	position;
    pfuPathSeg	*here;
};

/*
 *	function declarations
 */

extern PFUDLLEXPORT pfuPath *pfuNewPath(void);
extern PFUDLLEXPORT pfuPath *pfuSharePath(pfuPath *path);
extern PFUDLLEXPORT pfuPath *pfuCopyPath(pfuPath *path);
extern PFUDLLEXPORT pfuPath *pfuClosePath(pfuPath *path);
extern PFUDLLEXPORT int pfuFollowPath(pfuPath *path, float seconds, PFVEC3 where, PFVEC3 orient);
extern PFUDLLEXPORT int pfuPrintPath(pfuPath *path);
extern PFUDLLEXPORT int pfuAddPath(pfuPath *path, PFVEC3 first, PFVEC3 final);
extern PFUDLLEXPORT int pfuAddArc(pfuPath *path, PFVEC3 center, float radius, PFVEC2 angles);
extern PFUDLLEXPORT int pfuAddFillet(pfuPath *path, float radius);
extern PFUDLLEXPORT int pfuAddSpeed(pfuPath *path, float desired, float rate);
extern PFUDLLEXPORT int pfuAddDelay(pfuPath *path, float delay);
extern PFUDLLEXPORT int pfuAddFile(pfuPath *path, char *name);

	/*------------------------- Cliptexture Config Utilities -----------------*/

/* Cliptexture and ImageCache Configuration */

#define PFCLIPTEX_MAX_NUM_FORMAT_ARGS 6 /* max args allowed for format string */

/* cliptexture fname argument tokens */
enum {
    PFCLIPTEX_FNAMEARG_INVALID = -1,
    PFCLIPTEX_FNAMEARG_LEVEL = MAX_TILE_FILENAME_UNIQUE_ARGS,
    PFCLIPTEX_FNAMEARG_IMAGE_CACHE_BASE
}; /* FNAME TOKENS */


/* state needed to initialize an image cache */
typedef struct { /* pfuImgCacheConfig */
    char *name;        /* name of image cache */
    int extFormat;     /* performer external format */
    int intFormat;     /* performer internal format */
    int imgFormat;     /* performer image format */
    int components;    /* number of components in format */
    int memRegOrg[3];     /* formerly cache */
    int memRegSize[3];     /* formerly cache */
    int texRegOrg[3];     /* formerly valid region */
    int texRegSize[3];     /* formerly valid region */
    int texSize[3];       /* size of destination texture */
    int size[3];          /* size of image cache (was virtual size) */
    int header;        /* header offset */
    int tilesInFile[3];   /* number of tiles in each file */
    int tileSize[3];      /* dimensions of each tile */
    int pageSize;      /* for direct i/o from disk; overrides ct setting */
    char *base;       /* base name */
    int numParams;     /* number of format arguments */
    char *format;     /* file name format string */
    int args[PFIMAGECACHE_MAX_TILE_FILENAME_ARGS]; /* file name params */
    pfList *sStreams;  /* array of S dimension streams */
    pfList *tStreams;  /* array of T dimension streams */
    pfList *rStreams;  /* array of R dimension streams */
    char *defaultTile;/* default tile pathname */
    pfReadImageTileFuncType readFunc; /* user-defined read function */
    int lookahead; /* extra border tiles in the mem region */
    int clipSize; /* for checking tex region and setting reasonable defaults */
} pfuImgCacheConfig;

typedef struct _pfuClipTexConfig { /* pfuClipTexConfig */
    char *name;        /* name of clip texture */
    int extFormat;     /* performer external format */
    int intFormat;     /* performer internal format */
    int imgFormat;     /* performer image format */
    int components;    /* number of components in format */
    int imgSize[3];    /* size of the cliptexture */
    int minICache[3];  /* smallest image cache dimensions */
    int tileSize[3];   /* dimensions of each tile */
    int clipSize;
    int invBorder;     /* invalid border */
    int effLevels;     /* number of effective levels */
    int allocLevels;   /* number of levels allocated starting at 0 (finest) */
    int hdrOffset;     /* bytes to skip in header of each tile file */
    int pageSize;      /* for direct i/o from disk; passed to icaches */

    void *icData;      /* image cache info */
    pfImageCache *(*icFunc)(pfClipTexture *ct, int level, 
			   struct _pfuClipTexConfig *icInfo);
    void *tileData;    /* image tile info */
    pfImageTile *(*tileFunc)(pfClipTexture *ct, int level, 
			    struct _pfuClipTexConfig *icInfo);
    pfList *sStreams;  /* array of S dimension streams */
    pfList *tStreams;  /* array of T dimension streams */
    pfList *rStreams;  /* array of R dimension streams */

    pfReadImageTileFuncType readFunc; /* user-defined read function */
    int lookahead; /* extra border tiles in the mem region */
} pfuClipTexConfig;

	/*-------------------------- Collision Detection -------------------------*/

#define PFUCOLLIDE_STATIC	0
#define PFUCOLLIDE_DYNAMIC	1

#define PFUCOLLIDE_GROUND	0x1
#define PFUCOLLIDE_OBJECT	0x2
#define PFUCOLLIDE_MASK		0x3

extern PFUDLLEXPORT void	pfuCollisionChan(pfChannel *chan);
extern PFUDLLEXPORT pfChannel* pfuGetCollisionChan(void);

extern PFUDLLEXPORT void 	pfuCollideSetup(pfNode *node, int mode, int mask);

extern PFUDLLEXPORT int 	pfuCollideGrnd(pfCoord *coord, pfNode *node, PFVEC3 zpr);

extern PFUDLLEXPORT int 	pfuCollideGrndObj(pfCoord *coord, pfNode *grndNode, PFVEC3 zpr, pfSeg *seg, pfNode *objNode, PFVEC3 hitPos, PFVEC3 hitNorm);

extern PFUDLLEXPORT int 	pfuCollideObj(pfSeg *seg, pfNode *objNode, PFVEC3 hitPos, PFVEC3 hitNorm);

	/*------------------------ Timer Control --------------------------*/

struct _pfuTimer
{
    double	tstart, tstop, tdelta;

    int	frames;
    double	tnow;
    double	fraction;

    void	(*func)(struct _pfuTimer *timer);
    void	*data;
    int	dataSize;

};

typedef struct _pfuTimer	pfuTimer;

extern PFUDLLEXPORT pfuTimer* pfuNewTimer(void *arena, int size);
extern PFUDLLEXPORT void pfuInitTimer(pfuTimer *timer, double start, double delta, void (*func)(pfuTimer*), void *data);
extern PFUDLLEXPORT void pfuStartTimer(pfuTimer *timer);
extern PFUDLLEXPORT void pfuStopTimer(pfuTimer *timer);
extern PFUDLLEXPORT void pfuEvalTimers(void);
extern PFUDLLEXPORT int pfuEvalTimer(pfuTimer *timer);
extern PFUDLLEXPORT int pfuActiveTimer(pfuTimer * timer);

	/*-------------------- Hash Tables ---------------------------*/

typedef struct _pfuHashElt
{
    int			id;       	/* User field */
    int			listIndex;	/* Index into flat list in pfuHashTable */
    unsigned int	key;		/* Hash key */
    void*		data;		/* Reference to data of 'eltSize' bytes */
    void		*userData;

} pfuHashElt;

typedef struct _pfuHashBucket
{
    int		nelts;
    pfuHashElt	*elts;

    struct _pfuHashBucket	*next;

} pfuHashBucket;

typedef struct _pfuHashTable
{
    void		*arena;
    int			eltSize;
    int			realeltSize;

    int			numBuckets;
    pfuHashBucket	**buckets;

    /* Flat list of hash elements provides linear ordering */
    int			listCount;
    int			listAvail;
    pfuHashElt		**list;

} pfuHashTable;

extern PFUDLLEXPORT pfuHashTable*	pfuNewHTable(int numb, int eltsize, void* arena);
extern PFUDLLEXPORT void		pfuDelHTable(pfuHashTable* ht);
extern PFUDLLEXPORT void		pfuResetHTable(pfuHashTable* ht);
extern PFUDLLEXPORT pfuHashElt*	pfuEnterHash(pfuHashTable* ht, pfuHashElt* elt);
extern PFUDLLEXPORT int		pfuRemoveHash(pfuHashTable* ht, pfuHashElt* elt);
extern PFUDLLEXPORT int		pfuFindHash(pfuHashTable* ht, pfuHashElt* elt);
extern PFUDLLEXPORT int		pfuHashGSetVerts(pfGeoSet *gset);
extern PFUDLLEXPORT int		pfuCalcHashSize(int size);

	/*-------------------- Geometric Simplification ---------------------------*/


extern PFUDLLEXPORT pfLOD*	pfuBoxLOD(pfGroup *grp, int flat, pfVec4* clr);
extern PFUDLLEXPORT pfGeoSet*	pfuMakeBoxGSet(pfBox *box, PFVEC4 clr, int flat);

	/*-------------------- Texture Loading ------------------*/

#define	PFUTEX_APPLY	0
#define	PFUTEX_SHOW	1
#define PFUTEX_DEFINE	2

extern PFUDLLEXPORT pfTexture*	pfuNewSharedTex(const char *_filename, void *_arena);
extern PFUDLLEXPORT pfList*		pfuGetSharedTexList(void);
extern PFUDLLEXPORT pfList *	pfuMakeTexList(pfNode *node);
extern PFUDLLEXPORT pfList *	pfuMakeSceneTexList(pfScene *node);
extern PFUDLLEXPORT void	pfuDownloadTexList(pfList *list, int style);
extern PFUDLLEXPORT int 	pfuGetTexSize(pfTexture *tex);

	/*----------------- Texture Animation ----------*/

typedef struct pfuProjectorData
{
    pfTexture	*handle;
    pfList	*frames;
    pfList	*screens;
    int	numScreens;
} pfuProjectorData;

extern PFUDLLEXPORT void pfuNewTexList(pfTexture *tex);
extern PFUDLLEXPORT pfList *pfuLoadTexListFiles(pfList *_movieTexList, char _nameList[][PF_MAXSTRING], int _len);
extern PFUDLLEXPORT pfList *pfuLoadTexListFmt(pfList *_movieTexList, const char *_fmtStr, int _start, int _end);
extern PFUDLLEXPORT pfSequence *pfuNewProjector(pfTexture *_handle);
extern PFUDLLEXPORT int pfuProjectorPreDrawCB(pfTraverser *_trav, void *_travData);
extern PFUDLLEXPORT void pfuProjectorMovie(pfSequence *_proj, pfList *_movie);
extern PFUDLLEXPORT pfList *pfuGetProjectorScreenList(pfSequence *_proj);
extern PFUDLLEXPORT void pfuAddProjectorScreen(pfSequence *_proj, pfTexture *_screen);
extern PFUDLLEXPORT void pfuRemoveProjectorScreen(pfSequence *_proj, pfTexture *_screen);
extern PFUDLLEXPORT void pfuReplaceProjectorScreen(pfSequence *_proj, pfTexture *_old, pfTexture *_new);
extern PFUDLLEXPORT pfTexture* pfuGetProjectorHandle(pfSequence *_proj);
extern PFUDLLEXPORT void pfuProjectorHandle(pfSequence *_proj, pfTexture *_new);

	/*----------------- Detail Texture  --------------------------------*/

typedef struct {
    char *texname;
    char *detailname;
    int level;
    pfTexture *detailtex;
    int setSpline;
    pfVec2 spline[4];
} pfuDetailInfo;

extern PFUDLLEXPORT void pfuLoadDetailTextures(pfList *list, pfuDetailInfo *di, int ndi);
	/*----------------- Texture Memory  --------------------------------*/

extern PFUDLLEXPORT void pfuShowTextureMemory( pfPipeWindow *pw, pfList *texList, int format );
extern PFUDLLEXPORT void pfuClearTextureMemory( void );

	/*---------------- Random Numbers -------------------*/

extern PFUDLLEXPORT void pfuRandomize (int seed);
extern PFUDLLEXPORT long pfuRandomLong (void);
extern PFUDLLEXPORT float pfuRandomFloat (void);
extern PFUDLLEXPORT void pfuRandomColor (PFVEC4 rgba, float minColor, float maxColor);

	/*------------------------ Flybox Control ---------------------------*/

extern PFUDLLEXPORT int pfuOpenFlybox(char *p);
extern PFUDLLEXPORT int pfuReadFlybox(int *dioval, float *inbuf);
extern PFUDLLEXPORT int pfuGetFlybox(float *analog, int *but);
extern PFUDLLEXPORT int pfuGetFlyboxActive(void);
extern PFUDLLEXPORT int pfuInitFlybox(void);

	/*------------------------ Smoke Simulation ---------------------------*/

typedef struct _pfuSmoke	pfuSmoke;

#define PFUSMOKE_BILLOW		0
#define PFUSMOKE_EXPLOSION	1
#define PFUSMOKE_FIRE		2	/* yes */
#define PFUSMOKE_SMOKE		3	/* yes */
#define PFUSMOKE_EXHAUST	4
#define PFUSMOKE_DUST		5	/* yes */
#define PFUSMOKE_MISSLE		6	/* yes */
#define PFUSMOKE_MISSILE	6	/* yes */ /* alternate spelling */
#define PFUSMOKE_SMOKE_FIRE	7

#define PFUSMOKE_NUM_TYPES	8

#define PFUSMOKE_STOP		0
#define PFUSMOKE_START		1

/* After pfInit() but before pfConfig()*/
extern PFUDLLEXPORT void pfuInitSmokes(void);

extern PFUDLLEXPORT pfuSmoke * pfuNewSmoke(void);
extern PFUDLLEXPORT void pfuSmokeType(pfuSmoke *smoke, int type);

extern PFUDLLEXPORT void pfuSmokeOrigin(pfuSmoke* smoke, PFVEC3 origin, float radius);
extern PFUDLLEXPORT void pfuSmokeDir(pfuSmoke* smoke, PFVEC3 dir);

extern PFUDLLEXPORT void pfuSmokeVelocity(pfuSmoke* smoke, float turbulence, float speed);
extern PFUDLLEXPORT void pfuGetSmokeVelocity(pfuSmoke* smoke, float *turbulence, float *speed);
extern PFUDLLEXPORT void pfuSmokeMode(pfuSmoke* smoke, int mode);

/* Draw Process only */
extern PFUDLLEXPORT void pfuDrawSmokes(PFVEC3 eye);

extern PFUDLLEXPORT void pfuSmokeTex(pfuSmoke* smoke, pfTexture* tex);
extern PFUDLLEXPORT void pfuSmokeDuration(pfuSmoke* smoke, float dur);
extern PFUDLLEXPORT void pfuSmokeDensity(pfuSmoke* smoke, float dens, float diss, float expansion);
extern PFUDLLEXPORT void pfuGetSmokeDensity(pfuSmoke* smoke, float *dens, float *diss, float *expansion);

extern PFUDLLEXPORT void pfuSmokeColor(pfuSmoke* smoke, PFVEC3 bgn, PFVEC3 end);

	/*------------------------ LightPointState Utilities --------------*/

extern PFUDLLEXPORT void pfuMakeLPStateShapeTex(pfLPointState *lps, pfTexture *tex, int size);
extern PFUDLLEXPORT void pfuMakeLPStateRangeTex(pfLPointState *lps, pfTexture *tex, int size, pfFog *fog);

	/*------------------------ Draw Styles ---------------------------*/

#define PFUSTYLE_FILLED		0
#define PFUSTYLE_LINES		1
#define PFUSTYLE_SCRIBED	2
#define PFUSTYLE_DASHED		3
#define PFUSTYLE_HIDDEN		4
#define PFUSTYLE_HALOED		5
#define PFUSTYLE_POINTS		6
#define PFUSTYLE_COUNT		7

extern PFUDLLEXPORT void pfuPreDrawStyle(int style, PFVEC4 scribeColor);
extern PFUDLLEXPORT void pfuPostDrawStyle(int style);

#ifdef IRISGL
/* vack pfVec4 into unsigned byte */
#define PFU_PACK_COLOR4(_cv) (((char)(((_cv)[0])*255)) \
		    | (((char)(((_cv)[1])*255)) << 8) \
		    | (((char)(((_cv)[2])*255)) << 16) \
		    | (((char)(((_cv)[3])*255)) << 24))
#else /* OPENGL */
#define PFU_PACK_COLOR4(_cv) (((char)(((_cv)[3]))*255) \
		    | (((char)(((_cv)[2])*255)) << 8) \
		    | (((char)(((_cv)[1])*255)) << 16) \
		    | (((char)(((_cv)[0])*255)) << 24))
#endif /* GL type */		    

/*
 * snapwin decles
 */

#ifdef __LITTLE_ENDIAN
#define PFU_CPACKTORGB(l,r,g,b)		\
    do {				\
    int val = (int)(l);			\
    (r) = (val >>  0) & 0xff;		\
    (g) = (val >>  8) & 0xff;		\
    (b) = (val >> 16) & 0xff;		\
    } while(0)
#define PFU_CPACKTORGBA(l,r,g,b,a)	\
    do {				\
    int val = (int)(l);			\
    (r) = (val >>  0) & 0xff;		\
    (g) = (val >>  8) & 0xff;		\
    (b) = (val >> 16) & 0xff;		\
    (a) = (val >> 24) & 0xff;		\
    } while(0)
#else /* OPENGL */
#define PFU_CPACKTORGB(l,r,g,b)		\
    do {				\
    int val = (int)(l);			\
    (r) = (val >>  24) & 0xff;		\
    (g) = (val >>  16) & 0xff;		\
    (b) = (val >>  8) & 0xff;		\
    } while(0)
#define PFU_CPACKTORGBA(l,r,g,b,a)	\
    do {				\
    int val = (int)(l);			\
    (r) = (val >>  24) & 0xff;		\
    (g) = (val >>  16) & 0xff;		\
    (b) = (val >>  8) & 0xff;		\
    (a) = (val >>  0) & 0xff;		\
    } while(0)
#endif /* GL type */


extern PFUDLLEXPORT void pfuCalcNormalizedChanXY(float* _px, float* _py, pfChannel* _chan, int _xpos, int _ypos);
extern PFUDLLEXPORT void pfuCalcNormalizedChanXYd(double* _px, double* _py, pfChannel* _chan, int _xpos, int _ypos);
extern PFUDLLEXPORT int pfuSaveImage(char* _name, int _xorg, int _yorg, int _xsize, int _ysize, int _saveAlpha);


	/*
	 * ------------------- Cliptexture utilities ---------------------
	 */

extern PFUDLLEXPORT	pfPipe *pfuAddMPClipTextureToPipes(pfMPClipTexture *_master,
					   pfPipe *_masterpipe,
					   pfPipe *_pipes[]);

extern PFUDLLEXPORT void pfuAddMPClipTexturesToPipes(pfList *_mpcliptextures, 
					pfPipe * _masterpipe,
					pfPipe *_pipes[]);


extern PFUDLLEXPORT pfClipTexture *pfuMakeClipTexture(pfuClipTexConfig *config);

extern PFUDLLEXPORT void pfuInitClipTexConfig(pfuClipTexConfig *config);
extern PFUDLLEXPORT void pfuFreeClipTexConfig(pfuClipTexConfig *config);

extern PFUDLLEXPORT pfImageCache *pfuMakeImageCache(pfTexture *texture,
				       int level,
				       pfuImgCacheConfig *config);
extern PFUDLLEXPORT void pfuInitImgCacheConfig(pfuImgCacheConfig *config);
extern PFUDLLEXPORT void pfuFreeImgCacheConfig(pfuImgCacheConfig *config);

extern PFUDLLEXPORT void pfuCalcVirtualClipTexParams(
        int nLevels,        /* total number of virtual levels */
        int clipSize,
        int invalidBorder,
        float minLODTexPix, /* lower bound on the resolution the hardware */
                            /* will access, based on calculation of */
                            /* tex/pix partial derivatives (don't */
                            /* forget to add in LODbiasS,LODbiasT!) */
                            /* or a height-above-terrain lookup table */
        float minLODLoaded, /* float for fade-in */
        float maxLODLoaded, /* may use someday if we get a way to reuse LODs */
        float bboxMinDist,  /* min distance from clip center in texture coords*/
        float bboxMaxDist,  /* max distance from clip center in texture coords*/
        float tradeoff, /* 0. means go fine (hi resolution at expense of area)*/
                        /* 1. means go coarse (area at expense of resolution) */
        const struct pfVirtualClipTexLimits *limits,
        int *return_LODOffset,
        int *return_numEffectiveLevels,
        float *return_minLOD,
        float *return_maxLOD);

extern PFUDLLEXPORT void pfuReallyInvalidateClipTexture(pfClipTexture *clipTex,
                                           pfuBarrier *barr);

extern PFUDLLEXPORT int pfuCalcTexDerivs(
  float xa, float ya, float wa, float sa, float ta, /* vertex a screen coords */
  float xb, float yb, float wb, float sb, float tb, /* vertex b screen coords */
  float xc, float yc, float wc, float sc, float tc, /* vertex c screen coords */
  float x, float y,   /* screen coords at which to evaluate */
    float *s, float *t, /* return tex coords at x,y (optional) */
    float *ds_dx,       /* return ds/dx (optional) */
    float *dt_dx,       /* return dt/dx (optional) */
    float *ds_dy,       /* return ds/dy (optional) */
    float *dt_dy);      /* return dt/dy (optional) */

	/*
	 * ------------------- Macros for typecasting ---------------------
	 */


#if defined(__STDC__) || defined(__cplusplus)

#ifndef __PFU_ISECT_C__
#define	pfuSegsIsectNode(_node, _segs, _nseg, _mode, _mask, _bcyl, \
			 _isects, _discFunc) \
	pfuSegsIsectNode((pfNode*) _node, _segs, _nseg, _mode, _mask, _bcyl, \
			 _isects, _discFunc)
#endif

#ifndef _PFU_TRAV_C_
#define	pfuTravCalcBBox(_node, _box) \
    pfuTravCalcBBox((pfNode *)_node, _box)

#define	pfuCalcNodeBBox(_node, _box) \
    pfuTravCalcBBox((pfNode *)_node, _box)

#define	pfuTravCountNumVerts(_node) \
    pfuTravCountNumVerts((pfNode *)_node)

#define	pfuTravSetDListMode(_node, _enable) \
    pfuTravSetDListMode((pfNode *)_node, _enable)

#define	pfuTravCompileDLists(_node, _delMask) \
    pfuTravCompileDLists((pfNode *)_node, _delMask)

#define	pfuTravDrawDLists(_node) \
    pfuTravDrawDLists((pfNode *)_node)

#define	pfuTravCreatePackedAttrs(_node, _format, _delMask) \
    pfuTravCreatePackedAttrs((pfNode *)_node, _format, _delMask)


#endif

#ifndef _PFU_COLLIDE_C_		/* can't def these for collide.c */
#define pfuCollideSetup(_node, _mode, _mask) \
	pfuCollideSetup((pfNode *)_node, _mode, _mask)

#define pfuCollideGrnd(coord, node, zpr)	    \
	pfuCollideGrnd(coord, (pfNode *)node, zpr)

#define pfuCollideGrndObj(coord, grndNode, zpr, seg, objNode, hitPos,hitNorm) \
	pfuCollideGrndObj(coord, (pfNode*)grndNode, zpr, seg,		    \
			    (pfNode*)objNode, hitPos, hitNorm)

#define pfuCollideObj(seg, objNode, hitPos, hitNorm)		\
	pfuCollideObj(seg, (pfNode*)objNode, hitPos, hitNorm)

#endif	/* _PFU_COLLIDE_C_ */
#endif	/* defined(__STDC__) || defined(__cplusplus) */

#ifndef __PFU_AUTOLIST_H__
#if !PF_CPLUSPLUS_API /* Also in C++ header pfuAutoList.h */
/* ------------------ pfuAutoList Macros --------------------- */
#if defined(__STDC__) || defined(__cplusplus)

#define pfuResetList(_list)		pfuResetList((pfuAutoList*)_list)
#define pfuGetNum(_list)		pfuGetNum((pfuAutoList*)_list)
#define pfuAdd(_list, _elt)		pfuAdd((pfuAutoList*)_list, _elt)
#define pfuInsert(_list, _index, _elt)	pfuInsert((pfuAutoList*)_list, _index, _elt)
#define pfuSearch(_list, _elt)		pfuSearch((pfuAutoList*)_list, _elt)
#define pfuSearchForType(_list, _type)	pfuSearchForType((pfuAutoList*)_list, _type)
#define pfuRemove(_list, _elt)		pfuRemove((pfuAutoList*)_list, _elt)
#define pfuReplace(_list, _old, _new)	pfuReplace((pfuAutoList*)_list, _old, _new)
#define pfuSet(_list, _index, _elt)	pfuSet((pfuAutoList*)_list, _index, _elt)
#define pfuGet(_list, _index)		pfuGet((pfuAutoList*)_list, _index)

#endif /* defined(__STDC__) || defined(__cplusplus) */
#endif /* !PF_CPLUSPLUS_API */
#endif	/* __PFU_AUTOLIST_H__ */


#ifndef _PFU_PROCESS_MANAGER_H_
#if !PF_CPLUSPLUS_API 
/* ------------------ pfuBaseProcessManager Macros --------------------- */
#if defined(__STDC__) || defined(__cplusplus)
#endif /* defined(__STDC__) || defined(__cplusplus) */
#define pfuSelectProcessManager(m) pfuSelectProcessManager((pfuBaseProcessManager *)m)
#endif /* !PF_CPLUSPLUS_API */
#endif	/* _PFU_PROCESS_MANAGER_H_ */


#ifdef __cplusplus
}
#endif


#endif /* __PFUTIL_H__ */
