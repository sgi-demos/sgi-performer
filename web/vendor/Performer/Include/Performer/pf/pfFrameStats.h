//
// Copyright 1995, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfFrameStats.h		Stats include file
//
// $Revision: 1.141 $
// $Date: 2002/11/06 22:47:16 $
//

#ifndef __PF_FRAMESTATS_H__
#define __PF_FRAMESTATS_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#ifdef __sgi
#include <bstring.h>
#else
#include <string.h>
#endif  /* __linux__ */


#include <Performer/pf/pfUpdatable.h>
#include <Performer/pf/pfPipe.h>
#include <Performer/pr/pfStats.h>


class pfScene;
class pfChannel;
class _pfCuller;
class pfFrameStats;
struct pfChanUpBuffer;



extern "C" {
/* -------------- pfFrameStats Tokens and Structs -------------------- */
/* 
 * pfFrameStats is inherited from pfStats of libpr
 */
 

#define PFRTMON_BASE     	   1000


/* pfFStatsClassMode() */

#define PFFSTATS_CLASSES_START	    100
#define PFFSTATS_CLASSES	    (0 + PFFSTATS_CLASSES_START)
#define PFFSTATS_DB		    (1 + PFFSTATS_CLASSES_START)

#define PFFSTATS_CULL		    (3 + PFFSTATS_CLASSES_START)
#define PFFSTATS_PFTIMES	    (4 + PFFSTATS_CLASSES_START)

#define PFFSTATS_RTMON	    	    (6 + PFFSTATS_CLASSES_START)
#define PFFSTATS_NUM_CLASSES	    7
#define PFFSTATS_TOTAL_CLASSES	    (PFFSTATS_NUM_CLASSES + PFFSTATS_CLASSES_START)

/* pfFStatsClass and pfChanStatsMode() */

#define PFFSTATS_ENDB		    (0x01 << PFSTATS_EN_BITS)
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

/* rtmon stats class */
#define PFFSTATS_RTMON_BASIC	    0x10000000
#define PFFSTATS_RTMON_MASK	    0x10000000   

/* host timestamps */
#define PFFSTATS_HTSTAMP_PROC_APP	    0x00100000
#define PFFSTATS_HTSTAMP_PROC_CULL	    0x00200000
#define PFFSTATS_HTSTAMP_PROC_DRAW	    0x00300000
#define PFFSTATS_HTSTAMP_PROC_LPOINT	    0x00400000
#define PFFSTATS_HTSTAMP_PROC_ISECT	    0x00500000
#define PFFSTATS_HTSTAMP_PROC_DBASE	    0x00600000
#define PFFSTATS_HTSTAMP_PROC_COMPUTE       0x00700000
#define PFFSTATS_HTSTAMP_PROC_MASK          0x00800000

/* user tstamps need to have high bit set */
#define PFFSTATS_HTSTAMP_USER       0x80000000

#define PFFSTATS_HTSTAMP_LOC_START	    0x00000001
#define PFFSTATS_HTSTAMP_LOC_PRE	    0x00000002
#define PFFSTATS_HTSTAMP_LOC_POST	    0x00000003
#define PFFSTATS_HTSTAMP_LOC_END	    0x00000004
#define PFFSTATS_HTSTAMP_LOC_MASK    	    0x0000000f


#define PFFSTATS_MASK		    (PFFSTATS_PFTIMES_MASK | \
				     PFFSTATS_DB_MASK |      \
				     PFFSTATS_CULL_MASK |    \
				     PFFSTATS_RTMON_MASK |    \
				     PFSTATS_MASK)
								
/* pfFStatsAttr() */
#define PFFSTATS_UPDATE_FRAMES		2000
#define PFFSTATS_UPDATE_SECS		3000
#define PFFSTATS_PFTIMES_HIST_FRAMES	4000

/* pfChanStatsMode() */
#define PFCSTATS_DRAW 1

typedef struct pfFStatsValProc
{
    float 	total;		/* PFFSTATSVAL_*_TOTAL		*/
    float	app;		/* PFFSTATSVAL_*_APP		*/
    float	cull;		/* PFFSTATSVAL_*_CULL		*/
    float	draw;		/* PFFSTATSVAL_*_DRAW		*/
    float	isect;		/* PFFSTATSVAL_*_ISECT		*/
    float	dbase;		/* PFFSTATSVAL_*_DBASE		*/
    float	lpoint;		/* PFFSTATSVAL_*_LPOINT		*/
    float	compute;	/* PFFSTATSVAL_*_COMPUTE	*/
} pfFStatsValProc;

