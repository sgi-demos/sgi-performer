/*
 * Copyright (c) 1993 Lightscape Graphics Software, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that (i) the above copyright notices and this
 * permission notice appear in all copies of the software and related
 * documentation, and (ii) the name of Lightscape Graphics Software may not be
 * used in any advertising or publicity relating to the software
 * without the specific, prior written permission of
 * Lightscape Graphics Software.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL LIGHTSCAPE GRAPHICS SOFTWARE BE LIABLE FOR ANY SPECIAL,
 * INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY
 * THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * pflsb.c:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#ifdef __linux__
#include <asm/byteorder.h>
#endif

#ifndef __linux__
#ifdef	_POSIX_SOURCE
extern char *strdup (const char *s1);
#endif
#endif

#ifdef WIN32
#include <windows.h>
#include <limits.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#ifdef WIN32
#include "pflsb.h"
#else
#include <Performer/pfdb/pflsb.h>
#endif

#ifndef P_32_SWAP
#define	P_32_SWAP(a) {							\
	uint _tmp = *(uint *)a;				\
	((char *)a)[0] = ((char *)&_tmp)[3];				\
	((char *)a)[1] = ((char *)&_tmp)[2];				\
	((char *)a)[2] = ((char *)&_tmp)[1];				\
	((char *)a)[3] = ((char *)&_tmp)[0];				\
}
#endif  /* P_32_SWAP */

#ifndef P_16_SWAP
#define	P_16_SWAP(a) {							\
	ushort _tmp = *(ushort *)a;				\
	((char *)a)[0] = ((char *)&_tmp)[1];				\
	((char *)a)[1] = ((char *)&_tmp)[0];				\
}
#endif  /* P_16_SWAP */


void swapLSBpatchVertex(LSBpatchVertex *data)
{
#ifdef __LITTLE_ENDIAN
	int i;
	for(i=0; i<3; i++)
		P_32_SWAP(&data->color[i]);
#endif
}

void swapLSBtopPatchVertexTextured(LSBtopPatchVertexTextured *data)
{
#ifdef __LITTLE_ENDIAN
	int i;
	for(i=0; i<3; i++)
		P_32_SWAP(&data->coord[i]);
	for(i=0; i<3; i++)
		P_32_SWAP(&data->color[i]);
	for(i=0; i<2; i++)
		P_32_SWAP(&data->uv[i]);
#endif
}

void swapLSBpatchLeaf(LSBpatchLeaf *data)
{
#ifdef __LITTLE_ENDIAN
	int i;
	for(i=0; i<4; i++)
		P_16_SWAP(&data->vertices[i]);
	for(i=0; i<4; i++)
		P_16_SWAP(&data->midVertices[i]);
#endif
}

void swapLSBpatchClusterLong(LSBpatchClusterLong *data)
{
#ifdef __LITTLE_ENDIAN
	int i;

	for(i=0; i<4; i++)
		P_32_SWAP(&data->plane[i]);

	P_32_SWAP(&data->materialId);
	P_32_SWAP(&data->layerId);
	P_32_SWAP(&data->textureId);
	P_32_SWAP(&data->numTopNodes);
	P_32_SWAP(&data->numNodes);
	P_32_SWAP(&data->numTopVertices);
	P_32_SWAP(&data->numVertices);
#endif
}

void swapLSBpatchCluster(LSBpatchCluster *data)
{
#ifdef __LITTLE_ENDIAN
	int i;

	for(i=0; i<4; i++)
		P_32_SWAP(&data->plane[i]);

	P_32_SWAP(&data->materialId);
	P_32_SWAP(&data->layerId);
	P_32_SWAP(&data->textureId);
	P_16_SWAP(&data->numTopNodes);
	P_16_SWAP(&data->numNodes);
	P_16_SWAP(&data->numTopVertices);
	P_16_SWAP(&data->numVertices);
#endif
}

void swapLSBheader(LSBheader *data)
{
#ifdef __LITTLE_ENDIAN
	int i;

	P_16_SWAP(&data->LSBversion);
	P_16_SWAP(&data->fileFlags);
	P_32_SWAP(&data->numPatchClusters);
	P_32_SWAP(&data->numTopNodes);
	P_32_SWAP(&data->numNodes);
	P_32_SWAP(&data->numTopPatchVertices);
	P_32_SWAP(&data->numPatchVertices);
	P_32_SWAP(&data->numMaterials);
	P_32_SWAP(&data->numLayers);
	P_32_SWAP(&data->numTextures);
#endif
}

void swapLSBtopPatchVertex(LSBtopPatchVertex *data)
{
#ifdef __LITTLE_ENDIAN
	int i;
	for(i=0; i<3; i++)
		P_32_SWAP(&data->coord[i]);
	for(i=0; i<3; i++)
		P_32_SWAP(&data->color[i]);
#endif
}







/* global data used by loader */
static int	 numTris		= 0;
static int	 numQuads		= 0;
static pfdGeom	*geom			= NULL;
static int	 geomSize		= 256;
static int	 clustersPerGeode	= 0;
static pfGroup	*groupNode		= NULL;
static float	 spatialSize		= 0.0f;
static int	 spatialCount		= 0;
static char	*defaultLayerName	= "default";
static int	 useLongData		= 0;

