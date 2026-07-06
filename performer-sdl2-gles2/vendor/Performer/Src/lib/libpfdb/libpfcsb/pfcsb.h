/*
 * Copyright (C) 1996, Silicon Graphics, Inc.
 * All Rights Reserved.
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
 *  pfCsb.h $Revision: 1.5 $
 */


#ifndef __PFCSB_H__
#define __PFCSB_H__


#include <sys/types.h>


#ifdef __cplusplus
extern "C" {
#endif


#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFCSB_DLLEXPORT __declspec(dllexport)
#else
#define PFCSB_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFCSB_DLLEXPORT
#endif

/*
 *  defines for csdConverterMode_csb
 */
#define CSCSB_SAVE_TEXTURE_IMAGE	  1
#define CSCSB_SAVE_TEXTURE_PATH		  2
#define CSCSB_SHARE_GS_OBJECTS		  3
#define CSCSB_COMPILE_GL		  4


/*
 *  values for csdConverterMode_csb(CSCSB_COMPILE_GL, value)
 */
#define CSCSB_COMPILE_GL_OFF		  0
#define CSCSB_COMPILE_GL_ON		  1
#define CSCSB_COMPILE_GL_AS_SAVED	  2

/*
 * History of version numbers
 */
#define CSB_VERSION_BILLBOARD_FORWARD_UP     -890 // Added forward and up
						  // vectors to csBillboard
#define CSB_VERSION_FOG                      -890 // New csFog node
#define CSB_VERSION_TRANSFORM                -893 // Save all transformation
						  // info not just matrix.
#define CSB_VERSION_MOVED_MATERIAL_BIT       -893 // moved bit in csContext 
						  // for stipple transp.
#define CSB_VERSION_ADDED_MATERIAL_MODE      -893 // added material mode enable
						  // to appearance
#define CSB_VERSION_BITMASK64_FOG            -894 // added fog modes, upped 
						  // bitmask to 64 bits
#define CSB_VERSION_ADDED_DEPTH_ENABLE       -895 // added depthEnable field 
						  // to appearance
#define CSB_VERSION_REORDERD_INHERTANCE_MASK -896 // Used to reorder the 
						  // inhertance mask
#define CSB_VERSION_FIXED_OPBOOL             -897 // Checkpoint for defining
						  // opBool as int instead
						  // of a size defined by
						  // OS or platform
#define CSB_VERSION_BPCULL_DATA              -898 // added backpatch culling 
						  // data
#define CSB_VERSION_TRIM_LOOP_CLOSED         -899 // added trim loop closed 
						  // info to opParaSurface.
#define CSB_VERSION_TEX_STIP_N_TWOSIDE       -990 // added texture transform,
						  // line stipple, and two-sided                                                  // materials
#define CSB_VERSION_CONNECTIONS              -991
#define CSB_VERSION_CUSTOMSTORELOAD          -992

// Current version number
#define CSB_VERSION_NUMBER      	     -890


typedef int32_t         int32;
typedef float   csFloat;

   // max table value
   
#define L_COUNT         42

/*
 *  typedefs prototypes for csb
 */

/*
 *  structure used to to make lists of things to store
 */
typedef struct list_s {
    void *data;
    int id;
    int size;
    struct list_s *next;
    struct list_s *bnext;
} list_t;

typedef struct
{
    int flat_shade;
    int wire_frame;
    
} appearance_info_t;

#ifdef __cplusplus
}
#endif


/*
 *  Structure used to specify the current csb file and the state used
 *  in loading or storing it.
 */
 
class csdFile_csb{
public:
    csdFile_csb() {};
    ~csdFile_csb(){};

public:
    const char *func_name;
    FILE *fp;
    int endian_flip;
    size_t (*fread)(void *ptr, size_t size, size_t nitems, FILE *stream);
    int l_num[L_COUNT];
    int buf_size;
    int32 *buf;
    pfNode *root;
    int root_list;
    int read_error;
    int32 version;
    void **rl_list[L_COUNT];
    int32 *geo_ref;              // keep track of useage of geoSets
    int32 *iset_size;            // keep track of index set lengths
    int32 *gstate_mtl_mode;      // keep track of geoStates' materialMode flag
    void *arena;
    int32 curListLength;
    
};   // end class csdFile_csb


#ifdef __cplusplus
extern "C" {
#endif

typedef void (*csdDecryptFuncType_csb)(int size, uint *key, void *data);

typedef void (*csdEncryptFuncType_csb)(int size, uint *key, void *src,
				       void *dst);
				       
typedef int (*csdDescendUserDataFuncType_csb)(void *udata, void *parent,
					      csdFile_csb *csbf);
					      
typedef int (*csdStoreUserDataFuncType_csb)(void *udata, void *parent,
					    csdFile_csb *csbf);
					    
typedef void *(*csdLoadUserDataFuncType_csb)(csdFile_csb *csbf);

typedef int (*csdDescendCustomObjFuncType_csb)(void *obj,
					       csdFile_csb *csbf);
					       
typedef int (*csdStoreCustomObjFuncType_csb)(void *obj,
					     csdFile_csb *csbf);
					     
typedef void *(*csdNewCustomObjFuncType_csb)(void);

typedef int (*csdLoadCustomObjFuncType_csb)(void *obj,
					    csdFile_csb *csbf);


PFCSB_DLLEXPORT pfNode*	pfdLoadFile_csb(const char *fileName);


PFCSB_DLLEXPORT void	pfdConverterMode_csb(int mode, int value);
PFCSB_DLLEXPORT int	pfdGetConverterMode_csb(int mode);
PFCSB_DLLEXPORT void	pfdConverterVal_csb(int which, float val);
PFCSB_DLLEXPORT float	pfdGetConverterVal_csb(int which);
PFCSB_DLLEXPORT void	pfdConverterAttr_csb(int which, void *attr);
PFCSB_DLLEXPORT void*	pfdGetConverterAttr_csb(int which);
PFCSB_DLLEXPORT size_t	pfdLoadData_csb(void *data, size_t size, csdFile_csb *csbf);
PFCSB_DLLEXPORT size_t	pfdStoreData_csb(void *data, size_t size, csdFile_csb *csbf);

#define csdLoadObject_csb(index,_obj, _csbf) \
	_csbf->callLoadFunc(index,(void *)(_obj), _csbf)

#define csdNewObject_csb(index, _csbf) \
	_csbf->callNewFunc(index, _csbf)
	
#define CSB_FREAD 	csb->fread 
#define CSB_FWRITE 	fwrite

#ifdef __cplusplus
}
#endif

#endif /* __PFCSB_H__ */
