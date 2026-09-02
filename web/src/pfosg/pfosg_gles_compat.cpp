/* pfosg_gles_compat.cpp - GLES2 builds only (web and native ANGLE).
 *
 * A batched immediate-mode mini-GL: just enough of the legacy fixed-function
 * surface (matrix stacks, glBegin/glEnd, attrib push/pop) that SGI's
 * libpfutil gui.c panel, the stats overlay, and perfly's messages — all
 * written against IRIX-era GL — draw unmodified on GLES2.  Vertices are
 * batched per glBegin/glEnd and drawn through a small color-only shader;
 * QUADS/POLYGON are converted (GLES2 draws the other primitive modes
 * natively).  Everything the batch clobbers (program, buffer binding,
 * attrib enables) is restored so osg::State's caches stay truthful.
 *
 * Declarations live in the shim GL/gl.h (included by Performer/pr.h), which
 * also interposes glGetIntegerv/glGetFloatv so legacy matrix queries answer
 * from the stacks here. */

#ifdef PFOSG_GLES2

#include <GL/gl.h>
#undef glGetIntegerv          /* this file needs the real queries */
#undef glGetFloatv

#include <cmath>
#include <cstring>
#include <cstdio>
#include <vector>

/* ---- 4x4 column-major matrices, GL conventions ---------------------------- */

struct M4 { GLfloat m[16]; };

static M4 identity()
{
    M4 r = {{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}};
    return r;
}

/* r = a * b (column-major, GL order: b applies to vertices first) */
static M4 mul(const M4& a, const M4& b)
{
    M4 r;
    for (int c = 0; c < 4; c++)
        for (int i = 0; i < 4; i++)
            r.m[c*4+i] = a.m[0*4+i]*b.m[c*4+0] + a.m[1*4+i]*b.m[c*4+1] +
                         a.m[2*4+i]*b.m[c*4+2] + a.m[3*4+i]*b.m[c*4+3];
    return r;
}

static std::vector<M4>& mvStack()
{
    static std::vector<M4> s(1, identity());
    return s;
}
static std::vector<M4>& projStack()
{
    static std::vector<M4> s(1, identity());
    return s;
}
static GLenum g_matrixMode = GL_MODELVIEW;
static std::vector<M4>& curStack()
{
    return g_matrixMode == GL_PROJECTION ? projStack() : mvStack();
}

extern "C" {

void glMatrixMode(GLenum mode) { g_matrixMode = mode; }
void glLoadIdentity(void)      { curStack().back() = identity(); }
void glLoadMatrixf(const GLfloat* m)
{
    memcpy(curStack().back().m, m, sizeof(M4));
}
void glMultMatrixf(const GLfloat* m)
{
    M4 b; memcpy(b.m, m, sizeof b.m);
    curStack().back() = mul(curStack().back(), b);
}
void glPushMatrix(void) { curStack().push_back(curStack().back()); }
void glPopMatrix(void)  { if (curStack().size() > 1) curStack().pop_back(); }

void glOrtho(GLdouble l, GLdouble r, GLdouble b, GLdouble t,
             GLdouble n, GLdouble f)
{
    M4 o = identity();
    o.m[0]  = (GLfloat)(2.0 / (r - l));
    o.m[5]  = (GLfloat)(2.0 / (t - b));
    o.m[10] = (GLfloat)(-2.0 / (f - n));
    o.m[12] = (GLfloat)(-(r + l) / (r - l));
    o.m[13] = (GLfloat)(-(t + b) / (t - b));
    o.m[14] = (GLfloat)(-(f + n) / (f - n));
    glMultMatrixf(o.m);
}
void glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    M4 t = identity();
    t.m[12] = x; t.m[13] = y; t.m[14] = z;
    glMultMatrixf(t.m);
}
void glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    M4 s = identity();
    s.m[0] = x; s.m[5] = y; s.m[10] = z;
    glMultMatrixf(s.m);
}
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    float len = sqrtf(x*x + y*y + z*z);
    if (len < 1e-6f) return;
    x /= len; y /= len; z /= len;
    float c = cosf(angle * 3.14159265358979f / 180.0f);
    float s = sinf(angle * 3.14159265358979f / 180.0f);
    float ic = 1.0f - c;
    M4 r = identity();
    r.m[0] = x*x*ic + c;   r.m[4] = x*y*ic - z*s; r.m[8]  = x*z*ic + y*s;
    r.m[1] = y*x*ic + z*s; r.m[5] = y*y*ic + c;   r.m[9]  = y*z*ic - x*s;
    r.m[2] = z*x*ic - y*s; r.m[6] = z*y*ic + x*s; r.m[10] = z*z*ic + c;
    glMultMatrixf(r.m);
}

