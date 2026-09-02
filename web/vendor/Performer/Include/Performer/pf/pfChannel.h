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
// pfChannel.h		Channel class include file
//
// $Revision: 1.286 $
// $Date: 2002/09/17 23:58:13 $
// $Revision: 1.286 $
// $Date: 2002/09/17 23:58:13 $

#ifndef __PF_CHANNEL_H__
#define __PF_CHANNEL_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <math.h>
#include <string.h>


#include <Performer/pf/pfUpdatable.h>
#include <Performer/pf/pfPipe.h>
#include <Performer/pf/pfPipeWindow.h>
#include <Performer/pf/pfPipeVideoChannel.h>
#include <Performer/pf/pfEarthSky.h>
#include <Performer/pf/pfLODState.h>
#include <Performer/pf/pfFrameStats.h>
#include <Performer/pf/pfScene.h>

#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfDispList.h>





class pfCteChanData;
class pfCompositor;
class pfSphereMapWarp;



// XXX remove, used for comparisons between 2.4 and 2.5
#define _PF_VERSION_2_5 1

struct _pfLSourceState : public pfStruct
{
    void		construct() {}
    _pfLSourceState()	{}

    pfLightSource	*lsource;
    pfLight             *light;
    pfMatrix		mat, invMat;
    int			shadEnable, projEnable, active;
    pfDispList		*dlist;		// For shadows

    int		operator==(const _pfLSourceState&) {
	return 0;
    }
    int		operator!=(const _pfLSourceState&) {
	return 1;
    }
};

#define PF_PHONGMAP_REQUEST_SPECULAR 1
#define PF_PHONGMAP_REQUEST_DIFFUSE  2


struct pfPhongMapCBData {
  pfTexture *specMap;
  pfTexture *warpedMap;
  pfFlux    *lastLightFlux;
  pfFlux    *lightFlux;
  pfFlux    *warperFlux;
  pfFlux    *drawFlagFlux;
  int slices;
  int rings;
  char which; //What kind of map to generate
};


SLIST_DECLARE(_pfLSourceState, _pfLSourceStateList);

#define PFCHAN_HAS_OFFSETS	0x1
#define PFCHAN_USE_CULL_VOL	0x2
#define PFCHAN_MULTIPASS        0x4
#define PFCHAN_SHADOWS		0x8
#define PFCHAN_LODSTATE_ACTIVE	0x100
#define PFCHAN_STRESS_ACTIVE	0x200
#define PFCHAN_ASD_STRESS_ACTIVE 0x400
#define PFCHAN_ASD_LODSCALE_ACTIVE 0x800

#define PFCHAN_LPOINT_DODL	0x1

// Buffer used to safely propagate info from draw back to app.
struct pfChanUpBuffer : public pfStruct
{
    PF_ULOCK_T		lock;
    
    pfMPHistStats	mp;
    _pfFrameStatsData	cur;
    double		afterSwap, drawProcStart;
    uint 		clearFlag, dirtyFlags, newStats;
    
    float		curLoad;
    
};

#define PF_BINMASK_WIDTH    (sizeof(uint64_t) * 8)
#define PF_BINMASK_TOP_BIT  ((uint64_t)1 << (PF_BINMASK_WIDTH-1))

typedef _pfTemplateList<uint64_t>    _pfUint64List;

// this structure contains data that can be shared by different copies of
// pfChannel. For example, the display lists cannot be included here.
typedef struct _pfBinDataType {
    int         sortOrdersPriority; /* used to determine what sortOrders
				       a child will have (it inherits it
				       from a parent with the highest 
				       priority value). 
				       Default is 0 - meaning don't inherit */
    uint64_t    childOrderMask;// mask used by children, mask of all parents
                               // of a child is combined to create a sort key
    int         index;         // bin index
    
    uint64_t    rootParentsMask; // mask of the first 63 root parents
    _pfUint64List *rootParents;  // mask of all additional bin root parents
    _pfIntList  *children;     // list of all bin children (subbins)
    
    int         flags;         // bin flags  

    // XXX optional new items:
    void        *preDrawCBack;  // pre-draw callbacks
    void        *postDrawCBack; // post-draw callbacks
    void        *preCullCBack;  // pre-cull callbacks
    void        *postCullCBack; // post-cull callbacks
    void        *userData;      // callback user data
    int         userDataSize;   // size of user data
    //void        *polytop;      // polytopes used to cull into some bin
} _pfBinData;

typedef _pfTemplateList2<_pfBinData>  _pfBinList;


class pfChannel;
class _pfCuller;
class pfCullProgram;

extern pfChannel *_pfDrawChannel;

///////////////////////////////////////////////////////////////////////////

