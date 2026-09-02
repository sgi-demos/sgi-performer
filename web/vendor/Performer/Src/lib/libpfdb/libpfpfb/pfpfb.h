/*
 * Copyright (C) 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * This source code ("Source Code") was originally derived from a
 * code base owned by Silicon Graphics, Inc. ("SGI")
 * 
 * LICENSE: SGI grants the user ("Licensee") permission to reproduce,
 * distribute, and create derivative works from this Source Code,
 * provided that: (1) the user reproduces this entire notice within
 * both source and binary format redistributions and any accompanying
 * materials such as documentation in printed or electronic format;
 * (2) the Source Code is not to be used, or ported or modified for
 * use, except in conjunction with OpenGL Performer; and (3) the
 * names of Silicon Graphics, Inc.  and SGI may not be used in any
 * advertising or publicity relating to the Source Code without the
 * prior written permission of SGI.  No further license or permission
 * may be inferred or deemed or construed to exist with regard to the
 * Source Code or the code base of which it forms a part. All rights
 * not expressly granted are reserved.
 * 
 * This Source Code is provided to Licensee AS IS, without any
 * warranty of any kind, either express, implied, or statutory,
 * including, but not limited to, any warranty that the Source Code
 * will conform to specifications, any implied warranties of
 * merchantability, fitness for a particular purpose, and freedom
 * from infringement, and any warranty that the documentation will
 * conform to the program, or any warranty that the Source Code will
 * be error free.
 * 
 * IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
 * LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
 * ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
 * SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
 * OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
 * PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
 * OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
 * USE, THE SOURCE CODE.
 * 
 * Contact information:  Silicon Graphics, Inc., 
 * 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
 * or:  http://www.sgi.com
 */

/*
 *  pfpfb.h $Revision: 1.27 $
 */


#ifndef __PFPFB_H__
#define __PFPFB_H__

#ifdef WIN32
#include <windows.h>
#include "wintypes.h"
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFPDB__ */
#else
#define PFPFB_DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*
 *  defines for pfdConverterMode_pfb
 */
#define PFPFB_SAVE_TEXTURE_IMAGE	  1
#define PFPFB_SAVE_TEXTURE_PATH		  2
#define PFPFB_SHARE_GS_OBJECTS		  3
#define PFPFB_COMPILE_GL		  4
#define PFPFB_SAVE_TEXTURE_PFI		  5
#define PFPFB_DONT_LOAD_USER_FUNCS	  6


/*
 *  values for pfdConverterMode_pfb(PFPFB_COMPILE_GL, value)
 */
#define PFPFB_COMPILE_GL_OFF		  0
#define PFPFB_COMPILE_GL_ON		  1
#define PFPFB_COMPILE_GL_AS_SAVED	  2


/*
 *  typedefs prototypes for pfb
 */
typedef void (*pfdDecryptFuncType_pfb)(int size, uint *key, void *data);
typedef void (*pfdEncryptFuncType_pfb)(int size, uint *key, void *src,
				       void *dst);
typedef int (*pfdDescendUserDataFuncType_pfb)(void *udata, pfObject *parent,
					     void *handle);
typedef int (*pfdStoreUserDataFuncType_pfb)(void *udata, pfObject *parent,
					    void *handle);
typedef void *(*pfdLoadUserDataFuncType_pfb)(void *handle);
typedef int (*pfdDescendCustomNodeFuncType_pfb)(pfNode *node, void *handle);
typedef int (*pfdStoreCustomNodeFuncType_pfb)(pfNode *node, void *handle);
typedef pfNode *(*pfdNewCustomNodeFuncType_pfb)(void);
typedef int (*pfdLoadCustomNodeFuncType_pfb)(pfNode *node, void *handle);

extern PFPFB_DLLEXPORT pfNode*	pfdLoadFile_pfb(const char *fileName);
extern PFPFB_DLLEXPORT int	pfdStoreFile_pfb(pfNode *root, const char *fileName);
extern PFPFB_DLLEXPORT void	pfdConverterMode_pfb(int mode, int value);
extern PFPFB_DLLEXPORT int	pfdGetConverterMode_pfb(int mode);
extern PFPFB_DLLEXPORT void	pfdConverterVal_pfb(int which, float val);
extern PFPFB_DLLEXPORT float	pfdGetConverterVal_pfb(int which);
extern PFPFB_DLLEXPORT void	pfdConverterAttr_pfb(int which, void *attr);
extern PFPFB_DLLEXPORT void*	pfdGetConverterAttr_pfb(int which);
extern PFPFB_DLLEXPORT size_t	pfdLoadData_pfb(void *data, int size, void *handle);
extern PFPFB_DLLEXPORT size_t	pfdStoreData_pfb(void *data, int size, void *handle);
extern PFPFB_DLLEXPORT int	pfdDescendObject_pfb(pfObject *obj, void *handle);
extern PFPFB_DLLEXPORT int	pfdLoadObjectRef_pfb(pfObject **obj, void *handle);
extern PFPFB_DLLEXPORT int	pfdStoreObjectRef_pfb(pfObject *obj, void *handle);
extern PFPFB_DLLEXPORT int	pfdCrypt_pfb(uint *key,
			     pfdEncryptFuncType_pfb encrypt_func,
			     pfdDecryptFuncType_pfb decrypt_func);
extern PFPFB_DLLEXPORT int	pfdUserDataSlot_pfb(int slot,
				    pfdDescendUserDataFuncType_pfb descend_func,
				    pfdStoreUserDataFuncType_pfb store_func,
				    pfdLoadUserDataFuncType_pfb load_func);
