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
 * pfmedit.h 
 *
 *    $Revision: 1.38 $
 *    $Date: 2002/11/07 23:44:16 $
 *
 */
#ifndef __PFMEDIT_H__
#define __PFMEDIT_H__

#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFDB_DLLEXPORT __declspec(dllexport)
#else
#define PFDB_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFDB_DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /*---------------------- Medit Options ------------------------*/

/*
 * Medit loader modes.  Specify in mode argument to 
 * pfdConverterMode() and pfdGetConverterMode() 
 */

#define PFM_noflatten	 1	/* Don't flatten scene graph after load	    */
#define PFM_notexture	 2	/* Don't load textures			    */
#define PFM_lodfade	 4	/* Optimise LOD's for PFLOD_FADE	    */
#define PFM_fastlighting 8	/* Use lmcolor wherever possible	    */
#define PFM_nomultisample 16	/* Reality engine but not multisampling	    */
#define PFM_convertliteral 32	/* Each subobject becomes a named pfGroup   */


#ifdef __cplusplus
}
#endif

#endif