extern "C" {     // EXPORT to CAPI

/* ----------------------- pfChannel Tokens ----------------------- */


#define PFMPASS_GLOBAL_AMBIENT	      0x1
#define PFMPASS_EMISSIVE_PASS	      0x2
#define PFMPASS_NONTEX_SCENE	      0x4
#define PFMPASS_NO_TEXTURE_ALPHA      0x8
#define PFMPASS_FORCE_MULTISAMPLE     0x10
#define PFMPASS_BLACK_PASS            0x20

/* pfChanTravMode() (Uses PFTRAV_ tokens) */
#define PFCULL_VIEW		0x1
#define PFCULL_SORT		0x2
#define PFCULL_GSET		0x4
#define PFCULL_IGNORE_LSOURCES	0x8
#define PFCULL_PROGRAM      	0x10
#define PFCULL_ALL		(PFCULL_VIEW|PFCULL_SORT|PFCULL_GSET)

/* pfChanShare() */
#define PFCHAN_FOV		    0x1
#define PFCHAN_VIEW		    0x2
#define PFCHAN_NEARFAR		    0x4
#define PFCHAN_SCENE		    0x10
#define PFCHAN_STRESS	    	    0x20
#define PFCHAN_STRESSFILTER         PFCHAN_STRESS
#define PFCHAN_LOD		    0x40
#define PFCHAN_EARTHSKY		    0x80
#define PFCHAN_SWAPBUFFERS	    0x100
#define PFCHAN_VIEW_OFFSETS	    0x200
#define PFCHAN_STATS_DRAWMODE	    0x400
#define PFCHAN_APPFUNC		    0x1000
#define PFCHAN_CULLFUNC		    0x2000
#define PFCHAN_DRAWFUNC		    0x4000
#define PFCHAN_VIEWPORT		    0x8000
#define PFCHAN_SWAPBUFFERS_HW	    0x10000
#define PFCHAN_CULL_VOLUME	    0x20000
#define PFCHAN_GSTATE		    0x40000
#define PFCHAN_PRICLASS_LIST	    0x80000
#define PFCHAN_LPOINTFUNC	    0x100000
#define PFCHAN_POSTLPOINTFUNC       0x200000
    
#define PFDRAW_OFF		0
#define PFDRAW_ON		1
   
/* pfChanLODAttr() */
#define PFLOD_SCALE		2
#define PFLOD_FADE		3
#define PFLOD_STRESS_PIX_LIMIT	4
#define PFLOD_FRUST_SCALE	5

/* pfChanBinOrder,Sort() */
#define PFSORT_MAX_KEYS		64

/* The default bin */
#define PFSORT_DEFAULT_BIN      -1

/* Shaded stuff is drawn dead last, so it uses the last bin */
#define PFSORT_SHADER_BIN       3  //0x7fff
#define PFSORT_SHADER_BIN_ORDER 9998

/* patchy fog bin */
#define PFSORT_PATCHYFOG_BIN       4
#define PFSORT_PATCHYFOG_BIN_ORDER 2

/* opaque polygons are first */
#define PFSORT_OPAQUE_BIN	0
#define PFSORT_OPAQUE_BIN_ORDER 0
/* then transparent polygons */
#define PFSORT_TRANSP_BIN	1		
#define PFSORT_TRANSP_BIN_ORDER 1
/* Calligraphics must be rendered last */
#define PFSORT_LPSTATE_BIN 2
#define PFSORT_LPSTATE_BIN_ORDER 9999

#define PFSORT_NO_ORDER		-1

/* bin flags */
#define PFBIN_NONEXCLUSIVE_BIN       (1<<0)
#define PFBIN_DONT_DRAW_BY_DEFAULT   (1<<1)
#define PFBIN_DONT_RUN_CULL_PROGRAM  (1<<2)

/* bin callback type */
#define PFBIN_CALLBACK_PRE_DRAW   1
#define PFBIN_CALLBACK_POST_DRAW  2

/* pfChanBinSort() */
#define PFSORT_NO_SORT		0
#define PFSORT_BY_STATE		1
#define PFSORT_QUICK		2
#define PFSORT_FRONT_TO_BACK	3
#define PFSORT_BACK_TO_FRONT	4
#define PFSORT_DRAW_ORDER	5
#define PFSORT_END		-1

/* PFSORT_BY_STATE only */
#define PFSORT_STATE_BGN	10
#define PFSORT_STATE_END	11

/* pfChanProjMode() - defines viewport projection */
#define PFCHAN_PROJ_OUTPUT_VIEWPORT	0 
#define PFCHAN_PROJ_VIEWPORT	1 
#define PFCHAN_PROJ_WINDOW	2

/* ----------------pfChannel Related Routines------------------ */

/* pfChanPick() */
extern DLLEXPORT void	pfNodePickSetup(pfNode* _node);
} // extern "C" END of C include export

