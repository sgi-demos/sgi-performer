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
 * file: input.c  
 * --------------
 * 
 * $Revision: 1.1 $
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
#include <signal.h>
#include <sys/types.h>
#include <sys/sysmp.h>
#if defined(__linux__has_schedctl_h__) || !defined(__linux__)
#include <sys/schedctl.h>
#endif  /* __linux__ */
#include <sys/prctl.h>

#ifdef	_POSIX_SOURCE
#ifdef	_LANGUAGE_C_PLUS_PLUS
extern void (*sigset (int sig, void (*disp)(...)))(...);
extern void sginap(int);
#else
extern void (*sigset (int sig, void (*disp)()))();
extern long sginap(long);
#endif
#endif

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include "input.h"
#include "eventq.h"

/* uncomment DO_TIME to see times of pfuGetEvents > BIG_TIME */
/*
 * #define DO_TIME 1
 */

#define BIG_TIME 0.001

	/*----------------------------------------------------------*/

static pfDataPool   	*UtilDP=NULL;
static pfuDeviceInput  	*DeviceInput=NULL;
static int 		havePotentialFocus = 1;
	/*----------------------------------------------------------*/

static void	initDataPool(void);
static void	openXInput(int chan);
static void	mpXInput(int chan);
static void	collectXInput(int chan);
static void	callUserHandler(unsigned long _dev, void *_val);




	/*----------------------------------------------------------*/

static void
initDataPool(void)
{
    if (DeviceInput)
	return;
	
    UtilDP = pfuGetUtilDPool();
    if (!(DeviceInput = (pfuDeviceInput *)  pfDPoolFind(UtilDP, PFU_XIO_DATA_DPID)))
    {
	pfNotify(PFNFY_FATAL, PFNFY_SYSERR, 
	    "InitpfuDeviceInputPool - didn't find pfuDeviceInput  in Util Data Pool.");
	return;
    }

    if (!(DeviceInput->mouse = (pfuMouse *) pfDPoolFind(UtilDP, PFU_MOUSE_DATA_DPID)))
    {
	pfNotify(PFNFY_FATAL, PFNFY_SYSERR, 
		    "InitpfuDeviceInputPool - didn't find pfuMouse struct.");
    }

    DeviceInput->eq = pfuNewEventQ(UtilDP, PFU_XEVENT_DATA_DPID);
    if (!(DeviceInput->eq))
    {
	pfNotify(PFNFY_FATAL, PFNFY_SYSERR, 
		 "InitpfuDeviceInputPool - didn't find pfuEventQueue "
		 "struct.");
    }
}

void
pfuInitMultiChanInput(pfChannel *Chan[], int NumChans, int mode)
{
    pfPipeWindow	*pw;
    pid_t	    	fpid;
    int			chan=0;

    if (NumChans > MAX_INPUT_CHANS)
    {
	pfNotify (PFNFY_WARN, PFNFY_USAGE, "pfuInitInputs() has more than %d "
		  "channels to initialize.", MAX_INPUT_CHANS);
	NumChans = MAX_INPUT_CHANS;
    }

    if (DeviceInput)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuInitMultiChanInput() Input already "
		 "initialized. Returning...");
	return;
    }

    initDataPool();
    pfDPoolLock(DeviceInput);

    DeviceInput->numChan = NumChans;
    
    for (chan = 0; chan < NumChans; chan++)
    {
	if (Chan[chan] == NULL)
	    pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuInitMultiChanInput() - chan %d is NULL", chan);
	    
	pw = pfGetChanPWin(Chan[chan]);

	if (pw == NULL)
	{
	    pfNotify(PFNFY_WARN, PFNFY_USAGE,
		     "pfuInitInput() Null pfPipeWindow.");
	    continue;
	}

	DeviceInput->ichan[chan].pipeWin = pw;
	DeviceInput->ichan[chan].pipe = pfGetPWinPipe(pw);
	DeviceInput->ichan[chan].xWin = pfGetPWinWSWindow(pw);
    
	switch(mode) 
	{
	  case PFUINPUT_NOFORK_X:
	    DeviceInput->mode = PFUINPUT_X;
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		"pfuInitMultiChanInput() requires changing mode to forked X input");
	  case PFUINPUT_X:
	    DeviceInput->mode = PFUINPUT_X;
	    DeviceInput->forked = 1;
	    if ((fpid = fork()) < 0)
		pfNotify(PFNFY_FATAL, PFNFY_SYSERR,
			 "pfuInitInput() Fork failed.");
	    else if (!fpid)
	    {
		mpXInput(chan);
	    }
	    break;
	  case PFUINPUT_GL:
	    DeviceInput->mode = PFUINPUT_GL;
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		"pfuInitMultiChanInput() IRISGL input specified. X input required for multiple chans. "
		" only using chan 0.");
		DeviceInput->numChan = NumChans = 1;
	    break;
	  default:
	    pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuInitInput() Unknown "
		     "mode %d", mode);
	}
    }
    
    pfDPoolUnlock(DeviceInput);
}

