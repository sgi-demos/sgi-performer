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
#include <osg/Depth>
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
#include <osg/LineWidth>
#include <osg/ShadeModel>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/PolytopeIntersector>
#include <osgViewer/Viewer>

#include <SDL.h>

#include <execinfo.h>

#include <iostream>

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <vector>

/* ---- shim object structs -------------------------------------------------- */

/* PfOsgGSet / PfOsgTex live in pfosg_internal.h (shared with the pfpfb
 * loader support in pfosg_pfb.cpp) */

struct PfOsgESky {
    osg::Vec4 clearColor = osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    bool haveClear = false;
    /* Performer-like defaults so an esky with only ground colors set still
     * clears to a plausible sky */
    osg::Vec4 skyTop  = osg::Vec4(0.28f, 0.42f, 0.72f, 1.0f);
    osg::Vec4 skyBot  = osg::Vec4(0.36f, 0.52f, 0.83f, 1.0f);
    osg::Vec4 horiz   = osg::Vec4(0.72f, 0.68f, 0.6f, 1.0f);
    osg::Vec4 grndFar = osg::Vec4(0.3f, 0.15f, 0.05f, 1.0f);
    osg::Vec4 grndNear = osg::Vec4(0.5f, 0.3f, 0.1f, 1.0f);
    float grndHt = -1.0f;
    float horizAngle = 10.0f;           /* degrees of horizon band */
    int clearMode = PFES_FAST;
    bool dirty = true;                  /* geometry/colors need a rebuild */

    /* sky dome + ground sheet (built lazily when a SKY mode is active) */
    osg::ref_ptr<osg::MatrixTransform> xform;
    osg::ref_ptr<osg::Geometry> skyGeom;
    osg::ref_ptr<osg::Geometry> grndGeom;
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

void pfosgDebugPick(float px, float py)
{
    /* unproject the pixel to a world-space ray (embedded viewers don't
     * play well with View::computeIntersections) */
    osg::Camera* cam = S.viewer->getCamera();
    osg::Matrixd vpw = cam->getViewMatrix() * cam->getProjectionMatrix() *
                       cam->getViewport()->computeWindowMatrix();
    osg::Matrixd inv = osg::Matrixd::inverse(vpw);
    osg::Vec3d nearPt = osg::Vec3d(px, py, 0.0) * inv;
    osg::Vec3d farPt  = osg::Vec3d(px, py, 1.0) * inv;

    /* depth-buffer probe: what actually won this pixel last frame?  compare
     * with the road-plane prediction to distinguish "never rasterized"
     * (depth = 1, clear) from "depth-blocked by an invisible occluder". */
    {
        float dz = -1.0f;
        glReadPixels((int)px, (int)py, 1, 1,
                     GL_DEPTH_COMPONENT, GL_FLOAT, &dz);
        /* predicted window-z of the z=0 ground plane along this pixel ray */
        double t = nearPt.z() / (nearPt.z() - farPt.z());
        osg::Vec3d gp = nearPt + (farPt - nearPt) * t;
        osg::Vec3d win = gp * vpw;
        double fovy = 0, ar = 0, zn = 0, zf = 0;
        cam->getProjectionMatrixAsPerspective(fovy, ar, zn, zf);
        fprintf(stderr,
                "pfosg: depth at (%.0f,%.0f) = %.6f (ground plane predicts "
                "%.6f at world %.1f %.1f)  renderer=\"%s\" GL %s  "
                "proj fovy=%.2f near=%.6f far=%.1f\n",
                px, py, dz, win.z(), gp.x(), gp.y(),
                (const char*)glGetString(GL_RENDERER),
                (const char*)glGetString(GL_VERSION), fovy, zn, zf);
    }

    osg::ref_ptr<osgUtil::LineSegmentIntersector> lsi =
        new osgUtil::LineSegmentIntersector(nearPt, farPt);
    osgUtil::IntersectionVisitor iv(lsi.get());
    S.viewer->getSceneData()->accept(iv);
    osgUtil::LineSegmentIntersector::Intersections& hits =
        lsi->getIntersections();
    if (!hits.empty()) {
        fprintf(stderr, "pfosg: pick at (%.0f,%.0f): %zu hits\n",
                px, py, hits.size());
        int n = 0;
        for (const auto& hit : hits) {
            if (n++ >= 4) break;
            std::string path;
            for (osg::Node* pn : hit.nodePath) {
                path += "/";
                path += pn->getName().empty() ? pn->className()
                                              : pn->getName();
            }
            const osg::Vec3d& p = hit.getWorldIntersectPoint();
            fprintf(stderr, "  hit %d: %s drawable=%s world=(%.1f %.1f %.1f)\n",
                    n, path.c_str(),
                    hit.drawable.valid()
                        ? (hit.drawable->getName().empty()
                               ? hit.drawable->className()
                               : hit.drawable->getName().c_str())
                        : "?",
                    p.x(), p.y(), p.z());
            osg::Geometry* geom = hit.drawable.valid()
                                      ? hit.drawable->asGeometry() : nullptr;
            if (geom) {
                const osg::Vec3Array* va =
                    dynamic_cast<const osg::Vec3Array*>(geom->getVertexArray());
                const osg::Vec4Array* ca =
                    dynamic_cast<const osg::Vec4Array*>(geom->getColorArray());
                const osg::StateSet* ss = geom->getStateSet();
                const osg::Texture2D* tx = ss
                    ? dynamic_cast<const osg::Texture2D*>(
                          ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE))
                    : nullptr;
                fprintf(stderr,
                        "        verts=%zu prims=%u color0=(%.2f %.2f %.2f %.2f) "
                        "tex=%s texmode=%s\n",
                        va ? va->size() : 0,
                        geom->getNumPrimitiveSets(),
                        ca && ca->size() ? (*ca)[0].r() : -1.0f,
                        ca && ca->size() ? (*ca)[0].g() : -1.0f,
                        ca && ca->size() ? (*ca)[0].b() : -1.0f,
                        ca && ca->size() ? (*ca)[0].a() : -1.0f,
                        tx ? tx->getName().c_str() : "(none)",
                        ss && (ss->getTextureMode(0, GL_TEXTURE_2D)
                               & osg::StateAttribute::ON) ? "ON" : "off");
                for (unsigned pi = 0; pi < geom->getNumPrimitiveSets(); pi++) {
                    const osg::DrawArrays* da =
                        dynamic_cast<const osg::DrawArrays*>(
                            geom->getPrimitiveSet(pi));
                    if (da)
                        fprintf(stderr, "        prim%u: mode=0x%x first=%d "
                                "count=%d\n", pi, da->getMode(),
                                da->getFirst(), da->getCount());
                }
                const osg::Vec2Array* ta = dynamic_cast<const osg::Vec2Array*>(
                    geom->getTexCoordArray(0));
                if (va && va->size() <= 16)
                    for (size_t vi = 0; vi < va->size(); vi++)
                        fprintf(stderr,
                                "        v%zu=(%.1f %.1f %.1f) uv=(%.2f %.2f) "
                                "c=(%.2f %.2f %.2f %.2f)\n", vi,
                                (*va)[vi].x(), (*va)[vi].y(), (*va)[vi].z(),
                                ta && vi < ta->size() ? (*ta)[vi].x() : -99.0f,
                                ta && vi < ta->size() ? (*ta)[vi].y() : -99.0f,
                                ca && vi < ca->size() ? (*ca)[vi].r() : -1.0f,
                                ca && vi < ca->size() ? (*ca)[vi].g() : -1.0f,
                                ca && vi < ca->size() ? (*ca)[vi].b() : -1.0f,
                                ca && vi < ca->size() ? (*ca)[vi].a() : -1.0f);
            }
        }
    } else {
        fprintf(stderr, "pfosg: pick at (%.0f,%.0f): no hits\n", px, py);
    }

