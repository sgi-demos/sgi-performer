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
// pfDispListOptimizer.h
//
// $Revision: 1.23 $
// $Date: 2002/11/21 02:39:28 $
//
//
#ifndef __PF_DISP_LIST_OPTIMIZER_H__
#define __PF_DISP_LIST_OPTIMIZER_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfDispList.h>
#include <Performer/pf/pfLists.h>

class pfDispListOptimizer;

extern "C" 
{
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

}



#define PFDISPLISTOPTIMIZER ((pfDispListOptimizer*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFDISPLISTOPTIMIZERBUFFER ((pfDispListOptimizer*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfDispListOptimizer : public pfObject
{
public:		// Constructors/Destructors
    //CAPI:basename DLOptimizer

    pfDispListOptimizer();
    virtual ~pfDispListOptimizer();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:	

    void	setDispList(pfDispList *_dlist) { dlist = _dlist; }

    void	reset(void);
    int		advance(void); // non-zero if advanced anywhere.

    void        setNumServers(int num){numServers = num;}
    int         getNumServers(void){return numServers;}

    void        setMyId(int id){myId = id;}
    int         getMyId(void){return myId;}

    void        setChannel(pfChannel *c);
    pfChannel   *getChannel(void){return chan;}

    void	initModelviewMatrix(pfMatrix& M);
    void	initProjectionMatrix(pfMatrix& M);

    pfGeoSet 	*pf_cloneGSet(pfGeoSet *gset, int *attr_size, int lengths);

    int		getNumGSetsProcessed(void) {return numGSetsProcessed;}

    void	setTimeSlice(float t) {timeSlice = t;}
    float	getTimeSlice(void) {return timeSlice;}

    void	    setOptimizationMask(unsigned int m) {optimizationMask = m;}
    unsigned int    getOptimizationMask(void) {return optimizationMask;}

    int		getNumTrisRemoved(void) { return numTrisRemoved; }
    int		getNumLinesRemoved(void) { return numLinesRemoved; }
    int		getNumPointsRemoved(void) { return numPointsRemoved; }
    int		getNumVertsRemoved(void) { return numVertsRemoved; }

    void		setUserFunc(pfSidekickFunc func, void *userFuncData);
    pfSidekickFunc	getUserFunc(void) { return userFunc; }
    void		*getUserFuncData(void) { return userFuncData; }

    void		setCullFace(int mode);
    int			getCullFace(void) { return cullFaceMode; }

    // ============================================
    // Methods for use by user callback function.
    // ============================================

    void 		*getTmpMemory(int size);
    pfGeoSet		*getTmpGSet(void);
    pfMatrix		*getMatrix(int which);
    pfGeoSet		*cloneGSet(pfGeoSet *gset, unsigned int copyMask);

    pfGeoSet		*optimize(pfGeoSet *gset);


PFINTERNAL:

protected: // Members.

    DLElt		*dlistHead;
    pfDispList		*dlist;
    int			dlistSize;

    DLLink		*dlistLink;

    int			numServers, myId;
    int			gsetCounter;
    int			linkIndex;
    int			numGSetsProcessed;

    int			numTrisRemoved;
    int			numPointsRemoved;
    int			numLinesRemoved;
    int			numVertsRemoved;

    pfMatStack		*modelViewMat;
    pfMatStack		*projectionMat;
    pfMatrix		combinedMat;

    int			cullFaceMode;
    int			defaultCullFaceMode;
    int			overrideCullFaceMode;

    float		cullFaceSign;

    int			doneDL;
    pfChannel		*chan;

    float		timeSlice; // Time limit for a single advance 
				   // invokation.

    unsigned int	optimizationMask;

    pfSidekickFunc	userFunc;
    void		*userFuncData;

    // Use this to make binary-compatible changes.
    pfDispListOptimizerExtraData	*dlOptimizerExtraData;

private:
    static pfType *classType;
};

#endif // !__PF_DISP_LIST_OPTIMIZER_H__
