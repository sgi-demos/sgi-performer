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
// pfLODState.h
//
// $Revision: 1.24 $
// $Date: 2002/05/30 00:33:56 $
//
//
#ifndef __PF_LODSTATE_H__
#define __PF_LODSTATE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf/pfUpdatable.h>
#include <Performer/pr/pfLinMath.h>


extern "C" {     // EXPORT to CAPI

/* ----------------------- pfLODState Tokens ----------------------- */

    /* pfLOD */
#define PFLOD_ALL_TRANSITIONS   -1
    
    /* pfLODStateAttr() */
#define PFLODSTATE_RANGE_RANGEOFFSET 		0
#define PFLODSTATE_RANGE_RANGESCALE		1
#define PFLODSTATE_TRANSITION_RANGEOFFSET	2
#define PFLODSTATE_TRANSITION_RANGESCALE	3
#define PFLODSTATE_RANGE_STRESSOFFSET		4
#define PFLODSTATE_RANGE_STRESSSCALE		5
#define PFLODSTATE_TRANSITION_STRESSOFFSET	6
#define PFLODSTATE_TRANSITION_STRESSSCALE	7
#define PFLODSTATE_RANGE_FOVOFFSET		8
#define PFLODSTATE_RANGE_FOVSCALE		9
#define PFLODSTATE_NUM_PARAMS			10

} // extern "C" END of C include export


#define PFLODSTATE ((pfLODState*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFLODSTATEBUFFER ((pfLODState*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfLODState : public pfUpdatable
{
public:

    void setAttr(int attr, float val)  {
        PFLODSTATE->nb_setAttr(attr, val);
    }

    float getAttr(int attr)  {
        return PFLODSTATE->nb_getAttr(attr);
    }

    int setName(const char *name)  {
        return PFLODSTATE->nb_setName(name);
    }

    const char* getName() const  {
        return PFLODSTATE->nb_getName();
    }

    pfLODState* find(const char *findName)  {
        return nb_find(findName);
    }
public:	    // Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfLODState();
    virtual ~pfLODState();

protected:
    //CAPI:private
    pfLODState(pfBuffer *buf);
    pfLODState(const pfLODState *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }
    
public:			// pfMemory virtual functions
    virtual int	    	  checkDelete();


PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	   pf_applyUpdate(const pfUpdatable *prev,  int upID);
    virtual pfUpdatable*   pf_bufferClone(pfBuffer *buf);


PFINTERNAL:		// class specific sets and gets
    void		   nb_setAttr(int attr, float val);
    
    float		   nb_getAttr(int attr);
    
    int		    	   nb_setName(const char *name);
    
    const char*	    	   nb_getName() const {
	return pfName;
    }

PFINTERNAL:		// class specific functions
    //CAPI:verb
    static pfLODState*      nb_find(const char *findName);
    
friend class pfLOD;
friend class pfASD;
friend class pfChannel;

protected:	    // data
    char		    *pfName;
    float		    params[PFLODSTATE_NUM_PARAMS];

private:
    static pfType *classType;
};

PF_UPDATABLE_REF(pfLODState, _pfLODStateRef);

#endif  // !__PF_LODSTATE_H__
