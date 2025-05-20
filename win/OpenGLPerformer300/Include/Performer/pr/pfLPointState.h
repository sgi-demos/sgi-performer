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
// pfLPointState.h
//
// $Revision: 1.30 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_LPOINTSTATE_H__
#define __PF_LPOINTSTATE_H__

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
/* ------------------ pfLPointState Tokens --------------------- */

/* pfLPStateMode() */

#define PFLPS_SIZE_MODE         10
#define PFLPS_SIZE_MODE_ON      1
#define PFLPS_SIZE_MODE_OFF     0

#define PFLPS_TRANSP_MODE       20
#define PFLPS_TRANSP_MODE_ON    1
#define PFLPS_TRANSP_MODE_OFF   0
#define PFLPS_TRANSP_MODE_ALPHA 1
#define PFLPS_TRANSP_MODE_TEX   2       

#define PFLPS_FOG_MODE          30
#define PFLPS_FOG_MODE_ON       1
#define PFLPS_FOG_MODE_OFF      0
#define PFLPS_FOG_MODE_ALPHA    1
#define PFLPS_FOG_MODE_TEX      2

#define PFLPS_RANGE_MODE        40
#define PFLPS_RANGE_MODE_DEPTH  0
#define PFLPS_RANGE_MODE_TRUE   1

#define PFLPS_DIR_MODE          50
#define PFLPS_DIR_MODE_ON       1
#define PFLPS_DIR_MODE_OFF      0
#define PFLPS_DIR_MODE_ALPHA    1
#define PFLPS_DIR_MODE_TEX      2

#define PFLPS_SHAPE_MODE        60
#define PFLPS_SHAPE_MODE_UNI    0
#define PFLPS_SHAPE_MODE_BI     1
#define PFLPS_SHAPE_MODE_BI_COLOR       2

#define PFLPS_DRAW_MODE		70
#define PFLPS_DRAW_MODE_RASTER	0
#define PFLPS_DRAW_MODE_CALLIGRAPHIC	1

#define PFLPS_QUALITY_MODE	80
#define PFLPS_QUALITY_MODE_HIGH 0
#define PFLPS_QUALITY_MODE_MEDIUM 1
#define PFLPS_QUALITY_MODE_LOW 2

#define PFLPS_CALLBACK_MODE	 90
#define PFLPS_CALLBACK_MODE_OFF  0
#define PFLPS_CALLBACK_MODE_PRE  1
#define PFLPS_CALLBACK_MODE_POST 2

#define PFLPS_DEBUNCHING_MODE	  91
#define PFLPS_DEBUNCHING_MODE_ON  1
#define PFLPS_DEBUNCHING_MODE_OFF 0


/* pfLPStateVal() */

#define PFLPS_SIZE_MIN_PIXEL    100
#define PFLPS_SIZE_ACTUAL       101
#define PFLPS_SIZE_MAX_PIXEL    102

#define PFLPS_TRANSP_PIXEL_SIZE 200     
#define PFLPS_TRANSP_EXPONENT   201
#define PFLPS_TRANSP_SCALE      202
#define PFLPS_TRANSP_CLAMP      203

#define PFLPS_FOG_SCALE         300
#define PFLPS_INTENSITY         400
#define PFLPS_SIZE_DIFF_THRESH  500

#define PFLPS_SIGNIFICANCE	600

#define PFLPS_MIN_DEFOCUS	700
#define PFLPS_MAX_DEFOCUS	701

extern pfLPointState* pfGetCurLPState(void);
} // extern C

#define PFLPOINTSTATE ((pfLPointState*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFLPOINTSTATEBUFFER ((pfLPointState*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfLPointState : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename LPState
    pfLPointState();
    virtual ~pfLPointState();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);  
    virtual int getGLHandle() const { return -1; }

public:
    // sets and gets
    // CAPI:basename 
    // CAPI:basename LPState
    void        setMode(int _mode, int _val);
    int         getMode(int _mode) const;
    void        setVal(int _attr, float _val);
    float       getVal(int _attr) const;
    void        setShape(float _horiz, float _vert, float _roll, float _falloff, float _ambient);
    void        getShape(float *_horiz, float *_vert, float *_roll, float *_falloff, float *_ambient) const;
    void        setBackColor(float r, float g, float b, float a);
    void        getBackColor(float *r, float *g, float *b, float *a); 
    // CAPI:basename
    void        setRasterFunc(pfRasterFuncType _rasterCallback, void *_data) {rasterCallback = _rasterCallback; rasterData = _data;}
    void        getRasterFunc(pfRasterFuncType *_rasterCallback, void **_data) {*_rasterCallback = rasterCallback; *_data = rasterData;}
    void        setCalligFunc(pfCalligFuncType _calligraphicCallback, void *_data) {calligraphicCallback = _calligraphicCallback; calligraphicData = _data;}
    void        getCalligFunc(pfCalligFuncType *_calligraphicCallback, void **_data) {*_calligraphicCallback = calligraphicCallback; *_data = calligraphicData;}

public:
    // other functions
    // CAPI:verb ApplyLPState
    void     apply();
    // CAPI:verb MakeLPStateRangeTex
    void     makeRangeTex(pfTexture *_tex, int _size, pfFog* _fog);
    // CAPI:verb MakeLPStateShapeTex
    void     makeShapeTex(pfTexture *_tex, int _size);


PFINTERNAL:  // XXX - should make protected???

    uint        flags;

    int         rangeMode;
    int         sizeRangeMode;
    int         transpRangeMode;
    int         fogRangeMode;
    int         dirMode, shapeMode;

    float       minTranspRange, invTranspExtent;
    float       minPixSize, maxPixSize, actualSize, sizeDiffThresh;
    float       transpPixSize, transpScale, transpExp, transpClamp;
    float       fogScale, invFogScale;

    pfVec4      backColor;

    float       roll, intensity;
    pfMatrix    rollMat;
    float       horizEnv, vertEnv, horizParam, vertParam;
    float	cosTeta;
    float       falloff, ambient, ambientScale;
    pfRasterFuncType rasterCallback;
    void        *rasterData;
    pfCalligFuncType calligraphicCallback;
    void	*calligraphicData;
    int		callbackMode;
    int		debunching;

    // Special Calligraphic
    uint	drawMode;	// calligraphic or raster
    uint	qualityMode;	// high, medium, low (fast)
    float	significance;	// for overload management
    float	minDefocus, maxDefocus;	// clamp global defocus value

private:
    static pfType *classType;
};

#endif // !__PF_LPOINTSTATE_H__
