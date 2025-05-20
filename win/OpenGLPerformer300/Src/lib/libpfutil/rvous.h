/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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
 */

/*
 * rvous.h
 *
 * $Revision: 1.7 $
 *
 */

#ifndef __RVOUS__
#define __RVOUS__

#include <Performer/pr.h>
#include <Performer/pfutil-DLL.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PFURV_MAXSLAVES		2

#define PFURV_GARBAGE		-1
#define PFURV_READY		10
#define PFURV_SYNC		11
#define PFURV_SYNCACK		12
#define PFURV_RESUME		13

typedef struct 
{
    int	master;
    int	numSlaves;
    int	slaves[PFURV_MAXSLAVES];

} pfuRendezvous;

extern PFUDLLEXPORT void pfuInitRendezvous(pfuRendezvous *rvous, int numSlaves);
extern PFUDLLEXPORT void pfuMasterRendezvous(pfuRendezvous *rvous);
extern PFUDLLEXPORT void pfuSlaveRendezvous(pfuRendezvous *rvous, int id);

#ifdef __cplusplus
}
#endif

#endif