#define TRI(_a, _b, _c) {\
	geom->primtype = PFGS_POLYS;\
	geom->numVerts = 3;\
	pfCopyVec4(geom->colors[0], colors[(_a)]);\
	pfCopyVec4(geom->colors[1], colors[(_b)]);\
	pfCopyVec4(geom->colors[2], colors[(_c)]);\
	pfCopyVec3(geom->coords[0], coords[(_a)]);\
	pfCopyVec3(geom->coords[1], coords[(_b)]);\
	pfCopyVec3(geom->coords[2], coords[(_c)]);\
	if (geom->tbind[0] == PFGS_PER_VERTEX) \
	    {\
	    pfCopyVec2(geom->texCoords[0][0], texCoords[(_a)]);\
	    pfCopyVec2(geom->texCoords[0][1], texCoords[(_b)]);\
	    pfCopyVec2(geom->texCoords[0][2], texCoords[(_c)]);\
	    }\
	pfdAddBldrGeom(geom, 1);\
	numTris++;\
	}

#define QUAD(_a, _b, _c, _d) {\
	geom->primtype = PFGS_POLYS;\
	geom->numVerts = 4;\
	pfCopyVec4(geom->colors[0], colors[(_a)]);\
	pfCopyVec4(geom->colors[1], colors[(_b)]);\
	pfCopyVec4(geom->colors[2], colors[(_c)]);\
	pfCopyVec4(geom->colors[3], colors[(_d)]);\
	pfCopyVec3(geom->coords[0], coords[(_a)]);\
	pfCopyVec3(geom->coords[1], coords[(_b)]);\
	pfCopyVec3(geom->coords[2], coords[(_c)]);\
	pfCopyVec3(geom->coords[3], coords[(_d)]);\
	if (geom->tbind[0] == PFGS_PER_VERTEX) \
	    {\
	    pfCopyVec2(geom->texCoords[0][0], texCoords[(_a)]);\
	    pfCopyVec2(geom->texCoords[0][1], texCoords[(_b)]);\
	    pfCopyVec2(geom->texCoords[0][2], texCoords[(_c)]);\
	    pfCopyVec2(geom->texCoords[0][3], texCoords[(_d)]);\
	    }\
	pfdAddBldrGeom(geom, 1);\
	numQuads++;\
	}

#define	VI(_n)	(node->vertices[_n]->index)
#define CI(_n)	(((LSpatchVertex *)(node->children[_n]))->index)

static void
makeTri(
    LSpatchNode *node, 
    pfVec4 *colors, 
    pfVec3 *coords, 
    pfVec2 *texCoords)
{
    unsigned int mask, leafType;
    LSpatchNode **child;
    int t;

    geom->cbind = PFGS_PER_VERTEX;
    geom->nbind = PFGS_OFF;
    geom->tbind[0] = (texCoords == NULL) ? PFGS_OFF : PFGS_PER_VERTEX;
    for(t = 1; t < 4; t++)
        geom->tbind[t] = PFGS_OFF;

    if (node->isLeaf)
    {
	leafType = 0;
	child = node->children;
	mask   = 1; if (*child++ != NULL) leafType |= mask;
	mask <<= 1; if (*child++ != NULL) leafType |= mask;
	mask <<= 1; if (*child   != NULL) leafType |= mask;

	switch (leafType)
	{
	case 0:
	    TRI(VI(0), VI(1), VI(2));
	    break;
        case 1:
	    TRI(VI(0), CI(0), VI(2));
	    TRI(CI(0), VI(1), VI(2));
	    break;
        case 2:
	    TRI(VI(1), CI(1), VI(0));
	    TRI(CI(1), VI(2), VI(0));
	    break;
        case 3:
	    TRI(VI(0), CI(0), CI(1));
	    TRI(CI(0), VI(1), CI(1));
	    TRI(CI(1), VI(2), VI(0));
	    break;
        case 4:
	    TRI(VI(2), CI(2), VI(1));
	    TRI(CI(2), VI(0), VI(1));
	    break;
        case 5:
	    TRI(VI(2), CI(2), CI(0));
	    TRI(CI(2), VI(0), CI(0));
	    TRI(CI(0), VI(1), VI(2));
	    break;
        case 6:
	    TRI(VI(1), CI(1), CI(2));
	    TRI(CI(1), VI(2), CI(2));
	    TRI(CI(2), VI(0), VI(1));
	    break;
        case 7:
	    TRI(VI(0), CI(0), CI(2));
	    TRI(CI(0), VI(1), CI(1));
	    TRI(CI(1), VI(2), CI(2));
	    TRI(CI(0), CI(1), CI(2));
	    break;
	}
    }
    else
    {
	/* just emit this node without recursion */
	TRI(VI(0), VI(1), VI(2));
    }
}

