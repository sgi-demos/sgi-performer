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
 * file: colorcube.c		
 * -----------------
 *
 * routine for constructing colored cube geoset
 *
 * $Revision: 1.11 $ $Date: 2000/10/06 21:26:43 $
 */

#include <Performer/pr.h>
#include <stdlib.h>

#define	CUBE_SIZE		1.0f

pfGeoSet*
MakeColorCube(void *arena)
{
    pfGeoSet *gset;
    pfGeoState *gst;

    pfVec4 *scolors;
    pfVec3 *snorms, *scoords;
    ushort *nindex, *vindex, *cindex;

    /*
     * Data arrays to be passed to pfGSetAttr should be allocated
     * from heap memory...
     */
    scolors = (pfVec4 *)pfMalloc(4 * sizeof(pfVec4), arena);
    pfSetVec4(scolors[0], 1.0f, 0.0f, 0.0f, 0.5f);
    pfSetVec4(scolors[1], 0.0f, 1.0f, 0.0f, 0.5f);
    pfSetVec4(scolors[2], 0.0f, 0.0f, 1.0f, 0.5f);
    pfSetVec4(scolors[3], 1.0f, 1.0f, 1.0f, 0.5f);

    snorms = (pfVec3 *)pfMalloc(6 * sizeof(pfVec3), arena);
    pfSetVec3(snorms[0],  0.0f,  0.0f,  1.0f);
    pfSetVec3(snorms[1],  0.0f,  0.0f, -1.0f);
    pfSetVec3(snorms[2],  0.0f,  1.0f,  0.0f);
    pfSetVec3(snorms[3],  0.0f, -1.0f,  0.0f);
    pfSetVec3(snorms[4],  1.0f,  0.0f,  0.0f);
    pfSetVec3(snorms[5], -1.0f,  0.0f,  0.0f);

    scoords = (pfVec3 *)pfMalloc(8 * sizeof(pfVec3), arena);
    pfSetVec3(scoords[0], -CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE);
    pfSetVec3(scoords[1],  CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE);
    pfSetVec3(scoords[2],  CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE);
    pfSetVec3(scoords[3], -CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE);
    pfSetVec3(scoords[4], -CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE);
    pfSetVec3(scoords[5],  CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE);
    pfSetVec3(scoords[6],  CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE);
    pfSetVec3(scoords[7], -CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE);

    nindex = (ushort *)pfMalloc(6 * sizeof(ushort), arena);
    nindex[0] = 0; nindex[1] = 5; nindex[2] = 1;
    nindex[3] = 4; nindex[4] = 2; nindex[5] = 3;

    vindex = (ushort *)pfMalloc(24 * sizeof(ushort), arena);
    vindex[0]  = 0; vindex[1]  = 1; vindex[2]  = 2; vindex[3]  = 3; /* front */
    vindex[4]  = 0; vindex[5]  = 3; vindex[6]  = 7; vindex[7]  = 4; /* left */
    vindex[8]  = 4; vindex[9]  = 7; vindex[10] = 6; vindex[11] = 5; /* back */
    vindex[12] = 1; vindex[13] = 5; vindex[14] = 6; vindex[15] = 2; /* right */
    vindex[16] = 3; vindex[17] = 2; vindex[18] = 6; vindex[19] = 7; /* top */
    vindex[20] = 0; vindex[21] = 4; vindex[22] = 5; vindex[23] = 1; /* bottom */

    cindex = (ushort *)pfMalloc(24 * sizeof(ushort), arena);
    cindex[0]  = 0; cindex[1]  = 1; cindex[2]  = 2; cindex[3]  = 3;
    cindex[4]  = 0; cindex[5]  = 1; cindex[6]  = 2; cindex[7]  = 3;
    cindex[8]  = 0; cindex[9]  = 1; cindex[10] = 2; cindex[11] = 3;
    cindex[12] = 0; cindex[13] = 1; cindex[14] = 2; cindex[15] = 3;
    cindex[16] = 0; cindex[17] = 1; cindex[18] = 2; cindex[19] = 3;
    cindex[20] = 0; cindex[21] = 1; cindex[22] = 2; cindex[23] = 3;

    gset = pfNewGSet(arena);

    /*
     * set the coordinate, normal and color arrays
     * and their corresponding index arrays
     */
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, scoords, vindex);
    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_PRIM, snorms, nindex);
    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, scolors, cindex);
    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 6);
    
    /* 
     * create a new geostate from shared memory,
     * disable texturing and enable transparency
     */
    gst = pfNewGState(arena);
    pfGStateMode(gst, PFSTATE_ENTEXTURE, 0);
    pfGStateMode(gst, PFSTATE_TRANSPARENCY, 1);

    pfGSetGState(gset, gst);
    return gset;
}
