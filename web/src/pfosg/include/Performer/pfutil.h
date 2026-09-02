/* Performer/pfutil.h - pfosg shim: forwards to the REAL pfutil.h so tokens,
 * struct layouts (pfuMouse, pfuEventStream, widgets...) and declarations are
 * exactly the originals.  The shim implements the functions the samples
 * actually execute; the rest resolve at link time only if referenced. */
#ifndef PFOSG_PFUTIL_H
#define PFOSG_PFUTIL_H

#include <Performer/pf.h>   /* shim: object handles + real prmath */

/* compat the real (Windows-tree) pfutil.h expects from its surroundings */
#include <stdio.h>          /* FILE */
#ifndef PF_USPTR_T
#define PF_USPTR_T void     /* IRIX usptr_t; only ever used as a pointer */
#endif
typedef unsigned long ulong;
typedef void* HWND;
typedef void* HFONT;
typedef struct { int lfHeight; } LOGFONT;

/* real pfutil.h typedefs Display itself when it isn't already defined */
#ifndef PFOSG_DISPLAY_DEFINED
#define PFOSG_DISPLAY_DEFINED
typedef void Display;
#endif

/* normally from pr.h (clip-texture tile filename argument counts) */
#ifndef MAX_TILE_FILENAME_UNIQUE_ARGS
#define MAX_TILE_FILENAME_UNIQUE_ARGS       14
#define PFIMAGECACHE_MAX_TILE_FILENAME_ARGS 16
#endif

/* real pfutil.h gates its C++-only includes on PF_CPLUSPLUS_API (0 here) */
#include_next <Performer/pfutil.h>

#endif /* PFOSG_PFUTIL_H */
