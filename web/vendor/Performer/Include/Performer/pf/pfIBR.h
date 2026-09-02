//
// Copyright 1999, 2000, Silicon Graphics, Inc.
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
// pfIBR.h
//

#ifndef __PF_IBR_H__
#define __PF_IBR_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pr.h>
#include <Performer/prmath.h>
#include <Performer/pr/pfType.h>
#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfCombiner.h>

#include <Performer/pf.h>
#include <Performer/pf/pfChannel.h>

// Export to C API.
extern "C" {
/* constants */
#define PFDD_MAX_RINGS 128   /* maximum number of rings of views */

/* different ways of generating directions */
#define PFDD_2D_ROTATE_AROUND_UP    1
#define PFDD_RINGS_OF_VIEWS         2
#define PFDD_3D_UNIFORM             3

/* flags */
#define PFIBR_USE_2D_TEXTURES   (1<<1)
#define PFIBR_3D_VIEWS          (1<<2)
#define PFIBR_NEAREST           (1<<3)
#define PFIBR_USE_REG_COMBINERS (1<<4)
#define PFIBR_MIPMAP_3DTEXTURES (1<<5)
#define PFIBR_USE_PROXY         (1<<6)
#define PFIBR_FAST_BLEND        (1<<7)


}

typedef struct _pfRingInfo {
    int   index0;    // index of the first direction in this ring
    int   gindex0;   // index of the first group in this ring
    int   numDirs;   // directions in this ring
    float horAngle;  // angle from horizon;
} _pfRingInfo;

#define PFDD_EDGES_PER_VERTEX  30

// vertices
typedef struct {
    int neighbors[PFDD_EDGES_PER_VERTEX+1]; /* neighboring vertices 
                                               (the last one must be -1) */
} _pfVertType;

// triangles
typedef struct
{
    int    verts[3];
    int    neighbors[3];
    pfVec3 edgeNormals[3];
} _pfTriType;

typedef struct _pfTriInfo {
    int           numVert;
    _pfVertType *vertices;
    int           numTris;
    _pfTriType  *tris;
    pfVec3      extraPt;
} _pfTriInfo;

struct _pfGridType;

//////////////////////////////////////////////////////////////////////
#define PFDIRDATA ((pfDirData*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFDIRDATABUFFER ((pfDirData*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfDirData : public pfObject {

    //class type
    static pfType  *classType;

 private:
    int     numDirs;   // number of directions
    pfVec3  *dirs;     // set of 3d directions
    void    **data;    // set of void pointers
    int     flags;

    // structures to speed-up finding of nearest views
    int      numRings;      // # of rings (1 ring in 2d case)
    _pfRingInfo rings[PFDD_MAX_RINGS];
    int      ringIndex[PFDD_MAX_RINGS+1]; // index of the first view in the ring

    _pfTriInfo  tris;
    struct _pfGridType *grid;
    int         xsize, ysize;

public:

    pfDirData();
    virtual ~pfDirData();
    
    //// type checking functions
    static void init();
    static pfType* getClassType() {return classType;}

    // 
    void setData(int num, pfVec3 *directions, void **userData);
    void getDirData(int *num, pfVec3 **directions, void ***userData);

    void setDirections(int num, pfVec3 *directions);
    void generateDirections(int num, int type, float *data);
    void getDirections(int *num, pfVec3 **directions);

    int  getNumGroups(int *viewsPerGroup);
    int  getGroup(int group, int *views);
    int  getNeighboringViews(int viewIndex, int **neighbors);

    int  getFlags(int which) const {
	return flags & which;
    }

    int  findData(pfVec3 *dir, pfVec3 *resDir, void **resData);
    int  findData2(pfVec3 *dir, pfVec3 *resDir, void **resData, float *weight);
    int  findData3(pfVec3 *dir, pfVec3 *resDir, void **resData, float *weight);
    int  findData4(pfVec3 *dir, pfVec3 *resDir, void **resData, float *weight);


public:
    void *_pfDirDataExtraData;
};

/***************************************************************************/


#define PFIBRTEXTURE ((pfIBRtexture*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFIBRTEXTUREBUFFER ((pfIBRtexture*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfIBRtexture : public pfObject {

    //class type
    static pfType  *classType;

 private:
    pfTexture **tex;
    pfDirData *texPerView;

    float    direction;
    int      numRings;      // # of rings (1 ring in 2d case)
    int      ringIndex[PFDD_MAX_RINGS+1]; // index of the first texture in the ring (used by 3D mipmapped textures only)
    int      rsize[PFDD_MAX_RINGS]; // used by 3D textures only
    int      flags;

    static pfCombiner *combiner;  // used on NVidia hw (static to reduce state changes)
    static pfCombiner *combiner3D; // for blending 4 textures

public:

    pfIBRtexture();
    virtual ~pfIBRtexture();
    
    //// type checking functions
    static void init();
    static pfType* getClassType() {return classType;}

    // 
    void loadIBRTexture(char *_format, int _numTex, int _skipTex);
    void setIBRTexture(pfTexture **_textures, int _numTex);
    void getIBRTextures(pfTexture ***textures, int *numTex);
    void setIBRdirections(pfVec3 *directions, int numDirs);
    void getIBRdirections(pfVec3 **directions, int *numDirs);

    int  getNumIBRTextures(void);
    pfDirData *getDirData(void)
    {
	return texPerView;
    }

    void computeProxyTexCoords(pfIBRnode *node, pfVec3 *center, float aspect, float scale, pfVec2 *texShift);
    pfTexture *getDefaultTexture(void);

    void setFlags(int which, int value);
    int  getFlags(int which) const {
	return flags & which;
    }

    void setDirection(float dir);
    float  getDirection(void) const {
	return direction;
    }


public:
    void *_pfIBRtexExtraData;
};

#endif //__PF_IBR_H__
