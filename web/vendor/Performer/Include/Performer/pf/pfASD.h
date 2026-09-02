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
// pfASD.h
//
// $Revision: 1.79 $
// $Date: 2002/05/30 00:33:56 $
//
//
#ifndef __PF_ASD_H__
#define __PF_ASD_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfLists.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pf/pfLODState.h>
#include <Performer/pr/pfFlux.h>
#include <Performer/pr/pfClipTexture.h>


class pfBuffer;

extern "C" {     // EXPORT to CAPI

#define PFASD_MAXCHANNEL	10
#define PFASD_MAXCHANNELID	100
#define PFASD_MAXLOD		50

/* evalMethod */
#define PFASD_DIST		0x1
#define PFASD_DIST_SQUARE	0x2
#define PFASD_CALLBACK		0x3

/* setMode Tokens */
#define PFASD_TRAV_VERTEX  	0x1

/* setMode Values */
#define PFASD_NO_MORPH		0x1
#define PFASD_C_MORPH		0x0

#define PFASD_FACE_NODRAW	0x1
#define PFASD_FACE_NOQUERY	0x2

/* setAttr Tokens */
/* _attr */
#define PFASD_LODS		0x01
#define PFASD_COORDS		0x02
#define PFASD_NORMALS		0x04
#define PFASD_COLORS		0x08
#define PFASD_TCOORDS		0x10
#define PFASD_FACES		0x20
#define PFASD_OVERALL_ATTR	0x40
#define PFASD_PER_VERTEX_ATTR	0x80
#define PFASD_NO_ATTR		0

/* setDrawCode Tokens */
#define PFASD_BBOX		0x4
#define PFASD_STATS		0x8

#define PFASD_NIL_ID		-1

} // extern "C" END of C include export


typedef struct
{
    int	dummy;
} pfASDExtra;

typedef struct pfASDEnlarge
{
  /* Renamed these from near and far to nearPlane and farPlane due to win32 */
  float nearPlane, farPlane, fov;
} pfASDEnlarge;

class pfBuffer;

