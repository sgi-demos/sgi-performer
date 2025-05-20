// 
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
// pfState.h
//
// $Revision: 1.123 $
// $Date: 2002/03/20 03:20:47 $
//
#ifndef __PF_STATE_H__
#define __PF_STATE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif




#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfList.h>
#include <Performer/pr.h>

#include <windows.h>
#include <GL/gl.h>

#define PFMV_MODEL_TYPE		0x1
#define PFMV_INV_MODEL		0x2
#define PFMV_FULL_INV_MAT	0x4
#define PFMV_INV_VIEWPOINT	0x8
#define PFMV_INV_VIEWMAT	0x10


#define _PF_MAXSTACK    	32	/* Maximum number of entries in state stack */

// _pfGLNameSpace groups together all structures that map 
// pf indices to GL names and can be shared amongst pfState groups
typedef struct _pfGLNameSpace
{
    /* CAREFUL - this is a bad idea if PF_MAX_LIGHTS can ever be of any size */
    pfLight *lights[PF_MAX_LIGHTS]; /* store pfState-rel light names */
    int	    newDL;		/* next GL name to use */
    int	    maxDL;		/* max of current GL name range */
    pfList  *dls;		/* gl generated object names (dls) */
    GLuint  newTex;		/* next TextureObject name to use */
    int	    maxTex;		/* max of current TextureObject name range */
    pfList  *textures;		/* gl generated TextureObject names (dls) */
} _pfGLNameSpace;

extern "C" {     // EXPORT to CAPI
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

}

#define PFSTATE ((pfState*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFSTATEBUFFER ((pfState*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfState : public pfObject
{
public:
    // constructors and destructors
    pfState();
    virtual ~pfState();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);
	
public:
    // other functions
    // CAPI:verb
    void	select();
    void	load();
    void	attach(pfState *_state1);


PFINTERNAL: // XXX - should make protected???

    pfGeoState	    stack[_PF_MAXSTACK];	/* State stack */
    pfGeoState	    saveStack[_PF_MAXSTACK];/* Stack for backing up pfGeoState mods */
    pfGeoState      *curgs;             /* Current geostate, NOT top of stack */
    pfGeoState      *saveState;	   	/* Top of saveStack */
    int	    gsXformDirty; 	/* Used in IMApplyGState */
    uint64_t gsModvec;		/* modvec at the time of the last geostate */
    int	    sp;			/* Stack pointer */
    int	    dirtySize;		/* Size of dirty list */
    int	    *dirty;		/* List dirty values indexed by id */
    int	    lightDirtySize;	/* Size of lightDirty list */
    int	    *lightDirty;	/* List dirty values for lights indexed by id */
    int	    xformDirty;		/* Dirty value incremented by transforms */
    int	    transpOver;		/* transparency mechanism override */
    int	    decalOver;		/* decal mechanism override */
    float   pixScale;		/* dynamic resolution scaled video output factor */
    pfList  *curTex;		/* currently bound textures */
    pfList  *texState;		/* per-state texture state */
    _pfGLNameSpace *glNames;	/* gl name space items - can be shared */

    ushort  frontlmc, backlmc;  /* Current front/back material color mode */

    ushort  enlmc;		/* ColorMaterial enable flag */
    ushort  texTarget[PF_MAX_TEXTURES];	
				/* Currently enabled OpenGL texture target */

private:
    static pfType *classType;

};

#if defined(__linux__) || defined(WIN32)
/*#define PFPRDA(_type, _i) ((_type)PRDA[_i]) */
#define PFPRDA(_type, _i) (*((_type *)(&(PRDA[_i]))))
#else
#define PFPRDA(_type, _i) (((_type##*)PRDA->usr_prda.fill)[_i])
#endif  /* __linux__ */


      #if !defined(__linux__) || defined (__linux__has_MP__)
           #define pfCurDList	PFPRDA(pfDispList*, 16) /* MP Linux & IRIX */
      #else 
           extern pfDispList 	*pfCurDList;            /* SP Linux */
      #endif


extern pfGeoState	*pfCurGState;
extern pfGeoState	*pfSaveGState;
extern pfList		*pfCurGStateTable;
extern pfState		*pfCurState;
extern _pfGLNameSpace	*pfCurGLNames;
extern pfVec4		*pfCurCtab;
extern uint64_t		 pfCurModVec;
extern uint64_t		 pfInGStateFlag;


#endif // !__PF_STATE_H__
