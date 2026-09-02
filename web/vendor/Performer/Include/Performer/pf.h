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
 * pf.h		Include file for Performer rapid prototyping library.
 *
 * $Revision: 1.460 $
 * $Date: 2002/10/28 23:22:57 $
 *
 */

#ifndef __PF_H__
#define __PF_H__

/* 
 * When compiling with C++, the following options are available:
 *	
 *	Default			   -> use C++ API/types
 *	#define PF_C_API 1         -> augment C++ API with C API
 *	#define PF_CPLUSPLUS_API 0 -> do not use C++ API/types
 */

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


#include <Performer/pr.h>
#if PF_CPLUSPLUS_API
#include <Performer/pr/pfLinMath.h>  /* need math types */
#endif


#if PF_CPLUSPLUS_API

class pfBuffer;
//pfObjects
class pfPath;
class pfVolFog;
class pfRotorWash;
class pfStateMapList;
class pfShadow;
class pfDirData;
class pfIBRtexture;
class pfLoadBalance;
class pfCompositor;
class pfCteChanData;
class pfDispListOptimizer;
//pfUpdatables
class pfPipe;
class pfChannel;
class pfPipeVideoChannel;
class pfPipeWindow;
class pfEarthSky;
class pfFrameStats;
class pfLODState;
class pfMPClipTexture;
// pfNodes
class pfNode;
class pfLightSource;
class pfLightPoint;
class pfText;
// pfGroups
class pfGroup;
class pfScene;
class pfPartition;
class pfSCS;
class pfDCS;
class pfFCS;
class pfDoubleSCS;
class pfDoubleDCS;
class pfDoubleFCS;
class pfLOD;
class pfLayer;
class pfSwitch;
class pfMorph;
class pfSequence;
//pfGeodes
class pfGeode;
class pfASD;
class pfBillboard;
class pfIBRnode;
//pfTraversers
class pfTraverser;
//Cull Program
class pfCullProgram;
class pfNameStack;
#else

typedef struct _pfBuffer	pfBuffer;
/* pfObjects */
typedef struct _pfPath		pfPath;
typedef struct _pfVolFog 	pfVolFog;
typedef struct _pfRotorWash 	pfRotorWash;
typedef struct _pfStateMapList  pfStateMapList;
typedef struct _pfShadow        pfShadow;
typedef struct _pfDirData       pfDirData;
typedef struct _pfIBRtexture    pfIBRtexture;
typedef struct _pfLoadBalance   pfLoadBalance;
typedef struct _pfCompositor    pfCompositor;
typedef struct _pfCteChanData   pfCteChanData;
typedef struct _pfDispListOptimizer pfDispListOptimizer;
/* pfUpdatables */
typedef struct _pfPipe		pfPipe;
typedef struct _pfChannel	pfChannel;
typedef struct _pfPipeVideoChannel pfPipeVideoChannel;
typedef struct _pfPipeWindow	pfPipeWindow;
typedef struct _pfEarthSky	pfEarthSky;
typedef struct _pfFrameStats	pfFrameStats;
typedef struct _pfLODState	pfLODState;
typedef struct _pfMPClipTexture pfMPClipTexture;
/* pfNodes */
typedef struct _pfNode		pfNode;
typedef struct _pfLightSource	pfLightSource;
typedef struct _pfLightPoint	pfLightPoint;
typedef struct _pfText		pfText;
/* pfGroups */
typedef struct _pfGroup		pfGroup;
typedef struct _pfScene		pfScene;
typedef struct _pfPartition	pfPartition;
typedef struct _pfSCS		pfSCS;
typedef struct _pfDCS		pfDCS;
typedef struct _pfFCS		pfFCS;
typedef struct _pfDoubleSCS	pfDoubleSCS;
typedef struct _pfDoubleDCS	pfDoubleDCS;
typedef struct _pfDoubleFCS	pfDoubleFCS;
typedef struct _pfLOD		pfLOD;
typedef struct _pfLayer		pfLayer;
typedef struct _pfSwitch	pfSwitch;
typedef struct _pfMorph		pfMorph;
typedef struct _pfSequence	pfSequence;
/* pfGeodes */
typedef struct _pfGeode		pfGeode;
typedef struct _pfASD		pfASD;
typedef struct _pfBillboard	pfBillboard;
typedef struct _pfIBRnode	pfIBRnode;
/* pfTraversers */
typedef struct _pfTraverser	pfTraverser;
/* Cull Program */
typedef struct _pfCullProgram   pfCullProgram;
typedef struct _pfNameStack     pfNameStack;
#endif /* PF_CPLUSPLUS_API */

/* ----------------------- pfCullProgram Types ---------------- */

typedef int (*_pfCullPgInstruction)(pfCullProgram *, int);

/* ----------------------- pfShadow Types --------------------- */

typedef float (*_pfBlendFunc)(float);

/* ----------------------- pfNode Types ----------------------- */

typedef int (*pfNodeTravFuncType)(pfTraverser *_trav, void *_userData);

/* ----------------------- pfASD Types ------------------------ */
typedef struct _pfASDFace
{
    int level;
    int tsid;
    int vert[3];
    int attr[3];
    int refvert[3];
    int sattr[3];
    int child[4];
    ushort gstateid;
    ushort mask;
} pfASDFace;

typedef struct _pfASDLODRange
{
    float switchin;
    float morph;
} pfASDLODRange;

typedef struct _pfASDVert
{
    pfVec3 v0, vd;
    int neighborid[2];
    int vertid;
} pfASDVert;

typedef struct _pfASDTileNum
{
    short s[3][2]; /* the verts are taken care of by parents */
    short c[2];
} pfASDTileNum;

typedef struct _pfTerrainRegisteredVertex pfASDRegisteredVertex;

typedef struct _pfTerrainRegisteredTriangle pfASDRegisteredTriangle;

typedef float (*pfASDEvalFuncType)(pfASD *_asd, pfVec3 _eyept, int _faceid, int _refvertid);
typedef void (*pfASDCalcVirtualClipTexParamsFuncType)(
        int nLevels,        /* total number of virtual levels */
        int clipSize,
        int invalidBorder,
        float minLODTexPix, /* lower bound on the resolution the hardware
                               will access, based on calculation of
                               tex/pix partial derivatives (don't
                               forget to add in LODbiasS,LODbiasT!)
                               or a height-above-terrain lookup table */
        float minLODLoaded, /* float for fade-in */
        float maxLODLoaded, /* may use someday if we get a way to reuse LODs*/
        float bboxMinDist,  /* min distance from clip center in texture coords*/
        float bboxMaxDist,  /* max distance from clip center in texture coords*/
        float tradeoff, /* 0. means go fine (hi resolution at expense of area)
                           1. means go coarse (area at expense of resolution) */
        const struct pfVirtualClipTexLimits *limits,
        int *return_LODOffset,
        int *return_numEffectiveLevels,
        float *return_minLOD,
        float *return_maxLOD);

/* ----------------------- pfLOD Types ----------------------- */

typedef float (*pfLODEvalFuncType)(pfLOD *lod, pfChannel *chan, pfMatrix *offset);

/* --------------------pfChannel Types ----------------------- */

typedef void (*pfChanFuncType)(pfChannel* _chan, void* _userData);

/* ----------------------- pfPipe Types ----------------------- */

typedef void (*pfPipeFuncType)(pfPipe* _pipe);
typedef void (*pfPipeSwapFuncType)(pfPipe* _pipe, pfPipeWindow *_pwin);

/* --------------------pfPipeWindow Types ----------------------- */

typedef void (*pfPWinFuncType)(pfPipeWindow *_pw);

#include <Performer/pfstats.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PFTRAV_STACK_MAX        256
#define PFTRAV_DEPTH_MAX        256

/* pfChanTravMode() pfNodeTravMask/Func() */
#define PFTRAV_ISECT		0 
#define PFTRAV_APP		1 /* travFunc, travMode, travMask */
#define PFTRAV_CULL		2 /* travFunc, travMode, travMask */
#define PFTRAV_DRAW		3 /* travFunc, travMode, travMask */
#define PFTRAV_LPOINT		4 /* travFunc, travMode, travMask */
#define PFTRAV_COMPUTE		5
#define PFTRAV_POSTLPOINT       6 /* travFunc */
#define PFTRAV_MULTIPASS        7 /* travMode */

/*-------------------------- Initialization --------------------------*/

typedef void (*pfStageFuncType)(int _pipe, uint _stage);
typedef void (*pfCreateProcessFuncType)(int _pipe, uint _stage, pid_t _pid);

/* pfMultiprocess() */
#define PFMP_FORK_ISECT		0x01
#define PFMP_FORK_CULL		0x02
#define PFMP_FORK_DRAW		0x04
#define PFMP_FORK_DBASE		0x08
#define PFMP_FORK_LPOINT       	0x10  
#define PFMP_FORK_COMPUTE      	0x20  
#define PFMP_CULLoDRAW          0x10000  /* 65536 */
#define PFMP_CULL_DL_DRAW       0x20000  /* 131072 */
#define PFMP_FORK_CULL_SIDEKICK 0x080  

#define PFMP_DEFAULT            -1
#define PFMP_APPCULLDRAW        0
#define PFMP_APPCULL_DL_DRAW    (PFMP_CULL_DL_DRAW)
#define PFMP_APPCULL_DRAW       (PFMP_FORK_DRAW)
#define PFMP_APP_CULLDRAW       (PFMP_FORK_CULL)
#define PFMP_APP_CULL_DL_DRAW   (PFMP_FORK_CULL | PFMP_CULL_DL_DRAW) /* 131074 */
#define PFMP_APP_CULL_DRAW      (PFMP_FORK_CULL | PFMP_FORK_DRAW)  /* 6 */
#define PFMP_APPCULLoDRAW       (PFMP_FORK_DRAW | PFMP_CULLoDRAW) /* 65540 */
#define PFMP_APP_CULLoDRAW      (PFMP_FORK_CULL | PFMP_FORK_DRAW | PFMP_CULLoDRAW) /* 65542 */

/* pfGetPID(), pfGetStage() */
#define PFPROC_APP		0x0001
#define PFPROC_CULL		0x0002
#define PFPROC_DRAW		0x0004
#define PFPROC_ISECT		0x0008
#define PFPROC_DBASE        	0x0010
#define PFPROC_CLOCK            0x0020
#define PFPROC_LPOINT          	0x0080
#define PFPROC_COMPUTE         	0x0100
#define PFPROC_QUEUE_SORT	0x0200
#define PFPROC_QUEUE_SERVICE	0x0400
#define PFPROC_CULL_SIDEKICK	0x0800
#define PFPROC_MASK		0x0fff

/* THREAD tokens are OR'ed into stage mask */
#define PFPROC_THREAD1		0x1000
#define PFPROC_THREAD2		0x2000
#define PFPROC_THREAD3		0x3000
#define PFPROC_THREAD4		0x4000
#define PFPROC_THREAD5		0x5000
#define PFPROC_THREAD6		0x6000
#define PFPROC_THREAD7		0x7000
#define PFPROC_MAX_THREADS	8
#define PFPROC_THREAD_SHIFT	12
#define PFPROC_THREAD_MASK	0xf000


/* Multithread parameters (currently, only CULL_SIDEKICK has parameters) */
#define PFSK_POLICY		1
#define	PFSK_OPTIMIZATION	2
#define PFSK_SAFETY_MARGIN	3
#define PFSK_USER_FUNC		4
#define PFSK_USER_FUNC_DATA	5

/* Synchronization policy (PFSK_POLICY) tokens for pfMultithreadParami */
#define PFSK_CULL_DONE		1
#define	PFSK_CULL_FRAME_DONE	2
#define PFSK_SIDEKICK_DONE	3

/* Parameters for pfSKGetNumPrimsRemoved */
#define PFSK_TRIS		1
#define PFSK_LINES		2
#define PFSK_POINTS		3
#define PFSK_VERTS		4

typedef pfGeoSet *(*pfSidekickFunc)(pfGeoSet *gset, pfDispListOptimizer *op, void *userData);

#ifdef pfInit
#undef pfInit
#undef pfIsInited
#undef pfExit
#endif

extern DLLEXPORT int	pfInit(void);
extern DLLEXPORT int	pfIsInited(void);
extern DLLEXPORT void	pfExit(void);
extern DLLEXPORT int	pfMultipipe(int _numPipes);
extern DLLEXPORT int	pfGetMultipipe(void);
extern DLLEXPORT int	pfHyperpipe(int _numHyperPipes);
extern DLLEXPORT int	pfGetHyperpipe(pfPipe *_pipe);
extern DLLEXPORT int	pfHyperpipe2D(pfCompositor *comp);
extern DLLEXPORT int	pfMultiprocess(int _mpMode);
extern DLLEXPORT int	pfGetMultiprocess(void);
extern DLLEXPORT int	pfGetMPBitmask(void);
extern DLLEXPORT int	pfMultithread(int _pipe, uint _stage, int _nprocs);
extern DLLEXPORT int    pfGetMultithread(int _pipe, uint _stage);

extern DLLEXPORT int	pfMultithreadParami(int _pipe, uint param, unsigned int value);
extern DLLEXPORT unsigned int	pfGetMultithreadParami(int _pipe, uint param);
extern DLLEXPORT int	pfMultithreadParamf(int _pipe, uint param, float value);
extern DLLEXPORT float	pfGetMultithreadParamf(int _pipe, uint param);

extern DLLEXPORT int	pfMultithreadParam(int _pipe, uint param, void *value);
extern DLLEXPORT void 	*pfGetMultithreadParam(int _pipe, uint param);

extern DLLEXPORT int    pfSKGetNumPrimsRemoved(int pipe, int primType);

extern DLLEXPORT void     pfExerciseArena(uint _processMask, uint _ntimes);

extern DLLEXPORT int	pfConfig(void);
extern DLLEXPORT int	pfIsConfiged(void);
extern DLLEXPORT pid_t	pfGetPID(int _pipe, uint _stage);
extern DLLEXPORT uint	pfGetStage(pid_t _pid, int *_pipe);
extern DLLEXPORT void	pfStageConfigFunc(int _pipe, uint _stage, pfStageFuncType _configFunc);
extern DLLEXPORT pfStageFuncType pfGetStageConfigFunc(int _pipe, uint _stage);
extern DLLEXPORT int 	pfConfigStage(int _pipe, uint _stage);
extern DLLEXPORT const char * pfGetStageName(int pipe, uint _stage);
extern DLLEXPORT const char * pfGetPIDName(pid_t pid);


extern DLLEXPORT void	pfCreateProcessFunc(pfCreateProcessFuncType func);
extern DLLEXPORT pfCreateProcessFuncType pfGetCreateProcessFunc(void);
extern DLLEXPORT void	pfProcessMiscCPU(int cpu);
extern DLLEXPORT int	pfGetProcessMiscCPU(void);
extern DLLEXPORT void	pfPrintProcessState(FILE *fp);

/*---------------------------- Frame Control ---------------------------*/

typedef void (*pfIsectFuncType)(void *_data);
typedef void (*pfDBaseFuncType)(void *_data);
typedef void (*pfSyncFuncType)(void);
typedef void (*pfComputeFuncType)(void *_data);

/* pfPhase() */
#define PFPHASE_FLOAT		0
#define PFPHASE_LOCK		1
#define PFPHASE_FREE_RUN	2
#define PFPHASE_LIMIT		3
#define PFPHASE_MODE_MASK	0xf
#define PFPHASE_SPIN_DRAW	0x10000

extern DLLEXPORT void	pfAppFrame(void);
extern DLLEXPORT int	pfSync(void);
extern DLLEXPORT int	pfFrame(void);
extern DLLEXPORT void	pfApp(void);
extern DLLEXPORT void	pfCull(void);
extern DLLEXPORT void	pfDraw(void);
extern DLLEXPORT void     pfDrawBin(int bin);
extern DLLEXPORT void     pfDrawScene(void);
extern DLLEXPORT void	pfLPoint(void);
extern DLLEXPORT void	pfCompute(void);


extern DLLEXPORT void	pfIsectFunc(pfIsectFuncType _func);
extern DLLEXPORT pfIsectFuncType pfGetIsectFunc(void);
extern DLLEXPORT void*	pfAllocIsectData(int _bytes);
extern DLLEXPORT void*	pfGetIsectData(void);
extern DLLEXPORT void	pfPassIsectData(void);

extern DLLEXPORT void	pfDBase(void);
extern DLLEXPORT void	pfDBaseFunc(pfDBaseFuncType _func);
extern DLLEXPORT pfDBaseFuncType pfGetDBaseFunc(void);
extern DLLEXPORT void*	pfAllocDBaseData(int _bytes);
extern DLLEXPORT void*	pfGetDBaseData(void);
extern DLLEXPORT void	pfPassDBaseData(void);

extern DLLEXPORT void	pfComputeFunc(pfComputeFuncType _func);
extern DLLEXPORT pfComputeFuncType pfGetComputeFunc(void);
extern DLLEXPORT void*	pfAllocComputeData(int _bytes);
extern DLLEXPORT void*	pfGetComputeData(void);
extern DLLEXPORT void	pfPassComputeData(void);

extern DLLEXPORT int 	pfGetLastPipeFrameCount(int _pipe);
extern DLLEXPORT int 	pfGetPipeDrawCount(int _pipe);
extern DLLEXPORT int 	pfGetPipeNum(void);

extern DLLEXPORT void	pfPhase(int _phase);
extern DLLEXPORT int	pfGetPhase(void);
extern DLLEXPORT void	pfVideoRate(float _vrate);
extern DLLEXPORT float	pfGetVideoRate(void);
extern DLLEXPORT float	pfFrameRate(float _rate);
extern DLLEXPORT float	pfGetFrameRate(void);
extern DLLEXPORT int	pfFieldRate(int _fields);
extern DLLEXPORT int	pfGetFieldRate(void);
extern DLLEXPORT int	pfGetFrameCount(void);

extern DLLEXPORT double	pfGetFrameTimeStamp(void);
extern DLLEXPORT void	pfFrameTimeStamp(double t);

extern DLLEXPORT pfFlux*	pfGetFrameTimeFlux(void);

extern DLLEXPORT int	pfGetId(void *_mem);
extern DLLEXPORT int	pfAsyncDelete(void *_mem);
extern DLLEXPORT int	pfCopy(void *_dst, void *_src);

extern DLLEXPORT void	pfProcessPriorityUpgrade (int state);
extern DLLEXPORT int	pfGetProcessPriorityUpgrade (void);
extern DLLEXPORT void	pfProcessHighestPriority (int pri);
extern DLLEXPORT int	pfGetProcessHighestPriority (void);

/*-------------------------- Event Instrumentation ------------------------*/

