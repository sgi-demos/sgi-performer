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
//  pfString.h - 3D Text
// 
// $Revision: 1.11 $
// $Date: 2002/06/05 21:23:19 $
// 
//

#ifndef __PF_FONT_H__ 
#define __PF_FONT_H__ 

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfList.h>

//-----------------------------------------------------------
extern "C" {
/* setMode Tokens */
#define	PFFONT_CHAR_SPACING	0
#define PFFONT_NUM_CHARS	1
#define PFFONT_RETURN_CHAR	2

/* setMode Values */
#define PFFONT_CHAR_SPACING_FIXED	0
#define PFFONT_CHAR_SPACING_VARIABLE	1
#define PFFONT_ASCII		128

/* setVal Tokens */
#define PFFONT_UNIT_SCALE	0

/* setAttr Tokens */
#define PFFONT_GSTATE		0
#define PFFONT_BBOX		1
#define PFFONT_SPACING		2
#define PFFONT_NAME		3
}

#define PFFONT ((pfFont*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFFONTBUFFER ((pfFont*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfFont : public pfObject
{
public:
    pfFont();
    virtual ~pfFont();

public:
    static void		init();
    static pfType*	getClassType() {return classType; }
    static pfGeoState*	defaultGState;

public:
    virtual int compare (const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

public:
    void	setCharGSet(int _ascii, pfGeoSet *_gset);
    pfGeoSet*	getCharGSet(int _ascii);
    void	setCharSpacing(int _ascii, pfVec3 &_spacing);
    const pfVec3*	getCharSpacing(int _ascii);
    void	setAttr(int _which, void *attr);
    void*	getAttr(int _which);
    void	setVal(int _which, float _val);
    float	getVal(int _which);
    void	setMode(int mode, int val);
    int		getMode(int mode);

public:
    friend class pfString;

protected:
    int		fontType;
    int		nChars;
    int		returnChar;
    char*	name;
    pfBox	bbox;
    float	unitScale;
    pfGeoState* gstate;
    pfList	*charGSets;
    pfVec3	*charSpacings;
    pfVec3	spacing;
    int		fixedWidth;
private:
    static pfType *classType;
};


#endif // !__PF_FONT_H__
