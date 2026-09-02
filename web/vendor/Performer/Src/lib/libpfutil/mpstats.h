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
 * mpstats.h - MultiPipe Stats
 *
 * $Revision: 1.7 $
 * $Date: 2002/05/15 00:31:09 $
 *
 */

/****************************************************************************/
/* User buffer declarations for historical frame log                        */
/****************************************************************************/

typedef struct {
    double 		   start;		    /* quad align */
    pfFStatsValPFTimesCull cull[PFFSTATS_MAX_HIST_FRAMES];    /* channel cull */
    pfFStatsValPFTimesDraw draw[PFFSTATS_MAX_HIST_FRAMES];    /* channel draw */
} pfuMPSChannelStamp;

typedef struct {
    double 			start;
    pfFStatsValPFTimesApp	app[PFFSTATS_MAX_HIST_FRAMES];	
    pfFStatsValPFTimesIsect	isect[PFFSTATS_MAX_HIST_FRAMES]; 
    pfuMPSChannelStamp		*channels;	
} pfuMPSFrameStamp;

/* User configuration information */

typedef struct {
    int 			nChannels;
    int				nChansPipe[3];
    pfChannel 		      **channels;  /* pointers list to pfChannel pointer */
    pfFrameStats	      **fstats;    /* pointers list to fstats channel pointer */
} pfuMPSConfiguration;

/* MPipeStats control structure */

typedef struct {
    unsigned int nFrames;	    /* nframes requested, USER USE	     */
    int		 nPipes;	    /* number of gfx pipes sampled, USER USE */   
    unsigned int frCounter;	    /* internal MPipeStats counter	     */
    int		 endMPipeStats;	    /* flag of completion		     */
} pfuMPSControl;


/*****************************************************************************/
/* Multipipe stats pfuAPI declarations                                       */
/*****************************************************************************/

PFUDLLEXPORT int pfuCreateMPipeStatsBuffers(void);   /* allocates MPipeStats user buffers */

PFUDLLEXPORT int pfuDestroyMPipeStatsBuffers(void);  /* frees MPipeStats user buffers     */      

PFUDLLEXPORT void pfuInitMPipeStats(void);	        /* Gets configuration, init stats    */

PFUDLLEXPORT void pfuResetMPipeStats(void);          /* Stops the MPipeStats sampling     */


PFUDLLEXPORT int pfuGetMPipeFrameStamp(void);	/* Grab MPipeStats, update frCounter */
                                        /* returns 0 if it finished	     */

PFUDLLEXPORT void pfuRewindMPipeFrameStamps(void);	/* frCounter = 0	   	     */	
	   
PFUDLLEXPORT void pfuDrawMPipeStats(int);	        /* Draw MPipeStats on req. channel   */

PFUDLLEXPORT void pfuDumpMPipeStats(const char *);   /* Dumps user buffers into file      */
				        
PFUDLLEXPORT void pfuGetMPipeStatsConfig(void);	/* internal API			     */


/*****************************************************************************/
/* static query   definitions                                                */
/*****************************************************************************/

#define PREV_MPSFRAME	0 /* it could change in the future */

static unsigned int AppQuery[] = {
    PFFSTATS_BUF_PREV | PFFSTATSVAL_PFTIMES_HIST_LAST_APP, 
    NULL
};
static unsigned int IsectQuery[] = {
    PFFSTATS_BUF_PREV | PFFSTATSVAL_PFTIMES_HIST_LAST_ISECT, 
    NULL
};
static unsigned int ChanDrawQuery[] = {
    PFFSTATS_BUF_PREV | PFFSTATSVAL_PFTIMES_HIST_LAST_DRAW, 
    NULL
};

static unsigned int ChanCullQuery[] = {
    PFFSTATS_BUF_PREV | PFFSTATSVAL_PFTIMES_HIST_LAST_CULL, 
    NULL
};


/*****************************************************************************/
/* dump file formats                                                         */
/*****************************************************************************/
/* STRINGS */
const char frameString[] =
    "============================================================================\n"; 
