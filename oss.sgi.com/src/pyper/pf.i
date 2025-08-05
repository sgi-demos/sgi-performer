
// This file contains the public interface from the Performer
// header file Performer/pf.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%section "libpf"

%{
#include <Performer/pf.h>
%}

class pfBuffer;
//pfObjects
class pfPath;
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
class pfLOD;
class pfLayer;
class pfSwitch;
class pfMorph;
class pfSequence;
//pfGeodes
class pfGeode;
class pfASD;
class pfBillboard;
//pfTraversers
class pfTraverser;

/* pfChanTravMode() pfNodeTravMask/Func() */
#define PFTRAV_ISECT            0
#define PFTRAV_APP              1
#define PFTRAV_CULL             2
#define PFTRAV_DRAW             3
#define PFTRAV_LPOINT           4
#define PFTRAV_COMPUTE          5

/* pfMultiprocess() */
#define PFMP_FORK_ISECT		0x01
#define PFMP_FORK_CULL		0x02
#define PFMP_FORK_DRAW		0x04
#define PFMP_FORK_DBASE		0x08
#define PFMP_FORK_LPOINT       	0x10  
#define PFMP_FORK_COMPUTE      	0x20  
#define PFMP_CULLoDRAW          0x10000  /* 65536 */
#define PFMP_CULL_DL_DRAW       0x20000  /* 131072 */

#define PFMP_DEFAULT            -1
#define PFMP_APPCULLDRAW        0
#define PFMP_APPCULL_DL_DRAW    (PFMP_CULL_DL_DRAW)
#define PFMP_APPCULL_DRAW       (PFMP_FORK_DRAW)
#define PFMP_APP_CULLDRAW       (PFMP_FORK_CULL)
#define PFMP_APP_CULL_DL_DRAW   (PFMP_FORK_CULL | PFMP_CULL_DL_DRAW) /* 131074 */
#define PFMP_APP_CULL_DRAW      (PFMP_FORK_CULL | PFMP_FORK_DRAW)  /* 6 */
#define PFMP_APPCULLoDRAW       (PFMP_FORK_DRAW | PFMP_CULLoDRAW) /* 65540 */
#define PFMP_APP_CULLoDRAW      (PFMP_FORK_CULL | PFMP_FORK_DRAW | PFMP_CULLoDRAW) /* 65542 */

extern int	pfInit(void);
extern int	pfIsInited(void);
extern void	pfExit(void);
extern int	pfMultipipe(int _numPipes);
extern int	pfGetMultipipe(void);
extern int	pfHyperpipe(int _numHyperPipes);
extern int	pfGetHyperpipe(pfPipe *_pipe);
extern int	pfMultiprocess(int _mpMode);
extern int	pfGetMultiprocess(void);
extern int	pfGetMPBitmask(void);
extern int	pfMultithread(int _pipe, uint _stage, int _nprocs);
extern int      pfGetMultithread(int _pipe, uint _stage);

extern int	pfConfig(void);
extern int	pfIsConfiged(void);
extern pid_t	pfGetPID(int _pipe, uint _stage);
extern uint	pfGetStage(pid_t _pid, int *_pipe);
extern void	pfStageConfigFunc(int _pipe, uint _stage, pfStageFuncType _configFunc);
extern pfStageFuncType pfGetStageConfigFunc(int _pipe, uint _stage);
extern int 	pfConfigStage(int _pipe, uint _stage);
extern const char * pfGetStageName(int pipe, uint _stage);
extern const char * pfGetPIDName(pid_t pid);

extern void	pfCreateProcessFunc(pfCreateProcessFuncType func);
extern pfCreateProcessFuncType pfGetCreateProcessFunc(void);
extern void	pfProcessMiscCPU(int cpu);
extern int	pfGetProcessMiscCPU(void);
extern void	pfPrintProcessState(FILE *fp);

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

extern void	pfAppFrame(void);
extern int	pfSync(void);
extern int	pfFrame(void);
extern void	pfApp(void);
extern void	pfCull(void);
extern void	pfDraw(void);
extern void     pfDrawBin(int bin);
extern void     pfDrawScene(void);
extern void	pfLPoint(void);
extern void	pfCompute(void);


extern void	pfIsectFunc(pfIsectFuncType _func);
extern pfIsectFuncType pfGetIsectFunc(void);
extern void*	pfAllocIsectData(int _bytes);
extern void*	pfGetIsectData(void);
extern void	pfPassIsectData(void);

extern void	pfDBase(void);
extern void	pfDBaseFunc(pfDBaseFuncType _func);
extern pfDBaseFuncType pfGetDBaseFunc(void);
extern void*	pfAllocDBaseData(int _bytes);
extern void*	pfGetDBaseData(void);
extern void	pfPassDBaseData(void);

extern void	pfComputeFunc(pfComputeFuncType _func);
extern pfComputeFuncType pfGetComputeFunc(void);
extern void*	pfAllocComputeData(int _bytes);
extern void*	pfGetComputeData(void);
extern void	pfPassComputeData(void);

extern int 	pfGetLastPipeFrameCount(int _pipe);
extern int 	pfGetPipeDrawCount(int _pipe);
extern int 	pfGetPipeNum(void);

extern void	pfPhase(int _phase);
extern int	pfGetPhase(void);
extern void	pfVideoRate(float _vrate);
extern float	pfGetVideoRate(void);
extern float	pfFrameRate(float _rate);
extern float	pfGetFrameRate(void);
extern int	pfFieldRate(int _fields);
extern int	pfGetFieldRate(void);
extern int	pfGetFrameCount(void);

extern double	pfGetFrameTimeStamp(void);
extern void	pfFrameTimeStamp(double t);

extern pfFlux*	pfGetFrameTimeFlux(void);

extern int	pfGetId(void *_mem);
extern int	pfAsyncDelete(void *_mem);
extern int	pfCopy(void *_dst, void *_src);






















