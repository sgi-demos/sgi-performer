/*
 * gles2_min.h - the GLES2 subset used by the port, loadable everywhere.
 *
 * On Emscripten <GLES2/gl2.h> is the real thing.  On native platforms the
 * functions are resolved through SDL_GL_GetProcAddress after context
 * creation, which works for both true ES2 contexts (Linux/Windows drivers,
 * ANGLE) and the desktop GL 2.1 fallback on macOS.  Code must stay inside
 * the ES2-compatible intersection; this header is the allowlist.
 */
#ifndef GLES2_MIN_H
#define GLES2_MIN_H

#ifdef __EMSCRIPTEN__

#include <GLES2/gl2.h>
static inline int gles2_min_load(void) { return 1; }

#else

#include <SDL_video.h>
#include <stddef.h>
#include <stdint.h>

typedef unsigned int   GLenum;
typedef unsigned char  GLboolean;
typedef unsigned int   GLbitfield;
typedef int            GLint;
typedef int            GLsizei;
typedef unsigned int   GLuint;
typedef float          GLfloat;
typedef float          GLclampf;
typedef char           GLchar;
typedef uint8_t        GLubyte;
typedef ptrdiff_t      GLsizeiptr;

#define GL_DEPTH_BUFFER_BIT   0x00000100
#define GL_COLOR_BUFFER_BIT   0x00004000
#define GL_TRIANGLES          0x0004
#define GL_DEPTH_TEST         0x0B71
#define GL_LEQUAL             0x0203
#define GL_FLOAT              0x1406
#define GL_VENDOR             0x1F00
#define GL_RENDERER           0x1F01
#define GL_VERSION            0x1F02
#define GL_ARRAY_BUFFER       0x8892
#define GL_STATIC_DRAW        0x88E4
#define GL_FRAGMENT_SHADER    0x8B30
#define GL_VERTEX_SHADER      0x8B31
#define GL_COMPILE_STATUS     0x8B81
#define GL_LINK_STATUS        0x8B82
#define GL_FALSE              0
#define GL_TRUE               1

/* X-macro: every GL entry point the project may call on native builds. */
#define GLES2_MIN_FUNCS(X) \
    X(const GLubyte*, glGetString, (GLenum name)) \
    X(void, glViewport, (GLint x, GLint y, GLsizei w, GLsizei h)) \
    X(void, glClearColor, (GLclampf r, GLclampf g, GLclampf b, GLclampf a)) \
    X(void, glClear, (GLbitfield mask)) \
    X(void, glEnable, (GLenum cap)) \
    X(void, glDepthFunc, (GLenum func)) \
    X(GLuint, glCreateShader, (GLenum type)) \
    X(void, glShaderSource, (GLuint s, GLsizei n, const GLchar* const* src, const GLint* len)) \
    X(void, glCompileShader, (GLuint s)) \
    X(void, glGetShaderiv, (GLuint s, GLenum pname, GLint* params)) \
    X(void, glGetShaderInfoLog, (GLuint s, GLsizei bufSize, GLsizei* len, GLchar* log)) \
    X(GLuint, glCreateProgram, (void)) \
    X(void, glAttachShader, (GLuint p, GLuint s)) \
    X(void, glLinkProgram, (GLuint p)) \
    X(void, glGetProgramiv, (GLuint p, GLenum pname, GLint* params)) \
    X(void, glGetProgramInfoLog, (GLuint p, GLsizei bufSize, GLsizei* len, GLchar* log)) \
    X(void, glUseProgram, (GLuint p)) \
    X(GLint, glGetAttribLocation, (GLuint p, const GLchar* name)) \
    X(GLint, glGetUniformLocation, (GLuint p, const GLchar* name)) \
    X(void, glUniform1f, (GLint loc, GLfloat v)) \
    X(void, glGenBuffers, (GLsizei n, GLuint* buffers)) \
    X(void, glBindBuffer, (GLenum target, GLuint buffer)) \
    X(void, glBufferData, (GLenum target, GLsizeiptr size, const void* data, GLenum usage)) \
    X(void, glEnableVertexAttribArray, (GLuint index)) \
    X(void, glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean norm, GLsizei stride, const void* ptr)) \
    X(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count))

#define GLES2_MIN_DECL(ret, name, args) typedef ret (*name##_fn) args; extern name##_fn name;
GLES2_MIN_FUNCS(GLES2_MIN_DECL)
#undef GLES2_MIN_DECL

#ifdef GLES2_MIN_IMPLEMENTATION
#define GLES2_MIN_DEF(ret, name, args) name##_fn name = NULL;
GLES2_MIN_FUNCS(GLES2_MIN_DEF)
#undef GLES2_MIN_DEF

static int gles2_min_load(void)
{
    int ok = 1;
#define GLES2_MIN_LOAD(ret, name, args) \
    name = (name##_fn)SDL_GL_GetProcAddress(#name); \
    if (!name) ok = 0;
    GLES2_MIN_FUNCS(GLES2_MIN_LOAD)
#undef GLES2_MIN_LOAD
    return ok;
}
#endif /* GLES2_MIN_IMPLEMENTATION */

#endif /* !__EMSCRIPTEN__ */

#endif /* GLES2_MIN_H */
