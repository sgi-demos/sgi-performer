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
 * pfdTMesher.c
 *
 * $Revision: 1.94 $
 * $Date: 2002/10/09 22:41:50 $
 *
 * This code is adapted from the triangle mesh code supplied in the
 * /usr/people/4Dgifts directory. It has been modified to work with 
 * IRIS Performer pfGeoSet data structures and optimized for better 
 * performance.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef WIN32
#include <alloca.h>
#endif
#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#endif

#ifdef  _POSIX_SOURCE
#ifdef mips
extern long random(void);
#endif
#endif
#ifdef WIN32
#define random rand
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

#define SORTOF 1.0e-4f

/*#define SHOWADJ*/

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

static int 	TriHashSize;
static int 	EdgeHashSize;

typedef ushort  VertIndex;

typedef struct Edge
{
    struct Edge *hnext;
    struct Tri  *tri;
    VertIndex   index0, index1; /* Index into coordinate array */
    ushort	edgeId;	    	/* Id of triangle edge: 0-2 */
    ushort	pad;    
    struct Tri  *adjTri;
}               Edge;

typedef struct Tri
{
    struct Tri  *next, *prev, *hnext;
#ifdef DECIMATE
    Edge	*edges[3], *adjEdges[3];
#endif
    struct Tri  *(adj[3]);
    VertIndex   vert[3];
    VertIndex	flags;
    pfVec3	norm;
    int         adjCount;
    int         used;
    int	    	id;
}               Tri;

typedef struct TriList
{
    Tri            *next, *prev;	
}               TriList;

static TriList *AdjTriList[4];

#define	DEFAULT_NUM_TRIS	101	/* Should be prime */

static Tri    	**TriHashList = NULL;
static Tri    	*DefaultTriHashList[DEFAULT_NUM_TRIS];

static Edge   	**EdgeHashList = NULL;
static Edge   	*DefaultEdgeHashList[DEFAULT_NUM_TRIS];

static Edge    	*EdgeArray = NULL;
static Edge    	DefaultEdgeArray[DEFAULT_NUM_TRIS*3];
static Edge    	*FreeEdge = NULL;

static Tri     	*Tris = NULL;
static Tri      DefaultTris[DEFAULT_NUM_TRIS];
static Tri	*FreeTri = NULL;

static void	hashTriBegin(int ntris);
static int	hashTriAdd(Tri * tri);
static void	hashTriEnd(void);
static void	hashEdgeBegin(int ntris);
static void	hashEdgeAdd(Tri * tri, ushort e, VertIndex v0, VertIndex v1);
static void	hashEdgeEnd(void);

static TriList	*makeTriList(void);
static Tri	*initTri(void);
static void	insertTri(TriList * list, Tri * item);
static void	removeTri(TriList * list, Tri * item);
static int	triEqual(Tri * tri1, Tri * tri2);

static void	freeTris(void);
static void	makeTris(int ntris);

static void	eliminatePrims(TriList *triList, int ntris);
static void	computeTriAdjacencies(TriList *triList, int ntris);
static void	computeLineAdjacencies(TriList *triList, int ntris);
static void	computeNormal(Tri *tri);
static void	adjustAdjacencies(Tri * tri, int vertsPerPrim);

static void	initStrip(void);
static Tri*	chooseSeed(void);
static int	concaveCheck(pfVec3 norm, ushort v0, ushort v1, ushort v2, ushort v3);
static void	retessellate(Tri *tri, int ncom, int numTextures);
static Tri	*getNextTri(Tri * tri, int numTextures);
static Tri	*getNextLine(Tri * tri);
static int	 coplanar(Tri *tri, Tri *adjtri, int mode, int t);

/* Shorthand for mod arithmetic */	    
static int	NextVertIndex[3] ={1, 2, 0};
static int	NextEdgeIndex[3] ={2, 0, 1};

#define NORM_OK	    0x1


	/*------------------------------------------------------*/

static void
freeTris(void)
{
    if (Tris != DefaultTris)
	pfFree(Tris);
}


	/*------------------------------------------------------*/

/*
 * Triangle hashing 
*/

static void
hashTriBegin(int ntris)
{
    int            i;

    if (ntris > DEFAULT_NUM_TRIS)
    {
    	TriHashSize = pfuCalcHashSize(ntris);
    	TriHashList = (Tri **) pfMalloc(TriHashSize * sizeof(Tri *), NULL);
    }
    else
    {
    	TriHashSize = DEFAULT_NUM_TRIS;
    	TriHashList = DefaultTriHashList;
    }
    for (i=0; i<TriHashSize; i++)
	TriHashList[i] = NULL;
}

#define hashTri(tri)						\
  	((ushort) ((tri)->vert[0] ^ (tri)->vert[1] ^ (tri)->vert[2]) % TriHashSize)

static int
hashTriAdd(Tri * tri)
{
    int            pos;
    Tri            *tptr;

    for (tptr = TriHashList[pos = hashTri(tri)]; tptr; tptr = tptr->hnext)
	if (triEqual(tri, tptr) == 1)
	    return 1;
    tri->hnext = TriHashList[pos];
    TriHashList[pos] = tri;
    return 0;
}

static void
hashTriEnd(void)
{
    if (TriHashList != DefaultTriHashList)
    	pfFree(TriHashList);
}

	/*------------------------------------------------------*/

/*
 * Edge hashing 
 */

static void
hashEdgeBegin(int ntris)
{
    int            i;

    if (ntris > DEFAULT_NUM_TRIS)
    {
    	EdgeHashSize = pfuCalcHashSize(ntris);
    	EdgeHashList = (Edge **) pfMalloc(EdgeHashSize * sizeof(Edge *), NULL);
    	EdgeArray = (Edge *) pfMalloc(3 * ntris * sizeof(Edge), NULL);
    }
    else
    {
    	EdgeHashSize = DEFAULT_NUM_TRIS;
    	EdgeHashList = DefaultEdgeHashList;
    	EdgeArray = DefaultEdgeArray;
    }
    for (i=0; i<EdgeHashSize; i++)
	EdgeHashList[i] = NULL;

    FreeEdge = EdgeArray;
}

/* 
 * hashEdge must be independent of the ordering of index0 and index1
 * so that (index0, index1) and (index1, index0) hash to the same
 * location.
*/
#define hashEdge(index0, index1) 	\
		((uint)((uint)(index0) * (uint)(index1)) % EdgeHashSize)

#define hashEdgeFind(v0, v1)		\
		EdgeHashList[hashEdge((v0), (v1))]

static void
hashLineAdd(ushort lineId, VertIndex i0, VertIndex i1)
{
    int            pos;
    Edge           *edge;

    pos = hashEdge(i0, i1);
    edge = FreeEdge++;
    edge->index0 = i0;
    edge->index1 = i1;
    edge->edgeId = lineId;
    edge->hnext = EdgeHashList[pos];
    EdgeHashList[pos] = edge;
}

static void
hashEdgeAdd(Tri * tri, ushort edgeId, VertIndex i0, VertIndex i1)
{
    int            pos;
    Edge           *edge;

    pos = hashEdge(i0, i1);
    edge = FreeEdge++;
    edge->index0 = i0;
    edge->index1 = i1;
    edge->edgeId = edgeId;
    edge->tri = tri;
    edge->adjTri = NULL;
    edge->hnext = EdgeHashList[pos];
    EdgeHashList[pos] = edge;

#ifdef DECIMATE
    tri->edges[edgeId] = edge;
#endif
}

static void
hashEdgeEnd(void)
{
    if (EdgeHashList != DefaultEdgeHashList)
    	pfFree(EdgeHashList);

    if (EdgeArray != DefaultEdgeArray)
    	pfFree(EdgeArray);
}

	/*------------------------------------------------------*/

static TriList *
makeTriList(void)
{
    /* allocate space for and initialize a tri list */
    TriList        *tmp;

    if ((tmp = (TriList *) (pfMalloc(sizeof(TriList), NULL))) == 0)
    {
	pfNotify(PFNFY_FATAL, PFNFY_RESOURCE, "makeTriList: out of memory");
	pfExit();
	exit(1);
    }
    tmp->next = tmp->prev = (Tri *) tmp;
    return tmp;
}

static void
freeTriList(TriList *tlist)
{
    pfFree(tlist);
}

static void
makeTris(int ntris)
{
    if (ntris > DEFAULT_NUM_TRIS)
	Tris = (Tri *) pfMalloc(ntris * sizeof(Tri), NULL);
    else 
	Tris = DefaultTris;

    FreeTri = Tris;
}

static Tri*
initTri(void)
{
    /* Initialize a tri */
    Tri            *tmp;

    tmp = FreeTri++;

#ifdef CAUTIOUS
    tmp->prev = tmp->next = 0;
    tmp->vert[0] = tmp->vert[1] = tmp->vert[2] = 0;
#endif

    tmp->adj[0] = tmp->adj[1] = tmp->adj[2] = 0;
#ifdef DECIMATE
    tmp->adjEdges[0] = tmp->adjEdges[1] = tmp->adjEdges[2] = 0;
#endif
    tmp->adjCount = 0;
    tmp->used = FALSE;
    tmp->flags = 0;

    return tmp;
}

static void
insertTri(TriList * list, Tri * item)
{
    /* insert the new item at the head of the list */
    item->prev =(Tri *) list;
    item->next = list->next;
    list->next = item;
    (item->next)->prev = item;
}

static void
removeTri(TriList * list, Tri * item)
{
    (list, list);
    /* delete the item from the list */
    item->prev->next = item->next;
    item->next->prev = item->prev;
#ifdef CAUTIOUS
    item->prev = item->next = 0;
#endif
}

/* 
 * Return:
 *  0 if tris are not equal
 *  1 if tris are equal and have the same orientation 
 *  2 if tris are equal and have the opposite orientation 
*/
static int
triEqual(Tri * tri1, Tri * tri2)
{
    int            i;

    for (i=0; i<3; i++)
    {
	if (tri1->vert[0] == tri2->vert[i])
	{
	    int	i1, i2;

	    i1 = NextVertIndex[i];
	    i2 = NextVertIndex[i1];

	    if (tri1->vert[1] == tri2->vert[i2] && 
		tri1->vert[2] == tri2->vert[i1])
		return 2;

	    if (tri1->vert[1] != tri2->vert[i1] || 
		tri1->vert[2] != tri2->vert[i2])
		return 0;
	   
	    return 1;
	}
    }
    return 0;
}

	/*------------------------------------------------------*/

#ifdef CAUTIOUS
#define INITMESH	replaceB = FALSE
#define SWAPMESH	replaceB = !replaceB
#define REPLACEVERT(v)	{vert[replaceB] = (v); SWAPMESH;}
#else
#define INITMESH
#define SWAPMESH
#define REPLACEVERT(v)	
#endif

#define isMember(v, t)	((v == t->vert[0]) | \
			(v == t->vert[1]) | (v == t->vert[2]))

