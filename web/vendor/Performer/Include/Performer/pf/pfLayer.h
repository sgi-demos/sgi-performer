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
// pfLayer.h		Layer class include file
//
// $Revision: 1.55 $
// $Date: 2002/05/30 00:33:56 $
//
#ifndef __PF_LAYER_H__
#define __PF_LAYER_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfLists.h>
#include <Performer/pf/pfChannel.h>

class pfBuffer;

#define PFLAYER ((pfLayer*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFLAYERBUFFER ((pfLayer*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfLayer : public pfGroup
{
public:

    void setBase(pfNode *n)  {
        PFLAYER->nb_setBase(n);
    }

    pfNode* getBase() const  {
        return PFLAYER->nb_getBase();
    }

    void setDecal(pfNode *n)  {
        PFLAYER->nb_setDecal(n);
    }

    pfNode* getDecal() const  {
        return PFLAYER->nb_getDecal();
    }

    void setMode(int mode)  {
        PFLAYER->nb_setMode(mode);
    }

    int getMode() const  {
        return PFLAYER->nb_getMode();
    }

    void setPlane(pfPlane *plane)  {
        PFLAYER->nb_setPlane(plane);
    }

    pfPlane* getPlane() const  {
        return PFLAYER->nb_getPlane();
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfLayer();
    virtual ~pfLayer();
    
protected:
    pfLayer(pfBuffer *buf);
    pfLayer(const pfLayer *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);

PFINTERNAL:		// pfNode virtual traversals
    virtual int 	    app(pfTraverser *trav);
    virtual int 	    ASDtrav(pfTraverser *trav);
    virtual pfNode*	    nb_clone();
    virtual int 	    nb_cull(int mode, int cullResult, _pfCuller *trav);
    virtual int 	    nb_cullProgram(int mode, int cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int 	    nb_intersect(_pfIsector *isector);
    virtual int 	    nb_traverse(pfTraverser *trav);
    
PFINTERNAL:		// Specific sets and gets
    void		    nb_setBase(pfNode *n);
    pfNode*		    nb_getBase() const {
	return childList.getApp(0);
    }

    void		    nb_setDecal(pfNode *n);
    pfNode*		    nb_getDecal() const {
	return childList.getApp(1);
    }

    void		    nb_setMode(int mode);
    int 		    nb_getMode() const {
	return baseMode;
    }

    void		    nb_setPlane(pfPlane *plane);
    pfPlane* 		    nb_getPlane(void) const {
	return plane;
    }


protected:
    int			    baseMode;		
    pfPlane		    *plane;

private:
    static pfType *classType;
};


#endif // !__PF_LAYER_H__
