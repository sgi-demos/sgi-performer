/*
 * Copyright 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 *
 * file pfstats.h
 * --------------	
 * Include file for Performer statistics for the prototyping library.
 *
 * $Revision: 1.39 $
 * $Date: 1997/05/18 21:13:48 $
 *
 */

#ifndef __PFSTATS_H__
#define __PFSTATS_H__

/* Default enables C++ API for C++ compiles */
#ifndef PF_CPLUSPLUS_API
#ifdef __cplusplus
#define PF_CPLUSPLUS_API 1 /* enable C++ API, types */
#else
#define PF_CPLUSPLUS_API 0 /* do not enable C++ API, types */
#endif
#endif

/* Default only enables C API if not using C++ API */
#ifndef PF_C_API
#define PF_C_API !PF_CPLUSPLUS_API 
#endif


#ifdef __cplusplus
extern "C" {

#endif

#if PF_C_API
/*
 * ----------------------------- C API --------------------------------
 */

/* ----------------------------- pfFrameStats -------------------------- */


#if !PF_CPLUSPLUS_API /* Also in C++ header pfFrameStats.h */
/* -------------- pfFrameStats Tokens and Structs -------------------- */
/* 
 * pfFrameStats is inherited from pfStats of libpr
 */
/* pfFStatsClassMode() */

#define PFFSTATS_CLASSES_START	    100
#define PFFSTATS_CLASSES	    (0 + PFFSTATS_CLASSES_START)
#define PFFSTATS_DB		    (1 + PFFSTATS_CLASSES_START)

#define PFFSTATS_CULL		    (3 + PFFSTATS_CLASSES_START)
#define PFFSTATS_PFTIMES	    (4 + PFFSTATS_CLASSES_START)
#define PFFSTATS_RTMON	    	    (6 + PFFSTATS_CLASSES_START)

/* pfFStatsClass and pfChanStatsMode() */

#define PFFSTATS_ENDB		    (0x01 << PFSTATS_EN_BITS)
#define PFFSTATSHW_ENGFXPIPE	    (PFSTATSHW_ENGFXPIPE)				
#define PFFSTATS_ENCULL		    (0x04 << PFSTATS_EN_BITS)
#define PFFSTATS_ENPFTIMES	    (0x08 << PFSTATS_EN_BITS)
#define PFFSTATS_ENGFXPFTIMES	    (PFFSTATS_ENPFTIMES | PFSTATSHW_ENGFXPIPE_TIMES)
#define PFFSTATS_ENMEM		    ((0x10 << PFSTATS_EN_BITS) | \
				     PFSTATS_ENMEM)
#define PFFSTATS_ENRTMON	    (0x20 << PFSTATS_EN_BITS) 
#define PFFSTATS_EN_MASK	    ((0xff << PFSTATS_EN_BITS) | \
				     PFSTATS_EN_MASK)

/* These are combined with EN masks for printing and for queries */	
#define PFFSTATS_BUF_CUR	    (0x1000u << PFSTATS_EN_BITS)
#define PFFSTATS_BUF_AVG	    (0x2000u << PFSTATS_EN_BITS)
#define PFFSTATS_BUF_CUM	    (0x4000u << PFSTATS_EN_BITS)
#define PFFSTATS_BUF_PREV	    (0x8000u << PFSTATS_EN_BITS)
#define PFFSTATS_BUF_MASK	    (0xf000u << PFSTATS_EN_BITS)

/* db class */
#define PFFSTATS_DB_VIS		    0x00010000
#define PFFSTATS_DB_EVAL	    0x00020000
#define PFFSTATS_DB_CLEAN	    0x00040000
#define PFFSTATS_DB_UPDATE	    0x00080000
#define PFFSTATS_DB_MASK	   (0x000f0000)

#define PFFSTATSHW_GFXPIPE_MASK	    (PFSTATSHW_GFXPIPE_MASK)

/* cull class */
#define PFFSTATS_CULL_TRAV	    0x00800000
#define PFFSTATS_CULL_MASK	    0x00800000

/* process frame times class */
#define PFFSTATS_PFTIMES_BASIC	    0x01000000
#define PFFSTATS_PFTIMES_HIST	    0x02000000
#define PFFSTATS_PFTIMES_MASK	    0x03000000   
 

#define PFFSTATS_MASK		    (PFFSTATS_PFTIMES_MASK | \
				     PFFSTATS_DB_MASK |      \
				     PFFSTATS_CULL_MASK |    \
				     PFSTATS_MASK)
								
/* pfFStatsAttr() */
#define PFFSTATS_UPDATE_FRAMES		2000
#define PFFSTATS_UPDATE_SECS		3000
#define PFFSTATS_PFTIMES_HIST_FRAMES	4000

/* pfChanStatsMode() */
#define PFCSTATS_DRAW 1

typedef struct pfFStatsValProc
{
    float 	total;		    /* PFFSTATSVAL_*_TOTAL		    */
    float	app;		    /* PFFSTATSVAL_*_APP		    */
    float	cull;		    /* PFFSTATSVAL_*_CULL		    */
    float	draw;		    /* PFFSTATSVAL_*_DRAW		    */
    float	isect;		    /* PFFSTATSVAL_*_ISECT		    */
    float	dbase;		    /* PFFSTATSVAL_*_DBASE		    */
} pfFStatsValProc;

