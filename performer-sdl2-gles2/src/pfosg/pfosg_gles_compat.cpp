/* pfosg_gles_compat.cpp - Emscripten only.  Emscripten's <GL/gl.h>
 * DECLARES the legacy immediate-mode / fixed-function entry points (so the
 * desktop GUI/stats drawing code compiles) but only IMPLEMENTS them under
 * -sLEGACY_GL_EMULATION, which we don't use (it conflicts with the GLES2
 * shader path the scene renders through).  The web build is GUI-less, so
 * provide inert no-op definitions here; the panel/stats overlays simply do
 * not draw on the web.  Signatures match <GL/gl.h> exactly. */

#ifdef __EMSCRIPTEN__

#include <GL/gl.h>

extern "C" {

void glBegin(GLenum) {}
void glEnd(void) {}
void glVertex2f(GLfloat, GLfloat) {}
void glVertex3f(GLfloat, GLfloat, GLfloat) {}
void glVertex2fv(const GLfloat*) {}
void glVertex3fv(const GLfloat*) {}
void glColor3f(GLfloat, GLfloat, GLfloat) {}
void glColor4f(GLfloat, GLfloat, GLfloat, GLfloat) {}
void glColor3fv(const GLfloat*) {}
void glColor4fv(const GLfloat*) {}
void glColor3ub(GLubyte, GLubyte, GLubyte) {}
void glNormal3f(GLfloat, GLfloat, GLfloat) {}
void glNormal3fv(const GLfloat*) {}
void glTexCoord2f(GLfloat, GLfloat) {}

void glMatrixMode(GLenum) {}
void glLoadIdentity(void) {}
void glLoadMatrixf(const GLfloat*) {}
void glPushMatrix(void) {}
void glPopMatrix(void) {}
void glMultMatrixf(const GLfloat*) {}
void glOrtho(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble) {}
void glTranslatef(GLfloat, GLfloat, GLfloat) {}
void glRotatef(GLfloat, GLfloat, GLfloat, GLfloat) {}
void glScalef(GLfloat, GLfloat, GLfloat) {}

void glPushAttrib(GLbitfield) {}
void glPopAttrib(void) {}
void glPushClientAttrib(GLbitfield) {}
void glPopClientAttrib(void) {}
void glShadeModel(GLenum) {}

/* GLU isn't available on wasm; the samples only use it for error strings */
const GLubyte* gluErrorString(GLenum) { return (const GLubyte*)""; }

}   /* extern "C" */

#endif /* __EMSCRIPTEN__ */
