/*
 * Copyright 2000, Silicon Graphics, Inc.
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


#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pfdu.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef WIN32
#include <sys/resource.h>
#endif

#ifdef WIN32
#define READ_MODE "rb"
#define WRITE_MODE "wb"
#else
#define READ_MODE "r"
#define WRITE_MODE "w"
#endif

#define MAX_LODS 100
#define SMALL_EPS 0.00000001

#define MIN_TRIS        120
#define MAX_TRIS 	8000

#define BUILD_PAGING 		0
#define PAGING 			0

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define OTHERFACE(face,edge) (((edge->f1)==(face))?(edge->f2):(edge->f1))

#define NORMAL 0

typedef struct meshEdge {
    int level;
    unsigned int v1, v2;  /* v1 is expected to be the "newest" vertex */
    struct meshEdge *ch1, *ch2;
    struct meshFace *f1, *f2;    /* Adjacency for same LOD only! */
    struct meshEdge *next;
} meshEdge;

typedef struct  meshFace {
    unsigned int id;
    struct meshEdge *ex, *ey, *eh;  /* horizontal, vertical, hypotenuse */
    struct meshFace *ch1, *ch2, *ch3, *ch4;
    struct meshFace *next;
} meshFace;

static int lodLevel;
static meshFace *meshFaces[MAX_LODS];
static meshEdge *meshEdges[MAX_LODS];
static pfVec3 *srcData;
static pfVec3 *Vd;
PFDUDLLEXPORT void pfdBreakTiles(int numlods, pfASDLODRange *lods, int numverts, pfASDVert *verts, int numfaces, pfASDFace *faces, int numfaces0, char *prename, char *conf);

PFDUDLLEXPORT void pfdPagingLookahead(char *confname, char *pagename, int *lookahead);

PFDUDLLEXPORT void pfdWriteFile(int ***FACE, int **numf, int ***V, int **numv, pfASDFace *faces, pfASDVert *verts, pfBox *facebounds, int lod, int x, int y, char *prename);

PFDUDLLEXPORT pfBox * pfdGetTerrainBounds(pfASDFace *faces, pfASDVert *verts, int numfaces0, int numfaces);

PFDUDLLEXPORT pfASD * pfdLoadConfig(char *fname, char *pagename);

void computeBBox(pfBox *facebounds, pfASDFace *faces, pfASDVert *verts, int _faceid);

static int *dvmap;
static int dvcnt;
static int *vmap;
static int vcnt;
pfASDVert *verts;
pfVec3 *dverts;


int 
finalmap(int vid)
{
    if(vmap[vid] == -1)
	vmap[vid] = vcnt++;
    return(vmap[vid]);
}

int 
map(int vid)
{
    if(dvmap[vid] == -1)
	dvmap[vid] = dvcnt++;
    return(dvmap[vid]);
}

/* this is a rough estimate. 
 * the total number of edges in LODi starting with 1 triangle in LOD0 is
 * Edge[i] = 1.5*(2^i + 4^i) with Edge[0] = 3.
 * Each edge corresponds to one vertex.
 * the total number of vertices in LODi is
 * V[i] = 3*2^i + 2*4^i + 1;
 * We will roughly count the total number of verts starting with
 * numfaces0 triangles in LOD0 as
 * V[i] = numface0*(3*2^i + 2*4^i + 1).
 */

int
maxverts(int lods, int numfaces0, int numverts)
{
    int v;

    if(lods == -1)
	return numverts;
    lods = lods - 2;
    v = 3*(1<<lods) + 2*(1<<(2*lods)) + 1;
    return (numfaces0*v);
}

int
mylog(int n)
{
    int i, j;
    
    i = 0;
    j = 1;
    while(j < n)
    {
	j *= 2;
	i++;
    }
    return ++i;
}

int
maxlod(int lods, int numx, int numy)
{
    if(lods != -1)
	return(lods);
    if(mylog(numx) > mylog(numy))
	return(mylog(numy)+1);
    return(mylog(numx)+1);
}

static ComputeNormal(pfVec3 norm, pfVec3 v, pfVec3 v1, pfVec3 v2,
                     pfVec3 v3, pfVec3 v4)
{
    pfVec3 t1, t2, s1, s2;

    PFSUB_VEC3(t1, v1, v);
    PFSUB_VEC3(t2, v2, v);
    pfCrossVec3(s1, t1, t2);
    pfNormalizeVec3(s1);

    PFSUB_VEC3(t1, v2, v);
    PFSUB_VEC3(t2, v3, v);
    pfCrossVec3(s2, t1, t2);
    pfNormalizeVec3(s2);

    if (v4)
    {
        PFADD_VEC3(s1, s1, s2);

        PFSUB_VEC3(t1, v3, v);
        PFSUB_VEC3(t2, v4, v);
        pfCrossVec3(s2, t1, t2);
        pfNormalizeVec3(s2);

        PFADD_VEC3(s1, s1, s2);

        PFSUB_VEC3(t1, v4, v);
        PFSUB_VEC3(t2, v1, v);
        pfCrossVec3(s2, t1, t2);
        pfNormalizeVec3(s2);
    }

    PFADD_VEC3(norm, s1, s2);
    pfNormalizeVec3(norm);
}


static meshEdge *addEdge(unsigned int v1, unsigned int v2)
{
    meshEdge *newEdge;

    newEdge = (meshEdge *) pfMalloc(sizeof(meshEdge), pfGetSharedArena());
    if(newEdge == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
	"can't alloc addEdge v1 %d, v2 %d\n", v1, v2);
	exit(0);
    }
    newEdge->level = lodLevel;
    newEdge->v1 = v1;
    newEdge->v2 = v2;
    newEdge->ch1 = newEdge->ch2 = NULL;
    newEdge->f1 = newEdge->f2 = NULL;
    newEdge->next = meshEdges[lodLevel];
    meshEdges[lodLevel] = newEdge;
    return newEdge;
}


static void addFace(meshEdge *ex, meshEdge *ey, meshEdge *eh, meshFace *parent)
{
    meshFace *newFace;

    newFace = (meshFace *) pfMalloc(sizeof(meshFace), pfGetSharedArena());
    if(newFace == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		"can't alloc addFace\n");
	exit(0);
    }
    newFace->ex = ex;
    newFace->ey = ey;
    newFace->eh = eh;
    newFace->ch1 = newFace->ch2 = newFace->ch3 = newFace->ch4 = NULL;

    if (parent)
    {
        if (!parent->ch1)
            parent->ch1 = newFace;
        else if (!parent->ch2)
            parent->ch2 = newFace;
        else if (!parent->ch3)
            parent->ch3 = newFace;
        else if (!parent->ch4)
            parent->ch4 = newFace;
    }

    if (ex->level == lodLevel)
	if (ex->f1)
	    ex->f2 = newFace;
        else
	    ex->f1 = newFace;

    if (ey->level == lodLevel)
	if (ey->f1)
	    ey->f2 = newFace;
        else
	    ey->f1 = newFace;

    if (eh->level == lodLevel)
	if (eh->f1)
	    eh->f2 = newFace;
        else
	    eh->f1 = newFace;

    newFace->next = meshFaces[lodLevel];
    meshFaces[lodLevel] = newFace;
}


static void SplitVertex(unsigned int refvertv, unsigned int v1, unsigned int v2)
{
    pfVec3 vect, diff, newpos;

    PFSUB_VEC3(vect, srcData[v2], srcData[v1]);
    PFSUB_VEC3(diff, srcData[refvertv], srcData[v1]);
    PFADD_SCALED_VEC3(newpos, srcData[v1], PFDOT_VEC3(vect, diff) /
                      PFDOT_VEC3(vect, vect), vect);

    PFSUB_VEC3(dverts[map(refvertv)], newpos, srcData[refvertv]);
}

static void     prune_faces(pfASDVert *v, pfASDFace *faces, int numfaces0);
static void     prune_subtree(pfASDVert *v, pfASDFace *faces, int faceId);
static int 	traverse_subtree(
				pfASDVert 	*v, 
				pfASDFace 	*faces,
				int 		faceId, 
				float 		a, 
				float 		b, 
				float 		c, 
				float 		d, 
				float 		eps);


/*=========================================================================*/
static void     prune_faces(pfASDVert *v, pfASDFace *faces, int numfaces0)
/*=========================================================================*/
{
    int	i;

    for (i = 0 ; i < numfaces0 ; i ++ )
	prune_subtree (v, faces, i);
}

/*=========================================================================*/
static void     prune_subtree(pfASDVert *v, pfASDFace *faces, int faceId)
/*=========================================================================*/
{
    pfASDFace		*face;
    pfVec3		v0, v1, v2;
    pfVec3		d0, d1, n;
    float		a, b, c, d;
    int			i;
    float		area;
    int			cnt, children;

    face = &faces[faceId];

    pfCopyVec3 (v0, v[face -> vert[0]].v0);
    pfCopyVec3 (v1, v[face -> vert[1]].v0);
    pfCopyVec3 (v2, v[face -> vert[2]].v0);

    pfSubVec3 (d0, v1, v0);
    pfSubVec3 (d1, v2, v0);
    pfCrossVec3 (n, d0, d1);

    area = pfNormalizeVec3 (n);

    /* 
     *	Face plane equation. 
     */
    a = n[0];
    b = n[1];
    c = n[2];
    d = - (a * v0[0] + b * v0[1] + c * v0[2]);

    traverse_subtree (v, faces, faceId, a, b, c, d, sqrt(area) * 0.001);
}

