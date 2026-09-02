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
 * $Revision: 1.165 $
 *
 */

#include <stdlib.h>
#include <stdarg.h>
#ifndef WIN32
#include <unistd.h>
#endif
#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#endif 
#include <signal.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/sysmp.h>
#endif
#if defined(__linux__has_schedctl_h__) || defined(mips) || defined(__win32__has_schedctl_h__)
#include <sys/schedctl.h>
#endif 
#ifndef WIN32
#include <sys/prctl.h>
#endif

#ifdef	_POSIX_SOURCE
#ifdef	_LANGUAGE_C_PLUS_PLUS
extern void (*sigset (int sig, void (*disp)(...)))(...);
extern void sginap(int);
#else
extern void (*sigset (int sig, void (*disp)()))();
extern long sginap(long);
#endif
#endif

#ifndef WIN32
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#endif // WIN32

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
#ifndef WIN32
static void	mpXInput(int chan);
static void	collectXInput(int chan);
#else
static void     openWin32Input(int chan);
#endif
static void	callUserHandler(unsigned long _dev, void *_val);
#ifdef WIN32
static void     initWin32Input(void); 
LRESULT CALLBACK collectWin32Input(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
PFUDLLEXPORT void pfuCollectWin32EventStream(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam,
					   pfuEventStream *events, pfuMouse *mouse,
					   int handlerMask, pfuEventHandlerFuncType handlerFunc);
typedef struct _pfuWin32Event {
  HWND hwnd;
  UINT uMsg;
  WPARAM wParam; // we should copy this and lParam
  LPARAM lParam;
  double time;
} pfuWin32Event;

#endif /* WIN32 */

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

#ifdef WIN32
LRESULT CALLBACK collectWin32InputWrapper(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  int i;

  collectWin32Input(hwnd,uMsg,wParam,lParam);

  for(i=0; i<DeviceInput->numChan; i++) {
    if(hwnd == DeviceInput->ichan[i].wnd[0])
      return CallWindowProc(DeviceInput->ichan[i].oldWndProc[0],hwnd,uMsg,wParam,lParam);
    else if(hwnd == DeviceInput->ichan[i].wnd[1])
      return CallWindowProc(DeviceInput->ichan[i].oldWndProc[1],hwnd,uMsg,wParam,lParam);
  }

  /* should never happen ... */
  pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"Default window handler being invoked for event 0x%x",hwnd);

  return DefWindowProc(hwnd,uMsg,wParam,lParam);

}
#endif

#ifdef WIN32
static void displayWarningAboutNULLHWND(const char *funcName,const char *str)
{
  //
  // this notifies the user that pfFrame() has not been called and
  // hence the window has not been realized. The call to SetWindowLong
  // won't fail, but it won't have any effect either. Basically
  // what this means is that input won't work.
  //

  pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
	   "%s(), NULL WS %s.\n",funcName,str);
  pfNotify(PFNFY_DEBUG,PFNFY_MORE,
	   "could it be that pfFrame() has not been called?");
  pfNotify(PFNFY_DEBUG,PFNFY_MORE,
	   "can't initalize input until window has been realized.");
}
#endif

PFUDLLEXPORT void
pfuInitMultiChanInput(pfChannel *Chan[], int NumChans, int mode)
{
    pfPipeWindow	*pw;
    pid_t	    	fpid;
    int			chan=0;
#ifdef WIN32
    HWND hwnd;
#endif

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
		     "pfuInitMultiChanInput() Null pfPipeWindow.");
	    continue;
	}

	DeviceInput->ichan[chan].pipeWin = pw;
	DeviceInput->ichan[chan].pipe = pfGetPWinPipe(pw);
	DeviceInput->ichan[chan].xWin = pfGetPWinWSWindow(pw);

