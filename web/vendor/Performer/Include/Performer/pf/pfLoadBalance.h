//
// Copyright 1999, 2000, Silicon Graphics, Inc.
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
// pfLoadBalance.h
//

#ifndef __PF_LOADBALANCE_H__
#define __PF_LOADBALANCE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pr.h>
#include <Performer/prmath.h>
#include <Performer/pr/pfType.h>
#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfTexture.h>

#include <Performer/pf.h>
#include <Performer/pf/pfChannel.h>

extern "C" {

/* constants */

#define		PFLOAD_COEFF	1

/* flags */

}

#define PFLOADBALANCE ((pfLoadBalance*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFLOADBALANCEBUFFER ((pfLoadBalance*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfLoadBalance : public pfObject {

    //class type
    static pfType  *classType;

public:

	//CAPI:newargs
    pfLoadBalance(int size);
    virtual ~pfLoadBalance();
    
    //// type checking functions
    static void init();
    static pfType* getClassType() {return classType;}

	//CAPI:virtual
	// setLoad() sets what the load is on a specified unit.  This is the load
	//     that'll be used during the load balancing calculations in balance().
    virtual void setLoad(int index, float load) {newLoad[index] = load; };

	// getLoad() returns the load that was set by setLoad() for a 
	//     specified unit.
    virtual float getLoad(int index) {return newLoad[index]; };

	// balance() takes the new load values specified by setLoad() and figures
	//     out the desired work load by each unit.  Then balance() takes the
	//     the load coefficient (PFLOAD_COEFF) into account while calculating
	//     what the work load should be for each unit.
    virtual void balance(void);
    
	// getWork() returns the percentage of work (a value from 0 to 1) to be
	//     done by the specified unit.
    virtual float getWork(int index) 
    {
        if (index < numActive)
	    return newWork[index]; 
	else
	    return 0.0;
    };

	// setVal() and getVal() currently accept PFLOAD_COEFF as an argument.
	//     The value of PFLOAD_COEFF determines how fast or how slow the
	//     balancer transitions from the current work load to the desired
	//     work load.
    virtual void  setVal(int what, float val);
    virtual float  getVal(int what);

	// setNumActive() and getNumActive() set and get the number of units
	//     to balance.
    virtual void  setNumActive(int num) { numActive = num; }
    virtual int   getNumActive(void) { return (numActive); }

protected:
    int		numWorkers;
    int		numActive;

    float	*oldLoad;
    float	*newLoad;

    float	*oldWork;
    float	*newWork;

    float	coeff;		// Filter convergence coefficient. Always in 
				// the range (0..1]. Closer to 1 means that 
				// the filter will use new results earlier.

    void        *spare;         // For keeping binary compatibility 
                                // in minor releases.

};

#endif //__PF_LOADBALANCE_H__
