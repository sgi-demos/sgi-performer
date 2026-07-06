/* ============================================================================
 *  pfosg_perfly.cpp - the shim surface perfly needs beyond the core:
 *
 *    1. linmath helpers implemented over osg::Matrix (real prmath.h decls)
 *    2. the pfu input system, fed by SDL events from pfFrame's pump
 *    3. a functional pfiXformer (trackball / fly / drive motion models)
 *    4. a large battalion of accept-and-ignore stubs: stats, calligraphics,
 *       compositors, video channels, GUI widgets, clip textures - IRIX
 *       hardware and multiprocess machinery with no equivalent here.
 *
 *  Definitions are compiled against the REAL pfutil.h/pfui.h declarations,
 *  so signatures are verified by the compiler.
 * ==========================================================================*/
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfui.h>
#include <Performer/pfdu.h>

/* pfui.h wraps its C API in self-casting convenience macros; they must not
 * rewrite our function DEFINITIONS */
#undef pfiSelectXformerModel
#undef pfiGetXformerCurModel
#undef pfiGetXformerCurModelIndex
#undef pfiStopXformer
#undef pfiResetXformerPosition
#undef pfiCenterXformer
#undef pfiXformerAutoInput
#undef pfiXformerMat
#undef pfiGetXformerMat
#undef pfiXformerCoord
#undef pfiGetXformerCoord
#undef pfiXformerResetCoord
#undef pfiXformerNode
#undef pfiXformerAutoPosition
#undef pfiEnableXformerCollision
#undef pfiDisableXformerCollision
#undef pfiCollideXformer
#undef pfiUpdateXformer
#undef pfiIXformFlyMode
#undef pfuCollideSetup
#undef pfuTravSetDListMode
#undef pfuTravCompileDLists
#undef pfuTravCreatePackedAttrs

#include "pfosg_internal.h"

#include <osg/Matrixd>
#include <osg/MatrixTransform>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static PfOsgState& S = pfosgState;

#define D2R(x) ((x) * (M_PI / 180.0))
#define R2D(x) ((x) * (180.0 / M_PI))

/* ==== 1. linmath helpers ==================================================== */

