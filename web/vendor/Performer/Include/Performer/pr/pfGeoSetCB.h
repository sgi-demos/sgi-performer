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
// pfGeoSet.h
//
// $Revision: 1.3 $
// $Date: 2002/07/31 21:24:35 $
//
//

#ifndef __PF_GEOSETCB_H__
#define __PF_GEOSETCB_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfGeoSet.h>


// Exported Tokens
extern "C" {     // EXPORT to CAPI

typedef void (*pfGSetCBType)(pfGeoSet *, void *);

}

#define PFGEOSETCB ((pfGeoSetCB*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFGEOSETCBBUFFER ((pfGeoSetCB*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfGeoSetCB : public pfGeoSet
{
public:
    // constructors and destructors
    // CAPI:basename GSetCB
    pfGeoSetCB();
    virtual ~pfGeoSetCB();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:

    virtual void	pr_virtualDraw(void);

    void		setCBIndex(void);

    void		setDrawCB(pfGSetCBType cb, void *data) { 
			    draw_callback = cb; 
			    cb_data = data;
			    setCBIndex();
			    };

    pfGSetCBType 	getDrawCB(void) {return draw_callback; };
    void 	 	*getDrawCBData(void) {return cb_data; };

private:
    static pfType 	*classType;

    pfGSetCBType	draw_callback;
    void		*cb_data;
};

#endif // !__PF_GEOSETCB_H__
