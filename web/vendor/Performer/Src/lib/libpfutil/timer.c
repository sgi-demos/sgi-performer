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
 * timer.c
*/

#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#endif 
#include <Performer/pf.h>
#include <Performer/pfutil.h>

static pfuTimer *TimerList[128];
static int     NumTimers = 0;


PFUDLLEXPORT pfuTimer        *
pfuNewTimer(void *arena, int size)
{
    pfuTimer        *timer = (pfuTimer *)pfMalloc((unsigned int)sizeof(pfuTimer), arena);
    timer->data = pfMalloc((unsigned int)size, arena);
    timer->dataSize = size;

    timer->tstart = 1000000000000000000.0;
    timer->tstop = 1000000000000000000.0;
    timer->tdelta = 0.0;
    timer->frames = 0;

    return timer;
}

PFUDLLEXPORT void
pfuInitTimer(pfuTimer * timer, double start, double delta,
	  void(*func)(pfuTimer *), void *data)
{
    timer->tstart = start;
    timer->tdelta = delta;
    timer->tstop = timer->tstart + delta;
    timer->func = func;

    bcopy(data, timer->data, (unsigned int)timer->dataSize);
}

PFUDLLEXPORT void
pfuStartTimer(pfuTimer * timer)
{
    TimerList[NumTimers++] = timer;
}

PFUDLLEXPORT int
pfuActiveTimer(pfuTimer * timer)
{
    double          now = pfGetTime();

    return (now >= timer->tstart && now <= timer->tstop);
}

PFUDLLEXPORT void
pfuEvalTimers(void)
{
    int            i;
    double          now;

    now = pfGetTime();

    for (i = 0; i < NumTimers; i++)
    {
	pfuTimer        *timer = TimerList[i];

	if (now >= timer->tstop)
	{
	    if (timer->frames > 0)
	    {
		int            j;

		NumTimers--;
		for (j = i; j < NumTimers; j++)
		    TimerList[j] = TimerList[j + 1];

		i--;

		continue;
	    }
	    else
		timer->fraction = 1.0;
	}
	else if (now < timer->tstart)
	    continue;
	else
	    timer->fraction = (now - timer->tstart) / timer->tdelta;

	timer->tnow = now;
	timer->frames++;

	(*timer->func) (timer);
    }
}

PFUDLLEXPORT int
pfuEvalTimer(pfuTimer * timer)
{
    double          now;

    now = pfGetTime();

    if (now >= timer->tstop)
    {
	if (timer->frames > 0)
	{
	    pfuStopTimer(timer);
	    return 0;
	}
	else
	    timer->fraction = 1.0;
    }
    else if (now < timer->tstart)
	return 0;
    else
	timer->fraction = (now - timer->tstart) / timer->tdelta;

    timer->tnow = now;
    timer->frames++;

    (*timer->func) (timer);

    return 1;
}

PFUDLLEXPORT void
pfuStopTimer(pfuTimer * timer)
{
    int            i, j;

    for (i = 0; i < NumTimers; i++)
    {
	if (TimerList[i] == timer)
	{
	    NumTimers--;
	    for (j = i; j < NumTimers; j++)
		TimerList[j] = TimerList[j + 1];
	}
    }
}