/*=========================================================================*/
static int traverse_subtree (pfASDVert *v, pfASDFace *faces, int faceId, 
			float _a, float _b, float _c, float _d, float _eps)
/*=========================================================================*/
{
    pfASDFace		*face;
    float		x, y, z;
    int			result;
    int			is_children;
    pfVec3		v0, v1, v2;
    pfVec3		d0, d1, n;
    float		a, b, c, d;
    int			i;
    float		area;

    face = &faces[faceId];
    is_children = 0;

    /*
     *	=============================================
     *	Test parent face against this one. 
     *	=============================================
     */
    result = 1;
    for (i = 0 ; i < 3 ; i ++)
    {
	if (face -> refvert[i] != PFASD_NIL_ID)
	{
	    x = v[face -> refvert[i]].v0[0];
	    y = v[face -> refvert[i]].v0[1];
	    z = v[face -> refvert[i]].v0[2];

	    d = x * _a + y * _b + z * _c + _d;
	    if (d > _eps)
		result = 0;
	}
    }

    /*
     *	=============================================
     *	Prepare for children
     *	=============================================
     */
    pfCopyVec3 (v0, v[face -> vert[0]].v0);
    pfCopyVec3 (v1, v[face -> vert[1]].v0);
    pfCopyVec3 (v2, v[face -> vert[2]].v0);

    pfSubVec3 (d0, v1, v0);
    pfSubVec3 (d1, v2, v0);
    pfCrossVec3 (n, d0, d1);

    area = pfNormalizeVec3 (n);

    /* 
     *	Face plane equation. 
     */
    a = n[0];
    b = n[1];
    c = n[2];
    d = - (a * v0[0] + b * v0[1] + c * v0[2]);

    for (i = 0 ; i < 4 ; i ++)
    {
	if (face -> child[i] != PFASD_NIL_ID)
	{
	    is_children = 1;
	    if (traverse_subtree (v, faces, face -> child[i], 
					a, b, c, d, fsqrt(area) * 0.001) == 0)
		result = 0;
        }
    }

    /*
     *	===========================================
     *	Can we prune here ? 
     *	===========================================
     */
    if ((result == 1) && is_children)
    {
        /*
	 *	Zero out all children. 	
	 */
	for (i = 0 ; i < 4 ; i ++)
	    face -> child[i] = PFASD_NIL_ID;

#if 0
	for (i = 0 ; i < 3 ; i ++)
	    face -> refvert[i] = PFASD_NIL_ID;
#endif
    }

    return (result);
}


PFDUDLLEXPORT pfASD *
pfdBuildASD(unsigned int numx, unsigned int numy, pfVec3 *data, int mode, int inputlods, int buildpaging, char *prename, char *confname, char *pagename, int *lookahead)
{
    int i, j;
    float aspectRatio;
    int numRects;
    int rectWidth;
    meshEdge *edge1, *edge2, *theEdge;
    int edgeSplit;
    unsigned int inc;
    meshEdge *edgeList;
    meshFace *faceList;
    meshEdge *ep, *tep;
    meshFace *fp, *tfp;
    unsigned int dist;
    unsigned int refvertvertex;
    meshEdge *xEdge, *yEdge;
    unsigned int vert0, vert1, vert2;
    unsigned int refvert0, refvert1, refvert2;
    pfVec3 cp0, cp1, cp2;
    unsigned int tempv;
    float switchdist;
    int tsid=0;
    pfASD *asd;
    pfGeoState *gstate;
    pfGeoState **gstates;
    pfMaterial *material;
    pfASDLODRange *lods;
    pfASDFace *faces;
    pfVec3 *v0, *vd, *n0, *nd;
    int numverts, numdata, numnormals, numlods, numfaces, numfaces0;
    unsigned int num_singles=0;
    int tsidlevels;

    float *attrs;
    int stride;

    int nf, tmp_numfaces0, tmp_lods;;
    
    if ((numx < 2) || (numy < 2))
    {
        pfNotify(PFNFY_WARN, PFNFY_ASSERT,
                 "pfdBuildASD unsuccessful; bad data dimensions.");
        return(NULL);
    }

    for (i=0; i<MAX_LODS; ++i)  {
	meshFaces[i] = NULL;
	meshEdges[i] = NULL;
    }

    if(getenv("PFASDLODSIZE")!=NULL)
    {
	inputlods = atoi(getenv("PFASDLODSIZE"));
	pfNotify(PFNFY_INFO, PFNFY_PRINT,
                     "PFASDLODSIZE overriding size to %d\n",
			inputlods);
    }

        /* Total number of posts */
    numdata = numx * numy;

    srcData = data;

    lodLevel = 0;
    
        /****************************************************
        * Construct temporary edges and faces for all lods
        ****************************************************/

    /*
     *  Split the initial data block rectangle up into
     *  rectangles with aspect ratios close to 1.  These 
     *  near-square rectangles are then each divided into
     *  two triangles, and all the triangles will collectively
     *  serve as the lowest level of detail (level 0).
     */

    aspectRatio = (float) (numx-1) / (numy-1);

    if (aspectRatio < 1.0)
    {
        numRects = (int) ((1.0 / aspectRatio) + 0.5);
        rectWidth = numy / numRects;

	theEdge = addEdge(0, numx-1);

        for (i=0, inc=0; i<numRects-1; ++i, inc+=rectWidth)  {
	   edge1 = addEdge((inc+1)*numx - 1, (inc+rectWidth+1)*numx - 1);
	   edge2 = addEdge(inc*numx, (inc+rectWidth+1)*numx - 1);
	   addFace(theEdge, edge1, edge2, NULL);
	   theEdge = addEdge((inc+rectWidth)*numx, (inc+rectWidth+1)*numx - 1);
	   edge1 = addEdge(inc*numx, (inc+rectWidth)*numx);
	   addFace(theEdge, edge1, edge2, NULL);
	}
	edge1 = addEdge((inc+1)*numx - 1, numx*numy - 1);
	edge2 = addEdge(inc*numx, numx*numy - 1);
	addFace(theEdge, edge1, edge2, NULL);
	theEdge = addEdge(numx*(numy-1), numx*numy - 1);
	edge1 = addEdge(inc*numx, numx*(numy-1));
	addFace(theEdge, edge1, edge2, NULL);
    }
    else
    {
        numRects = (int) (aspectRatio + 0.5);
        rectWidth = numx / numRects;

        theEdge = addEdge(0, numx*(numy-1));

        for (i=0, inc=0; i<numRects-1; ++i, inc+=rectWidth)  {
	   edge1 = addEdge((numy-1)*numx+inc, (numy-1)*numx+inc+rectWidth);
	   edge2 = addEdge(inc, (numy-1)*numx + inc + rectWidth); 
	   addFace(edge1, theEdge, edge2, NULL);
	   edge1 = addEdge(inc, inc+rectWidth);
	   theEdge = addEdge(inc+rectWidth, (numy-1)*numx+inc+rectWidth);
	   addFace(edge1, theEdge, edge2, NULL);
	}
	edge1 = addEdge((numy-1)*numx + inc, numx*numy -1);
	edge2 = addEdge(inc, numx*numy - 1);
	addFace(edge1, theEdge, edge2, NULL);
	edge1 = addEdge(inc, numx-1);
	theEdge = addEdge(numx-1, numx*numy - 1);
	addFace(edge1, theEdge, edge2, NULL);
    }

   	tmp_numfaces0 = 2*(numRects+1);
        tmp_lods = maxlod(inputlods, numx, numy);
        nf = tmp_numfaces0*(1<<(tmp_numfaces0*tmp_lods -2));
        pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
                "pfdBuildASD(): need about %d KBytes of memory to store all points\n"
                "and %d KBytes of memory to store all faces,\n"
                "and %dK bytes tmp space\n",
                (sizeof(pfASDVert)*maxverts(tmp_lods, tmp_numfaces0, numdata))/(1<<10),
                (nf*sizeof(pfASDFace))/(1<<10),
                (sizeof(int)*numdata+sizeof(meshFace)*nf+sizeof(meshEdge)*nf)/(1<<10));

        vmap = (int *) pfMalloc(sizeof(int)*numdata, pfGetSharedArena());
        dvmap = (int *) pfMalloc(sizeof(int)*numdata, pfGetSharedArena());
        for(i = 0; i < numdata; i++)
	{
                vmap[i] = -1;
                dvmap[i] = -1;
	}

        vcnt = dvcnt = 0;
        numverts = maxverts(tmp_lods, tmp_numfaces0, numdata);
/* this is the array of generated vertices */
        verts = (pfASDVert *) pfCalloc(
                numverts,
                sizeof(pfASDVert),
                pfGetSharedArena());

/* this is the temp array to hold delta values */
        dverts = (pfVec3 *) pfCalloc(
                numverts,
                sizeof(pfVec3),
                pfGetSharedArena());

    /* level 0 is in the Edge/Face structures now */
