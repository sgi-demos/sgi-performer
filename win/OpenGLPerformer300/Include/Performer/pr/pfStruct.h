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
// pfStruct.h
//
// $Revision: 1.9 $
// $Date: 2002/07/30 21:59:23 $
//
//

#ifndef __PF_STRUCT_H__
#define __PF_STRUCT_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <sys/types.h>

#include <Performer/pfDLL.h>


// The pfStruct provides new and delete operators for use by
// structs and classes not derived from pfMemory/pfObject.
// This allows the easy creation of structs or other lightweight
// classes in shared memory without the overhead of pfMemory.

// CAUTION: since the allocation of storage for arrays of objects,
// e.g. "pfStruct *objs = new pfStruct[10]"; uses the global new 
// operator, these will not be allocated from shared memory unless 
// explicitly allocated and initialized, e.g. by using pfStruct::malloc()
// and explicitly invoking a constructor-like function for each 
// array element.

class DLLEXPORT pfStruct
{
public:
    //CAPI:private
    // default new operator allocates from shared arena, if any
    void* operator new(size_t s);
    void* operator new(size_t s, void *arena);
    void  operator delete(void *);
};

// When pfStruct is used as a base class, it adds one word of
// storage to the derived class size.  Often this extra memory 
// usage is undesirable, so the PFSTRUCT_DECLARE macro may be 
// used instead for similar functionality.

#define PFSTRUCT_DECLARE 			      \
public:						      \
    void* operator new(size_t s) {		      \
	return pfStruct::operator new(s);	      \
    }      					      \
    void* operator new(size_t s, void *arena) {	      \
	return pfStruct::operator new(s, arena);      \
    }						      \
    void  operator delete(void *d) {		      \
	delete (pfStruct *)d;			      \
    }						      \

#endif //  __PF_STRUCT_H__