static void
makeQuad(
    LSpatchNode *node, 
    pfVec4 *colors, 
    pfVec3 *coords, 
    pfVec2 *texCoords)
{
    unsigned int mask, leafType;
    LSpatchNode **child;

    geom->cbind = PFGS_PER_VERTEX;
    geom->nbind = PFGS_OFF;
    geom->tbind[0] = (texCoords == NULL) ? PFGS_OFF : PFGS_PER_VERTEX;

    if (node->isLeaf)
    {
	leafType = 0;
	child = node->children;
	mask   = 1; if (*child++ != NULL) leafType |= mask;
	mask <<= 1; if (*child++ != NULL) leafType |= mask;
	mask <<= 1; if (*child++ != NULL) leafType |= mask;
	mask <<= 1; if (*child   != NULL) leafType |= mask;

	switch (leafType)
	{
        case 0:
	    QUAD(VI(0), VI(1), VI(2), VI(3));
	    break;
        case 1:
	    QUAD(VI(0), CI(0), VI(2), VI(3));
	    TRI(CI(0), VI(1), VI(2));
	    break;
        case 2:
	    QUAD(VI(1), CI(1), VI(3), VI(0));
	    TRI(CI(1), VI(2), VI(3));
	    break;
        case 3:
	    TRI(VI(0), CI(0), VI(3));
	    TRI(CI(0), VI(1), CI(1));
	    QUAD(CI(0), CI(1), VI(2), VI(3));
	    break;
        case 4:
	    QUAD(VI(2), CI(2), VI(0), VI(1));
	    TRI(CI(2), VI(3), VI(0));
	    break;
        case 5:
	    QUAD(VI(0), CI(0), CI(2), VI(3));
	    QUAD(CI(0), VI(1), VI(2), CI(2));
	    break;
        case 6:
	    TRI(VI(1), CI(1), VI(0));
	    TRI(CI(1), VI(2), CI(2));
	    QUAD(CI(1), CI(2), VI(3), VI(0));
	    break;
        case 7:
	    QUAD(VI(0), CI(0), CI(2), VI(3));
	    QUAD(CI(0), CI(1), VI(2), CI(2));
	    TRI(CI(0), VI(1), CI(1));
	    break;
        case 8:
	    QUAD(VI(3), CI(3), VI(1), VI(2));
	    TRI(CI(3), VI(0), VI(1));
	    break;
        case 9:
	    TRI(VI(3), CI(3), VI(2));
	    TRI(CI(3), VI(0), CI(0));
	    QUAD(CI(3), CI(0), VI(1), VI(2));
	    break;
        case 10:
	    QUAD(VI(1), CI(1), CI(3), VI(0));
	    QUAD(CI(1), VI(2), VI(3), CI(3));
	    break;
        case 11:
	    QUAD(VI(3), CI(3), CI(1), VI(2));
	    QUAD(CI(3), CI(0), VI(1), CI(1));
	    TRI(CI(3), VI(0), CI(0));
	    break;
        case 12:
	    TRI(VI(2), CI(2), VI(1));
	    TRI(CI(2), VI(3), CI(3));
	    QUAD(CI(2), CI(3), VI(0), VI(1));
	    break;
        case 13:
	    QUAD(VI(2), CI(2), CI(0), VI(1));
	    QUAD(CI(2), CI(3), VI(0), CI(0));
	    TRI(CI(2), VI(3), CI(3));
	    break;
        case 14:
	    QUAD(VI(1), CI(1), CI(3), VI(0));
	    QUAD(CI(1), CI(2), VI(3), CI(3));
	    TRI(CI(1), VI(2), CI(2));
	    break;
        case 15:
	    QUAD(VI(0), CI(0), CI(2), CI(3));
	    TRI(CI(2), VI(3), CI(3));
	    QUAD(VI(1), CI(1), CI(2), CI(0));
	    TRI(CI(1), VI(2), CI(2));
	    break;
	}
    }
    else
    {
	/* just emit this node without recursion */
	TRI(VI(0), VI(1), VI(3));
	TRI(VI(1), VI(2), VI(3));
    }
}

static void
buildPolygons(
    int recursionDepth,
    int recursionLimit,
    LSpatchNode *node,
    pfVec4 *colors,
    pfVec3 *coords,
    pfVec2 *texCoords)
{
    if (node->isLeaf || recursionDepth >= recursionLimit)
    {
	if (NODE_IS_TRIANGLE(node))
	    makeTri( node, colors, coords, texCoords);
	else
	    makeQuad(node, colors, coords, texCoords);
    }
    else
    {
	int i;
	for (i = 0; i < 4; i++)
	    buildPolygons(recursionDepth+1, recursionLimit,
		node->children[i], colors, coords, texCoords);
    }
}

static int
loadPatchVertex(
    FILE *fp, 
    pfVec4 *color)
{
    LSBpatchVertex lsbPatchVertex;

    if (fread((void *)(&lsbPatchVertex), 
	sizeof(LSBpatchVertex), 1, fp) <= 0)
	return 0;

	swapLSBpatchVertex(&lsbPatchVertex);
    pfCopyVec3(*color, lsbPatchVertex.color);
    (*color)[3] = 1.0f;

    return 1;
}