typedef struct pfFStatsValNode
{
    float 	total;		    /* PFFSTATSVAL_*_TOTAL		    */
    float	bboards;	    /* PFFSTATSVAL_*_BBOARDS		    */
    float	bboardGSets;	    /* PFFSTATSVAL_*_BBOARD_GSETS	    */
    float	geodes;		    /* PFFSTATSVAL_*_GEODES		    */
    float	groups;		    /* PFFSTATSVAL_*_GROUPS		    */
    float	switches;	    /* PFFSTATSVAL_*_SWITCHES		    */
    float	lods;		    /* PFFSTATSVAL_*_LODS		    */
    float	lodsFade;	    /* PFFSTATSVAL_*_LODS_FADE		    */
    float 	sequences;	    /* PFFSTATSVAL_*_SEQUENCES		    */
    float	layers;		    /* PFFSTATSVAL_*_LAYERS		    */
    float	dcs;		    /* PFFSTATSVAL_*_DCS		    */
    float	fcs;		    /* PFFSTATSVAL_*_FCS		    */
    float	scs;		    /* PFFSTATSVAL_*_SCS		    */
    float 	lpointNodes;	    /* PFFSTATSVAL_*_LPOINT_NODES	    */
    float	lpointNodesOmniDir; /* PFFSTATSVAL_*_LPOINTNODES_OMNIDIR    */
    float	lpointNodesUniDir;  /* PFFSTATSVAL_*_LPOINTNODES_UNIDIR	    */
    float	lpointNodesBiDir;   /* PFFSTATSVAL_*_LPOINTNODES_BIDIR	    */
    float 	lpoints;	    /* PFFSTATSVAL_*_LPOINTS		    */
    float	lpointsOmniDir;	    /* PFFSTATSVAL_*_LPOINTS_OMNIDIR	    */
    float	lpointsUniDir;	    /* PFFSTATSVAL_*_LPOINTS_UNIDIR	    */
    float	lpointsBiDir;	    /* PFFSTATSVAL_*_LPOINTS_BIDIR	    */
    float 	lsources;	    /* PFFSTATSVAL_*_LSOURCES		    */
    float 	partitions;	    /* PFFSTATSVAL_*_PARTITIONS		    */
    float 	texts;              /* PFFSTATSVAL_*_TEXTS		    */
} pfFStatsValNode;

typedef struct pfFStatsValMem
{
    pfFStatsValProc 	proc; 
} pfFStatsValMem;

#define PFFSTATS_MAX_HIST_FRAMES 10

typedef struct pfFStatsValHistIndex	/* PFFSTATSVAL_PFTIMES_HIST_INDEX   */
{   /* indecies into time stamp history arrays for each process */
    int	frame;
    int	app;
    int	cull;
    int	draw;
    int	isect;
    int	dbase;
} pfFStatsValHistIndex;

typedef struct pfFStatsValPFTimesApp	/* PFFSTATSVAL_PFTIMES_HIST_APP   */
{
    int	frame;			/* app frame stamp */
    int pad;
    double	start;		/* start */
    double	enterSync;	/* time enter pfSync */
    double	afterClean;	/* time after clean in pfSync - before sleep */
    double	afterSync;	/* time return from pfSync */
    double	pfFrameStart, pfFrameEnd;
} pfFStatsValPFTimesApp;

typedef struct pfFStatsValPFTimesCull	/* PFFSTATSVAL_PFTIMES_HIST_CULL */
{
    int	frame;		/* app frame stamp */
    int pad;
    double	beginUpdate, endUpdate; /* update of node changes from APP */
    double	start;	/* start cull - before call channel CULL callback */
    double	end;	/* end cull - after return from channel CULL callback */
} pfFStatsValPFTimesCull;

typedef struct pfFStatsValPFTimesDraw	/* PFFSTATSVAL_PFTIMES_HIST_DRAW    */
{
    int	frame; /* app frame stamp */
    int pad;
    double	start;	/* start draw - before channel DRAW callback */
    double	end;	/* end draw - after return from channel CULL callback */
    double	startInc, endInc; /* start/end of period to do pipe incremental pipe update for frame */
    double	pfDrawStart;	/* time of start of pfDraw() */
    double	pfDrawEnd;	/* time returning from pfDraw() */
    double	afterSwap;	/* time pipe swapbuffers() finishes (est.) */
} pfFStatsValPFTimesDraw;

typedef struct pfFStatsValPFTimesIsect	/* PFFSTATSVAL_PFTIMES_HIST_ISECT */
{
    int	frame; /* app frame stamp */
    int pad;
    double	start;	/* time before call pfIsectFunc() callback */
    double	end;	/* time after call pfIsectFunc() callback */
} pfFStatsValPFTimesIsect;

typedef struct pfFStatsValPFTimesDBase	/* PFFSTATSVAL_PFTIMES_HIST_DBASE */
{
    int	frame; /* app frame stamp */
    int pad;
    double	start;	/* time before call pfDBaseFunc() callback */
    double	end;	/* time after call pfDBaseFunc() callback */
} pfFStatsValPFTimesDBase;

typedef struct pfFStatsValPFTimesHistAll	/* PFFSTATSVAL_PFTIMES_HIST_ALL */
{
    int		appFrame;   /* PFFSTATSVAL_PFTIMES_HIST_APPFRAME */
			    /* Number of frames to MP profile */
    int		nFrames;    /* PFFSTATSVAL_PFTIMES_HIST_NFRAMES */
    /* Process indexing pointers to current History buffer 
     * - PFFSTATSVAL_PFTIMES_HIST_INDEX */	
    pfFStatsValHistIndex    index;
    /*  History of APP time stamps - PFFSTATSVAL_PFTIMES_HIST_ALL_APP */	
    pfFStatsValPFTimesApp   app[PFFSTATS_MAX_HIST_FRAMES]; 
    /*  History of Cull time stamps - PFFSTATSVAL_PFTIMES_HIST_ALL_CULL */	
    pfFStatsValPFTimesCull  cull[PFFSTATS_MAX_HIST_FRAMES]; 
    /* History of Draw time stamps - PFFSTATSVAL_PFTIMES_HIST_ALL_DRAW */	
    pfFStatsValPFTimesDraw  draw[PFFSTATS_MAX_HIST_FRAMES]; 
    /* History of isect time stamps - PFFSTATSVAL_PFTIMES_HIST_ALL_ISECT */
    pfFStatsValPFTimesIsect isect[PFFSTATS_MAX_HIST_FRAMES]; 
    /* History of dbase time stamps - PFFSTATSVAL_PFTIMES_HIST_ALL_DBASE */
    pfFStatsValPFTimesDBase dbase[PFFSTATS_MAX_HIST_FRAMES]; 
} pfFStatsValPFTimesHist;

