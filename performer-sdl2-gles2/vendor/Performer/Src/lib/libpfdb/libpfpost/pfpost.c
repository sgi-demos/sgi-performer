/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
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
 * pfpost.c - simple 2D grid loader
 * 
 * $Revision: 1.11 $ 
 * $Date: 2002/11/07 21:29:51 $
 */

/*
 * This is sample loader that uses reads in a simple 2D grid of points,
 * generates pfGeoSets and a spatially organized scene graph.  
 * The .post format is not an official format and is likely to change 
 * substantially or disappear in future releases.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef WIN32
#include <X11/keysym.h>
#endif
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#ifdef WIN32
#include "pfpost.h"
#define MAXFLOAT FLT_MAX

#else
#include <Performer/pfdb/pfpost.h>
#endif

#ifdef __linux__
#include <asm/byteorder.h>
#define MAXFLOAT ((float)1e+37)
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


size_t swapped_fread32(void *ptr, size_t size, size_t nitems, FILE *stream)
{
        unsigned char *p = ptr;
        int i, n;

		if((size != 4) && (nitems > 1))
		{
			n = (size * nitems)/4;
		}
		else if((size != 4) && nitems == 1)
		{
			n = size/4;
		}
		else
		{
			n = nitems;
		}

        /* do 32bit swap */
        nitems = fread(ptr, 4, n, stream);
        for(i=0; i<n; ++i)
        {
            P_32_SWAP(p);
            p += 4;
        }
        return nitems;
}

size_t swapped_fread16(void *ptr, size_t size, size_t nitems, FILE *stream)
{
        unsigned char *p = ptr;
        int i;

        /* do 16bit swap */
        nitems = fread(ptr, 2, nitems, stream);
        for(i=0; i<nitems; ++i)
        {
            P_16_SWAP(p);
            p += 2;
        }
        return nitems;
}

size_t pfb_fread(void *ptr, size_t size, size_t nitems, FILE *stream)
{
#ifdef __LITTLE_ENDIAN
	if(size == 1)
	{
		/* normal fread */
		nitems = fread(ptr, size, nitems, stream);
		return nitems;
	}
	else if(size == 2)
	{
		/* swapped_fread16 */
		nitems = swapped_fread16(ptr, size, nitems, stream);
		return nitems;
	}
	else if((size == 4) || (size%4 == 0))
	{
		/* swapped_fread32 */
		nitems = swapped_fread32(ptr, size, nitems, stream);
		return nitems;
	}
#else
	/* normal fread */
	nitems = fread(ptr, size, nitems, stream);
	return nitems;
#endif
}



/*
 *	pfpost.c -- generate quad or N-tree from post data 
 */

typedef struct
{
    /* tile-group minimum chunk size */
    int		cellSize;
    
    /* tile-group subdivision count */
    int		branch;
    
    /* tile size in x and y */
    int		countX;
    int		countY;
    
    /* grid post origin */
    float	originX;
    float	originY;
    
    /* grid post spacing */
    float	deltaX;
    float	deltaY;
    
    /* grid posts: row-major, (countX+1)*(countY+1) */
    pfVec3	*verts;
    pfVec3	*norms;
    pfVec4	*colors;
    pfVec2	*texCoords;
    
    /* texture information */
    char	textureName[256];
    pfMaterial  *mtl;
    pfTexture	*texture;
    pfTexEnv	*texEnv;
    float	textureScale;
    pfGeoState	*geoState;
} Tile;

static void MakePost(Tile *tile);
static void PrintPost(Tile *tile);
static pfGroup *MakeTile(Tile *tile, int x, int y, int dx, int dy);

#define COLLIDE_MASK 0x20

static int	CellSize = 2;  /* create geosets containing CellSize X CellSize */
static int	Branch = 3;  /* each group has Branch X Branch children */

static char	TexFile[PF_MAXSTRING] = "tgen.rgb";

static int	Colors = 1;
static int	Normals = 1;
static int	MaxCountY = 32768;

#define	INDEX(_t,_r,_c)	(((_t)->countX+1)*(_r)+(_c))

