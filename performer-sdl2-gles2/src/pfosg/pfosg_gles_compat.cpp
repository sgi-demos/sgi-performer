/* pfosg_gles_compat.cpp - GLES2 builds only (web and native ANGLE).  The
 * legacy immediate-mode / fixed-function entry points are DECLARED so the
 * desktop GUI/stats drawing code compiles (on web by Emscripten's
 * <GL/gl.h>; natively by the shim GL/gl.h) but GLES2 has no implementation
 * (Emscripten only implements them under -sLEGACY_GL_EMULATION, which
 * conflicts with the GLES2 shader path).  The GLES2 builds are GUI-less,
 * so provide inert no-op definitions here; the panel/stats overlays simply
 * do not draw.  Signatures match <GL/gl.h> exactly. */

#ifdef PFOSG_GLES2

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

#endif /* PFOSG_GLES2 */
