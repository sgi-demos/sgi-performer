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
// pfDataPool.h          datapool memory manager 
//
// $Revision: 1.19 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_DATAPOOL_H__
#define __PF_DATAPOOL_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>

typedef struct _pfDPoolStruct pfDPoolStruct;

#define PFDATAPOOL ((pfDataPool*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFDATAPOOLBUFFER ((pfDataPool*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfDataPool : public pfObject
{
private:
    // constructors and destructors
    // CAPI:basename DPool
    pfDataPool();

public:
    virtual ~pfDataPool();
    // CAPI:verb
    static pfDataPool*	create(uint _size, char* _name); // does a new
    static pfDataPool*	attach(char* _name); // does a new

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

public:
    // sets and gets
    const char*	   getName();

    static void    setAttachAddr(void *_addr);
    static void*   getAttachAddr();
    // CAPI:verb GetDPoolSize
    int getDPoolSize();

public:
    // other functions
    volatile void* alloc(unsigned int _size, int _id);
    volatile void* find(int _id);
    int		   free(void* _dpmem);
    // CAPI:verb
    int		   release();

    // CAPI:noverb
    static int     lock(void* _dpmem) { 
	return lock(_dpmem, -1, TRUE);
    }
    // CAPI:verb DPoolSpinLock
    static int     lock(void* _dpmem, int spins, int block); 
    // CAPI:noverb
    static void	   unlock(void* _dpmem);
    static int 	   test(void* _dpmem);

private:
    pfDPoolStruct *dps; // pointer to structure allocated from datapool

private:
    static pfType *classType;
};


#endif // !__PF_DATAPOOL_H__
