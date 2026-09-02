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
// pfCuller.h		Culler class include file
//
// $Revision: 1.107 $
// $Date: 2002/10/23 21:20:47 $
//
#ifndef __PF_CULLER_H__
#define __PF_CULLER_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif



#include <Performer/pf/pfLists.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfStruct.h>
#include <Performer/pr/pfISL.h>


class pfChannel;


#define PFSORT_FUNCS_MAX	64
#define	PFSORT_STACK_MAX	(2*PFTRAV_STACK_MAX)

#define PFSORT_XFORM_DIRTY	0x1
#define PFSORT_FUNCS_DIRTY	0x2
#define PFSORT_MODES_DIRTY	0x4


class pfCullProgram;

#define _PFCULLER ((_pfCuller*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define _PFCULLERBUFFER ((_pfCuller*)buf->pf_indexUpdatable(this))

class _pfCuller : public pfTraverser
{
};

#endif	//  !__PF_CULLER_H__