/* legacy-aware query interposers (see GL/gl.h) */
void pfoglGetIntegerv(GLenum pname, GLint* params)
{
    if (pname == GL_MATRIX_MODE) { *params = (GLint)g_matrixMode; return; }
    glGetIntegerv(pname, params);
}
void pfoglGetFloatv(GLenum pname, GLfloat* params)
{
    if (pname == GL_PROJECTION_MATRIX) {
        memcpy(params, projStack().back().m, sizeof(M4)); return;
    }
    if (pname == GL_MODELVIEW_MATRIX) {
        memcpy(params, mvStack().back().m, sizeof(M4)); return;
    }
    glGetFloatv(pname, params);
}

}   /* extern "C" */

/* ---- immediate-mode batching ------------------------------------------------ */

static GLfloat g_color[4] = {1, 1, 1, 1};
static GLenum g_prim = GL_TRIANGLES;
static bool g_inBegin = false;
static std::vector<GLfloat> g_verts;   /* x,y,z,r,g,b,a interleaved */

static GLuint g_prog = 0, g_vbo = 0;
static GLint g_umvp = -1;

static void initBatchProgram()
{
    static const char* vs =
        "attribute vec4 a_pos;\n"
        "attribute vec4 a_color;\n"
        "uniform mat4 u_mvp;\n"
        "varying vec4 v_color;\n"
        "void main() { gl_Position = u_mvp * a_pos; v_color = a_color; }\n";
    static const char* fs =
        "precision mediump float;\n"
        "varying vec4 v_color;\n"
        "void main() { gl_FragColor = v_color; }\n";
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vs, nullptr);
    glCompileShader(v);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fs, nullptr);
    glCompileShader(f);
    g_prog = glCreateProgram();
    glAttachShader(g_prog, v);
    glAttachShader(g_prog, f);
    glBindAttribLocation(g_prog, 0, "a_pos");
    glBindAttribLocation(g_prog, 1, "a_color");
    glLinkProgram(g_prog);
    GLint ok = 0;
    glGetProgramiv(g_prog, GL_LINK_STATUS, &ok);
    if (!ok) fprintf(stderr, "pfosg: gles2 overlay program failed to link\n");
    g_umvp = glGetUniformLocation(g_prog, "u_mvp");
    glDeleteShader(v);
    glDeleteShader(f);
    glGenBuffers(1, &g_vbo);
}

static void flushBatch()
{
    size_t nverts = g_verts.size() / 7;
    if (!nverts) return;

    GLenum mode = g_prim;
    std::vector<GLfloat>* data = &g_verts;
    static std::vector<GLfloat> tris;
    if (g_prim == GL_QUADS) {
        /* independent quads -> two triangles each */
        tris.clear();
        for (size_t q = 0; q + 3 < nverts; q += 4) {
            static const int idx[6] = {0, 1, 2, 0, 2, 3};
            for (int i = 0; i < 6; i++)
                tris.insert(tris.end(),
                            g_verts.begin() + (q + idx[i]) * 7,
                            g_verts.begin() + (q + idx[i]) * 7 + 7);
        }
        data = &tris;
        mode = GL_TRIANGLES;
        nverts = tris.size() / 7;
    } else if (g_prim == GL_POLYGON) {
        mode = GL_TRIANGLE_FAN;    /* convex polys: same vertex order */
    }

    /* save what the batch clobbers so osg::State's caches stay truthful */
    GLint prevProg = 0, prevVbo = 0, en0 = 0, en1 = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProg);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevVbo);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &en0);
    glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &en1);

    if (!g_prog) initBatchProgram();
    glUseProgram(g_prog);
    M4 mvp = mul(projStack().back(), mvStack().back());
    glUniformMatrix4fv(g_umvp, 1, GL_FALSE, mvp.m);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(data->size() * sizeof(GLfloat)),
                 data->data(), GL_STREAM_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 28, (const void*)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 28, (const void*)12);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(mode, 0, (GLsizei)nverts);

    if (!en0) glDisableVertexAttribArray(0);
    if (!en1) glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)prevVbo);
    glUseProgram((GLuint)prevProg);
}

