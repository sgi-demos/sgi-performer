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
// pfSCS.h		Static Coordinate Systems include file
//
// $Revision: 1.53 $
// $Date: 2002/05/30 00:33:56 $
//
#ifndef __PF_SCS_H__
#define __PF_SCS_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfGroup.h>

class pfBuffer;

#define PFSCS ((pfSCS*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFSCSBUFFER ((pfSCS*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfSCS : public pfGroup
{
public:

    void getMat(pfMatrix& m)  {
        PFSCS->nb_getMat(m);
    }

    const pfMatrix* getMatPtr()  {
        return PFSCS->nb_getMatPtr();
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfSCS(pfMatrix& m);
    virtual ~pfSCS();

protected:
    //CAPI:private
    pfSCS(pfBuffer *buf, pfMatrix &m);
    pfSCS(const pfSCS *prev, pfBuffer *buf);	// for cloning

    // constructor used by subclasses
    pfSCS(pfBuffer *buf); // no mat alloc or update for subclasses
    
public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);

PFINTERNAL:		// pfNode virtual traversals
    virtual void	    nb_clean(uint64_t cleanBits, pfNodeCounts *counts);
    virtual pfNode*	    nb_clone();
    virtual int 	    app(pfTraverser *trav);
    virtual int 	    ASDtrav(pfTraverser *trav);
    virtual int 	    nb_cull(int mode, int cullResult, _pfCuller *trav);
    virtual int 	    nb_cullProgram(int mode, int cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int 	    nb_flatten(pfTraverser *trav);
    virtual int 	    nb_intersect(_pfIsector *isector);
    virtual void 	    nb_write(pfTraverser *trav, uint which, uint verbose);

PFINTERNAL:		// pfSCS virtual traversals
    //CAPI:virtual
    virtual void 	    nb_getMat(pfMatrix& m);
    virtual const pfMatrix* nb_getMatPtr() {
	return xformp;
    }


protected:

    pfMatrix	*xformp;	// Pointer -> don't multi buffer matrix
    pfMatrix	*xformInvp;	// Pointer -> don't multi buffer matrix
    float	maxScale;
    int		nonUnitScale;

private:
    static pfType *classType;
    
};

#endif // !__PF_SCS_H__
