/*
 * pfdGeoBuilder.c
 *
 * $Revision: 1.100 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * The geobuilder is a low-level state-free geometry construction
 * library. It is used by the pfdBuilder to build IRIS Performer
 * scene graphs.
 *
 *
 * Copyright 1995, Silicon Graphics, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#endif
#ifndef WIN32
#include <alloca.h>
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

/* ###################################################################### */
/* #                                                                    # */
/* #  Private pfdGeoBuilder data structures and function declarations   # */
/* #                                                                    # */
/* ###################################################################### */

#define SORTOF 1.0e-4f

/* #define CHECK_ALLOCATIONS /* */
#define CHECK_DUPLICATE

struct _pfdGeoBuilder
{
    pfdPrim	    *triList;
    pfdPrim	    *availTri;

    pfVec3	    *coordList;
    pfVec3	    *normList;
    pfVec2	    *texCoordList[PF_MAX_TEXTURES];
    pfVec4	    *colorList;

    int             numPoints;
    int             pointCounts[4][4][PF_MAX_TEXTURES + 1];
    pfdPrim         *pointBins[4][4][PF_MAX_TEXTURES + 1];

    int             numIndexedPoints;
    int             indexedPointCounts[4][4][PF_MAX_TEXTURES + 1];
    pfdPrim         *indexedPointBins[4][4][PF_MAX_TEXTURES + 1];

    int	    	    numTris;
    int	    	    triCounts[4][4][PF_MAX_TEXTURES + 1];
    pfdPrim	    *triBins[4][4][PF_MAX_TEXTURES + 1];

    int	    	    numIndexedTris;
    int	    	    indexedTriCounts[4][4][PF_MAX_TEXTURES + 1];
    pfdPrim	    *indexedTriBins[4][4][PF_MAX_TEXTURES + 1];

    int	    	    numLines;
    int	    	    lineCounts[4][4][PF_MAX_TEXTURES + 1];
    pfdPrim	    *lineBins[4][4][PF_MAX_TEXTURES + 1];

    int	    	    numIndexedLines;
    int	    	    indexedLineCounts[4][4][PF_MAX_TEXTURES + 1];
    pfdPrim	    *indexedLineBins[4][4][PF_MAX_TEXTURES + 1];

    int	    	    numLineStrips;
    int	    	    linestripCounts[4][4][PF_MAX_TEXTURES + 1];
    pfdGeom	    *linestripBins[4][4][PF_MAX_TEXTURES + 1];

    int	    	    numIndexedLineStrips;
    int	    	    indexedlinestripCounts[4][4][PF_MAX_TEXTURES + 1];
    pfdGeom	    *indexedlinestripBins[4][4][PF_MAX_TEXTURES + 1];

    int             numFlatLineStrips;
    int             flatlinestripCounts[4][4][PF_MAX_TEXTURES + 1];
    pfdGeom         *flatlinestripBins[4][4][PF_MAX_TEXTURES + 1];

    int             numIndexedFlatLineStrips;
    int             indexedflatlinestripCounts[4][4][PF_MAX_TEXTURES + 1];
    pfdGeom         *indexedflatlinestripBins[4][4][PF_MAX_TEXTURES + 1];

    pfList	    *gsetList;

    int	    	    autoColors;
    int	    	    autoNormals;
    int	    	    autoTexture;
    int	    	    autoOrient;
    int	    	    collapse;
    int 	    mesh;
    int		    limit;
    int		    shareIndexLists;

#ifdef CHECK_DUPLICATE
    /* for quick comparison of coordinates */
    pfuHashTable    *hashTable;	    
#endif

    pfList	    *mallocList;
    pfList	    *mallocSizeList;

    void	    *sharedArena;
};

#define TRICHUNK	 128		/* initial allocation size */
#define MAXTRICHUNK	8192		/* maximum allocation size */
#define LINK_TRI	  69
#define HASHSIZE	   7

static pfdPrim * getPrim(pfdGeoBuilder *bldr);
static void addPoint(pfdGeoBuilder *bldr, pfdPrim *point);
static void addIndexedPoint(pfdGeoBuilder *bldr, pfdPrim *point);
static void addPoints(pfdGeoBuilder *bldr, pfdGeom *points);
static void addLine(pfdGeoBuilder *bldr, pfdPrim *line);
static void addIndexedLine(pfdGeoBuilder *bldr, pfdPrim *line);
static void addLines(pfdGeoBuilder *bldr, pfdGeom *lines);
static void addLineStrip(pfdGeoBuilder *bldr, pfdGeom *strip);
static void addIndexedLineStrip(pfdGeoBuilder *bldr, pfdGeom *strip);
static void addLineStrips(pfdGeoBuilder *bldr, pfdGeom *strip, int num);
static void addTri(pfdGeoBuilder *bldr, pfdPrim *tri);
static void addIndexedTri(pfdGeoBuilder *bldr, pfdPrim *tri);
static void addPoly(pfdGeoBuilder *bldr, pfdGeom *polygon);
static void copyPrim(pfdPrim *src, pfdPrim *dst);
static void copyGeom(pfdPrim *tri, pfdGeom *pgon, int v0, int v1, int v2);
static void copyLineStrip(pfdGeom *dst, pfdGeom *src);
static void moveTriList(pfdGeoBuilder *ts, int nb1, int cb1, int tb1, int nb2, int cb2, int tb2);
static void moveIndexedTriList(pfdGeoBuilder *ts, int nb1, int cb1, int tb1, int nb2, int cb2, int
tb2);
static void collapseAttr(pfdGeoBuilder *bldr, pfdPrim *tri, int attr);
static void freeStripBins(pfdGeom *bin[4][4][PF_MAX_TEXTURES + 1], int count[4][4][PF_MAX_TEXTURES + 1]);
static void makeTriGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind);
static void makeIndexedTriGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind);
static void makeLineGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind);
static void makeFlatLineGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind);
static void makeIndexedLineGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind);
static void makeIndexedFlatLineGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind);
static void makePointGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind);
static void makeIndexedPointGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind);
static void makeGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind);
static int decompConcave(pfdGeom *pgon, pfdPrim *triList, int asum, int x, int y);
static int triangulatePoly(pfdGeom *pgon, pfdPrim *triList);

#ifdef	OLD_STYLE_FREE
static void freeTris(pfdGeoBuilder *bldr);
#endif

/* ###################################################################### */
/* #                                                                    # */
/* #                    pfdGeoBuilder Functions                         # */
/* #                                                                    # */
/* ###################################################################### */

PFDUDLLEXPORT pfdGeoBuilder*
pfdNewGeoBldr(void)
{
    pfdGeoBuilder	*bldr;
    void *sharedArena = pfGetSharedArena();

    /* allocate builder structure in unshared memory */
    if ((bldr = (pfdGeoBuilder*) pfCalloc(1, sizeof(pfdGeoBuilder), sharedArena)) == NULL)
	return NULL;

    /* initialize shared arena pointer for this pfdGeoBuilder */
    bldr->sharedArena = sharedArena;

    /* start resulting geoset list off small ... it grows as needed */
    if ((bldr->gsetList = pfNewList(sizeof(pfGeoSet*), 1, NULL)) == NULL)
	return NULL;

    /* keep track of allocations for eventual deallocation */
    if ((bldr->mallocList = pfNewList(sizeof(void*),16,NULL)) == NULL)
	return NULL;

#ifdef CHECK_ALLOCATIONS
    /* keep track of allocation sizes as well */
    if ((bldr->mallocSizeList = pfNewList(sizeof(long),16,NULL)) == NULL)
	return NULL;
#endif

    /* allocate a chunk of primitives */
    if ((bldr->triList = pfCalloc(TRICHUNK, sizeof(pfdPrim), NULL)) == NULL)
	return NULL;
    pfAdd(bldr->mallocList, bldr->triList);
#ifdef CHECK_ALLOCATIONS
    pfAdd(bldr->mallocSizeList, (void *)(TRICHUNK*sizeof(pfdPrim)));
#endif

    bldr->triList[TRICHUNK-1].next  = bldr->triList;
    bldr->triList[TRICHUNK-1].flags = LINK_TRI;
    bldr->availTri = bldr->triList;

#ifdef CHECK_DUPLICATE
    bldr->hashTable = pfuNewHTable(HASHSIZE, sizeof(pfVec3), NULL);
#endif

    /* builder is empty */
    bldr->numTris			= 0;
    bldr->numIndexedTris		= 0;

    bldr->numPoints			= 0;
    bldr->numIndexedPoints		= 0;

    bldr->numLines			= 0;
    bldr->numIndexedLines		= 0;

    bldr->numLineStrips			= 0;
    bldr->numIndexedLineStrips		= 0;

    bldr->numFlatLineStrips		= 0;
    bldr->numIndexedFlatLineStrips	= 0;

    /* don't generate face colors if face or vertex colors are unspecified */
    bldr->autoColors  = PFDGBLDR_COLORS_PRESERVE;

    /* generate face normals if face or vertex normals are unspecified */
    bldr->autoNormals = PFDGBLDR_NORMALS_MISSING;

    /* don't modify texture coordinate specifications */
    bldr->autoTexture = PFDGBLDR_TEXTURE_PRESERVE;

    /* reverse vertex ordering based on computed and supplied normals */
    bldr->autoOrient  = PFDGBLDR_ORIENT_VERTICES;

    /* form triangle meshes from geosets */
    bldr->mesh = TRUE;

    /* collapse per_vertex or per_prim colors/normals */
    bldr->collapse = TRUE;

    /* maximum number of primitives processed as one chunk */
    bldr->limit = 20000;

    /* share index lists between per vertex geoset attributes when possible */
    bldr->shareIndexLists = TRUE;

    return bldr;    
}

PFDUDLLEXPORT void
pfdDelGeoBldr(pfdGeoBuilder *bldr)
{
    if (bldr != NULL)
    {
	int i;
	int n;

#ifdef	OLD_STYLE_FREE
	freeTris(bldr);
#endif

	if (bldr->mallocList != NULL)
	{
	    n = pfGetNum(bldr->mallocList);
	    for (i=0; i<n; i++)
		pfFree(pfGet(bldr->mallocList,i));
	    pfDelete(bldr->mallocList);
	}

#ifdef CHECK_ALLOCATIONS
	if (bldr->mallocSizeList != NULL)
	{
	    static int freed = 0;
	    int prevFreed = freed;
	    int n = pfGetNum(bldr->mallocSizeList);
	    for (i=0;i<n;i++)
		freed += (long)pfGet(bldr->mallocSizeList,i);
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
		"Freed %f MB",(freed-prevFreed)/(1024.0*1024.0));
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
		"Total Freed %f MB",freed/(1024.0*1024.0));
	    pfDelete(bldr->mallocSizeList);
	}
#endif

#ifdef CHECK_DUPLICATE
	pfuDelHTable(bldr->hashTable);
#endif

	pfDelete(bldr->gsetList);
	pfFree(bldr);
    }
}

PFDUDLLEXPORT void
pfdGeoBldrMode(pfdGeoBuilder *bldr, int mode, int val)
{
    switch (mode) 
    {
    case PFDGBLDR_AUTO_NORMALS:
	bldr->autoNormals = val;
	break;    

    case PFDGBLDR_AUTO_COLORS:
	bldr->autoColors = val;
	break;    
    
    case PFDGBLDR_AUTO_TEXTURE:
	bldr->autoTexture = val;
	break;    

    case PFDGBLDR_AUTO_ORIENT:
	bldr->autoOrient = val;
	break;    

    case PFDGBLDR_MESH_ENABLE:
	bldr->mesh = val;
	break;
    
    case PFDGBLDR_COLLAPSE_ATTRS:
	bldr->collapse = val;
	break;    

    case PFDGBLDR_BUILD_LIMIT:
	bldr->limit = val;
	break;    

    case PFDGBLDR_SHARE_INDEX_LISTS:
	bldr->shareIndexLists = val;
	break;    

    default:
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfdGeoBldrMode() unknown mode %d", mode);
	break;
    }
}

PFDUDLLEXPORT int
pfdGetGeoBldrMode(pfdGeoBuilder *bldr, int mode)
{
    switch (mode) 
    {
    case PFDGBLDR_AUTO_NORMALS:
	return bldr->autoNormals;

    case PFDGBLDR_AUTO_COLORS:
	return bldr->autoColors;

    case PFDGBLDR_AUTO_TEXTURE:
	return bldr->autoTexture;

    case PFDGBLDR_AUTO_ORIENT:
	return bldr->autoOrient;

    case PFDGBLDR_MESH_ENABLE:
	return bldr->mesh;

    case PFDGBLDR_COLLAPSE_ATTRS:
	return bldr->collapse;

    case PFDGBLDR_BUILD_LIMIT:
	return bldr->limit;

    case PFDGBLDR_SHARE_INDEX_LISTS:
	return bldr->shareIndexLists;

    default:
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfdGetGeoBldrMode() unknown mode %d", mode);
	return -1;
    }
}

PFDUDLLEXPORT int
pfdGetNumTris(pfdGeoBuilder *bldr)
{
    return bldr->numTris + bldr->numIndexedTris;
}

/*
 *	Yair: Compute the number of active texture units in a pfdGeom.
 */
PFDUDLLEXPORT void
pfdPreprocessGeomTextureUnits (pfdGeom *geom)
{
    int k;

    geom->numTextures = 0;

    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
    {
	if (geom->tbind[k] == PFGS_PER_VERTEX)
	    geom->numTextures = k + 1;
	else
	    return;
    }
}

PFDUDLLEXPORT void
pfdPreprocessPrimTextureUnits (pfdPrim *prim)
{
    int k;

    prim->numTextures = 0;

    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
    {
	if (prim->tbind[k] == PFGS_PER_VERTEX)
	    prim->numTextures = k + 1;
	else
	    return;
    }
}
/*
 * add an arbitrary pfdGeom to a pfdGeoBuilder
 */
PFDUDLLEXPORT void
pfdAddGeom(pfdGeoBuilder *bldr, pfdGeom *pb, int numGeom)
{
    pfdPrim tri = {0};
    int i, j, k;

    pfdPreprocessGeomTextureUnits (pb);

    if (!(pb->flags & PFD_INDEXED)) /* not indexed */
    {
	switch (pb->primtype) 
	{
 	case PFGS_POINTS:
	    pfdAddPoints(bldr, pb);
	    break;

	case PFGS_LINES:
	    pfdAddLines(bldr, pb);
	    break;

	case PFGS_LINESTRIPS:
	case PFGS_FLAT_LINESTRIPS:
	    pfdAddLineStrips(bldr, pb, numGeom);
	    break;

	case PFGS_TRIS:
	    tri.numTextures = pb->numTextures;
	    tri.nbind = pb->nbind;
	    tri.cbind = pb->cbind;
	    for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
		tri.tbind[i] = pb->tbind[i];
	    for (i = 0; i < pb->numVerts; ) 
	    {
		for (j = 0; j < 3; j++, i++) 
		{
		    /* position */
		    pfCopyVec3(tri.coords[j], pb->coords[i]);

		    /* color */
 		    if (tri.cbind == PFGS_PER_VERTEX)
			pfCopyVec4(tri.colors[j], pb->colors[i]);
		    else
		    if (tri.cbind == PFGS_PER_PRIM)
			pfCopyVec4(tri.colors[j], pb->colors[i/3]);
		    else 
		    if (tri.cbind == PFGS_OVERALL)
		    	pfCopyVec4(tri.colors[j], pb->colors[0]);

		    /* normal */
		    if (tri.nbind == PFGS_PER_VERTEX)
			pfCopyVec3(tri.norms[j], pb->norms[i]);
		    else
		    if (tri.nbind == PFGS_PER_PRIM)
			pfCopyVec3(tri.norms[j], pb->norms[i/3]);
		    else 
		    if (tri.nbind == PFGS_OVERALL)
			pfCopyVec3(tri.norms[j], pb->norms[0]);

		    /* texture */
		    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
			if (tri.tbind[k] == PFGS_PER_VERTEX)
			    pfCopyVec2(tri.texCoords[k][j], pb->texCoords[k][i]);
		}
		pfdAddTri(bldr, &tri);
	    }
	    break;

	case PFGS_POLYS:
	    pfdAddPoly(bldr, pb);
	    break;

   	default:
	    break;
	}
    }
    else 
    {
	switch(pb->primtype) 
	{
        case PFGS_POINTS:
            pfdAddIndexedPoints(bldr, pb);
            break;

        case PFGS_LINES:
            pfdAddIndexedLines(bldr, pb);
            break;

        case PFGS_LINESTRIPS:
        case PFGS_FLAT_LINESTRIPS:
            pfdAddIndexedLineStrips(bldr, pb, numGeom);
            break;

        case PFGS_TRIS:
	    tri.numTextures = pb->numTextures;
            tri.nbind = pb->nbind;
            tri.cbind = pb->cbind;
	    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
		tri.tbind[k] = pb->tbind[k];

            tri.coordList    = pb->coordList;
            tri.normList     = pb->normList;
            tri.colorList    = pb->colorList;

	    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
		tri.texCoordList[k] = pb->texCoordList[k];

            for (i = 0; i < pb->numVerts; ) 
	    {
                for (j = 0; j < 3; j++, i++) 
		{
		    /* position */
		    tri.icoords[j] = pb->icoords[i];

		    /* color */
                    if (tri.cbind == PFGS_PER_VERTEX)
			tri.icolors[j] = pb->icolors[i];
                    else
		    if (tri.cbind == PFGS_PER_PRIM)
			tri.icolors[j] = pb->icolors[i/3];
                    else 
		    if (tri.cbind == PFGS_OVERALL)
			tri.icolors[j] = pb->icolors[0];

		    /* normal */
                    if (tri.nbind == PFGS_PER_VERTEX)
			tri.inorms[j] = pb->inorms[i];
                    else
		    if (tri.cbind == PFGS_PER_PRIM)
			tri.inorms[j] = pb->inorms[i/3];
                    else 
		    if (tri.nbind == PFGS_OVERALL)
			tri.inorms[j] = pb->inorms[0];

		    /* texture */
		    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
			if (tri.tbind[k] == PFGS_PER_VERTEX)
			    tri.itexCoords[k][j] = pb->itexCoords[k][i];
                }
                pfdAddIndexedTri(bldr, &tri);
            }
            break;

        case PFGS_POLYS:
            pfdAddIndexedPoly(bldr, pb);
            break;

        default:
            break;
	}
    }
}

static pfdPrim *
getPrim(pfdGeoBuilder *bldr)
{
    pfdPrim	*t		= NULL;
    int		 oldChunkSize	= TRICHUNK;
    int		 newChunkSize	= 0;

    /* 
     * Create and link in a new chunk of triangles whose size is twice 
     * the size of the previous chunk. Use last triangle in chunk as link.
    */
    if (bldr->availTri->flags == LINK_TRI)
    {
    	if (bldr->availTri->next == bldr->triList)
	{
	    /* determine old chunk size */
	    t = bldr->triList;
	    while (&t[oldChunkSize-1] != bldr->availTri)
	    {
	        t = t[oldChunkSize-1].next;
	        oldChunkSize <<= 1;

		if (oldChunkSize > MAXTRICHUNK)
		    oldChunkSize >>= 1;
	    }

	    /* determine new chunk size */
	    newChunkSize = oldChunkSize << 1;

	    if (newChunkSize > MAXTRICHUNK)
		newChunkSize >>= 1;

#ifdef CHECK_ALLOCATIONS
	    {
		static int taTRIS = 0;

		pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
		    "Allocated %d tris (%d bytes), total of %f MB",
		    newChunkSize,
		    sizeof(pfdPrim),
		    newChunkSize*sizeof(pfdPrim)/(1024.0*1024.0));

		taTRIS += newChunkSize;

		pfAdd(bldr->mallocSizeList,
		    (void *)(newChunkSize*sizeof(pfdPrim)));
	    	pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
		    "Total Allocated tris = %d (Total MB: %f)",
		    taTRIS,
		    taTRIS*sizeof(pfdPrim)/(1024.*1024.));
	    }
#endif
	    bldr->availTri = t[oldChunkSize-1].next = 
		pfCalloc(newChunkSize, sizeof(pfdPrim), NULL);
	    pfAdd(bldr->mallocList,bldr->availTri);

	    /* Point last tri in chunk to head of linked list */
	    t = &bldr->availTri[newChunkSize-1];
	    t->next = bldr->triList;
	    t->flags = LINK_TRI;
	}
    	else
	    bldr->availTri = bldr->availTri->next;
    }

    t = bldr->availTri;
    bldr->availTri++;
    return t;
}

#ifdef	OLD_STYLE_FREE
static void
freeTris(pfdGeoBuilder *bldr)
{
    pfdPrim		*tri;
    int		chunkSize = TRICHUNK;
    static int FreedTris = 0;
    int prevFreedTris = FreedTris;

    /* Free linked list of triangle chunks */
    tri = bldr->triList;
    do
    {
	pfdPrim		*next;

	next = tri[chunkSize-1].next;
	pfFree(tri);
	FreedTris++;
	tri = next;

	chunkSize <<= 1;

	if (chunkSize > MAXTRICHUNK)
	    chunkSize >>= 1;
    } while (tri != bldr->triList);

    fprintf(stderr, "Freed another %d tris for a total of %d\n",
	FreedTris-prevFreedTris, FreedTris);
}
#endif

/* ###################################################################### */
/* #                                                                    # */
/* #       Add a pfdGeom of specified type to a pfdGeoBuilder           # */
/* #                                                                    # */
/* ###################################################################### */

void
pfdAddPoly(pfdGeoBuilder *bldr, pfdGeom *polygon)
{
    if (polygon == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddPoly() NULL polygon.");
	return;
    }

    if (polygon->numVerts < 3)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddPoly() "
	    "Degenerate polygon has %d vertices", polygon->numVerts);
	return;
    }

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessGeomTextureUnits (polygon);
    polygon->flags = 0;
    addPoly(bldr, polygon);
}