    /* second pass: pixel-frustum pick.  LineSegmentIntersector skips
     * degenerate/zero-area triangles, which can still rasterize as visible
     * slivers; PolytopeIntersector catches them. */
    {
        double vw = cam->getViewport()->width();
        double vh = cam->getViewport()->height();
        double nx = 2.0 * px / vw - 1.0, ny = 2.0 * py / vh - 1.0;
        double dx = 4.0 / vw, dy = 4.0 / vh;  /* ~2px half-extent */
        osg::ref_ptr<osgUtil::PolytopeIntersector> pti =
            new osgUtil::PolytopeIntersector(
                osgUtil::Intersector::PROJECTION,
                nx - dx, ny - dy, nx + dx, ny + dy);
        osgUtil::IntersectionVisitor piv(pti.get());
        S.viewer->getSceneData()->accept(piv);
        int n = 0;
        for (const auto& hit : pti->getIntersections()) {
            if (n++ >= 8) break;
            std::string path;
            for (osg::Node* pn : hit.nodePath) {
                path += "/";
                path += pn->getName().empty() ? pn->className()
                                              : pn->getName();
            }
            osg::Vec3d p = hit.localIntersectionPoint;
            if (!hit.matrix.valid()) ; else p = p * (*hit.matrix);
            fprintf(stderr,
                    "  poly %d: %s drawable=%s prim=%u %s=(%.1f %.1f %.1f)\n",
                    n, path.c_str(),
                    hit.drawable.valid()
                        ? (hit.drawable->getName().empty()
                               ? hit.drawable->className()
                               : hit.drawable->getName().c_str())
                        : "?",
                    hit.primitiveIndex,
                    hit.matrix.valid() ? "world" : "local",
                    p.x(), p.y(), p.z());
            osg::Geometry* g = hit.drawable.valid()
                                   ? hit.drawable->asGeometry() : nullptr;
            if (g) {
                const osg::Vec3Array* va =
                    dynamic_cast<const osg::Vec3Array*>(g->getVertexArray());
                const osg::Vec4Array* ca =
                    dynamic_cast<const osg::Vec4Array*>(g->getColorArray());
                const osg::StateSet* ss = g->getStateSet();
                const osg::Texture2D* tx = ss
                    ? dynamic_cast<const osg::Texture2D*>(
                          ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE))
                    : nullptr;
                fprintf(stderr,
                        "        verts=%zu prims=%u color0=(%.2f %.2f %.2f %.2f)"
                        " tex=%s\n",
                        va ? va->size() : 0, g->getNumPrimitiveSets(),
                        ca && ca->size() ? (*ca)[0].r() : -1.0f,
                        ca && ca->size() ? (*ca)[0].g() : -1.0f,
                        ca && ca->size() ? (*ca)[0].b() : -1.0f,
                        ca && ca->size() ? (*ca)[0].a() : -1.0f,
                        tx ? tx->getName().c_str() : "(none)");
                if (va)
                    for (size_t vi = 0; vi < va->size() && vi < 12; vi++)
                        fprintf(stderr, "        v%zu=(%.2f %.2f %.2f)\n", vi,
                                (*va)[vi].x(), (*va)[vi].y(), (*va)[vi].z());
            }
        }
        if (n == 0)
            fprintf(stderr, "  poly: no hits\n");
    }
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

/* build the osg::Geometry from a PfOsgGSet spec (de-indexed per-vertex).
 * Mirrors the pfb2osg loader: FLAT_* primitives carry per-triangle colors
 * and normals (arrays omit each strip's leading verts), and surface
 * primitives are re-emitted as independent triangles with long edges
 * subdivided — Apple's Metal rasterizer drops huge camera-plane-straddling
 * slivers (see apps/shim_tests/sliver_check.c), and GLES2 has no
 * QUADS/POLYGON. */
