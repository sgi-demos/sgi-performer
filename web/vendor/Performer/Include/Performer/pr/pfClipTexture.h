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
// pfClipTexture.h
//
// $Revision: 1.76 $
// $Date: 2002/03/14 21:11:10 $
//
//

#ifndef __PF_CLIPTEXTURE_H__
#define __PF_CLIPTEXTURE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfImageTile.h>
#include <Performer/pr/pfImageCache.h>



struct _pfLoc {
    public:
// CAPI: private
    void copy(_pfLoc src) {
	Shad.copy(src.Shad);
	Cur.copy(src.Cur);
    }
    int compare(const _pfLoc other) const {
	Shad.compare(other.Shad);
	Cur.compare(other.Cur);
	return 0;
    }
    void set(int value) {
	Shad.set(value);
	Cur.set(value);
    }
#if 0
    void print(FILE *file, char *prefix, char *str) {
	fprintf(file, "%s%s.Shad", prefix, str);
	Shad.print(file, "", "");
	fprintf(file, "%s%s.Cur", prefix, str);
	Cur.print(file, "", "");
    }
#endif
    void print(FILE *file, char *prefix, char *str) {
	Shad.print(file, prefix, str, "Shad");
	Cur.print(file, prefix, str, "Cur");
    }

    void operator=(int value) {
	set(value);
    }
    _pfDim Shad;
    _pfDim Cur;
};




















extern "C" {     // EXPORT to CAPI

/* ------------------ pfImageTile Related Functions--------------------- */
#define PFCLIPTEXTURE_MAX_LEVELS 32

} // extern "C" END of C include export

#define PFCLIPTEXTURE ((pfClipTexture*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFCLIPTEXTUREBUFFER ((pfClipTexture*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfClipTexture : public pfTexture
{

public:
    // constructors and destructors
    // CAPI:basename ClipTexture
    pfClipTexture();
    virtual ~pfClipTexture();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }






    //CAPI:verb GetClipTextureMaxClipSize
    static int getMaxClipSize( int bytespertexel );

    // CAPI:verb IsClipTextureEmulated
    int isEmulated(void) { return emulated; }
    
    // CAPI:verb GetClipTextureNumClippedLevels
    int getNumClippedLevels(void) { return clippedLevels; }
        
    /* generic set-get methods for future use */
    
    // CAPI:verb GetClipTextureCteAttr
    void* getCteAttr( int which );
    
    // CAPI:verb ClipTextureCteAttr
    void setCteAttr( int which, void* val );


    
    

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

/*----------------------------- pfImageTile --------------------------------*/
public:
    // sets and gets
    // CAPI:basename ClipTexture
    void		setLevel(int _level, pfObject* _levelObj);
    pfObject*		getLevel(int _level);
    
    void		setCenter(int _s, int _t, int _r);
    void		getCenter(int *_s, int *_t, int *_r);
    void		getCurCenter(int *_s, int *_t, int *_r);

    void		setClipSize(int _clipSize);
    int			getClipSize(void);

    void		setVirtualLODOffset(int _offset);
    int			getVirtualLODOffset(void);

    void		setNumEffectiveLevels(int _levels);
    int			getNumEffectiveLevels(void);

    void		setNumAllocatedLevels(int _levels);
    int			getNumAllocatedLevels(void);

    void		setVirtualSize(int _width, int _height, int _depth);
    void		getVirtualSize(int *_width, int *_height, int *_depth);

    void		setInvalidBorder(int _nTexels);
    int			getInvalidBorder(void);

    void		setLevelPhaseMargin(int level, int size);
    int			getLevelPhaseMargin(int level);

    void		setLevelPhaseShift(int level, int phaseShiftS, int phaseShiftT, int phaseShiftR);
    void		getLevelPhaseShift(int level, int *phaseShiftS, int *phaseShiftT, int *phaseShiftR);

    void		getOffset(int *_s, int *_t, int *_r);	// get clip subtex Offset

    void                setTexLoadTime(float _msec);
    float               getTexLoadTime(void);

    void	        setMaster(pfClipTexture *_master);
    pfClipTexture*      getMaster(void) const {return master;};
    pfList*             getSlaves(void) const {return slaves;};

    void                setMinClipSizeRatio(float _size);
    float               getMinClipSizeRatio(void);

    //Choose combinations of blurring methods to handle texel overload
    void                setDTRMode(uint _mask);
    uint                getDTRMode(void);
    
    //for virtual clipmap load control
    float               getMinDTRLOD(void);

    //how fast (frames per level) DTR fades in a level
    void                setDTRFadeCount(int _count);
    int                 getDTRFadeCount(void);

    //fraction of texLoadTime to leave as a cushion for minLOD
    void                setDTRBlurMargin(float frac);
    float               getDTRBlurMargin(void);

    void                setLODOffsetLimit(int _lo, int _hi);
    void                getLODOffsetLimit(int *_lo, int *_hi);
    void                setNumEffectiveLevelsLimit(int _lo, int _hi);
    void                getNumEffectiveLevelsLimit(int *_lo, int *_hi);
    void                setMinLODLimit(float _lo, float _hi);
    void                getMinLODLimit(float *_lo, float *_hi);
    void                setMaxLODLimit(float _lo, float _hi);
    void                getMaxLODLimit(float *_lo, float *_hi);
    void		setLODBiasLimit(float _Slo, float _Shi, float _Tlo, float _Thi, float _Rlo, float _Rhi);
    void		getLODBiasLimit(float *_Slo, float *_Shi, float *_Tlo, float *_Thi, float *_Rlo, float *_Rhi);

    // need to remember set values since DTR can change them
    // CAPI:virtual
    virtual void        setLODRange(float _min, float _max);
    virtual void        getLODRange(float *_min, float *_max);
    // CAPI:novirtual
    void                getCurLODRange(float *_min, float *_max);

    //so they get updated in the DRAW process during pr_apply()
    // CAPI:virtual
    virtual void        setLODBias(float _s, float _t, float _r);
    virtual void        getLODBias(float *_s, float *_t, float *_r);
    // CAPI:novirtual
    void                getCurLODBias(float *_s, float *_t, float *_r);

    // CAPI:verb
    void		apply(void);
    // CAPI:verb ApplyClipTextureMemReg
    void		applyMemReg(void);
    // CAPI:verb ApplyClipTextureTexReg
    void		applyTexReg(void);
    
    // CAPI:verb UpdateClipTextureMemReg
    void                updateMemReg(void);
    // CAPI:verb UpdateClipTextureTexReg
    void                updateTexReg(void);
    // CAPI:verb
    void                update(void);
    void                invalidate(void);
    // CAPI:verb ApplyClipTextureVirtualParams
    void                applyVirtualParams(int _offset, int _levels);
    // CAPI:verb ApplyClipTextureCenter
    void                applyCenter(int _s, int _t, int _r);
    // CAPI:verb IsClipTextureVirtual
    int			isVirtual(void);
















