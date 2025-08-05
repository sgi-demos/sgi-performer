
// This file contains the public interface from the Performer
// header file Perfomer/pfutil.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%section "libpfu"

%{
#include <Performer/pfutil.h>
%}

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

extern void	    pfuInit(void);
extern void	    pfuInitClasses(void);
extern void	    pfuExitUtil(void);
extern void	    pfuDPoolSize(long size);
extern long	    pfuGetDPoolSize(void);

/*---------------- pfuAutoList CAPI ------------------------------*/


	/*---------------- Processor Control -----------------------------*/

extern int pfuRunProcOn(int cpu);
extern int pfuLockDownProc(int cpu);
extern int pfuLockDownApp(void);
extern int pfuLockDownCull(pfPipe *);
extern int pfuLockDownDraw(pfPipe *);
extern int pfuLockDownLPoint(pfPipe *);
extern int pfuPrioritizeProcs(int onOff);
extern int pfuRunDiskProcsOn(int cpu);

	/*----------------- Process Manager C-API ------------------------*/


	/*----------------- Clip Center Node C-API ------------------------*/


	/*------------------ Multiprocess Rendezvous ---------------------*/


	/*----------------------- GLX Mixed Mode -------------------------*/

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
    int         flags;
    int         modifiers;
    int         xpos, ypos;
    float       xchan, ychan;
    double      posTime;

    /* These are used by the GUI and pfiXformer
     * GUI needs Last click positional info
     * Xformers need last and middle click and release info
     */
    int         click;          
%readonly
    int         clickPos[PFUDEV_MOUSE_DOWN_MASK][2];    
%readonly
    int         clickPosLast[2];
    int         release;
%readonly
    int         releasePos[PFUDEV_MOUSE_DOWN_MASK][2]; 
%readonly
    int         releasePosLast[2];
%readonly
    double      clickTime[PFUDEV_MOUSE_DOWN_MASK];
    double      clickTimeLast;
%readonly
    double      releaseTime[PFUDEV_MOUSE_DOWN_MASK];
    double      releaseTimeLast;
    int         winSizeX;       
    int         winSizeY;
    ulong       inWin;          
%readwrite
} pfuMouse;


typedef struct _pfuEventQueue pfuEventQueue;

typedef struct _pfuEventStream
{
    int         buttonFlags;
    int         frameStamp;
    int         numDevs;
    int         numKeys;
%readonly
    int         devQ[PFUDEV_MAX_INDEVS];
%readonly
    int         devCount[PFUDEV_MAX_DEVS];
%readonly
    int         devVal[PFUDEV_MAX_INDEVS];
%readonly
    int         keyQ[PFUDEV_MAX_INKEYS];
%readonly
    int         keyCount[PFUDEV_KEY_MAP_SIZE];
%readwrite
} pfuEventStream;

typedef struct _pfuEvent
{
    int dev;
    int val;
    int count;
} pfuEvent;

extern pfuEventQueue *pfuNewEventQ(pfDataPool *dp, int id);
extern void pfuResetEventStream(pfuEventStream *es);
extern void pfuResetEventQ(pfuEventQueue *eq);
extern void pfuAppendEventQ(pfuEventQueue *eq0, pfuEventQueue *eq1);
extern void pfuAppendEventQStream(pfuEventQueue *eq, pfuEventStream *es);
extern void pfuEventQStream(pfuEventQueue *eq, pfuEventStream *es);
extern pfuEventStream *pfuGetEventQStream(pfuEventQueue *eq);
extern void pfuGetEventQEvents(pfuEventStream *events, pfuEventQueue *eq);
extern void pfuIncEventQFrame(pfuEventQueue *eq);
extern void pfuEventQFrame(pfuEventQueue *eq, int val);
extern int pfuGetEventQFrame(pfuEventQueue *eq);
extern void pfuIncEventStreamFrame(pfuEventStream *es);
extern void pfuEventStreamFrame(pfuEventStream *es, int val);
extern int pfuGetEventStreamFrame(pfuEventStream *es);

typedef struct _pfuCustomEvent
{
    int dev;
    int val;
} pfuCustomEvent;

typedef void (*pfuEventHandlerFuncType)(int _dev, void* _val, pfuCustomEvent *_pfuevent);

/* pfuInitInput() types */
#define PFUINPUT_X		0
#define PFUINPUT_GL		1 
#define PFUINPUT_NOFORK_X	2