void compileGSet(PfOsgGSet* g)
{
    g->dirty = false;
    osg::Geometry* geom = g->geom.get();
    geom->getPrimitiveSetList().clear();

    GLenum glmode = GL_TRIANGLES;
    int fixed = 0, flatSkip = 0;
    bool strip = false;
    switch (g->ptype) {
        case PFGS_POINTS:     glmode = GL_POINTS;         fixed = 1; break;
        case PFGS_LINES:      glmode = GL_LINES;          fixed = 2; break;
        case PFGS_TRIS:       glmode = GL_TRIANGLES;      fixed = 3; break;
        case PFGS_QUADS:      glmode = GL_QUADS;          fixed = 4; break;
        case PFGS_TRISTRIPS:  glmode = GL_TRIANGLE_STRIP; strip = true; break;
        case PFGS_FLAT_TRISTRIPS:
            glmode = GL_TRIANGLE_STRIP; strip = true; flatSkip = 2; break;
        case PFGS_TRIFANS:    glmode = GL_TRIANGLE_FAN;   strip = true; break;
        case PFGS_FLAT_TRIFANS:
            glmode = GL_TRIANGLE_FAN;   strip = true; flatSkip = 2; break;
        case PFGS_LINESTRIPS: glmode = GL_LINE_STRIP;     strip = true; break;
        case PFGS_FLAT_LINESTRIPS:
            glmode = GL_LINE_STRIP;     strip = true; flatSkip = 1; break;
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

    /* per attribute: element index for global vertex v (offset k in prim p).
     * flatAttr marks colors/normals, whose arrays omit each strip's first
     * flatSkip vertices in FLAT_* primitives. */
    auto srcIndex = [&](const PfOsgGSet::Attr& a, int v, int p, int k,
                        bool flatAttr) -> int {
        switch (a.binding) {
            case PFGS_OVERALL:    return a.ilist ? a.ilist[0] : 0;
            case PFGS_PER_PRIM:   return a.ilist ? a.ilist[p] : p;
            case PFGS_PER_VERTEX: {
                int idx = v;
                if (flatSkip && flatAttr) {
                    idx = 0;
                    for (int q = 0; q < p; q++)
                        idx += g->lengths[q] - flatSkip;
                    idx += k < flatSkip ? 0 : k - flatSkip;
                }
                return a.ilist ? a.ilist[idx] : idx;
            }
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
            int i = srcIndex(va, v, p, k, false);
            vout->push_back(i >= 0 && va.data
                ? osg::Vec3(va.data[i*3], va.data[i*3+1], va.data[i*3+2])
                : osg::Vec3());
            if (cout_) {
                const PfOsgGSet::Attr& a = g->attr[PFGS_COLOR4];
                i = srcIndex(a, v, p, k, true);
                cout_->push_back(osg::Vec4(a.data[i*4], a.data[i*4+1],
                                           a.data[i*4+2], a.data[i*4+3]));
            }
            if (nout) {
                const PfOsgGSet::Attr& a = g->attr[PFGS_NORMAL3];
                i = srcIndex(a, v, p, k, true);
                nout->push_back(osg::Vec3(a.data[i*3], a.data[i*3+1],
                                          a.data[i*3+2]));
            }
            if (tout) {
                const PfOsgGSet::Attr& a = g->attr[PFGS_TEXCOORD2];
                i = srcIndex(a, v, p, k, false);
                tout->push_back(osg::Vec2(a.data[i*2], a.data[i*2+1]));
            }
        }
    }

    /* surface primitives: independent triangles + subdivision (the same
     * pass as the pfb2osg loader; see the header comment) */
    bool surface = glmode == GL_TRIANGLES || glmode == GL_QUADS ||
                   glmode == GL_TRIANGLE_STRIP ||
                   glmode == GL_TRIANGLE_FAN || glmode == GL_POLYGON;
    static const float subdivMax = [] {
        const char* e = getenv("PFOSG_SUBDIV");
        return e ? (float)atof(e) : 25.0f;
    }();
    bool retriangulated = false;
    if (surface && subdivMax > 0.0f) {
        struct VR { osg::Vec3 p; osg::Vec4 c; osg::Vec3 n; osg::Vec2 t; };
        auto rec = [&](int i) {
            VR r;
            r.p = (*vout)[i];
            if (cout_) r.c = (*cout_)[i];
            if (nout)  r.n = (*nout)[i];
            if (tout)  r.t = (*tout)[i];
            return r;
        };
        auto mid = [&](const VR& a, const VR& b) {
            VR m;
            m.p = (a.p + b.p) * 0.5f;
            m.c = (a.c + b.c) * 0.5f;
            m.n = a.n + b.n;
            m.n.normalize();
            m.t = (a.t + b.t) * 0.5f;
            return m;
        };

        struct Item { VR a, b, c; int d; };
        std::vector<Item> work;
        int first = 0;
        for (int p = 0; p < g->nprims; p++) {
            int plen = strip ? g->lengths[p] : fixed;
            if (glmode == GL_TRIANGLES) {
                for (int k = 0; k + 2 < plen; k += 3)
                    work.push_back({rec(first + k), rec(first + k + 1),
                                    rec(first + k + 2), 0});
            } else if (glmode == GL_QUADS) {
                for (int k = 0; k + 3 < plen; k += 4) {
                    work.push_back({rec(first + k), rec(first + k + 1),
                                    rec(first + k + 2), 0});
                    work.push_back({rec(first + k), rec(first + k + 2),
                                    rec(first + k + 3), 0});
                }
            } else if (glmode == GL_TRIANGLE_STRIP) {
                for (int t = 0; t + 2 < plen; t++) {
                    int a = first + t, b = first + t + 1, c = first + t + 2;
                    if (t & 1) std::swap(a, b);
                    work.push_back({rec(a), rec(b), rec(c), 0});
                }
            } else {                     /* TRIANGLE_FAN / POLYGON */
                for (int t = 0; t + 2 < plen; t++)
                    work.push_back({rec(first), rec(first + t + 1),
                                    rec(first + t + 2), 0});
            }
            first += plen;
        }

        osg::ref_ptr<osg::Vec3Array> nv = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec4Array> nc = cout_ ? new osg::Vec4Array : nullptr;
        osg::ref_ptr<osg::Vec3Array> nn = nout ? new osg::Vec3Array : nullptr;
        osg::ref_ptr<osg::Vec2Array> nt = tout ? new osg::Vec2Array : nullptr;
        auto emit = [&](const VR& r) {
            nv->push_back(r.p);
            if (nc) nc->push_back(r.c);
            if (nn) nn->push_back(r.n);
            if (nt) nt->push_back(r.t);
        };
        const float mx2 = subdivMax * subdivMax;
        while (!work.empty()) {
            Item it = work.back();
            work.pop_back();
            float e0 = (it.b.p - it.a.p).length2();
            float e1 = (it.c.p - it.b.p).length2();
            float e2 = (it.a.p - it.c.p).length2();
            float longest = std::max(e0, std::max(e1, e2));
            float area2 = ((it.b.p - it.a.p) ^ (it.c.p - it.a.p)).length2();
            if (it.d >= 8 || longest <= mx2 || area2 < 1e-8f) {
                emit(it.a); emit(it.b); emit(it.c);
                continue;
            }
            if (e0 >= e1 && e0 >= e2) {
                VR m = mid(it.a, it.b);
                work.push_back({it.a, m, it.c, it.d + 1});
                work.push_back({m, it.b, it.c, it.d + 1});
            } else if (e1 >= e2) {
                VR m = mid(it.b, it.c);
                work.push_back({it.a, it.b, m, it.d + 1});
                work.push_back({it.a, m, it.c, it.d + 1});
            } else {
                VR m = mid(it.c, it.a);
                work.push_back({it.a, it.b, m, it.d + 1});
                work.push_back({m, it.b, it.c, it.d + 1});
            }
        }
        vout = nv;
        if (cout_) cout_ = nc;
        if (nout)  nout = nn;
        if (tout)  tout = nt;
        retriangulated = true;
    }

    geom->setVertexArray(vout);
    if (cout_) geom->setColorArray(cout_, osg::Array::BIND_PER_VERTEX);
    if (nout)  geom->setNormalArray(nout, osg::Array::BIND_PER_VERTEX);
    if (tout)  geom->setTexCoordArray(0, tout);

    if (retriangulated) {
        geom->addPrimitiveSet(
            new osg::DrawArrays(GL_TRIANGLES, 0, (int)vout->size()));
    } else if (strip) {
        int first = 0;
        for (int p = 0; p < g->nprims; p++) {
            geom->addPrimitiveSet(new osg::DrawArrays(glmode, first,
                                                      g->lengths[p]));
            first += g->lengths[p];
        }
    } else {
        geom->addPrimitiveSet(new osg::DrawArrays(glmode, 0, vtotal));
    }

    /* FLAT primitives shade with the provoking (last) vertex, preserved by
     * the triangle emission order above */
    if (flatSkip || g->flatShade)
        geom->getOrCreateStateSet()->setAttributeAndModes(
            new osg::ShadeModel(osg::ShadeModel::FLAT));
    if (g->lineWidth != 1.0f && (glmode == GL_LINES ||
                                 glmode == GL_LINE_STRIP))
        geom->getOrCreateStateSet()->setAttributeAndModes(
            new osg::LineWidth(g->lineWidth));

    geom->dirtyBound();
    geom->dirtyGLObjects();
}

/* ---- EarthSky sky/ground bands ---------------------------------------------
 * Performer's pfEarthSky draws an eye-following background: a sky dome with
 * a horizon band (HORIZ color at the horizon, SKY_BOT at horizAngle
 * elevation, gradient to SKY_TOP at the zenith) and a ground sheet at world
 * height grndHt (GRND_NEAR under the eye fading to GRND_FAR at the horizon).
 * Rendered before the scene, depth writes off, in a unit dome scaled to
 * ~0.9*far and translated to the eye each frame. */

static const int ESKY_SEG = 24;

static void rebuildESkyArrays(PfOsgESky* e)
{
    float ha = osg::clampBetween(e->horizAngle, 0.5f, 45.0f);
    const float elev[4] = {0.0f, ha, ha + (90.0f - ha) / 3.0f,
                           ha + 2.0f * (90.0f - ha) / 3.0f};
    osg::Vec4 ringCol[5] = {
        e->horiz, e->skyBot,
        e->skyBot * (2.0f / 3.0f) + e->skyTop * (1.0f / 3.0f),
        e->skyBot * (1.0f / 3.0f) + e->skyTop * (2.0f / 3.0f),
        e->skyTop};

    osg::Vec3Array* sv = (osg::Vec3Array*)e->skyGeom->getVertexArray();
    osg::Vec4Array* sc = (osg::Vec4Array*)e->skyGeom->getColorArray();
    int n = 0;
    for (int k = 0; k < 4; k++) {
        float el = osg::DegreesToRadians(elev[k]);
        for (int i = 0; i <= ESKY_SEG; i++, n++) {
            float az = 2.0f * (float)osg::PI * i / ESKY_SEG;
            (*sv)[n].set(cosf(az) * cosf(el), sinf(az) * cosf(el), sinf(el));
            (*sc)[n] = ringCol[k];
        }
    }
    (*sv)[n].set(0, 0, 1);          /* apex */
    (*sc)[n] = ringCol[4];
    sv->dirty();
    sc->dirty();

    osg::Vec3Array* gv = (osg::Vec3Array*)e->grndGeom->getVertexArray();
    osg::Vec4Array* gc = (osg::Vec4Array*)e->grndGeom->getColorArray();
    /* center z (relative ground depth) is refreshed per frame */
    for (int i = 0; i <= ESKY_SEG; i++) {
        float az = 2.0f * (float)osg::PI * i / ESKY_SEG;
        (*gv)[1 + i].set(cosf(az), sinf(az), 0.0f);   /* meets the horizon */
        (*gc)[1 + i] = e->grndFar;
    }
    (*gc)[0] = e->grndNear;
    gv->dirty();
    gc->dirty();
}

static osg::Geometry* newESkyGeometry(int nverts)
{
    osg::Geometry* g = new osg::Geometry;
    g->setUseDisplayList(false);
    osg::Vec3Array* v = new osg::Vec3Array(nverts);
    osg::Vec4Array* c = new osg::Vec4Array(nverts);
    v->setDataVariance(osg::Object::DYNAMIC);
    c->setDataVariance(osg::Object::DYNAMIC);
    g->setVertexArray(v);
    g->setColorArray(c, osg::Array::BIND_PER_VERTEX);
    return g;
}

static void buildESkyNodes(PfOsgESky* e)
{
    const int RING = ESKY_SEG + 1;
    e->skyGeom = newESkyGeometry(4 * RING + 1);
    osg::DrawElementsUShort* si =
        new osg::DrawElementsUShort(GL_TRIANGLES);
    for (int k = 0; k < 3; k++)
        for (int i = 0; i < ESKY_SEG; i++) {
            int a = k * RING + i, b = (k + 1) * RING + i;
            si->push_back(a); si->push_back(a + 1); si->push_back(b);
            si->push_back(a + 1); si->push_back(b + 1); si->push_back(b);
        }
    for (int i = 0; i < ESKY_SEG; i++) {          /* apex fan */
        int a = 3 * RING + i;
        si->push_back(a); si->push_back(a + 1); si->push_back(4 * RING);
    }
    e->skyGeom->addPrimitiveSet(si);

    e->grndGeom = newESkyGeometry(1 + RING);      /* cone to the horizon */
    osg::DrawElementsUShort* gi =
        new osg::DrawElementsUShort(GL_TRIANGLES);
    for (int i = 0; i < ESKY_SEG; i++) {
        gi->push_back(0); gi->push_back(1 + i); gi->push_back(2 + i);
    }
    e->grndGeom->addPrimitiveSet(gi);

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(e->skyGeom);
    geode->addDrawable(e->grndGeom);
    geode->setCullingActive(false);

    e->xform = new osg::MatrixTransform;
    e->xform->addChild(geode);
    osg::StateSet* ss = e->xform->getOrCreateStateSet();
    ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF |
                             osg::StateAttribute::PROTECTED);
    ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF |
                              osg::StateAttribute::PROTECTED);
    ss->setMode(GL_FOG, osg::StateAttribute::OFF);
    ss->setAttributeAndModes(
        new osg::Depth(osg::Depth::ALWAYS, 0.0, 1.0, false));
    ss->setRenderBinDetails(-10, "RenderBin");    /* behind everything */
}

