/* ============================================================================
 *  pfosg.cpp - stage-A Performer API shim over OpenSceneGraph + SDL2.
 *
 *  Implements the C-API subset declared in src/pfosg/include/Performer/.
 *  Deliberately throwaway (the native SDL2+GLES2 backend replaces it), so it
 *  is thin: single process (PFMP_* accepted but ignored), one pipe, one
 *  window, one channel.
 *
 *  Env: PFOSG_SCREENSHOT=<file.png> writes frame 30 to disk (verification).
 * ==========================================================================*/
#include <Performer/pf.h>
#include <Performer/pfdu.h>

#include "pfb2osg.h"

#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/Switch>
#include <osg/TexEnv>
#include <osg/TexMat>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>
#include <osgViewer/Viewer>

#include <SDL.h>

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

/* ---- shim object structs -------------------------------------------------- */

struct PfOsgGSet {
    osg::ref_ptr<osg::Geometry> geom;
    int ptype = PFGS_TRIS;
    int nprims = 0;
    int* lengths = nullptr;
    struct Attr {
        int binding = PFGS_OFF;
        const float* data = nullptr;
        const ushort* ilist = nullptr;
    } attr[4];
    bool dirty = true;
};

struct PfOsgTex {
    osg::ref_ptr<osg::Texture2D> tex;
    osg::ref_ptr<osg::Image> img;
};

struct PfOsgESky {
    osg::Vec4 clearColor = osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    bool haveClear = false;
    /* Performer-like defaults so an esky with only ground colors set still
     * clears to a plausible sky */
    osg::Vec4 skyTop  = osg::Vec4(0.28f, 0.42f, 0.72f, 1.0f);
    osg::Vec4 grndFar = osg::Vec4(0.3f, 0.15f, 0.05f, 1.0f);
    osg::Vec4 grndNear = osg::Vec4(0.5f, 0.3f, 0.1f, 1.0f);
    int clearMode = PFES_FAST;
};

struct PfOsgDCS;   /* osg::MatrixTransform subclass, below */
struct PfOsgLOD;   /* osg::Switch subclass, below */

struct PfOsgHit {
    osg::Vec3 point;
    osg::Vec3 normal;
    bool valid = false;
};

/* ---- global state (shared with pfosg_perfly.cpp) --------------------------- */

#include "pfosg_internal.h"

PfOsgState pfosgState;