typedef struct pfFStatsValNode
{
    float 	total;		    /* PFFSTATSVAL_*_TOTAL		    */
    float	asds;		    /* PFFSTATSVAL_*_ASDS		    */
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
    float 	nodes;		    /* PFFSTATSVAL_*_LSOURCES		    */
    float 	partitions;	    /* PFFSTATSVAL_*_PARTITIONS		    */
    float 	texts;              /* PFFSTATSVAL_*_TEXTS     		    */
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
    int lpoint;
    int compute;
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
    double	startInc, endInc; /* start/end of period to do pipe incremental pipe update for frame */
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
    double	drawLPointEnd;	/* time returning from pfDraw() */
    double	preSwap;	/* just before swapbuffers (with finish) */
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

typedef struct pfFStatsValPFTimesLPoint	/* PFFSTATSVAL_PFTIMES_HIST_LPOINT */
{
    int	frame; /* app frame stamp */
    int pad;
    double	start;	/* time before call pfDBaseFunc() callback */
    double	end;	/* time after call pfDBaseFunc() callback */
} pfFStatsValPFTimesLPoint;

typedef struct pfFStatsValPFTimesCompute	/* PFFSTATSVAL_PFTIMES_HIST_COMPUTE */
{
    int	frame; /* app frame stamp */
    int pad;
    double	start;	/* time before call pfComputeFunc() callback */
    double	end;	/* time after call pfComputeFunc() callback */
} pfFStatsValPFTimesCompute;

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
    /* History of lpoint time stamps - PFFSTATSVAL_PFTIMES_HIST_ALL_LPOINT */
    pfFStatsValPFTimesLPoint lpoint[PFFSTATS_MAX_HIST_FRAMES]; 
    /* History of compute time stamps - PFFSTATSVAL_PFTIMES_HIST_ALL_COMPUTE */
    pfFStatsValPFTimesCompute compute[PFFSTATS_MAX_HIST_FRAMES]; 
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
    /* History of dbase time stamps - PFFSTATSVAL_PFTIMES_HIST_LAST_DBASE */
    pfFStatsValPFTimesDBase dbase; 
    /* History of lpoint time stamps - PFFSTATSVAL_PFTIMES_HIST_LAST_LPOINT */
    pfFStatsValPFTimesLPoint lpoint; 
    /* History of compute time stamps - PFFSTATSVAL_PFTIMES_HIST_LAST_COMPUTE */
    pfFStatsValPFTimesCompute compute; 
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

} // extern C

typedef pfFStatsValProc pfProcStats;

typedef struct pfFrameTimes
{
    double 	start,  end, startDraw;
    // measured from host
    float	dspSecs, delay;		    
    pfProcStats	proc; 
    pfProcStats	appStamp; 
    pfProcStats numFrames;
    // draw time as measured from pipe
    float	geomTime, fillTime, drawTime;
    int	missedFrame;
    pfProcStats	misses;
} pfFrameTimes;

typedef struct pfFStatsValNode pfNodeCounts;
#define _PFFSTATS_NUM_NODE_COUNTS (sizeof(pfNodeCounts)/sizeof(float))

typedef struct pfFStatsValMem pfMemFStats;
#define _PFFSTATS_NUM_MEM_COUNTS (PFFSTATSVAL_MEM_COUNTS)

typedef pfFStatsValPFTimesHist pfMPHistStats;

struct _pfFrameStatsData 
{
    int 	frame; // must be first
    int 	nframes;
	