/*** returns the index of the vertex in tri that is not in tri2 ***/
static int
notCommon(Tri * tri, Tri * tri2)
{
    ushort v;

    v = tri->vert[0];
    if (v != tri2->vert[0] && v != tri2->vert[1] && v != tri2->vert[2])
	return 0;

    v = tri->vert[1];
    if (v != tri2->vert[0] && v != tri2->vert[1] && v != tri2->vert[2])
	return 1;

    v = tri->vert[2];
    if (v != tri2->vert[0] && v != tri2->vert[1] && v != tri2->vert[2])
	return 2;

    /* 
     * Tris are equal and perhaps back-to-back. Any index would do but
     * return 0 for laughs.
    */
    return 0;
}

static int MaxTris = 1000;
static int ShowStrips = 0;
static int LocalLighting = 0;
static int Retessellate = 1;
static int Indexed = 0;
#ifdef DECIMATE
static int Decimate = 1;
static float DecimateEdgeTol = .02f, DecimatePlaneTol = .02f;
#endif

static void
copyAttr2(pfVec2 **dst, ushort **ilist, pfVec2 *src, ushort v[], int n)
{
    int    i;

    if (Indexed)
    {
	for (i=0; i<n; i++)
	{
	    *(*ilist) = v[i];
	    (*ilist) += 1;
	}
    }
    else
    {
	for (i=0; i<n; i++)
	{
	    pfCopyVec2(**dst, src[v[i]]);
	    (*dst) += 1;
	}
    }
}

static void
copyAttr3(pfVec3 **dst, ushort **ilist, pfVec3 *src, ushort v[], int n)
{
    int    i;

    if (Indexed)
    {
	for (i=0; i<n; i++)
	{
	    *(*ilist) = v[i];
	    (*ilist) += 1;
	}
    }
    else
    {
	for (i=0; i<n; i++)
	{
	    pfCopyVec3(**dst, src[v[i]]);
	    (*dst) += 1;
	}
    }
}

static void
copyAttr4(pfVec4 **dst, ushort **ilist, pfVec4 *src, ushort v[], int n)
{
    int    i;

    if (Indexed)
    {
	for (i=0; i<n; i++)
	{
	    *(*ilist) = v[i];
	    (*ilist) += 1;
	}
    }
    else
    {
	for (i=0; i<n; i++)
	{
	    pfCopyVec4(**dst, src[v[i]]);
	    (*dst) += 1;
	}
    }
}

PFDUDLLEXPORT void
pfdMesherMode(int mode, int val)
{
    switch(mode) {
    case PFDMESH_LOCAL_LIGHTING:
	LocalLighting = val;
	break;
    case PFDMESH_SHOW_TSTRIPS:
	ShowStrips = val;
	break;
    case PFDMESH_RETESSELLATE:
	Retessellate = val;
	break;
    case PFDMESH_INDEXED:
	Indexed = val;
	break;
    case PFDMESH_MAX_TRIS:
	MaxTris = val;
	break;
    default:
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdMesherMode: Unknown token "
		    "%d",  mode);
    }
}

PFDUDLLEXPORT int
pfdGetMesherMode(int mode)
{
    switch(mode) {
    case PFDMESH_LOCAL_LIGHTING:
	return LocalLighting;
    case PFDMESH_SHOW_TSTRIPS:
	return ShowStrips;
    case PFDMESH_RETESSELLATE:
	return Retessellate;
    case PFDMESH_INDEXED:
	return Indexed;
    case PFDMESH_MAX_TRIS:
	return MaxTris;
    default:
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdGetMesherMode: Unknown token "
		    "%d",  mode);
	return -1;
    }
}

#define RAND1	((random() & 0xffff) / 65535.0f)

/*
 * Assign each strip a random color. The first triangle in the strip is
 * slightly darker than the rest.
*/
PFDUDLLEXPORT void
pfdShowStrips(pfGeoSet *gset)
{
    int	np, i, j, t = 0, *lengths, k, prim;
    ushort	*foo, *iclrs;
    pfVec4	*clrs;
    pfMaterial	*mtl;
    pfGeoState	*gstate;
    void	*arena = pfGetArena(gset);

    prim = pfGetGSetPrimType(gset);

    if (prim != PFGS_TRISTRIPS &&  prim != PFGS_FLAT_TRISTRIPS &&
        prim != PFGS_LINESTRIPS && prim != PFGS_FLAT_LINESTRIPS)
        return;

    np = pfGetGSetNumPrims(gset);
    lengths = pfGetGSetPrimLengths(gset);

    for(i=0; i<np; i++)
    {
    	if(prim == PFGS_FLAT_TRISTRIPS || prim == PFGS_FLAT_LINESTRIPS)
	    t += lengths[i]-2;
	else
	    t += lengths[i];
    }

    pfGetGSetAttrLists(gset, PFGS_COLOR4, (void**)&clrs, &foo);	
    
    /* 
     * Do not delete color list if gset is indexed since other geosets
     * may need to reference it.
    */
    if (foo == NULL)
	pfDelete(clrs);

    pfDelete(foo);

    clrs = pfMalloc(sizeof(pfVec4) * t, arena);
    if (Indexed)
    {
	iclrs = pfMalloc(sizeof(ushort) * t, arena);
	for(i=0; i<t; i++)
	    iclrs[i] = i;
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, clrs, iclrs);
    }
    else
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, clrs, NULL);

    /* Turn on lmcolor AD mode so we can see strip colors */
    gstate = pfGetGSetGState(gset);
    if (gstate && (mtl = pfGetGStateAttr(gstate, PFSTATE_FRONTMTL)))
    	pfMtlColorMode(mtl, PFMTL_FRONT, PFMTL_CMODE_AD);

    for(i=0, k=0; i<np; i++)
    {
	pfVec4	clr;
	int	n;

	pfSetVec4(clr, RAND1, RAND1, RAND1, 1.0f);

    	if(prim == PFGS_FLAT_TRISTRIPS || prim == PFGS_FLAT_LINESTRIPS)
	{
    	    pfScaleVec4(clrs[k++], .8f, clr);
	    j = 1;
	    n = lengths[i]-2;
	}
	else
	{
    	    pfScaleVec4(clrs[k++], .8f, clr);
    	    pfScaleVec4(clrs[k++], .8f, clr);
    	    pfScaleVec4(clrs[k++], .8f, clr);
	    j = 3;
	    n = lengths[i];
	}

    	for(; j<n; j++, k++)
/*	    pfScaleVec4(clrs[k], (float)(lengths[i] - j) / (float)lengths[i], clr);*/
	    pfCopyVec4(clrs[k], clr);
    }
}


/*
 * Input: A pfGeoSet consisting of independent triangles, PFGS_TRIS.
 * 	The input may be indexed or nonindexed.
 *
 * Output: The input pfGeoSet if it cannot strip the input or a 
 * 	single pfGeoSet that is a collection of tstrips.
 * 	The output pfGeoSet is nonindexed.
*/

static ushort  *Cindex, *Nindex, *tindex[PF_MAX_TEXTURES];
static pfVec3  *Norms, *Coords;
static pfVec4  *Colors;
static pfVec2  *texCoords[PF_MAX_TEXTURES];
static int	MixedBindFlag, CBind, NBind, tbind[PF_MAX_TEXTURES];

#define	NFLAT	0x1
#define	CFLAT	0x2

static int	NextEdge;
static int	SwapFlag;

#ifdef SHOWADJ
static pfVec4	*AdjColors;
#endif

/*
 * Check if strip consists of a quad and retessellate the quad if possible.
*/

static int
swapTriList(int primsInStrip, TriList * triList, int numTextures)
{
    Tri    * tri1, * tri2;
    int    i, j;
    ushort vi, vj;
    int		t;

    /* must be two primitives in strip to retessellate */
    if (primsInStrip != 2)
        return 0;

    /* check for a quad */
    tri1 = triList->next;
    tri2 = tri1->next;

    /* just check for simple strips now */
    if (tri1->adjCount != 1 || tri2->adjCount != 1)
        return 0;

    if (!(tri1->flags & NORM_OK))
        computeNormal(tri1);

    if (!(tri2->flags & NORM_OK))
        computeNormal(tri2);

    /* Make sure two triangles are coplanar */
    if (!PFALMOST_EQUAL_VEC3(tri1->norm, tri2->norm, SORTOF))
      return 0;

    /* Make sure PER_PRIM colors of the two triangles are the same */
    if (CBind == PFGS_PER_PRIM &&
        !PFALMOST_EQUAL_VEC4(Colors[Cindex[tri1->id]],
                             Colors[Cindex[tri2->id]], SORTOF))
        return 0;

    i = notCommon(tri1, tri2);
    j = notCommon(tri2, tri1);

    vi = tri1->vert[i];
    vj = tri2->vert[j];

#if 0
    /* Make sure vertex colors are the same */
    if (CBind == PFGS_PER_VERTEX &&
        (!PFALMOST_EQUAL_VEC4(Colors[Cindex[vi]],
                              Colors[Cindex[vj]], SORTOF) ||
         !PFALMOST_EQUAL_VEC4(Colors[Cindex[vi]],
                              Colors[Cindex[(vj+1) % 3]], SORTOF)))
        return 0;

#else

        /* XXXXX assume color and texcoord are linearly interpolated in
           hardware. needs to be changed if it is not true any more - zhz */
        if (CBind == PFGS_PER_VERTEX &&
            !coplanar(tri1, tri2, PFGS_COLOR4, 0))
            return 0;

	for (t = 0 ; t < numTextures ; t ++)
	    if (tbind[t] == PFGS_PER_VERTEX &&
		!coplanar(tri1, tri2, PFGS_TEXCOORD2, t))
		return 0;

#if CHECK_NORMAL
        /* XXX we should generate normal after we tessellate */
        /* Only retessellate if all normals are the same. */
        if (NBind == PFGS_PER_VERTEX &&
            (!PFALMOST_EQUAL_VEC3(Norms[Nindex[vi]],
                                Norms[Nindex[vj]], SORTOF) ||
             !PFALMOST_EQUAL_VEC3(Norms[Nindex[vi]],
                                Norms[Nindex[(vj+1) % 3]], SORTOF) ||
             !PFALMOST_EQUAL_VEC3(Norms[Nindex[vi]],
                                Norms[Nindex[(vj+2) % 3]], SORTOF)))
            return 0;
#endif
#endif

    /* Make sure two triangles form convex quad */
    if (concaveCheck(tri1->norm, vi, tri1->vert[NextVertIndex[i]],
                                 vj, tri2->vert[NextVertIndex[j]]))
      return 0;

    /* retessellate quad in strip */
    if (tri1->adj[0])
    {
        tri1->vert[2] = vj;
        NextEdge = 0;
    }
    else if (tri1->adj[1])
    {
        tri1->vert[0] = vj;
        NextEdge = 1;
    }
    else
    {
        tri1->vert[1] = vj;
        NextEdge = 2;
    }

    if (tri2->adj[0])
    {
        Tri * tmp = tri2->adj[0];
        tri2->vert[0] = vi;
        tri2->adj[0] = tri2->adj[2];
        tri2->adj[2] = tmp;
    }
    else if (tri2->adj[1])
    {
        Tri * tmp = tri2->adj[1];
        tri2->vert[1] = vi;
        tri2->adj[1] = tri2->adj[0];
        tri2->adj[0] = tmp;
    }
    else
    {
        Tri * tmp = tri2->adj[2];
        tri2->vert[2] = vi;
        tri2->adj[2] = tri2->adj[1];
        tri2->adj[1] = tmp;
    }

    return 1;
}

