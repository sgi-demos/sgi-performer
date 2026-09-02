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
 * mpstats.c - MultiPipe Stats
 *
 * $Revision: 1.8 $
 * $Date: 2002/05/15 00:31:09 $
 *
 */

#include <Performer/pf.h> /* IRIS Performer includes */
#include <Performer/pfutil.h> 
#include "stdio.h"

#include "mpstats.h"   /* MPipeStats include         */

/* multiprocess shared data structure */
typedef struct
{
    pfuMPSControl    	    mpsControl;       /* mps control data		 */
    pfuMPSConfiguration     mpsConfig;        /* pipes/channels config	 */
    pfuMPSFrameStamp	   *mpsFrameStamps;  /* user memory stats  buffer */
    int 		    nFrames;
    int			    nSampledPipes;	           /* user input */
} SharedMPipeStats;

SharedMPipeStats *MPS;

/* toolkit definitions */

PFUDLLEXPORT int
pfuManageMPipeStats(int nFrames, int nSampledPipes)
{
    static int firstMPipeStats=0;
    static int testNum = 0;
    char fname [32] ;
    void *arena;
    
    if(!firstMPipeStats){
	firstMPipeStats = 1;
	
	arena = pfGetSharedArena();
	MPS = pfMalloc(sizeof(SharedMPipeStats), arena);	
	
	/* user data */
	MPS->nFrames = nFrames;
	MPS->nSampledPipes = nSampledPipes;
	MPS->mpsControl.endMPipeStats = 0;
	
	/* internal MPS data buffers */
	pfuCreateMPipeStatsBuffers();
	pfuInitMPipeStats();
	
	pfuRewindMPipeFrameStamps();
    }
    
    if(MPS->mpsControl.frCounter >= MPS->mpsControl.nFrames)
    {
        sprintf (fname, "MPipeStats%d.log", testNum++);
	pfuDumpMPipeStats(fname);
	
	/* reset the stats trace */
        pfuResetMPipeStats() ;
        pfuRewindMPipeFrameStamps() ;
	MPS->mpsControl.endMPipeStats = 0;
        firstMPipeStats = 0;
	
	return(0);  /* give the finish code to the requester */
    }
    else 
	pfuGetMPipeFrameStamp();
    
    return(1);
}

PFUDLLEXPORT int
pfuCreateMPipeStatsBuffers(void)
{
    int i;
    void *arena;
    
    arena = pfGetSharedArena();
    
    /* getting the pipes/channels configuration */
    pfuGetMPipeStatsConfig();
    
    /* allocating frame stamp pointers array, connecting it */
    MPS->mpsFrameStamps = (pfuMPSFrameStamp *)
	pfMalloc(MPS->mpsControl.nFrames * sizeof(pfuMPSFrameStamp),
		 arena);
    
    /* allocating the data stamps, connecting them */
    /* they are intelazed in order to get cache locality */
    for(i=0; i<MPS->mpsControl.nFrames; i++)
    {
	MPS->mpsFrameStamps[i].channels = (pfuMPSChannelStamp *)
	    pfMalloc(MPS->mpsConfig.nChannels * sizeof(pfuMPSChannelStamp),
		     arena);	    	
    }

    return 0;
}


/* internal utility function */

PFUDLLEXPORT void 
pfuGetMPipeStatsConfig(void)
{
    int i, j, prevpipes;
    void *arena;
    
    arena = pfGetSharedArena();
    
    /* initialize it in function of the user needs */
    MPS->mpsControl.nFrames = MPS->nFrames;  
    MPS->mpsControl.nPipes = MPS->nSampledPipes;
    
    /* getting the channel distribution of the requested pipes */
    for (i=0; i<MPS->mpsControl.nPipes; i++)
	MPS->mpsConfig.nChansPipe[i] = pfGetPipeNumChans(pfGetPipe(i));
    
    /* calculating the total number of channels */
    MPS->mpsConfig.nChannels = 0;
    for (i=0; i<MPS->mpsControl.nPipes; i++)
	MPS->mpsConfig.nChannels += MPS->mpsConfig.nChansPipe[i];
    /* allocating for pointers list of pfChannel and pfFrameStats ptrs */
    MPS->mpsConfig.channels = (pfChannel **) 
	pfMalloc(MPS->mpsConfig.nChannels * sizeof(pfChannel *),
		 arena); 
    MPS->mpsConfig.fstats = (pfFrameStats **)
	pfMalloc(MPS->mpsConfig.nChannels * sizeof(pfFrameStats *),
		 arena);
    
    
    /* Getting the pointers to channels and fstats */
    prevpipes = 0;
    for (i=0; i<MPS->mpsControl.nPipes; i++)
    {
	for (j=0; j<MPS->mpsConfig.nChansPipe[i]; j++)
	{
	    MPS->mpsConfig.channels[j + prevpipes] = 
		(pfChannel *) (pfGetPipeChan(pfGetPipe(i), j));
	    MPS->mpsConfig.fstats[j + prevpipes] =
		pfGetChanFStats(MPS->mpsConfig.channels[j + prevpipes]);
	}
	prevpipes += MPS->mpsConfig.nChansPipe[i]; /* i suppose whole use */
    }
}


