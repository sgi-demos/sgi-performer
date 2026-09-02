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
// pfType.h
//
// $Revision: 1.26 $
// $Date: 2002/10/03 23:05:37 $
//
#ifndef __PF_TYPE_H__
#define __PF_TYPE_H__



#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <sys/types.h>

#include <Performer/pr.h>
#include <Performer/pfDLL.h>



class DLLEXPORT pfType
{
public:
    //CAPI:private
    void* operator 	new(size_t s);
    void operator 	delete(void*) {};

public:
    //CAPI:newargs
    pfType(pfType *parent, char *name);
    ~pfType();


public:
    //CAPI:noverb
    pfType *getParent() {
	return parent;
    }
    //CAPI:verb IsDerivedFrom
    int		isDerivedFrom(pfType *ancestor) {
	if (ancestor->typeBit)
	    return (ancestor->typeBit & ancestorMask);
	else 
	    return pr_isAncestor(ancestor);
    }
    //CAPI:verb MaxTypes
    static void setMaxTypes(int _n);

    //CAPI:private
    const char *getName() {
	return name;
    }

protected:
    int		  pr_isAncestor(pfType *ancestor);

private:
    uint	 magic;
    uint 	 typeBit;	// internal class fast bit rep
    uint	 ancestorMask;	// mask of internal ancestors
    int	         numAncestors;
    pfType**	 ancestorList;
    pfType*	 parent;
    char*	 name;

protected:
    friend class pfMemory;
    static pfType*	firstMemoryType;
    static pfType*	lastMemoryType;
};

#endif // !__PF_TYPE_H__
