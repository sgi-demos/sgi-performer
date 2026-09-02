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
 * pr.h	  Include file for Peformer performance rendering library.
 *
 * $Revision: 1.649 $
 * $Date: 2002/12/08 23:02:33 $
 *
 */

#ifndef __PR_H__
#define __PR_H__


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

/* enable 1.2 compatibility #defines */
/* #define PF_COMPAT_1_2 */
/* enable 2.0 compatibility #defines */
/* #define PF_COMPAT_2_0 */

/* Performer version constants */
#define PF_MAJOR_VERSION        3
#define PF_MINOR_VERSION        0
#define PF_MAINT_VERSION        0


#include <sys/types.h>		/* unsigned int, unsigned int */
#include <stdio.h>		/* __file_s for ulocks.h */


#include <ulocks.h>



#include <errno.h>		/* for PF_US wrappers */


/* 
 * Registry entries on Windows can be found under the
 * HKEY_LOCAL_MACHINE using the following subkey:
 *
 */
#define PF_REGISTRY_SUBKEY "SOFTWARE\\SGI\\OpenGL Performer"

#include <time.h>


#include <Performer/pfDLL.h>

#ifdef __ia64__
#define GL_GLEXT_PROTOTYPES 1
#endif

#ifdef __linux__
#define GL_GLEXT_LEGACY 1
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <GL/gl.h> /* GL tokens */
#include <GL/glu.h>
#include "opengl.h"		/* GL based pf tokens */

#ifdef  __linux__
#ifndef __ia64__
#define __linux__has_MP__           1
#endif
#define __linux__has_sysmp_h__      1
#define __linux__wants_MwmUtil_h__  1
#endif /* __linux__ */

#define PRINT_DEBUG 1

#ifdef PRINT_DEBUG
#ifdef __linux__
#include "linuxstubs.h"
#endif
#define STUB
#define DEBUGSTUB(x)
#define ERRORSTUB(x)
#endif


/* typedef of X-based Performer Types */
#ifdef IRIX5
typedef	int XSGIvcChannelInfo;
#endif
#if (!defined(__linux__has_XSGIvc__) && defined(__linux__)) || defined(WIN32)
typedef int XSGIvcChannelInfo;
#endif

typedef	XSGIvcChannelInfo *pfWSVideoChannelInfo;
typedef int                    VisualID;
typedef int                    pfFBConfig; /* pfFBConfig will merely be a pixelformat ID */
typedef void*                  pfWSConnection;
typedef HANDLE                 pfWSDrawable; /* handle to window or pbuffer */
typedef HWND                   Window; 
typedef HWND                   pfWSWindow;



#include <math.h>
#define sinf(m) sin((float)m)
#define cosf(m) cos((float)m)
#define sqrtf(m) sqrt((float)m)
#define expf(m) exp((float)m)
#define tanf(m) tan((float)m)
#define floorf(m) floor((float)m)
#define atan2f(m,n) atan2((float)m,(float)n)
#define asinf(m) asin((float)m)
#define acosf(m) acos((float)m)
#ifdef __linux__
#include <GL/glx.h>
#include <stdint.h>
typedef GLXContext	pfGLContext;
typedef void*		pfGLXFBConfig;
typedef void            USTMSCpair;
#define fsqrt(m)    sqrtf(m)
#define fexp(m)     expf(m)
#define fsin(m)     sinf(m)
#define fcos(m)     cosf(m)
#define ftan(m)     tanf(m)
#define ffloor(m)   floorf(m)
#define PR_SALL     0
#define SIG_PF      __sighandler_t
#else

typedef HGLRC pfGLContext;


#endif


#if (!defined(__linux__has_MP__) && defined(__linux__)) || defined(WIN32)  /* SP Linux or WIN32 */

extern DLLEXPORT void*	  _pfSharedArena;
extern DLLEXPORT size_t	  _pfSharedArenaSize;

#define PF_ULOCK_T			int*
#define PF_PID_T			int
#define PF_USEMA_T			int
#define PF_USPTR_T			void

#define PF_USNEWLOCK(arena) 		new int
#define PF_USFREELOCK(lock, arena) 	delete lock
#define PF_USNEWSEMA(sema, arena, val)	do { sema = new int;} while (0)
#define PF_USFREESEMA(sema, arena)	delete sema

#define PF_USTESTSEMA(sema)		_pfussemastub((PF_USEMA_T*)sema)
#define PF_USPSEMA(sema)		_pfussemastub((PF_USEMA_T*)sema)
#define PF_USVSEMA(sema)		_pfussemastub((PF_USEMA_T*)sema)
#define PF_USSETLOCK(lock) 		_pfuslockstub(lock)
#define PF_USCSETLOCK(lock, spins)	_pfuslockstub(lock)
#define PF_USWSETLOCK(lock, spins)	_pfuslockstub(lock)
#define PF_USUNSETLOCK(lock) 		_pfuslockstub(lock)
#define PF_USTESTLOCK(lock) 		_pfuslockstub(lock)
#define PF_USINIT(name)			new int
#define PF_USDETACH(arena)		delete arena

extern int _pfussemastub(PF_USEMA_T *sema);
extern int _pfuslockstub(PF_ULOCK_T lock);

#else /* MP Linux & IRIX */

extern void*	  _pfSharedArena;
extern size_t	  _pfSharedArenaSize;

#define PF_ULOCK_T			ulock_t
#define PF_PID_T			pid_t
#define PF_USEMA_T			usema_t
#define PF_USPTR_T			usptr_t

#define PF_USNEWSEMA(sema, arena, val)	do {		\
	if((sema = usnewsema(arena, val)) == NULL)	\
	    pfNotify(PFNFY_FATAL, PFNFY_INTERNAL, 	\
		     "usnewsema() errno = %d", errno); 	\
	} while (0)
#define PF_USPSEMA(sema) do {				\
	if(uspsema(sema) < 0)				\
	    pfNotify(PFNFY_FATAL, PFNFY_INTERNAL, 	\
		     "PF_USPSEMA() errno = %d", errno); \
	} while (0)
#define PF_USVSEMA(sema) do {				\
	if(usvsema(sema) < 0)				\
	    pfNotify(PFNFY_FATAL, PFNFY_INTERNAL, 	\
		     "PF_USVSEMA() errno = %d", errno); \
	} while (0)

#define PF_USTESTSEMA(sema)		ustestsema(sema)
#define PF_USFREESEMA(sema, arena)	usfreesema(sema, arena)
#define PF_USNEWLOCK(arena) 		usnewlock(arena)
#define PF_USFREELOCK(lock, arena) 	usfreelock(lock, arena)
#define PF_USSETLOCK(lock) 		pfGentle_ussetlock(lock, 1000)
#define PF_USCSETLOCK(lock, spins)	uscsetlock(lock, spins)
#define PF_USWSETLOCK(lock, spins)	pfGentle_ussetlock(lock, spins)
#define PF_USUNSETLOCK(lock) 		usunsetlock(lock)
#define PF_USTESTLOCK(lock) 		ustestlock(lock)
#define PF_USINIT(name)			usinit(name)
#define PF_USDETACH(arena)		usdetach(arena)

#endif /* __linux__ */

#if defined(__linux__) || defined(WIN32)
#define PRDA_SIZE       32
extern void* PRDA[PRDA_SIZE];

#ifdef __cplusplus
extern "C" {
#endif
extern DLLEXPORT void* amalloc(size_t size, void*);
extern DLLEXPORT void* arealloc(void* ptr, size_t size, void*);
extern DLLEXPORT void  afree(void* ptr, void*);
extern DLLEXPORT long  sginap(long);
extern int    mpin (void*, int);
extern int    munpin (void*, int);
extern DLLEXPORT int    oserror();
extern int   _ushlockdefspin;
extern void   blockproc(int pid);
extern void   unblockproc(int pid);
extern int   m_get_myid();
extern unsigned int set_fpc_csr(unsigned int);
extern unsigned int get_fpc_csr(void);
extern int   sproc (void (*)(void *), int, void *);
typedef void (sprocFunc)(void*,size_t);
extern int sprocsp(sprocFunc,int,void*,caddr_t,size_t);
extern void  m_sync(void);
extern void  glPolygonOffsetEXT( GLfloat factor, GLfloat bias );
extern unsigned long test_then_add(unsigned long *stuff, unsigned long junk);
extern DLLEXPORT uint32_t test_then_add32(uint32_t *, uint32_t);
extern DLLEXPORT uint32_t test_and_set32(uint32_t *, uint32_t);
extern DLLEXPORT uint32_t add_then_test32(uint32_t *, uint32_t);
#ifdef __cplusplus
}
#endif
#endif




#ifndef FALSE
#define FALSE 		0
#endif

#ifndef TRUE
#define TRUE 		1
#endif

#define PF_OFF		0
#define PF_ON		1

/* Texture coordinates */    
#define PF_S		0
#define PF_T		1
#define PF_R		2
#define PF_Q		3

/* Coordinate spaces */
#define PF_EYESPACE 0
#define PF_WORLDSPACE 1
#define PF_OBJSPACE 2

#define PF_MAXSTRING	256

/*-------------------------- set/unset bitmasks ------------------------*/

#define PFFLAG_SET(flag, mask)	    ((flag) |= (mask))
#define PFFLAG_UNSET(flag, mask)    ((flag) &= ~(mask))
#define PFFLAG_BOOL_SET(flag, mask, val) \
	    (val ? PFFLAG_SET(flag, mask) : PFFLAG_UNSET(flag, mask))
#define PFFLAG_BOOL_GET(flag, mask) (((flag) & (mask)) ? 1 : 0)

/*------------------------------ classes --------------------------------*/

#if PF_CPLUSPLUS_API 
class	pfType;
class	pfObject;
class	pfMemory;
class	pfCycleMemory;
class	pfCycleBuffer;
class	pfFluxMemory;
class	pfFlux;
class	pfEngine;
class	pfList;
class 	pfImageTile;
class	pfQueue;

/* graphics */
class	pfImageCache;
class   pfClipTexture;
class	pfDispList;
class	pfGeoSet;
class	pfGeoState;
class	pfColortable;
class	pfFont;
class	pfMaterial;
class	pfLight;
class	pfLightModel;
class	pfTexture;
class	pfTexEnv;
class	pfTexLoad;
class	pfFog;
class	pfHighlight;
class	pfLPointState;
class	pfTexGen;
class	pfTexLOD;
class	pfTextureValidMap;
class	pfSprite;
class	pfState;
class	pfStats;
class	pfString;
class	pfVideoChannel;
class	pfWindow;
class   pfCalligraphic;
class   pfFBState;
class   pfCombiner;
class   pfNameSpace;
/* utility */
class	pfDataPool;
class	pfFile;
class   pfISLTexCoordData;
class   pfByteBank;
class   pfGSetBank;
#else
typedef struct _pfObject	pfObject;
typedef struct _pfMemory	pfMemory;
typedef struct _pfCycleMemory	pfCycleMemory;
typedef struct _pfCycleBuffer	pfCycleBuffer;
typedef struct _pfFluxMemory	pfFluxMemory;
typedef struct _pfFlux		pfFlux;
typedef struct _pfEngine	pfEngine;
typedef struct _pfList          pfList;
typedef struct _pfImageTile     pfImageTile;
typedef struct _pfQueue		pfQueue;

/* graphics */
typedef struct _pfImageCache	pfImageCache;
typedef struct _pfClipTexture	pfClipTexture;
typedef struct _pfDispList	pfDispList;
typedef struct _pfFont		pfFont;
typedef struct _pfGeoSet	pfGeoSet;
typedef struct _pfGeoState	pfGeoState;
typedef struct _pfColortable 	pfColortable;
typedef struct _pfMaterial	pfMaterial;
typedef struct _pfLight		pfLight;
typedef struct _pfLightModel	pfLightModel;
typedef struct _pfTexture	pfTexture;
typedef struct _pfTexEnv	pfTexEnv;
typedef struct _pfTexLoad	pfTexLoad;
typedef struct _pfFog		pfFog;
typedef struct _pfHighlight 	pfHighlight;
typedef struct _pfLPointState	pfLPointState;
typedef struct _pfTexGen	pfTexGen;
typedef struct _pfTexLOD	pfTexLOD;
typedef struct _pfTextureValidMap	pfTextureValidMap;
typedef struct _pfSprite	pfSprite;
typedef struct _pfState		pfState;
typedef struct _pfStats		pfStats;
typedef struct _pfString	pfString;
typedef struct _pfVideoChannel	pfVideoChannel;
typedef struct _pfWindow	pfWindow;
typedef struct _pfCalligraphic  pfCalligraphic;
typedef struct _pfFBState       pfFBState;
typedef struct _pfCombiner	pfCombiner;
typedef struct _pfNameSpace                 pfNameSpace;

/* utility */
typedef struct _pfDataPool	pfDataPool;
typedef struct _pfFile          pfFile;
typedef struct _pfISLTexCoordData pfISLTexCoordData;
typedef struct _pfByteBank pfByteBank;
typedef struct _pfGSetBank pfGSetBank;

#endif /* PF_CPLUSPLUS_API */

#include <Performer/prmath.h>
#include <Performer/prstats.h>

/* ------------------ pfCalligraphic typedefs  ----------- */

typedef enum {
    pfXSlewQuality0 = 0,
    pfXSlewQuality1 = 1,
    pfXSlewQuality2 = 2,
    pfYSlewQuality0 = 3,
    pfYSlewQuality1 = 4,
    pfYSlewQuality2 = 5,
    pfDefocusQuality0 = 6,
    pfDefocusQuality1 = 7
} pfCalligSlewTableEnum;

typedef float pfCalligSlewTable[256][256];

typedef enum {
    pfRedGammaTable = 0,
    pfGreenGammaTable = 1,
    pfBlueGammaTable = 2
} pfCalligGammaTableEnum;

typedef float pfCalligGammaTable[1024];

#ifdef __LPB_H__
/* LPB_info is defined in the driver lpb.h that is already included */
#else
typedef struct _LPB_info LPB_info;
#define __LPB_H__
#endif


/* ------------------ pfObject Types --------------------- */

typedef void (*pfCopyFuncType)(pfObject* , const pfObject* );
typedef void (*pfCompareFuncType)(const pfObject*, const pfObject *);
typedef void (*pfDeleteFuncType)(pfObject* );
typedef void (*pfMergeFuncType)(pfObject* );
typedef void (*pfPrintFuncType)(const pfObject*, uint, uint, char*, FILE*);
typedef int  (*pfRefFuncType)(void *);

/* -------------------- pfImageCache Types ----------------- */

/* Texture subload cost table definition */
typedef struct {
    int    wid;
    int    ht;
    int    alignS;
    int    alignT;
    int    alignR;
    float  *cost1BPT; /* 2d tables */
    float  *cost2BPT;
    float  *cost3BPT;
    float  *cost4BPT;
} pfTexSubloadCostTable;

typedef void (*pfImageCacheReadQueueFuncType)(pfImageTile *itile);
typedef void (*pfTileFileNameFuncType)(pfImageTile *tile);

/* -------------------- pfImageTile Types ----------------- */

typedef int (*pfReadImageTileFuncType)(pfImageTile *itile, int nTexels);
typedef int (*pfValidateImageTileFuncType)(pfImageTile *itile);

/* -------------------- pfQueue Types -------------------------- */

typedef struct {
    pfList *sortList;
    volatile int *inSize;
    int inLo, inHi;
    volatile int *outSize;
    int outLo, outHi;
} pfQueueSortFuncData;

typedef void (*pfQueueSortFuncType)(pfQueueSortFuncData *data);
typedef int (*pfQueueServiceFuncType)(void *data);

/* ------------------- pfClipTexture Types --------------------- */

typedef struct pfVirtualClipTexLimits
{
    struct {int lo, hi;} LODOffset, numEffectiveLevels;
    struct {float lo, hi;} minLOD, maxLOD;
} pfVirtualClipTexLimits;

/* -------------------- pfDispList Types ----------------------- */

typedef void (*pfDListFuncType)(void *_data);

/* -------------------- pfGeoState Types ----------------------- */

typedef int (*pfGStateFuncType)(pfGeoState *_gs, void *_userData);

/* -------------------- pfEngine Types ------------------------- */

typedef void (*pfEngineFuncType)(pfEngine *_engine);

/* -------------------- pfFlux Types ------------------------- */

typedef int (*pfFluxInitFuncType)(pfFluxMemory *_fmem);
typedef void (*pfFluxDataFuncType)(void *_data, void *_funcData);
typedef void (*pfFluxUserCopyFuncType)(pfFlux *flux, void *dst, void *src);

/* -------------------- pfLPointState Types -------------------- */
typedef struct {
	pfLPointState 	*lpstate;  /* Read Only LPState */
	pfGeoSet	*geoset;   /* Read Only GeoSet */
	void 		*userData; /* Provided when setting the callback */
	float		*sizes;	   /* Write Only - resulting sizes */
	float		*alphas;   /* Write Only - resulting alphas */
} pfRasterData;

typedef void (*pfRasterFuncType)(pfRasterData *rasterData);

typedef struct {
	pfLPointState 	*lpstate;  /* Read Only LPState */
	pfGeoSet	*geoset;   /* Read Only GeoSet */
	void 		*userData; /* Provided when setting the callback */
	unsigned short	*index;	   /* Read Write - index of visible lpoints */
	int		*n;	   /* Read Write - number of visible lpoints */
	pfVec3		*coords2D; /* Read Write - screen space X,Y,Z */
	float		*intensity;/* Write Only - resulting intensity */
	float		**focus;   /* Write Only - optional per lpoint (de)focus */
	float		**drawTime;/* Write Only - optional per lpoint drawTime */
} pfCalligData;

typedef void (*pfCalligFuncType)(pfCalligData *calligData);
/* --------------------------------------------------------------------------------
   OpenGL Shader / Performer integration 
   --------------------------------------------------------------------------------
*/

#ifdef LOCALBUILD
/*
  Shaders are currently unimplemented on
  WIN32,
  linux, 
  IRIX O32
  IRIX N64
*/
#endif

#ifdef __linux__
#if (__GNUC__ >= 3)
#define __GCC3__ 1
#else
#define _PF_NO_SHADER_ 1
#endif
#endif


#if defined(_PF_NO_SHADER_) || !defined(__cplusplus) || defined(WIN32) || ((!defined(__linux__)) && (_MIPS_SIM == _MIPS_SIM_ABI32 || _MIPS_SIM == _MIPS_SIM_ABI64) || (__GNUC__ < 3))

#define _PF_NO_SHADER_
typedef void islAppearance;
typedef void islShader;
typedef void islCompileAction;
typedef void islAppearanceSnapshotData;
typedef void islSnapshotAction;
typedef void islCopyAction;
typedef void islAppearanceCopyData;
typedef void islShape;
typedef void islDrawAction;
#else 
#ifndef STL_USE_SGI_ALLOCATORS 
#define STL_USE_SGI_ALLOCATORS
#endif
#ifndef STL_SGI_THREADS
#define STL_SGI_THREADS
#endif

/* For some reason, STL on linux is strange and requires that 
   std::size_t and std::ptrdiff_t are defined. These fail to be
   defined properly, so nothing compiles */

#if defined(__linux__) 

#ifdef __GCC3__
namespace std 
{
  using ::ptrdiff_t;
  using ::size_t;
}
#endif /* __GCC3__ */
#endif /*__linux__ */
#ifndef _PF_NO_SHADER_
#include <shader/isl.h>
#endif /* shader */
#endif

/* --------------------------------------------------------------------------------
   OpenGL Shader / Performer integration 
   --------------------------------------------------------------------------------
*/

typedef int (*pfTexApplyCallbackType)(const islAppearance *app,
				      const char *texName,
				      void *userData);

typedef void (*pfTexComputeCallbackType)(pfISLTexCoordData *d,
					 const char *texName,
					 void *userData);


typedef pfVec2* (*pfTexCoordCallbackType)(pfISLTexCoordData *d,
					  const float texCoordID,
					  void *userData);

/* Function to specify texture coordinate generation callbacks */
DLLEXPORT void pfShaderTexCoordFunc(islAppearance *a, 
				    pfTexCoordCallbackType func, 
				    void *udata);
/* Function to specify texture apply callbacks */
DLLEXPORT void pfShaderTexApplyFunc(islAppearance *a, 
				    pfTexApplyCallbackType func, 
				    void *udata);
/* Function to specify texture compute callbacks */
DLLEXPORT void pfShaderTexComputeFunc(islAppearance *a, 
				      pfTexComputeCallbackType func, 
				      void *udata);

/* Function for telling performer that an appearance has been modified */
DLLEXPORT void pfAppearanceChanged(islAppearance *appearance); 

/* Function for telling performer that a shader within an appearance has changed */
DLLEXPORT void pfShaderChanged(islAppearance *appearance, islShader *shader);

/* 
   These are two utility functions which assume texture names are filenames
   and try to load the textures from files. The compute function will load the
   texture in the cull and the apply action will apply the texture in the draw
*/
DLLEXPORT void pfDefaultTextureComputeCallback(pfISLTexCoordData *d,
					  const char *texName,
					  void *userData);

DLLEXPORT int pfDefaultTextureApplyCallback(const islAppearance *app,
					     const char *texName,
					     void *userData);


#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUGLIBS
#ifdef __ANSI_CPP__
#define PFASSERTDEBUG(_exp)  ((_exp)?((void)0):pfNotify(PFNFY_FATAL, PFNFY_INTERNAL, "Assertion failed: %s in %s at line %d", # _exp , __FILE__, __LINE__))
#else
#define PFASSERTDEBUG(_exp)  ((_exp)?((void)0):pfNotify(PFNFY_FATAL, PFNFY_INTERNAL, "Assertion failed: %s in %s at line %d", "_exp" , __FILE__, __LINE__))
#endif
#define PFDPRINT1(a) fprintf(stderr, a)
#define PFDPRINT2(a,b) fprintf(stderr, a, b)
#define PFDPRINT3(a,b,c) fprintf(stderr, a, b, c)
#else
#define PFASSERTDEBUG(a)
#define PFDPRINT1(a)
#define PFDPRINT2(a,b)
#define PFDPRINT3(a,b,c)
#endif /* DEBUGLIBS */

#ifdef __ANSI_CPP__
#define PFASSERTALWAYS(_exp)  ((_exp)?((void)0):pfNotify(PFNFY_FATAL, PFNFY_INTERNAL, "Assertion failed: %s in %s at line %d", # _exp , __FILE__, __LINE__))
#else
#define PFASSERTALWAYS(_exp)  ((_exp)?((void)0):pfNotify(PFNFY_FATAL, PFNFY_INTERNAL, "Assertion failed: %s in %s at line %d", "_exp" , __FILE__, __LINE__))
#endif



/* pfQSys() - GL Type Constants */
#define PFGL_IRISGL		0
#define PFGL_OPENGL		1
#define PFGL_NOGL		2

/*---------------------- Initialization Routines ---------------*/

/*
 * full libpr initialization including shared memory arena and classes 
 */

extern DLLEXPORT void	prInit(void);
extern DLLEXPORT int	prIsInited(void);
extern DLLEXPORT void	prExit(void);

#ifndef __PF_H__
/* 
 * Macros for libpr -> libpf inheritance.
 */
#define	pfInit		prInit
#define	pfIsInited	prIsInited
#define	pfExit		prExit
#endif	/* __PF_H__ */

/*------------------------ Shared Memory ----------------------*/
 
/* XXX - Arenas not ported to linux yet. */
extern DLLEXPORT int		pfInitArenas(void);
extern DLLEXPORT int		pfFreeArenas(void);

extern DLLEXPORT PF_USPTR_T* 	pfGetSemaArena(void);
extern DLLEXPORT void 		pfSemaArenaSize(size_t size);
extern DLLEXPORT size_t		pfGetSemaArenaSize(void);
extern DLLEXPORT void		pfSemaArenaBase(void *base);
extern DLLEXPORT void*		pfGetSemaArenaBase(void);

extern DLLEXPORT void*    	pfGetSharedArena(void);
extern DLLEXPORT void 		pfSharedArenaSize(size_t size);
extern DLLEXPORT size_t		pfGetSharedArenaSize(void);
extern DLLEXPORT void		pfSharedArenaBase(void *base);
extern DLLEXPORT void*		pfGetSharedArenaBase(void);

extern DLLEXPORT void	        pfTmpDir(char *dir);
extern const DLLEXPORT char *	pfGetTmpDir(void);

/* --------------------------- pfObject --------------------------------*/

/* 
 * pfPrint() 
 * verbosity levels for printing -- XXX should we just use notify tokens ??? 
*/
#define PFPRINT_VB_OFF	    0	/* no printing */
#define PFPRINT_VB_ON	    1	/* default - minimal printing */
#define PFPRINT_VB_NOTICE   1	/* default - minimal printing */
#define PFPRINT_VB_INFO     2	/* basic info */
#define PFPRINT_VB_DEBUG    3	/* prints everything */

/* -------------------------------- pfState -----------------------------*/

/* pfOverride() Modes, pfGStateMode(), pfGStateInherit() */
#define PFSTATE_ENLIGHTING    	             0x1
#define PFSTATE_ENTEXTURE     	             0x4 
#define PFSTATE_TRANSPARENCY  	             0x10
#define PFSTATE_ALPHAFUNC	             0x20
#define PFSTATE_ENFOG	    	             0x40
#define PFSTATE_ANTIALIAS	             0x100
#define PFSTATE_CULLFACE	             0x200
#define PFSTATE_ENCOLORTABLE   	             0x400
#define PFSTATE_DECAL	    	             0x4000
#define PFSTATE_SHADEMODEL	             0x8000
#define PFSTATE_ENWIREFRAME	             0x10000
#define PFSTATE_ENHIGHLIGHTING	             0x100000
#define PFSTATE_ENLPOINTSTATE	             0x400000
#define PFSTATE_ENTEXGEN	             0x1000000
#define PFSTATE_ENTEXMAT	             0x4000000
#define PFSTATE_ENTEXLOD	             0x10000000
#define PFSTATE_ENCOMBINER               0x200000000

/* pfGStateVal()  */
#define PFSTATE_ALPHAREF	             0x40000

