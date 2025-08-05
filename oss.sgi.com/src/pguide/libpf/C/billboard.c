/*
 * Copyright 1995, Silicon Graphics, Inc.
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
 *
 * billboard.c: routine to make simple billboards
 *
 * $Revision: 1.1 $ 
 * $Date: 2000/11/21 21:39:36 $
 */

#include <Performer/pf.h>

pfVec2          BBTexCoords[] ={{0.0f, 0.0f},
			       {1.0f, 0.0f},
			       {1.0f, 1.0f},
			       {0.0f, 1.0f}};

                                  /* XZ plane for bbds  */
pfVec3         BBVertCoords[4] = {{-0.5f, 0.0f, 0.0f},
				  { 0.5f, 0.0f, 0.0f},
				  { 0.5f, 0.0f, 1.0f},
				  {-0.5f, 0.0f, 1.0f}};

pfVec3		BBAxes[4] = {{1.0f, 0.0f, 0.0f}, /* X */
			     {0.0f, 1.0f, 0.0f}, /* Y */
			     {0.0f, 0.0f, 1.0f}, /* Z */
			     {0.0f, 0.0f, 1.0f}}; /* pt world Z up*/
    
int		BBPrimLens[] = { 4 };

pfVec4		BBColors[] = {{1.0, 1.0, 1.0, 1.0}};

/*
 * For pedagogical use only. Reasonable performance
 * requires more then one pfGeoSet per pfBillboard.
 */

pfBillboard*
MakeABill(pfVec3 pos, pfGeoState *gst, int bbType)
{
    pfGeoSet *gset;
    pfBillboard *bill;
    void *arena = pfGetSharedArena();
    
    gset = pfNewGSet(arena);

    pfGSetAttr(gset, PFGS_COORD3, 
	       PFGS_PER_VERTEX, BBVertCoords, NULL);
    pfGSetAttr(gset, PFGS_TEXCOORD2, 
	       PFGS_PER_VERTEX, BBTexCoords, NULL);
    pfGSetAttr(gset, PFGS_COLOR4, 
	       PFGS_OVERALL, BBColors, NULL);
    pfGSetPrimLengths(gset, BBPrimLens);
    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 1);
    pfGSetGState(gset, gst);
    
    bill = pfNewBboard();
    switch (bbType)
    {
    case PF_X:			/* axial */
    case PF_Y:
    case PF_Z:
	pfBboardAxis(bill, BBAxes[bbType]);
	pfBboardMode(bill, PFBB_ROT, PFBB_AXIAL_ROT);
	break;
    case 3:			/* point */
	pfBboardAxis(bill, BBAxes[bbType]);
	pfBboardMode(bill, PFBB_ROT, PFBB_POINT_ROT_WORLD);
	break;
    }
    pfAddGSet(bill, gset);
    pfBboardPos(bill, 0, pos);
    
    return bill;
}
