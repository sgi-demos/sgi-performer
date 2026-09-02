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
 * file: cusror.h	
 * --------------
 *
 * $Revision: 1.2 $
 * $Date: 2002/04/11 23:44:51 $
 * 
 */

#ifndef __CURSOR_H__
#define __CURSOR_H__

#ifndef WIN32
#include <X11/X.h>
#include <X11/Xlib.h>
#endif

#include <Performer/pf.h>
#include <Performer/pfutil.h>

#define DIRTY_TYPE	    0x1
#define DIRTY_SEL	    0x2
#define DIRTY_COLOR	    0x4

typedef  struct pfuCursorMgmt
{
    int		dirty;
    int		type;
    int		curXCursor;
    Cursor	invisibleCursor;
    pfVec3	fgColor, bgColor;
    Cursor	XCursors[PFU_NUM_CUSORS];
} pfuCursorMgmt;

#endif /* __CURSOR_H__ */
