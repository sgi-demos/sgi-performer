//
// Copyright 1999, 2000, Silicon Graphics, Inc.
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
// pfShadow.h
//

#ifndef __PF_SHADOW_H__
#define __PF_SHADOW_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pr.h>
#include <Performer/prmath.h>
#include <Performer/pr/pfType.h>
#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfLight.h>

#include <Performer/pf.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfSCS.h>
#include <Performer/pf/pfDoubleSCS.h>
#include <Performer/pf/pfIBR.h>

/* Export to C API. */
extern "C" {
/* pfShadow flags */
#define PFSHD_BLEND_TEXTURES        (1<<0)

/* pfShadow parameters */
#define PFSHD_PARAM_TEXTURE_SIZE     1
#define PFSHD_PARAM_NUM_TEXTURES     2
}


typedef _pfTemplateList<float> _pfFloatListT; // better than the default float list

typedef float (*_pfBlendFunc)(float);


struct __pfViewType;
struct __pfTextureInfo;

#define PFSHADOW ((pfShadow*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFSHADOWBUFFER ((pfShadow*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfShadow : public pfObject {

    //class type
    static pfType  *classType;

 private:
    int                flags;
    void              *casterNodes;
    void              *lightSources;
    int                numChannels;
    pfChannel        **channels;
    pfFlux           **views;     // views per channel

    pfMaterial        *mtl;
    pfLightModel      *lmodel;
    pfTexEnv          *tenv;
    pfTexGen          *tgen;
    _pfFloatListT     *ambientFactor; // ambient factor per light

    pfFrustum         *shadowFrust;
    
    float             (*blendFunc)(float);

public:

    //CAPI:newargs
    pfShadow();
    virtual ~pfShadow();
    
    //// type checking functions
    static void init();
    static pfType* getClassType() {return classType;}

    // shadow casters
    void setNumCasters(int index);
    int  getNumCasters(void);
    void setShadowCaster(int index, pfNode *caster, pfMatrix &mat);
    void adjustCasterCenter(int index, pfVec3 &trans);
    pfNode *getShadowCaster(int index);
    pfMatrix *getShadowCasterMatrix(int index);

    // light sources
    void setNumSources(int num);
    int  getNumSources(void);
    void setSourcePos(int index, float x, float y, float z, float w);
    void getSourcePos(int index, float *x, float *y, float *z, float *w);
    void setLight(int index, pfLight *light);
    pfLight *getLight(int index);
    void setAmbientFactor(int light, float factor);
    float getAmbientFactor(int light);

    void setShadowTexture(int caster, int light, pfTexture *tex);
    pfTexture *getShadowTexture(int caster, int light);
    void setTextureBlendFunc(_pfBlendFunc blendFunc);

    /////
    void addChannel(pfChannel *channel);
    void updateView(void);
    void updateCaster(int index, pfMatrix &mat);

    void apply(void);
    void draw(pfChannel *chan);

    void setFlags(int which, int value);
    int  getFlags(int which) const {
	return flags & which;
    }
    void  setVal(int caster, int light, int which, float  val);
    float getVal(int caster, int light, int which);
    pfDirData *getDirData(int caster, int light);


 private:
    __pfViewType *getView(pfChannel *channel, int &index);
    void initializeViews(pfChannel *chan);

public:
    void *_pfShadowExtraData;
};

#endif //__PF_SHADOW_H__