PFDUDLLEXPORT void
pfdAddIndexedPoly(pfdGeoBuilder *bldr, pfdGeom *polygon)
{
    int		t;

    if (polygon == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddIndexedPoly() NULL polygon.");
	return;
    }

    if (polygon->numVerts < 3)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddIndexedPoly() "
	    "Degenerate polygon has %d vertices", polygon->numVerts);
	return;
    }

    polygon->flags = PFD_INDEXED;
    
    bldr->coordList = polygon->coordList;
    bldr->normList = polygon->normList;
    bldr->colorList = polygon->colorList;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	bldr->texCoordList[t] = polygon->texCoordList[t];

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessGeomTextureUnits (polygon);
    addPoly(bldr, polygon);
}

PFDUDLLEXPORT void
pfdAddPoints(pfdGeoBuilder *bldr, pfdGeom *points)
{
    if (points == NULL)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddPoints() NULL Points.");
        return;
    }

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessGeomTextureUnits (points);
    points->flags = 0;
    addPoints(bldr, points);
}

PFDUDLLEXPORT void
pfdAddIndexedPoints(pfdGeoBuilder *bldr, pfdGeom *points)
{
    int		t;

    if (points == NULL)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddIndexedPoints() NULL Points.");
        return;
    }

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessGeomTextureUnits (points);

    points->flags = PFD_INDEXED;

    bldr->coordList = points->coordList;
    bldr->normList = points->normList;
    bldr->colorList = points->colorList;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	bldr->texCoordList[t] = points->texCoordList[t];

    addPoints(bldr, points);
}

PFDUDLLEXPORT void
pfdAddLines(pfdGeoBuilder *bldr, pfdGeom *lines)
{
    if (lines == NULL)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddLines() NULL lines.");
        return;
    }

    if (lines->numVerts < 2)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddLines() "
            "Degenerate line has %d vertices", lines->numVerts);
        return;
    }

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessGeomTextureUnits (lines);

    lines->flags = 0;
    addLines(bldr, lines);
}

PFDUDLLEXPORT void
pfdAddIndexedLines(pfdGeoBuilder *bldr, pfdGeom *lines)
{
    int		t; 

    if (lines == NULL)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddIndexedLines() NULL Lines.");
        return;
    }

    if (lines->numVerts < 2)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddIndexedLines() "
            "Degenerate line has %d vertices", lines->numVerts);
        return;
    }

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessGeomTextureUnits (lines);

    lines->flags = PFD_INDEXED;

    bldr->coordList = lines->coordList;
    bldr->normList = lines->normList;
    bldr->colorList = lines->colorList;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	bldr->texCoordList[t] = lines->texCoordList[t];

    addLines(bldr, lines);
}

PFDUDLLEXPORT void
pfdAddLineStrips(pfdGeoBuilder *bldr, pfdGeom *strip, int num)
{
    pfdGeom *tmp;
    int i;

    if (strip == NULL)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddLines() NULL lines.");
        return;
    }

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessGeomTextureUnits (strip);

    for (i=0, tmp=strip; i< num; i++, tmp=tmp->next) {
    if (tmp->numVerts < 2)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddLines() "
            "Degenerate line has %d vertices", tmp->numVerts);
        return;
    }

    tmp->flags = 0;
    }

    addLineStrips(bldr, strip, num);
}

/* user can't delete strip->coordList/normList/colorList/texCoordList 
 * because we do not duplicate them in the builder bldr 
 */
PFDUDLLEXPORT void
pfdAddIndexedLineStrips(pfdGeoBuilder *bldr, pfdGeom *strip, int num)
{
    pfdGeom *tmp;
    int i, t;

    for (i=0, tmp=strip; i<num; i++, tmp=tmp->next)
    {
    if (tmp == NULL)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddIndexedLines() NULL Lines.");
        return;
    }

    if (tmp->numVerts < 2)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddIndexedLines() "
            "Degenerate line has %d vertices", tmp->numVerts);
        return;
    }

    tmp->flags = PFD_INDEXED;
    }

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessGeomTextureUnits (strip);

    bldr->coordList = strip->coordList;
    bldr->normList = strip->normList;
    bldr->colorList = strip->colorList;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	bldr->texCoordList[t] = strip->texCoordList[t];

    addLineStrips(bldr, strip, num);
}

/* ###################################################################### */
/* #                                                                    # */
/* #       Add a pfdPrim of specified type to a pfdGeoBuilder           # */
/* #                                                                    # */
/* ###################################################################### */

PFDUDLLEXPORT void 
pfdAddPoint(pfdGeoBuilder *bldr, pfdPrim *point)
{
    pfdPrim *p;

    if (point == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddPoint() NULL point.");
	return;
    }

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessPrimTextureUnits (point);

    point->flags = 0;

    p = getPrim(bldr);
    copyPrim(point, p);

    addPoint(bldr, p);
}

PFDUDLLEXPORT void
pfdAddLine(pfdGeoBuilder *bldr, pfdPrim *line)
{
    pfdPrim *l;

    if (line == NULL)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdAddLine() NULL line.");
        return;
    }

    /* Do not add line if it is a point */
    if (pfEqualVec3(line->coords[0], line->coords[1]))
	return;

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessPrimTextureUnits (line);

    line->flags = 0;

    l = getPrim(bldr);
    copyPrim(line, l);

    addLine(bldr, l);
}

PFDUDLLEXPORT void
pfdAddTri(pfdGeoBuilder *bldr, pfdPrim *t)
{
    pfdPrim *tri;

    if (bldr == NULL || t == NULL)
	return;

    /* Do not add tri if any two verts are the same */
    if (pfEqualVec3(t->coords[0], t->coords[1]) || 
	pfEqualVec3(t->coords[1], t->coords[2]) || 
	pfEqualVec3(t->coords[2], t->coords[0]))
	return;

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessPrimTextureUnits (t);

    t->flags = 0;

    tri = getPrim(bldr);
    copyPrim(tri, t);

    addTri(bldr, tri);
}

PFDUDLLEXPORT void
pfdAddIndexedTri(pfdGeoBuilder *bldr, pfdPrim *t)
{
    pfdPrim	*tri;
    pfVec3	*c;
    int		k;

    if (bldr == NULL || t == NULL)
	return;

    c = t->coordList;

    /* Do not add tri if any two verts are the same */
    if (pfEqualVec3(c[t->icoords[0]], c[t->icoords[1]]) || 
	pfEqualVec3(c[t->icoords[1]], c[t->icoords[2]]) || 
	pfEqualVec3(c[t->icoords[2]], c[t->icoords[0]]))
	return;

    /*
     *	Yair: Compute number of active texture units on input polygon.
     */
    pfdPreprocessPrimTextureUnits (t);

    t->flags = PFD_INDEXED;

    bldr->coordList = t->coordList;
    bldr->normList = t->normList;
    bldr->colorList = t->colorList;
    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
	bldr->texCoordList[k] = t->texCoordList[k];

    tri = getPrim(bldr);
    *tri = *t;

    addIndexedTri(bldr, tri);
}

/* ###################################################################### */
/* #                                                                    # */
/* #      Private helper functions to add points to a pfdGeoBuilder     # */
/* #                                                                    # */
/* ###################################################################### */

static void
addPoint(pfdGeoBuilder *bldr, pfdPrim *point)
{
    pfdPrim 	*tmp;
    int 	 i;
    int 	 np;

    pfdPreprocessPrimTextureUnits(point);

    if (point->cbind == PFGS_PER_PRIM)
	point->cbind = PFGS_PER_VERTEX;
    if (point->nbind == PFGS_PER_PRIM)
	point->nbind = PFGS_PER_VERTEX;

    np = bldr->pointCounts[point->nbind][point->cbind][point->numTextures];

    /* Add point to appropriate geoset bin */
    tmp = bldr->pointBins[point->nbind][point->cbind][point->numTextures];

    /* sort by width */
    if (np == 0 || (tmp->pixelsize <= point->pixelsize)) 
    {
        bldr->pointBins[point->nbind][point->cbind][point->numTextures] = point;
        point->next = tmp;
    }
    else
    if (np == 1)
    {
	point->next = tmp->next;
	tmp->next = point;
    }
    else
    {
	/* sort by width */
        for (i=np; i>1; i--) 
	{
            if (tmp->next->pixelsize > point->pixelsize) 
                tmp = tmp->next;
	    else
		break;
        }
        point->next = tmp->next;
        tmp->next = point;
    }
    bldr->pointCounts[point->nbind][point->cbind][point->numTextures]++;
    bldr->numPoints++;
}

static void
addIndexedPoint(pfdGeoBuilder *bldr, pfdPrim *point)
{
    pfdPrim *tmp;
    int i;

    pfdPreprocessPrimTextureUnits(point);

    if (point->cbind == PFGS_PER_PRIM)
	point->cbind = PFGS_PER_VERTEX;
    if (point->nbind == PFGS_PER_PRIM)
	point->nbind = PFGS_PER_VERTEX;

    /* Add point to appropriate geoset bin */
    tmp = bldr->indexedPointBins[point->nbind][point->cbind][point->numTextures];

    /* sort by point size */
    if ((bldr->indexedPointCounts[point->nbind][point->cbind][point->numTextures]==0) ||
	(tmp->pixelsize <= point->pixelsize)) 
    {
        bldr->indexedPointBins[point->nbind][point->cbind][point->numTextures] = point;
        point->next = tmp;
    }
    else
    if (bldr->indexedPointCounts[point->nbind][point->cbind][point->numTextures]==1)
    {
	point->next = tmp->next;
	tmp->next = point;
    }
    else
    {
        for (i=bldr->indexedPointCounts[point->nbind][point->cbind][point->numTextures]; i>1; i--) 
	{
	    /* sort by point size */
            if (tmp->next->pixelsize > point->pixelsize) 
                tmp = tmp->next;
	    else
		break;
        }
        point->next = tmp->next;
        tmp->next = point;
    }

    bldr->indexedPointCounts[point->nbind][point->cbind][point->numTextures]++;
    bldr->numIndexedPoints++;
}

static void
addPoints(pfdGeoBuilder *bldr, pfdGeom *points)
{
    pfdPrim	*tri;
    pfdPrim	*triList;
    int 	 i;

    pfdPreprocessGeomTextureUnits(points);

    /* build up linked list of points */
    triList = NULL;
    for (i=0; i<points->numVerts; i++)
    {
        tri = getPrim(bldr);
        tri->next = triList;
        triList = tri;
    }

    /* fill in triList with points */
    tri = triList;
    for (i=0; i<points->numVerts; i++)
    {
        copyGeom(tri, points, i, 0, 0);
        tri->pixelsize = points->pixelsize; 
        tri = tri->next;
    }

    while (triList)
    {
        pfdPrim *next;

        next = triList->next;
        if (points->flags & PFD_INDEXED)
            addIndexedPoint(bldr, triList);
        else
            addPoint(bldr, triList);
        triList = next;
    }
}

/* ###################################################################### */
/* #                                                                    # */
/* #      Private helper functions to add lines to a pfdGeoBuilder      # */
/* #                                                                    # */
/* ###################################################################### */

static void
addLine(pfdGeoBuilder *bldr, pfdPrim *line)
{
    pfdPrim *tmp;
    int i;

    pfdPreprocessPrimTextureUnits(line);

    /* Add line to appropriate geoset bin */
    tmp = bldr->lineBins[line->nbind][line->cbind][line->numTextures];

    /* sort by width */
    if ((bldr->lineCounts[line->nbind][line->cbind][line->numTextures]==0) 
	|| (tmp->pixelsize <= line->pixelsize))
    {
        bldr->lineBins[line->nbind][line->cbind][line->numTextures] = line;
        line->next = tmp;
    }
    else
    if (bldr->lineCounts[line->nbind][line->cbind][line->numTextures]==1)
    {
	line->next = tmp->next;
	tmp->next = line;
    }
    else
    {
	/* sort by width */
        for (i=bldr->lineCounts[line->nbind][line->cbind][line->numTextures]; i>1; i--) 
	{
            if (tmp->next->pixelsize > line->pixelsize) 
                tmp = tmp->next;
	    else
		break;
        }
        line->next = tmp->next;
        tmp->next = line;
    }

    bldr->lineCounts[line->nbind][line->cbind][line->numTextures]++;
    bldr->numLines++;
}

static void
addIndexedLine(pfdGeoBuilder *bldr, pfdPrim *line)
{
    pfdPrim *tmp;
    int i;

    pfdPreprocessPrimTextureUnits(line);

    /* Add line to appropriate geoset bin */
    tmp = bldr->indexedLineBins[line->nbind][line->cbind][line->numTextures];
    if ((bldr->indexedLineCounts[line->nbind][line->cbind][line->numTextures]==0) ||
	(tmp->pixelsize <= line->pixelsize)) /* sort by width */
    {
	bldr->indexedLineBins[line->nbind][line->cbind][line->numTextures] = line;
	line->next = tmp;
    }
    else
    if (bldr->indexedLineCounts[line->nbind][line->cbind][line->numTextures]==1)
    {
	line->next = tmp;
	tmp->next = line;
    }
    else 
    {
    	for (i=bldr->indexedLineCounts[line->nbind][line->cbind][line->numTextures]; i>1; i--) 
	{
	    /* sort by width */
	    if (tmp->next->pixelsize > line->pixelsize) 
	    	tmp = tmp->next;
	    else
		break;
	}
	line->next = tmp->next;
	tmp->next = line;
    }

    bldr->indexedLineCounts[line->nbind][line->cbind][line->numTextures]++;
    bldr->numIndexedLines++;
}

static void
addLines(pfdGeoBuilder *bldr, pfdGeom *lines)
{
    pfdPrim	*triList, *tri;
    int		i;
    int		numVerts = lines->numVerts;
    pfdGeom	*dup = NULL;

    if (lines->numVerts < 2)
	return;

    pfdPreprocessGeomTextureUnits(lines);
	
#ifdef CHECK_DUPLICATE_LINES
#ifdef CHECK_DUPLICATE
    for (i=0; i<lines->numVerts; i++)
    {
	pfuHashElt  he;

	if (lines->flags & PFD_INDEXED)
	    he.data = (void*)lines->coordList[lines->icoords[i]];
	else
	    he.data = (void*)lines->coords[i];

	/* Remove duplicate vertex */
	if (pfuEnterHash(bldr->hashTable, &he))
	{
	    int    j, k;

	    /* make a copy of entire pfdGeom before first modification */
	    if (dup == NULL)
	    {
		dup = pfdNewGeom(lines->numVerts);
		pfdCopyGeom(dup, lines);
	    }

	    lines->numVerts--;
	    for (j=i; j<lines->numVerts; j++)
	    {
		if (lines->flags & PFD_INDEXED)
		{
		    lines->icoords[j] = lines->icoords[j+1];

		    if (lines->nbind == PFGS_PER_VERTEX)
			lines->inorms[j] = lines->inorms[j+1];

		    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
			if (lines->tbind[k] == PFGS_PER_VERTEX)
			    lines->itexCoords[k][j] = lines->itexCoords[k][j+1];

		    if (lines->cbind == PFGS_PER_VERTEX)
			lines->icolors[j] = lines->icolors[j+1];
		}
		else
		{
		    pfCopyVec3(lines->coords[j], lines->coords[j+1]);

		    if (lines->nbind == PFGS_PER_VERTEX)
			pfCopyVec3(lines->norms[j], lines->norms[j+1]);

		    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
			if (lines->tbind[k] == PFGS_PER_VERTEX)
			    pfCopyVec2(lines->texCoords[k][j], 
						    lines->texCoords[k][j+1]);

		    if (lines->cbind == PFGS_PER_VERTEX)
			pfCopyVec4(lines->colors[j], lines->colors[j+1]);
		}
	    }
	    i--;
	}
    }
    pfuResetHTable(bldr->hashTable);
#endif
#endif

    if (lines->numVerts < 2)
    {
	/* restore original pfdGeom */
	pfdCopyGeom(lines, dup);
	pfdDelGeom(dup);
	return;
    }
	
    /* Build up linked list of lines */
    triList = NULL;
    for (i=0; i<lines->numVerts/2; i++)
    {
	tri = getPrim(bldr);
	tri->next = triList;
	triList = tri;
    }

    /* Fill in triList with lines */
    tri = triList;
    for (i=0; i<lines->numVerts; i=i+2)
    {
	copyGeom(tri, lines, i, i+1, 0);
	tri->pixelsize = lines->pixelsize; 
	tri = tri->next;
    }

    while (triList) 
    { 
	pfdPrim *next;

	next = triList->next;
	if (lines->flags & PFD_INDEXED)
	    addIndexedLine(bldr, triList);
	else
	    addLine(bldr, triList);
	triList = next;
    }

    if (dup != NULL)
    {
	/* restore original pfdGeom */
	pfdCopyGeom(lines, dup);
	pfdDelGeom(dup);
	return;
    }
}

static void
addLineStrip(pfdGeoBuilder *bldr, pfdGeom *strip)
{
    pfdGeom *tmp;
    int i;

    pfdPreprocessGeomTextureUnits(strip);

    /* Add line to appropriate geoset bin */
    if (strip->primtype & PFGS_LINESTRIPS) 
    {
    	tmp = bldr->linestripBins[strip->nbind][strip->cbind][strip->numTextures];

	/* sort by width */
    	if ((bldr->linestripCounts[strip->nbind][strip->cbind][strip->numTextures]==0) ||
		(tmp->pixelsize <= strip->pixelsize)) 
    	{
        	bldr->linestripBins[strip->nbind][strip->cbind][strip->numTextures] = strip;
        	strip->next = tmp;
    	}
    	else
	if (bldr->linestripCounts[strip->nbind][strip->cbind][strip->numTextures] == 1)
	{
		strip->next = tmp->next;
	    	tmp->next = strip;
	}
	else
    	{
	    for (i=bldr->linestripCounts[strip->nbind][strip->cbind][strip->numTextures]; i>1; i--) 
	    {
		/* sort by width */
		if (tmp->next->pixelsize > strip->pixelsize) 
		    tmp = tmp->next;
		else
		    break;
	    }
	    strip->next = tmp->next;
	    tmp->next = strip;
    	}
	
    	bldr->linestripCounts[strip->nbind][strip->cbind][strip->numTextures]++;
    	bldr->numLines += strip->numVerts-1;
    }
    else 
    {
    	tmp = bldr->flatlinestripBins[strip->nbind][strip->cbind][strip->numTextures];

	/* sort by width */
    	if ((bldr->flatlinestripCounts[strip->nbind][strip->cbind][strip->numTextures]==0) ||
	    (tmp->pixelsize <= strip->pixelsize)) 
    	{
	    bldr->flatlinestripBins[strip->nbind][strip->cbind][strip->numTextures] = strip;
	    strip->next = tmp;
    	}
    	else
	if (bldr->flatlinestripCounts[strip->nbind][strip->cbind][strip->numTextures] == 1)
	{
	    strip->next = tmp;
	    tmp->next = strip;
	}
	else
    	{
	    for (i=bldr->flatlinestripCounts[strip->nbind][strip->cbind][strip->numTextures]; i>1; i--) 
	    {
		/* sort by width */
            	if (tmp->next->pixelsize > strip->pixelsize) 
                	tmp = tmp->next;
		else
		    break;
	    }
	    strip->next = tmp->next;
	    tmp->next = strip;
    	}
	
    	bldr->flatlinestripCounts[strip->nbind][strip->cbind][strip->numTextures]++;
    	bldr->numLines += strip->numVerts-1;
    }
}

static void
addIndexedLineStrip(pfdGeoBuilder *bldr, pfdGeom *strip)
{
    pfdGeom *tmp;
    int i;

    pfdPreprocessGeomTextureUnits(strip);

    /* Add line to appropriate geoset bin */
    if (strip->primtype & PFGS_LINESTRIPS) 
    {
    	tmp = bldr->indexedlinestripBins[strip->nbind][strip->cbind][strip->numTextures];

	/* sort by width */
    	if ((bldr->indexedlinestripCounts[strip->nbind][strip->cbind][strip->numTextures]==0) ||
	    (tmp->pixelsize <= strip->pixelsize)) 
    	{
	    bldr->indexedlinestripBins[strip->nbind][strip->cbind][strip->numTextures] = strip;
	    strip->next = tmp;
    	}
    	else 
	if (bldr->indexedlinestripCounts[strip->nbind][strip->cbind][strip->numTextures] == 1)
	{
	    strip->next = tmp;
	    tmp->next = strip;
	}
	else
    	{
	    for (i=bldr->indexedlinestripCounts[strip->nbind][strip->cbind][strip->numTextures]; i>1; i--) 
	    {
		/* sort by width */
            	if (tmp->next->pixelsize > strip->pixelsize) 
		    tmp = tmp->next;
		else
		    break;
	    }
	    strip->next = tmp->next;
	    tmp->next = strip;
    	}
	
    	bldr->indexedlinestripCounts[strip->nbind][strip->cbind][strip->numTextures]++;
    	bldr->numIndexedLines += strip->numVerts-1;
    }
    else
    {        
	tmp = bldr->indexedflatlinestripBins[strip->nbind][strip->cbind][strip->numTextures];

	/* sort by width */
        if ((bldr->indexedflatlinestripCounts[strip->nbind][strip->cbind][strip->numTextures]==0)||
		(tmp->pixelsize <= strip->pixelsize)) 
        {
	    bldr->indexedflatlinestripBins[strip->nbind][strip->cbind][strip->numTextures] = strip;
	    strip->next = tmp;
        }
        else
	if (bldr->indexedflatlinestripCounts[strip->nbind][strip->cbind][strip->numTextures] == 1)
	{
	    strip->next = tmp;
	    tmp->next = strip;
	}
	else
        {
	    for (i=bldr->indexedflatlinestripCounts[strip->nbind][strip->cbind][strip->numTextures]; i>1; i--) 
	    {
		/* sort by width */
                if (tmp->next->pixelsize > strip->pixelsize) 
		    tmp = tmp->next;
		else
		    break;
	    }
	    strip->next = tmp->next;
	    tmp->next = strip;
        }

        bldr->indexedflatlinestripCounts[strip->nbind][strip->cbind][strip->numTextures]++;
        bldr->numIndexedLines += strip->numVerts-1;
    }
}