PFDUDLLEXPORT pfGeoSet*
pfdMeshGSet(pfGeoSet *gset)
{
    register Tri   *tri;
    int         i, j, k, flatStrip = 0;
    TriList     *triList, *newTriList, *singleList, *quadList, *stripList;
    pfVec3      *newCoords = NULL, *newNorms = NULL;
    pfVec2      /* *texCoords,*/ *newTexCoords[PF_MAX_TEXTURES];
    pfVec4      *newColors = NULL;
    ushort	*inewCoords, *inewNorms, *inewTexCoords[PF_MAX_TEXTURES], *inewColors;
    ushort      *vindex /*, *tindex */;
    
#ifdef SHOWADJ
    ushort	*iadjColors;
#endif
    
    int         /*tbind,*/ nprims, numStripVerts = 0;
    int         primsInStrip, numSingles = 0, numQuads = 0, numStrips = 0;
    int	    	*lengthList = NULL, flatQuads = 1;
    int		primType, vertsPerPrim;
    pfGeoSet	*gt = NULL;
    pfGeoState	*gstate;
    void    	*arena = pfGetArena(gset);
    int		t;
    int		numTextures;
    int		got_unique_texture;

    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	newTexCoords[t] = NULL;

    primType = pfGetGSetPrimType(gset);
    if (primType == PFGS_TRIS)
	vertsPerPrim = 3;
    else if (primType == PFGS_LINES)
	vertsPerPrim = 2;
    else
	return NULL;

    numTextures = pfGetGSetNumTextures(gset);
    /* 
     * pfuHashGSetVerts will share vertices amongst triangles/lines and
     * will generate new PER_VERTEX attribute lists. If 'Indexed' is set,
     * then don't hash verts because we assume that the user wants
     * the geoset attribute lists unchanged.	
    */
    pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&Coords, &vindex);
    pfGetGSetAttrLists(gset, PFGS_COLOR4, (void**)&Colors, &Cindex);
    for (t = 0 ; t < numTextures ; t ++)
	pfGetGSetMultiAttrLists(gset, PFGS_TEXCOORD2, t, 
				(void**)&texCoords[t], &tindex[t]);
    pfGetGSetAttrLists(gset, PFGS_NORMAL3, (void**)&Norms, &Nindex);
    CBind = pfGetGSetAttrBind(gset, PFGS_COLOR4);
    NBind = pfGetGSetAttrBind(gset, PFGS_NORMAL3);
    for (t = 0 ; t < numTextures ; t ++)
	tbind[t] = pfGetGSetMultiAttrBind(gset, PFGS_TEXCOORD2, t);

#if 0
    /* Not fixing for mutli-texture (commented out code). */

    /* this is a test for tricky vertex colors */
    Colors = (pfVec4 *)pfMalloc(sizeof(pfVec4)*24, pfGetArena(gset));
    for(i = 0; i < 24; i++)
    {
        Colors[i][0] = texCoords[i][0];
        Colors[i][1] = texCoords[i][1];
        Colors[i][2] = Colors[i][3] = 1;
printf("vert %f %f %f, c %f %f\n", Coords[i][0], Coords[i][1], Coords[i][2],
        Colors[i][0], Colors[i][1]);
    }
    CBind = tbind;

    Colors[10][0] = Colors[13][0] = 1;
    Colors[10][1] = Colors[13][1] = 0;

    Colors[12][0] = Colors[17][0] = 1;
    Colors[12][1] = Colors[12][1] = 0;

    Colors[16][0] = Colors[19][0] = 1;
    Colors[16][1] = Colors[19][1] = 0;

    Colors[9][0] = Colors[14][0] = Colors[15][0] =0;
    Colors[20][0] = Colors[3][0] = Colors[23][0] =0;
    Colors[8][0] = 0;

    Colors[9][1] = Colors[14][1] = Colors[15][1] =0;
    Colors[20][1] = Colors[3][1] = Colors[23][1] =0;
    Colors[8][1] = 0;

    Coords[12][1] = Coords[17][1] = 20;

    pfGSetAttr(gset, PFGS_COLOR4, CBind, (void*)Colors, Cindex);
#endif

    /*
     * if non-indexed or per-vertex attributes are not sharing
     * the vertex index array, hash 'em
     */
    got_unique_texture = 0;
    for (t = 0 ; t < numTextures ; t ++)
        if ((tbind[t] == PFGS_PER_VERTEX) && (vindex != tindex[t]))
	    got_unique_texture = 1;

    if (vindex == NULL || 
	(CBind == PFGS_PER_VERTEX) && (vindex != Cindex) ||
	(NBind == PFGS_PER_VERTEX) && (vindex != Nindex) ||
	got_unique_texture ||
	!Indexed)
    {
	if (pfuHashGSetVerts(gset) == 0)
	    return NULL;
	pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&Coords, &vindex);
	pfGetGSetAttrLists(gset, PFGS_COLOR4, (void**)&Colors, &Cindex);
	for (t = 0 ; t < numTextures ; t ++)
	    pfGetGSetMultiAttrLists(gset, PFGS_TEXCOORD2, t,
                                (void**)&texCoords[t], &tindex[t]);
	pfGetGSetAttrLists(gset, PFGS_NORMAL3, (void**)&Norms, &Nindex);
	CBind = pfGetGSetAttrBind(gset, PFGS_COLOR4);
	NBind = pfGetGSetAttrBind(gset, PFGS_NORMAL3);
	for (t = 0 ; t < numTextures ; t ++)
	    tbind[t] = pfGetGSetMultiAttrBind(gset, PFGS_TEXCOORD2, t);
    }

    nprims = pfGetGSetNumPrims(gset);
    if (nprims == 1)
	return NULL;

    /* Set flag if one attribute is per-vertex and the other per-primitive */
    if (CBind == PFGS_PER_VERTEX && NBind == PFGS_PER_PRIM)
        MixedBindFlag = NFLAT;
    else if (NBind == PFGS_PER_VERTEX && CBind == PFGS_PER_PRIM)
	MixedBindFlag = CFLAT;
    else 
	MixedBindFlag = 0;

    gstate = pfGetGSetGState(gset);

    /*** initialize lists ***/
    triList = makeTriList();
    newTriList = makeTriList();
    singleList = makeTriList();
    quadList = makeTriList();
    stripList = makeTriList();
    for (i = 0; i < 4; i++)
	AdjTriList[i] = makeTriList();

    makeTris(nprims);
    for (j = 0, k=0; j < nprims; j++)
    {
	tri = initTri();
	tri->id = j;
	insertTri(triList, tri);
	if (primType == PFGS_TRIS)
	{
	    for (i = 0; i < 3; i++, k++)
		tri->vert[i] = vindex[k];
	}
	else
	{
	    for (i = 0; i < 2; i++, k++)
		tri->vert[i] = vindex[k];
	    tri->vert[2] = 0xffff;
	}
    }

#ifdef SHOWADJ
    {
	AdjColors = pfMalloc(sizeof(pfVec4) * nprims, arena);
	if (Indexed)
	{
	    iadjColors = pfMalloc(sizeof(ushort) * nprims, arena);
	    for (j = 0; j < nprims; j++)
		iadjColors[j] = j;
	}
    }