PFPFB_DLLEXPORT void
pfdInitConverter_post(void)
{
}

PFPFB_DLLEXPORT void
pfdConverterMode_post(int mode, int val)
{
    switch (mode) {
    case PFPOST_COLORS:
	Colors = val;
	break;
    case PFPOST_NORMALS:
	Normals = val;
	break;
    case PFPOST_BRANCH:
	Branch = val;
	break;
    case PFPOST_CELLSIZE:
	CellSize = val;
	break;
    default:
 	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfdConverterMode_post: Unknown mode %d", mode);
    }
}

PFPFB_DLLEXPORT int
pfdGetConverterMode_post(int mode)
{
    switch (mode) {
    case PFPOST_COLORS:
	return Colors;
    case PFPOST_NORMALS:
	return Normals;
    case PFPOST_BRANCH:
	return Branch;
    case PFPOST_CELLSIZE:
	return CellSize;
    default:
 	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfdGetConverterMode_post: Unknown mode %d", mode);
	return -1;
    }
}

PFPFB_DLLEXPORT void
pfdConverterAttr_post(int which, void *attr)
{
    switch (which) {
    case PFPOST_TEXFILE:
	if (attr)
	    strcpy(TexFile, (char *)attr);
	break;
    default:
 	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfdConverterAttr_post: Unknown which %d", which);
    }
}

PFPFB_DLLEXPORT void *
pfdGetConverterAttr_post(int which)
{
    switch (which) {
    case PFPOST_TEXFILE:
	return TexFile;
    default:
 	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfdGetConverterAttr_post: Unknown attr %d", which);
	return NULL;
    }
}

