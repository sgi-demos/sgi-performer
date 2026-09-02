//
//
// Copyright 2001, Silicon Graphics, Inc.
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
// pfCteChanData.h	Cliptexture Emulation: Channel Data header file
//
//

/******************************************************************************/

#ifndef __PF_CTE_CHANDATA_H__
#define __PF_CTE_CHANDATA_H__

/******************************************************************************/

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

/******************************************************************************/


#include <Performer/pr.h>
#include <Performer/pr/pfType.h>
#include <Performer/pr/pfClipTexture.h>
#include <Performer/pr/pfGeoSet.h>

#include <Performer/pf.h>
#include <Performer/pf/pfChannel.h>



/******************************************************************************/


/******************************************************************************/


#define PFCTECHANDATA ((pfCteChanData*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFCTECHANDATABUFFER ((pfCteChanData*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfCteChanData : public pfObject
{

    //class type
    static pfType* classType;


public:
    pfCteChanData( pfChannel* chan );
    virtual ~pfCteChanData();


    static void init();
    static pfType* getClassType() {return classType;}


};



/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

extern DLLEXPORT void cteDrawOverlayTextures( pfChannel* chan, pfClipTexture* cliptex );

#ifdef __cplusplus
}
#endif

/******************************************************************************/


#endif
/* __PF_CTE_CHANDATA_H__ */