    pfMemFStats		mem;
    // GfxPipe Bottleneck
    pfStatsValGfxPipeTimes  btlnck;
    // DB counts - count these in _PFFSTATS_NUM_DB_COUNTS below
    pfNodeCounts 	nodesVisible;
    pfNodeCounts	nodesAppCleaned;
    pfNodeCounts	nodesAppUpdated;
    pfNodeCounts	nodesEvaluatedApp;
    pfNodeCounts	nodesEvaluatedClean;
    pfNodeCounts	nodesEvaluatedCull;
    pfNodeCounts	nodesEvaluatedDraw;
    pfNodeCounts	nodesEvaluatedIsect;
    pfNodeCounts	nodesEvaluatedCompute;
    float		appCBs;
    float		cullCBs;
    float		drawCBs;
    // avg time spent drawing chan stats
    // Cull counts
    pfNodeCounts 	nodesCullTraversed;
    pfIsectCounts 	nodeCullCounts;
    pfIsectCounts 	gsetCullCounts;
    float 		numSortedGSets;
    float		statsDraw, drawProcStart; 
    int			numStatsDraws;
    // Process Frame Time counts -- Must be last stats
    //	    before end !!!
    pfFrameTimes	frameTimes;
    // marks end of counts data
    int			end; 
    // these needs to be last 
    int			dirtyCount;
    pfStats*		prStats;  
    
    //CAPI:private
    void    clear(uint _which);
    void    copy(const _pfFrameStatsData *_src, uint _which);
    void    accumulate(_pfFrameStatsData *_new, 
		       uint _which, uint _mode[PFFSTATS_TOTAL_CLASSES]);
    void    average(_pfFrameStatsData *_cum, uint _which, 
		    uint _mode[PFFSTATS_TOTAL_CLASSES], int _numFrames);
};


