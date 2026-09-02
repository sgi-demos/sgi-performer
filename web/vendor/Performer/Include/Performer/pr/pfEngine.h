//
//
// Copyright 1997, Silicon Graphics, Inc.
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
// pfEngine.h
//
// $Revision: 1.13 $
// $Date: 2002/03/14 21:11:10 $
//

#ifndef __PF_ENGINE_H__
#define __PF_ENGINE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfMemory.h>
#include <Performer/pr/pfList.h>
#include <Performer/pr/pfState.h>
#include <Performer/pr/pfLinMath.h>


typedef struct {
    void *data;
    ushort *ilist;
    int icount;
    int offset;
    int stride;
    int flags;
} engine_src_t;




// Exported Tokens
extern "C" {     // EXPORT to CAPI

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

}


#define PFENGINE ((pfEngine*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFENGINEBUFFER ((pfEngine*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfEngine : public pfObject
{
public:
    // Constructors, destructors
    pfEngine(int _function);
public:
    virtual ~pfEngine();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int	copy(const pfMemory *_src);
    virtual int	compare(const pfMemory *_mem) const;
    virtual int	print(uint _travMode, uint _verbose, char *_prefix, FILE *file);

public:
    // sets and gets
    void	setSrc(int _index, void *_data, ushort *_ilist, int _icount, int _offset, int _stride);
    void	getSrc(int _index, void **_data, ushort **_ilist, int *_icount, int *_offset, int *_stride) const;
    int		getNumSrcs(void) const;
    void	setDst(void *_data, ushort *_ilist, int _offset, int _stride);
    void	getDst(void **_data, ushort **_ilist, int *_offset, int *_stride) const;
    void	setIterations(int _iterations, int _itemsPer);
    void	getIterations(int *_iterations, int *_itemsPer) const;
    int		getFunction(void) const;
    void	setUserFunction(pfEngineFuncType _func);
    pfEngineFuncType	getUserFunction(void) const;
    void	setMode(int _mode, int _val);
    int		getMode(int _mode) const;
    void	setMask(uint _mask);
    uint	getMask(void) const;
    void	setEvaluationRange(pfVec3& _center, float _min, float _max);
    void	getEvaluationRange(pfVec3& _center, float *_min, float *_max) const;


public:
    // other functions
    void	srcChanged(void);
    void	evaluate(int _mask);
    // CAPI:verb EngineEvaluateEye
    void	evaluate(int _mask, pfVec3 _eye_pos);




private:
    pfEngineFuncType func;
    int function;
    int max_srcs;
    int alloc_srcs;
    int num_srcs;
    engine_src_t *srcs;
    void *dst_data;
    ushort *dst_ilist;
    int dst_offset;
    int dst_stride;
    int dst_flags;
    int iterations;
    int items;
    uint flags;
    uint mask;
    pfVec3 center;
    float min, max;

private:
    static pfType *classType;
};

#endif // !__PF_ENGINE_H__