#define PFCHANNEL ((pfChannel*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFCHANNELBUFFER ((pfChannel*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfChannel : public pfUpdatable 
{
public:

    int getFrustType() const  {
        return PFCHANNEL->nb_getFrustType();
    }

    void setAspect(int  which, float xyaspect)  {
        PFCHANNEL->nb_setAspect(which, xyaspect);
    }

    float getAspect()  {
        return PFCHANNEL->nb_getAspect();
    }

    void getFOV(float *fovH, float *fovV) const  {
        PFCHANNEL->nb_getFOV(fovH, fovV);
    }

    void setNearFar(float n, float f)  {
        PFCHANNEL->nb_setNearFar(n, f);
    }

    void getNearFar(float *n, float *f) const  {
        PFCHANNEL->nb_getNearFar(n, f);
    }

    void getNear(pfVec3& _ll, pfVec3& _lr, pfVec3& _ul, pfVec3& _ur) const  {
        PFCHANNEL->nb_getNear(_ll, _lr, _ul, _ur);
    }

    void getFar(pfVec3& _ll, pfVec3& _lr, pfVec3& _ul, pfVec3& _ur) const  {
        PFCHANNEL->nb_getFar(_ll, _lr, _ul, _ur);
    }

    void getPtope(pfPolytope *dst) const  {
        PFCHANNEL->nb_getPtope(dst);
    }

    int getEye(pfVec3& eye) const  {
        return PFCHANNEL->nb_getEye(eye);
    }

    void makePersp(float l, float r, float b, float t)  {
        PFCHANNEL->nb_makePersp(l, r, b, t);
    }

    void makeOrtho(float l, float r, float b, float t)  {
        PFCHANNEL->nb_makeOrtho(l, r, b, t);
    }

    void makeOutputPersp(float l, float r, float b, float t)  {
        PFCHANNEL->nb_makeOutputPersp(l, r, b, t);
    }

    void makeOutputOrtho(float l, float r, float b, float t)  {
        PFCHANNEL->nb_makeOutputOrtho(l, r, b, t);
    }

    void getLeftRightBottomTop(float *l, float *r, float *b, float *t)  {
        PFCHANNEL->nb_getLeftRightBottomTop(l, r, b, t);
    }

    void makeSimple(float fov)  {
        PFCHANNEL->nb_makeSimple(fov);
    }

    void orthoXform(pfFrustum *fr, const pfMatrix& mat)  {
        PFCHANNEL->nb_orthoXform(fr, mat);
    }

    int contains(const pfVec3& pt) const  {
        return PFCHANNEL->nb_contains(pt);
    }

    int contains(const pfSphere *sphere) const  {
        return PFCHANNEL->nb_contains(sphere);
    }

    int contains(const pfBox *box) const  {
        return PFCHANNEL->nb_contains(box);
    }

    int contains(const pfCylinder *cyl) const  {
        return PFCHANNEL->nb_contains(cyl);
    }

    void apply()  {
        PFCHANNEL->nb_apply();
    }

    pfPipe* getPipe() const  {
        return PFCHANNEL->nb_getPipe();
    }

    pfPipeWindow* getPWin()  {
        return PFCHANNEL->nb_getPWin();
    }

    int getPWinIndex()  {
        return PFCHANNEL->nb_getPWinIndex();
    }

    pfPipeVideoChannel* getPVChan() const  {
        return PFCHANNEL->nb_getPVChan();
    }

    pfPipeVideoChannel* getFramePVChan() const  {
        return PFCHANNEL->nb_getFramePVChan();
    }

    void setPWinPVChanIndex(int num)  {
        PFCHANNEL->nb_setPWinPVChanIndex(num);
    }

    int getPWinPVChanIndex() const  {
        return PFCHANNEL->nb_getPWinPVChanIndex();
    }

    void setFOV(float fovH, float fovV)  {
        PFCHANNEL->nb_setFOV(fovH, fovV);
    }

    void setViewport(float l, float r, float b, float t)  {
        PFCHANNEL->nb_setViewport(l, r, b, t);
    }

    void getViewport(float *l, float *r, float *b, float *t) const  {
        PFCHANNEL->nb_getViewport(l, r, b, t);
    }

    void setOutputViewport(float l, float r, float b, float t)  {
        PFCHANNEL->nb_setOutputViewport(l, r, b, t);
    }

    void getOutputViewport(float *l, float *r, float *b, float *t) const  {
        PFCHANNEL->nb_getOutputViewport(l, r, b, t);
    }

    void getOrigin(int  *xo, int  *yo) const  {
        PFCHANNEL->nb_getOrigin(xo, yo);
    }

    void getSize(int  *xs, int  *ys) const  {
        PFCHANNEL->nb_getSize(xs, ys);
    }

    void getOutputOrigin(int  *xo, int  *yo) const  {
        PFCHANNEL->nb_getOutputOrigin(xo, yo);
    }

    void getOutputSize(int  *xs, int  *ys) const  {
        PFCHANNEL->nb_getOutputSize(xs, ys);
    }

    void setPixScale(float s)  {
        PFCHANNEL->nb_setPixScale(s);
    }

    void setMinPixScale(float min)  {
        PFCHANNEL->nb_setMinPixScale(min);
    }

    void setMaxPixScale(float max)  {
        PFCHANNEL->nb_setMaxPixScale(max);
    }

    float getPixScale() const  {
        return PFCHANNEL->nb_getPixScale();
    }

    float getMinPixScale() const  {
        return PFCHANNEL->nb_getMinPixScale();
    }

    float getMaxPixScale() const  {
        return PFCHANNEL->nb_getMaxPixScale();
    }

    void setProjMode(int mode)  {
        PFCHANNEL->nb_setProjMode(mode);
    }

    int getProjMode() const  {
        return PFCHANNEL->nb_getProjMode();
    }

    void setShare(uint  mask)  {
        PFCHANNEL->nb_setShare(mask);
    }

    uint getShare() const  {
        return PFCHANNEL->nb_getShare();
    }

    void setAutoAspect(int  which)  {
        PFCHANNEL->nb_setAutoAspect(which);
    }

    int getAutoAspect() const  {
        return PFCHANNEL->nb_getAutoAspect();
    }

    void getBaseFrust(pfFrustum *frust) const  {
        PFCHANNEL->nb_getBaseFrust(frust);
    }

    void setViewOffsets(pfVec3& xyz, pfVec3& hpr)  {
        PFCHANNEL->nb_setViewOffsets(xyz, hpr);
    }

    void getViewOffsets(pfVec3& xyz, pfVec3& hpr) const  {
        PFCHANNEL->nb_getViewOffsets(xyz, hpr);
    }

    void setView(pfVec3& vp, pfVec3& vd)  {
        PFCHANNEL->nb_setView(vp, vd);
    }

    void setViewD(pfVec3d& vp, pfVec3d& vd)  {
        PFCHANNEL->nb_setViewD(vp, vd);
    }

    void getView(pfVec3& vp, pfVec3& vd)  {
        PFCHANNEL->nb_getView(vp, vd);
    }

    void getViewD(pfVec3d& vp, pfVec3d& vd)  {
        PFCHANNEL->nb_getViewD(vp, vd);
    }

    void setViewMat(pfMatrix& mat)  {
        PFCHANNEL->nb_setViewMat(mat);
    }

    void setViewMatD(pfMatrix4d& mat)  {
        PFCHANNEL->nb_setViewMatD(mat);
    }

    void getViewMat(pfMatrix& mat) const  {
        PFCHANNEL->nb_getViewMat(mat);
    }

    void getViewMatD(pfMatrix4d& mat) const  {
        PFCHANNEL->nb_getViewMatD(mat);
    }

    void getOffsetViewMat(pfMatrix& mat) const  {
        PFCHANNEL->nb_getOffsetViewMat(mat);
    }

    void setCullPtope(const pfPolytope *vol)  {
        PFCHANNEL->nb_setCullPtope(vol);
    }

    int getCullPtope(pfPolytope *vol, int space) const  {
        return PFCHANNEL->nb_getCullPtope(vol, space);
    }

    void* allocChanData(int size)  {
        return PFCHANNEL->nb_allocChanData(size);
    }

    void setChanData(void *data, size_t size)  {
        PFCHANNEL->nb_setChanData(data, size);
    }

    void* getChanData() const  {
        return PFCHANNEL->nb_getChanData();
    }

    size_t getChanDataSize() const  {
        return PFCHANNEL->nb_getChanDataSize();
    }

    void setTravFunc(int trav, pfChanFuncType func)  {
        PFCHANNEL->nb_setTravFunc(trav, func);
    }

    pfChanFuncType getTravFunc(int trav) const  {
        return PFCHANNEL->nb_getTravFunc(trav);
    }

    void setTravMode(int  trav, int  mode)  {
        PFCHANNEL->nb_setTravMode(trav, mode);
    }

    int getTravMode(int  trav) const  {
        return PFCHANNEL->nb_getTravMode(trav);
    }

    void setTravMask(int  which, uint  mask)  {
        PFCHANNEL->nb_setTravMask(which, mask);
    }

    uint getTravMask(int  which) const  {
        return PFCHANNEL->nb_getTravMask(which);
    }

    void setStressFilter(float frac, float low, float high, float s, float max)  {
        PFCHANNEL->nb_setStressFilter(frac, low, high, s, max);
    }

    void getStressFilter(float *frac, float *low, float *high, float *s, float *max) const  {
        PFCHANNEL->nb_getStressFilter(frac, low, high, s, max);
    }

    void setStress(float stress)  {
        PFCHANNEL->nb_setStress(stress);
    }

    float getStress() const  {
        return PFCHANNEL->nb_getStress();
    }

    float getLoad() const  {
        return PFCHANNEL->nb_getLoad();
    }

    void setScene(pfScene *s)  {
        PFCHANNEL->nb_setScene(s);
    }

    pfScene* getScene() const  {
        return PFCHANNEL->nb_getScene();
    }

    void setESky(pfEarthSky *_es)  {
        PFCHANNEL->nb_setESky(_es);
    }

    pfEarthSky* getESky() const  {
        return PFCHANNEL->nb_getESky();
    }

    void setGState(pfGeoState *_gstate)  {
        PFCHANNEL->nb_setGState(_gstate);
    }

    pfGeoState* getGState() const  {
        return PFCHANNEL->nb_getGState();
    }

    void setGStateTable(pfList *list)  {
        PFCHANNEL->nb_setGStateTable(list);
    }

    pfList* getGStateTable() const  {
        return PFCHANNEL->nb_getGStateTable();
    }

    void setLODAttr(int  attr, float val)  {
        PFCHANNEL->nb_setLODAttr(attr, val);
    }

    float getLODAttr(int  attr) const  {
        return PFCHANNEL->nb_getLODAttr(attr);
    }

    void setLODState(const pfLODState *ls)  {
        PFCHANNEL->nb_setLODState(ls);
    }

    void getLODState(pfLODState *ls) const  {
        PFCHANNEL->nb_getLODState(ls);
    }

    void setLODStateList(pfList *stateList)  {
        PFCHANNEL->nb_setLODStateList(stateList);
    }

    pfList* getLODStateList() const  {
        return PFCHANNEL->nb_getLODStateList();
    }

    int setStatsMode(uint  _mode, uint  _val)  {
        return PFCHANNEL->nb_setStatsMode(_mode, _val);
    }

    pfFrameStats* getFStats()  {
        return PFCHANNEL->nb_getFStats();
    }

    void setCalligEnable(int _enable)  {
        PFCHANNEL->nb_setCalligEnable(_enable);
    }

    int getCalligEnable() const  {
        return PFCHANNEL->nb_getCalligEnable();
    }

    pfCalligraphic* getCurCallig()  {
        return PFCHANNEL->nb_getCurCallig();
    }

    void setBinSort(int bin, int sortType, uint64_t *sortOrders)  {
        PFCHANNEL->nb_setBinSort(bin, sortType, sortOrders);
    }

    int getBinSort(int bin, uint64_t *sortOrders)  {
        return PFCHANNEL->nb_getBinSort(bin, sortOrders);
    }

    void setBinSortPriority(int bin, int priority)  {
        PFCHANNEL->nb_setBinSortPriority(bin, priority);
    }

    int getBinSortPriority(int bin) const  {
        return PFCHANNEL->nb_getBinSortPriority(bin);
    }

    void setBinOrder(int bin, int order)  {
        PFCHANNEL->nb_setBinOrder(bin, order);
    }

    int getBinOrder(int bin) const  {
        return PFCHANNEL->nb_getBinOrder(bin);
    }

    void setBinChildOrderMask(int bin, uint64_t orderMask)  {
        PFCHANNEL->nb_setBinChildOrderMask(bin, orderMask);
    }

    uint64_t getBinChildOrderMask(int bin) const  {
        return PFCHANNEL->nb_getBinChildOrderMask(bin);
    }

    void setBinFlags(int bin, int flags)  {
        PFCHANNEL->nb_setBinFlags(bin, flags);
    }

    int getBinFlags(int bin) const  {
        return PFCHANNEL->nb_getBinFlags(bin);
    }

    void setBinCallBack(int bin, int type, pfDListFuncType func)  {
        PFCHANNEL->nb_setBinCallBack(bin, type, func);
    }

    pfDListFuncType getBinCallBack(int bin, int type)  {
        return PFCHANNEL->nb_getBinCallBack(bin, type);
    }

    void setBinUserData(int bin, void *userData, int size)  {
        PFCHANNEL->nb_setBinUserData(bin, userData, size);
    }

    void* getBinUserData(int bin, int *size)  {
        return PFCHANNEL->nb_getBinUserData(bin, size);
    }

    int getFreeBin()  {
        return PFCHANNEL->nb_getFreeBin();
    }

    int findSubBin(int bin1, int bin2, int create)  {
        return PFCHANNEL->nb_findSubBin(bin1, bin2, create);
    }

    int findBinParent(int bin, int lastKnownParent)  {
        return PFCHANNEL->nb_findBinParent(bin, lastKnownParent);
    }

    pfCullProgram* getCullProgram()  {
        return PFCHANNEL->nb_getCullProgram();
    }

    void addBinChild(int child, int root, uint64_t rootMask)  {
        PFCHANNEL->nb_addBinChild(child, root, rootMask);
    }

    int isSubbinOf(int bin1, int bin2)  {
        return PFCHANNEL->nb_isSubbinOf(bin1, bin2);
    }

    int findBinByMask(uint64_t binParentsMask, _pfUint64List *binParents, uint64_t childOrderMask, int sortOderParent, int create)  {
        return PFCHANNEL->nb_findBinByMask(binParentsMask, binParents, childOrderMask, sortOderParent, create);
    }

    int attach(pfChannel *_chan1)  {
        return PFCHANNEL->nb_attach(_chan1);
    }

    int detach(pfChannel *_chan1)  {
        return PFCHANNEL->nb_detach(_chan1);
    }

    int ASDattach(pfChannel *_chan1)  {
        return PFCHANNEL->nb_ASDattach(_chan1);
    }

    int ASDdetach(pfChannel *_chan1)  {
        return PFCHANNEL->nb_ASDdetach(_chan1);
    }

    void passChanData()  {
        PFCHANNEL->nb_passChanData();
    }

    int pick(int mode, float px, float py, float radius, pfHit **pickList[])  {
        return PFCHANNEL->nb_pick(mode, px, py, radius, pickList);
    }

    void clear()  {
        PFCHANNEL->nb_clear();
    }

    void drawStats()  {
        PFCHANNEL->nb_drawStats();
    }

    int isect(pfNode *node, pfSegSet *segSet, pfHit **hits[], pfMatrix *ma)  {
        return PFCHANNEL->nb_isect(node, segSet, hits, ma);
    }

    void computeMatrices(pfMatrix *model, pfMatrix *proj)  {
        PFCHANNEL->nb_computeMatrices(model, proj);
    }
public:
    //CAPI:basename Chan
    //CAPI:updatable
    //CAPI:newargs
    pfChannel(pfPipe *p);
    virtual ~pfChannel();
    
protected:
    // Run-time creation.
    pfChannel(pfBuffer *buf, pfPipe *parent);
    // Copy for buffer.
    pfChannel(const pfChannel* prev, pfBuffer *buf);
    
public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }
    