protected:

    _pfLoc              center;                 // clip center
    _pfDim              offset;                 // clip texture offset
    _pfDim              size;                   // Clip texture size

    int			invalidBorder;		// Clip invalid border
    int			clipSize;		// Clip Size (square)

    int			clippedLevels;
    int			lodOffset;
    int			numEffectiveLevels;
    int			numAllocatedLevels;
    int			clipDirty;
    int			phaseS[PFCLIPTEXTURE_MAX_LEVELS];
    int			phaseT[PFCLIPTEXTURE_MAX_LEVELS];
    int			phaseR[PFCLIPTEXTURE_MAX_LEVELS];
    int			bloatSize[PFCLIPTEXTURE_MAX_LEVELS];
    int                 formatted;
    int                 invalid;
    int                 shadInvalid;
    pfClipTexture	*master; //if slave, points to master cliptexture
    pfList		*slaves; //if master, points to slave list

    uint                DTRMode; // bitmask: which modes to control texture
    float               minDTRLOD; //minimum texture LOD to use for load control
    float               minSetLOD; //minimum texture LOD
    float               maxSetLOD; //max texture LOD
    float               availTime; //time available for cliptex downloads
    float               blurMargin; //blur to lod loaded by availTime*(1-margin)
    int                 curLevel; //last level change
    int                 fadeCount; //fades left
    float               minClipSizeRatio; //smallest allowed clip size for DTR.
    int                 minClipSize; //derived from clip size ratio & clip size
    float               fadeRate;

    int                 LODOffsetLimitLo, LODOffsetLimitHi;
    int                 numEffectiveLevelsLimitLo, numEffectiveLevelsLimitHi;
    float               minLODLimitLo, minLODLimitHi;
    float               maxLODLimitLo, maxLODLimitHi;
    float		lodBiasSLimitLo, lodBiasSLimitHi;
    float		lodBiasTLimitLo, lodBiasTLimitHi;
    float		lodBiasRLimitLo, lodBiasRLimitHi;

    int                 iLoadCount; // for double buffering invalid load objects
    pfList              *invalidLoads; //track invalidate pyramid texloads
    float               lodBiasS, lodBiasT, lodBiasR;
    int			isvirtual;

    pfTexture           *texture; //cliptexture's parent pfTexture.

    int emulated;
    float fluxedMinDTRLOD[2];

    void* extraData;
    
    pfTexture** cteTex;
    pfMatrix** cteTexMat;
    int* goodAreaHalfSize;
    
    /* pointer to fluxed cte-data */
    pfFlux* fluxCte;

     
public:

    /* index of this cliptexture in listEmulatedClipTex */
    int cteIndex;

    int centerCounter;
    int cteCenterS;    
    int cteCenterT;    
    int cteCenterR;    

private:
    static pfType *classType;



};




#endif // !__PF_IMAGETILE_H__
