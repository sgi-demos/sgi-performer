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
 * pfpost.h 
 *
 *    $Revision: 1.3 $
 *    $Date: 2002/11/07 18:06:03 $
 *
 */

#ifndef __PFPOST_H__
#define __PFPOST_H__

#ifdef WIN32
#include <windows.h>
#include "wintypes.h"
#endif

#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFPFB_DLLEXPORT
#endif


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Used as the mode argument to 
 * pfdConverterMode() and pfdGetConverterMode() 
 */
#define PFPOST_COLORS	0
#define PFPOST_NORMALS	1
#define PFPOST_CELLSIZE	2  /* CellSize X CellSize per geoset */
#define PFPOST_BRANCH	3  /* Maximum branching factor at groups/geodes */

/*
 * Used as the mode argument to 
 * pfdConverterAttr() and pfdGetConverterAttr() 
 */

#define PFPOST_TEXFILE 	1

PFPFB_DLLEXPORT extern pfNode* pfdLoadFile_post (char *fileName);

PFPFB_DLLEXPORT extern void    pfdConverterMode_post(int which, int val);
PFPFB_DLLEXPORT extern int     pfdGetConverterMode_post(int mode);

PFPFB_DLLEXPORT extern void    pfdConverterAttr_post(int which, void* attr);
PFPFB_DLLEXPORT extern void*   pfdGetConverterAttr_post(int which);

#ifdef __cplusplus
}
#endif

#endif