typedef struct pfFStatsValPFTimesHistLast	/* PFFSTATSVAL_PFTIMES_HIST_LAST */
{
    int		appFrame;   /* PFFSTATSVAL_PFTIMES_HIST_LAST_APPFRAME */
    /*  History of APP time stamps - PFFSTATSVAL_PFTIMES_HIST_LAST_APP */	
    pfFStatsValPFTimesApp   app; 
    /*  History of Cull time stamps - PFFSTATSVAL_PFTIMES_HIST_LAST_CULL */	
    pfFStatsValPFTimesCull  cull; 
    /* History of Draw time stamps - PFFSTATSVAL_PFTIMES_HIST_LAST_DRAW */	
    pfFStatsValPFTimesDraw  draw; 
    /* History of isect time stamps - PFFSTATSVAL_PFTIMES_HIST_LAST_ISECT */
    pfFStatsValPFTimesIsect isect; 
    /* History of isect time stamps - PFFSTATSVAL_PFTIMES_HIST_LAST_DBASE */
    pfFStatsValPFTimesDBase dbase; 
} pfFStatsValPFTimesHistLast;

typedef struct pfFStatsValPFTimes
{
    /* start - start of frame or period */
    double 		    start;	/* PFFSTATSVAL_PFTIMES_START */
    /* end - end of frame or period */
    double		    end;	/* PFFSTATSVAL_PFTIMES_END */
    /*
     * proc - time for each process stage.  total is the total
     * performer latency for a frame, including the estimated
     * time for swapbuffers to occur.  This does not include any
     * additional time for latency incurred by the actual display
     * hardware.
     */
    pfFStatsValProc	    proc;	/* PFFSTATSVAL_PFTIMES_PROC */
    /* appStamp - current app frame stamp for each stage */
    pfFStatsValProc	    appStamp;	/* PFFSTATSVAL_PFTIMES_APPSTAMP */
    /* numFrames - number of frames executed in each process stage */
    pfFStatsValProc	    numFrames;	/* PFFSTATSVAL_PFTIMES_NUMFRAMES */
    /*
     * misses - number of frames each stage missed goal frame rate
     * here, total refers to a dectection of an overall miss of a
     * frame -- even if no singe stage detected a missed frame.
     */
    pfFStatsValProc	    misses;	/* PFFSTATSVAL_PFTIMES_MISSES */
    pfFStatsValPFTimesHist  hist;	/* PFFSTATSVAL_PFTIMES_HIST_LAST */
} pfFStatsValPFTimes;

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfFrameStats CAPI ------------------------------*/

extern pfFrameStats*        pfNewFStats(void);
extern pfType*              pfGetFStatsClassType(void);
extern uint                 pfFStatsClass(pfFrameStats *_fstats, uint _mask, int _val);
extern uint                 pfGetFStatsClass(pfFrameStats *_fstats, uint _emask);
extern uint                 pfFStatsClassMode(pfFrameStats *_fstats, int _class, uint _mask, int _val);
extern uint                 pfGetFStatsClassMode(pfFrameStats *_fstats, int _class);
extern void                 pfFStatsAttr(pfFrameStats *_fstats, int _attr, float _val);
extern float                pfGetFStatsAttr(pfFrameStats *_fstats, int _attr);
extern uint                 pfGetOpenFStats(pfFrameStats *_fstats, uint _emask);
extern uint                 pfOpenFStats(pfFrameStats *_fstats, uint _enmask);
extern uint                 pfCloseFStats(uint _enmask);
extern void                 pfResetFStats(pfFrameStats *_fstats);
extern void                 pfClearFStats(pfFrameStats *_fstats, uint _which);
extern void                 pfAccumulateFStats(pfFrameStats *_fstats, pfFrameStats* _src, unsigned int _which);
extern void                 pfAverageFStats(pfFrameStats *_fstats, pfFrameStats* _src, unsigned int _which, int _num);
extern void                 pfFStatsCountGSet(pfFrameStats *_fstats, pfGeoSet *gset);
extern int                  pfQueryFStats(pfFrameStats *_fstats, uint _which, void *_dst, int _size);
extern int                  pfMQueryFStats(pfFrameStats *_fstats, uint *_which, void *_dst, int _size);
extern void                 pfDrawFStats(pfFrameStats *_fstats, pfChannel *_chan);
extern void                 pfCopyFStats(pfFrameStats *_fstats, const pfFrameStats *_prev, uint _dstBufSel, uint _srcBufSel, uint _which);
extern void                 pfFStatsCountNode(pfFrameStats *_fstats, int _class, uint _mode, pfNode * _node);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header pfFrameStats.h */

/* 
 *Frame Stats Exposed Value Types and Tokens
 */

typedef struct pfFStatsValEvaluatedNodes    /* PFFSTATSVAL_EVALUATED	    */
{
	/* proc is the process(es) in which a given node type has
	 * its main evaluation step -- if any.
	 * However, there are separate evaluation counts for each process.
	 */ 
    pfFStatsValNode	proc;		    /* PFFSTATSVAL_EVALUATED_PROC   */
    pfFStatsValNode	app;		    /* PFFSTATSVAL_EVALUATED_APP    */
    pfFStatsValNode	clean;		    /* PFFSTATSVAL_EVALUATED_APP    */
    pfFStatsValNode	cull;		    /* PFFSTATSVAL_EVALUATED_CULL   */
    pfFStatsValNode	draw;		    /* PFFSTATSVAL_EVALUATED_DRAW   */
    pfFStatsValNode	isect;		    /* PFFSTATSVAL_EVALUATED_ISECT  */
} pfFStatsValEvaluatedNodes;

typedef struct pfFStatsValDB	    /* PFFSTATSVAL_DB			    */
{
	/* visible nodes are nodes in the database that are
	 * at least partially within viewing frustum 
	 */
    pfFStatsValNode		visibleNodes;	/* PFFSTATSVAL_VISIBLE	    */
	/* evaluated nodes have some special step of additional computation
	 * that must be performed in some process: such as LOD evaluation.
	 */
    pfFStatsValEvaluatedNodes   evaluatedNodes;	/* PFFSTATSVAL_EVALUATED    */
} pfFStatsValDB;

