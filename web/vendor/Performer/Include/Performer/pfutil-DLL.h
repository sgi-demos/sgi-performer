#ifndef __PFUTIL_DLL_H__
#define __PFUTIL_DLL_H__


#ifdef WIN32
#ifdef __BUILD_PFUTIL__
#define PFUDLLEXPORT __declspec(dllexport)
#else /*__BUILD_PFUTIL__ */
#define PFUDLLEXPORT __declspec(dllimport)
#endif /*__BUILD_PFUTIL__ */
#else /* !WIN32 */
#define PFUDLLEXPORT
#endif


#endif
