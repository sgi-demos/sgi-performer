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
 * queryvchan.c 
 *
 *  example run:
 *	queryvchan
 *
 * command line options:
 * 
 *
 *
 * $Revision: 1.13 $
 * $Date: 2000/10/06 21:26:46 $
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <X11/keysym.h>
#include <Performer/pfutil.h>

extern int XSGIvcQueryScreenInputSyncSource(
    Display *           /* display */,
    int                 /* screen */,
    int *               /* syncVoltageReturn */,
    int *               /* genlockSourceReturn */,
    Bool *              /* genlockAchievedReturn */
);


int doCalligraphics = 0;

static char OptionStr[] = "C?";

static int
docmdline(int argc, char **argv)
{
     extern char *optarg;
     extern int  optind;
     int opt;
        
     while ((opt = getopt(argc,argv,OptionStr)) != -1) {
	 switch(opt) {
	 /* custom options */
	 case 'C':
		doCalligraphics ^= 1;
		break;
	 case '?':
	 default:
	    fprintf(stderr,"Usage: %s [c]", argv[0]);
	    exit(0);
	    break;
	} /* switch */
    } /* while */
    return 0;    
}

char VChanState[][80] = {{"off"}, {"active"}};

int
main(int argc, char *argv[])
{
    Display		*dsp;
    pfVideoChannel	*vChan;
    XSGIvcScreenInfo  server;
    pfWSVideoChannelInfo vChanInfo;
    int			arg;
    char*		str;
    int			q;
    int			nScreens, nVChans, scr, vc, nc;

    pfInit();


    pfQueryFeature(PFQFTR_XSGIVC, &q);
    if (!q)
    {
	pfNotify(PFNFY_NOTICE,PFNFY_RESOURCE,
	"XSGIvc and all dependent video operations are NOT supported on this platform");
    }
    else
    {
	pfQueryFeature(PFQFTR_DVR, &q);
	if (!q)
	{
	    pfNotify(PFNFY_NOTICE,PFNFY_RESOURCE,
	    "Dynamic resolution is NOT supported on this platform");
	}
    }
    
    /* cmdline options */
    arg = docmdline(argc, argv);

    /* Initialize Performer */
    pfNotifyLevel(PFNFY_INFO);
    pfInitState(NULL);
    
    /* Initialize Calligraphic HW */
    if (doCalligraphics)
    {
	pfQueryFeature(PFQFTR_CALLIGRAPHIC, &q);
	if (!q)
	{
	    pfNotify(PFNFY_NOTICE,PFNFY_RESOURCE,
	    "Calligraphic points are NOT supported on this platform");
	}
	
	if (pfCalligInitBoard(0) == TRUE)
	{
	    /* get the memory size */
	    pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
		"StartCalligraphic: board(%d) has %d Bytes of memory",
		0, pfGetCalligBoardMemSize(0));
	}
	else
	{
	    pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
	    "Could not init calligraphic board %d");
	}
    }


    /* initialize X and video display */
    dsp = pfGetCurWSConnection();
    nScreens = ScreenCount(dsp);

    for (scr=0; scr < nScreens; scr++)
    {
	XSGIvcQueryVideoScreenInfo(dsp, scr, &server);
	nVChans = pfGetNumScreenVChans(scr);
	if (nVChans <= 0)
	{
	    fprintf(stderr, "XSGIVC not supported on screen %d\n", scr);
	    continue;
	}
	fprintf(stderr, "Screen %d is on a %s with %d channels, flags=0x%x.\n", 
	    scr, server.graphicsType, nVChans, server.flags);
	for (vc=0, nc=0; nc < nVChans; vc++)
	{
	    vChan = pfNewVChan(NULL);
	    pfVChanId(vChan, vc);
	    pfVChanScreen(vChan, scr);
	    vChanInfo = pfGetVChanInfo(vChan);
	    if (vChanInfo)
	    {
		int	xOrg, yOrg, xSize, ySize, active;
		int	dx, dy;
		int	t1, t2, genlocked;
		pfGetVChanOrigin(vChan, &xOrg, &yOrg);
		pfGetVChanSize(vChan, &xSize, &ySize);
		active = pfIsVChanActive(vChan);
		fprintf(stderr, "\tChannel %d of loc[%d,%d] size[%dx%d] running at %fHz is %s.\n", 
		    vc, xOrg, yOrg, xSize, ySize, vChanInfo->vfinfo.verticalRetraceRate,
		    VChanState[active]);
		pfGetVChanMinDeltas(vChan, &dx, &dy);
		fprintf(stderr, "\t\tMin Deltas: dx=%d dy=%d\n", dx, dy);
		fprintf(stderr, "\t\tCalligraphic: 0x%p\n", pfGetVChanCallig(vChan));
		XSGIvcQueryScreenInputSyncSource(dsp, scr, &t1, &t2, &genlocked);
		fprintf(stderr, "\t\tGenlock: 0x%d\n", genlocked);
		if (active)
		    nc++;
	    }
	    else
	    {
		 fprintf(stderr, "NO VChan info on chan %d screen %d\n", vc, scr);
	    }
	}
    }
    return 0;
}