PFPFB_DLLEXPORT pfNode*
pfdLoadFile_post(char *postFile)
{
    int	row;
    int	col;
    void	*arena;
    Tile	tile;
    char	filepath[256];
    FILE	*fp;
    float	maxZ = -MAXFLOAT, minT = MAXFLOAT, maxT = -MAXFLOAT, zscale;

    int	numX;
    int	numY;
    float	extent;
    int	numPosts;
    int	numTextures;
    char *texFile = TexFile;
    
    /* open post-file file */
    if (pfFindFile(postFile, filepath, R_OK) < 0)
    {
	fprintf(stderr, "WARNING: unable to find post file \"%s\"\n", postFile);
	return NULL;
    }
    if ((fp = fopen(filepath, "r")) == NULL)
    {
	fprintf(stderr, "ERROR: error opening post file \"%s\"\n", postFile);
	return NULL;
    }
    if (pfFindFile(texFile, filepath, R_OK) < 0)
    {
	fprintf(stderr, "WARNING: unable to find texture file \"%s\"\n", texFile);
	return NULL;
    }
    strcpy(tile.textureName, filepath);

    /* read post-file header */
    pfb_fread(&numX, sizeof(int), 1, fp);
    pfb_fread(&numY, sizeof(int), 1, fp);
    pfb_fread(&extent, sizeof(float), 1, fp);
    pfb_fread(&numPosts, sizeof(int), 1, fp);
    pfb_fread(&numTextures, sizeof(int), 1, fp);

    /* derive tile details from post-file values */
    tile.countX = numX - 1;
    tile.countY = numY - 1;
    tile.originX = 0.0f;
    tile.originY = 0.0f;
    tile.deltaX = extent/tile.countX;
    tile.deltaY = extent/tile.countY;

    printf("nx=%d ny=%d nx*ny=%d, numPosts=%d\n",
	   tile.countX, 
	   tile.countY,
	   tile.countX*tile.countY,
	   numPosts);
    
    tile.cellSize = CellSize;
    tile.branch = Branch;
    
    /* get pointer to shared-memory allocation arena */
    arena = pfGetSharedArena();
    
    /* allocate and initialize texture-texture data */
    tile.texture = pfNewTex(arena);
    if (!pfLoadTexFile(tile.texture, tile.textureName))
	printf("error: unable to load texture \"%s\"\n", tile.textureName);
    
    tile.texEnv = pfNewTEnv(arena);
    pfTEnvMode(tile.texEnv, PFTE_MODULATE);
    
    tile.mtl = pfNewMtl(pfGetSharedArena());
    pfMtlColor(tile.mtl, PFMTL_AMBIENT, 0.2f, 0.2f, 0.2f);
    pfMtlColor(tile.mtl, PFMTL_DIFFUSE, 0.8f, 0.8f, 0.8f);
    pfMtlColorMode(tile.mtl, PFMTL_FRONT, PFMTL_CMODE_AMBIENT_AND_DIFFUSE);

    /* allocate and initialize new geo-state */
    tile.geoState = pfNewGState(arena);

    pfGStateAttr(tile.geoState, PFSTATE_TEXTURE, tile.texture);
    pfGStateAttr(tile.geoState, PFSTATE_TEXENV, tile.texEnv);

    pfGStateAttr(tile.geoState, PFSTATE_FRONTMTL, tile.mtl);
    if (Normals)
	pfGStateMode(tile.geoState, PFSTATE_ENLIGHTING, 1);
    else
	pfGStateMode(tile.geoState, PFSTATE_ENLIGHTING, 0);

    /* allocate vertex data area */
    tile.verts  = (pfVec3 *)pfMalloc(2*numPosts*sizeof(pfVec3), arena);
    tile.norms  = (pfVec3 *)pfMalloc(2*numPosts*sizeof(pfVec3), arena);
    tile.colors   = (pfVec4 *)pfMalloc(2*numPosts*sizeof(pfVec4), arena);
    tile.texCoords = (pfVec2 *)pfMalloc(2*numPosts*sizeof(pfVec2), arena);
    
    /* read vertex data */
    pfb_fread(tile.verts,  numPosts*sizeof(pfVec3), 1, fp);
    pfb_fread(tile.norms,  numPosts*sizeof(pfVec3), 1, fp);
    pfb_fread(tile.colors,   numPosts*sizeof(pfVec4), 1, fp);
    pfb_fread(tile.texCoords, numPosts*sizeof(pfVec2), 1, fp);
    
    /* break asociation with post-data file */
    fclose(fp);
    
    /* pretend there are fewer */
    tile.countY  = PF_MIN2(tile.countY, MaxCountY);

    /* recompute tile X and Y axis coordinates */
    for (row = 0; row <= tile.countY; row++)
	for (col = 0; col <= tile.countX; col++)
	{
	    int index = INDEX(&tile, row, col);
	    maxZ = PF_MAX2(maxZ, tile.verts[index][2]);
	    minT = PF_MIN2(minT, tile.verts[index][0]);
	    minT = PF_MIN2(minT, tile.verts[index][1]);
	    maxT = PF_MAX2(maxT, tile.verts[index][0]);
	    maxT = PF_MAX2(maxT, tile.verts[index][1]);
	}
    
    tile.textureScale = 5.0f / (maxT - minT);
    zscale = 1.0f/maxZ;
    
    for (row = 0; row <= tile.countY; row++)
	for (col = 0; col <= tile.countX; col++)
	{
	    int	index	= INDEX(&tile, row, col);
	    
	    float  *verts  = tile.verts[index];
	    float  *norms  = tile.norms[index];
	    float  *colors   = tile.colors[index];
	    float  *texCoords = tile.texCoords[index];
	    
	    verts[PF_X] = tile.originX + col*tile.deltaX;
	    verts[PF_Y] = tile.originY + row*tile.deltaY;

	    if (PF_ABSLT(verts[PF_Z], 0.0001f))
		verts[PF_Z] = 0.0f;
	    
	    if (PF_ABSLT(norms[PF_X], 0.0001f))
		norms[PF_X] = 0.0f;
	    if (PF_ABSLT(norms[PF_Y], 0.0001f))
		norms[PF_Y] = 0.0f;
	    if (PF_ABSLT(norms[PF_Z], 0.0001f))
		norms[PF_Z] = 0.0f;
	    
#ifdef OWNCOLORS
	    calcColor(zscale * verts[PF_Z], colors);
#else
	    colors[3] = 1.0f;	/* opaque!!! */
#endif
	    
	    texCoords[PF_X] = verts[PF_X]*tile.textureScale;
	    texCoords[PF_Y] = verts[PF_Y]*tile.textureScale;
	}
    
    /* build database hierarchy */
    return (pfNode *)MakeTile(&tile, 0, 0, tile.countX, tile.countY);
}

