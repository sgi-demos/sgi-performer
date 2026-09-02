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
// pfGeoState.h
//
// $Revision: 1.91 $
// $Date: 2002/09/11 01:33:31 $
//

#ifndef __PF_GEOSTATE_H__
#define __PF_GEOSTATE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>


class DLLEXPORT pfGeoState;
extern pfGeoState	*pfCurGState;
extern pfGeoState	*pfSaveGState;
extern uint64_t		 pfCurModVec;
extern uint64_t		 pfInGStateFlag;

extern "C" {

/* ----------------pfGeoState Related Functions ------------------ */

extern  pfGeoState*   pfGetCurGState(void);
extern  pfGeoState*   pfGetCurIndexedGState(int _index);
extern  pfList*	      pfGetCurGStateTable(void);
}

//
// Structure containing all GL and other state Performer cares about.
// The size of this structure must be integral multiple of 4-bytes.
//
#define PFGEOSTATE ((pfGeoState*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFGEOSTATEBUFFER ((pfGeoState*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfGeoState : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename GState
    pfGeoState();
    virtual ~pfGeoState();

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
    void	setMode(uint64_t _attr, int _a);
    int 	getMode(uint64_t _attr) const;
    int		getCurMode(uint64_t _attr) const;
    int		getCombinedMode(uint64_t _attr, const pfGeoState *_combState) const;

    void	setMultiMode(uint64_t _attr, int _index, int _a);
    int 	getMultiMode(uint64_t _attr, int _index) const;
    int		getCurMultiMode(uint64_t _attr, int _index) const;
    int		getCombinedMultiMode(uint64_t _attr, int _index, const pfGeoState *_combState) const;

    void	setVal(uint64_t _attr, float _a);
    float	getVal(uint64_t _attr) const;
    float	getCurVal(uint64_t _attr) const;
    float	getCombinedVal(uint64_t _attr, const pfGeoState *_combState) const;

    void        setMultiVal(uint64_t _attr, float *_values);
    int         getMultiVal(uint64_t _attr, float *_values);
    int         getCurMultiVal(uint64_t _attr, float *_values);
    int         getCombinedMultiVal(uint64_t _attr, float *_values, 
				    const pfGeoState *_combGState);

    void	setInherit(uint64_t _mask);
    uint64_t    getInherit() const { return ~modvec; }

    void	setAttr(uint64_t _attr, void* _a);
    void*	getAttr(uint64_t _attr) const;
    void*	getCurAttr(uint64_t _attr) const;
    void*	getCombinedAttr(uint64_t _attr, const pfGeoState *_combState) const;

    void	setMultiAttr(uint64_t _attr, int _index, void* _a);
    void*	getMultiAttr(uint64_t _attr, int _index) const;
    void*	getCurMultiAttr(uint64_t _attr, int _index) const;
    void*	getCombinedMultiAttr(uint64_t _attr, int _index, const pfGeoState *_combState) const;

    void	setFuncs(pfGStateFuncType _preFunc, pfGStateFuncType _postFunc, void *_data);    
    void	getFuncs(pfGStateFuncType *_preFunc, pfGStateFuncType *_postFunc, void **_data) const {
	*_preFunc = preFunc; *_postFunc = postFunc; *_data = data;
    }

    int		getNumTextures() { return (num_textures); }
    
public:
    // other functions
    // CAPI:verb
    void	load();
    void	apply();
    void	makeBasic();
    //CAPI:verb ApplyGStateTable
    static void	applyTable(pfList *_gstab);


PFINTERNAL: // XXX - should make protected???
    
    uint64_t	    	modvec;	    // modify vector (current only)
    uint64_t	    	override;   // override vector (current only)

    pfLight	 	**lights;   // light sources, demand malloced 
    pfLightModel 	*lm;	    // light model 
    pfMaterial		*frontmtl;  // front material 
    pfMaterial		*backmtl;   // back material 

    int			num_textures; // Number of valid textures in array.
    pfTexture		*texture[PF_MAX_TEXTURES];   
				    // main texture map 
    pfTexture		*detailtex[PF_MAX_TEXTURES]; 
				    // detail texture map (current only)
    pfTexEnv		*tev[PF_MAX_TEXTURES];	    
				    // texture environment 
    pfTexGen		*tgen[PF_MAX_TEXTURES];	    
				    // texgen structure 
    pfTexLOD		*tlod[PF_MAX_TEXTURES];	    
				    // tex lod structure 
    pfTexLOD		*tlodtex[PF_MAX_TEXTURES];   
				    // tex lod structure from a pfTexture
    pfMatrix		*tmat[PF_MAX_TEXTURES];	    
				    // texture matrix
    pfLPointState	*lpstate;   // lightpoint state 
    pfColortable	*ctab;	    // color table 
    pfFog		*fog;	    // fog model 
    pfHighlight		*hlight;    // highlight structure 
    pfList		*gstab;	    // indexed gstate table (current only)
    pfPlane		*decalplane; // reference plane for decal geometry
    pfCombiner		*combiner;   // register combiner

    float               aref;	    // afunction reference value 
    
    // CAREFUL: These shorts have to be big enough to hold their 
    // corresponding GL enums.
    //
    // ------------------------ 
    ushort	    flags;	    
    ushort	    cullface;	    // front/back face cull 
    // ------------------------ 
    ushort          transp;	    // transparency function 
    ushort          decal;	    // decal function 
    // ------------------------ 
    ushort          antialias;	    // aliasing mode 
    ushort	    shade;	    // shademodel 
    // ------------------------ 
    ushort          afunc;	    // afunction function 
    // The enables can just be chars -- they are really bools 
    char	    enlight;	    // enable lighting 
    char	    enlpstate;	    // lightpoint state enable 
    // ------------------------ 
    char	    enfog;	    // fog enable 
    float           fogScale;       // fog scale value
    float           fogOffset[4];   // fog reference point and offset
    char	    enwf;	    // enable wireframe mode for geosets 
    char	    enhlight;	    // highlighting 
    char	    enci;	    // enable color index 
    char	    encombiner;	    // enable register combiner
    // ------------------------ 
    char	    entex[PF_MAX_TEXTURES];	    // texture enable 
    char	    entgen[PF_MAX_TEXTURES];	    // texgen enable 
    char	    entlod[PF_MAX_TEXTURES];	    // tex lod enable
    char	    entmat[PF_MAX_TEXTURES];	    // tex matrix enable 
    // ------------------------ 
    ushort	    enmask;	    // mask of all enables 
  
    // unwindGState(); preFunc(); pfDrawGSet(); postFunc(); 
    pfGStateFuncType preFunc;
    pfGStateFuncType postFunc;
    void	    *data;

private:
    static pfType 	*classType;
};
     
#endif // !__PF_GEOSTATE_H__