printf("level 0 is in the Edge/Face structures now\n");

        /* Now, go do all other lodLevels */
    while ((inputlods == -1)||(lodLevel < inputlods-1))
    {
	edgeSplit = FALSE;
	++lodLevel;
	edgeList = meshEdges[lodLevel-1];
	faceList = meshFaces[lodLevel-1];

    /*
     *  Go to each "straight" edge in the edge list and
     *  refvert it if possible.
     */

printf("process LOD level %d.\n", lodLevel);
                /**************************************************************
                * Split all edges at lodLevel-1 and create edge for lodLevel
                **************************************************************/
    for (ep = edgeList; ep != NULL; ep = ep->next)
    {
        if ((ep->v1 / numx) == (ep->v2 / numx))
	{
	    dist = abs((ep->v1 % numx) - (ep->v2 % numx));
	    if (dist > 1)  {
	    /*******************************************************
            * refvert in the middle and add the new edges as children
             *******************************************************/

	        refvertvertex = MIN(ep->v1, ep->v2) + (dist / 2);
		SplitVertex(refvertvertex, ep->v1, ep->v2);
	        ep->ch1 = addEdge(refvertvertex, ep->v1);
	        ep->ch2 = addEdge(refvertvertex, ep->v2);
		edgeSplit = TRUE;
	    }
        }
	else if ((ep->v1 % numx) == (ep->v2 % numx))
	{
	    dist = abs((ep->v1 / numx) - (ep->v2 / numx));
	    if (dist > 1)  {
		/*******************************************************
                * refvert in the middle and add the new edges as children
                 *******************************************************/
	        refvertvertex = MIN(ep->v1, ep->v2) + numx * (dist / 2);
		SplitVertex(refvertvertex, ep->v1, ep->v2);
	        ep->ch1 = addEdge(refvertvertex, ep->v1);
	        ep->ch2 = addEdge(refvertvertex, ep->v2);
		edgeSplit = TRUE;
            }
	}
    }

    /* If no edges were refverttable, we can quit the loop now. */

    if (!edgeSplit)
	break;

                /*
                * for face list in lodLevel-1, construct new faces for lodLevel
                * add new faces as children to lodLevel-1 faces
                */
    /*
     *  Now visit each face, refvertting "non-straight" edges
     *  only if both of the other two edges (presumably
     *  straight) have been refvert.
     */

    for (fp = faceList; fp != NULL; fp = fp->next)
    {
        if (fp->ex->ch1 && fp->ey->ch1 && !(fp->eh->ch1))  {
            refvertvertex = (fp->ex->ch1->v1 % numx) +
                          (fp->ey->ch1->v1 / numx)*numx;
            SplitVertex(refvertvertex, fp->eh->v1, fp->eh->v2);
	    fp->eh->ch1 = addEdge(refvertvertex, fp->eh->v1);
            fp->eh->ch2 = addEdge(refvertvertex, fp->eh->v2);
        }
    }

    /*
     *  Now visit each face, and derive the new child faces (if any).
     *  Each face should either have 0, 1 or 3 refvert vertices,
     *  and faces with 1 refvert vertex will never have this
     *  vertex on the hypotenuse.
     */

    for (fp = faceList; fp != NULL; fp = fp->next)
    {
	if (fp->ex->ch1 && fp->ey->ch1 && fp->eh->ch1)  {

	    edge1 = addEdge(fp->eh->ch1->v1, fp->ex->ch1->v1);
	    edge2 = addEdge(fp->eh->ch1->v1, fp->ey->ch1->v1);
	    theEdge = addEdge(fp->ex->ch1->v1, fp->ey->ch1->v1);
	    addFace(edge2, edge1, theEdge, fp);

	    tempv = fp->eh->ch1->v2;
	    if (fp->ex->ch1->v2 == tempv)  {
	        addFace(fp->ex->ch1, edge1, fp->eh->ch1, fp);
		xEdge = fp->ex->ch2;
	    }
	    else if (fp->ex->ch2->v2 == tempv)  {
		addFace(fp->ex->ch2, edge1, fp->eh->ch1, fp);
		xEdge = fp->ex->ch1;
	    }
	    else if (fp->ey->ch1->v2 == tempv)  {
		addFace(edge2, fp->ey->ch1, fp->eh->ch1, fp);
		yEdge = fp->ey->ch2;
	    }
	    else  {    /* assuming fp->ey->ch2->v2 == tempv */
		addFace(edge2, fp->ey->ch2, fp->eh->ch1, fp);
		yEdge = fp->ey->ch1;
	    } 

	    tempv = fp->eh->ch2->v2;
	    if (fp->ex->ch1->v2 == tempv)  {
	        addFace(fp->ex->ch1, edge1, fp->eh->ch2, fp);
		xEdge = fp->ex->ch2;
	    }
	    else if (fp->ex->ch2->v2 == tempv)  {
		addFace(fp->ex->ch2, edge1, fp->eh->ch2, fp);
		xEdge = fp->ex->ch1;
	    }
	    else if (fp->ey->ch1->v2 == tempv)  {
		addFace(edge2, fp->ey->ch1, fp->eh->ch2, fp);
		yEdge = fp->ey->ch2;
	    }
	    else  {    /* assuming fp->ey->ch2->v2 == tempv */
		addFace(edge2, fp->ey->ch2, fp->eh->ch2, fp);
		yEdge = fp->ey->ch1;
	    } 

	    addFace(xEdge, yEdge, theEdge, fp);
	}

	else if (fp->ex->ch1)
	{
	    if ((fp->ey->v1 == fp->eh->v1) || (fp->ey->v1 == fp->eh->v2))  {
                theEdge = addEdge(fp->ex->ch1->v1, fp->ey->v1);
		if (fp->ex->ch1->v2 == fp->ey->v2)  {
		    addFace(fp->ex->ch1, fp->ey, theEdge, fp);
		    addFace(fp->ex->ch2, theEdge, fp->eh, fp);
		}
		else  {
		    addFace(fp->ex->ch2, fp->ey, theEdge, fp);
		    addFace(fp->ex->ch1, theEdge, fp->eh, fp);
		}
            }
            else
	    {
                theEdge = addEdge(fp->ex->ch1->v1, fp->ey->v2);
		if (fp->ex->ch1->v2 == fp->ey->v1)  {
		    addFace(fp->ex->ch1, fp->ey, theEdge, fp);
		    addFace(fp->ex->ch2, theEdge, fp->eh, fp);
		}
		else  {
		    addFace(fp->ex->ch2, fp->ey, theEdge, fp);
		    addFace(fp->ex->ch1, theEdge, fp->eh, fp);
		}
	    }

            ++num_singles;
	}

	else if (fp->ey->ch1)
	{
	    if ((fp->ex->v1 == fp->eh->v1) || (fp->ex->v1 == fp->eh->v2))  {
                theEdge = addEdge(fp->ey->ch1->v1, fp->ex->v1);
		if (fp->ey->ch1->v2 == fp->ex->v2)  {
		    addFace(fp->ex, fp->ey->ch1, theEdge, fp);
		    addFace(theEdge, fp->ey->ch2, fp->eh, fp);
		}
		else  {
		    addFace(fp->ex, fp->ey->ch2, theEdge, fp);
		    addFace(theEdge, fp->ey->ch1, fp->eh, fp);
		}
            }
            else
	    {
                theEdge = addEdge(fp->ey->ch1->v1, fp->ex->v2);
		if (fp->ey->ch1->v2 == fp->ex->v1)  {
		    addFace(fp->ex, fp->ey->ch1, theEdge, fp);
		    addFace(theEdge, fp->ey->ch2, fp->eh, fp);
		}
		else  {
		    addFace(fp->ex, fp->ey->ch2, theEdge, fp);
		    addFace(theEdge, fp->ey->ch1, fp->eh, fp);
		}
	    }

            ++num_singles;
	}
    }
    }

        /* Done with all edges and faces */
    numlods = lodLevel+1;

        /* Count numfaces for lod 0 */
    for (fp=meshFaces[0], numfaces0=0; fp != NULL; fp = fp->next)
       ++numfaces0;

        /* Count numfaces for all lods */
    for (i=0, numfaces=0; i<numlods; ++i)
        for (fp=meshFaces[i]; fp != NULL; fp=fp->next, ++numfaces)
	   fp->id = numfaces;

        /**************************************
        * Now, compute normals for vertices
        **************************************/
