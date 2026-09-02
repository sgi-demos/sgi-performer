/* GL/glu.h - pfosg shim: route to the platform header.
 * Native GLES2 builds have no GLU; the samples only use it for error
 * strings, so declare just that (no-op definition in
 * pfosg_gles_compat.cpp, as on Emscripten). */
#pragma once
#if defined(PFOSG_GLES2)
#include <GL/gl.h>
#ifdef __cplusplus
extern "C" {
#endif
const GLubyte* gluErrorString(GLenum error);
#ifdef __cplusplus
}
#endif
#elif defined(__APPLE__)
#define GL_SILENCE_DEPRECATION 1
#include <OpenGL/glu.h>
#else
#include_next <GL/glu.h>
#endif