static int
loadTopPatchVertex(
    FILE *fp, 
    pfVec4 *color, 
    pfVec3 *coord, 
    pfVec2 *texCoord)
{
    if (texCoord != NULL)
    {
	LSBtopPatchVertexTextured lsbPatchVertexTextured;
	if (fread((void *)(&lsbPatchVertexTextured),
	    sizeof(LSBtopPatchVertexTextured), 1, fp) <= 0)
	    return 0;

	swapLSBtopPatchVertexTextured(&lsbPatchVertexTextured);
	pfCopyVec3(*coord, lsbPatchVertexTextured.coord);
	pfCopyVec3(*color, lsbPatchVertexTextured.color);
	(*color)[3] = 1.0f;
	pfCopyVec2(*texCoord, lsbPatchVertexTextured.uv);
    }
    else
    {
	LSBtopPatchVertex lsbPatchVertex;
	if (fread((void *)(&lsbPatchVertex), 
	    sizeof(LSBtopPatchVertex), 1, fp) <= 0)
	    return 0;

	swapLSBtopPatchVertex(&lsbPatchVertex);
	pfCopyVec3(*coord, lsbPatchVertex.coord);
	pfCopyVec3(*color, lsbPatchVertex.color);
	(*color)[3] = 1.0f;
    }

    return 1;
}

static int
readPatchLeaf(
    FILE *fp, 
    unsigned int *verts, 
    unsigned int *midVerts)
{
    LSBpatchLeaf lsbPatchLeaf;
    LSBpatchLeaf lsbPatchLeafLong;

    if (useLongData)
    {
	if (fread((void *)(&lsbPatchLeaf), 
	    sizeof(LSBpatchLeaf), 1, fp) <= 0)
		return 0;

	swapLSBpatchLeaf(&lsbPatchLeaf);
	verts[0] = lsbPatchLeafLong.vertices[0];
	verts[1] = lsbPatchLeafLong.vertices[1];
	verts[2] = lsbPatchLeafLong.vertices[2];
	verts[3] = lsbPatchLeafLong.vertices[3];
	midVerts[0] = lsbPatchLeafLong.midVertices[0];
	midVerts[1] = lsbPatchLeafLong.midVertices[1];
	midVerts[2] = lsbPatchLeafLong.midVertices[2];
	midVerts[3] = lsbPatchLeafLong.midVertices[3];
    }
    else
    {
	if (fread((void *)(&lsbPatchLeaf), 
	    sizeof(LSBpatchLeaf), 1, fp) <= 0)
		return 0;

	swapLSBpatchLeaf(&lsbPatchLeaf);
	verts[0] = lsbPatchLeaf.vertices[0];
	verts[1] = lsbPatchLeaf.vertices[1];
	verts[2] = lsbPatchLeaf.vertices[2];
	verts[3] = lsbPatchLeaf.vertices[3];
	midVerts[0] = lsbPatchLeaf.midVertices[0];
	midVerts[1] = lsbPatchLeaf.midVertices[1];
	midVerts[2] = lsbPatchLeaf.midVertices[2];
	midVerts[3] = lsbPatchLeaf.midVertices[3];
    }

    return 1;
}

static LSpatchNode *
loadPatchNodes(
    FILE *fp, 
    LSpatchNode *root, 
    LSpatchNode **nodes, 
    LSpatchVertex *vertices)
{
    int i, n;
    LSpatchNode *node;
    unsigned int verts[4], midVerts[4];

    switch (getc(fp))
    {
    case 'i':
	if (root != NULL)
	    node = root;
	else
	{
	    node = *nodes;
	    (*nodes)++;
	}

	for (i = 0; i < 4; i++)
	{
	    node->children[i] = loadPatchNodes(fp, NULL, nodes, vertices);
	    if (node->children[i] == NULL)
	    {
		node = NULL;
		break;
	    }
	}

	if (node != NULL)
	{
	    node->isLeaf = 0;
	    if (NODE_IS_TRIANGLE(node->children[0]))
	    {
		for (i = 0; i < 3; i++)
		    node->vertices[i] = node->children[i]->vertices[i];
		node->vertices[3] = NULL;
	    }
	    else
		for (i = 0; i < 4; i++)
		    node->vertices[i] = node->children[i]->vertices[i];
	}
	break;

    case 'l' :
	if (!readPatchLeaf(fp, verts, midVerts))
	    return NULL;

	if (root != NULL)
	    node = root;
	else
	{
	    node = *nodes;
	    (*nodes)++;
	}

	node->isLeaf = 1;
	node->children[3] = NULL;
	node->vertices[3] = NULL;
	n = ((verts[3] == 0) ? 3 : 4);
	for (i = 0; i < n; i++)
	{
	    node->vertices[i] = &(vertices[verts[i] - 1]);
	    node->children[i] = (midVerts[i] == 0)
		? NULL
		: (LSpatchNode *)(&(vertices[midVerts[i] - 1]));
	}
	break;

    default :
	node = NULL;
	break;
    }

    return node;
}