extern DLLEXPORT void pfInitializeEvents(void);
extern DLLEXPORT void pfResetEvents(void);
extern DLLEXPORT void pfEventSampleOn(void);
extern DLLEXPORT void pfEventSampleOff(void);
extern DLLEXPORT void pfWriteEvents(char *filename);

#if PF_C_API

/*
 * ----------------------------- C API --------------------------------
 */

/****************************** Non-nodes ***********************************/



#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/*----------------- pfPipe Functions ------------------*/

extern DLLEXPORT pfPipe*	pfGetPipe(int _pipeNum);
extern DLLEXPORT int	pfInitPipe(pfPipe *_pipe, pfPipeFuncType _configFunc);

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfPipe CAPI ------------------------------*/

extern DLLEXPORT pfType*              pfGetPipeClassType(void);
extern DLLEXPORT void                 pfPipeSwapFunc(pfPipe* _pipe, pfPipeSwapFuncType func);
extern DLLEXPORT pfPipeSwapFuncType   pfGetPipeSwapFunc(const pfPipe* _pipe);
extern DLLEXPORT float                pfGetPipeLoad(const pfPipe* _pipe);
extern DLLEXPORT void                 pfGetPipeSize(const pfPipe* _pipe, int  *xs, int  *ys);
extern DLLEXPORT void                 pfPipeScreen(pfPipe* _pipe, int scr);
extern DLLEXPORT int                  pfGetPipeScreen(const pfPipe* _pipe);
extern DLLEXPORT void                 pfPipeServer(pfPipe* _pipe, int dis);
extern DLLEXPORT int                  pfGetPipeServer(const pfPipe* _pipe);
extern DLLEXPORT void                 pfPipeWSConnectionName(pfPipe* _pipe, const char *name);
extern DLLEXPORT const char*          pfGetPipeWSConnectionName(const pfPipe* _pipe);
extern DLLEXPORT pfMPClipTexture*     pfGetPipeMPClipTexture(const pfPipe* _pipe, int  i);
extern DLLEXPORT int                  pfGetPipeNumMPClipTextures(const pfPipe* _pipe);
extern DLLEXPORT pfChannel*           pfGetPipeChan(const pfPipe* _pipe, int  i);
extern DLLEXPORT int                  pfGetPipeNumChans(const pfPipe* _pipe);
extern DLLEXPORT pfPipeWindow*        pfGetPipePWin(const pfPipe* _pipe, int  i);
extern DLLEXPORT int                  pfGetPipeNumPWins(const pfPipe* _pipe);
extern DLLEXPORT void                 pfPipeIncrementalStateChanNum(pfPipe* _pipe, int _num);
extern DLLEXPORT int                  pfGetPipeIncrementalStateChanNum(const pfPipe* _pipe);
extern DLLEXPORT int                  pfGetPipeHyperId(const pfPipe* _pipe);
extern DLLEXPORT void                 pfBindPipeVChans(pfPipe* _pipe);
extern DLLEXPORT void                 pfUnbindPipePVChans(pfPipe* _pipe);
extern DLLEXPORT int                  pfMovePWin(pfPipe* _pipe, int  where, pfPipeWindow *_pw);
extern DLLEXPORT void                 pfAddMPClipTexture(pfPipe* _pipe, pfMPClipTexture *clip);
extern DLLEXPORT int                  pfRemoveMPClipTexturePipe(pfPipe* _pipe, pfMPClipTexture *clip);
extern DLLEXPORT void                 pfPipeTotalTexLoadTime(pfPipe* _pipe, float _totalTexLoadTime);
extern DLLEXPORT float                pfGetPipeTotalTexLoadTime(const pfPipe* _pipe);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfBuffer CAPI ------------------------------*/

extern DLLEXPORT pfBuffer*            pfNewBuffer(void);
extern DLLEXPORT void                 pfBufferScope(pfBuffer* _buffer, pfObject *obj, int scope);
extern DLLEXPORT int                  pfGetBufferScope(pfBuffer* _buffer, pfObject *obj);
extern DLLEXPORT void                 pfMergeBuffer(void);
extern DLLEXPORT int                  pfUnrefDelete(void *mem);
extern DLLEXPORT int                  pfDelete(void *mem);
extern DLLEXPORT int                  pfBufferInsert(void *parent, int index, void *child);
extern DLLEXPORT int                  pfBufferRemove(void *parent, void *child);
extern DLLEXPORT int                  pfBufferAdd(void *parent, void *child);
extern DLLEXPORT int                  pfBufferReplace(void *parent, void *oldChild, void *newChild);
extern DLLEXPORT int                  pfBufferSet(void *parent, int index, void *child);
extern DLLEXPORT void                 pfSelectBuffer(pfBuffer* _buffer);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ------------------ pfBuffer Related Functions--------------------- */

extern DLLEXPORT pfBuffer*	pfGetCurBuffer(void);
#endif /* !PF_CPLUSPLUS_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ------------------- pfPipeWindow Tokens ----------------------- */
/* pfWinType */

#define PFPWIN_TYPE_X	    	PFWIN_TYPE_X
#define PFPWIN_TYPE_STATS   	PFWIN_TYPE_STATS
#define PFPWIN_TYPE_UNMANAGED  	PFWIN_TYPE_UNMANAGED
#define PFPWIN_TYPE_PBUFFER  	PFWIN_TYPE_PBUFFER
#define PFPWIN_TYPE_SHARE   	0x10000
#define PFPWIN_TYPE_NOXEVENTS   0x20000
#define PFPWIN_TYPE_MASK    (PFWIN_TYPE_MASK | PFPWIN_TYPE_SHARE | PFPWIN_TYPE_NOXEVENTS)

/* ----------------pfPipeWindow Related Routines------------------ */
extern DLLEXPORT void		pfInitGfx(void);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfPipeWindow CAPI ------------------------------*/