namespace {

PfOsgState& S = pfosgState;

void fatalIfUninited(const char* fn)
{
    if (!S.inited) {
        fprintf(stderr, "pfosg: %s called before pfInit()\n", fn);
        exit(1);
    }
}

std::string resolveFile(const char* name)
{
    if (!name) return std::string();
    std::vector<std::string> candidates;
    candidates.push_back(name);
    for (const std::string& dir : S.filePath)
        candidates.push_back(dir + "/" + name);
    for (const std::string& c : candidates) {
        FILE* f = fopen(c.c_str(), "rb");
        if (f) { fclose(f); return c; }
    }
    return std::string();
}

void openWindow()
{
    if (S.winOpen) return;
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    S.window = SDL_CreateWindow(S.winName.c_str(),
        S.winX ? S.winX : SDL_WINDOWPOS_CENTERED,
        S.winY ? S.winY : SDL_WINDOWPOS_CENTERED,
        S.winW, S.winH,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    S.glctx = S.window ? SDL_GL_CreateContext(S.window) : nullptr;
    if (!S.glctx) {
        fprintf(stderr, "pfosg: cannot open GL window: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_GL_SetSwapInterval(1);

    int dw = 0, dh = 0;
    SDL_GL_GetDrawableSize(S.window, &dw, &dh);

    S.viewer = new osgViewer::Viewer;
    S.viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    S.gw = S.viewer->setUpViewerAsEmbeddedInWindow(0, 0, dw, dh);

    /* sun, as pfEarthSky would provide */
    S.viewer->setLightingMode(osg::View::SKY_LIGHT);
    osg::Light* sun = S.viewer->getLight();
    sun->setPosition(osg::Vec4(0.3f, -0.4f, 1.0f, 0.0f));
    sun->setAmbient(osg::Vec4(0.35f, 0.35f, 0.35f, 1.0f));
    sun->setDiffuse(osg::Vec4(1.0f, 1.0f, 0.95f, 1.0f));

    S.viewer->getCamera()->setAllowEventFocus(false);
    S.viewer->realize();
    S.winOpen = true;
}

/* build the osg::Geometry from a PfOsgGSet spec (de-indexed per-vertex) */
void compileGSet(PfOsgGSet* g)
{
    g->dirty = false;
    osg::Geometry* geom = g->geom.get();
    geom->getPrimitiveSetList().clear();

    GLenum glmode = GL_TRIANGLES;
    int fixed = 0;
    bool strip = false;
    switch (g->ptype) {
        case PFGS_POINTS:     glmode = GL_POINTS;         fixed = 1; break;
        case PFGS_LINES:      glmode = GL_LINES;          fixed = 2; break;
        case PFGS_TRIS:       glmode = GL_TRIANGLES;      fixed = 3; break;
        case PFGS_QUADS:      glmode = GL_QUADS;          fixed = 4; break;
        case PFGS_TRISTRIPS:  glmode = GL_TRIANGLE_STRIP; strip = true; break;
        case PFGS_TRIFANS:    glmode = GL_TRIANGLE_FAN;   strip = true; break;
        case PFGS_LINESTRIPS: glmode = GL_LINE_STRIP;     strip = true; break;
        case PFGS_POLYS:      glmode = GL_POLYGON;        strip = true; break;
        default:
            fprintf(stderr, "pfosg: geoset prim type %d not implemented\n",
                    g->ptype);
            return;
    }
    if (strip && !g->lengths) {
        fprintf(stderr, "pfosg: strip geoset without pfGSetPrimLengths\n");
        return;
    }

    int vtotal = 0;
    if (strip)
        for (int p = 0; p < g->nprims; p++) vtotal += g->lengths[p];
    else
        vtotal = g->nprims * fixed;

    /* per attribute: element index for global vertex v in prim p */
    auto srcIndex = [&](const PfOsgGSet::Attr& a, int v, int p) -> int {
        switch (a.binding) {
            case PFGS_OVERALL:    return a.ilist ? a.ilist[0] : 0;
            case PFGS_PER_PRIM:   return a.ilist ? a.ilist[p] : p;
            case PFGS_PER_VERTEX: return a.ilist ? a.ilist[v] : v;
        }
        return -1;
    };

    osg::ref_ptr<osg::Vec3Array> vout = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> cout_;
    osg::ref_ptr<osg::Vec3Array> nout;
    osg::ref_ptr<osg::Vec2Array> tout;
    if (g->attr[PFGS_COLOR4].binding != PFGS_OFF)    cout_ = new osg::Vec4Array;
    if (g->attr[PFGS_NORMAL3].binding != PFGS_OFF)   nout = new osg::Vec3Array;
    if (g->attr[PFGS_TEXCOORD2].binding != PFGS_OFF) tout = new osg::Vec2Array;

    int v = 0;
    for (int p = 0; p < g->nprims; p++) {
        int plen = strip ? g->lengths[p] : fixed;
        for (int k = 0; k < plen; k++, v++) {
            const PfOsgGSet::Attr& va = g->attr[PFGS_COORD3];
            int i = srcIndex(va, v, p);
            vout->push_back(i >= 0 && va.data
                ? osg::Vec3(va.data[i*3], va.data[i*3+1], va.data[i*3+2])
                : osg::Vec3());
            if (cout_) {
                const PfOsgGSet::Attr& a = g->attr[PFGS_COLOR4];
                i = srcIndex(a, v, p);
                cout_->push_back(osg::Vec4(a.data[i*4], a.data[i*4+1],
                                           a.data[i*4+2], a.data[i*4+3]));
            }
            if (nout) {
                const PfOsgGSet::Attr& a = g->attr[PFGS_NORMAL3];
                i = srcIndex(a, v, p);
                nout->push_back(osg::Vec3(a.data[i*3], a.data[i*3+1],
                                          a.data[i*3+2]));
            }
            if (tout) {
                const PfOsgGSet::Attr& a = g->attr[PFGS_TEXCOORD2];
                i = srcIndex(a, v, p);
                tout->push_back(osg::Vec2(a.data[i*2], a.data[i*2+1]));
            }
        }
    }

    geom->setVertexArray(vout);
    if (cout_) geom->setColorArray(cout_, osg::Array::BIND_PER_VERTEX);
    if (nout)  geom->setNormalArray(nout, osg::Array::BIND_PER_VERTEX);
    if (tout)  geom->setTexCoordArray(0, tout);

    if (strip) {
        int first = 0;
        for (int p = 0; p < g->nprims; p++) {
            geom->addPrimitiveSet(new osg::DrawArrays(glmode, first,
                                                      g->lengths[p]));
            first += g->lengths[p];
        }
    } else {
        geom->addPrimitiveSet(new osg::DrawArrays(glmode, 0, vtotal));
    }
    geom->dirtyBound();
    geom->dirtyGLObjects();
}

void applyChannel()
{
    if (!S.viewer) return;
    if (S.scene && S.viewer->getSceneData() != S.scene.get())
        S.viewer->setSceneData(S.scene.get());

    for (PfOsgGSet* g : S.gsets)
        if (g->dirty) compileGSet(g);

    if (S.esky && S.esky->haveClear)
        S.viewer->getCamera()->setClearColor(S.esky->clearColor);
    else if (S.esky && (S.esky->clearMode == PFES_SKY_GRND ||
                        S.esky->clearMode == PFES_SKY))
        S.viewer->getCamera()->setClearColor(S.esky->skyTop);

    S.viewer->getCamera()->getOrCreateStateSet()->setMode(GL_LIGHTING,
        S.defaultLighting ? osg::StateAttribute::ON : osg::StateAttribute::OFF);

    int dw = 1, dh = 1;
    SDL_GL_GetDrawableSize(S.window, &dw, &dh);
    double aspect = (double)dw / (double)dh;
    double fovy = (S.fovV > 0.0f)
        ? S.fovV
        : osg::RadiansToDegrees(
              2.0 * atan(tan(osg::DegreesToRadians((double)S.fovH) * 0.5) / aspect));
    S.viewer->getCamera()->setProjectionMatrixAsPerspective(
        fovy, aspect, S.nearD, S.farD);

    if (S.haveViewMat) {
        /* Performer view matrix: eye-to-world, row-vector convention.
         * Row 0 = right, row 1 = forward (+Y), row 2 = up (+Z), row 3 = eye */
        const osg::Matrixd& m = S.viewMat;
        osg::Vec3d eye(m(3, 0), m(3, 1), m(3, 2));
        osg::Vec3d fwd(m(1, 0), m(1, 1), m(1, 2));
        osg::Vec3d up (m(2, 0), m(2, 1), m(2, 2));
        S.viewer->getCamera()->setViewMatrixAsLookAt(eye, eye + fwd, up);
    } else if (S.haveView) {
        /* Performer view: Z-up, +Y forward; hpr = heading(Z), pitch(X),
         * roll(Y) */
        double h = osg::DegreesToRadians(S.hpr.x());
        double p = osg::DegreesToRadians(S.hpr.y());
        double r = osg::DegreesToRadians(S.hpr.z());
        osg::Matrixd rot = osg::Matrixd::rotate(r, osg::Vec3d(0, 1, 0)) *
                           osg::Matrixd::rotate(p, osg::Vec3d(1, 0, 0)) *
                           osg::Matrixd::rotate(h, osg::Vec3d(0, 0, 1));
        osg::Vec3d fwd = osg::Matrixd::transform3x3(osg::Vec3d(0, 1, 0), rot);
        osg::Vec3d up  = osg::Matrixd::transform3x3(osg::Vec3d(0, 0, 1), rot);
        S.viewer->getCamera()->setViewMatrixAsLookAt(S.eye, S.eye + fwd, up);
    }
}

}   /* anonymous namespace */

/* ---- pfDCS / pfLOD osg subclasses ------------------------------------------ */

struct PfOsgDCS : public osg::MatrixTransform {
    osg::Vec3 trans;
    osg::Vec3 rotHpr;
    float scale = 1.0f;
    void update()
    {
        /* Performer row-vector order: v * S * R * T */
        osg::Matrix m = osg::Matrix::scale(scale, scale, scale) *
            osg::Matrix::rotate(osg::DegreesToRadians(rotHpr.z()), osg::Vec3(0, 1, 0)) *
            osg::Matrix::rotate(osg::DegreesToRadians(rotHpr.y()), osg::Vec3(1, 0, 0)) *
            osg::Matrix::rotate(osg::DegreesToRadians(rotHpr.x()), osg::Vec3(0, 0, 1)) *
            osg::Matrix::translate(trans);
        setMatrix(m);
    }
};

struct PfOsgLOD : public osg::Switch {
    pfLODEvalFuncType evalFunc = nullptr;
    std::vector<float> ranges;

    struct Eval : public osg::NodeCallback {
        void operator()(osg::Node* node, osg::NodeVisitor* nv) override
        {
            PfOsgLOD* lod = static_cast<PfOsgLOD*>(node);
            if (lod->evalFunc) {
                float v = lod->evalFunc((pfLOD*)lod, (pfChannel*)&nv, nullptr);
                int n = (int)lod->getNumChildren();
                int child = (int)v;                /* no ranges: floor(v) */
                if (!lod->ranges.empty()) {
                    child = -1;
                    for (size_t i = 0; i + 1 < lod->ranges.size(); i++)
                        if (v >= lod->ranges[i] && v < lod->ranges[i + 1])
                            { child = (int)i; break; }
                }
                lod->setAllChildrenOff();
                if (child >= 0 && child < n)
                    lod->setSingleChildOn((unsigned)child);
            }
            traverse(node, nv);
        }
    };
};

/* ---- init / frame loop ------------------------------------------------------*/

extern "C" int pfInit(void)
{
    if (S.inited) return 1;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "pfosg: SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    S.startTicks = SDL_GetTicks() / 1000.0;
    S.inited = true;
    return 1;
}

extern "C" int pfMultiprocess(int) { return 1; }
extern "C" int pfConfig(void) { fatalIfUninited("pfConfig"); return 1; }

extern "C" void pfExit(void)
{
    if (S.glctx) SDL_GL_DeleteContext(S.glctx);
    if (S.window) SDL_DestroyWindow(S.window);
    S.viewer = nullptr;
    S.keep.clear();
    if (S.inited) SDL_Quit();
    S.inited = false;
}

extern "C" int pfSync(void) { return 1; }

extern "C" int pfFrame(void)
{
    fatalIfUninited("pfFrame");
    openWindow();

    double now = pfGetTime();
    pfosgInputBeginFrame(now);
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            pfExit();
            exit(0);
        }
        if (ev.type == SDL_WINDOWEVENT &&
            ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            int dw = 0, dh = 0;
            SDL_GL_GetDrawableSize(S.window, &dw, &dh);
            S.gw->resized(0, 0, dw, dh);
            S.gw->getEventQueue()->windowResize(0, 0, dw, dh);
        }
        pfosgInputSDLEvent(ev, now);
    }

    applyChannel();

    /* user DRAW callback: pfClearChan/pfDraw inside it are no-ops because
     * viewer.frame() below performs the actual clear+draw */
    if (S.drawFunc)
        S.drawFunc((pfChannel*)&S, nullptr);

    S.viewer->frame();
    SDL_GL_SwapWindow(S.window);
    S.frameCount++;

    const char* shot = getenv("PFOSG_SCREENSHOT");
    if (shot && S.frameCount == 30) {
        int dw = 0, dh = 0;
        SDL_GL_GetDrawableSize(S.window, &dw, &dh);
        osg::ref_ptr<osg::Image> img = new osg::Image;
        img->readPixels(0, 0, dw, dh, GL_RGB, GL_UNSIGNED_BYTE);
        if (osgDB::writeImageFile(*img, shot))
            fprintf(stderr, "pfosg: wrote %s\n", shot);
    }
    return 1;
}

extern "C" double pfGetTime(void)
{
    return SDL_GetTicks() / 1000.0 - S.startTicks;
}

extern "C" void pfInitClock(double t)
{
    S.startTicks = SDL_GetTicks() / 1000.0 - t;
}

extern "C" float pfFrameRate(float rate) { return rate; }

extern "C" void* pfGetSharedArena(void) { return nullptr; }
extern "C" void* pfMalloc(size_t nbytes, void*) { return calloc(1, nbytes); }

extern "C" void pfFilePath(const char* path)
{
    S.filePath.clear();
    if (!path) return;
    const char* p = path;
    while (*p) {
        const char* colon = strchr(p, ':');
        size_t n = colon ? (size_t)(colon - p) : strlen(p);
        if (n) S.filePath.emplace_back(p, n);
        p += n + (colon ? 1 : 0);
    }
}

extern "C" void pfEnable(int target)
{
    if (target == PFEN_LIGHTING) S.defaultLighting = true;
}

extern "C" void pfDisable(int target)
{
    if (target == PFEN_LIGHTING) S.defaultLighting = false;
}

/* ---- notification -----------------------------------------------------------*/

static int pfosg_notifyLevel = PFNFY_NOTICE;

extern "C" void pfNotifyLevel(int severity) { pfosg_notifyLevel = severity; }
extern "C" int pfGetNotifyLevel(void) { return pfosg_notifyLevel; }

extern "C" void pfNotify(int severity, int, const char* format, ...)
{
    if (severity > pfosg_notifyLevel) return;
    if (!format) format = "";     /* PFNFY_MORE continuation lines pass NULL */
    static const char* sev[] = { "ALWAYS", "FATAL", "WARN",
                                 "NOTICE", "INFO", "DEBUG" };
    char msg[2048];
    va_list ap;
    va_start(ap, format);
    vsnprintf(msg, sizeof msg, format, ap);
    va_end(ap);
    fprintf(stderr, "PF %s: %s\n",
            (severity >= 0 && severity <= 5) ? sev[severity] : "?", msg);
    if (severity == PFNFY_FATAL) {
        pfExit();
        exit(1);
    }
}

/* ---- math helpers ------------------------------------------------------------*/

/* pfSinCos/pfTan/... are macros in the real prmath.h */

extern "C" void (pfSetVec3)(pfVec3 v, float x, float y, float z)
{
    v[0] = x; v[1] = y; v[2] = z;
}

extern "C" void (pfCopyVec3)(pfVec3 dst, const pfVec3 v)
{
    dst[0] = v[0]; dst[1] = v[1]; dst[2] = v[2];
}

extern "C" void (pfMakeScaleMat)(pfMatrix mat, float x, float y, float z)
{
    memset(mat, 0, sizeof(pfMatrix));
    mat[0][0] = x; mat[1][1] = y; mat[2][2] = z; mat[3][3] = 1.0f;
}

extern "C" void (pfMakeTransMat)(pfMatrix mat, float x, float y, float z)
{
    memset(mat, 0, sizeof(pfMatrix));
    mat[0][0] = mat[1][1] = mat[2][2] = mat[3][3] = 1.0f;
    mat[3][0] = x; mat[3][1] = y; mat[3][2] = z;
}

extern "C" void (pfPostRotMat)(pfMatrix dst, const pfMatrix m, float degrees,
                               float x, float y, float z)
{
    osg::Matrixf a((const float*)m);
    osg::Matrixf r = osg::Matrixf::rotate(osg::DegreesToRadians(degrees),
                                          osg::Vec3(x, y, z));
    osg::Matrixf out = a * r;      /* row-vector: post-rotation */
    memcpy(dst, out.ptr(), sizeof(pfMatrix));
}

extern "C" void (pfXformPt3)(pfVec3 dst, const pfVec3 v, const pfMatrix m)
{
    float x = v[0], y = v[1], z = v[2];
    dst[0] = x * m[0][0] + y * m[1][0] + z * m[2][0] + m[3][0];
    dst[1] = x * m[0][1] + y * m[1][1] + z * m[2][1] + m[3][1];
    dst[2] = x * m[0][2] + y * m[1][2] + z * m[2][2] + m[3][2];
}

/* ---- scene graph --------------------------------------------------------------*/

static pfNode* wrap(osg::Node* n)
{
    S.keep.push_back(n);
    return (pfNode*)n;
}

extern "C" pfScene* pfNewScene(void) { return (pfScene*)wrap(new osg::Group); }
extern "C" pfGroup* pfNewGroup(void) { return (pfGroup*)wrap(new osg::Group); }
extern "C" pfGeode* pfNewGeode(void) { return (pfGeode*)wrap(new osg::Geode); }

extern "C" pfDCS* pfNewDCS(void)
{
    return (pfDCS*)wrap(new PfOsgDCS);
}

extern "C" pfSCS* pfNewSCS(pfMatrix mat)
{
    osg::Matrixf m((const float*)mat);
    osg::MatrixTransform* xf = new osg::MatrixTransform(m);
    xf->setDataVariance(osg::Object::STATIC);
    return (pfSCS*)wrap(xf);
}

extern "C" pfLOD* pfNewLOD(void)
{
    PfOsgLOD* lod = new PfOsgLOD;
    lod->setUpdateCallback(new PfOsgLOD::Eval);
    return (pfLOD*)wrap(lod);
}

extern "C" pfLightSource* pfNewLSource(void)
{
    /* marker only: a real osg::LightSource would install a default-parameter
     * GL_LIGHT0 that tramples the viewer's sun.  The shim's sun (SKY_LIGHT
     * in openWindow) plays the role of the sample's single light source. */
    osg::ref_ptr<osg::Group> ls = new osg::Group;
    ls->setName("pfLightSource");
    return (pfLightSource*)wrap(ls.get());
}

extern "C" int (pfAddChild)(pfGroup* group, pfNode* child)
{
    osg::Group* g = ((osg::Node*)group)->asGroup();
    if (!g || !child) return 0;
    return g->addChild((osg::Node*)child) ? 1 : 0;
}

extern "C" int (pfGetNodeBSphere)(pfNode* node, pfSphere* sphere)
{
    if (!node || !sphere) return 0;
    /* geosets may still be un-compiled (no pfFrame yet); compile so the
     * bound is real - matches Performer computing bounds on demand */
    for (PfOsgGSet* g : S.gsets)
        if (g->dirty) compileGSet(g);
    osg::BoundingSphere bs = ((osg::Node*)node)->getBound();
    sphere->center[0] = bs.center().x();
    sphere->center[1] = bs.center().y();
    sphere->center[2] = bs.center().z();
    sphere->radius = bs.radius();
    return 1;
}

extern "C" void (pfNodeName)(pfNode* node, const char* name)
{
    if (node && name) ((osg::Node*)node)->setName(name);
}

extern "C" void (pfNodeTravMask)(pfNode*, int, unsigned int, int, int) {}

extern "C" void (pfDCSTrans)(pfDCS* dcs, float x, float y, float z)
{
    PfOsgDCS* d = (PfOsgDCS*)dcs;
    d->trans.set(x, y, z);
    d->update();
}

extern "C" void (pfDCSRot)(pfDCS* dcs, float h, float p, float r)
{
    PfOsgDCS* d = (PfOsgDCS*)dcs;
    d->rotHpr.set(h, p, r);
    d->update();
}

extern "C" void (pfDCSScale)(pfDCS* dcs, float s)
{
    PfOsgDCS* d = (PfOsgDCS*)dcs;
    d->scale = s;
    d->update();
}

extern "C" void (pfLODRange)(pfLOD* lod, int index, float range)
{
    PfOsgLOD* l = (PfOsgLOD*)lod;
    if ((int)l->ranges.size() <= index) l->ranges.resize(index + 1);
    l->ranges[index] = range;
}

extern "C" void (pfLODUserEvalFunc)(pfLOD* lod, pfLODEvalFuncType func)
{
    ((PfOsgLOD*)lod)->evalFunc = func;
}

/* ---- geosets --------------------------------------------------------------------*/

extern "C" pfGeoSet* pfNewGSet(void*)
{
    PfOsgGSet* g = new PfOsgGSet;
    g->geom = new osg::Geometry;
    g->geom->setUseDisplayList(false);
    g->geom->setDataVariance(osg::Object::DYNAMIC);
    S.gsets.push_back(g);
    return (pfGeoSet*)g;
}

extern "C" void pfGSetPrimType(pfGeoSet* gset, int type)
{
    ((PfOsgGSet*)gset)->ptype = type;
    ((PfOsgGSet*)gset)->dirty = true;
}

extern "C" void pfGSetNumPrims(pfGeoSet* gset, int n)
{
    ((PfOsgGSet*)gset)->nprims = n;
    ((PfOsgGSet*)gset)->dirty = true;
}

extern "C" void pfGSetPrimLengths(pfGeoSet* gset, int* lengths)
{
    ((PfOsgGSet*)gset)->lengths = lengths;
    ((PfOsgGSet*)gset)->dirty = true;
}

extern "C" void pfGSetAttr(pfGeoSet* gset, int attr, int binding,
                           void* alist, ushort* ilist)
{
    PfOsgGSet* g = (PfOsgGSet*)gset;
    if (attr < 0 || attr > 3) return;
    g->attr[attr].binding = binding;
    g->attr[attr].data = (const float*)alist;
    g->attr[attr].ilist = ilist;
    g->dirty = true;
}

extern "C" void pfGSetGState(pfGeoSet* gset, pfGeoState* gstate);

extern "C" int (pfAddGSet)(pfGeode* geode, pfGeoSet* gset)
{
    osg::Geode* gd = ((osg::Node*)geode)->asGeode();
    if (!gd || !gset) return 0;
    return gd->addDrawable(((PfOsgGSet*)gset)->geom.get()) ? 1 : 0;
}

extern "C" pfGeoSet* (pfGetGSet)(pfGeode* geode, int index)
{
    osg::Geode* gd = ((osg::Node*)geode)->asGeode();
    if (!gd || index < 0 || index >= (int)gd->getNumDrawables())
        return nullptr;
    osg::Drawable* dr = gd->getDrawable(index);
    for (PfOsgGSet* g : S.gsets)
        if (g->geom.get() == dr)
            return (pfGeoSet*)g;
    return nullptr;
}

extern "C" void pfGetGSetAttrLists(pfGeoSet* gset, int attr,
                                   void** alist, ushort** ilist)
{
    PfOsgGSet* g = (PfOsgGSet*)gset;
    if (alist) *alist = nullptr;
    if (ilist) *ilist = nullptr;
    if (!g || attr < 0 || attr > 3) return;
    if (alist) *alist = (void*)g->attr[attr].data;
    if (ilist) *ilist = (ushort*)g->attr[attr].ilist;
}

/* ---- textures ---------------------------------------------------------------------*/

extern "C" pfTexture* pfNewTex(void*)
{
    PfOsgTex* t = new PfOsgTex;
    t->tex = new osg::Texture2D;
    t->tex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    t->tex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    t->tex->setFilter(osg::Texture::MIN_FILTER,
                      osg::Texture::LINEAR_MIPMAP_LINEAR);
    t->tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    return (pfTexture*)t;
}

extern "C" int pfLoadTexFile(pfTexture* tex, const char* fileName)
{
    PfOsgTex* t = (PfOsgTex*)tex;
    std::string path = resolveFile(fileName);
    if (path.empty()) {
        pfNotify(PFNFY_WARN, PFNFY_PRINT,
                 "pfLoadTexFile: could not find \"%s\"", fileName);
        return 0;
    }
    size_t dot = path.find_last_of('.');
    std::string ext = dot == std::string::npos ? "" : path.substr(dot + 1);
    if (ext == "pfi")
        t->img = pfb2osgLoadPfiImage(path);
    else
        t->img = osgDB::readImageFile(path);   /* .rgb via OSG's sgi plugin */
    if (!t->img) {
        pfNotify(PFNFY_WARN, PFNFY_PRINT,
                 "pfLoadTexFile: could not read \"%s\"", path.c_str());
        return 0;
    }
    t->tex->setImage(t->img.get());
    return 1;
}

extern "C" void pfTexFilter(pfTexture* tex, int which, int filter)
{
    PfOsgTex* t = (PfOsgTex*)tex;
    if (which == PFTEX_MINFILTER)
        t->tex->setFilter(osg::Texture::MIN_FILTER,
            filter == PFTEX_POINT ? osg::Texture::NEAREST
          : filter == PFTEX_BILINEAR ? osg::Texture::LINEAR
          : osg::Texture::LINEAR_MIPMAP_LINEAR);
    else
        t->tex->setFilter(osg::Texture::MAG_FILTER,
            filter == PFTEX_POINT ? osg::Texture::NEAREST
                                  : osg::Texture::LINEAR);
}

extern "C" void pfGetTexImage(pfTexture* tex, uint** image, int* comp,
                              int* sx, int* sy, int* sz)
{
    PfOsgTex* t = (PfOsgTex*)tex;
    osg::Image* img = t->img.get();
    if (image) *image = img ? (uint*)img->data() : nullptr;
    if (comp)
        *comp = img ? osg::Image::computeNumComponents(img->getPixelFormat()) : 0;
    if (sx) *sx = img ? img->s() : 0;
    if (sy) *sy = img ? img->t() : 0;
    if (sz) *sz = img ? img->r() : 0;
}

extern "C" pfTexEnv* pfNewTEnv(void*)
{
    osg::TexEnv* e = new osg::TexEnv(osg::TexEnv::MODULATE);
    e->ref();                          /* shim-owned */
    return (pfTexEnv*)e;
}

extern "C" void pfTEnvMode(pfTexEnv* tev, int mode)
{
    osg::TexEnv* e = (osg::TexEnv*)tev;
    switch (mode) {
        case PFTE_BLEND: e->setMode(osg::TexEnv::BLEND);    break;
        case PFTE_DECAL: e->setMode(osg::TexEnv::DECAL);    break;
        default:         e->setMode(osg::TexEnv::MODULATE); break;
    }
}

extern "C" void pfTEnvBlendColor(pfTexEnv* tev, float r, float g, float b, float a)
{
    ((osg::TexEnv*)tev)->setColor(osg::Vec4(r, g, b, a));
}

/* ---- geostate ------------------------------------------------------------------*/

struct PfOsgGState {
    osg::ref_ptr<osg::StateSet> ss;
    float alphaRef = 0.0f;
    int alphaFunc = PFAF_OFF;
    void applyAlpha()
    {
        if (alphaFunc == PFAF_OFF) {
            ss->removeAttribute(osg::StateAttribute::ALPHAFUNC);
            return;
        }
        ss->setAttributeAndModes(new osg::AlphaFunc(
            (osg::AlphaFunc::ComparisonFunction)(GL_NEVER + alphaFunc - 1),
            alphaRef));
    }
};

extern "C" pfGeoState* pfNewGState(void*)
{
    PfOsgGState* g = new PfOsgGState;
    g->ss = new osg::StateSet;
    return (pfGeoState*)g;
}

extern "C" void pfGStateMode(pfGeoState* gs, int mode, int val)
{
    if (!gs) return;
    PfOsgGState* g = (PfOsgGState*)gs;
    osg::StateSet* ss = g->ss.get();
    switch (mode) {
    case PFSTATE_ENLIGHTING:
        ss->setMode(GL_LIGHTING, val ? osg::StateAttribute::ON
                                     : osg::StateAttribute::OFF);
        break;
    case PFSTATE_ENTEXTURE:
        if (!val) ss->setTextureMode(0, GL_TEXTURE_2D,
                                     osg::StateAttribute::OFF);
        /* ON is implied by setting the texture attribute */
        break;
    case PFSTATE_TRANSPARENCY:
        if (val != PFTR_OFF) {
            ss->setMode(GL_BLEND, osg::StateAttribute::ON);
            ss->setAttributeAndModes(
                new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        } else {
            ss->setMode(GL_BLEND, osg::StateAttribute::OFF);
            ss->setRenderingHint(osg::StateSet::OPAQUE_BIN);
        }
        break;
    case PFSTATE_ALPHAFUNC:
        g->alphaFunc = val;
        g->applyAlpha();
        break;
    case PFSTATE_CULLFACE:
        if (val == PFCF_OFF)
            ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
        else {
            static const osg::CullFace::Mode m[] = {
                osg::CullFace::BACK, osg::CullFace::BACK,
                osg::CullFace::FRONT, osg::CullFace::FRONT_AND_BACK };
            ss->setAttributeAndModes(new osg::CullFace(m[val]));
        }
        break;
    case PFSTATE_ENTEXMAT:
        /* the TexMat attribute (PFSTATE_TEXMAT) carries the state */
        break;
    case PFSTATE_ENFOG:
    case PFSTATE_ENWIREFRAME:
        break;
    default:
        fprintf(stderr, "pfosg: pfGStateMode(%d) not implemented\n", mode);
        break;
    }
}

extern "C" void pfGStateVal(pfGeoState* gs, int which, float val)
{
    PfOsgGState* g = (PfOsgGState*)gs;
    if (which == PFSTATE_ALPHAREF) {
        g->alphaRef = val;
        g->applyAlpha();
    }
}

extern "C" void pfGStateAttr(pfGeoState* gs, int which, void* attr)
{
    PfOsgGState* g = (PfOsgGState*)gs;
    osg::StateSet* ss = g->ss.get();
    switch (which) {
    case PFSTATE_TEXTURE:
        ss->setTextureAttributeAndModes(0, ((PfOsgTex*)attr)->tex.get());
        break;
    case PFSTATE_TEXENV:
        ss->setTextureAttributeAndModes(0, (osg::TexEnv*)attr);
        break;
    case PFSTATE_TEXMAT: {
        osg::Matrixf m((const float*)attr);   /* pfMatrix* */
        ss->setTextureAttributeAndModes(0, new osg::TexMat(m));
        break;
    }
    default:
        fprintf(stderr, "pfosg: pfGStateAttr(%d) not implemented\n", which);
        break;
    }
}

extern "C" void pfGSetGState(pfGeoSet* gset, pfGeoState* gstate)
{
    if (!gset || !gstate) return;
    ((PfOsgGSet*)gset)->geom->setStateSet(((PfOsgGState*)gstate)->ss.get());
}

extern "C" void (pfSceneGState)(pfScene* scene, pfGeoState* gs)
{
    if (!scene || !gs) return;
    ((osg::Node*)scene)->setStateSet(((PfOsgGState*)gs)->ss.get());
}

/* ---- earth-sky --------------------------------------------------------------------*/

extern "C" pfEarthSky* pfNewESky(void) { return (pfEarthSky*)new PfOsgESky; }

extern "C" void pfESkyMode(pfEarthSky* esky, int mode, int val)
{
    if (esky && mode == PFES_BUFFER_CLEAR)
        ((PfOsgESky*)esky)->clearMode = val;
}

extern "C" void pfESkyAttr(pfEarthSky*, int, float)
{
    /* ground height etc.: no visual yet (sky/ground quads are M-later) */
}

extern "C" void pfESkyColor(pfEarthSky* esky, int which,
                            float r, float g, float b, float a)
{
    PfOsgESky* e = (PfOsgESky*)esky;
    switch (which) {
    case PFES_CLEAR:
        e->clearColor.set(r, g, b, a);
        e->haveClear = true;
        break;
    case PFES_SKY_TOP: e->skyTop.set(r, g, b, a); break;
    case PFES_GRND_FAR: e->grndFar.set(r, g, b, a); break;
    case PFES_GRND_NEAR: e->grndNear.set(r, g, b, a); break;
    }
}

/* ---- pipe / window / channel --------------------------------------------------------*/

extern "C" pfPipe* pfGetPipe(int index)
{
    fatalIfUninited("pfGetPipe");
    return (pfPipe*)(index == 0 ? &S : nullptr);
}

extern "C" pfPipeWindow* pfNewPWin(pfPipe*) { return (pfPipeWindow*)&S; }

extern "C" void pfPWinType(pfPipeWindow*, int) {}

extern "C" void pfPWinName(pfPipeWindow*, const char* name)
{
    if (name) S.winName = name;
}

extern "C" void pfPWinOriginSize(pfPipeWindow*, int x, int y, int xs, int ys)
{
    S.winX = x; S.winY = y;
    if (xs > 0) S.winW = xs;
    if (ys > 0) S.winH = ys;
}

extern "C" void pfGetPWinSize(pfPipeWindow*, int* xs, int* ys)
{
    if (xs) *xs = S.winW;
    if (ys) *ys = S.winH;
}

extern "C" int pfOpenPWin(pfPipeWindow*)
{
    openWindow();
    return 1;
}

extern "C" void pfPWinConfigFunc(pfPipeWindow*, pfPWinFuncType func)
{
    S.winConfigFunc = func;
}

extern "C" void pfConfigPWin(pfPipeWindow* pw)
{
    /* single process: run the config callback right here */
    if (S.winConfigFunc) S.winConfigFunc(pw);
    else openWindow();
}

extern "C" void* pfGetCurWSConnection(void)
{
    static int dummyDisplay = 0;
    return &dummyDisplay;
}

extern "C" unsigned long pfGetPWinWSWindow(pfPipeWindow*) { return 0; }

extern "C" pfChannel* pfNewChan(pfPipe*) { return (pfChannel*)&S; }

extern "C" void pfChanScene(pfChannel*, pfScene* scene)
{
    S.scene = ((osg::Node*)scene)->asGroup();
}

extern "C" void pfChanFOV(pfChannel*, float fovh, float fovv)
{
    S.fovH = fovh;
    S.fovV = fovv;
}

extern "C" void pfChanNearFar(pfChannel*, float nearDist, float farDist)
{
    S.nearD = nearDist;
    S.farD = farDist;
}

extern "C" void pfChanView(pfChannel*, pfVec3 xyz, pfVec3 hpr)
{
    S.eye.set(xyz[0], xyz[1], xyz[2]);
    S.hpr.set(hpr[0], hpr[1], hpr[2]);
    S.haveView = true;
    S.haveViewMat = false;
}

extern "C" void pfChanViewMat(pfChannel*, pfMatrix mat)
{
    S.viewMat.set(&mat[0][0]);      /* both row-major, row-vector */
    S.haveViewMat = true;
}

extern "C" void pfGetChanViewMat(pfChannel*, pfMatrix mat)
{
    if (S.haveViewMat) {
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                mat[r][c] = (float)S.viewMat(r, c);
    } else {
        pfCoord coord;
        pfSetVec3(coord.xyz, (float)S.eye.x(), (float)S.eye.y(),
                  (float)S.eye.z());
        pfSetVec3(coord.hpr, (float)S.hpr.x(), (float)S.hpr.y(),
                  (float)S.hpr.z());
        pfMakeCoordMat(mat, &coord);
    }
}

extern "C" void pfGetChanView(pfChannel*, pfVec3 xyz, pfVec3 hpr)
{
    xyz[0] = (float)S.eye.x(); xyz[1] = (float)S.eye.y();
    xyz[2] = (float)S.eye.z();
    hpr[0] = (float)S.hpr.x(); hpr[1] = (float)S.hpr.y();
    hpr[2] = (float)S.hpr.z();
}

extern "C" void pfGetChanFOV(pfChannel*, float* fovh, float* fovv)
{
    if (fovh) *fovh = S.fovH;
    if (fovv) *fovv = S.fovV;
}

extern "C" void pfChanESky(pfChannel*, pfEarthSky* esky)
{
    S.esky = (PfOsgESky*)esky;
}

extern "C" void pfChanTravFunc(pfChannel*, int trav, pfChanFuncType func)
{
    if (trav == PFTRAV_DRAW) S.drawFunc = (PfosgChanFunc)func;
}

void pfosgCompileDirtyGSets(void)
{
    for (PfOsgGSet* g : S.gsets)
        if (g->dirty) compileGSet(g);
}

extern "C" void pfClearChan(pfChannel*) {}   /* viewer.frame() clears */
extern "C" void pfDraw(void) {}              /* viewer.frame() draws  */
extern "C" void pfDrawChanStats(pfChannel*) {}

/* ---- intersection ---------------------------------------------------------------------*/

static PfOsgHit pf_hitPool[PFIS_MAX_SEGS];
static pfHit* pf_hitPtrs[PFIS_MAX_SEGS];

extern "C" int (pfNodeIsectSegs)(pfNode* node, pfSegSet* segSet, pfHit** hits[])
{
    if (!node || !segSet) return 0;
    /* make sure app-built geometry exists before intersecting */
    for (PfOsgGSet* g : S.gsets)
        if (g->dirty) compileGSet(g);

    int found = 0;
    for (int i = 0; i < PFIS_MAX_SEGS; i++) {
        pf_hitPool[i].valid = false;
        pf_hitPtrs[i] = (pfHit*)&pf_hitPool[i];
        if (hits) hits[i] = &pf_hitPtrs[i];
        if (!(segSet->activeMask & (1u << i))) continue;

        const pfSeg& seg = segSet->segs[i];
        osg::Vec3d start(seg.pos[0], seg.pos[1], seg.pos[2]);
        osg::Vec3d end = start + osg::Vec3d(seg.dir[0], seg.dir[1],
                                            seg.dir[2]) * seg.length;
        osg::ref_ptr<osgUtil::LineSegmentIntersector> lsi =
            new osgUtil::LineSegmentIntersector(start, end);
        osgUtil::IntersectionVisitor iv(lsi.get());
        ((osg::Node*)node)->accept(iv);
        if (lsi->containsIntersections()) {
            const osgUtil::LineSegmentIntersector::Intersection& hit =
                lsi->getFirstIntersection();
            osg::Vec3d p = hit.getWorldIntersectPoint();
            osg::Vec3d n = hit.getWorldIntersectNormal();
            pf_hitPool[i].point.set(p.x(), p.y(), p.z());
            pf_hitPool[i].normal.set(n.x(), n.y(), n.z());
            pf_hitPool[i].valid = true;
            found++;
        }
    }
    return found;
}

extern "C" int pfQueryHit(pfHit* hit, int which, void* dst)
{
    PfOsgHit* h = (PfOsgHit*)hit;
    if (!h || !dst) return 0;
    switch (which) {
    case PFQHIT_POINT: {
        /* world-space point; PFQHIT_XFORM is identity to match */
        float* v = (float*)dst;
        v[0] = h->point.x(); v[1] = h->point.y(); v[2] = h->point.z();
        return h->valid;
    }
    case PFQHIT_NORM: {
        float* v = (float*)dst;
        v[0] = h->normal.x(); v[1] = h->normal.y(); v[2] = h->normal.z();
        return h->valid;
    }
    case PFQHIT_XFORM: {
        float* m = (float*)dst;
        memset(m, 0, 16 * sizeof(float));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
        return h->valid;
    }
    case PFQHIT_FLAGS:
        *(int*)dst = h->valid ? 1 : 0;
        return h->valid;
    }
    return 0;
}

/* ---- X11 stubs (see include/X11/Xlib.h) -------------------------------------------------*/

extern "C" int XSelectInput(Display*, Window, long) { return 0; }
extern "C" int XMapWindow(Display*, Window) { return 0; }
extern "C" int XEventsQueued(Display*, int) { return 0; }
extern "C" int XNextEvent(Display*, XEvent*) { return 0; }
extern "C" int XLookupString(XKeyEvent*, char*, int, KeySym*, XComposeStatus*)
{ return 0; }

/* ---- pfdu: database loading --------------------------------------------------------------*/

extern "C" int pfdInitConverter(const char*) { return 1; }

extern "C" pfNode* pfdLoadFile(const char* fileName)
{
    std::string path = resolveFile(fileName);
    if (path.empty()) {
        pfNotify(PFNFY_WARN, PFNFY_PRINT,
                 "pfdLoadFile: could not find \"%s\"", fileName);
        return nullptr;
    }
    osg::ref_ptr<osg::Node> node = pfb2osgLoadFile(path);
    if (!node) return nullptr;
    S.keep.push_back(node);
    return (pfNode*)node.get();
}