extern "C" {

void (pfSetVec4)(pfVec4 v, float x, float y, float z, float w)
{
    v[0] = x; v[1] = y; v[2] = z; v[3] = w;
}

void pfCombineVec3(pfVec3 dst, float a, const pfVec3 v1,
                   float b, const pfVec3 v2)
{
    dst[0] = a * v1[0] + b * v2[0];
    dst[1] = a * v1[1] + b * v2[1];
    dst[2] = a * v1[2] + b * v2[2];
}

float pfNormalizeVec3(pfVec3 v)
{
    float len = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (len > 0.0f) {
        v[0] /= len; v[1] /= len; v[2] /= len;
    }
    return len;
}

void pfMakeIdentMat(pfMatrix m)
{
    memset(m, 0, sizeof(pfMatrix));
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

void pfCopyMat(pfMatrix dst, const pfMatrix src)
{
    memcpy(dst, src, sizeof(pfMatrix));
}

void pfMakeRotMat(pfMatrix m, float degrees, float x, float y, float z)
{
    osg::Matrixf r = osg::Matrixf::rotate(D2R(degrees), osg::Vec3(x, y, z));
    memcpy(m, r.ptr(), sizeof(pfMatrix));
}

void pfMakeCoordMat(pfMatrix m, const pfCoord* c)
{
    /* row-vector: v * R(roll about Y, pitch about X, heading about Z) * T */
    osg::Matrixd rot =
        osg::Matrixd::rotate(D2R(c->hpr[2]), osg::Vec3d(0, 1, 0)) *
        osg::Matrixd::rotate(D2R(c->hpr[1]), osg::Vec3d(1, 0, 0)) *
        osg::Matrixd::rotate(D2R(c->hpr[0]), osg::Vec3d(0, 0, 1)) *
        osg::Matrixd::translate(c->xyz[0], c->xyz[1], c->xyz[2]);
    for (int r = 0; r < 4; r++)
        for (int cc = 0; cc < 4; cc++)
            m[r][cc] = (float)rot(r, cc);
}

void pfMakeVecRotVecMat(pfMatrix m, const pfVec3 v1, const pfVec3 v2)
{
    osg::Matrixd r = osg::Matrixd::rotate(
        osg::Vec3d(v1[0], v1[1], v1[2]), osg::Vec3d(v2[0], v2[1], v2[2]));
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m[i][j] = (float)r(i, j);
}

void pfPreMultMat(pfMatrix mat, const pfMatrix m)
{
    osg::Matrixf a((const float*)m), b((const float*)mat);
    osg::Matrixf out = a * b;               /* row-vector: pre = m * mat */
    memcpy(mat, out.ptr(), sizeof(pfMatrix));
}

void pfPostTransMat(pfMatrix dst, const pfMatrix m, float x, float y, float z)
{
    osg::Matrixf a((const float*)m);
    osg::Matrixf out = a * osg::Matrixf::translate(x, y, z);
    memcpy(dst, out.ptr(), sizeof(pfMatrix));
}

void pfPreRotMat(pfMatrix dst, float degrees, float x, float y, float z,
                 pfMatrix m)
{
    osg::Matrixf a((const float*)m);
    osg::Matrixf out = osg::Matrixf::rotate(D2R(degrees), osg::Vec3(x, y, z)) * a;
    memcpy(dst, out.ptr(), sizeof(pfMatrix));
}

void pfPreScaleMat(pfMatrix dst, float xs, float ys, float zs, pfMatrix m)
{
    osg::Matrixf a((const float*)m);
    osg::Matrixf out = osg::Matrixf::scale(xs, ys, zs) * a;
    memcpy(dst, out.ptr(), sizeof(pfMatrix));
}

}   /* extern "C" (math) */

/* ==== 2. pfu input system ===================================================== */

static pfuMouse pf_mouse;
static pfuEventStream pf_events;
static int pf_inputInited = 0;

static void mouseButtonBit(int sdlButton, int* bit)
{
    switch (sdlButton) {
        case SDL_BUTTON_LEFT:   *bit = PFUDEV_MOUSE_LEFT_DOWN;   break;
        case SDL_BUTTON_MIDDLE: *bit = PFUDEV_MOUSE_MIDDLE_DOWN; break;
        case SDL_BUTTON_RIGHT:  *bit = PFUDEV_MOUSE_RIGHT_DOWN;  break;
        default: *bit = 0; break;
    }
}

static int buttonIndex(int bit)
{
    /* pfuMouse clickPos arrays are indexed by button bit (1, 2, 4 -> 1, 2, 4
     * fits in [PFUDEV_MOUSE_DOWN_MASK][...] = [7][...]) */
    return bit & PFUDEV_MOUSE_DOWN_MASK;
}

static void queueDev(int dev, int val)
{
    if (pf_events.numDevs >= PFUDEV_MAX_INDEVS) return;
    pf_events.devQ[pf_events.numDevs] = dev;
    pf_events.devVal[pf_events.numDevs] = val;
    pf_events.numDevs++;
    if (dev >= 0 && dev < PFUDEV_MAX_DEVS)
        pf_events.devCount[dev]++;
}

static void queueKey(int key)
{
    if (key < 0 || key >= PFUDEV_KEY_MAP_SIZE) return;
    if (pf_events.numKeys < PFUDEV_MAX_INKEYS)
        pf_events.keyQ[pf_events.numKeys++] = key;
    pf_events.keyCount[key]++;
    queueDev(PFUDEV_KEYBD, key);
}

void pfosgInputBeginFrame(double /*now*/)
{
    pf_mouse.click = 0;
    pf_mouse.release = 0;
    if (S.window) {
        SDL_GetWindowSize(S.window, &pf_mouse.winSizeX, &pf_mouse.winSizeY);
        pf_mouse.inWin = 1;
    }
}

void pfosgInputSDLEvent(const SDL_Event& ev, double now)
{
    switch (ev.type) {
    case SDL_MOUSEMOTION:
        pf_mouse.xpos = ev.motion.x;
        pf_mouse.ypos = pf_mouse.winSizeY - ev.motion.y;   /* GL: y up */
        pf_mouse.posTime = now;
        break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
        int bit = 0;
        mouseButtonBit(ev.button.button, &bit);
        if (!bit) break;
        int x = ev.button.x;
        int y = pf_mouse.winSizeY - ev.button.y;
        int bi = buttonIndex(bit) - 1;
        if (bi < 0 || bi >= PFUDEV_MOUSE_DOWN_MASK) bi = 0;
        if (ev.type == SDL_MOUSEBUTTONDOWN) {
            pf_mouse.flags |= bit;
            pf_mouse.click |= bit;
            pf_mouse.clickPos[bi][0] = x;
            pf_mouse.clickPos[bi][1] = y;
            pf_mouse.clickPosLast[0] = x;
            pf_mouse.clickPosLast[1] = y;
            pf_mouse.clickTime[bi] = now;
            pf_mouse.clickTimeLast = now;
        } else {
            pf_mouse.flags &= ~bit;
            pf_mouse.release |= bit;
            pf_mouse.releasePos[bi][0] = x;
            pf_mouse.releasePos[bi][1] = y;
            pf_mouse.releasePosLast[0] = x;
            pf_mouse.releasePosLast[1] = y;
            pf_mouse.releaseTime[bi] = now;
            pf_mouse.releaseTimeLast = now;
        }
        break;
    }
    case SDL_KEYDOWN: {
        SDL_Keycode k = ev.key.keysym.sym;
        int mods = 0;
        SDL_Keymod sm = SDL_GetModState();
        if (sm & KMOD_SHIFT) mods |= PFUDEV_MOD_SHIFT;
        if (sm & KMOD_CTRL)  mods |= PFUDEV_MOD_CTRL;
        if (sm & KMOD_ALT)   mods |= PFUDEV_MOD_ALT;
        pf_mouse.modifiers = mods;
        pf_mouse.flags = (pf_mouse.flags & PFUDEV_MOUSE_DOWN_MASK) | mods;

        if (k == SDLK_ESCAPE)      queueKey(27);
        else if (k == SDLK_LEFT)   queueDev(PFUDEV_LEFTARROWKEY, 1);
        else if (k == SDLK_RIGHT)  queueDev(PFUDEV_RIGHTARROWKEY, 1);
        else if (k == SDLK_UP)     queueDev(PFUDEV_UPARROWKEY, 1);
        else if (k == SDLK_DOWN)   queueDev(PFUDEV_DOWNARROWKEY, 1);
        else if (k >= SDLK_F1 && k <= SDLK_F12)
            queueDev(PFUDEV_F1KEY + (k - SDLK_F1), 1);
        else if (k < 128) {
            int key = (int)k;
            /* apply shift for letters, as GL/X keyboards did */
            if (key >= 'a' && key <= 'z' && (sm & KMOD_SHIFT))
                key -= 'a' - 'A';
            queueKey(key);
        }
        break;
    }
    case SDL_WINDOWEVENT:
        if (ev.window.event == SDL_WINDOWEVENT_EXPOSED)
            queueDev(PFUDEV_REDRAW, 1);
        else if (ev.window.event == SDL_WINDOWEVENT_CLOSE)
            queueDev(PFUDEV_WINQUIT, 1);
        break;
    default:
        break;
    }
}

extern "C" {

void pfuInitInput(pfPipeWindow*, int /*mode*/) { pf_inputInited = 1; }
void pfuInitMultiChanInput(pfChannel**, int, int) { pf_inputInited = 1; }
void pfuExitInput(void) { pf_inputInited = 0; }
void pfuCollectInput(void) { /* pfFrame's SDL pump collects continuously */ }

void pfuGetMouse(pfuMouse* dst)
{
    *dst = pf_mouse;
    if (dst->winSizeX > 0 && dst->winSizeY > 0) {
        dst->xchan = 2.0f * dst->xpos / dst->winSizeX - 1.0f;
        dst->ychan = 2.0f * dst->ypos / dst->winSizeY - 1.0f;
    }
}

void pfuGetEvents(pfuEventStream* dst)
{
    *dst = pf_events;
    /* consumed: reset the live stream */
    memset(&pf_events, 0, sizeof(pf_events));
    pf_events.frameStamp = (int)S.frameCount;
}

void pfuMouseButtonClick(pfuMouse*, int, int, int, double) {}

}   /* extern "C" (input) */

/* ==== 3. pfiXformer ========================================================== */

struct PfiXf {
    int model = PFITDF_TRACKBALL;
    pfCoord coord;                  /* current position/orientation */
    pfCoord resetCoord;
    pfuMouse* mouse = nullptr;
    pfuEventStream* events = nullptr;
    pfChannel* chan = nullptr;
    pfNode* node = nullptr;         /* scene, for centering + collisions */
    pfDCS* dcs = nullptr;           /* trackball target (perfly's sceneDCS) */
    int collide = 0;
    float speed = 0.0f;
    float bsRadius = 100.0f;
    pfVec3 bsCenter;
    double lastTime = 0.0;
    int lastFlags = 0;
    int lastX = 0, lastY = 0;
    /* trackball state: rotation about the database center + dolly/pan */
    float tbH = 0.0f, tbP = 0.0f;
    osg::Vec3d tbTrans;
};

