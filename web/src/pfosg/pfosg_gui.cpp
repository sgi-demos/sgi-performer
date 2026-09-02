/* pfosg_gui.cpp - support for SGI's libpfutil gui.c, compiled unmodified.
 *
 * The GUI panel draws with immediate-mode GL inside the aux channel's DRAW
 * callback (see pfosgRunAuxChannels in pfosg.cpp).  This file supplies what
 * gui.c links against beyond core shim functions: immediate-mode libpr state
 * and matrix calls (real GL only during the draw phase), heap-backed data
 * pools and pfLists, node class-type queries, an X-font replacement that
 * renders a built-in 5x7 bitmap font as GL quads, and no-op cursors/popups.
 */

#include <Performer/pf.h>
#include <Performer/pfutil.h>

/* pfutil.h wraps parts of its C API in self-casting macros; keep our
 * definitions un-rewritten */
#undef pfuDrawString
#undef pfuDrawStringPos

#include "pfosg_internal.h"

#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Switch>

#include <GL/gl.h>                 /* desktop GL; on wasm, emscripten's GL
                                    * emulation header (immediate-mode entry
                                    * points are no-op'd in pfosg_gles_compat.cpp) */

#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

bool pfosgInDrawPhase = false;

/* ---- immediate-mode libpr state / matrices -------------------------------- */

extern "C" void pfPushState(void)
{
    if (pfosgInDrawPhase) glPushAttrib(GL_ALL_ATTRIB_BITS);
}

extern "C" void pfPopState(void)
{
    if (pfosgInDrawPhase) glPopAttrib();
}

extern "C" void pfBasicState(void)
{
    if (!pfosgInDrawPhase) return;
#ifndef PFOSG_GLES2
    /* fixed-function enums: GL_INVALID_ENUM per call on GLES2 */
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_COLOR_MATERIAL);
#endif
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glShadeModel(GL_SMOOTH);
}

extern "C" void pfMakeBasicState(void) { pfBasicState(); }

extern "C" void pfPushMatrix(void)
{
    if (pfosgInDrawPhase) glPushMatrix();
}

extern "C" void pfPushIdentMatrix(void)
{
    if (pfosgInDrawPhase) { glPushMatrix(); glLoadIdentity(); }
}

extern "C" void pfPopMatrix(void)
{
    if (pfosgInDrawPhase) glPopMatrix();
}

extern "C" void pfMultMatrix(pfMatrix m)
{
    if (pfosgInDrawPhase) glMultMatrixf(&m[0][0]);
}

extern "C" void pfRotate(int axis, float degrees)
{
    if (pfosgInDrawPhase)
        glRotatef(degrees, axis == PF_X ? 1.f : 0.f,
                  axis == PF_Y ? 1.f : 0.f, axis == PF_Z ? 1.f : 0.f);
}

extern "C" void pfScale(float x, float y, float z)
{
    if (pfosgInDrawPhase) glScalef(x, y, z);
}

extern "C" void pfTranslate(float x, float y, float z)
{
    if (pfosgInDrawPhase) glTranslatef(x, y, z);
}