static void updateESky()
{
    PfOsgESky* e = S.esky;
    bool wantSky = e && (e->clearMode == PFES_SKY ||
                         e->clearMode == PFES_SKY_GRND);
    if (!wantSky) {
        if (e && e->xform) e->xform->setNodeMask(0);
        return;
    }
    if (!e->xform) {
        buildESkyNodes(e);
        e->dirty = true;
    }
    if (S.root && !S.root->containsNode(e->xform.get()))
        S.root->insertChild(0, e->xform.get());
    e->xform->setNodeMask(~0u);
    e->grndGeom->setNodeMask(e->clearMode == PFES_SKY_GRND ? ~0u : 0u);
    if (e->dirty) {
        rebuildESkyArrays(e);
        e->dirty = false;
    }

    osg::Vec3d eye =
        S.viewer->getCamera()->getInverseViewMatrix().getTrans();
    float R = 0.9f * S.farD;
    if (R <= 0) R = 1.0f;
    e->xform->setMatrix(osg::Matrixd::scale(R, R, R) *
                        osg::Matrixd::translate(eye));

    /* ground center sits at world grndHt below the eye */
    osg::Vec3Array* gv = (osg::Vec3Array*)e->grndGeom->getVertexArray();
    float zc = (e->grndHt - (float)eye.z()) / R;
    zc = osg::clampBetween(zc, -1.0f, -0.0001f);
    if ((*gv)[0].z() != zc) {
        (*gv)[0].set(0, 0, zc);
        gv->dirty();
    }
}

