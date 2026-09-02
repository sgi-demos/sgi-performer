/* compat shim: route <GL/gl.h> to the platform's desktop GL 1.x header.
 * Performer headers only need GL types and tokens from it.  The native
 * GLES2 backend will eventually replace this with its own token header. */
#pragma once
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION 1
#include <OpenGL/gl.h>
#else
#include_next <GL/gl.h>
#endif
