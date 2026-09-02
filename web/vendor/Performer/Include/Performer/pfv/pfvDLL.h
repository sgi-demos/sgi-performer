//
// Copyright 2001, 2002 Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfvDLL.h
//
// $Revision: 1.3 $
// $Date: 2002/11/14 22:00:41 $
//

#ifndef __PFV_DLL_H
#define __PFV_DLL_H

#ifndef WIN32
#define PFV_DLLEXPORT
#else
#ifdef __BUILD_PFV__
#define PFV_DLLEXPORT __declspec(dllexport)
#else
#define PFV_DLLEXPORT __declspec(dllimport)
#endif /*__BUILD_PFV__ */
#endif  /* WIN32 */

#ifndef WIN32
#define PFVM_DLLEXPORT
#else
#define PFVM_DLLEXPORT __declspec(dllexport)
#endif

#endif /* __PFV_DLL_H */