typedef struct pfFStatsValCullTest  /* PFFSTATSVAL_CULLTEST		    */
{
    float	total;			/* PFFSTATSVAL_CULLTEST_*_TOTAL	    */
    float	in;			/* PFFSTATSVAL_CULLTEST_*_IN	    */
    float	out;			/* PFFSTATSVAL_CULLTEST_*_OUT	    */
    float	partial;		/* PFFSTATSVAL_CULLTEST_*_PARTIAL   */
} pfFStatsValCullTest;

typedef struct pfFStatsValCull	/* PFFSTATSVAL_CULL			    */
{ /* counts of nodes traversed by the cull traversal and the results of
   * cull tests on nodes and gsets.
   * this also includes the counts sorted pfGeoSets - which happens ad
   * the end of the cull task.
   */
    pfFStatsValNode    	    traversedNodes; /* PFFSTATSVAL_CULLTRAV	    */
    pfFStatsValCullTest	    isectNodes;	    /* PFFSTATSVAL_CULLTEST_NODES   */
    pfFStatsValCullTest	    isectGSets;	    /* PFFSTATSVAL_CULLTEST_GSETS   */
    float 		    sortedGSets;    /* PFFSTATSVAL_CULL_SORTED_GSETS*/
} pfFStatsValCull;

/*
 * pfFrameStatsValues - holds the values of a pfFrameStats buffer.
 *	This structure has fields for each pfFrameStats class,  and a
 *	pfstats field for holding the values of the inherited pfStats structure.
 */
typedef struct pfFrameStatsValues	/* PFSTATSVAL_ALL		    */
{
				/* app frame time-stame */
    int		    frame;	/* PFFSTATSVAL_FRAME		    */	    
    int		    numFrames;	/* PFFSTATSVAL_NUMFRAMES	    */  
    int		    numIsectFrames;  /* PFFSTATSVAL_NUMISECTFRAMES  */  
    pfFStatsValPFTimes	    pfTimes;	/* PFFSTATSVAL_PFTIMES		    */
    pfFStatsValDB	    db;		/* PFFSTATSVAL_DB		    */
    pfFStatsValCull	    cull;	/* PFFSTATSVAL_CULL		    */
    pfStatsValues	    pfstats;	/* PFFSTATSVAL_PFSTATS		    */
} pfFrameStatsValues;

/* Frame Stats Query Tokens */	
#define PFFSTATSVAL_START			PFSTATSVAL_NUM_TOKENS

#define PFFSTATSVAL_FRAME			(PFFSTATSVAL_START + 1000)
#define PFFSTATSVAL_NUMFRAMES			(PFFSTATSVAL_START + 1020)
#define PFFSTATSVAL_NUMISECTFRAMES /* XXX */	(PFFSTATSVAL_START + 1030)
#define PFFSTATSVAL_PFSTATS /* XXX */		(PFFSTATSVAL_START + 1040)

#define PFFSTATSVAL_PFTIMES			(PFFSTATSVAL_START + 2000)
#define PFFSTATSVAL_PFTIMES_START		(PFFSTATSVAL_START + 2010)
#define PFFSTATSVAL_PFTIMES_END			(PFFSTATSVAL_START + 2020)

#define PFFSTATSVAL_PFTIMES_NUMFRAMES		(PFFSTATSVAL_START + 2100)
#define PFFSTATSVAL_PFTIMES_NUMFRAMES_TOTAL	(PFFSTATSVAL_START + 2110)
#define PFFSTATSVAL_PFTIMES_NUMFRAMES_APP	(PFFSTATSVAL_START + 2120)
#define PFFSTATSVAL_PFTIMES_NUMFRAMES_CULL	(PFFSTATSVAL_START + 2130)
#define PFFSTATSVAL_PFTIMES_NUMFRAMES_DRAW	(PFFSTATSVAL_START + 2140)
#define PFFSTATSVAL_PFTIMES_NUMFRAMES_ISECT	(PFFSTATSVAL_START + 2150)
#define PFFSTATSVAL_PFTIMES_NUMFRAMES_DBASE	(PFFSTATSVAL_START + 2160)

#define PFFSTATSVAL_PFTIMES_APPSTAMP		(PFFSTATSVAL_START + 3000)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_TOTAL	(PFFSTATSVAL_START + 3010)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_APP	(PFFSTATSVAL_START + 3020)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_CULL	(PFFSTATSVAL_START + 3030)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_DRAW	(PFFSTATSVAL_START + 3040)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_ISECT	(PFFSTATSVAL_START + 3050)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_DBASE	(PFFSTATSVAL_START + 3060)

#define PFFSTATSVAL_PFTIMES_PROC		(PFFSTATSVAL_START + 3100)
#define PFFSTATSVAL_PFTIMES_PROC_TOTAL		(PFFSTATSVAL_START + 3110)
#define PFFSTATSVAL_PFTIMES_PROC_APP		(PFFSTATSVAL_START + 3120)
#define PFFSTATSVAL_PFTIMES_PROC_CULL		(PFFSTATSVAL_START + 3130)
#define PFFSTATSVAL_PFTIMES_PROC_DRAW		(PFFSTATSVAL_START + 3140)
#define PFFSTATSVAL_PFTIMES_PROC_ISECT		(PFFSTATSVAL_START + 3150)
#define PFFSTATSVAL_PFTIMES_PROC_DBASE		(PFFSTATSVAL_START + 3160)

#define PFFSTATSVAL_PFTIMES_MISSES		(PFFSTATSVAL_START + 3200)
#define PFFSTATSVAL_PFTIMES_MISSES_TOTAL	(PFFSTATSVAL_START + 3210)
#define PFFSTATSVAL_PFTIMES_MISSES_APP		(PFFSTATSVAL_START + 3220)
#define PFFSTATSVAL_PFTIMES_MISSES_CULL		(PFFSTATSVAL_START + 3230)
#define PFFSTATSVAL_PFTIMES_MISSES_DRAW		(PFFSTATSVAL_START + 3240)
#define PFFSTATSVAL_PFTIMES_MISSES_ISECT	(PFFSTATSVAL_START + 3250)
#define PFFSTATSVAL_PFTIMES_MISSES_DBASE	(PFFSTATSVAL_START + 3260)