public:			// pfMemory virtual functions
    
PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int  updateId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);
    virtual void	    pf_copyToDraw(const pfUpdatable *_srcChan);
    virtual void	    pf_updateBufferList(pfBuffer *buf, int addRemove) {
        buf->pf_updateList(this, addRemove);
    }
    
PFINTERNAL:		// pfFrustum lookalike sets/gets
    int		nb_getFrustType() const { 
	return viewFrust->getFrustType();
    }
    void	nb_setAspect(int  which, float xyaspect);
    float	nb_getAspect() { 
	return viewFrust->getAspect();
    }

    void	nb_getFOV(float *fovH, float *fovV) const {
	viewFrust->getFOV(fovH, fovV);
    }
    
    void	nb_setNearFar(float n, float f);
    void	nb_getNearFar(float *n, float *f) const {
	viewFrust->getNearFar(n, f);
    }
    void 	nb_getNear(pfVec3& _ll, pfVec3& _lr, pfVec3& _ul, pfVec3& _ur) const {
	viewFrust->getNear(_ll, _lr, _ul, _ur);
    }
    void	nb_getFar(pfVec3& _ll, pfVec3& _lr, pfVec3& _ul, pfVec3& _ur) const {
	viewFrust->getFar(_ll, _lr, _ul, _ur);
    }
    void	nb_getPtope(pfPolytope *dst) const { 
	viewFrust->getPtope(dst);
    }
    int		nb_getEye(pfVec3& eye) const {
	return viewFrust->getEye(eye);
    }