/* pfOverride() Attributes, pfGStateAttr(), pfGStateInherit()  */
#define PFSTATE_FRONTMTL 	             0x2 
#define PFSTATE_TEXTURE       	             0x8 
#define PFSTATE_TEXENV	    	             0x80
#define PFSTATE_COLORTABLE    	             0x800 
#define PFSTATE_BACKMTL       	             0x1000
#define PFSTATE_FOG	    	             0x2000
#define PFSTATE_LIGHTMODEL	             0x20000
#define PFSTATE_LIGHTS	    	             0x80000
#define PFSTATE_HIGHLIGHT	             0x200000
#define PFSTATE_LPOINTSTATE	             0x800000
#define PFSTATE_TEXGEN		             0x2000000
#define PFSTATE_TEXMAT		             0x8000000
#define PFSTATE_TEXLOD		             0x20000000
#define PFSTATE_DECALPLANE	             0x40000000
#define PFSTATE_COMBINER                 0x400000000

#define PFSTATE_ALL		\
      (	PFSTATE_ENLIGHTING    	| \
	PFSTATE_ENTEXTURE     	| \
	PFSTATE_TRANSPARENCY  	| \
	PFSTATE_ALPHAFUNC	| \
	PFSTATE_ENFOG	    	| \
	PFSTATE_ANTIALIAS	| \
	PFSTATE_CULLFACE	| \
	PFSTATE_ENCOLORTABLE   	| \
	PFSTATE_DECAL	    	| \
	PFSTATE_SHADEMODEL	| \
	PFSTATE_ENWIREFRAME	| \
	PFSTATE_ENHIGHLIGHTING	| \
	PFSTATE_ENLPOINTSTATE	| \
	PFSTATE_ENTEXGEN	| \
	PFSTATE_ENTEXLOD	| \
	PFSTATE_ENTEXMAT	| \
	PFSTATE_ALPHAREF	| \
	PFSTATE_FRONTMTL 	| \
	PFSTATE_TEXTURE       	| \
	PFSTATE_TEXENV	    	| \
	PFSTATE_COLORTABLE    	| \
	PFSTATE_BACKMTL       	| \
	PFSTATE_FOG	    	| \
	PFSTATE_LIGHTMODEL	| \
	PFSTATE_LIGHTS	    	| \
	PFSTATE_HIGHLIGHT	| \
	PFSTATE_LPOINTSTATE	| \
	PFSTATE_TEXGEN 		| \
	PFSTATE_TEXLOD 		| \
	PFSTATE_TEXMAT 		| \
	PFSTATE_DECALPLANE	| \
	PFSTATE_ENCOMBINER	| \
	PFSTATE_COMBINER)

#if defined(__linux__) || defined(WIN32)
  /*
    Performer supports up to 4 texture units on linux systems.
  */
#define	PF_MAX_TEXTURES   4
#else
  /*
    On machines without the headers, we should only define structures for one
    texture, otherwise the memory bandwidth hit hurts performance.
  */
#define PF_MAX_TEXTURES   1
#endif

/*
 * ------------------------------------------------------------------------
 *   Alignment supplier (e.g. terrain) constants and callback definitions
 */
#define PR_QUERY_POSITION       0x0001
#define PR_QUERY_NORMAL         0x0002
#define PR_QUERY_COLOR          0x0004
#define PR_QUERY_Z              0x0008
#define PR_QUERY_FRAME          0x0010
#define PR_QUERY_RECORD_FRAME   0x0020
#define PR_QUERY_TRI_TEXTURE    0x0040
#define PR_QUERY_TRI_COLOR      0x0080
#define PR_QUERY_TRI_NORMAL     0x0100
#define PR_QUERY_TRI_COORD      0x0200

#define PR_ALIGN_TRANSLATE              0x0001
#define PR_ALIGN_AXIS_BILLBOARD         0x0002
#define PR_ALIGN_NORMAL                 0x0004
#define PR_ALIGN_AZIMUTH                0x0008

#define PR_QUERY_DIRECTIONAL            0x0001


/*---------------------------------- Draw Modes ----------------------------*/


/* pfTransparency() */
#define PFTR_OFF	    	0
#define PFTR_ON	    		1
#define PFTR_FAST    		2
#define PFTR_HIGH_QUALITY  	3
#define PFTR_BLEND_ALPHA	4
#define PFTR_MS_ALPHA		5
#define PFTR_MS_ALPHA_MASK	6

#define PFTR_NO_OCCLUDE		0x100

/* pfAntialias() */
#define PFAA_OFF	    	0
#define PFAA_ON	    	    	1
#define PFAA_MSPOINT		2
#define PFAA_MSAREA		3
#define PFAA_MSCENTER_POINT	4
#define PFAA_SMOOTH		5
#define PFAA_LINE_SMOOTH	6
#define PFAA_POINT_SMOOTH	7

/* pfDecal() */
#define PFDECAL_OFF	    		0
#define PFDECAL_LAYER	    		2
#define PFDECAL_BASE	    		3
#define PFDECAL_BASE_FAST  		4
#define PFDECAL_BASE_HIGH_QUALITY   	5
#define PFDECAL_BASE_STENCIL		6
#define PFDECAL_BASE_DISPLACE		7
#define PFDECAL_LAYER_FAST  		8
#define PFDECAL_LAYER_HIGH_QUALITY   	9
#define PFDECAL_LAYER_STENCIL		10
#define PFDECAL_LAYER_DISPLACE		11
#define PFDECAL_LAYER_DISPLACE_AWAY	12
#define PFDECAL_MODE_MASK		0x000f
#define PFDECAL_PLANE			0x0010
#define PFDECAL_ENABLE_MASK		0x001f
#define PFDECAL_LAYER_0			0x0000
#define PFDECAL_LAYER_1			0x0100
#define PFDECAL_LAYER_2			0x0200
#define PFDECAL_LAYER_3			0x0300
#define PFDECAL_LAYER_4			0x0400
#define PFDECAL_LAYER_5			0x0500
#define PFDECAL_LAYER_6			0x0600
#define PFDECAL_LAYER_7			0x0700
#define PFDECAL_LAYER_MASK		0xff00
#define PFDECAL_LAYER_SHIFT		8
#define PFDECAL_LAYER_OFFSET		0x10000

/* pfEnable()/pfDisable() */
#define PFEN_LIGHTING	    	2

#define PFEN_TEXTURE	    	3
#define PFEN_TEXTURE_0	    	PFEN_TEXTURE
#define PFEN_TEXTURE_1	    	(PFEN_TEXTURE_0 + 1)
#define PFEN_TEXTURE_2	    	(PFEN_TEXTURE_1 + 1)
#define PFEN_TEXTURE_3	    	(PFEN_TEXTURE_2 + 1)

#define PFEN_FOG	    	(PFEN_TEXTURE_3 + 1)
#define PFEN_WIREFRAME		(PFEN_FOG + 1)
#define PFEN_COLORTABLE		(PFEN_WIREFRAME + 1)
#define PFEN_HIGHLIGHTING	(PFEN_COLORTABLE + 1)

#define PFEN_TEXGEN		(PFEN_HIGHLIGHTING + 1)
#define PFEN_TEXGEN_0		PFEN_TEXGEN
#define PFEN_TEXGEN_1		(PFEN_TEXGEN_0 + 1)
#define PFEN_TEXGEN_2		(PFEN_TEXGEN_1 + 1)
#define PFEN_TEXGEN_3		(PFEN_TEXGEN_2 + 1)

#define PFEN_LPOINTSTATE	(PFEN_TEXGEN_3 + 1)

#define PFEN_TEXMAT		(PFEN_LPOINTSTATE + 1)
#define PFEN_TEXMAT_0		PFEN_TEXMAT
#define PFEN_TEXMAT_1		(PFEN_TEXMAT_0 + 1)
#define PFEN_TEXMAT_2		(PFEN_TEXMAT_1 + 1)
#define PFEN_TEXMAT_3		(PFEN_TEXMAT_2 + 1)

#define PFEN_TEXLOD		(PFEN_TEXMAT_3 + 1)
#define PFEN_TEXLOD_0		PFEN_TEXLOD
#define PFEN_TEXLOD_1		(PFEN_TEXLOD_0 + 1)
#define PFEN_TEXLOD_2		(PFEN_TEXLOD_1 + 1)
#define PFEN_TEXLOD_3		(PFEN_TEXLOD_2 + 1)

#define PFEN_TEXTURE_NOW        (PFEN_TEXLOD_3 +1)
#define PFEN_TEXTURE_0_NOW      PFEN_TEXTURE_NOW
#define PFEN_TEXTURE_1_NOW      (PFEN_TEXTURE_0_NOW +1)
#define PFEN_TEXTURE_2_NOW      (PFEN_TEXTURE_1_NOW +1)
#define PFEN_TEXTURE_3_NOW      (PFEN_TEXTURE_2_NOW +1)

#define PFEN_COMBINER		(PFEN_TEXTURE_3_NOW + 1)

/* pfClear() */
#define PFCL_COLOR		0x01
#define PFCL_DEPTH		0x02
#define PFCL_MSDEPTH		0x04
#define PFCL_STENCIL		0x08
#define PFCL_DITHER		0x10

/* pfGLOverride() */
#define PFGL_OFF                    0

#define PFGL_TRANSPARENCY           10
#define PFGL_TRANSP_MSALPHA         11
#define PFGL_TRANSP_BLEND           12

#define PFGL_DECAL                  20
#define PFGL_DECAL_STENCIL          21
#define PFGL_DECAL_DISPLACE         22
#define PFGL_DECAL_PLANE            23


extern DLLEXPORT void	pfShadeModel(int _model);
extern DLLEXPORT int	pfGetShadeModel(void);
extern DLLEXPORT void	pfTransparency(int _type);
extern DLLEXPORT int	pfGetTransparency(void);
extern DLLEXPORT void	pfAlphaFunc(float _ref, int _func);
extern DLLEXPORT void	pfGetAlphaFunc(float* _ref, int* _func);
extern DLLEXPORT void	pfAntialias(int _type);
extern DLLEXPORT int	pfGetAntialias(void);
extern DLLEXPORT void	pfDecal(int _mode);
extern DLLEXPORT void	pfApplyDecalPlane(pfPlane *_plane);
extern DLLEXPORT pfPlane	*pfGetCurDecalPlane(void);
extern DLLEXPORT int	pfGetDecal(void);
extern DLLEXPORT void	pfCullFace(int _cull);
extern DLLEXPORT int	pfGetCullFace(void);
extern DLLEXPORT void 	pfApplyTMat(pfMatrix *mat);
extern DLLEXPORT void	pfEnable(uint64_t _target);
extern DLLEXPORT void	pfDisable(uint64_t _target);
extern DLLEXPORT int	pfGetEnable(int _target);
#if !PF_CPLUSPLUS_API
extern DLLEXPORT void 	pfClear(int _which, const pfVec4 _col);
#else
extern DLLEXPORT void 	pfClear(int _which, const pfVec4 *_col);
#endif /* !PF_CPLUSPLUS_API */
extern DLLEXPORT void     pfGLOverride(int _mode, float _val);
extern DLLEXPORT float    pfGetGLOverride(int _mode);

/*------------------------ GL Matrix Stack ----------------------------*/

extern DLLEXPORT void	pfScale(float _x, float _y, float _z);
extern DLLEXPORT void	pfTranslate(float _x, float _y, float _z);
extern DLLEXPORT void	pfRotate(int _axis, float _degrees);
extern DLLEXPORT void	pfPushMatrix(void);
extern DLLEXPORT void	pfPushIdentMatrix(void);
extern DLLEXPORT void	pfPopMatrix(void);
extern DLLEXPORT void	pfLoadMatrix(const PFMATRIX _m);
extern DLLEXPORT void	pfMultMatrix(const PFMATRIX _m);

/*------------------------ Precision modes ----------------------------*/

enum pfPrecisionMode
{
    PF_PRECISION_SS=0,
    PF_PRECISION_SD=1,
    PF_PRECISION_DS=2,
    PF_PRECISION_DD=3
};

/*---------------------------- Collaboration ------------------------*/

extern void	pfCollabEnque(int , ...);

/*---------------------------- Notification -------------------------*/

/* pfNotify() severity */
#define PFNFY_ALWAYS		0
#define PFNFY_FATAL		1
#define PFNFY_WARN		2
#define PFNFY_NOTICE		3
#define PFNFY_INFO		4
#define PFNFY_DEBUG		5
#define PFNFY_FP_DEBUG		6
#define PFNFY_INTERNAL_DEBUG	7
#define PFNFY_LEVEL_MAX		7

/* pfNotify()  error */
#define PFNFY_USAGE		1
#define PFNFY_RESOURCE		2
#define PFNFY_SYSERR		3
#define PFNFY_ASSERT		4
#define PFNFY_PRINT		5
#define PFNFY_INTERNAL		6
#define PFNFY_FP_OVERFLOW	7
#define PFNFY_FP_DIVZERO	8
#define PFNFY_FP_INVALID	9
#define PFNFY_FP_UNDERFLOW	10
#define PFNFY_MORE		-1	/* Continuation of the previous err */

typedef struct
{
    int        severity;
    int        pferrno;
    char       *emsg;
} pfNotifyData;

typedef void (*pfNotifyFuncType)(pfNotifyData *);

extern DLLEXPORT void	pfNotifyHandler(pfNotifyFuncType _handler);
extern DLLEXPORT pfNotifyFuncType pfGetNotifyHandler(void);
extern DLLEXPORT void	pfDefaultNotifyHandler(pfNotifyData  *notice);
extern DLLEXPORT void	pfNotifyLock(void);
extern DLLEXPORT void	pfNotifyUnlock(void);
extern DLLEXPORT void	pfNotifyLevel(int _severity);
extern DLLEXPORT int	pfGetNotifyLevel(void);
/* PRINTFLIKE3 */
extern DLLEXPORT void	pfNotify(int _severity, int _error, char *_format, ...);
extern DLLEXPORT int	_pf_readonly_notifyLevel;
#define		PFNOTIFY(sev, args) ((sev) <= _pf_readonly_notifyLevel ? \
			 pfNotify args : (void)0)

/*------------------- Isection and Traversal Tokens ---------------- */

/* return values of intersection routines pfSegsIsect<*>(), pf<*>Isect<*> */
#define PFIS_FALSE	    0x000
#define PFIS_MAYBE	    0x001 /* MAYBE bit is always set when PFIS_TRUE is */
#define PFIS_TRUE	    0x002 /* TRUE  bit it always set when all_in  */
#define PFIS_ALL_IN	    0x004
#define PFIS_START_IN       0x008
#define PFIS_END_IN	    0x010
#define PFIS_TRIANGLE       0x020

/* isect modes */
#define PFTRAV_IS_PRIM		0x01 /* report isects at poly level */
#define PFTRAV_IS_GSET		0x02 /* report isects at geosets */
#define PFTRAV_IS_STRING	0x04 /* use the bounding cylinder */
#define PFTRAV_IS_NORM		0x08 /* return normal */
#define PFTRAV_IS_CULL_BACK	0x10 /* ignore backfacing polygons */
#define PFTRAV_IS_CULL_FRONT	0x20 /* ignore frontfacing polygons */
#define PFTRAV_IS_BCYL		0x40 /* use the bounding cylinder */

/* return values for all traversal callbacks, including intersection */
#define PFTRAV_CONT		0
#define PFTRAV_PRUNE		1
#define PFTRAV_TERM		2
#define PFTRAV_MASK		0x000f

/* additional traversal return values specific to isect discriminators */
#define PFTRAV_IS_IGNORE	0x0100 /* ignore this intersection */
#define PFTRAV_IS_ALL_SEGS	0x0200 /* prune/term applies to all segs */
#define PFTRAV_IS_CLIP_START	0x0400 /* prune/cont with start clipped seg */
#define PFTRAV_IS_CLIP_END	0x0800 /* prune/cont with end clipped seg */
    
/* set ops pf<*>IsectMask() */
#define PF_SET			1
#define PF_OR			2
#define PF_AND			3

/* traversal/set modes pfGSetIsectMask() */
#define PFTRAV_SELF		0x010
#define PFTRAV_DESCEND		0x020
#define PFTRAV_SET_FROM_CHILD	0x040 /* set by combining mask with child masks */
#define PFTRAV_IS_CACHE		0x080 /* enable isection caching within geosets */
#define PFTRAV_IS_UNCACHE	0x100 /* disable isection caching within geosets */

/*------------------------------ Clock Routines ---------------------------*/

extern DLLEXPORT double		pfGetTime(void);
extern DLLEXPORT pid_t		pfInitClock(double _time);
extern DLLEXPORT void		pfWrapClock(void);
extern DLLEXPORT void		pfClockName(char *_name);
extern DLLEXPORT const char*	pfGetClockName(void);

#define	PFCLOCK_DEFAULT   0
#define PFCLOCK_APPWRAP   1
#define	PFCLOCK_FORKWRAP  2
#define	PFCLOCK_NOWRAP    3
extern DLLEXPORT void		pfClockMode(int _mode);
extern DLLEXPORT int		pfGetClockMode(void);

/*----------------------------------- File Paths --------------------------------*/

extern DLLEXPORT void 		pfFilePath(const char* _path);
extern DLLEXPORT void		pfFilePathv(const char *s,...);
extern DLLEXPORT const char* 	pfGetFilePath(void);
extern DLLEXPORT int		pfFindFile(const char* _file, char _path[PF_MAXSTRING], int _amode);


/*------------------------------- Video Clock Routines ---------------------*/

extern DLLEXPORT int		pfStartVClock(void);
extern DLLEXPORT void		pfStopVClock(void);
extern DLLEXPORT void 		pfInitVClock(int _ticks);
extern DLLEXPORT void		pfVClockOffset(int _offset);
extern DLLEXPORT int		pfGetVClockOffset(void);
extern DLLEXPORT int 		pfGetVClock(void);
extern DLLEXPORT int 		pfVClockSync(int _rate, int _offset);

/*----------------------- pfVideoChannel Routines ------------------------*/

extern pfVideoChannel	*pfGetCurVChan(void);

/*----------------------------- pfWindow Routines ------------------------*/

extern DLLEXPORT void		prInitGfx(void);
extern DLLEXPORT pfWindow		*pfGetCurWin(void);

#ifndef __PF_H__
/* 
 * Macros for libpr -> libpf inheritance.
 */
#define	pfInitGfx		prInitGfx
#endif	/* __PF_H__	*/

/*------------------------- Window System Routines -----------------------*/

extern DLLEXPORT void 		pfCloseWSConnection(pfWSConnection _dsp);

extern DLLEXPORT pfFBConfig	pfChooseFBConfig(pfWSConnection _dsp, int _screen, int *_attr);
extern DLLEXPORT pfFBConfig	pfChooseFBConfigData(void **dst, pfWSConnection dsp, int screen, int *attr, void *arena);
extern DLLEXPORT void 		pfSelectWSConnection(pfWSConnection);
extern DLLEXPORT pfWSConnection	pfOpenWSConnection(const char *_str, int _shared);
extern DLLEXPORT pfWSConnection	pfOpenScreen(int _screen, int _shared);
extern DLLEXPORT pfWSConnection	pfGetCurWSConnection(void);
extern DLLEXPORT const char*	pfGetWSConnectionName(pfWSConnection);
extern DLLEXPORT void		pfGetScreenSize(int screen, int *_x, int *_y);
extern DLLEXPORT int		pfGetNumScreenVChans(int scr);

/*-------------------------------- Query Features -----------------------*/

/* 
 * feature query return bitmasks 
 */
#define PFQFTR_FALSE	0x0	/* feature is not available */
#define PFQFTR_TRUE	0x1	/* feature is available on system */
#define PFQFTR_FAST_BIT	0x2	/* feature is reasonable to use in real-time */
#define PFQFTR_FAST	(PFQFTR_TRUE | PFQFTR_FAST_BIT)	

/* 
 * feature queries
 */
#define PFQFTR_BIT	    		    0x800000	    /* int */
#define PFQFTR_VSYNC	    		    0x800110	    /* int */
#define PFQFTR_VSYNC_SET		    0x800120	    /* int */
#define PFQFTR_GANGDRAW			    0x800210	    /* int */
#define PFQFTR_HYPERPIPE		    0x800230	    /* int */
#define PFQFTR_STEREO_IN_WINDOW		    0x800310	    /* int */
#define PFQFTR_TAG_CLEAR		    0x800320	    /* int */
#define PFQFTR_XSGIVC		    	    0x800330	    /* int */
#define PFQFTR_PBUFFER		    	    0x800340	    /* int */
#define PFQFTR_DMEDIA		    	    0x800350	    /* int */
#define PFQFTR_VLDMEDIA		    	    0x800360	    /* int */
#define PFQFTR_INTERLACE	    	    0x800370	    /* int */

#define PFQFTR_MULTISAMPLE		    0x800410	    /* int */
#define PFQFTR_MULTISAMPLE_TRANSP	    0x800420	    /* int */
#define PFQFTR_MULTISAMPLE_ROUND_POINTS	    0x800430	    /* int */
#define PFQFTR_MULTISAMPLE_STENCIL	    0x800440	    /* int */

#define PFQFTR_ACCUMULATION                 0x800450        /* int */

#define PFQFTR_COLOR_ABGR		    0x800510	    /* int */

#define PFQFTR_DISPLACE_POLYGON		    0x800610	    /* int */
#define PFQFTR_POLYMODE			    0x800710	    /* int */
#define PFQFTR_TRANSPARENCY	            0x800810	    /* int */
#define PFQFTR_MTL_CMODE	            0x800910	    /* int */
    
#define PFQFTR_FOG_SPLINE		    0x800a10	    /* int */
#define PFQFTR_FOG_LAYERED		    0x800a20	    /* int */
#define PFQFTR_LINE_DIST_TEXGEN		    0x800a30	    /* int */

#define PFQFTR_ALPHA_FUNC		    0x800b10	    /* int */
#define PFQFTR_ALPHA_FUNC_COMPARE_REF	    0x800b20	    /* int */

#define PFQFTR_BLENDCOLOR		    0x800c10	    /* int */
#define PFQFTR_BLEND_FUNC_SUBTRACT	    0x800c20	    /* int */
#define PFQFTR_BLEND_FUNC_MINMAX	    0x800c30	    /* int */

#define PFQFTR_FRAMEZOOM	    	    0x800d10	    /* int */
#define PFQFTR_GLSPRITE		    	    0x800d20	    /* int */
#define PFQFTR_LIGHTPOINT	    	    0x800d30	    /* int */
#define PFQFTR_DVR	    	    	    0x800d40	    /* int */
#define PFQFTR_DECAL_PLANE    	    	    0x800d50	    /* int */

/* some more OpenGL vs IRIS GL distinctions */
#define PFQFTR_LMODEL_ATTENUATION	    0x800e10    /* int */
#define PFQFTR_LIGHT_ATTENUATION	    0x800e20    /* int */
#define PFQFTR_LIGHT_CLR_SPECULAR	    0x800e30    /* int */

#define PFQFTR_TEXTURE			    0x801110	    /* int */
#define PFQFTR_TEXTURE_16BIT_IFMTS	    0x801210	    /* int */
#define PFQFTR_TEXTURE_SUBTEXTURE	    0x801310	    /* int */
#define PFQFTR_TEXTURE_TRILINEAR	    0x801410	    /* int */

#define PFQFTR_TEXTURE_ANISOTROPIC          0x801420

#define PFQFTR_TEXTURE_DETAIL		    0x801510	    /* int */
#define PFQFTR_TEXTURE_SHARPEN		    0x801610	    /* int */
#define PFQFTR_TEXTURE_3D		    0x801710	    /* int */
#define PFQFTR_TEXTURE_PROJECTIVE	    0x801810	    /* int */
#define PFQFTR_TEXTURE_EDGE_CLAMP    	    0x801910	    /* int */
#define PFQFTR_TEXTURE_CLIPMAP    	    0x801920	    /* int */
#define PFQFTR_TEXTURE_5551                 0x801930	    /* int */
#define PFQFTR_TEXTURE_MINFILTER_BILINEAR_CMP 0x801940	    /* int */
#define PFQFTR_TEXTURE_SHADOW               0x801950        /* int */
#define PFQFTR_SHADOW_AMBIENT               0x801951        /* int */
#define PFQFTR_TEXTURE_LOD_RANGE            0x801960        /* int */
#define PFQFTR_TEXTURE_LOD_BIAS             0x801961        /* int */
#define PFQFTR_READ_MSDEPTH_BUFFER	    0x802110	    /* int */
#define PFQFTR_COPY_MSDEPTH_BUFFER	    0x802120	    /* int */
#define PFQFTR_READ_TEXTURE_MEMORY	    0x802210	    /* int */
#define PFQFTR_COPY_TEXTURE_MEMORY	    0x802220	    /* int */
#define PFQFTR_CALLIGRAPHIC                 0x803010        /* int */
#define PFQFTR_PIPE_STATS                   0x804010        /* int */



#define PFQFTR_AALINES                      0x805050
#define PFQFTR_AAPOINTS                     0x805060

#define PFQFTR_REGISTER_COMBINERS           0x805100

#define PFQFTR_STIPPLE                      0x805200

/* Feature set tokens */
#define PFFTR_BIT			    0x400000
#define PFFTR_MACH			    0x400110
#define PFFTR_VSYNC			    0x400120
#define PFFTR_VSYNC_SET		    	    0x400130
#define PFFTR_MULTISAMPLE		    0x400140
#define PFFTR_MULTISAMPLE_ROUND_POINTS	    0x400150
#define PFFTR_ALPHA_FUNC_ALL		    0x400160
#define PFFTR_DISPLACE_POLYGON		    0x400170
#define PFFTR_POLYMODE			    0x400180
#define PFFTR_FOG_SPLINE		    0x400190
#define PFFTR_FOG_LAYERED		    0x400191
#define PFFTR_GANGDRAW			    0x4001a0
#define PFFTR_HYPERPIPE		    	    0x4001b0
#define PFFTR_FAST_TRANSPARENCY		    0x4001c0	
#define PFFTR_FAST_MTL_CMODE		    0x4001d0	
#define PFFTR_PBUFFER			    0x4001e0
#define PFFTR_TEXTURE_FAST		    0x401110
#define PFFTR_TEXTURE_16BIT_IFMTS	    0x401120
#define PFFTR_TEXTURE_SUBTEXTURE	    0x401130
#define PFFTR_TEXTURE_3D		    0x401140
#define PFFTR_TEXTURE_DETAIL		    0x401150
#define PFFTR_TEXTURE_SHARPEN		    0x401160
#define PFFTR_TEXTURE_OBJECT		    0x401170
#define PFFTR_TEXTURE_PROJECTIVE	    0x401180
#define PFFTR_TEXTURE_TRILINEAR	    	    0x401190
#define PFFTR_TEXTURE_EDGE_CLAMP    	    0x4011a0
#define PFFTR_TEXTURE_CLIPMAP    	    0x4011b0
#define PFFTR_TEXTURE_5551                  0x4011c0
#define PFFTR_TEXTURE_MINFILTER_BILINEAR_CMP 0x4011d0
#define PFFTR_TEXTURE_SHADOW                0x4011e0
#define PFFTR_TEXTURE_LOD_RANGE             0x4011f0
#define PFFTR_TEXTURE_LOD_BIAS              0x4011f1