#define PFFSTATSVAL_PFTIMES_HIST_ALL		(PFFSTATSVAL_START + 3400)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_APPFRAME	(PFFSTATSVAL_START + 3410)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_NFRAMES	(PFFSTATSVAL_START + 3420)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_INDEX	(PFFSTATSVAL_START + 3430)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_APP	(PFFSTATSVAL_START + 3440)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_CULL	(PFFSTATSVAL_START + 3450)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_DRAW	(PFFSTATSVAL_START + 3460)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_ISECT	(PFFSTATSVAL_START + 3470)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_DBASE	(PFFSTATSVAL_START + 3480)


#define PFFSTATSVAL_PFTIMES_HIST_LAST		(PFFSTATSVAL_START + 3500)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_APPFRAME	(PFFSTATSVAL_START + 3510)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_APP	(PFFSTATSVAL_START + 3520)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_CULL	(PFFSTATSVAL_START + 3530)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_DRAW	(PFFSTATSVAL_START + 3540)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_ISECT	(PFFSTATSVAL_START + 3550)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_DBASE	(PFFSTATSVAL_START + 3560)


#define PFFSTATSVAL_DB				(PFFSTATSVAL_START + 6000)

#define PFFSTATSVAL_VISIBLE			(PFFSTATSVAL_START + 7100)
#define PFFSTATSVAL_VISIBLE_TOTAL		(PFFSTATSVAL_START + 7110)
#define PFFSTATSVAL_VISIBLE_BBOARDS		(PFFSTATSVAL_START + 7120)
#define PFFSTATSVAL_VISIBLE_BBOARD_GSETS	(PFFSTATSVAL_START + 7130)
#define PFFSTATSVAL_VISIBLE_GEODES		(PFFSTATSVAL_START + 7140)
#define PFFSTATSVAL_VISIBLE_GROUPS		(PFFSTATSVAL_START + 7150)
#define PFFSTATSVAL_VISIBLE_SWITCHES		(PFFSTATSVAL_START + 7160)
#define PFFSTATSVAL_VISIBLE_LODS		(PFFSTATSVAL_START + 7170)
#define PFFSTATSVAL_VISIBLE_LODS_FADE		(PFFSTATSVAL_START + 7180)
#define PFFSTATSVAL_VISIBLE_SEQUENCES		(PFFSTATSVAL_START + 7190)
#define PFFSTATSVAL_VISIBLE_LAYERS		(PFFSTATSVAL_START + 7200)
#define PFFSTATSVAL_VISIBLE_DCS			(PFFSTATSVAL_START + 7210)
#define PFFSTATSVAL_VISIBLE_FCS			(PFFSTATSVAL_START + 7215)
#define PFFSTATSVAL_VISIBLE_SCS			(PFFSTATSVAL_START + 7220)
#define PFFSTATSVAL_VISIBLE_LPOINTNODES_TOTAL	(PFFSTATSVAL_START + 7230)
#define PFFSTATSVAL_VISIBLE_LPOINTNODES_OMNIDIR	(PFFSTATSVAL_START + 7240)
#define PFFSTATSVAL_VISIBLE_LPOINTNODES_UNIDIR	(PFFSTATSVAL_START + 7250)
#define PFFSTATSVAL_VISIBLE_LPOINTNODES_BIDIR	(PFFSTATSVAL_START + 7260)
#define PFFSTATSVAL_VISIBLE_LPOINTS_TOTAL	(PFFSTATSVAL_START + 7270)
#define PFFSTATSVAL_VISIBLE_LPOINTS_OMNIDIR	(PFFSTATSVAL_START + 7280)
#define PFFSTATSVAL_VISIBLE_LPOINTS_UNIDIR	(PFFSTATSVAL_START + 7290)
#define PFFSTATSVAL_VISIBLE_LPOINTS_BIDIR	(PFFSTATSVAL_START + 7300)
#define PFFSTATSVAL_VISIBLE_LSOURCES		(PFFSTATSVAL_START + 7310)
#define PFFSTATSVAL_VISIBLE_PARTITIONS		(PFFSTATSVAL_START + 7320)
#define PFFSTATSVAL_VISIBLE_TEXTS		(PFFSTATSVAL_START + 7330)

#define PFFSTATSVAL_EVALUATED			(PFFSTATSVAL_START + 8000)

#define PFFSTATSVAL_EVALUATED_PROC		(PFFSTATSVAL_START + 8100)
#define PFFSTATSVAL_EVALUATED_PROC_BBOARDS	(PFFSTATSVAL_START + 8120)
#define PFFSTATSVAL_EVALUATED_PROC_BBOARD_GSETS	(PFFSTATSVAL_START + 8130)
#define PFFSTATSVAL_EVALUATED_PROC_GEODES	(PFFSTATSVAL_START + 8140)
#define PFFSTATSVAL_EVALUATED_PROC_GROUPS	(PFFSTATSVAL_START + 8150)
#define PFFSTATSVAL_EVALUATED_PROC_SWITCHES	(PFFSTATSVAL_START + 8160)
#define PFFSTATSVAL_EVALUATED_PROC_LODS		(PFFSTATSVAL_START + 8170)
#define PFFSTATSVAL_EVALUATED_PROC_LODS_FADE	(PFFSTATSVAL_START + 8180)
#define PFFSTATSVAL_EVALUATED_PROC_SEQUENCES	(PFFSTATSVAL_START + 8190)
#define PFFSTATSVAL_EVALUATED_PROC_LAYERS	(PFFSTATSVAL_START + 8200)
#define PFFSTATSVAL_EVALUATED_PROC_DCS		(PFFSTATSVAL_START + 8210)
#define PFFSTATSVAL_EVALUATED_PROC_FCS		(PFFSTATSVAL_START + 8215)
#define PFFSTATSVAL_EVALUATED_PROC_SCS		(PFFSTATSVAL_START + 8220)
#define PFFSTATSVAL_EVALUATED_PROC_LPOINTNODES	(PFFSTATSVAL_START + 8230)
#define PFFSTATSVAL_EVALUATED_PROC_LPOINTNODES_OMNIDIR	(PFFSTATSVAL_START + 8240)
#define PFFSTATSVAL_EVALUATED_PROC_LPOINTNODES_UNIDIR	(PFFSTATSVAL_START + 8250)
#define PFFSTATSVAL_EVALUATED_PROC_LPOINTNODES_BIDIR	(PFFSTATSVAL_START + 8260)
#define PFFSTATSVAL_EVALUATED_PROC_LPOINTS	(PFFSTATSVAL_START + 8270)
#define PFFSTATSVAL_EVALUATED_PROC_LPOINTS_OMNIDIR  (PFFSTATSVAL_START + 8280)
#define PFFSTATSVAL_EVALUATED_PROC_LPOINTS_UNIDIR   (PFFSTATSVAL_START + 8290)
#define PFFSTATSVAL_EVALUATED_PROC_LPOINTS_BIDIR    (PFFSTATSVAL_START + 8300)
#define PFFSTATSVAL_EVALUATED_PROC_LSOURCES	(PFFSTATSVAL_START + 8310)
#define PFFSTATSVAL_EVALUATED_PROC_PARTITIONS	(PFFSTATSVAL_START + 8320)
#define PFFSTATSVAL_EVALUATED_PROC_TEXTS	(PFFSTATSVAL_START + 8330)

