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
// pfFog.h
//
// $Revision: 1.47 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_FOG_H__
#define __PF_FOG_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>


#define PFFOG_TABLESIZE		4096
#define PFFOG_MAXPOINTS		12


// !!! This returns 1-fog which == alpha for light points 
#define PFFOG_GET_VAL(fog, r)    \
    (((r) >= (fog)->opaqueTotal) ? 0.0f : (((r) <= (fog)->onsetTotal) ? 1.0f : \
    (fog)->table[(int)((PFFOG_TABLESIZE-1) * ((r) - (fog)->onsetTotal) * (fog)->invExtent)]))

extern "C" {     // EXPORT to CAPI

/* ------------------ pfFog Related Functions--------------------- */

extern  pfFog*   pfGetCurFog(void);

} // extern "C" END of C include export

#define PFFOG ((pfFog*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFFOGBUFFER ((pfFog*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfFog : public pfObject
{
public:
    // constructors and destructors
    pfFog();
    virtual ~pfFog();

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

/*----------------------------- pfFog --------------------------------*/

public:
    // sets and gets
    // CAPI:basename
    void     setFogType(int _type);
    int      getFogType() const { return userFogType; }

    // CAPI:basename Fog
    void     setRange(float _onset, float _opaque);
    void     getRange(float* _onset, float* _opaque) const {
	if (_onset != NULL) *_onset = onset;
	if (_opaque != NULL) *_opaque = opaque;
    }

    void     setOffsets(float _onset, float _opaque);
    void     getOffsets(float *_onset, float *_opaque) const {
	if (_onset != NULL) *_onset = onsetOffset;
	if (_opaque != NULL) *_opaque = opaqueOffset;
    }

    void     setRamp(int _points, float* _range, float* _density, float _bias);
    void     getRamp(int* _points, float* _range, float* _density, float* _bias) const;

    void     setColor(float _r, float _g, float _b);
    void     getColor(float* _r, float* _g, float* _b) const {
	PFGET_VEC3(color, _r, _g, _b);
    }

    float    getDensity(float _range) const {
	return 1.0f - PFFOG_GET_VAL(this, _range);
    }
  
public:
    // other functions
    // CAPI:verb
    void     apply();


protected:
    int		index;
    int 	dirty;
    int		fogType;
    int		userFogType;
    pfVec3      color;
    float       onset, onsetTotal;
    float       onsetOffset;
    float       opaque, opaqueTotal;
    float       opaqueOffset;
    float	invExtent;
    float	bias;
    int        	points;
    float       range[PFFOG_MAXPOINTS]; 	// normalized to [0..1] 
    float       density[PFFOG_MAXPOINTS];	// normalized to [0..1] 
    float       ramp[PFFOG_MAXPOINTS * 2];
    float	*table;
  
private:
    static pfType *classType;
};

#endif // !__PF_FOG_H__
