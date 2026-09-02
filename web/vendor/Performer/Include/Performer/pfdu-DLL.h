#ifndef __PFDU_DLL_H__
#define __PFDU_DLL_H__

#ifdef WIN32
#ifdef __BUILD_PFDU__
#define PFDUDLLEXPORT __declspec(dllexport)
#else /*__BUILD_PFUTIL__ */
#define PFDUDLLEXPORT __declspec(dllimport)
#endif /*__BUILD_PFUTIL__ */
#else /* !WIN32 */
#define PFDUDLLEXPORT
#endif


#endif