#define PFFSTATSVAL_EVALUATED_APP		(PFFSTATSVAL_START + 8400)
#define PFFSTATSVAL_EVALUATED_APP_TOTAL		(PFFSTATSVAL_START + 8410)
#define PFFSTATSVAL_EVALUATED_APP_BBOARDS	(PFFSTATSVAL_START + 8420)
#define PFFSTATSVAL_EVALUATED_APP_BBOARD_GSETS	(PFFSTATSVAL_START + 8430)
#define PFFSTATSVAL_EVALUATED_APP_GEODES	(PFFSTATSVAL_START + 8440)
#define PFFSTATSVAL_EVALUATED_APP_GROUPS	(PFFSTATSVAL_START + 8450)
#define PFFSTATSVAL_EVALUATED_APP_SWITCHES	(PFFSTATSVAL_START + 8460)
#define PFFSTATSVAL_EVALUATED_APP_LODS		(PFFSTATSVAL_START + 8470)
#define PFFSTATSVAL_EVALUATED_APP_LODS_FADE	(PFFSTATSVAL_START + 8480)
#define PFFSTATSVAL_EVALUATED_APP_SEQUENCES	(PFFSTATSVAL_START + 8490)
#define PFFSTATSVAL_EVALUATED_APP_LAYERS	(PFFSTATSVAL_START + 8500)
#define PFFSTATSVAL_EVALUATED_APP_DCS		(PFFSTATSVAL_START + 8510)
#define PFFSTATSVAL_EVALUATED_APP_FCS		(PFFSTATSVAL_START + 8515)
#define PFFSTATSVAL_EVALUATED_APP_SCS		(PFFSTATSVAL_START + 8520)
#define PFFSTATSVAL_EVALUATED_APP_LPOINTNODES	(PFFSTATSVAL_START + 8530)
#define PFFSTATSVAL_EVALUATED_APP_LPOINTNODES_OMNIDIR	(PFFSTATSVAL_START + 8540)
#define PFFSTATSVAL_EVALUATED_APP_LPOINTNODES_UNIDIR	(PFFSTATSVAL_START + 8550)
#define PFFSTATSVAL_EVALUATED_APP_LPOINTNODES_BIDIR	(PFFSTATSVAL_START + 8560)
#define PFFSTATSVAL_EVALUATED_APP_LPOINTS	(PFFSTATSVAL_START + 8570)
#define PFFSTATSVAL_EVALUATED_APP_LPOINTS_OMNIDIR	(PFFSTATSVAL_START + 8580)
#define PFFSTATSVAL_EVALUATED_APP_LPOINTS_UNIDIR	(PFFSTATSVAL_START + 8590)
#define PFFSTATSVAL_EVALUATED_APP_LPOINTS_BIDIR	(PFFSTATSVAL_START + 8600)
#define PFFSTATSVAL_EVALUATED_APP_LSOURCES	(PFFSTATSVAL_START + 8610)
#define PFFSTATSVAL_EVALUATED_APP_PARTITIONS	(PFFSTATSVAL_START + 8620)
#define PFFSTATSVAL_EVALUATED_APP_TEXTS	        (PFFSTATSVAL_START + 8630)

#define PFFSTATSVAL_EVALUATED_CULL		(PFFSTATSVAL_START + 8700)
#define PFFSTATSVAL_EVALUATED_CULL_TOTAL	(PFFSTATSVAL_START + 8710)
#define PFFSTATSVAL_EVALUATED_CULL_BBOARDS	(PFFSTATSVAL_START + 8720)
#define PFFSTATSVAL_EVALUATED_CULL_BBOARD_GSETS	(PFFSTATSVAL_START + 8730)
#define PFFSTATSVAL_EVALUATED_CULL_GEODES	(PFFSTATSVAL_START + 8740)
#define PFFSTATSVAL_EVALUATED_CULL_GROUPS	(PFFSTATSVAL_START + 8750)
#define PFFSTATSVAL_EVALUATED_CULL_SWITCHES	(PFFSTATSVAL_START + 8760)
#define PFFSTATSVAL_EVALUATED_CULL_LODS		(PFFSTATSVAL_START + 8770)
#define PFFSTATSVAL_EVALUATED_CULL_LODS_FADE	(PFFSTATSVAL_START + 8780)
#define PFFSTATSVAL_EVALUATED_CULL_SEQUENCES	(PFFSTATSVAL_START + 8790)
#define PFFSTATSVAL_EVALUATED_CULL_LAYERS	(PFFSTATSVAL_START + 8800)
#define PFFSTATSVAL_EVALUATED_CULL_DCS		(PFFSTATSVAL_START + 8810)
#define PFFSTATSVAL_EVALUATED_CULL_FCS		(PFFSTATSVAL_START + 8815)
#define PFFSTATSVAL_EVALUATED_CULL_SCS		(PFFSTATSVAL_START + 8820)
#define PFFSTATSVAL_EVALUATED_CULL_LPOINTNODES	(PFFSTATSVAL_START + 8830)
#define PFFSTATSVAL_EVALUATED_CULL_LPOINTNODES_OMNIDIR	(PFFSTATSVAL_START + 8840)
#define PFFSTATSVAL_EVALUATED_CULL_LPOINTNODES_UNIDIR	(PFFSTATSVAL_START + 8850)
#define PFFSTATSVAL_EVALUATED_CULL_LPOINTNODES_BIDIR	(PFFSTATSVAL_START + 8860)
#define PFFSTATSVAL_EVALUATED_CULL_LPOINTS	(PFFSTATSVAL_START + 8870)
#define PFFSTATSVAL_EVALUATED_CULL_LPOINTS_OMNIDIR	(PFFSTATSVAL_START + 8880)
#define PFFSTATSVAL_EVALUATED_CULL_LPOINTS_UNIDIR	(PFFSTATSVAL_START + 8890)
#define PFFSTATSVAL_EVALUATED_CULL_LPOINTS_BIDIR	(PFFSTATSVAL_START + 8900)
#define PFFSTATSVAL_EVALUATED_CULL_LSOURCES	(PFFSTATSVAL_START + 8910)
#define PFFSTATSVAL_EVALUATED_CULL_PARTITIONS	(PFFSTATSVAL_START + 8920)
#define PFFSTATSVAL_EVALUATED_CULL_TEXTS	(PFFSTATSVAL_START + 8930)

