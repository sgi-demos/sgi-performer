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
// pfLOD.h		Level of Detail switch include file
//
// $Revision: 1.61 $
// $Date: 2002/05/30 00:33:56 $
//
#ifndef __PF_LOD_H__
#define __PF_LOD_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfLODState.h>
#include <Performer/pr/pfFlux.h>
#include <Performer/pf/pfTraverser.h>

class pfBuffer;

// Draw child:
//	 n if ranges[n] <= range < ranges[n+1], 
//	 no child if range < ranges[0] || range > ranges[numRanges-1]

#define PFLOD ((pfLOD*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFLODBUFFER ((pfLOD*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfLOD : public pfGroup
{
public:

    void setCenter(pfVec3& c)  {
        PFLOD->nb_setCenter(c);
    }

    void getCenter(pfVec3& c) const  {
        PFLOD->nb_getCenter(c);
    }

    void setRange(int index, float range)  {
        PFLOD->nb_setRange(index, range);
    }

    int getNumRanges() const  {
        return PFLOD->nb_getNumRanges();
    }

    float getRange(int index) const  {
        return PFLOD->nb_getRange(index);
    }

    void setTransition(int index, float delta)  {
        PFLOD->nb_setTransition(index, delta);
    }

    int getNumTransitions() const  {
        return PFLOD->nb_getNumTransitions();
    }

    float getTransition(int index) const  {
        return PFLOD->nb_getTransition(index);
    }

    void setLODState(pfLODState *ls)  {
        PFLOD->nb_setLODState(ls);
    }

    pfLODState* getLODState() const  {
        return PFLOD->nb_getLODState();
    }

    void setLODStateIndex(int index)  {
        PFLOD->nb_setLODStateIndex(index);
    }

    int getLODStateIndex() const  {
        return PFLOD->nb_getLODStateIndex();
    }

    void setRangeFlux(int index, pfFlux *flux)  {
        PFLOD->nb_setRangeFlux(index, flux);
    }

    int getNumRangeFluxes() const  {
        return PFLOD->nb_getNumRangeFluxes();
    }

    pfFlux* getRangeFlux(int index) const  {
        return PFLOD->nb_getRangeFlux(index);
    }

    void setUserEvalFunc(pfLODEvalFuncType evalFunc)  {
        PFLOD->nb_setUserEvalFunc(evalFunc);
    }

    pfLODEvalFuncType getUserEvalFunc()  {
        return PFLOD->nb_getUserEvalFunc();
    }

    float evaluate(const pfChannel *chan, const pfMatrix *offset)  {
        return PFLOD->nb_evaluate(chan, offset);
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfLOD();
    virtual ~pfLOD();

protected:
    pfLOD(pfBuffer *buf);
    pfLOD(const pfLOD *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:		// pfMemory virtual functions
    virtual int 	checkDelete(void);    

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);

PFINTERNAL:		// pfNode virtual traversals
    virtual int 	app(pfTraverser *trav);
    virtual int 	ASDtrav(pfTraverser *trav);
    virtual pfNode*    	nb_clone();
    virtual int 	nb_cull(int mode, int cullResult, _pfCuller *trav);
    virtual int 	nb_cullProgram(int mode, int cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int 	nb_flatten(pfTraverser *trav);
    virtual int 	nb_intersect(_pfIsector *isector);
    virtual int 	nb_traverse(pfTraverser *trav);

PFINTERNAL:		// Class specific sets and gets
    void		nb_setCenter(pfVec3& c);    
    void		nb_getCenter(pfVec3& c) const {
	c = center;
    }    

    void		nb_setRange(int index, float range);
    int 		nb_getNumRanges() const {
	return ranges.getNum();
    }
    float		nb_getRange(int index) const;

    void		nb_setTransition(int index, float delta);
    int 		nb_getNumTransitions() const {
	return transitions.getNum();
    }
    float		nb_getTransition(int index) const;
    
    void		nb_setLODState(pfLODState *ls);
    pfLODState*		nb_getLODState() const;

    void		nb_setLODStateIndex(int index);
    int			nb_getLODStateIndex() const {
	return lodStateIndex;
    }

    void		nb_setRangeFlux(int index, pfFlux *flux);
    int 		nb_getNumRangeFluxes() const {
	return fluxes.getNum();
    }
    pfFlux*		nb_getRangeFlux(int index) const;

    void		nb_setUserEvalFunc(pfLODEvalFuncType evalFunc);
    pfLODEvalFuncType	nb_getUserEvalFunc();

    //CAPI:verb
    float		nb_evaluate(const pfChannel *chan, const pfMatrix *offset);
    


protected:
    _pfFloatList	transitions;
    _pfFloatList 	ranges;
    pfVec3		center;
    float		defaultTransition;
    int			lodStateIndex;
    _pfLODStateRef	lodState;
    _pfVoidList		fluxes;
    pfLODEvalFuncType	userEvaluationFunc;

private:
    static pfType *classType;
};

#endif // !__PF_LOD_H__
