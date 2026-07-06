/* win32stubs.h - pfosg stand-in for the Windows-tree header of the same
 * name (the real one drags in <windows.h>).  The real pfutil.h includes it
 * unconditionally; everything it declared exists natively on POSIX. */
#ifndef PFOSG_WIN32STUBS_H
#define PFOSG_WIN32STUBS_H

#include <strings.h>     /* bzero, bcopy */
#include <unistd.h>      /* getpagesize, access */
#include <sys/time.h>    /* gettimeofday */
#include <GL/gl.h>

#endif /* PFOSG_WIN32STUBS_H */
