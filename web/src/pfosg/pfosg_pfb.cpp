/* pfosg_pfb.cpp - real implementations backing SGI's pfpfb.c loader (the
 * shipped .pfb/.pfa reader, compiled unmodified): the node types and object
 * attributes the demo databases actually use.  The long tail of exotica
 * (fluxes, engines, image caches, IBR, ASD...) is generated stubs in
 * pfosg_pfb_stubs.cpp. */

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#include "pfosg_internal.h"

#include <osg/Billboard>
#include <osg/PolygonOffset>
#include <osg/Sequence>
#include <osg/Switch>

#include <cstdlib>
#include <cstring>

static PfOsgState& S = pfosgState;

static pfNode* wrapNode(osg::Node* n)
{
    S.keep.push_back(n);
    return (pfNode*)n;
}

/* ---- class-type cookies ------------------------------------------------ */

static int bboardTypeCookie, seqTypeCookie, layerTypeCookie, gsetTypeCookie;

extern "C" pfType* pfGetBboardClassType(void) { return (pfType*)&bboardTypeCookie; }
extern "C" pfType* pfGetSeqClassType(void)    { return (pfType*)&seqTypeCookie; }
extern "C" pfType* pfGetLayerClassType(void)  { return (pfType*)&layerTypeCookie; }
extern "C" pfType* pfGetGSetClassType(void)   { return (pfType*)&gsetTypeCookie; }

/* ---- billboards ---------------------------------------------------------
 * Performer sets billboard positions before its geosets; OSG ties a position
 * to each drawable index.  Buffer the positions and apply them as drawables
 * (geosets) are added. */

struct PfOsgBillboard : public osg::Billboard {
    std::vector<osg::Vec3> pending;
    void setPendingPos(unsigned i, const osg::Vec3& p)
    {
        if (pending.size() <= i) pending.resize(i + 1);
        pending[i] = p;
    }
    bool addDrawable(osg::Drawable* d) override
    {
        unsigned idx = getNumDrawables();
        bool r = osg::Billboard::addDrawable(d);
        if (r && idx < pending.size())
            osg::Billboard::setPosition(idx, pending[idx]);
        return r;
    }
};

extern "C" pfBillboard* pfNewBboard(void)
{
    PfOsgBillboard* bb = new PfOsgBillboard;
    bb->setMode(osg::Billboard::POINT_ROT_EYE);
    return (pfBillboard*)wrapNode(bb);
}

extern "C" void pfBboardMode(pfBillboard* b, int mode, int val)
{
    osg::Billboard* bb = dynamic_cast<osg::Billboard*>((osg::Node*)b);
    if (!bb || mode != PFBB_ROT) return;
    switch (val) {
    case PFBB_AXIAL_ROT:       bb->setMode(osg::Billboard::AXIAL_ROT); break;
    case PFBB_POINT_ROT_EYE:   bb->setMode(osg::Billboard::POINT_ROT_EYE); break;
    case PFBB_POINT_ROT_WORLD: bb->setMode(osg::Billboard::POINT_ROT_WORLD); break;
    }
}

extern "C" void pfBboardAxis(pfBillboard* b, const pfVec3 axis)
{
    osg::Billboard* bb = dynamic_cast<osg::Billboard*>((osg::Node*)b);
    if (bb) bb->setAxis(osg::Vec3(axis[0], axis[1], axis[2]));
}

extern "C" void pfBboardPos(pfBillboard* b, int i, const pfVec3 pos)
{
    PfOsgBillboard* bb = dynamic_cast<PfOsgBillboard*>((osg::Node*)b);
    if (bb && i >= 0)
        bb->setPendingPos((unsigned)i, osg::Vec3(pos[0], pos[1], pos[2]));
}

/* ---- sequences ---------------------------------------------------------- */

/* The pfb loader sets per-frame times (pfSeqTime) BEFORE attaching the
 * children; osg::Sequence::addChild would insert a default time ahead of
 * them, orphaning the authored durations (every stoplight state would run
 * the 1s default).  When times are already loaded, attach children without
 * touching the frame-time list so child i keeps its authored time i. */
struct PfOsgSequence : public osg::Sequence {
    bool addChild(osg::Node* child) override
    {
        if (_frameTime.size() > getNumChildren())
            return osg::Group::addChild(child);
        return osg::Sequence::addChild(child);
    }
};

extern "C" pfSequence* pfNewSeq(void)
{
    osg::Sequence* sq = new PfOsgSequence;
    sq->setDefaultTime(1.0f);
    return (pfSequence*)wrapNode(sq);
}

