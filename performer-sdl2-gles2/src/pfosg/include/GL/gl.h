/* GL/gl.h - pfosg shim: route to the platform's GL header.
 *
 * Desktop builds get the real desktop GL.  Native GLES2 builds (ANGLE)
 * get <GLES2/gl2.h> plus DECLARATIONS of the legacy immediate-mode /
 * fixed-function entry points and tokens that the GUI/stats overlay code
 * references — their definitions are the inert no-ops in
 * pfosg_gles_compat.cpp (the GLES2 builds are GUI-less).  On Emscripten
 * the same declarations come from emscripten's own <GL/gl.h>. */
#pragma once
#if defined(PFOSG_GLES2) && !defined(__EMSCRIPTEN__)

#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef double GLdouble;

/* legacy fixed-function tokens (desktop <GL/gl.h> values) */
#define GL_QUADS                  0x0007
#define GL_POLYGON                0x0009
#define GL_MODELVIEW              0x1700
#define GL_PROJECTION             0x1701
#define GL_LIGHTING               0x0B50
#define GL_LIGHT0                 0x4000
#define GL_ALPHA_TEST             0x0BC0
#define GL_COLOR_MATERIAL         0x0B57
#define GL_FOG                    0x0B60
#define GL_FLAT                   0x1D00
#define GL_SMOOTH                 0x1D01
#define GL_ENABLE_BIT             0x00002000
#define GL_POLYGON_BIT            0x00000008
#define GL_SCISSOR_BIT            0x00080000
#define GL_ALL_ATTRIB_BITS        0xFFFFFFFF
#define GL_CLIENT_ALL_ATTRIB_BITS 0xFFFFFFFF
/* tokens the SGI pfpfb loader stores as data (never fed to GLES2) */
#define GL_AMBIENT                0x1200
#define GL_DIFFUSE                0x1201
#define GL_SPECULAR               0x1202
#define GL_EMISSION               0x1600
#define GL_CLAMP                  0x2900
#define GL_EXP                    0x0800
#define GL_EXP2                   0x0801
#define GL_EYE_LINEAR             0x2400
#define GL_OBJECT_LINEAR          0x2401
#define GL_SPHERE_MAP             0x2402
#define GL_PIXEL_MAP_I_TO_I       0x0C70
#define GL_PIXEL_MAP_I_TO_R       0x0C72
#define GL_PIXEL_MAP_I_TO_G       0x0C73
#define GL_PIXEL_MAP_I_TO_B       0x0C74
#define GL_PIXEL_MAP_I_TO_A       0x0C75

/* legacy entry points; no-op definitions in pfosg_gles_compat.cpp */
void glBegin(GLenum mode);
void glEnd(void);
void glVertex2f(GLfloat x, GLfloat y);
void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void glVertex2fv(const GLfloat* v);
void glVertex3fv(const GLfloat* v);
void glColor3f(GLfloat r, GLfloat g, GLfloat b);
void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void glColor3fv(const GLfloat* v);
void glColor4fv(const GLfloat* v);
void glColor3ub(GLubyte r, GLubyte g, GLubyte b);
void glNormal3f(GLfloat x, GLfloat y, GLfloat z);
void glNormal3fv(const GLfloat* v);
void glTexCoord2f(GLfloat s, GLfloat t);

void glMatrixMode(GLenum mode);
void glLoadIdentity(void);
void glLoadMatrixf(const GLfloat* m);
void glPushMatrix(void);
void glPopMatrix(void);
void glMultMatrixf(const GLfloat* m);
void glOrtho(GLdouble l, GLdouble r, GLdouble b, GLdouble t,
             GLdouble n, GLdouble f);
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glScalef(GLfloat x, GLfloat y, GLfloat z);

void glPushAttrib(GLbitfield mask);
void glPopAttrib(void);
void glPushClientAttrib(GLbitfield mask);
void glPopClientAttrib(void);
void glShadeModel(GLenum mode);

#ifdef __cplusplus
}
#endif

#elif defined(__APPLE__)
#define GL_SILENCE_DEPRECATION 1
#include <OpenGL/gl.h>
#else
#include_next <GL/gl.h>
#endif
