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
// pfMPClipTexture.h
//
// $Revision: 1.36 $
// $Date: 2002/05/30 00:33:56 $
//
//
#ifndef __PF_MPClipTexture_H__
#define __PF_MPClipTexture_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf/pfUpdatable.h>
#include <Performer/pf/pfPipe.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfImageTile.h>
#include <Performer/pr/pfImageCache.h>
#include <Performer/pr/pfClipTexture.h>


class pfClipTexture;

extern "C" {     // EXPORT to CAPI

/* ----------------------- pfMPClipTexture Tokens ----------------------- */
/* for master/slave mpcliptextures */

/* center s, t, r */
#define PFMPCLIPTEXTURE_SHARE_CENTER  0x01 

/* DTR parameters: DTRMode, texLoadTime,texLoadTimeFrac, fadeCount, blurMargin*/
#define PFMPCLIPTEXTURE_SHARE_DTR     0x02 

/* tex level params; LODbias (S, T, R), iborder */
#define PFMPCLIPTEXTURE_SHARE_EDGE    0x04

/* tex level params; minLOD, maxLOD */
#define PFMPCLIPTEXTURE_SHARE_LOD  0x08

/* lodOffset, numEffective levels */
#define PFMPCLIPTEXTURE_SHARE_VIRTUAL 0x10

/* share the default set of shared groups */
#define PFMPCLIPTEXTURE_SHARE_DEFAULT  (PFMPCLIPTEXTURE_SHARE_CENTER | \
                                        PFMPCLIPTEXTURE_SHARE_DTR    | \
                                        PFMPCLIPTEXTURE_SHARE_EDGE   | \
                                        PFMPCLIPTEXTURE_SHARE_LOD    | \
                                        PFMPCLIPTEXTURE_SHARE_VIRTUAL)
/* share all current and future share groups */
#define PFMPCLIPTEXTURE_SHARE_ALL     ~0
} // extern "C" END of C include export


