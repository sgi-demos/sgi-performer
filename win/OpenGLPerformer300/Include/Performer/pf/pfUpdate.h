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
// pfUpdate.h		UpdateList class include file
//
// $Revision: 1.134 $
// $Date: 2002/08/10 00:20:16 $
//
#ifndef __PF_UPDATE_H__
#define __PF_UPDATE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfLists.h>


// internal class for tracking MP updates
struct _pfUpdate
{
    PFSTRUCT_DECLARE


    int 			updatableId;	
    int 			updateId;
};	

#define _PFUPDATELIST ((_pfUpdateList*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define _PFUPDATELISTBUFFER ((_pfUpdateList*)buf->pf_indexUpdatable(this))

class DLLEXPORT _pfUpdateList : public _pfHashTable 
{

    _pfVoidList		freeList;
    int 		free;
    _pfUpdatablePList	newList;
    _pfIntList		deleteList;

};
#endif	// !_PF_UPDATE_H