extern DLLEXPORT pfPipeWindow*        pfNewPWin(pfPipe *p);
extern DLLEXPORT pfType*              pfGetPWinClassType(void);
extern DLLEXPORT void                 pfPWinPrintSwapGroup(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinName(pfPipeWindow* _pwin, const char *name);
extern DLLEXPORT const char*          pfGetPWinName(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinWSConnectionName(pfPipeWindow* _pwin, const char *name);
extern DLLEXPORT const char*          pfGetPWinWSConnectionName(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinMode(pfPipeWindow* _pwin, int mode, int val);
extern DLLEXPORT int                  pfGetPWinMode(pfPipeWindow* _pwin, int _mode);
extern DLLEXPORT void                 pfPWinType(pfPipeWindow* _pwin, uint type);
extern DLLEXPORT uint                 pfGetPWinType(const pfPipeWindow* _pwin);
extern DLLEXPORT pfState*             pfGetPWinCurState(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinAspect(pfPipeWindow* _pwin, int x, int y);
extern DLLEXPORT void                 pfGetPWinAspect(pfPipeWindow* _pwin, int *x, int *y);
extern DLLEXPORT void                 pfPWinOriginSize(pfPipeWindow* _pwin, int xo, int yo, int xs, int ys);
extern DLLEXPORT void                 pfPWinOrigin(pfPipeWindow* _pwin, int xo, int yo);
extern DLLEXPORT void                 pfGetPWinOrigin(pfPipeWindow* _pwin, int *xo, int *yo);
extern DLLEXPORT void                 pfGetPWinScreenOrigin(pfPipeWindow* _pwin, int *xo, int *yo);
extern DLLEXPORT void                 pfPWinSize(pfPipeWindow* _pwin, int xs, int ys);
extern DLLEXPORT void                 pfGetPWinSize(pfPipeWindow* _pwin, int *xs, int *ys);
extern DLLEXPORT void                 pfPWinFullScreen(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfGetPWinCurOriginSize(pfPipeWindow* _pwin, int *xo, int *yo, int *xs, int *ys);
extern DLLEXPORT void                 pfGetPWinCurScreenOriginSize(pfPipeWindow* _pwin, int *xo, int *yo, int *xs, int *ys);
extern DLLEXPORT void                 pfPWinOverlayWin(pfPipeWindow* _pwin, pfWindow *ow);
extern DLLEXPORT pfWindow*            pfGetPWinOverlayWin(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinStatsWin(pfPipeWindow* _pwin, pfWindow *sw);
extern DLLEXPORT pfWindow*            pfGetPWinStatsWin(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinScreen(pfPipeWindow* _pwin, int screen);
extern DLLEXPORT int                  pfGetPWinScreen(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinShare(pfPipeWindow* _pwin, int mode);
extern DLLEXPORT uint                 pfGetPWinShare(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinWSWindow(pfPipeWindow* _pwin, pfWSConnection dsp, pfWSWindow wsw);
extern DLLEXPORT Window               pfGetPWinWSWindow(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinWSDrawable(pfPipeWindow* _pwin, pfWSConnection dsp, pfWSDrawable gxw);
extern DLLEXPORT pfWSDrawable         pfGetPWinWSDrawable(pfPipeWindow* _pwin);
extern DLLEXPORT pfWSDrawable         pfGetPWinCurWSDrawable(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinFBConfigData(pfPipeWindow* _pwin, void *_data);
extern DLLEXPORT void*                pfGetPWinFBConfigData(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinFBConfigAttrs(pfPipeWindow* _pwin, int *attr);
extern DLLEXPORT int*                 pfGetPWinFBConfigAttrs(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinFBConfigId(pfPipeWindow* _pwin, int vId);
extern DLLEXPORT int                  pfGetPWinFBConfigId(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinIndex(pfPipeWindow* _pwin, int index);
extern DLLEXPORT int                  pfGetPWinIndex(pfPipeWindow* _pwin);
extern DLLEXPORT pfWindow*            pfGetPWinSelect(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinGLCxt(pfPipeWindow* _pwin, pfGLContext gc);
extern DLLEXPORT pfGLContext          pfGetPWinGLCxt(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinSwapBarrier(pfPipeWindow* _pwin, int _barrierName);
extern DLLEXPORT int                  pfGetPWinSwapBarrier(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinList(pfPipeWindow* _pwin, pfList *_wl);
extern DLLEXPORT pfList*              pfGetPWinList(const pfPipeWindow* _pwin);
extern DLLEXPORT int                  pfAttachPWinWin(pfPipeWindow* _pwin, pfWindow *w1);
extern DLLEXPORT int                  pfDetachPWinWin(pfPipeWindow* _pwin, pfWindow *w1);
extern DLLEXPORT int                  pfAttachPWin(pfPipeWindow* _pwin, pfPipeWindow *pw1);
extern DLLEXPORT int                  pfDetachPWin(pfPipeWindow* _pwin, pfPipeWindow *pw1);
extern DLLEXPORT void                 pfAttachPWinSwapGroup(pfPipeWindow* _pwin, pfPipeWindow *_w1);
extern DLLEXPORT void                 pfAttachPWinWinSwapGroup(pfPipeWindow* _pwin, pfWindow *_w1);
extern DLLEXPORT void                 pfDetachPWinSwapGroup(pfPipeWindow* _pwin);
extern DLLEXPORT pfWindow*            pfSelectPWin(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfSwapPWinBuffers(pfPipeWindow* _pwin);
extern DLLEXPORT pfFBConfig           pfChoosePWinFBConfig(pfPipeWindow* _pwin, int *attr);
extern DLLEXPORT int                  pfIsPWinOpen(pfPipeWindow* _pwin);
extern DLLEXPORT int                  pfIsManagedPWin(pfPipeWindow* _pwin);
extern DLLEXPORT int                  pfPWinInSwapGroup(pfPipeWindow* _pwin);
extern DLLEXPORT int                  pfQueryPWin(pfPipeWindow* _pwin, int _which, int *_dst);
extern DLLEXPORT int                  pfMQueryPWin(pfPipeWindow* _pwin, int *_which, int *_dst);
extern DLLEXPORT pfPipe*              pfGetPWinPipe(pfPipeWindow* _pwin);
extern DLLEXPORT int                  pfGetPWinPipeIndex(const pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfPWinConfigFunc(pfPipeWindow* _pwin, pfPWinFuncType _func);
extern DLLEXPORT pfPWinFuncType       pfGetPWinConfigFunc(pfPipeWindow* _pwin);
extern DLLEXPORT int                  pfGetPWinChanIndex(pfPipeWindow* _pwin, pfChannel *_chan);
extern DLLEXPORT void                 pfConfigPWin(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfOpenPWin(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfClosePWin(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfClosePWinGL(pfPipeWindow* _pwin);
extern DLLEXPORT int                  pfRemoveChan(pfPipeWindow* _pwin, pfChannel *_chan);
extern DLLEXPORT int                  pfAddChan(pfPipeWindow* _pwin, pfChannel *_chan);
extern DLLEXPORT void                 pfInsertChan(pfPipeWindow* _pwin, int _where, pfChannel *_chan);
extern DLLEXPORT int                  pfMoveChan(pfPipeWindow* _pwin, int _where, pfChannel *_chan);
extern DLLEXPORT pfChannel*           pfGetChan(pfPipeWindow* _pwin, int which);
extern DLLEXPORT int                  pfGetNumChans(const pfPipeWindow* _pwin);
extern DLLEXPORT int                  pfPWinAddPVChan(pfPipeWindow* _pwin, pfPipeVideoChannel *_vchan);
extern DLLEXPORT void                 pfPWinPVChan(pfPipeWindow* _pwin, int _num, pfPipeVideoChannel *_vchan);
extern DLLEXPORT void                 pfPWinRemovePVChan(pfPipeWindow* _pwin, pfPipeVideoChannel *_vchan);
extern DLLEXPORT void                 pfPWinRemovePVChanIndex(pfPipeWindow* _pwin, int _num);
extern DLLEXPORT pfPipeVideoChannel*  pfGetPWinPVChan(pfPipeWindow* _pwin, int _num);
extern DLLEXPORT pfPipeVideoChannel*  pfGetPWinPVChanId(pfPipeWindow* _pwin, int _num);
extern DLLEXPORT pfPipeVideoChannel*  pfGetPWinPosPVChan(pfPipeWindow* _pwin, int _x, int _y);
extern DLLEXPORT int                  pfGetPWinPVChanIndex(pfPipeWindow* _pwin, pfPipeVideoChannel *_vchan);
extern DLLEXPORT int                  pfGetPWinNumPVChans(const pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfBindPWinPVChans(pfPipeWindow* _pwin);
extern DLLEXPORT void                 pfUnbindPWinPVChans(pfPipeWindow* _pwin);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfVideoChannel Tokens ----------------------- */

/* pfPVChanDVRMode */
#define PFPVC_DVR_OFF		0x0
#define PFPVC_DVR_MANUAL	0x1
#define PFPVC_DVR_AUTO		0x2
#define PFPVC_DVR_MASK		0x3
#define PFPVC_DVR_NOVR		0x4

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfPipeVideoChannel CAPI ------------------------------*/

extern DLLEXPORT pfPipeVideoChannel*  pfNewPVChan(pfPipe *p);
extern DLLEXPORT pfType*              pfGetPVChanClassType(void);
extern DLLEXPORT void                 pfPVChanMode(pfPipeVideoChannel* _pvchan, int _mode, int _val);
extern DLLEXPORT int                  pfGetPVChanMode(pfPipeVideoChannel* _pvchan, int _mode);
extern DLLEXPORT void                 pfPVChanWSWindow(pfPipeVideoChannel* _pvchan, pfWSWindow _wsWin);
extern DLLEXPORT pfWSWindow           pfGetPVChanWSWindow(const pfPipeVideoChannel* _pvchan);
extern DLLEXPORT void                 pfPVChanCallig(pfPipeVideoChannel* _pvchan, pfCalligraphic *_ca);
extern DLLEXPORT pfCalligraphic*      pfGetPVChanCallig(const pfPipeVideoChannel* _pvchan);
extern DLLEXPORT void                 pfGetPVChanOrigin(pfPipeVideoChannel* _pvchan, int *_xo, int *_yo);
extern DLLEXPORT void                 pfGetPVChanSize(pfPipeVideoChannel* _pvchan, int *_xs, int *_ys);
extern DLLEXPORT void                 pfGetPVChanMinDeltas(const pfPipeVideoChannel* _pvchan, int *_dx, int *_dy);
extern DLLEXPORT void                 pfGetPVChanScreenOutputOrigin(pfPipeVideoChannel* _pvchan, int *_xo, int *_yo);
extern DLLEXPORT void                 pfPVChanOutputOrigin(pfPipeVideoChannel* _pvchan, int _xo, int _yo);
extern DLLEXPORT void                 pfGetPVChanOutputOrigin(pfPipeVideoChannel* _pvchan, int *_xo, int *_yo);
extern DLLEXPORT void                 pfPVChanOutputSize(pfPipeVideoChannel* _pvchan, int _xs, int _ys);
extern DLLEXPORT void                 pfGetPVChanOutputSize(pfPipeVideoChannel* _pvchan, int *_xs, int *_ys);
extern DLLEXPORT void                 pfPVChanAreaScale(pfPipeVideoChannel* _pvchan, float _s);
extern DLLEXPORT float                pfGetPVChanAreaScale(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT void                 pfPVChanMinScale(pfPipeVideoChannel* _pvchan, float _xs, float _ys);
extern DLLEXPORT void                 pfGetPVChanMinScale(const pfPipeVideoChannel* _pvchan, float *_xs, float *_ys);
extern DLLEXPORT void                 pfPVChanMaxScale(pfPipeVideoChannel* _pvchan, float _xs, float _ys);
extern DLLEXPORT void                 pfGetPVChanMaxScale(const pfPipeVideoChannel* _pvchan, float *_xs, float *_ys);
extern DLLEXPORT void                 pfPVChanScale(pfPipeVideoChannel* _pvchan, float _xs, float _ys);
extern DLLEXPORT void                 pfGetPVChanScale(const pfPipeVideoChannel* _pvchan, float *_xs, float *_ys);
extern DLLEXPORT void                 pfPVChanFullRect(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT int                  pfGetPVChanScreen(const pfPipeVideoChannel* _pvchan);
extern DLLEXPORT void                 pfPVChanId(pfPipeVideoChannel* _pvchan, int _index);
extern DLLEXPORT int                  pfGetPVChanId(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT pfWSVideoChannelInfo pfGetPVChanInfo(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT void                 pfPVChanDVRMode(pfPipeVideoChannel* _pvchan, int _mode);
extern DLLEXPORT int                  pfGetPVChanDVRMode(const pfPipeVideoChannel* _pvchan);
extern DLLEXPORT void                 pfPVChanStressFilter(pfPipeVideoChannel* _pvchan, float frac, float low, float high, float ps, float s, float max);
extern DLLEXPORT void                 pfGetPVChanStressFilter(const pfPipeVideoChannel* _pvchan, float *frac, float *low, float *high, float *ps, float *s, float *max);
extern DLLEXPORT void                 pfPVChanStress(pfPipeVideoChannel* _pvchan, float stress);
extern DLLEXPORT float                pfGetPVChanStress(const pfPipeVideoChannel* _pvchan);
extern DLLEXPORT float                pfGetPVChanLoad(const pfPipeVideoChannel* _pvchan);
extern DLLEXPORT void                 pfPVChanMinIncScale(pfPipeVideoChannel* _pvchan, float _xs, float _ys);
extern DLLEXPORT void                 pfGetPVChanMinIncScale(const pfPipeVideoChannel* _pvchan, float *_xs, float *_ys);
extern DLLEXPORT void                 pfPVChanMaxIncScale(pfPipeVideoChannel* _pvchan, float _xs, float _ys);
extern DLLEXPORT void                 pfGetPVChanMaxIncScale(const pfPipeVideoChannel* _pvchan, float *_xs, float *_ys);
extern DLLEXPORT void                 pfPVChanMinDecScale(pfPipeVideoChannel* _pvchan, float _xs, float _ys);
extern DLLEXPORT void                 pfGetPVChanMinDecScale(const pfPipeVideoChannel* _pvchan, float *_xs, float *_ys);
extern DLLEXPORT void                 pfPVChanMaxDecScale(pfPipeVideoChannel* _pvchan, float _xs, float _ys);
extern DLLEXPORT void                 pfGetPVChanMaxDecScale(const pfPipeVideoChannel* _pvchan, float *_xs, float *_ys);
extern DLLEXPORT void                 pfSelectPVChan(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT void                 pfApplyPVChan(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT void                 pfBindPVChan(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT void                 pfUnbindPVChan(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT int                  pfIsPVChanBound(const pfPipeVideoChannel* _pvchan);
extern DLLEXPORT int                  pfIsPVChanActive(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT pfPipe*              pfGetPVChanPipe(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT pfPipeWindow*        pfGetPVChanPWin(pfPipeVideoChannel* _pvchan);
extern DLLEXPORT int                  pfGetPVChanPWinIndex(pfPipeVideoChannel* _pvchan);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfChannel Tokens ----------------------- */


#define PFMPASS_GLOBAL_AMBIENT	      0x1
#define PFMPASS_EMISSIVE_PASS	      0x2
#define PFMPASS_NONTEX_SCENE	      0x4
#define PFMPASS_NO_TEXTURE_ALPHA      0x8
#define PFMPASS_FORCE_MULTISAMPLE     0x10
#define PFMPASS_BLACK_PASS            0x20

/* pfChanTravMode() (Uses PFTRAV_ tokens) */
#define PFCULL_VIEW		0x1
#define PFCULL_SORT		0x2
#define PFCULL_GSET		0x4
#define PFCULL_IGNORE_LSOURCES	0x8
#define PFCULL_PROGRAM      	0x10
#define PFCULL_ALL		(PFCULL_VIEW|PFCULL_SORT|PFCULL_GSET)

/* pfChanShare() */
#define PFCHAN_FOV		    0x1
#define PFCHAN_VIEW		    0x2
#define PFCHAN_NEARFAR		    0x4
#define PFCHAN_SCENE		    0x10
#define PFCHAN_STRESS	    	    0x20
#define PFCHAN_STRESSFILTER         PFCHAN_STRESS
#define PFCHAN_LOD		    0x40
#define PFCHAN_EARTHSKY		    0x80
#define PFCHAN_SWAPBUFFERS	    0x100
#define PFCHAN_VIEW_OFFSETS	    0x200
#define PFCHAN_STATS_DRAWMODE	    0x400
#define PFCHAN_APPFUNC		    0x1000
#define PFCHAN_CULLFUNC		    0x2000
#define PFCHAN_DRAWFUNC		    0x4000
#define PFCHAN_VIEWPORT		    0x8000
#define PFCHAN_SWAPBUFFERS_HW	    0x10000
#define PFCHAN_CULL_VOLUME	    0x20000
#define PFCHAN_GSTATE		    0x40000
#define PFCHAN_PRICLASS_LIST	    0x80000
#define PFCHAN_LPOINTFUNC	    0x100000
#define PFCHAN_POSTLPOINTFUNC       0x200000
    
#define PFDRAW_OFF		0
#define PFDRAW_ON		1
   
/* pfChanLODAttr() */
#define PFLOD_SCALE		2
#define PFLOD_FADE		3
#define PFLOD_STRESS_PIX_LIMIT	4
#define PFLOD_FRUST_SCALE	5

/* pfChanBinOrder,Sort() */
#define PFSORT_MAX_KEYS		64

/* The default bin */
#define PFSORT_DEFAULT_BIN      -1

/* Shaded stuff is drawn dead last, so it uses the last bin */
#define PFSORT_SHADER_BIN       3  //0x7fff
#define PFSORT_SHADER_BIN_ORDER 9998

/* patchy fog bin */
#define PFSORT_PATCHYFOG_BIN       4
#define PFSORT_PATCHYFOG_BIN_ORDER 2

/* opaque polygons are first */
#define PFSORT_OPAQUE_BIN	0
#define PFSORT_OPAQUE_BIN_ORDER 0
/* then transparent polygons */
#define PFSORT_TRANSP_BIN	1		
#define PFSORT_TRANSP_BIN_ORDER 1
/* Calligraphics must be rendered last */
#define PFSORT_LPSTATE_BIN 2
#define PFSORT_LPSTATE_BIN_ORDER 9999

#define PFSORT_NO_ORDER		-1

/* bin flags */
#define PFBIN_NONEXCLUSIVE_BIN       (1<<0)
#define PFBIN_DONT_DRAW_BY_DEFAULT   (1<<1)
#define PFBIN_DONT_RUN_CULL_PROGRAM  (1<<2)

/* bin callback type */
#define PFBIN_CALLBACK_PRE_DRAW   1
#define PFBIN_CALLBACK_POST_DRAW  2

/* pfChanBinSort() */
#define PFSORT_NO_SORT		0
#define PFSORT_BY_STATE		1
#define PFSORT_QUICK		2
#define PFSORT_FRONT_TO_BACK	3
#define PFSORT_BACK_TO_FRONT	4
#define PFSORT_DRAW_ORDER	5
#define PFSORT_END		-1

/* PFSORT_BY_STATE only */
#define PFSORT_STATE_BGN	10
#define PFSORT_STATE_END	11

/* pfChanProjMode() - defines viewport projection */
#define PFCHAN_PROJ_OUTPUT_VIEWPORT	0 
#define PFCHAN_PROJ_VIEWPORT	1 
#define PFCHAN_PROJ_WINDOW	2

/* ----------------pfChannel Related Routines------------------ */

/* pfChanPick() */
extern DLLEXPORT void	pfNodePickSetup(pfNode* _node);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfChannel CAPI ------------------------------*/

extern DLLEXPORT pfChannel*           pfNewChan(pfPipe *p);
extern DLLEXPORT pfType*              pfGetChanClassType(void);
extern DLLEXPORT int                  pfGetChanFrustType(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanAspect(pfChannel* _chan, int  which, float xyaspect);
extern DLLEXPORT float                pfGetChanAspect(pfChannel* _chan);
extern DLLEXPORT void                 pfGetChanFOV(const pfChannel* _chan, float *fovH, float *fovV);
extern DLLEXPORT void                 pfChanNearFar(pfChannel* _chan, float n, float f);
extern DLLEXPORT void                 pfGetChanNearFar(const pfChannel* _chan, float *n, float *f);
extern DLLEXPORT void                 pfGetChanNear(const pfChannel* _chan, PFVEC3 _ll, PFVEC3 _lr, PFVEC3 _ul, PFVEC3 _ur);
extern DLLEXPORT void                 pfGetChanFar(const pfChannel* _chan, PFVEC3 _ll, PFVEC3 _lr, PFVEC3 _ul, PFVEC3 _ur);
extern DLLEXPORT void                 pfGetChanPtope(const pfChannel* _chan, pfPolytope *dst);
extern DLLEXPORT int                  pfGetChanEye(const pfChannel* _chan, PFVEC3 eye);
extern DLLEXPORT void                 pfMakePerspChan(pfChannel* _chan, float l, float r, float b, float t);
extern DLLEXPORT void                 pfMakeOrthoChan(pfChannel* _chan, float l, float r, float b, float t);
extern DLLEXPORT void                 pfMakeOutputPerspChan(pfChannel* _chan, float l, float r, float b, float t);
extern DLLEXPORT void                 pfMakeOutputOrthoChan(pfChannel* _chan, float l, float r, float b, float t);
extern DLLEXPORT void                 pfLeftRightBottomTopChan(pfChannel* _chan, float *l, float *r, float *b, float *t);
extern DLLEXPORT void                 pfMakeSimpleChan(pfChannel* _chan, float fov);
extern DLLEXPORT void                 pfOrthoXformChan(pfChannel* _chan, pfFrustum *fr, const PFMATRIX mat);
extern DLLEXPORT int                  pfChanContainsPt(const pfChannel* _chan, const PFVEC3 pt);
extern DLLEXPORT int                  pfChanContainsSphere(const pfChannel* _chan, const pfSphere *sphere);
extern DLLEXPORT int                  pfChanContainsBox(const pfChannel* _chan, const pfBox *box);
extern DLLEXPORT int                  pfChanContainsCyl(const pfChannel* _chan, const pfCylinder *cyl);
extern DLLEXPORT void                 pfApplyChan(pfChannel* _chan);
extern DLLEXPORT pfPipe*              pfGetChanPipe(const pfChannel* _chan);
extern DLLEXPORT pfPipeWindow*        pfGetChanPWin(pfChannel* _chan);
extern DLLEXPORT int                  pfGetChanPWinIndex(pfChannel* _chan);
extern DLLEXPORT pfPipeVideoChannel*  pfGetChanPVChan(const pfChannel* _chan);
extern DLLEXPORT pfPipeVideoChannel*  pfGetChanFramePVChan(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanPWinPVChanIndex(pfChannel* _chan, int num);
extern DLLEXPORT int                  pfGetChanPWinPVChanIndex(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanFOV(pfChannel* _chan, float fovH, float fovV);
extern DLLEXPORT void                 pfChanViewport(pfChannel* _chan, float l, float r, float b, float t);
extern DLLEXPORT void                 pfGetChanViewport(const pfChannel* _chan, float *l, float *r, float *b, float *t);
extern DLLEXPORT void                 pfChanOutputViewport(pfChannel* _chan, float l, float r, float b, float t);
extern DLLEXPORT void                 pfGetChanOutputViewport(const pfChannel* _chan, float *l, float *r, float *b, float *t);
extern DLLEXPORT void                 pfGetChanOrigin(const pfChannel* _chan, int  *xo, int  *yo);
extern DLLEXPORT void                 pfGetChanSize(const pfChannel* _chan, int  *xs, int  *ys);
extern DLLEXPORT void                 pfGetChanOutputOrigin(const pfChannel* _chan, int  *xo, int  *yo);
extern DLLEXPORT void                 pfGetChanOutputSize(const pfChannel* _chan, int  *xs, int  *ys);
extern DLLEXPORT void                 pfChanPixScale(pfChannel* _chan, float s);
extern DLLEXPORT void                 pfChanMinPixScale(pfChannel* _chan, float min);
extern DLLEXPORT void                 pfChanMaxPixScale(pfChannel* _chan, float max);
extern DLLEXPORT float                pfGetChanPixScale(const pfChannel* _chan);
extern DLLEXPORT float                pfGetChanMinPixScale(const pfChannel* _chan);
extern DLLEXPORT float                pfGetChanMaxPixScale(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanProjMode(pfChannel* _chan, int mode);
extern DLLEXPORT int                  pfGetChanProjMode(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanShare(pfChannel* _chan, uint  mask);
extern DLLEXPORT uint                 pfGetChanShare(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanAutoAspect(pfChannel* _chan, int  which);
extern DLLEXPORT int                  pfGetChanAutoAspect(const pfChannel* _chan);
extern DLLEXPORT void                 pfGetChanBaseFrust(const pfChannel* _chan, pfFrustum *frust);
extern DLLEXPORT void                 pfChanViewOffsets(pfChannel* _chan, PFVEC3 xyz, PFVEC3 hpr);
extern DLLEXPORT void                 pfGetChanViewOffsets(const pfChannel* _chan, PFVEC3 xyz, PFVEC3 hpr);
extern DLLEXPORT void                 pfChanView(pfChannel* _chan, PFVEC3 vp, PFVEC3 vd);
extern DLLEXPORT void                 pfChanViewD(pfChannel* _chan, PFVEC3D vp, PFVEC3D vd);
extern DLLEXPORT void                 pfGetChanView(pfChannel* _chan, PFVEC3 vp, PFVEC3 vd);
extern DLLEXPORT void                 pfGetChanViewD(pfChannel* _chan, PFVEC3D vp, PFVEC3D vd);
extern DLLEXPORT void                 pfChanViewMat(pfChannel* _chan, PFMATRIX mat);
extern DLLEXPORT void                 pfChanViewMatD(pfChannel* _chan, PFMATRIX4D mat);
extern DLLEXPORT void                 pfGetChanViewMat(const pfChannel* _chan, PFMATRIX mat);
extern DLLEXPORT void                 pfGetChanViewMatD(const pfChannel* _chan, PFMATRIX4D mat);
extern DLLEXPORT void                 pfGetChanOffsetViewMat(const pfChannel* _chan, PFMATRIX mat);
extern DLLEXPORT void                 pfChanCullPtope(pfChannel* _chan, const pfPolytope *vol);
extern DLLEXPORT int                  pfGetChanCullPtope(const pfChannel* _chan, pfPolytope *vol, int space);
extern DLLEXPORT void*                pfAllocChanData(pfChannel* _chan, int size);
extern DLLEXPORT void                 pfChanData(pfChannel* _chan, void *data, size_t size);
extern DLLEXPORT void*                pfGetChanData(const pfChannel* _chan);
extern DLLEXPORT size_t               pfGetChanDataSize(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanTravFunc(pfChannel* _chan, int trav, pfChanFuncType func);
extern DLLEXPORT pfChanFuncType       pfGetChanTravFunc(const pfChannel* _chan, int trav);
extern DLLEXPORT void                 pfChanTravMode(pfChannel* _chan, int  trav, int  mode);
extern DLLEXPORT int                  pfGetChanTravMode(const pfChannel* _chan, int  trav);
extern DLLEXPORT void                 pfChanTravMask(pfChannel* _chan, int  which, uint  mask);
extern DLLEXPORT uint                 pfGetChanTravMask(const pfChannel* _chan, int  which);
extern DLLEXPORT void                 pfChanStressFilter(pfChannel* _chan, float frac, float low, float high, float s, float max);
extern DLLEXPORT void                 pfGetChanStressFilter(const pfChannel* _chan, float *frac, float *low, float *high, float *s, float *max);
extern DLLEXPORT void                 pfChanStress(pfChannel* _chan, float stress);
extern DLLEXPORT float                pfGetChanStress(const pfChannel* _chan);
extern DLLEXPORT float                pfGetChanLoad(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanScene(pfChannel* _chan, pfScene *s);
extern DLLEXPORT pfScene*             pfGetChanScene(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanESky(pfChannel* _chan, pfEarthSky *_es);
extern DLLEXPORT pfEarthSky*          pfGetChanESky(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanGState(pfChannel* _chan, pfGeoState *_gstate);
extern DLLEXPORT pfGeoState*          pfGetChanGState(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanGStateTable(pfChannel* _chan, pfList *list);
extern DLLEXPORT pfList*              pfGetChanGStateTable(const pfChannel* _chan);
extern DLLEXPORT void                 pfChanLODAttr(pfChannel* _chan, int  attr, float val);
extern DLLEXPORT float                pfGetChanLODAttr(const pfChannel* _chan, int  attr);
extern DLLEXPORT void                 pfChanLODState(pfChannel* _chan, const pfLODState *ls);
extern DLLEXPORT void                 pfGetChanLODState(const pfChannel* _chan, pfLODState *ls);
extern DLLEXPORT void                 pfChanLODStateList(pfChannel* _chan, pfList *stateList);
extern DLLEXPORT pfList*              pfGetChanLODStateList(const pfChannel* _chan);
extern DLLEXPORT int                  pfChanStatsMode(pfChannel* _chan, uint  _mode, uint  _val);
extern DLLEXPORT pfFrameStats*        pfGetChanFStats(pfChannel* _chan);
extern DLLEXPORT void                 pfChanCalligEnable(pfChannel* _chan, int _enable);
extern DLLEXPORT int                  pfGetChanCalligEnable(const pfChannel* _chan);
extern DLLEXPORT pfCalligraphic*      pfGetChanCurCallig(pfChannel* _chan);
extern DLLEXPORT void                 pfChanBinSort(pfChannel* _chan, int bin, int sortType, uint64_t *sortOrders);
extern DLLEXPORT int                  pfGetChanBinSort(pfChannel* _chan, int bin, uint64_t *sortOrders);
extern DLLEXPORT void                 pfChanBinSortPriority(pfChannel* _chan, int bin, int priority);
extern DLLEXPORT int                  pfGetChanBinSortPriority(const pfChannel* _chan, int bin);
extern DLLEXPORT void                 pfChanBinOrder(pfChannel* _chan, int bin, int order);
extern DLLEXPORT int                  pfGetChanBinOrder(const pfChannel* _chan, int bin);
extern DLLEXPORT void                 pfChanBinChildOrderMask(pfChannel* _chan, int bin, uint64_t orderMask);
extern DLLEXPORT uint64_t             pfGetChanBinChildOrderMask(const pfChannel* _chan, int bin);
extern DLLEXPORT void                 pfChanBinFlags(pfChannel* _chan, int bin, int flags);
extern DLLEXPORT int                  pfGetChanBinFlags(const pfChannel* _chan, int bin);
extern DLLEXPORT void                 pfChanBinCallBack(pfChannel* _chan, int bin, int type, pfDListFuncType func);
extern DLLEXPORT pfDListFuncType      pfGetChanBinCallBack(pfChannel* _chan, int bin, int type);
extern DLLEXPORT void                 pfChanBinUserData(pfChannel* _chan, int bin, void *userData, int size);
extern DLLEXPORT void*                pfGetChanBinUserData(pfChannel* _chan, int bin, int *size);
extern DLLEXPORT int                  pfGetChanFreeBin(pfChannel* _chan);
extern DLLEXPORT int                  pfChanFindSubBin(pfChannel* _chan, int bin1, int bin2, int create);
extern DLLEXPORT int                  pfChanFindBinParent(pfChannel* _chan, int bin, int lastKnownParent);
extern DLLEXPORT pfCullProgram*       pfGetChanCullProgram(pfChannel* _chan);
extern DLLEXPORT void                 pfChanAddBinChild(pfChannel* _chan, int child, int root, uint64_t rootMask);
extern DLLEXPORT int                  pfChanIsSubbinOf(pfChannel* _chan, int bin1, int bin2);
extern DLLEXPORT int                  pfAttachChan(pfChannel* _chan, pfChannel *_chan1);
extern DLLEXPORT int                  pfDetachChan(pfChannel* _chan, pfChannel *_chan1);
extern DLLEXPORT int                  pfASDattachChan(pfChannel* _chan, pfChannel *_chan1);
extern DLLEXPORT int                  pfASDdetachChan(pfChannel* _chan, pfChannel *_chan1);
extern DLLEXPORT void                 pfPassChanData(pfChannel* _chan);
extern DLLEXPORT int                  pfChanPick(pfChannel* _chan, int mode, float px, float py, float radius, pfHit **pickList[]);
extern DLLEXPORT void                 pfClearChan(pfChannel* _chan);
extern DLLEXPORT void                 pfDrawChanStats(pfChannel* _chan);
extern DLLEXPORT int                  pfChanNodeIsectSegs(pfChannel* _chan, pfNode *node, pfSegSet *segSet, pfHit **hits[], pfMatrix *ma);
extern DLLEXPORT void                 pfComputeMatricesChan(pfChannel* _chan, pfMatrix *model, pfMatrix *proj);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfEarthSky Tokens ----------------------- */

/* pfESkyMode() */
#define PFES_BUFFER_CLEAR	300
#define PFES_TAG		301
#define PFES_FAST		302
#define PFES_SKY		303
#define PFES_SKY_GRND		304
#define PFES_CLOUDS		305
#define PFES_OVERCAST		306
#define PFES_SKY_CLEAR		307
#define PFES_MULTIPASS_FOG	308
		   
/* pfESkyAttr() */
#define PFES_CLOUD_TOP		310
#define PFES_CLOUD_BOT		311
#define PFES_TZONE_TOP		312
#define PFES_TZONE_BOT		313
#define PFES_GRND_FOG_TOP	316
#define PFES_HORIZ_ANGLE	317
#define PFES_GRND_HT		318
#define PFES_GRND_FOG_BOT	319
#define PFES_GRND_FOG_TZONE	320
#define PFES_FOG_TOP		321
#define PFES_MPFOG_TOP		322
#define PFES_MPFOG_BOT		323
#define PFES_MPFOG_MINRADIUS	324
#define PFES_MPFOG_MAXRADIUS	325		   
#define PFES_TENT_DISTANCE_FACTOR	326		   

/* pfESkyColor()  */
#define PFES_SKY_TOP		350
#define PFES_SKY_BOT		351
#define PFES_HORIZ		352
#define PFES_GRND_FAR		353
#define PFES_GRND_NEAR		354
#define PFES_CLEAR		357

/* pfESkyFog() */
#define PFES_GRND		380
#define PFES_GENERAL		381
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfEarthSky CAPI ------------------------------*/

extern DLLEXPORT pfEarthSky*          pfNewESky(void);
extern DLLEXPORT pfType*              pfGetESkyClassType(void);
extern DLLEXPORT void                 pfESkyMode(pfEarthSky* _esky, int  mode, int  val);
extern DLLEXPORT int                  pfGetESkyMode(pfEarthSky* _esky, int  mode);
extern DLLEXPORT void                 pfESkyAttr(pfEarthSky* _esky, int  mode, float val);
extern DLLEXPORT float                pfGetESkyAttr(pfEarthSky* _esky, int  mode);
extern DLLEXPORT void                 pfESkyColor(pfEarthSky* _esky, int  which, float r, float g, float b, float a);
extern DLLEXPORT void                 pfGetESkyColor(pfEarthSky* _esky, int  which, float *r, float *g, float *b, float *a);
extern DLLEXPORT void                 pfESkyFog(pfEarthSky* _esky, int  which, pfFog *fog);
extern DLLEXPORT pfFog*               pfGetESkyFog(pfEarthSky* _esky, int  which);
extern DLLEXPORT void                 pfESkyFogDensities(pfEarthSky* _esky, int numpt, float *elevations, float *densities);
extern DLLEXPORT void                 pfGetESkyFogDensities(pfEarthSky* _esky, int *numpt, float **elevations, float **densities);
extern DLLEXPORT void                 pfESkyFogTextureElevations(pfEarthSky* _esky, int n, float *elev);
extern DLLEXPORT void                 pfGetESkyFogTextureElevations(pfEarthSky* _esky, int *n, float **elev);
extern DLLEXPORT void                 pfESkyFogTexture(pfEarthSky* _esky, pfTexture *tex);
extern DLLEXPORT pfTexture*           pfGetESkyFogTexture(pfEarthSky* _esky);
extern DLLEXPORT void                 pfESkyFogTextureColorTable(pfEarthSky* _esky, int n, unsigned char *table);
extern DLLEXPORT void                 pfGetESkyFogTextureColorTable(pfEarthSky* _esky, int *n, unsigned char **table);
extern DLLEXPORT void                 pfESkyLoadFogTextureColorTable(pfEarthSky* _esky);
extern DLLEXPORT void                 pfESkyMakeFogTexture(pfEarthSky* _esky);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfMPClipTexture Tokens ----------------------- */
/* for master/slave mpcliptextures */

/* center s, t, r */
#define PFMPCLIPTEXTURE_SHARE_CENTER  0x01 

/* DTR parameters: DTRMode, texLoadTime,texLoadTimeFrac, fadeCount, blurMargin*/
#define PFMPCLIPTEXTURE_SHARE_DTR     0x02 

/* tex level params; LODbias (S, T, R), iborder */
#define PFMPCLIPTEXTURE_SHARE_EDGE    0x04

/* tex level params; minLOD, maxLOD */
#define PFMPCLIPTEXTURE_SHARE_LOD  0x08

/* lodOffset, numEffective levels */
#define PFMPCLIPTEXTURE_SHARE_VIRTUAL 0x10

/* share the default set of shared groups */
#define PFMPCLIPTEXTURE_SHARE_DEFAULT  (PFMPCLIPTEXTURE_SHARE_CENTER | \
                                        PFMPCLIPTEXTURE_SHARE_DTR    | \
                                        PFMPCLIPTEXTURE_SHARE_EDGE   | \
                                        PFMPCLIPTEXTURE_SHARE_LOD    | \
                                        PFMPCLIPTEXTURE_SHARE_VIRTUAL)
/* share all current and future share groups */
#define PFMPCLIPTEXTURE_SHARE_ALL     ~0
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfMPClipTexture CAPI ------------------------------*/

extern DLLEXPORT pfMPClipTexture*     pfNewMPClipTexture(void);
extern DLLEXPORT pfType*              pfGetMPClipTextureClassType(void);
extern DLLEXPORT void                 pfMPClipTextureClipTexture(pfMPClipTexture* _mpcliptexture, pfClipTexture *_clip);
extern DLLEXPORT pfClipTexture*       pfGetMPClipTextureClipTexture(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureCenter(pfMPClipTexture* _mpcliptexture, int _s, int _t, int _r);
extern DLLEXPORT void                 pfGetMPClipTextureCenter(pfMPClipTexture* _mpcliptexture, int *_s, int *_t, int *_r);
extern DLLEXPORT void                 pfMPClipTextureInvalidBorder(pfMPClipTexture* _mpcliptexture, int _invalidBorder);
extern DLLEXPORT int                  pfGetMPClipTextureInvalidBorder(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureVirtualLODOffset(pfMPClipTexture* _mpcliptexture, int _lodOffset);
extern DLLEXPORT int                  pfGetMPClipTextureVirtualLODOffset(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureNumEffectiveLevels(pfMPClipTexture* _mpcliptexture, int _effectiveLevels);
extern DLLEXPORT int                  pfGetMPClipTextureNumEffectiveLevels(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureMaster(pfMPClipTexture* _mpcliptexture, pfMPClipTexture *_master);
extern DLLEXPORT pfMPClipTexture*     pfGetMPClipTextureMaster(const pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT pfList*              pfGetMPClipTextureSlaves(const pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureShareMask(pfMPClipTexture* _mpcliptexture, uint _sharemask);
extern DLLEXPORT uint                 pfGetMPClipTextureShareMask(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureLODRange(pfMPClipTexture* _mpcliptexture, float _min, float _max);
extern DLLEXPORT void                 pfGetMPClipTextureLODRange(pfMPClipTexture* _mpcliptexture, float *_min, float *_max);
extern DLLEXPORT pfPipe*              pfGetMPClipTexturePipe(const pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureDTRMode(pfMPClipTexture* _mpcliptexture, uint _mask);
extern DLLEXPORT uint                 pfGetMPClipTextureDTRMode(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureLODBias(pfMPClipTexture* _mpcliptexture, float _lodBiasS, float _lodBiasT, float _lodBiasR);
extern DLLEXPORT void                 pfGetMPClipTextureLODBias(pfMPClipTexture* _mpcliptexture, float *_lodBiasS, float *_lodBiasT, float *_lodBiasR);
extern DLLEXPORT void                 pfMPClipTextureMagFilter(pfMPClipTexture* _mpcliptexture, uint _magFilter);
extern DLLEXPORT uint                 pfGetMPClipTextureMagFilter(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureDTRFadeCount(pfMPClipTexture* _mpcliptexture, int _count);
extern DLLEXPORT int                  pfGetMPClipTextureDTRFadeCount(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureDTRBlurMargin(pfMPClipTexture* _mpcliptexture, float frac);
extern DLLEXPORT float                pfGetMPClipTextureDTRBlurMargin(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureTexLoadTime(pfMPClipTexture* _mpcliptexture, float _msec);
extern DLLEXPORT float                pfGetMPClipTextureTexLoadTime(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureTexLoadTimeFrac(pfMPClipTexture* _mpcliptexture, float _frac);
extern DLLEXPORT float                pfGetMPClipTextureTexLoadTimeFrac(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT float                pfGetMPClipTextureCurTexLoadTime(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureBeginRecord(pfMPClipTexture* _mpcliptexture, const char *fileName);
extern DLLEXPORT void                 pfMPClipTextureEndRecord(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT int                  pfMPClipTextureIsRecording(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureBeginPlay(pfMPClipTexture* _mpcliptexture, const char *fileName);
extern DLLEXPORT void                 pfMPClipTextureEndPlay(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT int                  pfMPClipTextureIsPlaying(pfMPClipTexture* _mpcliptexture);
extern DLLEXPORT void                 pfMPClipTextureLODOffsetLimit(pfMPClipTexture* _mpcliptexture, int _lo, int _hi);
extern DLLEXPORT void                 pfGetMPClipTextureLODOffsetLimit(pfMPClipTexture* _mpcliptexture, int *_lo, int *_hi);
extern DLLEXPORT void                 pfMPClipTextureNumEffectiveLevelsLimit(pfMPClipTexture* _mpcliptexture, int _lo, int _hi);
extern DLLEXPORT void                 pfGetMPClipTextureNumEffectiveLevelsLimit(pfMPClipTexture* _mpcliptexture, int *_lo, int *_hi);
extern DLLEXPORT void                 pfMPClipTextureMinLODLimit(pfMPClipTexture* _mpcliptexture, float _lo, float _hi);
extern DLLEXPORT void                 pfGetMPClipTextureMinLODLimit(pfMPClipTexture* _mpcliptexture, float *_lo, float *_hi);
extern DLLEXPORT void                 pfMPClipTextureMaxLODLimit(pfMPClipTexture* _mpcliptexture, float _lo, float _hi);
extern DLLEXPORT void                 pfGetMPClipTextureMaxLODLimit(pfMPClipTexture* _mpcliptexture, float *_lo, float *_hi);
extern DLLEXPORT void                 pfMPClipTextureLODBiasLimit(pfMPClipTexture* _mpcliptexture, float _Slo, float _Shi, float _Tlo, float _Thi, float _Rlo, float _Rhi);
extern DLLEXPORT void                 pfGetMPClipTextureLODBiasLimit(pfMPClipTexture* _mpcliptexture, float *_Slo, float *_Shi, float *_Tlo, float *_Thi, float *_Rlo, float *_Rhi);
extern DLLEXPORT void                 pfApplyMPClipTexture(pfMPClipTexture* _mpcliptexture);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */

#define PFVFOG_PATCHY_FOG  1
#define PFVFOG_LAYERED_FOG 2

/* fog attributes */
#define PFVFOG_COLOR        0x01
#define PFVFOG_DENSITY      0x02
#define PFVFOG_RESOLUTION   0x03
#define PFVFOG_DENSITY_BIAS 0x04
#define PFVFOG_MODE         0x05
#define PFVFOG_PATCHY_MODE  0x06
#define PFVFOG_LAYERED_MODE 0x07

#define PFVFOG_3D_TEX_SIZE  0x08
#define PFVFOG_MAX_DISTANCE 0x09

#define PFVFOG_ROTATE_NODE                 0x0c
#define PFVFOG_PATCHY_TEXTURE_BOTTOM       0x0d
#define PFVFOG_PATCHY_TEXTURE_TOP          0x0e

#define PFVFOG_LIGHT_SHAFT_ATTEN_SCALE     0x10
#define PFVFOG_LIGHT_SHAFT_ATTEN_TRANSLATE 0x11
#define PFVFOG_LIGHT_SHAFT_DARKEN_FACTOR   0x12

/* fog mode */
#define PFVFOG_LINEAR       GL_LINEAR
#define PFVFOG_EXP          GL_EXP
#define PFVFOG_EXP2         GL_EXP2

/* binary flags */
#define PFVFOG_FLAG_CLOSE_SURFACES     (1<<0) /* use stencil test to eliminate 
						 problems caused by surfaces 
						 with equal depth */
#define PFVFOG_FLAG_FORCE_2D_TEXTURE   (1<<1) /* forces using of 2D textures
						 for layered fog */
#define PFVFOG_FLAG_LAYERED_PATCHY_FOG (1<<2) /* use layered fog to control 
						 density of a patchy fog */
#define PFVFOG_FLAG_FORCE_PATCHY_PASS  (1<<3) /* do not test whether patchy 
						 fog bin is culled out */
#define PFVFOG_FASTER_PATCHY_FOG       (1<<4) /* use faster algorithm for 
						 patchy fog */
#define PFVFOG_FLAG_NO_OBJECT_IN_FOG   (1<<5) /* speeds up the faster algorithm
						 by assuming that no scene 
						 object is inside fog */

#define PFVFOG_FLAG_SELF_SHADOWING       (1<<6) /* enables self-shadowing of
						   layered fog */
#define PFVFOG_FLAG_DARKEN_OBJECTS       (1<<7) /* enables darkening of objects
						   under self-shadowed layered
						   fog */
#define PFVFOG_FLAG_FOG_FILTER           (1<<8) /* simulate secondary 
						   scattering in layered fog */
#define PFVFOG_FLAG_PATCHY_FOG_1DTEXTURE (1<<9) /* use a 1D texture to color 
						   surface of patchy fog. */
#define PFVFOG_FLAG_SEPARATE_NODE_BINS   (1<<10) /* use different bins for each
						    node supplied by addNode */
#define PFVFOG_FLAG_SCREEN_BOUNDING_RECT (1<<11) /* use bounding rectangle
						    in screen space to limit 
						    the areas that need to be
						    processesed - on by default
						    Actually, there is no 
						    reason to switch it off. */
#define PFVFOG_FLAG_DRAW_NODES_SEPARATELY (1<<12) /* draw fog nodes separately
						     in back-to-front order */
#define PFVFOG_FLAG_USER_PATCHY_FOG_TEXTURE (1<<13) /* user-supplied texture is
						       used on the surface of
						       patchy fog */
#define PFVFOG_FLAG_LIGHT_SHAFT           (1<<14) /* pfVolFog defines a light
						     shaft */
#define PFVFOG_FLAG_USE_CULL_PROGRAM      (1<<15) /* use cull programs to speed
						     up rendering passes */

#define PFVFOG_LAST_USER_FLAG             ((uint64_t)(1<<24))
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfVolFog CAPI ------------------------------*/

extern DLLEXPORT pfVolFog*            pfNewVolFog(void *arena);
extern DLLEXPORT pfType*              pfGetVolFogClassType(void);
extern DLLEXPORT void                 pfVolFogSetColor(pfVolFog* _volfog, float _r, float _g, float _b);
extern DLLEXPORT void                 pfVolFogGetColor(pfVolFog* _volfog, float *r, float *g, float *b);
extern DLLEXPORT void                 pfVolFogSetFlags(pfVolFog* _volfog, int which, int val);
extern DLLEXPORT int                  pfVolFogGetFlags(pfVolFog* _volfog, int which);
extern DLLEXPORT void                 pfVolFogSetVal(pfVolFog* _volfog, int which, float  val);
extern DLLEXPORT void                 pfVolFogGetVal(pfVolFog* _volfog, int which, float *val);
extern DLLEXPORT void                 pfVolFogSetAttr(pfVolFog* _volfog, int which, void *attr);
extern DLLEXPORT void                 pfVolFogGetAttr(pfVolFog* _volfog, int which, void *attr);
extern DLLEXPORT void                 pfApplyVolFog(pfVolFog* _volfog, pfScene *scene);
extern DLLEXPORT void                 pfDrawVolFog(pfVolFog* _volfog, pfChannel *channel);
extern DLLEXPORT void                 pfVolFogAddChannel(pfVolFog* _volfog, pfChannel *channel);
extern DLLEXPORT void                 pfVolFogUpdateView(pfVolFog* _volfog);
extern DLLEXPORT void                 pfVolFogRotateCS(pfVolFog* _volfog, pfMatrix *mat);
extern DLLEXPORT void                 pfVolFogAddNode(pfVolFog* _volfog, pfNode *node);
extern DLLEXPORT void                 pfVolFogDeleteNode(pfVolFog* _volfog, pfNode *node);
extern DLLEXPORT void                 pfVolFogSetDensity(pfVolFog* _volfog, float density);
extern DLLEXPORT float                pfVolFogGetDensity(pfVolFog* _volfog);
extern DLLEXPORT void                 pfVolFogSetIndexVal(pfVolFog* _volfog, int which, int index, float  val);
extern DLLEXPORT void                 pfVolFogGetIndexVal(pfVolFog* _volfog, int which, int index, float *val);
extern DLLEXPORT void                 pfVolFogSetNodeVal(pfVolFog* _volfog, int which, pfNode *node, float  val);
extern DLLEXPORT void                 pfVolFogGetNodeVal(pfVolFog* _volfog, int which, pfNode *node, float *val);
extern DLLEXPORT void                 pfVolFogSetIndexAttr(pfVolFog* _volfog, int which, int index, void *attr);
extern DLLEXPORT void                 pfVolFogGetIndexAttr(pfVolFog* _volfog, int which, int index, void *attr);
extern DLLEXPORT void                 pfVolFogSetNodeAttr(pfVolFog* _volfog, int which, pfNode *node, void *attr);
extern DLLEXPORT void                 pfVolFogGetNodeAttr(pfVolFog* _volfog, int which, pfNode *node, void *attr);
extern DLLEXPORT pfTexture*           pfGetVolFogTexture(pfVolFog* _volfog);
extern DLLEXPORT void                 pfVolFogAddPoint(pfVolFog* _volfog, float elevation, float density);
extern DLLEXPORT void                 pfVolFogAddColoredPoint(pfVolFog* _volfog, float elevation, float density, float _r, float _g, float _b);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfStateMapList CAPI ------------------------------*/

extern DLLEXPORT pfStateMapList*      pfNewStateMapList(pfStateMapList *last, void *arena);
extern DLLEXPORT pfType*              pfGetStateMapListClassType(void);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfRotorWash CAPI ------------------------------*/

extern DLLEXPORT pfRotorWash*         pfNewRotorWash(int mesh_rings, int mesh_spokes, void *arena);
extern DLLEXPORT pfType*              pfGetRotorWashClassType(void);
extern DLLEXPORT pfNode*              pfGetRotorWashNode(pfRotorWash* _rotorwash);
extern DLLEXPORT pfTexture*           pfGetRotorWashTexture(pfRotorWash* _rotorwash, pfGeoState *gstate, int frame);
extern DLLEXPORT void                 pfRotorWashTextures(pfRotorWash* _rotorwash, pfGeoState **gstates, int num, char *name, int numtex);
extern DLLEXPORT void                 pfRotorWashColor(pfRotorWash* _rotorwash, pfGeoState *geostate, float r, float g, float b, float a);
extern DLLEXPORT void                 pfRotorWashRadii(pfRotorWash* _rotorwash, float in_1, float in_2, float out_1, float out_2);
extern DLLEXPORT void                 pfRotorWashAlpha(pfRotorWash* _rotorwash, float a_inner, float a_outer);
extern DLLEXPORT void                 pfRotorWashDisplacement(pfRotorWash* _rotorwash, float value);
extern DLLEXPORT void                 pfRotorWashSubdivisionLevel(pfRotorWash* _rotorwash, int level);
extern DLLEXPORT int                  pfGetRotorWashSubdivisionLevel(pfRotorWash* _rotorwash);
extern DLLEXPORT void                 pfRotorWashMaxTriangles(pfRotorWash* _rotorwash, int maxTris);
extern DLLEXPORT int                  pfGetRotorWashMaxTriangles(pfRotorWash* _rotorwash);
extern DLLEXPORT void                 pfRotorWashHighlight(pfRotorWash* _rotorwash, float red, float green, float blue);
extern DLLEXPORT void                 pfRotorWashUnhighlight(pfRotorWash* _rotorwash);
extern DLLEXPORT void                 pfRotorWashCompute(pfRotorWash* _rotorwash, pfNode *terrain, float x, float y,float phase);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
/* cull program instruction opcodes */
#define PFCULLPG_TEST_POLYTOPE            0

#define PFCULLPG_ASSIGN_BIN_MAYBE         1
#define PFCULLPG_ASSIGN_BIN_TRUE          1  // true is the same as maybe
#define PFCULLPG_ASSIGN_BIN_ALL_IN        2
#define PFCULLPG_ASSIGN_BIN_ALL_OUT       3
#define PFCULLPG_ASSIGN_BIN_FALSE         3  // false is the same as all out
#define PFCULLPG_ASSIGN_BIN               4

#define PFCULLPG_ADD_BIN_MAYBE            5
#define PFCULLPG_ADD_BIN_TRUE             5  // true is the same as maybe
#define PFCULLPG_ADD_BIN_ALL_IN           6
#define PFCULLPG_ADD_BIN_ALL_OUT          7
#define PFCULLPG_ADD_BIN_FALSE            7  // false is the same as all out
#define PFCULLPG_ADD_BIN                  8

#define PFCULLPG_JUMP_MAYBE               9
#define PFCULLPG_JUMP_TRUE                9  // true is the same as maybe
#define PFCULLPG_JUMP_ALL_IN             10
#define PFCULLPG_JUMP_ALL_OUT            11
#define PFCULLPG_JUMP_FALSE              11
#define PFCULLPG_JUMP                    12

#define PFCULLPG_TEST_IS_SUBBIN_OF       13

#define PFCULLPG_RETURN                  14

/* bit flag for return instruction */
#define PFCULLPG_TEST_TRANSPARENCY       (1<<0)
#define PFCULLPG_DONT_TEST_TRANSPARENCY  (1<<1)
#define PFCULLPG_TEST_LPOINTS            (1<<2)
#define PFCULLPG_DONT_TEST_LPOINTS       (1<<3)
#define PFCULLPG_CULL_ON_ALL_IN          (1<<4) // cull based on the last test
#define PFCULLPG_CULL_ON_ALL_OUT         (1<<5) // cull based on the last test
#define PFCULLPG_CULL                    (1<<6) // cull 

/* flags stored with cull program */
#define PFCULLPG_GEOSET_PROGRAM_ACTIVATED (1<<0)
#define PFCULLPG_NODE_PROGRAM_ACTIVATED   (1<<1)
#define PFCULLPG_DONOT_XFORM_BBOXES       (1<<2)

/* flags used in resetCullProgram */
#define PFCULLPG_GEOSET_PROGRAM           (1<<0)
#define PFCULLPG_NODE_PROGRAM             (1<<1)
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfCullProgram CAPI ------------------------------*/

extern DLLEXPORT int                  pfCullProgramTestPolytope(pfCullProgram* _cullprogram, int index);
extern DLLEXPORT void                 pfCullProgramAddBinParent(pfCullProgram* _cullprogram, int bin);
extern DLLEXPORT int                  pfCullProgramIsSubbinOf(pfCullProgram* _cullprogram, int bin);
extern DLLEXPORT void                 pfCullProgramResetBinParents(pfCullProgram* _cullprogram);
extern DLLEXPORT void                 pfCullProgramResetPgm(pfCullProgram* _cullprogram, int flags);
extern DLLEXPORT void                 pfCullProgramAddPgmOpcode(pfCullProgram* _cullprogram, int opcode, int data);
extern DLLEXPORT void                 pfCullProgramAddPgmInstruction(pfCullProgram* _cullprogram, _pfCullPgInstruction instruction, int data);
extern DLLEXPORT void                 pfCullProgramNumPolytopes(pfCullProgram* _cullprogram, int i);
extern DLLEXPORT int                  pfGetCullProgramNumPolytopes(pfCullProgram* _cullprogram);
extern DLLEXPORT void                 pfCullProgramPolytope(pfCullProgram* _cullprogram, int index, pfPolytope *pol);
extern DLLEXPORT pfPolytope*          pfGetCullProgramPolytope(pfCullProgram* _cullprogram, int index);
extern DLLEXPORT void                 pfCullProgramFlags(pfCullProgram* _cullprogram, int which, int value);
extern DLLEXPORT int                  pfGetCullProgramFlags(pfCullProgram* _cullprogram, int which);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
/* pfShadow flags */
#define PFSHD_BLEND_TEXTURES        (1<<0)

/* pfShadow parameters */
#define PFSHD_PARAM_TEXTURE_SIZE     1
#define PFSHD_PARAM_NUM_TEXTURES     2
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfShadow CAPI ------------------------------*/

extern DLLEXPORT pfShadow*            pfNewShadow(void);
extern DLLEXPORT pfType*              pfGetShadowClassType(void);
extern DLLEXPORT void                 pfShadowNumCasters(pfShadow* _shadow, int index);
extern DLLEXPORT int                  pfGetShadowNumCasters(pfShadow* _shadow);
extern DLLEXPORT void                 pfShadowShadowCaster(pfShadow* _shadow, int index, pfNode *caster, PFMATRIX mat);
extern DLLEXPORT void                 pfShadowAdjustCasterCenter(pfShadow* _shadow, int index, PFVEC3 trans);
extern DLLEXPORT pfNode*              pfGetShadowShadowCaster(pfShadow* _shadow, int index);
extern DLLEXPORT pfMatrix*            pfGetShadowShadowCasterMatrix(pfShadow* _shadow, int index);
extern DLLEXPORT void                 pfShadowNumSources(pfShadow* _shadow, int num);
extern DLLEXPORT int                  pfGetShadowNumSources(pfShadow* _shadow);
extern DLLEXPORT void                 pfShadowSourcePos(pfShadow* _shadow, int index, float x, float y, float z, float w);
extern DLLEXPORT void                 pfGetShadowSourcePos(pfShadow* _shadow, int index, float *x, float *y, float *z, float *w);
extern DLLEXPORT void                 pfShadowLight(pfShadow* _shadow, int index, pfLight *light);
extern DLLEXPORT pfLight*             pfGetShadowLight(pfShadow* _shadow, int index);
extern DLLEXPORT void                 pfShadowAmbientFactor(pfShadow* _shadow, int light, float factor);
extern DLLEXPORT float                pfGetShadowAmbientFactor(pfShadow* _shadow, int light);
extern DLLEXPORT void                 pfShadowShadowTexture(pfShadow* _shadow, int caster, int light, pfTexture *tex);
extern DLLEXPORT pfTexture*           pfGetShadowShadowTexture(pfShadow* _shadow, int caster, int light);
extern DLLEXPORT void                 pfShadowTextureBlendFunc(pfShadow* _shadow, _pfBlendFunc blendFunc);
extern DLLEXPORT void                 pfShadowAddChannel(pfShadow* _shadow, pfChannel *channel);
extern DLLEXPORT void                 pfShadowUpdateView(pfShadow* _shadow);
extern DLLEXPORT void                 pfShadowUpdateCaster(pfShadow* _shadow, int index, PFMATRIX mat);
extern DLLEXPORT void                 pfShadowApply(pfShadow* _shadow);
extern DLLEXPORT void                 pfShadowDraw(pfShadow* _shadow, pfChannel *chan);
extern DLLEXPORT void                 pfShadowFlags(pfShadow* _shadow, int which, int value);
extern DLLEXPORT int                  pfGetShadowFlags(const pfShadow* _shadow, int which);
extern DLLEXPORT void                 pfShadowVal(pfShadow* _shadow, int caster, int light, int which, float  val);
extern DLLEXPORT float                pfGetShadowVal(pfShadow* _shadow, int caster, int light, int which);
extern DLLEXPORT pfDirData*           pfGetShadowDirData(pfShadow* _shadow, int caster, int light);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
/* constants */
#define PFDD_MAX_RINGS 128   /* maximum number of rings of views */

/* different ways of generating directions */
#define PFDD_2D_ROTATE_AROUND_UP    1
#define PFDD_RINGS_OF_VIEWS         2
#define PFDD_3D_UNIFORM             3

/* flags */
#define PFIBR_USE_2D_TEXTURES   (1<<1)
#define PFIBR_3D_VIEWS          (1<<2)
#define PFIBR_NEAREST           (1<<3)
#define PFIBR_USE_REG_COMBINERS (1<<4)
#define PFIBR_MIPMAP_3DTEXTURES (1<<5)
#define PFIBR_USE_PROXY         (1<<6)
#define PFIBR_FAST_BLEND        (1<<7)


#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfDirData CAPI ------------------------------*/

extern DLLEXPORT pfDirData*           pfNewDirData(void *arena);
extern DLLEXPORT pfType*              pfGetDirDataClassType(void);
extern DLLEXPORT void                 pfDirDataData(pfDirData* _dirdata, int num, pfVec3 *directions, void **userData);
extern DLLEXPORT void                 pfGetDirDataDirData(pfDirData* _dirdata, int *num, pfVec3 **directions, void ***userData);
extern DLLEXPORT void                 pfDirDataDirections(pfDirData* _dirdata, int num, pfVec3 *directions);
extern DLLEXPORT void                 pfDirDataGenerateDirections(pfDirData* _dirdata, int num, int type, float *data);
extern DLLEXPORT void                 pfGetDirDataDirections(pfDirData* _dirdata, int *num, pfVec3 **directions);
extern DLLEXPORT int                  pfGetDirDataNumGroups(pfDirData* _dirdata, int *viewsPerGroup);
extern DLLEXPORT int                  pfGetDirDataGroup(pfDirData* _dirdata, int group, int *views);
extern DLLEXPORT int                  pfGetDirDataNeighboringViews(pfDirData* _dirdata, int viewIndex, int **neighbors);
extern DLLEXPORT int                  pfGetDirDataFlags(const pfDirData* _dirdata, int which);
extern DLLEXPORT int                  pfDirDataFindData(pfDirData* _dirdata, pfVec3 *dir, pfVec3 *resDir, void **resData);
extern DLLEXPORT int                  pfDirDataFindData2(pfDirData* _dirdata, pfVec3 *dir, pfVec3 *resDir, void **resData, float *weight);
extern DLLEXPORT int                  pfDirDataFindData3(pfDirData* _dirdata, pfVec3 *dir, pfVec3 *resDir, void **resData, float *weight);
extern DLLEXPORT int                  pfDirDataFindData4(pfDirData* _dirdata, pfVec3 *dir, pfVec3 *resDir, void **resData, float *weight);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfIBRtexture CAPI ------------------------------*/

extern DLLEXPORT pfIBRtexture*        pfNewIBRtexture(void *arena);
extern DLLEXPORT pfType*              pfGetIBRtextureClassType(void);
extern DLLEXPORT void                 pfIBRtextureLoadIBRTexture(pfIBRtexture* _ibrtexture, char *_format, int _numTex, int _skipTex);
extern DLLEXPORT void                 pfIBRtextureIBRTexture(pfIBRtexture* _ibrtexture, pfTexture **_textures, int _numTex);
extern DLLEXPORT void                 pfGetIBRtextureIBRTextures(pfIBRtexture* _ibrtexture, pfTexture ***textures, int *numTex);
extern DLLEXPORT void                 pfIBRtextureIBRdirections(pfIBRtexture* _ibrtexture, pfVec3 *directions, int numDirs);
extern DLLEXPORT void                 pfGetIBRtextureIBRdirections(pfIBRtexture* _ibrtexture, pfVec3 **directions, int *numDirs);
extern DLLEXPORT int                  pfGetIBRtextureNumIBRTextures(pfIBRtexture* _ibrtexture);
extern DLLEXPORT pfDirData*           pfGetIBRtextureDirData(pfIBRtexture* _ibrtexture);
extern DLLEXPORT void                 pfIBRtextureComputeProxyTexCoords(pfIBRtexture* _ibrtexture, pfIBRnode *node, pfVec3 *center, float aspect, float scale, pfVec2 *texShift);
extern DLLEXPORT pfTexture*           pfGetIBRtextureDefaultTexture(pfIBRtexture* _ibrtexture);
extern DLLEXPORT void                 pfIBRtextureFlags(pfIBRtexture* _ibrtexture, int which, int value);
extern DLLEXPORT int                  pfGetIBRtextureFlags(const pfIBRtexture* _ibrtexture, int which);
extern DLLEXPORT void                 pfIBRtextureDirection(pfIBRtexture* _ibrtexture, float dir);
extern DLLEXPORT float                pfGetIBRtextureDirection(const pfIBRtexture* _ibrtexture);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */

/* constants */

#define		PFLOAD_COEFF	1

/* flags */

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfLoadBalance CAPI ------------------------------*/

extern DLLEXPORT pfLoadBalance*       pfNewLoadBalance(int size);
extern DLLEXPORT pfType*              pfGetLoadBalanceClassType(void);
extern DLLEXPORT void                 pfLoadBalanceLoad(pfLoadBalance* _loadbalance, int index, float load);
extern DLLEXPORT float                pfGetLoadBalanceLoad(pfLoadBalance* _loadbalance, int index);
extern DLLEXPORT void                 pfLoadBalanceBalance(pfLoadBalance* _loadbalance);
extern DLLEXPORT float                pfGetLoadBalanceWork(pfLoadBalance* _loadbalance, int index);
extern DLLEXPORT void                 pfLoadBalanceVal(pfLoadBalance* _loadbalance, int what, float val);
extern DLLEXPORT float                pfGetLoadBalanceVal(pfLoadBalance* _loadbalance, int what);
extern DLLEXPORT void                 pfLoadBalanceNumActive(pfLoadBalance* _loadbalance, int num);
extern DLLEXPORT int                  pfGetLoadBalanceNumActive(pfLoadBalance* _loadbalance);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */

/* constants */

/* Compositor types */

#define		PFCOMP_TYPE	0
#define		PFCOMP_2x2	1
#define		PFCOMP_4x1HORIZ	2
#define		PFCOMP_4x1VERT	3
#define		PFCOMP_4xAA	4
#define		PFCOMP_2x1HORIZ	5
#define		PFCOMP_2x1VERT	6

/* flags */
#define		PFLOAD_BALANCE  5
#define		PFCOMP_CLIPPING 6

#define		PFCOMP_MAX_INPUTS	4

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfCompositor CAPI ------------------------------*/

extern DLLEXPORT pfCompositor*        pfNewCompositor(void);
extern DLLEXPORT pfType*              pfGetCompositorClassType(void);
extern DLLEXPORT int                  pfGetCompositorCompType(pfCompositor* _compositor);
extern DLLEXPORT int                  pfCompositorAddChild(pfCompositor* _compositor, int pipe_id);
extern DLLEXPORT int                  pfCompositorAddCompositor(pfCompositor* _compositor, pfCompositor *comp);
extern DLLEXPORT pfCompositor*        pfGetCompositorChild(pfCompositor* _compositor, int index);
extern DLLEXPORT pfChannel*           pfGetCompositorChan(pfCompositor* _compositor, int index);
extern DLLEXPORT pfPipe*              pfGetCompositorPipe(pfCompositor* _compositor, int index);
extern DLLEXPORT int                  pfGetCompositorPipeId(pfCompositor* _compositor, int index);
extern DLLEXPORT void                 pfCompositorPerspective(pfCompositor* _compositor, uint32_t index, float left, float right, float bottom, float top);
extern DLLEXPORT void                 pfCompositorOrtho(pfCompositor* _compositor, uint32_t index, float left, float right, float bottom, float top);
extern DLLEXPORT void                 pfCompositorNearFar(pfCompositor* _compositor, uint32_t index, float n, float f);
extern DLLEXPORT void                 pfGetCompositorNearFar(pfCompositor* _compositor, uint32_t index, float *n, float *f);
extern DLLEXPORT void                 pfGetCompositorFrustum(pfCompositor* _compositor, uint32_t index, float *left, float *right, float *bottom, float *top);
extern DLLEXPORT int                  pfGetCompositorFrustType(pfCompositor* _compositor);
extern DLLEXPORT void                 pfCompositorViewport(pfCompositor* _compositor, float left, float right, float bottom, float top);
extern DLLEXPORT void                 pfGetCompositorViewport(pfCompositor* _compositor, float *left, float *right, float *bottom, float *top);
extern DLLEXPORT void                 pfCompositorVal(pfCompositor* _compositor, int what, float val);
extern DLLEXPORT float                pfGetCompositorVal(pfCompositor* _compositor, int what);
extern DLLEXPORT void                 pfCompositorMode(pfCompositor* _compositor, int what, int val);
extern DLLEXPORT int                  pfGetCompositorMode(pfCompositor* _compositor, int what);
extern DLLEXPORT void                 pfCompositorLoadBalancer(pfCompositor* _compositor, pfLoadBalance *balancer);
extern DLLEXPORT pfLoadBalance*       pfGetCompositorLoadBalancer(pfCompositor* _compositor);
extern DLLEXPORT void                 pfCompositorChildViewport(pfCompositor* _compositor, uint32_t index, float left, float right, float bottom, float top);
extern DLLEXPORT void                 pfCompositorChildFrustum(pfCompositor* _compositor, uint32_t index, float left, float right, float bottom, float top);
extern DLLEXPORT void                 pfCompositorChildSubdivision(pfCompositor* _compositor, uint32_t index, uint32_t xorigin, uint32_t yorigin, uint32_t width, uint32_t height);
extern DLLEXPORT void                 pfGetCompositorChildViewport(pfCompositor* _compositor, uint32_t index, float *left, float *right, float *bottom, float *top);
extern DLLEXPORT void                 pfGetCompositorChildFrustum(pfCompositor* _compositor, uint32_t index, float *left, float *right, float *bottom, float *top);
extern DLLEXPORT void                 pfGetCompositorChildSubdivision(pfCompositor* _compositor, uint32_t index, uint32_t *xorigin, uint32_t *yorigin, uint32_t *width, uint32_t *height);
extern DLLEXPORT void                 pfCompositorMasterPipe(pfCompositor* _compositor, pfPipe *master);
extern DLLEXPORT pfPipe*              pfGetCompositorMasterPipe(pfCompositor* _compositor);
extern DLLEXPORT void                 pfCompositorReconfig(pfCompositor* _compositor);
extern DLLEXPORT void                 pfCompositorChannelClipped(pfCompositor* _compositor, uint64_t chan_num, unsigned char clip_this_channel);
extern DLLEXPORT unsigned char        pfGetCompositorChannelClipped(pfCompositor* _compositor, uint64_t chan_num);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfCteChanData CAPI ------------------------------*/

extern DLLEXPORT pfCteChanData*       pfNewCteChanData( pfChannel* chan , void *arena);
extern DLLEXPORT pfType*              pfGetCteChanDataClassType(void);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
#endif

extern DLLEXPORT void cteDrawOverlayTextures( pfChannel* chan, pfClipTexture* cliptex );

#ifdef __cplusplus
#endif /* !PF_CPLUSPLUS_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C"  */
/* pfGeoSet optimization types */
#define PFSK_BACKFACE_CULL            	0x00000001
#define PFSK_FRUSTUM_CULL               0x00000002

#if 0
typedef pfGeoSet *(*pfSidekickFunc)(pfGeoSet *gset, pfDispListOptimizer *op, void *userData);
#endif

/* Matrices available for Sidekick user callback. */

#define PFSK_MODELVIEW			1
#define PFSK_PROJECTION			2
#define PFSK_COMBINED			3

/* Masks for attribute copying in cloneGSet */

#define PFSK_COORD3		0x001
#define PFSK_COLOR4		0x002
#define PFSK_NORMAL3		0x004
#define PFSK_TEXCOORD2		0x008
#define PFSK_ATTR_LENGTHS	0x010

typedef struct
{
    int spare;
} pfDispListOptimizerExtraData;

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfDispListOptimizer CAPI ------------------------------*/

extern DLLEXPORT pfDispListOptimizer* pfNewDLOptimizer(void *arena);
extern DLLEXPORT pfType*              pfGetDLOptimizerClassType(void);
extern DLLEXPORT void                 pfDLOptimizerDispList(pfDispListOptimizer* _dloptimizer, pfDispList *_dlist);
extern DLLEXPORT void                 pfDLOptimizerReset(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT int                  pfDLOptimizerAdvance(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT void                 pfDLOptimizerNumServers(pfDispListOptimizer* _dloptimizer, int num);
extern DLLEXPORT int                  pfGetDLOptimizerNumServers(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT void                 pfDLOptimizerMyId(pfDispListOptimizer* _dloptimizer, int id);
extern DLLEXPORT int                  pfGetDLOptimizerMyId(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT void                 pfDLOptimizerChannel(pfDispListOptimizer* _dloptimizer, pfChannel *c);
extern DLLEXPORT pfChannel*           pfGetDLOptimizerChannel(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT void                 pfDLOptimizerInitModelviewMatrix(pfDispListOptimizer* _dloptimizer, PFMATRIX M);
extern DLLEXPORT void                 pfDLOptimizerInitProjectionMatrix(pfDispListOptimizer* _dloptimizer, PFMATRIX M);
extern DLLEXPORT int                  pfGetDLOptimizerNumGSetsProcessed(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT void                 pfDLOptimizerTimeSlice(pfDispListOptimizer* _dloptimizer, float t);
extern DLLEXPORT float                pfGetDLOptimizerTimeSlice(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT void                 pfDLOptimizerOptimizationMask(pfDispListOptimizer* _dloptimizer, unsigned int m);
extern DLLEXPORT unsigned int         pfGetDLOptimizerOptimizationMask(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT int                  pfGetDLOptimizerNumTrisRemoved(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT int                  pfGetDLOptimizerNumLinesRemoved(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT int                  pfGetDLOptimizerNumPointsRemoved(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT int                  pfGetDLOptimizerNumVertsRemoved(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT void                 pfDLOptimizerUserFunc(pfDispListOptimizer* _dloptimizer, pfSidekickFunc func, void *userFuncData);
extern DLLEXPORT pfSidekickFunc       pfGetDLOptimizerUserFunc(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT void*                pfGetDLOptimizerUserFuncData(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT void                 pfDLOptimizerCullFace(pfDispListOptimizer* _dloptimizer, int mode);
extern DLLEXPORT int                  pfGetDLOptimizerCullFace(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT void*                pfGetDLOptimizerTmpMemory(pfDispListOptimizer* _dloptimizer, int size);
extern DLLEXPORT pfGeoSet*            pfGetDLOptimizerTmpGSet(pfDispListOptimizer* _dloptimizer);
extern DLLEXPORT pfMatrix*            pfGetDLOptimizerMatrix(pfDispListOptimizer* _dloptimizer, int which);
extern DLLEXPORT pfGeoSet*            pfDLOptimizerCloneGSet(pfDispListOptimizer* _dloptimizer, pfGeoSet *gset, unsigned int copyMask);
extern DLLEXPORT pfGeoSet*            pfDLOptimizerOptimize(pfDispListOptimizer* _dloptimizer, pfGeoSet *gset);

#endif /* !PF_C_API */

/******************************** Nodes *************************************/



#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfNode Tokens ----------------------- */

/* additional intersection modes for libpf */
#define PFTRAV_IS_GEODE		0x100 /* intersect with geode bounding volume */
#define PFTRAV_IS_TEXT		0x200 /* intersect with text bounding volume */
#define PFTRAV_IS_PATH		0x400 /* return path information */
#define PFTRAV_IS_NO_PART	0x800 /* don't use partitions */


#define PFPK_M_NEAREST		0x0 /* return nearest pick - the default */
#define PFPK_M_ALL		0x800 /* return all hits instead of last one */

/* additional pfHit validity bits in flags */
#define PFHIT_XFORM		0x40 /* transformation is valid */

/* additional pfHit queries for libpf  */
#define PFQHIT_NODE		20
#define PFQHIT_NAME		21
#define PFQHIT_XFORM		22
#define PFQHIT_PATH		23

/* pfNodeBufferMode() */
#define PFN_AUTO_CLONE		0

/* pfNodeTravMode() */
#define PFN_CULL_SORT	0
    #define PFN_CULL_SORT_UNCONTAINED  0
    #define PFN_CULL_SORT_CONTAINED  1

/* NOTE: 1.2 -> 2.0 porting: PFN_BOUND_STATIC/DYNAMIC are 
 * superseded by PFBOUND_STATIC and PFBOUND_DYNAMIC
 */

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfNode CAPI ------------------------------*/

extern DLLEXPORT pfType*              pfGetNodeClassType(void);
extern DLLEXPORT void                 pfNodeTravMask(pfNode* _node, int which, uint mask, int setMode, int bitOp);
extern DLLEXPORT pfNode*              pfFindNode(pfNode* _node, const char *_name, pfType *_type);
extern DLLEXPORT int                  pfNodeName(pfNode* _node, const char *name);
extern DLLEXPORT const char*          pfGetNodeName(const pfNode* _node);
extern DLLEXPORT void                 pfNodeTravFuncs(pfNode* _node, int which, pfNodeTravFuncType pre, pfNodeTravFuncType post);
extern DLLEXPORT void                 pfGetNodeTravFuncs(const pfNode* _node, int which, pfNodeTravFuncType *pre, pfNodeTravFuncType *post);
extern DLLEXPORT void                 pfNodeTravData(pfNode* _node, int which, void *data);
extern DLLEXPORT void*                pfGetNodeTravData(const pfNode* _node, int which);
extern DLLEXPORT unsigned int         pfGetNodeTravMask(const pfNode* _node, int which);
extern DLLEXPORT void                 pfNodeTravMode(pfNode* _node, int which, int mode, int val);
extern DLLEXPORT int                  pfGetNodeTravMode(const pfNode* _node, int which, int mode);
extern DLLEXPORT void                 pfNodeBufferMode(pfNode* _node, int mode, int val);
extern DLLEXPORT int                  pfGetNodeBufferMode(const pfNode* _node, int mode);
extern DLLEXPORT pfGroup*             pfGetParent(const pfNode* _node, int i);
extern DLLEXPORT int                  pfGetNumParents(const pfNode* _node);
extern DLLEXPORT void                 pfNodeBSphere(pfNode* _node, pfSphere *sph, int mode);
extern DLLEXPORT int                  pfGetNodeBSphere(pfNode* _node, pfSphere *sph);
extern DLLEXPORT pfNode*              pfLookupNode(const char *_name, pfType* _type);
extern DLLEXPORT int                  pfNodeIsectSegs(pfNode* _node, pfSegSet *segSet, pfHit **hits[]);
extern DLLEXPORT int                  pfFlatten(pfNode* _node, int _mode);
extern DLLEXPORT pfNode*              pfClone(pfNode* _node, int _mode);
extern DLLEXPORT pfNode*              pfBufferClone(pfNode* _node, int _mode, pfBuffer *buf);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
/* -------------------pfNode Macros ----------------------- */

#if defined(__STDC__) || defined(__cplusplus)
#if !PF_CPLUSPLUS_API
/* base class casting #defines */
#define pfNodeName(_node, _name) 					\
	pfNodeName((pfNode*)_node, _name)
#define pfGetNodeName(_node)						\
	pfGetNodeName((pfNode*)_node)
#define pfClone(_node, mode)						\
	pfClone((pfNode*)_node, mode)
#define pfBufferClone(_node, mode, buf)					\
	pfBufferClone((pfNode*)_node, mode, buf)
#define pfFlatten(_node, mode)						\
	pfFlatten((pfNode*)_node, mode)
#define pfGetParent(_node, _i)						\
	pfGetParent((pfNode*)_node, _i)
#define pfGetNumParents(_node)						\
	pfGetNumParents((pfNode*)_node)
#define pfNodeBSphere(_node, _sph, _type) 				\
	pfNodeBSphere((pfNode*)_node, _sph, _type)
#define pfGetNodeBSphere(_node, _sph)					\
	pfGetNodeBSphere((pfNode*)_node, _sph)
#define pfNodeTravFuncs(_node, _which, _pre, _post)			\
	pfNodeTravFuncs((pfNode*)_node, _which, _pre, _post)
#define pfGetNodeTravFuncs(_node, _which, _pre, _post)			\
	pfGetNodeTravFuncs((pfNode*)_node, _which, _pre, _post)
#define pfNodeTravData(_node, _which, _data)				\
	pfNodeTravData((pfNode*)_node, _which, _data)
#define pfGetNodeTravData(_node, _which) 				\
	pfGetNodeTravData((pfNode*)_node, _which)
#define pfNodeTravMask(_node, _which, _mask, _setMode, _bitOp)		\
	pfNodeTravMask((pfNode*)_node, _which, _mask, _setMode, _bitOp)
#define pfGetNodeTravMask(_node, _which)				\
	pfGetNodeTravMask((pfNode*)_node, _which)
#define pfNodeIsectSegs(_node, _segSet, _isects)			\
	pfNodeIsectSegs((pfNode*)_node, _segSet, _isects)
#define pfNodePickSetup(_node)						\
	pfNodePickSetup((pfNode*)_node)
#define pfFindNode(_node, _name, _type)                                 \
        pfFindNode((pfNode*)_node, _name, _type)
#endif /* !PF_CPLUSPLUS_API */
#endif /* __STDC__ || __cplusplus */
#endif /* !PF_CPLUSPLUS_API */

/*
 * Internal Nodes
 */



#ifdef PF_C_API
/*---------------- pfGroup CAPI ------------------------------*/

extern DLLEXPORT pfGroup*             pfNewGroup(void);
extern DLLEXPORT pfType*              pfGetGroupClassType(void);
extern DLLEXPORT int                  pfAddChild(pfGroup* _group, pfNode *child);
extern DLLEXPORT int                  pfInsertChild(pfGroup* _group, int  index, pfNode *child);
extern DLLEXPORT int                  pfRemoveChild(pfGroup* _group, pfNode *child);
extern DLLEXPORT int                  pfReplaceChild(pfGroup* _group, pfNode *oldn, pfNode *newn);
extern DLLEXPORT int                  pfBufferAddChild(pfGroup* _group, pfNode *child);
extern DLLEXPORT int                  pfBufferRemoveChild(pfGroup* _group, pfNode *child);
extern DLLEXPORT pfNode*              pfGetChild(const pfGroup* _group, int  i);
extern DLLEXPORT int                  pfGetNumChildren(const pfGroup* _group);
extern DLLEXPORT int                  pfSearchChild(const pfGroup* _group, pfNode *n);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
/* ------------------ pfGroup Macros --------------------- */

#if defined(__STDC__) || defined(__cplusplus)
#if !PF_CPLUSPLUS_API
#define pfAddChild(_group, _child)					\
	pfAddChild((pfGroup*)_group, (pfNode*)_child)
#define pfInsertChild(_group, _index, _child)				\
	pfInsertChild((pfGroup*)_group, _index, (pfNode*)_child)
#define pfRemoveChild(_group, _child)					\
	pfRemoveChild((pfGroup*)_group, (pfNode*)_child)
#define pfReplaceChild(_group, _old, _new)				\
	pfReplaceChild((pfGroup*)_group, (pfNode*)_old, (pfNode*)_new)

#define pfBufferAddChild(_group, _child)			\
	pfBufferAddChild((pfGroup*)_group, (pfNode*)_child)
#define pfBufferRemoveChild(_group, _child)			\
	pfBufferRemoveChild((pfGroup*)_group, (pfNode*)_child)

#define	pfGetChild(_group, _index)					\
	pfGetChild((pfGroup*)_group, _index)
#define pfGetNumChildren(_group)					\
	pfGetNumChildren((pfGroup*)_group)

#define pfSearchChild(_group, _child)					\
	pfSearchChild((pfGroup*)_group, (pfNode*)_child)

#endif /* !PF_CPLUSPLUS_API */
#endif /* __STDC__ || __cplusplus */
#endif /* !PF_CPLUSPLUS_API */


#ifdef PF_C_API
/*---------------- pfScene CAPI ------------------------------*/

extern DLLEXPORT pfScene*             pfNewScene(void);
extern DLLEXPORT pfType*              pfGetSceneClassType(void);
extern DLLEXPORT void                 pfSceneGState(pfScene* _scene, pfGeoState *gs);
extern DLLEXPORT pfGeoState*          pfGetSceneGState(const pfScene* _scene);
extern DLLEXPORT void                 pfSceneGStateIndex(pfScene* _scene, int gs);
extern DLLEXPORT int                  pfGetSceneGStateIndex(const pfScene* _scene);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfSCS CAPI ------------------------------*/

extern DLLEXPORT pfSCS*               pfNewSCS(PFMATRIX m);
extern DLLEXPORT pfType*              pfGetSCSClassType(void);
extern DLLEXPORT void                 pfGetSCSMat(pfSCS* _scs, PFMATRIX m);
extern DLLEXPORT const pfMatrix*      pfGetSCSMatPtr(pfSCS* _scs);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfDCS Tokens ----------------------- */

#define PFDCS_MATRIX_TYPE	1
#define PFDCS_UNCONSTRAINED	0xffffffff
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfDCS CAPI ------------------------------*/

extern DLLEXPORT pfDCS*               pfNewDCS(void);
extern DLLEXPORT pfType*              pfGetDCSClassType(void);
extern DLLEXPORT void                 pfGetDCSMat(pfDCS* _dcs, PFMATRIX m);
extern DLLEXPORT const pfMatrix*      pfGetDCSMatPtr(pfDCS* _dcs);
extern DLLEXPORT void                 pfDCSMatType(pfDCS* _dcs, uint val);
extern DLLEXPORT uint                 pfGetDCSMatType(const pfDCS* _dcs);
extern DLLEXPORT void                 pfDCSMat(pfDCS* _dcs, PFMATRIX m);
extern DLLEXPORT void                 pfDCSCoord(pfDCS* _dcs, pfCoord *c);
extern DLLEXPORT void                 pfDCSRot(pfDCS* _dcs, float h, float p, float r);
extern DLLEXPORT void                 pfDCSTrans(pfDCS* _dcs, float x, float y, float z);
extern DLLEXPORT void                 pfDCSScale(pfDCS* _dcs, float s);
extern DLLEXPORT void                 pfDCSScaleXYZ(pfDCS* _dcs, float xs, float ys, float zs);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfFCS Tokens ----------------------- */

#define PFFCS_MATRIX_TYPE	1
#define PFFCS_UNCONSTRAINED	0xffffffff
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfFCS CAPI ------------------------------*/

extern DLLEXPORT pfFCS*               pfNewFCS(pfFlux *_flux);
extern DLLEXPORT pfType*              pfGetFCSClassType(void);
extern DLLEXPORT void                 pfGetFCSMat(pfFCS* _fcs, PFMATRIX m);
extern DLLEXPORT const pfMatrix*      pfGetFCSMatPtr(pfFCS* _fcs);
extern DLLEXPORT void                 pfFCSMatType(pfFCS* _fcs, uint val);
extern DLLEXPORT uint                 pfGetFCSMatType(const pfFCS* _fcs);
extern DLLEXPORT void                 pfFCSFlux(pfFCS* _fcs, pfFlux *_flux);
extern DLLEXPORT pfFlux*              pfGetFCSFlux(pfFCS* _fcs);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfDoubleSCS CAPI ------------------------------*/

extern DLLEXPORT pfDoubleSCS*         pfNewDoubleSCS(PFMATRIX4D m);
extern DLLEXPORT pfType*              pfGetDoubleSCSClassType(void);
extern DLLEXPORT void                 pfGetDoubleSCSMat(pfDoubleSCS* _doublescs, PFMATRIX4D m);
extern DLLEXPORT const pfMatrix4d*    pfGetDoubleSCSMatPtr(pfDoubleSCS* _doublescs);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfDoubleDCS Tokens ----------------------- */

#define PFDOUBLEDCS_MATRIX_TYPE	1
#define PFDOUBLEDCS_UNCONSTRAINED	0xffffffff
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfDoubleDCS CAPI ------------------------------*/

extern DLLEXPORT pfDoubleDCS*         pfNewDoubleDCS(void);
extern DLLEXPORT pfType*              pfGetDoubleDCSClassType(void);
extern DLLEXPORT void                 pfGetDoubleDCSMat(pfDoubleDCS* _doubledcs, PFMATRIX4D m);
extern DLLEXPORT const pfMatrix4d*    pfGetDoubleDCSMatPtr(pfDoubleDCS* _doubledcs);
extern DLLEXPORT void                 pfDoubleDCSMatType(pfDoubleDCS* _doubledcs, uint val);
extern DLLEXPORT uint                 pfGetDoubleDCSMatType(const pfDoubleDCS* _doubledcs);
extern DLLEXPORT void                 pfDoubleDCSMat(pfDoubleDCS* _doubledcs, PFMATRIX4D m);
extern DLLEXPORT void                 pfDoubleDCSCoordd(pfDoubleDCS* _doubledcs, pfCoordd *c);
extern DLLEXPORT void                 pfDoubleDCSRot(pfDoubleDCS* _doubledcs, double h, double p, double r);
extern DLLEXPORT void                 pfDoubleDCSTrans(pfDoubleDCS* _doubledcs, double x, double y, double z);
extern DLLEXPORT void                 pfDoubleDCSScale(pfDoubleDCS* _doubledcs, double s);
extern DLLEXPORT void                 pfDoubleDCSScaleXYZ(pfDoubleDCS* _doubledcs, double xs, double ys, double zs);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfDoubleFCS Tokens ----------------------- */

#define PFDOUBLEFCS_MATRIX_TYPE	1
#define PFDOUBLEFCS_UNCONSTRAINED	0xffffffff
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfDoubleFCS CAPI ------------------------------*/

extern DLLEXPORT pfDoubleFCS*         pfNewDoubleFCS(pfFlux *_flux);
extern DLLEXPORT pfType*              pfGetDoubleFCSClassType(void);
extern DLLEXPORT void                 pfGetDoubleFCSMat(pfDoubleFCS* _doublefcs, PFMATRIX4D m);
extern DLLEXPORT const pfMatrix4d*    pfGetDoubleFCSMatPtr(pfDoubleFCS* _doublefcs);
extern DLLEXPORT void                 pfDoubleFCSMatType(pfDoubleFCS* _doublefcs, uint val);
extern DLLEXPORT uint                 pfGetDoubleFCSMatType(const pfDoubleFCS* _doublefcs);
extern DLLEXPORT void                 pfDoubleFCSFlux(pfDoubleFCS* _doublefcs, pfFlux *_flux);
extern DLLEXPORT pfFlux*              pfGetDoubleFCSFlux(pfDoubleFCS* _doublefcs);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfLODState Tokens ----------------------- */

    /* pfLOD */
#define PFLOD_ALL_TRANSITIONS   -1
    
    /* pfLODStateAttr() */
#define PFLODSTATE_RANGE_RANGEOFFSET 		0
#define PFLODSTATE_RANGE_RANGESCALE		1
#define PFLODSTATE_TRANSITION_RANGEOFFSET	2
#define PFLODSTATE_TRANSITION_RANGESCALE	3
#define PFLODSTATE_RANGE_STRESSOFFSET		4
#define PFLODSTATE_RANGE_STRESSSCALE		5
#define PFLODSTATE_TRANSITION_STRESSOFFSET	6
#define PFLODSTATE_TRANSITION_STRESSSCALE	7
#define PFLODSTATE_RANGE_FOVOFFSET		8
#define PFLODSTATE_RANGE_FOVSCALE		9
#define PFLODSTATE_NUM_PARAMS			10

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfLODState CAPI ------------------------------*/

extern DLLEXPORT pfLODState*          pfNewLODState(void);
extern DLLEXPORT pfType*              pfGetLODStateClassType(void);
extern DLLEXPORT void                 pfLODStateAttr(pfLODState* _lodstate, int attr, float val);
extern DLLEXPORT float                pfGetLODStateAttr(pfLODState* _lodstate, int attr);
extern DLLEXPORT int                  pfLODStateName(pfLODState* _lodstate, const char *name);
extern DLLEXPORT const char*          pfGetLODStateName(const pfLODState* _lodstate);
extern DLLEXPORT pfLODState*          pfFindLODState(const char *findName);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfLOD CAPI ------------------------------*/

extern DLLEXPORT pfLOD*               pfNewLOD(void);
extern DLLEXPORT pfType*              pfGetLODClassType(void);
extern DLLEXPORT void                 pfLODCenter(pfLOD* _lod, PFVEC3 c);
extern DLLEXPORT void                 pfGetLODCenter(const pfLOD* _lod, PFVEC3 c);
extern DLLEXPORT void                 pfLODRange(pfLOD* _lod, int index, float range);
extern DLLEXPORT int                  pfGetLODNumRanges(const pfLOD* _lod);
extern DLLEXPORT float                pfGetLODRange(const pfLOD* _lod, int index);
extern DLLEXPORT void                 pfLODTransition(pfLOD* _lod, int index, float delta);
extern DLLEXPORT int                  pfGetLODNumTransitions(const pfLOD* _lod);
extern DLLEXPORT float                pfGetLODTransition(const pfLOD* _lod, int index);
extern DLLEXPORT void                 pfLODLODState(pfLOD* _lod, pfLODState *ls);
extern DLLEXPORT pfLODState*          pfGetLODLODState(const pfLOD* _lod);
extern DLLEXPORT void                 pfLODLODStateIndex(pfLOD* _lod, int index);
extern DLLEXPORT int                  pfGetLODLODStateIndex(const pfLOD* _lod);
extern DLLEXPORT void                 pfLODRangeFlux(pfLOD* _lod, int index, pfFlux *flux);
extern DLLEXPORT int                  pfGetLODNumRangeFluxes(const pfLOD* _lod);
extern DLLEXPORT pfFlux*              pfGetLODRangeFlux(const pfLOD* _lod, int index);
extern DLLEXPORT void                 pfLODUserEvalFunc(pfLOD* _lod, pfLODEvalFuncType evalFunc);
extern DLLEXPORT pfLODEvalFuncType    pfGetLODUserEvalFunc(pfLOD* _lod);
extern DLLEXPORT float                pfEvaluateLOD(pfLOD* _lod, const pfChannel *chan, const pfMatrix *offset);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
/* ----------------------- pfSwitch Tokens ----------------------- */

/* pfSwitchVal() */
#define PFSWITCH_ON		-1
#define PFSWITCH_OFF		-2
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfSwitch CAPI ------------------------------*/

extern DLLEXPORT pfSwitch*            pfNewSwitch(void);
extern DLLEXPORT pfType*              pfGetSwitchClassType(void);
extern DLLEXPORT int                  pfSwitchVal(pfSwitch* _switch, float val);
extern DLLEXPORT float                pfGetSwitchVal(const pfSwitch* _switch);
extern DLLEXPORT int                  pfSwitchValFlux(pfSwitch* _switch, pfFlux *_valFlux);
extern DLLEXPORT pfFlux*              pfGetSwitchValFlux(const pfSwitch* _switch);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfMorph CAPI ------------------------------*/

extern DLLEXPORT pfMorph*             pfNewMorph(void);
extern DLLEXPORT pfType*              pfGetMorphClassType(void);
extern DLLEXPORT int                  pfMorphAttr(pfMorph* _morph, int  index, int  attr, int  nelts, void *dst, int  nsrcs, float *alist[], ushort *ilist[], int  n[]);
extern DLLEXPORT int                  pfGetMorphNumAttrs(const pfMorph* _morph);
extern DLLEXPORT int                  pfGetMorphSrc(const pfMorph* _morph, int  index, int  src, float **alist, ushort **ilist, int  *n);
extern DLLEXPORT int                  pfGetMorphNumSrcs(const pfMorph* _morph, int  index);
extern DLLEXPORT void*                pfGetMorphDst(const pfMorph* _morph, int  index);
extern DLLEXPORT int                  pfMorphWeights(pfMorph* _morph, int  index, float *weights);
extern DLLEXPORT int                  pfGetMorphWeights(const pfMorph* _morph, int  index, float *weights);
extern DLLEXPORT void                 pfEvaluateMorph(pfMorph* _morph);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
/* ----------------------- pfBuffer Tokens ----------------------- */

/* pfSeqFrame() */
#define PFSEQ_STOP	0
#define PFSEQ_START	1
#define PFSEQ_PAUSE	2
#define PFSEQ_RESUME	3

#define PFSEQ_ALL	-1

/* pfSeqInterval() */ 
#define PFSEQ_CYCLE	0
#define PFSEQ_SWING	1
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfSequence CAPI ------------------------------*/

extern DLLEXPORT pfSequence*          pfNewSeq(void);
extern DLLEXPORT pfType*              pfGetSeqClassType(void);
extern DLLEXPORT void                 pfSeqDuration(pfSequence* _seq, float sp, int nRep);
extern DLLEXPORT void                 pfGetSeqDuration(const pfSequence* _seq, float *sp, int *nRep);
extern DLLEXPORT void                 pfSeqInterval(pfSequence* _seq, int imode, int beg, int e);
extern DLLEXPORT void                 pfGetSeqInterval(const pfSequence* _seq, int *imode, int *beg, int *e);
extern DLLEXPORT void                 pfSeqMode(pfSequence* _seq, int m);
extern DLLEXPORT int                  pfGetSeqMode(const pfSequence* _seq);
extern DLLEXPORT void                 pfSeqTime(pfSequence* _seq, int index, double time);
extern DLLEXPORT double               pfGetSeqTime(const pfSequence* _seq, int index);
extern DLLEXPORT int                  pfGetSeqFrame(const pfSequence* _seq, int *rep);
extern DLLEXPORT void                 pfSeqEvaluation(pfSequence* _seq, int state);
extern DLLEXPORT int                  pfGetSeqEvaluation(pfSequence* _seq);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfLayer CAPI ------------------------------*/

extern DLLEXPORT pfLayer*             pfNewLayer(void);
extern DLLEXPORT pfType*              pfGetLayerClassType(void);
extern DLLEXPORT void                 pfLayerBase(pfLayer* _layer, pfNode *n);
extern DLLEXPORT pfNode*              pfGetLayerBase(const pfLayer* _layer);
extern DLLEXPORT void                 pfLayerDecal(pfLayer* _layer, pfNode *n);
extern DLLEXPORT pfNode*              pfGetLayerDecal(const pfLayer* _layer);
extern DLLEXPORT void                 pfLayerMode(pfLayer* _layer, int mode);
extern DLLEXPORT int                  pfGetLayerMode(const pfLayer* _layer);
extern DLLEXPORT void                 pfLayerPlane(pfLayer* _layer, pfPlane *plane);
extern DLLEXPORT pfPlane*             pfGetLayerPlane(const pfLayer* _layer);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ------------------ pfLayer Macros --------------------- */

#if defined(__STDC__) || defined(__cplusplus)
#if !PF_CPLUSPLUS_API
#define pfLayerBase(_layer, _node)					\
	pfLayerBase(_layer, (pfNode*)_node)
#define pfLayerDecal(_layer, _node)					\
	pfLayerDecal(_layer, (pfNode*)_node)
#endif /* !PF_CPLUSPLUS_API */
#endif /* __STDC__ || __cplusplus */
#endif /* !PF_CPLUSPLUS_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfPartition Tokens ----------------------- */

/* pfPartVal() */
/* for controlling # pfGeoSets/cell in automatic build */
#define PFPART_FINENESS		1 
#define PFPART_DEBUG		2

/* pfPartAttr() for specifing explicit parameters for the partition */
#define PFPART_ORIGIN	  	1
#define PFPART_MIN_SPACING	2
#define PFPART_MAX_SPACING	3


#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfPartition CAPI ------------------------------*/

extern DLLEXPORT pfPartition*         pfNewPart(void);
extern DLLEXPORT pfType*              pfGetPartClassType(void);
extern DLLEXPORT void                 pfPartVal(pfPartition* _part, int which, float val);
extern DLLEXPORT float                pfGetPartVal(pfPartition* _part, int which);
extern DLLEXPORT void                 pfPartAttr(pfPartition* _part, int  which, void *attr);
extern DLLEXPORT void*                pfGetPartAttr(pfPartition* _part, int  which);
extern DLLEXPORT void                 pfBuildPart(pfPartition* _part);
extern DLLEXPORT void                 pfUpdatePart(pfPartition* _part);

#endif /* !PF_C_API */

/*
 * Leaf Nodes 
 */



#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfLightPoint Tokens ----------------------- */

/* pfLPointShape() */
#define PFLP_OMNIDIRECTIONAL	0
#define PFLP_UNIDIRECTIONAL	1
#define PFLP_BIDIRECTIONAL	2

/* pfLPointColor() */
#define PFLP_OVERALL		-1
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfLightPoint CAPI ------------------------------*/

extern DLLEXPORT pfLightPoint*        pfNewLPoint(int n);
extern DLLEXPORT pfType*              pfGetLPointClassType(void);
extern DLLEXPORT int                  pfGetNumLPoints(const pfLightPoint* _lpoint);
extern DLLEXPORT void                 pfLPointSize(pfLightPoint* _lpoint, float s);
extern DLLEXPORT float                pfGetLPointSize(const pfLightPoint* _lpoint);
extern DLLEXPORT void                 pfLPointFogScale(pfLightPoint* _lpoint, float onset, float opaque);
extern DLLEXPORT void                 pfGetLPointFogScale(const pfLightPoint* _lpoint, float *onset, float *opaque);
extern DLLEXPORT void                 pfLPointRot(pfLightPoint* _lpoint, float azim, float elev, float roll);
extern DLLEXPORT void                 pfGetLPointRot(const pfLightPoint* _lpoint, float *azim, float *elev, float *roll);
extern DLLEXPORT void                 pfLPointShape(pfLightPoint* _lpoint, int  dir, float he, float ve, float f);
extern DLLEXPORT void                 pfGetLPointShape(const pfLightPoint* _lpoint, int  *dir, float *he, float *ve, float *f);
extern DLLEXPORT pfGeoSet*            pfGetLPointGSet(const pfLightPoint* _lpoint);
extern DLLEXPORT void                 pfLPointPos(pfLightPoint* _lpoint, int  i, PFVEC3 p);
extern DLLEXPORT void                 pfGetLPointPos(const pfLightPoint* _lpoint, int  i, PFVEC3 p);
extern DLLEXPORT void                 pfLPointColor(pfLightPoint* _lpoint, int  i, PFVEC4 clr);
extern DLLEXPORT void                 pfGetLPointColor(const pfLightPoint* _lpoint, int  i, PFVEC4 clr);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  */

/* pfLSourceMode() */    	
#define PFLS_PROJTEX_ENABLE		0
#define	PFLS_SHADOW_ENABLE		1
#define	PFLS_FOG_ENABLE			2
#define	PFLS_FREEZE_SHADOWS		3

/* pfLSourceVal() */    	
#define PFLS_SHADOW_DISPLACE_SCALE	10	
#define PFLS_SHADOW_DISPLACE_OFFSET	11
#define PFLS_SHADOW_SIZE		12	
#define PFLS_INTENSITY			13

/* pfLSourceAttr() */
#define PFLS_SHADOW_TEX			20
#define PFLS_PROJ_TEX			21
#define PFLS_PROJ_FOG			22
#define PFLS_PROJ_FRUST			23

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfLightSource CAPI ------------------------------*/

extern DLLEXPORT pfLightSource*       pfNewLSource(void);
extern DLLEXPORT pfType*              pfGetLSourceClassType(void);
extern DLLEXPORT void                 pfLSourceColor(pfLightSource* _lsource, int _which, float _r, float _g, float _b);
extern DLLEXPORT void                 pfGetLSourceColor(pfLightSource* _lsource, int _which, float* _r, float* _g, float* _b);
extern DLLEXPORT void                 pfLSourceAmbient(pfLightSource* _lsource, float _r, float _g, float _b);
extern DLLEXPORT void                 pfGetLSourceAmbient(pfLightSource* _lsource, float* _r, float* _g, float* _b);
extern DLLEXPORT void                 pfLSourcePos(pfLightSource* _lsource, float _x, float _y, float _z, float _w);
extern DLLEXPORT void                 pfGetLSourcePos(pfLightSource* _lsource, float* _x, float* _y, float* _z, float* _w);
extern DLLEXPORT void                 pfLSourceAtten(pfLightSource* _lsource, float _a0, float _a1, float _a2);
extern DLLEXPORT void                 pfGetLSourceAtten(pfLightSource* _lsource, float* _a0, float* _a1, float* _a2);
extern DLLEXPORT void                 pfSpotLSourceCutoffDelta(pfLightSource* _lsource, float _f1);
extern DLLEXPORT float                pfGetSpotLSourceCutoffDelta(pfLightSource* _lsource);
extern DLLEXPORT void                 pfSpotLSourceDir(pfLightSource* _lsource, float _x, float _y, float _z);
extern DLLEXPORT void                 pfGetSpotLSourceDir(pfLightSource* _lsource, float* _x, float* _y, float* _z);
extern DLLEXPORT void                 pfSpotLSourceCone(pfLightSource* _lsource, float _f1, float _f2);
extern DLLEXPORT void                 pfGetSpotLSourceCone(pfLightSource* _lsource, float* _f1, float* _f2);
extern DLLEXPORT void                 pfLSourceOn(pfLightSource* _lsource);
extern DLLEXPORT void                 pfLSourceOff(pfLightSource* _lsource);
extern DLLEXPORT int                  pfIsLSourceOn(pfLightSource* _lsource);
extern DLLEXPORT void                 pfLSourceMode(pfLightSource* _lsource, int mode, int val);
extern DLLEXPORT int                  pfGetLSourceMode(const pfLightSource* _lsource, int mode);
extern DLLEXPORT void                 pfLSourceVal(pfLightSource* _lsource, int mode, float val);
extern DLLEXPORT float                pfGetLSourceVal(const pfLightSource* _lsource, int mode);
extern DLLEXPORT void                 pfLSourceAttr(pfLightSource* _lsource, int attr, void *obj);
extern DLLEXPORT void*                pfGetLSourceAttr(const pfLightSource* _lsource, int attr);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfGeode CAPI ------------------------------*/

extern DLLEXPORT pfGeode*             pfNewGeode(void);
extern DLLEXPORT pfType*              pfGetGeodeClassType(void);
extern DLLEXPORT int                  pfAddGSet(pfGeode* _geode, pfGeoSet *gset);
extern DLLEXPORT int                  pfInsertGSet(pfGeode* _geode, int index, pfGeoSet *gset);
extern DLLEXPORT int                  pfReplaceGSet(pfGeode* _geode, pfGeoSet *oldgs, pfGeoSet *newgs);
extern DLLEXPORT int                  pfRemoveGSet(pfGeode* _geode, pfGeoSet *gset);
extern DLLEXPORT int                  pfCountShadedGSets(pfGeode* _geode);
extern DLLEXPORT pfGeoSet*            pfGetGSet(const pfGeode* _geode, int i);
extern DLLEXPORT int                  pfGetNumGSets(const pfGeode* _geode);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
/* ------------------ pfGeode Macros --------------------- */

#if defined(__STDC__) || defined(__cplusplus)
#if !PF_CPLUSPLUS_API
#define pfAddGSet(_geode, _gset)					\
	pfAddGSet((pfGeode*)_geode, _gset)
#define pfInsertGSet(_geode, _index, _gset)				\
	pfInsertGSet((pfGeode*)_geode, _index, _gset)
#define pfReplaceGSet(_geode, _old, _new)				\
	pfReplaceGSet((pfGeode*)_geode, _old, _new)
#define pfRemoveGSet(_geode, _gset)					\
	pfRemoveGSet((pfGeode*)_geode, _gset)
#define pfGetGSet(_geode, _index)					\
	pfGetGSet((pfGeode*)_geode, _index)
#define pfGetNumGSets(_geode)						\
	pfGetNumGSets((pfGeode*)_geode)
#endif /* !PF_CPLUSPLUS_API */
#endif /* __STDC__ || __cplusplus */
#endif /* !PF_CPLUSPLUS_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

#define PFASD_MAXCHANNEL	10
#define PFASD_MAXCHANNELID	100
#define PFASD_MAXLOD		50

/* evalMethod */
#define PFASD_DIST		0x1
#define PFASD_DIST_SQUARE	0x2
#define PFASD_CALLBACK		0x3

/* setMode Tokens */
#define PFASD_TRAV_VERTEX  	0x1

/* setMode Values */
#define PFASD_NO_MORPH		0x1
#define PFASD_C_MORPH		0x0

#define PFASD_FACE_NODRAW	0x1
#define PFASD_FACE_NOQUERY	0x2

/* setAttr Tokens */
/* _attr */
#define PFASD_LODS		0x01
#define PFASD_COORDS		0x02
#define PFASD_NORMALS		0x04
#define PFASD_COLORS		0x08
#define PFASD_TCOORDS		0x10
#define PFASD_FACES		0x20
#define PFASD_OVERALL_ATTR	0x40
#define PFASD_PER_VERTEX_ATTR	0x80
#define PFASD_NO_ATTR		0

/* setDrawCode Tokens */
#define PFASD_BBOX		0x4
#define PFASD_STATS		0x8

#define PFASD_NIL_ID		-1

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfASD CAPI ------------------------------*/

extern DLLEXPORT pfASD*               pfNewASD(void);
extern DLLEXPORT pfType*              pfGetASDClassType(void);
extern DLLEXPORT void                 pfASDAttr(pfASD* _asd, int _which, int _type, int _size, void *_attr);
extern DLLEXPORT void                 pfGetASDAttr(pfASD* _asd, int _which, int *_type, int *_size, void **_attr);
extern DLLEXPORT void                 pfASDMorphAttrs(pfASD* _asd, int _mc);
extern DLLEXPORT int                  pfGetASDMorphAttrs(pfASD* _asd);
extern DLLEXPORT void                 pfGetASDActiveGeom(pfASD* _asd, pfChannel *_chan, pfList *_geom);
extern DLLEXPORT void                 pfASDNumBaseFaces(pfASD* _asd, int num);
extern DLLEXPORT int                  pfGetASDNumBaseFaces(pfASD* _asd);
extern DLLEXPORT void                 pfASDGStates(pfASD* _asd, pfGeoState **gs, int num);
extern DLLEXPORT void                 pfGetASDGStates(pfASD* _asd, pfGeoState ***gs, int *num);
extern DLLEXPORT pfGeoState*          pfGetASDGState(pfASD* _asd, int num);
extern DLLEXPORT int                  pfGetASDNumGStates(pfASD* _asd);
extern DLLEXPORT void                 pfASDSyncGroup(pfASD* _asd, uint _syncGroup);
extern DLLEXPORT uint                 pfGetASDSyncGroup(pfASD* _asd);
extern DLLEXPORT void                 pfASDEnableClipRings(pfASD* _asd);
extern DLLEXPORT void                 pfASDNumClipRings(pfASD* _asd, int _numrings);
extern DLLEXPORT void                 pfASDClipRings(pfASD* _asd, float *_rings);
extern DLLEXPORT float*               pfGetASDClipRings(pfASD* _asd);
extern DLLEXPORT int                  pfGetASDNumClipRings(pfASD* _asd);
extern DLLEXPORT void                 pfASDFaceBBoxes(pfASD* _asd, pfBox *_box);
extern DLLEXPORT pfBox*               pfGetASDFaceBBoxes(pfASD* _asd);
extern DLLEXPORT void                 pfASDFaceBBox(pfASD* _asd, pfBox *_facebbox, int _faceid);
extern DLLEXPORT void                 pfGetASDFaceBBox(pfASD* _asd, pfBox *_facebbox, int _faceid);
extern DLLEXPORT void                 pfASDBBox(pfASD* _asd, pfBox *_box);
extern DLLEXPORT void                 pfGetASDBBox(pfASD* _asd, pfBox *_box);
extern DLLEXPORT void                 pfASDConfig(pfASD* _asd);
extern DLLEXPORT void                 pfASDMaxMorphDepth(pfASD* _asd, int _m, float __morphweightconstraint);
extern DLLEXPORT void                 pfGetASDMaxMorphDepth(pfASD* _asd, int *_m, float *__morphweightconstraint);
extern DLLEXPORT void                 pfASDLODState(pfASD* _asd, pfLODState *ls);
extern DLLEXPORT pfLODState*          pfGetASDLODState(pfASD* _asd);
extern DLLEXPORT void                 pfASDLODStateIndex(pfASD* _asd, int _lsi);
extern DLLEXPORT int                  pfGetASDLODStateIndex(pfASD* _asd);
extern DLLEXPORT void                 pfASDEvalMethod(pfASD* _asd, int method);
extern DLLEXPORT int                  pfGetASDEvalMethod(pfASD* _asd);
extern DLLEXPORT void                 pfASDEvalFunc(pfASD* _asd, pfASDEvalFuncType _eval);
extern DLLEXPORT pfASDEvalFuncType    pfGetASDEvalFunc(pfASD* _asd);
extern DLLEXPORT void                 pfASDMask(pfASD* _asd, uint _which, uint _mask, int _id);
extern DLLEXPORT void                 pfASDCullEnlarge(pfASD* _asd, float fov, float nearPlane, float farPlane);
extern DLLEXPORT void                 pfASDMorphWeight(pfASD* _asd, int _vertid, float _morphweight);
extern DLLEXPORT void                 pfASDUnsetMorphWeight(pfASD* _asd, int _vertid);
extern DLLEXPORT void                 pfASDInitMask(pfASD* _asd, uint _which);
extern DLLEXPORT void                 pfASDClearAllMasks(pfASD* _asd, uint _which);
extern DLLEXPORT void                 pfASDCalcVirtualClipTexParamsFunc(pfASD* _asd, pfASDCalcVirtualClipTexParamsFuncType _func);
extern DLLEXPORT pfASDCalcVirtualClipTexParamsFuncType pfGetASDCalcVirtualClipTexParamsFunc(pfASD* _asd);
extern DLLEXPORT int                  pfASDIsPaging(pfASD* _asd);
extern DLLEXPORT int                  pfASDIsPageMaster(pfASD* _asd);
extern DLLEXPORT void                 pfASDInitPaging(pfASD* _asd);
extern DLLEXPORT void                 pfASDTileSize(pfASD* _asd, float **_tsize);
extern DLLEXPORT float**              pfGetASDTileSize(pfASD* _asd);
extern DLLEXPORT void                 pfASDPageSize(pfASD* _asd, short **_page);
extern DLLEXPORT short**              pfGetASDPageSize(pfASD* _asd);
extern DLLEXPORT void                 pfASDTotalTiles(pfASD* _asd, short **_tilenum);
extern DLLEXPORT short**              pfGetASDTotalTiles(pfASD* _asd);
extern DLLEXPORT void                 pfASDMaxTileMemSize(pfASD* _asd, int _tilefaces, int _tileverts);
extern DLLEXPORT void                 pfGetASDMaxTileMemSize(pfASD* _asd, int *_tilefaces, int *_tileverts);
extern DLLEXPORT void                 pfASDOrigin(pfASD* _asd, pfVec3 *_min);
extern DLLEXPORT pfVec3*              pfGetASDOrigin(pfASD* _asd);
extern DLLEXPORT void                 pfASDPageFname(pfASD* _asd, char *_fname);
extern DLLEXPORT char*                pfGetASDPageFname(pfASD* _asd);
extern DLLEXPORT void                 pfASDOverrideViewingParams(pfASD* _asd, pfChannel *chan, PFMATRIX M);
extern DLLEXPORT int                  pfASDAddQueryArray(pfASD* _asd, float *_vertices, float *_down, int nofVertices, uint _mask, pfFlux *_results);
extern DLLEXPORT void                 pfASDDeleteQueryArray(pfASD* _asd, int _index);
extern DLLEXPORT void                 pfASDQueryArrayElement(pfASD* _asd, int _arrayIndex, int _elementIndex, float *_vertex, float *_down);
extern DLLEXPORT unsigned int         pfASDContainsQueryArray(pfASD* _asd, float *_vertices, float *_down, int _nofVertices);
extern DLLEXPORT int                  pfASDAddQueryTriangles(pfASD* _asd, float *_v, float *_t, float *_c, int _nofTriangles, float *_base, float *_down, float *_projection, float *_azimuth, unsigned int _opcode, uint _mask, pfFlux *_results);
extern DLLEXPORT void                 pfASDDeleteQueryGeoSet(pfASD* _asd, int _index);
extern DLLEXPORT int                  pfASDAddQueryGeoSet(pfASD* _asd, pfGeoSet *gset, float *_down, uint _mask, pfFlux *_results);
extern DLLEXPORT void                 pfASDReplaceQueryGeoSet(pfASD* _asd, int index, pfGeoSet *gset, float *_down);
extern DLLEXPORT void                 pfASDProjectPointFinestPositionNormal(pfASD* _asd, float *_base, float *_down, unsigned int _flags, float *_base_pos, float *_base_normal);
extern DLLEXPORT void                 pfASDProjectPointFinestPosition(pfASD* _asd, float *_base, float *_down, unsigned int _flags, float *_base_pos);
extern DLLEXPORT void                 pfASDGetQueryArrayPositionSpan(pfASD* _asd, int _index, pfBox *_box);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfText CAPI ------------------------------*/

extern DLLEXPORT pfText*              pfNewText(void);
extern DLLEXPORT pfType*              pfGetTextClassType(void);
extern DLLEXPORT int                  pfAddString(pfText* _text, pfString *str);
extern DLLEXPORT int                  pfInsertString(pfText* _text, int index, pfString *str);
extern DLLEXPORT int                  pfReplaceString(pfText* _text, pfString *oldgs, pfString *newgs);
extern DLLEXPORT int                  pfRemoveString(pfText* _text, pfString *str);
extern DLLEXPORT pfString*            pfGetString(const pfText* _text, int i);
extern DLLEXPORT int                  pfGetNumStrings(const pfText* _text);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfBillboard Tokens ----------------------- */

/* Modes */
#define PFBB_ROT		PFSPRITE_ROT

/* Rotation values */
#define PFBB_AXIAL_ROT		PFSPRITE_AXIAL_ROT		
#define PFBB_POINT_ROT_EYE	PFSPRITE_POINT_ROT_EYE
#define PFBB_POINT_ROT_WORLD	PFSPRITE_POINT_ROT_WORLD		
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfBillboard CAPI ------------------------------*/

extern DLLEXPORT pfBillboard*         pfNewBboard(void);
extern DLLEXPORT pfType*              pfGetBboardClassType(void);
extern DLLEXPORT void                 pfBboardAxis(pfBillboard* _bboard, const PFVEC3 axis);
extern DLLEXPORT void                 pfGetBboardAxis(pfBillboard* _bboard, PFVEC3 axis);
extern DLLEXPORT void                 pfBboardMode(pfBillboard* _bboard, int  _mode, int  _val);
extern DLLEXPORT int                  pfGetBboardMode(pfBillboard* _bboard, int  _mode);
extern DLLEXPORT void                 pfBboardPos(pfBillboard* _bboard, int  i, const PFVEC3 pos);
extern DLLEXPORT void                 pfGetBboardPos(pfBillboard* _bboard, int  i, PFVEC3 pos);
extern DLLEXPORT void                 pfBboardPosFlux(pfBillboard* _bboard, pfFlux *_flux);
extern DLLEXPORT pfFlux*              pfGetBboardPosFlux(pfBillboard* _bboard);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
#define PFIBRN_USE_PROXY           (1<<0) /* use proxy, not a billboard */
#define PFIBRN_VARY_PROXY_GEOSETS  (1<<1) /* one group of geosets per view 
					     (only for proxy) */
#define PFIBRN_TEXCOORDS_DEFINED   (1<<2) /* set automatically when setProxyTexCoords() is called */
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfIBRnode CAPI ------------------------------*/

extern DLLEXPORT pfIBRnode*           pfNewIBRnode(void);
extern DLLEXPORT pfType*              pfGetIBRnodeClassType(void);
extern DLLEXPORT void                 pfIBRnodeIBRtexture(pfIBRnode* _ibrnode, pfIBRtexture *tex);
extern DLLEXPORT pfIBRtexture*        pfGetIBRnodeIBRtexture(pfIBRnode* _ibrnode);
extern DLLEXPORT void                 pfIBRnodeAngles(pfIBRnode* _ibrnode, int i, float horAngle, float verAngle);
extern DLLEXPORT void                 pfGetIBRnodeAngles(pfIBRnode* _ibrnode, int i, float *horAngle, float *verAngle);
extern DLLEXPORT int                  pfGetIBRnodeNumAngles(pfIBRnode* _ibrnode);
extern DLLEXPORT void                 pfIBRnodeProxyTexCoords(pfIBRnode* _ibrnode, pfVec2 ***texCoords);
extern DLLEXPORT pfVec2***            pfGetIBRnodeProxyTexCoords(pfIBRnode* _ibrnode);
extern DLLEXPORT void                 pfIBRnodeFlags(pfIBRnode* _ibrnode, int which, int value);
extern DLLEXPORT int                  pfGetIBRnodeFlags(const pfIBRnode* _ibrnode, int which);

#endif /* !PF_C_API */

/******************** End Nodes ****************************/



#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfPath Tokens ----------------------- */

/* modes for APP traversal */
#define PFTRAV_SW_CUR		0x0000000
#define PFTRAV_SW_ALL		0x0001000
#define PFTRAV_SW_NONE		0x0002000
#define PFTRAV_SW_MASK     	0x0003000

#define PFTRAV_SEQ_CUR		0x0000000
#define PFTRAV_SEQ_ALL		0x0004000
#define PFTRAV_SEQ_NONE     	0x0008000
#define PFTRAV_SEQ_MASK     	0x000c000

#define PFTRAV_LOD_CUR		0x0000000
#define PFTRAV_LOD_RANGE0	0x0010000
#define PFTRAV_LOD_ALL		0x0020000
#define PFTRAV_LOD_NONE		0x0040000
#define PFTRAV_LOD_MASK     	0x0070000

#define PFTRAV_LAYER_BOTH	0x0000000
#define PFTRAV_LAYER_NONE	0x0100000
#define PFTRAV_LAYER_BASE	0x0200000
#define PFTRAV_LAYER_DECAL	0x0400000
#define PFTRAV_LAYER_MASK     	0x0700000

/* pfCullPath() */
#define PFPATH_IGNORE_SWITCHES	0x0
#define PFPATH_EVAL_LOD		0x1
#define PFPATH_EVAL_SWITCH	0x2
#define PFPATH_EVAL_SEQUENCE	0x4
#define PFPATH_EVAL_SWITCHES	(PFPATH_EVAL_LOD | PFPATH_EVAL_SWITCH | \
				  PFPATH_EVAL_SEQUENCE)

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfPath CAPI ------------------------------*/

extern DLLEXPORT pfPath*              pfNewPath(void);
extern DLLEXPORT pfType*              pfGetPathClassType(void);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ------------------ pfTraverser Related Functions--------------------- */

extern DLLEXPORT void	pfCullResult(int _result);
extern DLLEXPORT int	pfGetParentCullResult(void);
extern DLLEXPORT int	pfGetCullResult(void);
extern DLLEXPORT int	pfCullPath(pfPath *_path,  pfNode *_root, int _mode);

#define pfCullPath(_path, _root, _mode) \
	pfCullPath(_path, (pfNode*)_root, _mode)

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfTraverser CAPI ------------------------------*/

extern DLLEXPORT pfChannel*           pfGetTravChan(const pfTraverser* _trav);
extern DLLEXPORT pfNode*              pfGetTravNode(const pfTraverser* _trav);
extern DLLEXPORT void                 pfGetTravMat(const pfTraverser* _trav, PFMATRIX mat);
extern DLLEXPORT void                 pfGetTravMatD(const pfTraverser* _trav, PFMATRIX4D mat);
extern DLLEXPORT int                  pfTravIsSinglePrecision(pfTraverser* _trav);
extern DLLEXPORT int                  pfGetTravIndex(const pfTraverser* _trav);
extern DLLEXPORT const pfPath*        pfGetTravPath(const pfTraverser* _trav);

#endif /* !PF_C_API */

#endif /* PF_C_API */

/************************************************************/


DLLEXPORT extern pfBuffer	*_pfCurrentBuffer;
extern pfBuffer 		*_pfAppBuffer;



#ifdef __cplusplus
}
#endif

#endif /* !__PF_H__ */
