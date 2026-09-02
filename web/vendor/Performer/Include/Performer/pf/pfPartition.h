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
// pfPartition.C		Cell Set Spatial Organizer
//
// $Revision: 1.51 $
// $Date: 2002/05/30 00:33:56 $
//

#ifndef __PF_PARTITION_H__
#define __PF_PARTITION_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfGroup.h>

class pfBuffer;
class _pfCell;

class _pfGGPairList;
class _pfCellList;


extern "C" {     // EXPORT to CAPI

/* ----------------------- pfPartition Tokens ----------------------- */

/* pfPartVal() */
/* for controlling # pfGeoSets/cell in automatic build */
#define PFPART_FINENESS		1 
#define PFPART_DEBUG		2

/* pfPartAttr() for specifing explicit parameters for the partition */
#define PFPART_ORIGIN	  	1
#define PFPART_MIN_SPACING	2
#define PFPART_MAX_SPACING	3


} // extern "C" END of C include export

#define PFPARTITION ((pfPartition*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFPARTITIONBUFFER ((pfPartition*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfPartition : public pfGroup
{
public:

    void setVal(int which, float val)  {
        PFPARTITION->nb_setVal(which, val);
    }

    float getVal(int which)  {
        return PFPARTITION->nb_getVal(which);
    }

    void setAttr(int  which, void *attr)  {
        PFPARTITION->nb_setAttr(which, attr);
    }

    void* getAttr(int  which)  {
        return PFPARTITION->nb_getAttr(which);
    }

    void build()  {
        PFPARTITION->nb_build();
    }

    void update()  {
        PFPARTITION->nb_update();
    }
public:		// Constructors/Destructors
    //CAPI:basename Part
    //CAPI:updatable
    //CAPI:newargs
    pfPartition();
    virtual ~pfPartition();

protected:
    //CAPI:private
    pfPartition(pfBuffer *buf);
    pfPartition(const pfPartition *prev, pfBuffer *buf); 	// for cloning

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }
    
public:			// pfMemory virtual functions
    
PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int  upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);


PFINTERNAL:		// pfNode virtual traversals
    virtual void	    nb_clean(uint64_t cleanBits, pfNodeCounts *counts);
    virtual pfNode*	    nb_clone();
    virtual int  	    nb_intersect(_pfIsector *isector);
    virtual int 	    nb_cull(int  mode, int  cullResult, _pfCuller *trav);
    virtual int 	    nb_cullProgram(int  mode, int  cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);

PFINTERNAL:		// specific sets and gets
    void	nb_setVal(int which, float val);
    float	nb_getVal(int which);
    void	nb_setAttr(int  which, void *attr);
    void*	nb_getAttr(int  which);
    
PFINTERNAL:		// specific methods
    //CAPI:verb
    void	nb_build();
    void	nb_update();


protected:

    pfVec3	minSpacing;
    pfVec3	maxSpacing;
    pfVec3	origin;
    int		flags;
    float	fineness;
    pfVec3	min;
    pfVec3	max;
    pfVec3	delta;
    pfVec3	deltaInv;
    int 	nCells[3];
    _pfGGPairList *ggPairList;	/* list of ggpairs */
    _pfCellList	*cellList;	/* list of cells */
    pfNodeList	*funkyNodeList;	/* list of DCSes, SCSes, swtiches and sequences */
    _pfVoidList	*pathList;	/* list of paths */

    pfGeode	*gridLines;	/* for diagnostic purposes */

private:
    static pfType *classType;
};


#endif // !__PF_PARTITION_H__