extern PFPFB_DLLEXPORT int	pfdUserData_pfb(pfdDescendUserDataFuncType_pfb descend_func,
				pfdStoreUserDataFuncType_pfb store_func,
				pfdLoadUserDataFuncType_pfb load_func);
extern PFPFB_DLLEXPORT int	pfdAddCustomNode_pfb(pfType *type, const char *name,
				  pfdDescendCustomNodeFuncType_pfb descend_func,
				  pfdStoreCustomNodeFuncType_pfb store_func,
				  pfdNewCustomNodeFuncType_pfb new_func,
				  pfdLoadCustomNodeFuncType_pfb load_func);
extern PFPFB_DLLEXPORT int	pfdDeleteCustomNode_pfb(pfType *type);
extern PFPFB_DLLEXPORT void	pfdLoadNeededDSOs_pfb(const char *fileName);
extern PFPFB_DLLEXPORT int	pfdCleanShare_pfb(void);

#ifndef __cplusplus
#define pfdDescendObject_pfb(_obj, _handle) \
	pfdDescendObject_pfb((pfObject *)(_obj), _handle)
#define pfdLoadObjectRef_pfb(_obj, _handle) \
	pfdLoadObjectRef_pfb((pfObject **)(_obj), _handle)
#define pfdStoreObjectRef_pfb(_obj, _handle) \
	pfdStoreObjectRef_pfb((pfObject *)(_obj), _handle)
#endif


/*
 *  typedefs and prototypes for pfa
 */
typedef pfdDescendUserDataFuncType_pfb	 pfdDescendUserDataFuncType_pfa;
typedef pfdStoreUserDataFuncType_pfb	 pfdStoreUserDataFuncType_pfa;
typedef pfdLoadUserDataFuncType_pfb	 pfdLoadUserDataFuncType_pfa;
typedef pfdDescendCustomNodeFuncType_pfb pfdDescendCustomNodeFuncType_pfa;
typedef pfdStoreCustomNodeFuncType_pfb	 pfdStoreCustomNodeFuncType_pfa;
typedef pfdNewCustomNodeFuncType_pfb	 pfdNewCustomNodeFuncType_pfa;
typedef pfdLoadCustomNodeFuncType_pfb	 pfdLoadCustomNodeFuncType_pfa;

extern PFPFB_DLLEXPORT pfNode*	pfdLoadFile_pfa(const char *fileName);
extern PFPFB_DLLEXPORT int	pfdStoreFile_pfa(pfNode *root, const char *fileName);
extern PFPFB_DLLEXPORT void	pfdConverterMode_pfa(int mode, int value);
extern PFPFB_DLLEXPORT int	pfdGetConverterMode_pfa(int mode);
extern PFPFB_DLLEXPORT void	pfdConverterVal_pfa(int which, float val);
extern PFPFB_DLLEXPORT float	pfdGetConverterVal_pfa(int which);
extern PFPFB_DLLEXPORT void	pfdConverterAttr_pfa(int which, void *attr);
extern PFPFB_DLLEXPORT void*	pfdGetConverterAttr_pfa(int which);
extern PFPFB_DLLEXPORT size_t	pfdLoadData_pfa(void *data, int size, void *handle);
extern PFPFB_DLLEXPORT size_t	pfdStoreData_pfa(void *data, int size, void *handle);
extern PFPFB_DLLEXPORT int	pfdDescendObject_pfa(pfObject *obj, void *handle);
extern PFPFB_DLLEXPORT int	pfdLoadObjectRef_pfa(pfObject **obj, void *handle);
extern PFPFB_DLLEXPORT int	pfdStoreObjectRef_pfa(pfObject *obj, void *handle);
extern PFPFB_DLLEXPORT int	pfdUserDataSlot_pfa(int slot,
				    pfdDescendUserDataFuncType_pfa descend_func,
				    pfdStoreUserDataFuncType_pfa store_func,
				    pfdLoadUserDataFuncType_pfa load_func);
extern PFPFB_DLLEXPORT int	pfdUserData_pfa(pfdDescendUserDataFuncType_pfa descend_func,
				pfdStoreUserDataFuncType_pfa store_func,
				pfdLoadUserDataFuncType_pfa load_func);
extern PFPFB_DLLEXPORT int	pfdAddCustomNode_pfa(pfType *type, const char *name,
				  pfdDescendCustomNodeFuncType_pfa descend_func,
				  pfdStoreCustomNodeFuncType_pfa store_func,
				  pfdNewCustomNodeFuncType_pfa new_func,
				  pfdLoadCustomNodeFuncType_pfa load_func);
extern PFPFB_DLLEXPORT int	pfdDeleteCustomNode_pfa(pfType *type);
extern PFPFB_DLLEXPORT void	pfdLoadNeededDSOs_pfa(const char *fileName);
extern PFPFB_DLLEXPORT int	pfdCleanShare_pfa(void);

#ifndef __cplusplus
#define pfdDescendObject_pfa(_obj, _handle) \
	pfdDescendObject_pfa((pfObject *)(_obj), _handle)
#define pfdLoadObjectRef_pfa(_obj, _handle) \
	pfdLoadObjectRef_pfa((pfObject **)(_obj), _handle)
#define pfdStoreObjectRef_pfa(_obj, _handle) \
	pfdStoreObjectRef_pfa((pfObject *)(_obj), _handle)
#endif


#ifdef __cplusplus
}
#endif

#endif /* __PFPFB_H__ */