#define PFFSTATSVAL_EVALUATED_DRAW		(PFFSTATSVAL_START + 9000)
#define PFFSTATSVAL_EVALUATED_DRAW_TOTAL	(PFFSTATSVAL_START + 9010)
#define PFFSTATSVAL_EVALUATED_DRAW_BBOARDS	(PFFSTATSVAL_START + 9020)
#define PFFSTATSVAL_EVALUATED_DRAW_BBOARD_GSETS	(PFFSTATSVAL_START + 9030)
#define PFFSTATSVAL_EVALUATED_DRAW_GEODES	(PFFSTATSVAL_START + 9040)
#define PFFSTATSVAL_EVALUATED_DRAW_GROUPS	(PFFSTATSVAL_START + 9050)
#define PFFSTATSVAL_EVALUATED_DRAW_SWITCHES	(PFFSTATSVAL_START + 9060)
#define PFFSTATSVAL_EVALUATED_DRAW_LODS		(PFFSTATSVAL_START + 9070)
#define PFFSTATSVAL_EVALUATED_DRAW_LODS_FADE	(PFFSTATSVAL_START + 9080)
#define PFFSTATSVAL_EVALUATED_DRAW_SEQUENCES	(PFFSTATSVAL_START + 9090)
#define PFFSTATSVAL_EVALUATED_DRAW_LAYERS	(PFFSTATSVAL_START + 9100)
#define PFFSTATSVAL_EVALUATED_DRAW_DCS		(PFFSTATSVAL_START + 9110)
#define PFFSTATSVAL_EVALUATED_DRAW_FCS		(PFFSTATSVAL_START + 9115)
#define PFFSTATSVAL_EVALUATED_DRAW_SCS		(PFFSTATSVAL_START + 9120)
#define PFFSTATSVAL_EVALUATED_DRAW_LPOINTNODES	(PFFSTATSVAL_START + 9130)
#define PFFSTATSVAL_EVALUATED_DRAW_LPOINTNODES_OMNIDIR	(PFFSTATSVAL_START + 9140)
#define PFFSTATSVAL_EVALUATED_DRAW_LPOINTNODES_UNIDIR	(PFFSTATSVAL_START + 9150)
#define PFFSTATSVAL_EVALUATED_DRAW_LPOINTNODES_BIDIR	(PFFSTATSVAL_START + 9160)
#define PFFSTATSVAL_EVALUATED_DRAW_LPOINTS	(PFFSTATSVAL_START + 9170)
#define PFFSTATSVAL_EVALUATED_DRAW_LPOINTS_OMNIDIR	(PFFSTATSVAL_START + 9180)
#define PFFSTATSVAL_EVALUATED_DRAW_LPOINTS_UNIDIR	(PFFSTATSVAL_START + 9190)
#define PFFSTATSVAL_EVALUATED_DRAW_LPOINTS_BIDIR	(PFFSTATSVAL_START + 9200)
#define PFFSTATSVAL_EVALUATED_DRAW_LSOURCES	(PFFSTATSVAL_START + 9210)
#define PFFSTATSVAL_EVALUATED_DRAW_PARTITIONS	(PFFSTATSVAL_START + 9220)
#define PFFSTATSVAL_EVALUATED_DRAW_TEXTS    	(PFFSTATSVAL_START + 9230)

