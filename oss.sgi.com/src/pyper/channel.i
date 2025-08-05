
// This file contains the public interface from the Performer
// header file pf/pfChannel.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pf/pfChannel.h>
%}

class pfChannel
{
public:

    int getFrustType() const;
    void setAspect(int  which, float xyaspect);
    float getAspect();
    void getFOV(float *fovH, float *fovV) const;
    void setNearFar(float n, float f);
    void getNearFar(float *n, float *f) const;
    void getNear(pfVec3& _ll, pfVec3& _lr, pfVec3& _ul, pfVec3& _ur) const;
    void getFar(pfVec3& _ll, pfVec3& _lr, pfVec3& _ul, pfVec3& _ur) const;
    void getPtope(pfPolytope *dst) const;
    int getEye(pfVec3& eye) const;
    void makePersp(float l, float r, float b, float t);
    void makeOrtho(float l, float r, float b, float t);
    void makeSimple(float fov);
    void orthoXform(pfFrustum *fr, const pfMatrix& mat);
    void apply();
    pfPipe* getPipe() const;
    pfPipeWindow* getPWin();
    int getPWinIndex();
    pfPipeVideoChannel* getPVChan() const;
    pfPipeVideoChannel* getFramePVChan() const;
    void setPWinPVChanIndex(int num);
    int getPWinPVChanIndex() const;
    void setFOV(float fovH, float fovV);
    void setViewport(float l, float r, float b, float t);
    void getViewport(float *l, float *r, float *b, float *t) const;
    void setOutputViewport(float l, float r, float b, float t);
    void getOutputViewport(float *l, float *r, float *b, float *t) const;
    void getOrigin(int  *xo, int  *yo) const;
    void getSize(int  *xs, int  *ys) const;
    void getOutputOrigin(int  *xo, int  *yo) const;
    void getOutputSize(int  *xs, int  *ys) const;
    void setPixScale(float s);
    void setMinPixScale(float min);
    void setMaxPixScale(float max);
    float getPixScale() const;
    float getMinPixScale() const;
    float getMaxPixScale() const;
    void setProjMode(int mode);
    int getProjMode() const;
    void setShare(uint  mask);
    uint getShare() const;
    void setAutoAspect(int  which);
    int getAutoAspect() const;
    void getBaseFrust(pfFrustum *frust) const;
    void setViewOffsets(pfVec3& xyz, pfVec3& hpr);
    void getViewOffsets(pfVec3& xyz, pfVec3& hpr) const;
    void setView(pfVec3& vp, pfVec3& vd);
    void getView(pfVec3& vp, pfVec3& vd);
    void setViewMat(pfMatrix& mat);
    void getViewMat(pfMatrix& mat) const;
    void getOffsetViewMat(pfMatrix& mat) const;
    int getCullPtope(pfPolytope *vol, int space) const;
    void* allocChanData(int size);
    void setChanData(void *data, size_t size);
    void* getChanData() const;
    size_t getChanDataSize() const;
    void setTravFunc(int trav, pfChanFuncType func);
    pfChanFuncType getTravFunc(int trav) const;
    void setTravMode(int  trav, int  mode);
    int getTravMode(int  trav) const;

    void setTravMask(int  which, unsigned int  mask);
    unsigned int getTravMask(int  which) const;

    void setStressFilter(float frac, float low, float high, float s, float max);
    void getStressFilter(float *frac, float *low, float *high, float *s, float *max) const;
    void setStress(float stress);
    float getStress() const;
    float getLoad() const;
    void setScene(pfScene *s);
    pfScene* getScene() const;
    void setESky(pfEarthSky *_es);
    pfEarthSky* getESky() const;
    void setGState(pfGeoState *_gstate);
    pfGeoState* getGState() const;
    void setGStateTable(pfList *list);
    pfList* getGStateTable() const;
    void setLODAttr(int  attr, float val)  ;
    float getLODAttr(int  attr) const  ;
    void setLODState(const pfLODState *ls)  ;
    void getLODState(pfLODState *ls) const  ;
    void setLODStateList(pfList *stateList)  ;
    pfList* getLODStateList() const  ;
    int setStatsMode(unsigned int  _mode, unsigned int _val)  ;
    pfFrameStats* getFStats()  ;
    int attach(pfChannel *_chan1)  ;
    int detach(pfChannel *_chan1)  ;
    int ASDattach(pfChannel *_chan1)  ;
    int ASDdetach(pfChannel *_chan1)  ;
    void passChanData()  ;
    int pick(int mode, float px, float py, float radius, pfHit **pickList[]);
    void clear()  ;
    void drawStats()  ;

    pfChannel(pfPipe *p);
    virtual ~pfChannel();
    
    static pfType* getClassType();
};