#endif

    eliminatePrims(triList, nprims);

    if (primType == PFGS_TRIS)
	computeTriAdjacencies(triList, nprims);
    else
	computeLineAdjacencies(triList, nprims);

    /*** search for connected triangles and output ***/
    while (1)
    {
	/*** output singular triangles, if any ***/
	while ((tri = AdjTriList[0]->next) != (Tri *) AdjTriList[0])
	{
	    removeTri(AdjTriList[0], tri);
	    insertTri(singleList, tri);
	    tri->used = TRUE;
	    numSingles++;
	}
	/* choose a seed triangle with the minimum number of adjacencies */
	tri = chooseSeed();
	if(tri == NULL)
	    break;

	insertTri(newTriList, tri);
	adjustAdjacencies(tri, vertsPerPrim);

	primsInStrip = 1;
	initStrip();

#if OLD
	/*** extend in one direction using triangles with min adjacencies ***/
	if (primType == PFGS_TRIS)
	    tri = getNextTri(tri, numTextures);
	else
	    tri = getNextLine(tri);
#else
        if (Retessellate)
        {
            Tri * newtri;
     
            if (primType == PFGS_TRIS)
                newtri = getNextTri(tri, numTextures);
            else
                newtri = getNextLine(tri);
     
            /* check if triangle has adjacencies but not added to strip */
     
            if (!newtri && tri->adjCount)
            {
                /* retesselate strip if possible */
                if (swapTriList(primsInStrip, newTriList, numTextures))
                {
                    if (primType == PFGS_TRIS)
                        newtri = getNextTri(tri, numTextures);
                    else
                        newtri = getNextLine(tri);
                }
            }
     
            tri = newtri;
        }
        else
        {
            if (primType == PFGS_TRIS)
                tri = getNextTri(tri, numTextures);
            else
                tri = getNextLine(tri);
        }
#endif

	while (tri && primsInStrip < MaxTris)
	{
	    removeTri(AdjTriList[tri->adjCount], tri);
	    insertTri(newTriList, tri);
	    adjustAdjacencies(tri, vertsPerPrim);
	    primsInStrip++;

	    if (primType == PFGS_TRIS)
		tri = getNextTri(tri, numTextures);
	    else
		tri = getNextLine(tri);
	}

	if (primsInStrip == 1)		/* Single triangle */
	{
	    tri = newTriList->next;
	    removeTri(newTriList, tri);
	    insertTri(singleList, tri);
	    numSingles++;
	} 
	else if (primsInStrip == 2 && primType == PFGS_TRIS)	/* Quad */
	{
	    Tri		*trin;
	    ushort	v[4], i;


	    tri = newTriList->next->next;
	    removeTri(newTriList, tri);
	    insertTri(quadList, tri);

	    trin = newTriList->next;
	    removeTri(newTriList, trin);
	    insertTri(quadList, trin);

	    /* 
	     * See if two triangles form concave quad
	     */
	    computeNormal(tri);
	    computeNormal(trin);

	    i = notCommon(tri, trin);
	    v[0] = tri->vert[i];
	    v[1] = tri->vert[(i + 1) % 3];
	    v[2] = tri->vert[(i + 2) % 3];
	    i = notCommon(trin, tri);
	    v[3] = trin->vert[i];

	    if(!PFALMOST_EQUAL_VEC3(tri->norm, trin->norm, SORTOF) ||
	       concaveCheck(tri->norm, v[0], v[1], v[2], v[3]))
		flatQuads = 0;

	    /* 
	     * See if two triangles forming quad have same normal/color. 
	     * If not, we cannot use PFGS_QUADS and must 
	     * PFGS_TRISTRIPS instead.
	    */
	    if(NBind == PFGS_PER_PRIM)
	    {
		float	*nrm0 = Norms[Nindex[tri->id]],
			*nrm1 = Norms[Nindex[trin->id]];

		if(!PFALMOST_EQUAL_VEC3(nrm0, nrm1, SORTOF))
		    flatQuads = 0;
	    }
	    if(CBind == PFGS_PER_PRIM)
	    {
		float	*clr0 = Colors[Cindex[tri->id]],
			*clr1 = Colors[Cindex[trin->id]];

		if(!PFALMOST_EQUAL_VEC4(clr0, clr1, SORTOF))
		    flatQuads = 0;
	    }

	    numQuads++;
	} 
	else			/* A real triangle/line strip! */
	{
	    /*
	     * Add tris to stripList. This reverses strip direction.
	    */
	    tri = newTriList->next;
	    tri->used = -100;		/* Signal end of strip */
	    removeTri(newTriList, tri);
	    insertTri(stripList, tri);

	    while((tri = newTriList->next) != (Tri*)newTriList)
	    {
	    	removeTri(newTriList, tri);
	    	insertTri(stripList, tri);
	    }
	    numStripVerts += primsInStrip + vertsPerPrim-1;
	    numStrips++;
	}

    }	/* End while */

    if ((numSingles + numQuads + numStrips) != 0)
    {

	int num, bind, vertCount;

	vertCount = numSingles * vertsPerPrim + numQuads * 4 + numStripVerts;

	gt = pfNewGSet(arena);

	pfGSetGState(gt, gstate);

	/* 
	 * See if we have just independent triangles or quads in which case
	 * we can use PFGS_TRIS or PFGS_QUADS for better performance.
        */	
	if(numStrips == 0 && numQuads == 0)
	{
	    pfGSetNumPrims(gt, numSingles);
	    pfGSetPrimType(gt, primType);
	}
	else if(numStrips == 0 && numSingles == 0 && 
		flatQuads && primType == PFGS_TRIS)
	{
	    pfGSetNumPrims(gt, numQuads);
	    pfGSetPrimType(gt, PFGS_QUADS);
	}
	else
	{
	    int	n = numSingles + numQuads + numStrips;
	    
	    lengthList = (int *) pfMalloc(sizeof(int) * n, arena);

	    pfGSetNumPrims(gt, n);

	    if(!MixedBindFlag && !LocalLighting && 
	       (CBind == PFGS_PER_PRIM || NBind == PFGS_PER_PRIM))
	    {
		if (primType == PFGS_TRIS)
		    pfGSetPrimType(gt, PFGS_FLAT_TRISTRIPS);
		else if (primType == PFGS_LINES)
		    pfGSetPrimType(gt, PFGS_FLAT_LINESTRIPS);
		flatStrip = 1;
	    }
	    else
	    {
		if (primType == PFGS_TRIS)
		    pfGSetPrimType(gt, PFGS_TRISTRIPS);
		else if (primType == PFGS_LINES)
		    pfGSetPrimType(gt, PFGS_LINESTRIPS);
		flatStrip = 0;
	    }

	    pfGSetPrimLengths(gt, lengthList);
	}

	switch(NBind)
	{
    	case PFGS_PER_VERTEX:
	    if (Indexed)
	    {
		inewNorms = pfMalloc(sizeof(ushort) * vertCount, arena);
		pfGSetAttr(gt, PFGS_NORMAL3, PFGS_PER_VERTEX, 
			   Norms, inewNorms);
	    }
	    else
	    {
		newNorms = pfMalloc(sizeof(pfVec3) * vertCount, arena);
		pfGSetAttr(gt, PFGS_NORMAL3, PFGS_PER_VERTEX, newNorms, NULL);
	    }
	    break;
	case PFGS_PER_PRIM:
	    if(lengthList)	/* tstrips rather than independents */
	    {
		if(flatStrip)   
		{
		    /* 
		     * Special flat-shaded line/tstrip doesn't need  
  		     * first one/two normals in a strip 
		    */
		    num = numSingles + numQuads * 2 + 
			numStripVerts - numStrips * (vertsPerPrim-1);
		    bind = PFGS_PER_VERTEX;
		}
		else 
		{
		    /* 
		     * Here we're guaranteed that all tris in a strip have the
		     * same normal.
  		    */
		    num = numSingles + numQuads + numStrips;
		    bind = PFGS_PER_PRIM;
		}
	    }
	    else	/* no strips at all */
	    {
		num = numSingles + numQuads;
		bind = PFGS_PER_PRIM;
	    }

	    /* Now allocate normals and indices */
	    if (Indexed)
	    {
		inewNorms = pfMalloc(sizeof(ushort) * num, arena);
		pfGSetAttr(gt, PFGS_NORMAL3, bind, Norms, inewNorms);
	    }
	    else
	    {
		newNorms = pfMalloc(sizeof(pfVec3) * num, arena);
		pfGSetAttr(gt, PFGS_NORMAL3, bind, newNorms, NULL);
	    }
	    break;
    	case PFGS_OVERALL:
	    if (Indexed)
	    {
		inewNorms = pfMalloc(sizeof(ushort), arena);
		inewNorms[0] = 0;
		pfGSetAttr(gt, PFGS_NORMAL3, NBind, Norms, inewNorms);
	    }
	    else
	    {
		newNorms = pfMalloc(sizeof(pfVec3), arena);
		PFCOPY_VEC3(newNorms[0], Norms[0]);
		pfGSetAttr(gt, PFGS_NORMAL3, NBind, newNorms, NULL);
	    }
	    break;
    	case PFGS_OFF:
    	    pfGSetAttr(gt, PFGS_NORMAL3, NBind, NULL, NULL);
	    break;
	}

	switch(CBind)
	{
    	case PFGS_PER_VERTEX:
	    if (Indexed)
	    {
		inewColors = pfMalloc(sizeof(ushort) * vertCount, arena);
		pfGSetAttr(gt, PFGS_COLOR4, PFGS_PER_VERTEX, 
			   Colors, inewColors);
	    }
	    else
	    {
		newColors = pfMalloc(sizeof(pfVec4) * vertCount, arena);
		pfGSetAttr(gt, PFGS_COLOR4, PFGS_PER_VERTEX, newColors, NULL);
	    }
	    break;
	case PFGS_PER_PRIM:
	    if(lengthList)	/* tstrips rather than independents */
	    {
		if(flatStrip)   
		{
		    /* 
		     * Special flat-shaded tstrip doesn't need  
		     * first two colors in a strip 
       		    */
		    num = numSingles + numQuads * 2 + 
			numStripVerts - numStrips * (vertsPerPrim-1);
		    bind = PFGS_PER_VERTEX;
		}
		else 
		{
		    /* 
		     * Here we're guaranteed that all tris in a strip have the
		     * same color.
 		    */
		    num = numSingles + numQuads + numStrips;
		    bind = PFGS_PER_PRIM;
		}
	    }
	    else	/* no strips at all */
	    {
		num = numSingles + numQuads;
		bind = PFGS_PER_PRIM;
	    }

	    /* Now allocate colors and indices */
	    if (Indexed)
	    {
		inewColors = pfMalloc(sizeof(ushort) * num, arena);
		pfGSetAttr(gt, PFGS_COLOR4, bind, Colors, inewColors);
	    }
	    else
	    {
		newColors = pfMalloc(sizeof(pfVec4) * num, arena);
		pfGSetAttr(gt, PFGS_COLOR4, bind, newColors, NULL);
	    }
	    break;
    	case PFGS_OVERALL:
	    if (Indexed)
	    {
		inewColors = pfMalloc(sizeof(ushort), arena);
		inewColors[0] = 0;
		pfGSetAttr(gt, PFGS_COLOR4, CBind, Colors, inewColors);
	    }
	    else
	    {
		newColors = pfMalloc(sizeof(pfVec4), arena);
		PFCOPY_VEC4(newColors[0], Colors[0]);
		pfGSetAttr(gt, PFGS_COLOR4, CBind, newColors, NULL);
	    }
	    break;
    	case PFGS_OFF:
    	    pfGSetAttr(gt, PFGS_COLOR4, CBind, NULL, NULL);
	    break;
	}

	for (t = 0 ; t < numTextures ; t ++)
	{
	    if(tbind[t] == PFGS_PER_VERTEX)
	    {
		if (Indexed)
		{
		    inewTexCoords[t] = 
				pfMalloc(sizeof(ushort) * vertCount, arena);
		    pfGSetMultiAttr(gt, PFGS_TEXCOORD2, t, tbind[t], 
				texCoords[t], inewTexCoords[t]);
		}
		else
		{
		    newTexCoords[t] = 
				pfMalloc(sizeof(pfVec2) * vertCount, arena);
		    pfGSetMultiAttr(gt, PFGS_TEXCOORD2, t, tbind[t], 
				newTexCoords[t], NULL);
		    
		}
	    }
	    else
		pfGSetMultiAttr(gt, PFGS_TEXCOORD2, t, tbind[t], NULL, NULL);
	}
	
	if (Indexed)
	{
	    inewCoords = pfMalloc(sizeof(ushort) * vertCount, arena);
	    pfGSetAttr(gt, PFGS_COORD3, PFGS_PER_VERTEX, Coords, inewCoords);
	}
	else
	{
	    newCoords = pfMalloc(sizeof(pfVec3) * vertCount, arena);
	    pfGSetAttr(gt, PFGS_COORD3, PFGS_PER_VERTEX, newCoords, NULL);
	}

#ifdef SHOWADJ
	if (Indexed)
	    pfGSetAttr(gt, PFGS_COLOR4, PFGS_PER_PRIM, AdjColors, iadjColors);
	else
	    pfGSetAttr(gt, PFGS_COLOR4, PFGS_PER_PRIM, AdjColors, NULL);
#endif

	k = 0;	/* lengthList counter */

	/* Now create tri and quad lists */
	if (numSingles > 0)
	{
	    while ((tri = singleList->next) != (Tri *) singleList)
	    {
		ushort  v[3];

		v[0] = tri->vert[0];
		v[1] = tri->vert[1];
		v[2] = tri->vert[2];

		if(lengthList)
		    lengthList[k++] = vertsPerPrim;

		copyAttr3(&newCoords, &inewCoords, Coords, v, vertsPerPrim);

		for (t = 0 ; t < numTextures ; t ++)
		    if (tbind[t] == PFGS_PER_VERTEX)
			copyAttr2(&newTexCoords[t], 
				&inewTexCoords[t], texCoords[t], 
				v, vertsPerPrim);

		if (NBind == PFGS_PER_VERTEX)
		    copyAttr3(&newNorms, &inewNorms, Norms, v, vertsPerPrim);
		else if(NBind == PFGS_PER_PRIM)
		    copyAttr3(&newNorms, &inewNorms, Norms, 
			      &Nindex[tri->id], 1);
		if (CBind == PFGS_PER_VERTEX)
		    copyAttr4(&newColors, &inewColors, Colors, v, vertsPerPrim);
		else if(CBind == PFGS_PER_PRIM)
		    copyAttr4(&newColors, &inewColors, Colors,
			      &Cindex[tri->id], 1);
		removeTri(singleList, tri);
	    }
	}
	if (numQuads > 0)	/* No lines here! */
	{
	    while ((tri = quadList->next) != (Tri *) quadList)
	    {
		ushort	v[4];

		i = notCommon(tri, tri->next);
		v[0] = tri->vert[i];
		v[1] = tri->vert[(i + 1) % 3];
		v[2] = tri->vert[(i + 2) % 3];
		i = notCommon(tri->next, tri);
		v[3] = tri->next->vert[i];

		if(lengthList)
		    lengthList[k++] = 4;
		else		/* Swap vert ordering since we're drawing */
		{			/* QUADS, not TSTRIPS */
		    ushort	vt = v[2];
		    v[2] = v[3];
		    v[3] = vt;
		}

		copyAttr3(&newCoords, &inewCoords, Coords, v, 4);

		for (t = 0 ; t < numTextures ; t ++)
		    if(tbind[t] == PFGS_PER_VERTEX)
			copyAttr2(&newTexCoords[t], &inewTexCoords[t], 
					texCoords[t], v, 4);

		if(NBind == PFGS_PER_VERTEX)
		    copyAttr3(&newNorms, &inewNorms, Norms, v, 4);

		else if(NBind == PFGS_PER_PRIM)
		{
		    ushort vt[2];

		    vt[0] = Nindex[tri->id];

		    /* Quads are being rendered as strips */
		    if(lengthList)	
		    {
			if (flatStrip)    /* FLAT_TSTRIP */
			{
			    vt[1] = Nindex[tri->next->id];
			    copyAttr3(&newNorms, &inewNorms, Norms, vt, 2);
			}
			else 
			    copyAttr3(&newNorms, &inewNorms, Norms, vt, 1);
		    }
		    else
		    {
			copyAttr3(&newNorms, &inewNorms, Norms, vt, 1);
		    }
		}

		if(CBind == PFGS_PER_VERTEX)
		    copyAttr4(&newColors, &inewColors, Colors, v, 4);
		else if(CBind == PFGS_PER_PRIM)
		{
		    ushort vt[2];

		    vt[0] = Cindex[tri->id]; 

		    /* Quads are being rendered as strips */
		    if(lengthList)
		    {
			if (flatStrip)    /* FLAT_TSTRIP */
			{
			    vt[1] = Cindex[tri->next->id];
			    copyAttr4(&newColors, &inewColors, Colors, vt, 2);
			}
			else
			    copyAttr4(&newColors, &inewColors, Colors, vt, 1);
		    }
		    else
		    {
			copyAttr4(&newColors, &inewColors, Colors, vt, 1);
		    }
		}

		removeTri(quadList, tri->next);
		removeTri(quadList, tri);
	    }
	}
	if (numStrips > 0)
	{
	    int            jj, tt;

	    for (t = 0; t < numStrips; t++)
	    {
		ushort	v[3];
                int lastVert;

		jj = 0;

		tri = stripList->next;

		if (primType == PFGS_LINES)
		{
		    if (tri->adj[0] == tri->next)
		    {
			v[0] = tri->vert[1];
			v[1] = tri->vert[0];
		    }
		    else
		    {
			v[0] = tri->vert[0];
			v[1] = tri->vert[1];
		    }
		    lastVert = v[1];
		}
		else
		{
		    /* start output with vertex that is not in the
			second triangle */
		    INITMESH;
		    i = notCommon(tri, tri->next);
		    v[0] = tri->vert[i];
		    v[1] = tri->vert[(i + 1) % 3];
		    v[2] = tri->vert[(i + 2) % 3];
		}

		copyAttr3(&newCoords, &inewCoords, Coords, v, vertsPerPrim);
		jj += vertsPerPrim;

		for (tt = 0 ; tt < numTextures ; tt ++)
		    if(tbind[tt] == PFGS_PER_VERTEX)
			copyAttr2(&newTexCoords[tt], &inewTexCoords[tt], 
					texCoords[tt], v, vertsPerPrim);

		if(NBind == PFGS_PER_VERTEX)
		    copyAttr3(&newNorms, &inewNorms, Norms, v, vertsPerPrim);
		else if(NBind == PFGS_PER_PRIM)
		    copyAttr3(&newNorms, &inewNorms, Norms,
			      &Nindex[tri->id], 1);

		if(CBind == PFGS_PER_VERTEX)
		    copyAttr4(&newColors, &inewColors, Colors, v, vertsPerPrim);
		else if(CBind == PFGS_PER_PRIM)
		    copyAttr4(&newColors, &inewColors, Colors, 
			      &Cindex[tri->id], 1);
		REPLACEVERT(tri->vert[i]);
		REPLACEVERT(tri->vert[(i + 1) % 3]);
		REPLACEVERT(tri->vert[(i + 2) % 3]);

		do
		{
		    VertIndex   nextvert;

		    /*** determine the next output vertex ***/
		    if (primType == PFGS_LINES)
		    {
			if (tri->next->vert[0] == lastVert) 
			    nextvert = tri->next->vert[1];
			else
			    nextvert = tri->next->vert[0];
		        lastVert = nextvert;
		    }
		    else
		    {
			i = notCommon(tri->next, tri);
			nextvert = (tri->next)->vert[i];
		    }

		    removeTri(stripList, tri);
		    tri = stripList->next;
#ifdef CAUTIOUS
		    /*** decide whether to swap or not ***/
		    if (isMember(vert[replaceB], tri->next))
		    {
			pfNotify(PFNFY_ALWAYS, PFNFY_PRINT, "pfdMeshGSet: swaptmesh();");
			SWAPMESH;
		    }
#endif
		    /*** output the next vertex ***/
		    copyAttr3(&newCoords, &inewCoords, Coords, &nextvert, 1);
		    jj++;
		    REPLACEVERT(nextvert);

		    for (tt = 0 ; tt < numTextures ; tt ++)
			if(tbind[tt] == PFGS_PER_VERTEX)
			{
			    copyAttr2(&newTexCoords[tt], &inewTexCoords[tt], 
				texCoords[tt], 
				&nextvert, 1);
			}

		    if(NBind == PFGS_PER_VERTEX)
		    {
			copyAttr3(&newNorms, &inewNorms, Norms, 
				  &nextvert, 1);
		    }
		    else if(NBind == PFGS_PER_PRIM && flatStrip)
		    {
			copyAttr3(&newNorms, &inewNorms, Norms, 
				  &Nindex[tri->id], 1);
		    }

		    if(CBind == PFGS_PER_VERTEX)
		    {
			copyAttr4(&newColors, &inewColors, Colors, 
				  &nextvert, 1);
		    }
		    else if(CBind == PFGS_PER_PRIM && flatStrip)
		    {
			copyAttr4(&newColors, &inewColors, Colors, 
				  &Cindex[tri->id], 1);
		    }

		} while (tri->used != -100);    /* -100 indicates end of strip */

		removeTri(stripList, tri);

		if(lengthList)
		    lengthList[k++] = jj;
	    }
	}

	if(ShowStrips)
	    pfdShowStrips(gt);

    }
    else
	gt = NULL;

    /*** free lists ***/
    freeTriList(triList);
    freeTriList(newTriList);
    freeTriList(singleList);
    freeTriList(quadList);
    freeTriList(stripList);
    for (i = 0; i < 4; i++)
	freeTriList(AdjTriList[i]);
    freeTris();

    return gt;
}

	/*---------------------------------------------------------*/