static void
addLineStrips(pfdGeoBuilder *bldr, pfdGeom *strip, int num)
{
    pfdGeom	*tmp, *tmp2;
    int		i;

    pfdPreprocessGeomTextureUnits(strip);

    /* all line strips are either indexed or regular */
    tmp = (pfdGeom *)pfdNewGeom(strip->numVerts);
    copyLineStrip(tmp, strip);
    if (tmp->flags & PFD_INDEXED)
    {
	addIndexedLineStrip(bldr, tmp);
    	for (i=1; i<num; i++) 
	{
	    tmp2 = tmp->next;
    	    tmp = (pfdGeom *)pfdNewGeom(tmp2->numVerts);
	    copyLineStrip(tmp, tmp2);
	    addIndexedLineStrip(bldr, tmp);
	}
    }
    else
    {
        addLineStrip(bldr, tmp);
	for (i=1; i<num; i++) 
	{
	    tmp2 = tmp->next;
    	    tmp = (pfdGeom *)pfdNewGeom(tmp2->numVerts);
	    copyLineStrip(tmp, tmp2);
	    addLineStrip(bldr, tmp);
	}
    }
}

/* ###################################################################### */
/* #                                                                    # */
/* #    Private helper functions to add polygons to a pfdGeoBuilder     # */
/* #                                                                    # */
/* ###################################################################### */

static void
addTri(pfdGeoBuilder *bldr, pfdPrim *tri)
{
    pfdPrim *tmp;

    pfdPreprocessPrimTextureUnits(tri);

    /* Add tri to appropriate geoset bin */
    tmp = bldr->triBins[tri->nbind][tri->cbind][tri->numTextures];

    if (bldr->triCounts[tri->nbind][tri->cbind][tri->numTextures]==0) 
    {
     	bldr->triBins[tri->nbind][tri->cbind][tri->numTextures] = tri;
     	tri->next = tmp;
    }
    else 
    {	
	tri->next = tmp->next;
	tmp->next = tri;
    }

    bldr->triCounts[tri->nbind][tri->cbind][tri->numTextures]++;
    bldr->numTris++;
}

static void
addIndexedTri(pfdGeoBuilder *bldr, pfdPrim *tri)
{
    pfdPrim *tmp;

    pfdPreprocessPrimTextureUnits(tri);

    /* Add tri to appropriate geoset bin */
    tmp = bldr->indexedTriBins[tri->nbind][tri->cbind][tri->numTextures];

    if (bldr->indexedTriCounts[tri->nbind][tri->cbind][tri->numTextures]==0)
    {
        bldr->indexedTriBins[tri->nbind][tri->cbind][tri->numTextures] = tri;
        tri->next = tmp;
    }
    else
    {
        tri->next = tmp->next;
        tmp->next = tri;
    }

    bldr->indexedTriCounts[tri->nbind][tri->cbind][tri->numTextures]++;
    bldr->numIndexedTris++;
}

static void
addPoly(pfdGeoBuilder *bldr, pfdGeom *polygon)
{
    pfdPrim	*triList, *tri;
    int		i, ntris;
    pfdGeom	*dup = NULL;
    int		k;
    
    if (polygon->numVerts < 3)
	return;

    pfdPreprocessGeomTextureUnits(polygon);

#ifdef CHECK_DUPLICATE
    for (i=0; i<polygon->numVerts; i++)
    {
	pfuHashElt  		he;

	if (polygon->flags & PFD_INDEXED)
	    he.data = (void*)polygon->coordList[polygon->icoords[i]];
	else
	    he.data = (void*)polygon->coords[i];

	/* Remove duplicate vertex */
	if (pfuEnterHash(bldr->hashTable, &he))
	{
	    int    j;

	    /* make a copy of entire pfdGeom before first modification */
	    if (dup == NULL)
	    {
		dup = pfdNewGeom(polygon->numVerts);
		pfdCopyGeom(dup, polygon);
	    }

	    polygon->numVerts--;
	    for (j=i; j<polygon->numVerts; j++)
	    {
		if (polygon->flags & PFD_INDEXED)
		{
		    polygon->icoords[j] = polygon->icoords[j+1];

		    if (polygon->nbind == PFGS_PER_VERTEX)
			polygon->inorms[j] = polygon->inorms[j+1];

		    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
			if (polygon->tbind[k] == PFGS_PER_VERTEX)
			    polygon->itexCoords[k][j] = 
					polygon->itexCoords[k][j+1];

		    if (polygon->cbind == PFGS_PER_VERTEX)
			polygon->icolors[j] = polygon->icolors[j+1];
		}
		else
		{
		    pfCopyVec3(polygon->coords[j], polygon->coords[j+1]);

		    if (polygon->nbind == PFGS_PER_VERTEX)
			pfCopyVec3(polygon->norms[j], polygon->norms[j+1]);

		    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
			if (polygon->tbind[k] == PFGS_PER_VERTEX)
			    pfCopyVec2(polygon->texCoords[k][j], 
				       polygon->texCoords[k][j+1]);

		    if (polygon->cbind == PFGS_PER_VERTEX)
			pfCopyVec4(polygon->colors[j], polygon->colors[j+1]);
		}
	    }
	    i--;
	}
    }
    pfuResetHTable(bldr->hashTable);
#endif

    if (polygon->numVerts < 3)
    {
	/* restore original pfdGeom */
	pfdCopyGeom(polygon, dup);
	pfdDelGeom(dup);
	return;
    }

    /* Build up linked list of triangles */
    triList = NULL;
    for (i=0; i<polygon->numVerts-2; i++)
    {
	tri = getPrim(bldr);
	tri->next = triList;
	triList = tri;
    }

    /* Fill in triList with decomposed 'polygon' */
    ntris = triangulatePoly(polygon, triList);

    for (i=0; i<ntris; i++)
    { 
	pfdPrim *next;

	next = triList->next;
	if (polygon->flags & PFD_INDEXED)
	    addIndexedTri(bldr, triList);
	else
	    addTri(bldr, triList);
	triList = next;
    }

    /* restore original pfdGeom */
    if (dup != NULL)
    {
	pfdCopyGeom(polygon, dup);
	pfdDelGeom(dup);
    }
}

/* ###################################################################### */
/* #                                                                    # */
/* #                      pfdGeom Functions                             # */
/* #                                                                    # */
/* ###################################################################### */

/*
 * allocate and initialize a new pfdGeom structure data
 */
PFDUDLLEXPORT pfdGeom *
pfdNewGeom(int numV)
{
    pfdGeom *p = NULL;
    void *arena = pfGetSharedArena();
    int		i;
    int		t;

    /* set default primitive buffer size if needed */
    if (numV < 1)
	numV = 256;

    /* allocate main buffer structure */
    p = (pfdGeom *)pfCalloc(1, sizeof(pfdGeom), arena);

    /* initialize scalar data */
    p->flags	  = 0;
    p->nbind      = 0;
    p->cbind      = 0;
    for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	p->tbind[i]      = 0;
    p->numVerts   = numV;
    p->primtype   = 0;
    p->pixelsize  = 0.0f;
    p->next       = NULL;

    /* allocate arrays of length numV for each buffer component */
    p->coords     = (pfVec3 *)pfCalloc(numV, sizeof(pfVec3), arena);
    p->norms      = (pfVec3 *)pfCalloc(numV, sizeof(pfVec3), arena);
    p->colors     = (pfVec4 *)pfCalloc(numV, sizeof(pfVec4), arena);
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	p->texCoords[t]  = (pfVec2 *)pfCalloc(numV, sizeof(pfVec2), arena);

    p->coordList    = NULL;
    p->normList     = NULL;
    p->colorList    = NULL;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	p->texCoordList[t] = NULL;

    p->icoords    = (unsigned short *)pfCalloc(numV, sizeof(unsigned short), arena);
    p->inorms     = (unsigned short *)pfCalloc(numV, sizeof(unsigned short), arena);
    p->icolors    = (unsigned short *)pfCalloc(numV, sizeof(unsigned short), arena);
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	p->itexCoords[t] = (unsigned short *)pfCalloc(numV, sizeof(unsigned short), arena);

    /* return address of new pfdGeom */
    return p;
}

/*
 * resize internal pfdGeom data (keep old values intact where possible)
 */
PFDUDLLEXPORT void
pfdResizeGeom(pfdGeom *p, int numV)
{
    int	t;

    /* bail out if given a null pointer */
    if (p == NULL) 
	return;

    /* set default primitive buffer size if needed */
    if (numV < 1)
	numV = 256;
    
    /* no action needed if old and new sizes are the same */
    if (p->numVerts == numV)
	return;

    /* reallocate each buffer component array to length numV */
    p->coords     = (pfVec3 *)pfRealloc(p->coords,     numV*sizeof(pfVec3));
    p->norms      = (pfVec3 *)pfRealloc(p->norms,      numV*sizeof(pfVec3));
    p->colors     = (pfVec4 *)pfRealloc(p->colors,     numV*sizeof(pfVec4));
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	p->texCoords[t]  = (pfVec2 *)pfRealloc(
				p->texCoords[t],  numV*sizeof(pfVec2));

    p->icoords    = (unsigned short *)pfRealloc(p->icoords,    numV*sizeof(unsigned short));
    p->inorms     = (unsigned short *)pfRealloc(p->inorms,     numV*sizeof(unsigned short));
    p->icolors    = (unsigned short *)pfRealloc(p->icolors,    numV*sizeof(unsigned short));
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	p->itexCoords[t] = (unsigned short *)pfRealloc(
				p->itexCoords[t], numV*sizeof(unsigned short));

    /* update array-size information */
    p->numVerts   = numV;
}

/*
 * delete pfdGeom structure
 */
PFDUDLLEXPORT void
pfdDelGeom(pfdGeom *p)
{
    int		t;

    if (p != NULL) 
    {
	if (p->coords     != NULL) pfFree(p->coords    );
	if (p->norms      != NULL) pfFree(p->norms     );
	if (p->colors     != NULL) pfFree(p->colors    );
	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    if (p->texCoords[t]  != NULL) pfFree(p->texCoords[t] );

	if (p->icoords    != NULL) pfFree(p->icoords   );
	if (p->inorms     != NULL) pfFree(p->inorms    );
	if (p->icolors    != NULL) pfFree(p->icolors   );
	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    if (p->itexCoords[t] != NULL) pfFree(p->itexCoords[t]);

	pfFree(p);
    }
}

/*
 * copy pfdGeom contents
 */
PFDUDLLEXPORT void
pfdCopyGeom(pfdGeom *dst, pfdGeom *src)
{
    int v;
    int	i;

    /* check arguments */
    if (dst == NULL || src == NULL)
	return;
    if (dst == src)
	return;
    
#if 0
    /* *** NO, LET'S ASK THE USER TO PROVIDE A BIG-ENOUGH DEST *** */
    /* make sure destination pfdGeom is big enough */
    if (dst->numVerts < src->numVerts)
	pfdResizeGeom(dst, src->numVerts);
#endif

    /* perform the copy */
    dst -> numTextures = src -> numTextures;
    dst->flags	    = src->flags;
    dst->nbind      = src->nbind;
    dst->cbind      = src->cbind;
    for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	dst->tbind[i]      = src->tbind[i];
    dst->numVerts   = src->numVerts;
    dst->primtype   = src->primtype;
    dst->pixelsize  = src->pixelsize;
    dst->next       = src->next;

    dst->coordList    = src->coordList;
    dst->normList     = src->normList;
    dst->colorList    = src->colorList;
    for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	dst->texCoordList[i] = src->texCoordList[i];

    for (v = 0; v < src->numVerts; v++)
    {
	pfCopyVec3(dst->coords[v],    src->coords[v]);
	pfCopyVec3(dst->norms[v],     src->norms[v]);
	pfCopyVec4(dst->colors[v],    src->colors[v]);
	for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	    if (dst->texCoords[i] && src->texCoords[i])
		pfCopyVec2(dst->texCoords[i][v], src->texCoords[i][v]);

	dst->icoords[v]    = src->icoords[v];
	dst->inorms[v]     = src->inorms[v];
	dst->icolors[v]    = src->icolors[v];
	for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	    if(src->itexCoords[i] && dst->itexCoords[i])
	        dst->itexCoords[i][v] = src->itexCoords[i][v];
    }
}

/* 
 * reverse polygon vertex order (CW -> CCW)
 */
PFDUDLLEXPORT int
pfdReverseGeom(pfdGeom *polygon)
{
    int          i;
    int          j;
    int		 k;
    int		 half;

    /* can only reverse existing data */
    if (polygon == NULL)
	return 0;

    /* can only deal with facets */
    switch (polygon->primtype)
    {
    case PFGS_POINTS:
    case PFGS_LINES:
    case PFGS_LINESTRIPS:
    case PFGS_FLAT_LINESTRIPS:
	return 0;

    case PFGS_TRIS:
    case PFGS_QUADS:
    case PFGS_TRISTRIPS:
    case PFGS_FLAT_TRISTRIPS:
    case PFGS_POLYS:
	break;
    }

    half = polygon->numVerts/2;

    /* reverse vertex data */
    for (i = 0; i < half; i++)
    {
        pfVec3 temp;
        j = polygon->numVerts-i-1;
        pfCopyVec3(temp, polygon->coords[i]);
        pfCopyVec3(polygon->coords[i], polygon->coords[j]);
        pfCopyVec3(polygon->coords[j], temp);
    }

    /* reverse per-vertex normal data (if supplied) */
    if (polygon->nbind == PFGS_PER_VERTEX)
        for (i = 0; i < half; i++)
        {
            pfVec3 temp;
            j = polygon->numVerts-i-1;
            pfCopyVec3(temp, polygon->norms[i]);
            pfCopyVec3(polygon->norms[i], polygon->norms[j]);
            pfCopyVec3(polygon->norms[j], temp);
        }

    /* reverse per-vertex color data (if supplied) */
    if (polygon->cbind == PFGS_PER_VERTEX)
        for (i = 0; i < half; i++)
        {
            pfVec4 temp;
            j = polygon->numVerts-i-1;
            pfCopyVec4(temp, polygon->colors[i]);
            pfCopyVec4(polygon->colors[i], polygon->colors[j]);
            pfCopyVec4(polygon->colors[j], temp);
        }

    /* reverse per-vertex texture coordinates (if supplied) */
    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
	if (polygon->tbind[k] == PFGS_PER_VERTEX)
	    for (i = 0; i < half; i++)
	    {
		pfVec2 temp;
		j = polygon->numVerts-i-1;
		pfCopyVec2(temp, polygon->texCoords[k][i]);
		pfCopyVec2(polygon->texCoords[k][i], polygon->texCoords[k][j]);
		pfCopyVec2(polygon->texCoords[k][j], temp);
	    }
 
    return 1;
}

/* 
 * Orient polygons so their vertex ordering matches the supplied normals. 
 */
PFDUDLLEXPORT int
pfdOrientGeom(pfdGeom *polygon, int orientMode)
{
    int 	 i;
    int 	 j;
    int		 reversed;
    float	 xi, xj;
    float	 yi, yj;
    float	 zi, zj;
    pfVec3	 supplied;
    pfVec3	 computed;
#ifdef	HOPE_FOR_THE_BEST
    pfVec3	 edge0; 
    pfVec3	 edge1;	
#endif

    /* can only deal with valid data */
    if (polygon == NULL)
	return 0;

    /* test for known method of orientation */
    if ((orientMode != PFDGBLDR_ORIENT_NORMALS ) &&
        (orientMode != PFDGBLDR_ORIENT_VERTICES))
	return 0;

    /* check primitive type */
    switch (polygon->primtype)
    {
    /* can only deal with facets */
    case PFGS_POINTS:
    case PFGS_LINES:
    case PFGS_LINESTRIPS:
    case PFGS_FLAT_LINESTRIPS:
	return 0;

    /* line strips are difficult */
    case PFGS_TRISTRIPS:
    case PFGS_FLAT_TRISTRIPS:
	return 0;

    case PFGS_TRIS:
    case PFGS_QUADS:
    case PFGS_POLYS:
	break;
    }

    /* can only deal with three or more vertices */
    if (polygon->numVerts < 3)
	return 0;

    /* get average normal (may be unnormalized) */
    switch (polygon->nbind)
    {
    case PFGS_PER_VERTEX:
	/* average all supplied normals */
	pfCopyVec3(supplied, polygon->norms[0]);
	for (i = 1; i < polygon->numVerts; i++)
	    pfAddVec3(supplied, supplied, polygon->norms[i]);
	break;

    case PFGS_PER_PRIM:
    case PFGS_OVERALL:
	/* use the supplied normal */
	pfCopyVec3(supplied, polygon->norms[0]);
	break;
    
    default:
	/* can only perform reorientation if normals are supplied */
	return 0;
	break;
    }
    
#ifdef	HOPE_FOR_THE_BEST
    /* form (unnormalized) normal using first three vertices */
    pfSubVec3(edge0, polygon->coords[1], polygon->coords[0]);
    pfSubVec3(edge1, polygon->coords[2], polygon->coords[0]);
    pfCrossVec3(computed, edge0, edge1);
#else
    /* form (unnormalized) normal using Newell's projected area */
    pfSetVec3(computed, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < polygon->numVerts; i++)
    {
	j = (i + 1) % polygon->numVerts;

	xi = polygon->coords[i][PF_X];
	xj = polygon->coords[j][PF_X];
	yi = polygon->coords[i][PF_Y];
	yj = polygon->coords[j][PF_Y];
	zi = polygon->coords[i][PF_Z];
	zj = polygon->coords[j][PF_Z];

	computed[PF_X] += (yi - yj)*(zi + zj);
	computed[PF_Y] += (zi - zj)*(xi + xj);
	computed[PF_Z] += (xi - xj)*(yi + yj);
    }
#endif

    /* ordering ok if the dot product is non-negative */
    if (pfDotVec3(computed, supplied) >= 0.0f)
	return 0;

    if (orientMode == PFDGBLDR_ORIENT_NORMALS)
    {
	/* reverse normal data */
	switch (polygon->nbind)
	{
	case PFGS_PER_VERTEX:
	    for (i = 0; i < polygon->numVerts; i++)
		pfNegateVec3(polygon->norms[i], polygon->norms[i]);
	    break;

	case PFGS_PER_PRIM:
	case PFGS_OVERALL:
	    pfNegateVec3(polygon->norms[0], polygon->norms[0]);
	    break;
	}
	reversed = 1;
    }
    else
    if (orientMode == PFDGBLDR_ORIENT_VERTICES)
    {
	/* reverse vertex data */
	pfdReverseGeom(polygon);
	reversed = 1;
    }
    else
    {
	reversed = 0;
    }

    /* let caller know if pfdGeom was reversed */
    return reversed;
}

static void
copyGeom(pfdPrim *tri, pfdGeom *pgon, int v0, int v1, int v2)
{
    int		i, k, t;

    tri->numTextures = pgon->numTextures;
    tri->flags = pgon->flags;
    tri->nbind = pgon->nbind;
    tri->cbind = pgon->cbind;
    for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	tri->tbind[i] = pgon->tbind[i];

    if (tri->flags & PFD_INDEXED)
    {
	tri->coordList = pgon->coordList;
	tri->normList = pgon->normList;
	tri->colorList = pgon->colorList;
        for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    tri->texCoordList[t] = pgon->texCoordList[t];

	tri->icoords[0] = pgon->icoords[v0];
	tri->icoords[1] = pgon->icoords[v1];
	tri->icoords[2] = pgon->icoords[v2];

	switch (tri->nbind)
	{
	case PFGS_PER_VERTEX:
	    tri->inorms[0] = pgon->inorms[v0];
	    tri->inorms[1] = pgon->inorms[v1];
	    tri->inorms[2] = pgon->inorms[v2];
	    break;

	case PFGS_OVERALL:
	case PFGS_PER_PRIM:
	    tri->inorms[0] = pgon->inorms[0];
	    break;

	default:
	    break;
	}

	switch (tri->cbind)
	{
	case PFGS_PER_VERTEX:
	    tri->icolors[0] = pgon->icolors[v0];
	    tri->icolors[1] = pgon->icolors[v1];
	    tri->icolors[2] = pgon->icolors[v2];
	    break;

	case PFGS_OVERALL:
	case PFGS_PER_PRIM:
	    tri->icolors[0] = pgon->icolors[0];
	    break;

	default:
	    break;
	}

	for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
	    switch (tri->tbind[k])
	    {
		case PFGS_PER_VERTEX:
		    tri->itexCoords[k][0] = pgon->itexCoords[k][v0];
		    tri->itexCoords[k][1] = pgon->itexCoords[k][v1];
		    tri->itexCoords[k][2] = pgon->itexCoords[k][v2];
		    break;

		default:
		    break;
	    }
    } 
    else
    {
	pfCopyVec3(tri->coords[0], pgon->coords[v0]);
	pfCopyVec3(tri->coords[1], pgon->coords[v1]);
	pfCopyVec3(tri->coords[2], pgon->coords[v2]);

	switch (tri->nbind)
	{
	case PFGS_PER_VERTEX:
	    pfCopyVec3(tri->norms[0], pgon->norms[v0]);
	    pfCopyVec3(tri->norms[1], pgon->norms[v1]);
	    pfCopyVec3(tri->norms[2], pgon->norms[v2]);
	    break;

	case PFGS_OVERALL:
	case PFGS_PER_PRIM:
	    pfCopyVec3(tri->norms[0], pgon->norms[0]);
	    break;

	default:
	    break;
	}

	switch (tri->cbind)
	{
	case PFGS_PER_VERTEX:
	    pfCopyVec4(tri->colors[0], pgon->colors[v0]);
	    pfCopyVec4(tri->colors[1], pgon->colors[v1]);
	    pfCopyVec4(tri->colors[2], pgon->colors[v2]);
	    break;

	case PFGS_OVERALL:
	case PFGS_PER_PRIM:
	    pfCopyVec4(tri->colors[0], pgon->colors[0]);
	    break;

	default:
	    break;
	}

	for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
	    switch (tri->tbind[k])
	    {
		case PFGS_PER_VERTEX:
		    pfCopyVec2(tri->texCoords[k][0], pgon->texCoords[k][v0]);
		    pfCopyVec2(tri->texCoords[k][1], pgon->texCoords[k][v1]);
		    pfCopyVec2(tri->texCoords[k][2], pgon->texCoords[k][v2]);
		    break;

		default:
		break;
	    }

    }
}

