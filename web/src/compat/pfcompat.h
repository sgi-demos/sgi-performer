/*
 * pfcompat.h - force-included (-include) prefix for every TU that uses the
 * original Performer 3.0 *Windows* headers on a POSIX toolchain.
 *
 * The pristine headers in external/Include are never edited.  This shim
 * neutralizes their Windows-SDK type layer (wintypes.h) by pre-defining its
 * include guard and supplying the few names Performer headers actually use.
 */
#ifndef PFCOMPAT_H
#define PFCOMPAT_H

#include <sys/types.h>   /* ushort, uint, caddr_t, pid_t, mode_t */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

/* Suppress external/Include/Performer/wintypes.h (its guard macro). */
#define __WINTYPES_H

/* pr.h: skip <shader/isl.h> (IRIX OpenGL Shader integration). */
#ifndef _PF_NO_SHADER_
#define _PF_NO_SHADER_ 1
#endif

/* prmath.h uses __declspec directly; make it a no-op off Windows. */
#define __declspec(x)

/* pr.h only typedefs this under __linux__ or WIN32; macOS needs it too. */
typedef int XSGIvcChannelInfo;

/* Minimal Windows-SDK stand-ins used by the headers (pfWSDrawable etc.). */
typedef void*          HANDLE;
typedef void*          HWND;
typedef void*          HDC;
typedef void*          HGLRC;
typedef void*          HINSTANCE;
typedef int            BOOL;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned int   DWORD;
typedef int64_t        INT64;
typedef uint64_t       UINT64;
typedef unsigned int   UINT;
typedef int            INT;
typedef long           LONG;
typedef float          FLOAT;

/* Names wintypes.h would have provided. */
typedef void*          ulock_t;
typedef unsigned long  ulong;
typedef unsigned short ushort;
typedef unsigned int   uint;

#endif /* PFCOMPAT_H */