#if NORMAL
    numnormals = numdata;

    n0 = (pfVec3 *) pfMalloc(sizeof(pfVec3) * numnormals, pfGetSharedArena());
    nd = (pfVec3 *) pfMalloc(sizeof(pfVec3) * numnormals, pfGetSharedArena());

    ComputeNormal(n0[0], data[0], data[1], data[numx+1], data[numx], NULL);
    ComputeNormal(n0[numx*(numy-1)], data[numx*(numy-1)], data[numx*(numy-2)],
                  data[numx*(numy-2)+1], data[numx*(numy-1)+1], NULL);
    ComputeNormal(n0[numx*numy-1], data[numx*numy-1], data[numx*numy-2],
                  data[numx*(numy-1)-2], data[numx*(numy-1)-1], NULL);
    ComputeNormal(n0[numx-1], data[numx-1], data[2*numx-1], data[2*numx-2],
                  data[numx-2], NULL);

    for (i=1; i<numy-1; ++i)
    {
        ComputeNormal(n0[i*numx], data[i*numx], data[(i-1)*numx],
                      data[i*numx+1], data[(i+1)*numx], NULL);
        ComputeNormal(n0[(i+1)*numx-1], data[(i+1)*numx-1],
                      data[(i+2)*numx-1], data[(i+1)*numx-2],
                      data[i*numx-1], NULL);
    }

    for (i=1; i<numx-1; ++i)
    {
        ComputeNormal(n0[i], data[i], data[i+1], data[i+numx], data[i-1],
                      NULL);
        ComputeNormal(n0[numx*(numy-1)+i], data[numx*(numy-1)+i],
                      data[numx*(numy-1)+i-1], data[numx*(numy-2)+i],
                      data[numx*(numy-1)+i+1], NULL);
    }

    for (i=1; i<numx-1; ++i)
        for (j=1; j<numy-1; ++j)
            ComputeNormal(n0[numx*j+i], data[numx*j+i],
                          data[numx*(j+1)+i], data[numx*j+i-1],
                          data[numx*(j-1)+i], data[numx*j+i+1]);

    for (i=0; i<numnormals; ++i)
        PFSET_VEC3(nd[i], 0.0f, 0.0f, 0.0f);

#endif

        /*******************************************************************
        * Now, construct the pfASD faces by packaging the temporary faces
        * constructed above into pfASDFace structures.
        *******************************************************************/
    faces = (pfASDFace *) pfMalloc(sizeof(pfASDFace) * numfaces,
                                 pfGetSharedArena());

        /****************************************************************
        * Create normal, color and texture coord attributes if desired
        ****************************************************************/

#if NORMAL
    stride = 3;   /* normal is pfVec3 */
    attrs = (float *)pfMalloc(numnormals*sizeof(float)*stride*2, pfGetSharedArena());
#endif

    for (i=0; i<numlods; ++i)
    {
	pfNotify(PFNFY_INFO, PFNFY_PRINT,
		"assigning lod %d data\n", i);
	for (fp=meshFaces[i]; fp != NULL; fp=fp->next)
	{
   	    int xx;

	    faces[fp->id].level = i;
	    faces[fp->id].gstateid = 0;
	    faces[fp->id].mask = 0;

            /*
	     *  Assign face vertices and refvert vertices such
	     *  that points are arranged counter-clockwise,
	     *  that appropriate tstrip order is maintained,
	     *  and that refvert points correspond to vertex order.
	     */

            if (MAX(fp->ex->v1, fp->ex->v2) == MAX(fp->eh->v1, fp->eh->v2))
	    {
		/* triangle is the "upper" of a pair */

                vert0 = MIN(fp->ex->v1, fp->ex->v2);
		vert1 = MIN(fp->eh->v1, fp->eh->v2);
		vert2 = MAX(fp->eh->v1, fp->eh->v2);
		refvert0 = fp->ey->ch1 ? fp->ey->ch1->v1 : PFASD_NIL_ID;
		refvert1 = fp->eh->ch1 ? fp->eh->ch1->v1 : PFASD_NIL_ID;
		refvert2 = fp->ex->ch1 ? fp->ex->ch1->v1 : PFASD_NIL_ID;
	    }
	    else    /* triangle is the "lower" of a pair */
	    {
                vert0 = MAX(fp->ey->v1, fp->ey->v2);
		vert1 = MIN(fp->eh->v1, fp->eh->v2);
		vert2 = MIN(fp->ey->v1, fp->ey->v2);
		refvert0 = fp->eh->ch1 ? fp->eh->ch1->v1 : PFASD_NIL_ID;
		refvert1 = fp->ex->ch1 ? fp->ex->ch1->v1 : PFASD_NIL_ID;
		refvert2 = fp->ey->ch1 ? fp->ey->ch1->v1 : PFASD_NIL_ID;
	    }

	    xx = finalmap(vert0);
	    PFCOPY_VEC3(verts[xx].v0, data[vert0]);
	    PFCOPY_VEC3(verts[xx].vd, dverts[map(vert0)]);
	    faces[fp->id].vert[0] = xx;
	    xx = finalmap(vert1);
	    PFCOPY_VEC3(verts[xx].v0, data[vert1]);
	    PFCOPY_VEC3(verts[xx].vd, dverts[map(vert1)]);
	    faces[fp->id].vert[1] = xx;
	    xx = finalmap(vert2);
	    PFCOPY_VEC3(verts[xx].v0, data[vert2]);
	    PFCOPY_VEC3(verts[xx].vd, dverts[map(vert2)]);
	    faces[fp->id].vert[2] = xx;

#if NORMAL
	    attrs[2*finalmap(vert0)*stride+0] = n0[vert0][0];
	    attrs[2*finalmap(vert0)*stride+1] = n0[vert0][1];
	    attrs[2*finalmap(vert0)*stride+2] = n0[vert0][2];
            attrs[(2*finalmap(vert0)+1)*stride+0] = nd[vert0][0];
            attrs[(2*finalmap(vert0)+1)*stride+1] = nd[vert0][1];
            attrs[(2*finalmap(vert0)+1)*stride+2] = nd[vert0][2];

	    attrs[2*finalmap(vert1)*stride+0] = n0[vert1][0];
	    attrs[2*finalmap(vert1)*stride+1] = n0[vert1][1];
	    attrs[2*finalmap(vert1)*stride+2] = n0[vert1][2];
            attrs[(2*finalmap(vert1)+1)*stride+0] = nd[vert1][0];
            attrs[(2*finalmap(vert1)+1)*stride+1] = nd[vert1][1];
            attrs[(2*finalmap(vert1)+1)*stride+2] = nd[vert1][2];

	    attrs[2*finalmap(vert2)*stride+0] = n0[vert2][0];
	    attrs[2*finalmap(vert2)*stride+1] = n0[vert2][1];
	    attrs[2*finalmap(vert2)*stride+2] = n0[vert2][2];
            attrs[(2*finalmap(vert2)+1)*stride+0] = nd[vert2][0];
            attrs[(2*finalmap(vert2)+1)*stride+1] = nd[vert2][1];
            attrs[(2*finalmap(vert2)+1)*stride+2] = nd[vert2][2];
#endif

	    if(refvert0 != PFASD_NIL_ID)
	    {
	        PFCOPY_VEC3(verts[finalmap(refvert0)].v0, data[refvert0]);
	        PFCOPY_VEC3(verts[finalmap(refvert0)].vd, dverts[map(refvert0)]);
		faces[fp->id].refvert[0] = finalmap(refvert0);
	    }
	    else
		faces[fp->id].refvert[0] = refvert0;
	
	    if(refvert1 != PFASD_NIL_ID)
	    {
	    	PFCOPY_VEC3(verts[finalmap(refvert1)].v0, data[refvert1]);
	        PFCOPY_VEC3(verts[finalmap(refvert1)].vd, dverts[map(refvert1)]);
		faces[fp->id].refvert[1] = finalmap(refvert1);
            }
            else
                faces[fp->id].refvert[1] = refvert1;

	    if(refvert2 != PFASD_NIL_ID)
	    {
	    	PFCOPY_VEC3(verts[finalmap(refvert2)].v0, data[refvert2]);
	        PFCOPY_VEC3(verts[finalmap(refvert2)].vd, dverts[map(refvert2)]);
		faces[fp->id].refvert[2] = finalmap(refvert2);
            }
            else
                faces[fp->id].refvert[2] = refvert2;

            faces[fp->id].attr[0] = faces[fp->id].vert[0];
            faces[fp->id].attr[1] = faces[fp->id].vert[1];
            faces[fp->id].attr[2] = faces[fp->id].vert[2];
            faces[fp->id].sattr[0] = faces[fp->id].refvert[0];
            faces[fp->id].sattr[1] = faces[fp->id].refvert[1];
            faces[fp->id].sattr[2] = faces[fp->id].refvert[2];

	    faces[fp->id].child[0] = fp->ch1 ? fp->ch1->id : PFASD_NIL_ID;
	    faces[fp->id].child[1] = fp->ch2 ? fp->ch2->id : PFASD_NIL_ID;
	    faces[fp->id].child[2] = fp->ch3 ? fp->ch3->id : PFASD_NIL_ID;
	    faces[fp->id].child[3] = fp->ch4 ? fp->ch4->id : PFASD_NIL_ID;

#ifdef PRINTVERTS
	    for(xx = 0; xx < 3; xx++)
printf("face[%d].vert[%d], v[%d] %f %f %f, %f %f %f\n", 
	fp->id, xx, faces[fp->id].vert[xx], 
	verts[faces[fp->id].vert[xx]].v0[0],
	verts[faces[fp->id].vert[xx]].v0[1], 
	verts[faces[fp->id].vert[xx]].v0[2],
	verts[faces[fp->id].vert[xx]].vd[0],
	verts[faces[fp->id].vert[xx]].vd[1], 
	verts[faces[fp->id].vert[xx]].vd[2]);
#endif
	}
    }


    /*
     *   Attempt to compute somewhat reasonable default switchin/morph
     *   ranges for each level of detail.
     */

printf("assigning LODRanges\n");
    lods = (pfASDLODRange *) pfMalloc(sizeof(pfASDLODRange) * numlods,
                                    pfGetSharedArena());

    switchdist = PFDISTANCE_PT3(data[0], data[numx*numy-1]) * 50.0;