PFINTERNAL:		// pfFrustum lookalike functions
    //CAPI:verb
    void	   nb_makePersp(float l, float r, float b, float t);
    void	   nb_makeOrtho(float l, float r, float b, float t);
    void	   nb_makeOutputPersp(float l, float r, float b, float t);
    void	   nb_makeOutputOrtho(float l, float r, float b, float t);
    void	   nb_getLeftRightBottomTop(float *l, float *r, float *b, float *t){ nomViewFrust->getLeftRightBottomTop(l, r, b, t); };
    void	   nb_makeSimple(float fov);
    void	   nb_orthoXform(pfFrustum *fr, const pfMatrix& mat);

    // CAPI:verb ChanContainsPt
    int nb_contains(const pfVec3& pt) const;

    // CAPI:verb ChanContainsSphere
    int nb_contains(const pfSphere *sphere) const;
    // CAPI:verb ChanContainsBox
    int nb_contains(const pfBox *box) const;
    // CAPI:verb ChanContainsCyl
    int nb_contains(const pfCylinder *cyl) const;

    void nb_apply() {
	viewFrust->apply();
    }
    //CAPI:noverb
    
PFINTERNAL:		// class specific sets and gets
    pfPipe*	   nb_getPipe() const;

    // set/get PWin
    pfPipeWindow*   nb_getPWin();
    int		    nb_getPWinIndex();
    
    // set/get Video Channel
    pfPipeVideoChannel *nb_getPVChan(void) const; // get APP copy 
    pfPipeVideoChannel *nb_getFramePVChan(void) const; // get copy for this proc this frame
    void	    nb_setPWinPVChanIndex(int num);
    int		    nb_getPWinPVChanIndex() const { return pVChanIndex; }
        
    void	    nb_setFOV(float fovH, float fovV);

    void	    nb_setViewport(float l, float r, float b, float t);
    void	    nb_getViewport(float *l, float *r, float *b, float *t) const {
	*l = vpLeft; *r = vpRight; *b = vpBottom; *t = vpTop;	/*  */
    }
    void	    nb_setOutputViewport(float l, float r, float b, float t);
    void	    nb_getOutputViewport(float *l, float *r, float *b, float *t) const;
    
    void	    nb_getOrigin(int  *xo, int  *yo) const;
    void	    nb_getSize(int  *xs, int  *ys) const;
    void	    nb_getOutputOrigin(int  *xo, int  *yo) const;
    void	    nb_getOutputSize(int  *xs, int  *ys) const;
    
    void	    nb_setPixScale(float s);
    void	    nb_setMinPixScale(float min);
    void	    nb_setMaxPixScale(float max);
    float	    nb_getPixScale() const { return pixScale; }
    float	    nb_getMinPixScale() const { return minPixScale; }
    float	    nb_getMaxPixScale() const { return maxPixScale; }
    void	    nb_setProjMode(int mode);
    int		    nb_getProjMode() const { return projMode; }
    
    void	    nb_setShare(uint  mask);
    uint	    nb_getShare() const {
	return shareMask;
    }
    
    void	   nb_setAutoAspect(int  which);
    int 	   nb_getAutoAspect() const {
	return autoAspect;
    }

    void nb_getBaseFrust(pfFrustum *frust) const {
	pfMemory::copy(frust, nomViewFrust);
    }

    void	   nb_setViewOffsets(pfVec3& xyz, pfVec3& hpr);
    void	   nb_getViewOffsets(pfVec3& xyz, pfVec3& hpr) const {
	PFCOPY_VEC3(xyz, viewOffset.xyz);
	PFCOPY_VEC3(hpr, viewOffset.hpr);
    }
    void	   nb_setView(pfVec3& vp, pfVec3& vd);
    void	   nb_setViewD(pfVec3d& vp, pfVec3d& vd);
    void	   nb_getView(pfVec3& vp, pfVec3& vd);
    void	   nb_getViewD(pfVec3d& vp, pfVec3d& vd);
    
    void	   nb_setViewMat(pfMatrix& mat);
    void	   nb_setViewMatD(pfMatrix4d& mat);
    void	   nb_getViewMat(pfMatrix& mat) const {
	PFCOPY_MAT(mat, nomViewMat);
    }
    void	   nb_getViewMatD(pfMatrix4d& mat) const;
    void	   nb_getOffsetViewMat(pfMatrix& mat) const {
	PFCOPY_MAT(mat, viewMat);
    }
    
    void	   nb_setCullPtope(const pfPolytope *vol);
    int	   	   nb_getCullPtope(pfPolytope *vol, int space) const {
	int use_cull_vol = ((flags&PFCHAN_USE_CULL_VOL) != 0);
	pfMemory::copy(vol,
	    use_cull_vol ? (space == PF_WORLDSPACE ? cullVol : nomCullVol)
			 : (space == PF_WORLDSPACE ? viewFrust : nomViewFrust));
	return use_cull_vol;
    }
    // Allocate passthrough data chunk and return handle to it.
    //CAPI:verb AllocChanData
    void*	   nb_allocChanData(int size);
    //CAPI:verb ChanData
    void	   nb_setChanData(void *data, size_t size);
    //CAPI:verb GetChanData
    void*	   nb_getChanData() const { 
	return passThru;
    }
    //CAPI:verb GetChanDataSize
    size_t 	   nb_getChanDataSize() const { 
	return passThruSize;
    }
    //CAPI:noverb
    void	   nb_setTravFunc(int trav, pfChanFuncType func);
    pfChanFuncType nb_getTravFunc(int trav) const {
	return travFuncs[trav];
    }

    void	   nb_setTravMode(int  trav, int  mode);
    int 	   nb_getTravMode(int  trav) const;
    
    void	   nb_setTravMask(int  which, uint  mask);
    uint 	   nb_getTravMask(int  which) const {
	return travMasks[which];
    }
    
    void	   nb_setStressFilter(float frac, float low, float high, float s, float max);
    void	   nb_getStressFilter(float *frac, float *low, float *high, float *s, float *max) const {
    	*low = lowLoad; *high = highLoad; *max = maxStress; *frac = frameFrac;
	*s = stressParam;
    }
    
    void	   nb_setStress(float stress);
    float	   nb_getStress() const {
	return stressLevel;
    }
    float	   nb_getLoad() const {
	return curLoad;
    }
    
    // Current scene access methods.
    void	   nb_setScene(pfScene *s);
    pfScene*	   nb_getScene() const;
    
    void	   nb_setESky(pfEarthSky *_es);
    pfEarthSky*	   nb_getESky() const;
    
    void 	   nb_setGState(pfGeoState *_gstate);
    pfGeoState* nb_getGState() const {
	return gstate;
    }
    void 	   nb_setGStateTable(pfList *list);
    pfList*	   nb_getGStateTable() const {
	return gsTable;
    }
    
    void	   nb_setLODAttr(int  attr, float val);
    float	   nb_getLODAttr(int  attr) const;
    
    void	   nb_setLODState(const pfLODState *ls);
    void 	   nb_getLODState(pfLODState *ls) const;
    
    void 	   nb_setLODStateList(pfList *stateList);
    pfList*	   nb_getLODStateList() const { return lodStateList; }
    
    int 	   nb_setStatsMode(uint  _mode, uint  _val);    
    pfFrameStats*  nb_getFStats();

    void	   nb_setCalligEnable(int _enable);
    int		   nb_getCalligEnable(void) const { return calligEnable; }
    pfCalligraphic *nb_getCurCallig(void);

    void	   nb_setBinSort(int bin, int sortType, uint64_t *sortOrders);
    int		   nb_getBinSort(int bin, uint64_t *sortOrders);
    void	   nb_setBinSortPriority(int bin, int priority);
    int		   nb_getBinSortPriority(int bin) const {
	return (*bins)[bin].sortOrdersPriority;
    }

    void	   nb_setBinOrder(int bin, int order);
    int		   nb_getBinOrder(int bin) const {
	return binOrders->get(bin);

    }
    void	   nb_setBinChildOrderMask(int bin, uint64_t orderMask);
    uint64_t	   nb_getBinChildOrderMask(int bin) const {
	return (*bins)[bin].childOrderMask;
    }

    void	   nb_setBinFlags(int bin, int flags);
    int 	   nb_getBinFlags(int bin) const {
	return (*bins)[bin].flags;
    }

    void	   nb_setBinCallBack(int bin, int type, pfDListFuncType func);
    pfDListFuncType nb_getBinCallBack(int bin, int type);
    void           nb_setBinUserData(int bin, void *userData, int size);
    void           *nb_getBinUserData(int bin, int *size);

    int		   nb_getFreeBin(void) {
	// do not return a number lower that PFSORT_LPSTATE_BIN
	// to avoid saving a Database with a bin == LPSTATE_BIN
 	// and reloading it on a system with LPOINT process
	return PF_MAX2(bins->getNum(),PFSORT_LPSTATE_BIN+1);
    }
    int            nb_findSubBin(int bin1, int bin2, int create);
    int            nb_findBinParent(int bin, int lastKnownParent);
    
    pfCullProgram *nb_getCullProgram() {
	return cullPg;
    }





    pfCteChanData* pf_getCteData() 
    {
	return cte;
    }


    