extern "C" void pfClear(int which, const pfVec4 col)
{
    if (!pfosgInDrawPhase) return;
    GLbitfield mask = 0;
    if (which & PFCL_COLOR) {
        if (col) glClearColor(col[0], col[1], col[2], col[3]);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (which & PFCL_DEPTH) mask |= GL_DEPTH_BUFFER_BIT;
    if (!mask) return;
    /* Performer clears the channel's viewport, not the window */
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glPushAttrib(GL_SCISSOR_BIT);
    glEnable(GL_SCISSOR_TEST);
    glScissor(vp[0], vp[1], vp[2], vp[3]);
    glClear(mask);
    glPopAttrib();
}

/* ---- data pools (single process: plain heap, id-keyed) -------------------- */

static std::map<int, void*>& dpoolMap()
{
    static std::map<int, void*> m;
    return m;
}

extern "C" pfDataPool* pfuGetUtilDPool(void)
{
    static int cookie;
    return (pfDataPool*)&cookie;
}

extern "C" volatile void* pfDPoolAlloc(pfDataPool*, unsigned int size, int id)
{
    void*& slot = dpoolMap()[id];
    if (!slot) slot = calloc(1, size);
    return slot;
}

extern "C" volatile void* pfDPoolFind(pfDataPool*, int id)
{
    /* real Performer's pfuInitUtil pre-allocates zeroed entries for the GUI
     * ids at datapool creation (libpfutil shmem.c); mirror that by minting a
     * zeroed block on first lookup — generously sized, since the entry
     * structs (pfuGUI etc.) are private to their libraries */
    void*& slot = dpoolMap()[id];
    if (!slot) slot = calloc(1, 64 * 1024);
    return slot;
}

extern "C" int pfDPoolFree(pfDataPool*, void* mem)
{
    for (auto it = dpoolMap().begin(); it != dpoolMap().end(); ++it)
        if (it->second == mem) {
            free(mem);
            dpoolMap().erase(it);
            return 1;
        }
    return 0;
}

extern "C" int pfDPoolLock(void*) { return 1; }
extern "C" void pfDPoolUnlock(void*) {}

/* ---- pfList / pfPath ------------------------------------------------------- */

struct PfOsgList {
    unsigned magic = 0x1f11f7ed;
    std::vector<void*> v;
};

static PfOsgList* asList(const void* l) { return (PfOsgList*)l; }

extern "C" pfList* pfNewList(int, int, void*) { return (pfList*)new PfOsgList; }
extern "C" pfPath* pfNewPath(void) { return (pfPath*)new PfOsgList; }
extern "C" void pfResetList(pfList* l) { if (l) asList(l)->v.clear(); }
extern "C" void pfAdd(pfList* l, void* elt)
{
    if (l) asList(l)->v.push_back(elt);
}
extern "C" void* pfGet(const pfList* l, int i)
{
    if (!l || i < 0 || i >= (int)asList(l)->v.size()) return nullptr;
    return asList(l)->v[(size_t)i];
}
extern "C" int pfGetNum(const pfList* l)
{
    return l ? (int)asList(l)->v.size() : 0;
}
extern "C" int pfSearch(const pfList* l, void* elt)
{
    if (!l) return -1;
    auto& v = asList(l)->v;
    for (size_t i = 0; i < v.size(); i++)
        if (v[i] == elt) return (int)i;
    return -1;
}
extern "C" int pfCopy(void* dst, void* src)
{
    /* gui.c only copies pfPaths (node-pointer lists) */
    if (dst && src && asList(dst)->magic == 0x1f11f7ed &&
        asList(src)->magic == 0x1f11f7ed) {
        asList(dst)->v = asList(src)->v;
        return 1;
    }
    return 0;
}

/* ---- node class types / queries -------------------------------------------- */

static int nodeTypeCookie, groupTypeCookie, geodeTypeCookie,
           scsTypeCookie, switchTypeCookie;

extern "C" pfType* pfGetNodeClassType(void)   { return (pfType*)&nodeTypeCookie; }
extern "C" pfType* pfGetGroupClassType(void)  { return (pfType*)&groupTypeCookie; }
extern "C" pfType* pfGetGeodeClassType(void)  { return (pfType*)&geodeTypeCookie; }
extern "C" pfType* pfGetSCSClassType(void)    { return (pfType*)&scsTypeCookie; }
extern "C" pfType* pfGetSwitchClassType(void) { return (pfType*)&switchTypeCookie; }

/* extends pfIsOfType (pfosg_perfly.cpp) for scene-graph class cookies;
 * returns -1 when the cookie isn't one of ours */
int pfosgIsOfNodeClass(void* obj, void* type)
{
    osg::Node* n = (osg::Node*)obj;
    if (type == &nodeTypeCookie) return 1;
    if (type == &geodeTypeCookie)
        return dynamic_cast<osg::Geode*>(n) != nullptr;
    if (type == &groupTypeCookie)
        /* In modern OSG (3.4+) osg::Geode derives from osg::Group, so
         * asGroup() is non-null for a Geode.  Performer semantics — and
         * the pfb loader, which tests Group before Geode — require a Geode
         * to NOT be a Group, so exclude it explicitly. */
        return n->asGroup() != nullptr &&
               dynamic_cast<osg::Geode*>(n) == nullptr;
    if (type == &scsTypeCookie)
        return dynamic_cast<osg::MatrixTransform*>(n) != nullptr;
    if (type == &switchTypeCookie)
        return dynamic_cast<osg::Switch*>(n) != nullptr;
    return -1;
}

extern "C" const char* pfGetTypeName(const void* data)
{
    return data ? ((const osg::Object*)data)->className() : "NULL";
}

extern "C" pfGroup* (pfGetParent)(const pfNode* node, int i)
{
    const osg::Node* n = (const osg::Node*)node;
    if (!n || i < 0 || i >= (int)n->getNumParents()) return nullptr;
    return (pfGroup*)n->getParent((unsigned)i);
}

extern "C" void (pfGetSCSMat)(pfSCS* scs, pfMatrix m)
{
    const osg::Matrix& om = ((osg::MatrixTransform*)scs)->getMatrix();
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            m[r][c] = (float)om(r, c);
}

extern "C" float pfGetSwitchVal(const pfSwitch* sw)
{
    const osg::Switch* s = (const osg::Switch*)sw;
    for (unsigned i = 0; i < s->getNumChildren(); i++)
        if (s->getValue(i)) return (float)i;
    return -1.0f;   /* PFSWITCH_OFF */
}

/* ---- misc channel / window / highlight -------------------------------------- */

extern "C" int pfGetHyperpipe(pfPipe*) { return 0; }

extern "C" pfWindow* pfGetCurWin(void)
{
    return (pfWindow*)&pfosgState;
}

extern "C" void pfGetWinSize(const pfWindow*, int* xs, int* ys)
{
    /* window points, like pfGetPWinSize and pfuMouse */
    int ww = pfosgState.winW, wh = pfosgState.winH;
    if (pfosgState.window)
        SDL_GetWindowSize(pfosgState.window, &ww, &wh);
    if (xs) *xs = ww;
    if (ys) *ys = wh;
}

extern "C" int pfChanPick(pfChannel*, int, float, float, float,
                          pfHit** pickList[])
{
    if (pickList) *pickList = nullptr;
    return 0;
}

extern "C" void pfApplyHlight(pfHighlight*) {}
extern "C" void pfHlightColor(pfHighlight*, unsigned int, float, float, float) {}
extern "C" void pfHlightLineWidth(pfHighlight*, float) {}
extern "C" void pfHlightPntSize(pfHighlight*, float) {}

/* ---- X stubs: fonts exist (one fake match), cursors don't ------------------- */

extern "C" char** XListFonts(Display*, const char*, int, int* count)
{
    static char name[] = "pfosg-builtin";
    static char* list[] = { name, nullptr };
    if (count) *count = 1;
    return list;
}

extern "C" int XFreeFontNames(char**) { return 0; }
extern "C" int XFreeFont(Display*, void*) { return 0; }

extern "C" void* pfuCreateCursor(void* index) { return index; }
extern "C" void pfuLoadPWinCursor(pfPipeWindow*, int) {}

/* ---- popup menus (IRIS GL "pup": not supported, never selects) -------------- */

extern "C" long defpup(pfWSConnection, int, char*, ...) { return 1; }
extern "C" void addtopup(long, char*, ...) {}
extern "C" long dopup(long) { return -1; }
extern "C" void freepup(long) {}

/* ---- prmath leftovers -------------------------------------------------------- */

extern "C" void pfSetVec2(pfVec2 dst, float x, float y)
{
    dst[0] = x;
    dst[1] = y;
}

/* ---- built-in 5x7 font ------------------------------------------------------ */

/* Classic 5x7 upper-left-origin glyphs for ASCII 32..126; each glyph is five
 * column bytes, LSB = top row.  Rendered as chunky GL quads: authentic
 * enough for a 1990s vis-sim control panel. */
static const unsigned char FONT5X7[95][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /*   */ {0x00,0x00,0x5f,0x00,0x00}, /* ! */
    {0x00,0x07,0x00,0x07,0x00}, /* " */ {0x14,0x7f,0x14,0x7f,0x14}, /* # */
    {0x24,0x2a,0x7f,0x2a,0x12}, /* $ */ {0x23,0x13,0x08,0x64,0x62}, /* % */
    {0x36,0x49,0x55,0x22,0x50}, /* & */ {0x00,0x05,0x03,0x00,0x00}, /* ' */
    {0x00,0x1c,0x22,0x41,0x00}, /* ( */ {0x00,0x41,0x22,0x1c,0x00}, /* ) */
    {0x14,0x08,0x3e,0x08,0x14}, /* * */ {0x08,0x08,0x3e,0x08,0x08}, /* + */
    {0x00,0x50,0x30,0x00,0x00}, /* , */ {0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x00,0x60,0x60,0x00,0x00}, /* . */ {0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x3e,0x51,0x49,0x45,0x3e}, /* 0 */ {0x00,0x42,0x7f,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */ {0x21,0x41,0x45,0x4b,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7f,0x10}, /* 4 */ {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3c,0x4a,0x49,0x49,0x30}, /* 6 */ {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */ {0x06,0x49,0x49,0x29,0x1e}, /* 9 */
    {0x00,0x36,0x36,0x00,0x00}, /* : */ {0x00,0x56,0x36,0x00,0x00}, /* ; */
    {0x08,0x14,0x22,0x41,0x00}, /* < */ {0x14,0x14,0x14,0x14,0x14}, /* = */
    {0x00,0x41,0x22,0x14,0x08}, /* > */ {0x02,0x01,0x51,0x09,0x06}, /* ? */
    {0x32,0x49,0x79,0x41,0x3e}, /* @ */ {0x7e,0x11,0x11,0x11,0x7e}, /* A */
    {0x7f,0x49,0x49,0x49,0x36}, /* B */ {0x3e,0x41,0x41,0x41,0x22}, /* C */
    {0x7f,0x41,0x41,0x22,0x1c}, /* D */ {0x7f,0x49,0x49,0x49,0x41}, /* E */
    {0x7f,0x09,0x09,0x09,0x01}, /* F */ {0x3e,0x41,0x49,0x49,0x7a}, /* G */
    {0x7f,0x08,0x08,0x08,0x7f}, /* H */ {0x00,0x41,0x7f,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3f,0x01}, /* J */ {0x7f,0x08,0x14,0x22,0x41}, /* K */
    {0x7f,0x40,0x40,0x40,0x40}, /* L */ {0x7f,0x02,0x0c,0x02,0x7f}, /* M */
    {0x7f,0x04,0x08,0x10,0x7f}, /* N */ {0x3e,0x41,0x41,0x41,0x3e}, /* O */
    {0x7f,0x09,0x09,0x09,0x06}, /* P */ {0x3e,0x41,0x51,0x21,0x5e}, /* Q */
    {0x7f,0x09,0x19,0x29,0x46}, /* R */ {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7f,0x01,0x01}, /* T */ {0x3f,0x40,0x40,0x40,0x3f}, /* U */
    {0x1f,0x20,0x40,0x20,0x1f}, /* V */ {0x3f,0x40,0x38,0x40,0x3f}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */ {0x07,0x08,0x70,0x08,0x07}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */ {0x00,0x7f,0x41,0x41,0x00}, /* [ */
    {0x02,0x04,0x08,0x10,0x20}, /* \ */ {0x00,0x41,0x41,0x7f,0x00}, /* ] */
    {0x04,0x02,0x01,0x02,0x04}, /* ^ */ {0x40,0x40,0x40,0x40,0x40}, /* _ */
    {0x00,0x01,0x02,0x04,0x00}, /* ` */ {0x20,0x54,0x54,0x54,0x78}, /* a */
    {0x7f,0x48,0x44,0x44,0x38}, /* b */ {0x38,0x44,0x44,0x44,0x20}, /* c */
    {0x38,0x44,0x44,0x48,0x7f}, /* d */ {0x38,0x54,0x54,0x54,0x18}, /* e */
    {0x08,0x7e,0x09,0x01,0x02}, /* f */ {0x0c,0x52,0x52,0x52,0x3e}, /* g */
    {0x7f,0x08,0x04,0x04,0x78}, /* h */ {0x00,0x44,0x7d,0x40,0x00}, /* i */
    {0x20,0x40,0x44,0x3d,0x00}, /* j */ {0x7f,0x10,0x28,0x44,0x00}, /* k */
    {0x00,0x41,0x7f,0x40,0x00}, /* l */ {0x7c,0x04,0x18,0x04,0x78}, /* m */
    {0x7c,0x08,0x04,0x04,0x78}, /* n */ {0x38,0x44,0x44,0x44,0x38}, /* o */
    {0x7c,0x14,0x14,0x14,0x08}, /* p */ {0x08,0x14,0x14,0x18,0x7c}, /* q */
    {0x7c,0x08,0x04,0x04,0x08}, /* r */ {0x48,0x54,0x54,0x54,0x20}, /* s */
    {0x04,0x3f,0x44,0x40,0x20}, /* t */ {0x3c,0x40,0x40,0x20,0x7c}, /* u */
    {0x1c,0x20,0x40,0x20,0x1c}, /* v */ {0x3c,0x40,0x30,0x40,0x3c}, /* w */
    {0x44,0x28,0x10,0x28,0x44}, /* x */ {0x0c,0x50,0x50,0x50,0x3c}, /* y */
    {0x44,0x64,0x54,0x4c,0x44}, /* z */ {0x00,0x08,0x36,0x41,0x00}, /* { */
    {0x00,0x00,0x7f,0x00,0x00}, /* | */ {0x00,0x41,0x36,0x08,0x00}, /* } */
    {0x08,0x08,0x2a,0x1c,0x08}, /* ~ */
};

static pfuXFont pfosgCurFont = { 12, 12, (HFONT)1 };
static float pfosgRasterX = 0, pfosgRasterY = 0, pfosgRasterZ = 0;

/* Map the 5x7 glyph (6-unit advance) onto an X "pixelsize N" font.  gui.c
 * asks for sizes pre-inflated by its 1.6x "X11 fudge factor", and the panel
 * layout was tuned for fairly dense helvetica text, so the divisor is larger
 * than the naive glyph height. */
static float fontScale(const pfuXFont* f)
{
    int size = f && f->size > 0 ? f->size : 12;
    return (float)size / 16.0f;
}

static void drawStringAt(const char* s, float x, float y, float z)
{
    if (!s || !*s) return;
    float sc = fontScale(&pfosgCurFont);
    float px = sc;                     /* one glyph pixel */
    glBegin(GL_QUADS);
    for (; *s; s++, x += 6.0f * sc) {
        unsigned ch = (unsigned char)*s;
        if (ch < 32 || ch > 126) continue;
        const unsigned char* g = FONT5X7[ch - 32];
        for (int col = 0; col < 5; col++)
            for (int row = 0; row < 7; row++)
                if (g[col] & (1 << row)) {
                    /* row 0 = glyph top; baseline sits at y */
                    float gx = x + col * px;
                    float gy = y + (6 - row) * px;
                    glVertex3f(gx, gy, z);
                    glVertex3f(gx + px, gy, z);
                    glVertex3f(gx + px, gy + px, z);
                    glVertex3f(gx, gy + px, z);
                }
    }
    glEnd();
    /* X font drawing advances the raster position; gui.c relies on it to
     * append a slider's value text right after its label */
    pfosgRasterX = x;
    pfosgRasterY = y;
    pfosgRasterZ = z;
}

extern "C" void pfuLoadXFont(char*, pfuXFont* fnt)
{
    if (!fnt) return;
    if (fnt->size <= 0) fnt->size = 12;
    fnt->handle = fnt->size;
    fnt->info = (HFONT)1;
}

extern "C" void pfuMakeRasterXFont(char* name, pfuXFont* fnt)
{
    pfuLoadXFont(name, fnt);
}

extern "C" void pfuMakeXFontBitmaps(pfuXFont*) {}

extern "C" void pfuSetXFont(pfuXFont* f)
{
    if (f && f->size > 0) pfosgCurFont = *f;
}

extern "C" void pfuGetCurXFont(pfuXFont* f)
{
    if (f) *f = pfosgCurFont;
}

extern "C" int pfuGetXFontWidth(pfuXFont* f, const char* str)
{
    return (int)((str ? strlen(str) : 0) * 6.0f * fontScale(f));
}

extern "C" int pfuGetXFontHeight(pfuXFont* f)
{
    return (int)(9.0f * fontScale(f));
}

extern "C" void pfuCharPos(float x, float y, float z)
{
    pfosgRasterX = x;
    pfosgRasterY = y;
    pfosgRasterZ = z;
}

extern "C" void pfuDrawString(const char* s)
{
    if (!pfosgInDrawPhase) return;
    drawStringAt(s, pfosgRasterX, pfosgRasterY, pfosgRasterZ);
}

extern "C" void pfuDrawStringPos(const char* s, float x, float y, float z)
{
    if (!pfosgInDrawPhase) return;
    pfuCharPos(x, y, z);
    drawStringAt(s, x, y, z);
}

/* ---- pfuTraverser ----------------------------------------------------------- */

extern "C" void pfuInitTraverser(pfuTraverser* trav)
{
    if (trav) memset(trav, 0, sizeof(pfuTraverser));
}

/* ---- channel statistics overlay (pfDrawChanStats) ---------------------------
 * The classic perfly stats display, redrawn from the shim's numbers: frame
 * rate and frame-time strip chart (PFTIMES), plus scene geometry counts when
 * the Gfx/DB classes are enabled.  Runs inside the channel DRAW phase. */

static void statsText(float x, float y, int size, const char* s)
{
    pfuXFont f = { size, size, (HFONT)1 };
    pfuSetXFont(&f);
    drawStringAt(s, x, y, 0.0f);
}

extern "C" void pfDrawChanStats(pfChannel* ch)
{
    if (!pfosgInDrawPhase) return;
    PfOsgChan* c = pfosgChanOf(ch);
    unsigned cls = c ? c->statsClasses : 0x2;
    PfOsgState& S = pfosgState;

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    float vw = (float)vp[2], vh = (float)vp[3];
    if (vw < 64 || vh < 64) return;
    float u = vh / 800.0f;              /* layout unit, HiDPI-friendly */
    if (u < 0.5f) u = 0.5f;

    /* averages over the last second-ish of frames */
    float sum = 0, worst = 0;
    int n = 0;
    for (int i = 0; i < 60; i++) {
        int idx = (S.statsDtHead - 1 - i + 2 * PfOsgState::STATS_DTS) %
                  PfOsgState::STATS_DTS;
        float dt = S.statsDt[idx];
        if (dt <= 0) break;
        sum += dt;
        worst = dt > worst ? dt : worst;
        n++;
    }
    float avg = n ? sum / n : 0;
    float hz = avg > 0 ? 1.0f / avg : 0;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, vw, 0, vh, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                 GL_POLYGON_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bool gfx = (cls & (0x1 | 0x4 | 0x8)) != 0;   /* ENGFX | ENDB | ENCULL */
    float pad = 12 * u;
    float chartW = 240 * u, chartH = 64 * u;
    float lineH = 22 * u;
    float panelW = chartW + 2 * pad;
    float panelH = 2 * pad + chartH + lineH * (gfx ? 4.4f : 2.4f);
    float px0 = pad, py1 = vh - pad;    /* top-left anchor */
    float py0 = py1 - panelH;

    glColor4f(0.05f, 0.05f, 0.15f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2f(px0, py0); glVertex2f(px0 + panelW, py0);
    glVertex2f(px0 + panelW, py1); glVertex2f(px0, py1);
    glEnd();

    char buf[128];
    float ty = py1 - pad - 16 * u;
    glColor3f(0.95f, 0.95f, 0.7f);
    snprintf(buf, sizeof buf, "%5.1f Hz   %5.2f ms", hz, avg * 1000.0f);
    statsText(px0 + pad, ty, (int)(22 * u), buf);
    ty -= lineH * 1.2f;
    glColor3f(0.75f, 0.95f, 0.75f);
    snprintf(buf, sizeof buf, "app %4.1f  cull %4.1f  draw %4.1f ms",
             S.statsAppMs, S.statsCullMs, S.statsDrawMs);
    statsText(px0 + pad, ty, (int)(15 * u), buf);
    if (gfx) {
        ty -= lineH;
        glColor3f(0.75f, 0.85f, 0.95f);
        snprintf(buf, sizeof buf, "scene: %ld tris  %ld verts",
                 S.statsTris, S.statsVerts);
        statsText(px0 + pad, ty, (int)(15 * u), buf);
        ty -= lineH;
        snprintf(buf, sizeof buf, "       %ld geodes  %ld gsets",
                 S.statsGeodes, S.statsDrawables);
        statsText(px0 + pad, ty, (int)(15 * u), buf);
    }

    /* frame-time strip chart: one bar per recent frame, 60 Hz reference
     * line; green under 17.5ms, yellow under 33.3, red beyond */
    const int BARS = 80;
    float bx = px0 + pad, by = py0 + pad;
    float barW = chartW / BARS;
    const float fullScale = 0.0333f * 1.5f;      /* 1.5x a 30Hz frame */
    glBegin(GL_QUADS);
    for (int i = 0; i < BARS; i++) {
        int idx = (S.statsDtHead - BARS + i + 2 * PfOsgState::STATS_DTS) %
                  PfOsgState::STATS_DTS;
        float dt = S.statsDt[idx];
        if (dt <= 0) continue;
        if (dt <= 0.0175f)      glColor4f(0.2f, 0.9f, 0.2f, 0.9f);
        else if (dt <= 0.0333f) glColor4f(0.9f, 0.9f, 0.2f, 0.9f);
        else                    glColor4f(0.95f, 0.25f, 0.2f, 0.9f);
        float h = dt / fullScale;
        h = (h > 1.0f ? 1.0f : h) * chartH;
        glVertex2f(bx + i * barW, by);
        glVertex2f(bx + (i + 1) * barW - 1, by);
        glVertex2f(bx + (i + 1) * barW - 1, by + h);
        glVertex2f(bx + i * barW, by + h);
    }
    /* 60 Hz reference line */
    glColor4f(0.9f, 0.9f, 0.9f, 0.7f);
    float refH = (0.0167f / fullScale) * chartH;
    glVertex2f(bx, by + refH);
    glVertex2f(bx + chartW, by + refH);
    glVertex2f(bx + chartW, by + refH + 1);
    glVertex2f(bx, by + refH + 1);
    glEnd();

    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
