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
// pfSwitch.h		Switch include file
//
// $Revision: 1.48 $
// $Date: 2002/05/30 00:33:56 $
//
#ifndef __PF_SWITCH_H__
#define __PF_SWITCH_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfGroup.h>
#include <Performer/pr/pfFlux.h>

class pfBuffer;

extern "C" {
/* ----------------------- pfSwitch Tokens ----------------------- */

/* pfSwitchVal() */
#define PFSWITCH_ON		-1
#define PFSWITCH_OFF		-2
}

#define PFSWITCH ((pfSwitch*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFSWITCHBUFFER ((pfSwitch*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfSwitch : public pfGroup
{
public:

    int setVal(float val)  {
        return PFSWITCH->nb_setVal(val);
    }

    float getVal() const  {
        return PFSWITCH->nb_getVal();
    }

    int setValFlux(pfFlux *_valFlux)  {
        return PFSWITCH->nb_setValFlux(_valFlux);
    }

    pfFlux* getValFlux() const  {
        return PFSWITCH->nb_getValFlux();
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfSwitch();
    virtual ~pfSwitch();

protected:
    //CAPI:private
    pfSwitch(pfBuffer *buf);
    pfSwitch(const pfSwitch *prev, pfBuffer *buf);

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

PFINTERNAL:		// specific sets and gets
    int 		    nb_setVal(float val);
    float 		    nb_getVal() const {
	if (valFlux)
	    return *(float*)valFlux->getCurData();
	else
	    return floatVal;
    }
    int 		    nb_setValFlux(pfFlux *_valFlux);
    pfFlux* 		    nb_getValFlux() const {
	return valFlux;
    }


protected:
    int 		switchVal;
    float		floatVal;
    pfFlux		*valFlux;

private:
    static pfType *classType;
};

#endif // !__PF_SWITCH_H__
