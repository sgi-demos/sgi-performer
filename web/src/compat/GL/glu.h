/* compat shim: route <GL/glu.h> to the platform header. */
#pragma once
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION 1
#include <OpenGL/glu.h>
#else
#include_next <GL/glu.h>
#endif