const char absoluteString[] =
    "Absolute Timeline information (sec)\n...................................\n";
const char relativeString[] =
    "Relative Timeline information (sec)\n...................................\n";
const char appString[] =
    "_____________________________APP____________________________________________\n";
const char isectString[] =
    "_____________________________ISECT__________________________________________\n";
const char cdString[] =
    "_____________________________CULL & DRAW____________________________________\n";

/* FORMATS */
const char headerFormat[] = 
    "Nframes:%i Npipes:%i \n NChannels:%i Pipe0:%i Pipe1:%i Pipe2:%i\n"; 
const char appCounterFormat[] = "%sFrame: %i\n";
const char appFormat[] =
    "%s%sAppFrame: %i\nAppStart: %g\nenterSync: %g afterClean: %g afterSync: %g pfFrameStart:%g pfFrameEnd: %g\n";
const char isectFormat[] =
    "%s%sIsectFrame: %i\nStart: %g End: %g\n\n%sTotal Isect Time: %g\n";
const char channelsFormat1[] =
    "%sChan: %i\nFrame: %i\n\n%sCullFrame: %i\n   BeginUpDate: %g EndUpdate: %g\n   Start: %g End: %g\n";
const char channelsFormat2[]=
    "DrawFrame: %i\n   Start: %g End: %g\n   pfDrawStart: %g pfDrawEnd: %g AfterSwap: %g\n\n";
const char channelsFormat3[] =        
    "%sTotal Cull Time: %g\nTotal Draw Time: %g\n   PreDraw: %g\n   pfDraw: %g\n   PostDraw: %g\n";

#if 0
/*****************************************************************************/
/* Only for information 				                     */
/* internal stats structures defined in pfstats.h 	                     */
/*****************************************************************************/

typedef struct pfFStatsValHistIndex	/* PFFSTATSVAL_PFTIMES_HIST_INDEX   */
{   /* indecies into time stamp history arrays for each process */
    int	frame;
    int	app;
    int	cull;
    int	draw;
    int	isect;
} pfFStatsValHistIndex;

typedef struct pfFStatsValPFTimesApp	/* PFFSTATSVAL_PFTIMES_HIST_ISECT   */
{
    int	frame;			/* app frame stamp */
    double	start;		/* start */
    double	enterSync;	/* time enter pfSync */
    double	afterClean;	/* time after clean in pfSync - before sleep */
    double	afterSync;	/* time return from pfSync */
    double	pfFrameStart, pfFrameEnd;
} pfFStatsValPFTimesApp;

typedef struct pfFStatsValPFTimesCull	/* PFFSTATSVAL_PFTIMES_HIST_CULL */
{
    int	frame;			/* app frame stamp */
    double	beginUpdate, endUpdate; /* update of node changes from APP */
    double	start;		/* start cull - before call pfChanCullFunc() callback */
    double	end;		/* end cull - after return from pfChanCullFunc() callback */
} pfFStatsValPFTimesCull;

typedef struct pfFStatsValPFTimesDraw	/* PFFSTATSVAL_PFTIMES_HIST_DRAW    */
{
    int	frame; /* app frame stamp */
    double	start;		/* start draw - before pfChanDrawFunc() callback */
    double	end;		/* end draw - after return from pfChanDrawFunc() callback */
    double	pfDrawStart;	/* time of start of pfDraw() */
    double	pfDrawEnd;	/* time returning from pfDraw() */
    double	afterSwap;	/* time pipe swapbuffers() finishes (est.) */
} pfFStatsValPFTimesDraw;

typedef struct pfFStatsValPFTimesIsect	/* PFFSTATSVAL_PFTIMES_HIST_ISECT */
{
    int	frame; /* app frame stamp */
    double	start;	/* time before call pfIsectFunc() callback */
    double	end;	/* time after call pfIsectFunc() callback */
} pfFStatsValPFTimesIsect;

/******************************************************** end of info */
#endif