void
pfuInitInput(pfPipeWindow *pw, int mode)
{
    pid_t	    fpid;

    if (pw == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuInitInput() Null pfPipeWindow.");
	return;
    }

    if (DeviceInput)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuInitInput() Input already "
		 "initialized. Returning...");
	return;
    }

#if defined(__linux__) && !defined(__linux__has_MP__)
    if (mode == PFUINPUT_X)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE,
            "Linux: asynchronous X Input not supported.");
        pfNotify(PFNFY_WARN, PFNFY_USAGE,
            "pfuInitInput() mode forced to PFUINPUT_NOFORK_X.");
        mode = PFUINPUT_NOFORK_X;
    }
#endif
    
    initDataPool();

    pfDPoolLock(DeviceInput);

    DeviceInput->numChan = 1;
    DeviceInput->ichan[0].pipeWin = pw;
    DeviceInput->ichan[0].pipe = pfGetPWinPipe(pw);
    DeviceInput->ichan[0].xWin = pfGetPWinWSWindow(pw);

    switch(mode) {
      case PFUINPUT_NOFORK_X:
	DeviceInput->mode = PFUINPUT_X;
	break;
      case PFUINPUT_X:
        DeviceInput->mode = PFUINPUT_X;
	DeviceInput->forked = 1;
	if ((fpid = fork()) < 0)
	    pfNotify(PFNFY_FATAL, PFNFY_SYSERR, "pfuInitInput() Fork failed.");
	else if (!fpid)
	{
	    mpXInput(0);
	}
	break;
      case PFUINPUT_GL:
	DeviceInput->mode = PFUINPUT_GL;
	break;
      default:
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfuInitInput() Unknown "
		 "mode %d", mode);
    }

    pfDPoolUnlock(DeviceInput);
}

void pfuInputHandler(pfuEventHandlerFuncType userFunc, 
		    unsigned int mask)
{
    if (!DeviceInput)
    	initDataPool();

    DeviceInput->userHandler = userFunc;
    DeviceInput->handlerMask = mask;
}

void 
pfuExitInput(void)
{
    if (DeviceInput)
	DeviceInput->exit = 1;
}

/* pfuCalcNormalizedChanXY - normalize window relative x,y to full chan size 
 * for input
 */
void
pfuCalcNormalizedChanXY(float *px, float *py, 
			    pfChannel *chan, int xpos, int ypos)
{
    int ch_xo, ch_yo, ch_xs, ch_ys;
    
    pfGetChanOrigin(chan, &ch_xo, &ch_yo);
    pfGetChanSize(chan, &ch_xs, &ch_ys);
	
    *px = (float)(xpos - (ch_xo)) / (float)ch_xs;
    *py = (float)(ypos - (ch_yo)) / (float)ch_ys;
}
/* the following isn't really necessary, but is a convenience for libpfuiD
 * which wants to do everything in double */
void
pfuCalcNormalizedChanXYd(double *_px, double *_py, 
			    pfChannel *chan, int xpos, int ypos)
{
    float px, py;
    pfuCalcNormalizedChanXY(&px, &py, chan, xpos, ypos);
    *_px = px;
    *_py = py;
}


/*
 * Map mouse from window coordinates to channel coordinates. If mouse
 * is in channel its coords will range from -1 to 1. Return TRUE if
 * mouse is in channel viewport.
*/
int
pfuMapMouseToChan(pfuMouse *mouse, pfChannel *chan)
{
    int   xo, yo, xs, ys, xc, yc;

    /* get channel origin and size */
    pfGetChanOrigin(chan, &xo, &yo);
    pfGetChanSize(chan, &xs, &ys);
    
    /* Find window coordinates of channel center. */
    xc = xo + xs / 2.0f;
    yc = yo + ys / 2.0f;

    /* Map mouse into channel coords. */
    if (xs <= 0.0f)
	mouse->xchan = 0.0f;
    else
	mouse->xchan = 2.0f * (mouse->xpos - xc) / (float)xs;
    if (ys <= 0.0f)
	mouse->ychan = 0.0f;
    else
	mouse->ychan = 2.0f * (mouse->ypos - yc) / (float)ys;

    if ((xs <= 0.0f) || (ys <= 0.0f))
	return 0;

    return PFUMOUSE_IN_CHAN(mouse);
}

