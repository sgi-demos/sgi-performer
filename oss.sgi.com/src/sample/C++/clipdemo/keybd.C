/*
 * Copyright 1998, 1999, 2000, Silicon Graphics, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h> // for sigset for forked X event handler process 
#include <getopt.h> // for cmdline handler 
#include <X11/keysym.h>

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfPipe.h>
#include <Performer/pf/pfChannel.h>

#include <Performer/pr/pfLight.h>

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#include "clipdemo.h"
#include "pov.h"

void GetXInput(pfWSConnection);

void 
InitXInput(pfWSConnection dsp)
{
    Window w;
    
    /* wait for X Window to exist in Performer shared memory */
   if (w = Shared->pw[0]->getWSWindow())
   {
	XSelectInput(dsp, w, PointerMotionMask |
			ButtonPressMask | ButtonReleaseMask | 
			KeyPressMask | KeyReleaseMask);
	XMapWindow(dsp, w);
	XFlush(dsp);
	Shared->XInputInited = 1;
    }
}

//
// DoXInput() runs an asychronous forked even handling process.
//  Shared memory structures can be read from this process
//  but NO performer calls that set any structures should be 
//  issues by routines in this process.

void
DoXInput(void)
{
    // windows from draw should now exist so can attach X input handling
    // to the X window 
    
    Display *dsp = pfGetCurWSConnection();
    
    prctl(PR_TERMCHILD);        // Exit when parent does 
    sigset(SIGHUP, SIG_DFL);    // Exit when sent SIGHUP by TERMCHILD 
    
    InitXInput(dsp);
    
    while (1)
    {
	XEvent          event;
	if (!Shared->XInputInited)
	    InitXInput(dsp);
	if (Shared->XInputInited)
	{
	    XPeekEvent(dsp, &event);
		GetXInput(dsp);
	}
    }
}

void
GetXInput(pfWSConnection dsp)
{
    static int x=640, y=512;
    
    if (XEventsQueued(dsp, QueuedAfterFlush))
    while (XEventsQueued(dsp, QueuedAlready))
    {
	XEvent event;
	
	XNextEvent(dsp, &event);
	
	switch (event.type) 
	{
	case ConfigureNotify:
	    break;
	case FocusIn:
	    Shared->inWindow = 1;
	    break;
	case FocusOut:
	    Shared->inWindow = 0;
	    break;
	case MotionNotify: 
	    {
		XMotionEvent *motion_event = (XMotionEvent *) &event;
		x =  motion_event->x;
		y = Shared->winSizeY - motion_event->y;
		Shared->inWindow = 1;
	    }
	    break;
	case ButtonPress: 
	    {
		XButtonEvent *button_event = (XButtonEvent *) &event;
		x = event.xbutton.x;
		y = Shared->winSizeY - event.xbutton.y;
		Shared->inWindow = 1;
		switch (button_event->button) {
		case Button1:
		    Shared->mouseButtons |= Button1Mask;
		    break;
		case Button2:
		    Shared->mouseButtons |= Button2Mask;
		    break;
		case Button3:
		    Shared->mouseButtons |= Button3Mask;
		    break;
		}
	    }
	    break;
	case ButtonRelease:
	    {
		XButtonEvent *button_event = (XButtonEvent *) &event;
		switch (button_event->button) {
		case Button1:
		    Shared->mouseButtons &= ~Button1Mask;
		    break;
		case Button2:
		    Shared->mouseButtons &= ~Button2Mask;
		    break;
		case Button3:
		    Shared->mouseButtons &= ~Button3Mask;
		    break;
		}
	    }
	    break;
	case KeyPress:
	    {
		char buf[100];
		int rv;
		KeySym ks;
		rv = XLookupString(&event.xkey, buf, sizeof(buf), &ks, 0);
		switch(ks) {
		case XK_Escape: 
		    Shared->exitFlag = 1;
		    break;
		case XK_r:
		    Shared->reset = 1;
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,  "Reset");
		    break;
		case XK_space:
		    Shared->halt = 1;
		    break;
		case XK_g:
		    Shared->drawStats = !Shared->drawStats;
		    break;
		case XK_G:
		    Shared->sampleStats = 1;
		    break;
		case XK_v:
                    Shared->view_mode += 1;
                    if( Shared->view_mode > SUB_VIEW_TETHER)
                      Shared->view_mode = 0;
		    break;
		case XK_e:
                        Shared->elevels = 1;
		    break;
		case XK_E:
                        Shared->elevels = -1;
		    break;
		case XK_o:
                        Shared->offset = 1;
		    break;
		case XK_O:
                        Shared->offset = -1;
		    break;
		default:
		    break;
		}
	    }
	    break;
	default:
	    break;
	}// switch 
    }
    Shared->mouseX = x;
    Shared->mouseY = y;
}

