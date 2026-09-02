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
 *
 * gldlist.c: example of using a sproc process to compile GL display lists
 *
 * $Revision: 1.11 $ $Date: 2000/10/06 21:26:33 $ 
 *
 */

#include <stdlib.h>
#include <time.h>
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <sys/types.h>
#include <sys/prctl.h>

/*
 *	Usage() -- print usage advice and exit. This
 *      procedure is executed in the application process.
 */
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: gldlist file.ext ...\n");
    exit(1);
}

/*
 * Data in shared memory.
 */ 
struct
{
    pfNode *root;
    int ready;
} *share;

static void
dlproc(void *data)
{
    pfPipeWindow *pw = (pfPipeWindow *) data;
    pfWindow *noportwin;
    void *arena = pfGetSharedArena();

    while (share->ready < 1) sginap(CLK_TCK/5);

    noportwin = pfNewWin(arena);
    pfWinShare(noportwin, PFWIN_SHARE_GL_OBJS);
    pfWinType(noportwin, PFWIN_TYPE_NOPORT);
    pfAttachPWinWin(pw, noportwin);
    pfOpenWin(noportwin);

    pfuTravCompileDLists(share->root, 0);
    share->ready = 2;
}

static void
createproc(pfPipeWindow *pw)
{
    void *arena = pfGetSharedArena();

    pfOpenPWin(pw);
    if (sproc(dlproc, PR_SALL, pw) < 0) {
	pfNotify(PFNFY_FATAL, PFNFY_SYSERR, "sproc failed");
    }
    share->ready = 1;
    while (share->ready < 2) sginap(CLK_TCK/5);
}

static void
drawfunc(pfChannel *chan, void *data)
{
    pfClearChan(chan);
    pfDraw();
}

int
main (int argc, char *argv[])
{
    float	    t = 0.0f;
    pfScene	    *scene;
    pfPipe	    *p;
    pfPipeWindow    *pw;
    pfChannel	    *chan;
    pfSphere	    bsphere;
    void	    *arena;

    if (argc < 2)
	Usage();

    /* Initialize Performer */
    pfInit();

    arena = pfGetSharedArena();
    share = pfCalloc(1, sizeof(*share), arena);

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess( PFMP_DEFAULT );			

    /* Load all loader DSO's before pfConfig() forks */
    pfdInitConverter(argv[1]);

    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     */
    pfConfig();			

    /* Append to Performer search path, PFPATH, files in 
     *	    /usr/share/Performer/data */
    pfFilePath(".:/usr/share/Performer/data");

    /* Read a single file, of any known type. */
    if ((share->root = pfdLoadFile(argv[1])) == NULL) 
    {
	pfExit();
	exit(-1);
    }

    /* Attach loaded file to a new pfScene. */
    scene = pfNewScene();
    pfAddChild(scene, share->root);
    /* Create a pfLightSource and attach it to scene. */
    pfAddChild(scene, pfNewLSource());
    
    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinType(pw, PFPWIN_TYPE_X);
    pfPWinName(pw, "OpenGL Performer");
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    pfPWinConfigFunc(pw, createproc);
    pfConfigPWin(pw);
    
    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanFOV(chan, 45.0f, 0.0f);
    pfChanTravFunc(chan, PFTRAV_DRAW, drawfunc);    

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (share->root, &bsphere);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere.radius);

    /* Simulate for twenty seconds. */
    while (t < 20.0f)
    {
	pfCoord	   view;
	float      s, c;

	/* Go to sleep until next frame time. */
	pfSync();		

	/* Initiate cull/draw for this frame. */
	pfFrame();
	
	/* Compute new view position. */
	t = pfGetTime();
	pfSinCos(45.0f*t, &s, &c);
	pfSetVec3(view.hpr, 45.0f*t, -10.0f, 0);
	pfSetVec3(view.xyz, 2.0f * bsphere.radius * s, 
		  -2.0f * bsphere.radius *c, 
		  0.5f * bsphere.radius);
	pfChanView(chan, view.xyz, view.hpr);
    }

    /* Terminate parallel processes and exit. */
    pfExit();
}