/* Works for both tris and lines */
static void
eliminatePrims(TriList *triList, int nprims)
{
    register Tri    *tri, *tmpTri, *nextTri;
    int            degenerateCount;
    int            equivalentCount;
    int            count, i;

#define ELIMINATE_DEGENERATE
#ifdef ELIMINATE_DEGENERATE
    /*** eliminate degenerate prims ***/
#ifdef	VERBOSE
    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "eliminatePrims: "
	"eliminating degenerate prims");
#endif
    degenerateCount = 0;
    for (tri = triList->next; tri != (Tri *) triList;)
    {
	if ((tri->vert[0] == tri->vert[1]) ||
	    (tri->vert[0] == tri->vert[2]) ||
	    (tri->vert[1] == tri->vert[2]))
	{
	    degenerateCount += 1;
	    tmpTri = tri->next;
	    removeTri(triList, tri);
	    tri = tmpTri;
	} 
	else
	    tri = tri->next;
    }
#ifdef	VERBOSE
    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "eliminatePrims: "
	"%d degenerate prims eliminated", degenerateCount);
#endif
#endif

#define ELIMINATE_REDUNDANT
#ifdef ELIMINATE_REDUNDANT
    /*** eliminate equivalent prims ***/
#ifdef	VERBOSE
    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "eliminatePrims: "
	"eliminating equivalent prims");
#endif
    count = 0;
    equivalentCount = 0;

    hashTriBegin(nprims);
    i = -1;
    for (tri = triList->next; tri != (Tri *) triList;)
    {
	count += 1;
	i++;
	if (hashTriAdd(tri))
	{
	    equivalentCount += 1;
#ifdef VERBOSE
	    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "eliminatePrims: redundant");
	    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "  %g %g %g", 
		tri->vert[0]->x, tri->vert[0]->y, tri->vert[0]->z);
	    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "  %g %g %g", 
		tri->vert[1]->x, tri->vert[1]->y, tri->vert[1]->z);
	    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "  %g %g %g", 
		tri->vert[2]->x, tri->vert[2]->y, tri->vert[2]->z);
	    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "  i = %d", nprims - i - 1);
#endif
	    nextTri = tri->next;
	    removeTri(triList, tri);
	    tri = nextTri;
	} 
	else
	{
	    tri = tri->next;
	}
    }
    hashTriEnd();
#ifdef	VERBOSE
    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "eliminatePrims: "
	"%d equivalent prims eliminated", equivalentCount);
#endif
#endif
}

static void
computeTriAdjacencies(TriList *triList, int ntris)
{
    register Tri   *tri, *nextTri;
    register int   count, i, numTris=0;
    register Edge  *edge;
    VertIndex       index0, index1;

#ifdef	VERBOSE
    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "computeTriAdjacencies: "
	"computing adjacent triangles");
#endif
    hashEdgeBegin(ntris);

#ifdef	VERBOSE
    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "computeTriAdjacencies: "
	"adding edges to hash table");
#endif

    /* Add directed triangle edges to edge hash table */
    for (tri = triList->next; tri != (Tri *) triList; tri = tri->next)
    {
#ifdef DECIMATE
	if (Decimate)
	    computeNormal(tri);
	numTris++;
#endif
	hashEdgeAdd(tri, 0, tri->vert[0], tri->vert[1]);
	hashEdgeAdd(tri, 1, tri->vert[1], tri->vert[2]);
	hashEdgeAdd(tri, 2, tri->vert[2], tri->vert[0]);
    }

#ifdef	VERBOSE
    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "computeTriAdjacencies: "
	"processing triangles");