#define PFFTR_TEXTURE_ANISOTROPIC           0x4011f2 

#define PFFTR_FRAMEZOOM		    	    0x402100
#define PFFTR_GLSPRITE		    	    0x402110
#define PFFTR_LIGHTPOINT	    	    0x402120
#define PFFTR_TAG_CLEAR			    0x402140
#define PFFTR_DECAL_PLANE                   0x402150
#define PFFTR_XSGIVC		    	    0x402210	
#define PFFTR_DVR	    	    	    0x402220
#define PFFTR_CALLIGRAPHIC                  0x403010
#define PFFTR_PIPE_STATS                    0x404010
#define PFFTR_DMEDIA		    	    0x404020
#define PFFTR_VLDMEDIA		    	    0x404030
#define PFFTR_INTERLACE	    		    0x404040


#define PFFTR_FAST_AALINES                 0x404080
#define PFFTR_FAST_AAPOINTS                0x404090

#define PFFTR_REGISTER_COMBINERS           0x404100
#define PFFTR_STIPPLE                      0x404200

#define _PFFTR_NO_STATE_DLS		    0x410110
#define _PFFTR_NO_MIPMAPS		    0x410120




extern DLLEXPORT const char *pfGetMachString(void);
extern DLLEXPORT const char *pfGetRelString(void);
extern float 	pfGetIRIXRelease(void);
extern DLLEXPORT int	pfQueryFeature(int64_t _which, int *_dst);
extern DLLEXPORT int	pfMQueryFeature(int *_which, int *_dst);
extern DLLEXPORT void	pfFeature(int _which, int _val);
extern DLLEXPORT void   pfStippleMasks(int numLevels, GLubyte **masks);

/*-------------------------------- Query System ----------------------------*/

/* 
 * absolute config queries - returns 1 value 
 */
#define PFQSYS_BIT			    0x200000	/* int */
#define PFQSYS_MAJOR_VERSION		    0x200101	/* int */
#define PFQSYS_MINOR_VERSION		    0x200102	/* int */
#define PFQSYS_MAINT_VERSION		    0x200103	/* int */
#define PFQSYS_GL			    0x200110	/* int */
#define PFQSYS_IRIX_MAJOR_VERSION	    0x200120	/* int */
#define PFQSYS_IRIX_MINOR_VERSION	    0x200121	/* int */

#define PFQSYS_NUM_CPUS			    0x200210	/* int */
#define PFQSYS_NUM_CPUS_AVAILABLE	    0x200220    /* int */
#define PFQSYS_NUM_SCREENS		    0x200310	/* int */
#define PFQSYS_SIZE_PIX_X		    0x200410   	/* int */
#define PFQSYS_SIZE_PIX_Y		    0x200510   	/* int */

#define PFQSYS_MAX_SNG_RGB_BITS	    	    0x200610   	/* int */
#define PFQSYS_MAX_SNG_ALPHA_BITS    	    0x200620   	/* int */
#define PFQSYS_MAX_DBL_RGB_BITS	    	    0x200630   	/* int */
#define PFQSYS_MAX_DBL_ALPHA_BITS	    0x200640   	/* int */
#define PFQSYS_MAX_SNG_CI_BITS		    0x200650   	/* int */
#define PFQSYS_MAX_DBL_CI_BITS		    0x200660   	/* int */
#define PFQSYS_MAX_SNG_OVERLAY_CI_BITS	    0x200670   	/* int */
#define PFQSYS_MAX_DBL_OVERLAY_CI_BITS	    0x200680   	/* int */
#define PFQSYS_MAX_DEPTH_BITS		    0x200690   	/* int */
#define PFQSYS_MIN_DEPTH_VAL		    0x2006a0   	/* int */
#define PFQSYS_MAX_DEPTH_VAL		    0x2006b0   	/* int */
#define PFQSYS_MAX_STENCIL_BITS		    0x2006c0   	/* int */
#define PFQSYS_MAX_MS_SAMPLES		    0x2006d0    /* int */
#define PFQSYS_MAX_MS_DEPTH_BITS	    0x2006e0    /* int */
#define PFQSYS_MAX_MS_STENCIL_BITS	    0x2006f0	/* int */
#define PFQSYS_MAX_DBL_ACCUM_BITS           0x200700    /* int */
#define PFQSYS_MAX_SNG_ACCUM_BITS           0x200710    /* int */
#define PFQSYS_MAX_LIGHTS		    0x200810    /* int */
#define PFQSYS_TEXTURE_MEMORY_BYTES	    0x200920    /* int */
#define PFQSYS_MAX_TEXTURE_SIZE		    0x200930    /* int */
#define PFQSYS_MAX_CLIPTEXTURE_SIZE	    0x200940	/* int */
#define PFQSYS_CLIPTEXTURE_CENTER_ALIGNMENT 0x200950	/* int */
#define PFQSYS_MIN_CLIPTEXTURE_INVALID_BORDER 0x200960	/* int */
#define PFQSYS_TEX_SUBLOAD_ALIGNMENT_S      0x200970	/* int */
#define PFQSYS_TEX_SUBLOAD_ALIGNMENT_T      0x200971	/* int */
#define PFQSYS_TEX_SUBLOAD_ALIGNMENT_R      0x200972	/* int */
#define PFQSYS_CALLIGRAPHIC_PIPE0	    0x200980    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE1	    0x200981    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE2	    0x200982    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE3	    0x200983    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE4	    0x200984    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE5	    0x200985    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE6	    0x200986    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE7	    0x200987    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE8	    0x200988    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE9	    0x200989    /* int */


#define PFQSYS_MAX_TEXTURES                 0x201002

#define PFQSYS_MAX_ANISOTROPY               0x201003
#define PFQSYS_MAX_GENERAL_COMBINERS        0x201004

extern DLLEXPORT int	pfQuerySys(int _which, int *_dst);
extern DLLEXPORT int	pfMQuerySys(int *_which, int *_dst);

#define PFQPERF_DEFAULT_TEXLOAD_TABLE       0x01
#define PFQPERF_USER_TEXLOAD_TABLE          0x02
#define PFQPERF_CUR_TEXLOAD_TABLE           0x04
extern DLLEXPORT int      pfQueryPerf(int _which, void *_dst);
extern DLLEXPORT int      pfPerf(int _which, void *_dst);

/* -------------------Image Cache ------------------------------------------ */

#define MAX_TILE_FILENAME_UNIQUE_ARGS			14
#define PFIMAGECACHE_MAX_TILE_FILENAME_ARGS		16

/* ------------------- uslock stuff ---------------------------------------- */

extern DLLEXPORT int pfGentle_ussetlock(ulock_t lock, int spins);

/* ------------------- Test And Set Locking -------------------------------- */

#define PF_TAS_LOCK(_lock)  

#define PF_TAS_UNLOCK(_lock)    *(_lock) = 0


/*
 * ------------------------------------------------------------------------
 */

/***
 ***	comparison functions
 ***/


/*
 * ---------------------- C API --------------------------------
 */

#if PF_C_API


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfObject Tokens ----------------------- */

/* 
 * pfPrint() 
 * verbosity levels for printing -- XXX should we just use notify tokens ??? 
 */
#define PFPRINT_VB_OFF	    0	/* no printing */
#define PFPRINT_VB_ON	    1	/* default - minimal printing */
#define PFPRINT_VB_NOTICE   1	/* default - minimal printing */
#define PFPRINT_VB_INFO     2	/* basic info */
#define PFPRINT_VB_DEBUG    3	/* prints everything */

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfObject CAPI ------------------------------*/

extern DLLEXPORT pfType*              pfGetObjectClassType(void);
extern DLLEXPORT void                 pfCopyFunc(pfCopyFuncType _func);
extern DLLEXPORT pfCopyFuncType       pfGetCopyFunc(void);
extern DLLEXPORT void                 pfDeleteFunc(pfDeleteFuncType _func);
extern DLLEXPORT pfMergeFuncType      pfGetMergeFunc(void);
extern DLLEXPORT void                 pfMergeFunc(pfMergeFuncType _func);
extern DLLEXPORT pfDeleteFuncType     pfGetDeleteFunc(void);
extern DLLEXPORT void                 pfPrintFunc(pfPrintFuncType _func);
extern DLLEXPORT pfPrintFuncType      pfGetPrintFunc(void);
extern DLLEXPORT void                 pfCopyFuncSlot(int _slot, pfCopyFuncType _func);
extern DLLEXPORT pfCopyFuncType       pfGetCopyFuncSlot(int _slot);
extern DLLEXPORT void                 pfDeleteFuncSlot(int _slot, pfDeleteFuncType _func);
extern DLLEXPORT pfDeleteFuncType     pfGetDeleteFuncSlot(int _slot);
extern DLLEXPORT void                 pfMergeFuncSlot(int _slot, pfMergeFuncType _func);
extern DLLEXPORT pfMergeFuncType      pfGetMergeFuncSlot(int _slot);
extern DLLEXPORT void                 pfPrintFuncSlot(int _slot, pfPrintFuncType _func);
extern DLLEXPORT pfPrintFuncType      pfGetPrintFuncSlot(int _slot);
extern DLLEXPORT int                  pfGetGLHandle(const pfObject *_obj);
extern DLLEXPORT void                 pfDeleteGLHandle(pfObject *_obj);
extern DLLEXPORT void                 pfUserData(pfObject* _obj, void* data);
extern DLLEXPORT void*                pfGetUserData(pfObject* _obj);
extern DLLEXPORT void                 pfUserDataSlot(pfObject* _obj, int _slot, void* data);
extern DLLEXPORT void*                pfGetUserDataSlot(pfObject* _obj, int _slot);
extern DLLEXPORT int                  pfGetNumUserData(pfObject* _obj);
extern DLLEXPORT int                  pfGetNamedUserDataSlot(const char *_name);
extern DLLEXPORT const char*          pfGetUserDataSlotName(int _slot);
extern DLLEXPORT int                  pfGetNumNamedUserDataSlots(void);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */

/* ------------------ pfObject Macros --------------------- */

#if defined(__STDC__) || defined(__cplusplus)
#if !PF_CPLUSPLUS_API
#define pfGetGLHandle(obj)		pfGetGLHandle((pfObject*)obj)
#define pfDeleteGLHandle(obj)		pfDeleteGLHandle((pfObject*)obj)
#define pfUserData(obj, data)		pfUserData((pfObject*)obj, data)
#define pfUserDataSlot(obj, slot, data)	\
			pfUserDataSlot((pfObject*)obj, slot, data)
#define pfGetUserData(obj)		pfGetUserData((pfObject*)obj)
#define pfGetUserDataSlot(obj, slot)	pfGetUserDataSlot((pfObject*)obj, slot)
#define pfGetNumUserData(obj)		pfGetNumUserData((pfObject*)obj)
#endif /* !PF_CPLUSPLUS_API */
#endif /* __STDC__ || __cplusplus */


#endif /* !PF_CPLUSPLUS_API */


#ifdef PF_C_API
/*---------------- pfType CAPI ------------------------------*/

extern DLLEXPORT pfType*              pfNewType(pfType *parent, char *name);
extern DLLEXPORT pfType*              pfGetTypeParent(pfType* _type);
extern DLLEXPORT int                  pfIsDerivedFrom(pfType* _type, pfType *ancestor);
extern DLLEXPORT void                 pfMaxTypes(int _n);

#endif /* !PF_C_API */



#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ------------------ pfImageCache Related Definitions --------------------- */
#define PF_DTR_TEXLOAD      0x01 /*check if tex region can be loaded in time */
#define PF_DTR_MEMLOAD      0x02 /*check if memory tiles where loaded in time */
#define PF_DTR_READSORT     0x04 /*sort memory tile read queue */
#define PF_DTR_TEXSIZE      0x08 /*shrink size of textured level (obsolete) */
/*
** PFIMAGECACHE_MAX_TILE_FILENAME_ARGS has been moved to pr.h so
** pfutil.h can use it.
*/

#define PFIMAGECACHE_AUTOCENTER				0x1

#define PFIMAGECACHE_AUTOCREATE_STREAMSERVER_QUEUES 	0x2

#define PFIMAGECACHE_AUTOSET_TILE_FILENAME		0x3

#define PFIMAGECACHE_AUTOSET_TILE_READQUEUE		0x4

#define PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_S		0
#define PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_T		1
#define PFIMAGECACHE_TILE_FILENAMEARG_VSIZE_R		2
#define PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_S		3
#define PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_T		4
#define PFIMAGECACHE_TILE_FILENAMEARG_TILENUM_R		5
#define PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_S		6
#define PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_T		7
#define PFIMAGECACHE_TILE_FILENAMEARG_TILEORG_R		8
#define PFIMAGECACHE_TILE_FILENAMEARG_STREAMSERVERNAME	9
#define PFIMAGECACHE_TILE_FILENAMEARG_CACHENAME		10
#define PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_S		11
#define PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_T		12
#define PFIMAGECACHE_TILE_FILENAMEARG_FILENUM_R		13
 
#define PFIMAGECACHE_S_DIMENSION			0x0
#define PFIMAGECACHE_T_DIMENSION			0x1
#define PFIMAGECACHE_R_DIMENSION			0x2

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfImageCache CAPI ------------------------------*/

extern DLLEXPORT pfImageCache*        pfNewImageCache(void *arena);
extern DLLEXPORT pfType*              pfGetImageCacheClassType(void);
extern DLLEXPORT void                 pfImageCacheName(pfImageCache* _imagecache, const char *_name);
extern DLLEXPORT const char*          pfGetImageCacheName(pfImageCache* _imagecache);
extern DLLEXPORT void                 pfImageCacheProtoTile(pfImageCache* _imagecache, pfImageTile *tile);
extern DLLEXPORT pfImageTile*         pfGetImageCacheProtoTile(pfImageCache* _imagecache);
extern DLLEXPORT void                 pfImageCacheTexRegionOrg(pfImageCache* _imagecache, int _originS, int _originT, int _originR);
extern DLLEXPORT void                 pfImageCacheTexRegionSize(pfImageCache* _imagecache, int _sizeS, int _sizeT, int _sizeR);
extern DLLEXPORT void                 pfGetImageCacheTexRegionOrg(pfImageCache* _imagecache, int *_originS, int *_originT, int *_originR);
extern DLLEXPORT void                 pfGetImageCacheTexRegionSize(pfImageCache* _imagecache, int *_sizeS, int *_sizeT, int *_sizeR);
extern DLLEXPORT void                 pfGetImageCacheCurTexRegionOrg(pfImageCache* _imagecache, int *_originS, int *_originT, int *_originR);
extern DLLEXPORT void                 pfGetImageCacheCurTexRegionSize(pfImageCache* _imagecache, int *_sizeS, int *_sizeT, int *_sizeR);
extern DLLEXPORT void                 pfImageCacheMemRegionOrg(pfImageCache* _imagecache, int _tileS, int _tileT, int _tileR);
extern DLLEXPORT void                 pfImageCacheMemRegionSize(pfImageCache* _imagecache, int _tileS, int _tileT, int _tileR);
extern DLLEXPORT void                 pfGetImageCacheMemRegionOrg(pfImageCache* _imagecache, int *_orgTileS, int *_orgTileT, int *_orgTileR);
extern DLLEXPORT void                 pfGetImageCacheMemRegionSize(pfImageCache* _imagecache, int *_sizeS, int *_sizeT, int *_sizeR);
extern DLLEXPORT void                 pfGetImageCacheCurMemRegionOrg(pfImageCache* _imagecache, int *_orgS, int *_orgT, int *_orgR);
extern DLLEXPORT void                 pfGetImageCacheCurMemRegionSize(pfImageCache* _imagecache, int *_sizeS, int *_sizeT, int *_sizeR);
extern DLLEXPORT void                 pfImageCacheTexSize(pfImageCache* _imagecache, int _s, int _t, int _r);
extern DLLEXPORT void                 pfGetImageCacheTexSize(pfImageCache* _imagecache, int *_s, int *_t, int *_r);
extern DLLEXPORT void                 pfGetImageCacheTexRegionOffset(pfImageCache* _imagecache, int *_s, int *_t, int *_r);
extern DLLEXPORT void                 pfImageCacheTex(pfImageCache* _imagecache, void *_dst, int _lvl, int _type);
extern DLLEXPORT void                 pfGetImageCacheTex(pfImageCache* _imagecache, void **_dst, int *_lvl, int *_type);
extern DLLEXPORT void                 pfImageCacheImageSize(pfImageCache* _imagecache, int _sizeS, int _sizeT, int _sizeR);
extern DLLEXPORT void                 pfGetImageCacheImageSize(pfImageCache* _imagecache, int *_sizeS, int *_sizeT, int *_sizeR);
extern DLLEXPORT void                 pfImageCacheReadQueueFunc(pfImageCache* _imagecache, pfImageCacheReadQueueFuncType _func);
extern DLLEXPORT void                 pfImageCacheTileFileNameFunc(pfImageCache* _imagecache, pfTileFileNameFuncType _func);
extern DLLEXPORT pfImageCacheReadQueueFuncType pfGetImageCacheReadQueueFunc(pfImageCache* _imagecache);
extern DLLEXPORT pfTileFileNameFuncType pfGetImageCacheTileFileNameFunc(pfImageCache* _imagecache);
extern DLLEXPORT void                 pfImageCacheFileStreamServer(pfImageCache* _imagecache, int _dim, int _which, const char *_device);
extern DLLEXPORT const char*          pfGetImageCacheFileStreamServer(pfImageCache* _imagecache, int _dim, int _which);
extern DLLEXPORT void                 pfImageCacheStreamServerQueue(pfImageCache* _imagecache, int _dim, int _which, pfQueue *_q);
extern DLLEXPORT pfQueue*             pfGetImageCacheStreamServerQueue(pfImageCache* _imagecache, int _dim, int _which);
extern DLLEXPORT pfQueue*             pfGetImageCacheStreamServerQueueByName(pfImageCache* _imagecache, const char *_name);
extern DLLEXPORT void                 pfImageCacheTileFileNameFormat(pfImageCache* _imagecache, const char *_fmtString, int _nArgs, int *_argList);
extern DLLEXPORT void                 pfGetImageCacheTileFileNameFormat(pfImageCache* _imagecache, const char **_fmtString, int *_nArgs, const int **_argList);
extern DLLEXPORT int                  pfGetImageCacheNumStreamServers(const pfImageCache* _imagecache, int _dim);
extern DLLEXPORT void                 pfImageCacheMode(pfImageCache* _imagecache, int _mode, int _val);
extern DLLEXPORT int                  pfGetImageCacheMode(pfImageCache* _imagecache, int _mode);
extern DLLEXPORT void                 pfImageCacheMaster(pfImageCache* _imagecache, pfImageCache *_master);
extern DLLEXPORT pfImageCache*        pfGetImageCacheMaster(const pfImageCache* _imagecache);
extern DLLEXPORT pfList*              pfGetImageCacheSlaves(const pfImageCache* _imagecache);
extern DLLEXPORT void                 pfImageCacheDTRMode(pfImageCache* _imagecache, uint _mode);
extern DLLEXPORT uint                 pfGetImageCacheDTRMode(pfImageCache* _imagecache);
extern DLLEXPORT pfImageTile*         pfGetImageCacheTile(pfImageCache* _imagecache, int _s, int _t, int _r);
extern DLLEXPORT const pfList*        pfGetImageCacheLoadUpdates(pfImageCache* _imagecache);
extern DLLEXPORT void                 pfImageCacheCalcTexRegion(pfImageCache* _imagecache, int *_orgS, int *_orgT, int *_orgR, int *_sizeS, int *_sizeT, int *_sizeR);
extern DLLEXPORT void                 pfImageCacheCalcMemRegion(pfImageCache* _imagecache, int *_orgS, int *_orgT, int *_orgR, int *_sizeS, int *_sizeT, int *_sizeR);
extern DLLEXPORT int                  pfIsImageCacheValid(pfImageCache* _imagecache, int _s, int _t, int _r, int _sizeS, int _sizeT, int _sizeR);
extern DLLEXPORT int                  pfIsValidImageCache(pfImageCache* _imagecache);
extern DLLEXPORT int                  pfIsImageCacheTexRegChanged(pfImageCache* _imagecache);
extern DLLEXPORT float                pfDTRApplyImageCache(pfImageCache* _imagecache, float _msec);
extern DLLEXPORT void                 pfApplyImageCache(pfImageCache* _imagecache);
extern DLLEXPORT void                 pfInvalidateImageCache(pfImageCache* _imagecache);
extern DLLEXPORT void                 pfAutoConfigFileNameImageCache(pfImageTile *_itile);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ------------------ pfImageTile Related Functions--------------------- */
#define PFCLIPTEXTURE_MAX_LEVELS 32

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfClipTexture CAPI ------------------------------*/

