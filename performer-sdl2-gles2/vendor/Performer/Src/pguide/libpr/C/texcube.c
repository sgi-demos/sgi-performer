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
 * texcube.c		routine for constructing a textured cube geoset
 *
 * $Revision: 1.10 $ $Date: 2000/10/06 21:26:47 $
 */

#include <Performer/pr.h>

#define	CUBE_SIZE		1.0f

pfGeoSet*
MakeTexCube(void *arena)
{
    pfGeoSet *gset;
    pfGeoState *gst;
    pfTexture *tex;
    pfTexEnv *tenv;

    pfVec3 *verts, *norms;
    pfVec2 *tcoords;
    ushort *vindex, *nindex, *tindex;

    /*
     * Data arrays to be passed to pfGSetAttr should be allocated
     * from heap memory...
     */
    verts = (pfVec3 *)pfMalloc(8 * sizeof(pfVec3), arena);
    pfSetVec3(verts[0], -CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE);
    pfSetVec3(verts[1],  CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE);
    pfSetVec3(verts[2],  CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE);
    pfSetVec3(verts[3], -CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE);
    pfSetVec3(verts[4], -CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE);
    pfSetVec3(verts[5],  CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE);
    pfSetVec3(verts[6],  CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE);
    pfSetVec3(verts[7], -CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE);

    vindex = (ushort *)pfMalloc(24 * sizeof(ushort), arena);
    vindex[0]  = 0; vindex[1]  = 1; vindex[2]  = 2; vindex[3]  = 3; /* front */
    vindex[4]  = 0; vindex[5]  = 3; vindex[6]  = 7; vindex[7]  = 4; /* left */
    vindex[8]  = 4; vindex[9]  = 7; vindex[10] = 6; vindex[11] = 5; /* back */
    vindex[12] = 1; vindex[13] = 5; vindex[14] = 6; vindex[15] = 2; /* right */
    vindex[16] = 3; vindex[17] = 2; vindex[18] = 6; vindex[19] = 7; /* top */
    vindex[20] = 0; vindex[21] = 4; vindex[22] = 5; vindex[23] = 1; /* bottom */

    norms = (pfVec3 *)pfMalloc(6 * sizeof(pfVec3), arena);
    pfSetVec3(norms[0],  0.0f,  0.0f,  1.0f);
    pfSetVec3(norms[1],  0.0f,  0.0f, -1.0f);
    pfSetVec3(norms[2],  0.0f,  1.0f,  0.0f);
    pfSetVec3(norms[3],  0.0f, -1.0f,  0.0f);
    pfSetVec3(norms[4],  1.0f,  0.0f,  0.0f);
    pfSetVec3(norms[5], -1.0f,  0.0f,  0.0f);

    nindex = (ushort *)pfMalloc(6 * sizeof(ushort), arena);
    nindex[0] = 0; nindex[1] = 5; nindex[2] = 1;
    nindex[3] = 4; nindex[4] = 2; nindex[5] = 3;

    tcoords = (pfVec2 *)pfMalloc(4 * sizeof(pfVec2), arena);
    pfSetVec2(tcoords[0], 0.0f, 0.0f);
    pfSetVec2(tcoords[1], 1.0f, 0.0f);
    pfSetVec2(tcoords[2], 1.0f, 1.0f);
    pfSetVec2(tcoords[3], 0.0f, 1.0f);

    tindex = (ushort *)pfMalloc(24 * sizeof(ushort), arena);
    tindex[0]  = 0; tindex[1]  = 1; tindex[2]  = 2; tindex[3]  = 3;
    tindex[4]  = 0; tindex[5]  = 1; tindex[6]  = 2; tindex[7]  = 3;
    tindex[8]  = 0; tindex[9]  = 1; tindex[10] = 2; tindex[11] = 3;
    tindex[12] = 0; tindex[13] = 1; tindex[14] = 2; tindex[15] = 3;
    tindex[16] = 0; tindex[17] = 1; tindex[18] = 2; tindex[19] = 3;
    tindex[20] = 0; tindex[21] = 1; tindex[22] = 2; tindex[23] = 3;

    gset = pfNewGSet(arena);

    /*
     * set the coordinate, normal and color arrays
     * and their corresponding index arrays
     */
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, verts, vindex);
    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_PRIM, norms, nindex);
    pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, tindex);
    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 6);
    
    /*
     * create a geostate from shared memory, enable 
     * texturing (in case that's not the default), and
     * set the geostate for this geoset 
     */
    gst = pfNewGState(arena);
    pfGStateMode(gst, PFSTATE_ENTEXTURE, 1);
    pfGSetGState(gset, gst);
    
    /* 
     * create a new texture from shared memory,
     * load a texture file and add texture to geostate 
     */
    tex = pfNewTex(arena);
    pfLoadTexFile(tex, "brick.rgba");
    pfGStateAttr(gst, PFSTATE_TEXTURE, tex);

    /*
     * create a new texture environment from shared memory,
     * decal the texture since the geoset has no color to 
     * modulate, set the texture environment for this geostate
     */
    tenv = pfNewTEnv(arena);
    pfTEnvMode(tenv, PFTE_DECAL);
    pfGStateAttr(gst, PFSTATE_TEXENV, tenv);

    return gset;
}