static int pfi_travelClassCookie;   /* pfiGetIXformTravelClassType identity */

static PfiXf* asXf(pfiXformer* xf) { return (PfiXf*)xf; }

extern "C" {

void pfiInit(void) {}
pfType* pfiGetIXformTravelClassType(void)
{
    return (pfType*)&pfi_travelClassCookie;
}
pfType* pfiGetIXformDriveClassType(void) { return (pfType*)0; }
void pfiIXformFlyMode(pfiInputXformFly*, int, int) {}

pfiTDFXformer* pfiNewTDFXformer(void*)
{
    PfiXf* xf = new PfiXf;
    pfSetVec3(xf->coord.xyz, 0, 0, 0);
    pfSetVec3(xf->coord.hpr, 0, 0, 0);
    xf->resetCoord = xf->coord;
    return (pfiTDFXformer*)xf;
}

void pfiSelectXformerModel(pfiXformer* xf, int which)
{
    asXf(xf)->model = which;
    asXf(xf)->speed = 0.0f;
}

pfiInputXform* pfiGetXformerCurModel(pfiXformer* xf)
{
    return (pfiInputXform*)xf;      /* opaque cookie; only passed back to us */
}

int pfiGetXformerCurModelIndex(pfiXformer* xf) { return asXf(xf)->model; }

void pfiStopXformer(pfiXformer* xf) { asXf(xf)->speed = 0.0f; }

void pfiResetXformerPosition(pfiXformer* xf)
{
    asXf(xf)->coord = asXf(xf)->resetCoord;
    asXf(xf)->speed = 0.0f;
}

void pfiXformerAutoInput(pfiXformer* xf, pfChannel* chan, pfuMouse* mouse,
                         pfuEventStream* events)
{
    asXf(xf)->chan = chan;
    asXf(xf)->mouse = mouse;
    asXf(xf)->events = events;
}

void pfiXformerMat(pfiXformer* xf, pfMatrix mat)
{
    /* set position from matrix: eye = row 3, orientation from rows */
    PfiXf* x = asXf(xf);
    pfSetVec3(x->coord.xyz, mat[3][0], mat[3][1], mat[3][2]);
    float h = R2D(atan2(-mat[1][0], mat[1][1]));
    float p = R2D(asin(mat[1][2] > 1 ? 1 : (mat[1][2] < -1 ? -1 : mat[1][2])));
    pfSetVec3(x->coord.hpr, h, p, 0.0f);
}

void pfiGetXformerMat(pfiXformer* xf, pfMatrix mat)
{
    pfMakeCoordMat(mat, &asXf(xf)->coord);
}

void pfiXformerCoord(pfiXformer* xf, pfCoord* coord)
{
    asXf(xf)->coord = *coord;
}

void pfiGetXformerCoord(pfiXformer* xf, pfCoord* coord)
{
    *coord = asXf(xf)->coord;
}

void pfiXformerResetCoord(pfiXformer* xf, pfCoord* resetPos)
{
    asXf(xf)->resetCoord = *resetPos;
}

void pfiXformerNode(pfiXformer* xf, pfNode* node)
{
    PfiXf* x = asXf(xf);
    x->node = node;
    pfSphere sph;
    if (node && (pfGetNodeBSphere)(node, &sph) && sph.radius > 0) {
        x->bsRadius = sph.radius;
        pfCopyVec3(x->bsCenter, sph.center);
    }
}

void pfiXformerAutoPosition(pfiXformer* xf, pfChannel*, pfDCS* dcs)
{
    asXf(xf)->dcs = dcs;            /* trackball target */
}

void pfiCenterXformer(pfiXformer* xf)
{
    PfiXf* x = asXf(xf);
    pfSetVec3(x->coord.xyz,
              x->bsCenter[0],
              x->bsCenter[1] - 2.0f * x->bsRadius,
              x->bsCenter[2] + 0.35f * x->bsRadius);
    pfSetVec3(x->coord.hpr, 0.0f, -8.0f, 0.0f);
    x->speed = 0.0f;
}

void pfiEnableXformerCollision(pfiXformer* xf)  { asXf(xf)->collide = 1; }
void pfiDisableXformerCollision(pfiXformer* xf) { asXf(xf)->collide = 0; }

int pfiCollideXformer(pfiXformer* xf)
{
    PfiXf* x = asXf(xf);
    if (!x->collide || !x->node) return 0;
    /* ground-follow: clamp eye above terrain */
    pfSegSet ss;
    pfHit** hits[PFIS_MAX_SEGS];
    memset(&ss, 0, sizeof(ss));
    ss.activeMask = 1;
    ss.isectMask = 0xFFFFFFFF;
    ss.mode = PFTRAV_IS_PRIM;
    pfSetVec3(ss.segs[0].pos, x->coord.xyz[0], x->coord.xyz[1],
              x->coord.xyz[2] + x->bsRadius);
    pfSetVec3(ss.segs[0].dir, 0, 0, -1);
    ss.segs[0].length = 2.0f * x->bsRadius;
    if ((pfNodeIsectSegs)(x->node, &ss, hits)) {
        pfVec3 p;
        pfQueryHit(*hits[0], PFQHIT_POINT, p);
        /* original drive height above ground: 2.0 units (pfiDrive.cxx) */
        float minz = p[2] + 2.0f;
        if (x->coord.xyz[2] < minz) {
            x->coord.xyz[2] = minz;
            return 1;
        }
    }
    return 0;
}

int pfiCollideXformerD(pfiXformer* xf) { return pfiCollideXformer(xf); }

void pfiUpdateXformer(pfiXformer* xf)
{
    PfiXf* x = asXf(xf);
    double now = pfGetTime();
    float dt = (float)(now - x->lastTime);
    x->lastTime = now;
    if (dt <= 0.0f || dt > 1.0f) dt = 1.0f / 60.0f;
    if (!x->mouse) return;

    pfuMouse* m = x->mouse;
    float xn = 0.0f, yn = 0.0f;      /* pointer, centered [-1,1], y up */
    if (m->winSizeX > 0 && m->winSizeY > 0) {
        xn = 2.0f * m->xpos / m->winSizeX - 1.0f;
        yn = 2.0f * m->ypos / m->winSizeY - 1.0f;
    }
    int lmb = m->flags & PFUDEV_MOUSE_LEFT_DOWN;
    int mmb = m->flags & PFUDEV_MOUSE_MIDDLE_DOWN;
    int rmb = m->flags & PFUDEV_MOUSE_RIGHT_DOWN;

    switch (x->model) {
    case PFITDF_TRACKBALL: {
        /* the TDF trackball transforms perfly's sceneDCS, not the eye:
         * LMB rotates the database about its center, MMB pans, RMB dollies */
        int dx = m->xpos - x->lastX;
        int dy = m->ypos - x->lastY;
        if (x->dcs && (lmb || mmb || rmb)) {
            if (lmb && !mmb) {
                x->tbH += dx * 0.4f;                 /* heading about Z */
                x->tbP += dy * 0.4f;                 /* pitch about X */
            }
            if (rmb) {
                /* dolly: move the database toward/away along view (+Y) */
                x->tbTrans.y() -= dy * 0.002 * x->bsRadius;
            }
            if (mmb) {
                x->tbTrans.x() += dx * 0.001 * x->bsRadius;
                x->tbTrans.z() += dy * 0.001 * x->bsRadius;
            }
            osg::Vec3d c(x->bsCenter[0], x->bsCenter[1], x->bsCenter[2]);
            osg::Matrixd mm =
                osg::Matrixd::translate(-c) *
                osg::Matrixd::rotate(D2R(x->tbP), osg::Vec3d(1, 0, 0)) *
                osg::Matrixd::rotate(D2R(x->tbH), osg::Vec3d(0, 0, 1)) *
                osg::Matrixd::translate(c) *
                osg::Matrixd::translate(x->tbTrans);
            ((osg::MatrixTransform*)(osg::Node*)x->dcs)->setMatrix(mm);
        }
        break;
    }
    case PFITDF_FLY:
    case PFITDF_DRIVE: {
        /* constants and control law transcribed from the shipped libpfui
         * sources (pfiDrive.cxx / pfiFly.cxx / pfiXformer.cxx):
         *  - accel is constant at startAccel = max(2, dbSize*0.0015) [drive]
         *    or max(5, dbSize*0.006) [fly]; LMB accelerates, RMB decelerates
         *    through zero into reverse; releasing keeps the speed (cruise)
         *  - a quick middle-click stops; steering only happens with focus
         *    (some button held)
         *  - pointer steering is (0.5-x)*|0.5-x| * angularVel(180 deg/s):
         *    quadratic deadening, no dead zone, 45 deg/s at window edge
         *  - fly pitch INTEGRATES the pointer's vertical deflection at
         *    0.75x the rate; drive decays pitch/roll by 0.95 per frame  */
        float dbSize = 2.0f * x->bsRadius;
        float startAccel, maxSpeed;
        if (x->model == PFITDF_DRIVE) {
            startAccel = dbSize * 0.0015f > 2.0f ? dbSize * 0.0015f : 2.0f;
            maxSpeed = dbSize * 2.0f;
        } else {
            startAccel = dbSize * 0.006f > 5.0f ? dbSize * 0.006f : 5.0f;
            maxSpeed = dbSize * 5.0f > 200.0f ? dbSize * 5.0f : 200.0f;
        }

        /* quick middle-click = stop (pfiHaveFastMouseClick) */
        int mi = PFUDEV_MOUSE_MIDDLE_DOWN - 1;
        if ((m->release & PFUDEV_MOUSE_MIDDLE_DOWN) &&
            m->releaseTime[mi] - m->clickTime[mi] < 0.3)
            x->speed = 0.0f;

        if (lmb && rmb) {
            /* MOTION_DIRECTION in the original; treat as coast */
        } else if (lmb) {
            x->speed += startAccel * dt;
        } else if (rmb) {
            x->speed -= startAccel * dt;   /* decelerate into reverse */
        }
        if (x->speed >  maxSpeed) x->speed =  maxSpeed;
        if (x->speed < -maxSpeed) x->speed = -maxSpeed;

        int focus = lmb || mmb || rmb;
        if (focus) {
            /* their x = (0.5 - icoord) in [-0.5, 0.5]; ours xn in [-1, 1] */
            float sx = -0.5f * xn;
            float sy = -0.5f * yn;
            sx *= fabsf(sx);
            sy *= fabsf(sy);
            x->coord.hpr[0] += sx * 180.0f * dt;
            if (x->model == PFITDF_FLY)
                x->coord.hpr[1] -= sy * 180.0f * 0.75f * dt; /* pointer up
                                                                = climb */
        }
        if (x->model == PFITDF_DRIVE || !focus) {
            /* level out (drive always; fly when coasting hands-off) */
            if (x->model == PFITDF_DRIVE) {
                x->coord.hpr[1] *= 0.95f;
                x->coord.hpr[2] *= 0.95f;
            }
        }

        float h = (float)D2R(x->coord.hpr[0]);
        float p = (float)D2R(x->coord.hpr[1]);
        pfVec3 fwd;
        pfSetVec3(fwd, -sinf(h) * cosf(p), cosf(h) * cosf(p), sinf(p));
        x->coord.xyz[0] += fwd[0] * x->speed * dt;
        x->coord.xyz[1] += fwd[1] * x->speed * dt;
        x->coord.xyz[2] += fwd[2] * x->speed * dt;
        break;
    }
    default:
        break;
    }

    x->lastX = m->xpos;
    x->lastY = m->ypos;
    x->lastFlags = m->flags;
}

}   /* extern "C" (xformer) */