printf("switchdist %f\n", switchdist);
    for (i=0; i<numlods; ++i, switchdist/=2.0)  {
       lods[i].switchin = switchdist;
       lods[i].morph = lods[i].switchin * 0.25;
    }
    lods[0].morph = 0.0;

    /*
     *   Assign triangle strip IDs.  Each row of triangles within
     *   each level of detail will belong to the same tristrip.
     *   The exception is the finest level of detail of blocks with
     *   non-"n^2 + 1" dimensions, where irregular subdivisions may
     *   have occurred.
     */

printf("assigning triangle tsid\n");
        /***************************************************************
        * The finest level may not be a full level because of possible
        * "single refverts".  Then, output single triangle tstrips only.
        ***************************************************************/

    if (num_singles)
    {
        tsidlevels = numlods-1;
        for (fp=meshFaces[numlods-1]; fp != NULL; fp=fp->next)
            faces[fp->id].tsid = 0;
        tsid += 2;
        
    }
    else
        tsidlevels = numlods;
        
        /*********************************************************************
        * All other levels make up a full tree.  Therefore, tmesh all levels.
        *********************************************************************/

    for (i=0; i < tsidlevels; ++i)  {

        for (tfp=meshFaces[i]; tfp != NULL; tfp=tfp->next)
	    if (tfp->ey->v2 == 0 || tfp->ey->v1 == 0)
		break;

        fp = tfp;

	while (fp)
	{
            while (fp)
	    {
                faces[fp->id].tsid = tsid;  ++tsid;
	        fp = OTHERFACE(fp, fp->eh);
                faces[fp->id].tsid = tsid;  ++tsid;
	        fp = OTHERFACE(fp, fp->ey);
            }
	    tsid += 2;

	    tfp = OTHERFACE(tfp, tfp->ex);
	    if (tfp)
		tfp = OTHERFACE(tfp, tfp->eh);
            fp = tfp;
        }
    }


        /************************************************************
        * Finally, free up the temporary mesh edge/face structures
        ************************************************************/

printf("free memory...\n");
    /* Finally, free up the temporary mesh edge/face structures */

    for (i=0; i<numlods; ++i)  {
	for (ep=meshEdges[i]; ep != NULL;)  {
	    tep = ep->next;
	    pfFree(ep);
	    ep = tep;
        }
	for (fp=meshFaces[i]; fp != NULL;)  {
	    tfp = fp->next;
	    pfFree(fp);
	    fp = tfp;
        }
    }


#if NORMAL
    pfFree(n0);
    pfFree(nd);
#endif

    pfFree(vmap);
    pfFree(dvmap);
    pfFree(dverts);


/* we will get rid off flat faces */

    prune_faces(verts, faces, numfaces0);

    if(buildpaging)
    {
	printf("break data into paging tiles\n");
        pfdBreakTiles(numlods, lods, numverts, verts, numfaces, faces, numfaces0, prename, confname);
        pfdPagingLookahead(confname, pagename, lookahead);
        return NULL;
    }

printf("create ASD node\n");
    asd = pfNewASD();
    pfASDAttr(asd, PFASD_LODS, 0, numlods, lods);
    pfASDAttr(asd, PFASD_COORDS, 0, vcnt, verts);

#ifdef PRINTVERTS
    {
	int xx;
	for(xx = 0; xx < 450; xx++)
	    printf("verts[%d], %f %f %f\n", xx, verts[xx].v0[0],
		verts[xx].v0[1], verts[xx].v0[2]);
    }
#endif

    pfASDAttr(asd, PFASD_FACES, 0, numfaces, faces);
    pfASDNumBaseFaces(asd, numfaces0);
printf("total %d verts, %d faces\n", vcnt, numfaces);
#if NORMAL
    pfASDAttr(asd, PFASD_PER_VERTEX_ATTR, PFASD_NORMALS, numnormals, attrs);
    pfASDMorphAttrs(asd, PFASD_NORMALS);
#else
    {
	float *normal;
	normal = (float *)pfMalloc(sizeof(float)*6, pfGetSharedArena());
	normal[0] = normal[1] = 0;
	normal[2] = 1.0;
	normal[3] = normal[4] = normal[5] = 0.0;
	pfASDAttr(asd, PFASD_OVERALL_ATTR, PFASD_NORMALS, 1, normal);
    }
#endif

    pfASDConfig(asd);
    
    /*
     *  Make a default GeoState for the newly loaded ASD node.
     *  This just uses the default material; note that the
     *  pfMtlColorMode must be enabled to allow pfGeoSet or other
     *  RGB color commands to override this default color setting.
     */

    gstate = pfNewGState(pfGetSharedArena());
    gstates = pfMalloc(sizeof(pfGeoState*), pfGetSharedArena());
    gstates[0] = gstate;
    material = pfNewMtl(pfGetSharedArena());
    pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_OFF);
    pfMtlColor(material, PFMTL_AMBIENT,  0.8f, 0.8f, 0.8f);
    pfMtlColor(material, PFMTL_DIFFUSE,  0.8f, 0.8f, 0.8f);
    pfMtlColor(material, PFMTL_SPECULAR, 0.f, 0.f, 0.f);

    pfGStateAttr(gstate, PFSTATE_FRONTMTL, material);
    pfGStateMode(gstate, PFSTATE_ENLIGHTING, PF_ON);
    pfASDGStates(asd, gstates, 1);
    pfRef(gstate);

    if(pfIsOfType(asd, pfGetASDClassType()))
        printf("in pfdBuildASD, this is an ASD node\n");

printf("the arena size is %d K bytes\n", pfGetSharedArenaSize()/(1<<10));
printf("return asd\n");
    return asd;
}

PFDUDLLEXPORT void
pfdStoreASD(pfASD *asd, FILE *f)
{
    int numlods, numverts, numattrs, numfaces, numfaces0;
    pfASDLODRange *lods;
    pfASDVert *verts;
    pfASDFace *faces;
    float *a, *attrs, *overall_attrs;
    int bind, n, overall_mode, attr_mode;
    int overall_stride, stride;
    int morphattr;

printf("inside pfdStoreASD\n");
    pfGetASDAttr(asd, PFASD_LODS, &bind, &numlods, (void **)&lods);
    fwrite(&numlods, sizeof(int), 1, f);
    fwrite(lods, sizeof(pfASDLODRange), numlods, f);
printf("numlods %d\n", numlods);
    pfGetASDAttr(asd, PFASD_COORDS, &bind, &numverts, (void **)&verts);
    fwrite(&numverts, sizeof(int), 1, f);
    fwrite(verts, sizeof(pfASDVert), numverts, f);
printf("numverts %d\n", numverts);

#if NORMAL
    overall_mode = 0;
    attr_mode = 0;
    overall_stride = stride = 0;
    numattrs = 0;
    pfGetASDAttr(asd, PFASD_OVERALL_ATTR, &bind, &n, (void **)&a);
    overall_attrs = a;
    overall_mode = bind;
    if(bind&PFASD_NORMALS)
    {
	overall_stride += 3;
    }
    if(bind&PFASD_COLORS) 
    {
	overall_stride += 4;
    }
    if(bind&PFASD_TCOORDS)
    {
	overall_stride += 2;
    }
    pfGetASDAttr(asd, PFASD_PER_VERTEX_ATTR, &bind, &n, (void **)&a);
    attrs = a;
    attr_mode = bind;
    numattrs = n;
    if(bind&PFASD_NORMALS)
    {
	stride += 3;
    }
    if(bind&PFASD_COLORS)
    {
        stride += 4;
    }
    if(bind&PFASD_TCOORDS)
    {
        stride += 2;
    }
    bind = PFASD_PER_VERTEX_ATTR;
    fwrite(&bind, sizeof(int), 1, f);
    fwrite(&attr_mode, sizeof(int), 1, f);
    fwrite(&numattrs, sizeof(int), 1, f);
    fwrite(attrs, sizeof(float), numattrs * 2 * stride, f);
printf("per_vertex numattrs %d, attrmode %d\n", numattrs, attr_mode);
    bind = PFASD_OVERALL_ATTR;
    fwrite(&bind, sizeof(int), 1, f);
    fwrite(&overall_mode, sizeof(int), 1, f);
    if(overall_mode == 0)
	n = 0;
    else
    	n = 1;
    fwrite(&n, sizeof(int), 1, f);
    fwrite(overall_attrs, sizeof(float), 1 * 2 * overall_stride, f);
printf("overall numattrs %d, attrmode %d\n", n, overall_mode);
    
#endif

    pfGetASDAttr(asd, PFASD_FACES, &bind, &numfaces, (void **)&faces);
    fwrite(&numfaces, sizeof(int), 1, f);
    numfaces0 = pfGetASDNumBaseFaces(asd);
    fwrite(&numfaces0, sizeof(int), 1, f);
    fwrite(faces, sizeof(pfASDFace), numfaces, f);
printf("numfaces %d, numfaces0 %d\n", numfaces, numfaces0);

    morphattr = pfGetASDMorphAttrs(asd);
    fwrite(&morphattr, sizeof(int), 1, f);
printf("morphattr %d\n", morphattr);
}