static int
readPatchCluster(FILE *fp,
    unsigned short *flags,
    pfVec4 *plane,
    unsigned int *materialId,
    unsigned int *layerId,
    unsigned int *textureId,
    unsigned int *numTopNodes,
    unsigned int *numNodes,
    unsigned int *numTopVerts,
    unsigned int *numVertices)
{
    unsigned short clustFlags;
    unsigned short junk;

    if (fread((void *)(&clustFlags), sizeof(unsigned short), 1, fp) <= 0)
	return(0);
    if (fread((void *)(&junk),       sizeof(unsigned short), 1, fp) <= 0)
	return(0);

#ifdef __LITTLE_ENDIAN
	P_16_SWAP(&clustFlags);
	P_16_SWAP(&junk);
#endif

    if (clustFlags & LSB_BIG_CLUSTER) 
    {
	LSBpatchClusterLong lsbLongCluster;
	useLongData = 1;

	if(fread((void *)(&lsbLongCluster.plane),
	    (long)(sizeof(LSBpatchClusterLong))-4, 1, fp) <= 0)
	    return 0;

	swapLSBpatchClusterLong(&lsbLongCluster);
	*flags = clustFlags;
	pfSetVec4(*plane, 
	    lsbLongCluster.plane[0], 
	    lsbLongCluster.plane[1],
	    lsbLongCluster.plane[2], 
	    lsbLongCluster.plane[3]);
	*materialId  = lsbLongCluster.materialId;
	*layerId     = lsbLongCluster.layerId;
	*textureId   = lsbLongCluster.textureId;
	*numTopNodes = lsbLongCluster.numTopNodes;
	*numNodes    = lsbLongCluster.numNodes;
	*numTopVerts = lsbLongCluster.numTopVertices;
	*numVertices = lsbLongCluster.numVertices;
    } 
    else 
    {
        LSBpatchCluster lsbPatchCluster;
	useLongData = 0;

	if (fread((void *)(&lsbPatchCluster.plane), 
	    (long)(sizeof(LSBpatchCluster))-4, 1, fp) <= 0)
	    return(0);

	swapLSBpatchCluster(&lsbPatchCluster);
	*flags = clustFlags;
	pfSetVec4(*plane, 
            lsbPatchCluster.plane[0], 
	    lsbPatchCluster.plane[1],
            lsbPatchCluster.plane[2], 
	    lsbPatchCluster.plane[3]);
	*materialId  = lsbPatchCluster.materialId;
	*layerId     = lsbPatchCluster.layerId;
	*textureId   = lsbPatchCluster.textureId;
	*numTopNodes = lsbPatchCluster.numTopNodes;
	*numNodes    = lsbPatchCluster.numNodes;
	*numTopVerts = lsbPatchCluster.numTopVertices;
	*numVertices = lsbPatchCluster.numVertices;
    }

    return 1;
}

static void
initPatchNodes(
    LSpatchNode *node, 
    pfVec3 *coords, 
    pfVec2 *texCoords)
{
    unsigned int i, j, n;
    LSpatchVertex *v;

    if (!node->isLeaf)
    {
	n = NUM_NODE_VERTICES(node);
	for (i = 0; i < n; i++)
	{
	    j = (i + 1) % n;
	    v = node->children[i]->vertices[j];
	    if (v->flags)
	    {
		v->flags = 0;

		pfCombineVec3(coords[v->index],
		    0.5, coords[node->vertices[i]->index],
		    0.5, coords[node->vertices[j]->index]);

		if (texCoords != NULL)
		    pfCombineVec2(texCoords[v->index],
			0.5, texCoords[node->vertices[i]->index],
			0.5, texCoords[node->vertices[j]->index]);
	    }
	}

	if (NODE_IS_RECT(node))
	{
	    v = node->children[2]->vertices[0];
	    v->flags = 0;
	    pfCombineVec3(coords[v->index],
		0.5, coords[node->children[0]->vertices[1]->index],
		0.5, coords[node->children[2]->vertices[3]->index]);
	    if (texCoords != NULL)
	    {
		pfCombineVec2(texCoords[v->index],
		    0.5, texCoords[node->children[0]->vertices[1]->index],
		    0.5, texCoords[node->children[2]->vertices[3]->index]);
	    }
	}

	for (i = 0; i < 4; i++)
	    initPatchNodes(node->children[i], coords, texCoords);
    }
}