#endif

    for (tri = triList->next; tri != (Tri *) triList; tri = tri->next)
    {
	/* For all edges of 'tri' */
	for (i = 0; i < 3; i++)
	{
	    Edge	*edgei = NULL;

	    /* We've already found an adjacency for this edge */
	    if (tri->adj[i])
		continue;

	    /* Current edge is index0 -> index1 */
	    index0 = tri->vert[i];
	    index1 = tri->vert[NextVertIndex[i]];

	    /* 'edge' is head of linked list of possible adjacent edges */
	    edge = hashEdgeFind(index0, index1);

	    /* Run through linked list and find an adjacent edge */
	    for (; edge; edge = edge->hnext)
	    {
		float          *t0, *t1;
		int		ok;

		/*
		 * Skip to next edge in list if 'edge' belongs to 'tri' or
		 * edge adjacency has already been computed 
		 */
		if (edge->tri == tri)
		{
		    if(edge->index0 == index0 && edge->index1 == index1)
			edgei = edge;	/* Remember edge 'i' */
		    continue;
		}
		if(edge->adjTri)
		    continue;

		/*
		 * See if edge endpoints match (index0, index1). Also make
		 * sure edge is properly oriented. 
		 */
		if (edge->index0 != index1 || edge->index1 != index0)
		    continue;

		nextTri = edge->tri;

		ok = 1;

		/* 
		 * If we require smoothing, break strip if colors or
		 * normals are different across adjacent triangles 
	        */
		if (!MixedBindFlag && LocalLighting)
		{
		    if (CBind == PFGS_PER_PRIM)
		    {
			t0 = Colors[Cindex[tri->id]];
			t1 = Colors[Cindex[nextTri->id]];
			if (!PFALMOST_EQUAL_VEC4(t0, t1, SORTOF))
			    ok = 0;
		    }
		    if (NBind == PFGS_PER_PRIM)
		    {
			t0 = Norms[Nindex[tri->id]];
			t1 = Norms[Nindex[nextTri->id]];
			if (!PFALMOST_EQUAL_VEC3(t0, t1, SORTOF))
			    ok = 0;
		    }
		}

		/*
		 * We must compute edge adjacencies differently if one
		 * binding is per-vertex and the other is per-primitive. In
		 * this case, two triangles are adjacent only if the
		 * per-primitive attributes match. 
		 */
		switch (MixedBindFlag) {
		case NFLAT:
		    t0 = Norms[Nindex[tri->id]];
		    t1 = Norms[Nindex[nextTri->id]];
		    if (!PFALMOST_EQUAL_VEC3(t0, t1, SORTOF))
			ok = 0;
		    break;
		case CFLAT:
		    t0 = Colors[Cindex[tri->id]];
		    t1 = Colors[Cindex[nextTri->id]];
		    if (!PFALMOST_EQUAL_VEC4(t0, t1, SORTOF))
			ok = 0;
		    break;
		}
		
		/* Tris aren't adjacent if they are back-to-back */
		if (triEqual(tri, nextTri) == 2)
		    ok = 0;

		/* Found an adjacent edge */
		if (ok)
		{
		    int	eid = edge->edgeId;

		    tri->adj[i] = nextTri;
		    nextTri->adj[eid] = tri;
		    edge->adjTri = tri;
#ifdef DECIMATE
		    tri->adjEdges[i] = edge;
#endif

		    /* Now set adjTri in edge 'i' */
		    while(edgei == NULL && (edge = edge->hnext) != NULL)
		    {
		        if(edge->tri == tri && 
			   edge->index0 == index0 && edge->index1 == index1)
			    edgei = edge;
		    }
		    edgei->adjTri = nextTri;
#ifdef DECIMATE
		    nextTri->adjEdges[eid] = edgei;
#endif

		    break;
		}
	    }
	}
    }

#ifdef DECIMATE
    if (Decimate)
    {
	int	numDecimated = 0;

	for (tri = triList->next; tri != (Tri *) triList; tri = tri->next)
	{
	    int		edge;

	    for (edge=0; edge<3; edge++)
	    {
		pfVec3	v0, v1;
		float	diff;
		int	i0, i1, i2, ai0, ai1, ai2, adjEdge;
		Tri	*adjTri;

		adjTri = tri->adj[edge];

		if (!adjTri)
		    continue;

		adjEdge = tri->adjEdges[edge]->edgeId;

		/* Check planarity */
		diff = 1.0f - pfDotVec3(tri->norm, adjTri->norm);
		
		if (PF_ABS(diff) > DecimatePlaneTol)
		    continue;

		i0 = NextEdgeIndex[edge];
		i1 = edge;
		i2 = NextVertIndex[edge];

		ai0 = NextEdgeIndex[adjEdge];
		ai1 = adjEdge;
		ai2 = NextVertIndex[adjEdge];

		if (!tri->adj[i0] && !adjTri->adj[ai2])
		{
		    pfSubVec3(v0, 
			      Coords[tri->vert[i1]],
			      Coords[tri->vert[i0]]);

		    pfSubVec3(v1, 
			      Coords[tri->vert[i1]],
			      Coords[adjTri->vert[ai0]]);

		    pfNormalizeVec3(v0);
		    pfNormalizeVec3(v1);

		    diff = -1.0f - pfDotVec3(v0, v1);

		    if (PF_ABS(diff) < DecimateEdgeTol)
		    {
#ifdef NOISY
			fprintf(stderr, 
				"norms (%.4f %.4f %.4f) "
				"(%.4f %.4f %.4f)\n",
				tri->norm[0], tri->norm[1], tri->norm[2], 
				adjTri->norm[0], adjTri->norm[1], adjTri->norm[2]); 
			fprintf(stderr, 
				"edges (%.4f %.4f %.4f) "
				"(%.4f %.4f %.4f)\n",
				v0[0], v0[1], v0[2], 
				v1[0], v1[1], v1[2]); 
#endif

			tri->vert[i1] = adjTri->vert[ai0];
			tri->adj[i1] = adjTri->adj[ai0];
			tri->edges[i1] = adjTri->edges[ai0];
			tri->edges[i1]->tri = tri;
			tri->edges[i1]->edgeId = i1;
			tri->adjEdges[i1] = adjTri->adjEdges[ai0];
			if (tri->adjEdges[i1])
			{
			    tri->adjEdges[i1]->adjTri = tri;
			    tri->adj[i1]->adj[tri->adjEdges[i1]->edgeId] = tri;
			}

			removeTri(triList, adjTri);
			numDecimated++;
			continue;
		    }
		}
		if (!tri->adj[i2] && !adjTri->adj[ai0])
		{
		    pfSubVec3(v0, 
			      Coords[tri->vert[i2]],
			      Coords[tri->vert[i0]]);

		    pfSubVec3(v1, 
			      Coords[tri->vert[i2]],
			      Coords[adjTri->vert[ai0]]);

		    pfNormalizeVec3(v0);
		    pfNormalizeVec3(v1);

		    diff = -1.0f - pfDotVec3(v0, v1);

		    if (PF_ABS(diff) < DecimateEdgeTol)
		    {

#ifdef NOISY
			fprintf(stderr, 
				"norms (%.4f %.4f %.4f) "
				"(%.4f %.4f %.4f)\n",
				tri->norm[0], tri->norm[1], tri->norm[2], 
				adjTri->norm[0], adjTri->norm[1], adjTri->norm[2]); 
			fprintf(stderr, 
				"edges (%.4f %.4f %.4f) "
				"(%.4f %.4f %.4f)\n",
				v0[0], v0[1], v0[2], 
				v1[0], v1[1], v1[2]); 
#endif

			tri->vert[i2] = adjTri->vert[ai0];
			tri->adj[i1] = adjTri->adj[ai2];	
			tri->edges[i1] = adjTri->edges[ai2];
			tri->edges[i1]->tri = tri;
			tri->edges[i1]->edgeId = i1;
			tri->adjEdges[i1] = adjTri->adjEdges[ai2];
			if (tri->adjEdges[i1])
			{
			    tri->adjEdges[i1]->adjTri = tri;
			    tri->adj[i1]->adj[tri->adjEdges[i1]->edgeId] = tri;
			}

			removeTri(triList, adjTri);
			numDecimated++;
			continue;
		    }
		}
	    }
	}
/*	pfNotify(PFNFY_INFO, PFNFY_PRINT, "Total Tris = %d,"
		 "Decimated Tris = %d, Percentage Decimated = %.2f%%\n",
		 numTris, numDecimated, (float)numDecimated / (float)numTris * 100.0f);
*/
    }
#endif

    hashEdgeEnd();

#ifdef CHECK_CONSISTENCY
{
    /*** Test adjacency pointers for consistency ***/
    for (tri = triList->next; tri != (Tri *) triList; tri = tri->next)
    {
	int	j, k;

	for (i = 0; i < 3; i++)
	{
	    if (nextTri = tri->adj[i])
	    {
		for (j = 0, k = 0; j < 3; j++)
		{
		    if (tri == nextTri->adj[j])
			k += 1;
		}
		if (k != 1)
		{
		    pfNotify(PFNFY_WARN, PFNFY_PRINT, 
			"pfdMeshGSet: %x to %x k = %d", tri, nextTri, k);
		}
	    }
	}
    }
}
#endif

    i = 0;
    /*** Compute adjacency statistics and sort tris by adjacent count ***/
    for (tri = triList->next; tri != (Tri *) triList;)
    {
	count = (tri->adj[0] != 0) + (tri->adj[1] != 0) + (tri->adj[2] != 0);
	tri->adjCount = count;
	nextTri = tri->next;

#ifdef SHOWADJ

	    if (tri->adjCount == 0)
		pfSetVec4(clr, 1.0f, 1.0f, 1.0f, 1.0f);
	    else if (tri->adjCount == 1)
		pfSetVec4(clr, 1.0f, 0.0f, 0.0f, 1.0f);
	    else if (tri->adjCount == 2)
		pfSetVec4(clr, 0.0f, 1.0f, 0.0f, 1.0f);
	    else if (tri->adjCount == 3)
		pfSetVec4(clr, 0.0f, 0.0f, 1.0f, 1.0f);

	    pfScaleVec4(AdjColors[i], (RAND1 + .5f) / 1.5f, clr);

	    i++;

	count = 0;
	tri->adjCount = 0;
#endif

	removeTri(triList, tri);
	insertTri(AdjTriList[count], tri);
	tri = nextTri;
    }
}


static void
computeLineAdjacencies(TriList *triList, int nlines)
{
    Tri   	*tri, *nextTri;
    int   	count, i;
    Tri		**pointHash0, **pointHash1; 

    pointHash0 = (Tri**)alloca(sizeof(Tri*) * nlines * 2);
    pointHash1 = (Tri**)alloca(sizeof(Tri*) * nlines * 2);
    bzero(pointHash0, sizeof(Tri*) * nlines * 2);
    bzero(pointHash1, sizeof(Tri*) * nlines * 2);

#ifdef	VERBOSE
    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT, "computeLineAdjacencies: "
	"computing adjacent lines");