int
pfuMouseInChan(pfuMouse *mouse, pfChannel *chan)
{
    int   xo, yo, xs, ys, xc, yc, clickX, clickY;

    /* get channel origin and size */
    pfGetChanOrigin(chan, &xo, &yo);
    pfGetChanSize(chan, &xs, &ys);
    
    /* Find window coordinates of channel center. */
    xc = xo + xs / 2.0f;
    yc = yo + ys / 2.0f;

    /* Map mouse into channel coords. */
    if (xs <= 0.0f)
	mouse->xchan = 0.0f;
    else
	mouse->xchan = 2.0f * (mouse->xpos - xc) / (float)xs;
    if (ys <= 0.0f)
	mouse->ychan = 0.0f;
    else
	mouse->ychan = 2.0f * (mouse->ypos - yc) / (float)ys;

    if ((xs <= 0.0f) || (ys <= 0.0f))
	return 0;

    if (mouse->flags & PFUDEV_MOUSE_DOWN_MASK)
    {
	clickX = mouse->clickPosLast[0];
	clickY = mouse->clickPosLast[1];

    /* if mouse is down, required to have clicked in this channel */
	return ((clickX >= xo && clickX <=(xo + xs) &&
	    clickY >= yo && clickY <=(yo + ys)));
    }

    return PFUMOUSE_IN_CHAN(mouse);
}

/*
 * Copy internal pfuMouse structure into 'mouse'
*/
void
pfuGetMouse(pfuMouse* mouse)
{    
    XEvent	    event;
    int		    chan;
    
    if (!DeviceInput)
    	initDataPool();

    for (chan = 0; chan < DeviceInput->numChan; chan++)
    {
	if (!DeviceInput->forked && 
	    (DeviceInput->mode == PFUINPUT_X) && 
	    !DeviceInput->inited[chan])
	    openXInput(chan);
	if (!DeviceInput->inited[chan])
	    return;

	if ((DeviceInput->mode == PFUINPUT_X) && 
	    (!DeviceInput->forked &&
	     (XEventsQueued(DeviceInput->ichan[chan].dsp, QueuedAfterFlush))))
	    collectXInput(chan);
    }

    pfDPoolLock(DeviceInput->mouse);
    *mouse = *DeviceInput->mouse;
    pfDPoolUnlock(DeviceInput->mouse);
}


/*
 * Copy internal pfuEventQueue structure into 'event'. Also reset internal
 * pfuEventQueue structure.
*/
void
pfuGetEvents(pfuEventStream* events)
{
    int dfs, efs;
    XEvent	    event;
    int		    chan;
    int		    events_found = 0;

#ifdef DO_TIME
    float start_time, acquire_time, total_time;
#endif    
    if (!DeviceInput)
    	initDataPool();

    for (chan = 0; chan < DeviceInput->numChan; chan++)
    {
	if (!DeviceInput->forked && 
	    (DeviceInput->mode == PFUINPUT_X) && 
	    !DeviceInput->inited[chan])
	    openXInput(chan);
	if (!DeviceInput->inited[chan])
	    return;

	if ((DeviceInput->mode != PFUINPUT_GL) && 
	    (!DeviceInput->forked &&
	     (XEventsQueued(DeviceInput->ichan[chan].dsp, QueuedAfterFlush))))
	    collectXInput(chan);

	dfs = pfuGetEventQFrame(DeviceInput->eq);
	efs = pfuGetEventStreamFrame(events);

	if ((efs < dfs) || (dfs == 0) || (efs == 0))
	{
	    pfuEventStreamFrame(events, dfs);
	    events_found = 1;
#ifdef DO_TIME
	    if (chan == 0)
		start_time = pfGetTime();
#endif
	    /* Get a snapshot of events*/
	    pfuGetEventQEvents(events, DeviceInput->eq);

	    /* Reset events so next colletion will start afresh */
	    DeviceInput->resetEvents = 1;

	}
    }
    if (events_found)
    {
#ifdef DO_TIME
	total_time = pfGetTime() - start_time;
	if (total_time > BIG_TIME)
	{
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
		     "pfuGetEvents: [total=%f acquire=%f]", total_time,
		     acquire_time);
	}
#endif
    }
    else
    {
	events->numDevs = 0;
    }
}

/*
 * Collect device and mouse input into internal pfuMouse and pfuEventQueue
 * structures. This only has meaning for GL input since the forked X input
 * process automatically collects inputs.
 *
 * If using GL input, this should be called only from the draw process.
 *
 * Can only have one channel for IRIS GL input so if IRIS GL assume channel 0.
*/
void
pfuCollectInput(void)
{
    if (!DeviceInput)    
	initDataPool();

    switch(DeviceInput->mode) {
    case PFUINPUT_X:
	/* Do nothing, input is automatically collected */
	break;
    case PFUINPUT_GL:
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"pfuCollectInput: running OpenGL - must use X input.");
	break;
    }
}


static void
callUserHandler(unsigned long dev, void *val)
{
    pfuCustomEvent  customEvent;
    pfuEventStream  *events;
    
    events = pfuGetEventQStream(DeviceInput->eq);

    customEvent.dev = PFUDEV_NULL; 
    customEvent.val = 0; 
    DeviceInput->userHandler(dev, val, &customEvent);
    /* if have return device, place in queue */
    if (customEvent.dev)
    {
	events->devQ[events->numDevs] = customEvent.dev;
	events->devVal[events->numDevs] = customEvent.val;
	if (customEvent.dev < PFUDEV_MAX_DEVS)
	    events->devCount[customEvent.dev] += 1;
	else
	    pfNotify(PFNFY_WARN,PFNFY_USAGE,
		"pfuCollectInput - user device %d higher than max %d", 
		customEvent.dev, PFUDEV_MAX_DEVS);
	events->numDevs++;
    }
}