void applyChannel()
{
    if (!S.viewer) return;
    if (S.scene) {
        if (!S.root) S.root = new osg::Group;
        if (S.viewer->getSceneData() != S.root.get())
            S.viewer->setSceneData(S.root.get());
        if (!S.root->containsNode(S.scene.get())) {
            /* replace a previous scene, keeping the esky child */
            for (unsigned i = S.root->getNumChildren(); i-- > 0;) {
                osg::Node* ch = S.root->getChild(i);
                if (!S.esky || ch != (osg::Node*)S.esky->xform.get())
                    S.root->removeChild(i, 1);
            }
            S.root->addChild(S.scene.get());
            /* Performer's default depth func is LEQUAL (OSG's is LESS); SGI
             * databases rely on it for coplanar base/decal geometry */
            S.scene->getOrCreateStateSet()->setAttributeAndModes(
                new osg::Depth(osg::Depth::LEQUAL));
        }
    }

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

    /* honor the main channel's viewport (perfly shrinks the 3D view to make
     * room for the GUI panel strip); OSG scissors its clear to the viewport,
     * leaving the panel region to the aux channel */
    double vx = 0, vy = 0, vw = dw, vh = dh;
    for (PfOsgChan* m : S.chans) {
        if (!m->isMain) continue;
        vx = m->vpL * dw;
        vy = m->vpB * dh;
        vw = (m->vpR - m->vpL) * dw;
        vh = (m->vpT - m->vpB) * dh;
        /* the main channel's near/far is authoritative for the projection */
        S.nearD = m->nearD;
        S.farD = m->farD;
        break;
    }
    osg::Viewport* vp = S.viewer->getCamera()->getViewport();
    if (!vp)
        S.viewer->getCamera()->setViewport((int)vx, (int)vy, (int)vw, (int)vh);
    else if ((int)vp->x() != (int)vx || (int)vp->y() != (int)vy ||
             (int)vp->width() != (int)vw || (int)vp->height() != (int)vh)
        vp->setViewport((int)vx, (int)vy, (int)vw, (int)vh);

    double aspect = vh > 0 ? vw / vh : 1.0;
    double fovy = (S.fovV > 0.0f)
        ? S.fovV
        : osg::RadiansToDegrees(
              2.0 * atan(tan(osg::DegreesToRadians((double)S.fovH) * 0.5) / aspect));
    /* the channel's near/far is authoritative, as in real Performer.  OSG's
     * default per-frame auto near/far pushes the near plane right up against
     * the closest geometry, and Apple's GL-on-Metal drops whole triangles
     * that straddle such a near plane (the "missing road triangle" wedge). */
    S.viewer->getCamera()->setComputeNearFarMode(
        osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
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

    updateESky();       /* needs the final view matrix (eye position) */
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
    osg::Vec3 center;

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

/* count scene geometry for the stats overlay (approximate: everything the
 * loader emits is independent triangles; strips count as n-2) */
static void countSceneStats(void)
{
    struct CountVisitor : osg::NodeVisitor {
        long tris = 0, verts = 0, geodes = 0, drawables = 0;
        CountVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
        void apply(osg::Geode& g) override
        {
            geodes++;
            for (unsigned i = 0; i < g.getNumDrawables(); i++) {
                osg::Geometry* geom = g.getDrawable(i)->asGeometry();
                if (!geom) continue;
                drawables++;
                const osg::Array* va = geom->getVertexArray();
                verts += va ? (long)va->getNumElements() : 0;
                for (unsigned p = 0; p < geom->getNumPrimitiveSets(); p++) {
                    const osg::PrimitiveSet* ps = geom->getPrimitiveSet(p);
                    unsigned n = ps->getNumIndices();
                    switch (ps->getMode()) {
                    case GL_TRIANGLES:      tris += n / 3; break;
                    case GL_TRIANGLE_STRIP:
                    case GL_TRIANGLE_FAN:
                    case GL_POLYGON:        tris += n > 2 ? n - 2 : 0; break;
                    case GL_QUADS:          tris += n / 2; break;
                    default: break;
                    }
                }
            }
            traverse(g);
        }
    } cv;
    if (S.scene) S.scene->accept(cv);
    S.statsTris = cv.tris;
    S.statsVerts = cv.verts;
    S.statsGeodes = cv.geodes;
    S.statsDrawables = cv.drawables;
}

extern "C" int pfFrame(void)
{
    fatalIfUninited("pfFrame");
    openWindow();

    /* frame timing for the stats overlay */
    static osg::Timer_t lastTick = 0, lastExitTick = 0;
    osg::Timer* timer = osg::Timer::instance();
    osg::Timer_t entryTick = timer->tick();
    if (lastTick) {
        S.statsDt[S.statsDtHead] =
            (float)timer->delta_s(lastTick, entryTick);
        S.statsDtHead = (S.statsDtHead + 1) % PfOsgState::STATS_DTS;
    }
    lastTick = entryTick;
    /* "app" = the application's work since pfFrame last returned (sim,
     * input, callbacks) — the between-frames slice, like real Performer */
    float appOutside = lastExitTick
        ? (float)timer->delta_m(lastExitTick, entryTick) : 0.0f;
    if (S.frameCount % 64 == 1) countSceneStats();

    double now = pfGetTime();
    pfosgInputBeginFrame(now);

    /* debug: synthesize a window resize at frame N (repro harness) */
    static long testResize = getenv("PFOSG_TEST_RESIZE")
        ? atol(getenv("PFOSG_TEST_RESIZE")) : 0;
    if (testResize && S.frameCount == testResize)
        SDL_SetWindowSize(S.window, 1470, 862);

    /* debug: hide all nodes with the given name (bisection harness) */
    static const char* hideName = getenv("PFOSG_HIDE");
    if (hideName && S.viewer->getSceneData()) {
        struct HideVisitor : osg::NodeVisitor {
            const char* name;
            int hid = 0;
            HideVisitor(const char* n)
                : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), name(n) {}
            void apply(osg::Node& node) override {
                if (node.getName() == name && node.getNodeMask()) {
                    node.setNodeMask(0);
                    hid++;
                } else
                    traverse(node);
            }
        } hv(hideName);
        S.viewer->getSceneData()->accept(hv);
        if (hv.hid)
            fprintf(stderr, "pfosg: PFOSG_HIDE hid %d \"%s\"\n",
                    hv.hid, hideName);
    }


    /* debug: synthetic clicks "frame:x:y[,frame:x:y...]" (window points,
     * top-down, like real SDL mouse events); button releases 5 frames on */
    static const char* testClick = getenv("PFOSG_TEST_CLICK");
    if (testClick) {
        const char* p = testClick;
        while (*p) {
            long f = 0; int cx = 0, cy = 0; int n = 0;
            if (sscanf(p, "%ld:%d:%d%n", &f, &cx, &cy, &n) == 3) {
                if (S.frameCount == f || S.frameCount == f + 5) {
                    SDL_Event ev;
                    memset(&ev, 0, sizeof ev);
                    ev.motion.type = SDL_MOUSEMOTION;
                    ev.motion.x = cx; ev.motion.y = cy;
                    SDL_PushEvent(&ev);
                    memset(&ev, 0, sizeof ev);
                    ev.button.type = S.frameCount == f ? SDL_MOUSEBUTTONDOWN
                                                       : SDL_MOUSEBUTTONUP;
                    ev.button.button = SDL_BUTTON_LEFT;
                    ev.button.x = cx; ev.button.y = cy;
                    SDL_PushEvent(&ev);
                    fprintf(stderr, "pfosg: test click %s at (%d,%d)\n",
                            S.frameCount == f ? "DOWN" : "UP", cx, cy);
                }
                p += n;
            }
            if (*p == ',') p++; else break;
        }
    }

    /* debug: programmatic pick "frame:px:py" (drawable pixels, GL y-up) */
    static const char* testPick = getenv("PFOSG_TEST_PICK");
    if (testPick) {
        long f = 0; float px = 0, py = 0;
        if (sscanf(testPick, "%ld:%f:%f", &f, &px, &py) == 3 &&
            f == S.frameCount)
            pfosgDebugPick(px, py);
    }

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
        if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_F9) {
            /* debug pause: freeze sim time, keep rendering */
            if (!S.paused) {
                S.pausedAt = now;
                S.paused = true;
                fprintf(stderr, "pfosg: PAUSED (F9 resumes; F10 picks geometry under pointer)\n");
            } else {
                /* remove the pause gap from the clock */
                S.startTicks += (SDL_GetTicks() / 1000.0 - S.startTicks)
                                - S.pausedAt;
                S.paused = false;
                fprintf(stderr, "pfosg: resumed\n");
            }
            continue;                    /* not forwarded to the app */
        }
        if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_F10) {
            /* debug pick: identify the geometry under the mouse pointer */
            int mx = 0, my = 0;
            SDL_GetMouseState(&mx, &my);
            int ww = 1, wh = 1, dw = 1, dh = 1;
            SDL_GetWindowSize(S.window, &ww, &wh);
            SDL_GL_GetDrawableSize(S.window, &dw, &dh);
            pfosgDebugPick(mx * (float)dw / ww,
                           (wh - my) * (float)dh / wh);   /* GL y-up */
            continue;
        }
                pfosgInputSDLEvent(ev, now);
    }

    applyChannel();

    /* app time = between-frames work + this frame's pre-render section */
    S.statsAppMs = appOutside + (float)osg::Timer::instance()->delta_m(
        entryTick, osg::Timer::instance()->tick());

    /* let the OSG renderer record cull/draw times for the stats overlay
     * (osgViewer::View pre-installs a "Camera" stats object with collection
     * off; flip its "rendering" switch rather than replacing it) */
    {
        osg::Camera* cam = S.viewer->getCamera();
        if (!cam->getStats()) cam->setStats(new osg::Stats("pfosg-camera"));
        if (!cam->getStats()->collectStats("rendering"))
            cam->getStats()->collectStats("rendering", true);
    }

    S.viewer->frame();

    {
        double v = 0.0;
        osg::Stats* st = S.viewer->getCamera()->getStats();
        if (st && st->getAveragedAttribute("Cull traversal time taken", v))
            S.statsCullMs = (float)(v * 1000.0);
        if (st && st->getAveragedAttribute("Draw traversal time taken", v))
            S.statsDrawMs = (float)(v * 1000.0);
        static bool dumpStats = getenv("PFOSG_STATS_DEBUG") != nullptr;
        if (dumpStats && st && S.frameCount == 20) {
            dumpStats = false;
            st->report(std::cerr,
                       S.viewer->getFrameStamp()->getFrameNumber() - 1);
        }
    }
    lastExitTick = osg::Timer::instance()->tick();

    /* channel DRAW callbacks run after the scene render: pfClearChan/pfDraw
     * inside them are no-ops (viewer.frame() did the clear+draw), so what
     * remains is their overlay drawing — perfly's messages and stats on the
     * main channel, the libpfutil GUI panel on its own channel */
    pfosgRunAuxChannels();
    SDL_GL_SwapWindow(S.window);
    S.frameCount++;

    /* once-per-second heartbeat: wall time, sim time, frame, camera pos */
    {
        static double lastLog = 0.0;
        double wall = SDL_GetTicks() / 1000.0;
        if (wall - lastLog >= 1.0) {
            lastLog = wall;
            osg::Vec3d eye = S.viewer->getCamera()->getInverseViewMatrix()
                                 .getTrans();
            fprintf(stderr,
                    "pfosg: wall=%.1f sim=%.1f frame=%ld eye=(%.1f %.1f %.1f)%s\n",
                    wall, pfGetTime(), S.frameCount,
                    eye.x(), eye.y(), eye.z(),
                    S.paused ? " [PAUSED]" : "");
        }
    }

    const char* shot = getenv("PFOSG_SCREENSHOT");
    static long shotFrame = getenv("PFOSG_SCREENSHOT_FRAME")
        ? atol(getenv("PFOSG_SCREENSHOT_FRAME")) : 30;
    static long shotEvery = getenv("PFOSG_SCREENSHOT_EVERY")
        ? atol(getenv("PFOSG_SCREENSHOT_EVERY")) : 0;
    bool capture = shot && (shotEvery > 0 ? (S.frameCount % shotEvery == 0 &&
                                             S.frameCount > 0)
                                          : S.frameCount == shotFrame);
    if (capture) {
        int dw = 0, dh = 0;
        SDL_GL_GetDrawableSize(S.window, &dw, &dh);
        osg::ref_ptr<osg::Image> img = new osg::Image;
        img->readPixels(0, 0, dw, dh, GL_RGB, GL_UNSIGNED_BYTE);
        char name[1024];
        if (shotEvery > 0)
            snprintf(name, sizeof name, "%s.%05ld.png", shot, S.frameCount);
        else
            snprintf(name, sizeof name, "%s", shot);
        if (osgDB::writeImageFile(*img, name))
            fprintf(stderr, "pfosg: wrote %s\n", name);
    }
    return 1;
}