/* ###################################################################### */
/* #                                                                    # */
/* #         Construct geosets from pfdGeoBuilder geometry              # */
/* #                                                                    # */
/* ###################################################################### */

static void
copyLineStrip(pfdGeom *dst, pfdGeom *src)
{
    int i, k, num;

    dst->numTextures = src->numTextures;
    num = dst->numVerts = src->numVerts;
    dst->nbind = src->nbind;
    dst->cbind = src->cbind;
    for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	dst->tbind[i] = src->tbind[i];
    dst->primtype = src->primtype;
    dst->flags = src->flags;
    dst->pixelsize= src->pixelsize;
   
    if (src->flags & PFD_INDEXED)
    {
    	for (i=0; i<num; i++) 
    	{
	    dst->icoords[i] = src->icoords[i];
	}
	switch (src->cbind)
	{
	case PFGS_PER_VERTEX:
	    for (i=0; i<num-1; i++)
	    {
          	dst->icolors[i] = src->icolors[i];
	    }
	    if (src->primtype & PFGS_LINESTRIPS)
		dst->icolors[i] = src->icolors[i];
	    break;
	case PFGS_PER_PRIM:
	case PFGS_OVERALL:
	    dst->icolors[0] = src->icolors[0];
	    break;
	default:
	    break;
	}	
	switch (src->nbind)
	{
	case PFGS_PER_VERTEX:
	    for (i=0; i<num-1; i++)
            {
                dst->inorms[i] = src->inorms[i];
            }
            if (src->primtype & PFGS_LINESTRIPS)
                dst->inorms[i] = src->inorms[i];
            break;
        case PFGS_PER_PRIM:     
        case PFGS_OVERALL:      
            dst->inorms[0] = src->inorms[0];  
            break;      
        default:        
            break;      
        }       
	for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
	    switch (src->tbind[k])
	    {
		case PFGS_PER_VERTEX:
		    for (i=0; i<num; i++)
		    {
			dst->itexCoords[k][i] = src->itexCoords[k][i];
		    }
		    break;
		case PFGS_PER_PRIM:     
		case PFGS_OVERALL:      
		    dst->itexCoords[k][0] = src->itexCoords[k][0];
		    break;
		default:        
		    break;      
	    }  
    } 
    else
    {
        for (i=0; i<num; i++)
        {
           pfCopyVec3(dst->coords[i], src->coords[i]);
 	}
	switch (src->cbind)
	{
	case PFGS_PER_VERTEX:
	    for (i=0; i<num-1; i++)
	    {
		pfCopyVec4(dst->colors[i], src->colors[i]);
	    }
	    if (src->primtype & PFGS_LINESTRIPS)
		pfCopyVec4(dst->colors[i], src->colors[i]);
	    break;
	case PFGS_OVERALL:
        case PFGS_PER_PRIM:
	    pfCopyVec4(dst->colors[0], src->colors[0]);
	    break;
	default:
	    break;
	}
	switch (src->nbind)
	{
	case PFGS_PER_VERTEX:
	    for (i=0; i<num-1; i++)
            {
                pfCopyVec3(dst->norms[i], src->norms[i]);
            }
	    if (src->primtype & PFGS_LINESTRIPS)
		pfCopyVec3(dst->norms[i], src->norms[i]);
            break;
        case PFGS_OVERALL:
        case PFGS_PER_PRIM:
	    pfCopyVec3(dst->norms[0], src->norms[0]);
            break;
        default:
            break;
        }
	for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
	    switch (src->tbind[k])
	    {
		case PFGS_PER_VERTEX:
		    for (i=0; i<num; i++)
		    {
			pfCopyVec2(dst->texCoords[k][i], src->texCoords[k][i]);
		    }
		    break;
		case PFGS_OVERALL:
		case PFGS_PER_PRIM:
		    pfCopyVec2(dst->texCoords[k][0], src->texCoords[k][0]);
		    break;
		default:
		    break;
	    }
    }
}

void
copyPrim(pfdPrim *dst, pfdPrim *src)
{
    int i, j;

    dst->flags     = src->flags;
    dst->nbind     = src->nbind;
    dst->cbind     = src->cbind;
    for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	dst->tbind[i]     = src->tbind[i];
    dst->next      = src->next;
    dst->pixelsize = src->pixelsize;
    dst->numTextures = src->numTextures;

    for (i = 0; i < 3; i++) {
    	pfCopyVec3(dst->coords[i], src->coords[i]);
    	pfCopyVec4(dst->colors[i], src->colors[i]);
    	pfCopyVec3(dst->norms[i], src->norms[i]);
	for (j = 0 ; j < PF_MAX_TEXTURES ; j ++)
	    if ((src -> tbind[j] == PFGS_PER_VERTEX) && 
		(dst->texCoords[j]) && 
		(src->texCoords[j]))
		pfCopyVec2(dst->texCoords[j][i], src->texCoords[j][i]);
    }
}

static void 
moveTriList(pfdGeoBuilder *ts, 
	    int nb1, int cb1, int numTex1, 
	    int nb2, int cb2, int numTex2)
{
    pfdPrim	    *tri;
    
    if ((nb1 == nb2) && (cb1 == cb2) && (numTex1 == numTex2))
	return;
	
    if (ts->triCounts[nb2][cb2][numTex2] == 0)
    {
	ts->triBins[nb2][cb2][numTex2] = ts->triBins[nb1][cb1][numTex1];
    }
    else
    {    
	tri = ts->triBins[nb2][cb2][numTex2];
	
	while(tri->next)
	    tri = tri->next;

	tri->next = ts->triBins[nb1][cb1][numTex1];
    }
    
    ts->triBins[nb1][cb1][numTex1] = NULL;
    ts->triCounts[nb2][cb2][numTex2] += ts->triCounts[nb1][cb1][numTex1];
    ts->triCounts[nb1][cb1][numTex1] = 0;

    return;
}

static void 
moveIndexedTriList(pfdGeoBuilder *ts, 
	    int nb1, int cb1, int numTex1, 
	    int nb2, int cb2, int numTex2)
{
    pfdPrim	    *tri;
    
    if ((nb1 == nb2) && (cb1 == cb2) && (numTex1 == numTex2))
	return;
 	
    if (ts->indexedTriCounts[nb2][cb2][numTex2] == 0)
    {
	ts->indexedTriBins[nb2][cb2][numTex2] = 
		ts->indexedTriBins[nb1][cb1][numTex1];
    }
    else
    {    
	tri = ts->indexedTriBins[nb2][cb2][numTex2];
	
	while(tri->next)
	    tri = tri->next;

	tri->next = ts->indexedTriBins[nb1][cb1][numTex1];
    }
    
    ts->indexedTriBins[nb1][cb1][numTex1] = NULL;
    ts->indexedTriCounts[nb2][cb2][numTex2] += 
	    ts->indexedTriCounts[nb1][cb1][numTex1];
    ts->indexedTriCounts[nb1][cb1][numTex1] = 0;
    return;
}

static void
collapseAttr(pfdGeoBuilder *bldr, pfdPrim *tri, int attr)
{
    pfdPrim		*t;
    float          	*val, *tval;
    int	    		success, collapseBind;

    if (!tri)
	return;

    t = tri;

    if (attr == PFGS_NORMAL3)
    {
	if (tri->nbind == PFGS_PER_VERTEX)
	{
	    collapseBind = PFGS_PER_PRIM;
	    success = 1;
	    if (tri->flags & PFD_INDEXED)
	    {
		while (tri && success)
		{
		    val = bldr->normList[tri->inorms[0]];
		    tval = bldr->normList[tri->inorms[1]];
		    if (PFALMOST_EQUAL_VEC3(val, tval, SORTOF))
		    {
			tval = bldr->normList[tri->inorms[2]];
			if (!PFALMOST_EQUAL_VEC3(val, tval, SORTOF))
			    success = 0;
		    }
		    else
			success = 0;
		    tri = tri->next;
		}
	    }
	    else
	    {
		while (tri && success)
		{
		    val = tri->norms[0];
		    tval = tri->norms[1];
		    if (PFALMOST_EQUAL_VEC3(val, tval, SORTOF))
		    {
			tval = tri->norms[2];
			if (!PFALMOST_EQUAL_VEC3(val, tval, SORTOF))
			    success = 0;
		    }
		    else
			success = 0;
		    tri = tri->next;
		}
	    }
	}
	else if (tri->nbind == PFGS_PER_PRIM)
	{
	    collapseBind = PFGS_OVERALL;
	    success = 1;
	    if (tri->flags & PFD_INDEXED)
	    {
		val = bldr->normList[tri->inorms[0]];
		tri = tri->next;
		while (tri && success)
		{
		    tval = bldr->normList[tri->inorms[0]];
		    if (!PFALMOST_EQUAL_VEC3(val, tval, SORTOF))
			success = 0;
		    tri = tri->next;
		}
	    }
	    else
	    {
		val = tri->norms[0];
		tri = tri->next;
		while (tri && success)
		{
		    tval = tri->norms[0];
		    if (!PFALMOST_EQUAL_VEC3(val, tval, SORTOF))
			success = 0;
		    tri = tri->next;
		}
	    }
	}
	if (success)
	{
	    if (t->flags & PFD_INDEXED)
		moveIndexedTriList(bldr, t->nbind, t->cbind, t->numTextures,
				   collapseBind, t->cbind, t->numTextures);
	    else
		moveTriList(bldr, t->nbind, t->cbind, t->numTextures,
			    collapseBind, t->cbind, t->numTextures);

	    /* Success, change binding type in first tris */
	    t->nbind = collapseBind;
	}
    }
    else if (attr == PFGS_COLOR4)
    {
	if (tri->cbind == PFGS_PER_VERTEX)
	{
	    collapseBind = PFGS_PER_PRIM;
	    success = 1;
	    if (tri->flags & PFD_INDEXED)
	    {
		while (tri && success)
		{
		    val = bldr->colorList[tri->icolors[0]];
		    tval = bldr->colorList[tri->icolors[1]];
		    if (PFALMOST_EQUAL_VEC4(val, tval, SORTOF))
		    {
			tval = bldr->colorList[tri->icolors[2]];
			if (!PFALMOST_EQUAL_VEC4(val, tval, SORTOF))
			    success = 0;
		    }
		    else
			success = 0;
		    tri = tri->next;
		}
	    }
	    else
	    {
		while (tri && success)
		{
		    val = tri->colors[0];
		    tval = tri->colors[1];
		    if (PFALMOST_EQUAL_VEC4(val, tval, SORTOF))
		    {
			tval = tri->colors[2];
			if (!PFALMOST_EQUAL_VEC4(val, tval, SORTOF))
			    success = 0;
		    }
		    else
			success = 0;
		    tri = tri->next;
		}
	    }
	}
	else if (tri->cbind == PFGS_PER_PRIM)
	{
	    collapseBind = PFGS_OVERALL;
	    success = 1;
	    if (tri->flags & PFD_INDEXED)
	    {
		val = bldr->colorList[tri->icolors[0]];
		tri = tri->next;
		while (tri && success)
		{
		    tval = bldr->colorList[tri->icolors[0]];
		    if (!PFALMOST_EQUAL_VEC4(val, tval, SORTOF))
			success = 0;
		    tri = tri->next;
		}
	    }
	    else
	    {
		val = tri->colors[0];
		tri = tri->next;
		while (tri && success)
		{
		    tval = tri->colors[0];
		    if (!PFALMOST_EQUAL_VEC4(val, tval, SORTOF))
			success = 0;
		    tri = tri->next;
		}
	    }
	}

	if (success)
	{
	    if (t->flags & PFD_INDEXED)
		moveIndexedTriList(bldr, t->nbind, t->cbind, t->numTextures,
				   t->nbind, collapseBind, t->numTextures);
	    else
		moveTriList(bldr, t->nbind, t->cbind, t->numTextures,
			    t->nbind, collapseBind, t->numTextures);

	    /* Success, change binding type in first tris */
	    t->cbind = collapseBind;
	}
    }
}

static void
freeStripBins(pfdGeom *bin[4][4][PF_MAX_TEXTURES + 1], int count[4][4][PF_MAX_TEXTURES + 1])
{
    int i, j, k;
    pfdGeom *t;

    for (i = 0; i < 4; i++)
    {
	for (j = 0; j < 4; j++)
	{
	    for (k = 0; k < PF_MAX_TEXTURES + 1 ; k++)
	    {
		while(count[i][j][k] > 0)
		{
		    t = bin[i][j][k]->next;
		    pfdDelGeom(bin[i][j][k]);
		    bin[i][j][k] = t;
		    count[i][j][k]--;
		}
	    }
	}
    }
}

PFDUDLLEXPORT const pfList*
pfdBuildGSets(pfdGeoBuilder *bldr)
{
    int             nbind;
    int             cbind;
    int             tbind;
    pfdPrim	    *tri;
    int		    k;

    /* make sure geoset list is empty */
    pfResetList(bldr->gsetList);

    /* 
     * Run through geoset bins and collapse attributes, e.g. - 
     * change binding from PER_VERTEX to PER_PRIM if 
     * all of the PER_VERTEX attributes of each tri are the same.
     */
    if (bldr->collapse)
    {
	/* Collapse color PER_VERTEX to PER_PRIM */
	cbind = PFGS_PER_VERTEX;
	for (nbind = 3; nbind >= 0; nbind--)
	    for (tbind = PF_MAX_TEXTURES ; tbind >= 0; tbind--)
	    {
		/* collapse non-indexed bin */
		tri = bldr->triBins[nbind][cbind][tbind];
		if (tri)
		    collapseAttr(bldr, tri, PFGS_COLOR4);

		/* collapse indexed bin */
		tri = bldr->indexedTriBins[nbind][cbind][tbind];
		if (tri)
		    collapseAttr(bldr, tri, PFGS_COLOR4);
	    }
	/* Collapse normal PER_VERTEX to PER_PRIM */
	nbind = PFGS_PER_VERTEX;
	for (cbind = 3; cbind >= 0; cbind--)
	    for (tbind = PF_MAX_TEXTURES ; tbind >= 0; tbind--)
	    {
		/* collapse non-indexed bin */
		tri = bldr->triBins[nbind][cbind][tbind];
		if (tri)
		    collapseAttr(bldr, tri, PFGS_NORMAL3);

		/* collapse indexed bin */
		tri = bldr->indexedTriBins[nbind][cbind][tbind];
		if (tri)
		    collapseAttr(bldr, tri, PFGS_NORMAL3);
	    }
	/* Collapse color PER_PRIM to OVERALL */
	cbind = PFGS_PER_PRIM;
	for (nbind = 3; nbind >= 0; nbind--)
	    for (tbind = PF_MAX_TEXTURES ; tbind >= 0; tbind--)
	    {
		/* collapse non-indexed bin */
		tri = bldr->triBins[nbind][cbind][tbind];
		if (tri)
		    collapseAttr(bldr, tri, PFGS_COLOR4);

		/* collapse indexed bin */
		tri = bldr->indexedTriBins[nbind][cbind][tbind];
		if (tri)
		    collapseAttr(bldr, tri, PFGS_COLOR4);
	    }
	/* Collapse normal PER_PRIM to OVERALL */
	nbind = PFGS_PER_PRIM;
	for (cbind = 3; cbind >= 0; cbind--)
	    for (tbind = PF_MAX_TEXTURES ; tbind >= 0; tbind--)
	    {
		/* collapse non-indexed bin */
		tri = bldr->triBins[nbind][cbind][tbind];
		if (tri)
		    collapseAttr(bldr, tri, PFGS_NORMAL3);

		/* collapse indexed bin */
		tri = bldr->indexedTriBins[nbind][cbind][tbind];
		if (tri)
		    collapseAttr(bldr, tri, PFGS_NORMAL3);
	    }
    }

    /* Run through bins again and create pfGeoSet(s) */
    for (nbind = 3; nbind >= 0; nbind--)
	for (cbind = 3; cbind >= 0; cbind--)
	    for (tbind = PF_MAX_TEXTURES ; tbind >= 0; tbind--)
		makeGSets(bldr, nbind, cbind, tbind);

    /* reset pfdGeoBuilder bins now that data is in geoset form */
    bzero(bldr->pointBins, sizeof(bldr->pointBins));
    bzero(bldr->pointCounts, sizeof(bldr->pointCounts));
    bldr->numPoints = 0;

    bzero(bldr->indexedPointBins, sizeof(bldr->indexedPointBins));
    bzero(bldr->indexedPointCounts, sizeof(bldr->indexedPointCounts));
    bldr->numIndexedPoints = 0;

    bzero(bldr->triBins, sizeof(bldr->triBins));
    bzero(bldr->triCounts, sizeof(bldr->triCounts));
    bldr->numTris = 0;

    bzero(bldr->indexedTriBins, sizeof(bldr->indexedTriBins));
    bzero(bldr->indexedTriCounts, sizeof(bldr->indexedTriCounts));
    bldr->numIndexedTris = 0;

    bzero(bldr->lineBins, sizeof(bldr->lineBins));
    bzero(bldr->lineCounts, sizeof(bldr->lineCounts));
    bldr->numLines = 0;

    bzero(bldr->indexedLineBins, sizeof(bldr->indexedLineBins));
    bzero(bldr->indexedLineCounts, sizeof(bldr->indexedLineCounts));
    bldr->numIndexedLines = 0;

    freeStripBins(bldr->linestripBins, bldr->linestripCounts);
    bzero(bldr->linestripBins, sizeof(bldr->linestripBins));
    bzero(bldr->linestripCounts, sizeof(bldr->linestripCounts));
    bldr->numLineStrips = 0;

    freeStripBins(bldr->indexedlinestripBins, bldr->indexedlinestripCounts);
    bzero(bldr->indexedlinestripBins, sizeof(bldr->indexedlinestripBins));
    bzero(bldr->indexedlinestripCounts, sizeof(bldr->indexedlinestripCounts));
    bldr->numIndexedLineStrips = 0;

    freeStripBins(bldr->flatlinestripBins, bldr->flatlinestripCounts);
    bzero(bldr->flatlinestripBins, sizeof(bldr->flatlinestripBins));
    bzero(bldr->flatlinestripCounts, sizeof(bldr->flatlinestripCounts));
    bldr->numFlatLineStrips = 0;

    freeStripBins(bldr->indexedflatlinestripBins, bldr->indexedflatlinestripCounts);
    bzero(bldr->indexedflatlinestripBins, sizeof(bldr->indexedflatlinestripBins));
    bzero(bldr->indexedflatlinestripCounts, sizeof(bldr->indexedflatlinestripCounts));
    bldr->numIndexedFlatLineStrips = 0;

    bldr->coordList    = NULL;
    bldr->normList     = NULL;
    bldr->colorList    = NULL;

    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
	bldr->texCoordList[k] = NULL;

    bldr->availTri = bldr->triList;

    /* return list of builder's pfGeoSets to caller */
    return bldr->gsetList;
}

/* ###################################################################### */
/* #                                                                    # */
/* #             Private pfGeoSet construction functions                # */
/* #                                                                    # */
/* ###################################################################### */

/*
 * makeTriGSets -- Make pfGeoSet(s) for non-indexed triangles
 */