void
pfuMouseButtonClick(pfuMouse *mouse, int _button, int _x, int _y, double _time)
{
    mouse->flags |= _button;
    mouse->clickPos[_button][0] = _x;
    mouse->clickPos[_button][1] = _y;
    mouse->clickPosLast[0] = _x;
    mouse->clickPosLast[1] = _y;
    mouse->click |= _button;
    mouse->clickTime[_button] = mouse->clickTimeLast = _time;
    /* If have click, clear corresponding release */
    mouse->release &= ~_button;
}

void
pfuMouseButtonRelease(pfuMouse *mouse, int _button, int _x, int _y, double _time)
{
    mouse->flags &= ~_button;
    mouse->releasePos[_button][0] = _x;
    mouse->releasePos[_button][1] = _y;
    mouse->releasePosLast[0] = _x;
    mouse->releasePosLast[1] = _y;
    mouse->release |= _button;
    mouse->releaseTime[_button] =  mouse->releaseTimeLast = _time;
    /* If have release, clear corresponding click */
    mouse->click &= ~_button;
}



        /*------------------------ X Input ----------------------------*/


/*
 * This is sample code which uses X, not GL, to receive mouse 
 * and keyboard input. 
 * This is useful when you want to get these events but not open a GL window 
 * to do so. This code was written specifically for full-screen applications. 
 *
 * Although it is possible to create an 'input-only' GL window(e.g.-noport), 
 * performance and cleanliness reasons prompt us to recommend the method
 * given below.
 *
 * NOTE: these routines will currently support only one X process
 *	in the first pipe.
 */
 
static Atom	wm_protocols, wm_delete_window;
static long	    theEventmask;

static void
openXInput(int chan)
{
    pfWSConnection	    dsp;
    pfPipe		    *p;
    const char		    *str;

    p = DeviceInput->ichan[chan].pipe;
    if (!p)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
            "openXInput has NULL pipe.  Call pfuInitInput.");
	return;
    }
    
    if (!(DeviceInput->ichan[chan].xWin =
	  pfGetPWinWSWindow(DeviceInput->ichan[chan].pipeWin)))
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
            "openXInput waiting on X window for chan %d.", chan);
	return;
    }
    
    str = pfGetPipeWSConnectionName(p);
    
    if (!(dsp = pfOpenWSConnection(str, FALSE)))
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, 
		"openXInput() Can't open display %s", str);
	return;
    }
    DeviceInput->ichan[chan].dsp = dsp;

    /* do an XSync to make sure the window is inited for this connection */
    XSync(dsp,FALSE);
   
    theEventmask = (FocusChangeMask | ExposureMask | VisibilityChangeMask |
		    /* StructureNotifyMask | */
		    KeyPressMask | KeyReleaseMask | 
		    ButtonPressMask | ButtonReleaseMask |
		    PointerMotionMask);

    wm_protocols = XInternAtom(dsp, "WM_PROTOCOLS", 1);
    wm_delete_window = XInternAtom(dsp, "WM_DELETE_WINDOW", 1);

    XSetWMProtocols(dsp, DeviceInput->ichan[chan].xWin,
		    &wm_delete_window, 1);
    XSelectInput(dsp, DeviceInput->ichan[chan].xWin, theEventmask);
    XMapWindow(dsp, DeviceInput->ichan[chan].xWin);
    XSync(dsp,FALSE);
    
    DeviceInput->inited[chan]= 1;
    
    if (DeviceInput->forked)
	pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	    "Asynchronous X Input process %d opened on Display %s", getpid(), DisplayString(dsp));
    else
	pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	    "X Input opened on Display %s", DisplayString(dsp));
}

static void
mpXInput(int chan)
{
    XEvent	    event;
    Window	    xWin;
    int		    haveOpenWin = 0;

    #if !defined(__linux__)
    prctl(PR_TERMCHILD);        /* Exit when parent does */
    #else
	prctl(PR_SET_PDEATHSIG, SIGHUP, 0, 0, 0);
    #endif  /* __linux__ */
    sigset(SIGHUP, SIG_DFL);    /* Exit when sent SIGHUP by TERMCHILD */

    pfuRunProcOn(pfGetProcessMiscCPU());

    initDataPool();
    
    while (!haveOpenWin)
    {
	if (pfIsPWinOpen(DeviceInput->ichan[chan].pipeWin))
	{
	    DeviceInput->ichan[chan].xWin = 
		pfGetPWinWSWindow(DeviceInput->ichan[chan].pipeWin);
	    if (DeviceInput->ichan[chan].xWin)
		haveOpenWin = 1;
	}
	if (!haveOpenWin)
	    sginap(2);
    }
    
    pfNotify (PFNFY_INFO, PFNFY_PRINT, "X input process %d running "
	      "for channel %d", getpid(), chan);

    while (!DeviceInput->exit)
    {
	if (!DeviceInput->inited[chan])
	{
	    openXInput(chan);
	}
	if (DeviceInput->inited[chan])
	{
	    XPeekEvent(DeviceInput->ichan[chan].dsp, &event);
	    collectXInput(chan);
	}
	
    }
    exit(EXIT_SUCCESS);
}