static osg::Sequence* asSeq(void* s)
{
    return dynamic_cast<osg::Sequence*>((osg::Node*)s);
}

extern "C" void pfSeqDuration(pfSequence* s, float speed, int nReps)
{
    if (asSeq(s)) asSeq(s)->setDuration(speed, nReps);
}

extern "C" void pfSeqInterval(pfSequence* s, int imode, int beg, int e)
{
    if (asSeq(s))
        asSeq(s)->setInterval(imode == PFSEQ_SWING
                                  ? osg::Sequence::SWING
                                  : osg::Sequence::LOOP, beg, e);
}

extern "C" void pfSeqMode(pfSequence* s, int m)
{
    if (!asSeq(s)) return;
    switch (m) {
    case PFSEQ_START:  asSeq(s)->setMode(osg::Sequence::START); break;
    case PFSEQ_STOP:   asSeq(s)->setMode(osg::Sequence::STOP); break;
    case PFSEQ_PAUSE:  asSeq(s)->setMode(osg::Sequence::PAUSE); break;
    case PFSEQ_RESUME: asSeq(s)->setMode(osg::Sequence::RESUME); break;
    }
}

extern "C" void pfSeqTime(pfSequence* s, int index, double time)
{
    if (!asSeq(s)) return;
    if (index < 0)
        asSeq(s)->setDefaultTime((float)time);
    else
        asSeq(s)->setTime((unsigned)index, (float)time);
}

/* ---- light-point state (sizing only; see pfosg_internal.h) -------------- */

extern "C" pfLPointState* pfNewLPState(void*)
{
    return (pfLPointState*)new PfOsgLPState;
}

extern "C" void pfLPStateVal(pfLPointState* lps, int attr, float val)
{
    PfOsgLPState* s = (PfOsgLPState*)lps;
    if (!s) return;
    switch (attr) {
    case 100 /*PFLPS_SIZE_MIN_PIXEL*/: s->sizeMinPixel = val; break;
    case 101 /*PFLPS_SIZE_ACTUAL*/:    s->sizeActual = val; break;
    case 102 /*PFLPS_SIZE_MAX_PIXEL*/: s->sizeMaxPixel = val; break;
    }
}

extern "C" void pfLPStateMode(pfLPointState* lps, int mode, int val)
{
    PfOsgLPState* s = (PfOsgLPState*)lps;
    if (s && mode == 10 /*PFLPS_SIZE_MODE*/) s->sizeMode = val;
}

/* ---- layers (decals) ----------------------------------------------------
 * child 0 is the base surface; later children are coplanar decals drawn
 * with increasing polygon offset, as in the pfb2osg loader. */

struct PfOsgLayer : public osg::Group {
    bool addChild(osg::Node* child) override
    {
        unsigned idx = getNumChildren();
        if (idx > 0 && child)
            child->getOrCreateStateSet()->setAttributeAndModes(
                new osg::PolygonOffset(-1.0f * idx, -2.0f * idx));
        return osg::Group::addChild(child);
    }
};

extern "C" pfLayer* pfNewLayer(void)
{
    return (pfLayer*)wrapNode(new PfOsgLayer);
}

extern "C" void pfLayerMode(pfLayer*, int) {}   /* PFDECAL_*: offset is fine */

/* ---- switches ------------------------------------------------------------ */

extern "C" int pfSwitchVal(pfSwitch* sw, float val)
{
    osg::Switch* s = dynamic_cast<osg::Switch*>((osg::Node*)sw);
    if (!s) return 0;
    if (val == (float)PFSWITCH_ON)
        s->setAllChildrenOn();
    else if (val == (float)PFSWITCH_OFF)
        s->setAllChildrenOff();
    else {
        int i = (int)val;
        s->setAllChildrenOff();
        if (i >= 0 && i < (int)s->getNumChildren())
            s->setSingleChildOn((unsigned)i);
    }
    return 1;
}

extern "C" pfFlux* pfGetSwitchValFlux(const pfSwitch*) { return nullptr; }

/* ---- memory / refcounts (heap-backed, never reclaimed) ------------------ */

extern "C" int pfRef(void*) { return 1; }
extern "C" int pfUnref(void*) { return 1; }
extern "C" int pfGetRef(const void*) { return 1; }
extern "C" void* pfGetArena(void*) { return nullptr; }
extern "C" void* pfRealloc(void* data, size_t nbytes)
{
    return realloc(data, nbytes);
}