#ifdef WIN32
	/*
	 * Force input to get re-initialized if user calls this func 
         * more than once.
	 */
        DeviceInput->inited[chan] = 0;
	openWin32Input(chan);
	if(mode != PFUINPUT_NOFORK_X) {
	  pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"pfuInitMultiChanInput(): %s"
		" invalid input mode on win32. Setting mode to PFUINPUT_NOFORK_X",
		mode == PFUINPUT_GL ? "PFUINPUT_GL" : "PFUINPUT_X");
          mode = PFUINPUT_NOFORK_X;
        }
#endif
    
	switch(mode) 
	{
#ifndef WIN32
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
#else /* WIN32 */
	case PFUINPUT_NOFORK_X:
	  DeviceInput->mode = PFUINPUT_NOFORK_X;
	  DeviceInput->numChan = NumChans;
	  /* don't break ... */
	case PFUINPUT_X:
	  DeviceInput->mode = PFUINPUT_X;
	  DeviceInput->forked = 0;
	  break;
#endif /* WIN32 */
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

PFUDLLEXPORT void
pfuInitInput(pfPipeWindow *pw, int mode)
{
    pid_t	    fpid;
#ifdef WIN32
    HWND hwnd;
#endif

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

#ifdef WIN32
    /* see comment in pfuInitMultiChanInput() about this variable ... */
    DeviceInput->inited[0] = 0;
    openWin32Input(0);

        if(mode == PFUINPUT_NOFORK_X || mode == PFUINPUT_X) {
          pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
                "pfuInitInput(): %s"
                " invalid input mode on win32. Setting mode to PFUINPUT_NOFORK_X",
                mode == PFUINPUT_X ? "PFUINPUT_X" : "PFUINPUT_GL");
          mode = PFUINPUT_NOFORK_X;
        }
#endif

    switch(mode) {
      case PFUINPUT_NOFORK_X:
	DeviceInput->mode = PFUINPUT_X;
	break;
      case PFUINPUT_X:
        DeviceInput->mode = PFUINPUT_X;
#ifdef WIN32
	DeviceInput->forked = 0;
#else
	DeviceInput->forked = 1;
	if ((fpid = fork()) < 0)
	    pfNotify(PFNFY_FATAL, PFNFY_SYSERR, "pfuInitInput() Fork failed.");
	else if (!fpid)
	{
	    mpXInput(0);
	}
#endif
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

PFUDLLEXPORT
void pfuInputHandler(pfuEventHandlerFuncType userFunc, 
		    unsigned int mask)
{
    if (!DeviceInput)
    	initDataPool();

    DeviceInput->userHandler = userFunc;
    DeviceInput->handlerMask = mask;
}

PFUDLLEXPORT void 
pfuExitInput(void)
{
    if (DeviceInput)
	DeviceInput->exit = 1;
}

/* pfuCalcNormalizedChanXY - normalize window relative x,y to full chan size 
 * for input
 */
PFUDLLEXPORT void
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
PFUDLLEXPORT void
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
PFUDLLEXPORT int
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

PFUDLLEXPORT int
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
PFUDLLEXPORT void
pfuGetMouse(pfuMouse* mouse)
{    
    int		    chan;
    
    if (!DeviceInput)
    	initDataPool();

    /* no need to check if there are any events queued on win32 
     * so this entire block of code is unecessary for this platform
     */
    for (chan = 0; chan < DeviceInput->numChan; chan++)
    {
#ifdef WIN32
      if(!DeviceInput->inited[chan])
	openWin32Input(chan);
#else
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
#endif
    }

    pfDPoolLock(DeviceInput->mouse);
    *mouse = *DeviceInput->mouse;
    pfDPoolUnlock(DeviceInput->mouse);
}


/*
 * Copy internal pfuEventQueue structure into 'event'. Also reset internal
 * pfuEventQueue structure.
*/
PFUDLLEXPORT void
pfuGetEvents(pfuEventStream* events)
{
    int dfs, efs;
    int		    chan;
    int		    events_found = 0;

#ifdef DO_TIME
    float start_time, acquire_time, total_time;
#endif    
    if (!DeviceInput)
    	initDataPool();

    for (chan = 0; chan < DeviceInput->numChan; chan++)
    {
#ifndef WIN32
	if (!DeviceInput->forked && 
	    (DeviceInput->mode == PFUINPUT_X) && 
	    !DeviceInput->inited[chan])
	    openXInput(chan);
	if (!DeviceInput->inited[chan])
	{
	    events -> numDevs = 0;
	    return;
	}
#else
	if(!DeviceInput->inited[chan])
	  openWin32Input(chan);
#endif

#ifndef WIN32
	if ((DeviceInput->mode != PFUINPUT_GL) && 
	    (!DeviceInput->forked &&
	     (XEventsQueued(DeviceInput->ichan[chan].dsp, QueuedAfterFlush))))
	    collectXInput(chan);
	/* 
	 * no need to call collectXXXInput() on win32 because there will
	 * never been any pending events
	 *
	 */
#endif

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
PFUDLLEXPORT void
pfuCollectInput(void)
{
    if (!DeviceInput)    
	initDataPool();

    switch(DeviceInput->mode) {
    case PFUINPUT_X:
	/* Do nothing, input is automatically collected */
	break;
    case PFUINPUT_GL:
#ifndef WIN32
      /* XXX Alex -- re-enable this warning later ... */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	"pfuCollectInput: running OpenGL - must use X input.");
#endif
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


PFUDLLEXPORT void
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

PFUDLLEXPORT void
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
 
#ifndef WIN32
static Atom	wm_protocols, wm_delete_window;
#endif
static long	    theEventmask;

#ifdef WIN32
static void
openWin32Input(int chan)
{
  HWND hwnd;
  int initializedWins = 1;
  pfPipeWindow *pw = DeviceInput->ichan[chan].pipeWin;

  if (DeviceInput->inited[chan])
    return; // no need to go any further ...

  /* store pointer to old wndprocs in ichan */
  hwnd = pfGetWinWSDrawable(pfGetPWinSelect(pw));
  if(hwnd == NULL) {
    displayWarningAboutNULLHWND("openWin32Input","Drawable");
    initializedWins = 0;
  } else {
    DeviceInput->ichan[chan].wnd[0] = hwnd;
    DeviceInput->ichan[chan].oldWndProc[0] =
      SetWindowLong(hwnd, GWL_WNDPROC, (LONG) collectWin32InputWrapper);
  }
  
  hwnd = pfGetWinWSWindow(pfGetPWinSelect(pw));
  if(hwnd == NULL) {
    displayWarningAboutNULLHWND("openWin32Input","Window");
    initializedWins = 0;
  } else {
    DeviceInput->ichan[chan].wnd[1] = hwnd;
    DeviceInput->ichan[chan].oldWndProc[1] =
      SetWindowLong(hwnd, GWL_WNDPROC, (LONG) collectWin32InputWrapper);
  }

  if(initializedWins == 1) {
    DeviceInput->inited[chan] = 1;
    /* reset the two following, just in case ... */
    DeviceInput->ichan[chan].pipe = pfGetPWinPipe(pw);
    DeviceInput->ichan[chan].xWin = pfGetPWinWSWindow(pw);
  }
}
#endif

static void
openXInput(int chan)
{
#ifdef WIN32
    pfNotify(PFNFY_WARN,PFNFY_PRINT,"openXInput() NYI on win32");
#else
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
#endif
}

#ifndef WIN32
static void
mpXInput(int chan)
{
    XEvent	    event;
    Window	    xWin;
    int		    haveOpenWin = 0;

    #if defined(sgi)
    prctl(PR_TERMCHILD);        /* Exit when parent does */
    #else
    #ifdef __linux__
	prctl(PR_SET_PDEATHSIG, SIGHUP, 0, 0, 0);
    #endif /* __linux__ */
    #endif  /* sgi*/
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
#endif

/*
 * Flush input queue and update the shared pfuMouse and pfuEventQueue structures.
 */
#ifndef WIN32
static void
collectXInput(int chan)
#else
LRESULT CALLBACK
collectWin32Input(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
#endif
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
#ifndef WIN32
    pfGetPWinSize(DeviceInput->ichan[chan].pipeWin, &(mouse.winSizeX),
		  &(mouse.winSizeY));
#else
    /* XXX Alex -- we are setting the mouse's winSizeX and winSizeY
     * members by getting an WM_SIZE callback so there is no need to
     * do it here. Besides, we would have to loop over all of the channels
     * to determine which one we need to get the size from if we wanted
     * to do it here so ... let's just not do it.
     */
#if 0
    pfGetPWinSize(DeviceInput->ichan[0].pipeWin, &(mouse.winSizeX),
		  &(mouse.winSizeY));
#endif
#endif
    
    /* hack for internal event Q */
    events.buttonFlags = DeviceInput->eq->eventP->buttonFlags &
	  PFUDEV_MOD_MASK;
#ifdef WIN32
    pfuCollectWin32EventStream(hwnd, uMsg, wParam, lParam, &events, &mouse,
			       DeviceInput->handlerMask, DeviceInput->userHandler);
#else
    pfuCollectXEventStream(DeviceInput->ichan[chan].dsp, &events, &mouse, 
	    DeviceInput->handlerMask, DeviceInput->userHandler);
#endif
	    
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

#ifdef WIN32
    return 0; /* XXX Alex change this so that the pfuCollectWin32EventStream()
	      * function returns 1 or 0 depending on whether or not the 
	      * event was handled*/
#endif
}


#ifdef WIN32
PFUDLLEXPORT void
pfuCollectWin32EventStream(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam,
			   pfuEventStream *events, pfuMouse *mouse, 
			   int handlerMask, pfuEventHandlerFuncType handlerFunc)
{
    int			keymod;
    pfuMouse		junkMouse;
    int			haveMouse = 1;

    HWND parentWindow = hwnd;
    LPTSTR tmpName[101];
    pfuWin32Event event;

    event.hwnd = hwnd; /* or should this too point to the parentWindow, if any? */
    event.uMsg = uMsg;
    event.wParam = wParam;
    event.lParam = lParam;
    event.time = pfGetTime();

    /* we want the inWin member of the pfuMouse to be == to the
     * HWND for the pipe window regardless of whether or not
     * the event occurred in the child or parent of the pipe win
     * In theory this variable is just a bool but it's used in
     * some sample programs to check the window id of the pipe
     * window. gui.c works with just a bool but ...
     */
    if(GetClassName(hwnd,tmpName,100) > 0) {
      if(!strcmp(tmpName,"pfChildWNDCLASS"))
	parentWindow = (HWND)GetWindowLong(hwnd,GWL_HWNDPARENT);
      if(parentWindow == NULL)
	parentWindow = hwnd; /* and spew error message ... */
    } else {
      pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"Unable to determine class name of win = 0x%x",hwnd);
    }
    
    keymod = events->buttonFlags & PFUDEV_MOD_MASK;
    
    if (!mouse)
    {
      mouse = &junkMouse;
      haveMouse = 0;
    }
    
    mouse->click = 0;
    mouse->release = 0;

    if ((handlerMask == PFUINPUT_CATCH_ALL) && handlerFunc)
      callUserHandler((unsigned long) 0L, (void*) &event);
    else switch (uMsg) {
    case WM_MOUSEMOVE: 
      havePotentialFocus = 1;
      mouse->inWin = parentWindow;
      
      if ((handlerMask & PointerMotionMask) && handlerFunc)
	callUserHandler(0L, (void*) &event);
      else {
	/* Get WINDOW relative mouse position */
	short xxx = lParam & 0xffff, yyy = (lParam >> 16) & 0xffff ;
	mouse->xpos =  xxx;
	mouse->ypos =  mouse->winSizeY - yyy;
	mouse->posTime =  pfuMapXTime(event.time);
	
	if(GetKeyState(VK_SHIFT))
	  keymod |= PFUDEV_MOD_SHIFT;
	if(GetKeyState(VK_CAPITAL))
	  keymod |= PFUDEV_MOD_CAPS_LOCK;
	if(GetKeyState(VK_CONTROL))
	  keymod |= PFUDEV_MOD_CTRL;
	if(GetKeyState(VK_MENU))
	  keymod |= PFUDEV_MOD_ALT;
      }
      break;

    case WM_PAINT:
      if ((handlerMask & ExposureMask) && handlerFunc)
	  callUserHandler(0L, (void*) &event);
      else if (events)
	{
	  events->devQ[events->numDevs] = PFUDEV_REDRAW;
	  events->devVal[events->numDevs] = 1;
	  events->devCount[PFUDEV_REDRAW] += 1;
	  events->numDevs++;
	}
      havePotentialFocus = 1;
      break;
      
    case WM_SIZE:
      if ((handlerMask & StructureNotifyMask) && handlerFunc)
	  callUserHandler(0L, (void*) &event);
      else if (events) {
	events->devQ[events->numDevs] = PFUDEV_REDRAW;
	events->devVal[events->numDevs] = 1;
	events->devCount[PFUDEV_REDRAW] += 1;
	events->numDevs++;
      }
      mouse->winSizeX = LOWORD(lParam);
      mouse->winSizeY = HIWORD(lParam);
      havePotentialFocus = 1;
      break;
            
    case WM_QUIT:
      if ((handlerMask & StructureNotifyMask) && handlerFunc)
	  callUserHandler(0L, (void*) &event);
      else if (events) {
	events->devQ[events->numDevs] = PFUDEV_WINQUIT;
	events->devVal[events->numDevs] = 1;
	events->devCount[PFUDEV_WINQUIT] += 1;
	events->numDevs++;
      }
      break;
			
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
      if (uMsg == WM_KILLFOCUS)
	mouse->inWin = 0;
      else
        mouse->inWin = parentWindow;
      if ((handlerMask & FocusChangeMask) && handlerFunc)
	callUserHandler(0L, (void*) &event);
      break;
      
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
      SetCapture(parentWindow);
      {
	short xxx = lParam & 0xffff, yyy = (lParam >> 16) & 0xffff ;
	mouse->xpos =  xxx;
	mouse->ypos =  mouse->winSizeY - yyy;
      }
      mouse->posTime = pfuMapXTime(event.time);
      mouse->inWin = parentWindow;
      switch(uMsg) {
      case WM_LBUTTONDOWN:
	pfuMouseButtonClick(mouse, PFUDEV_MOUSE_LEFT_DOWN, 
			    mouse->xpos, mouse->ypos, mouse->posTime);
	break;
      case WM_MBUTTONDOWN:
	pfuMouseButtonClick(mouse, PFUDEV_MOUSE_MIDDLE_DOWN, 
			    mouse->xpos, mouse->ypos, mouse->posTime);
	break;
      case WM_RBUTTONDOWN:
	pfuMouseButtonClick(mouse, PFUDEV_MOUSE_RIGHT_DOWN, 
			    mouse->xpos, mouse->ypos, mouse->posTime);
	break;
      }
      if(GetKeyState(VK_SHIFT))
	keymod |= PFUDEV_MOD_SHIFT;
      if(GetKeyState(VK_CAPITAL))
	keymod |= PFUDEV_MOD_CAPS_LOCK;
      if(GetKeyState(VK_CONTROL))
	keymod |= PFUDEV_MOD_CTRL;
      if(GetKeyState(VK_MENU))
	keymod |= PFUDEV_MOD_ALT;
      
      if ((handlerMask & ButtonPressMask) && handlerFunc)
	callUserHandler(0L, (void*) &event);
      havePotentialFocus = 1;
      break;
      
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
      ReleaseCapture();
      switch(uMsg) {
      case WM_LBUTTONUP:
	pfuMouseButtonRelease(mouse, PFUDEV_MOUSE_LEFT_DOWN, 
			      mouse->xpos, mouse->ypos, mouse->posTime);
	break;
      case WM_MBUTTONUP:
	pfuMouseButtonRelease(mouse, PFUDEV_MOUSE_MIDDLE_DOWN, 
			      mouse->xpos, mouse->ypos, mouse->posTime);
	break;
      case WM_RBUTTONUP:
	pfuMouseButtonRelease(mouse, PFUDEV_MOUSE_RIGHT_DOWN, 
			      mouse->xpos, mouse->ypos, mouse->posTime);
	break;
      }
      mouse->flags &= ~(PFUDEV_MOD_MASK);
      if ((handlerMask & ButtonReleaseMask) && handlerFunc)
	callUserHandler(0L, (void*) &event);
      break;
      
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      havePotentialFocus = 1;
      if ((handlerMask & KeyPressMask) && handlerFunc)
	callUserHandler(0L, (void*) &event);
      else if (events) {
	/*
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
		 "CollectWin32Input: PRESS keysym 0x%x -> %c", wParam,wParam);
	*/
	if ((wParam >= VK_F1) && (wParam <= VK_F12)) { /* is function key. This won't catch F10 */
	  int dev = (int)wParam - VK_F1 + 1;
	  events->devQ[events->numDevs] = dev;
	  events->devVal[events->numDevs] = 1;
	  events->devCount[dev] += 1;
	  events->numDevs++;
	} else {
	  int dev;
	  switch(wParam) {
	    /* Eat keys that normally don't show up on other OS in here */
	  case VK_NUMLOCK:
	    break;
	  /* check for arrow keys or modifiers */
	  case VK_LEFT:
	    events->devQ[events->numDevs] = dev = PFUDEV_LEFTARROWKEY;
	    events->devVal[events->numDevs] = wParam;
	    events->devCount[dev] += 1;
	    events->numDevs++;
	    break;
	  case VK_RIGHT:
	    events->devQ[events->numDevs] = dev = PFUDEV_RIGHTARROWKEY;
	    events->devVal[events->numDevs] = wParam;
	    events->devCount[dev] += 1;
	    events->numDevs++;
	    break;
	  case VK_UP:
	    events->devQ[events->numDevs] = dev = PFUDEV_UPARROWKEY;
	    events->devVal[events->numDevs] = wParam;
	    events->devCount[dev] += 1;
	    events->numDevs++;
	    break;
	  case VK_DOWN:
	    events->devQ[events->numDevs] = dev = PFUDEV_DOWNARROWKEY;
	    events->devVal[events->numDevs] = wParam;
	    events->devCount[dev] += 1;
	    events->numDevs++;
	    break;
	  case VK_SHIFT:
	    /* Shift is special in that there is no bit in the message that specifies
	       if it's the right or left. We have to compare scan codes here */
	    {
	      /* The scan code is bits 16->23 of lParam */
	      UINT eventScanCode = (lParam & 0x00ff0000) >> 16;
	      UINT virtualKey = MapVirtualKey(eventScanCode, 3);
	      if(virtualKey == VK_LSHIFT) {
		keymod |= PFUDEV_MOD_LEFT_SHIFT_SET; 
	      }
	      else if(virtualKey == VK_RSHIFT) {
		keymod |= PFUDEV_MOD_RIGHT_SHIFT_SET;
	      }
	      /* otherwise some unknown shift is going down, ignore it */
	    }
	    break;
	  case VK_CONTROL:
	    if(!(lParam & (1 << 24))) { /* 24th bit of lParam indicates if it's a right ctrl */
	      keymod |= PFUDEV_MOD_LEFT_CTRL_SET; 
	    }
	    else {
	      keymod |= PFUDEV_MOD_RIGHT_CTRL_SET; 
	    }
	    break;
	  case VK_MENU:
	    if(!(lParam & (1 << 24))) { /* 24th bit of lParam indicates if it's a right alt */
	      keymod |= PFUDEV_MOD_LEFT_ALT_SET; 
	    }
	    else {
	      keymod |= PFUDEV_MOD_RIGHT_ALT_SET; 
	    }
	    break;
	  default:
	    /* and finally check for all other key combos */
	    if (events->numKeys >= PFUDEV_MAX_INKEYS -1)
	      break;
	  
	    if (!(events->devCount[PFUDEV_KEYBD])) { /* just record std keybd device once */
	      events->devQ[events->numDevs] = PFUDEV_KEYBD;
	      events->devCount[PFUDEV_KEYBD] += 1;
	      events->numDevs++;
	    }
	    
	    /* Now, translate keyboard state into a character */
	    {
	      char keyState[256];
	      char returnBuf[2];
	      int retVal;
	      if(!GetKeyboardState(keyState))
		break;
	      
	      /* Convert virtual key kode to an ascii character */
	      retVal = ToAscii(wParam,  /* virtual code */
			       (lParam & 0x00ff0000) >> 16, /* scan code is in bits 16-23 of lParam */
			       keyState,
			       returnBuf,
			       0);
	      
	      if( (retVal == 0) || (retVal == 2)) {
		/* no translation possible or translation resulted in 2 keys (why?!?) */
		break;
	      }
	      
	      /* Now, we should have an ascii value for the key */
	      dev = returnBuf[0];
	      
	    }

	    events->keyQ[events->numKeys] = dev;
	    events->keyCount[dev] += 1;
	    events->numKeys++;

	    break;
	  } /* end switch */
	}
      }
      break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
      havePotentialFocus = 1;
      if ((handlerMask & KeyPressMask) && handlerFunc)
	callUserHandler(0L, (void*) &event);
      else if (events) {
	switch(wParam) {
	case VK_SHIFT:
	  /* Shift is special in that there is no bit in the message that specifies
	     if it's the right or left. We have to compare scan codes here */
	  {
	    /* The scan code is bits 16->23 of lParam */
	    UINT eventScanCode = (lParam & 0x00ff0000) >> 16;
	    UINT virtualKey = MapVirtualKey(eventScanCode, 3);
	    if(virtualKey == VK_LSHIFT) {
	      keymod &= ~PFUDEV_MOD_LEFT_SHIFT_SET; 
	    }
	    else if(virtualKey == VK_RSHIFT) {
	      keymod &= ~PFUDEV_MOD_RIGHT_SHIFT_SET;
	    }
	    else {/* some unknown shift is going up. Untoggle both to be safe. This shouldn't happen */ 
	      keymod &= (PFUDEV_MOD_LEFT_SHIFT_SET | PFUDEV_MOD_RIGHT_SHIFT_SET);
	    }
	  }
	  break;
	case VK_CONTROL:
	  if(!(lParam & (1 << 24))) { /* 24th bit of lParam indicates if it's a right control */
	    keymod &= ~PFUDEV_MOD_LEFT_CTRL_SET; 
	  }
	  else {
	    keymod &= ~PFUDEV_MOD_RIGHT_CTRL_SET; 
	  }
	  break;
	case VK_MENU:
	  if(!(lParam & (1 << 24))) { /* 24th bit of lParam indicates if it's a right alt */
	    keymod &= ~PFUDEV_MOD_LEFT_ALT_SET; 
	  }
	  else {
	    keymod &= ~PFUDEV_MOD_RIGHT_ALT_SET; 
	  }
	  break;
	default: 
	  break;
	}
      }
    default:
      if (handlerFunc)
	callUserHandler(0L, (void*) &event);
      break;
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
#endif /* WIN32 */

PFUDLLEXPORT void
pfuCollectXEventStream(pfWSConnection dsp,pfuEventStream *events, pfuMouse *mouse, 
		int handlerMask, pfuEventHandlerFuncType handlerFunc)
{
#ifdef WIN32
    pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfuCollectXEventStream() NYI on win32");
    pfNotify(PFNFY_WARN,PFNFY_MORE, "use pfuCollectWin32EventStream() instead.");
#else
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
		if ((handlerMask & ButtonReleaseMask) && handlerFunc)
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
#endif /* WIN32 */
}

PFUDLLEXPORT double
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
