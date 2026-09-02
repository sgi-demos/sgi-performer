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
// pfIsector.h		Isector class include file
//
// $Revision: 1.44 $
// $Date: 2002/05/30 00:33:56 $
//
#ifndef __PF_ISECTOR_H__
#define __PF_ISECTOR_H__


#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf/pfTraverser.h>
#include <Performer/pr/pfGeoSet.h>


#if 0
#define PFISECTOR_INIT(isector, nSegs) 					\
    PFTRAVERSER_INIT(isector) 						\
    aa = alloca(sizeof(pfNode*) * PFTRAV_DEPTH_MAX);  			\
    (isector)->curIsectPath = (pfNode**) aa;			        \
    aa = alloca(sizeof(pfSegSet) * PFTRAV_DEPTH_MAX);  			\
    (isector)->segSetStack = (pfSegSet*) aa;			        \
    (isector)->curSegSet = (isector)->segSetStack; 		   	\
    aa = alloca(sizeof(pfCylinder) * PFTRAV_DEPTH_MAX);			\
    (isector)->cylStack = (pfCylinder*)	aa;
#else
// Yair: Add double-precision members.
#define PFISECTOR_INIT(isector, nSegs) 					\
    PFTRAVERSER_INIT(isector) 						\
    \
    aa = alloca(sizeof(pfNode*) * PFTRAV_DEPTH_MAX);  			\
    (isector)->curIsectPath = (pfNode**) aa;			        \
    \
    aa = alloca(sizeof(pfSegSet) * PFTRAV_DEPTH_MAX);  			\
    (isector)->segSetStack = (pfSegSet*) aa;			        \
    (isector)->curSegSet = (isector)->segSetStack; 		   	\
    \
    aa = alloca(sizeof(pfCylinder) * PFTRAV_DEPTH_MAX);			\
    (isector)->cylStack = (pfCylinder*)	aa;				\
    \
    aa = alloca(sizeof(pfSegSetd) * PFTRAV_DEPTH_MAX);  		\
    (isector)->segSetStackd = (pfSegSetd*) aa;			        \
    (isector)->curSegSetd = (isector)->segSetStackd; 		   	\
    \
    aa = alloca(sizeof(int) * PFTRAV_DEPTH_MAX);  			\
    (isector)->precision = (int *) aa;			        	\
    (isector)->curPrecision = (isector)->precision; 		   	\
    \
    aa = alloca(sizeof(pfCylinderd) * PFTRAV_DEPTH_MAX);		\
    (isector)->cylStackd = (pfCylinderd*)	aa;
#endif

class pfChannel;

#define _PFISECTOR ((_pfIsector*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define _PFISECTORBUFFER ((_pfIsector*)buf->pf_indexUpdatable(this))

class DLLEXPORT _pfIsector : public pfTraverser
{
PFINTERNAL:
    _pfIsector();
    // no destructor


PFINTERNAL:
    void*		userData;       // userData from segSet
    uint 		isectMask;	// isectMask from segSet
    int			(*discFunc)(pfHit*);

    pfHit	     	*hits[32];	// array of pointers to hits

    // maintain a stack of segsets for easy passing into libpr
    // only the segment portion is kept current until required
    // for passing into libpr

    uint 		activeStack[PFTRAV_DEPTH_MAX];	// stack of active masks
    uint 		*curActive;

    int 		segSP;		 // xform stack index 

    // Single precision version of stacks.
    pfSegSet		*segSetStack;	 // stack of xformed clipped segs
    pfSegSet		*curSegSet;	 // pointer to top of segSetStack
    pfCylinder		*cylStack;	 // stack of xformed cylinders
    pfCylinder		*curCyl;	 // pointer to top of cylStack

    // Double-precision version of stacks.
    pfSegSetd		*segSetStackd;	 // stack of xformed clipped segs
    pfSegSetd		*curSegSetd;	 // pointer to top of segSetStack
    pfCylinderd		*cylStackd;	 // stack of xformed cylinders
    pfCylinderd		*curCyld;	 // pointer to top of cylStack

    int			*precision;	 // Stack of precision level (SP or DP).
    int			*curPrecision;   // pointer to top of stack.

    int 		numSegs;	 // max number of segments (may be fewer active)
    pfPath*		tmpPath;         // for scratch use in returns
    int 		tmpPathValid;	 // tmpPath has been set for current node
    int 		pfHitValidMask;// bitmask 1bit/segment.  when set indicates
					 // libpf portions of hit already set for node 

    pfNode**		curIsectPath;	// path to curNode
                                        // simple array for speed
public:
    void *_pfIsectorExtraData;
};

#endif	// !__PF_ISECTOR_H__
