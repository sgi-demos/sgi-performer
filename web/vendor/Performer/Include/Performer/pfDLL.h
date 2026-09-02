#ifndef __PFDLL_H__
#define __PFDLL_H__

#ifdef WIN32
#if defined(__BUILDPF__) || defined(__BUILDPR__)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif /* localbuild */
#else /* win32 */
/* On irix and linux, DLLEXPORT does nothing */
#define DLLEXPORT
#endif

#endif
