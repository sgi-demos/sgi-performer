/*
 * Copyright 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 *
 * pfrpc.h 
 *
 *    $Revision: 1.6 $
 *    $Date: 2002/11/17 01:40:32 $
 *
 */

#ifndef __PFRPC_H__
#define __PFRPC_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFRPC_DLLEXPORT __declspec(dllexport)
#else
#define PFRPC_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFRPC_DLLEXPORT
#endif

/*
 * Used as the mode argument to 
 * pfdConverterMode() and pfdGetConverterMode() 
 */
#define PFRPC_USE_USER_IBRNODE	1  /* use user defined pfIBRnode */


/*
 * Used as the argument to 
 * pfdConverterVal() and pfdGetConverterVal() 
 */
#define PFRPC_SKIP_TEXTURES	1  /* how may textures are skipped when 
				      reading the input file */

#define PFRPC_CROP_LEFT	        2  /* how many pixel are croped from input image */
#define PFRPC_CROP_RIGHT	3
#define PFRPC_CROP_TOP	        4
#define PFRPC_CROP_BOTTOM	5
#define PFRPC_SCALE_WIDTH	6
#define PFRPC_SCALE_HEIGHT	7
#define PFRPC_FLIP_TEXTURES	8  /* flip around horizontal axis */

#define PFRPC_NEAREST	              9
#define PFRPC_USE_NEAREST_RING	     10
#define PFRPC_RING_FILE	             11
#define PFRPC_COMBINED_TEXTURE_SIZE  12



/* Used as the mode argument to 
 * pfdConverterAttr() and pfdGetConverterAttr() 
 */
#define PFRPC_USER_IBRNODE 	1


extern PFRPC_DLLEXPORT pfNode* pfdLoadFile_rpc(char *fileName);

extern PFRPC_DLLEXPORT void    pfdConverterMode_rpc(int which, int val);
extern PFRPC_DLLEXPORT int     pfdGetConverterMode_rpc(int mode);

extern PFRPC_DLLEXPORT void    pfdConverterVal_rpc(int which, float val);
extern PFRPC_DLLEXPORT float   pfdGetConverterVal_rpc(int which);

extern PFRPC_DLLEXPORT void    pfdConverterAttr_rpc(int which, void* attr);
extern PFRPC_DLLEXPORT void*   pfdGetConverterAttr_rpc(int which);

#ifdef __cplusplus
}
#endif

#endif