extern "C" {

void glBegin(GLenum mode)
{
    g_prim = mode;
    g_verts.clear();
    g_inBegin = true;
}

void glEnd(void)
{
    if (!g_inBegin) return;
    g_inBegin = false;
    flushBatch();
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    if (!g_inBegin) return;
    g_verts.push_back(x);
    g_verts.push_back(y);
    g_verts.push_back(z);
    g_verts.insert(g_verts.end(), g_color, g_color + 4);
}
void glVertex2f(GLfloat x, GLfloat y)        { glVertex3f(x, y, 0.0f); }
void glVertex2fv(const GLfloat* v)           { glVertex3f(v[0], v[1], 0.0f); }
void glVertex3fv(const GLfloat* v)           { glVertex3f(v[0], v[1], v[2]); }

void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    g_color[0] = r; g_color[1] = g; g_color[2] = b; g_color[3] = a;
}
void glColor3f(GLfloat r, GLfloat g, GLfloat b)  { glColor4f(r, g, b, 1.0f); }
void glColor3fv(const GLfloat* v)  { glColor4f(v[0], v[1], v[2], 1.0f); }
void glColor4fv(const GLfloat* v)  { glColor4f(v[0], v[1], v[2], v[3]); }
void glColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
    glColor4f(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

/* accepted, unused by the overlay drawing */
void glNormal3f(GLfloat, GLfloat, GLfloat) {}
void glNormal3fv(const GLfloat*) {}
void glTexCoord2f(GLfloat, GLfloat) {}
void glIndexi(GLint) {}
void glShadeModel(GLenum) {}
void glPushClientAttrib(GLbitfield) {}
void glPopClientAttrib(void) {}

}   /* extern "C" */

/* ---- attrib push/pop --------------------------------------------------------
 * Saves/restores the small set of real GLES2 state the overlay code touches
 * (pfBasicState, pfClear's scissor, message blending), regardless of mask —
 * a superset restore is conservative and keeps osg::State's caches valid
 * because the restored values equal what OSG last applied. */

struct AttrSave {
    GLboolean depth, cull, blend, scissor, dither;
    GLboolean depthMask;
    GLint depthFunc;
    GLint sbox[4], vp[4];
    GLint blendSrcRGB, blendDstRGB, blendSrcA, blendDstA;
    GLfloat lineWidth;
};
static std::vector<AttrSave> g_attrStack;

extern "C" {

void glPushAttrib(GLbitfield)
{
    AttrSave s;
    s.depth   = glIsEnabled(GL_DEPTH_TEST);
    s.cull    = glIsEnabled(GL_CULL_FACE);
    s.blend   = glIsEnabled(GL_BLEND);
    s.scissor = glIsEnabled(GL_SCISSOR_TEST);
    s.dither  = glIsEnabled(GL_DITHER);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &s.depthMask);
    glGetIntegerv(GL_DEPTH_FUNC, &s.depthFunc);
    glGetIntegerv(GL_SCISSOR_BOX, s.sbox);
    glGetIntegerv(GL_VIEWPORT, s.vp);
    glGetIntegerv(GL_BLEND_SRC_RGB, &s.blendSrcRGB);
    glGetIntegerv(GL_BLEND_DST_RGB, &s.blendDstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &s.blendSrcA);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &s.blendDstA);
    glGetFloatv(GL_LINE_WIDTH, &s.lineWidth);
    g_attrStack.push_back(s);
}

void glPopAttrib(void)
{
    if (g_attrStack.empty()) return;
    const AttrSave& s = g_attrStack.back();
    (s.depth   ? glEnable : glDisable)(GL_DEPTH_TEST);
    (s.cull    ? glEnable : glDisable)(GL_CULL_FACE);
    (s.blend   ? glEnable : glDisable)(GL_BLEND);
    (s.scissor ? glEnable : glDisable)(GL_SCISSOR_TEST);
    (s.dither  ? glEnable : glDisable)(GL_DITHER);
    glDepthMask(s.depthMask);
    glDepthFunc((GLenum)s.depthFunc);
    glScissor(s.sbox[0], s.sbox[1], s.sbox[2], s.sbox[3]);
    glViewport(s.vp[0], s.vp[1], s.vp[2], s.vp[3]);
    glBlendFuncSeparate((GLenum)s.blendSrcRGB, (GLenum)s.blendDstRGB,
                        (GLenum)s.blendSrcA, (GLenum)s.blendDstA);
    glLineWidth(s.lineWidth);
    g_attrStack.pop_back();
}

/* GLU isn't available on GLES2; the samples only use it for error strings */
const GLubyte* gluErrorString(GLenum) { return (const GLubyte*)""; }

}   /* extern "C" */

#endif /* PFOSG_GLES2 */