static void
makeTriGSets(pfdGeoBuilder *bldr, int _nbind, int _cbind, int _tbind)
{
    int           i;
    int           j;
    int           k;
    int	    	  ntris;
    int	    	  totalTris;
    pfGeoSet     *gset		= NULL;
    pfdPrim      *t		= NULL;
    pfdPrim      *tri		= NULL;
    pfVec3       *coords	= NULL;
    pfVec3       *norms		= NULL;
    pfVec4       *colors	= NULL;
    pfVec2       *texCoords[PF_MAX_TEXTURES];

    if (bldr == NULL)
	return;

    for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	texCoords[i] = NULL;

    tri       = bldr->triBins  [_nbind][_cbind][_tbind];
    totalTris = bldr->triCounts[_nbind][_cbind][_tbind];

    while (totalTris > 0)
    {

	int	nbind = _nbind;
	int	cbind = _cbind;
	int	tbind = _tbind;

	gset = pfNewGSet(bldr->sharedArena);

	/*
	 * note: limit geoset size to 20K triangles to ensure that attribute
	 * triangle mesher indices stay within 64k unsigned short limit. 
	 */
	ntris = PF_MIN2(totalTris, bldr->limit);

	pfGSetPrimType(gset, PFGS_TRIS);
	pfGSetNumPrims(gset, ntris);

	/* generate vertex position attribute array */
	{
	    coords = pfMalloc(sizeof(pfVec3) * 3 * ntris, bldr->sharedArena);
	    j = 0;
	    t = tri;
	    for (i = 0; i < ntris; i++)
	    {
		pfCopyVec3(coords[j++], t->coords[0]);
		pfCopyVec3(coords[j++], t->coords[1]);
		pfCopyVec3(coords[j++], t->coords[2]);

		t = t->next;
	    }
	    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	}

	/* generate color attribute array */
	if (bldr->autoColors == PFDGBLDR_COLORS_DISCARD)
	{
	    cbind = PFGS_OFF;
	    colors = NULL;
	}
	else
	if ((bldr->autoColors == PFDGBLDR_COLORS_MISSING && cbind == PFGS_OFF) ||
	    (bldr->autoColors == PFDGBLDR_COLORS_GENERATE))
	{
	    cbind = PFGS_OVERALL;
	    colors = pfMalloc(sizeof(pfVec4), bldr->sharedArena);
	    pfuRandomColor(colors[0], 0.5f, 1.0f);
	} 
	else
	{
	    switch (cbind)
	    {
	    case PFGS_PER_VERTEX:
		colors = pfMalloc(sizeof(pfVec4) * 3 * ntris, bldr->sharedArena);
		j = 0;
		t = tri;
		for (i = 0; i < ntris; i++)
		{
		    pfCopyVec4(colors[j++], t->colors[0]);
		    pfCopyVec4(colors[j++], t->colors[1]);
		    pfCopyVec4(colors[j++], t->colors[2]);
		    t = t->next;
		}
		break;

	    case PFGS_PER_PRIM:
		colors = pfMalloc(sizeof(pfVec4) * ntris, bldr->sharedArena);
		t = tri;
		for (i = 0; i < ntris; i++)
		{
		    pfCopyVec4(colors[i], t->colors[0]);
		    t = t->next;
		}
		break;

	    case PFGS_OVERALL:
		colors = pfMalloc(sizeof(pfVec4), bldr->sharedArena);
		pfCopyVec4(colors[0], tri->colors[0]);
		break;

	    case PFGS_OFF:
		colors = NULL;
		break;
	    }
	}
	pfGSetAttr(gset, PFGS_COLOR4, cbind, colors, NULL);

	/* generate texture attribute array */
	if (bldr->autoTexture == PFDGBLDR_TEXTURE_DISCARD)
	{
	    tbind = 0;
   	    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
		texCoords[k] = NULL;
	}
	else
	if ((bldr->autoTexture == PFDGBLDR_TEXTURE_MISSING && tbind == 0) ||
	    (bldr->autoTexture == PFDGBLDR_TEXTURE_GENERATE))
	{
	    static int firstTime = 1;
	    if (firstTime)
	    {
		firstTime = 0;
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		    "makeGSets: geobuilder texgen not yet implemented");
	    }
	    tbind = 0;
   	    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
		texCoords[k] = NULL;
	} 
	else
	{
	    switch (tbind)
	    {
	    case 0:
		for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
		    texCoords[k] = NULL;
		break;
	    
	    default:
		tbind = _tbind;
		for (k = 0 ; k < tbind ; k ++)
		    texCoords[k] = pfMalloc(sizeof(pfVec2) * 3 * ntris, 
			bldr->sharedArena);
		j = 0;
		t = tri;
		for (i = 0; i < ntris; i++)
		{
		    for (k = 0 ; k < tbind ; k ++)
		    {
			pfCopyVec2(texCoords[k][j  ], t->texCoords[k][0]);
			pfCopyVec2(texCoords[k][j+1], t->texCoords[k][1]);
			pfCopyVec2(texCoords[k][j+2], t->texCoords[k][2]);
		    }
		    j += 3;
		    t = t->next;
		}
		break;
	    }
	}
  
	if (tbind == 0)
	    pfGSetAttr (gset, PFGS_TEXCOORD2, PFGS_OFF, NULL, NULL);
	else
	    for (k = 0 ; k < tbind ; k ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, k, PFGS_PER_VERTEX, 
				texCoords[k], NULL);

	/* generate normal attribute array */
	if (bldr->autoNormals == PFDGBLDR_NORMALS_DISCARD)
	{
	    nbind = PFGS_OFF;
	    norms = NULL;
	}
	else
	if ((bldr->autoNormals == PFDGBLDR_NORMALS_MISSING && nbind == PFGS_OFF) ||
	    (bldr->autoNormals == PFDGBLDR_NORMALS_GENERATE))
	{
	    nbind = PFGS_PER_PRIM;
	    norms = pfMalloc(sizeof(pfVec3) * ntris, bldr->sharedArena);
	    for (i = 0; i < ntris; i++)
	    {
		pfVec3          v10;
		pfVec3          v21;

		/* compute face normal */
		pfSubVec3(v10, coords[3*i + 1], coords[3*i + 0]);
		pfSubVec3(v21, coords[3*i + 2], coords[3*i + 1]);
		pfCrossVec3(norms[i], v10, v21);
		pfNormalizeVec3(norms[i]);
	    }
	}
	else
	{
	    switch (nbind)
	    {
	    case PFGS_PER_VERTEX:
		norms = pfMalloc(sizeof(pfVec3) * 3 * ntris, bldr->sharedArena);
		j = 0;
		t = tri;
		for (i = 0; i < ntris; i++)
		{
		    pfCopyVec3(norms[j++], t->norms[0]);
		    pfCopyVec3(norms[j++], t->norms[1]);
		    pfCopyVec3(norms[j++], t->norms[2]);
		    t = t->next;
		}
		break;

	    case PFGS_PER_PRIM:
		norms = pfMalloc(sizeof(pfVec3) * ntris, bldr->sharedArena);
		t = tri;
		for (i = 0; i < ntris; i++)
		{
		    pfCopyVec3(norms[i], t->norms[0]);
		    t = t->next;
		}
		break;

	    case PFGS_OVERALL:
		norms = pfMalloc(sizeof(pfVec3), bldr->sharedArena);
		pfCopyVec3(norms[0], tri->norms[0]);
		break;

	    case PFGS_OFF:
		norms = NULL;
		break;
	    }
	}
	pfGSetAttr(gset, PFGS_NORMAL3, nbind, norms, NULL);

	/* are normals consistent with CCW vertex interpretation */
	if ((norms != NULL) &&
	    ((bldr->autoOrient == PFDGBLDR_ORIENT_NORMALS) ||
	     (bldr->autoOrient == PFDGBLDR_ORIENT_VERTICES)))
	{
	    for (i = 0; i < ntris; i++)
	    {
		pfVec3	 supplied;
		pfVec3	 computed;
		pfVec3   v10;
		pfVec3   v21;

		/* get average (possibly unnormalized) normal vector */
		switch (nbind)
		{
		case PFGS_PER_VERTEX:
		    /* average all supplied normals */
		    pfCopyVec3(supplied,           norms[3*i    ]);
		    pfAddVec3 (supplied, supplied, norms[3*i + 1]);
		    pfAddVec3 (supplied, supplied, norms[3*i + 2]);
		    break;

		case PFGS_PER_PRIM:
		    /* use the supplied normal */
		    pfCopyVec3(supplied, norms[i]);
		    break;
		
		case PFGS_OVERALL:
		    /* use the supplied normal */
		    pfCopyVec3(supplied, norms[0]);
		    break;
		}
		
		/* compute geometric face normal from vertices*/
		pfSubVec3(v10, coords[3*i + 1], coords[3*i + 0]);
		pfSubVec3(v21, coords[3*i + 2], coords[3*i + 1]);
		pfCrossVec3(computed, v10, v21);

		/* ordering ok if the dot product is non-negative */
		if (pfDotVec3(computed, supplied) >= 0.0f)
		    continue;

		if (bldr->autoOrient == PFDGBLDR_ORIENT_NORMALS)
		{
		    /* reverse direction of vertex normals */
		    switch (nbind)
		    {
		    case PFGS_PER_VERTEX:
			pfNegateVec3(norms[3*i    ], norms[3*i    ]);
			pfNegateVec3(norms[3*i + 1], norms[3*i + 1]);
			pfNegateVec3(norms[3*i + 2], norms[3*i + 2]);
			break;

		    case PFGS_PER_PRIM:
			pfNegateVec3(norms[i], norms[i]);
			break;
		    
		    case PFGS_OVERALL:
			pfNegateVec3(norms[0], norms[0]);
			break;
		    }
		}
		else
		if (bldr->autoOrient == PFDGBLDR_ORIENT_VERTICES)
		{
		    /* reverse orientation (CW .vs CCW) of triangle */
		    {
			pfVec3 exchange;
			pfCopyVec3(exchange,           coords[3*i    ]);
			pfCopyVec3(coords[3*i    ],    coords[3*i + 2]);
			pfCopyVec3(coords[3*i + 2],    exchange);
		    }
		    if (cbind == PFGS_PER_VERTEX && colors != NULL)
		    {
			pfVec4 exchange;
			pfCopyVec4(exchange,           colors[3*i    ]);
			pfCopyVec4(colors[3*i    ],    colors[3*i + 2]);
			pfCopyVec4(colors[3*i + 2],    exchange);
		    }
		    if (nbind == PFGS_PER_VERTEX && norms != NULL)
		    {
			pfVec3 exchange;
			pfCopyVec3(exchange,           norms[3*i    ]);
			pfCopyVec3(norms[3*i    ],     norms[3*i + 2]);
			pfCopyVec3(norms[3*i + 2],     exchange);
		    }
		    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
			if (tbind > 0 && texCoords[k] != NULL)
			{
			    pfVec2 exchange;
			    pfCopyVec2(exchange,           
							texCoords[k][3*i    ]);
			    pfCopyVec2(texCoords[k][3*i    ], 
							texCoords[k][3*i + 2]);
			    pfCopyVec2(texCoords[k][3*i + 2], 
							exchange);
			}
		}
	    }
	}

	/* perform triangle strip generation */
	if (bldr->mesh)
	{
	    pfGeoSet *strip = NULL;
	    strip = pfdMeshGSet(gset);
	    if (strip != NULL)
	    {
		pfDelete(gset);
		gset = strip;
	    }
	}

	/* remember this geoset */
	pfAdd(bldr->gsetList, gset);
	totalTris -= ntris;

	/* update starting point in primitive chain */
	tri = t;
    }
}

/*
 * makeIndexedTriGSets -- Make pfGeoSet(s) for indexed triangles
 *
 *	Yair: The parameter _tbind holds the number of active texture units
 *	      for this bin.
 */
static void
makeIndexedTriGSets(pfdGeoBuilder *bldr, int _nbind, int _cbind, int _tbind)
{
    int           i;
    int           j;
    int		  k;
    int	    	  ntris;
    int	    	  totalTris;
    pfGeoSet     *gset		= NULL;
    pfdPrim      *t		= NULL;
    pfdPrim      *tri		= NULL;
    pfVec3       *coords	= NULL;
    pfVec3       *norms		= NULL;
    pfVec4       *colors	= NULL;
    pfVec2       *texCoords[PF_MAX_TEXTURES];
    ushort	 *icoords	= NULL;
    ushort	 *inorms	= NULL;
    ushort	 *icolors	= NULL;
    ushort	 *itexCoords[PF_MAX_TEXTURES];
    int icoords_share;

    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
    {
	texCoords[k] = NULL;
	itexCoords[k] = NULL;
    }

    tri       = bldr->indexedTriBins  [_nbind][_cbind][_tbind];
    totalTris = bldr->indexedTriCounts[_nbind][_cbind][_tbind];

    while (totalTris > 0)
    {
	int	nbind = _nbind;
	int	cbind = _cbind;
	int	tbind = _tbind;

	gset = pfNewGSet(bldr->sharedArena);

	/*
	 * Limit geoset size to 10K triangles to ensure that attribute
	 * triangle mesher indices stay within 32k unsigned short limit. 
	 */
	ntris = PF_MIN2(totalTris, bldr->limit);

	pfGSetPrimType(gset, PFGS_TRIS);
	pfGSetNumPrims(gset, ntris);

	/* generate vertex position attribute array */
	{
	    coords = bldr->coordList;

	    icoords = pfMalloc(sizeof(ushort) * 3 * ntris, 
						bldr->sharedArena);
	    j = 0;
	    t = tri;
	    for (i = 0; i < ntris; i++)
	    {
		icoords[j++] = t->icoords[0];
		icoords[j++] = t->icoords[1];
		icoords[j++] = t->icoords[2];
		t = t->next;
	    }
	}
	pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, icoords);

	/* generate color attribute array */
	if (bldr->autoColors == PFDGBLDR_COLORS_DISCARD)
	{
	    cbind = PFGS_OFF;
	    colors = NULL;
	    icolors = NULL;
	}
	else
	if ((bldr->autoColors == PFDGBLDR_COLORS_MISSING && cbind == PFGS_OFF) ||
	    (bldr->autoColors == PFDGBLDR_COLORS_GENERATE))
	{
	    cbind = PFGS_OVERALL;

	    colors = pfMalloc(sizeof(pfVec4), bldr->sharedArena);
	    pfuRandomColor(colors[0], 0.5f, 1.0f);

	    icolors = pfMalloc(sizeof(ushort), bldr->sharedArena);
	    icolors[0] = 0;
	} 
	else
	{
	    switch (cbind)
	    {
	    case PFGS_PER_VERTEX:
		colors = bldr->colorList;
		j = 0;
		t = tri;
		icoords_share = bldr->shareIndexLists;
		for (i = 0; icoords_share && i < ntris; i++)
		{
		    if (icoords[j++] != t->icolors[0] ||
			icoords[j++] != t->icolors[1] ||
			icoords[j++] != t->icolors[2])
			icoords_share = FALSE;
		    t = t->next;
		}
		if (icoords_share)
		    icolors = icoords;
		else
		{
		    icolors = pfMalloc(sizeof(ushort) * 3 * ntris,
				       bldr->sharedArena);
		    j = 0;
		    t = tri;
		    for (i = 0; i < ntris; i++)
		    {
			icolors[j++] = t->icolors[0];
			icolors[j++] = t->icolors[1];
			icolors[j++] = t->icolors[2];
			t = t->next;
		    }
		}

		break;

	    case PFGS_PER_PRIM:
		colors = bldr->colorList;
		icolors = pfMalloc(sizeof(ushort) * ntris, bldr->sharedArena);
		t = tri;
		for (i = 0; i < ntris; i++)
		{
		    icolors[i] = t->icolors[0];
		    t = t->next;
		}
		break;

	    case PFGS_OVERALL:
		colors = bldr->colorList;
		icolors = pfMalloc(sizeof(ushort), bldr->sharedArena);
		icolors[0] = tri->icolors[0];
		break;

	    case PFGS_OFF:
		colors = NULL;
		icolors = NULL;
		break;
	    }
	}
	pfGSetAttr(gset, PFGS_COLOR4, cbind, colors, icolors);

	/* generate texture attribute array */
	if (bldr->autoTexture == PFDGBLDR_TEXTURE_DISCARD)
	{
	    tbind = 0;
	    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
	    {
		texCoords[k] = NULL;
		itexCoords[k] = NULL;
	    }
	}
	else
	if ((bldr->autoTexture == PFDGBLDR_TEXTURE_MISSING && tbind == 0) ||
	    (bldr->autoTexture == PFDGBLDR_TEXTURE_GENERATE))
	{
	    static int firstTime = 1;
	    if (firstTime)
	    {
		firstTime = 0;
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		    "makeGSets: geobuilder texgen not yet implemented");
	    }
	    tbind = 0;
	    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
	    {
		texCoords[k] = NULL;
		itexCoords[k] = NULL;
	    }
	} 
	else
	{
	    switch (tbind)
	    {
	    case 0:
		for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
		{
		    texCoords[k] = NULL;
		    itexCoords[k] = NULL;
		}
		break;
	    
	    default:
		tbind = _tbind;
		for (k = 0 ; k < tbind ; k ++)
		    texCoords[k] = bldr->texCoordList[k];
		j = 0;
		t = tri;
		icoords_share = bldr->shareIndexLists;
		for (i = 0; icoords_share && i < ntris; i++)
		{
		    for (k = 0 ; k < tbind ; k ++)
			if (icoords[j] != t->itexCoords[k][0] ||
			    icoords[j+1] != t->itexCoords[k][1] ||
			    icoords[j+2] != t->itexCoords[k][2])
			    icoords_share = FALSE;

		    j += 3;
		    t = t->next;
		}
		if (icoords_share)
		    for (k = 0 ; k < tbind ; k ++)
			itexCoords[k] = icoords;
		else
		{
		    for (k = 0 ; k < tbind ; k ++)
			itexCoords[k] = pfMalloc(sizeof(ushort) * 3 * ntris,
					  bldr->sharedArena);
		    j = 0;
		    t = tri;
		    for (i = 0; i < ntris; i++)
		    {
			for (k = 0 ; k < tbind ; k ++)
			{
			    itexCoords[k][j  ] = t->itexCoords[k][0];
			    itexCoords[k][j+1] = t->itexCoords[k][1];
			    itexCoords[k][j+2] = t->itexCoords[k][2];
			}
			j += 3;
			t = t->next;
		    }
		}
		break;
	    }
	}

	if (tbind == 0)
	    pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_OFF, NULL, NULL);
	else
	    for (k = 0 ; k < tbind ; k ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, k, 
				PFGS_PER_VERTEX, texCoords[k], itexCoords[k]);

	/* generate normal attribute array */
	if (bldr->autoNormals == PFDGBLDR_NORMALS_DISCARD)
	{
	    nbind = PFGS_OFF;
	    norms = NULL;
	    inorms = NULL;
	}
	else
	if ((bldr->autoNormals == PFDGBLDR_NORMALS_MISSING && nbind == PFGS_OFF) ||
	    (bldr->autoNormals == PFDGBLDR_NORMALS_GENERATE))
	{
	    nbind = PFGS_PER_PRIM;
	    norms = pfMalloc(sizeof(pfVec3) * ntris, bldr->sharedArena);
	    inorms = pfMalloc(sizeof(ushort) * ntris, bldr->sharedArena);
	    for (i = 0; i < ntris; i++)
	    {
		pfVec3          v10;
		pfVec3          v21;

		/* compute face normal */
		pfSubVec3(v10, 
		    coords[icoords[3*i + 1]], 
		    coords[icoords[3*i + 0]]);
		pfSubVec3(v21, 
		    coords[icoords[3*i + 2]], 
		    coords[icoords[3*i + 1]]);
		pfCrossVec3(norms[i], v10, v21);
		pfNormalizeVec3(norms[i]);

		inorms[i] = i;
	    }
	}
	else
	{
	    switch (nbind)
	    {
	    case PFGS_PER_VERTEX:
		norms = bldr->normList;
		j = 0;
		t = tri;
		icoords_share = bldr->shareIndexLists;
		for (i = 0; icoords_share && i < ntris; i++)
		{
		    if (icoords[j++] != t->inorms[0] ||
			icoords[j++] != t->inorms[1] ||
			icoords[j++] != t->inorms[2])
			icoords_share = FALSE;
		    t = t->next;
		}
		if (icoords_share)
		    inorms = icoords;
		else
		{
		    inorms = pfMalloc(sizeof(ushort) * 3 * ntris,
				      bldr->sharedArena);
		    j = 0;
		    t = tri;
		    for (i = 0; i < ntris; i++)
		    {
			inorms[j++] = t->inorms[0];
			inorms[j++] = t->inorms[1];
			inorms[j++] = t->inorms[2];
			t = t->next;
		    }
		}
		break;

	    case PFGS_PER_PRIM:
		norms = bldr->normList;
		inorms = pfMalloc(sizeof(ushort) * ntris, bldr->sharedArena);
		t = tri;
		for (i = 0; i < ntris; i++)
		{
		    inorms[i] = t->inorms[0];
		    t = t->next;
		}
		break;

	    case PFGS_OVERALL:
		norms = bldr->normList;
		inorms = pfMalloc(sizeof(ushort), bldr->sharedArena);
		inorms[0] = tri->inorms[0];
		break;

	    case PFGS_OFF:
		norms = NULL;
		inorms = NULL;
		break;
	    }
	}
	pfGSetAttr(gset, PFGS_NORMAL3, nbind, norms, inorms);

	/* are normals consistent with CCW vertex interpretation */
	if ((norms != NULL && inorms != NULL) &&
	    ((bldr->autoOrient == PFDGBLDR_ORIENT_NORMALS) ||
	     (bldr->autoOrient == PFDGBLDR_ORIENT_VERTICES)))
	{
	    for (i = 0; i < ntris; i++)
	    {
		pfVec3	 supplied;
		pfVec3	 computed;
		pfVec3   v10;
		pfVec3   v21;

		/* get average (possibly unnormalized) normal vector */
		switch (nbind)
		{
		case PFGS_PER_VERTEX:
		    /* average all supplied normals */
		    pfCopyVec3(supplied,           norms[inorms[3*i    ]]);
		    pfAddVec3 (supplied, supplied, norms[inorms[3*i + 1]]);
		    pfAddVec3 (supplied, supplied, norms[inorms[3*i + 2]]);
		    break;

		case PFGS_PER_PRIM:
		    /* use the supplied normal */
		    pfCopyVec3(supplied, norms[inorms[i]]);
		    break;
		
		case PFGS_OVERALL:
		    /* use the supplied normal */
		    pfCopyVec3(supplied, norms[inorms[0]]);
		    break;
		}
		
		/* compute geometric face normal from vertices*/
		pfSubVec3(v10, 
		    coords[icoords[3*i + 1]], 
		    coords[icoords[3*i + 0]]);
		pfSubVec3(v21, 
		    coords[icoords[3*i + 2]], 
		    coords[icoords[3*i + 1]]);
		pfCrossVec3(computed, v10, v21);

		/* ordering ok if the dot product is non-negative */
		if (pfDotVec3(computed, supplied) >= 0.0f)
		    continue;

		if (bldr->autoOrient == PFDGBLDR_ORIENT_NORMALS)
		{
#if 0
		    /* reverse direction of vertex normals */
		    switch (nbind)
		    {
		    case PFGS_PER_VERTEX:
			pfNegateVec3(
			    norms[inorms[3*i    ]], 
			    norms[inorms[3*i    ]]);
			pfNegateVec3(
			    norms[inorms[3*i + 1]], 
			    norms[inorms[3*i + 1]]);
			pfNegateVec3(
			    norms[inorms[3*i + 2]], 
			    norms[inorms[3*i + 2]]);
			break;

		    case PFGS_PER_PRIM:
			pfNegateVec3(
			    norms[inorms[i]], 
			    norms[inorms[i]]);
			break;
		    
		    case PFGS_OVERALL:
			pfNegateVec3(
			    norms[inorms[0]], 
			    norms[inorms[0]]);
			break;
		    }
#else
		    static int firstTime = 1;
		    if (firstTime)
		    {
			firstTime = 0;
			pfNotify(PFNFY_WARN, PFNFY_PRINT, 
			    "makeGSets: it's too risky to modify indexed normals");
		    }
#endif
		}
		else
		if (bldr->autoOrient == PFDGBLDR_ORIENT_VERTICES)
		{
		    /* reverse orientation (CW .vs CCW) of triangle */
		    {
			ushort exchange;
			exchange         = icoords[3*i    ];
			icoords[3*i    ] = icoords[3*i + 2];
			icoords[3*i + 2] = exchange;
		    }
		    if (cbind == PFGS_PER_VERTEX && icolors != NULL &&
			icolors != icoords)
		    {
			ushort exchange;
			exchange         = icolors[3*i    ];
			icolors[3*i    ] = icolors[3*i + 2];
			icolors[3*i + 2] = exchange;
		    }
		    if (nbind == PFGS_PER_VERTEX && inorms != NULL &&
			inorms != icoords)
		    {
			ushort exchange;
			exchange         = inorms[3*i    ];
			inorms[3*i    ]  = inorms[3*i + 2];
			inorms[3*i + 2]  = exchange;
		    }

		    for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
			if (tbind > 0 && itexCoords[k] != NULL &&
			    itexCoords[k] != icoords)
			{
			    ushort exchange;
			    exchange             = itexCoords[k][3*i    ];
			    itexCoords[k][3*i    ]  = itexCoords[k][3*i + 2];
			    itexCoords[k][3*i + 2]  = exchange;
			}
		}
	    }
	}

	/* perform triangle strip generation */
	if (bldr->mesh)
	{
	    pfGeoSet *strip = NULL;
	    strip = pfdMeshGSet(gset);
	    if (strip != NULL)
	    {
		pfDelete(gset);
		gset = strip;
	    }
	}

	/* remember this geoset */
	pfAdd(bldr->gsetList, gset);
	totalTris -= ntris;

	/* update starting point in primitive chain */
	tri = t;
    }
}