#endif

    for (tri = triList->next; tri != (Tri *) triList; tri = tri->next)
    {
	for (i=0; i<2; i++)
	{
	    int	v;

	    v = tri->vert[i];

	    if (pointHash0[v] == NULL)
		pointHash0[v] = tri;
	    else if (pointHash1[v] == NULL)
	    {
		int		ok = 1;
		float		*t0, *t1;

		nextTri = pointHash0[v];

		/* 
		 * If we require smoothing, break strip if colors or
		 * normals are different across adjacent lines 
	        */
		if (!MixedBindFlag && LocalLighting)
		{
		    if (CBind == PFGS_PER_PRIM)
		    {
			t0 = Colors[Cindex[tri->id]];
			t1 = Colors[Cindex[nextTri->id]];
			if (!PFALMOST_EQUAL_VEC4(t0, t1, SORTOF))
			    ok = 0;
		    }
		    if (NBind == PFGS_PER_PRIM)
		    {
			t0 = Norms[Nindex[tri->id]];
			t1 = Norms[Nindex[nextTri->id]];
			if (!PFALMOST_EQUAL_VEC3(t0, t1, SORTOF))
			    ok = 0;
		    }
		}

		/*
		 * We must compute edge adjacencies differently if one
		 * binding is per-vertex and the other is per-primitive. In
		 * this case, two lines are adjacent only if the
		 * per-primitive attributes match. 
		 */
		switch (MixedBindFlag) {
		case NFLAT:
		    t0 = Norms[Nindex[tri->id]];
		    t1 = Norms[Nindex[nextTri->id]];
		    if (!PFALMOST_EQUAL_VEC3(t0, t1, SORTOF))
			ok = 0;
		    break;
		case CFLAT:
		    t0 = Colors[Cindex[tri->id]];
		    t1 = Colors[Cindex[nextTri->id]];
		    if (!PFALMOST_EQUAL_VEC4(t0, t1, SORTOF))
			ok = 0;
		    break;
		}
		
		/* Lines aren't adjacent if they are colinear */
		if ((tri->vert[0] == nextTri->vert[0] && 
		     tri->vert[1] == nextTri->vert[1]) ||
		    (tri->vert[0] == nextTri->vert[1] && 
		     tri->vert[1] == nextTri->vert[0]))
		    ok = 0;

		/* Found an adjacent point */
		if (ok)
		{
		    pointHash1[v] = tri;

		    tri->adj[i] = nextTri;
		    if (v == nextTri->vert[0])
			nextTri->adj[0] = tri;
		    else
			nextTri->adj[1] = tri;
		}
	    }
	}
    }
#ifdef CHECK_CONSISTENCY
{
    /*** Test adjacency pointers for consistency ***/
    for (tri = triList->next; tri != (Tri *) triList; tri = tri->next)
    {
	int	j, k;

	for (i = 0; i < 2; i++)
	{
	    if (nextTri = tri->adj[i])
	    {
		for (j = 0, k = 0; j < 2; j++)
		{
		    if (tri == nextTri->adj[j])
			k += 1;
		}
		if (k != 1)
		{
		    pfNotify(PFNFY_WARN, PFNFY_PRINT, 
			"pfdMeshGSet: %x to %x k = %d", tri, nextTri, k);
		}
	    }
	}
    }
}
#endif

    i = 0;
    /*** Compute adjacency statistics and sort tris by adjacent count ***/
    for (tri = triList->next; tri != (Tri *) triList;)
    {
	count = (tri->adj[0] != 0) + (tri->adj[1] != 0);
	tri->adjCount = count;
	nextTri = tri->next;

#ifdef SHOWADJ

	    if (tri->adjCount == 0)
		pfSetVec4(clr, 1.0f, 1.0f, 1.0f, 1.0f);
	    else if (tri->adjCount == 1)
		pfSetVec4(clr, 1.0f, 0.0f, 0.0f, 1.0f);
	    else if (tri->adjCount == 2)
		pfSetVec4(clr, 0.0f, 1.0f, 0.0f, 1.0f);

	    pfScaleVec4(AdjColors[i], (RAND1 + .5f) / 1.5f, clr);

	    i++;

	count = 0;
	tri->adjCount = 0;
#endif

	removeTri(triList, tri);
	insertTri(AdjTriList[count], tri);
	tri = nextTri;
    }
}

static void
computeNormal(Tri *tri)
{
    pfVec3	v0, v1;

    pfSubVec3(v0, Coords[tri->vert[1]], Coords[tri->vert[0]]);
    pfSubVec3(v1, Coords[tri->vert[2]], Coords[tri->vert[0]]);

    pfCrossVec3(tri->norm, v0, v1);

    pfNormalizeVec3(tri->norm);

    tri->flags |= NORM_OK;
}

static void
adjustAdjacencies(Tri * tri, int vertsPerPrim)
{
#ifndef __linux__
    register        i, j;
#else
    register        int i, j;
#endif
    Tri            *adjtri;

    tri->used = TRUE;
    for (i = 0; i < vertsPerPrim; i++)
    {
        adjtri = tri->adj[i];
	if (adjtri == NULL)
	    continue;

	removeTri(AdjTriList[adjtri->adjCount], adjtri);
	adjtri->adjCount--;
	insertTri(AdjTriList[adjtri->adjCount], adjtri);

	for (j = 0; j < vertsPerPrim; j++)
	{
	    if (tri == adjtri->adj[j])
	    {
		adjtri->adj[j] = NULL;
		break;
	    }
	}
    }
}

static void
initStrip(void)
{
    NextEdge = -1;
    SwapFlag = 0;
}

static Tri*
chooseSeed(void)
{
    Tri *tri;

    if ((tri = AdjTriList[1]->next) != (Tri *) AdjTriList[1])
        removeTri(AdjTriList[1], tri);
    else if ((tri = AdjTriList[2]->next) != (Tri *) AdjTriList[2])
        removeTri(AdjTriList[2], tri);
    else if ((tri = AdjTriList[3]->next) != (Tri *) AdjTriList[3])
        removeTri(AdjTriList[3], tri);
    else
	tri = NULL;

    return tri;
}

/* Return TRUE if planar quad is concave */

static int
concaveCheck(pfVec3 norm, ushort v0, ushort v1, ushort v2, ushort v3)
{
    pfVec3	e0, e1, c;
    int	sign;

    pfSubVec3(e0, Coords[v1], Coords[v0]);
    pfSubVec3(e1, Coords[v2], Coords[v1]);
    pfCrossVec3(c, e1, e0);
    sign = pfDotVec3(norm, c) > 0;

    pfSubVec3(e0, Coords[v3], Coords[v2]);
    pfCrossVec3(c, e0, e1);
    if (sign != (pfDotVec3(norm, c) > 0))
	return TRUE;

    pfSubVec3(e1, Coords[v0], Coords[v3]);
    pfCrossVec3(c, e1, e0);
    if (sign != (pfDotVec3(norm, c) > 0))
	return TRUE;

    pfSubVec3(e0, Coords[v1], Coords[v0]);
    pfCrossVec3(c, e0, e1);
    if (sign != (pfDotVec3(norm, c) > 0))
	return TRUE;

    return FALSE;
}	


/* compute the first non-zero difference between two vertices in the
   x, y, or z */

static float
deltalength(Tri *tri, int v, int w)
{
    float l;

    if((l=(Coords[tri->vert[v]][0]-Coords[tri->vert[w]][0]))<SORTOF)
        if((l=(Coords[tri->vert[v]][1]-Coords[tri->vert[w]][1]))<SORTOF)
            l=Coords[tri->vert[v]][2]-Coords[tri->vert[w]][2];
    return l;
}

/* every triangle has been converted into indexed geoset triangle. */
static int
coplanar(Tri *tri, Tri *adjtri, int mode, int t)
{
    pfVec3 t0, t1;
    pfVec3 tnorm1, tnorm2;
    float l;

    switch(mode)
    {
	case PFGS_TEXCOORD2:
            l = deltalength(tri, 1, 2);
	    pfSetVec3(t0, 
		texCoords[t][tindex[t][tri->vert[1]]][0]-texCoords[t][tindex[t][tri->vert[2]]][0],
		texCoords[t][tindex[t][tri->vert[1]]][1]-texCoords[t][tindex[t][tri->vert[2]]][1],
		l); 

            l = deltalength(tri, 2, 0);
	    pfSetVec3(t1,
                texCoords[t][tindex[t][tri->vert[2]]][0]-texCoords[t][tindex[t][tri->vert[0]]][0],
                texCoords[t][tindex[t][tri->vert[2]]][1]-texCoords[t][tindex[t][tri->vert[0]]][1],
		l);

	    pfCrossVec3(tnorm1, t0, t1);
	    pfNormalizeVec3(tnorm1);
		
            l = deltalength(adjtri, 1, 2);
	    pfSetVec3(t0, 
                texCoords[t][tindex[t][adjtri->vert[1]]][0]-texCoords[t][tindex[t][adjtri->vert[2]]][0],
                texCoords[t][tindex[t][adjtri->vert[1]]][1]-texCoords[t][tindex[t][adjtri->vert[2]]][1],
		l);

            l = deltalength(adjtri, 2, 0);
            pfSetVec3(t1,
                texCoords[t][tindex[t][adjtri->vert[2]]][0]-texCoords[t][tindex[t][adjtri->vert[0]]][0],
                texCoords[t][tindex[t][adjtri->vert[2]]][1]-texCoords[t][tindex[t][adjtri->vert[0]]][1],
		l);

	    pfCrossVec3(tnorm2, t0, t1);
            pfNormalizeVec3(tnorm2);
   	    
	    if (!PFALMOST_EQUAL_VEC3(tnorm1, tnorm2, SORTOF))
	    	return 0;
	    return 1;
	break;

   	case PFGS_COLOR4:
	    l = deltalength(tri, 1, 2);
	    pfSetVec3(t0,
		Colors[Cindex[tri->vert[1]]][0]-Colors[Cindex[tri->vert[2]]][0],
		Colors[Cindex[tri->vert[1]]][1]-Colors[Cindex[tri->vert[2]]][1],
		l);
	    l = deltalength(tri, 2, 0);
	    pfSetVec3(t1,
		Colors[Cindex[tri->vert[2]]][0]-Colors[Cindex[tri->vert[0]]][0],
		Colors[Cindex[tri->vert[2]]][1]-Colors[Cindex[tri->vert[0]]][1],
		l);
	    pfCrossVec3(tnorm1, t0, t1);
	    pfNormalizeVec3(tnorm1);

	    l = deltalength(adjtri, 1, 2);
            pfSetVec3(t0,
                Colors[Cindex[adjtri->vert[1]]][0]-Colors[Cindex[adjtri->vert[2]]][0],
                Colors[Cindex[adjtri->vert[1]]][1]-Colors[Cindex[adjtri->vert[2]]][1], 
                l);
            l = deltalength(adjtri, 2, 0);
            pfSetVec3(t1,
                Colors[Cindex[adjtri->vert[2]]][0]-Colors[Cindex[adjtri->vert[0]]][0],
                Colors[Cindex[adjtri->vert[2]]][1]-Colors[Cindex[adjtri->vert[0]]][1],
                l);
            pfCrossVec3(tnorm2, t0, t1);  	
	    pfNormalizeVec3(tnorm2);

            if (!PFALMOST_EQUAL_VEC3(tnorm1, tnorm2, SORTOF))
                return 0;

	    l = deltalength(tri, 1, 2);
            pfSetVec3(t0,
                Colors[Cindex[tri->vert[1]]][2]-Colors[Cindex[tri->vert[2]]][2],
                Colors[Cindex[tri->vert[1]]][3]-Colors[Cindex[tri->vert[2]]][3],
                l);
            l = deltalength(tri, 2, 0);
            pfSetVec3(t1,
                Colors[Cindex[tri->vert[2]]][2]-Colors[Cindex[tri->vert[0]]][2],
                Colors[Cindex[tri->vert[2]]][3]-Colors[Cindex[tri->vert[0]]][3],
                l);
            pfCrossVec3(tnorm1, t0, t1);        
	    pfNormalizeVec3(tnorm1);

            l = deltalength(adjtri, 1, 2);      
            pfSetVec3(t0,       
                Colors[Cindex[adjtri->vert[1]]][2]-Colors[Cindex[adjtri->vert[2]]][2],
                Colors[Cindex[adjtri->vert[1]]][3]-Colors[Cindex[adjtri->vert[2]]][3], 
                l);
            l = deltalength(adjtri, 2, 0);
            pfSetVec3(t1,       
                Colors[Cindex[adjtri->vert[2]]][2]-Colors[Cindex[adjtri->vert[0]]][2],
                Colors[Cindex[adjtri->vert[2]]][3]-Colors[Cindex[adjtri->vert[0]]][3],
                l);
            pfCrossVec3(tnorm2, t0, t1);
            pfNormalizeVec3(tnorm2);

            if (!PFALMOST_EQUAL_VEC3(tnorm1, tnorm2, SORTOF))
                return 0;

	    return 1;
	break;
    }
}