/*
 *	PrintPost -- print post data
 */

void
PrintPost (Tile *tile)
{
    int row;
    int col;
    int rows = tile->countX + 1;
    int cols = tile->countY + 1;
    
    printf("tile count:  %d %d\n", tile->countX,  tile->countY);
    printf("tile origin: %g %g\n", tile->originX, tile->originY);
    printf("tile delta:  %g %g\n", tile->deltaX,  tile->deltaY);
    
    for (row = 0; row < rows; row++)
    {
	printf("%s", (row == 0) ? "  post data: " : "             ");
	for (col = 0; col < cols; col++)
	    printf("%g ", tile->verts[row*cols + col][PF_Z]);
	printf("\n");
    }
}

/*
 *	MakeTile -- generate a tile
 */

typedef struct {
    int numGroups;
    int numGeodes;
    int numGeosets;
    int numTriangles;
    int numVertices;
} StatData;

pfNode *
_MakeTile (Tile *tile, int minX, int minY, int maxX, int maxY, int depth, 
	   StatData *stats)
{
    pfGeoSet   *gset;
    pfNode *result;
    int numX = maxX - minX;
    int numY = maxY - minY;
    
    /* internal or leaf node */
    if (numX <= tile->cellSize*tile->branch && 
	numY <= tile->cellSize*tile->branch)
    {
	void	*arena;
	pfGeode     *geode;
	
	/* get pointer to shared-memory allocation arena */
	arena = pfGetSharedArena();
	
	/* allocate and initialize new geometry-node */
	geode = pfNewGeode();
	stats->numGeodes++;

	/* allocate and initialize new geo-datas */
	if (numX <= numY)
	{
	    int	x;
	    
	    /* make a triangle-strip for each column */
	    for (x = 0; x < numX; x++)
	    {
		int			y;
		ushort	*index;
		int	*primLens;
		
		/* allocate index list */
		primLens = (int *)pfMalloc(sizeof(int), arena);

		index = (ushort *)pfMalloc((2*numY + 2)*sizeof(ushort),
						 arena);
		/* initialize index list */
		for (y = 0; y <= numY; y++)
		{
		    index[2*y    ] = INDEX(tile, minY + y, minX + x    );
		    index[2*y + 1] = INDEX(tile, minY + y, minX + x + 1);
		}

		/* allocate new geo-data */
		gset = pfNewGSet(arena);
		pfAddGSet(geode, gset);
		stats->numGeosets++;
		
		pfGSetPrimType(gset, PFGS_TRISTRIPS);
		pfGSetNumPrims(gset, 1);
		primLens[0] = 2*numY+2;
		pfGSetPrimLengths(gset, primLens);
		pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX,
			   tile->verts, index);

		if (Colors)
		    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, 
			       tile->colors, index);

		if (Normals)
		    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, 
			       tile->norms, index);

		pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, 
			   tile->texCoords, index);
		
		pfGSetGState(gset, tile->geoState);

		stats->numTriangles += 2*numY;
		stats->numVertices  += 2*(numY + 1);
	    }
	}
	else
	{
	    int	y;
	    
	    /* make a triangle-strip for each row */
	    for (y = 0; y < numY; y++)
	    {
		int			x;
		ushort	*index;
		int	*primLens;
		
		/* allocate index list */
		primLens = (int *)pfMalloc(sizeof(int), arena);
		index = (ushort *)pfMalloc((2*numX + 2)*sizeof(ushort),
						 arena);
		
		/* initialize index list */
		for (x = 0; x <= numX; x++)
		{
		    index[2*x    ] = INDEX(tile, minY + y + 1, minX + x);
		    index[2*x + 1] = INDEX(tile, minY + y,     minX + x);
		}
		
		/* allocate new geo-data */
		gset = pfNewGSet(arena);
		pfAddGSet(geode, gset);
		stats->numGeosets++;
		
		pfGSetPrimType(gset, PFGS_TRISTRIPS);
		pfGSetNumPrims(gset, 1);
		primLens[0] = 2*numX+2;
		pfGSetPrimLengths(gset, primLens);
		pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, 
			   tile->verts, index);
		if (Normals)
		    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, 
			       tile->norms, index);
		if (Colors)
		    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX,
			       tile->colors, index);

		pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX,
			   tile->texCoords, index);
		
		stats->numTriangles += 2*numX;
		stats->numVertices  += 2*(numX + 1);
		
		/* add this column's geo-data to the geo-state */
		pfGSetGState(gset, tile->geoState);
	    }
	}
	
	
	/* return leaf geometry-node as result */
	result = (pfNode *)geode;
    }
    else
    {
	int cellsX;
	int cellsY;
	int groupsX;
	int groupsY;
	int groupSizeX;
	int groupSizeY;
	int row;
	int col;
	
	pfGroup *parent;
	pfNode *child;
	
	/* determine number of cells in each direction */
	cellsX = (numX + tile->cellSize - 1)/tile->cellSize;
	cellsY = (numY + tile->cellSize - 1)/tile->cellSize;
	
	/* how many groups there would be total across */
	groupsX = (cellsX + tile->branch - 1) / tile->branch;
	groupsY = (cellsY + tile->branch - 1) / tile->branch;

	/* limit that by the requested max branching factor */
	groupsX = PF_MIN2(groupsX, tile->branch);
	groupsY = PF_MIN2(groupsY, tile->branch);

	/* cells per group */
	groupSizeX = (cellsX + groupsX - 1 ) / groupsX;
	groupSizeY = (cellsY + groupsY - 1 ) / groupsY;

	/* posts per group */
	groupSizeX *= tile->cellSize;
	groupSizeY *= tile->cellSize;

	/* make group node */
	parent = pfNewGroup();
	stats->numGroups++;
	
	/* generate subordinate tiles */
	for (row = 0; row < groupsY; row++)
	    for (col = 0; col < groupsX; col++)
	    {
		int gx0 = minX + col * groupSizeX;
		int gy0 = minY + row * groupSizeY;
		int gx1 = PF_MIN2(gx0 + groupSizeX, maxX);
		int gy1 = PF_MIN2(gy0 + groupSizeY, maxY);
		
		/* generate subordinate child node */
		child = _MakeTile(tile, gx0, gy0, gx1, gy1, depth+1, stats);
		
		/* add new child to current group */
		pfAddChild(parent, child);
	    }
	
	/* return rows*cols group as result */
	result = (pfNode *)parent;
    }
    
    /* normal exit */
    return result;
}

pfGroup*
MakeTile (Tile *tile, int minX, int minY, int maxX, int maxY)
{
    pfGroup *group;
    StatData stats;
    
    /* reset statistics */
    stats.numGroups	= 0;
    stats.numGeodes	= 0;
    stats.numGeosets	= 0;
    stats.numTriangles	= 0;
    stats.numVertices	= 0;
    
    group = (pfGroup *)_MakeTile(tile, minX, minY, maxX, maxY, 0, &stats);
    
    /* print statistics */
    fprintf(stderr, "tile statistics:\n");
    fprintf(stderr, "%8lu groups\n",	stats.numGroups);
    fprintf(stderr, "%8lu geodes\n",	stats.numGeodes);
    fprintf(stderr, "%8lu geosets\n",	stats.numGeosets);
    fprintf(stderr, "%8lu triangles\n",	stats.numTriangles);
    fprintf(stderr, "%8lu vertices\n",	stats.numVertices);
    fprintf(stderr, "%6.2f tris/geoset", (stats.numTriangles/
					  (float)stats.numGeosets));
    
    return group;
}