static int
readClusters(
    FILE *fp,
    LSBheader *lsbHeader,
    LSmaterial *materials,
    LSlayer *layers,
    LStexture *textures,
    int recursionLimit
    )
{
    unsigned short flags;
    unsigned int i, j, k;
    unsigned int materialId, layerId, textureId;
    unsigned int numOtherVertices, numTopVerts, numVerts, numTopNodes, numNodes;
    pfVec4 plane, *curColor;
    pfVec3 *curCoord;
    pfVec2 *curTexCoord;
    LSpatchVertex *curVertex;
    LSpatchNode *curTopNode, *curNode;
    LSpatchCluster thisPatchCluster;
    LSpatchCluster *curCluster = &thisPatchCluster;

    int		 nodeCount	= 256;
    int		 vertexCount	= 256;
    LSpatchNode	*nodes		= pfCalloc(nodeCount,   sizeof(LSpatchNode),   NULL);
    LSpatchVertex	
		*vertices	= pfCalloc(vertexCount, sizeof(LSpatchVertex), NULL);
    pfVec4	*pfColors	= pfCalloc(vertexCount, sizeof(pfVec4),        NULL);
    pfVec3	*pfCoords	= pfCalloc(vertexCount, sizeof(pfVec3),        NULL);
    pfVec2	*pfTexCoords	= pfCalloc(vertexCount, sizeof(pfVec2),        NULL);

    for (i = 0; i < lsbHeader->numPatchClusters; i++)
    {
	if (!readPatchCluster(fp, &flags, &plane, &materialId, &layerId,
	    &textureId, &numTopNodes, &numNodes, &numTopVerts, &numVerts))
	{
	    if (nodes       != NULL) pfFree(nodes);
	    if (vertices    != NULL) pfFree(vertices);
	    if (pfColors    != NULL) pfFree(pfColors);
	    if (pfCoords    != NULL) pfFree(pfCoords);
	    if (pfTexCoords != NULL) pfFree(pfTexCoords);
	    return 0;
	}

	curCluster->flags = flags;
	pfCopyVec4(curCluster->plane, plane);

	if (materialId == 0)
	    curCluster->material = NULL;
	else
	    curCluster->material = &(materials[materialId - 1]);

	/* specify layer for this cluster */
#ifdef	BUILD_LAYERS_INDEPENDENTLY
	if (layerId == 0)
	{
	    curCluster->layer = NULL;
	    pfdSelectBldrName(defaultLayerName);
	}
	else
	{
	    curCluster->layer = &(layers[layerId - 1]);
	    pfdSelectBldrName(curCluster->layer->name);
	}
#endif

	/* specify texture for this cluster */
	if (textureId == 0)
	{
	    curCluster->texture = NULL;
	    pfdBldrStateAttr(PFSTATE_TEXTURE, NULL);
	    pfdBldrStateMode(PFSTATE_ENTEXTURE, PFTR_OFF);
	}
	else
	{
	    pfTexture *tex = (pfTexture *)
		pfdGetTemplateObject(pfGetTexClassType());
	    curCluster->texture = &(textures[textureId - 1]);
	    pfTexName(tex, curCluster->texture->name);
	    pfdBldrStateAttr(PFSTATE_TEXTURE, tex);
	    pfdBldrStateMode(PFSTATE_ENTEXTURE, PFTR_ON);
	}

	/* update vertex-count array sizes and pointers */
	if (vertexCount < numVerts) 
	{
	    vertexCount = numVerts;
	    pfColors = pfRealloc(pfColors, vertexCount*sizeof(pfVec4));
	    if (pfColors == NULL)
		return 0;
	    pfCoords = pfRealloc(pfCoords, vertexCount*sizeof(pfVec3));
	    if (pfCoords == NULL)
		return 0;
	    pfTexCoords = pfRealloc(pfTexCoords, vertexCount*sizeof(pfVec2));
	    if (pfTexCoords == NULL) 
		return 0;
	    vertices = pfRealloc(vertices, numVerts*sizeof(LSpatchVertex));
	    if (vertices == NULL) 
		return 0;
	}
	curColor    = pfColors;
	curCoord    = pfCoords;
	curVertex   = vertices;

	curTexCoord = (curCluster->flags & LSB_TEXTURED) ? pfTexCoords : NULL;

	/* update node-count array sizes and pointers */
	if (nodeCount < numNodes) 
	{
	    nodeCount = numNodes;
	    nodes = pfRealloc(nodes, numNodes*sizeof(LSpatchNode));
	    if (nodes == NULL) 
		return 0;
	}
	curTopNode = nodes;

	numOtherVertices = numVerts - numTopVerts;
	for (j = 0; j < numOtherVertices; j++, curColor++, curCoord++, curVertex++)
	{
	    if (!loadPatchVertex(fp, curColor))
		return 0;
	    curVertex->flags = 1; /* means that geometry needs to be initialized */
	    curVertex->index = j;
	    if (curCluster->flags & LSB_TEXTURED)
		curTexCoord++;
	}
	for (j = 0; j < numTopVerts; j++, curColor++, curCoord++, curVertex++)
	{
	    if (!loadTopPatchVertex(fp, curColor, curCoord, curTexCoord))
		return 0;
	    curVertex->flags = 0;
	    curVertex->index = numOtherVertices + j;
	    if (curCluster->flags & LSB_TEXTURED)
		curTexCoord++;
	}

	/* temporary node info */
	curNode = nodes + numTopNodes;
	for (j = 0; j < numTopNodes; j++, curTopNode++)
	    if (!loadPatchNodes(fp, curTopNode, &curNode, vertices))
		return 0;

	/* initialize geometry */
	for (j = 0, curTopNode = nodes; j < numTopNodes; j++, curTopNode++)
	    initPatchNodes(curTopNode, pfCoords, pfTexCoords);

	/* create geometry */
	if (curCluster->flags & LSB_TEXTURED)
	    for (k = 0, curTopNode = nodes; k < numTopNodes; k++, curTopNode++)
		buildPolygons(0, recursionLimit, curTopNode, pfColors, pfCoords, pfTexCoords);
	else
	    for (k = 0, curTopNode = nodes; k < numTopNodes; k++, curTopNode++)
		buildPolygons(0, recursionLimit, curTopNode, pfColors, pfCoords, NULL);

	/* go ahead and build this cluster's geometry */
	if ((clustersPerGeode > 0) &&
	    ( ((clustersPerGeode == 1) || ((i > 0) && (i % clustersPerGeode) == 0)) || 
	      (i == lsbHeader->numPatchClusters - 1)) )
	    pfAddChild(groupNode, pfdBuild());
    }

    if (nodes       != NULL) pfFree(nodes);
    if (vertices    != NULL) pfFree(vertices);
    if (pfColors    != NULL) pfFree(pfColors);
    if (pfCoords    != NULL) pfFree(pfCoords);
    if (pfTexCoords != NULL) pfFree(pfTexCoords);

    return 1;
}