PFDUDLLEXPORT void
pfdBreakTiles(int numlods, pfASDLODRange *lods, int numverts, pfASDVert *verts, int numfaces, pfASDFace *faces, int numfaces0, char *prename, char *conf)
{
    int i, j, k;
    pfASDFace *face;
    pfVec3 min, max;
    pfVec3 *origin;
    int ***FACE, ***V;
    int **numv, **numf;
    int cnt, fcnt;
    int x, y;
    pfVec3 grid; /* length of the grid cell */
    int *facequeue, qstart, qend;
    FILE *confp;
    short **totaltiles;
    float **tilesize;
    int *maxfaces, *maxverts;
    int *vflag, *fflag;

    pfBox *facebounds;

    pfBox bbox;

    vflag = (int *)pfCalloc(numverts, sizeof(int), pfGetSharedArena());
    facequeue = (int *)pfMalloc(numfaces*sizeof(int), pfGetSharedArena());
    fflag = (int *)pfCalloc(numfaces, sizeof(int), pfGetSharedArena());
    qstart = qend = 0;

    confp = (FILE *)fopen(conf, WRITE_MODE);
    totaltiles = (short **)pfMalloc((numlods+1)*sizeof(short*), pfGetSharedArena());
    tilesize = (float **)pfMalloc((numlods+1)*sizeof(float *), pfGetSharedArena());
    maxfaces = (int *)pfMalloc((numlods+1)*sizeof(int), pfGetSharedArena());
    maxverts = (int *)pfMalloc((numlods+1)*sizeof(int), pfGetSharedArena());

    for(i = 0; i <= numlods; i++)
    {
        totaltiles[i] = (short *)pfMalloc(2*sizeof(short), pfGetSharedArena());
        tilesize[i] = (float *)pfMalloc(2*sizeof(float), pfGetSharedArena());
    }

    PFCOPY_VEC3(min, verts[0].v0);
    PFCOPY_VEC3(max, verts[0].v0);
    for(i = 0; i < numverts; i++)
    {
        verts[i].neighborid[0]=verts[i].neighborid[1]=PFASD_NIL_ID;
        verts[i].vertid = i;
        if(verts[i].v0[0] < min[0]) min[0] = verts[i].v0[0];
        if(verts[i].v0[1] < min[1]) min[1] = verts[i].v0[1];
        if(verts[i].v0[2] < min[2]) min[2] = verts[i].v0[2];
        if(verts[i].v0[0] > max[0]) max[0] = verts[i].v0[0];
        if(verts[i].v0[1] > max[1]) max[1] = verts[i].v0[1];
        if(verts[i].v0[2] > max[2]) max[2] = verts[i].v0[2];
    }

    fprintf(confp, "%s\n", prename);
    fprintf(confp, "%d\n", numfaces0);
    fprintf(confp, "%d\n", numlods);
    for(i = 0; i < numlods; i++)
        fprintf(confp, "%f %f\n", lods[i].switchin, lods[i].morph);

    fprintf(confp, "%f %f %f\n", min[0], min[1], min[2]);
    fprintf(confp, "%f %f %f\n", max[0], max[1], max[2]);
    PFCOPY_VEC3(bbox.min, min);
    PFCOPY_VEC3(bbox.max, max);
    origin = (pfVec3*)pfMalloc(sizeof(pfVec3)*numlods, pfGetSharedArena());
    for(i = 0; i< numlods; i++)
	PFCOPY_VEC3(origin[i], min);
/* all lods have the same min corner */
#if 0
    pfASDOrigin(asd, origin);
    pfASDBBox(asd, &bbox);
    pfASDMorphAttrs(asd, PFASD_NO_ATTR);
    pfASDNumBaseFaces(asd, numfaces0);
    pfASDPageFname(asd, prename);
#endif


/* find all the bboxes for all the faces. these boxes are stored in
   the files too */

    facebounds = pfdGetTerrainBounds(faces, verts, numfaces0, numfaces);

    FACE = (int ***)pfCalloc(1, sizeof(int**), pfGetSharedArena());
    FACE[0] = (int **)pfCalloc(1, sizeof(int*), pfGetSharedArena());
    FACE[0][0] = (int *)pfCalloc(numfaces0, sizeof(int), pfGetSharedArena());
    V = (int ***)pfCalloc(1, sizeof(int**), pfGetSharedArena());
    V[0] = (int **)pfCalloc(1, sizeof(int*), pfGetSharedArena());
    V[0][0] = (int *)pfCalloc(numfaces0*6, sizeof(int), pfGetSharedArena());
    numv = (int **)pfCalloc(1, sizeof(int*), pfGetSharedArena());
    numv[0] = (int *)pfCalloc(1, sizeof(int), pfGetSharedArena());
    numf = (int **)pfCalloc(1, sizeof(int*), pfGetSharedArena());
    numf[0] = (int *)pfCalloc(1, sizeof(int), pfGetSharedArena());

    numf[0][0] = 0;
    numv[0][0] = 0;
    for(i = 0; i < numfaces0; i++)
    {
        face = &faces[i];

        FACE[0][0][i] = i;
        fflag[i] = 1;
        for(j = 0; j < 3; j++)
            if(vflag[face->vert[j]] == 0)
            {
                V[0][0][numv[0][0]++] = face->vert[j];
                vflag[face->vert[j]] = 1;
            }

        for(j = 0; j < 3; j++)
        {
            int sid;

            sid = face->refvert[j];
            if(sid > PFASD_NIL_ID)
            {
                if(verts[sid].neighborid[0] == PFASD_NIL_ID)
                    verts[sid].neighborid[0] = i;
                else if(verts[sid].neighborid[1] == PFASD_NIL_ID)
                    verts[sid].neighborid[1] = i;
                else
                    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "pfdBreakTile:\n refvert %d has too many neighbors\n", sid);
                if(vflag[sid] == 0)
                {
                    V[0][0][numv[0][0]++] = sid;
                    vflag[sid] = 1;
                }
            }
        }

        facequeue[qend++] = i; /* put tris on the queue for next lod breakup */
    }

    numf[0][0] = numfaces0;
    pfdWriteFile(FACE, numf, V, numv, faces, verts, facebounds, 0, 1, 1, prename);
    totaltiles[0][0] = totaltiles[0][1] = 1;
    tilesize[0][0] = max[0]-min[0];
    tilesize[0][1] = max[1]-min[1];
    maxfaces[0] = numfaces0;
    maxverts[0] = numv[0][0];

    pfFree(FACE[0][0]);
    pfFree(FACE[0]);
    pfFree(FACE);
    pfFree(V[0][0]);
    pfFree(V[0]);
    pfFree(V);
    pfFree(numv[0]);
    pfFree(numv);
    pfFree(numf[0]);
    pfFree(numf);

    while(qstart != qend)
    {
    float maxedge;
    int level;

    cnt = qstart;
    fcnt = 0;
    maxedge = 0;
    face = &faces[facequeue[cnt]];
    level = face->level;
    printf("check out faces in level %d\n", level);
    while((cnt != qend)&&(face->level == level))
         /* this is the triangles of this level */
    {
        float e;
        int a, b;
        int ii;

        fcnt++;
        for(ii = 0; ii < 3; ii++)
        {
            a = face->vert[ii];
            b = face->vert[(ii+1)%3];
            e = PFDISTANCE_PT3(verts[a].v0, verts[b].v0);
            if(e > maxedge)
                maxedge = e;
        }
        face = &faces[facequeue[++cnt]];
/*
        printf("while check out faces in level %d\n", face->level);
*/
    }
       
    x = (max[0] - min[0])/(2*maxedge) + 1; /* estimated x num of cells */
    y = (max[1] - min[1])/(2*maxedge) + 1; /* estimated y num of cells */

printf("x %d, y %d, fcnt %d fcnt/(x*y) %d \n", x, y, fcnt, fcnt/(x*y));
    if(fcnt/(x*y) < MIN_TRIS) /* if the cell has too few triangles */
    {
        x = sqrt(fcnt/MIN_TRIS) + 1;
        y = sqrt(fcnt/MIN_TRIS) + 1;
    }

    while(fcnt/(x*y) > MAX_TRIS/2) /* if the cell contains too many triangles */
    {
	x = 2*x;
	y = 2*y;
    }

    grid[0] = (max[0] - min[0])/(float)x; /* enlarge the cell */
    grid[1] = (max[1] - min[1])/(float)y;

/* we will construct tiles for the next LOD based on this LOD */
    level += 1;
    totaltiles[level][0] = x;
    totaltiles[level][1] = y;
    tilesize[level][0] = grid[0];
    tilesize[level][1] = grid[1];
/* 8 is an experimental number */
  
    FACE = (int ***)pfCalloc(x, sizeof(int**), pfGetSharedArena());
    V = (int ***)pfCalloc(x, sizeof(int**), pfGetSharedArena());
    numf = (int **)pfCalloc(x, sizeof(int*), pfGetSharedArena());
    numv = (int **)pfCalloc(x, sizeof(int*), pfGetSharedArena());

    for(i = 0; i < x; i++)
    {
        FACE[i] = (int **)pfCalloc(y, sizeof(int*), pfGetSharedArena());
        V[i] = (int **)pfCalloc(y, sizeof(int*), pfGetSharedArena());
        numf[i] = (int *)pfCalloc(y, sizeof(int), pfGetSharedArena());
        numv[i] = (int *)pfCalloc(y, sizeof(int), pfGetSharedArena());
        for(j = 0; j < y; j++)
        {
            FACE[i][j] = (int *)pfCalloc(MAX_TRIS, sizeof(int), pfGetSharedArena());
            V[i][j] = (int *)pfCalloc(MAX_TRIS*6, sizeof(int), pfGetSharedArena());
        }
    }

    while(qstart != cnt) /* allocate children of tris in this LOD */
    {
        int ix, iy;
        float xx, yy;
        int faceid;

        face = &faces[facequeue[qstart]];
/*
        printf("process facequeue[%d]=%d\n", qstart, facequeue[qstart]);
        printf("face->level %d\n", face->level);
*/
        faceid = facequeue[qstart];
        xx = (verts[face->vert[0]].v0[0]-min[0])/grid[0];
        ix = (verts[face->vert[0]].v0[0]-min[0])/grid[0];
        if(((xx - ix) < SMALL_EPS)&&(xx > SMALL_EPS))
            ix = ix - 1;
        yy = (verts[face->vert[0]].v0[1]-min[1])/grid[1];
        iy = (verts[face->vert[0]].v0[1]-min[1])/grid[1];
        if(((yy - iy) < SMALL_EPS)&&(yy > SMALL_EPS))
            iy = iy - 1;
        for(i = 0; i < 4; i++)
        {
            if(face->child[i] > PFASD_NIL_ID) /* got a child, put it
                                                        in the cell */
            {
                int cid;
                pfASDFace *cface;

                cid = face->child[i];
                cface = &faces[cid];
/*
                printf("checking out child face %d\n", cid);
*/
                if(fflag[cid] == 0)
                {
/*
                printf("assign child face %d, level %d\n", cid, cface->level);
*/
                    FACE[ix][iy][numf[ix][iy]] = cid;
                    fflag[cid] = 1;
                    numf[ix][iy]++;
/* XXXX if there are too many triangles, enlarge the storage space. hopefully there won't be more than 2*MAX_TRIS triangles. otherwise, serious memory problems */
		    if(numf[ix][iy] >= MAX_TRIS)
			FACE[ix][iy] = pfRealloc(FACE[ix][iy], 2*MAX_TRIS*sizeof(int));
                }
                for(j = 0; j < 3; j++)
                {
                    int sid;
                    int sx, sy;

                    sid = cface->refvert[j];
                    if(sid > PFASD_NIL_ID)
                    {
                        if(vflag[sid]==0)
                        {
                        xx = (verts[sid].v0[0]+verts[sid].vd[0]-min[0])/grid[0];                        sx = (verts[sid].v0[0]+verts[sid].vd[0]-min[0])/grid[0];                        if(((xx-sx) < SMALL_EPS)&&(xx > SMALL_EPS))
                        sx -= 1;
                        yy = (verts[sid].v0[1]+verts[sid].vd[1]-min[1])/grid[1];                        sy = (verts[sid].v0[1]+verts[sid].vd[1]-min[1])/grid[1];                        if(((yy-sy) < SMALL_EPS)&&(yy > SMALL_EPS))
                        sy -= 1;
                        V[sx][sy][numv[sx][sy]] = sid;
                        numv[sx][sy]++;
			if(numv[sx][sy] >= MAX_TRIS*6)
			    V[sx][sy] = pfRealloc(V[sx][sy], 12*MAX_TRIS*sizeof(int));
                        vflag[sid] = 1;
                        }
                        if(verts[sid].neighborid[0] == PFASD_NIL_ID)
                            verts[sid].neighborid[0] = cid;
                        else if(verts[sid].neighborid[1] == PFASD_NIL_ID)
                            verts[sid].neighborid[1] = cid;

                    }
                }
                facequeue[qend++] = cid;
            }
        }
        qstart++;
    }

    maxfaces[level] = 0;
    maxverts[level] = 0;
    for(i = 0; i < x; i++)
        for(j = 0; j < y; j++)
        {
            if(numf[i][j] > maxfaces[level])
                maxfaces[level] = numf[i][j];
            if(numv[i][j] > maxverts[level])
                maxverts[level] = numv[i][j];
        }

    pfdWriteFile(FACE, numf, V, numv, faces, verts, facebounds, level, x, y, prename);

    for(i = 0; i < x; i++)
    {
        for(j = 0; j < y; j++)
        {
            pfFree(FACE[i][j]);
            pfFree(V[i][j]);
        }
        pfFree(FACE[i]);
        pfFree(V[i]);
        pfFree(numf[i]);
        pfFree(numv[i]);
    }

    pfFree(FACE);
    pfFree(V);
    pfFree(numv);
    pfFree(numf);
    }

    for(i = 0; i < numlods; i++)
        fprintf(confp, "%d %d\n", totaltiles[i][0], totaltiles[i][1]);

    for(i = 0; i < numlods; i++)
        fprintf(confp, "%f %f\n", tilesize[i][0], tilesize[i][1]);

    for(i = 0; i < numlods; i++)
    	fprintf(confp, "%d %d\n", maxfaces[i], maxverts[i]);

    pfFree(verts);
    pfFree(facebounds);
    pfFree(maxfaces);
    pfFree(maxverts);
    fclose(confp);

}


