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
// pfBillboard.h		Billboard include file
//
// $Revision: 1.63 $
// $Date: 2002/11/05 22:48:15 $
//
#ifndef __PF_BILLBOARD_H__
#define __PF_BILLBOARD_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfLists.h>
#include <Performer/pf/pfCuller.h>
#include <Performer/pr/pfSprite.h>


extern "C" {     // EXPORT to CAPI

/* ----------------------- pfBillboard Tokens ----------------------- */

/* Modes */
#define PFBB_ROT		PFSPRITE_ROT

/* Rotation values */
#define PFBB_AXIAL_ROT		PFSPRITE_AXIAL_ROT		
#define PFBB_POINT_ROT_EYE	PFSPRITE_POINT_ROT_EYE
#define PFBB_POINT_ROT_WORLD	PFSPRITE_POINT_ROT_WORLD		
} // extern "C" END of C include export


#define PFBILLBOARD ((pfBillboard*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFBILLBOARDBUFFER ((pfBillboard*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfBillboard : public pfGeode
{
public:

    void setAxis(const pfVec3& axis)  {
        PFBILLBOARD->nb_setAxis(axis);
    }

    void getAxis(pfVec3& axis)  {
        PFBILLBOARD->nb_getAxis(axis);
    }

    void setMode(int  _mode, int  _val)  {
        PFBILLBOARD->nb_setMode(_mode, _val);
    }

    int getMode(int  _mode)  {
        return PFBILLBOARD->nb_getMode(_mode);
    }

    void setPos(int  i, const pfVec3& pos)  {
        PFBILLBOARD->nb_setPos(i, pos);
    }

    void getPos(int  i, pfVec3& pos)  {
        PFBILLBOARD->nb_getPos(i, pos);
    }

    void setPosFlux(pfFlux *_flux)  {
        PFBILLBOARD->nb_setPosFlux(_flux);
    }

    pfFlux* getPosFlux()  {
        return PFBILLBOARD->nb_getPosFlux();
    }
public:		// Constructors/Destructors
    //CAPI:basename Bboard
    //CAPI:updatable
    //CAPI:newargs
    pfBillboard();
    virtual ~pfBillboard();

protected:
    pfBillboard(pfBuffer *buf);
    pfBillboard(const pfBillboard *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    	pf_applyUpdate(const pfUpdatable *prev, int  upId);
    virtual pfUpdatable*	pf_bufferClone(pfBuffer *buf);

PFINTERNAL:		// pfNode virtual traversals
    virtual void	nb_clean(uint64_t cleanBits, pfNodeCounts *counts);
    virtual pfNode*	nb_clone();
    virtual int 	nb_cull(int  mode, int  cullResult, _pfCuller *trav);
    virtual int 	nb_cullProgram(int  mode, int  cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int  	nb_flatten(pfTraverser *trav);
    virtual int  	nb_intersect(_pfIsector *isector);

    virtual void pf_draw(int  i, _pfCuller *cull, pfVec3 *eye);
    virtual void pf_draw(pfGeoState *gstate, pfDispList *dl, pfGeomStats *gstats, int  i, _pfCuller *cull, pfVec3 *eye);

PFINTERNAL:		// class specific sets and gets
    void 	nb_setAxis(const pfVec3& axis);
    void 	nb_getAxis(pfVec3& axis);

    void	nb_setMode(int  _mode, int  _val);
    int 	nb_getMode(int  _mode);

    void 	nb_setPos(int  i, const pfVec3& pos);
    void 	nb_getPos(int  i, pfVec3& pos);

    void 	nb_setPosFlux(pfFlux *_flux);
    pfFlux* 	nb_getPosFlux(void);


protected:
    friend class _pfCuller;

    pfSprite 	*bbSprite;
    pfVec3	*positions;
    int 	posSize;
    pfFlux	*posFlux;

private:
    static pfType *classType;
};

#endif // !__PF_BILLBOARD_H___