#define PFFSTATSVAL_EVALUATED_ISECT		(PFFSTATSVAL_START + 10000)
#define PFFSTATSVAL_EVALUATED_ISECT_TOTAL	(PFFSTATSVAL_START + 10010)
#define PFFSTATSVAL_EVALUATED_ISECT_BBOARDS	(PFFSTATSVAL_START + 10020)
#define PFFSTATSVAL_EVALUATED_ISECT_BBOARD_GSETS (PFFSTATSVAL_START + 10030)
#define PFFSTATSVAL_EVALUATED_ISECT_GEODES	(PFFSTATSVAL_START + 10040)
#define PFFSTATSVAL_EVALUATED_ISECT_GROUPS	(PFFSTATSVAL_START + 10050)
#define PFFSTATSVAL_EVALUATED_ISECT_SWITCHES	(PFFSTATSVAL_START + 10060)
#define PFFSTATSVAL_EVALUATED_ISECT_LODS	(PFFSTATSVAL_START + 10070)
#define PFFSTATSVAL_EVALUATED_ISECT_LODS_FADE	(PFFSTATSVAL_START + 10080)
#define PFFSTATSVAL_EVALUATED_ISECT_SEQUENCES	(PFFSTATSVAL_START + 10090)
#define PFFSTATSVAL_EVALUATED_ISECT_LAYERS	(PFFSTATSVAL_START + 10100)
#define PFFSTATSVAL_EVALUATED_ISECT_DCS		(PFFSTATSVAL_START + 10110)
#define PFFSTATSVAL_EVALUATED_ISECT_FCS		(PFFSTATSVAL_START + 10115)
#define PFFSTATSVAL_EVALUATED_ISECT_SCS		(PFFSTATSVAL_START + 10120)
#define PFFSTATSVAL_EVALUATED_ISECT_LPOINTNODES	(PFFSTATSVAL_START + 10130)
#define PFFSTATSVAL_EVALUATED_ISECT_LPOINTNODES_OMNIDIR	(PFFSTATSVAL_START + 10140)
#define PFFSTATSVAL_EVALUATED_ISECT_LPOINTNODES_UNIDIR	(PFFSTATSVAL_START + 10150)
#define PFFSTATSVAL_EVALUATED_ISECT_LPOINTNODES_BIDIR	(PFFSTATSVAL_START + 10160)
#define PFFSTATSVAL_EVALUATED_ISECT_LPOINTS	(PFFSTATSVAL_START + 10170)
#define PFFSTATSVAL_EVALUATED_ISECT_LPOINTS_OMNIDIR	(PFFSTATSVAL_START + 10180)
#define PFFSTATSVAL_EVALUATED_ISECT_LPOINTS_UNIDIR	(PFFSTATSVAL_START + 10190)
#define PFFSTATSVAL_EVALUATED_ISECT_LPOINTS_BIDIR	(PFFSTATSVAL_START + 10200)
#define PFFSTATSVAL_EVALUATED_ISECT_LSOURCES	(PFFSTATSVAL_START + 10210)
#define PFFSTATSVAL_EVALUATED_ISECT_PARTITIONS	(PFFSTATSVAL_START + 10220)
#define PFFSTATSVAL_EVALUATED_ISECT_TEXTS	(PFFSTATSVAL_START + 10230)

#define PFFSTATSVAL_CULL			(PFFSTATSVAL_START + 12000)
#define PFFSTATSVAL_CULLTRAV			(PFFSTATSVAL_START + 12100)
#define PFFSTATSVAL_CULLTRAV_TOTAL		(PFFSTATSVAL_START + 12110)
#define PFFSTATSVAL_CULLTRAV_BBOARDS		(PFFSTATSVAL_START + 12120)
#define PFFSTATSVAL_CULLTRAV_BBOARD_GSETS	(PFFSTATSVAL_START + 12130)
#define PFFSTATSVAL_CULLTRAV_GEODES		(PFFSTATSVAL_START + 12140)
#define PFFSTATSVAL_CULLTRAV_GROUPS		(PFFSTATSVAL_START + 12150)
#define PFFSTATSVAL_CULLTRAV_SWITCHES		(PFFSTATSVAL_START + 12160)
#define PFFSTATSVAL_CULLTRAV_LODS		(PFFSTATSVAL_START + 12170)
#define PFFSTATSVAL_CULLTRAV_LODS_FADE		(PFFSTATSVAL_START + 12180)
#define PFFSTATSVAL_CULLTRAV_SEQUENCES		(PFFSTATSVAL_START + 12190)
#define PFFSTATSVAL_CULLTRAV_LAYERS		(PFFSTATSVAL_START + 12200)
#define PFFSTATSVAL_CULLTRAV_DCS		(PFFSTATSVAL_START + 12210)
#define PFFSTATSVAL_CULLTRAV_FCS		(PFFSTATSVAL_START + 12215)
#define PFFSTATSVAL_CULLTRAV_SCS		(PFFSTATSVAL_START + 12220)
#define PFFSTATSVAL_CULLTRAV_LPOINTNODES	(PFFSTATSVAL_START + 12230)
#define PFFSTATSVAL_CULLTRAV_LPOINTNODES_OMNIDIR    (PFFSTATSVAL_START + 12240)
#define PFFSTATSVAL_CULLTRAV_LPOINTNODES_UNIDIR	(PFFSTATSVAL_START + 12250)
#define PFFSTATSVAL_CULLTRAV_LPOINTNODES_BIDIR	(PFFSTATSVAL_START + 12260)
#define PFFSTATSVAL_CULLTRAV_LSOURCES		(PFFSTATSVAL_START + 12270)
#define PFFSTATSVAL_CULLTRAV_PARTITIONS		(PFFSTATSVAL_START + 12280)
#define PFFSTATSVAL_CULLTRAV_TEXTS		(PFFSTATSVAL_START + 12290)

#define PFFSTATSVAL_CULLTEST_NODES		(PFFSTATSVAL_START + 12400)
#define PFFSTATSVAL_CULLTEST_NODES_TOTAL	(PFFSTATSVAL_START + 12410)
#define PFFSTATSVAL_CULLTEST_NODES_IN		(PFFSTATSVAL_START + 12420)
#define PFFSTATSVAL_CULLTEST_NODES_OUT		(PFFSTATSVAL_START + 12430)
#define PFFSTATSVAL_CULLTEST_NODES_PARTIAL	(PFFSTATSVAL_START + 12440)

#define PFFSTATSVAL_CULLTEST_GSETS		(PFFSTATSVAL_START + 12500)
#define PFFSTATSVAL_CULLTEST_GSETS_TOTAL	(PFFSTATSVAL_START + 12510)
#define PFFSTATSVAL_CULLTEST_GSETS_IN		(PFFSTATSVAL_START + 12520)
#define PFFSTATSVAL_CULLTEST_GSETS_OUT		(PFFSTATSVAL_START + 12530)
#define PFFSTATSVAL_CULLTEST_GSETS_PARTIAL	(PFFSTATSVAL_START + 12540)

#define PFFSTATSVAL_CULL_SORTED_GSETS		(PFFSTATSVAL_START + 12600)


#define PFFSTATSVAL_NUM_TOKENS			(PFSTATSVAL_NUM_TOKENS + 10000000)

#endif /* !PF_CPLUSPLUS_API */
#endif /* PF_C_API */

#ifdef __cplusplus
}
#endif

#endif /* !__PFSTATS_H__ */