PFINTERNAL:		// class specific methods
    void           nb_addBinChild(int child, int root, uint64_t rootMask);
    int            nb_isSubbinOf(int bin1, int bin2);
    //CAPI:private
    int            nb_findBinByMask(uint64_t binParentsMask, _pfUint64List *binParents, uint64_t childOrderMask, int sortOderParent, int create);
    void          pfInitBinCallbacks(uint64_t *parentsMask, _pfUint64List **parents, int bin);
    void          pfOpenBinCallbacks(uint64_t *parentsMask, _pfUint64List *parents, int child);
    void          pfCloseBinCallbacks(uint64_t *parentsMask, _pfUint64List *parents, int child);
    //CAPI:public
    //CAPI:verb
    int 	   nb_attach(pfChannel *_chan1);
    int 	   nb_detach(pfChannel *_chan1);

    int 	   nb_ASDattach(pfChannel *_chan1);
    int            nb_ASDdetach(pfChannel *_chan1);

    //CAPI:verb PassChanData
    void	   nb_passChanData();
    
    //CAPI:verb ChanPick
    int		   nb_pick(int mode, float px, float py, float radius, pfHit **pickList[]);

    void	   nb_clear();

    //CAPI:verb DrawChanStats
    void	   nb_drawStats();

    //CAPI:verb ChanNodeIsectSegs
    int 	   nb_isect(pfNode *node, pfSegSet *segSet, pfHit **hits[], pfMatrix *ma);

    void 	   nb_computeMatrices(pfMatrix *model, pfMatrix *proj) { pf_computeViewingMatrix(*model, *proj); }

    // Compute OpenGL viewing matrix without calling OpenGL.

    void	   pf_computeViewingMatrix(pfMatrix& modelview, pfMatrix& projection);
    