PFUDLLEXPORT int 
pfuDestroyMPipeStatsBuffers(void)
{
    int i;
    
    /* deallocating channel data stamps */
    for(i=0; i<MPS->mpsControl.nFrames; i++) 
	pfFree(MPS->mpsFrameStamps[i].channels);
    
    /* deallocating frame stamp pointers array */
    pfFree(MPS->mpsFrameStamps); 
    
    /* also dealloc the MPSControl structures */
    pfFree(MPS->mpsConfig.channels);
    pfFree(MPS->mpsConfig.fstats);
    
    /* final dealloc */
    pfFree(MPS);

    return 0;
}



PFUDLLEXPORT void
pfuInitMPipeStats(void)
{
    int i;
    
    for (i=0; i<MPS->mpsConfig.nChannels; i++)
    {
	/* be sure that we all stats are disabled*/
	pfFStatsClass(MPS->mpsConfig.fstats[i],
		      PFSTATS_ALL, PFSTATS_OFF);
	
	/* enable the PFTIMES stats */
	pfFStatsClass(MPS->mpsConfig.fstats[i],
		      PFFSTATS_ENPFTIMES, PFSTATS_SET);
	
	/* enable the HIST mode */
	pfFStatsClassMode(MPS->mpsConfig.fstats[i],
			  PFFSTATS_PFTIMES,
			  PFFSTATS_PFTIMES_HIST, PFSTATS_SET);
	
	/* statistics accumulation and averaging is disabled */
	pfFStatsAttr(MPS->mpsConfig.fstats[i], PFFSTATS_UPDATE_FRAMES, 0.0f);
    }        
}



PFUDLLEXPORT void 
pfuResetMPipeStats(void)
{
    int i;
    
    for (i=0; i<MPS->mpsConfig.nChannels; i++)
    {
	/* be sure that we all stats are disabled*/
	pfFStatsClass(MPS->mpsConfig.fstats[i], PFSTATS_ALL, PFSTATS_OFF);
    }
}



pfFStatsValPFTimesApp dts_app;
pfFStatsValPFTimesIsect dts_isect;
pfFStatsValPFTimesCull dts_cull;
pfFStatsValPFTimesDraw dts_draw;

PFUDLLEXPORT int 
pfuGetMPipeFrameStamp(void)
{
    int i;
    
    /* read app and isect frame data*/
    pfMQueryFStats(MPS->mpsConfig.fstats[0],
		   AppQuery,
		   MPS->mpsFrameStamps[MPS->mpsControl.frCounter].app,
		   0);  /* it could be the size in a future release */
    pfMQueryFStats(MPS->mpsConfig.fstats[0],
		   IsectQuery,
		   MPS->mpsFrameStamps[MPS->mpsControl.frCounter].isect,
		   0); /* it could be the size in a future release */
    
    /* read cull and draw frame data per channel*/
    for(i=0; i<MPS->mpsConfig.nChannels; i++)
    {
	pfMQueryFStats(MPS->mpsConfig.fstats[i],
		       ChanCullQuery,
		       MPS->mpsFrameStamps[MPS->mpsControl.frCounter].channels[i].cull,
		       0); /* it could be the size in a future release */
	pfMQueryFStats(MPS->mpsConfig.fstats[i],
		       ChanDrawQuery,
		       MPS->mpsFrameStamps[MPS->mpsControl.frCounter].channels[i].draw,
		       0); /* it could be the size in a future release */
    }
    
    /* control status */
    MPS->mpsControl.frCounter++;
    if (MPS->mpsControl.frCounter > MPS->mpsControl.nFrames)
    {
	MPS->mpsControl.endMPipeStats = 1;
	return(0);
    }
    return(1); 
}



PFUDLLEXPORT void
pfuRewindMPipeFrameStamps(void)
{
    MPS->mpsControl.frCounter = 0;    
}


/* AWAITING SHARON pfDrawStats_NEW */

/*ARGSUSED*/
PFUDLLEXPORT void 
pfuDrawMPipeStats(int drawchannel)
{
    /* passing the stats channel, n_channels and fstat pointer list */
    
    /* pfDrawStats_NEW(MPS->mpsConfig.channels[drawchannel] ,
       MPS->mpsConfig.fstats,
       MPS->mpsConfig.nChannels);*/
    
}