#define PFASD ((pfASD*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFASDBUFFER ((pfASD*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfASD : public pfNode
{
public:

    void setAttr(int _which, int _type, int _size, void *_attr)  {
        PFASD->nb_setAttr(_which, _type, _size, _attr);
    }

    void getAttr(int _which, int *_type, int *_size, void **_attr)  {
        PFASD->nb_getAttr(_which, _type, _size, _attr);
    }

    void setMorphAttrs(int _mc)  {
        PFASD->nb_setMorphAttrs(_mc);
    }

    int getMorphAttrs()  {
        return PFASD->nb_getMorphAttrs();
    }

    void getActiveGeom(pfChannel *_chan, pfList *_geom)  {
        PFASD->nb_getActiveGeom(_chan, _geom);
    }

    void setNumBaseFaces(int num)  {
        PFASD->nb_setNumBaseFaces(num);
    }

    int getNumBaseFaces()  {
        return PFASD->nb_getNumBaseFaces();
    }

    void setGStates(pfGeoState **gs, int num)  {
        PFASD->nb_setGStates(gs, num);
    }

    void getGStates(pfGeoState ***gs, int *num)  {
        PFASD->nb_getGStates(gs, num);
    }

    pfGeoState* getGState(int num)  {
        return PFASD->nb_getGState(num);
    }

    int getNumGStates()  {
        return PFASD->nb_getNumGStates();
    }

    void setSyncGroup(uint _syncGroup)  {
        PFASD->nb_setSyncGroup(_syncGroup);
    }

    uint getSyncGroup()  {
        return PFASD->nb_getSyncGroup();
    }

    void enableClipRings()  {
        PFASD->nb_enableClipRings();
    }

    void setNumClipRings(int _numrings)  {
        PFASD->nb_setNumClipRings(_numrings);
    }

    void setClipRings(float *_rings)  {
        PFASD->nb_setClipRings(_rings);
    }

    float* getClipRings()  {
        return PFASD->nb_getClipRings();
    }

    int getNumClipRings()  {
        return PFASD->nb_getNumClipRings();
    }

    void setFaceBBoxes(pfBox *_box)  {
        PFASD->nb_setFaceBBoxes(_box);
    }

    pfBox* getFaceBBoxes()  {
        return PFASD->nb_getFaceBBoxes();
    }

    void setFaceBBox(pfBox *_facebbox, int _faceid)  {
        PFASD->nb_setFaceBBox(_facebbox, _faceid);
    }

    void getFaceBBox(pfBox *_facebbox, int _faceid)  {
        PFASD->nb_getFaceBBox(_facebbox, _faceid);
    }

    void setBBox(pfBox *_box)  {
        PFASD->nb_setBBox(_box);
    }

    void getBBox(pfBox *_box)  {
        PFASD->nb_getBBox(_box);
    }

    void config()  {
        PFASD->nb_config();
    }

    void setMaxMorphDepth(int _m, float __morphweightconstraint)  {
        PFASD->nb_setMaxMorphDepth(_m, __morphweightconstraint);
    }

    void getMaxMorphDepth(int *_m, float *__morphweightconstraint)  {
        PFASD->nb_getMaxMorphDepth(_m, __morphweightconstraint);
    }

    void setLODState(pfLODState *ls)  {
        PFASD->nb_setLODState(ls);
    }

    pfLODState* getLODState()  {
        return PFASD->nb_getLODState();
    }

    void setLODStateIndex(int _lsi)  {
        PFASD->nb_setLODStateIndex(_lsi);
    }

    int getLODStateIndex()  {
        return PFASD->nb_getLODStateIndex();
    }

    void setEvalMethod(int method)  {
        PFASD->nb_setEvalMethod(method);
    }

    int getEvalMethod()  {
        return PFASD->nb_getEvalMethod();
    }

    void setEvalFunc(pfASDEvalFuncType _eval)  {
        PFASD->nb_setEvalFunc(_eval);
    }

    pfASDEvalFuncType getEvalFunc()  {
        return PFASD->nb_getEvalFunc();
    }

    void setMask(uint _which, uint _mask, int _id)  {
        PFASD->nb_setMask(_which, _mask, _id);
    }

    void setCullEnlarge(float fov, float nearPlane, float farPlane)  {
        PFASD->nb_setCullEnlarge(fov, nearPlane, farPlane);
    }

    void setMorphWeight(int _vertid, float _morphweight)  {
        PFASD->nb_setMorphWeight(_vertid, _morphweight);
    }

    void unsetMorphWeight(int _vertid)  {
        PFASD->nb_unsetMorphWeight(_vertid);
    }

    void initMask(uint _which)  {
        PFASD->nb_initMask(_which);
    }

    void clearAllMasks(uint _which)  {
        PFASD->nb_clearAllMasks(_which);
    }

    void setCalcVirtualClipTexParamsFunc(pfASDCalcVirtualClipTexParamsFuncType _func)  {
        PFASD->nb_setCalcVirtualClipTexParamsFunc(_func);
    }

    pfASDCalcVirtualClipTexParamsFuncType getCalcVirtualClipTexParamsFunc()  {
        return PFASD->nb_getCalcVirtualClipTexParamsFunc();
    }

    int isPaging()  {
        return PFASD->nb_isPaging();
    }

    int isPageMaster()  {
        return PFASD->nb_isPageMaster();
    }

    void initPaging()  {
        PFASD->nb_initPaging();
    }

    void setTileSize(float **_tsize)  {
        PFASD->nb_setTileSize(_tsize);
    }

    float** getTileSize()  {
        return PFASD->nb_getTileSize();
    }

    void setPageSize(short **_page)  {
        PFASD->nb_setPageSize(_page);
    }

    short** getPageSize()  {
        return PFASD->nb_getPageSize();
    }

    void setTotalTiles(short **_tilenum)  {
        PFASD->nb_setTotalTiles(_tilenum);
    }

    short** getTotalTiles()  {
        return PFASD->nb_getTotalTiles();
    }

    void setMaxTileMemSize(int _tilefaces, int _tileverts)  {
        PFASD->nb_setMaxTileMemSize(_tilefaces, _tileverts);
    }

    void getMaxTileMemSize(int *_tilefaces, int *_tileverts)  {
        PFASD->nb_getMaxTileMemSize(_tilefaces, _tileverts);
    }

    void setOrigin(pfVec3 *_min)  {
        PFASD->nb_setOrigin(_min);
    }

    pfVec3* getOrigin()  {
        return PFASD->nb_getOrigin();
    }

    void setPageFname(char *_fname)  {
        PFASD->nb_setPageFname(_fname);
    }

    char* getPageFname()  {
        return PFASD->nb_getPageFname();
    }

    void overrideViewingParams(pfChannel *chan, pfMatrix& M)  {
        PFASD->nb_overrideViewingParams(chan, M);
    }

    int addQueryArray(float *_vertices, float *_down, int nofVertices, uint _mask, pfFlux *_results)  {
        return PFASD->nb_addQueryArray(_vertices, _down, nofVertices, _mask, _results);
    }

    void deleteQueryArray(int _index)  {
        PFASD->nb_deleteQueryArray(_index);
    }

    void setQueryArrayElement(int _arrayIndex, int _elementIndex, float *_vertex, float *_down)  {
        PFASD->nb_setQueryArrayElement(_arrayIndex, _elementIndex, _vertex, _down);
    }

    unsigned int containsQueryArray(float *_vertices, float *_down, int _nofVertices)  {
        return PFASD->nb_containsQueryArray(_vertices, _down, _nofVertices);
    }

    int addQueryTriangles(float *_v, float *_t, float *_c, int _nofTriangles, float *_base, float *_down, float *_projection, float *_azimuth, unsigned int _opcode, uint _mask, pfFlux *_results)  {
        return PFASD->nb_addQueryTriangles(_v, _t, _c, _nofTriangles, _base, _down, _projection, _azimuth, _opcode, _mask, _results);
    }

    void deleteQueryGeoSet(int _index)  {
        PFASD->nb_deleteQueryGeoSet(_index);
    }

    int addQueryGeoSet(pfGeoSet *gset, float *_down, uint _mask, pfFlux *_results)  {
        return PFASD->nb_addQueryGeoSet(gset, _down, _mask, _results);
    }

    void replaceQueryGeoSet(int index, pfGeoSet *gset, float *_down)  {
        PFASD->nb_replaceQueryGeoSet(index, gset, _down);
    }

    void projectPointFinestPositionNormal(float *_base, float *_down, unsigned int _flags, float *_base_pos, float *_base_normal)  {
        PFASD->nb_projectPointFinestPositionNormal(_base, _down, _flags, _base_pos, _base_normal);
    }

    void projectPointFinestPosition(float *_base, float *_down, unsigned int _flags, float *_base_pos)  {
        PFASD->nb_projectPointFinestPosition(_base, _down, _flags, _base_pos);
    }

    void getQueryArrayPositionSpan(int _index, pfBox *_box)  {
        PFASD->nb_getQueryArrayPositionSpan(_index, _box);
    }

    pfNode* clone()  {
        return PFASD->nb_clone();
    }

    void clean(uint64_t cleanBits, pfNodeCounts *counts)  {
        PFASD->nb_clean(cleanBits, counts);
    }

    int cull(int  mode, int  cullResult, _pfCuller *trav)  {
        return PFASD->nb_cull(mode, cullResult, trav);
    }

    int cullProgram(int  mode, int  cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav)  {
        return PFASD->nb_cullProgram(mode, cullResult, cullPgInfo, trav);
    }

    int flatten(pfTraverser *trav)  {
        return PFASD->nb_flatten(trav);
    }

    int intersect(_pfIsector *isector)  {
        return PFASD->nb_intersect(isector);
    }

    void write(pfTraverser *trav, uint which, uint verbose)  {
        PFASD->nb_write(trav, which, verbose);
    }
public:		// Constructors/Destructors
    //CAPI:basename ASD
    //CAPI:updatable
    //CAPI:newargs
    pfASD();
    virtual ~pfASD();

protected:
    pfASD(pfBuffer *buf);
    pfASD(const pfASD *srcASD, pfBuffer *dstASD);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }
    pfASDCalcVirtualClipTexParamsFuncType CalcVirtualClipTexParamsFunc;
    

