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
// pfLightSource.h		LightSource include file
//
// $Revision: 1.32 $
// $Date: 2002/05/30 00:33:56 $
//
#ifndef __PF_LIGHTSOURCE_H__
#define __PF_LIGHTSOURCE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfLists.h>


extern "C" {	/* export to CAPI */

/* pfLSourceMode() */    	
#define PFLS_PROJTEX_ENABLE		0
#define	PFLS_SHADOW_ENABLE		1
#define	PFLS_FOG_ENABLE			2
#define	PFLS_FREEZE_SHADOWS		3

/* pfLSourceVal() */    	
#define PFLS_SHADOW_DISPLACE_SCALE	10	
#define PFLS_SHADOW_DISPLACE_OFFSET	11
#define PFLS_SHADOW_SIZE		12	
#define PFLS_INTENSITY			13

/* pfLSourceAttr() */
#define PFLS_SHADOW_TEX			20
#define PFLS_PROJ_TEX			21
#define PFLS_PROJ_FOG			22
#define PFLS_PROJ_FRUST			23

}  /* end of export to CAPI */

class pfBuffer;

#define PFLIGHTSOURCE ((pfLightSource*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFLIGHTSOURCEBUFFER ((pfLightSource*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfLightSource : public pfNode
{
public:

    void setColor(int _which, float _r, float _g, float _b)  {
        PFLIGHTSOURCE->nb_setColor(_which, _r, _g, _b);
    }

    void getColor(int _which, float* _r, float* _g, float* _b)  {
        PFLIGHTSOURCE->nb_getColor(_which, _r, _g, _b);
    }

    void setAmbient(float _r, float _g, float _b)  {
        PFLIGHTSOURCE->nb_setAmbient(_r, _g, _b);
    }

    void getAmbient(float* _r, float* _g, float* _b)  {
        PFLIGHTSOURCE->nb_getAmbient(_r, _g, _b);
    }

    void setPos(float _x, float _y, float _z, float _w)  {
        PFLIGHTSOURCE->nb_setPos(_x, _y, _z, _w);
    }

    void getPos(float* _x, float* _y, float* _z, float* _w)  {
        PFLIGHTSOURCE->nb_getPos(_x, _y, _z, _w);
    }

    void setAtten(float _a0, float _a1, float _a2)  {
        PFLIGHTSOURCE->nb_setAtten(_a0, _a1, _a2);
    }

    void getAtten(float* _a0, float* _a1, float* _a2)  {
        PFLIGHTSOURCE->nb_getAtten(_a0, _a1, _a2);
    }

    void setSpotCutoffDelta(float _f1)  {
        PFLIGHTSOURCE->nb_setSpotCutoffDelta(_f1);
    }

    float getSpotCutoffDelta()  {
        return PFLIGHTSOURCE->nb_getSpotCutoffDelta();
    }

    void setSpotDir(float _x, float _y, float _z)  {
        PFLIGHTSOURCE->nb_setSpotDir(_x, _y, _z);
    }

    void getSpotDir(float* _x, float* _y, float* _z)  {
        PFLIGHTSOURCE->nb_getSpotDir(_x, _y, _z);
    }

    void setSpotCone(float _f1, float _f2)  {
        PFLIGHTSOURCE->nb_setSpotCone(_f1, _f2);
    }

    void getSpotCone(float* _f1, float* _f2)  {
        PFLIGHTSOURCE->nb_getSpotCone(_f1, _f2);
    }

    void on()  {
        PFLIGHTSOURCE->nb_on();
    }

    void off()  {
        PFLIGHTSOURCE->nb_off();
    }

    int isOn()  {
        return PFLIGHTSOURCE->nb_isOn();
    }

    void setMode(int mode, int val)  {
        PFLIGHTSOURCE->nb_setMode(mode, val);
    }

    int getMode(int mode) const  {
        return PFLIGHTSOURCE->nb_getMode(mode);
    }

    void setVal(int mode, float val)  {
        PFLIGHTSOURCE->nb_setVal(mode, val);
    }

    float getVal(int mode) const  {
        return PFLIGHTSOURCE->nb_getVal(mode);
    }

    void setAttr(int attr, void *obj)  {
        PFLIGHTSOURCE->nb_setAttr(attr, obj);
    }

    void* getAttr(int attr) const  {
        return PFLIGHTSOURCE->nb_getAttr(attr);
    }
public:		// Constructors/Destructors
    //CAPI:basename LSource
    //CAPI:updatable
    //CAPI:newargs
    pfLightSource();
    virtual ~pfLightSource();

protected:
    pfLightSource(pfBuffer *buf);
    pfLightSource(const pfLightSource *prev, pfBuffer *buf);
    
public:
    // per class functions;
    static void	   	    init();
    static pfType* 	    getClassType() { return classType; }
    
public:			// pfMemory virtual functions
    virtual int 	    checkDelete(void);


PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);
    virtual void	    pf_updateBufferList(pfBuffer *buf, int addRemove) {
        buf->pf_updateList(this, addRemove);
    }
    
