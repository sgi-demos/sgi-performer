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
// pfDCS.h		Dynamic Coordinate Systems include file
//
// $Revision: 1.66 $
// $Date: 2002/05/30 00:33:56 $
//

#ifndef __PF_DCS_H__
#define __PF_DCS_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfSCS.h>


class pfBuffer;

extern "C" {     // EXPORT to CAPI

/* ----------------------- pfDCS Tokens ----------------------- */

#define PFDCS_MATRIX_TYPE	1
#define PFDCS_UNCONSTRAINED	0xffffffff
} // extern "C" END of C include export

#define PFDCS ((pfDCS*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFDCSBUFFER ((pfDCS*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfDCS : public pfSCS
{
public:

    void getMat(pfMatrix& m)  {
        PFDCS->nb_getMat(m);
    }

    const pfMatrix* getMatPtr()  {
        return PFDCS->nb_getMatPtr();
    }

    void setMatType(uint val)  {
        PFDCS->nb_setMatType(val);
    }

    uint getMatType() const  {
        return PFDCS->nb_getMatType();
    }

    void setMat(pfMatrix& m)  {
        PFDCS->nb_setMat(m);
    }

    void setCoord(pfCoord *c)  {
        PFDCS->nb_setCoord(c);
    }

    void setRot(float h, float p, float r)  {
        PFDCS->nb_setRot(h, p, r);
    }

    void setTrans(float x, float y, float z)  {
        PFDCS->nb_setTrans(x, y, z);
    }

    void setScale(float s)  {
        PFDCS->nb_setScale(s);
    }

    void setScale(float xs, float ys, float zs)  {
        PFDCS->nb_setScale(xs, ys, zs);
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfDCS();
    virtual ~pfDCS();

protected:
    pfDCS(pfBuffer *buf);
    pfDCS(const pfDCS *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:		// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);

PFINTERNAL:		// pfNode virtual traversals
    virtual void	    nb_clean(uint64_t cleanBits, pfNodeCounts *counts);
    virtual pfNode*	    nb_clone();
    virtual int 	    nb_flatten(pfTraverser *trav);
    virtual int 	    nb_intersect(_pfIsector *isector);
    virtual void 	    nb_write(pfTraverser *trav, uint which, uint verbose);

PFINTERNAL:		// pfSCS virtual traversals
    //CAPI:virtual
    virtual void 	    nb_getMat(pfMatrix& m);
    virtual const pfMatrix* nb_getMatPtr();

PFINTERNAL:		// class specific sets and gets
    //CAPI:backdoor
    void	nb_setMatType(uint val);
    uint 	nb_getMatType() const;

    void	nb_setMat(pfMatrix& m);

    void	nb_setCoord(pfCoord *c);
    void	nb_setRot(float h, float p, float r);
    void	nb_setTrans(float x, float y, float z);

    void	nb_setScale(float s);
    //CAPI:verb DCSScaleXYZ
    void	nb_setScale(float xs, float ys, float zs);

protected:
    int 	dcsFlags;
    uint 	matType;
    pfCoord	coord;
    pfVec3	scales;
    pfSphere	childBSphere;
    pfVec3	matScales;      // contains scales currently in xform mat 
    pfMatrix	xform;
    pfMatrix	xformInv;

private:
    static pfType *classType;
};

#endif // !__PF_DCS_H__
