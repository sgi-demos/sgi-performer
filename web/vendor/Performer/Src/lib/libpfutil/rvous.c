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
 * rvous.c
 *
 * $Revision: 1.9 $
 *
 */

#include "rvous.h"

static int slaveState(pfuRendezvous *rvous);

PFUDLLEXPORT void 
pfuInitRendezvous(pfuRendezvous *rvous, int numSlaves)
{
    int	i;

    rvous->master = PFURV_GARBAGE;

    for(i=0; i<PFURV_MAXSLAVES; i++)
	rvous->slaves[i] = PFURV_GARBAGE;

    rvous->numSlaves = numSlaves;
}

PFUDLLEXPORT void 
pfuMasterRendezvous(pfuRendezvous *rvous)
{
    while(slaveState(rvous) != PFURV_READY) {}
    rvous->master = PFURV_SYNC;
    while(slaveState(rvous) != PFURV_SYNCACK) {}
    rvous->master = PFURV_RESUME;
}

PFUDLLEXPORT void 
pfuSlaveRendezvous(pfuRendezvous *rvous, int id)
{
    rvous->slaves[id] = PFURV_READY;
    while(rvous->master != PFURV_SYNC) {}
    rvous->slaves[id] = PFURV_SYNCACK;
    while(rvous->master == PFURV_SYNC) {}
}

static int
slaveState(pfuRendezvous *rvous)
{
    int	i, val;

    val = rvous->slaves[0];

    for(i=1; i<rvous->numSlaves; i++)
	if(rvous->slaves[i] != val)
	    return PFURV_GARBAGE;

    return val;
}