/*
 * makeLineGSets -- Make pfGeoSet(s) for non-indexed lines and linestrips
 *
 * Yair: tbind holds the number of active texture units in this bin.
 */
static void
makeLineGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind)
{
    pfGeoSet     *gset;
    int           i, j, k, m, t;
    int	    	  totalLines, totalLineStrips;
    int	    	  numLines, numLineStrips, numLineStripVerts;
    int	   	 *lengthList;
    pfdPrim	 *lines, *l1;
    pfdGeom      *linestrips, *l2;

    pfVec3       *coords	= NULL;
    pfVec3       *norms		= NULL;
    pfVec4       *colors	= NULL;
    pfVec2       *texCoords[PF_MAX_TEXTURES];

    ushort	 *icoords	= NULL;
    ushort	 *inorms	= NULL;
    ushort	 *icolors	= NULL;
    ushort	 *itexCoords[PF_MAX_TEXTURES];

    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
    {
	texCoords[t] = NULL;
	itexCoords[t] = NULL;
    }
    
    lines           = bldr->lineBins       [nbind][cbind][tbind];
    totalLines      = bldr->lineCounts     [nbind][cbind][tbind];

    linestrips      = bldr->linestripBins  [nbind][cbind][tbind];
    totalLineStrips = bldr->linestripCounts[nbind][cbind][tbind];

    while ((totalLines!=0) || (totalLineStrips!=0)) 
    {
	l1 = lines;
	l2 = linestrips;
	if ((totalLines!=0) && ((totalLineStrips==0) ||
		(l1->pixelsize > l2->pixelsize)))  /* geoset for lines only */
	{
	gset = pfNewGSet(bldr->sharedArena);
	numLines = 1;
	while ((totalLines>1) && (l1->pixelsize==l1->next->pixelsize))
	{
	    numLines++;
	    l1 = l1->next;
	    totalLines--;
	}
	totalLines--;

	pfGSetPrimType(gset, PFGS_LINES);
	pfGSetNumPrims(gset, numLines);
        pfGSetLineWidth(gset, lines->pixelsize);


	switch (cbind)
	{
	case PFGS_PER_VERTEX:
	    colors = pfMalloc(sizeof(pfVec4) * 2 * numLines, bldr->sharedArena);
	    j = 0;
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		pfCopyVec4(colors[j++], l1->colors[0]);
		pfCopyVec4(colors[j++], l1->colors[1]);

		l1 = l1->next;
	    }
	    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, NULL);
	    break;

	case PFGS_PER_PRIM:
	    colors = pfMalloc(sizeof(pfVec4) * numLines, bldr->sharedArena);
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		pfCopyVec4(colors[i], l1->colors[0]);

		l1 = l1->next;
	    }
	    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_PRIM, colors, NULL);
	    break;

	case PFGS_OVERALL:
	    colors = pfMalloc(sizeof(pfVec4), bldr->sharedArena);
	    pfCopyVec4(colors[0], lines->colors[0]);
	    pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
	    break;

	case PFGS_OFF:
	    pfGSetAttr(gset, PFGS_COLOR4, cbind, NULL, NULL);
	    break;
	}

	switch (nbind)
	{
	case PFGS_PER_VERTEX:
	    norms = pfMalloc(sizeof(pfVec3) * 2 * numLines, bldr->sharedArena);
	    j = 0;
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		pfCopyVec3(norms[j++], l1->norms[0]);
		pfCopyVec3(norms[j++], l1->norms[1]);

		l1 = l1->next;
	    }
	    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
	    break;

	case PFGS_PER_PRIM:
	    norms = pfMalloc(sizeof(pfVec3) * numLines, bldr->sharedArena);
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		pfCopyVec3(norms[i], l1->norms[0]);

		l1 = l1->next;
	    }
	    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_PRIM, norms, NULL);
	    break;

	case PFGS_OVERALL:
	    norms = pfMalloc(sizeof(pfVec3), bldr->sharedArena);
	    pfCopyVec3(norms[0], lines->norms[0]);
	    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, norms, NULL);
	    break;

	case PFGS_OFF:
	    pfGSetAttr(gset, PFGS_NORMAL3, nbind, NULL, NULL);
	    break;
	}

	if (tbind != 0)
	{
 	    for (t = 0 ; t < tbind ; t ++)
		texCoords[t] = pfMalloc(sizeof(pfVec2) * 2 * numLines, 
					bldr->sharedArena);
	    j = 0;
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		for (t = 0 ; t < tbind ; t ++)
		{
		    pfCopyVec2(texCoords[t][j  ], l1->texCoords[t][0]);
		    pfCopyVec2(texCoords[t][j+1], l1->texCoords[t][1]);
		}
		j += 2;
		l1 = l1->next;
	    }
 	    for (t = 0 ; t < tbind ; t ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, 
				PFGS_PER_VERTEX, texCoords[t], NULL);
	}
	{
	    coords = pfMalloc(sizeof(pfVec3) * 2 * numLines, bldr->sharedArena);
	    j = 0;
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		pfCopyVec3(coords[j++], l1->coords[0]);
		pfCopyVec3(coords[j++], l1->coords[1]);

		l1 = l1->next;
	    }
	    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	}

	/* perform line strip generation */
	if (bldr->mesh)
	{
	    pfGeoSet *strip = NULL;
	    strip = pfdMeshGSet(gset);
	    if (strip != NULL)
	    {
		pfDelete(gset);
		gset = strip;
	    }
	}

        pfAdd(bldr->gsetList, gset);
	lines = l1;
    }
    else if ((totalLineStrips!=0) && ((totalLines==0) ||
		(l1->pixelsize<l2->pixelsize))) /* line strips only */
    {
        gset = pfNewGSet(bldr->sharedArena);
        numLineStrips = 1;
	numLineStripVerts = l2->numVerts;
        while ((totalLineStrips>1) && (l2->pixelsize==l2->next->pixelsize))
        {
            numLineStrips++;
	    numLineStripVerts += l2->next->numVerts;
            l2 = l2->next;
	    totalLineStrips--;
        }
	totalLineStrips--;

        pfGSetPrimType(gset, PFGS_LINESTRIPS);
        pfGSetNumPrims(gset, numLineStrips);

	lengthList = (int *) pfMalloc( sizeof(int) * numLineStrips, bldr->sharedArena);
	pfGSetPrimLengths(gset, lengthList);
	for (k=0, l2=linestrips; k<numLineStrips; k++, l2=l2->next)
	{
	    lengthList[k] = l2->numVerts;
	}

	pfGSetLineWidth(gset, linestrips->pixelsize);

        switch (cbind)
        {
        case PFGS_PER_VERTEX:
            colors = pfMalloc(sizeof(pfVec4) * numLineStripVerts, bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
		for (m = 0; m < l2->numVerts; m++) {
                    pfCopyVec4(colors[j++], l2->colors[m]);
		}
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, NULL);
            break;

        case PFGS_PER_PRIM:
            colors = pfMalloc(sizeof(pfVec4) * numLineStrips, bldr->sharedArena);
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                pfCopyVec4(colors[i], l2->colors[0]);
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_PRIM, colors, NULL);
            break;

        case PFGS_OVERALL:
            colors = pfMalloc(sizeof(pfVec4), bldr->sharedArena);
            pfCopyVec4(colors[0], linestrips->colors[0]);
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_COLOR4, cbind, NULL, NULL);
            break;
        }

        switch (nbind)
        {
        case PFGS_PER_VERTEX:
            norms = pfMalloc(sizeof(pfVec3) * numLineStripVerts, bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++) {
                    pfCopyVec3(norms[j++], l2->norms[m]);
		}
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
            break;

        case PFGS_PER_PRIM:
            norms = pfMalloc(sizeof(pfVec3) * numLineStrips, bldr->sharedArena);
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                pfCopyVec3(norms[i], l2->norms[0]);
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_PRIM, norms, NULL);
            break;

        case PFGS_OVERALL:
            norms = pfMalloc(sizeof(pfVec3), bldr->sharedArena);
            pfCopyVec3(norms[0], linestrips->norms[0]);
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, norms, NULL);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_NORMAL3, nbind, NULL, NULL);
            break;
        }
        if (tbind != 0)
        {
 	    for (t = 0 ; t < tbind ; t ++)
		texCoords[t] = pfMalloc(sizeof(pfVec2) * numLineStripVerts, 
					bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
		for (m = 0; m < l2->numVerts; m++) 
		{
		    for (t = 0 ; t < tbind ; t ++)
			pfCopyVec2(texCoords[t][j], l2->texCoords[t][m]);

		    j ++;
		}
                l2 = l2->next;
            }
 	    for (t = 0 ; t < tbind ; t ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, 
				PFGS_PER_VERTEX, texCoords[t], NULL);
        }
        {
            coords = pfMalloc(sizeof(pfVec3) * numLineStripVerts, bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
		for (m = 0; m < l2->numVerts; m++)
                {
                    pfCopyVec3(coords[j++], l2->coords[m]);
		}
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
        }

        linestrips = l2;
        pfAdd(bldr->gsetList, gset);
    }
    else
    {
        gset = pfNewGSet(bldr->sharedArena);
	numLines = 1;
        numLineStrips = 1;
        numLineStripVerts = l2->numVerts;
        while ((totalLineStrips>1) && (l2->pixelsize==l2->next->pixelsize))
        {
            numLineStrips++;
            numLineStripVerts += l2->next->numVerts;
            l2 = l2->next;
            totalLineStrips--;
        }
	totalLineStrips--;

        while ((totalLines>1) && (l1->pixelsize==l1->next->pixelsize))
        {
            numLines++;
            l1 = l1->next;
            totalLines--;
        }
	totalLines--;

	m = numLineStrips + numLines;
        pfGSetPrimType(gset, PFGS_LINESTRIPS);
        pfGSetNumPrims(gset, m);

        lengthList = (int *) pfMalloc( sizeof(int) * m, bldr->sharedArena);
        pfGSetPrimLengths(gset, lengthList);
	for (k=0, l1=lines; k<numLines; l1=l1->next, k++) 
	{
	    lengthList[k] = 2;
	}
	for (l2=linestrips; k<m; l2=l2->next, k++)
	{
	    lengthList[k] = l2->numVerts;
	}
        pfGSetLineWidth(gset, linestrips->pixelsize);

        switch (cbind)
        {
        case PFGS_PER_VERTEX:
            colors = pfMalloc(sizeof(pfVec4) * (2*numLines + numLineStripVerts), bldr->sharedArena);
            j = 0;
	    l1 = lines;
            l2 = linestrips;
	    for (i = 0; i < numLines; i++) 
	    {
                pfCopyVec4(colors[j++], l1->colors[0]);
                pfCopyVec4(colors[j++], l1->colors[1]);
	
		l1 = l1->next;
	    }
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++) {
                    pfCopyVec4(colors[j++], l2->colors[m]);
                }
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, NULL);
            break;

        case PFGS_PER_PRIM:
            colors = pfMalloc(sizeof(pfVec4) * (numLines+numLineStrips), bldr->sharedArena);
	    j = 0;
	    l1 = lines;
            l2 = linestrips;

            for (i = 0; i < numLines; i++)
            {
                pfCopyVec4(colors[j++], l1->colors[0]);
		l1 = l1->next;
	    }
            for (i = 0; i < numLineStrips; i++)
            {
                pfCopyVec4(colors[j++], l2->colors[0]);
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_PRIM, colors, NULL);
            break;

        case PFGS_OVERALL:
            colors = pfMalloc(sizeof(pfVec4), bldr->sharedArena);
            pfCopyVec4(colors[0], linestrips->colors[0]);
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_COLOR4, cbind, NULL, NULL);
            break;
        }

        switch (nbind)
        {
        case PFGS_PER_VERTEX:
            norms = pfMalloc(sizeof(pfVec3) * (2*numLines + numLineStripVerts), bldr->sharedArena);
            j = 0;
            l1 = lines;
            l2 = linestrips;
            for (i = 0; i < numLines; i++)
            {
                pfCopyVec3(norms[j++], l1->norms[0]);
                pfCopyVec3(norms[j++], l1->norms[1]);

                l1 = l1->next;
            }
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++) {
                    pfCopyVec3(norms[j++], l2->norms[m]);
                }
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
            break;

        case PFGS_PER_PRIM:
            norms = pfMalloc(sizeof(pfVec3) * (numLines+numLineStrips), bldr->sharedArena);
            j = 0;
            l1 = lines;
            l2 = linestrips;

            for (i = 0; i < numLines; i++)
            {
                pfCopyVec3(norms[j++], l1->norms[0]);
                l1 = l1->next;
            }
            for (i = 0; i < numLineStrips; i++)
            {
                pfCopyVec3(norms[j++], l2->norms[0]);
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_PRIM, norms, NULL);
            break;

        case PFGS_OVERALL:
            norms = pfMalloc(sizeof(pfVec3), bldr->sharedArena);
            pfCopyVec3(norms[0], linestrips->norms[0]);
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, norms, NULL);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_NORMAL3, nbind, NULL, NULL);
            break;
        }

        if (tbind != 0)
	{
	    for (t = 0 ; t < tbind ; t ++)
		texCoords[t] = pfMalloc(sizeof(pfVec2) * 
			(2*numLines+numLineStripVerts), bldr->sharedArena);
	    j = 0;
	    l1 = lines;
	    l2 = linestrips;
	    for (i = 0; i < numLines; i++)
	    {
		for (t = 0 ; t < tbind ; t ++)
		{
		    pfCopyVec2(texCoords[t][j  ], l1->texCoords[t][0]);
		    pfCopyVec4(texCoords[t][j+1], l1->texCoords[t][1]);
		}

		j += 2;
		l1 = l1->next;
	    }
	    for (i = 0; i < numLineStrips; i++)
	    {
		for (m = 0; m < l2->numVerts; m++) 
		{
		    for (t = 0 ; t < tbind ; t ++)
			pfCopyVec2(texCoords[t][j], l2->texCoords[t][m]);

		    j ++;
		}
		l2 = l2->next;
	    }

	    for (t = 0 ; t < tbind ; t ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, 
				PFGS_PER_VERTEX, texCoords[t], NULL);
	}

        {
	coords = pfMalloc(sizeof(pfVec3) * (2*numLines+numLineStripVerts), bldr->sharedArena);
	j = 0;
	l1 = lines;
	for (i = 0; i < numLines; i++)
	{
	    pfCopyVec3(coords[j++], l1->coords[0]);
	    pfCopyVec3(coords[j++], l1->coords[1]);

	    l1 = l1->next;
	}
	l2 = linestrips;
	for (i = 0; i < numLineStrips; i++)
	{
	    for (m = 0; m < l2->numVerts; m++)
	    {
		pfCopyVec3(coords[j++], l2->coords[m]);
	    }
	    l2 = l2->next;
	}
	pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
        }
	lines = l1;
	linestrips = l2;
        pfAdd(bldr->gsetList, gset);
    }
    }
}

/*
 * makeFlatLineGSets -- Make pfGeoSet(s) for non-indexed flat lines and linestrips
 */
static void
makeFlatLineGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind)
{
    pfGeoSet     *gset;
    int           i, j, k, m, t;
    int	    	  totalLineStrips;
    int	    	  numLineStrips, numLineStripVerts;
    int	   	 *lengthList;
    pfdGeom      *linestrips, *l2;

    pfVec3       *coords	= NULL;
    pfVec3       *norms		= NULL;
    pfVec4       *colors	= NULL;
    pfVec2       *texCoords[PF_MAX_TEXTURES];

    ushort	 *icoords	= NULL;
    ushort	 *inorms	= NULL;
    ushort	 *icolors	= NULL;
    ushort	 *itexCoords[PF_MAX_TEXTURES];

    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
    {
	texCoords[t] = NULL;
	itexCoords[t] = NULL;
    }
    
    linestrips      = bldr->flatlinestripBins  [nbind][cbind][tbind];
    totalLineStrips = bldr->flatlinestripCounts[nbind][cbind][tbind];

    while (totalLineStrips > 0) 
    {
        gset = pfNewGSet(bldr->sharedArena);
	l2 = linestrips;
        numLineStrips = 1;
        numLineStripVerts = l2->numVerts;
        while ((totalLineStrips>1) && (l2->pixelsize==l2->next->pixelsize))
        {
            numLineStrips++;
            numLineStripVerts += l2->next->numVerts;
            l2 = l2->next;
            totalLineStrips--;
        }
	totalLineStrips--;

        pfGSetPrimType(gset, PFGS_FLAT_LINESTRIPS);
        pfGSetNumPrims(gset, numLineStrips);

        lengthList = (int *) pfMalloc( sizeof(int) * numLineStrips, bldr->sharedArena);
        pfGSetPrimLengths(gset, lengthList);
        for (k=0, l2=linestrips; k<numLineStrips; k++, l2=l2->next)
        {
            lengthList[k] = l2->numVerts;
        }

        pfGSetLineWidth(gset, linestrips->pixelsize);

        switch (cbind)
        {
        case PFGS_PER_VERTEX:
            colors = pfMalloc(sizeof(pfVec4) * (numLineStripVerts-numLineStrips), bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts-1; m++) {
                    pfCopyVec4(colors[j++], l2->colors[m]);
                }
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, NULL);
            break;

        case PFGS_PER_PRIM:
            colors = pfMalloc(sizeof(pfVec4) * numLineStrips, bldr->sharedArena);
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                pfCopyVec4(colors[i], l2->colors[0]);
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_PRIM, colors, NULL);
            break;

        case PFGS_OVERALL:
            colors = pfMalloc(sizeof(pfVec4), bldr->sharedArena);
            pfCopyVec4(colors[0], linestrips->colors[0]);
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_COLOR4, cbind, NULL, NULL);
            break;
        }

        switch (nbind)
        {
        case PFGS_PER_VERTEX:
            norms = pfMalloc(sizeof(pfVec3) * (numLineStripVerts-numLineStrips), bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts-1; m++) {
                    pfCopyVec3(norms[j++], l2->norms[m]);
                }
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
            break;

        case PFGS_PER_PRIM:
            norms = pfMalloc(sizeof(pfVec3) * numLineStrips, bldr->sharedArena);
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                pfCopyVec3(norms[i], l2->norms[0]);
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_PRIM, norms, NULL);
            break;

        case PFGS_OVERALL:
            norms = pfMalloc(sizeof(pfVec3), bldr->sharedArena);
            pfCopyVec3(norms[0], linestrips->norms[0]);
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, norms, NULL);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_NORMAL3, nbind, NULL, NULL);
            break;

        }
        if (tbind != 0)
        {
	    for (t = 0 ; t < tbind ; t ++)
		texCoords[t] = pfMalloc(sizeof(pfVec2) * numLineStripVerts, 
						bldr->sharedArena);            
	    j = 0;
	    l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++)
                {
		    for (t = 0 ; t < tbind ; t ++)
			pfCopyVec2(texCoords[t][j], l2->texCoords[t][m]);

		    j ++;
                }
                l2 = l2->next;
            }
	    for (t = 0 ; t < tbind ; t ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, 
				PFGS_PER_VERTEX, texCoords[t], NULL);
        }
        {
            coords = pfMalloc(sizeof(pfVec3) * numLineStripVerts, bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++)
                {
                    pfCopyVec3(coords[j++], l2->coords[m]);
                }
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
        }
        linestrips = l2;
        pfAdd(bldr->gsetList, gset);
    }
}

/*
 * makeIndexedLineGSets -- Make pfGeoSet(s) for indexed lines and linestrips
 */