/*
 * Flush input queue and update the shared pfuMouse and pfuEventQueue structures.
 */
static void
collectXInput(int chan)
{
    pfuMouse		mouse;
    pfuEventStream	events;


    /* clear out the local events structure */
    pfuResetEventStream(&events);
    
    /* Copy DeviceInput mouse structure into mouse */
    pfDPoolLock(DeviceInput->mouse);
    mouse = *DeviceInput->mouse;
    pfDPoolUnlock(DeviceInput->mouse);
    
    /* this is called in the APP so the window size has been updated. */
    pfGetPWinSize(DeviceInput->ichan[chan].pipeWin, &(mouse.winSizeX),
		  &(mouse.winSizeY));
    
    /* hack for internal event Q */
    events.buttonFlags = DeviceInput->eq->eventP->buttonFlags &
	  PFUDEV_MOD_MASK;
    pfuCollectXEventStream(DeviceInput->ichan[chan].dsp, &events, &mouse, 
	    DeviceInput->handlerMask, DeviceInput->userHandler);
	    
    /* Copy mouse structure into DeviceInput */
    pfDPoolLock(DeviceInput->mouse);
    *DeviceInput->mouse = mouse;	
    pfDPoolUnlock(DeviceInput->mouse);

    /* accumulate structure into DeviceInput with events locked */
    if (DeviceInput->resetEvents)
    {
	pfuResetEventQ(DeviceInput->eq);
	DeviceInput->resetEvents = 0;
    }
    pfuAppendEventQStream(DeviceInput->eq, &events);
    pfuIncEventQFrame(DeviceInput->eq);
}