extern "C" unsigned int pfGetNodeTravMask(const pfNode* node, int which)
{
    if (which == PFTRAV_CULL && node)
        return ((const osg::Node*)node)->getNodeMask() ? 0xffffffff : 0;
    return 0xffffffff;
}

/* ---- gset draw attributes ------------------------------------------------ */

extern "C" int* pfGetGSetPrimLengths(const pfGeoSet* g)
{
    return g ? ((const PfOsgGSet*)g)->lengths : nullptr;
}

extern "C" int pfGetGSetAttrBind(const pfGeoSet* g, int attr)
{
    if (!g || attr < 0 || attr > 3) return 0;
    return ((const PfOsgGSet*)g)->attr[attr].binding;
}

extern "C" void pfGSetDrawMode(pfGeoSet* g, int mode, int val)
{
    if (!g) return;
    PfOsgGSet* gs = (PfOsgGSet*)g;
    switch (mode) {
    case PFGS_FLATSHADE:  gs->flatShade = val != 0; break;
    case PFGS_WIREFRAME:  gs->wireframe = val != 0; break;
    case PFGS_COMPILE_GL: gs->compileGL = val != 0; break;
    }
    gs->dirty = true;
}

extern "C" int pfGetGSetDrawMode(const pfGeoSet* g, int mode)
{
    if (!g) return 0;
    const PfOsgGSet* gs = (const PfOsgGSet*)g;
    switch (mode) {
    case PFGS_FLATSHADE:  return gs->flatShade;
    case PFGS_WIREFRAME:  return gs->wireframe;
    case PFGS_COMPILE_GL: return gs->compileGL;
    }
    return 0;
}

extern "C" void pfGSetLineWidth(pfGeoSet* g, float w)
{
    if (g) ((PfOsgGSet*)g)->lineWidth = w;
}

extern "C" float pfGetGSetLineWidth(const pfGeoSet* g)
{
    return g ? ((const PfOsgGSet*)g)->lineWidth : 1.0f;
}

extern "C" void pfGSetPntSize(pfGeoSet* g, float s)
{
    if (g) ((PfOsgGSet*)g)->pntSize = s;
}

extern "C" float pfGetGSetPntSize(const pfGeoSet* g)
{
    return g ? ((const PfOsgGSet*)g)->pntSize : 1.0f;
}

extern "C" void pfGSetIsectMask(pfGeoSet* g, unsigned int mask, int, int)
{
    if (g) ((PfOsgGSet*)g)->isectMask = mask;
}

extern "C" unsigned int pfGetGSetIsectMask(const pfGeoSet* g)
{
    return g ? ((const PfOsgGSet*)g)->isectMask : 0xffffffff;
}

/* ---- texture attributes -------------------------------------------------- */

static osg::Texture::WrapMode wrapModeOf(int type)
{
    return type == PFTEX_CLAMP ? osg::Texture::CLAMP_TO_EDGE
                               : osg::Texture::REPEAT;
}

extern "C" void pfTexRepeat(pfTexture* t, int wrap, int type)
{
    PfOsgTex* tx = (PfOsgTex*)t;
    if (!tx || !tx->tex) return;
    if (wrap == PFTEX_WRAP || wrap == PFTEX_WRAP_S) {
        tx->repeatS = type;
        tx->tex->setWrap(osg::Texture::WRAP_S, wrapModeOf(type));
    }
    if (wrap == PFTEX_WRAP || wrap == PFTEX_WRAP_T) {
        tx->repeatT = type;
        tx->tex->setWrap(osg::Texture::WRAP_T, wrapModeOf(type));
    }
}

extern "C" int pfGetTexRepeat(const pfTexture* t, int wrap)
{
    const PfOsgTex* tx = (const PfOsgTex*)t;
    if (!tx) return 0;
    return wrap == PFTEX_WRAP_T ? tx->repeatT : tx->repeatS;
}

extern "C" void pfTexFormat(pfTexture* t, int format, int type)
{
    PfOsgTex* tx = (PfOsgTex*)t;
    if (!tx) return;
    switch (format) {
    case PFTEX_INTERNAL_FORMAT: tx->intFormat = type; break;
    case PFTEX_EXTERNAL_FORMAT: tx->extFormat = type; break;
    case PFTEX_IMAGE_FORMAT:    tx->imgFormat = type; break;
    }
}