static void
makeIndexedLineGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind)
{
    pfGeoSet     *gset;
    int           i, j, k, m, t;
    int	    	  totalLines, totalLineStrips;
    int	    	  numLines, numLineStrips, numLineStripVerts;
    int	   	 *lengthList;
    pfdPrim	 *lines, *l1;
    pfdGeom      *linestrips, *l2;
    int icoords_share;
    int ilist_size;

    pfVec3       *coords        = NULL;
    pfVec3       *norms         = NULL;
    pfVec4       *colors        = NULL;
    pfVec2       *texCoords[PF_MAX_TEXTURES];

    ushort       *icoords       = NULL;
    ushort       *inorms        = NULL;
    ushort       *icolors       = NULL;
    ushort       *itexCoords[PF_MAX_TEXTURES];

    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
    {
        texCoords[t] = NULL;
        itexCoords[t] = NULL;
    }

    
    lines           = bldr->indexedLineBins       [nbind][cbind][tbind];
    totalLines      = bldr->indexedLineCounts     [nbind][cbind][tbind];

    linestrips      = bldr->indexedlinestripBins  [nbind][cbind][tbind];
    totalLineStrips = bldr->indexedlinestripCounts[nbind][cbind][tbind];

    while ((totalLines!=0) || (totalLineStrips!=0)) 
    {
	l1 = lines;
	l2 = linestrips;
	if ((totalLines!=0)&&((totalLineStrips==0) ||
		(l1->pixelsize > l2->pixelsize)))
	{
	gset = pfNewGSet(bldr->sharedArena);
	numLines = 1;
	while ((totalLines>1) && (l1->pixelsize==l1->next->pixelsize))
	{
	    numLines++;
	    l1 = l1->next;
	    totalLines--;
	}
	totalLines--;

	pfGSetPrimType(gset, PFGS_LINES);
	pfGSetNumPrims(gset, numLines);
        pfGSetLineWidth(gset, lines->coords[2][0]);

	{
	    icoords = pfMalloc(sizeof(unsigned short) * 2 * numLines,
			       bldr->sharedArena);
	    j = 0;
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		icoords[j++] = l1->icoords[0];
		icoords[j++] = l1->icoords[1];

		l1 = l1->next;
	    }
	    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, 
		       bldr->coordList, icoords);
	}

	switch (cbind)
	{
	case PFGS_PER_VERTEX:
	    ilist_size = sizeof(unsigned short) * 2 * numLines;
            icolors = pfMalloc(ilist_size, bldr->sharedArena);
	    j = 0;
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		icolors[j++] = l1->icolors[0];
		icolors[j++] = l1->icolors[1];

		l1 = l1->next;
	    }
	    if (bldr->shareIndexLists &&
		memcmp(icoords, icolors, ilist_size) == 0)
	    {
		pfFree(icolors);
		icolors = icoords;
	    }
	    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX,
		       bldr->colorList, icolors);
	    break;

	case PFGS_PER_PRIM:
	    icolors = pfMalloc(sizeof(unsigned short) * numLines, bldr->sharedArena);
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		icolors[i] = l1->icolors[0];

		l1 = l1->next;
	    }
	    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_PRIM, bldr->colorList, icolors);
	    break;

	case PFGS_OVERALL:
	    icolors = pfMalloc(sizeof(unsigned short), bldr->sharedArena);
	    icolors[0] = lines->icolors[0];
	    pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, bldr->colorList, icolors);
	    break;

	case PFGS_OFF:
	    pfGSetAttr(gset, PFGS_COLOR4, cbind, NULL, NULL);
	    break;
	}

	switch (nbind)
	{
	case PFGS_PER_VERTEX:
	    ilist_size = sizeof(unsigned short) * 2 * numLines;
            inorms = pfMalloc(ilist_size, bldr->sharedArena);
	    j = 0;
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		inorms[j++] = l1->inorms[0];
		inorms[j++] = l1->inorms[1];

		l1 = l1->next;
	    }
	    if (bldr->shareIndexLists &&
		memcmp(icoords, inorms, ilist_size) == 0)
	    {
		pfFree(inorms);
		inorms = icoords;
	    }
	    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, 
						bldr->normList, inorms);
	    break;

	case PFGS_PER_PRIM:
	    inorms = pfMalloc(sizeof(unsigned short) * numLines, bldr->sharedArena);
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		inorms[i] = l1->inorms[0];

		l1 = l1->next;
	    }
	    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_PRIM, 
						bldr->normList, inorms);
	    break;

	case PFGS_OVERALL:
	    inorms = pfMalloc(sizeof(unsigned short), bldr->sharedArena);
	    inorms[0] = lines->inorms[0];
	    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, 
						bldr->normList, inorms);
	    break;

	case PFGS_OFF:
	    pfGSetAttr(gset, PFGS_NORMAL3, nbind, NULL, NULL);
	    break;
	}

	if (tbind != 0)
	{
	    ilist_size = sizeof(unsigned short) * 2 * numLines;
	    for (t = 0 ; t < tbind ; t ++)
		itexCoords[t] = pfMalloc(ilist_size, bldr->sharedArena);
	    j = 0;
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		for (t = 0 ; t < tbind ; t ++)
		{
		    itexCoords[t][j  ] = l1->itexCoords[t][0];
		    itexCoords[t][j+1] = l1->itexCoords[t][1];
		}

		j += 2;
		l1 = l1->next;
	    }

	    for (t = 0 ; t < tbind ; t ++)
		if (bldr->shareIndexLists &&
		    memcmp(icoords, itexCoords[t], ilist_size) == 0)
		{
		    pfFree(itexCoords[t]);
		    itexCoords[t] = icoords;
		}

	    for (t = 0 ; t < tbind ; t ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, PFGS_PER_VERTEX,
					bldr->texCoordList[t], itexCoords[t]);
	}


	/* perform line strip generation */
	if (bldr->mesh)
	{
	    pfGeoSet *strip = NULL;
	    strip = pfdMeshGSet(gset);
	    if (strip != NULL)
	    {
		pfDelete(gset);
		gset = strip;
	    }
	}

	lines = l1;
        pfAdd(bldr->gsetList, gset);
    }
    else if ((totalLineStrips!=0)&&((totalLines==0) ||
	(l1->pixelsize < l2->pixelsize))) /* line strips */
    {
        gset = pfNewGSet(bldr->sharedArena);
        numLineStrips = 1;
	numLineStripVerts = l2->numVerts;
        while ((totalLineStrips>1) && (l2->pixelsize==l2->next->pixelsize))
        {
            numLineStrips++;
	    numLineStripVerts += l2->next->numVerts;
            l2 = l2->next;
	    totalLineStrips--;
        }
	totalLineStrips--;

        pfGSetPrimType(gset, PFGS_LINESTRIPS);
        pfGSetNumPrims(gset, numLineStrips);

	lengthList = (int *) pfMalloc( sizeof(int) * numLineStrips, bldr->sharedArena);
	pfGSetPrimLengths(gset, lengthList);
	for (k=0, l2=linestrips; k<numLineStrips; k++, l2=l2->next)
	{
	    lengthList[k] = l2->numVerts;
	}

	pfGSetLineWidth(gset, linestrips->pixelsize);

	{
	    icoords = pfMalloc(sizeof(unsigned short) * numLineStripVerts,
			       bldr->sharedArena);
	    j = 0;
	    l2 = linestrips;
	    for (i = 0; i < numLineStrips; i++)
	    {
		for (m = 0; m < l2->numVerts; m++)
		{
		    icoords[j++] = l2->icoords[m];
		}
		l2 = l2->next;
	    }
	    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, 
		       bldr->coordList, icoords);
	}

        switch (cbind)
        {
        case PFGS_PER_VERTEX:
	    ilist_size = sizeof(unsigned short) * numLineStripVerts;
            icolors = pfMalloc(ilist_size, bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
		for (m = 0; m < l2->numVerts; m++) {
                    icolors[j++] = l2->icolors[m];
		}
                l2 = l2->next;
            }
	    if (bldr->shareIndexLists &&
		memcmp(icoords, icolors, ilist_size) == 0)
	    {
		pfFree(icolors);
		icolors = icoords;
	    }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, 
				bldr->colorList, icolors);
            break;

        case PFGS_PER_PRIM:
            icolors = pfMalloc(sizeof(unsigned short) * numLineStrips, bldr->sharedArena);
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                icolors[i] = l2->icolors[0];
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_PRIM, 
                                    bldr->colorList, icolors);
            break;

        case PFGS_OVERALL:
            icolors = pfMalloc(sizeof(unsigned short), bldr->sharedArena);
            icolors[0] = linestrips->icolors[0];
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, 
                                    bldr->colorList, icolors);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_COLOR4, cbind, NULL, NULL);
            break;
        }

        switch (nbind)
        {
        case PFGS_PER_VERTEX:
	    ilist_size = sizeof(unsigned short) * numLineStripVerts;
            inorms = pfMalloc(ilist_size, bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++) {
                    inorms[j++] = l2->inorms[m];
		}
                l2 = l2->next;
            }
	    if (bldr->shareIndexLists &&
		memcmp(icoords, inorms, ilist_size) == 0)
	    {
		pfFree(inorms);
		inorms = icoords;
	    }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, 
                                                bldr->normList, inorms);
            break;

        case PFGS_PER_PRIM:
            inorms = pfMalloc(sizeof(unsigned short) * numLineStrips, bldr->sharedArena);
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                inorms[i] = l2->inorms[0];
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_PRIM, 
                                                bldr->normList, inorms);
            break;

        case PFGS_OVERALL:
            inorms = pfMalloc(sizeof(unsigned short), bldr->sharedArena);
            inorms[0] = linestrips->inorms[0];
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, 
                                                bldr->normList, inorms);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_NORMAL3, nbind, NULL, NULL);
            break;
        }
        if (tbind != 0)
        {
	    ilist_size = sizeof(unsigned short) * numLineStripVerts;

	    for (t = 0 ; t < tbind ; t ++)
		itexCoords[t] = pfMalloc(ilist_size, bldr->sharedArena);

	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
		for (m = 0; m < l2->numVerts; m++) 
		{
		    for (t = 0 ; t < tbind ; t ++)
			itexCoords[t][j] = l2->itexCoords[t][m];

		    t ++;
		}
                l2 = l2->next;
            }

	    for (t = 0 ; t < tbind ; t ++)
		if (bldr->shareIndexLists &&
		    memcmp(icoords, itexCoords[t], ilist_size) == 0)
		{
		    pfFree(itexCoords[t]);
		    itexCoords[t] = icoords;
		}

	    for (t = 0 ; t < tbind ; t ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, PFGS_PER_VERTEX,  
                                    bldr->texCoordList[t], itexCoords[t]);
        }

        linestrips = l2;
        pfAdd(bldr->gsetList, gset);
    }
    else
    {
        gset = pfNewGSet(bldr->sharedArena);
	numLines = 1;
        numLineStrips = 1;
        numLineStripVerts = l2->numVerts;
        while ((totalLineStrips>1) && (l2->pixelsize==l2->next->pixelsize))
        {
            numLineStrips++;
            numLineStripVerts += l2->next->numVerts;
            l2 = l2->next;
            totalLineStrips--;
        }
	totalLineStrips--;
        while ((totalLines>1) && (l1->pixelsize==l1->next->pixelsize))
        {
            numLines++;
            l1 = l1->next;
            totalLines--;
        }
	totalLines--;

	m = numLineStrips + numLines;
        pfGSetPrimType(gset, PFGS_LINESTRIPS);
        pfGSetNumPrims(gset, m);

        lengthList = (int *) pfMalloc( sizeof(int) * m, bldr->sharedArena);
        pfGSetPrimLengths(gset, lengthList);
	for (k=0, l1=lines; k<numLines; l1=l1->next, k++) 
	{
	    lengthList[k] = 2;
	}
	for (l2=linestrips; k<m; l2=l2->next, k++)
	{
	    lengthList[k] = l2->numVerts;
	}
        pfGSetLineWidth(gset, linestrips->pixelsize);

	{
	    icoords = pfMalloc(sizeof(unsigned short) *
			       (2*numLines+numLineStrips), bldr->sharedArena);
	    j = 0;
	    l1 = lines;
	    for (i = 0; i < numLines; i++)
	    {
		icoords[j++] = l1->icoords[0];
		icoords[j++] = l1->icoords[1];

		l1 = l1->next;
	    }
	    l2 = linestrips;
	    for (i = 0; i < numLineStrips; i++)
	    {
		for (m = 0; m < l2->numVerts; m++)
		{
		    icoords[j++] = l2->icoords[m];
		}
		l2 = l2->next;
	    }
	    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, 
		       bldr->coordList, icoords);
	}
	
        switch (cbind)
        {
        case PFGS_PER_VERTEX:
	    ilist_size = sizeof(unsigned short) *
			 (2*numLines + numLineStripVerts);
            icolors = pfMalloc(ilist_size, bldr->sharedArena);
            j = 0;
	    l1 = lines;
            l2 = linestrips;
	    for (i = 0; i < numLines; i++) 
	    {
                icolors[j++] = l1->icolors[0];
                icolors[j++] = l1->icolors[1];
	
		l1 = l1->next;
	    }
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++) {
                    icolors[j++] = l2->icolors[m];
                }
                l2 = l2->next;
            }
	    if (bldr->shareIndexLists &&
		memcmp(icoords, icolors, ilist_size) == 0)
	    {
		pfFree(icolors);
		icolors = icoords;
	    }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, 
                                    bldr->colorList, icolors);
            break;

        case PFGS_PER_PRIM:
            icolors = pfMalloc(sizeof(unsigned short) * (numLines+numLineStrips), bldr->sharedArena);
	    j = 0;
	    l1 = lines;
            l2 = linestrips;

            for (i = 0; i < numLines; i++)
            {
                icolors[j++] = l1->icolors[0];
		l1 = l1->next;
	    }
            for (i = 0; i < numLineStrips; i++)
            {
                icolors[j++] = l2->icolors[0];
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_PRIM, 
                                    bldr->colorList, icolors);
            break;

        case PFGS_OVERALL:
            icolors = pfMalloc(sizeof(unsigned short), bldr->sharedArena);
            icolors[0] = linestrips->icolors[0];
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, 
                                    bldr->colorList, icolors);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_COLOR4, cbind, NULL, NULL);
            break;
        }

        switch (nbind)
        {
        case PFGS_PER_VERTEX:
	    ilist_size = sizeof(unsigned short) *
			 (2*numLines + numLineStripVerts);
            inorms = pfMalloc(ilist_size, bldr->sharedArena);
            j = 0;
            l1 = lines;
            l2 = linestrips;
            for (i = 0; i < numLines; i++)
            {
                inorms[j++] = l1->inorms[0];
                inorms[j++] = l1->inorms[1];

                l1 = l1->next;
            }
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++) {
                    inorms[j++] = l2->inorms[m];
                }
                l2 = l2->next;
            }
	    if (bldr->shareIndexLists &&
		memcmp(icoords, inorms, ilist_size) == 0)
	    {
		pfFree(inorms);
		inorms = icoords;
	    }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, 
                                                bldr->normList, inorms);
            break;

        case PFGS_PER_PRIM:
            inorms = pfMalloc(sizeof(unsigned short) * (numLines+numLineStrips), bldr->sharedArena);
            j = 0;
            l1 = lines;
            l2 = linestrips;

            for (i = 0; i < numLines; i++)
            {
                inorms[j++] = l1->inorms[0];
                l1 = l1->next;
            }
            for (i = 0; i < numLineStrips; i++)
            {
                inorms[j++] = l2->inorms[0];
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_PRIM, 
                                                bldr->normList, inorms);
            break;

        case PFGS_OVERALL:
            inorms = pfMalloc(sizeof(unsigned short), bldr->sharedArena);
            inorms[0] = linestrips->inorms[0];
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, 
                                                bldr->normList, inorms);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_NORMAL3, nbind, NULL, NULL);
            break;
        }

        if (tbind != 0)
        {
	    ilist_size = sizeof(unsigned short) *
			 (2*numLines + numLineStripVerts);

	    for (t = 0 ; t < tbind ; t ++)
		itexCoords[t] = pfMalloc(ilist_size, bldr->sharedArena);

            j = 0;
            l1 = lines;
            l2 = linestrips;
            for (i = 0; i < numLines; i++)
            {
		for (t = 0 ; t < tbind ; t ++)
		{
		    itexCoords[t][j  ] = l1->itexCoords[t][0];
		    itexCoords[t][j+1] = l1->itexCoords[t][1];
		}

		j += 2;
                l1 = l1->next;
            }
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++) 
		{
		    for (t = 0 ; t < tbind ; t ++)
			itexCoords[t][j] = l2->itexCoords[t][m];
                }
		j ++;
                l2 = l2->next;
            }
	    for (t = 0 ; t < tbind ; t ++)
		if (bldr->shareIndexLists &&
		    memcmp(icoords, itexCoords[t], ilist_size) == 0)
		{
		    pfFree(itexCoords[t]);
		    itexCoords[t] = icoords;
		}

	    for (t = 0 ; t < tbind ; t ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, PFGS_PER_VERTEX, 
                                    bldr->texCoordList[t], itexCoords[t]);
	}

	lines = l1;
	linestrips = l2;
        pfAdd(bldr->gsetList, gset);
    }
    }
}

/*
 * makeIndexedFlatLineGSets -- Make pfGeoSet(s) for indexed flat lines and linestrips
 */
static void
makeIndexedFlatLineGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind)
{
    pfGeoSet     *gset;
    int           i, j, k, m, t;
    int	    	  totalLineStrips;
    int	    	  numLineStrips, numLineStripVerts;
    int	   	 *lengthList;
    pfdGeom      *linestrips, *l2;

    pfVec3       *coords        = NULL;
    pfVec3       *norms         = NULL;
    pfVec4       *colors        = NULL;
    pfVec2       *texCoords[PF_MAX_TEXTURES];

    ushort       *icoords       = NULL;
    ushort       *inorms        = NULL;
    ushort       *icolors       = NULL;
    ushort       *itexCoords[PF_MAX_TEXTURES];

    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
    {
        texCoords[t] = NULL;
        itexCoords[t] = NULL;
    }

    
    linestrips      = bldr->indexedflatlinestripBins  [nbind][cbind][tbind];
    totalLineStrips = bldr->indexedflatlinestripCounts[nbind][cbind][tbind];

    while (totalLineStrips > 0) 
    {
	l2 = linestrips;
        gset = pfNewGSet(bldr->sharedArena);
        numLineStrips = 1;
        numLineStripVerts = l2->numVerts;
        while ((totalLineStrips>1) && (l2->pixelsize==l2->next->pixelsize))
        {
            numLineStrips++;
            numLineStripVerts += l2->next->numVerts;
            l2 = l2->next;
            totalLineStrips--;
        }
	totalLineStrips--;

        pfGSetPrimType(gset, PFGS_FLAT_LINESTRIPS);
        pfGSetNumPrims(gset, numLineStrips);

        lengthList = (int *) pfMalloc( sizeof(int) * numLineStrips, bldr->sharedArena);
        pfGSetPrimLengths(gset, lengthList);
        for (k=0, l2=linestrips; k<numLineStrips; k++, l2=l2->next)
        {
            lengthList[k] = l2->numVerts;
        }

        pfGSetLineWidth(gset, linestrips->pixelsize);

        switch (cbind)
        {
        case PFGS_PER_VERTEX:
            icolors = pfMalloc(sizeof(unsigned short) * (numLineStripVerts-numLineStrips), bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts-1; m++) {
                    icolors[j++] = l2->icolors[m];
                }
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, 
                                    bldr->colorList, icolors);
            break;

        case PFGS_PER_PRIM:
            icolors = pfMalloc(sizeof(unsigned short) * numLineStrips, bldr->sharedArena);
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                icolors[i] = l2->icolors[0];
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_PRIM, 
                                    bldr->colorList, icolors);
            break;

        case PFGS_OVERALL:
            icolors = pfMalloc(sizeof(unsigned short), bldr->sharedArena);
            icolors[0] = linestrips->icolors[0];
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, 
                                    bldr->colorList, icolors);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_COLOR4, cbind, NULL, NULL);
            break;
        }

        switch (nbind)
        {
        case PFGS_PER_VERTEX:
            inorms = pfMalloc(sizeof(unsigned short) * (numLineStripVerts-numLineStrips), bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts-1; m++) {
                    inorms[j++] = l2->inorms[m];
                }
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, 
                                                bldr->normList, inorms);
            break;

        case PFGS_PER_PRIM:
            inorms = pfMalloc(sizeof(unsigned short) * numLineStrips, bldr->sharedArena);
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                inorms[i] = l2->inorms[0];
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_PRIM, 
                                                bldr->normList, inorms);
            break;

        case PFGS_OVERALL:
            inorms = pfMalloc(sizeof(unsigned short), bldr->sharedArena);
            inorms[0] = linestrips->inorms[0];
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, 
                                                bldr->normList, inorms);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_NORMAL3, nbind, NULL, NULL);
            break;

        }
        if (tbind != 0)
        {
	    for (t = 0 ; t < tbind ; t ++)
		itexCoords[t] = pfMalloc(sizeof(unsigned short) * 
					numLineStripVerts, bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++)
                {
		    for (t = 0 ; t < tbind ; t ++)
			itexCoords[t][j] = l2->itexCoords[t][m];

		    j ++;
                }
                l2 = l2->next;
            }

	    for (t = 0 ; t < tbind ; t ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, PFGS_PER_VERTEX, 
                                    bldr->texCoordList[t], itexCoords[t]);
        }
        {
            icoords = pfMalloc(sizeof(unsigned short) * numLineStripVerts, bldr->sharedArena);
	    j = 0;
            l2 = linestrips;
            for (i = 0; i < numLineStrips; i++)
            {
                for (m = 0; m < l2->numVerts; m++)
                {
                    icoords[j++] = l2->icoords[m];
                }
                l2 = l2->next;
            }
            pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, 
                                        bldr->coordList, icoords);
        }

        linestrips = l2;
        pfAdd(bldr->gsetList, gset);
    }
}

/*
 * makePointGSets -- Make pfGeoSet(s) for non-indexed points
 */
