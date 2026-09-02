/******************************************************************************
**
** (C) Copyright 1991, 1992, 1993 Lightscape Graphics Software, Ltd.
**
** $Header: /ptools/plroot/performer/2.6/perf/lib/libpfdb/libpflsb/RCS/pflsb.h,v 1.10 2002/11/07 19:03:11 rmech Exp $
**
** $Log: pflsb.h,v $
** Revision 1.10  2002/11/07 19:03:11  rmech
** ported to win32
**
** Revision 1.9  1995/04/06 05:08:43  mtj
** removed unneeded fields
**
 * Revision 1.8  1995/01/05  21:14:19  jrohlf
 * pfdBuilder/pfdGeoBuilder API changes
 *
 * Revision 1.7  1994/11/19  05:30:10  mtj
 * db/io rename straggler
 *
 * Revision 1.6  1994/11/02  19:50:07  mtj
 * convert file loaders to use the pfdb library
 * where appropriate
 *
 * Revision 1.5  1994/10/26  23:39:41  mtj
 * changes for -ansiposix compatibility
 *
 * Revision 1.4  1994/08/20  01:15:46  mtj
 * massive long->int conversion
 *
 * Revision 1.3  1994/03/26  02:07:06  jrohlf
 * fix broken tagalong from 1.2
 *
 * Revision 1.3  1994/03/01  17:41:03  mtj
 * new lightscape lsb loader -- uses the builder
 *
**
******************************************************************************/

#ifndef __PFLSB_H__
#define __PFLSB_H__

#define LSB_VERSION_1  1

#define LSB_SIGNATURE  "Lightscape Radiosity LSB Format"
#define LSB_BEGIN_CHAR  '!'

/* PatchCluster Flags */
#define LSB_BIG_CLUSTER  0x0001
#define LSB_TEXTURED	 0x0002

typedef struct lsbHeader {
    short LSBversion;
    short fileFlags;
    unsigned int  numPatchClusters;
    unsigned int  numTopNodes;
    unsigned int  numNodes;
    unsigned int  numTopPatchVertices;
    unsigned int  numPatchVertices;
    unsigned int  numMaterials;
    unsigned int  numLayers;
    unsigned int  numTextures;
} LSBheader;

typedef struct lsbTopPatchVertex {
    float coord[3];
    float color[3];
} LSBtopPatchVertex;

typedef struct lsbPatchVertex {
    float color[3];
} LSBpatchVertex;

typedef struct lsbTopPatchVertexTextured {
    float coord[3];
    float color[3];
    float uv[2];
} LSBtopPatchVertexTextured;

typedef struct lsbPatchCluster {
    short flags;
    float plane[4];
    unsigned int  materialId;
    unsigned int  layerId;
    unsigned int  textureId;
    unsigned short numTopNodes;
    unsigned short numNodes;
    unsigned short numTopVertices;
    unsigned short numVertices;
} LSBpatchCluster;

typedef struct lsbPatchLeaf {
    unsigned short vertices[4];
    unsigned short midVertices[4];
} LSBpatchLeaf;

typedef struct lsbPatchClusterLong {
    short flags;
    float plane[4];
    unsigned int  materialId;
    unsigned int  layerId;
    unsigned int  textureId;
    unsigned int  numTopNodes;
    unsigned int  numNodes;
    unsigned int  numTopVertices;
    unsigned int  numVertices;
} LSBpatchClusterLong;

typedef struct lsbPatchLeafLong {
    unsigned int  vertices[4];
    unsigned int  midVertices[4];
} LSBpatchLeafint ;

#define NODE_IS_TRIANGLE(node)    ((node)->vertices[3] == (LSpatchVertex *)0)
#define NODE_IS_RECT(node)      ((node)->vertices[3] != (LSpatchVertex *)0)
#define ELEMENT_IS_TRIANGLE(e)    NODE_IS_TRIANGLE(e->node)
#define ELEMENT_IS_RECT(e)      NODE_IS_RECT(e->node)
#define NUM_NODE_VERTICES(node)    (NODE_IS_TRIANGLE(node) ? 3 : 4)
#define NUM_PATCH_VERTICES(node)  (PATCH_IS_TRIANGLE(node) ? 3 : 4)
#define NUM_ELEMENT_VERTICES(node)  (ELEMENT_IS_TRIANGLE(node) ? 3 : 4)

typedef struct lsMaterial {
    char *name;
    pfGroup *group;
} LSmaterial;

typedef struct lsLayer {
    char *name;
} LSlayer;

typedef struct lsTexture {
    char 		*name;
    pfTexture 		*tex;
    pfGeoState 		*gstate;
} LStexture;

typedef struct lsPatchVertex {
    unsigned short flags;
    unsigned int  index;
} LSpatchVertex;

typedef struct lsPatchNode {
    short isLeaf;
    LSpatchVertex *vertices[4];
    struct lsPatchNode *children[4];
} LSpatchNode;

typedef struct lsPatchCluster {
    unsigned short flags;
    pfVec4 plane;
    LSmaterial *material;
    LSlayer *layer;
    LStexture *texture;
    pfNode *node;
    pfGeoSet *gset;
} LSpatchCluster;

#endif