#define PFFRAMESTATS ((pfFrameStats*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFFRAMESTATSBUFFER ((pfFrameStats*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfFrameStats : public pfUpdatable
{
public:

    uint setClass(uint _mask, int _val)  {
        return PFFRAMESTATS->nb_setClass(_mask, _val);
    }

    uint getClass(uint _emask)  {
        return PFFRAMESTATS->nb_getClass(_emask);
    }

    uint setClassMode(int _class, uint _mask, int _val)  {
        return PFFRAMESTATS->nb_setClassMode(_class, _mask, _val);
    }

    uint getClassMode(int _class)  {
        return PFFRAMESTATS->nb_getClassMode(_class);
    }

    void setAttr(int _attr, float _val)  {
        PFFRAMESTATS->nb_setAttr(_attr, _val);
    }

    float getAttr(int _attr)  {
        return PFFRAMESTATS->nb_getAttr(_attr);
    }

    uint getOpen(uint _emask)  {
        return PFFRAMESTATS->nb_getOpen(_emask);
    }

    uint open(uint _enmask)  {
        return PFFRAMESTATS->nb_open(_enmask);
    }

    uint close(uint _enmask)  {
        return nb_close(_enmask);
    }

    void reset()  {
        PFFRAMESTATS->nb_reset();
    }

    void clear(uint _which)  {
        PFFRAMESTATS->nb_clear(_which);
    }

    void accumulate(pfFrameStats* _src, unsigned int _which)  {
        PFFRAMESTATS->nb_accumulate(_src, _which);
    }

    void average(pfFrameStats* _src, unsigned int _which, int _num)  {
        PFFRAMESTATS->nb_average(_src, _which, _num);
    }

    void RTMonTStamp(uint tstamp) const  {
        PFFRAMESTATS->nb_RTMonTStamp(tstamp);
    }

    void RTMonTStamp(uint tstamp, int pnum,  int chanNum)  {
        nb_RTMonTStamp(tstamp, pnum, chanNum);
    }

    void count(pfGeoSet *gset)  {
        PFFRAMESTATS->nb_count(gset);
    }

    void subtract(int tris,int lines,int points,int verts)  {
        PFFRAMESTATS->nb_subtract(tris, lines, points, verts);
    }

    int query(uint _which, void *_dst, int _size)  {
        return PFFRAMESTATS->nb_query(_which, _dst, _size);
    }

    int mQuery(uint *_which, void *_dst, int _size)  {
        return PFFRAMESTATS->nb_mQuery(_which, _dst, _size);
    }

    void draw(pfChannel *_chan)  {
        PFFRAMESTATS->nb_draw(_chan);
    }

    void copy(const pfFrameStats *_prev, uint _dstBufSel, uint _srcBufSel, uint _which)  {
        PFFRAMESTATS->nb_copy(_prev, _dstBufSel, _srcBufSel, _which);
    }

    void countNode(int _class, uint _mode, pfNode * _node)  {
        PFFRAMESTATS->nb_countNode(_class, _mode, _node);
    }
public:		// Constructors/Destructors
    //CAPI:basename FStats
    //CAPI:updatable
    //CAPI:newargs
    pfFrameStats();
    virtual ~pfFrameStats(void);

protected:
    pfFrameStats(pfBuffer *_buf);
    pfFrameStats(const pfFrameStats *_prev, pfBuffer *_buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }
    
public:		// pfMemory virtual functions
    virtual int 	 checkDelete(void);
    virtual int		 print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	 pf_applyUpdate(const pfUpdatable *_prev, int _upId);
    virtual pfUpdatable* pf_bufferClone(pfBuffer *_buf);
    virtual void	 pf_copyToDraw(const pfUpdatable *_prev);
    virtual void	 pf_updateBufferList(pfBuffer *buf, int addRemove) {
        buf->pf_updateList(this, addRemove);
    }

		/***** parent class copy routines - don't copy what we don't know ****/
    virtual int copy(const pfMemory *_src) {return ((pfMemory*)this)->copy(_src);}
    virtual void copy(const pfObject *_src) {((pfObject*)this)->copy(_src);}

PFINTERNAL:		// pfStats lookalike sets and gets
    uint 		 nb_setClass(uint _mask, int _val);
    uint 		 nb_getClass(uint _emask); // safe 
    uint 		 nb_setClassMode(int _class, uint _mask, int _val); // checks for legal vals 
    uint 		 nb_getClassMode(int _class);

    void 		 nb_setAttr(int _attr, float _val);
    float 		 nb_getAttr(int _attr);
    // CAPI:verb GetOpenFStats
    uint 		 nb_getOpen(uint _emask); // safe 

PFINTERNAL:		// pfStats lookalike functions
    //CAPI:verb
    uint 		 nb_open(uint _enmask);
    static uint		 nb_close(uint _enmask) {
	    return pfStats::close(_enmask);
    }
    void 		 nb_reset(void);
    void 		 nb_clear(uint _which);
    void 		 nb_accumulate(pfFrameStats* _src, unsigned int _which);
    void 		 nb_average(pfFrameStats* _src, unsigned int _which, int _num);
    void		 nb_RTMonTStamp(uint tstamp) const;
    
    // CAPI:verb RTMonTStamp
    static void		 nb_RTMonTStamp(uint tstamp, int pnum,  int chanNum);
    
    // CAPI:verb FStatsCountGSet
    void		 nb_count(pfGeoSet *gset) {
	cur.prStats->count(gset);
    }
    // CAPI:verb FStatsSubtract
    void		 nb_subtract(int tris,int lines,int points,int verts); 

    //CAPI:verb
    int			nb_query(uint _which, void *_dst, int _size);
    int			nb_mQuery(uint *_which, void *_dst, int _size);

PFINTERNAL:		// pfFrameStats functions
    //CAPI:verb
    void		 nb_draw(pfChannel *_chan);
    void		 nb_copy(const pfFrameStats *_prev, uint _dstBufSel, uint _srcBufSel, uint _which);
    //CAPI:noverb
    void		 nb_countNode(int _class, uint _mode, pfNode * _node);


private:
    // this is for the GE timing mode for RE
    int		geMode;
    // and is set each frame 
    // in the draw process and then
    // propagated back up
    // stats enable settings
    uint		enable;
    // mask of opened classes
    uint		openMask;
    // multiprocess configuration
    uint		mpconfig; 
    char		mpstring[80];
    // flags for dirty modes and data for UPDATE  
    uint		dirtyFlags;
    // mask of fields to clear due to mode changes
    uint		clearFlag; 
    // mask of stats that have been accumulated 
    // to limit data copied upstream to app
    uint		newStats; 
    // cache of open modes
    uint		omode[PFFSTATS_TOTAL_CLASSES];
    // cache of enabled modes
    uint		emode[PFFSTATS_TOTAL_CLASSES];
    uint		mode[PFFSTATS_TOTAL_CLASSES];
    
    _pfUpdatableRef	parent;	// currently always a channel		   
    
    // store start time of prev frame
    //		used for counting extended frames
    double		periodAppStart, afterSwap, prevSwap; 
    int			prevMissed;
    
    float		videoRate; 
    
    pfMPHistStats	mp;	  // Process times history
    _pfFrameStatsData	avg;	  // averaged stats for last update period
    _pfFrameStatsData	cur;	  // in-progress stats
    _pfFrameStatsData	cum;	  // stats accumulated each frame
    _pfFrameStatsData	prev;	  // stats accumulated each frame
    _pfFrameStatsData	prevCull; // for CULLoDRAW mode
    
friend class pfChannel;
    
PFINTERNAL:
    int			updateFrames;
    float		updateSecs;
    pfCalligStats	callig;
    uint32_t		calligLock;	// spin lock for MP callig update lpoint->app


private:
    static pfType *classType;
    
public:
    void *_pfFrameStatsExtraData;
};