/* ==== 4. stub battalion ====================================================== */

extern "C" {

/* --- X11 conveniences the samples use (Xlib macros originally) --- */
int DefaultScreen(void*)          { return 0; }
int ScreenCount(void*)            { return 1; }
int DisplayWidth(void*, int)      { return 1920; }
int DisplayHeight(void*, int)     { return 1080; }
int sysmp(int, ...)               { return 1; }

/* --- process / system --- */
int  pfMultipipe(int)                    { return 1; }
int  pfGetMultipipe(void)                { return 1; }
int  pfMultithreadParami(int, int, int)  { return 0; }
void pfMultithreadParamf(int, int, float) {}
void pfPhase(int)                        {}
int  pfGetPhase(void)                    { return PFPHASE_FREE_RUN; }
int  pfFieldRate(int)                    { return 1; }
int  pfGetFieldRate(void)                { return 1; }
float pfGetFrameRate(void)               { return 60.0f; }
int  pfGetFrameCount(void)               { return (int)S.frameCount; }
double pfGetFrameTimeStamp(void)         { return pfGetTime(); }
float pfGetVideoRate(void)               { return 60.0f; }
void pfVideoRate(float)                  {}
unsigned int pfGetMPBitmask(void)        { return 0; }
const char* pfGetMachString(void)        { return "pfosg"; }
int  pfQueryFeature(int, int* dst)       { if (dst) *dst = 0; return 0; }
int  pfMQueryFeature(int* which, int* dst)
{
    (void)which;
    if (dst) *dst = 0;
    return 0;
}
int  pfQuerySys(int, int* dst)           { if (dst) *dst = 0; return 0; }
int  pfIsectFunc(void (*)(void*))        { return 0; }
void pfStageConfigFunc(int, unsigned int, pfStageFuncType) {}
void pfConfigStage(int, unsigned int)    {}
void* pfCalloc(size_t n, size_t s, void*) { return calloc(n, s); }
void pfFree(void* p)                     { free(p); }
int  pfDelete(void*)                     { return 1; }   /* shim keeps refs */
int  pfGetId(void*)                      { return 0; }
int  pfIsOfType(void* obj, void* type)
{
    /* one real client: perfly asks whether the xformer's current model is a
     * travel model (fly/drive) to decide if the eye matrix drives the view */
    if (type == (void*)&pfi_travelClassCookie && obj) {
        int m = ((PfiXf*)obj)->model;
        return m == PFITDF_FLY || m == PFITDF_DRIVE;
    }
    return 0;
}
const char* pfGetFilePath(void)          { return ""; }
int  pfFindFile(const char* file, char path[PF_MAXSTRING], int)
{
    extern pfNode* pfdLoadFile(const char*);   /* uses same search rules */
    FILE* f = fopen(file, "rb");
    if (f) { fclose(f); strncpy(path, file, PF_MAXSTRING); return 1; }
    return 0;
}
void pfOverride(unsigned long long, int) {}
int  pfGetNotifyLevel(void);             /* defined in pfosg.cpp */

/* --- immediate mode / draw-callback helpers --- */
void pfPushState(void) {}
void pfPopState(void)  {}
void pfBasicState(void) {}
void pfMakeBasicState(void) {}
void pfPushMatrix(void) {}
void pfPushIdentMatrix(void) {}
void pfPopMatrix(void) {}
void pfMultMatrix(pfMatrix) {}
void pfRotate(int, float) {}
void pfClear(int, const pfVec4) {}
void pfCullFace(int) {}
void pfAntialias(int) {}
void pfApplyMtl(pfMaterial*) {}
void pfApplyLModel(pfLightModel*) {}
void pfLPoint(int) {}
void pfDrawString(const pfString*) {}

/* --- scene-graph extras --- */
int (pfRemoveChild)(pfGroup* g, pfNode* c)
{
    osg::Group* grp = ((osg::Node*)g)->asGroup();
    return grp && grp->removeChild((osg::Node*)c) ? 1 : 0;
}
pfNode* (pfGetChild)(pfGroup* g, int i)
{
    osg::Group* grp = ((osg::Node*)g)->asGroup();
    return (pfNode*)(grp && i >= 0 && i < (int)grp->getNumChildren()
                         ? grp->getChild(i) : nullptr);
}
int (pfGetNumChildren)(pfGroup* g)
{
    osg::Group* grp = ((osg::Node*)g)->asGroup();
    return grp ? (int)grp->getNumChildren() : 0;
}
int (pfGetNumParents)(pfNode* n)
{
    return (int)((osg::Node*)n)->getNumParents();
}
void (pfNodeBSphere)(pfNode*, pfSphere*, int) {}
void (pfNodeTravFuncs)(pfNode*, int, pfNodeTravFuncType, pfNodeTravFuncType) {}
void (pfNodeTravData)(pfNode*, int, void*) {}
pfNode* (pfGetTravNode)(pfTraverser*) { return nullptr; }
int  (pfFlatten)(pfNode*, int) { return 0; }
void (pfDCSMat)(pfDCS* dcs, pfMatrix m)
{
    osg::Matrixf mm((const float*)m);
    ((osg::MatrixTransform*)(osg::Node*)dcs)->setMatrix(mm);
}
void (pfDCSCoord)(pfDCS* dcs, pfCoord* c)
{
    pfMatrix m;
    pfMakeCoordMat(m, c);
    (pfDCSMat)(dcs, m);
}
void (pfGetDCSMat)(pfDCS* dcs, pfMatrix m)
{
    const osg::Matrixf mm =
        ((osg::MatrixTransform*)(osg::Node*)dcs)->getMatrix();
    memcpy(m, mm.ptr(), sizeof(pfMatrix));
}
pfPartition* pfNewPart(void) { return (pfPartition*)pfNewGroup(); }
void (pfBuildPart)(pfPartition*) {}
void (pfUpdatePart)(pfPartition*) {}
void (pfPartVal)(pfPartition*, int, float) {}
void (pfPartAttr)(pfPartition*, int, void*) {}
int (pfGetNumGSets)(pfGeode* g)
{
    osg::Geode* gd = ((osg::Node*)g)->asGeode();
    return gd ? (int)gd->getNumDrawables() : 0;
}
/* pfGetGSet / pfGetGSetAttrLists: real implementations in pfosg.cpp */
int pfGetGSetPrimType(pfGeoSet*) { return PFGS_TRIS; }
int pfGetGSetNumPrims(pfGeoSet*) { return 0; }
pfGeoState* pfGetGSetGState(pfGeoSet*) { return nullptr; }
pfGeoState* pfMakeBasicGState(pfGeoState* gs) { return gs; }
void pfCopy(void*, void*) {}
void pfPrint(void*, unsigned long long, int, void*) {}
pfList* pfNewList(int, int, void*) { return (pfList*)calloc(1, 64); }

/* --- materials / lights / fog --- */
pfMaterial* pfNewMtl(void*) { return (pfMaterial*)calloc(1, 64); }
void pfMtlColor(pfMaterial*, int, float, float, float) {}
void pfMtlColorMode(pfMaterial*, int, int) {}
pfLight* pfNewLight(void*) { return (pfLight*)calloc(1, 64); }
void pfLightPos(pfLight*, float, float, float, float) {}
void pfLightOn(pfLight*) {}
void pfLightOff(pfLight*) {}
void pfLightColor(pfLight*, int, float, float, float) {}
pfLightModel* pfNewLModel(void*) { return (pfLightModel*)calloc(1, 64); }
void pfLModelAmbient(pfLightModel*, float, float, float) {}
void pfLModelLocal(pfLightModel*, int) {}
void (pfLSourceColor)(pfLightSource*, int, float, float, float) {}
void (pfLSourcePos)(pfLightSource*, float, float, float, float) {}
pfFog* pfNewFog(void*) { return (pfFog*)calloc(1, 64); }
void pfFogType(pfFog*, int) {}
void pfFogColor(pfFog*, float, float, float) {}
void pfFogRange(pfFog*, float, float) {}
void pfESkyFog(pfEarthSky*, int, pfFog*) {}
pfHighlight* pfNewHlight(void*) { return (pfHighlight*)calloc(1, 64); }
void pfHlightMode(pfHighlight*, unsigned int) {}

/* --- strings / fonts (no font loader: pfdLoadFont_type1 returns NULL and
 *     perfly skips its 3D message) --- */
pfString* pfNewString(void*) { return (pfString*)calloc(1, 256); }
void pfStringFont(pfString*, pfFont*) {}
void pfStringMode(pfString*, int, int) {}
void pfStringColor(pfString*, float, float, float, float) {}
void pfStringString(pfString*, const char*) {}
void pfStringMat(pfString*, pfMatrix) {}
void pfFlattenString(pfString*) {}
const pfBox* pfGetStringBBox(pfString*)
{
    static pfBox b = { { -1, -1, -1 }, { 1, 1, 1 } };
    return &b;
}
int (pfAddString)(pfText*, pfString*) { return 1; }

/* --- frustum C API (prmath.h decls; enough for perfly's culling debug) --- */
struct PfFrustImpl {
    float nearD = 1.0f, farD = 10000.0f;
    float left = -1.0f, right = 1.0f, bot = -1.0f, top = 1.0f;
};
pfFrustum* pfNewFrust(void*) { return (pfFrustum*)new PfFrustImpl; }
void pfFrustNearFar(pfFrustum* fr, float n, float f)
{
    ((PfFrustImpl*)fr)->nearD = n;
    ((PfFrustImpl*)fr)->farD = f;
}
void pfGetFrustNearFar(const pfFrustum* fr, float* n, float* f)
{
    if (n) *n = ((const PfFrustImpl*)fr)->nearD;
    if (f) *f = ((const PfFrustImpl*)fr)->farD;
}
void pfGetFrustNear(const pfFrustum* fr, pfVec3 ll, pfVec3 lr,
                    pfVec3 ul, pfVec3 ur)
{
    const PfFrustImpl* x = (const PfFrustImpl*)fr;
    pfSetVec3(ll, x->left,  x->nearD, x->bot);
    pfSetVec3(lr, x->right, x->nearD, x->bot);
    pfSetVec3(ul, x->left,  x->nearD, x->top);
    pfSetVec3(ur, x->right, x->nearD, x->top);
}
void pfMakePerspFrust(pfFrustum* fr, float l, float r, float b, float t)
{
    PfFrustImpl* x = (PfFrustImpl*)fr;
    x->left = l; x->right = r; x->bot = b; x->top = t;
}
void pfMakeSimpleFrust(pfFrustum* fr, float fov)
{
    PfFrustImpl* x = (PfFrustImpl*)fr;
    float e = tanf((float)D2R(fov) * 0.5f) * x->nearD;
    x->left = -e; x->right = e; x->bot = -e; x->top = e;
}
void pfApplyFrust(const pfFrustum*) {}
int pfGetNotifyLevel_unused_(void);   /* real one lives in pfosg.cpp */

/* --- channel / pipe / window extras --- */
pfPipe* pfGetChanPipe(pfChannel*) { return pfGetPipe(0); }
pfPipeWindow* pfGetChanPWin(pfChannel*) { return (pfPipeWindow*)&pfosgState; }
pfPipeWindow* pfGetPipePWin(pfPipe*, int) { return (pfPipeWindow*)&pfosgState; }
int  pfGetPWinIndex(pfPipeWindow*) { return 0; }
void pfPWinIndex(pfPipeWindow*, int) {}
int  pfGetPWinType(pfPipeWindow*) { return 0; }
void pfGetPWinOrigin(pfPipeWindow*, int* x, int* y)
{
    if (x) *x = 0;
    if (y) *y = 0;
}
int  pfGetPWinScreen(pfPipeWindow*) { return 0; }
void pfClosePWin(pfPipeWindow*) {}
void pfPWinMode(pfPipeWindow*, int, int) {}
void pfPWinFBConfig(pfPipeWindow*, void*) {}
void pfPWinFBConfigAttrs(pfPipeWindow*, int*) {}
void pfPWinFBConfigId(pfPipeWindow*, int) {}
void pfSwapPWinBuffers(pfPipeWindow*)
{
    /* pfFrame swaps; DRAW-callback users calling this get a no-op */
}
int  pfGetPWinSelect(pfPipeWindow*) { return 1; }
void pfSelectPWin(pfPipeWindow*) {}
pfPipe* pfGetPWinPipe(pfPipeWindow*) { return pfGetPipe(0); }
void* pfGetPWinOverlayWin(pfPipeWindow*) { return nullptr; }
void pfSelectWin(pfWindow*) {}
void pfCloseWin(pfWindow*) {}
pfWindow* pfOpenNewNoPortWin(const char*, int) { return (pfWindow*)calloc(1, 16); }
int  pfGetPipeNum(void) { return 0; }
void pfPipeScreen(pfPipe*, int) {}
void pfGetPipeSize(pfPipe*, int* xs, int* ys)
{
    if (xs) *xs = pfosgState.winW;
    if (ys) *ys = pfosgState.winH;
}
const char* pfPipeWSConnectionName(pfPipe*, const char* name) { return name; }
int  pfGetPipeDrawCount(pfPipe*) { return 0; }
void pfPipeIncrementalStateChanNum(pfPipe*, int) {}
int  pfGetPipeNumMPClipTextures(pfPipe*) { return 0; }
pfMPClipTexture* pfGetPipeMPClipTexture(pfPipe*, int) { return nullptr; }
pfClipTexture* pfGetMPClipTextureClipTexture(pfMPClipTexture*) { return nullptr; }
void pfChanViewOffsets(pfChannel*, pfVec3, pfVec3) {}
void pfGetChanOffsetViewMat(pfChannel*, pfMatrix m) { pfMakeIdentMat(m); }
void pfChanViewport(pfChannel*, float, float, float, float) {}
void pfGetChanViewport(pfChannel*, float* l, float* r, float* b, float* t)
{
    if (l) *l = 0; if (r) *r = 1; if (b) *b = 0; if (t) *t = 1;
}
void pfGetChanOrigin(pfChannel*, int* x, int* y)
{
    if (x) *x = 0;
    if (y) *y = 0;
}
void pfGetChanSize(pfChannel*, int* xs, int* ys)
{
    int dw = pfosgState.winW, dh = pfosgState.winH;
    if (pfosgState.window) SDL_GL_GetDrawableSize(pfosgState.window, &dw, &dh);
    if (xs) *xs = dw;
    if (ys) *ys = dh;
}
void pfChanShare(pfChannel*, unsigned int) {}
unsigned int pfGetChanShare(pfChannel*) { return 0; }
void pfChanAutoAspect(pfChannel*, int) {}
void pfChanProjMode(pfChannel*, int) {}
void pfChanBinOrder(pfChannel*, int, int) {}
void pfChanLODAttr(pfChannel*, int, float) {}
float pfGetChanLODAttr(pfChannel*, int) { return 1.0f; }
void pfChanStressFilter(pfChannel*, float, float, float, float, float) {}
void pfChanStatsMode(pfChannel*, unsigned int, unsigned int) {}
pfFrameStats* pfGetChanFStats(pfChannel*)
{
    static int dummy;
    return (pfFrameStats*)&dummy;
}
void pfChanCullPtope(pfChannel*, void*) {}
void pfGetChanBaseFrust(pfChannel*, pfFrustum*) {}
void pfChanTravMode(pfChannel*, int, int) {}
int  pfGetChanTravMode(pfChannel*, int) { return 0; }
void pfChanTravMask(pfChannel*, int, unsigned int) {}
void pfMakeSimpleChan(pfChannel* chan, float fov) { pfChanFOV(chan, fov, -1.0f); }
void pfMakeOrthoChan(pfChannel*, float, float, float, float) {}
void pfAttachChan(pfChannel*, pfChannel*) {}
pfPipeVideoChannel* pfGetChanPVChan(pfChannel*)
{
    static int dummy;
    return (pfPipeVideoChannel*)&dummy;
}
void pfApp(void) {}
void pfCull(void) {}
unsigned int pfFStatsClass(pfFrameStats*, unsigned int, int) { return 0; }
unsigned int pfFStatsClassMode(pfFrameStats*, int, unsigned int, int) { return 0; }
void pfFStatsAttr(pfFrameStats*, int, float) {}
unsigned int pfGetFStatsClass(pfFrameStats*, unsigned int) { return 0; }
void pfFStatsCountNode(pfFrameStats*, int, pfNode*) {}
int pfChanNodeIsectSegs(pfChannel*, pfNode* node, pfSegSet* ss, pfHit** hits[])
{
    return (pfNodeIsectSegs)(node, ss, hits);
}

/* --- calligraphics (IRIX light-point hardware) --- */
pfCalligraphic* pfGetCurCallig(void) { return nullptr; }
pfCalligraphic* pfGetChanCurCallig(pfChannel*) { return nullptr; }
int  pfGetCalligChannel(pfCalligraphic*) { return -1; }
int  pfGetCalligBoardMemSize(int) { return 0; }
int  pfCalligInitBoard(int) { return 0; }
void pfCalligFilterSize(pfCalligraphic*, float, float) {}
void pfCalligDefocus(pfCalligraphic*, float) {}
void pfCalligRasterDefocus(pfCalligraphic*, float) {}
void pfCalligDrawTime(pfCalligraphic*, float) {}
void pfCalligZFootPrintSize(pfCalligraphic*, float) {}
void pfCalligSwapVME(int) {}
void pfChanCalligEnable(pfChannel*, int) {}

/* --- compositors / hyperpipes / video channels --- */
pfCompositor* pfNewCompositor(void) { return (pfCompositor*)calloc(1, 16); }
void pfCompositorAddChild(pfCompositor*, int) {}
void pfCompositorVal(pfCompositor*, int, float) {}
void pfCompositorMode(pfCompositor*, int, int) {}
int  pfGetCompositorMode(pfCompositor*, int) { return 0; }
void pfCompositorViewport(pfCompositor*, float, float, float, float) {}
void pfCompositorReconfig(pfCompositor*) {}
void pfCompositorMasterPipe(pfCompositor*, pfPipe*) {}
void pfCompositorChannelClipped(pfCompositor*, int, int) {}
void pfHyperpipe(int) {}
void pfHyperpipe2D(pfCompositor*) {}
void pfPVChanDVRMode(pfPipeVideoChannel*, int) {}
int  pfGetPVChanDVRMode(pfPipeVideoChannel*) { return PFPVC_DVR_OFF; }
void pfPVChanOutputSize(pfPipeVideoChannel*, int, int) {}
void pfGetPVChanOutputSize(pfPipeVideoChannel*, int* xs, int* ys)
{
    pfGetChanSize(nullptr, xs, ys);
}
void pfGetPVChanSize(pfPipeVideoChannel*, int* xs, int* ys)
{
    pfGetChanSize(nullptr, xs, ys);
}
void pfGetPVChanScale(pfPipeVideoChannel*, float* x, float* y)
{
    if (x) *x = 1.0f;
    if (y) *y = 1.0f;
}
void pfPVChanStressFilter(pfPipeVideoChannel*, float, float, float,
                          float, float, float) {}

/* --- pfd database utilities --- */
void pfdAddExtAlias(const char*, const char*) {}
void pfdBldrMode(int, int) {}
pfNode* pfdCleanTree(pfNode* n, pfuTravFuncType) { return n; }
void pfdCombineBillboards(pfNode*, int) {}
void pfdCombineLayers(pfNode*) {}
void pfdConverterMode(const char*, int, int) {}
pfNode* pfdFreezeTransforms(pfNode* n, pfuTravFuncType) { return n; }
const pfGeoState* pfdGetDefaultGState(void) { return pfNewGState(nullptr); }
pfFont* pfdLoadFont_type1(const char*, int) { return nullptr; }
void pfdMakeShared(pfNode*) {}
void pfdMakeSharedScene(pfScene*) {}
FILE* pfdOpenFile(const char* name) { return fopen(name, "r"); }
void pfdPrintSceneGraphStats(pfNode*, double) {}
int  pfdStoreFile(pfNode*, const char*) { return 0; }
void pfdFreeFileLoaders(void) {}

/* --- pfu utilities: process management, GUI, misc (accept and ignore) --- */
void pfuInit(void) {}
void pfuExitUtil(void) {}
void pfuInitDefaultProcessManager(void) {}
void pfuProcessManagerMode(int, int) {}
void pfuReconfigureProcessManager(void) {}
int  pfuLockDownProc(int) { return 0; }
int  pfuFreeAllCPUs(void) { return 0; }
void pfuConfigMCO(pfChannel**, int) {}
pfFBConfig pfuChooseFBConfig(Display*, int, int*, void*) { return 0; }
void pfuGetGLXWin(pfPipe*, pfuGLXWindow*) {}
pfuGLXWindow* pfuGLXWinopen(pfPipe*, pfPipeWindow* pw, const char*)
{
    /* "GLX window" == our SDL window; opening it is what matters */
    pfOpenPWin(pw);
    static int cookie;
    return (pfuGLXWindow*)&cookie;
}
void pfuCollideSetup(pfNode*, int, int) {}
int  pfuAddFile(pfuPath*, char*) { return 0; }
pfList* pfuMakeSceneTexList(pfScene*) { return nullptr; }
void pfuDownloadTexList(pfList*, int) {}
void pfuLoadDetailTextures(pfList*, pfuDetailInfo*, int) {}
int  pfuSaveImage(char*, int, int, int, int, int) { return 0; }
void pfuDrawMessage(pfChannel*, const char*, int, int, float, float,
                    int, int) {}
void pfuDrawMessageCI(pfChannel*, const char*, int, int, float, float,
                      int, int, int) {}
void pfuDrawTree(pfChannel*, pfNode*, pfVec3) {}
int  pfuFollowPath(pfuPath*, float, pfVec3, pfVec3) { return 0; }
pfuPath* pfuNewPath(void) { return nullptr; }
pfuPath* pfuDeletePath(pfuPath*) { return nullptr; }
void pfuTravSetDListMode(pfNode*, int) {}
void pfuTravCompileDLists(pfNode*, int) {}
void pfuTravCreatePackedAttrs(pfNode*, int, int) {}
void pfuDrawChanDVRBox(pfChannel*) {}
void pfuDrawPWin2DCursor(pfPipeWindow*, int, int) {}
void pfuMapWinColors(pfWindow*, pfVec3*, int, int) {}
void pfuAddMPClipTexturesToPipes(pfList*, pfPipe*, pfPipe*[]) {}
void pfuProcessClipCenters(pfNode*, pfList*) {}
void pfuProcessClipCentersWithChannel(pfNode*, pfList*, pfChannel*) {}
int  pfuGridifyClipTexture(pfClipTexture*) { return 0; }
int  pfuUnGridifyClipTexture(pfClipTexture*) { return 0; }

/* GUI: no panel is drawn; widgets store their values so the callers'
 * get/set round trips behave */
struct PfuWidgetImpl {
    int id = 0;
    int type = 0;
    int on = 0;
    float value = 0, defValue = 0, minv = 0, maxv = 1;
    char label[128];
    void (*actionFunc)(pfuWidget*);
};
void pfuInitGUI(pfPipeWindow*) {}
void pfuExitGUI(void) {}
void pfuEnableGUI(int) {}
void pfuUpdateGUI(pfuMouse*) {}
void pfuRedrawGUI(void) {}
void pfuResetGUI(void) {}
int  pfuInGUI(int, int) { return 0; }
void pfuGUIViewport(float, float, float, float) {}
void pfuGetGUIViewport(float* l, float* r, float* b, float* t)
{
    if (l) *l = 0; if (r) *r = 1; if (b) *b = 0; if (t) *t = 0.15f;
}
pfChannel* pfuGetGUIChan(void) { return nullptr; }
pfHighlight* pfuGetGUIHlight(void)
{
    static pfHighlight* hl = (pfHighlight*)calloc(1, 64);
    return hl;
}
void pfuGUICursor(int, int) {}
void pfuUpdateGUICursor(void) {}
void pfuCursorType(int) {}
int  pfuGetCursorType(void) { return 0; }
pfuPanel* pfuNewPanel(void) { return (pfuPanel*)calloc(1, 32); }
void pfuEnablePanel(pfuPanel*) {}
void pfuGetPanelOriginSize(pfuPanel*, float* x, float* y, float* xs, float* ys)
{
    if (x) *x = 0; if (y) *y = 0; if (xs) *xs = 0; if (ys) *ys = 0;
}
pfuWidget* pfuNewWidget(pfuPanel*, int type, int id)
{
    PfuWidgetImpl* w = new PfuWidgetImpl;
    w->type = type;
    w->id = id;
    return (pfuWidget*)w;
}
void pfuWidgetDim(pfuWidget* w, int, int, int, int) { (void)w; }
void pfuGetWidgetDim(pfuWidget*, int* x, int* y, int* xs, int* ys)
{
    if (x) *x = 0; if (y) *y = 0; if (xs) *xs = 0; if (ys) *ys = 0;
}
void pfuWidgetLabel(pfuWidget* w, const char* label)
{
    strncpy(((PfuWidgetImpl*)w)->label, label ? label : "",
            sizeof(((PfuWidgetImpl*)w)->label) - 1);
}
void pfuWidgetRange(pfuWidget* w, int, float minv, float maxv, float val)
{
    PfuWidgetImpl* wi = (PfuWidgetImpl*)w;
    wi->minv = minv; wi->maxv = maxv; wi->value = val;
}
void pfuWidgetValue(pfuWidget* w, float v) { ((PfuWidgetImpl*)w)->value = v; }
float pfuGetWidgetValue(pfuWidget* w) { return ((PfuWidgetImpl*)w)->value; }
void pfuWidgetDefaultValue(pfuWidget* w, float v)
{
    ((PfuWidgetImpl*)w)->defValue = v;
}
void pfuWidgetOnOff(pfuWidget* w, int on) { ((PfuWidgetImpl*)w)->on = on; }
void pfuWidgetDefaultOnOff(pfuWidget* w, int on)
{
    ((PfuWidgetImpl*)w)->on = on;
}
int  pfuIsWidgetOn(pfuWidget* w) { return ((PfuWidgetImpl*)w)->on; }
void pfuWidgetActionFunc(pfuWidget* w, pfuWidgetActionFuncType func)
{
    ((PfuWidgetImpl*)w)->actionFunc = (void (*)(pfuWidget*))func;
}
pfuWidgetActionFuncType pfuGetWidgetActionFunc(pfuWidget* w)
{
    return (pfuWidgetActionFuncType)((PfuWidgetImpl*)w)->actionFunc;
}
int  pfuGetWidgetId(pfuWidget* w) { return ((PfuWidgetImpl*)w)->id; }
void pfuWidgetSelections(pfuWidget*, pfuGUIString*, int*,
                         void (**)(pfuWidget*), int) {}
int  pfuGetWidgetSelection(pfuWidget*) { return 0; }
void pfuHideWidget(pfuWidget*) {}
void pfuUnhideWidget(pfuWidget*) {}
void pfuEnableWidget(pfuWidget*) {}
void pfuDisableWidget(pfuWidget*) {}
void pfuPreDrawStyle(int, pfVec4) {}
void pfuPostDrawStyle(int) {}

}   /* extern "C" (stubs) */