public:			// pfMemory virtual functions

    void nb_setAttr(int _which, int _type, int _size, void *_attr);
    void nb_getAttr(int _which, int *_type, int *_size, void **_attr);
    void nb_setMorphAttrs(int _mc);
    int  nb_getMorphAttrs(void);
    void nb_getActiveGeom(pfChannel *_chan, pfList *_geom);

    void nb_setNumBaseFaces(int num);
    int nb_getNumBaseFaces(void);
    void nb_setGStates(pfGeoState **gs, int num);
    void nb_getGStates(pfGeoState ***gs, int *num);
    pfGeoState *nb_getGState(int num);
    int nb_getNumGStates(void);
    
    void nb_setSyncGroup(uint _syncGroup);
    uint nb_getSyncGroup();

    void nb_enableClipRings(void);
    void nb_setNumClipRings(int _numrings);
    void nb_setClipRings(float *_rings);
    float *nb_getClipRings(void);
    int nb_getNumClipRings(void);

    void nb_setFaceBBoxes(pfBox *_box);
    pfBox* nb_getFaceBBoxes(void);
    void nb_setFaceBBox(pfBox *_facebbox, int _faceid);
    void nb_getFaceBBox(pfBox *_facebbox, int _faceid);
    void nb_setBBox(pfBox *_box);
    void nb_getBBox(pfBox *_box);
    void nb_config();

    void nb_setMaxMorphDepth(int _m, float __morphweightconstraint);
    void nb_getMaxMorphDepth(int *_m, float *__morphweightconstraint);

    void nb_setLODState(pfLODState *ls);
    pfLODState *nb_getLODState();
    void nb_setLODStateIndex(int _lsi);
    int nb_getLODStateIndex();

    void nb_setEvalMethod(int method);
    int  nb_getEvalMethod(void);
    void nb_setEvalFunc(pfASDEvalFuncType _eval);
    pfASDEvalFuncType nb_getEvalFunc(void);
    void nb_setMask(uint _which, uint _mask, int _id);
    void nb_setCullEnlarge(float fov, float nearPlane, float farPlane);
    void nb_setMorphWeight(int _vertid, float _morphweight);
    void nb_unsetMorphWeight(int _vertid);
    void nb_initMask(uint _which);
    void nb_clearAllMasks(uint _which);

    void nb_setCalcVirtualClipTexParamsFunc(pfASDCalcVirtualClipTexParamsFuncType _func);
    pfASDCalcVirtualClipTexParamsFuncType nb_getCalcVirtualClipTexParamsFunc(void);

    int nb_isPaging(void);
    int nb_isPageMaster(void);
    void nb_initPaging(void);
    void nb_setTileSize(float **_tsize);
    float** nb_getTileSize(void);
    void nb_setPageSize(short **_page);
    short** nb_getPageSize(void);
    void nb_setTotalTiles(short **_tilenum);
    short** nb_getTotalTiles(void);
    void nb_setMaxTileMemSize(int _tilefaces, int _tileverts);
    void nb_getMaxTileMemSize(int *_tilefaces, int *_tileverts);
    void nb_setOrigin(pfVec3 *_min);
    pfVec3* nb_getOrigin(void);
    void nb_setPageFname(char *_fname);
    char* nb_getPageFname(void);

    void  nb_overrideViewingParams(pfChannel *chan, pfMatrix& M);

    
    //CAPI:basename ASD
    //CAPI:virtual

    virtual int nb_addQueryArray(float *_vertices, float *_down, int nofVertices, uint _mask, pfFlux *_results);
    virtual void nb_deleteQueryArray(int _index);
    virtual void nb_setQueryArrayElement(int _arrayIndex, int _elementIndex, float *_vertex, float *_down);
    virtual unsigned int nb_containsQueryArray(float *_vertices, float *_down, int _nofVertices);

    virtual int nb_addQueryTriangles(float *_v, float *_t, float *_c, int _nofTriangles, float *_base, float *_down, float *_projection, float *_azimuth, unsigned int _opcode, uint _mask, pfFlux *_results);
    virtual void nb_deleteQueryGeoSet(int _index);
    virtual int nb_addQueryGeoSet(pfGeoSet *gset, float *_down, uint _mask, pfFlux *_results);
    virtual void nb_replaceQueryGeoSet(int index, pfGeoSet *gset, float *_down);


    //CAPI:verb ASDProjectPointFinestPositionNormal
    virtual void nb_projectPointFinestPositionNormal(float *_base, float *_down, unsigned int _flags, float *_base_pos, float *_base_normal);

    //CAPI:verb ASDProjectPointFinestPosition
    virtual void nb_projectPointFinestPosition(float *_base, float *_down, unsigned int _flags, float *_base_pos);

    //CAPI:verb ASDGetQueryArrayPositionSpan
    virtual void nb_getQueryArrayPositionSpan(int _index, pfBox *_box);

    //CAPI:noverb

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);