extern DLLEXPORT pfClipTexture*       pfNewClipTexture(void *arena);
extern DLLEXPORT pfType*              pfGetClipTextureClassType(void);
extern DLLEXPORT int                  pfGetClipTextureMaxClipSize( int bytespertexel );
extern DLLEXPORT int                  pfIsClipTextureEmulated(pfClipTexture* _cliptexture);
extern DLLEXPORT int                  pfGetClipTextureNumClippedLevels(pfClipTexture* _cliptexture);
extern DLLEXPORT void*                pfGetClipTextureCteAttr(pfClipTexture* _cliptexture,  int which );
extern DLLEXPORT void                 pfClipTextureCteAttr(pfClipTexture* _cliptexture,  int which, void* val );
extern DLLEXPORT void                 pfClipTextureLevel(pfClipTexture* _cliptexture, int _level, pfObject* _levelObj);
extern DLLEXPORT pfObject*            pfGetClipTextureLevel(pfClipTexture* _cliptexture, int _level);
extern DLLEXPORT void                 pfClipTextureCenter(pfClipTexture* _cliptexture, int _s, int _t, int _r);
extern DLLEXPORT void                 pfGetClipTextureCenter(pfClipTexture* _cliptexture, int *_s, int *_t, int *_r);
extern DLLEXPORT void                 pfGetClipTextureCurCenter(pfClipTexture* _cliptexture, int *_s, int *_t, int *_r);
extern DLLEXPORT void                 pfClipTextureClipSize(pfClipTexture* _cliptexture, int _clipSize);
extern DLLEXPORT int                  pfGetClipTextureClipSize(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureVirtualLODOffset(pfClipTexture* _cliptexture, int _offset);
extern DLLEXPORT int                  pfGetClipTextureVirtualLODOffset(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureNumEffectiveLevels(pfClipTexture* _cliptexture, int _levels);
extern DLLEXPORT int                  pfGetClipTextureNumEffectiveLevels(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureNumAllocatedLevels(pfClipTexture* _cliptexture, int _levels);
extern DLLEXPORT int                  pfGetClipTextureNumAllocatedLevels(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureVirtualSize(pfClipTexture* _cliptexture, int _width, int _height, int _depth);
extern DLLEXPORT void                 pfGetClipTextureVirtualSize(pfClipTexture* _cliptexture, int *_width, int *_height, int *_depth);
extern DLLEXPORT void                 pfClipTextureInvalidBorder(pfClipTexture* _cliptexture, int _nTexels);
extern DLLEXPORT int                  pfGetClipTextureInvalidBorder(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureLevelPhaseMargin(pfClipTexture* _cliptexture, int level, int size);
extern DLLEXPORT int                  pfGetClipTextureLevelPhaseMargin(pfClipTexture* _cliptexture, int level);
extern DLLEXPORT void                 pfClipTextureLevelPhaseShift(pfClipTexture* _cliptexture, int level, int phaseShiftS, int phaseShiftT, int phaseShiftR);
extern DLLEXPORT void                 pfGetClipTextureLevelPhaseShift(pfClipTexture* _cliptexture, int level, int *phaseShiftS, int *phaseShiftT, int *phaseShiftR);
extern DLLEXPORT void                 pfGetClipTextureOffset(pfClipTexture* _cliptexture, int *_s, int *_t, int *_r);
extern DLLEXPORT void                 pfClipTextureTexLoadTime(pfClipTexture* _cliptexture, float _msec);
extern DLLEXPORT float                pfGetClipTextureTexLoadTime(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureMaster(pfClipTexture* _cliptexture, pfClipTexture *_master);
extern DLLEXPORT pfClipTexture*       pfGetClipTextureMaster(const pfClipTexture* _cliptexture);
extern DLLEXPORT pfList*              pfGetClipTextureSlaves(const pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureMinClipSizeRatio(pfClipTexture* _cliptexture, float _size);
extern DLLEXPORT float                pfGetClipTextureMinClipSizeRatio(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureDTRMode(pfClipTexture* _cliptexture, uint _mask);
extern DLLEXPORT uint                 pfGetClipTextureDTRMode(pfClipTexture* _cliptexture);
extern DLLEXPORT float                pfGetClipTextureMinDTRLOD(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureDTRFadeCount(pfClipTexture* _cliptexture, int _count);
extern DLLEXPORT int                  pfGetClipTextureDTRFadeCount(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureDTRBlurMargin(pfClipTexture* _cliptexture, float frac);
extern DLLEXPORT float                pfGetClipTextureDTRBlurMargin(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfClipTextureLODOffsetLimit(pfClipTexture* _cliptexture, int _lo, int _hi);
extern DLLEXPORT void                 pfGetClipTextureLODOffsetLimit(pfClipTexture* _cliptexture, int *_lo, int *_hi);
extern DLLEXPORT void                 pfClipTextureNumEffectiveLevelsLimit(pfClipTexture* _cliptexture, int _lo, int _hi);
extern DLLEXPORT void                 pfGetClipTextureNumEffectiveLevelsLimit(pfClipTexture* _cliptexture, int *_lo, int *_hi);
extern DLLEXPORT void                 pfClipTextureMinLODLimit(pfClipTexture* _cliptexture, float _lo, float _hi);
extern DLLEXPORT void                 pfGetClipTextureMinLODLimit(pfClipTexture* _cliptexture, float *_lo, float *_hi);
extern DLLEXPORT void                 pfClipTextureMaxLODLimit(pfClipTexture* _cliptexture, float _lo, float _hi);
extern DLLEXPORT void                 pfGetClipTextureMaxLODLimit(pfClipTexture* _cliptexture, float *_lo, float *_hi);
extern DLLEXPORT void                 pfClipTextureLODBiasLimit(pfClipTexture* _cliptexture, float _Slo, float _Shi, float _Tlo, float _Thi, float _Rlo, float _Rhi);
extern DLLEXPORT void                 pfGetClipTextureLODBiasLimit(pfClipTexture* _cliptexture, float *_Slo, float *_Shi, float *_Tlo, float *_Thi, float *_Rlo, float *_Rhi);
extern DLLEXPORT void                 pfClipTextureLODRange(pfClipTexture* _cliptexture, float _min, float _max);
extern DLLEXPORT void                 pfGetClipTextureLODRange(pfClipTexture* _cliptexture, float *_min, float *_max);
extern DLLEXPORT void                 pfGetClipTextureCurLODRange(pfClipTexture* _cliptexture, float *_min, float *_max);
extern DLLEXPORT void                 pfClipTextureLODBias(pfClipTexture* _cliptexture, float _s, float _t, float _r);
extern DLLEXPORT void                 pfGetClipTextureLODBias(pfClipTexture* _cliptexture, float *_s, float *_t, float *_r);
extern DLLEXPORT void                 pfGetClipTextureCurLODBias(pfClipTexture* _cliptexture, float *_s, float *_t, float *_r);
extern DLLEXPORT void                 pfApplyClipTexture(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfApplyClipTextureMemReg(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfApplyClipTextureTexReg(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfUpdateClipTextureMemReg(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfUpdateClipTextureTexReg(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfUpdateClipTexture(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfInvalidateClipTexture(pfClipTexture* _cliptexture);
extern DLLEXPORT void                 pfApplyClipTextureVirtualParams(pfClipTexture* _cliptexture, int _offset, int _levels);
extern DLLEXPORT void                 pfApplyClipTextureCenter(pfClipTexture* _cliptexture, int _s, int _t, int _r);
extern DLLEXPORT int                  pfIsClipTextureVirtual(pfClipTexture* _cliptexture);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */

/* ------------------ pfClipTexture Macros --------------------- */

#if defined(__STDC__) || defined(__cplusplus)
#if !PF_CPLUSPLUS_API
#define pfClipTextureLevel(_ct, _level, _obj) pfClipTextureLevel(_ct, _level, (pfObject*)_obj)
#endif /* !PF_CPLUSPLUS_API */
#endif /* __STDC__ || __cplusplus */

#endif /* !PF_CPLUSPLUS_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfImageTile Tokens ----------------------- */

/* ------- pfImageTile Dirty Tokens ------ */
#define PFIMAGETILE_FILEORIGIN		0x1
#define PFIMAGETILE_SIZE		0x2
#define PFIMAGETILE_MEMFORMAT		0x4
#define PFIMAGETILE_NAME		0x8
#define PFIMAGETILE_FILENAME		0x10
#define PFIMAGETILE_FILEFORMAT		0x20
#define PFIMAGETILE_IMAGE		0x40
#define PFIMAGETILE_FILEREADFUNC	0x80
#define PFIMAGETILE_USER		0x100
extern DLLEXPORT pfQueue* pfGetGlobalReadQueue(void);
extern DLLEXPORT void pfDeleteGlobalReadQueue(void);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfImageTile CAPI ------------------------------*/

extern DLLEXPORT pfImageTile*         pfNewImageTile(void *arena);
extern DLLEXPORT pfType*              pfGetImageTileClassType(void);
extern DLLEXPORT void                 pfImageTileSize(pfImageTile* _imagetile, int _w, int _h, int _d);
extern DLLEXPORT void                 pfGetImageTileSize(pfImageTile* _imagetile, int *_w, int *_h,int *_d);
extern DLLEXPORT void                 pfImageTileOrigin(pfImageTile* _imagetile, int _s, int _t, int _r);
extern DLLEXPORT void                 pfGetImageTileOrigin(pfImageTile* _imagetile, int *_s, int *_t, int *_r);
extern DLLEXPORT void                 pfImageTileMem(pfImageTile* _imagetile, unsigned char *_img, int _nBytes);
extern DLLEXPORT unsigned char*       pfGetImageTileMem(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileMemQueue(pfImageTile* _imagetile, pfQueue *q);
extern DLLEXPORT const pfQueue*       pfGetImageTileMemQueue(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfGetImageTileNumImageComponents(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileMemImageFormat(pfImageTile* _imagetile, int _format);
extern DLLEXPORT int                  pfGetImageTileMemImageFormat(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileMemImageType(pfImageTile* _imagetile, int _type);
extern DLLEXPORT int                  pfGetImageTileMemImageType(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfGetImageTileMemImageTexelSize(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileMemInfo(pfImageTile* _imagetile, int _psize, int _lock);
extern DLLEXPORT void                 pfGetImageTileMemInfo(pfImageTile* _imagetile, int *_psize, int *_lock);
extern DLLEXPORT void                 pfImageTileName(pfImageTile* _imagetile, const char *_fname);
extern DLLEXPORT const char*          pfGetImageTileName(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileFileName(pfImageTile* _imagetile, const char *_fname);
extern DLLEXPORT const char*          pfGetImageTileFileName(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfGetImageTileFileImageTexelSize(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileFileTile(pfImageTile* _imagetile, int _tileS, int _tileT, int _tileR);
extern DLLEXPORT void                 pfGetImageTileFileTile(pfImageTile* _imagetile, int *_tileS, int *_tileT, int *_tileR);
extern DLLEXPORT void                 pfImageTileNumFileTiles(pfImageTile* _imagetile, int _nTilesS, int _nTilesT, int _nTilesR);
extern DLLEXPORT void                 pfGetImageTileNumFileTiles(pfImageTile* _imagetile, int *_nTilesS, int *_nTilesT, int *_nTilesR);
extern DLLEXPORT void                 pfImageTileFileImageFormat(pfImageTile* _imagetile, int _fileFmt);
extern DLLEXPORT int                  pfGetImageTileFileImageFormat(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileFileImageType(pfImageTile* _imagetile, int _fileType);
extern DLLEXPORT int                  pfGetImageTileFileImageType(pfImageTile* _imagetile);
extern DLLEXPORT unsigned char*       pfGetImageTileSubTile(pfImageTile* _imagetile, int _s, int _t, int _r);
extern DLLEXPORT unsigned char*       pfGetImageTileValidSubTile(pfImageTile* _imagetile, int _s, int _t, int _r);
extern DLLEXPORT void                 pfImageTileReadQueue(pfImageTile* _imagetile, pfQueue *q);
extern DLLEXPORT pfQueue*             pfGetImageTileReadQueue(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfGetImageTileTotalBytes(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfGetImageTileValidBytes(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfGetImageTileValidTexels(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileValidTexels(pfImageTile* _imagetile, int _nTexels);
extern DLLEXPORT int                  pfImageTileIsValid(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfImageTileIsLoading(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfImageTileIsDirty(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileReadFunc(pfImageTile* _imagetile, pfReadImageTileFuncType _func);
extern DLLEXPORT pfReadImageTileFuncType pfGetImageTileReadFunc(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileDefaultTile(pfImageTile* _imagetile, pfImageTile *_default);
extern DLLEXPORT pfImageTile*         pfGetImageTileDefaultTile(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileDefaultTileMode(pfImageTile* _imagetile, int _useDefault);
extern DLLEXPORT int                  pfGetImageTileDefaultTileMode(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileHeaderOffset(pfImageTile* _imagetile, int hdr);
extern DLLEXPORT int                  pfGetImageTileHeaderOffset(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTilePriority(pfImageTile* _imagetile, int priority);
extern DLLEXPORT int                  pfGetImageTilePriority(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileImageCache(pfImageTile* _imagetile, pfImageCache *_ic);
extern DLLEXPORT pfImageCache*        pfGetImageTileImageCache(pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileTileIndex(pfImageTile* _imagetile, int _s, int _t, int _r);
extern DLLEXPORT void                 pfGetImageTileTileIndex(pfImageTile* _imagetile, int *_s, int *_t, int *_r);
extern DLLEXPORT void                 pfImageTileFileNameFunc(pfImageTile* _imagetile, pfTileFileNameFuncType fnFunc);
extern DLLEXPORT pfTileFileNameFuncType pfGetImageTileFileNameFunc(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfLoadImageTile(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfLoadPartialImageTile(pfImageTile* _imagetile, int nTexels);
extern DLLEXPORT int                  pfLoadImageTileFile(pfImageTile* _imagetile, const char *_fname);
extern DLLEXPORT int                  pfLoadPartialImageTileFile(pfImageTile* _imagetile, const char *_fname, int _nTexels);
extern DLLEXPORT void                 pfFreeImageTileMem(pfImageTile* _imagetile);
extern DLLEXPORT int                  pfReadDirectImageTile(pfImageTile *itile, int ntexels);
extern DLLEXPORT int                  pfReadNormalImageTile(pfImageTile *itile, int ntexels);
extern DLLEXPORT int                  pfProcessOneReadImageTile(void *_data);
extern DLLEXPORT void                 pfImageTileSortFunc(pfQueueSortFuncData *_data);
extern DLLEXPORT unsigned char*       pfGetImageTileUnalignedMem(const pfImageTile* _imagetile);
extern DLLEXPORT short                pfGetImageTileUseMemQueue(const pfImageTile* _imagetile);
extern DLLEXPORT void                 pfImageTileUseMemQueue(pfImageTile* _imagetile, short useMemQueue);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ------------------ pfTexLoad Related Functions--------------------- */

/* PFTEXLOAD ATTRS */
#define	PFTLOAD_SOURCE			0
#define PFTLOAD_DEST			1

/* PFTEXLOAD VALUES */
#define PFTLOAD_SOURCE_LEVEL		10
#define	PFTLOAD_SOURCE_S		0
#define	PFTLOAD_SOURCE_T		1
#define	PFTLOAD_SOURCE_R		2
#define PFTLOAD_DEST_LEVEL		9
#define	PFTLOAD_DEST_S			3
#define	PFTLOAD_DEST_T			4
#define	PFTLOAD_DEST_R			5
#define	PFTLOAD_WIDTH			6
#define	PFTLOAD_HEIGHT			7
#define	PFTLOAD_DEPTH			8

/* PFTEXLOAD MODES */

/* TEXLOAD SOURCE TYPES */
#define PFTLOAD_SOURCE			0
#define	PFTLOAD_SOURCE_IMAGEARRAY	0
#define	PFTLOAD_SOURCE_IMAGETILE	1
#define PFTLOAD_SOURCE_TEXTURE		2
#define	PFTLOAD_SOURCE_VIDEO		3
#define	PFTLOAD_SOURCE_FRAMEBUFFER	4

/* TEXLOAD DEST TYPES */
#define PFTLOAD_DEST			1
#define PFTLOAD_DEST_TEXTURE		0

/* TEXLOAD SYNC TYPE */
#define PFTLOAD_SYNC			2
#define PFTLOAD_SYNC_TEXTURE		0
#define PFTLOAD_SYNC_FRAME		1
#define PFTLOAD_SYNC_IMMEDIATE		2
#define PFTLOAD_SYNC_DURING_IDLE	3
#define PFTLOAD_SYNC_OFF		4

/* TEXLOAD AUTOREFERENCE DLIST LOADS */
#define PFTLOAD_AUTOREF			3

/* TEXLOAD SYNC ON SOURCE DATA */
#define PFTLOAD_SYNC_SOURCE		4

/* PFTEXLOAD DIRTY BITS */
#define PFTLOAD_DIRTY_SOURCETYPE	0x1
#define PFTLOAD_DIRTY_DESTTYPE		0x2
#define PFTLOAD_DIRTY_SYNC		0x4
#define PFTLOAD_DIRTY_SOURCE		0x8
#define PFTLOAD_DIRTY_DEST		0x10
#define PFTLOAD_DIRTY_SIZE		0x20
#define PFTLOAD_DIRTY_FRAME		0x40

/* PFTEXLOAD STATUS */
#define PFTLOAD_STATUS_WAITING		0x0
#define PFTLOAD_STATUS_COMPLETE		0x1
#define PFTLOAD_STATUS_PARTIAL		0x2
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfTexLoad CAPI ------------------------------*/

extern DLLEXPORT pfTexLoad*           pfNewTLoad(void *arena);
extern DLLEXPORT pfType*              pfGetTLoadClassType(void);
extern DLLEXPORT void                 pfTLoadAttr(pfTexLoad* _tload, int _attr, void *_val);
extern DLLEXPORT void*                pfGetTLoadAttr(pfTexLoad* _tload, int _attr);
extern DLLEXPORT void                 pfTLoadMode(pfTexLoad* _tload, int _mode, int _val);
extern DLLEXPORT int                  pfGetTLoadMode(pfTexLoad* _tload, int _mode);
extern DLLEXPORT void                 pfTLoadVal(pfTexLoad* _tload, int _which, float _val);
extern DLLEXPORT float                pfGetTLoadVal(pfTexLoad* _tload, int _which);
extern DLLEXPORT void                 pfTLoadSrcOrg(pfTexLoad* _tload, int _s, int _t, int _r);
extern DLLEXPORT void                 pfGetTLoadSrcOrg(pfTexLoad* _tload, int *_s, int *_t, int *_r);
extern DLLEXPORT void                 pfTLoadDstOrg(pfTexLoad* _tload, int _s, int _t, int _r);
extern DLLEXPORT void                 pfGetTLoadDstOrg(pfTexLoad* _tload, int *_s, int *_t, int *_r);
extern DLLEXPORT void                 pfTLoadSrc(pfTexLoad* _tload, void *_src);
extern DLLEXPORT void*                pfGetTLoadSrc(pfTexLoad* _tload);
extern DLLEXPORT void                 pfTLoadSrcLevel(pfTexLoad* _tload, int _lvl);
extern DLLEXPORT int                  pfGetTLoadSrcLevel(pfTexLoad* _tload);
extern DLLEXPORT void                 pfTLoadDst(pfTexLoad* _tload, void *_tex);
extern DLLEXPORT void*                pfGetTLoadDst(pfTexLoad* _tload);
extern DLLEXPORT void                 pfTLoadDstLevel(pfTexLoad* _tload, int _lvl);
extern DLLEXPORT int                  pfGetTLoadDstLevel(pfTexLoad* _tload);
extern DLLEXPORT void                 pfTLoadSize(pfTexLoad* _tload, int _w, int _h, int _d);
extern DLLEXPORT void                 pfGetTLoadSize(pfTexLoad* _tload, int *_w, int *_h, int *_d);
extern DLLEXPORT void                 pfTLoadFrame(pfTexLoad* _tload, int _frameCount);
extern DLLEXPORT int                  pfGetTLoadFrame(pfTexLoad* _tload);
extern DLLEXPORT int                  pfGetTLoadPrevLoadedTexels(pfTexLoad* _tload);
extern DLLEXPORT int                  pfGetTLoadDirty(pfTexLoad* _tload);
extern DLLEXPORT int                  pfTLoadDirty(pfTexLoad* _tload, int _dirtmask);
extern DLLEXPORT int                  pfGetTLoadStatus(pfTexLoad* _tload);
extern DLLEXPORT int                  pfApplyTLoad(pfTexLoad* _tload);
extern DLLEXPORT int                  pfApplyPartial(pfTexLoad* _tload, int _texels);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfQueue Tokens ----------------------- */
#define PFQUEUE_PROC_ACTIVE_ALWAYS		0x0
#define PFQUEUE_PROC_ACTIVE_INSIDE_RANGE	0x1
#define PFQUEUE_PROC_ACTIVE_OUTSIDE_RANGE	0x2
#define PFQUEUE_PROC_ACTIVE_LESS_THAN		0x3
#define PFQUEUE_PROC_ACTIVE_GREATER_THAN	0x4
#define PFQUEUE_PROC_EXIT			0xffffffff //special value; ugh!

extern DLLEXPORT int pfGetNumGlobalQueueServiceProcs(void);
extern DLLEXPORT int pfGetGlobalQueueServiceProcPID(int _n);
extern DLLEXPORT pfQueue* pfGetGlobalQueueServiceProcQueue(int _n);

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfQueue CAPI ------------------------------*/

extern DLLEXPORT pfQueue*             pfNewQueue(int _eltSize, int _nElts, void *arena);
extern DLLEXPORT pfType*              pfGetQueueClassType(void);
extern DLLEXPORT void                 pfQueueArrayLen(pfQueue* _queue, int _length);
extern DLLEXPORT int                  pfGetQueueArrayLen(pfQueue* _queue);
extern DLLEXPORT int                  pfGetQueueNum(pfQueue* _queue);
extern DLLEXPORT int                  pfGetQueueElementSize(pfQueue* _queue);
extern DLLEXPORT int                  pfGetQueueNumServiceProcs(pfQueue* _queue);
extern DLLEXPORT int                  pfGetQueueServiceProcPID(pfQueue* _queue, int which);
extern DLLEXPORT void                 pfQueueSortFunc(pfQueue* _queue, pfQueueSortFuncType _func);
extern DLLEXPORT pfQueueSortFuncType  pfGetQueueSortFunc(pfQueue* _queue);
extern DLLEXPORT void                 pfQueueSortMode(pfQueue* _queue, int _bool);
extern DLLEXPORT int                  pfGetQueueSortMode(pfQueue* _queue);
extern DLLEXPORT void                 pfQueueInputRange(pfQueue* _queue, int _low, int _hi);
extern DLLEXPORT void                 pfGetQueueInputRange(pfQueue* _queue, int *_low, int *_hi);
extern DLLEXPORT void                 pfQueueOutputRange(pfQueue* _queue, int _low, int _hi);
extern DLLEXPORT void                 pfGetQueueOutputRange(pfQueue* _queue, int *_low, int *_hi);
extern DLLEXPORT int                  pfGetQueueSortProcPID(pfQueue* _queue);
extern DLLEXPORT void                 pfInsertQueueElt(pfQueue* _queue, void *_elt);
extern DLLEXPORT void                 pfInsertFrontQueueElt(pfQueue* _queue, void *_elt);
extern DLLEXPORT void*                pfRemoveQueueElt(pfQueue* _queue);
extern DLLEXPORT void*                pfAttemptRemoveQueueElt(pfQueue* _queue);
extern DLLEXPORT int                  pfAddQueueServiceProc(pfQueue* _queue, pfQueueServiceFuncType _fnc);
extern DLLEXPORT int                  pfSignalAllQueueServiceProcs(pfQueue* _queue, int _count, int _token);
extern DLLEXPORT void                 pfNotifyQueueSortProc(pfQueue* _queue);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ------------------ pfFog Related Functions--------------------- */

extern  pfFog*   pfGetCurFog(void);

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfFog CAPI ------------------------------*/

extern DLLEXPORT pfFog*               pfNewFog(void *arena);
extern DLLEXPORT pfType*              pfGetFogClassType(void);
extern DLLEXPORT void                 pfFogType(pfFog* _fog, int _type);
extern DLLEXPORT int                  pfGetFogType(const pfFog* _fog);
extern DLLEXPORT void                 pfFogRange(pfFog* _fog, float _onset, float _opaque);
extern DLLEXPORT void                 pfGetFogRange(const pfFog* _fog, float* _onset, float* _opaque);
extern DLLEXPORT void                 pfFogOffsets(pfFog* _fog, float _onset, float _opaque);
extern DLLEXPORT void                 pfGetFogOffsets(const pfFog* _fog, float *_onset, float *_opaque);
extern DLLEXPORT void                 pfFogRamp(pfFog* _fog, int _points, float* _range, float* _density, float _bias);
extern DLLEXPORT void                 pfGetFogRamp(const pfFog* _fog, int* _points, float* _range, float* _density, float* _bias);
extern DLLEXPORT void                 pfFogColor(pfFog* _fog, float _r, float _g, float _b);
extern DLLEXPORT void                 pfGetFogColor(const pfFog* _fog, float* _r, float* _g, float* _b);
extern DLLEXPORT float                pfGetFogDensity(const pfFog* _fog, float _range);
extern DLLEXPORT void                 pfApplyFog(pfFog* _fog);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ------------------ pfColortable Related Functions--------------------- */

extern  pfColortable*   pfGetCurCtab(void);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfColortable CAPI ------------------------------*/

extern DLLEXPORT pfColortable*        pfNewCtab(int _size, void *arena);
extern DLLEXPORT pfType*              pfGetCtabClassType(void);
extern DLLEXPORT int                  pfGetCtabSize(const pfColortable* _ctab);
extern DLLEXPORT int                  pfCtabColor(pfColortable* _ctab, int _index, PFVEC4 _acolor);
extern DLLEXPORT int                  pfGetCtabColor(const pfColortable* _ctab, int _index, PFVEC4 _acolor);
extern DLLEXPORT pfVec4*              pfGetCtabColors(const pfColortable* _ctab);
extern DLLEXPORT void                 pfApplyCtab(pfColortable* _ctab);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfDataPool CAPI ------------------------------*/

extern DLLEXPORT pfDataPool*          pfCreateDPool(uint _size, char* _name);
extern DLLEXPORT pfDataPool*          pfAttachDPool(char* _name);
extern DLLEXPORT pfType*              pfGetDPoolClassType(void);
extern DLLEXPORT const char*          pfGetDPoolName(pfDataPool* _dpool);
extern DLLEXPORT void                 pfDPoolAttachAddr(void *_addr);
extern DLLEXPORT void*                pfGetDPoolAttachAddr(void);
extern DLLEXPORT int                  pfGetDPoolSize(pfDataPool* _dpool);
extern DLLEXPORT volatile void*       pfDPoolAlloc(pfDataPool* _dpool, unsigned int _size, int _id);
extern DLLEXPORT volatile void*       pfDPoolFind(pfDataPool* _dpool, int _id);
extern DLLEXPORT int                  pfDPoolFree(pfDataPool* _dpool, void* _dpmem);
extern DLLEXPORT int                  pfReleaseDPool(pfDataPool* _dpool);
extern DLLEXPORT int                  pfDPoolLock(void* _dpmem);
extern DLLEXPORT int                  pfDPoolSpinLock(void* _dpmem, int spins, int block);
extern DLLEXPORT void                 pfDPoolUnlock(void* _dpmem);
extern DLLEXPORT int                  pfDPoolTest(void* _dpmem);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfDispList Tokens ----------------------- */

#define PFDL_FLAT	    0
#define PFDL_RING	    1
#define PFDL_END_OF_LIST    1
#define PFDL_END_OF_FRAME   2
#define PFDL_RETURN	    3

#define PF_MAX_BANKS		32

/* pfDListMode() */
#define PFDL_MODE_COMPILE_GL	    1

/* Preprocess token */
#define PFDL_PREPROCESS_LPSTATE 0x1


/* ----------------pfDispList Related Routines------------------ */

extern pfDispList* pfGetCurDList(void);
extern void    pfDrawGLObj(GLOBJECT _obj);
	    
#ifdef DLSTATS
    if (pfCurDLStats) pfCurDLStats->cmds[cmd]++;		
#endif
extern pfDispList* pfCurDrawDList;
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfDispList CAPI ------------------------------*/

extern DLLEXPORT pfDispList*          pfNewDList(int _type, int _size, void *arena);
extern DLLEXPORT pfType*              pfGetDListClassType(void);
extern DLLEXPORT int                  pfGetDListSize(const pfDispList* _dlist);
extern DLLEXPORT void                 pfDListMode(pfDispList* _dlist, int _mode, int _val);
extern DLLEXPORT int                  pfGetDListMode(const pfDispList* _dlist, int _mode);
extern DLLEXPORT int                  pfGetDListType(const pfDispList* _dlist);
extern DLLEXPORT int                  pfDrawDList(pfDispList* _dlist);
extern DLLEXPORT int                  pfIsEmptyDList(pfDispList* _dlist);
extern DLLEXPORT int                  pfCompileDList(pfDispList* _dlist);
extern DLLEXPORT int                  pfPreprocessDList(pfDispList* _dlist, int flag);
extern DLLEXPORT void                 pfOpenDList(pfDispList* _dlist);
extern DLLEXPORT void                 pfCloseDList(void);
extern DLLEXPORT int                  pfAppendDList(pfDispList* _dlist, const pfDispList *_src);
extern DLLEXPORT void                 pfResetDList(pfDispList* _dlist);
extern DLLEXPORT void                 pfResetBanksDList(pfDispList* _dlist);
extern DLLEXPORT void                 pfAddDListCmd(int _cmd);
extern DLLEXPORT void                 pfDListCallback(pfDListFuncType _callback, int _bytes, void* _data);
extern DLLEXPORT pfGeoSet*            pfGetDListTmpGSet(pfDispList* _dlist, int myId);
extern DLLEXPORT void*                pfGetDListTmpBuffer(pfDispList* _dlist, int myId, int size);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
/* setMode Tokens */
#define	PFFONT_CHAR_SPACING	0
#define PFFONT_NUM_CHARS	1
#define PFFONT_RETURN_CHAR	2

/* setMode Values */
#define PFFONT_CHAR_SPACING_FIXED	0
#define PFFONT_CHAR_SPACING_VARIABLE	1
#define PFFONT_ASCII		128

/* setVal Tokens */
#define PFFONT_UNIT_SCALE	0

/* setAttr Tokens */
#define PFFONT_GSTATE		0
#define PFFONT_BBOX		1
#define PFFONT_SPACING		2
#define PFFONT_NAME		3
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfFont CAPI ------------------------------*/

extern DLLEXPORT pfFont*              pfNewFont(void *arena);
extern DLLEXPORT pfType*              pfGetFontClassType(void);
extern DLLEXPORT void                 pfFontCharGSet(pfFont* _font, int _ascii, pfGeoSet *_gset);
extern DLLEXPORT pfGeoSet*            pfGetFontCharGSet(pfFont* _font, int _ascii);
extern DLLEXPORT void                 pfFontCharSpacing(pfFont* _font, int _ascii, PFVEC3 _spacing);
extern DLLEXPORT const pfVec3*        pfGetFontCharSpacing(pfFont* _font, int _ascii);
extern DLLEXPORT void                 pfFontAttr(pfFont* _font, int _which, void *attr);
extern DLLEXPORT void*                pfGetFontAttr(pfFont* _font, int _which);
extern DLLEXPORT void                 pfFontVal(pfFont* _font, int _which, float _val);
extern DLLEXPORT float                pfGetFontVal(pfFont* _font, int _which);
extern DLLEXPORT void                 pfFontMode(pfFont* _font, int mode, int val);
extern DLLEXPORT int                  pfGetFontMode(pfFont* _font, int mode);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ----------------------- pfGeoSet Tokens ----------------------- */

/* primititve token numbers in gsdraw.m must match these numbers */
/* pfGSetPrimType() */
#define PFGS_POINTS	    	0
#define PFGS_LINES	    	1
#define PFGS_LINESTRIPS	    	2	
#define PFGS_TRIS	    	3
#define PFGS_QUADS	    	4
#define PFGS_TRISTRIPS	    	5
#define PFGS_FLAT_LINESTRIPS	6
#define PFGS_FLAT_TRISTRIPS	7
#define PFGS_POLYS		8
#define PFGS_TRIFANS		9
#define PFGS_FLAT_TRIFANS	10
#define PFGS_NUM_PRIMS		11

/* pfGSetAttr(), attribute types */
#define PFGS_COLOR4	    0
#define PFGS_NORMAL3	    1
#define PFGS_TEXCOORD2	    2
#define PFGS_COORD3	    3
#define PFGS_PACKED_ATTRS   5 /* needed for pfGSetDrawMode too */ 
#define PFGS_TEXCOORD3	    6 /* for vert arrrays only */

/* pfGSetAttr(), binding types */
#define PFGS_OFF	    0
#define PFGS_OVERALL	    1
#define PFGS_PER_PRIM	    2
#define PFGS_PER_VERTEX	    3

/* pfGSetDrawMode() */
#define PFGS_FLATSHADE	    	2
#define PFGS_WIREFRAME	    	3
#define PFGS_COMPILE_GL     	4
/* #define PFGS_PACKED_ATTRS  5 */
#define PFGS_DRAW_GLOBJ     	6

/* packed attr formats for pfGSetAttr() with PACKED_ATTRS */
	/* these are the fast path formats */
#define PFGS_PA_OFF		0
#define PFGS_PA_C4UBN3ST2FV3F	1
#define PFGS_PA_C4UBN3ST2F	2
#define PFGS_PA_C4UBT2F		3
#define PFGS_PA_T2S_FORMATS	4
	/* these are additional useful formats */
#define PFGS_PA_GEN_FORMATS	4
#define PFGS_PA_C4UBN3ST2SV3F	4
#define PFGS_PA_C4UBN3ST2S	5
#define PFGS_PA_C4UBT2S		6
#define PFGS_PA_T3_FORMATS	7
#define PFGS_PA_C4UBN3ST3FV3F	7
#define PFGS_PA_C4UBN3ST3F	8
#define PFGS_PA_C4UBT3F		9
#define PFGS_PA_T3S_FORMATS	10
#define PFGS_PA_C4UBN3ST3SV3F	10
#define PFGS_PA_C4UBN3ST3S	11
#define PFGS_PA_C4UBT3S		12
#define PFGS_PA_T2D_FORMATS	13
#define PFGS_PA_C4UBN3ST2DV3F	13
#define PFGS_PA_C4UBN3ST2D	14

/* pfQueryGSet() */
#define PFQGSET_NUM_TRIS	1
#define PFQGSET_NUM_VERTS	2
#define PFQGSET_NUM_QUERIES	2

/* pfGSetIsectMask() */
#define PFIS_ALL_MASK		0xffffffff /* all active mask */
#define PFIS_PICK_MASK		0x80000000 /* reserved for picking */
#define PFIS_SET_PICK		0x80000000 /* picking intersection set flag */ 
    
/* pfGSetPassFilter() */
#define PFGS_TEX_GSET            0x1
#define PFGS_NONTEX_GSET         0x2
#define PFGS_EMISSIVE_GSET       0x4
#define PFGS_NONEMISSIVE_GSET    0x8
#define PFGS_LAYER_GSET          0x10
#define PFGS_NONLAYER_GSET       0x20
#define PFGS_GSET_MASK           0xfff

/* pfGSetBBox() */
#define PFBOUND_STATIC		1
#define PFBOUND_DYNAMIC		2

/* Function for turning a pfFlux data buffer into a pfGeoSet */
extern DLLEXPORT int pfFluxedGSetInit(pfFluxMemory *_fmem);

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfGeoSet CAPI ------------------------------*/

extern DLLEXPORT pfGeoSet*            pfNewGSet(void *arena);
extern DLLEXPORT pfType*              pfGetGSetClassType(void);
extern DLLEXPORT void                 pfGSetNumPrims(pfGeoSet* _gset, int _n);
extern DLLEXPORT int                  pfGetGSetNumPrims(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetPrimType(pfGeoSet* _gset, int _type);
extern DLLEXPORT int                  pfGetGSetPrimType(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetPrimLengths(pfGeoSet* _gset, int *_lengths);
extern DLLEXPORT int*                 pfGetGSetPrimLengths(const pfGeoSet* _gset);
extern DLLEXPORT int                  pfGetGSetPrimLength(const pfGeoSet* _gset, int i);
extern DLLEXPORT void                 pfGSetAttr(pfGeoSet* _gset, int _attr, int _bind, void* _alist, unsigned short* _ilist);
extern DLLEXPORT void                 pfGSetMultiAttr(pfGeoSet* _gset, int _attr, int _index, int _bind, void* _alist, unsigned short* _ilist);
extern DLLEXPORT int                  pfGetGSetAttrBind(const pfGeoSet* _gset, int _attr);
extern DLLEXPORT int                  pfGetGSetMultiAttrBind(const pfGeoSet* _gset, int _attr, int _index);
extern DLLEXPORT void                 pfGetGSetAttrLists(const pfGeoSet* _gset, int _attr, void** _alist, unsigned short** _ilist);
extern DLLEXPORT void                 pfGetGSetMultiAttrLists(const pfGeoSet* _gset, int _attr, int _index, void** _alist, unsigned short** _ilist);
extern DLLEXPORT int                  pfGetGSetAttrRange(const pfGeoSet* _gset, int _attr, int *_min, int *_max);
extern DLLEXPORT int                  pfGetGSetMultiAttrRange(const pfGeoSet* _gset, int _attr, int _index, int *_min, int *_max);
extern DLLEXPORT void                 pfGSetDrawMode(pfGeoSet* _gset, int _mode, int _val);
extern DLLEXPORT int                  pfGetGSetDrawMode(const pfGeoSet* _gset, int _mode);
extern DLLEXPORT void                 pfGSetGState(pfGeoSet* _gset, pfGeoState *_gstate);
extern DLLEXPORT pfGeoState*          pfGetGSetGState(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetGStateIndex(pfGeoSet* _gset, int _id);
extern DLLEXPORT int                  pfGetGSetGStateIndex(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetHlight(pfGeoSet* _gset, pfHighlight *_hlight);
extern DLLEXPORT pfHighlight*         pfGetGSetHlight(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetDecalPlane(pfGeoSet* _gset, pfPlane *_plane);
extern DLLEXPORT pfPlane*             pfGetGSetDecalPlane(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetLineWidth(pfGeoSet* _gset, float _width);
extern DLLEXPORT float                pfGetGSetLineWidth(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetPntSize(pfGeoSet* _gset, float _s);
extern DLLEXPORT float                pfGetGSetPntSize(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetIsectMask(pfGeoSet* _gset, unsigned int _mask, int _setMode, int _bitOp);
extern DLLEXPORT unsigned int         pfGetGSetIsectMask(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetDrawBin(pfGeoSet* _gset, short bin);
extern DLLEXPORT int                  pfGetGSetDrawBin(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetDrawOrder(pfGeoSet* _gset, unsigned int order);
extern DLLEXPORT unsigned int         pfGetGSetDrawOrder(const pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetAppearance(pfGeoSet* _gset, islAppearance *appearance_);
extern DLLEXPORT islAppearance*       pfGetGSetAppearance(const pfGeoSet* _gset);
extern DLLEXPORT int                  pfGSetIsShaded(pfGeoSet* _gset);
extern DLLEXPORT int                  pfGetGSetNumTextures(pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetOptimize(pfGeoSet* _gset, int _state);
extern DLLEXPORT int                  pfGetGSetOptimize(pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetBBox(pfGeoSet* _gset, pfBox* _box, int _mode);
extern DLLEXPORT int                  pfGetGSetBBox(pfGeoSet* _gset, pfBox* _box);
extern DLLEXPORT void                 pfGSetBBoxFlux(pfGeoSet* _gset, pfFlux* _flux);
extern DLLEXPORT pfFlux*              pfGetGSetBBoxFlux(pfGeoSet* _gset);
extern DLLEXPORT void                 pfHideGSetStripPrim(pfGeoSet* _gset, int i);
extern DLLEXPORT void                 pfUnhideGSetStripPrim(pfGeoSet* _gset, int i);
extern DLLEXPORT int                  pfIsGSetStripPrimHidden(pfGeoSet* _gset, int i);
extern DLLEXPORT void                 pfGSetUpdateCteRefs(pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetCalcTexBBox(pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetTexBBox_i(pfGeoSet* _gset,  uint centerS, uint centerT, uint halfwidth, uint halfheight );
extern DLLEXPORT void                 pfGSetTexBBox_f(pfGeoSet* _gset,  float minS, float maxS, float minT, float maxT  );
extern DLLEXPORT int                  pfGetGSetTexBBox_i(pfGeoSet* _gset,  uint* centerS, uint* centerT, uint* halfwidth, uint* halfheight );
extern DLLEXPORT int                  pfGetGSetTexBBox_f(pfGeoSet* _gset,  float* minS, float* maxS, float* minT, float* maxT  );
extern DLLEXPORT void                 pfGSetCteAttr(pfGeoSet* _gset,  int which, void* val );
extern DLLEXPORT void*                pfGetGSetCteAttr(pfGeoSet* _gset,  int which );
extern DLLEXPORT void                 pfQuickCopyGSet(pfGeoSet* _gset, pfGeoSet *src);
extern DLLEXPORT void                 pfpfGSetQuickAttr(pfGeoSet* _gset, int _attr, void* _alist, unsigned short* _ilist);
extern DLLEXPORT void                 pfpfGSetQuickMultiAttr(pfGeoSet* _gset, int _attr, int _index, void* _alist, unsigned short* _ilist);
extern DLLEXPORT void                 pfpfGSetQuickPrimLengths(pfGeoSet* _gset, int *_lengths);
extern DLLEXPORT void                 pfQuickResetGSet(pfGeoSet* _gset, int extRefOnly);
extern DLLEXPORT void                 pfDrawGSet(pfGeoSet* _gset);
extern DLLEXPORT void                 pfDrawBBGSet(pfGeoSet* _gset);
extern DLLEXPORT void                 pfCompileGSet(pfGeoSet* _gset);
extern DLLEXPORT int                  pfQueryGSet(const pfGeoSet* _gset, unsigned int _which, void *_dst);
extern DLLEXPORT int                  pfMQueryGSet(const pfGeoSet* _gset, unsigned int *_which, void *_dst);
extern DLLEXPORT int                  pfGSetIsectSegs(pfGeoSet* _gset, pfSegSet *segSet, pfHit **hits[]);
extern DLLEXPORT void                 pfDrawHlightedGSet(pfGeoSet* _gset);
extern DLLEXPORT void                 pfGSetPassFilter(uint _mask);
extern DLLEXPORT uint                 pfGetGSetPassFilter(void);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfHit CAPI ------------------------------*/

extern DLLEXPORT pfType*              pfGetHitClassType(void);
extern DLLEXPORT int                  pfQueryHit(const pfHit* _hit, unsigned int _which, void *_dst);
extern DLLEXPORT int                  pfMQueryHit(const pfHit* _hit, unsigned int *_which, void *_dst);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfHits CAPI ------------------------------*/

extern DLLEXPORT pfType*              pfGetHitsClassType(void);
extern DLLEXPORT int                  pfHitsCheckNewProcess(int *outPidIndex);
extern DLLEXPORT void                 pfHitsReset(pfSegSet *segSet, pfHit **hits[]);
extern DLLEXPORT void                 pfHitsAdd(pfSegSet *segSet, pfHit **hits[], pfHit *newHit, int segmentId);
extern DLLEXPORT int                  pfGetHitsNofHits(pfSegSet *segSet, pfHit **hits[]);
extern DLLEXPORT pfHit*               pfGetHitsSpareHitAtom(void);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ------------------ pfHit Tokens --------------------- */

#define PFQHIT_FLAGS		1
#define PFQHIT_SEGNUM		2
#define PFQHIT_SEG		3
#define PFQHIT_POINT		4
#define PFQHIT_NORM		5
#define PFQHIT_VERTS		6
#define PFQHIT_TRI		7
#define PFQHIT_PRIM		8
#define PFQHIT_GSET		9
#define PFQHIT_STRING		10
#define PFQHIT_CHARINDEX	11
#define PFQHIT_FACEINDEX	12


/* pfHit flag values */
#define PFHIT_NONE		0x00 /* no intersection */
#define PFHIT_ISECT		0x01 /* intersection */
#define PFHIT_POINT		0x02 /* point info valid */
#define PFHIT_NORM		0x04 /* normal info valid */
#define PFHIT_TRI		0x08 /* triangle index valid */
#define PFHIT_PRIM		0x10 /* primitive index valid */
#define PFHIT_VERTS		0x20 /* triangle vertex info valid */
#endif /* !PF_CPLUSPLUS_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */

/* ----------------pfGeoState Related Functions ------------------ */

extern  pfGeoState*   pfGetCurGState(void);
extern  pfGeoState*   pfGetCurIndexedGState(int _index);
extern  pfList*	      pfGetCurGStateTable(void);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfGeoState CAPI ------------------------------*/

extern DLLEXPORT pfGeoState*          pfNewGState(void *arena);
extern DLLEXPORT pfType*              pfGetGStateClassType(void);
extern DLLEXPORT void                 pfGStateMode(pfGeoState* _gstate, uint64_t _attr, int _a);
extern DLLEXPORT int                  pfGetGStateMode(const pfGeoState* _gstate, uint64_t _attr);
extern DLLEXPORT int                  pfGetGStateCurMode(const pfGeoState* _gstate, uint64_t _attr);
extern DLLEXPORT int                  pfGetGStateCombinedMode(const pfGeoState* _gstate, uint64_t _attr, const pfGeoState *_combState);
extern DLLEXPORT void                 pfGStateMultiMode(pfGeoState* _gstate, uint64_t _attr, int _index, int _a);
extern DLLEXPORT int                  pfGetGStateMultiMode(const pfGeoState* _gstate, uint64_t _attr, int _index);
extern DLLEXPORT int                  pfGetGStateCurMultiMode(const pfGeoState* _gstate, uint64_t _attr, int _index);
extern DLLEXPORT int                  pfGetGStateCombinedMultiMode(const pfGeoState* _gstate, uint64_t _attr, int _index, const pfGeoState *_combState);
extern DLLEXPORT void                 pfGStateVal(pfGeoState* _gstate, uint64_t _attr, float _a);
extern DLLEXPORT float                pfGetGStateVal(const pfGeoState* _gstate, uint64_t _attr);
extern DLLEXPORT float                pfGetGStateCurVal(const pfGeoState* _gstate, uint64_t _attr);
extern DLLEXPORT float                pfGetGStateCombinedVal(const pfGeoState* _gstate, uint64_t _attr, const pfGeoState *_combState);
extern DLLEXPORT void                 pfGStateMultiVal(pfGeoState* _gstate, uint64_t _attr, float *_values);
extern DLLEXPORT int                  pfGetGStateMultiVal(pfGeoState* _gstate, uint64_t _attr, float *_values);
extern DLLEXPORT int                  pfGetGStateCurMultiVal(pfGeoState* _gstate, uint64_t _attr, float *_values);
extern DLLEXPORT void                 pfGStateInherit(pfGeoState* _gstate, uint64_t _mask);
extern DLLEXPORT uint64_t             pfGetGStateInherit(const pfGeoState* _gstate);
extern DLLEXPORT void                 pfGStateAttr(pfGeoState* _gstate, uint64_t _attr, void* _a);
extern DLLEXPORT void*                pfGetGStateAttr(const pfGeoState* _gstate, uint64_t _attr);
extern DLLEXPORT void*                pfGetGStateCurAttr(const pfGeoState* _gstate, uint64_t _attr);
extern DLLEXPORT void*                pfGetGStateCombinedAttr(const pfGeoState* _gstate, uint64_t _attr, const pfGeoState *_combState);
extern DLLEXPORT void                 pfGStateMultiAttr(pfGeoState* _gstate, uint64_t _attr, int _index, void* _a);
extern DLLEXPORT void*                pfGetGStateMultiAttr(const pfGeoState* _gstate, uint64_t _attr, int _index);
extern DLLEXPORT void*                pfGetGStateCurMultiAttr(const pfGeoState* _gstate, uint64_t _attr, int _index);
extern DLLEXPORT void*                pfGetGStateCombinedMultiAttr(const pfGeoState* _gstate, uint64_t _attr, int _index, const pfGeoState *_combState);
extern DLLEXPORT void                 pfGStateFuncs(pfGeoState* _gstate, pfGStateFuncType _preFunc, pfGStateFuncType _postFunc, void *_data);
extern DLLEXPORT void                 pfGetGStateFuncs(const pfGeoState* _gstate, pfGStateFuncType *_preFunc, pfGStateFuncType *_postFunc, void **_data);
extern DLLEXPORT int                  pfGetGStateNumTextures(pfGeoState* _gstate);
extern DLLEXPORT void                 pfLoadGState(pfGeoState* _gstate);
extern DLLEXPORT void                 pfApplyGState(pfGeoState* _gstate);
extern DLLEXPORT void                 pfMakeBasicGState(pfGeoState* _gstate);
extern DLLEXPORT void                 pfApplyGStateTable(pfList *_gstab);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */


/* ------------------ pfHighlight Tokens--------------------- */

/* pfGSetHlight() */
#define PFHL_INHERIT	    ((pfHighlight *) 0)
#define PFHL_ON		    ((pfHighlight *) 1)

/* pfHlightMode() */
#define PFHL_OFF	    0

#define PFHL_LINES	    0x001
#define PFHL_LINES_R	    (0x002 | PFHL_LINES) /* reverse fg and bf colors for lines */
#define PFHL_LINESPAT	    (0x004 | PFHL_LINES)
#define PFHL_LINESPAT2	    (0x008 | PFHL_LINESPAT) /* 2-pass patterning */
#define PFHL_LINESMASK	    0x00f

#define PFHL_FILL	    0x0010
#define PFHL_FILLPAT	    (0x0020 | PFHL_FILL)
#define PFHL_FILLPAT2	    (0x0040 | PFHL_FILLPAT) /* 2-pass patterning */
#define PFHL_FILLTEX	    (0x080 | PFHL_FILL)  /* tex overrides patterning  */
#define PFHL_FILL_R	    0x0100 /* reverse fg and bf colors for fill */
#define PFHL_FILLMASK	    0x01f0

#define PFHL_SKIP_BASE      0x0200
#define PFHL_POINTS	    0x0400
#define PFHL_NORMALS	    0x0800
#define PFHL_BBOX_LINES	    0x1000
#define PFHL_BBOX_FILL	    0x2000

#define PFHL_MASK	    0x3fff

/* pfHlightColor() */
#define PFHL_FGCOLOR	    0x1
#define PFHL_BGCOLOR	    0x2
/* pfHlightPat() */
#define PFHL_FGPAT    	    0x1
#define PFHL_BGPAT    	    0x2

/* ------------------ pfHighlight Related Functions--------------------- */

extern pfHighlight *pfGetCurHlight(void);

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfHighlight CAPI ------------------------------*/

extern DLLEXPORT pfHighlight*         pfNewHlight(void *arena);
extern DLLEXPORT pfType*              pfGetHlightClassType(void);
extern DLLEXPORT void                 pfHlightMode(pfHighlight* _hlight, unsigned int _mode);
extern DLLEXPORT unsigned int         pfGetHlightMode(const pfHighlight* _hlight);
extern DLLEXPORT pfGeoState*          pfGetHlightGState(const pfHighlight* _hlight);
extern DLLEXPORT void                 pfHlightGState(pfHighlight* _hlight, pfGeoState *_gstate);
extern DLLEXPORT void                 pfHlightGStateIndex(pfHighlight* _hlight, int _id);
extern DLLEXPORT int                  pfGetHlightGStateIndex(const pfHighlight* _hlight);
extern DLLEXPORT void                 pfHlightColor(pfHighlight* _hlight, unsigned int _which, float _r, float _g, float _b);
extern DLLEXPORT void                 pfGetHlightColor(const pfHighlight* _hlight, unsigned int _which, float *_r, float *_g, float *_b);
extern DLLEXPORT void                 pfHlightAlpha(pfHighlight* _hlight, float _a);
extern DLLEXPORT float                pfGetHlightAlpha(const pfHighlight* _hlight);
extern DLLEXPORT void                 pfHlightNormalLength(pfHighlight* _hlight, float _len, float _bboxScale);
extern DLLEXPORT void                 pfGetHlightNormalLength(const pfHighlight* _hlight, float *_len, float *_bboxScale);
extern DLLEXPORT void                 pfHlightLineWidth(pfHighlight* _hlight,  float _width );
extern DLLEXPORT float                pfGetHlightLineWidth(const pfHighlight* _hlight);
extern DLLEXPORT void                 pfHlightPntSize(pfHighlight* _hlight,  float _size );
extern DLLEXPORT float                pfGetHlightPntSize(const pfHighlight* _hlight);
extern DLLEXPORT void                 pfHlightLinePat(pfHighlight* _hlight, int _which, unsigned short _pat);
extern DLLEXPORT unsigned short       pfGetHlightLinePat(const pfHighlight* _hlight, int _which);
extern DLLEXPORT void                 pfHlightFillPat(pfHighlight* _hlight,  int _which, unsigned int *fillPat );
extern DLLEXPORT void                 pfGetHlightFillPat(const pfHighlight* _hlight, int _which, unsigned int *_pat);
extern DLLEXPORT void                 pfHlightTex(pfHighlight* _hlight, pfTexture *_tex);
extern DLLEXPORT pfTexture*           pfGetHlightTex(const pfHighlight* _hlight);
extern DLLEXPORT void                 pfHlightTEnv(pfHighlight* _hlight, pfTexEnv *_tev);
extern DLLEXPORT pfTexEnv*            pfGetHlightTEnv(const pfHighlight* _hlight);
extern DLLEXPORT void                 pfHlightTGen(pfHighlight* _hlight, pfTexGen *_tgen);
extern DLLEXPORT pfTexGen*            pfGetHlightTGen(const pfHighlight* _hlight);
extern DLLEXPORT void                 pfApplyHlight(pfHighlight* _hlight);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  */
#endif /* !PF_CPLUSPLUS_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ------------------ pfLight Related Functions--------------------- */

extern  int   pfGetCurLights(pfLight *_lights[PF_MAX_LIGHTS]);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfLight CAPI ------------------------------*/

extern DLLEXPORT pfLight*             pfNewLight(void *arena);
extern DLLEXPORT pfType*              pfGetLightClassType(void);
extern DLLEXPORT void                 pfLightColor(pfLight* _light, int _which, float _r, float _g, float _b);
extern DLLEXPORT void                 pfGetLightColor(const pfLight* _light, int _which, float* _r, float* _g, float* _b);
extern DLLEXPORT void                 pfLightAmbient(pfLight* _light, float _r, float _g, float _b);
extern DLLEXPORT void                 pfGetLightAmbient(const pfLight* _light, float* _r, float* _g, float* _b);
extern DLLEXPORT void                 pfLightPos(pfLight* _light, float _x, float _y, float _z, float _w);
extern DLLEXPORT void                 pfGetLightPos(const pfLight* _light, float* _x, float* _y, float* _z, float* _w);
extern DLLEXPORT void                 pfLightAtten(pfLight* _light, float _a0, float _a1, float _a2);
extern DLLEXPORT void                 pfGetLightAtten(const pfLight* _light, float* _a0, float* _a1, float* _a2);
extern DLLEXPORT void                 pfSpotLightDir(pfLight* _light, float _x, float _y, float _z);
extern DLLEXPORT void                 pfGetSpotLightDir(const pfLight* _light, float* _x, float* _y, float* _z);
extern DLLEXPORT void                 pfSpotLightCone(pfLight* _light, float _f1, float _f2);
extern DLLEXPORT void                 pfGetSpotLightCone(const pfLight* _light, float* _f1, float* _f2);
extern DLLEXPORT void                 pfSpotLightCutoffDelta(pfLight* _light, float _f1);
extern DLLEXPORT float                pfGetSpotLightCutoffDelta(const pfLight* _light);
extern DLLEXPORT void                 pfLightOn(pfLight* _light);
extern DLLEXPORT void                 pfLightOff(pfLight* _light);
extern DLLEXPORT int                  pfIsLightOn(pfLight* _light);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ------------------ pfLightModel Related Functions--------------------- */

extern  pfLightModel*   pfGetCurLModel(void);

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfLightModel CAPI ------------------------------*/

extern DLLEXPORT pfLightModel*        pfNewLModel(void *arena);
extern DLLEXPORT pfType*              pfGetLModelClassType(void);
extern DLLEXPORT void                 pfLModelLocal(pfLightModel* _lmodel, int _l);
extern DLLEXPORT int                  pfGetLModelLocal(const pfLightModel* _lmodel);
extern DLLEXPORT void                 pfLModelTwoSide(pfLightModel* _lmodel, int _t);
extern DLLEXPORT int                  pfGetLModelTwoSide(const pfLightModel* _lmodel);
extern DLLEXPORT void                 pfLModelColorControl(pfLightModel* _lmodel, int _c);
extern DLLEXPORT int                  pfGetLModelColorControl(const pfLightModel* _lmodel);
extern DLLEXPORT void                 pfLModelAmbient(pfLightModel* _lmodel, float _r, float _g, float _b);
extern DLLEXPORT void                 pfGetLModelAmbient(const pfLightModel* _lmodel, float* _r, float* _g, float* _b);
extern DLLEXPORT void                 pfLModelAtten(pfLightModel* _lmodel, float _a0, float _a1, float _a2);
extern DLLEXPORT void                 pfGetLModelAtten(const pfLightModel* _lmodel, float* _a0, float* _a1, float* _a2);
extern DLLEXPORT void                 pfApplyLModel(pfLightModel* _lmodel);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ------------------ pfLPointState Tokens --------------------- */

/* pfLPStateMode() */

#define PFLPS_SIZE_MODE         10
#define PFLPS_SIZE_MODE_ON      1
#define PFLPS_SIZE_MODE_OFF     0

#define PFLPS_TRANSP_MODE       20
#define PFLPS_TRANSP_MODE_ON    1
#define PFLPS_TRANSP_MODE_OFF   0
#define PFLPS_TRANSP_MODE_ALPHA 1
#define PFLPS_TRANSP_MODE_TEX   2       

#define PFLPS_FOG_MODE          30
#define PFLPS_FOG_MODE_ON       1
#define PFLPS_FOG_MODE_OFF      0
#define PFLPS_FOG_MODE_ALPHA    1
#define PFLPS_FOG_MODE_TEX      2

#define PFLPS_RANGE_MODE        40
#define PFLPS_RANGE_MODE_DEPTH  0
#define PFLPS_RANGE_MODE_TRUE   1

#define PFLPS_DIR_MODE          50
#define PFLPS_DIR_MODE_ON       1
#define PFLPS_DIR_MODE_OFF      0
#define PFLPS_DIR_MODE_ALPHA    1
#define PFLPS_DIR_MODE_TEX      2

#define PFLPS_SHAPE_MODE        60
#define PFLPS_SHAPE_MODE_UNI    0
#define PFLPS_SHAPE_MODE_BI     1
#define PFLPS_SHAPE_MODE_BI_COLOR       2

#define PFLPS_DRAW_MODE		70
#define PFLPS_DRAW_MODE_RASTER	0
#define PFLPS_DRAW_MODE_CALLIGRAPHIC	1

#define PFLPS_QUALITY_MODE	80
#define PFLPS_QUALITY_MODE_HIGH 0
#define PFLPS_QUALITY_MODE_MEDIUM 1
#define PFLPS_QUALITY_MODE_LOW 2

#define PFLPS_CALLBACK_MODE	 90
#define PFLPS_CALLBACK_MODE_OFF  0
#define PFLPS_CALLBACK_MODE_PRE  1
#define PFLPS_CALLBACK_MODE_POST 2

#define PFLPS_DEBUNCHING_MODE	  91
#define PFLPS_DEBUNCHING_MODE_ON  1
#define PFLPS_DEBUNCHING_MODE_OFF 0


/* pfLPStateVal() */

#define PFLPS_SIZE_MIN_PIXEL    100
#define PFLPS_SIZE_ACTUAL       101
#define PFLPS_SIZE_MAX_PIXEL    102

#define PFLPS_TRANSP_PIXEL_SIZE 200     
#define PFLPS_TRANSP_EXPONENT   201
#define PFLPS_TRANSP_SCALE      202
#define PFLPS_TRANSP_CLAMP      203

#define PFLPS_FOG_SCALE         300
#define PFLPS_INTENSITY         400
#define PFLPS_SIZE_DIFF_THRESH  500

#define PFLPS_SIGNIFICANCE	600

#define PFLPS_MIN_DEFOCUS	700
#define PFLPS_MAX_DEFOCUS	701

extern pfLPointState* pfGetCurLPState(void);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfLPointState CAPI ------------------------------*/

extern DLLEXPORT pfLPointState*       pfNewLPState(void *arena);
extern DLLEXPORT pfType*              pfGetLPStateClassType(void);
extern DLLEXPORT void                 pfLPStateMode(pfLPointState* _lpstate, int _mode, int _val);
extern DLLEXPORT int                  pfGetLPStateMode(const pfLPointState* _lpstate, int _mode);
extern DLLEXPORT void                 pfLPStateVal(pfLPointState* _lpstate, int _attr, float _val);
extern DLLEXPORT float                pfGetLPStateVal(const pfLPointState* _lpstate, int _attr);
extern DLLEXPORT void                 pfLPStateShape(pfLPointState* _lpstate, float _horiz, float _vert, float _roll, float _falloff, float _ambient);
extern DLLEXPORT void                 pfGetLPStateShape(const pfLPointState* _lpstate, float *_horiz, float *_vert, float *_roll, float *_falloff, float *_ambient);
extern DLLEXPORT void                 pfLPStateBackColor(pfLPointState* _lpstate, float r, float g, float b, float a);
extern DLLEXPORT void                 pfGetLPStateBackColor(pfLPointState* _lpstate, float *r, float *g, float *b, float *a);
extern DLLEXPORT void                 pfRasterFunc(pfLPointState* _lpstate, pfRasterFuncType _rasterCallback, void *_data);
extern DLLEXPORT void                 pfGetRasterFunc(pfLPointState* _lpstate, pfRasterFuncType *_rasterCallback, void **_data);
extern DLLEXPORT void                 pfCalligFunc(pfLPointState* _lpstate, pfCalligFuncType _calligraphicCallback, void *_data);
extern DLLEXPORT void                 pfGetCalligFunc(pfLPointState* _lpstate, pfCalligFuncType *_calligraphicCallback, void **_data);
extern DLLEXPORT void                 pfApplyLPState(pfLPointState* _lpstate);
extern DLLEXPORT void                 pfMakeLPStateRangeTex(pfLPointState* _lpstate, pfTexture *_tex, int _size, pfFog* _fog);
extern DLLEXPORT void                 pfMakeLPStateShapeTex(pfLPointState* _lpstate, pfTexture *_tex, int _size);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* pfMaterial Sides */
#define PFMTL_FRONT	    	0x1
#define PFMTL_BACK	    	0x2
#define PFMTL_BOTH		0x3

/* ------------------ pfMaterial Related Functions--------------------- */

extern  pfMaterial*   pfGetCurMtl(int _side);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfMaterial CAPI ------------------------------*/

extern DLLEXPORT pfMaterial*          pfNewMtl(void *arena);
extern DLLEXPORT pfType*              pfGetMtlClassType(void);
extern DLLEXPORT void                 pfMtlSide(pfMaterial* _mtl, int _side);
extern DLLEXPORT int                  pfGetMtlSide(pfMaterial* _mtl);
extern DLLEXPORT void                 pfMtlAlpha(pfMaterial* _mtl, float _alpha);
extern DLLEXPORT float                pfGetMtlAlpha(pfMaterial* _mtl);
extern DLLEXPORT void                 pfMtlShininess(pfMaterial* _mtl, float _shininess);
extern DLLEXPORT float                pfGetMtlShininess(pfMaterial* _mtl);
extern DLLEXPORT void                 pfMtlColor(pfMaterial* _mtl, int _acolor, float _r, float _g, float _b);
extern DLLEXPORT void                 pfGetMtlColor(pfMaterial* _mtl, int _acolor, float* _r, float* _g, float* _b);
extern DLLEXPORT void                 pfMtlColorMode(pfMaterial* _mtl, int _side, int _mode);
extern DLLEXPORT int                  pfGetMtlColorMode(pfMaterial* _mtl, int _side);
extern DLLEXPORT void                 pfApplyMtl(pfMaterial* _mtl);
extern DLLEXPORT void                 pfMtlFullCopy(pfMaterial* _mtl, pfMaterial *src);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to INTERNAL CAPI */
/* ----------------------- pfSprite Tokens ----------------------- */

/* pfSpriteMode() */
#define PFSPRITE_ROT			0

#define PFSPRITE_AXIAL_ROT		0
#define PFSPRITE_POINT_ROT_EYE		1
#define PFSPRITE_POINT_ROT_WORLD	2

#define PFSPRITE_MATRIX_THRESHOLD	20

/* ------------------ pfSprite Related Functions--------------------- */

extern  pfSprite*   pfGetCurSprite(void);

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfSprite CAPI ------------------------------*/

extern DLLEXPORT pfSprite*            pfNewSprite(void *arena);
extern DLLEXPORT pfType*              pfGetSpriteClassType(void);
extern DLLEXPORT void                 pfSpriteMode(pfSprite* _sprite, int _which, int _val);
extern DLLEXPORT int                  pfGetSpriteMode(const pfSprite* _sprite, int _which);
extern DLLEXPORT void                 pfSpriteAxis(pfSprite* _sprite, float _x, float _y, float _z);
extern DLLEXPORT void                 pfGetSpriteAxis(pfSprite* _sprite, float *_x, float *_y, float *_z);
extern DLLEXPORT void                 pfBeginSprite(pfSprite* _sprite);
extern DLLEXPORT void                 pfEndSprite(void);
extern DLLEXPORT void                 pfPositionSprite(float _x, float _y, float _z);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ------------------ pfState Related Functions--------------------- */

extern DLLEXPORT void		pfInitState(usptr_t* _arena);
extern DLLEXPORT pfState*	pfGetCurState(void);
extern DLLEXPORT void		pfPushState(void);
extern DLLEXPORT void		pfPopState(void);
extern DLLEXPORT void		pfGetState(pfGeoState *_gstate);
extern DLLEXPORT void		pfFlushState(void);
extern DLLEXPORT void		pfBasicState(void);
extern DLLEXPORT void		pfOverride(uint64_t _mask, int _val);
extern DLLEXPORT uint64_t 	pfGetOverride(void);

extern DLLEXPORT void		pfModelMat(PFMATRIX _mat);
extern DLLEXPORT void		pfGetModelMat(PFMATRIX _mat);
extern DLLEXPORT void		pfViewMat(PFMATRIX _mat);
extern DLLEXPORT void		pfGetViewMat(PFMATRIX _mat);
extern DLLEXPORT void		pfInvViewMat(PFMATRIX _mat);
extern DLLEXPORT void		pfGetInvViewMat(PFMATRIX _mat);
extern DLLEXPORT void     	pfProjMat(PFMATRIX _mat);
extern DLLEXPORT void		pfGetProjMat(PFMATRIX _mat);
extern DLLEXPORT void		pfTexMat(PFMATRIX _mat);
extern DLLEXPORT void		pfGetTexMat(PFMATRIX _mat);
extern DLLEXPORT void 		pfInvModelMat(PFMATRIX _mat);
extern DLLEXPORT void 		pfGetInvModelMat(PFMATRIX _mat);
extern DLLEXPORT void		pfPixScale(float _pscale);
extern DLLEXPORT float		pfGetPixScale(void);
extern DLLEXPORT void		pfNearPixDist(float _pd);
extern DLLEXPORT float		pfGetNearPixDist(void);

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfState CAPI ------------------------------*/

extern DLLEXPORT pfState*             pfNewState(void *arena);
extern DLLEXPORT pfType*              pfGetStateClassType(void);
extern DLLEXPORT void                 pfSelectState(pfState* _state);
extern DLLEXPORT void                 pfLoadState(pfState* _state);
extern DLLEXPORT void                 pfAttachState(pfState* _state, pfState *_state1);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
/* ----------------------- pfString Tokens ----------------------- */
/* pfStringMode() */
#define PFSTR_JUSTIFY		10
#define PFSTR_FIRST		10
#define PFSTR_MIDDLE		11
#define PFSTR_LAST		12
#define PFSTR_LEFT		PFSTR_FIRST
#define PFSTR_CENTER		PFSTR_MIDDLE
#define PFSTR_RIGHT		PFSTR_LAST

/* pfStringMode() */
#define PFSTR_CHAR_SIZE		30
#define PFSTR_CHAR		1
#define PFSTR_SHORT		2
#define PFSTR_INT		4

/* pfStringMode() */
#define PFSTR_AUTO_SPACING	40
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfString CAPI ------------------------------*/

extern DLLEXPORT pfString*            pfNewString(void *arena);
extern DLLEXPORT pfType*              pfGetStringClassType(void);
extern DLLEXPORT size_t               pfGetStringStringLength(const pfString* _string);
extern DLLEXPORT void                 pfStringMode(pfString* _string, int _mode, int _val);
extern DLLEXPORT int                  pfGetStringMode(const pfString* _string, int _mode);
extern DLLEXPORT void                 pfStringFont(pfString* _string, pfFont* _fnt);
extern DLLEXPORT pfFont*              pfGetStringFont(const pfString* _string);
extern DLLEXPORT void                 pfStringString(pfString* _string, const char* _cstr);
extern DLLEXPORT const char*          pfGetStringString(const pfString* _string);
extern DLLEXPORT const pfGeoSet*      pfGetStringCharGSet(const pfString* _string, int _index);
extern DLLEXPORT const pfVec3*        pfGetStringCharPos(const pfString* _string, int _index);
extern DLLEXPORT void                 pfStringSpacingScale(pfString* _string, float sx, float sy, float sz);
extern DLLEXPORT void                 pfGetStringSpacingScale(const pfString* _string, float *sx, float *sy, float *sz);
extern DLLEXPORT void                 pfStringGState(pfString* _string, pfGeoState *gs);
extern DLLEXPORT pfGeoState*          pfGetStringGState(const pfString* _string);
extern DLLEXPORT void                 pfStringColor(pfString* _string, float r,float g, float b, float a);
extern DLLEXPORT void                 pfGetStringColor(const pfString* _string, float *r, float *g, float *b, float *a);
extern DLLEXPORT void                 pfStringBBox(pfString* _string, const pfBox* newbox);
extern DLLEXPORT const pfBox*         pfGetStringBBox(const pfString* _string);
extern DLLEXPORT void                 pfStringMat(pfString* _string, const PFMATRIX _mat);
extern DLLEXPORT void                 pfGetStringMat(const pfString* _string, PFMATRIX _mat);
extern DLLEXPORT void                 pfStringIsectMask(pfString* _string, uint _mask, int _setMode, int _bitOp);
extern DLLEXPORT uint                 pfGetStringIsectMask(const pfString* _string);
extern DLLEXPORT void                 pfDrawString(pfString* _string);
extern DLLEXPORT void                 pfFlattenString(pfString* _string);
extern DLLEXPORT int                  pfStringIsectSegs(pfString* _string, pfSegSet *segSet, pfHit **hits[]);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfTexture Tokens ----------------------- */

#define PFTEX_DEFAULT			(0x7fff)
#define PFTEX_LEVEL_BASE		(PFTEX_DEFAULT-1)
#define PFTEX_LEVEL_ALL			(PFTEX_DEFAULT-2)

/* Bitmasks to create min/mag filters for pfTexFilter 
 *  LE/GE | add/mod | interp | bicubic/mipmap/detail/sharp | alpha/color | /min/mag
 */
#define	PFTEX_FAST			0x80000000
#define PFTEX_MAGFILTER			0x00000001
#define PFTEX_MINFILTER			0x00000002
#define PFTEX_COLOR			0x00000004
#define PFTEX_ALPHA			0x00000008
#define PFTEX_MIPMAP			0x00000010
#define PFTEX_SHARPEN			0x00000020
#define PFTEX_DETAIL			0x00000040
#define PFTEX_MODULATE			0x00000100
#define PFTEX_ADD			0x00000200
#define PFTEX_POINT			0x00001000
#define PFTEX_LINEAR			0x00002000
#define PFTEX_BILINEAR			0x00004000
#define PFTEX_TRILINEAR			0x00008000
#define PFTEX_QUADLINEAR		0x00010000
#define PFTEX_CLIPMAP			0x00020000
#define PFTEX_MIPMAP_POINT		(PFTEX_MIPMAP | PFTEX_POINT)
#define PFTEX_MIPMAP_LINEAR		(PFTEX_MIPMAP | PFTEX_LINEAR)
#define PFTEX_MIPMAP_BILINEAR		(PFTEX_MIPMAP | PFTEX_BILINEAR)
#define PFTEX_CLIPMAP_TRILINEAR		(PFTEX_CLIPMAP | PFTEX_TRILINEAR)
#define PFTEX_MIPMAP_TRILINEAR		(PFTEX_MIPMAP | PFTEX_TRILINEAR)
#define PFTEX_MIPMAP_QUADLINEAR		(PFTEX_MIPMAP | PFTEX_QUADLINEAR)
#define PFTEX_MAGFILTER_COLOR		(PFTEX_MAGFILTER | PFTEX_COLOR)
#define PFTEX_MAGFILTER_ALPHA		(PFTEX_MAGFILTER | PFTEX_ALPHA)
#define PFTEX_SHARPEN_COLOR		(PFTEX_SHARPEN | PFTEX_COLOR)
#define PFTEX_SHARPEN_ALPHA		(PFTEX_SHARPEN | PFTEX_ALPHA)
#define PFTEX_DETAIL_COLOR		(PFTEX_DETAIL | PFTEX_COLOR)
#define PFTEX_DETAIL_ALPHA		(PFTEX_DETAIL | PFTEX_ALPHA)
#define PFTEX_DETAIL_LINEAR		(PFTEX_DETAIL | PFTEX_LINEAR)
#define PFTEX_MODULATE_DETAIL		(PFTEX_DETAIL | PFTEX_MODULATE)
#define PFTEX_ADD_DETAIL		(PFTEX_DETAIL | PFTEX_ADD)


/* IRIS GL only filters */
#define PFTEX_BICUBIC			0x00000080
#define PFTEX_LEQUAL			0x00000400
#define PFTEX_GEQUAL			0x00000800
#define PFTEX_BICUBIC_LEQUAL		(PFTEX_BICUBIC | PFTEX_LEQUAL)
#define PFTEX_BICUBIC_GEQUAL		(PFTEX_BICUBIC | PFTEX_GEQUAL)
#define PFTEX_BICUBIC_LEQUAL		(PFTEX_BICUBIC | PFTEX_LEQUAL)
#define PFTEX_BICUBIC_GEQUAL		(PFTEX_BICUBIC | PFTEX_GEQUAL)
#define PFTEX_BILINEAR_LEQUAL		(PFTEX_BILINEAR | PFTEX_LEQUAL)
#define PFTEX_BILINEAR_GEQUAL		(PFTEX_BILINEAR | PFTEX_GEQUAL)

#define PFTEX_SHARPEN_SPLINE		2
#define PFTEX_DETAIL_SPLINE		3

#define PFTEX_MAX_SPLINE_POINTS		4

/* pfTexBorderType() tokens */
#define PFTEX_BORDER_NONE		0
#define PFTEX_BORDER_COLOR		1
#define PFTEX_BORDER_TEXELS		2

/* pfTexFormat() tokens */
#define PFTEX_INTERNAL_FORMAT		1
#define PFTEX_EXTERNAL_FORMAT		2
#define PFTEX_DETAIL_TEXTURE		3
#define PFTEX_GEN_MIPMAP_FORMAT		4
#define PFTEX_SUBLOAD_FORMAT		6
#define PFTEX_FAST_DEFINE		PFTEX_SUBLOAD_FORMAT
#define PFTEX_NO_TEXEL_OPS_FORMAT	7 /* no pixmode or scale/bias operations on texels */
#define PFTEX_4D_FORMAT			8
#define PFTEX_IMAGE_FORMAT		9

/* pfTexLoadMode() modes */
#define PFTEX_LOAD_LIST			1
#define PFTEX_LOAD_BASE			2
#define PFTEX_LOAD_SOURCE		3
#define PFTEX_LOAD_VIDEO_INTERLACED	4

/* pfTexLoadMode() values for PFTEX_LOAD_LIST */
#define PFTEX_LIST_APPLY		0
#define PFTEX_LIST_AUTO_IDLE		1
#define PFTEX_LIST_AUTO_SUBLOAD		2

/* pfTexLoadMode() values for PFTEX_LOAD_BASE */
#define PFTEX_BASE_APPLY		0
#define PFTEX_BASE_AUTO_SUBLOAD		1

/* pfTexLoadMode() values for PFTEX_LOAD_SOURCE */
#define PFTEX_SOURCE_IMAGE		0
#define PFTEX_SOURCE_FRAMEBUFFER	1
#define PFTEX_SOURCE_VIDEO		2
#define PFTEX_SOURCE_DMBUF		3
#define PFTEX_SOURCE_DMVIDEO		4

/* pfTexLoadMode() values for PFTEX_LOAD_VIDEO_INTERLACED */
#define PFTEX_VIDEO_INTERLACED_OFF	0
#define PFTEX_VIDEO_INTERLACED_ODD	1
#define PFTEX_VIDEO_INTERLACED_EVEN	2

/* pfTexLoadVal() */
#define PFTEX_LOAD_DMBUF		0
#define PFTEX_LOAD_VIDEO_VLSERVER	1
#define PFTEX_LOAD_VIDEO_VLPATH		2
#define PFTEX_LOAD_VIDEO_VLDRAIN	3


/* pfTex src/dest select tokens -- used for
 * pfTexLoadOrigin()
*/
#define PFTEX_ORIGIN_SOURCE		0
#define PFTEX_ORIGIN_DEST		1

/* more special external formats - currently IRIS GL only */
#define PFTEX_PACK_USHORT_5_6_5		0x8801
#define PFTEX_PACK_USHORT_4_4_4_4	0x8802

/* ------------------ pfTexture Related Functions--------------------- */

extern  pfTexture*   pfGetCurTex(void);
#if PF_C_API
extern void                 pfTexBorderColor(pfTexture* _tex, pfVec4 _clr);

#endif /* !PF_CPLUSPLUS_API */
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfTexture CAPI ------------------------------*/

extern DLLEXPORT pfTexture*           pfNewTex(void *arena);
extern DLLEXPORT pfType*              pfGetTexClassType(void);
extern DLLEXPORT void                 pfTexName(pfTexture* _tex, const char *_name);
extern DLLEXPORT const char*          pfGetTexName(const pfTexture* _tex);
extern DLLEXPORT void                 pfTexImage(pfTexture* _tex, unsigned int* _image, int _comp, int _sx, int _sy, int _sz);
extern DLLEXPORT void                 pfGetTexImage(const pfTexture* _tex, unsigned int** _image, int* _comp, int* _sx, int* _sy, int* _sz);
extern DLLEXPORT void                 pfGetTexBorderColor(pfTexture* _tex, pfVec4 *_clr);
extern DLLEXPORT void                 pfTexBorderType(pfTexture* _tex, int _type);
extern DLLEXPORT int                  pfGetTexBorderType(pfTexture* _tex);
extern DLLEXPORT void                 pfTexFormat(pfTexture* _tex, int _format, int type);
extern DLLEXPORT int                  pfGetTexFormat(const pfTexture* _tex, int _format);
extern DLLEXPORT void                 pfTexFilter(pfTexture* _tex, int _filt, int _type);
extern DLLEXPORT int                  pfGetTexFilter(const pfTexture* _tex, int _filt);
extern DLLEXPORT void                 pfTexRepeat(pfTexture* _tex, int _wrap, int _type);
extern DLLEXPORT int                  pfGetTexRepeat(const pfTexture* _tex, int _wrap);
extern DLLEXPORT void                 pfTexLODRange(pfTexture* _tex, float _min, float _max);
extern DLLEXPORT void                 pfGetTexLODRange(pfTexture* _tex, float *_min, float *_max);
extern DLLEXPORT void                 pfTexLODBias(pfTexture* _tex, float _biasS, float _biasT, float _biasR);
extern DLLEXPORT void                 pfGetTexLODBias(pfTexture* _tex, float *_biasS, float *_biasT, float *_biasR);
extern DLLEXPORT void                 pfTexSpline(pfTexture* _tex, int _type, pfVec2 *_pts, float _clamp);
extern DLLEXPORT void                 pfGetTexSpline(const pfTexture* _tex, int _type, pfVec2 *_pts, float *_clamp);
extern DLLEXPORT void                 pfTexDetail(pfTexture* _tex, int _l, pfTexture *_detail);
extern DLLEXPORT void                 pfGetTexDetail(const pfTexture* _tex, int *_l, pfTexture **_detail);
extern DLLEXPORT pfTexture*           pfGetTexDetailTex(const pfTexture* _tex);
extern DLLEXPORT void                 pfDetailTexTile(pfTexture* _tex, int _j, int _k, int _m, int _n, int scram);
extern DLLEXPORT void                 pfGetDetailTexTile(const pfTexture* _tex, int *_j, int *_k, int *_m, int *_n, int *scram);
extern DLLEXPORT void                 pfTexList(pfTexture* _tex, pfList *_list);
extern DLLEXPORT pfList*              pfGetTexList(const pfTexture* _tex);
extern DLLEXPORT void                 pfTexFrame(pfTexture* _tex, float _frame);
extern DLLEXPORT float                pfGetTexFrame(const pfTexture* _tex);
extern DLLEXPORT void                 pfTexLoadImage(pfTexture* _tex, unsigned int* _image);
extern DLLEXPORT unsigned int*        pfGetTexLoadImage(const pfTexture* _tex);
extern DLLEXPORT void                 pfTexLoadMode(pfTexture* _tex, int _mode, int _val);
extern DLLEXPORT int                  pfGetTexLoadMode(const pfTexture* _tex, int _mode);
extern DLLEXPORT void                 pfTexLoadVal(pfTexture* _tex, int _mode, void* _val);
extern DLLEXPORT void*                pfGetTexLoadVal(const pfTexture* _tex, int _mode);
extern DLLEXPORT void                 pfTexLevel(pfTexture* _tex, int _level, pfTexture* _ltex);
extern DLLEXPORT pfTexture*           pfGetTexLevel(pfTexture* _tex, int _level);
extern DLLEXPORT void                 pfTexLoadOrigin(pfTexture* _tex, int _which, int _xo, int _yo);
extern DLLEXPORT void                 pfGetTexLoadOrigin(pfTexture* _tex, int _which, int *_xo, int *_yo);
extern DLLEXPORT void                 pfTexLoadSize(pfTexture* _tex, int _xs, int _ys);
extern DLLEXPORT void                 pfGetTexLoadSize(const pfTexture* _tex, int *_xs, int *_ys);
extern DLLEXPORT void                 pfTexValidMap(pfTexture* _tex, pfTextureValidMap *_validMap);
extern DLLEXPORT pfTextureValidMap*   pfGetTexValidMap(const pfTexture* _tex);
extern DLLEXPORT void                 pfGetCurTexLODRange(pfTexture* _tex, float *_min, float *_max);
extern DLLEXPORT void                 pfGetCurTexLODBias(pfTexture* _tex, float *_biasS, float *_biasT, float *_biasR);
extern DLLEXPORT void                 pfTexAnisotropy(pfTexture* _tex, const int _degree);
extern DLLEXPORT int                  pfGetTexAnisotropy(const pfTexture* _tex);
extern DLLEXPORT void                 pfApplyTex(pfTexture* _tex);
extern DLLEXPORT void                 pfMultiApplyTex(pfTexture* _tex, int myID);
extern DLLEXPORT void                 pfApplyTexMinLOD(pfTexture* _tex, float _min);
extern DLLEXPORT void                 pfApplyTexMaxLOD(pfTexture* _tex, float _max);
extern DLLEXPORT void                 pfApplyTexLODBias(pfTexture* _tex, float _biasS, float _biasT, float _biasR);
extern DLLEXPORT void                 pfFormatTex(pfTexture* _tex);
extern DLLEXPORT void                 pfLoadTex(pfTexture* _tex);
extern DLLEXPORT void                 pfLoadTexLevel(pfTexture* _tex, int _level);
extern DLLEXPORT void                 pfSubloadTex(pfTexture* _tex, int _source, uint *image, int _xsrc, int _ysrc, int _srcwid, int _xdst, int _ydst, int _xsize, int _ysize);
extern DLLEXPORT void                 pfSubloadTexLevel(pfTexture* _tex, int _source, uint *image, int _xsrc, int _ysrc, int _srcwid, int _xdst, int _ydst, int _xsize, int _ysize, int _level);
extern DLLEXPORT int                  pfLoadTexFile(pfTexture* _tex, const char* _fname);
extern DLLEXPORT int                  pfSaveTexFile(pfTexture* _tex, const char* _fname);
extern DLLEXPORT int                  pfSaveFileTypeTex(pfTexture* _tex, const char* _fname, const char* _type);
extern DLLEXPORT void                 pfFreeTexImage(pfTexture* _tex);
extern DLLEXPORT void                 pfIdleTex(pfTexture* _tex);
extern DLLEXPORT int                  pfIsTexLoaded(const pfTexture* _tex);
extern DLLEXPORT int                  pfIsTexFormatted(const pfTexture* _tex);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfTexEnv Tokens ----------------------- */

/* pfTEnvComponent()  */
#define PFTE_COMP_OFF		0

/* fast default for pfTEnvMode */
#define PFTEV_FAST		0x80000000

/* ------------------ pfTexEnv Related Functions--------------------- */

extern  pfTexEnv*   pfGetCurTEnv(void);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfTexEnv CAPI ------------------------------*/

extern DLLEXPORT pfTexEnv*            pfNewTEnv(void *arena);
extern DLLEXPORT pfType*              pfGetTEnvClassType(void);
extern DLLEXPORT void                 pfTEnvMode(pfTexEnv* _tenv, int _mode);
extern DLLEXPORT int                  pfGetTEnvMode(const pfTexEnv* _tenv);
extern DLLEXPORT void                 pfTEnvComponent(pfTexEnv* _tenv, int _comp);
extern DLLEXPORT int                  pfGetTEnvComponent(const pfTexEnv* _tenv);
extern DLLEXPORT void                 pfTEnvBlendColor(pfTexEnv* _tenv, float _r, float _g, float _b, float _a);
extern DLLEXPORT void                 pfGetTEnvBlendColor(pfTexEnv* _tenv, float* _r, float* _g, float* _b, float* _a);
extern DLLEXPORT void                 pfApplyTEnv(pfTexEnv* _tenv);
extern DLLEXPORT void                 pfMultiApplyTEnv(pfTexEnv* _tenv, int myID);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ------------------ pfTexGen Related Functions--------------------- */

extern  pfTexGen*   pfGetCurTGen(void);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfTexGen CAPI ------------------------------*/

extern DLLEXPORT pfTexGen*            pfNewTGen(void *arena);
extern DLLEXPORT pfType*              pfGetTGenClassType(void);
extern DLLEXPORT void                 pfTGenMode(pfTexGen* _tgen, int _texCoord, int _mode);
extern DLLEXPORT int                  pfGetTGenMode(const pfTexGen* _tgen, int _texCoord);
extern DLLEXPORT void                 pfTGenPoint(pfTexGen* _tgen, int _texCoord, float _x, float _y, float _z, float _dx, float _dy, float _dz);
extern DLLEXPORT void                 pfGetTGenPoint(pfTexGen* _tgen, int _texCoord, float *_x, float *_y, float *_z);
extern DLLEXPORT void                 pfTGenPlane(pfTexGen* _tgen, int _texCoord, float _x, float _y, float _z, float _d);
extern DLLEXPORT void                 pfGetTGenPlane(pfTexGen* _tgen, int _texCoord, float* _x, float* _y, float* _z, float* _d);
extern DLLEXPORT void                 pfApplyTGen(pfTexGen* _tgen);
extern DLLEXPORT void                 pfMultiApplyTGen(pfTexGen* _tgen, int myID);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ------------------ pfTexLOD Related Functions--------------------- */

extern  pfTexLOD*   pfGetCurTLOD(void);
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfTexLOD CAPI ------------------------------*/

extern DLLEXPORT pfTexLOD*            pfNewTLOD(void *arena);
extern DLLEXPORT pfType*              pfGetTLODClassType(void);
extern DLLEXPORT void                 pfTLODRange(pfTexLOD* _tlod, float _min, float _max);
extern DLLEXPORT void                 pfGetTLODRange(pfTexLOD* _tlod, float *_min, float *_max);
extern DLLEXPORT void                 pfTLODBias(pfTexLOD* _tlod, float _biasS, float _biasT, float _biasR);
extern DLLEXPORT void                 pfGetTLODBias(pfTexLOD* _tlod, float *_biasS, float *_biasT, float *_biasR);
extern DLLEXPORT void                 pfApplyTLOD(pfTexLOD* _tlod);
extern DLLEXPORT void                 pfMultiApplyTLOD(pfTexLOD* _tlod, int myID);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfTextureValidMapLevel CAPI ------------------------------*/


#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfTextureValidMap CAPI ------------------------------*/

extern DLLEXPORT pfTextureValidMap*   pfNewTextureValidMap(int virtSizeS, int virtSizeT, int mapSizeS, int mapSizeT, int clipSizeS, int clipSizeT, int quantumS, int quantumT, void *arena);
extern DLLEXPORT void                 pfTextureValidMapConstruct(pfTextureValidMap* _texturevalidmap, int virtSizeS, int virtSizeT, int mapSizeS, int mapSizeT, int clipSizeS, int clipSizeT, int quantumS, int quantumT, void *arena);
extern DLLEXPORT void                 pfTextureValidMapDestruct(pfTextureValidMap* _texturevalidmap);
extern DLLEXPORT void                 pfTextureValidMapRecordTexLoad(pfTextureValidMap* _texturevalidmap, int level, int s0, int t0, int ns, int nt);
extern DLLEXPORT void                 pfTextureValidMapTracing(pfTextureValidMap* _texturevalidmap, int _tracing);
extern DLLEXPORT void                 pfTextureValidMapPrint(pfTextureValidMap* _texturevalidmap, FILE *fp, int minLevelSize, int maxLevelSize);
extern DLLEXPORT int                  pfTextureValidMapIsValidCenter(pfTextureValidMap* _texturevalidmap, int centerS, int centerT, int invalidBorder, int verbose);
extern DLLEXPORT int                  pfTextureValidMapIsValid(pfTextureValidMap* _texturevalidmap, int level, int s0, int t0, int ns, int nt);

#endif /* !PF_C_API */



#ifdef PF_C_API
/*---------------- pfCycleMemory CAPI ------------------------------*/

extern DLLEXPORT pfType*              pfGetCMemClassType(void);
extern DLLEXPORT pfCycleBuffer*       pfGetCMemCBuffer(pfCycleMemory* _cmem);
extern DLLEXPORT int                  pfGetCMemFrame(const pfCycleMemory* _cmem);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfCycleBuffer CAPI ------------------------------*/

extern DLLEXPORT pfCycleBuffer*       pfNewCBuffer(size_t nbytes, void *arena);
extern DLLEXPORT pfType*              pfGetCBufferClassType(void);
extern DLLEXPORT pfCycleMemory*       pfGetCBufferCMem(const pfCycleBuffer* cBuf, int index);
extern DLLEXPORT void*                pfGetCurCBufferData(const pfCycleBuffer* cBuf);
extern DLLEXPORT void                 pfCBufferChanged(pfCycleBuffer* cBuf);
extern DLLEXPORT void                 pfInitCBuffer(pfCycleBuffer* cBuf, void *data);
extern DLLEXPORT int                  pfCBufferConfig(int _numBuffers);
extern DLLEXPORT int                  pfGetCBufferConfig(void);
extern DLLEXPORT int                  pfCBufferFrame(void);
extern DLLEXPORT int                  pfGetCBufferFrameCount(void);
extern DLLEXPORT int                  pfGetCurCBufferIndex(void);
extern DLLEXPORT void                 pfCurCBufferIndex(int _index);
extern DLLEXPORT pfCycleBuffer*       pfGetCBuffer(void *data);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfFluxMemory CAPI ------------------------------*/

extern DLLEXPORT pfType*              pfGetFluxMemoryClassType(void);
extern DLLEXPORT pfFluxMemory*        pfGetFluxMemory(void *data);
extern DLLEXPORT pfFlux*              pfGetFluxMemoryFlux(pfFluxMemory* _fluxmemory);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/*
 *  Special config buffer number
 */
#define PFFLUX_DEFAULT_NUM_BUFFERS	0x01000000
#define PFFLUX_MAX_BUFFERS		0x0000ffff

/*
 *  Tokens for pfFlux::getNumBuffers
 */
#define PFFLUX_BUFFERS_SPECIFIED	1
#define PFFLUX_BUFFERS_GENERATED	2

/*
 *  modes
 */
#define PFFLUX_PUSH		0
#define PFFLUX_ON_DEMAND	1
#define PFFLUX_COPY_LAST_DATA	2
#define PFFLUX_WRITE_ONCE	3

/*
 *  masks
 */
#define PFFLUX_BASIC_MASK	0x00000001

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfFlux CAPI ------------------------------*/

extern DLLEXPORT pfFlux*              pfNewFlux(size_t nbytes, int _numBuffers, void *arena);
extern DLLEXPORT pfFlux*              pfNewFluxInitFunc(pfFluxInitFuncType _initFunc, int _numBuffers, void *arena);
extern DLLEXPORT pfType*              pfGetFluxClassType(void);
extern DLLEXPORT void*                pfGetFluxCurData(pfFlux* _flux);
extern DLLEXPORT void*                pfGetFluxWritableData(pfFlux* _flux);
extern DLLEXPORT void*                pfGetFluxBufferData(pfFlux* _flux, int bufferid);
extern DLLEXPORT void                 pfFluxMode(pfFlux* _flux, int _mode, int _val);
extern DLLEXPORT int                  pfGetFluxMode(const pfFlux* _flux, int _mode);
extern DLLEXPORT void                 pfFluxMask(pfFlux* _flux, uint _mask);
extern DLLEXPORT uint                 pfGetFluxMask(const pfFlux* _flux);
extern DLLEXPORT void                 pfFluxSyncGroup(pfFlux* _flux, uint _syncGroup);
extern DLLEXPORT uint                 pfGetFluxSyncGroup(const pfFlux* _flux);
extern DLLEXPORT size_t               pfGetFluxDataSize(const pfFlux* _flux);
extern DLLEXPORT int                  pfGetFluxNumBuffers(const pfFlux* _flux, int _type);
extern DLLEXPORT pfFluxInitFuncType   pfGetFluxInitFunc(const pfFlux* _flux);
extern DLLEXPORT pfEngine*            pfGetFluxSrcEngine(const pfFlux* _flux, int _index);
extern DLLEXPORT int                  pfGetFluxNumSrcEngines(const pfFlux* _flux);
extern DLLEXPORT pfEngine*            pfGetFluxClientEngine(const pfFlux* _flux, int _index);
extern DLLEXPORT int                  pfGetFluxNumClientEngines(const pfFlux* _flux);
extern DLLEXPORT void                 pfFluxUserCopyFunc(pfFlux* _flux, pfFluxUserCopyFuncType func);
extern DLLEXPORT pfFluxUserCopyFuncType pfGetFluxUserCopyFunc(pfFlux* _flux);
extern DLLEXPORT void                 pfFluxWriteComplete(pfFlux* _flux);
extern DLLEXPORT void                 pfFluxSrcChanged(pfFlux* _flux);
extern DLLEXPORT void                 pfFluxInitData(pfFlux* _flux, void *_data);
extern DLLEXPORT void                 pfFluxCallDataFunc(pfFlux* _flux, pfFluxDataFuncType _func, void *_funcData);
extern DLLEXPORT void                 pfFluxEvaluate(pfFlux* _flux, int _mask);
extern DLLEXPORT void                 pfFluxEvaluateEye(pfFlux* _flux, int _mask, pfVec3 _eye_pos);
extern DLLEXPORT int                  pfFluxDefaultNumBuffers(int _numBuffers);
extern DLLEXPORT int                  pfGetFluxDefaultNumBuffers(void);
extern DLLEXPORT pfFlux*              pfGetFlux(void *_data);
extern DLLEXPORT void*                pfGetFluxCurDataFromData(void *_data);
extern DLLEXPORT void*                pfGetFluxWritableDataFromData(void *_data);
extern DLLEXPORT void                 pfFluxWriteCompleteFromData(void *_data);
extern DLLEXPORT void                 pfFluxFrame(int _frame);
extern DLLEXPORT void                 pfFluxKeepFrame(int _frame);
extern DLLEXPORT void                 pfFluxShiftFrameByPID(pid_t _pid, int _frame);
extern DLLEXPORT int                  pfGetFluxFrame(void);
extern DLLEXPORT void                 pfFluxEnableSyncGroup(uint _syncGroup);
extern DLLEXPORT void                 pfFluxDisableSyncGroup(uint _syncGroup);
extern DLLEXPORT int                  pfGetFluxEnableSyncGroup(uint _syncGroup);
extern DLLEXPORT void                 pfFluxSyncGroupReady(uint _syncGroup);
extern DLLEXPORT void                 pfFluxSyncComplete(void);
extern DLLEXPORT int                  pfGetFluxNamedSyncGroup(const char *_name);
extern DLLEXPORT const char*          pfGetFluxSyncGroupName(uint _syncGroup);
extern DLLEXPORT int                  pfGetFluxNumNamedSyncGroups(void);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/*
 *  functions
 */
#define PFENG_SUM		1
#define PFENG_MORPH		2
#define PFENG_TRANSFORM		3
#define PFENG_ALIGN		4
#define PFENG_MATRIX		5
#define PFENG_ANIMATE		6
#define PFENG_BBOX		7
#define PFENG_USER_FUNCTION	8
#define PFENG_TIME		9
#define PFENG_BLEND		10
#define PFENG_STROBE		11

/*
 *  Sources for PFENG_SUM
 */
#define PFENG_SUM_SRC_0			0
#define PFENG_SUM_SRC(n)		(PFENG_SUM_SRC_0 + n)

/*
 *  Sources for PFENG_MORPH
 */
#define PFENG_MORPH_FRAME		0
#define PFENG_MORPH_WEIGHTS		1
#define PFENG_MORPH_SRC_0		2
#define PFENG_MORPH_SRC(n)		(PFENG_MORPH_SRC_0 + n)

/*
 *  Sources for PFENG_TRANSFORM
 */
#define PFENG_TRANSFORM_SRC		0
#define PFENG_TRANSFORM_MATRIX		1

/*
 *  Sources for PFENG_ALIGN
 */
#define PFENG_ALIGN_POSITION		0
#define PFENG_ALIGN_NORMAL		1
#define PFENG_ALIGN_AZIMUTH		2

/*
 *  Sources for PFENG_MATRIX
 */
#define PFENG_MATRIX_ROT		0
#define PFENG_MATRIX_TRANS		1
#define PFENG_MATRIX_SCALE_UNIFORM	2
#define PFENG_MATRIX_SCALE_XYZ		3
#define PFENG_MATRIX_BASE_MATRIX	4

/*
 *  Sources for PFENG_ANIMATE
 */
#define PFENG_ANIMATE_FRAME		0
#define PFENG_ANIMATE_WEIGHTS		1
#define PFENG_ANIMATE_ROT		2
#define PFENG_ANIMATE_TRANS		3
#define PFENG_ANIMATE_SCALE_UNIFORM	4
#define PFENG_ANIMATE_SCALE_XYZ		5
#define PFENG_ANIMATE_BASE_MATRIX	6

/*
 *  Sources for PFENG_BBOX
 */
#define PFENG_BBOX_SRC			0

/*
 *  Sources for PFENG_USER_FUNCTION
 */
#define PFENG_USER_FUNCTION_SRC_0	0
#define PFENG_USER_FUNCTION_SRC(n)	(PFENG_SUM_SRC_0 + n)

/*
 *  Sources for PFENG_TIME
 */
#define PFENG_TIME_TIME			0
#define PFENG_TIME_SCALE		1

/*
 *  Sources for PFENG_BLEND
 */
#define PFENG_BLEND_FRAME		0
#define PFENG_BLEND_WEIGHTS		1
#define PFENG_BLEND_SRC			2

/*
 *  Sources for PFENG_STROBE
 */
#define PFENG_STROBE_TIME		0
#define PFENG_STROBE_TIMING		1
#define PFENG_STROBE_ON			2
#define PFENG_STROBE_OFF		3

/*
 *  modes
 */
#define PFENG_PULL		0
#define PFENG_RANGE_CHECK	1
#define PFENG_MATRIX_MODE	2
#define PFENG_TIME_MODE		3
#define PFENG_TIME_TRUNC	4

/*
 *  values for PFENG_MATRIX_MODE
 */
#define PFENG_MATRIX_PRE_MULT	0
#define PFENG_MATRIX_POST_MULT	1

/*
 *  values for PFENG_TIME_MODE
 */
#define PFENG_TIME_CYCLE	0
#define PFENG_TIME_SWING	1

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfEngine CAPI ------------------------------*/

extern DLLEXPORT pfEngine*            pfNewEngine(int _function, void *arena);
extern DLLEXPORT pfType*              pfGetEngineClassType(void);
extern DLLEXPORT void                 pfEngineSrc(pfEngine* _engine, int _index, void *_data, ushort *_ilist, int _icount, int _offset, int _stride);
extern DLLEXPORT void                 pfGetEngineSrc(const pfEngine* _engine, int _index, void **_data, ushort **_ilist, int *_icount, int *_offset, int *_stride);
extern DLLEXPORT int                  pfGetEngineNumSrcs(const pfEngine* _engine);
extern DLLEXPORT void                 pfEngineDst(pfEngine* _engine, void *_data, ushort *_ilist, int _offset, int _stride);
extern DLLEXPORT void                 pfGetEngineDst(const pfEngine* _engine, void **_data, ushort **_ilist, int *_offset, int *_stride);
extern DLLEXPORT void                 pfEngineIterations(pfEngine* _engine, int _iterations, int _itemsPer);
extern DLLEXPORT void                 pfGetEngineIterations(const pfEngine* _engine, int *_iterations, int *_itemsPer);
extern DLLEXPORT int                  pfGetEngineFunction(const pfEngine* _engine);
extern DLLEXPORT void                 pfEngineUserFunction(pfEngine* _engine, pfEngineFuncType _func);
extern DLLEXPORT pfEngineFuncType     pfGetEngineUserFunction(const pfEngine* _engine);
extern DLLEXPORT void                 pfEngineMode(pfEngine* _engine, int _mode, int _val);
extern DLLEXPORT int                  pfGetEngineMode(const pfEngine* _engine, int _mode);
extern DLLEXPORT void                 pfEngineMask(pfEngine* _engine, uint _mask);
extern DLLEXPORT uint                 pfGetEngineMask(const pfEngine* _engine);
extern DLLEXPORT void                 pfEngineEvaluationRange(pfEngine* _engine, PFVEC3 _center, float _min, float _max);
extern DLLEXPORT void                 pfGetEngineEvaluationRange(const pfEngine* _engine, PFVEC3 _center, float *_min, float *_max);
extern DLLEXPORT void                 pfEngineSrcChanged(pfEngine* _engine);
extern DLLEXPORT void                 pfEngineEvaluate(pfEngine* _engine, int _mask);
extern DLLEXPORT void                 pfEngineEvaluateEye(pfEngine* _engine, int _mask, pfVec3 _eye_pos);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfMemory CAPI ------------------------------*/

extern DLLEXPORT pfType*              pfGetMemoryClassType(void);
extern DLLEXPORT int                  pfGetMemoryArenaBytesUsed(void);
#if !PF_CPLUSPLUS_API
extern DLLEXPORT void*                pfMalloc(size_t nbytes, void *arena);
extern DLLEXPORT void*                pfCalloc(size_t numelem, size_t elsize, void *arena);
extern DLLEXPORT char*                pfStrdup(const char *str, void *arena);
extern DLLEXPORT void*                pfRealloc(void *data, size_t nbytes);
extern DLLEXPORT size_t               pfGetSize(void *data);
extern DLLEXPORT void*                pfGetArena(void *data);
extern DLLEXPORT void                 pfFree(void *data);
extern DLLEXPORT void*                pfGetData(const void *data);
extern DLLEXPORT pfMemory*            pfGetMemory(const void *data);
extern DLLEXPORT const char*          pfGetTypeName(const void *data);
extern DLLEXPORT pfType*              pfGetType(const void *data);
extern DLLEXPORT int                  pfIsOfType(const void *data, pfType *type);
extern DLLEXPORT int                  pfIsExactType(const void *data, pfType *type);
extern DLLEXPORT int                  pfIsFluxed(const void *data);
extern DLLEXPORT int                  pfRef(void* _mem);
extern DLLEXPORT int                  pfUnref(void* _mem);
extern DLLEXPORT int                  pfGetRef(const void* _mem);
extern DLLEXPORT int                  pfCompare(const void* _mem1, const void* _mem2);
extern DLLEXPORT int                  pfPrint(const void* _mem, uint _travMode, uint _verbose, FILE* file);
extern DLLEXPORT int                  prDelete(void* _mem);
extern DLLEXPORT int                  prUnrefGetRef(void* _mem);
extern DLLEXPORT int                  prUnrefDelete(void* _mem);
extern DLLEXPORT int                  prCopy(void* dst, const void* src);
#endif /* !PF_CPLUSPLUS_API */

#endif /* !PF_C_API */

#ifndef __PF_H__

/* #defines for libpr -> libpf CAPI inheritance */

#define pfDelete prDelete
#define pfUnrefGetRef prUnrefGetRef
#define pfUnrefDelete prUnrefDelete
#define pfCopy prCopy

#endif /* !__PF_H__ */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfFile Tokens ----------------------- */

#define PFRTF_NOACTION	    0
#define PFRTF_CREATE	    1
#define PFRTF_OPEN	    2
#define PFRTF_READ	    3
#define PFRTF_WRITE	    4
#define PFRTF_SEEK	    5
#define PFRTF_CLOSE	    6
#define PFRTF_PENDING	    7

#define PFRTF_STATUS	    0
#define PFRTF_CMD	    1
#define PFRTF_BYTES	    2
#define PFRTF_OFFSET	    3
#define PFRTF_PID           4
#define PFRTF_MAXREQ        32

/* ------------------ pfFile Related Functions--------------------- */

extern pfFile* pfOpenFile(char* _fname, int _oflag, ...);

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfFile CAPI ------------------------------*/

extern DLLEXPORT pfFile*              pfCreateFile(char* _fname, mode_t _mode);
extern DLLEXPORT pfType*              pfGetFileClassType(void);
extern DLLEXPORT int                  pfGetFileStatus(const pfFile* _file, int _attr);
extern DLLEXPORT int                  pfReadFile(pfFile* _file, char* _buf, int _nbyte);
extern DLLEXPORT int                  pfWriteFile(pfFile* _file, char* _buf, int _nbyte);
extern DLLEXPORT off_t                pfSeekFile(pfFile* _file, off_t off, int _whence);
extern DLLEXPORT int                  pfCloseFile(pfFile* _file);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfList CAPI ------------------------------*/

extern DLLEXPORT pfList*              pfNewList(int _eltSize, int _listLength, void *arena);
extern DLLEXPORT pfType*              pfGetListClassType(void);
extern DLLEXPORT int                  pfGetListEltSize(const pfList* _list);
extern DLLEXPORT void**               pfGetListArray(const pfList* _list);
extern DLLEXPORT void                 pfListArrayLen(pfList* _list, int alen);
extern DLLEXPORT int                  pfGetListArrayLen(const pfList* _list);
extern DLLEXPORT void                 pfNum(pfList* _list, int newNum);
extern DLLEXPORT int                  pfGetNum(const pfList* _list);
extern DLLEXPORT void                 pfSet(pfList* _list, int index, void *elt);
extern DLLEXPORT void*                pfGet(const pfList* _list, int index);
extern DLLEXPORT void                 pfResetList(pfList* _list);
extern DLLEXPORT void                 pfCombineLists(pfList* _lists, const pfList *a, const pfList *b);
extern DLLEXPORT void                 pfAdd(pfList* _lists, void *elt);
extern DLLEXPORT void*                pfRem(pfList* _lists);
extern DLLEXPORT void                 pfInsert(pfList* _lists, int index, void *elt);
extern DLLEXPORT int                  pfSearch(const pfList* _lists, void *elt);
extern DLLEXPORT int                  pfRemove(pfList* _lists, void *elt);
extern DLLEXPORT void                 pfRemoveIndex(pfList* _lists, int index);
extern DLLEXPORT int                  pfMove(pfList* _lists, int index, void *elt);
extern DLLEXPORT int                  pfFastRemove(pfList* _lists, void *elt);
extern DLLEXPORT void                 pfFastRemoveIndex(pfList* _lists, int index);
extern DLLEXPORT int                  pfReplace(pfList* _lists, void *oldElt, void *newElt);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ------------------ pfList/pfPath Macros --------------------- */
#if defined(__STDC__) || defined(__cplusplus)

#define pfGetListArrayLen(_list)	pfGetListArrayLen((pfList*)_list)
#define pfListArrayLen(_list, _len)	pfListArrayLen((pfList*)_list, _len)
#define pfGetListArray(_list)		pfGetListArray((pfList*)_list)
#define pfResetList(_list)		pfResetList((pfList*)_list)
#define pfCombineLists(_dst, _a, _b)	\
	pfCombineLists((pfList*)_dst, (pfList*)_a, (pfList*)_b)
#define pfNum(_list, _num)		pfNum((pfList*)_list, _num)
#define pfGetNum(_list)			pfGetNum((pfList*)_list)
#define pfAdd(_list, _elt)		pfAdd((pfList*)_list, _elt)
#define pfInsert(_list, _index, _elt)	pfInsert((pfList*)_list, _index, _elt)
#define pfSearch(_list, _elt)		pfSearch((pfList*)_list, _elt)
#define pfRemove(_list, _elt)		pfRemove((pfList*)_list, _elt)
#define pfReplace(_list, _old, _new)	pfReplace((pfList*)_list, _old, _new)
#define pfSet(_list, _index, _elt)	pfSet((pfList*)_list, _index, _elt)
#define pfGet(_list, _index)		pfGet((pfList*)_list, _index)

#endif /* defined(__STDC__) || defined(__cplusplus) */
#endif /* !PF_CPLUSPLUS_API */



#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfWindow Tokens ----------------------- */

/* FBConfig attribute array tokens */
#define PFFB_USE_GL              1       /* support GLX rendering */
#define PFFB_BUFFER_SIZE         2       /* depth of the color buffer */
#define PFFB_LEVEL               3       /* level in plane stacking */
#define PFFB_RGBA                4       /* true if RGBA mode */
#define PFFB_DOUBLEBUFFER        5       /* double buffering supported */
#define PFFB_STEREO              6       /* stereo buffering supported */
#define PFFB_AUX_BUFFERS         7       /* number of aux buffers */
#define PFFB_RED_SIZE            8       /* number of red component bits */
#define PFFB_GREEN_SIZE          9       /* number of green component bits */
#define PFFB_BLUE_SIZE           10      /* number of blue component bits */
#define PFFB_ALPHA_SIZE          11      /* number of alpha component bits */
#define PFFB_DEPTH_SIZE          12      /* number of depth bits */
#define PFFB_STENCIL_SIZE        13      /* number of stencil bits */
#define PFFB_ACCUM_RED_SIZE      14      /* number of red accum bits */
#define PFFB_ACCUM_GREEN_SIZE    15      /* number of green accum bits */
#define PFFB_ACCUM_BLUE_SIZE     16      /* number of blue accum bits */
#define PFFB_ACCUM_ALPHA_SIZE    17      /* number of alpha accum bits */

#ifdef __linux__
#define PFFB_SAMPLE_BUFFER	100000  /* the number of multisample buffers */
#define PFFB_SAMPLES		100001  /* number of samples per pixel */
#else
#define PFFB_SAMPLES		100000  /* number of samples per pixel */
#define PFFB_SAMPLE_BUFFER	100001  /* the number of multisample buffers */
#endif

/* pfWinMode */
#define PFWIN_AUTO_RESIZE	1
#define PFWIN_EXIT  		2
#define PFWIN_ORIGIN_LL		3
#define PFWIN_NOBORDER		4
#define PFWIN_HAS_OVERLAY	5
#define PFWIN_HAS_STATS		6

/* pfWinType */
#define PFWIN_TYPE_NOPORT	0x0100
#define PFWIN_TYPE_X		0x0200
#define PFWIN_TYPE_OVERLAY	0x0400
#define PFWIN_TYPE_STATS	0x0800
#define PFWIN_TYPE_UNMANAGED	0x1000
#define PFWIN_TYPE_PBUFFER	0x2000
#define PFWIN_TYPE_MASK		0x3f00


/* pfWinShare */
#define PFWIN_SHARE_TYPE	 0x0001
#define PFWIN_SHARE_MODE	 0x0002
#define PFWIN_SHARE_WSWINDOW	 0x0010
#define PFWIN_SHARE_FBCONFIG	 0x0020
#define PFWIN_SHARE_WSDRAWABLE_BIT  0x0040
#define PFWIN_SHARE_GL_CXT	 0x0080
#define PFWIN_SHARE_STATE_BIT	 0x0100
#define PFWIN_SHARE_GL_OBJS	 0x0200
#define PFWIN_SHARE_VISUALID	 0x0400
#define PFWIN_SHARE_OVERLAY_WIN	 0x1000
#define PFWIN_SHARE_STATS_WIN	 0x2000
#define PFWIN_SHARE_WSDRAWABLE	(PFWIN_SHARE_WSDRAWABLE_BIT | PFWIN_SHARE_FBCONFIG | PFWIN_SHARE_VISUALID | PFWIN_SHARE_WSWINDOW)
#define PFWIN_SHARE_STATE	(PFWIN_SHARE_STATE_BIT | PFWIN_SHARE_GL_CXT | PFWIN_SHARE_FBCONFIG | PFWIN_SHARE_VISUALID)

/* pfWinSelect */
#define PFWIN_GFX_WIN		-1
#define PFWIN_OVERLAY_WIN	-2
#define PFWIN_STATS_WIN		-3


/* 
 * pfQueryWin - framebuffer config queries - returns 1 value 
 */
#define PFQWIN_BIT		    0x100000	/* int */
#define PFQWIN_RGB_BITS	    	    0x100101	/* int */
#define PFQWIN_ALPHA_BITS	    0x100102	/* int */
#define PFQWIN_CI_BITS		    0x100103	/* int */
#define PFQWIN_DEPTH_BITS	    0x100104	/* int */
#define PFQWIN_MIN_DEPTH_VAL	    0x100105	/* int */
#define PFQWIN_MAX_DEPTH_VAL	    0x100106	/* int */
#define PFQWIN_MS_SAMPLES	    0x100107	/* int */
#define PFQWIN_STENCIL_BITS	    0x100108	/* int */
#define PFQWIN_DOUBLEBUFFER	    0x100109	/* int */ 
#define PFQWIN_STEREO		    0x10010a	/* int */
#define PFQWIN_NUM_AUX_BUFFERS	    0x10010b	/* int */  
#define PFQWIN_LEVEL		    0x10010c	/* int */
#define PFQWIN_ACCUM_RGB_BITS	    0x10010d	/* int */
#define PFQWIN_ACCUM_ALPHA_BITS	    0x10010e	/* int */

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfWindow CAPI ------------------------------*/

extern DLLEXPORT pfWindow*            pfNewWin(void *arena);
extern DLLEXPORT pfType*              pfGetWinClassType(void);
extern DLLEXPORT void                 pfWinName(pfWindow* _win, const char *_name);
extern DLLEXPORT const char*          pfGetWinName(const pfWindow* _win);
extern DLLEXPORT void                 pfWinMode(pfWindow* _win, int _mode, int _val);
extern DLLEXPORT int                  pfGetWinMode(const pfWindow* _win, int _mode);
extern DLLEXPORT void                 pfWinType(pfWindow* _win, unsigned int _type);
extern DLLEXPORT unsigned int         pfGetWinType(const pfWindow* _win);
extern DLLEXPORT pfState*             pfGetWinCurState(const pfWindow* _win);
extern DLLEXPORT void                 pfWinAspect(pfWindow* _win, int _x, int _y);
extern DLLEXPORT void                 pfGetWinAspect(const pfWindow* _win, int *_x, int *_y);
extern DLLEXPORT void                 pfWinOriginSize(pfWindow* _win, int _xo, int _yo, int _xs, int _ys);
extern DLLEXPORT void                 pfWinOrigin(pfWindow* _win, int _xo, int _yo);
extern DLLEXPORT void                 pfGetWinOrigin(const pfWindow* _win, int *_xo, int *_yo);
extern DLLEXPORT void                 pfWinSize(pfWindow* _win, int _xs, int _ys);
extern DLLEXPORT void                 pfGetWinSize(const pfWindow* _win, int *_xs, int *_ys);
extern DLLEXPORT void                 pfGetWinScreenOrigin(pfWindow* _win, int *_xo, int *_yo);
extern DLLEXPORT void                 pfWinFullScreen(pfWindow* _win);
extern DLLEXPORT void                 pfGetWinCurOriginSize(pfWindow* _win, int *_xo, int *_yo, int *_xs, int *_ys);
extern DLLEXPORT void                 pfGetWinCurScreenOriginSize(pfWindow* _win, int *_xo, int *_yo, int *_xs, int *_ys);
extern DLLEXPORT void                 pfWinOverlayWin(pfWindow* _win, pfWindow *_ow);
extern DLLEXPORT pfWindow*            pfGetWinOverlayWin(const pfWindow* _win);
extern DLLEXPORT void                 pfWinStatsWin(pfWindow* _win, pfWindow *_ow);
extern DLLEXPORT pfWindow*            pfGetWinStatsWin(const pfWindow* _win);
extern DLLEXPORT void                 pfWinScreen(pfWindow* _win, int s);
extern DLLEXPORT int                  pfGetWinScreen(const pfWindow* _win);
extern DLLEXPORT void                 pfWinShare(pfWindow* _win, uint _mode);
extern DLLEXPORT uint                 pfGetWinShare(const pfWindow* _win);
extern DLLEXPORT void                 pfWinSwapBarrier(pfWindow* _win, int _barrierName);
extern DLLEXPORT int                  pfGetWinSwapBarrier(pfWindow* _win);
extern DLLEXPORT int                  pfWinInSwapGroup(pfWindow* _win);
extern DLLEXPORT void                 pfWinWSWindow(pfWindow* _win, pfWSConnection _dsp, pfWSWindow _wsWin);
extern DLLEXPORT pfWSWindow           pfGetWinWSWindow(const pfWindow* _win);
extern DLLEXPORT void                 pfWinWSDrawable(pfWindow* _win, pfWSConnection _dsp, pfWSDrawable _wsWin);
extern DLLEXPORT pfWSDrawable         pfGetWinWSDrawable(const pfWindow* _win);
extern DLLEXPORT pfWSDrawable         pfGetWinCurWSDrawable(const pfWindow* _win);
extern DLLEXPORT void                 pfWinWSConnectionName(pfWindow* _win, const char *_name);
extern DLLEXPORT const char*          pfGetWinWSConnectionName(const pfWindow* _win);
extern DLLEXPORT void                 pfWinFBConfigData(pfWindow* _win, void *_data);
extern DLLEXPORT void*                pfGetWinFBConfigData(pfWindow* _win);
extern DLLEXPORT void                 pfWinFBConfigAttrs(pfWindow* _win, int *_attr);
extern DLLEXPORT int*                 pfGetWinFBConfigAttrs(const pfWindow* _win);
extern DLLEXPORT void                 pfWinPixelFormatAttrs(pfWindow* _win, GLint *iattrs, GLfloat *fattrs);
extern DLLEXPORT void                 pfGetWinPixelFormatAttrs(pfWindow* _win, GLint *iattrs, GLfloat *fattrs);
extern DLLEXPORT pfFBConfig           pfGetWinFBConfig(const pfWindow* _win);
extern DLLEXPORT void                 pfWinFBConfigId(pfWindow* _win, int _vId);
extern DLLEXPORT int                  pfGetWinFBConfigId(const pfWindow* _win);
extern DLLEXPORT void                 pfWinIndex(pfWindow* _win, int _index);
extern DLLEXPORT int                  pfGetWinIndex(const pfWindow* _win);
extern DLLEXPORT pfWindow*            pfGetWinSelect(pfWindow* _win);
extern DLLEXPORT void                 pfWinGLCxt(pfWindow* _win, pfGLContext _gCxt);
extern DLLEXPORT pfGLContext          pfGetWinGLCxt(const pfWindow* _win);
extern DLLEXPORT void                 pfWinList(pfWindow* _win, pfList *_wl);
extern DLLEXPORT pfList*              pfGetWinList(const pfWindow* _win);
#define pfGLXFBConfig int 
extern DLLEXPORT void                 pfOpenWin(pfWindow* _win);
extern DLLEXPORT void                 pfCloseWin(pfWindow* _win);
extern DLLEXPORT void                 pfCloseWinGL(pfWindow* _win);
extern DLLEXPORT int                  pfAttachWin(pfWindow* _win, pfWindow *_w1);
extern DLLEXPORT int                  pfDetachWin(pfWindow* _win, pfWindow *_w1);
extern DLLEXPORT void                 pfAttachWinSwapGroup(pfWindow* _win, pfWindow *_w1);
extern DLLEXPORT void                 pfDetachWinSwapGroup(pfWindow* _win);
extern DLLEXPORT pfWindow*            pfSelectWin(pfWindow* _win);
extern DLLEXPORT void                 pfSwapWinBuffers(pfWindow* _win);
extern DLLEXPORT pfFBConfig           pfChooseWinFBConfig(pfWindow* _win, int *_attr);
extern DLLEXPORT int                  pfIsWinOpen(const pfWindow* _win);
extern DLLEXPORT int                  pfIsManagedWin(const pfWindow* _win);
extern DLLEXPORT int                  pfQueryWin(pfWindow* _win, int _which, int *_dst);
extern DLLEXPORT int                  pfMQueryWin(pfWindow* _win, int *_which, int *_dst);
extern DLLEXPORT pfWindow*            pfOpenNewNoPortWin(const char *name, int screen);
extern DLLEXPORT void                 pfWinWndClassName(pfWindow* _win, char *name);
extern DLLEXPORT HCURSOR              pfGetWin32Cursor(const pfWindow* _win);
extern DLLEXPORT void                 pfWin32Cursor(pfWindow* _win, HCURSOR _cursor);
#define VisualID int

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfVideoChannel Tokens ----------------------- */

/* pfVChanMode */
#define PFVCHAN_SYNC		0
#define PFVCHAN_AUTO_APPLY	1

#define PFVCHAN_SYNC_FIELD	0
#define PFVCHAN_SYNC_SWAP	1


#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfVideoChannel CAPI ------------------------------*/

extern DLLEXPORT pfVideoChannel*      pfNewVChan(void *arena);
extern DLLEXPORT pfType*              pfGetVChanClassType(void);
extern DLLEXPORT void                 pfVChanWSWindow(pfVideoChannel* _vchan, pfWSWindow _wsWin);
extern DLLEXPORT pfWSWindow           pfGetVChanWSWindow(const pfVideoChannel* _vchan);
extern DLLEXPORT void                 pfGetVChanOrigin(pfVideoChannel* _vchan, int *_xo, int *_yo);
extern DLLEXPORT void                 pfGetVChanSize(pfVideoChannel* _vchan, int *_xs, int *_ys);
extern DLLEXPORT void                 pfGetVChanScreenOutputOrigin(pfVideoChannel* _vchan, int *_xo, int *_yo);
extern DLLEXPORT void                 pfVChanOutputOrigin(pfVideoChannel* _vchan, int _xo, int _yo);
extern DLLEXPORT void                 pfGetVChanOutputOrigin(pfVideoChannel* _vchan, int *_xo, int *_yo);
extern DLLEXPORT void                 pfVChanOutputSize(pfVideoChannel* _vchan, int _xs, int _ys);
extern DLLEXPORT void                 pfGetVChanOutputSize(pfVideoChannel* _vchan, int *_xs, int *_ys);
extern DLLEXPORT void                 pfVChanAreaScale(pfVideoChannel* _vchan, float _s);
extern DLLEXPORT float                pfGetVChanAreaScale(pfVideoChannel* _vchan);
extern DLLEXPORT void                 pfVChanScale(pfVideoChannel* _vchan, float _xs, float _ys);
extern DLLEXPORT void                 pfGetVChanScale(const pfVideoChannel* _vchan, float *_xs, float *_ys);
extern DLLEXPORT void                 pfVChanMinScale(pfVideoChannel* _vchan, float _xs, float _ys);
extern DLLEXPORT void                 pfGetVChanMinScale(const pfVideoChannel* _vchan, float *_xs, float *_ys);
extern DLLEXPORT void                 pfVChanMaxScale(pfVideoChannel* _vchan, float _xs, float _ys);
extern DLLEXPORT void                 pfGetVChanMaxScale(const pfVideoChannel* _vchan, float *_xs, float *_ys);
extern DLLEXPORT void                 pfGetVChanMinDeltas(const pfVideoChannel* _vchan, int *_dx, int *_dy);
extern DLLEXPORT void                 pfVChanFullRect(pfVideoChannel* _vchan);
extern DLLEXPORT void                 pfVChanScreen(pfVideoChannel* _vchan, int _screen);
extern DLLEXPORT int                  pfGetVChanScreen(const pfVideoChannel* _vchan);
extern DLLEXPORT void                 pfVChanId(pfVideoChannel* _vchan, int _index);
extern DLLEXPORT int                  pfGetVChanId(pfVideoChannel* _vchan);
extern DLLEXPORT void                 pfVChanMode(pfVideoChannel* _vchan, int mode, int val);
extern DLLEXPORT int                  pfGetVChanMode(pfVideoChannel* _vchan, int mode);
extern DLLEXPORT void                 pfVChanCallig(pfVideoChannel* _vchan, pfCalligraphic *_calligraphic);
extern DLLEXPORT pfCalligraphic*      pfGetVChanCallig(const pfVideoChannel* _vchan);
extern DLLEXPORT pfWSVideoChannelInfo pfGetVChanInfo(pfVideoChannel* _vchan);
extern DLLEXPORT void                 pfSelectVChan(pfVideoChannel* _vchan);
extern DLLEXPORT void                 pfApplyVChan(pfVideoChannel* _vchan);
extern DLLEXPORT void                 pfBindVChan(pfVideoChannel* _vchan);
extern DLLEXPORT void                 pfUnbindVChan(pfVideoChannel* _vchan);
extern DLLEXPORT int                  pfIsVChanBound(const pfVideoChannel* _vchan);
extern DLLEXPORT int                  pfIsVChanActive(pfVideoChannel* _vchan);

#endif /* !PF_C_API */


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

extern void 	pfSelectCallig(pfCalligraphic *_calligraphic);
extern DLLEXPORT pfCalligraphic * pfGetCurCallig(void);

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfCalligraphic CAPI ------------------------------*/

extern DLLEXPORT pfCalligraphic*      pfNewCallig(void *arena);
extern DLLEXPORT pfType*              pfGetCalligClassType(void);
extern DLLEXPORT int                  pfCalligInitBoard(int _numPipe);
extern DLLEXPORT unsigned int         pfCalligQueryBoard(int _numPipe);
extern DLLEXPORT int                  pfCalligCloseBoard(int _numPipe);
extern DLLEXPORT int                  pfCalligIsBoardInited(int _numPipe);
extern DLLEXPORT int                  pfGetCalligDeviceId(int _numPipe);
extern DLLEXPORT int                  pfCalligPartition(int _board, size_t *_allocate, int _n);
extern DLLEXPORT int                  pfCalligWaitForVME(int _board);
extern DLLEXPORT int                  pfCalligWaitForVISI(int _board);
extern DLLEXPORT void                 pfCalligSwapVME(int _board);
extern DLLEXPORT int                  pfGetCalligBoardMemSize(int _board);
extern DLLEXPORT void                 pfCalligZFootPrintSize(pfCalligraphic* _callig, float _size);
extern DLLEXPORT float                pfGetCalligZFootPrintSize(pfCalligraphic* _callig);
extern DLLEXPORT void                 pfCalligDrawTime(pfCalligraphic* _callig, float _time);
extern DLLEXPORT float                pfGetCalligDrawTime(pfCalligraphic* _callig);
extern DLLEXPORT void                 pfCalligFilterSize(pfCalligraphic* _callig, unsigned int _sizeX, unsigned int _sizeY);
extern DLLEXPORT void                 pfGetCalligFilterSize(pfCalligraphic* _callig, unsigned int *_sizeX, unsigned int *_sizeY);
extern DLLEXPORT void                 pfCalligDefocus(pfCalligraphic* _callig, float _defocus);
extern DLLEXPORT float                pfGetCalligDefocus(pfCalligraphic* _callig);
extern DLLEXPORT void                 pfCalligRasterDefocus(pfCalligraphic* _callig, float _defocus);
extern DLLEXPORT float                pfGetCalligRasterDefocus(pfCalligraphic* _callig);
extern DLLEXPORT void                 pfCalligStress(pfCalligraphic* _callig, float _stress);
extern DLLEXPORT float                pfGetCalligStress(pfCalligraphic* _callig);
extern DLLEXPORT int                  pfCalligChannel(pfCalligraphic* _callig, int _pipe, int _channel);
extern DLLEXPORT int                  pfGetCalligChannel(pfCalligraphic* _callig, int *_pipe, int *_channel);
extern DLLEXPORT void                 pfCalligWin(pfCalligraphic* _callig, float _xmin, float _ymin, float _width, float _height);
extern DLLEXPORT void                 pfGetCalligWin(pfCalligraphic* _callig, float *_xmin, float *_ymin, float *_width, float *_height);
extern DLLEXPORT void                 pfCalligMultisample(pfCalligraphic* _callig, int _n);
extern DLLEXPORT int                  pfGetCalligMultisample(pfCalligraphic* _callig);
extern DLLEXPORT int                  pfCalligDownLoadSlewTable(pfCalligraphic* _callig, pfCalligSlewTableEnum offset, pfCalligSlewTable Slew);
extern DLLEXPORT int                  pfCalligUpLoadSlewTable(pfCalligraphic* _callig, pfCalligSlewTableEnum offset, pfCalligSlewTable Slew);
extern DLLEXPORT int                  pfCalligDownLoadGammaTable(pfCalligraphic* _callig, pfCalligGammaTableEnum offset, pfCalligGammaTable Gamma);
extern DLLEXPORT int                  pfCalligUpLoadGammaTable(pfCalligraphic* _callig, pfCalligGammaTableEnum offset, pfCalligGammaTable Gamma);
extern DLLEXPORT void                 pfCalligExposureRatio(pfCalligraphic* _callig, float ratio);
extern DLLEXPORT float                pfGetCalligExposureRatio(pfCalligraphic* _callig);
extern DLLEXPORT void                 pfCalligXYSwap(pfCalligraphic* _callig, int flag);
extern DLLEXPORT int                  pfGetCalligXYSwap(pfCalligraphic* _callig);
extern DLLEXPORT void                 pfCalligProjMatrix(pfCalligraphic* _callig, pfMatrix *projMat);
extern DLLEXPORT void                 pfGetCalligProjMatrix(pfCalligraphic* _callig, pfMatrix *projMat);
extern DLLEXPORT void                 pfCollectCalligStats(pfCalligraphic* _callig, pfStats *stats);
extern DLLEXPORT int                  pfCalligIsInited(pfCalligraphic* _callig);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ------------------ pfCalligraphic Macros --------------------- */
#if defined(__STDC__) || defined(__cplusplus)

#endif /* defined(__STDC__) || defined(__cplusplus) */
#endif /* !PF_CPLUSPLUS_API */


#ifdef PF_C_API
/*---------------- pfCombiner CAPI ------------------------------*/

extern DLLEXPORT pfCombiner*          pfNewCombiner(void *arena);
extern DLLEXPORT pfType*              pfGetCombinerClassType(void);
extern DLLEXPORT void                 pfApplyCombiner(pfCombiner* _combiner);
extern DLLEXPORT void                 pfGeneralInputCombiner(pfCombiner* _combiner, GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
extern DLLEXPORT void                 pfGeneralOutputCombiner(pfCombiner* _combiner, GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum);
extern DLLEXPORT void                 pfFinalInputCombiner(pfCombiner* _combiner, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
extern DLLEXPORT void                 pfActiveCombinersCombiner(pfCombiner* _combiner, int count);
extern DLLEXPORT void                 pfActiveConstColorsCombiner(pfCombiner* _combiner, int count);
extern DLLEXPORT void                 pfCombinerConstColor0(pfCombiner* _combiner, const pfVec4 color);
extern DLLEXPORT void                 pfCombinerConstColor1(pfCombiner* _combiner, const pfVec4 color);
extern DLLEXPORT void                 pfCombinerGeneralConstColor0(pfCombiner* _combiner, GLenum stage, const pfVec4 color);
extern DLLEXPORT void                 pfCombinerGeneralConstColor1(pfCombiner* _combiner, GLenum stage, const pfVec4 color);
extern DLLEXPORT void                 pfCombinerFinalConstColor0(pfCombiner* _combiner, const pfVec4 color);
extern DLLEXPORT void                 pfCombinerFinalConstColor1(pfCombiner* _combiner, const pfVec4 color);
extern DLLEXPORT int                  pfPrintCombiner(pfCombiner* _combiner, uint _travMode, uint _verbose, char *prefix, FILE *file);
extern DLLEXPORT int                  pfGetMaxGeneralCombiners(pfCombiner* _combiner);

#endif /* !PF_C_API */


#ifdef PF_C_API
/*---------------- pfISLTexCoordData CAPI ------------------------------*/

extern DLLEXPORT pfISLTexCoordData*   pfNewISLTexCoordData(void *arena);
extern DLLEXPORT pfType*              pfGetISLTexCoordDataClassType(void);
extern DLLEXPORT void*                pfISLTexCoordDataBorrowMemory(pfISLTexCoordData* _isltexcoorddata, int size);
extern DLLEXPORT islAppearance*       pfGetISLTexCoordDataAppearance(pfISLTexCoordData* _isltexcoorddata);
extern DLLEXPORT pfGeoSet*            pfGetISLTexCoordDataGSet(pfISLTexCoordData* _isltexcoorddata);
extern DLLEXPORT pfMatrix*            pfGetISLTexCoordDataModelview(pfISLTexCoordData* _isltexcoorddata);
extern DLLEXPORT pfMatrix*            pfGetISLTexCoordDataViewmat(pfISLTexCoordData* _isltexcoorddata);

#endif /* !PF_C_API */
#endif /* PF_C_API */

#ifdef __cplusplus
}
#endif

#endif /* __PR_H__ */


