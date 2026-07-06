/* GL/gl.h - pfosg shim: route to the platform's desktop GL header. */
#pragma once
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION 1
#include <OpenGL/gl.h>
#else
#include_next <GL/gl.h>
#endif