PFINTERNAL:		// pfNode virtual traversals
    virtual int             app(pfTraverser *trav);
    virtual int             ASDtrav(pfTraverser *trav);
    virtual pfNode*	    nb_clone();
    virtual void	    nb_clean(uint64_t cleanBits, pfNodeCounts *counts);
    virtual int 	    nb_cull(int  mode, int  cullResult, _pfCuller *trav);
    virtual int 	    nb_cullProgram(int  mode, int  cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int  	    nb_flatten(pfTraverser *trav);
    virtual int  	    nb_intersect(_pfIsector *isector);
    virtual void 	    nb_write(pfTraverser *trav, uint which, uint verbose);
    

protected:		
    friend void terrainEval(void *data);
    friend void _pfASDFrame(void);
    friend void _pfASDSyncEngine(pfEngine *eng);
    friend void _pfASDLock(void);
    friend void _pfASDUnlock(void);
    
protected:
    // added for use by stats
    friend class pfFrameStats;

    int 		    pf_getDirMode() const;
    static int              pf_discFunc(pfHit *hitPR);

    
private:	// internal
    void		    pf_construct(void);
    void		    pf_copy(const pfASD *asd, int mode);

protected:
    friend class _pfCuller;

private:
    pfGroup		*group;
    int			groupID;

private:
    struct TerrainList 	*tl;
    pfGeoState		**gstates;
    int			numgstates;
    uint		syncGroup;
    pfLODState 		*lodState;
    int			lodStateIndex;
    float		*rings;
    int			numrings;
    pfBox		*ringboxes;
    struct TerrainInfo 	*m_info[2];
    struct pfASDEnlarge 	polyscale;
    int			needfork;
    int			paged_data;
    int			pageMaster;
    int			saved_code;
    struct GSetTable		*reg_gsets;
    pfList		*stack;

    int			app_savframe;
    int			app_chancnt;
    int			maxcid; /* max pid for all channels */
			/* entry is childid in pfGroup whose geometry is the 
			   active mesh for this channel. the index is the 
			   channel's pid */
    int			ctable[PFASD_MAXCHANNELID]; 

    pfFlux		*syncFlux;
    pfEngine		*syncEngine;

    int 		ASDEvalIndex[2][PFASD_MAXCHANNELID];

    /*
     *	Space for binary compatible extensions.
     */
    pfASDExtra		*ASDExtra; 

private:
    static pfType *classType;
};

#endif // !__PF_ASD_H__ 
