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
// pfMaterial.h
//
// $Revision: 1.55 $
// $Date: 2002/03/20 03:20:47 $
//
//

#ifndef __PF_MATERIAL_H__
#define __PF_MATERIAL_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>

#include <windows.h>
#include <GL/gl.h>


extern "C" {     // EXPORT to CAPI

/* pfMaterial Sides */
#define PFMTL_FRONT	    	0x1
#define PFMTL_BACK	    	0x2
#define PFMTL_BOTH		0x3

/* ------------------ pfMaterial Related Functions--------------------- */

extern  pfMaterial*   pfGetCurMtl(int _side);
} // extern "C" END of C include export

#define PFMATERIAL ((pfMaterial*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFMATERIALBUFFER ((pfMaterial*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfMaterial : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename Mtl
    pfMaterial();
    virtual ~pfMaterial();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);
    virtual int getGLHandle() const { return index; }

public:
    // sets and gets
    void	setSide(int _side);
    int 	getSide() { return side; }
    
    void	setAlpha(float _alpha);
    float	getAlpha() { return alpha; }

    void	setShininess(float _shininess);
    float	getShininess() { return shininess; }

    void	setColor(int _acolor, float _r, float _g, float _b);
    void	getColor(int _acolor, float* _r, float* _g, float* _b);

    void	setColorMode(int _side, int _mode);
    int		getColorMode(int _side);
	
public:
    // other functions
    // CAPI:verb
    void	apply();


PFINTERNAL: // XXX - should make protected???
    void        fullCopy(pfMaterial *src);
    int		index;    	// gl index 
    int		dirty;    	// needs re-defing 
    int		side;		// front, back, or both sides of polys 
    int		user_side;	// the side the user set
    short	frontlmc, backlmc;
    float       alpha;	
    pfVec4      ambient;
    pfVec4      diffuse;
    pfVec4      emission;
    pfVec4      specular, useSpecular;  
    pfVec4	*frontlmclr, *backlmclr;
    float       shininess; 
    int		emissive; // needed by gsdraw.m
    uint	enables;

private:
    static pfType *classType;
};

#endif // !__PF_MATERIAL_H__