PFDUDLLEXPORT void
pfdPagingLookahead(char *confile, char *pfile, int *lookahead)
{
    FILE *confp;
    FILE *pfp;
    int page[2], numfaces0, numlods, x, y, i;
    char prename[140];
    pfASDLODRange *lods;
    float dummy, tilesize[2];

    if((confp = (FILE *)fopen(confile, READ_MODE)) == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
	    "can't open file %s\n", confile);
	return;
    }

    if((pfp = (FILE *)fopen(pfile, WRITE_MODE)) == NULL)
    {
        pfNotify(PFNFY_WARN, PFNFY_PRINT,
            "can't open file %s\n", pfile);
        return;
    }

    fscanf(confp, "%s", prename);
    fscanf(confp, "%d", &numfaces0);
    fscanf(confp, "%d", &numlods);

    lods = (pfASDLODRange *) pfMalloc(sizeof(pfASDLODRange) * numlods,
                           pfGetSharedArena());
    for(i = 0; i < numlods; i++)
        fscanf(confp, "%f %f", &(lods[i].switchin), &(lods[i].morph));
    fscanf(confp, "%f %f %f", &dummy, &dummy, &dummy);
    fscanf(confp, "%f %f %f", &dummy, &dummy, &dummy);
    for(i = 0; i< numlods; i++)
    {
        int x, y;
        fscanf(confp, "%d %d", &x, &y);
    }
    for(i = 0; i < numlods; i++)
    {
        fscanf(confp, "%f %f", &tilesize[0], &tilesize[1]);
	printf("%f %f\n", tilesize[0], tilesize[1]);
	page[0] = (int)(lods[i].switchin/tilesize[0]) + lookahead[0];
	page[1] = (int)(lods[i].switchin/tilesize[1]) + lookahead[1];
	printf("lookahead %d %d\n", lookahead[0], lookahead[1]);
	printf("page %f %d\n", lods[i].switchin/tilesize[0], page[0]);
	fprintf(pfp, "%d %d\n", page[0], page[1]);
    }

    fclose(confp);
    fclose(pfp);
}

