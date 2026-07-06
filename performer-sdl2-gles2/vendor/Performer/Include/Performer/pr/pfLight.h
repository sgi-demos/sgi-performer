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
// pfLight.h
//
// $Revision: 1.50 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_LIGHT_H__
#define __PF_LIGHT_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>

extern "C" {     // EXPORT to CAPI

/* ------------------ pfLight Related Functions--------------------- */

extern  int   pfGetCurLights(pfLight *_lights[PF_MAX_LIGHTS]);
}

#define PFLIGHT ((pfLight*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFLIGHTBUFFER ((pfLight*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfLight : public pfObject
{
public:
    // constructors and destructors
    pfLight();
    virtual ~pfLight();

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
    void	setColor(int _which, float _r, float _g, float _b);
    void	getColor(int _which, float* _r, float* _g, float* _b) const;
    void	setAmbient(float _r, float _g, float _b);
    void	getAmbient(float* _r, float* _g, float* _b) const;
    void	setPos(float _x, float _y, float _z, float _w) {
	if (position[0] != _x || position[1] != _y || 
	    position[2] != _z || position[3] != _w)
	{
	    PFSET_VEC4(position, _x, _y, _z, _w); dirty++;
	}
    }
    void	getPos(float* _x, float* _y, float* _z, float* _w) const {
	PFGET_VEC4(position, _x, _y, _z, _w);
    }
    // OpenGL only, IrisGL places atten in the light
    void	setAtten(float _a0, float _a1, float _a2) {
	PFSET_VEC3(attn, _a0, _a1, _a2); dirty++;
    }
    void	getAtten(float* _a0, float* _a1, float* _a2) const {
	PFGET_VEC3(attn, _a0, _a1, _a2);
    }
    //CAPI:verb SpotLightDir
    void	setSpotDir(float _x, float _y, float _z) {
	PFSET_VEC3(spotdir, _x, _y, _z); dirty++;
    }
    //CAPI:verb GetSpotLightDir
    void	getSpotDir(float* _x, float* _y, float* _z) const {
	PFGET_VEC3(spotdir, _x, _y, _z);
    }
    //CAPI:verb SpotLightCone
    void	setSpotCone(float _f1, float _f2) {
	PFSET_VEC2(spotlight, _f1, _f2); dirty++;
    }
    //CAPI:verb GetSpotLightCone
    void	getSpotCone(float* _f1, float* _f2) const {
	PFGET_VEC2(spotlight, _f1, _f2);
    }
    //CAPI:verb SpotLightCutoffDelta
    void	setSpotCutoffDelta(float _f1) {
	cutoffdelta = _f1; dirty++;
    }
    //CAPI:verb GetSpotLightCutoffDelta
    float	getSpotCutoffDelta(void) const {
	return cutoffdelta;
    }
    
 public:
    // other functions
    void	on();
    void	off();
    // CAPI:verb IsLightOn
    int		isOn() { return (enable > 0); }
    // CAPI:noprbase
    // CAPI:private
    static void apply(pfLight* _lights[]);


private:
    int         index;     // gl index 
    int         dirty;	   // needs re-defing
    int         enable;	   // 1 = on, 0 = off 
    pfVec4      ambient;
    pfVec4      diffuse;
    pfVec4	specular;
    pfVec4      position;    
    pfVec3      spotdir;
    pfVec2      spotlight;
    pfVec3      attn;	   // OpenGL only
    float       cutoffdelta; // OpenGL fragment lighting soft cone edge angle

private:
    static pfType *classType;
};

//////////////////////////////// pfLightModel //////////////////////////////

extern "C" {     // EXPORT to CAPI
/* ------------------ pfLightModel Related Functions--------------------- */

extern  pfLightModel*   pfGetCurLModel(void);

} // extern "C" END of C include export

#define PFLIGHTMODEL ((pfLightModel*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFLIGHTMODELBUFFER ((pfLightModel*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfLightModel : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename LModel
    pfLightModel();
    virtual ~pfLightModel();

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
    void	setLocal(int _l) { local = _l ; dirty++; }
    int		getLocal() const { return local; }
    void	setTwoSide(int _t) { twoside = _t ; dirty++; }
    int		getTwoSide() const { return twoside; }
    void	setColorControl(int _c) { separate = _c ; dirty++; }
    int		getColorControl() const { return separate; }
    void	setAmbient(float _r, float _g, float _b) {
	PFSET_VEC3(ambient, _r, _g, _b); dirty++; 
    }
    void	getAmbient(float* _r, float* _g, float* _b) const {
	PFGET_VEC3(ambient, _r, _g, _b); 
    }
    // IrisGL only, OpenGL places atten in the light
    void	setAtten(float _a0, float _a1, float _a2) {
	PFSET_VEC3(attn, _a0, _a1, _a2); dirty++;
    }
    void	getAtten(float* _a0, float* _a1, float* _a2) const {
	PFGET_VEC3(attn, _a0, _a1, _a2);
    }
	
public:
    // other functions
    // CAPI:verb
    void	apply();


private:
    int        index;    // gl index 
    int        dlindex;  // dirty bit to mark whether dl has reserved a name 
    int        dirty;
    int	       twoside;
    int	       separate; // separate specular color
    int	       local;    
    pfVec4     ambient;
    pfVec3      attn;	// IRISGL only

private:
    static pfType *classType;

};

#endif // !__PF_LIGHT_H__



