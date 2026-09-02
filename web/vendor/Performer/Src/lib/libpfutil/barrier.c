/*
 * Copyright 1999, Silicon Graphics, Inc.
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
 * barrier.c
 * $Revision: 1.6 $
 *
 * Less lame barrier definition--
 * same API as barrier, but doesn't spin so it's a zillion times faster
 * when there's contention for processors.
 * (init_barrier() isn't implemented;
 * its semantics are unclear and its feasibility is doubtful anyway.)
 */

#if defined(__linux__has_mutex_h__) || defined(mips) || defined(__win32__has_mutex_h__)
    #include <mutex.h> /* for add_then_test32 */
#endif /* !__linux */
#include "barrier.h"

#ifdef WIN32
#include <Performer/ulocks.h>
#endif


struct _pfuBarrier {
    volatile int which;
    uint32_t count[2];
    usema_t *sema[2];
    usptr_t *usptr;
};

extern PFUDLLEXPORT pfuBarrier *
pfuBarrierCreate(PF_USPTR_T *usptr)
{
    pfuBarrier *b = NULL;
    b = usmalloc(sizeof(*b), usptr);
    if (b != NULL)
    {
	b->which = 0;
	b->count[0] = 0;
	b->count[1] = 0;
	b->sema[0] = usnewsema(usptr, 0);
	b->sema[1] = usnewsema(usptr, 0);
	b->usptr = usptr;
	if (b->sema[0] == NULL || b->sema[1] == NULL)
	{
	    if (b->sema[0] != NULL) usfreesema(b->sema[0], usptr);
	    if (b->sema[1] != NULL) usfreesema(b->sema[1], usptr);
	    usfree(b, usptr);
	    b = NULL;
	}
    }
    return b;
}

extern PFUDLLEXPORT void
pfuBarrierDestroy(pfuBarrier *b)
{
    usfreesema(b->sema[0], b->usptr);
    usfreesema(b->sema[1], b->usptr);
    usfree(b, b->usptr);
}

extern PFUDLLEXPORT void
pfuBarrierEnter(pfuBarrier *b, unsigned int n)
{
    int snapwhich = b->which; /* snapshot so all processes get same picture */
    if (add_then_test32(&b->count[snapwhich], 1) == n)
    {
	int i;
	b->count[snapwhich] = 0;
	b->which = !snapwhich;
	for (i = 0; i < n-1; ++i)
	    usvsema(b->sema[snapwhich]);
    }
    else
	uspsema(b->sema[snapwhich]);
}
