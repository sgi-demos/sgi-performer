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
// pfDoubleDCS.h       Double Precision Dynamic Coordinate Systems include file
//
// $Revision: 1.3 $
// $Date: 2002/05/30 00:33:56 $
//

#ifndef __PF_DOUBLE_DCS_H__
#define __PF_DOUBLE_DCS_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfDoubleSCS.h>


class pfBuffer;

extern "C" {     // EXPORT to CAPI

/* ----------------------- pfDoubleDCS Tokens ----------------------- */

#define PFDOUBLEDCS_MATRIX_TYPE	1
#define PFDOUBLEDCS_UNCONSTRAINED	0xffffffff
} // extern "C" END of C include export

#define PFDOUBLEDCS ((pfDoubleDCS*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFDOUBLEDCSBUFFER ((pfDoubleDCS*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfDoubleDCS : public pfDoubleSCS
{
public:

    void getMat(pfMatrix4d& m)  {
        PFDOUBLEDCS->nb_getMat(m);
    }

    const pfMatrix4d* getMatPtr()  {
        return PFDOUBLEDCS->nb_getMatPtr();
    }

    void setMatType(uint val)  {
        PFDOUBLEDCS->nb_setMatType(val);
    }

    uint getMatType() const  {
        return PFDOUBLEDCS->nb_getMatType();
    }

    void setMat(pfMatrix4d& m)  {
        PFDOUBLEDCS->nb_setMat(m);
    }

    void setCoordd(pfCoordd *c)  {
        PFDOUBLEDCS->nb_setCoordd(c);
    }

    void setRot(double h, double p, double r)  {
        PFDOUBLEDCS->nb_setRot(h, p, r);
    }

    void setTrans(double x, double y, double z)  {
        PFDOUBLEDCS->nb_setTrans(x, y, z);
    }

    void setScale(double s)  {
        PFDOUBLEDCS->nb_setScale(s);
    }

    void setScale(double xs, double ys, double zs)  {
        PFDOUBLEDCS->nb_setScale(xs, ys, zs);
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfDoubleDCS();
    virtual ~pfDoubleDCS();

protected:
    pfDoubleDCS(pfBuffer *buf);
    pfDoubleDCS(const pfDoubleDCS *prev, pfBuffer *buf);

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

PFINTERNAL:		// pfDoubleSCS virtual traversals
    //CAPI:virtual
    virtual void 	    nb_getMat(pfMatrix4d& m);
    virtual const pfMatrix4d* nb_getMatPtr();

PFINTERNAL:		// class specific sets and gets
    //CAPI:backdoor
    void	nb_setMatType(uint val);
    uint 	nb_getMatType() const;

    void	nb_setMat(pfMatrix4d& m);

    void	nb_setCoordd(pfCoordd *c);
    void	nb_setRot(double h, double p, double r);
    void	nb_setTrans(double x, double y, double z);

    void	nb_setScale(double s);
    //CAPI:verb DoubleDCSScaleXYZ
    void	nb_setScale(double xs, double ys, double zs);

protected:
    int 	dcsFlags;
    uint 	matType;
    pfCoordd	coord;
    pfVec4d	scales;
    pfSpheredd	childBSphere;
    pfVec4d	matScales;      // contains scales currently in xform mat 
    pfMatrix4d	xform;
    pfMatrix4d	xformInv;

private:
    static pfType *classType;
};

#endif // !__PF_DOUBLE_DCS_H__