PF_UPDATABLE_REF(pfFrameStats, _pfFrameStatsRef);


extern "C" {

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
    pfFStatsValNode	clean;		    /* PFFSTATSVAL_EVALUATED_CLEAN  */
    pfFStatsValNode	cull;		    /* PFFSTATSVAL_EVALUATED_CULL   */
    pfFStatsValNode	draw;		    /* PFFSTATSVAL_EVALUATED_DRAW   */
    pfFStatsValNode	isect;		    /* PFFSTATSVAL_EVALUATED_ISECT  */
    pfFStatsValNode	compute;	    /* PFFSTATSVAL_EVALUATED_COMPUTE */
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
    int			    preDraw, postDraw;
    int			    preCull, postCull;
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
#define PFFSTATSVAL_PFTIMES_NUMFRAMES_LPOINT	(PFFSTATSVAL_START + 2170)

#define PFFSTATSVAL_PFTIMES_APPSTAMP		(PFFSTATSVAL_START + 3000)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_TOTAL	(PFFSTATSVAL_START + 3010)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_APP	(PFFSTATSVAL_START + 3020)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_CULL	(PFFSTATSVAL_START + 3030)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_DRAW	(PFFSTATSVAL_START + 3040)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_ISECT	(PFFSTATSVAL_START + 3050)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_DBASE	(PFFSTATSVAL_START + 3060)
#define PFFSTATSVAL_PFTIMES_APPSTAMP_LPOINT	(PFFSTATSVAL_START + 3070)

#define PFFSTATSVAL_PFTIMES_PROC		(PFFSTATSVAL_START + 3100)
#define PFFSTATSVAL_PFTIMES_PROC_TOTAL		(PFFSTATSVAL_START + 3110)
#define PFFSTATSVAL_PFTIMES_PROC_APP		(PFFSTATSVAL_START + 3120)
#define PFFSTATSVAL_PFTIMES_PROC_CULL		(PFFSTATSVAL_START + 3130)
#define PFFSTATSVAL_PFTIMES_PROC_DRAW		(PFFSTATSVAL_START + 3140)
#define PFFSTATSVAL_PFTIMES_PROC_ISECT		(PFFSTATSVAL_START + 3150)
#define PFFSTATSVAL_PFTIMES_PROC_DBASE		(PFFSTATSVAL_START + 3160)
#define PFFSTATSVAL_PFTIMES_PROC_LPOINT		(PFFSTATSVAL_START + 3170)

#define PFFSTATSVAL_PFTIMES_MISSES		(PFFSTATSVAL_START + 3200)
#define PFFSTATSVAL_PFTIMES_MISSES_TOTAL	(PFFSTATSVAL_START + 3210)
#define PFFSTATSVAL_PFTIMES_MISSES_APP		(PFFSTATSVAL_START + 3220)
#define PFFSTATSVAL_PFTIMES_MISSES_CULL		(PFFSTATSVAL_START + 3230)
#define PFFSTATSVAL_PFTIMES_MISSES_DRAW		(PFFSTATSVAL_START + 3240)
#define PFFSTATSVAL_PFTIMES_MISSES_ISECT	(PFFSTATSVAL_START + 3250)
#define PFFSTATSVAL_PFTIMES_MISSES_DBASE	(PFFSTATSVAL_START + 3260)
#define PFFSTATSVAL_PFTIMES_MISSES_LPOINT	(PFFSTATSVAL_START + 3270)


#define PFFSTATSVAL_PFTIMES_HIST_ALL		(PFFSTATSVAL_START + 3400)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_APPFRAME	(PFFSTATSVAL_START + 3410)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_NFRAMES	(PFFSTATSVAL_START + 3420)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_INDEX	(PFFSTATSVAL_START + 3430)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_APP	(PFFSTATSVAL_START + 3440)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_CULL	(PFFSTATSVAL_START + 3450)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_DRAW	(PFFSTATSVAL_START + 3460)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_ISECT	(PFFSTATSVAL_START + 3470)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_DBASE	(PFFSTATSVAL_START + 3480)
#define PFFSTATSVAL_PFTIMES_HIST_ALL_LPOINT	(PFFSTATSVAL_START + 3490)


#define PFFSTATSVAL_PFTIMES_HIST_LAST		(PFFSTATSVAL_START + 3500)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_APPFRAME	(PFFSTATSVAL_START + 3510)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_APP	(PFFSTATSVAL_START + 3520)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_CULL	(PFFSTATSVAL_START + 3530)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_DRAW	(PFFSTATSVAL_START + 3540)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_ISECT	(PFFSTATSVAL_START + 3550)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_DBASE	(PFFSTATSVAL_START + 3560)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_LPOINT	(PFFSTATSVAL_START + 3570)
#define PFFSTATSVAL_PFTIMES_HIST_LAST_COMPUTE   (PFFSTATSVAL_START + 3580)


#define PFFSTATSVAL_DB				(PFFSTATSVAL_START + 6000)

#define PFFSTATSVAL_VISIBLE			(PFFSTATSVAL_START + 7100)
#define PFFSTATSVAL_VISIBLE_TOTAL		(PFFSTATSVAL_START + 7110)
#define PFFSTATSVAL_VISIBLE_ASDS		(PFFSTATSVAL_START + 7111)
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
#define PFFSTATSVAL_VISIBLE_TEXTS     		(PFFSTATSVAL_START + 7330)

#define PFFSTATSVAL_CALLBACKS     		(PFFSTATSVAL_START + 7400)
#define PFFSTATSVAL_CALLBACKS_APP     		(PFFSTATSVAL_START + 7410)
#define PFFSTATSVAL_CALLBACKS_CULL     		(PFFSTATSVAL_START + 7420)
#define PFFSTATSVAL_CALLBACKS_DRAW     		(PFFSTATSVAL_START + 7430)

#define PFFSTATSVAL_EVALUATED			(PFFSTATSVAL_START + 8000)

#define PFFSTATSVAL_EVALUATED_PROC		(PFFSTATSVAL_START + 8100)
#define PFFSTATSVAL_EVALUATED_PROC_TOTAL        (PFFSTATSVAL_START + 8105)
#define PFFSTATSVAL_EVALUATED_PROC_ASDS		(PFFSTATSVAL_START + 8110)
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
#define PFFSTATSVAL_EVALUATED_PROC_TEXTS     	(PFFSTATSVAL_START + 8330)

#define PFFSTATSVAL_EVALUATED_APP		(PFFSTATSVAL_START + 8400)
#define PFFSTATSVAL_EVALUATED_APP_TOTAL		(PFFSTATSVAL_START + 8410)
#define PFFSTATSVAL_EVALUATED_APP_ASDS		(PFFSTATSVAL_START + 8411)
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
#define PFFSTATSVAL_EVALUATED_APP_TEXTS     	(PFFSTATSVAL_START + 8630)
#define PFFSTATSVAL_EVALUATED_CULL		(PFFSTATSVAL_START + 8700)
#define PFFSTATSVAL_EVALUATED_CULL_TOTAL	(PFFSTATSVAL_START + 8710)
#define PFFSTATSVAL_EVALUATED_CULL_ASDS		(PFFSTATSVAL_START + 8711)
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
#define PFFSTATSVAL_EVALUATED_CULL_TEXTS     	(PFFSTATSVAL_START + 8930)

#define PFFSTATSVAL_EVALUATED_DRAW		(PFFSTATSVAL_START + 9000)
#define PFFSTATSVAL_EVALUATED_DRAW_TOTAL	(PFFSTATSVAL_START + 9010)
#define PFFSTATSVAL_EVALUATED_DRAW_ASDS		(PFFSTATSVAL_START + 9011)
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
#define PFFSTATSVAL_EVALUATED_ISECT_ASDS	(PFFSTATSVAL_START + 10011)
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
#define PFFSTATSVAL_EVALUATED_ISECT_TEXTS     	(PFFSTATSVAL_START + 10230)

#define PFFSTATSVAL_EVALUATED_COMPUTE		(PFFSTATSVAL_START + 11000)
#define PFFSTATSVAL_EVALUATED_COMPUTE_TOTAL	(PFFSTATSVAL_START + 11010)
#define PFFSTATSVAL_EVALUATED_COMPUTE_ASDS	(PFFSTATSVAL_START + 11011)
#define PFFSTATSVAL_EVALUATED_COMPUTE_BBOARDS	(PFFSTATSVAL_START + 11020)
#define PFFSTATSVAL_EVALUATED_COMPUTE_BBOARD_GSETS	(PFFSTATSVAL_START + 11030)
#define PFFSTATSVAL_EVALUATED_COMPUTE_GEODES	(PFFSTATSVAL_START + 11040)
#define PFFSTATSVAL_EVALUATED_COMPUTE_GROUPS	(PFFSTATSVAL_START + 11050)
#define PFFSTATSVAL_EVALUATED_COMPUTE_SWITCHES	(PFFSTATSVAL_START + 11060)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LODS	(PFFSTATSVAL_START + 11070)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LODS_FADE	(PFFSTATSVAL_START + 11080)
#define PFFSTATSVAL_EVALUATED_COMPUTE_SEQUENCES	(PFFSTATSVAL_START + 11090)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LAYERS	(PFFSTATSVAL_START + 11100)
#define PFFSTATSVAL_EVALUATED_COMPUTE_DCS	(PFFSTATSVAL_START + 11110)
#define PFFSTATSVAL_EVALUATED_COMPUTE_FCS	(PFFSTATSVAL_START + 11115)
#define PFFSTATSVAL_EVALUATED_COMPUTE_SCS	(PFFSTATSVAL_START + 11120)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LPOINTNODES	(PFFSTATSVAL_START + 11130)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LPOINTNODES_OMNIDIR	(PFFSTATSVAL_START + 11140)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LPOINTNODES_UNIDIR	(PFFSTATSVAL_START + 11150)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LPOINTNODES_BIDIR	(PFFSTATSVAL_START + 11160)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LPOINTS	(PFFSTATSVAL_START + 11170)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LPOINTS_OMNIDIR	(PFFSTATSVAL_START + 11180)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LPOINTS_UNIDIR	(PFFSTATSVAL_START + 11190)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LPOINTS_BIDIR	(PFFSTATSVAL_START + 11200)
#define PFFSTATSVAL_EVALUATED_COMPUTE_LSOURCES	(PFFSTATSVAL_START + 11210)
#define PFFSTATSVAL_EVALUATED_COMPUTE_PARTITIONS	(PFFSTATSVAL_START + 11220)
#define PFFSTATSVAL_EVALUATED_COMPUTE_TEXTS    	(PFFSTATSVAL_START + 11230)

#define PFFSTATSVAL_EVALUATED_CLEAN		(PFFSTATSVAL_START + 11300)
#define PFFSTATSVAL_EVALUATED_CLEAN_TOTAL		(PFFSTATSVAL_START + 11310)
#define PFFSTATSVAL_EVALUATED_CLEAN_ASDS		(PFFSTATSVAL_START + 11320)
#define PFFSTATSVAL_EVALUATED_CLEAN_BBOARDS	(PFFSTATSVAL_START + 11340)
#define PFFSTATSVAL_EVALUATED_CLEAN_BBOARD_GSETS	(PFFSTATSVAL_START + 11350)
#define PFFSTATSVAL_EVALUATED_CLEAN_GEODES	(PFFSTATSVAL_START + 11360)
#define PFFSTATSVAL_EVALUATED_CLEAN_GROUPS	(PFFSTATSVAL_START + 11370)
#define PFFSTATSVAL_EVALUATED_CLEAN_SWITCHES	(PFFSTATSVAL_START + 11380)
#define PFFSTATSVAL_EVALUATED_CLEAN_LODS		(PFFSTATSVAL_START + 11390)
#define PFFSTATSVAL_EVALUATED_CLEAN_LODS_FADE	(PFFSTATSVAL_START + 11400)
#define PFFSTATSVAL_EVALUATED_CLEAN_SEQUENCES	(PFFSTATSVAL_START + 11410)
#define PFFSTATSVAL_EVALUATED_CLEAN_LAYERS	(PFFSTATSVAL_START + 11420)
#define PFFSTATSVAL_EVALUATED_CLEAN_DCS		(PFFSTATSVAL_START + 11430)
#define PFFSTATSVAL_EVALUATED_CLEAN_FCS		(PFFSTATSVAL_START + 11440)
#define PFFSTATSVAL_EVALUATED_CLEAN_SCS		(PFFSTATSVAL_START + 11450)
#define PFFSTATSVAL_EVALUATED_CLEAN_LPOINTNODES	(PFFSTATSVAL_START + 11460)
#define PFFSTATSVAL_EVALUATED_CLEAN_LPOINTNODES_OMNIDIR	(PFFSTATSVAL_START + 11470)
#define PFFSTATSVAL_EVALUATED_CLEAN_LPOINTNODES_UNIDIR	(PFFSTATSVAL_START + 11480)
#define PFFSTATSVAL_EVALUATED_CLEAN_LPOINTNODES_BIDIR	(PFFSTATSVAL_START + 11490)
#define PFFSTATSVAL_EVALUATED_CLEAN_LPOINTS	(PFFSTATSVAL_START + 11500)
#define PFFSTATSVAL_EVALUATED_CLEAN_LPOINTS_OMNIDIR	(PFFSTATSVAL_START + 11510)
#define PFFSTATSVAL_EVALUATED_CLEAN_LPOINTS_UNIDIR	(PFFSTATSVAL_START + 11520)
#define PFFSTATSVAL_EVALUATED_CLEAN_LPOINTS_BIDIR	(PFFSTATSVAL_START + 11530)
#define PFFSTATSVAL_EVALUATED_CLEAN_LSOURCES	(PFFSTATSVAL_START + 11540)
#define PFFSTATSVAL_EVALUATED_CLEAN_PARTITIONS	(PFFSTATSVAL_START + 11550)
#define PFFSTATSVAL_EVALUATED_CLEAN_TEXTS     	(PFFSTATSVAL_START + 11560)

#define PFFSTATSVAL_CULL			(PFFSTATSVAL_START + 12000)
#define PFFSTATSVAL_CULLTRAV			(PFFSTATSVAL_START + 12100)
#define PFFSTATSVAL_CULLTRAV_TOTAL		(PFFSTATSVAL_START + 12110)
#define PFFSTATSVAL_CULLTRAV_ASDS		(PFFSTATSVAL_START + 12111)
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
#define PFFSTATSVAL_CULLTRAV_LPOINTS            (PFFSTATSVAL_START + 12262)    
#define PFFSTATSVAL_CULLTRAV_LPOINTS_OMNIDIR	(PFFSTATSVAL_START + 12264)
#define PFFSTATSVAL_CULLTRAV_LPOINTS_UNIDIR	(PFFSTATSVAL_START + 12266)
#define PFFSTATSVAL_CULLTRAV_LPOINTS_BIDIR	(PFFSTATSVAL_START + 12268)
#define PFFSTATSVAL_CULLTRAV_LSOURCES		(PFFSTATSVAL_START + 12270)
#define PFFSTATSVAL_CULLTRAV_PARTITIONS		(PFFSTATSVAL_START + 12280)
#define PFFSTATSVAL_CULLTRAV_TEXTS     		(PFFSTATSVAL_START + 12290)

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

} // extern C 

