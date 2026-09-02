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
// pfColortable.h - colortables
//
// $Revision: 1.32 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_COLORTABLE_H__
#define __PF_COLORTABLE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>

extern pfVec4 *pfCurCtab;

extern "C" {     // EXPORT to CAPI

/* ------------------ pfColortable Related Functions--------------------- */

extern  pfColortable*   pfGetCurCtab(void);
}

#define PFCOLORTABLE ((pfColortable*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFCOLORTABLEBUFFER ((pfColortable*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfColortable : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename Ctab
    //CAPI:private
    pfColortable();
    //CAPI:public
    pfColortable(int _size);
    virtual ~pfColortable();

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
    // CAPI:basename 
    int		getCtabSize() const { return size; }
    // CAPI:basename Ctab
    int		setColor(int _index, pfVec4& _acolor);
    // CAPI:err FALSE
    int		getColor(int _index, pfVec4& _acolor) const;
    pfVec4*	getColors() const { return table; }
public:
    //other functions
    // CAPI:verb
    void   apply();


private:
    pfVec4*    table;	/* the color table */
    int        size;	/* size of allocated table */

private:
    static pfType *classType;
};
#endif // !__PF_COLORTABLE_H__