PFDUDLLEXPORT pfASD *
pfdLoadConfig(char *fname, char *pagename)
{
    	FILE *confp, *pfp;
	int numlods;
	pfASDLODRange *lods;
        int *maxv, *maxf, mv, mf;
        int ci, cj;
        pfBox bbox;
        pfVec3 min;
 	pfVec3 *origin;
        short **totaltiles, **page;
        float **tilesize;
 	pfASD *asd;
 	int numfaces0;
	char *prename;

    	pfGeoState *gstate, **gstates;
    	pfMaterial *material;

	int i;
	
        if((confp = (FILE *)fopen(fname, READ_MODE))==NULL)
	{
	    pfNotify(PFNFY_WARN, PFNFY_ASSERT,
		"can't open file %s\n", fname);
	    return NULL;
 	}

 	if((pfp = (FILE *)fopen(pagename, READ_MODE))==NULL)
        {
            pfNotify(PFNFY_WARN, PFNFY_ASSERT,
                "can't open file %s\n", pagename);
            return NULL;
        }

	asd = pfNewASD();
	prename = pfMalloc(sizeof(char)*150, pfGetSharedArena());
	fscanf(confp, "%s", prename);
	pfASDPageFname(asd, prename);
        fscanf(confp, "%d", &numfaces0);
	pfASDNumBaseFaces(asd, numfaces0);
        fscanf(confp, "%d", &numlods);
        page = (short **)pfMalloc(numlods*sizeof(short*), pfGetSharedArena());
        totaltiles = (short **)pfMalloc(numlods*sizeof(short*), pfGetSharedArena());
        tilesize = (float **)pfMalloc(numlods*sizeof(float *), pfGetSharedArena());
  	maxf = (int *)pfMalloc(numlods*sizeof(int), pfGetSharedArena());
  	maxv = (int *)pfMalloc(numlods*sizeof(int), pfGetSharedArena());
        for(ci = 0; ci < numlods; ci++)
        {
            totaltiles[ci] = (short *)pfMalloc(2*sizeof(short), pfGetSharedArena());
            tilesize[ci] = (float *)pfMalloc(2*sizeof(float), pfGetSharedArena());
        }

        lods = (pfASDLODRange *) pfMalloc(sizeof(pfASDLODRange) * numlods,
                                    pfGetSharedArena());
        for(ci = 0; ci < numlods; ci++)
            fscanf(confp, "%f %f", &(lods[ci].switchin), &(lods[ci].morph));
        pfASDAttr(asd, PFASD_LODS, 0, numlods, lods);
        fscanf(confp, "%f %f %f", &min[0], &min[1], &min[2]);
        PFCOPY_VEC3(bbox.min, min);
        fscanf(confp, "%f %f %f", &bbox.max[0], &bbox.max[1], &bbox.max[2]);
	origin = (pfVec3*)pfMalloc(sizeof(pfVec3)*numlods, pfGetSharedArena());
    	for(i = 0; i< numlods; i++)
            PFCOPY_VEC3(origin[i], min);
        pfASDOrigin(asd, origin);
        pfASDBBox(asd, &bbox);
        for(ci = 0; ci < numlods; ci++)
	{
	    int x, y;
            fscanf(confp, "%d %d", &x, &y);
            totaltiles[ci][0] = (short)x;
	    totaltiles[ci][1] = (short)y;
	}
        pfASDTotalTiles(asd, totaltiles);

/*
        for(ci = 0; ci < numlods; ci++)
        {
	    int x, y;
            fscanf(confp, "%d %d", &x, &y);
	    page[ci][0] = (short)x;
	    page[ci][1] = (short)y;
	printf("page[%d] %d %d\n", ci, page[ci][0], page[ci][1]);
 	    if(page[ci][0] == 0) page[ci][0]+=1;
	    if(page[ci][1] == 0) page[ci][1]+=1;
        }
        pfASDPageSize(asd, page);
*/

        for(ci = 0; ci < numlods; ci++)
            fscanf(confp, "%f %f", &tilesize[ci][0], &tilesize[ci][1]);
        pfASDTileSize(asd, tilesize);
	mf = mv = -1;
     	for(ci = 0; ci < numlods; ci++)
	{
            fscanf(confp, "%d %d", &maxf[ci], &maxv[ci]);
	    if(maxf[ci] > mf) mf = maxf[ci];
	    if(maxv[ci] > mv) mv = maxv[ci];
	}
	printf("max faces %d, max verts %d\n", mf, mv);
        pfASDMaxTileMemSize(asd, mf, mv);

#if 0
 	for(ci = 0; ci < numlods; ci++)
	{
	    pfFree(totaltiles[ci]);
	    pfFree(tilesize[ci]);
	}
	pfFree(totaltiles);
	pfFree(tilesize);
	pfFree(maxf);
	pfFree(maxv);
	pfFree(origin);
#endif

	page = (short **)pfMalloc(numlods*sizeof(short*), pfGetSharedArena());
	for(ci = 0; ci < numlods; ci++)
        {
            int x, y;
            fscanf(pfp, "%d %d", &x, &y);
            page[ci] = (short *)pfMalloc(2*sizeof(short), pfGetSharedArena());
            page[ci][0] = (short)x;
            page[ci][1] = (short)y;
        printf("page[%d] %d %d\n", ci, page[ci][0], page[ci][1]);
            if(page[ci][0] == 0) page[ci][0]+=1;
            if(page[ci][1] == 0) page[ci][1]+=1;
        }
        pfASDPageSize(asd, page);
#if 0
	for(ci = 0; ci < numlods; ci++)
        {
            pfFree(page[ci]);
	}
	pfFree(page);
#endif

 	fclose(confp);
	fclose(pfp);

    /*
     *  Make a default GeoState for the newly loaded ASD node.
     *  This just uses the default material; note that the
     *  pfMtlColorMode must be enabled to allow pfGeoSet or other
     *  RGB color commands to override this default color setting.
     */

    gstate = pfNewGState(pfGetSharedArena());
    gstates = pfMalloc(sizeof(pfGeoState*), pfGetSharedArena());
    gstates[0] = gstate;
    material = pfNewMtl(pfGetSharedArena());
    pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_OFF);
    pfGStateAttr(gstate, PFSTATE_FRONTMTL, material);
    pfGStateMode(gstate, PFSTATE_ENLIGHTING, PF_ON);
    pfASDGStates(asd, gstates, 1);
    pfRef(gstate);

    pfASDInitPaging(asd);
    pfASDConfig(asd);

    return asd;
}

PFDUDLLEXPORT void
pfdWriteFile(int ***FACE, int **numf, int ***V, int **numv, pfASDFace *faces, pfASDVert *verts, pfBox *facebounds, int lod, int x, int y, char *prename)
{
    char fname[300];
    char tname[300];
    FILE *fp, *tp;
    int i, j, m, n;

    for(i = 0; i < x; i++)
        for(j = 0; j < y; j++)
        {
	/* if there are no data in this tile, don't write it out */
 	    if((numf[i][j]==0)&&(numv[i][j]==0))
		continue;
            sprintf(fname, "%s%02d%03d%03d", prename, lod, i, j);
	    /*
            sprintf(tname, "a%s%02d%03d%03d", prename, lod, i, j);
	    */
            printf("output file %s\n", fname);
            if(!(fp = (FILE *)fopen(fname, WRITE_MODE)))
            {
                printf("can't open file %s\n", fname);
                return;
            }
	    /*
            if(!(tp = (FILE *)fopen(tname, WRITE_MODE)))
            {
                printf("can't open file %s\n", tname);
                return;
            }
	    */
            fwrite(&(numf[i][j]), sizeof(int), 1, fp);
            fwrite(&(numv[i][j]), sizeof(int), 1, fp);
	    /*
            fprintf(tp, "%d ", (numf[i][j]));
            fprintf(tp, "%d\n", (numv[i][j]));
	    */
            for(m = 0; m < numf[i][j]; m++)
            {
		    fwrite(&FACE[i][j][m], sizeof(int), 1, fp);
                    fwrite(&faces[FACE[i][j][m]], sizeof(pfASDFace), 1, fp);
            }
            for(m = 0; m < numf[i][j]; m++)
            {
                    fwrite(&facebounds[FACE[i][j][m]], sizeof(pfBox), 1, fp);
		    /*
                    fprintf(tp, "%f %f %f %f %f %f\n", facebounds[FACE[i][j][m]].min[0], facebounds[FACE[i][j][m]].min[1], facebounds[FACE[i][j][m]].min[2], facebounds[FACE[i][j][m]].max[0], facebounds[FACE[i][j][m]].max[1], facebounds[FACE[i][j][m]].max[2]);
		    */
            }
            for(n = 0; n < numv[i][j]; n++)
            {
		    fwrite(&(verts[V[i][j][n]].vertid), sizeof(int), 1, fp);
                    fwrite(&verts[V[i][j][n]], sizeof(pfASDVert), 1, fp);
		    /*
                    fprintf(tp, "%d\n", verts[V[i][j][n]].vertid);
		    */
            }
            fclose(fp);
	    /*
            fclose(tp);
	    */
        }
}

PFDUDLLEXPORT pfBox *
pfdGetTerrainBounds(pfASDFace *faces, pfASDVert *verts, int numfaces0, int numfaces){
    int faceid;
    pfBox *facebounds;

    facebounds = (pfBox *)pfMalloc(sizeof(pfBox)*numfaces, pfGetSharedArena());
    for (faceid=0; faceid<numfaces0; faceid++)
            computeBBox(facebounds, faces, verts, faceid);
    return facebounds;
}

void
computeBBox(pfBox *facebounds, pfASDFace *faces, pfASDVert *verts, int _faceid)
{
    int i;
    const pfASDFace *face;
    pfBox *bounds;

    if (_faceid == PFASD_NIL_ID) return;

    face = &faces[_faceid];
    bounds = (pfBox*) &facebounds[_faceid];

    pfMakeEmptyBox(bounds);
    for (i=0; i<3; i++)
    {
        pfVec3 v, vd;
        int vertid;
        int refvertid;
        vertid = face->vert[i];
        refvertid = face->refvert[i];

        PFCOPY_VEC3(v, verts[vertid].v0);
        pfBoxExtendByPt(bounds, v);

        PFCOPY_VEC3(vd, verts[vertid].vd);
        v[0] += vd[0];
        v[1] += vd[1];
        v[2] += vd[2];
        pfBoxExtendByPt(bounds, v);

/*
        if(refvertid > PFASD_NIL_ID) {
            v = VERT[refvertid].v0;
            bounds->extendBy(v);
        }
*/
    }

  /*
   * Calculate child bounding volumes, and union with parent
   */
    for (i=0; i<4; i++)
    {
        int childfaceid;

        childfaceid = face->child[i];
        if (childfaceid == PFASD_NIL_ID) continue;

        computeBBox(facebounds, faces, verts, childfaceid);
        pfBoxExtendByBox(bounds, &facebounds[childfaceid]);
    }

/*
printf("bounds %f %f %f, %f %f %f\n", bounds->min[0],
bounds->min[1], bounds->min[2], bounds->max[0], bounds->max[1], bounds->max[2]);*/

}