#define PFMPCLIPTEXTURE ((pfMPClipTexture*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFMPCLIPTEXTUREBUFFER ((pfMPClipTexture*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfMPClipTexture : public pfUpdatable
{
public:

    void setClipTexture(pfClipTexture *_clip)  {
        PFMPCLIPTEXTURE->nb_setClipTexture(_clip);
    }

    pfClipTexture* getClipTexture()  {
        return PFMPCLIPTEXTURE->nb_getClipTexture();
    }

    void setCenter(int _s, int _t, int _r)  {
        PFMPCLIPTEXTURE->nb_setCenter(_s, _t, _r);
    }

    void getCenter(int *_s, int *_t, int *_r)  {
        PFMPCLIPTEXTURE->nb_getCenter(_s, _t, _r);
    }

    void setInvalidBorder(int _invalidBorder)  {
        PFMPCLIPTEXTURE->nb_setInvalidBorder(_invalidBorder);
    }

    int getInvalidBorder()  {
        return PFMPCLIPTEXTURE->nb_getInvalidBorder();
    }

    void setVirtualLODOffset(int _lodOffset)  {
        PFMPCLIPTEXTURE->nb_setVirtualLODOffset(_lodOffset);
    }

    int getVirtualLODOffset()  {
        return PFMPCLIPTEXTURE->nb_getVirtualLODOffset();
    }

    void setNumEffectiveLevels(int _effectiveLevels)  {
        PFMPCLIPTEXTURE->nb_setNumEffectiveLevels(_effectiveLevels);
    }

    int getNumEffectiveLevels()  {
        return PFMPCLIPTEXTURE->nb_getNumEffectiveLevels();
    }

    void setMaster(pfMPClipTexture *_master)  {
        PFMPCLIPTEXTURE->nb_setMaster(_master);
    }

    pfMPClipTexture* getMaster() const  {
        return PFMPCLIPTEXTURE->nb_getMaster();
    }

    pfList* getSlaves() const  {
        return PFMPCLIPTEXTURE->nb_getSlaves();
    }

    void setShareMask(uint _sharemask)  {
        PFMPCLIPTEXTURE->nb_setShareMask(_sharemask);
    }

    uint getShareMask()  {
        return PFMPCLIPTEXTURE->nb_getShareMask();
    }

    void setLODRange(float _min, float _max)  {
        PFMPCLIPTEXTURE->nb_setLODRange(_min, _max);
    }

    void getLODRange(float *_min, float *_max)  {
        PFMPCLIPTEXTURE->nb_getLODRange(_min, _max);
    }

    pfPipe* getPipe() const  {
        return PFMPCLIPTEXTURE->nb_getPipe();
    }

    void setDTRMode(uint _mask)  {
        PFMPCLIPTEXTURE->nb_setDTRMode(_mask);
    }

    uint getDTRMode()  {
        return PFMPCLIPTEXTURE->nb_getDTRMode();
    }

    void setLODBias(float _lodBiasS, float _lodBiasT, float _lodBiasR)  {
        PFMPCLIPTEXTURE->nb_setLODBias(_lodBiasS, _lodBiasT, _lodBiasR);
    }

    void getLODBias(float *_lodBiasS, float *_lodBiasT, float *_lodBiasR)  {
        PFMPCLIPTEXTURE->nb_getLODBias(_lodBiasS, _lodBiasT, _lodBiasR);
    }

    void setMagFilter(uint _magFilter)  {
        PFMPCLIPTEXTURE->nb_setMagFilter(_magFilter);
    }

    uint getMagFilter()  {
        return PFMPCLIPTEXTURE->nb_getMagFilter();
    }

    void setDTRFadeCount(int _count)  {
        PFMPCLIPTEXTURE->nb_setDTRFadeCount(_count);
    }

    int getDTRFadeCount()  {
        return PFMPCLIPTEXTURE->nb_getDTRFadeCount();
    }

    void setDTRBlurMargin(float frac)  {
        PFMPCLIPTEXTURE->nb_setDTRBlurMargin(frac);
    }

    float getDTRBlurMargin()  {
        return PFMPCLIPTEXTURE->nb_getDTRBlurMargin();
    }

    void setTexLoadTime(float _msec)  {
        PFMPCLIPTEXTURE->nb_setTexLoadTime(_msec);
    }

    float getTexLoadTime()  {
        return PFMPCLIPTEXTURE->nb_getTexLoadTime();
    }

    void setTexLoadTimeFrac(float _frac)  {
        PFMPCLIPTEXTURE->nb_setTexLoadTimeFrac(_frac);
    }

    float getTexLoadTimeFrac()  {
        return PFMPCLIPTEXTURE->nb_getTexLoadTimeFrac();
    }

    float getCurTexLoadTime()  {
        return PFMPCLIPTEXTURE->nb_getCurTexLoadTime();
    }

    void beginRecord(const char *fileName)  {
        PFMPCLIPTEXTURE->nb_beginRecord(fileName);
    }

    void endRecord()  {
        PFMPCLIPTEXTURE->nb_endRecord();
    }

    int isRecording()  {
        return PFMPCLIPTEXTURE->nb_isRecording();
    }

    void beginPlay(const char *fileName)  {
        PFMPCLIPTEXTURE->nb_beginPlay(fileName);
    }

    void endPlay()  {
        PFMPCLIPTEXTURE->nb_endPlay();
    }

    int isPlaying()  {
        return PFMPCLIPTEXTURE->nb_isPlaying();
    }

    void setLODOffsetLimit(int _lo, int _hi)  {
        PFMPCLIPTEXTURE->nb_setLODOffsetLimit(_lo, _hi);
    }

    void getLODOffsetLimit(int *_lo, int *_hi)  {
        PFMPCLIPTEXTURE->nb_getLODOffsetLimit(_lo, _hi);
    }

    void setNumEffectiveLevelsLimit(int _lo, int _hi)  {
        PFMPCLIPTEXTURE->nb_setNumEffectiveLevelsLimit(_lo, _hi);
    }

    void getNumEffectiveLevelsLimit(int *_lo, int *_hi)  {
        PFMPCLIPTEXTURE->nb_getNumEffectiveLevelsLimit(_lo, _hi);
    }

    void setMinLODLimit(float _lo, float _hi)  {
        PFMPCLIPTEXTURE->nb_setMinLODLimit(_lo, _hi);
    }

    void getMinLODLimit(float *_lo, float *_hi)  {
        PFMPCLIPTEXTURE->nb_getMinLODLimit(_lo, _hi);
    }

    void setMaxLODLimit(float _lo, float _hi)  {
        PFMPCLIPTEXTURE->nb_setMaxLODLimit(_lo, _hi);
    }

    void getMaxLODLimit(float *_lo, float *_hi)  {
        PFMPCLIPTEXTURE->nb_getMaxLODLimit(_lo, _hi);
    }

    void setLODBiasLimit(float _Slo, float _Shi, float _Tlo, float _Thi, float _Rlo, float _Rhi)  {
        PFMPCLIPTEXTURE->nb_setLODBiasLimit(_Slo, _Shi, _Tlo, _Thi, _Rlo, _Rhi);
    }

    void getLODBiasLimit(float *_Slo, float *_Shi, float *_Tlo, float *_Thi, float *_Rlo, float *_Rhi)  {
        PFMPCLIPTEXTURE->nb_getLODBiasLimit(_Slo, _Shi, _Tlo, _Thi, _Rlo, _Rhi);
    }

    void apply()  {
        PFMPCLIPTEXTURE->nb_apply();
    }

friend void pfPipe::nb_addMPClipTexture(pfMPClipTexture *clip);
friend int pfPipe::nb_removeMPClipTexture(pfMPClipTexture *clip);

public:	    // Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfMPClipTexture();
    virtual ~pfMPClipTexture();

protected:
    //CAPI:private
    pfMPClipTexture(pfBuffer *buf);
    pfMPClipTexture(const pfMPClipTexture *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }
    
public:			// pfMemory virtual functions
    virtual int	    	  checkDelete();


PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	   pf_applyUpdate(const pfUpdatable *prev,  int upID);
    virtual pfUpdatable*   pf_bufferClone(pfBuffer *buf);
    PF_USEMA_T         *pf_getSync(void) {return sync;};
    _pfMPClipTextureList *pf_getSlaves(void) { return &slaves;};


public:
    // Sets and gets
    //CAPI:basename MPClipTexture
    void			nb_setClipTexture(pfClipTexture *_clip);
    pfClipTexture*		nb_getClipTexture(void);

    void			nb_setCenter(int _s, int _t, int _r);
    void			nb_getCenter(int *_s, int *_t, int *_r);

    void			nb_setInvalidBorder(int _invalidBorder);
    int				nb_getInvalidBorder(void);

    void			nb_setVirtualLODOffset(int _lodOffset);
    int				nb_getVirtualLODOffset(void);

    void			nb_setNumEffectiveLevels(int _effectiveLevels);
    int				nb_getNumEffectiveLevels(void);

    void			nb_setMaster(pfMPClipTexture *_master);
    pfMPClipTexture		*nb_getMaster(void) const {return master.getNum() ? master[0] : NULL;};

    pfList                      *nb_getSlaves(void) const {return readslaves;};

    void			nb_setShareMask(uint _sharemask);
    uint			nb_getShareMask(void);

    void			nb_setLODRange(float _min, float _max);
    void			nb_getLODRange(float *_min, float *_max);

    pfPipe			*nb_getPipe(void) const {return pipe;};

    void                        nb_setDTRMode(uint _mask);
    uint                        nb_getDTRMode(void);


    void                        nb_setLODBias(float _lodBiasS, float _lodBiasT, float _lodBiasR);
    void                        nb_getLODBias(float *_lodBiasS, float *_lodBiasT, float *_lodBiasR);

    void                        nb_setMagFilter(uint _magFilter);
    uint                        nb_getMagFilter(void);

    void                        nb_setDTRFadeCount(int _count);
    int                         nb_getDTRFadeCount(void);

    void                        nb_setDTRBlurMargin(float frac);
    float                       nb_getDTRBlurMargin(void);

    void                        nb_setTexLoadTime(float _msec);
    float                       nb_getTexLoadTime(void);

    void                        nb_setTexLoadTimeFrac(float _frac);
    float                       nb_getTexLoadTimeFrac(void);

    float			nb_getCurTexLoadTime(void); // read-only

    void			nb_beginRecord(const char *fileName);
    void			nb_endRecord();
    int				nb_isRecording();
    void			nb_beginPlay(const char *fileName);
    void			nb_endPlay(); /* used to abort (not required) */
    int				nb_isPlaying();

    void                        nb_setLODOffsetLimit(int _lo, int _hi);
    void                        nb_getLODOffsetLimit(int *_lo, int *_hi);
    void                        nb_setNumEffectiveLevelsLimit(int _lo, int _hi);
    void                        nb_getNumEffectiveLevelsLimit(int *_lo, int *_hi);
    void                        nb_setMinLODLimit(float _lo, float _hi);
    void                        nb_getMinLODLimit(float *_lo, float *_hi);
    void                        nb_setMaxLODLimit(float _lo, float _hi);
    void                        nb_getMaxLODLimit(float *_lo, float *_hi);
    void		        nb_setLODBiasLimit(float _Slo, float _Shi, float _Tlo, float _Thi, float _Rlo, float _Rhi);
    void		        nb_getLODBiasLimit(float *_Slo, float *_Shi, float *_Tlo, float *_Thi, float *_Rlo, float *_Rhi);

    //CAPI:verb
    void			nb_apply(void);

protected:	    		// data
    pfPipe			*pipe; // pipe attached to clip texture
    int				cS, cT, cR;
    int				invalidBorder;
    pfClipTexture*		clip;
    int				lodOffset;
    int				numEffectiveLevels;
    float			minLOD, maxLOD;
    float                       DTRMaxIBorder;
    int                         DTRMaxSharpen;

    uint                        applyMask; // which masks need to be applied?
    _pfMPClipTextureList	master; // XXX wimpy: overkill for 1 master
    _pfMPClipTextureList	slaves;
    pfList                      *readslaves;
    PF_USEMA_T			*sync; //to synchronize slave with its master
    uint			shareMask; //what data master and slaves share
    uint                        DTRMode; //if tiles not there, how to blur

    const char			*recordFileName; // file being recorded into
    FILE			*recordFileP;
    const char			*playFileName;   // file being played back
    FILE			*playFileP;
    int				prevPlayFrameStamp, nextPlayFrameStamp;
    int				prevPlayFrameCount;

#ifdef KEEP_APPLIED
    int                         *applied; //set after first apply
#endif /* KEEP_APPLIED */
    float                       lodBiasS, lodBiasT, lodBiasR;
    uint                        magFilter;
    int                         fadeCount;
    float                       texLoadTime; //takes precedence over frac if >=0
    float                       texLoadTimeFrac;
    float                       curTexLoadTime;
    float                       blurMargin;

    int                         LODOffsetLimitLo, LODOffsetLimitHi;
    int                         numEffectiveLevelsLimitLo, numEffectiveLevelsLimitHi;
    float                       minLODLimitLo, minLODLimitHi;
    float                       maxLODLimitLo, maxLODLimitHi;
    float		        lodBiasSLimitLo, lodBiasSLimitHi;
    float		        lodBiasTLimitLo, lodBiasTLimitHi;
    float		        lodBiasRLimitLo, lodBiasRLimitHi;
private:
    static pfType *classType;
public:
    void * _pfMPClipTextureExtraData;
};

PF_UPDATABLE_REF(pfMPClipTexture, _pfMPClipTextureRef);

#endif  // !__PF_MPClipTexture_H__


