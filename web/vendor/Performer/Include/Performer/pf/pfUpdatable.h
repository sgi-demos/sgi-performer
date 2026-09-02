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
// pfUpdatable.h
//
// $Revision: 1.57 $
// $Date: 2002/09/20 09:08:10 $
//
//
#ifndef __PF_UPDATABLE_H__
#define __PF_UPDATABLE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf.h>  // _pfCurrentBuffer
#include <Performer/pr/pfObject.h>
#include <Performer/pf/pfBuffer.h>


#define	PF_UPDATABLE_REF(pfUp, pfUpRef)					\
struct pfUpRef								\
{									\
    int	id;								\
}


#define PFUPDATABLE ((pfUpdatable*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFUPDATABLEBUFFER ((pfUpdatable*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfUpdatable : public pfObject
{
public:
    //CAPI:updatable
    // unqualified new operator checks for pfConfig and uses 
    // shared arena for CAPI and external use.  
    void* operator 	new(size_t s);
    void  operator 	delete(void *p) {pfObject::operator delete(p);}
    
public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }
    
protected:
    // Create non-node updatable in specified buffer
    pfUpdatable(pfBuffer *buf);

    // Create node updatable in specified buffer
    pfUpdatable(pfBuffer *buf, int);

    // Copy updatable in specified buffer
    pfUpdatable(const pfUpdatable *u, pfBuffer *buf);
    
    // Delete updatable 
    virtual ~pfUpdatable();
    
public: // pfObject virtual functions
    // set user data and add update to propagate down stream
    virtual void	 setUserData(void *data) { setUserData(0, data); }
    virtual void	 setUserData(int _slot, void *data);

public: // pfUpdatable virtual functions
    virtual void	 pf_copyToDraw(const pfUpdatable*) {}
    
    // Create a copy of this in buffer 'buf'
    virtual pfUpdatable* pf_bufferClone(pfBuffer *buf) = 0;
    
protected:
    // Apply update to object so that object is equal to 'prev'.
    virtual void    	pf_applyUpdate(const pfUpdatable *prev, int  updateId);
    
    // Apply special update at buffer merge time
    virtual void    	pf_applyMergeUpdate(_pfMergeUp &mu);
    
    // Updatables with fully-implemented BUFFER_CLONE pf_copy() do not need 
    // to keep track of updates until they are bufferCloned.
    virtual int	    	pf_needsUpdate(void);

    // Return TRUE if updatable has been buffer-cloned.
    virtual int	    	pf_isBufferCloned(void);

    // Add updatable to one of buffer's special internal lists
    virtual void	pf_updateBufferList(pfBuffer *, int) {}
    
public:
    int 		 pf_getpfId(void) const { 
	return pfId; 
    } 

protected:
    int 	    	pfId;		// Unique id assigned by Performer
    
private:
    static pfType *classType;
};

PF_UPDATABLE_REF(pfUpdatable, _pfUpdatableRef);

#endif // !__PF_UPDATABLE_H__