static void
makePointGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind)
{
    pfGeoSet     *gset;
    int           i, t;
    int	    	  numPoints, totalPoints;
    pfdPrim	 *p, *points;

    pfVec3       *coords        = NULL;
    pfVec3       *norms         = NULL;
    pfVec4       *colors        = NULL;
    pfVec2       *texCoords[PF_MAX_TEXTURES];

    ushort       *icoords       = NULL;
    ushort       *inorms        = NULL;
    ushort       *icolors       = NULL;
    ushort       *itexCoords[PF_MAX_TEXTURES];

    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
    {
        texCoords[t] = NULL;
        itexCoords[t] = NULL;
    }
    
    points      = bldr->pointBins  [nbind][cbind][tbind];
    totalPoints = bldr->pointCounts[nbind][cbind][tbind];

    while (totalPoints > 0) 
    {
        gset = pfNewGSet(bldr->sharedArena);
	p = points;
        numPoints = 1;
        while ((totalPoints>1) && (p->pixelsize==p->next->pixelsize))
        {
            numPoints++;
            p = p->next;
            totalPoints--;
        }
	totalPoints--;

        pfGSetPrimType(gset, PFGS_POINTS);
        pfGSetNumPrims(gset, numPoints);

	pfGSetPntSize(gset, points->pixelsize);

        switch (cbind)
        {
        case PFGS_PER_VERTEX:
        case PFGS_PER_PRIM:
            colors = pfMalloc(sizeof(pfVec4) * numPoints, bldr->sharedArena);
	    p = points;
            for (i = 0; i < numPoints; i++)
            {
                pfCopyVec4(colors[i], p->colors[0]);
                p = p->next;
            }
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, NULL);
            break;

        case PFGS_OVERALL:
            colors = pfMalloc(sizeof(pfVec4), bldr->sharedArena);
            pfCopyVec4(colors[0], points->colors[0]);
            pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_COLOR4, cbind, NULL, NULL);
            break;
        }

        switch (nbind)
        {
        case PFGS_PER_VERTEX:
	case PFGS_PER_PRIM:
            norms = pfMalloc(sizeof(pfVec3) * numPoints, bldr->sharedArena);
            p = points; 
            for (i = 0; i < numPoints; i++)
            {
                pfCopyVec3(norms[i], p->norms[0]);
                p = p->next;
            }
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
            break;

        case PFGS_OVERALL:
            norms = pfMalloc(sizeof(pfVec3), bldr->sharedArena);
            pfCopyVec3(norms[0], points->norms[0]);
            pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, norms, NULL);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_NORMAL3, nbind, NULL, NULL);
            break;

        }
        if (tbind != 0)
        {
	    for (t = 0 ; t < tbind ; t ++)
		texCoords[t] = pfMalloc(sizeof(pfVec2) * numPoints, 
					bldr->sharedArena);
	    p = points;
            for (i = 0; i < numPoints; i++)
            {
		for (t = 0 ; t < tbind ; t ++)
		    pfCopyVec2(texCoords[t][i], p->texCoords[t][0]);

                p = p->next;
            }

	    for (t = 0 ; t < tbind ; t ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, PFGS_PER_VERTEX, 
					texCoords[t], NULL);
        }
        {
            coords = pfMalloc(sizeof(pfVec3) * numPoints, bldr->sharedArena);
            p = points;
            for (i = 0; i < numPoints; i++)
            {
                pfCopyVec3(coords[i], p->coords[0]);
                p = p->next;
            }
            pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
        }

	points = p;
        pfAdd(bldr->gsetList, gset);
    }
}

/*
 * makeIndexedPointGSets -- Make pfGeoSet(s) for indexed points
 */
static void
makeIndexedPointGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind)
{
    pfGeoSet     *gset;
    int           i, t;
    int	    	  numPoints, totalPoints;
    pfdPrim	 *p, *points;
    int icoords_share;

    pfVec3       *norms         = NULL;
    pfVec4       *colors        = NULL;
    pfVec2       *texCoords[PF_MAX_TEXTURES];

    ushort       *icoords       = NULL;
    ushort       *inorms        = NULL;
    ushort       *icolors       = NULL;
    ushort       *itexCoords[PF_MAX_TEXTURES];

    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
    {
        texCoords[t] = NULL;
        itexCoords[t] = NULL;
    }

    
    points      = bldr->indexedPointBins  [nbind][cbind][tbind];
    totalPoints = bldr->indexedPointCounts[nbind][cbind][tbind];

    while (totalPoints > 0)
    {
        gset = pfNewGSet(bldr->sharedArena);
        p = points;
        numPoints = 1;
        while ((totalPoints>1) && (p->pixelsize==p->next->pixelsize))
        {
            numPoints++;
            p = p->next;
            totalPoints--;
        }
	totalPoints--;

        pfGSetPrimType(gset, PFGS_POINTS);
        pfGSetNumPrims(gset, numPoints);

        pfGSetPntSize(gset, points->pixelsize);

        {
	    icoords = pfMalloc(sizeof(unsigned short) * numPoints,
			       bldr->sharedArena);
	    p = points;
	    for (i = 0; i < numPoints; i++)
	    {
		icoords[i] = p->icoords[0];
		p = p->next;
	    }
	    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, bldr->coordList,
		       icoords);
    	}

        switch (cbind)
	{
	case PFGS_PER_VERTEX:
	case PFGS_PER_PRIM:
	    p = points;
	    icoords_share = bldr->shareIndexLists;
	    for (i = 0; icoords_share && i < numPoints; i++)
	    {
		if (icoords[i] != p->icolors[0])
		    icoords_share = FALSE;
		p = p->next;
	    }
	    if (icoords_share)
		icolors = icoords;
	    else
	    {
		icolors = pfMalloc(sizeof(unsigned short) * numPoints,
				   bldr->sharedArena);
		p = points;
		for (i = 0; i < numPoints; i++)
		{
		    icolors[i] = p->icolors[0];
		    p = p->next;
		}
	    }
	    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, bldr->colorList,
		       icolors);
	    break;

	case PFGS_OVERALL:
            icolors = pfMalloc(sizeof(unsigned short), bldr->sharedArena);
            icolors[0] = points->icolors[0];
	    pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, bldr->colorList,
		       icolors);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_COLOR4, cbind, NULL, NULL);
            break;
        }

        switch (nbind)
        {
        case PFGS_PER_VERTEX:
        case PFGS_PER_PRIM:
	    p = points;
	    icoords_share = bldr->shareIndexLists;
	    for (i = 0; icoords_share && i < numPoints; i++)
	    {
		if (icoords[i] != p->inorms[0])
		    icoords_share = FALSE;
		p = p->next;
	    }
	    if (icoords_share)
		inorms = icoords;
	    else
	    {
		inorms = pfMalloc(sizeof(unsigned short) * numPoints,
				  bldr->sharedArena);
		p = points;
		for (i = 0; i < numPoints; i++)
		{
		    inorms[i] = p->inorms[0];
		    p = p->next;
		}
	    }
	    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, bldr->normList,
		       inorms);
	    break;

        case PFGS_OVERALL:
            inorms = pfMalloc(sizeof(unsigned short), bldr->sharedArena);
            inorms[0] = points->inorms[0];
	    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, bldr->normList,
		       inorms);
            break;

        case PFGS_OFF:
            pfGSetAttr(gset, PFGS_NORMAL3, nbind, NULL, NULL);
            break;

        }

        if (tbind != 0)
        {
	    p = points;
	    icoords_share = bldr->shareIndexLists;
	    for (i = 0; icoords_share && i < numPoints; i++)
	    {
		for (t = 0 ; t < tbind ; t ++)
		    if (icoords[i] != p->itexCoords[t][0])
			icoords_share = FALSE;
		p = p->next;
	    }
	    if (icoords_share)
	    {
		for (t = 0 ; t < tbind ; t ++)
		    itexCoords[t] = icoords;
	    }
	    else
	    {
		for (t = 0 ; t < tbind ; t ++)
		    itexCoords[t] = pfMalloc(sizeof(unsigned short) * numPoints,
				      bldr->sharedArena);
		p = points;
		for (i = 0; i < numPoints; i++)
		{
		    for (t = 0 ; t < tbind ; t ++)
			itexCoords[t][i] = p->itexCoords[t][0];

		    p = p->next;
		}
	    }
	    for (t = 0 ; t < tbind ; t ++)
		pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, PFGS_PER_VERTEX,
		       bldr->texCoordList[t], itexCoords[t]);
        }

        points = p;
    	pfAdd(bldr->gsetList, gset);
    }
}

/*
 * makeGSets -- Make pfGeoSet(s) from primitive bins
 */
static void
makeGSets(pfdGeoBuilder *bldr, int nbind, int cbind, int tbind)
{
    /* points */
    makePointGSets(bldr, nbind, cbind, tbind);

    /* indexed points */
    makeIndexedPointGSets(bldr, nbind, cbind, tbind);

    /* lines and line strips */
    makeLineGSets(bldr, nbind, cbind, tbind);

    /*indexed lines, indexed line strips */
    makeIndexedLineGSets(bldr, nbind, cbind, tbind);

    /* [flat lines ??? and] flat line strips */
    makeFlatLineGSets(bldr, nbind, cbind, tbind);

    /* indexed flat lines and indexed flat line strips */
    makeIndexedFlatLineGSets(bldr, nbind, cbind, tbind);

    /* triangles */
    makeTriGSets(bldr, nbind, cbind, tbind);

    /* indexedtriangles */
    makeIndexedTriGSets(bldr, nbind, cbind, tbind);
}

/* ###################################################################### */
/* #                                                                    # */
/* #                   Concave polygon decomposition                    # */
/* #                                                                    # */
/* ###################################################################### */

typedef struct _Vtx
{
    int            index;
    pfVec3          coord;
    struct _Vtx    *next;
} Vtx;

#define VX(p, c)    p->coord[c]

/*
 * Decompose concave polygon into triangles. Return number of triangles.
 */
static int
decompConcave(pfdGeom *pgon, pfdPrim *triList, int asum, int x, int y)
{
    Vtx		*p0, *p1, *p2, *t0, *vert;
    float	xmin, xmax, ymin, ymax;
    int		i, init, csum, chek;
    long	m[3];
    float 	a[3], b[3], c[3], s[3];
    int         numPolys = 0, numTris = 0;
    Vtx		**vList;
    pfdPrim	*tri = triList;

    /* allocate vList */
    vList = (Vtx **)alloca(pgon->numVerts*sizeof(Vtx*));
    for (i = 0; i < pgon->numVerts; i++)
	vList[i] = NULL;

    /* Make linked list of verts */
    vert = (Vtx *) alloca(sizeof(Vtx));
    vert->index = 0;
    if (pgon->flags & PFD_INDEXED)
	pfCopyVec3(vert->coord, pgon->coordList[pgon->icoords[0]]);
    else
	pfCopyVec3(vert->coord, pgon->coords[0]);
    p1 = vert;
    vList[0] = vert;

    for (i = 1; i < pgon->numVerts; i++)
    {
	p0 = (Vtx *) alloca(sizeof(Vtx));
	vList[i] = p0;
	p0->index = i;
	if (pgon->flags & PFD_INDEXED)
	    pfCopyVec3(p0->coord, pgon->coordList[pgon->icoords[i]]);
	else
	    pfCopyVec3(p0->coord, pgon->coords[i]);
	p1->next = p0;
	p1 = p0;
    }
    p1->next = vert;

    p0 = vert;
    p1 = p0->next;
    p2 = p1->next;
    m[0] = (long) p0;
    m[1] = (long) p1;
    m[2] = (long) p2;
    chek = 0;

    while (p0 != p2->next)
    {
	/* Polygon is self-intersecting so punt */
	if (chek && m[0] == (long) p0 && 
		    m[1] == (long) p1 && 
		    m[2] == (long) p2)
	{
#ifdef	CHASE
	    pfdPrim  *t;
#endif

	    if (tri->next != NULL)
		pfNotify(PFNFY_WARN, PFNFY_PRINT, "decompConcave() "
		    "Could not decompose concave polygon!");

#ifdef	CHASE
	    t = triList;
	    while (t != NULL && t->next != tri)
		t = t->next;
	    if (t != NULL)
		t->next = NULL;
#endif
    	    return numTris;
	}

	chek = 1;

	a[0] = VX(p1, y) - VX(p2, y);
	b[0] = VX(p2, x) - VX(p1, x);
	a[2] = VX(p0, y) - VX(p1, y);
	b[2] = VX(p1, x) - VX(p0, x);

	csum = ((b[0] * a[2] - b[2] * a[0] >= 0.0) ? 1 : 0);

	/* current angle is concave */
	if (csum ^ asum)
	{
	    p0 = p1;
	    p1 = p2;
	    p2 = p2->next;
	}

	/* current angle is convex */
	else
	{
	    xmin = (VX(p0, x) < VX(p1, x)) ? VX(p0, x) : VX(p1, x);
	    if (xmin > VX(p2, x))
		xmin = VX(p2, x);

	    xmax = (VX(p0, x) > VX(p1, x)) ? VX(p0, x) : VX(p1, x);
	    if (xmax < VX(p2, x))
		xmax = VX(p2, x);

	    ymin = (VX(p0, y) < VX(p1, y)) ? VX(p0, y) : VX(p1, y);
	    if (ymin > VX(p2, y))
		ymin = VX(p2, y);

	    ymax = (VX(p0, y) > VX(p1, y)) ? VX(p0, y) : VX(p1, y);
	    if (ymax < VX(p2, y))
		ymax = VX(p2, y);

	    for (init = 1, t0 = p2->next; t0 != p0; t0 = t0->next)
		if (VX(t0, x) >= xmin && VX(t0, x) <= xmax &&
		    VX(t0, y) >= ymin && VX(t0, y) <= ymax)
		{
		    if (init)
		    {
			a[1] = VX(p2, y) - VX(p0, y);
			b[1] = VX(p0, x) - VX(p2, x);
			init = 0;
			c[0] = VX(p1, x) * VX(p2, y) - VX(p2, x) * VX(p1, y);
			c[1] = VX(p2, x) * VX(p0, y) - VX(p0, x) * VX(p2, y);
			c[2] = VX(p0, x) * VX(p1, y) - VX(p1, x) * VX(p0, y);
		    }

		    s[0] = a[0] * VX(t0, x) + b[0] * VX(t0, y) + c[0];
		    s[1] = a[1] * VX(t0, x) + b[1] * VX(t0, y) + c[1];
		    s[2] = a[2] * VX(t0, x) + b[2] * VX(t0, y) + c[2];

		    if (asum)
		    {
			if (s[0] >= 0.0 && s[1] >= 0.0 && s[2] >= 0.0)
			    break;
		    }
		    else
			if (s[0] <= 0.0 && s[1] <= 0.0 && s[2] <= 0.0)
			    break;
		}

	    if (t0 != p0)
	    {
		p0 = p1;
		p1 = p2;
		p2 = p2->next;
	    }
	    else
	    {
		copyGeom(tri, pgon, p0->index, p1->index, p2->index); 
		numTris++;

		p0->next = p1->next;
		p1 = p2;
		p2 = p2->next;

	        m[0] = (long) p0;
	        m[1] = (long) p1;
	        m[2] = (long) p2;
	        chek = 0;
		
		tri = tri->next;
	    }
	}
    }

    copyGeom(tri, pgon, p0->index, p1->index, p2->index); 
    numTris++;

    return numTris;
}

/* 
 * Triangulate the polygon pgon, given a linked list of new triangles.
 * This also decomposes concave polygons. Return TRUE if pgon is concave.
 */
static int
triangulatePoly(pfdGeom *pgon, pfdPrim *triList)
{
    register float  *p0, *p1;
    register float  dx1, dy1, dx2, dy2, max;
    register int   i, flag, asum, csum, index, x, y, v0, v1, v, even;
    float	    as[3];
    pfdPrim	    *tri;

    /* First see if polygon is just a triangle */
    if (pgon->numVerts == 3)
    {
	copyGeom(triList, pgon, 0, 1, 2);
	return 1;
    }

    /* calculate signed areas */
    as[0] = as[1] = as[2] = 0.0;

    for (i = 0; i < pgon->numVerts; i++)
    {
	if (pgon->flags & PFD_INDEXED)
	{
	    p0 = pgon->coordList[pgon->icoords[i]];
	    p1 = pgon->coordList[pgon->icoords[(i + 1) % pgon->numVerts]];
	}
	else
	{
	    p0 = pgon->coords[i];
	    p1 = pgon->coords[(i + 1) % pgon->numVerts];
	}
	as[0] += p0[0] * p1[1] - p0[1] * p1[0];
	as[1] += p0[0] * p1[2] - p0[2] * p1[0];
	as[2] += p0[1] * p1[2] - p0[2] * p1[1];
    }

    /* select largest signed area */
    for (max = 0.0, index = 0, i = 0; i < 3; i++)
	if (as[i] >= 0.0)
	{
	    if (as[i] > max)
	    {
		max = as[i];
		index = i;
		flag = 1;
	    }
	}
	else
	{
	    as[i] = -as[i];
	    if (as[i] > max)
	    {
		max = as[i];
		index = i;
		flag = 0;
	    }
	}

    /* pointer offsets */
    switch (index)
    {
    case 0:
	x = 0;
	y = 1;
	break;

    case 1:
	x = 0;
	y = 2;
	break;

    case 2:
	x = 1;
	y = 2;
	break;
    }

    /* concave check */
    if (pgon->flags & PFD_INDEXED)
    {
	p0 = pgon->coordList[pgon->icoords[0]];
	p1 = pgon->coordList[pgon->icoords[1]];
    }
    else
    {
	p0 = pgon->coords[0];
	p1 = pgon->coords[1];
    }
    dx1 = p1[x] - p0[x];
    dy1 = p1[y] - p0[y];
    p0 = p1;
   if (pgon->flags & PFD_INDEXED)
	p1 = pgon->coordList[pgon->icoords[2]];
    else
	p1 = pgon->coords[2];
    dx2 = p1[x] - p0[x];
    dy2 = p1[y] - p0[y];
    asum = ((dx1 * dy2 - dx2 * dy1 >= 0.0) ? 1 : 0);

    for (i = 0; i < pgon->numVerts - 1; i++)
    {
	p0 = p1;

	if (pgon->flags & PFD_INDEXED)
	    p1 = pgon->coordList[pgon->icoords[(i + 3) % pgon->numVerts]];
	else
	    p1 = pgon->coords[(i + 3) % pgon->numVerts];

	dx1 = dx2;
	dy1 = dy2;
	dx2 = p1[x] - p0[x];
	dy2 = p1[y] - p0[y];
	csum = ((dx1 * dy2 - dx2 * dy1 >= 0.0) ? 1 : 0);

	if (csum ^ asum)
	{
	    pgon->flags |= PFD_CONCAVE;
	    return decompConcave(pgon, triList, flag, x, y);
	}
    }

    v0 = 0;
    v1 = 1;
    v = pgon->numVerts - 1;
  
    even = 1;

    tri = triList;

    /* 
     * Convert to triangles only. Do not fan out from a single vertex
     * but zigzag into triangle strip.
     */
    for (i = 0; i < pgon->numVerts - 2; i++)
    {
	if (even)
	{
	    copyGeom(tri, pgon, v0, v1, v);

	    v0 = v1;
	    v1 = v;
	    v = v0 + 1;
	}
	else
	{
	    copyGeom(tri, pgon, v1, v0, v);

	    v0 = v1;
	    v1 = v;
	    v = v0 - 1;
	}

	even = !even;
	tri = tri->next;
    }

    return pgon->numVerts - 2;
}

/* 
 * Triangulate the polygon pgon, given a linked list of new triangles.
 * This also decomposes concave polygons. Return TRUE if pgon is concave.
 */
extern PFDUDLLEXPORT int
pfdTriangulatePoly(pfdGeom *pgon, pfdPrim *triList)
{
    return triangulatePoly(pgon, triList);    
}

/* ###################################################################### */
/* #                                                                    # */
/* #               what is this doing in the pfdGeoBuilder?             # */
/* #                                                                    # */
/* ###################################################################### */

PFDUDLLEXPORT void
pfdPrintGSet(pfGeoSet *gset)
{
    int i, j, k;
    int num, PrimTypes;
    int *lengths;
    pfVec3 *Coords;
    pfVec4 *Colors;
    pfVec2 *texCoords;
    unsigned short *tindex;
    unsigned short *cindex;
    unsigned short *vindex;

    num = pfGetGSetNumPrims(gset);
    pfNotify(PFNFY_ALWAYS, PFNFY_PRINT, "pfdOutputGSet");
    pfNotify(PFNFY_ALWAYS, PFNFY_PRINT, "  numPrims %d", 
	pfGetGSetNumPrims(gset));
    PrimTypes = pfGetGSetPrimType(gset);
    pfNotify(PFNFY_ALWAYS, PFNFY_PRINT, "  primTypes %d", 
	pfGetGSetPrimType(gset));
    pfNotify(PFNFY_ALWAYS, PFNFY_PRINT, "  linewidth %f", 
	pfGetGSetLineWidth(gset));

    pfGetGSetAttrLists(gset, PFGS_COLOR4, (void**)&Colors, &cindex);
    pfNotify(PFNFY_ALWAYS, PFNFY_PRINT,
        "color %f %f %f %f", Colors[0][0], Colors[0][1], Colors[0][2], Colors[0][3]);
    pfNotify(PFNFY_ALWAYS, PFNFY_PRINT,
	"color bind %d", pfGetGSetAttrBind(gset, PFGS_COLOR4));
    pfGetGSetAttrLists(gset, PFGS_TEXCOORD2, (void**)&texCoords, &tindex);
    pfNotify(PFNFY_ALWAYS, PFNFY_PRINT,
        "tex bind %d", pfGetGSetAttrBind(gset, PFGS_TEXCOORD2));
/*
    pfNotify(PFNFY_ALWAYS, PFNFY_PRINT, "texCoords %f %f %f %f %f %f %f %f", 
	texCoords[0][0], texCoords[0][1],
 	texCoords[1][0], texCoords[1][1], 
	texCoords[2][0], texCoords[2][1],
	texCoords[3][0], texCoords[3][1]);
*/

    k = 0;
    if ((PrimTypes == PFGS_LINESTRIPS) ||
	(PrimTypes == PFGS_FLAT_LINESTRIPS))
    {
	lengths = pfGetGSetPrimLengths(gset);
	for (i = 0; i < num; i++) 
	{
	    pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&Coords, &vindex);
	    for (j = 0; j < lengths[i]; j++)
	    {
		pfNotify(PFNFY_ALWAYS, PFNFY_PRINT, 
		    "  vertex %f %f %f", 
		    Coords[k][0], Coords[k][1], Coords[k][2]);
		k++;
	    }
	    pfNotify(PFNFY_ALWAYS, PFNFY_PRINT, "  length %d", lengths[i]);
	}
    }
}