/* pfuInputHandler() handler mode */
#define PFUINPUT_CATCH_UNKNOWN	0
#define PFUINPUT_CATCH_SIM	1
#define PFUINPUT_CATCH_ALL	(~((unsigned int )0))

/*** routines to do auto-event collection for you ***/
extern void pfuInitInput(pfPipeWindow *_pw, int _mode);
extern void pfuInitMultiChanInput(pfChannel **_chanArray, int _numChans, int _mode);
extern void pfuExitInput(void);
extern int pfuMapMouseToChan(pfuMouse *mouse, pfChannel *chan);
extern int pfuMouseInChan(pfuMouse *mouse, pfChannel *chan);

/* get back the auto-collected events */
extern void	pfuGetMouse(pfuMouse *_mouse);
extern void	pfuGetEvents(pfuEventStream *_events);

/* this routine lets you get/record events yourself */
extern void	pfuInputHandler(pfuEventHandlerFuncType _userFunc, unsigned int mask);
extern void	pfuMouseButtonClick(pfuMouse *mouse, 
		    int _button, int _x, int _y, double _time);
extern void	pfuMouseButtonRelease(pfuMouse *mouse, 
		    int _button, int _x, int _y, double _time);
/* map X time stamps to pfGetTime() relative values */
extern double pfuMapXTime(double xtime);
		    
/*------------------------- Cursor Control ---------------------------*/

/*
 * Cursor utilities are in cursor.c and require GLX
 */

/* PFU Cursor targets */
#define PFU_CURSOR_MAIN  1
#define PFU_CURSOR_GUI   2

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


extern Cursor	pfuGetInvisibleCursor(void);
extern void	pfuLoadPWinCursor(pfPipeWindow *_w, int _c);
extern void 	pfuLoadWinCursor(pfWindow *_w, int _c);
extern void	pfuLoadChanCursor(pfChannel *_chan, int _c);
extern Cursor	pfuCreateCursor(Cursor index);

extern void	pfuDraw2DCursor(int mx, int my);
extern void	pfuDrawPWin2DCursor(pfPipeWindow *pw, int mx, int my);
extern void	pfuDrawVChan2DCursor(pfVideoChannel *vChan, int mx, int my);
extern void	pfuDrawPVChan2DCursor(pfPipeVideoChannel *vChan, int mx, int my);
extern void	pfuDrawChan2DCursor(pfChannel *chan, int mx, int my);
extern void	pfuCursorColor(pfVec3 fg, pfVec3 bg);

extern void	pfuCursorType(int val);
extern int	pfuGetCursorType(void);
extern void	pfuSelCursor(int c);
extern void	pfuCursor(Cursor c, int index);
extern Cursor	pfuGetCursor(int index);

/**** GUI maintains a cursor - in gui.c ****/

/* Cursor sel constants */
#define PFU_CURSOR_IN_MAIN 	0
#define PFU_CURSOR_IN_GUI 	1

extern void	pfuInitGUICursors(void);

extern void	pfuGUICursor(int target,  int c);
extern Cursor	pfuGetGUICursor(int target);

extern void	pfuGUICursorSel(int sel);
extern Cursor	pfuGetGUICursorSel(void);

extern void	pfuUpdateGUICursor(void);

/**** DVR utilities in gui.c ****/

extern void     pfuDrawChanDVRBox(pfChannel *chan);

	/*-------------------- OpenGL X Fonts -----------------------*/


	/*------------------------- Simple GUI ---------------------------*/


	/*----------------- Scene Graph Traversal ------------------------*/


	/*------------------------ MultiChannel Option ------------------------------*/

	/*----------------- MultiPipe Statistics --------------------------------*/

	/*----------------------- Path Following -----------------------------*/

	/*------------------------- Cliptexture Config Utilities -----------------*/

	/*-------------------------- Collision Detection -------------------------*/


	/*------------------------ Timer Control --------------------------*/


	/*-------------------- Hash Tables ---------------------------*/


	/*-------------------- Geometric Simplification ---------------------------*/

	/*-------------------- Texture Loading ------------------*/


	/*----------------- Texture Animation ----------*/

	/*----------------- Detail Texture  --------------------------------*/

	/*----------------- Texture Memory  --------------------------------*/

	/*---------------- Random Numbers -------------------*/


	/*------------------------ Flybox Control ---------------------------*/

	/*------------------------ Smoke Simulation ---------------------------*/

	/*------------------------ LightPointState Utilities --------------*/


	/*------------------------ Draw Styles ---------------------------*/

	/*
	 * ------------------- Cliptexture utilities ---------------------
	 */

