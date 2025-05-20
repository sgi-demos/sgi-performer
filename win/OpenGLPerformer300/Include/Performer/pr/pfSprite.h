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
// pfSprite.h
//
// $Revision: 1.24 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_SPRITE_H__
#define __PF_SPRITE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>

extern pfVec3		_pfSpriteTrans;
extern pfVec3		*_pfSpriteCoords;
extern pfVec3		*_pfSpriteNorms;
extern int		 _pfSpriteCoordSize;
extern pfSprite		*_pfCurSprite;

struct _pfBackDoorSpriteSinCos
{
    pfVec3	pos;
    float	ssin, scos;
};

struct _pfBackDoorSpriteMat
{
    pfVec3	pos;
    pfMatrix	mat;
};

/* internal defines that match the OpenGL defines */
#define AXIAL_ROT		0
#define POINT_ROT_WORLD		1
#define POINT_ROT_EYE		2

extern "C" {     // EXPORT to INTERNAL CAPI
/* ----------------------- pfSprite Tokens ----------------------- */

/* pfSpriteMode() */
#define PFSPRITE_ROT			0

#define PFSPRITE_AXIAL_ROT		0
#define PFSPRITE_POINT_ROT_EYE		1
#define PFSPRITE_POINT_ROT_WORLD	2

#define PFSPRITE_MATRIX_THRESHOLD	20

/* ------------------ pfSprite Related Functions--------------------- */

extern  pfSprite*   pfGetCurSprite(void);

}

#define PFSPRITE ((pfSprite*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFSPRITEBUFFER ((pfSprite*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfSprite : public pfObject
{
public:
    // constructors and destructors
    pfSprite();
    virtual ~pfSprite();

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
    void     setMode(int _which, int _val);
    int	     getMode(int _which) const;
    void     setAxis(float _x, float _y, float _z);
    void     getAxis(float *_x, float *_y, float *_z) { 
	PFGET_VEC3(axis, _x, _y, _z);
    }

public:
    // other functions
    // CAPI:verb
    void     	    begin();
    static void     end();
    static void    position(float _x, float _y, float _z);

public:

private:
    int		vertLimit;
    unsigned char rotMode;
    unsigned char glRotMode;
    ushort	flags;
    pfMatrix	*axisMat;
    pfVec3	axis;

private:
    static pfType *classType;
};
#endif // !__PF_SPRITE_H__
