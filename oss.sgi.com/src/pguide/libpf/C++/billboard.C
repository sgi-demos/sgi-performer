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
 */

/*
 * billboard.c: routine to make simple billboards
 *
 * $Revision: 1.1 $ 
 * $Date: 2000/11/21 21:39:37 $
 */

#include <Performer/pf/pfBillboard.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfSprite.h>

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

    pfVec2 *BBTexCoords = (pfVec2*)pfMalloc(4*sizeof(pfVec2), arena);
    BBTexCoords[0].set(0.0f, 0.0f);
    BBTexCoords[1].set(1.0f, 0.0f);
    BBTexCoords[2].set(1.0f, 1.0f);
    BBTexCoords[3].set(0.0f, 1.0f);

    pfVec3 *BBVertCoords = (pfVec3*)pfMalloc(4*sizeof(pfVec3), arena);
                                  /* XZ plane for bbds  */
    BBVertCoords[0].set(-0.5f, 0.0f, 0.0f);
    BBVertCoords[1].set(0.5f, 0.0f, 0.0f);
    BBVertCoords[2].set(0.5f, 0.0f, 1.0f);
    BBVertCoords[3].set(-0.5f, 0.0f, 1.0f);

    pfVec3 *BBAxes = (pfVec3*)pfMalloc(4*sizeof(pfVec3), arena);
    BBAxes[0].set(1.0f, 0.0f, 0.0f);
    BBAxes[1].set(0.0f, 1.0f, 0.0f); /* Y */
    BBAxes[2].set(0.0f, 0.0f, 1.0f); /* Z */
    BBAxes[3].set(0.0f, 0.0f, 1.0f); /* pt world Z up*/
    
    int *BBPrimLens = (int *)pfMalloc(sizeof(int), arena);
    BBPrimLens[0] = 4;

    pfVec4 *BBColors = (pfVec4*)pfMalloc(sizeof(pfVec4), arena);
    BBColors[0].set(1.0f, 1.0f, 1.0f, 1.0f);

    gset = new pfGeoSet();

    gset->setAttr(PFGS_COORD3, 
		  PFGS_PER_VERTEX, BBVertCoords, NULL);
    gset->setAttr(PFGS_TEXCOORD2, 
		  PFGS_PER_VERTEX, BBTexCoords, NULL);
    gset->setAttr(PFGS_COLOR4, 
		  PFGS_OVERALL, BBColors, NULL);
    gset->setPrimLengths(BBPrimLens);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(1);
    gset->setGState(gst);
    
    bill = new pfBillboard();
    switch (bbType)
    {
    case PF_X:			/* axial */
    case PF_Y:
    case PF_Z:
	bill->setAxis(BBAxes[bbType]);
	bill->setMode(PFBB_ROT, PFBB_AXIAL_ROT);
	break;
    case 3:			/* point */
	bill->setAxis(BBAxes[bbType]);
	bill->setMode(PFBB_ROT, PFBB_POINT_ROT_WORLD);
	break;
    }
    bill->addGSet(gset);
    bill->setPos(0, pos);
    
    return bill;
}