extern "C" double pfGetTime(void)
{
    if (S.paused) return S.pausedAt;
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

static int pfosg_notifyLevel =
    getenv("PFOSG_NOTIFY") ? atoi(getenv("PFOSG_NOTIFY")) : PFNFY_NOTICE;

extern "C" void pfNotifyLevel(int severity)
{
    /* env override wins, so loader diagnostics survive perfly lowering it */
    if (getenv("PFOSG_NOTIFY")) return;
    pfosg_notifyLevel = severity;
}
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
        void* frames[32];
        int n = backtrace(frames, 32);
        backtrace_symbols_fd(frames, n, 2);
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

extern "C" void (pfNodeTravMask)(pfNode* node, int which, unsigned int mask,
                                 int /*setMode*/, int /*bitOp*/)
{
    if (!node) return;
    /* CULL mask 0 = excluded from the rendering traversal (perfly uses this
     * to hide its cull-volume visualization from the channel that owns it) */
    if (which == PFTRAV_CULL)
        ((osg::Node*)node)->setNodeMask(mask ? ~0u : 0u);
    /* APP/DRAW/ISECT masks: not needed by the demos yet */
}

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

extern "C" float pfGetLODRange(const pfLOD* lod, int index)
{
    const PfOsgLOD* l = (const PfOsgLOD*)lod;
    if (!l || index < 0 || index >= (int)l->ranges.size()) return 0.0f;
    return l->ranges[(size_t)index];
}

extern "C" void pfLODCenter(pfLOD* lod, pfVec3 c)
{
    if (lod) ((PfOsgLOD*)lod)->center.set(c[0], c[1], c[2]);
}

extern "C" void pfGetLODCenter(const pfLOD* lod, pfVec3 c)
{
    const PfOsgLOD* l = (const PfOsgLOD*)lod;
    c[0] = l ? l->center.x() : 0;
    c[1] = l ? l->center.y() : 0;
    c[2] = l ? l->center.z() : 0;
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
    if (which == PFTEX_MINFILTER) {
        t->minFilt = filter;
        t->tex->setFilter(osg::Texture::MIN_FILTER,
            filter == PFTEX_POINT ? osg::Texture::NEAREST
          : filter == PFTEX_BILINEAR ? osg::Texture::LINEAR
          : osg::Texture::LINEAR_MIPMAP_LINEAR);
    } else {
        t->magFilt = filter;
        t->tex->setFilter(osg::Texture::MAG_FILTER,
            filter == PFTEX_POINT ? osg::Texture::NEAREST
                                  : osg::Texture::LINEAR);
    }
}

extern "C" int pfGetTexFilter(const pfTexture* tex, int which)
{
    const PfOsgTex* t = (const PfOsgTex*)tex;
    if (!t) return 0;
    return which == PFTEX_MINFILTER ? t->minFilt : t->magFilt;
}

extern "C" void pfGetTexImage(const pfTexture* tex, unsigned int** image,
                              int* comp, int* sx, int* sy, int* sz)
{
    const PfOsgTex* t = (const PfOsgTex*)tex;
    const osg::Image* img = t->img.get();
    if (image) *image = img ? (unsigned int*)img->data() : nullptr;
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
            /* no depth writes from blended surfaces (see pfb2osg loader) */
            ss->setAttributeAndModes(
                new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
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
    case PFSTATE_ENLPOINTSTATE:
    case PFSTATE_ENTEXGEN:
    case PFSTATE_ENHIGHLIGHTING:
    case PFSTATE_ANTIALIAS:
    case PFSTATE_ENCOLORTABLE:
    case PFSTATE_ENTEXLOD:
        break;      /* accepted and ignored */
    default: {
        static std::set<int> warned;
        if (warned.insert(mode).second)
            fprintf(stderr, "pfosg: pfGStateMode(%d) not implemented\n",
                    mode);
        break;
    }
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
        /* NULL texture (or one whose image failed to load) = untextured */
        if (attr && ((PfOsgTex*)attr)->tex)
            ss->setTextureAttributeAndModes(0, ((PfOsgTex*)attr)->tex.get());
        break;
    case PFSTATE_TEXENV:
        if (attr) ss->setTextureAttributeAndModes(0, (osg::TexEnv*)attr);
        break;
    case PFSTATE_TEXMAT: {
        if (!attr) break;
        osg::Matrixf m((const float*)attr);   /* pfMatrix* */
        ss->setTextureAttributeAndModes(0, new osg::TexMat(m));
        break;
    }
    case PFSTATE_FRONTMTL:
    case PFSTATE_BACKMTL:
    case PFSTATE_LIGHTMODEL:
    case PFSTATE_LIGHTS:
    case PFSTATE_FOG:
    case PFSTATE_HIGHLIGHT:
    case PFSTATE_LPOINTSTATE:
    case PFSTATE_TEXGEN:
        break;      /* accepted; lighting comes from the viewer's sun */
    default: {
        static std::set<int> warned;
        if (warned.insert(which).second)
            fprintf(stderr, "pfosg: pfGStateAttr(%d) not implemented\n",
                    which);
        break;
    }
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
    if (esky && mode == PFES_BUFFER_CLEAR) {
        ((PfOsgESky*)esky)->clearMode = val;
        ((PfOsgESky*)esky)->dirty = true;
    }
}

extern "C" void pfESkyAttr(pfEarthSky* esky, int attr, float val)
{
    PfOsgESky* e = (PfOsgESky*)esky;
    if (!e) return;
    switch (attr) {
    case PFES_GRND_HT:      e->grndHt = val; break;
    case PFES_HORIZ_ANGLE:  e->horizAngle = val; break;
    default: return;
    }
    e->dirty = true;
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
    case PFES_SKY_TOP:   e->skyTop.set(r, g, b, a); break;
    case PFES_SKY_BOT:   e->skyBot.set(r, g, b, a); break;
    case PFES_HORIZ:     e->horiz.set(r, g, b, a); break;
    case PFES_GRND_FAR:  e->grndFar.set(r, g, b, a); break;
    case PFES_GRND_NEAR: e->grndNear.set(r, g, b, a); break;
    default: return;
    }
    e->dirty = true;
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
    int ww = S.winW, wh = S.winH;
    if (S.window) SDL_GetWindowSize(S.window, &ww, &wh);
    if (xs) *xs = ww;
    if (ys) *ys = wh;
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

/* The main (3D scene) channel is whichever one pfChanScene designates —
 * perfly's GUI library creates ITS channel first, so creation order can't
 * pick it.  Until a scene is attached, the first channel acts as main. */
extern "C" pfChannel* pfNewChan(pfPipe*)
{
    PfOsgChan* c = new PfOsgChan;
    c->isMain = S.chans.empty();
    S.chans.push_back(c);
    return (pfChannel*)c;
}

static PfOsgChan* mainChan(void)
{
    for (PfOsgChan* c : S.chans)
        if (c->isMain) return c;
    return S.chans.empty() ? nullptr : S.chans[0];
}

/* legacy callers (and shim-internal code) pass &S or nullptr for "the
 * channel"; treat anything that isn't a registered channel as main */
static PfOsgChan* chanOf(pfChannel* ch)
{
    for (PfOsgChan* c : S.chans)
        if ((pfChannel*)c == ch) return c;
    return mainChan();
}

PfOsgChan* pfosgChanOf(pfChannel* ch) { return chanOf(ch); }

extern "C" void pfAddChan(pfPipeWindow*, pfChannel*) {}

/* frame statistics: the fstats handle IS the channel; class enables live on
 * the channel struct and pfDrawChanStats (pfosg_gui.cpp) reads them */
extern "C" pfFrameStats* pfGetChanFStats(pfChannel* ch)
{
    return (pfFrameStats*)chanOf(ch);
}

extern "C" unsigned int pfFStatsClass(pfFrameStats* fs, unsigned int mask,
                                      int val)
{
    PfOsgChan* c = chanOf((pfChannel*)fs);
    if (!c) return 0;
    unsigned prev = c->statsClasses;
    switch (val) {
    case PFSTATS_ON:      c->statsClasses |= mask; break;
    case PFSTATS_OFF:     c->statsClasses &= ~mask; break;
    case PFSTATS_SET:     c->statsClasses = mask; break;
    case PFSTATS_DEFAULT: c->statsClasses = 0x2; break;   /* PFTIMES */
    }
    return prev;
}

extern "C" void pfChanViewport(pfChannel* ch, float l, float r,
                               float b, float t)
{
    PfOsgChan* c = chanOf(ch);
    if (!c) return;
    c->vpL = l; c->vpR = r; c->vpB = b; c->vpT = t;
}

extern "C" void pfGetChanViewport(pfChannel* ch, float* l, float* r,
                                  float* b, float* t)
{
    PfOsgChan* c = chanOf(ch);
    if (l) *l = c ? c->vpL : 0;
    if (r) *r = c ? c->vpR : 1;
    if (b) *b = c ? c->vpB : 0;
    if (t) *t = c ? c->vpT : 1;
}

extern "C" void pfMakeOrthoChan(pfChannel* ch, float l, float r,
                                float b, float t)
{
    PfOsgChan* c = chanOf(ch);
    if (!c) return;
    c->ortho = true;
    c->orthoL = l; c->orthoR = r; c->orthoB = b; c->orthoT = t;
}

/* channel pixel queries are in SDL window points — the same space as
 * pfuMouse and pfGetPWinSize; only the shim's internal glViewport code
 * deals in HiDPI drawable pixels */
extern "C" void pfGetChanOrigin(pfChannel* ch, int* x, int* y)
{
    int ww = S.winW, wh = S.winH;
    if (S.window) SDL_GetWindowSize(S.window, &ww, &wh);
    PfOsgChan* c = chanOf(ch);
    if (x) *x = c ? (int)(c->vpL * ww) : 0;
    if (y) *y = c ? (int)(c->vpB * wh) : 0;
}

extern "C" void pfGetChanSize(pfChannel* ch, int* xs, int* ys)
{
    int ww = S.winW, wh = S.winH;
    if (S.window) SDL_GetWindowSize(S.window, &ww, &wh);
    PfOsgChan* c = chanOf(ch);
    if (xs) *xs = c ? (int)((c->vpR - c->vpL) * ww) : ww;
    if (ys) *ys = c ? (int)((c->vpT - c->vpB) * wh) : wh;
}

extern "C" void pfChanTravMode(pfChannel* ch, int trav, int mode)
{
    /* the GUI library turns its overlay on/off this way (pfuEnableGUI) */
    PfOsgChan* c = chanOf(ch);
    if (c && trav == PFTRAV_DRAW) c->drawOn = mode != 0;
}

extern "C" void pfGetChanOutputOrigin(pfChannel* ch, int* x, int* y)
{
    pfGetChanOrigin(ch, x, y);
}

extern "C" void pfGetChanOutputSize(pfChannel* ch, int* xs, int* ys)
{
    pfGetChanSize(ch, xs, ys);
}

extern "C" void pfChanScene(pfChannel* ch, pfScene* scene)
{
    S.scene = ((osg::Node*)scene)->asGroup();
    /* attaching a scene designates the main 3D channel */
    PfOsgChan* c = chanOf(ch);
    if (c) {
        for (PfOsgChan* o : S.chans) o->isMain = false;
        c->isMain = true;
    }
}

extern "C" void pfChanFOV(pfChannel*, float fovh, float fovv)
{
    S.fovH = fovh;
    S.fovV = fovv;
}

extern "C" void pfChanNearFar(pfChannel* ch, float nearDist, float farDist)
{
    /* per channel; applyChannel projects with the main channel's values
     * (the GUI channel sets -1..1 ortho depth, which must not leak into the
     * 3D projection while channel roles are still being assigned) */
    PfOsgChan* c = chanOf(ch);
    if (c) {
        c->nearD = nearDist;
        c->farD = farDist;
    }
    if (!c || c->isMain) {
        S.nearD = nearDist;
        S.farD = farDist;
    }
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

extern "C" void pfChanTravFunc(pfChannel* ch, int trav, pfChanFuncType func)
{
    /* stored per channel; the frame loop reads the main channel's DRAW func
     * (role assignment can change until pfChanScene runs) */
    PfOsgChan* c = chanOf(ch);
    if (!c) return;
    if (trav == PFTRAV_CULL) c->cullFunc = (PfosgChanFunc)func;
    if (trav == PFTRAV_DRAW) c->drawFunc = (PfosgChanFunc)func;
}

void pfosgRunAuxChannels(void)
{
    PfOsgChan* main = mainChan();
    bool any = false;
    for (PfOsgChan* c : S.chans)
        if (c->drawFunc) any = true;
    if (!any) return;

    int dw = 1, dh = 1;
    SDL_GL_GetDrawableSize(S.window, &dw, &dh);

    /* sandbox: raw fixed-function GL behind OSG's back, fully restored so
     * osg::State's cache stays truthful */
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    pfosgInDrawPhase = true;
    /* main channel's callback first (scene overlays), then aux overlays */
    for (int pass = 0; pass < 2; pass++)
        for (PfOsgChan* c : S.chans) {
            if ((pass == 0) != (c == main) || !c->drawFunc || !c->drawOn)
                continue;
            glViewport((int)(c->vpL * dw), (int)(c->vpB * dh),
                       (int)((c->vpR - c->vpL) * dw),
                       (int)((c->vpT - c->vpB) * dh));
            if (c->cullFunc) c->cullFunc((pfChannel*)c, nullptr);
            c->drawFunc((pfChannel*)c, nullptr);
        }
    pfosgInDrawPhase = false;

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopClientAttrib();
    glPopAttrib();
}

void pfosgCompileDirtyGSets(void)
{
    for (PfOsgGSet* g : S.gsets)
        if (g->dirty) compileGSet(g);
}

std::string pfosgResolveFile(const char* name)
{
    return resolveFile(name);
}

extern "C" void pfClearChan(pfChannel*) {}   /* viewer.frame() clears */
extern "C" void pfDraw(void) {}              /* viewer.frame() draws  */
/* pfDrawChanStats: real overlay in pfosg_gui.cpp */

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

extern "C" pfNode* pfdLoadFile_pfb(const char* fileName);
extern "C" pfNode* pfdLoadFile_pfa(const char* fileName);

extern "C" pfNode* pfdLoadFile(const char* fileName)
{
    std::string path = resolveFile(fileName);
    if (path.empty()) {
        pfNotify(PFNFY_WARN, PFNFY_PRINT,
                 "pfdLoadFile: could not find \"%s\"", fileName);
        return nullptr;
    }
    size_t dot = path.find_last_of('.');
    std::string ext = dot == std::string::npos ? "" : path.substr(dot + 1);

    /* SGI's shipped loader (pfpfb.c, compiled unmodified) is the default;
     * PFOSG_LOADER=pfb2osg selects the direct-to-OSG loader instead */
    static const char* pick = getenv("PFOSG_LOADER");
    bool useSgi = !(pick && strcmp(pick, "pfb2osg") == 0);
    if (ext == "pfa")
        return pfdLoadFile_pfa(path.c_str());
    if (ext == "pfb" && useSgi)
        return pfdLoadFile_pfb(path.c_str());

    osg::ref_ptr<osg::Node> node = pfb2osgLoadFile(path);
    if (!node) return nullptr;
    S.keep.push_back(node);
    return (pfNode*)node.get();
}
