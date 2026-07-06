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
 * random.c: $Revision: 1.8 $ $Date: 2002/05/15 00:31:09 $
 */

#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>

#ifdef mips  /* already defined in stdlib.h */
#ifdef  _POSIX_SOURCE
extern long random(void);
extern int srandom(int seed);
#endif
#endif

#ifdef WIN32
#define srandom srand
#define random rand
#endif

#include <Performer/pf.h>
#include <Performer/pfutil.h>

/*
 * FUNCTION:
 *	pfuRandomize
 *
 * DESCRIPTION:
 *	reset the random number generator to a known state.
 */
extern PFUDLLEXPORT void
pfuRandomize (int seed)
{
    srandom(seed);
}

extern PFUDLLEXPORT long
pfuRandomLong (void)
{
    return random();
}

extern PFUDLLEXPORT float
pfuRandomFloat (void)
{
    return (float)(pfuRandomLong() & 0xffff)/65535.0f;
}

/*
 * FUNCTION:
 *	pfuRandomColor
 *
 * DESCRIPTION:
 *	pick a random color within a designated range.
 */
extern PFUDLLEXPORT void
pfuRandomColor (pfVec4 rgba, float minColor, float maxColor)
{
    if (rgba != NULL)
    {
	rgba[0] = minColor + (maxColor - minColor)*pfuRandomFloat();
	rgba[1] = minColor + (maxColor - minColor)*pfuRandomFloat();
	rgba[2] = minColor + (maxColor - minColor)*pfuRandomFloat();
	rgba[3] = 1.0f;
    }
}
