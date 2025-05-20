#ifndef __WINTYPES_H
#define __WINTYPES_H

#include <wTypes.h>
#include <windef.h>

#ifdef __LOCAL_BUILD__
// XXX Alex, the four following csts should be made public
#endif

#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK_1

typedef unsigned int UINT;
#define uint UINT
#define uint64_t UINT64
#define int64_t INT64
#define pid_t DWORD
#define ushort unsigned short
typedef void* ulock_t;
typedef int int32_t;
typedef unsigned long mode_t; /* see IRIX <sys/types.h> for more info */
typedef unsigned ulong;

//#include <WinNT.h> // for __uint32_t
typedef unsigned int uint32_t;
typedef uint32_t __uint32_t;

typedef char * caddr_t;

// XXX -- remove this:
typedef float GLfloat;

typedef unsigned long int uintptr_t;
typedef int ptrdiff_t;

#ifndef INT32_MAX 
#define INT32_MAX 2147483647
#endif

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#ifndef False
#define False 0
#endif

#ifndef True
#define True 1
#endif

#endif // __WINTYPES