private:
friend class pfLOD;
friend class pfASD;
friend class pfEarthSky;
friend class _pfCuller;

    uint 		flags;
    
    // Pipe which channel draws on. This is set at creation time and 
    // cannot change during run time.
    _pfPipeRef		parentPipe;
    
    // current pfPipeWindow for drawing output
    _pfPipeWindowRef	pipeWin;
    
    // video channel for directing framebuffer output
    int			pVChanIndex;
    int			projMode;
    float		pixScale;
    float		minPixScale, maxPixScale;
    
    //////////////////// View ////////////////////////

#ifdef EXTRAP_VIEW
    double		viewTime, prevViewTime;
    pfMatrix	    	prevNomViewMat;	// Previous frame's nominal view 
#endif

    // The viewpoint/view direction of channel.
    pfCoord		nomViewCoord;
    pfMatrix	    	nomViewMat; 	// Nominal view matrix w/o offsets.
    pfMatrix	    	viewMat;	// View matrix with offsets.
    pfVec3		lodViewPoint;	// POV from which LODs are computed
    
    // Translational and rotational offsets of viewing frustum from view 
    // direction. Rotational offsets are measured in degrees. Translational
    // and rotational offsets are encoded in the viewing matrix. (However,
    // rotational *should* be in the projection but GL fog does not allow this).
    pfCoord		viewOffset;
    pfMatrix		offsetMat;
    pfVec3		offsetViewVec;	// View vec with rot offsets
    
    //////////////////// Frustum ////////////////////////

    // Viewing frustum used for clipping. nomViewFrust is nominal and 
    // viewFrust is nomViewFrust transformed by viewing transformation.
    pfFrustum		*nomViewFrust, *viewFrust;
    pfFrustum		*userViewFrust;
    
    // Volume used for culling if PFCHAN_USE_CULL_VOL is set in 'flags'
    pfPolytope		*nomCullVol, *cullVol;

    // cull program
    pfCullProgram       *cullPg;

    // Field of view in horizontal and vertical directions in degrees.
    float		fovHoriz, fovVert;
    
    // Specifies which FOV direction to automatically match to aspect ratio
    // of viewport.
    int 		autoAspect;
    
    // Distance, in pixels, to the near plane - required for lightpoints
    float		nearPixDist;
    
    //////////////////// Viewport ////////////////////////

    // Fractional viewport >=0. &  <=1.
    // displayed viewport
    float		vpLeft, vpRight, vpBottom, vpTop, vpXSize, vpYSize;
    // drawn viewport (may be different than displayed if DVR)
    float		vpOutLeft, vpOutRight, vpOutBottom, vpOutTop;
    float		vpOutXSize, vpOutYSize;
    
    //////////////////// Light Sources ////////////////////////

    // Only draw process pfChannel has light list. This is required
    // to multibuffer pfLights.
    pfLight		*lightList[PF_MAX_LIGHTS];	
    int 		numLights;	// Num lights used in current traversal
    _pfLSourceStateList 	projList;
    
    // Structures required for projected textures and shadows
    pfTexEnv   		*modEnv, *alphaEnv;
    
    //////////////////// LOD ////////////////////////

    // LOD variables. '2' indicates value is stored as squared value.
    float		lodScale;	// Overall scale
    float		lodAspect2;	// FOV mods
    float		lodFrustScale;	// lodAspect2 modifier
    float		lodMult2;	// Overall scale + FOV mods
    float		lodStress, lodStress2;	// Stress modification
    float		lodStressPix2;	// Max pixel size for stress mod
    float		lodPixConst2;	// Needed for computing LOD pix size.
    float		lodFade, lodFade2, invLodFade;	// Fade range
    pfLODState		*lodState;	// Global LODState

    // Table for indexed LODStates
    pfList 		*lodStateList;

    //////////////////// Load/Stress ////////////////////////

    float	curLoad;	// Most recent frame's load value.
    float	prevLoad;	// Previous frame's load value.
    float	lowLoad;   	// Load below which stress is decreased.
    float	highLoad;   	// Load above which stress is increased.
    float	maxStress;	// Maximum value stress may reach.
    float	frameFrac;	// Fraction of frame channel should occupy.
    float	stressParam;    // Stress multiplier.
    float	stressLevel; 	// Stress value computed from load.
    int 	stressFlag; 	// 1 = stress is explicitly; don't use filter
    int 	stressFrameStamp;// Frame when stress was last computed
    
    //////////////////// Share Group ////////////////////////
    
    // List of channels in channel group. Does NOT include 'this'.
    _pfChannelList	groupList;
    _pfChannelList	ASDGroupList;
    
    // Bitfield mask of attributes which are shared/not shared by channels
    // in a channel group. 1 = shared.
    uint 		shareMask;    
    
    //////////////////// Traversal Stuff ////////////////////////

    // masks for cull and nb_draw(isection unused)
    uint 		*travMasks;
    int			*travModes;	// Traversal modes.

    int			forceNoAppTrav;     // If on, must not run APP 
					    // traversal (because we are a 
					    // slave channel and the master
					    // channel calls it).
    
    // cullFunc and drawFunc are user's cull/draw routines which are 
    // called by Performer's cull/draw processes. passThru is 
    // user-specified data which is passed first to the cull then to 
    // the draw function.
    pfChanFuncType	*travFuncs;
    
    pfGeoState		*gstate;	// Global state for channel
    pfList		*gsTable;	// Table for indexed GeoStates

    // Cull traversers associated with this channel
    _pfCuller		**cullTrav;
    
    // Display list for light sources and non-binned scene geometry
    pfDispList		*lightDList, *sceneDList;
    // Display list for storing lpoint info from cull to give to LPoint proc & Draw
    pfDispList		*lpointDrawDList; 
    int			doLPointDL;

    //////////////////// Passthrough Data ////////////////////////

    void		*passThru;
    size_t 		passThruSize;
    int 		passFrameStamp;	// Frame stamp of passthrough data
    
    ///////////////////// Statistics ///////////////////////

    // Profiling timing data
    int 		appFrame;	// Frame of application process
    int 		profFlag;	// 1 == draw profile data
    uint 		profDrawMode;	// fields of stats data to display
    // vars for compute load
    float		drawTime;	// save elapsed draw time for stress calc
    float		pipeTime;	// save elapsed gfx pipe time for stress calc
    float		geomTime;	// save elapsed gfx pipe geom time for stress calc
    float		fillTime;	// save elapsed gfx pipe fill time for stress calc
    float		startPFDraw, endPFDraw; // save pfDraw times to pass to stats
    float		*stressRecord;
    float		*loadRecord;
    pfDispList		*prevDL;
    _pfFrameStatsRef 	stats, statsDSP, dftStats;
    
    pfChanUpBuffer  	**upstreamBuffer;
    
    
    ///////////////////// Sorting ///////////////////////

    _pfHashTable        *binHash;       // hash table used to find subbins 
                                        // based on a mask of parents 
    _pfBinList          *bins;          // list of bins
    _pfVoidList         *binSortOrders; // Sorting hierarchy for each draw bin
    _pfIntList          *binOrders;     // List of bin drawing orders
    _pfIntList          *orderedBins;   // Sorted ordering of draw bins
    _pfIntList          *orderedSubBins;// Sorted ordering of subbins
    _pfVoidList		binDLists;	// List of pfDispLists for draw bins
    _pfUint64List       *tempList;      // kept to avoid allocating a new list

    ///////////////////// Scene ///////////////////////

    _pfEarthSkyRef	earthSky;	// Screen clear structure
    _pfSceneRef		currentScene;	// Current scene to cull/draw.

    ///////////////////// Calligraphics ///////////////////////

    pfCalligraphic	*callig;	// overrides a callig on the video channel
    int			calligEnable;	// enable for calligraphics on this chan


    ///////////////////// Emulated ClipTextures ///////////////////////

    pfCteChanData*	cte;

    ///////////////////// Video Composition hardware support ///////////

    pfCompositor 	*compositor;
    uint32_t             compositorChanIndex;
    uint32_t             isCompositorMasterChannel;

    /////////////////////////////////////////////////////////////////////
    
private:
    static pfType *classType;
public:
    void *_pfChannelExtraData;
};

#endif    // !__PF_CHANNEL_H__

