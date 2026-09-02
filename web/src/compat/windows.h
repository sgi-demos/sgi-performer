/* compat shim: stand-in for <windows.h> on POSIX platforms.  The Windows
 * Performer headers include it unconditionally (e.g. pr/pfState.h); the
 * handful of types they actually use (HANDLE, HWND, HDC, HGLRC, DWORD, ...)
 * are supplied by the force-included pfcompat.h.  Add spellings here only as
 * compile errors demand them. */
#pragma once

#ifndef WINAPI
#define WINAPI
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef CALLBACK
#define CALLBACK
#endif

typedef void* LPVOID;
typedef const char* LPCSTR;
typedef int (*PROC)(void);
