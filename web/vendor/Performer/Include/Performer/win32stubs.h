#ifndef __WIN32STUBS_H__
#define __WIN32STUBS_H__

#include <windows.h>
#include <GL/gl.h>
#include <io.h>
#include <stdio.h>
#include "pfDLL.h"

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT void bzero(void *b,size_t length);
DLLEXPORT void bcopy(const void *src,void *dst,size_t length);
DLLEXPORT void gettimeofday(struct timeval*,void *);
DLLEXPORT size_t getpagesize();
DLLEXPORT int sysmp(int cmd,...);

  /* Access on windows is called _access */
#define access _access

DLLEXPORT void glColorPointerEXT(GLint size,GLenum type,GLsizei stride,
							GLsizei count,const GLvoid *ptr);
DLLEXPORT void glNormalPointerEXT(GLenum type,GLsizei stride,GLsizei count,const GLvoid *ptr);
DLLEXPORT void glTexCoordPointerEXT(GLint size,GLenum type,GLsizei stride,GLsizei count,const GLvoid *ptr);
DLLEXPORT void glVertexPointerEXT(GLint size,GLenum type,GLsizei stride,GLsizei count,const GLvoid *ptr);
DLLEXPORT void glDrawArraysEXT(GLenum mode,GLint first,GLsizei count);
DLLEXPORT void linuxSTUB();

DLLEXPORT int ffs(unsigned int i);
DLLEXPORT double trunc(double n);
DLLEXPORT long sginap(long ticks);

  /* Declare GL_ARB_multitexture function prototypes. This is necessary since they
     are not in windows headers */
#ifndef GL_ARB_multitexture
#define GL_TEXTURE0_ARB                   0x84C0
#define GL_TEXTURE1_ARB                   0x84C1
#define GL_TEXTURE2_ARB                   0x84C2
#define GL_TEXTURE3_ARB                   0x84C3
#define GL_TEXTURE4_ARB                   0x84C4
#define GL_TEXTURE5_ARB                   0x84C5
#define GL_TEXTURE6_ARB                   0x84C6
#define GL_TEXTURE7_ARB                   0x84C7
#define GL_TEXTURE8_ARB                   0x84C8
#define GL_TEXTURE9_ARB                   0x84C9
#define GL_TEXTURE10_ARB                  0x84CA
#define GL_TEXTURE11_ARB                  0x84CB
#define GL_TEXTURE12_ARB                  0x84CC
#define GL_TEXTURE13_ARB                  0x84CD
#define GL_TEXTURE14_ARB                  0x84CE
#define GL_TEXTURE15_ARB                  0x84CF
#define GL_TEXTURE16_ARB                  0x84D0
#define GL_TEXTURE17_ARB                  0x84D1
#define GL_TEXTURE18_ARB                  0x84D2
#define GL_TEXTURE19_ARB                  0x84D3
#define GL_TEXTURE20_ARB                  0x84D4
#define GL_TEXTURE21_ARB                  0x84D5
#define GL_TEXTURE22_ARB                  0x84D6
#define GL_TEXTURE23_ARB                  0x84D7
#define GL_TEXTURE24_ARB                  0x84D8
#define GL_TEXTURE25_ARB                  0x84D9
#define GL_TEXTURE26_ARB                  0x84DA
#define GL_TEXTURE27_ARB                  0x84DB
#define GL_TEXTURE28_ARB                  0x84DC
#define GL_TEXTURE29_ARB                  0x84DD
#define GL_TEXTURE30_ARB                  0x84DE
#define GL_TEXTURE31_ARB                  0x84DF
#define GL_ACTIVE_TEXTURE_ARB             0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB      0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB          0x84E2
#endif

#ifndef GL_ARB_multitexture
#define GL_ARB_multitexture 1
  void  _pfInitMultiTexPointers();
  void  glActiveTextureARB(GLenum);
  void  glMultiTexCoord2fARB(GLenum, GLfloat, GLfloat);
  void  glMultiTexCoord2fvARB(GLenum, const GLfloat *);
  typedef void (__stdcall * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
  typedef void (__stdcall * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
  typedef void (__stdcall * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
#endif

  


/* WINDOWS IS A PIECE OF SHIT. No floating point versions
   of anything. 
*/
#define ftrunc trunc
#define truncf trunc
#define fsqrt  sqrt
#define fexp   exp
#define ffloor floor
#define fceil  ceil
#define fcos   cos
#define fsin   sin

#ifndef FLT_MAX
#define FLT_MAX 3.40282347E+38
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_LOG2E
#define M_LOG2E         1.4426950408889634074
#endif

#ifdef __cplusplus
}
#endif

#endif // WIN32STUBS