void
pfuCollectXEventStream(pfWSConnection dsp,pfuEventStream *events, pfuMouse *mouse, 
		int handlerMask, pfuEventHandlerFuncType handlerFunc)
{
    static char	 	charbuf[10];
    XEvent		event;
    int			keymod;
    KeySym		keysym;
    pfuMouse		junkMouse;
    int			haveMouse = 1;
#if 0
    int			x, y;
#endif
    
    keymod = events->buttonFlags & PFUDEV_MOD_MASK;
    
    if (!mouse)
    {
	mouse = &junkMouse;
	haveMouse = 0;
    }
    
    mouse->click = 0;
    mouse->release = 0;

    /*
    // In Linux, XEventsQueued with QueuedAlready results in an
    // extreme amount of latency in SP mode since the event queue
    // only ever reports one event per call.  It's necessary to
    // change this to re-query the X server for more events after
    // the first one is pulled from the queue.  However this results
    // in a server roundtrip (in Linux) and probably causes a performance
    // hit.  So only being defined for LINUX
    */
#ifdef __linux__
    while (XPending(dsp)) 
#else
    while (XEventsQueued(dsp, QueuedAlready)) 
#endif
    {
	XNextEvent(dsp, &event);
	
next_X_event:

	if ((handlerMask == PFUINPUT_CATCH_ALL) && handlerFunc)
	{
	    callUserHandler((unsigned long) dsp, (void*) &event);
	} 
	else switch (event.type) 
	{
	    case MotionNotify: 
	    { /* toss a block of current mouse motion events in the Q all at once */
		int flag = 0, doLoop = 1;
		XEvent		tmpEvent;
		havePotentialFocus = 1;
		mouse->inWin = event.xmotion.window;

		do {
		    if (event.type == MotionNotify)
		    {
			tmpEvent = event;
			if (XEventsQueued(dsp, QueuedAlready))
			    XNextEvent(dsp, &event);
			else
			    doLoop = 0;
		    }
		    else
		    {
			flag = 1;
			doLoop = 0;
		    }
		} while (doLoop);
		    
		if ((handlerMask & PointerMotionMask) && handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &tmpEvent);
		} else 
		{
		    XMotionEvent *motion_event = (XMotionEvent *) &tmpEvent;
		    /* Get WINDOW relative mouse position */
		    mouse->xpos =  motion_event->x;
		    mouse->ypos =  mouse->winSizeY - motion_event->y;
		    mouse->posTime =  pfuMapXTime(motion_event->time);
		    if (mouse->flags)
		    {
			if (motion_event->state & ShiftMask)
			    keymod |= PFUDEV_MOD_SHIFT;
			if (motion_event->state & LockMask)
			    keymod |= PFUDEV_MOD_CAPS_LOCK;
			if (motion_event->state & ControlMask)
			    keymod |= PFUDEV_MOD_CTRL;
			if (motion_event->state & (Mod1Mask | Mod2Mask))
			    keymod |= PFUDEV_MOD_ALT;
		    }
		    /* 
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
		    "motion: %d %d=(%d) 0x%x 0x%x",
			mouse->xpos, mouse->ypos, 
			motion_event->y, 
			keymod & PFUDEV_MOD_MASK, 
			motion_event->state);
		    */    
		}
		if (flag)
			goto next_X_event;
	    }
	    break;
	   
	    case ClientMessage:
		if (events)
		{
		    if ((event.xclient.message_type == wm_protocols) &&
			(event.xclient.data.l[0] == wm_delete_window)) {
			events->devQ[events->numDevs] = PFUDEV_WINQUIT;
			events->devVal[events->numDevs] = 1;
			events->devCount[PFUDEV_WINQUIT] += 1;
			events->numDevs++;
		    } 
		}
		break;

	   case VisibilityNotify:
		if ((handlerMask & ExposureMask) && handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &event);
		} 
		else if (events)
		{
		    events->devQ[events->numDevs] = PFUDEV_REDRAW;
		    events->devVal[events->numDevs] = 1;
		    events->devCount[PFUDEV_REDRAW] += 1;
		    events->numDevs++;
		}
		havePotentialFocus = 1;
		break;
	   case ConfigureNotify: 
		if ((handlerMask & StructureNotifyMask) && handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &event);
		} 
		else if (events)
		{
		    /*
		    do {
		    }while (XCheckMaskEvent(dsp, StructureNotifyMask, &event));
		    */
		    events->devQ[events->numDevs] = PFUDEV_REDRAW;
		    events->devVal[events->numDevs] = 1;
		    events->devCount[PFUDEV_REDRAW] += 1;
		    events->numDevs++;
		}
		mouse->winSizeX = event.xconfigure.width;
		mouse->winSizeY = event.xconfigure.height;
		havePotentialFocus = 1;
		break;
	   case Expose:
		if ((handlerMask & ExposureMask) && handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &event);
		} 
		else if (events)
		{
		    do {
		    }while (XCheckMaskEvent(dsp, ExposureMask, &event));
		    events->devQ[events->numDevs] = PFUDEV_REDRAW;
		    events->devVal[events->numDevs] = 1;
		    events->devCount[PFUDEV_REDRAW] += 1;
		    events->numDevs++;
		}
		havePotentialFocus = 1;
		break;

	   case DestroyNotify:
		if ((handlerMask & StructureNotifyMask) && handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &event);
		} 
		else if (events)
		{
		    events->devQ[events->numDevs] = PFUDEV_WINQUIT;
		    events->devVal[events->numDevs] = 1;
		    events->devCount[PFUDEV_WINQUIT] += 1;
		    events->numDevs++;
		}
		break;

	   case FocusIn:
	   case FocusOut:
		 do {
		    }while (XCheckMaskEvent(dsp, StructureNotifyMask, &event));
		if (event.type == FocusOut)
		    mouse->inWin = 0;
		else
		    mouse->inWin = event.xfocus.window;
		if ((handlerMask & FocusChangeMask) && handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &event);
		}
		break;

	   case ButtonPress: 
		{
		    XButtonEvent *button_event = (XButtonEvent *) &event;

		    mouse->xpos = event.xbutton.x;
		    mouse->ypos = mouse->winSizeY - event.xbutton.y;
		    mouse->posTime = pfuMapXTime(button_event->time);
		    mouse->inWin = event.xbutton.window;
		    switch (button_event->button) {
			case Button1:
			    pfuMouseButtonClick(mouse, PFUDEV_MOUSE_LEFT_DOWN, 
				mouse->xpos, mouse->ypos, mouse->posTime);
			    break;
			case Button2:
			    pfuMouseButtonClick(mouse, PFUDEV_MOUSE_MIDDLE_DOWN, 
				mouse->xpos, mouse->ypos, mouse->posTime);
			    break;
			case Button3:
			    pfuMouseButtonClick(mouse, PFUDEV_MOUSE_RIGHT_DOWN, 
				mouse->xpos, mouse->ypos, mouse->posTime);
			    break;
		    }
		    if (button_event->state & ShiftMask)
			keymod |= PFUDEV_MOD_SHIFT;
		    if (button_event->state & LockMask)
			keymod |= PFUDEV_MOD_CAPS_LOCK;
		    if (button_event->state & ControlMask)
			keymod |= PFUDEV_MOD_CTRL;
		    if (button_event->state & (Mod1Mask | Mod2Mask))
			keymod |= PFUDEV_MOD_ALT;
		}
		if ((handlerMask & ButtonPressMask) && handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &event);
		}
		havePotentialFocus = 1;
		break;

	   case ButtonRelease:  
		{
		    XButtonEvent *button_event = (XButtonEvent *) &event;
		    /*
                    // hack for buggy XFree86 input
                    // -- if we get any mouse release, release them all.
                    // (except the middle.. it's harmless)
		    */
                    #ifdef __linux__
                    static int buttonwarn = 1;
                    if (buttonwarn)
                        pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
                            "Using experimental pfuMouse hack for XFree86\n");
                    buttonwarn=0;
                    pfuMouseButtonRelease(mouse, PFUDEV_MOUSE_LEFT_DOWN, 
                        mouse->xpos, mouse->ypos, mouse->posTime);
                    pfuMouseButtonRelease(mouse, PFUDEV_MOUSE_RIGHT_DOWN, 
                        mouse->xpos, mouse->ypos, mouse->posTime);
                    #endif
		    switch (button_event->button) {
			case Button1:
			    pfuMouseButtonRelease(mouse, PFUDEV_MOUSE_LEFT_DOWN, 
				mouse->xpos, mouse->ypos, mouse->posTime);
			    break;
			case Button2:
			    pfuMouseButtonRelease(mouse, PFUDEV_MOUSE_MIDDLE_DOWN, 
				mouse->xpos, mouse->ypos, mouse->posTime);
			    break;
			case Button3:
			    pfuMouseButtonRelease(mouse, PFUDEV_MOUSE_RIGHT_DOWN, 
				mouse->xpos, mouse->ypos, mouse->posTime);
			    break;
		    }
		    mouse->flags &= ~(PFUDEV_MOD_MASK);
		}
		if ((handlerMask & ButtonPressMask) && handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &event);
		}
		break;

	   case KeyPress:
		havePotentialFocus = 1;
		if ((handlerMask & KeyPressMask) && handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &event);
		} 
		else if (events)
		{
		    XKeyEvent *key_event = (XKeyEvent *) &event;
		    short val = key_event->keycode;
		    int ccount = XLookupString(key_event, charbuf, 10, &keysym, 0);
		    
		    /* 
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "CollectXInput: PRESS keysym 0x%x", keysym);
		     */
		    if (ccount == 1)
		    { /* if is ascii key */
			val = charbuf[0];
			/* pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "have ascii key %d=%c", val, val); */
			if (events->numKeys >= PFUDEV_MAX_INKEYS -1)
			    break;
			
			if (!(events->devCount[PFUDEV_KEYBD]))
			{
			    /* just record std keybd device once */
			    events->devQ[events->numDevs] = PFUDEV_KEYBD;
			    events->devCount[PFUDEV_KEYBD] += 1;
			    events->numDevs++;
			}
			
			events->keyQ[events->numKeys] = val;
			events->keyCount[val] += 1;
			events->numKeys++;
			
		    } else if ((keysym >= XK_F1) && (keysym <= XK_F12))
		    { /* is function key */
			int dev = (int)keysym - XK_F1 + 1;
			events->devQ[events->numDevs] = dev;
			events->devVal[events->numDevs] = 1;
			events->devCount[dev] += 1;
			events->numDevs++;
		    }
		    else if ((keysym >= XK_Left) && (keysym <= XK_Down))
		    { /* Arrow Key */
			int dev;
			switch(keysym)
			{
			case XK_Left: 
			    events->devQ[events->numDevs] = dev = 
					PFUDEV_LEFTARROWKEY;
			    events->devVal[events->numDevs] = val;
			    events->devCount[dev] += 1;
			    events->numDevs++;
			    break;
			case XK_Right:
			    events->devQ[events->numDevs] = dev = 
					PFUDEV_RIGHTARROWKEY;
			    events->devVal[events->numDevs] = val;
			    events->devCount[dev] += 1;
			    events->numDevs++;
			    break;
			case XK_Up: 
			    events->devQ[events->numDevs] = dev = 
					PFUDEV_UPARROWKEY;
			    events->devVal[events->numDevs] = val;
			    events->devCount[dev] += 1;
			    events->numDevs++;
			    break;
			case XK_Down:
			    events->devQ[events->numDevs] = dev = 
					PFUDEV_DOWNARROWKEY;
			    events->devVal[events->numDevs] = val;
			    events->devCount[dev] += 1;
			    events->numDevs++;
			    break;
			default: break;
			}
		    } 
		    else if ((keysym >= XK_Shift_L) && (keysym <= XK_Alt_R))
		    { /* is modifier key */	
			switch(keysym)
			{
			case XK_Shift_L: 
			    keymod |= PFUDEV_MOD_LEFT_SHIFT_SET; 
			    break;
			case XK_Shift_R: 
			    keymod |= PFUDEV_MOD_RIGHT_SHIFT_SET; 
			    break;
			case XK_Control_L: 
			    keymod |= PFUDEV_MOD_LEFT_CTRL_SET; 
			    break;
			case XK_Control_R: 
			    keymod |= PFUDEV_MOD_RIGHT_CTRL_SET; 
			    break;
			case XK_Alt_L: 
			    keymod |= PFUDEV_MOD_LEFT_ALT_SET; 
			    break;
			case XK_Alt_R: 
			    keymod |= PFUDEV_MOD_RIGHT_ALT_SET; 
			    break;
			case XK_Meta_L:
			    keymod |= PFUDEV_MOD_LEFT_ALT_SET 
					| PFUDEV_MOD_LEFT_SHIFT_SET; 
			    break;
			case XK_Meta_R:
			    keymod |= PFUDEV_MOD_RIGHT_ALT_SET 
					| PFUDEV_MOD_RIGHT_SHIFT_SET; 
			    break;
			default: 
			    break;
			}
		    }
		}
		break;
	    case KeyRelease:
		havePotentialFocus = 1;
		if ((handlerMask & KeyPressMask) && handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &event);
		} 
		else if (events)
		{
		    XKeyEvent *key_event = (XKeyEvent *) &event;
		    int ccount = XLookupString(key_event, charbuf, 10, &keysym, 0);
		    
		    if ((keysym >= XK_Shift_L) && (keysym <= XK_Alt_R))
		    { /* is modifier key */
			switch(keysym)
			{
			case XK_Shift_L: 
			    keymod &= ~PFUDEV_MOD_LEFT_SHIFT; 
			    break;
			case XK_Shift_R: 
			    keymod &= ~PFUDEV_MOD_RIGHT_SHIFT; 
			    break;
			case XK_Control_L: 
			    keymod &= ~PFUDEV_MOD_LEFT_CTRL; 
			    break;
			case XK_Control_R: 
			    keymod &= ~PFUDEV_MOD_RIGHT_CTRL; 
			    break;
			case XK_Alt_L: 
			    keymod &= ~PFUDEV_MOD_LEFT_ALT; 
			    break;
			case XK_Alt_R: 
			    keymod &= ~PFUDEV_MOD_RIGHT_ALT; 
			    break;
			case XK_Meta_L:
			    keymod &= ~(PFUDEV_MOD_LEFT_ALT 
					    | PFUDEV_MOD_LEFT_SHIFT); 
			    break;
			case XK_Meta_R:
			    keymod &= ~(PFUDEV_MOD_RIGHT_ALT 
					    | PFUDEV_MOD_RIGHT_SHIFT); 
			    break;
			default: 
			    break;
			}
		    }
		}
		break;
	    default:
		if (handlerFunc)
		{
		    callUserHandler((unsigned long) dsp, (void*) &event);
		}
		break;
	}
    }
    if (!(keymod & (PFUDEV_MOD_RIGHT_ALT | PFUDEV_MOD_LEFT_ALT)))
	keymod &= ~PFUDEV_MOD_ALT;
    if (!(keymod & (PFUDEV_MOD_RIGHT_SHIFT | PFUDEV_MOD_LEFT_SHIFT)))
	keymod &= ~PFUDEV_MOD_SHIFT;
    if (!(keymod & (PFUDEV_MOD_RIGHT_CTRL | PFUDEV_MOD_LEFT_CTRL)))
	keymod &= ~PFUDEV_MOD_CTRL;

    
    if (events)
	events->buttonFlags = (events->buttonFlags & ~PFUDEV_MOD_MASK) | keymod;

    if (haveMouse)
    {
	if (mouse->flags & PFUDEV_MOUSE_DOWN_MASK)
	    mouse->flags = (mouse->flags & ~PFUDEV_MOD_MASK) | keymod; 
	else
	    mouse->flags &= ~PFUDEV_MOD_MASK;
    
	mouse->modifiers = keymod;
    
#if 0
	x = mouse->xpos;
	y = mouse->ypos;
#endif

	if (mouse->inWin || havePotentialFocus)
	{ /* this gets around an bug where no INPUTCHANGE event gets recorded
	   * when the window starts up with the mouse in the window, 
	   * where mouse is considered in the window when it is really on the
	   * top or size boarder geometry, 
	   * and several other similar type situations.
	   */
	    if ((!(mouse->flags & PFUDEV_MOUSE_DOWN_MASK) ||
		/* if mouse is down - must have clicked in window */
		    ((mouse->clickPosLast[0] >= 0) &&
			(mouse->clickPosLast[0] <= (mouse->winSizeX))) &&
		    ((mouse->clickPosLast[1] >= 0) && 
			(mouse->clickPosLast[1] <= (mouse->winSizeY))))
		)
	    {
		/* don't clobber possible recorded window value in mouse->inWin */
		if (!mouse->inWin)
		    mouse->inWin = 1;
		havePotentialFocus = 0;
	    } 
	    else 
	    {
		mouse->inWin = 0;
	    }
	}
    }
}

double
pfuMapXTime(double xtime)
{
    
    static double xtimeOffset=0.0f;
    
    if (xtimeOffset == 0.0f)
    {
	double t = pfGetTime();
	xtimeOffset = (t * .001) - xtime;
    }
    return (xtime + xtimeOffset);
}