static int
readSignature(FILE *fp)
{
    char line[256];
    static char *signature = LSB_SIGNATURE;

    /* read LSB signature */
    if (fgets(line, 256, fp) == NULL)
	return 0;
    if (strncmp(signature, line, strlen(signature)) != 0)
	return 0;

    /* scan past signature data */
    do
    {
	if (fgets(line, 256, fp) == NULL)
	    return 0;
    }
    while (line[0] != LSB_BEGIN_CHAR);

    /* indicate success */
    return 1;
}

static int
readHeader(FILE *fp, LSBheader *header)
{
    if (fread((void *)header, sizeof(LSBheader), 1, fp) <= 0)
	return 0;

	swapLSBheader(header);
    /* indicate success */
    return 1;
}

static int
readMaterials(FILE *fp, void *arena, unsigned int nmaterials,
    LSmaterial **materials)
{
    unsigned int i;
    char line[256];

    /* check input arguments */
    if (nmaterials < 1 || materials == NULL)
	return 1;

    /* allocate an array of LSmaterial structures */
    *materials = (LSmaterial *)
	pfCalloc(nmaterials, sizeof(LSmaterial), arena);
    if (*materials == NULL)
	return 0;

    /* read material information */
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Materials");
    for (i = 0; i < nmaterials; i++)
    {
	if (fgets(line, 256, fp) == NULL)
	      return 0;

	line[strlen(line) - 1] = '\0';
	(*materials)[i].name = strdup(line);

	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, " %2d: %s", i, line);
    }

    /* indicate success */
    return 1;
}

static int
deallocateMaterials(unsigned int nmaterials, LSmaterial **materials)
{
    unsigned int i;

    /* no materials means nothing to deallocate */
    if (nmaterials < 1 || materials == NULL || *materials == NULL)
	return 1;

    /* delete each material's data */
    for (i = 0; i < nmaterials; i++)
	if ((*materials)[i].name != NULL)
	    free((*materials)[i].name);

    /* reset material array pointer */
    pfFree(*materials);
    *materials = NULL;

    /* indicate success */
    return 1;
}

static int
readLayers(FILE *fp, void *arena, unsigned int nlayers, LSlayer **layers)
{
    unsigned int i;
    char line[256];

    /* check input arguments */
    if (nlayers < 1 || layers == NULL)
	return 1;

    /* allocate an array of LSlayer structures */
    *layers = (LSlayer *)pfCalloc(nlayers, sizeof(LSlayer), arena);
    if (*layers == NULL)
	return 0;

    /* read layer information */
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Layers");
    for (i = 0; i < nlayers; i++)
    {
	if (fgets(line, 256, fp) == NULL)
	    return 0;

	line[strlen(line) - 1] = '\0';
	(*layers)[i].name = strdup(line);

	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, " %2d: %s", i, line);
    }

    /* indicate success */
    return 1;
}

static int
deallocateLayers(unsigned int nlayers, LSlayer **layers)
{
    unsigned int i;

    /* no layers means nothing to deallocate */
    if (layers == NULL || *layers == NULL)
	return 1;

    /* delete each layer's data */
    for (i = 0; i < nlayers; i++)
	if ((*layers)[i].name != NULL)
	    free((*layers)[i].name);

    /* reset layer array pointer */
    pfFree(*layers);
    *layers = NULL;

    /* indicate success */
    return 1;
}

static int
readTextures(FILE *fp, void *arena, unsigned int ntextures,
    LStexture **textures)
{
    unsigned int i;
    char line[256];

    /* check input arguments */
    if (ntextures < 1 || textures == NULL)
	return 1;

    /* allocate an array of LStexture structures */
    *textures = (LStexture *)pfCalloc(ntextures, sizeof(LStexture), arena);
    if (*textures == NULL)
	return 0;

    /* read texture information */
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Textures");
    for (i = 0; i < ntextures; i++)
    {
	if (fgets(line, 256, fp) == NULL)
	    return 0;

	line[strlen(line) - 1] = '\0';
	(*textures)[i].name = strdup(line);

	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, " %2d: %s", i, line);
    }

    /* indicate success */
    return 1;
}

static int
deallocateTextures(unsigned int ntextures, LStexture **textures)
{
    unsigned int i;

    /* no textures means nothing to deallocate */
    if (textures == NULL || *textures == NULL)
	return 1;

    /* delete each textures's data */
    for (i = 0; i < ntextures; i++)
	if ((*textures)[i].name != NULL)
	    free((*textures)[i].name);

    /* reset texture array pointer */
    pfFree(*textures);
    *textures = NULL;

    /* indicate success */
    return 1;
}