PFUDLLEXPORT void 
pfuDumpMPipeStats(const char *filename)
{
    int i, j;
    int ii[10];
    double dd[10], ddd[16];
    
    FILE *fid;
    
    fid = fopen(filename, "w");
    if (!fid) fprintf(stderr, "Failure on opening MPipeStats dump file\n");
    
    /* creating the dump file header for mpscope */
    
    ii[0] = MPS->mpsControl.nFrames;
    ii[1] = MPS->mpsControl.nPipes;
    ii[2] = MPS->mpsConfig.nChannels;
    ii[3] = MPS->mpsConfig.nChansPipe[0];
    ii[4] = MPS->mpsConfig.nChansPipe[1];
    ii[5] = MPS->mpsConfig.nChansPipe[2];
    fprintf(fid, headerFormat, ii[0], ii[1], ii[2], ii[3], ii[4], ii[5]);
    
    /* creating the time stamps */
    
    for (i=0; i<MPS->mpsControl.nFrames; i++)
    {	
	ii[0] = MPS->mpsFrameStamps[i].app[PREV_MPSFRAME].frame;		
	dd[0] = MPS->mpsFrameStamps[i].app[PREV_MPSFRAME].start;		
	dd[1] = MPS->mpsFrameStamps[i].app[PREV_MPSFRAME].enterSync;	
	dd[2] = MPS->mpsFrameStamps[i].app[PREV_MPSFRAME].afterClean;	
	dd[3] = MPS->mpsFrameStamps[i].app[PREV_MPSFRAME].afterSync;	
	dd[4] = MPS->mpsFrameStamps[i].app[PREV_MPSFRAME].pfFrameStart;	
	dd[5] = MPS->mpsFrameStamps[i].app[PREV_MPSFRAME].pfFrameEnd;	
	
	ii[1] = MPS->mpsFrameStamps[i].isect[PREV_MPSFRAME].frame;
	dd[6] = MPS->mpsFrameStamps[i].isect[PREV_MPSFRAME].start;
	dd[7] = MPS->mpsFrameStamps[i].isect[PREV_MPSFRAME].end;
	
	ddd[0] = (MPS->mpsFrameStamps[i].isect[PREV_MPSFRAME].end -
		  MPS->mpsFrameStamps[i].isect[PREV_MPSFRAME].start) * 1000.0f;
	
	/* saving in file */ 
	fprintf( fid, appCounterFormat, frameString, i);
	fprintf( fid, appFormat,  appString, absoluteString, ii[0], dd[0], dd[1], dd[2], dd[3], dd[4], dd[5]);
	fprintf( fid, isectFormat, isectString, absoluteString, ii[1], dd[6], dd[7], relativeString, ddd[0]); 
	
	for (j=0; j<MPS->mpsConfig.nChannels; j++)
	{
	    ii[0] = MPS->mpsFrameStamps[i].channels[j].cull[PREV_MPSFRAME].frame;	
	    dd[0] = MPS->mpsFrameStamps[i].channels[j].cull[PREV_MPSFRAME].beginUpdate;
	    dd[1] = MPS->mpsFrameStamps[i].channels[j].cull[PREV_MPSFRAME].endUpdate;
	    dd[2] = MPS->mpsFrameStamps[i].channels[j].cull[PREV_MPSFRAME].start;
	    dd[3] = MPS->mpsFrameStamps[i].channels[j].cull[PREV_MPSFRAME].end;
	    
            ddd[0] = (MPS->mpsFrameStamps[i].channels[j].cull[PREV_MPSFRAME].end - 
		      MPS->mpsFrameStamps[i].channels[j].cull[PREV_MPSFRAME].start) * 1000.0f;
	    
	    ii[1] = MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].frame;
	    dd[4] = MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].start;
	    dd[5] = MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].end;
	    dd[6] = MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].pfDrawStart;
	    dd[7] = MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].pfDrawEnd;
	    dd[8] = MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].afterSwap;
	    dd[9] = (MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].pfDrawEnd - 
		     MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].pfDrawStart);
	    
            ddd[1] = (MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].end - 
		      MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].start) * 1000.0f;
	    /* predraw channel callback */
	    ddd[2] = (MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].pfDrawStart - 
		      MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].start) * 1000.0f;
	    /* pfDraw */
	    ddd[3] = (MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].pfDrawEnd -
		      MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].pfDrawStart) * 1000.0f;
	    /* postdraw channel callback */
	    ddd[4] = (MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].end -
		      MPS->mpsFrameStamps[i].channels[j].draw[PREV_MPSFRAME].pfDrawEnd) * 1000.0f;	    
	    
	    /* dumping in file */
	    fprintf( fid, channelsFormat1, cdString, j, i, absoluteString, ii[0], dd[0], dd[1], dd[2], dd[3]);
	    fprintf( fid, channelsFormat2, ii[1], dd[4], dd[5], dd[6], dd[7], dd[8]);
	    fprintf( fid, channelsFormat3, relativeString, ddd[0], ddd [1], ddd[2], ddd[3], ddd[4]); 
	}	
    }
    
    fclose(fid);
}

