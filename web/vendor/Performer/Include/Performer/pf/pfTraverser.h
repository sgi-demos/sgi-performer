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
// pfTraverser.h		Traverser class include file
//
// $Revision: 1.101 $
// $Date: 2002/10/03 20:53:50 $
//

#ifndef __PF_TRAVERSER_H__
#define __PF_TRAVERSER_H__

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


#include <Performer/pf/pfFrameStats.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf.h>
#include <Performer/pr/pfStruct.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGenericMatrix.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfList.h>



extern "C" {     // EXPORT to CAPI
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

} // extern "C" END of C include export

#define PFPATH ((pfPath*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFPATHBUFFER ((pfPath*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfPath : public pfList
{
public:
    //CAPI:basename Path
    //CAPI:newargs
    pfPath() : pfList(sizeof(pfNode*), 4) {}


public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

private:
    static pfType *classType;
};

extern "C" {     // EXPORT to CAPI

/* ------------------ pfTraverser Related Functions--------------------- */

extern DLLEXPORT void	pfCullResult(int _result);
extern DLLEXPORT int	pfGetParentCullResult(void);
extern DLLEXPORT int	pfGetCullResult(void);
extern DLLEXPORT int	pfCullPath(pfPath *_path,  pfNode *_root, int _mode);

#define pfCullPath(_path, _root, _mode) \
	pfCullPath(_path, (pfNode*)_root, _mode)

} // extern "C" END of C include export

class DLLEXPORT pfTraverser : public pfStruct
{
public:
    //CAPI:basename Trav
    //CAPI:private
    pfTraverser();
    virtual ~pfTraverser();

public:
    pfChannel*  getChan() const;	// APPID Channel 
    pfNode *	getNode() const;	// APPID Node

    void	getMat(pfMatrix &mat) const {
	mStack.get4f(mat);
    }

    void	getMatD(pfMatrix4d &mat) const {
	mStack.get4d(mat);
    }

    int         isSinglePrecision() { return (mStack.isSinglePrecision()); }

    int		getIndex() const {
	return childIndex;
    }
    const pfPath* getPath() const {
	return &curPath;
    }


private:

    uint 			mode;
    pfChannel			*channel;	// XXX Maybe this belongs in
					   	// _pfCuller only
    pfNode			*curNode;
    pfPath			curPath;
    pfNode			**curPathArray;	// For fast curPath access
    int 			curDepth;	

    void*			userData;
    int 			whichChild;
    int 			childIndex;

    FILE			*file;
    int 			indentLevel;

    pfGenericMatStack		mStack;
    int 			xformDepth;

    int				invTransposeValid;
    pfMatrix			invTransposeMat;

    pfPath			*targetPath;
    int 			targetPathIndex;
    int 			isPathDriven;
    int 			pathMode;
    uint 			evalSwitches;
    
    // for user traversals
    int 			(*preFunc)(pfTraverser *trav);
    int 			(*postFunc)(pfTraverser *trav);
    uint 			masks[4];
    char			pathName[PF_MAXSTRING];
   
    int 			countStats;
    int				cbCounts;

    pfNodeCounts 		nodesTraversed;
    pfNodeCounts 		nodesEvaluated;
};

#endif	// !__PF_TRAVERSER_H__