/*
 * pfdLoadFile_lsb -- Load Lightscape ".lsb" files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_lsb (char *fileName)
{
    FILE		*lsbFile	= NULL;
    pfNode		*node		= NULL;
    LSmaterial		*materials	= NULL;
    LSlayer		*layers		= NULL;
    LStexture		*textures	= NULL;
    LSpatchCluster	*clusters	= NULL;
    LSBheader 		 lsbHeader;
    int			 recursionLimit	= 0;
    char		*ep		= NULL;

    double startTime	= pfGetTime();
    double elapsedTime	= 0.0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open ".lsb" file */
    if ((lsbFile = pfdOpenFile(fileName)) == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "pfdLoadFile_lsb: error opening file \"%s\"", fileName);
	return NULL;
    }

    /* read ".lsb" file "signature" section */
    if (!readSignature(lsbFile))
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "pfdLoadFile_lsb: error reading signature from \"%s\"", fileName);
	fclose(lsbFile);
	return NULL;
    }

    /* read ".lsb" file "header" section */
    if (!readHeader(lsbFile, &lsbHeader))
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "pfdLoadFile_lsb: error reading header from \"%s\"", fileName);
	fclose(lsbFile);
	return NULL;
    }

    /* update recursion limit from environment variable */
    if ((ep = getenv("LS_RECURSION_LIMIT")) != NULL)
	sscanf(ep, "%d", &recursionLimit);

    /* update clusters-per-geode from environment variable */
    if ((ep = getenv("LS_CLUSTERS_PER_GEODE")) != NULL)
	sscanf(ep, "%d", &clustersPerGeode);

    /* update spatial size from environment variable */
    if ((ep = getenv("LS_SPATIAL_SIZE")) != NULL)
	sscanf(ep, "%f", &spatialSize);

    /* update spatial count from environment variable */
    if ((ep = getenv("LS_SPATIAL_COUNT")) != NULL)
	sscanf(ep, "%d", &spatialCount);

    /* update file search path from environment variable */
    if ((ep = getenv("LS_TEXTURE_PATH")) != NULL)
    {
	const char *op = pfGetFilePath();
	char *np = NULL;
	if (op == NULL)
	    op = "";
	np = (char *)pfMalloc(strlen(op) + 1 + strlen(ep) + 1, NULL);
	strcpy(np, ep);
	strcat(np, ":");
	strcat(np, op);
	pfFilePath(np);
	pfFree(np);
    }

    /* disable lighting (that's what radiosity is all about ;-) */
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_OFF);
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);

    /* disable undesired automatic builder actions */
    pfdBldrMode(PFDBLDR_AUTO_NORMALS, PF_OFF);
    pfdBldrMode(PFDBLDR_AUTO_ORIENT,  PF_OFF);

    /* parent group for "build-clumps-of-clusters" mode */
    if (clustersPerGeode > 0)
	groupNode = pfNewGroup();

    /* reset global geometry counters */
    numTris  = 0;
    numQuads = 0;
    
    /* allocate geometry buffer structure */
    geom = pfdNewGeom(geomSize = 256);

    /* read ".lsb" file materials, layers, textures, and clusters */
    if (readMaterials(lsbFile, NULL, lsbHeader.numMaterials, &materials) &&
	readLayers   (lsbFile, NULL, lsbHeader.numLayers,    &layers)    &&
	readTextures (lsbFile, NULL, lsbHeader.numTextures,  &textures)  &&
	readClusters (lsbFile, &lsbHeader, materials, layers, textures, recursionLimit))
    {
	/* build scene graph of file's primitives unless it's already built */
	if (clustersPerGeode > 0)
	    node = (pfNode *)groupNode;
	else
	    node = pfdBuild();

	/* construct spatial octree hierarchy from geosets in scene graph */
	if (spatialSize != 0.0f || spatialCount != 0)
	{
	    pfGroup *group = pfNewGroup();
	    pfAddChild(group, node);
	    node = (pfNode *)pfdSpatialize(group, spatialSize, spatialCount);
	}

	/* use file name name for top-level pfNode */
	if (node != NULL)
	    pfNodeName(node, fileName);

	/* print statistics */
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_lsb: %s", fileName);
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Configuration Data:");
	if (ep != NULL)
	    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Texture path:       %s", ep);
	if (recursionLimit > 0)
	    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Recursion limit:    %8ld", 
		recursionLimit);
	if (clustersPerGeode > 0)
	    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Clusters per geode: %8ld", 
		clustersPerGeode);
	if (lsbHeader.numMaterials != 0)
	    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input materials:    %8ld", 
		lsbHeader.numMaterials);
	if (lsbHeader.numLayers != 0)
	    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input layers:       %8ld", 
		lsbHeader.numLayers);
	if (lsbHeader.numTextures != 0)
	    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input textures:     %8ld", 
		lsbHeader.numTextures);
	if (lsbHeader.numPatchClusters != 0)
	    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input clusters:     %8ld", 
		lsbHeader.numPatchClusters);
	if (numTris != 0 || numQuads != 0)
	    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
	if (numTris != 0)
	    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input triangles:    %8ld", 
		numTris);
	if (numQuads != 0)
	    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input quads:        %8ld", 
	    numQuads);
    }

    /* close input file */
    fclose(lsbFile);

    /* release allocated storage */
    deallocateMaterials(lsbHeader.numMaterials,      &materials);
    deallocateLayers   (lsbHeader.numLayers,         &layers);
    deallocateTextures (lsbHeader.numTextures,       &textures);

    /* release storage allocated for geometric primitives */
    pfdDelGeom(geom);

    /* release storage allocated by the builder */
    pfdResetBldrGeometry();

    /* return root node to caller */
    return node;
}