static void
retessellate(Tri *tri, int ncom, int numTextures)
{
    Tri            *adjTri;
    int            sharedEdge;
    int		   t;

    sharedEdge = (tri->adj[0]) ? 0 : ((tri->adj[1]) ? 1 : 2);

    if (sharedEdge == NextEdge)
	return;

    adjTri = tri->adj[sharedEdge];

    if (!(tri->flags & NORM_OK))
	computeNormal(tri);
    if (!(adjTri->flags & NORM_OK))
	computeNormal(adjTri);

    /* Make sure two triangles are coplanar */
    if (!PFALMOST_EQUAL_VEC3(tri->norm, adjTri->norm, SORTOF))
	return;

    /* Make sure PER_PRIM colors of the two triangles are the same */
    if (CBind == PFGS_PER_PRIM && 
	!PFALMOST_EQUAL_VEC4(Colors[Cindex[tri->id]], 
			     Colors[Cindex[adjTri->id]], SORTOF))
	return;

    {
	int            i, j;
	ushort          vi, vj;

	/* Retessellate tri and adjTri */
	i = notCommon(tri, adjTri);
	j = notCommon(adjTri, tri);

	vi = tri->vert[i];
	vj = adjTri->vert[j];

#if 0
    	/* Make sure vertex colors are the same */
	if (CBind == PFGS_PER_VERTEX &&
	    (!PFALMOST_EQUAL_VEC4(Colors[Cindex[vi]], 
			         Colors[Cindex[vj]], SORTOF) ||
	     !PFALMOST_EQUAL_VEC4(Colors[Cindex[vi]], 
			         Colors[Cindex[(vj+1) % 3]], SORTOF)))
  	    return;
#else

	/* XXXXX assume color and texcoord are linearly interpolated in
	   hardware. needs to be changed if it is not true any more - zhz */
	if (CBind == PFGS_PER_VERTEX &&
            !coplanar(tri, adjTri, PFGS_COLOR4, 0))
	    return;

	for (t = 0 ; t < numTextures ; t ++)
	    if (tbind[t] == PFGS_PER_VERTEX &&
		!coplanar(tri, adjTri, PFGS_TEXCOORD2, t))
		return;

#if CHECK_NORMAL
 	/* XXX we should generate normal after we tessellate */
	/* Only retessellate if all normals are the same. */
	if (NBind == PFGS_PER_VERTEX &&
	    (!PFALMOST_EQUAL_VEC3(Norms[Nindex[vi]],
				Norms[Nindex[vj]], SORTOF) ||
	     !PFALMOST_EQUAL_VEC3(Norms[Nindex[vi]],
				Norms[Nindex[(vj+1) % 3]], SORTOF) ||
	     !PFALMOST_EQUAL_VEC3(Norms[Nindex[vi]],
				Norms[Nindex[(vj+2) % 3]], SORTOF)))
	    return;
#endif
#endif

    	/* Make sure two triangles form convex quad */
	if (concaveCheck(tri->norm, vi,    tri->vert[NextVertIndex[i]], 
			 	    vj, adjTri->vert[NextVertIndex[j]]))
	    return;

	/* Swap vertices */
	tri->vert[ncom] = vj;

	if (ncom == NextVertIndex[i])
	{
	    adjTri->vert[NextVertIndex[j]] = vi;

	    if (adjTri->adj[j])
	    {
		removeTri(AdjTriList[tri->adjCount], tri);
		tri->adjCount++;
		insertTri(AdjTriList[tri->adjCount], tri);

		removeTri(AdjTriList[adjTri->adjCount], adjTri);
		adjTri->adjCount--;
		insertTri(AdjTriList[adjTri->adjCount], adjTri);

		if (adjTri->adj[j]->adj[0] == adjTri)
		    adjTri->adj[j]->adj[0] = tri;
		else if (adjTri->adj[j]->adj[1] == adjTri)
		    adjTri->adj[j]->adj[1] = tri;
		else if (adjTri->adj[j]->adj[2] == adjTri)
		    adjTri->adj[j]->adj[2] = tri;
	    }
	    /*
	     * Swap adjacencies. 
	     */
	    tri->adj[NextVertIndex[i]] = adjTri->adj[j];
	    adjTri->adj[NextVertIndex[j]] = tri->adj[i];

	    tri->adj[i] = adjTri;
	    adjTri->adj[j] = tri;

	    NextEdge = i;
	} 
	else
	{
	    int    ii, jj;

	    ii = NextVertIndex[NextVertIndex[i]];
	    jj = NextVertIndex[NextVertIndex[j]];

	    adjTri->vert[jj] = vi;

	    if (adjTri->adj[jj])
	    {
		removeTri(AdjTriList[tri->adjCount], tri);
		tri->adjCount++;
		insertTri(AdjTriList[tri->adjCount], tri);

		removeTri(AdjTriList[adjTri->adjCount], adjTri);
		adjTri->adjCount--;
		insertTri(AdjTriList[adjTri->adjCount], adjTri);

		if (adjTri->adj[jj]->adj[0] == adjTri)
		    adjTri->adj[jj]->adj[0] = tri;
		else if (adjTri->adj[jj]->adj[1] == adjTri)
		    adjTri->adj[jj]->adj[1] = tri;
		else if (adjTri->adj[jj]->adj[2] == adjTri)
		    adjTri->adj[jj]->adj[2] = tri;
	    }

	    /*
	     * Swap adjacencies. 
	     */
	    tri->adj[NextVertIndex[i]] = adjTri->adj[jj];
	    adjTri->adj[NextVertIndex[j]] = tri->adj[ii];

	    tri->adj[ii] = adjTri;
	    adjTri->adj[jj] = tri;

	    NextEdge = ii;
	}
    }
}


/* 
 * Return NULL if extending the mesh aint tri's edge 'adjedge' will
 * cause a swap or return the adjacent triangle otherwise.
 * Set NextEdge to the edge of the adjacent triangle which must be 
 * matched in order to extend the strip without swaps. 
 * If NextEdge < 0, we assume a new mesh is being started.
*/
static Tri*
swapCheck(Tri * tri, int adjedge, int numTextures)
{
    int            i;

    if (NextEdge < 0 || adjedge == NextEdge)
    {
	Tri            *adjTri = tri->adj[adjedge];
	/*
	 * Next edge to match is either i -> i+1 or i-1 -> i depending on
	 * swap. 
	 */
	i = notCommon(adjTri, tri);

	/* Toggle swap since we are adding a new vertex to mesh */
	SwapFlag = !SwapFlag;

	if (SwapFlag)
	    NextEdge = i;
	else
	    NextEdge = NextEdgeIndex[i];

	if (Retessellate && adjTri->adjCount == 1)
	    retessellate(adjTri, i, numTextures);

	return adjTri;
    } else			/* Don't Toggle swap since we are not Adding
				 * a new vertex to mesh */
	return NULL;
}

static Tri*
getNextLine(Tri * tri)
{
    switch (tri->adjCount)
    {
    case 0:
	return NULL;
    case 1:
	if (tri->adj[0])
	    return tri->adj[0];
	else if (tri->adj[1])
	    return tri->adj[1];

	return NULL;
    case 2:
	if (tri->adj[0]->adjCount < tri->adj[1]->adjCount)
	    return tri->adj[0];
	else
	    return tri->adj[1];
    }
    return NULL;
}

/* 
 * Return triangle which is adjacent to 'tri' edge NextEdge, 
 * and which has the minimum number of adjacencies. Return
 * NULL if no adjacent triangle can be added to the mesh without a swap.
*/
static Tri*
getNextTri(Tri * tri, int numTextures)
{
    int         min0, min1, adjedge, min, max, i, j;
    int	    	counts[3], adjList[3];
    Tri		*adjTri;

    switch (tri->adjCount)
    {
    case 0:
	return NULL;
    case 1:
	if (tri->adj[0])
	    adjedge = 0;
	else if (tri->adj[1])
	    adjedge = 1;
	else if (tri->adj[2])
	    adjedge = 2;

	return swapCheck(tri, adjedge, numTextures);

    case 2:
	if (tri->adj[0])
	{
	    min0 = 0;
	    min1 = (tri->adj[1] == 0) + 1;
	}
	else
	{
	    min0 = 1;
	    min1 = 2;
	}

	if ((tri->adj[min0])->adjCount < (tri->adj[min1])->adjCount)
	{
	    min = min0;
	    max = min1;
	}
	else if ((tri->adj[min0])->adjCount > (tri->adj[min1])->adjCount)
	{
	    min = min1;
	    max = min0;
	}
	else if (NextEdge < 0)	/* Adjacency counts are equal */
	{
	    static int	exclusive[] = {
		-1, 2, 1, 0
	    };
	    int	notAdj;

	    notAdj = exclusive[min0 + min1];

	    max = NextVertIndex[notAdj];
	    min = NextVertIndex[max];
	}
	else
	{
	    min = min0;
	    max = min1;
	}

	/* Try min triangle first */
        adjTri = swapCheck(tri, min, numTextures);

	if (adjTri == NULL)
	    return swapCheck(tri, max, numTextures);
	else
	    return adjTri;
    case 3:

	/* Bubble-sort adjacency counts */
	counts[0] = tri->adj[0]->adjCount;
	counts[1] = tri->adj[1]->adjCount;
	counts[2] = tri->adj[2]->adjCount;

	adjList[0] = 0; adjList[1] = 1; adjList[2] = 2;

	for(i=0; i<3; i++)
	{
	    for(j=i+1; j<3; j++)
	    {
		if(counts[j] < counts[i])
		{
		    int    tmp = counts[i];
		    counts[i] = counts[j];
		    counts[j] = tmp;

		    tmp = adjList[i];
		    adjList[i] = adjList[j];
		    adjList[j] = tmp;
		}
	    }

	    /* Try min triangle first */
	    adjTri = swapCheck(tri, adjList[i], numTextures);

	    if(adjTri != NULL)
		return adjTri;
	}
	return NULL;	

    default:
	pfNotify(PFNFY_WARN, PFNFY_DEBUG, "getNextTri: should not be here");
	/* NOTREACHED */
	return NULL;
    }
}
