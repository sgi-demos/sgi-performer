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
// $Revision: 1.38 $
// $Date: 2002/03/14 21:11:10 $
// 
//

#ifndef __PF_STRING_H__ 
#define __PF_STRING_H__ 

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>

//-----------------------------------------------------------
///////////////////////////// pfString //////////////////////////////////////

extern "C" {
/* ----------------------- pfString Tokens ----------------------- */
/* pfStringMode() */
#define PFSTR_JUSTIFY		10
#define PFSTR_FIRST		10
#define PFSTR_MIDDLE		11
#define PFSTR_LAST		12
#define PFSTR_LEFT		PFSTR_FIRST
#define PFSTR_CENTER		PFSTR_MIDDLE
#define PFSTR_RIGHT		PFSTR_LAST

/* pfStringMode() */
#define PFSTR_CHAR_SIZE		30
#define PFSTR_CHAR		1
#define PFSTR_SHORT		2
#define PFSTR_INT		4

/* pfStringMode() */
#define PFSTR_AUTO_SPACING	40
}

#define PFSTRING ((pfString*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFSTRINGBUFFER ((pfString*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfString : public pfObject
{

public:
    // constructors and destructors
    pfString();
    virtual ~pfString();

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
    size_t		getStringLength() const;
    void		setMode(int _mode, int _val);
    int	 		getMode(int _mode) const;
    void		setFont(pfFont* _fnt);
    pfFont*		getFont() const { return fnt; }
    void		setString(const char* _cstr);
    const char*		getString() const { return string; }
    const pfGeoSet*	getCharGSet(int _index) const;
    const pfVec3*     	getCharPos(int _index) const;
    void		setSpacingScale(float sx, float sy, float sz);
    void		getSpacingScale(float *sx, float *sy, float *sz) const {
	PFGET_VEC3(charSScale, sx, sy, sz);
    }
    void		setGState(pfGeoState *gs) {
	pfMemory::unref(gstate);
	gstate = gs;
	pfMemory::ref(gstate);
    }
    
    pfGeoState*	getGState(void) const { return gstate; }
    void		setColor(float r,float g, float b, float a) {
	PFSET_VEC4(clr, r,g,b,a);
    }
    void		getColor(float *r, float *g, float *b, float *a) const {
	PFGET_VEC4(clr, r, g, b, a);
    }
    void		setBBox(const pfBox* newbox) { bbox = *newbox; }
    const pfBox*	getBBox(void) const { return &bbox; }
    void		setMat(const pfMatrix &_mat);
    void		getMat(pfMatrix &_mat) const { _mat = mat; }
			
    void		setIsectMask(uint _mask, int _setMode, int _bitOp);
    uint 		getIsectMask() const { return isectMask; }

public:
    // other functions
    // CAPI:verb
    void	draw();
    void	flatten();
    // CAPI:verb StringIsectSegs
    int		isect(pfSegSet *segSet, pfHit **hits[]);


PFINTERNAL: // XXX - should make protected???
    uint	flags;	
    char	*string;	// Character string
    int		charSize;	// Size of string characters in bytes
    int		just;		// justification
    pfVec3	scales;		// x,y,z scale factors
    pfMatrix	mat;		// transformation matrix
    pfFont	*fnt;		// pfFont supplies character pfGeoSets
    pfList	*gsets;		// List of pfGeoSets defining string
    pfVec3	*posns;		// List of 3D origins of pfGeoSet letters
    pfGeoState	*gstate;	// GeoState specifies appearance
    pfVec4	clr;		// Global color
    pfVec3	charSScale;	// Spacing scale
    pfVec3	charNSScale;	// Normalized spacing scale
    int		maxSpacingDir;	// Direction of maximum spacing
    pfBox	bbox;		// Bound of string
    uint32_t	lock;		// Spin lock for MP operation
    uint	isectMask;	// Intersection mask

private:
    static pfType *classType;
};




#endif // !__PF_STRING_H__ 