typedef pfStatsValGfxPipeTimes pfFStatsValGfxPipeTimes;

#define PFFSTATSVAL_MEM				(PFFSTATSVAL_START + 6000)
#define PFFSTATSVAL_MEM_PROC			(PFFSTATSVAL_START + 6010)
#define PFFSTATSVAL_MEM_PROC_APP		(PFFSTATSVAL_START + 6020)
#define PFFSTATSVAL_MEM_PROC_CULL		(PFFSTATSVAL_START + 6030)
#define PFFSTATSVAL_MEM_PROC_DRAW		(PFFSTATSVAL_START + 6040)
#define PFFSTATSVAL_MEM_PROC_ISECT		(PFFSTATSVAL_START + 6050)
#define PFFSTATSVAL_MEM_PROC_LPOINT		(PFFSTATSVAL_START + 6060)
#define PFFSTATSVAL_MEM_PROC_COMPUTE		(PFFSTATSVAL_START + 6070)


// these are in pfProcess.C
extern pfNodeCounts *_pfCleanCountPtr;
extern pfNodeCounts *_pfUpdateCountPtr;
extern void _pfEnableCleanStats(int _en);
extern void _pfEnableUpdateStats(int _en);
extern void _pfEnableRTMonStats(int _en);
extern void _pfDrawChanStats(pfChannel *_chan, pfFrameStats *_stats);


#endif // !__PF_FRAMESTATS_H__