extern "C" int pfGetTexFormat(const pfTexture* t, int format)
{
    const PfOsgTex* tx = (const PfOsgTex*)t;
    if (!tx) return 0;
    switch (format) {
    case PFTEX_INTERNAL_FORMAT: return tx->intFormat;
    case PFTEX_EXTERNAL_FORMAT: return tx->extFormat;
    case PFTEX_IMAGE_FORMAT:    return tx->imgFormat;
    }
    return 0;
}

extern "C" void pfTexImage(pfTexture* t, unsigned int* image, int comp,
                           int sx, int sy, int sz)
{
    PfOsgTex* tx = (PfOsgTex*)t;
    if (!tx || !tx->tex) return;
    tx->imgData = image;
    tx->comp = comp; tx->sx = sx; tx->sy = sy; tx->sz = sz;
    if (!image || sx <= 0 || sy <= 0 || comp < 1 || comp > 4) return;
    GLenum fmt = comp == 4 ? GL_RGBA : comp == 3 ? GL_RGB
                : comp == 2 ? GL_LUMINANCE_ALPHA : GL_LUMINANCE;
    osg::Image* img = new osg::Image;
    unsigned char* copy = new unsigned char[(size_t)sx * sy * comp];
    memcpy(copy, image, (size_t)sx * sy * comp);
    img->setImage(sx, sy, 1, comp, fmt, GL_UNSIGNED_BYTE, copy,
                  osg::Image::USE_NEW_DELETE);
    tx->img = img;
    tx->tex->setImage(img);
}

/* pfGetTexImage: real (osg::Image-backed) in pfosg.cpp */

/* ---- switches / text / frustums ------------------------------------------ */

extern "C" pfSwitch* pfNewSwitch(void)
{
    osg::Switch* sw = new osg::Switch;
    sw->setAllChildrenOn();
    return (pfSwitch*)wrapNode(sw);
}

extern "C" pfText* pfNewText(void)
{
    /* 3D text nodes: not rendered; keep graph structure intact */
    osg::Group* gp = new osg::Group;
    gp->setName("pfText");
    return (pfText*)wrapNode(gp);
}

extern "C" pfType* pfGetFrustClassType(void)
{
    static int cookie;
    return (pfType*)&cookie;
}

extern "C" void pfMakeOrthoFrust(pfFrustum*, float, float, float, float) {}
extern "C" void pfOrthoXformFrust(pfFrustum*, const pfFrustum*,
                                  const pfMatrix) {}

/* ---- pfd share registry / user funcs / exotica ----------------------------
 * No object sharing: every loaded object is unique, which only costs
 * memory.  Custom user-function DSOs don't exist here. */

extern "C" pfdShare* pfdGetGlobalShare(void)
{
    static int cookie;
    return (pfdShare*)&cookie;
}

extern "C" int pfdAddSharedObject(pfdShare*, pfObject*) { return 0; }
extern "C" pfObject* pfdFindSharedObject(pfdShare*, pfObject*)
{
    return nullptr;
}
extern "C" pfList* pfdGetSharedList(pfdShare*, pfType*) { return nullptr; }
extern "C" int pfdCleanShare(pfdShare*) { return 0; }

extern "C" int pfdRegisterUserFunc(void*, const char*, const char*)
{
    return 0;
}
extern "C" void* pfdFindRegisteredUserFunc(char*) { return nullptr; }
extern "C" int pfdIsRegisteredUserFunc(void*) { return 0; }
extern "C" int pfdGetRegisteredUserFunc(void*, char**, char**) { return 0; }

extern "C" void pfdASDClipring(pfASD*, int) {}
extern "C" char* pfdGetAppearanceFilename(islAppearance*) { return nullptr; }
extern "C" islAppearance* pfdLoadAppearance(const char*) { return nullptr; }
extern "C" pfClipTexture* pfdLoadClipTexture(const char*) { return nullptr; }

/* ---- materials / lights / lmodel: accepted, not fed to the renderer ------ */

extern "C" void pfMtlShininess(pfMaterial*, float) {}
extern "C" float pfGetMtlShininess(pfMaterial*) { return 0.0f; }
extern "C" void pfMtlAlpha(pfMaterial*, float) {}
extern "C" float pfGetMtlAlpha(pfMaterial*) { return 1.0f; }
extern "C" void pfMtlSide(pfMaterial*, int) {}
extern "C" void pfLModelTwoSide(pfLightModel*, int) {}
extern "C" void pfGetLightPos(const pfLight*, float* x, float* y, float* z,
                              float* w)
{
    if (x) *x = 0; if (y) *y = 0; if (z) *z = 1; if (w) *w = 0;
}
extern "C" void pfLightAtten(pfLight*, float, float, float) {}