PFINTERNAL:	// Traversals
    virtual int 	    nb_cull(int mode, int cullResult, _pfCuller *trav);
    virtual int 	    nb_cullProgram(int mode, int cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int 	    nb_flatten(pfTraverser *trav);
    virtual int 	    nb_intersect(_pfIsector *isector);
    virtual void	    nb_write(pfTraverser *trav, uint which, uint verbose);
    
PFINTERNAL:		// pfLight lookalike sets/gets
    //CAPI:noverb
    void	nb_setColor(int _which, float _r, float _g, float _b);
    void	nb_getColor(int _which, float* _r, float* _g, float* _b);
    // 1.2 compatibility 
    void	nb_setAmbient(float _r, float _g, float _b);
    void	nb_getAmbient(float* _r, float* _g, float* _b);

    void	nb_setPos(float _x, float _y, float _z, float _w);
    void	nb_getPos(float* _x, float* _y, float* _z, float* _w);

    // OpenGL only, IrisGL places atten in the light model
    void	nb_setAtten(float _a0, float _a1, float _a2);
    void	nb_getAtten(float* _a0, float* _a1, float* _a2);

    // OpenGL only with fragment lighting
    //CAPI:verb SpotLSourceCutoffDelta
    void	nb_setSpotCutoffDelta(float _f1);
    //CAPI:verb GetSpotLSourceCutoffDelta
    float	nb_getSpotCutoffDelta(void);

    //CAPI:verb SpotLSourceDir
    void	nb_setSpotDir(float _x, float _y, float _z);
    //CAPI:verb GetSpotLSourceDir
    void	nb_getSpotDir(float* _x, float* _y, float* _z);

    //CAPI:verb SpotLSourceCone
    void	nb_setSpotCone(float _f1, float _f2);
    //CAPI:verb GetSpotLSourceCone
    void	nb_getSpotCone(float* _f1, float* _f2);

PFINTERNAL:		// pfLight lookalike functions
    void		   nb_on();
    void		   nb_off();
    //CAPI:verb IsLSourceOn
    int			   nb_isOn() { return lightEnable; }

PFINTERNAL:		// pfLightSource sets and gets
    //CAPI:public
    void		   nb_setMode(int mode, int val);
    int			   nb_getMode(int mode) const;

    void		   nb_setVal(int mode, float val);
    float		   nb_getVal(int mode) const;

    void		   nb_setAttr(int attr, void *obj);
    void*		   nb_getAttr(int attr) const;


protected:
    friend class pfChannel;

    pfLight	*prLight;	// libpr light source structure
    
    char 	lightEnable;	// 1 = on, 0 = off for GL light source
    char 	projEnable;	// 1 = on, 0 = off for projected texture
    char	shadEnable;	// 1 = on, 0 = off for shadows
    char	fogEnable;	// 1 = on, 0 = off for range attenuation

    int		freezeShad;	// 1 -> do not recompute shadow map

    pfTexture	*shadTex;	// texture for shadow map
    int		shadSize;	// pixel size of square shadow map
    int		shadFilter;	// min and magfilter
    pfVec2	shadDisplace;	// displacepolygon scale and offset	

    float	intensity;	// scale factor for GL, projtex, shadows

    // For projected textures and shadows only
    pfTexture   *projTex;
    pfFrustum   *projFrust;
    pfFog       *projFog;

private:
    static pfType *classType;
};

#endif // !__PF_LIGHTSOURCE_H__
