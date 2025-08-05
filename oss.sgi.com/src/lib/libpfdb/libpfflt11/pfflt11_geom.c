/*
 * Copyright 1992, 1995, Silicon Graphics, Inc.
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
 * pfflt_geom.c $Revision: 1.1 $ 
 * 
 * Version 11 .flt file converter.
*/
#include <stdio.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h> 
#include <varargs.h>
#include <math.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfdb/pfflt11.h>

#ifdef	_POSIX_SOURCE
extern int strncasecmp (const char *s1, const char *s2, size_t n);
#endif

#include <string.h>

#define VLISTSIZE	2048
#define MAXTERMS	16		

static char 		terms[MAXTERMS][80];  /* for comments */

static pfVec3	*LPointVertList;
static int	 LPVertSize, LPVertCount;

typedef struct _MtlLink
{
    struct _MtlLink	*next;
    pfMaterial		*pfmtl;

} MtlLink;

static LightPoint   *LightPointList;
static MtlLink      **SharedMtlList = NULL;
static int	    SharedMtlCount = 0;
static pfMaterial   *DefaultMtl = NULL;
static pfTexture    **TexList = NULL;
static pfTexEnv	    *ModTEnv = NULL;

/*
 * It is crucial to share textures between .flt files so these lists
 * store textures for sharing.
*/
static pfTexture    *SharedTexList[MG_MAX_TEXTURES];   /* XXX - should grow */
static pfTexEnv	    *SharedTevList[MG_MAX_TEXTURES];    /* XXX - should grow */
static int	    SharedTexCount = 0;

/* Database counters */
static int	    PolyCount[257], ConcaveCount;
static int	    OmniCount, DirCount;
static int	    GSetCount, GStateCount;
static int	    StripCount, StripTriCount, TriCount, QuadCount;
static int	    TexCount = 0;
static int	    TotalTris = 0;

static int	    GfxType;
static void	    *Arena;

static int	    getTexture(int);
static pfMaterial*  getMaterial(GeoState *gs);
static int	    addLPVert(pfVec3 v);
static char*	    addLPoint(mgPolygon *);
static pfNode*	    makeLPoint(void);
static pfNode*	    makeGeode(void);
static pfGeoState*  makeGState(GeoState *gstate);
static int	    parseComment(mgComment *);

#define	GFX_REALITY	0x1
#define	GFX_MULTISAMPLE	0x10

    /*------------------------------------------------------------*/


/*
 * Initialize lists and counters.
*/    
void
GeomInit11(void)
{
    int    	i;
    char	gv[32];

    /* Get Performer shared memory arena */
    Arena = pfGetSharedArena();

    /* Determine graphics configuration */
    GfxType = 0;
    {
	GfxType |= GFX_REALITY;
	GfxType |= GFX_MULTISAMPLE;
    }

    LPointVertList = (pfVec3 *) pfMalloc(sizeof(pfVec3) * VLISTSIZE, NULL);
    LPVertSize = VLISTSIZE;

    if(DefaultMtl == NULL)
    {
	DefaultMtl = pfNewMtl(Arena);
	pfMtlColorMode(DefaultMtl, PFMTL_FRONT, PFMTL_CMODE_AD);
	pfRef(DefaultMtl);
    }

    if(SharedMtlList == NULL)
	SharedMtlList = (MtlLink**)pfCalloc(64, sizeof(MtlLink*), NULL);

    /* 
     * Initialize counters 
    */

    StripCount = StripTriCount = TriCount = QuadCount = ConcaveCount = 0;

    GStateCount = GSetCount = 0;
    OmniCount = DirCount = 0;

    for (i = 0; i < 257; i++)
   	PolyCount[i] = 0;
    LightPointList = NULL;

    /* Allocate MODULATE texture environment which is most common. */
    if(ModTEnv == NULL)
    {
	ModTEnv = pfNewTEnv(Arena);
	pfRef(ModTEnv);
    }
}


/*
 * Clean up and print out statistics
*/
void
GeomExit11(void)
{
    int    numTris, i;
    MgFile  *mgf;

    /* Create list of Performer textures used. */
    TexCount = 0;
    mgf = RootMgFile11;
    while(mgf)
    {
	TexCount += mgf->pfTexCount;	
	mgf = mgf->next;	
    }

    if(TexList)
	pfFree(TexList);
    TexList = (pfTexture**)
	pfMalloc((unsigned int)(sizeof(pfTexture*)*TexCount), NULL);
    
    mgf = RootMgFile11;
    TexCount = 0;
    while(mgf)
    {
	for(i=0; i<mgf->pfTexCount; i++)
	    TexList[TexCount++] = mgf->pfTexList[i];
	mgf = mgf->next;	
    }

    if(RootMgFile11->root)
    {
	/*
	 * Print out statistics.
	*/
	if(pfGetNotifyLevel() >= PFNFY_INFO)
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT,
		"FLIGHT Geometry statistics : \n");

	    numTris = 0;
	    for (i = 0; i < 257; i++)
		if (PolyCount[i] != 0)
		{
		    pfNotify(PFNFY_INFO, PFNFY_MORE, "\t%ld-sided: %ld\n",
			i, PolyCount[i]);
		    if(i > 2)
			numTris += PolyCount[i] * (i - 2);
		}

	    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Concave: %ld\n", ConcaveCount);
	    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Triangles: %ld\n", numTris);

	    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Performer Geometry:\n");
	    pfNotify(PFNFY_INFO, PFNFY_MORE, "    TexCount = %ld, "
		"SharedMtlCount = %ld\n", TexCount, SharedMtlCount);

	    numTris = TriCount + QuadCount*2 + StripTriCount;
	    TotalTris += numTris;

	    pfNotify(PFNFY_INFO, PFNFY_MORE, "    Light Points: Omni = %d, "
		"Dir = %d\n", OmniCount, DirCount);

	    pfNotify(PFNFY_INFO, PFNFY_MORE, "    GeoStates = %ld, "
		"GeoSets = %ld\n", GStateCount, GSetCount);

	    if(GSetCount > 0.)
		pfNotify(PFNFY_INFO, PFNFY_MORE, "    Triangles/GeoSet "
		    "= %.2f\n", (float)numTris / (float)GSetCount);

	    pfNotify(PFNFY_INFO, PFNFY_MORE, "    Tris = %ld, Quads = %ld,\n", 
		TriCount, QuadCount);

	    pfNotify(PFNFY_INFO, PFNFY_MORE, "    Strips = %ld, Stripped "
		"Tris = %ld\n", StripCount, StripTriCount);

	    if(StripCount > 0.)
		pfNotify(PFNFY_INFO, PFNFY_MORE, "    Tris/Strip = %.2f\n", 
		    (float)StripTriCount / (float)StripCount);

	    pfNotify(PFNFY_INFO, PFNFY_MORE, "    Total Triangles: %ld\n",
		TotalTris);

	}
    }

    /* Free vertex buffers. */
    pfFree(LPointVertList);
}


/*
 * Return Performer material corresponding to FLIGHT material and
 * create new pfMaterial if necessary. This routine uses lmcolor mode
 * PFMTL_CMODE_AD to reduce number of materials needed.
*/
static pfMaterial*
getMaterial(GeoState *gs)
{
    int		mid;

    if (CurMgFile11->mgMtlList == NULL || gs->material == -1)
    {
	gs->material = -1;	/* XXX - default to std material */
	return DefaultMtl;
    }

    mid = gs->material;

    if(CurMgFile11->pfMtlList[mid] == NULL)
    {
	MtlLink		*mlink;
	pfMaterial	*pfm;
	mgMaterial	*mgm = &CurMgFile11->mgMtlList[mid];
	int		emissive;

	emissive = (mgm->emission[0] + mgm->emission[1] + mgm->emission[2]) > 
		   (mgm->diffuse[0] + mgm->diffuse[1] + mgm->diffuse[2]);

	/* Search for material on global list */
	mlink = SharedMtlList[mid];
	while(mlink)
	{
	    pfVec3	clr;

	    pfm = mlink->pfmtl;
	
	    /* Compare specular, emissive but ignore diffuse and ambient */
	
	    if (emissive)
	    {
		pfGetMtlColor(pfm, PFMTL_EMISSION, clr, clr+1, clr+2);
		if(!PFALMOST_EQUAL_VEC3(clr, mgm->emission, .01f))
		{
		    mlink = mlink->next;
		    continue;
		}
	    }
	    pfGetMtlColor(pfm, PFMTL_SPECULAR,  clr, clr+1, clr+2);
	    if(!PFALMOST_EQUAL_VEC3(clr, mgm->specular, .01f))
	    {
		mlink = mlink->next;
		continue;
	    }
	    if(pfGetMtlAlpha(pfm) != mgm->alpha)
	    {
		mlink = mlink->next;
		continue;
	    }
	    if(pfGetMtlShininess(pfm) != mgm->shininess)
	    {
		mlink = mlink->next;
		continue;
	    }
	    break;
	}

	if(mlink == NULL)
	{
	    mlink = (MtlLink*) pfMalloc(sizeof(MtlLink), NULL);
    	    CurMgFile11->pfMtlList[mid] = pfm = mlink->pfmtl = pfNewMtl(Arena);
	    pfRef(pfm);
	    SharedMtlCount++;

	    mlink->next = SharedMtlList[mid];
	    SharedMtlList[mid] = mlink;


    	    pfMtlColor(pfm, PFMTL_AMBIENT,  mgm->ambient[0],
				       	mgm->ambient[1], 
					mgm->ambient[2]);

    	    pfMtlColor(pfm, PFMTL_SPECULAR, mgm->specular[0],
				        mgm->specular[1],
				        mgm->specular[2]);

    	    pfMtlColor(pfm, PFMTL_EMISSION, mgm->emission[0],
				        mgm->emission[1],
				        mgm->emission[2]);

    	    pfMtlColor(pfm, PFMTL_DIFFUSE, mgm->diffuse[0],
				        mgm->diffuse[1],
				        mgm->diffuse[2]);

    	    pfMtlShininess(pfm, mgm->shininess);
    	    pfMtlAlpha(pfm, mgm->alpha);

	    /* 
	     * If material is not emissive we use lmcolor() mode to 
	     * change diffuse and ambient colors. Note that this will
	     * make ambient == diffuse color so the material may
	     * not look the same as it would in MultiGen.
	    */
	    if (!emissive)
		pfMtlColorMode(pfm, PFMTL_FRONT, PFMTL_CMODE_AD);
	    else
	    	pfMtlColorMode(pfm, PFMTL_FRONT, PFMTL_CMODE_COLOR);

        }
	else 
	    CurMgFile11->pfMtlList[mid] = mlink->pfmtl;
    }

    return CurMgFile11->pfMtlList[mid];
}


/* 
 * Return Performer texture corresponding to FLIGHT texture and 
 * create new pfTexture if necessary. Textures are shared between .flt
 * files by texture file name.
*/
static int
getTexture(int id)
{
    int             i;
    mgTextureRef   *tex;
    pfTexture      *pfTex;
    pfTexEnv       *pfTev;
    char	    attrFile[PF_MAXSTRING], path[PF_MAXSTRING];
    int             ifd;
    struct stat     stbuf;
    MgFile	    *cmf = CurMgFile11;

    if (id < 0)
	return id;

    if(cmf->pfTexIndex[id] >= 0)
	return cmf->pfTexIndex[id];
    else if(cmf->pfTexIndex[id] == -3)	/* -3 means texture not found */
	return -1;	  
    
    /* Find texture with index == id */
    for (i = 0; i < cmf->mgTexCount; i++)
    {
	if (id == cmf->mgTexList[i]->index)
	    break;
    }

    if (i == cmf->mgTexCount)
	return -1;

    tex = cmf->mgTexList[i];

    /* See if we can find texture file */
    if (!pfFindFile(tex->file, path, R_OK))
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
			"Could not open texture file %s.", tex->file );
	cmf->pfTexIndex[id] = -3;  /* -3 means texture not found */
	return -1;
    }

    /* See if texture was already loaded by another file */
    for (i = 0; i < SharedTexCount; i++)
    {
	const char *str =  pfGetTexName(SharedTexList[i]);
	if (!str)
	    continue;

	if(!strcmp(path, str))
	{
	    cmf->pfTexList[cmf->pfTexCount] = SharedTexList[i];
	    cmf->pfTEnvList[cmf->pfTexCount] = SharedTevList[i];
	    cmf->pfTexIndex[id] = cmf->pfTexCount;
	    cmf->pfTexCount++;
	    return cmf->pfTexIndex[id];
	}	    	    	
    }
    
    /* Could not share texture */
    pfTex = cmf->pfTexList[cmf->pfTexCount] = pfNewTex(Arena);
    pfRef(pfTex);
    cmf->pfTexIndex[id] = cmf->pfTexCount;
    pfLoadTexFile(pfTex, path);
    if(pfGetNotifyLevel() >= PFNFY_INFO)
    {
	int	comp, x, y, z;
   	unsigned int	*image;

	pfGetTexImage(pfTex, &image, &comp, &x, &y, &z);

	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Loaded texture %s: %d %d %d %d\n",
			tex->file, comp, x, y, z);
    }
    
    /* Read texture attribute file */
    sprintf(attrFile, "%s.attr", tex->file);
    if (pfFindFile(attrFile, path, R_OK) &&
       (ifd = open(path, O_RDONLY)) >= 0 &&
       fstat(ifd, &stbuf) == 0)
    {
	static long minf[] = { PFTEX_POINT, PFTEX_BILINEAR, PFTEX_MIPMAP, 
                               PFTEX_MIPMAP_POINT, PFTEX_MIPMAP_LINEAR, 
                               PFTEX_MIPMAP_BILINEAR, PFTEX_MIPMAP_TRILINEAR};
	static long magf[] = { PFTEX_POINT, PFTEX_BILINEAR }; 
	static long w[] =   {PFTEX_REPEAT, PFTEX_CLAMP, PFTEX_SELECT};
    	static long env[] = {PFTE_MODULATE, PFTE_BLEND, PFTE_DECAL};
	int comp;
	unsigned int *foo;

	mgTextureAttr	    texAttr;

	/* read the texture attribute file */
	read(ifd, (char*)&texAttr, sizeof(texAttr));
	close(ifd);

	pfTexFilter(pfTex, PFTEX_MAGFILTER, magf[texAttr.magfilter]);
	if(texAttr.wrap >= 0 && texAttr.wrap < 3)
	    pfTexRepeat(pfTex, PFTEX_WRAP, w[texAttr.wrap]);
	if(texAttr.wrapu >= 0 && texAttr.wrapu < 3)
	    pfTexRepeat(pfTex, PFTEX_WRAP_S, w[texAttr.wrapu]);
	if(texAttr.wrapv >= 0 && texAttr.wrapv < 3)
	    pfTexRepeat(pfTex, PFTEX_WRAP_T, w[texAttr.wrapv]);

	if(env[texAttr.envtype] == PFTE_MODULATE || texAttr.envtype < 0 ||
	   texAttr.envtype > 2)
	    pfTev = ModTEnv;
	else
	{
	    pfTev = pfNewTEnv(Arena);
    	    pfRef(pfTev);
	    pfTEnvMode(pfTev, env[texAttr.envtype]);
	}

	/* XXX texture environment color not supported */ 
    }    
    else
    {
    	pfNotify(PFNFY_WARN,PFNFY_RESOURCE, "Could not load "
	    "texure attribute file \"%s\"", attrFile);
	pfTev = ModTEnv;
    }

    cmf->pfTEnvList[cmf->pfTexCount] = pfTev;
    SharedTexList[SharedTexCount] = pfTex;
    SharedTevList[SharedTexCount] = pfTev;
    SharedTexCount++;

    cmf->pfTexCount++;
    return cmf->pfTexIndex[id];
}

/*
 * If a polygon is flagged as a light string, its vertices are light 
 * points so add them to a special list.
*/
static int
addLPVert(pfVec3 v)
{
    if (LPVertCount >= LPVertSize)
    {
	LPVertSize <<= 1;
	LPointVertList = (pfVec3 *) pfRealloc(LPointVertList,
	    (unsigned int)(LPVertSize * sizeof(pfVec3)));
    }
    LPVertCount++;
    PFCOPY_VEC3(LPointVertList[LPVertCount-1], v);
    return(LPVertCount - 1);
}


/*
 * The FLIGHT polygon is flagged as a light string so add it to a 
 * special list.
*/
static char*
addLPoint(mgPolygon * mgPoly)
{
    int           coms;
    int           numPoints; 
    LightPoint     *newLPoint, *lp;
    mgVertAbs      *aVert;
    register char  *ptr;
    int	   depth=0, done=0;
    pfVec3	   vert, vert2;
    mgComment	   *comment = NULL;
    mgMatrix	   *matrix = NULL;
    mgReplicate	   *replicate = NULL;
    mgVect	   *vector = NULL;
    mgBead	   *bead;

    /*
     * count points 
     */
    numPoints = 0;
    ptr = (char*)mgPoly + mgPoly->length;
    while(!done)
    {
	bead = (mgBead*)ptr;
	switch (bead->opcode)
	{
	case MG_VERT_ABS:
	case MG_VERT_SHAD:
	case MG_VERT_NORM:
	    if(matrix && replicate)
		numPoints += replicate->count + 1;
	    else
		numPoints++;
	    break;
	case MG_PUSH:
	    depth++;
	    break;
	case MG_POP:
	    depth--;
	    if(depth <= 0)
		done = 1;
	    break;
	case MG_VECTOR:
	    vector = (mgVect*)bead;
	    break;
	case MG_MATRIX:
	    matrix = (mgMatrix*)bead;
	    break;
	case MG_REPLICATE:
	    replicate = (mgReplicate*)bead;
	    break;
	case MG_COMMENT:
	    comment = (mgComment*)bead;
	    break;
	default:
	    if (bead->opcode < MG_NUM_OPCODES)
		pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		    "LoadFlt() Unsupported bead %ld \"%s\" found in polygon.",
		    bead->opcode,
		    OpcodeNames11[bead->opcode]);
	    else
		pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		    "LoadFlt() Unsupported bead %ld found in polygon.",
		    bead->opcode);
	    break;
	}
        ptr += bead->length;
    }

    /*
     * create the the lightpoint structure 
     */
    newLPoint = (LightPoint *) pfMalloc(sizeof(LightPoint), NULL);
    newLPoint->indexList = (int *) pfMalloc((unsigned int)(sizeof(int) * numPoints), NULL);
    newLPoint->numPoints = numPoints;
    newLPoint->next = NULL;
    newLPoint->set = -1;	   /* for sorting in makeLPoint() */
    newLPoint->name[0] = 0;
    numPoints = 0;

    /* Convert color */
    MG2PF_COLOR3(newLPoint->color, mgPoly->color1);

    newLPoint->color[3] = 1.0f;	   /* Set alpha */

    /*
     * Count points 
    */
    ptr = (char*)mgPoly + mgPoly->length;
    done = 0;
    while(!done)
    {
	bead = (mgBead*)ptr;
	switch (bead->opcode)
	{
	case MG_VERT_ABS:
	case MG_VERT_SHAD:
	case MG_VERT_NORM:
	    aVert = (mgVertAbs *) bead;
	    vert[0] = aVert->v[0];
	    vert[1] = aVert->v[1];
	    vert[2] = aVert->v[2];
	    if(matrix && replicate)
	    {
		int i;
 		for(i=0; i<replicate->count+1; i++)
		{
		    MG2PF_COORD3(vert2, vert);
		    newLPoint->indexList[numPoints] = addLPVert(vert2);
		    pfXformPt3(vert, vert, matrix->matrix);
		    /* matrix translate values are also scaled. #?$-@! */
		    numPoints++;
		}
	    }
	    else
	    {
	        if(matrix)
		    pfXformPt3(vert, vert, matrix->matrix);
		MG2PF_COORD3(vert2, vert);
		newLPoint->indexList[numPoints] = addLPVert(vert2);
		numPoints++;
	    }
	    break;
	case MG_PUSH:
	    depth++;
	    break;
	case MG_POP:
	    depth--;
	    if(depth <= 0)
		done = 1;
	    break;
	case MG_VECTOR:
	case MG_MATRIX:
	case MG_REPLICATE:
	case MG_COMMENT:
	    coms = parseComment(comment);
	    break;
	}
        ptr += bead->length;
    }

    /* get the name if any */
    if(coms)
	strcpy(newLPoint->name, terms[0]);

    /* 
     * set the direction vector and light type. some files 
     * have a bug here, so check vector for each type.
     */
    switch(mgPoly->drawType)
    {
	case 8:		/* omni-direction light */
	    newLPoint->lightType = PFLP_OMNIDIRECTIONAL;
	    break;
	case 9:		/* uni-directional light */
	    if(vector == NULL)
		newLPoint->lightType = PFLP_OMNIDIRECTIONAL;
	    else
	    {
		newLPoint->lightType = PFLP_UNIDIRECTIONAL;
		MG2PF_NORM3(newLPoint->vector, vector->vec);
	    }
	    break;
	case 10:	/* bi-directional light */
	    if(vector == NULL)
		newLPoint->lightType = PFLP_OMNIDIRECTIONAL;
	    else
	    {
		newLPoint->lightType = PFLP_BIDIRECTIONAL;
		MG2PF_NORM3(newLPoint->vector, vector->vec);
	    }
	    break;
    }
    if(newLPoint->lightType == PFLP_OMNIDIRECTIONAL)
  	OmniCount += numPoints;
    else
  	DirCount += numPoints;

    /* Prepend new LightPoint */
    lp = LightPointList;
    LightPointList = newLPoint;
    LightPointList->next = lp;

    return ptr;
}


/*
 * Add FLIGHT polygon to pfdGeoBuilder. Polygons are sorted by 
 * texture/material/backface into GeoStates. pfGeoSets are generated by 
 * the pfdGeoBuilder utility.
*/
char*
AddPoly11(mgPolygon * mgPoly)
{
    GeoState        *gstate;
    int            vertType;
    int             i, j;
    register char   *ptr;
    int	    depth=0, done=0, mixed;
    mgComment	    *comment = NULL;
    mgMatrix	    *matrix = NULL;
    mgReplicate	    *replicate = NULL;
    mgBead	    *bead;
    pfdGeom	    polygon, *pb;
    int		    t;

    pb = pfdNewGeom(4096);
    polygon = *pb;
    /* if it's a light point, create points */
    if(mgPoly->drawType >= 8 && mgPoly->drawType <= 10)
	return(addLPoint(mgPoly));

    polygon.numVerts = 0;
    polygon.flags = 0;
    vertType = 0;
    mixed = 0;

    /* 
     * Check for mixed vertex types. If a polygon has mixed vertex types
     * we default all vertices to MG_VERT_ABS.
    */
    ptr = (char*)mgPoly + mgPoly->length;
    while(!done)
    {
	/* Determine if vertex is textured by length of record */
	bead = (mgBead*)ptr;
	switch (bead->opcode)
	{
	case MG_VERT_ABS:

	    MG2PF_COORD3(polygon.coords[polygon.numVerts], ((mgVertAbs*)bead)->v);
	    if (bead->length > 16)
	    {
		bead->opcode |= MG_VERT_TEX;
		pfCopyVec2(polygon.texCoords[0][polygon.numVerts], ((mgVertAbs*)bead)->t);
	    }
	    polygon.numVerts++;

	    if(vertType == 0)
	    	vertType = bead->opcode;
	    else if(vertType != bead->opcode)
		mixed = 1;

	    break;
	case MG_VERT_SHAD:

	    MG2PF_COORD3(polygon.coords[polygon.numVerts], ((mgVertShad*)bead)->v);
	    MG2PF_COLOR3(polygon.colors[polygon.numVerts], ((mgVertShad*)bead)->color);
	    if (bead->length > 20)
	    {
		bead->opcode |= MG_VERT_TEX;
		pfCopyVec2(polygon.texCoords[0][polygon.numVerts], ((mgVertShad*)bead)->t);
	    }
	    polygon.numVerts++;

	    if(vertType == 0)
	    	vertType = bead->opcode;
	    else if(vertType != bead->opcode)
		mixed = 1;

	    break;
	case MG_VERT_NORM:

	    MG2PF_COORD3(polygon.coords[polygon.numVerts], ((mgVertNorm*)bead)->v);
	    MG2PF_NORM3(polygon.norms[polygon.numVerts], ((mgVertNorm*)bead)->n);
	    if (bead->length > 32)
	    {
		bead->opcode |= MG_VERT_TEX;
		pfCopyVec2(polygon.texCoords[0][polygon.numVerts], ((mgVertNorm*)bead)->t);
	    }
	    polygon.numVerts++;

	    if(vertType == 0)
	    	vertType = bead->opcode;
	    else if(vertType != bead->opcode)
		mixed = 1;

	    break;
	case MG_PUSH:
	    depth++;
	    break;
	case MG_POP:
	    depth--;
	    if(depth <= 0)
		done = 1;
	    break;
	case MG_MATRIX:
	    matrix = (mgMatrix*)bead;
	    break;
	case MG_REPLICATE:
	    replicate = (mgReplicate*)bead;
	    break;
	case MG_COMMENT:
	    comment = (mgComment*)bead;
	    break;
	case MG_POLYGON:
	    pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		    "LoadFlt() Unsupported bead %ld \"%s\" found in polygon.",
		    bead->opcode,
		    OpcodeNames11[bead->opcode]);
	    return ptr;
	default:
	    if (bead->opcode < MG_NUM_OPCODES)
		pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		    "LoadFlt() Unsupported bead %ld \"%s\" found in polygon.",
		    bead->opcode,
		    OpcodeNames11[bead->opcode]);
	    else
		pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		    "LoadFlt() Unsupported bead %ld found in polygon.",
		    bead->opcode);
	    break;
	}

        ptr += bead->length;
    }

    if (polygon.numVerts < 3)	/* Discard bad polygon */
	return ptr;

    /* Set mixed flag which will cause polygon to be flat-shaded */
    if (FlatFlag11 && (vertType & ~MG_VERT_TEX) == MG_VERT_NORM)
	mixed = 1;

    /* 
     * Default to ABS vert if vertices are mixed type. These polys will be
     * flat-shaded. 
    */
    if (mixed)
    {
	if (vertType & MG_VERT_TEX)
	    vertType = MG_VERT_ABS | MG_VERT_TEX;
	else
	    vertType = MG_VERT_ABS;
    }

    switch (vertType & ~MG_VERT_TEX) {
    case MG_VERT_ABS:
	polygon.nbind = PFGS_OFF;
	polygon.cbind = PFGS_PER_PRIM;
	MG2PF_COLOR3(polygon.colors[0], mgPoly->color1);
	break;
    case MG_VERT_SHAD:
	polygon.nbind = PFGS_OFF;
	polygon.cbind = PFGS_PER_VERTEX;
	break;
    case MG_VERT_NORM:
	polygon.nbind = PFGS_PER_VERTEX;
	polygon.cbind = PFGS_PER_PRIM;
	MG2PF_COLOR3(polygon.colors[0], mgPoly->color1);
	break;
    }
    if (vertType & MG_VERT_TEX)
    {
	polygon.tbind[0] = PFGS_PER_VERTEX;
	for (t = 1 ; t < PF_MAX_TEXTURES ; t ++)
	    polygon.tbind[t] = PFGS_OFF;
	mgPoly->texture = (short)getTexture(mgPoly->texture);
    }
    else
    {
	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    polygon.tbind[t] = PFGS_OFF;
	mgPoly->texture = -1;
    }

    /* FLIGHT requires multiplying polygon color by material diffuse */
    if (CurMgFile11->mgMtlList != NULL && mgPoly->material >= 0)
    {
	mgMaterial	*mgm = &CurMgFile11->mgMtlList[mgPoly->material];

    	polygon.colors[0][0] *= mgm->diffuse[0];
    	polygon.colors[0][1] *= mgm->diffuse[1];
    	polygon.colors[0][2] *= mgm->diffuse[2];
	polygon.colors[0][3] = mgm->alpha;
    }

    /* Find the geostate list to add the polygon to */
    for (gstate = CurMgFile11->gsList; gstate != NULL; gstate = gstate->next)
    {
	if (mgPoly->material == gstate->material && 
	    mgPoly->texture == gstate->texture &&
	    (mgPoly->drawType == 0) == gstate->backFace)
	{
	    break;
	}
    }

    /* Prepend new GeoState */
    if (gstate == NULL)
    {
	gstate = CurMgFile11->gsList;
	CurMgFile11->gsList = (GeoState *) pfMalloc(sizeof(GeoState), NULL);
	CurMgFile11->gsList->next = gstate;
	CurMgFile11->gsList->pfgs = NULL;
	gstate = CurMgFile11->gsList;
	gstate->material = mgPoly->material;
	gstate->texture = mgPoly->texture;
	gstate->backFace = (mgPoly->drawType == 0);
	gstate->bbuilder = NULL;
	gstate->builder = pfdNewGeoBldr();
	pfdGeoBldrMode(gstate->builder, PFDBLDR_AUTO_NORMALS, ComputeNormals11);
    }

    /* Add mgPoly to billboard builder if it is a billboard */
    if (mgPoly->textureTemplate == 2) 
    {
	if (gstate->bbuilder == NULL)
	{
	    gstate->bbuilder = pfdNewGeoBldr();
	    pfdGeoBldrMode(gstate->bbuilder, PFDBLDR_AUTO_NORMALS, ComputeNormals11);
	}
	
	pfdAddPoly(gstate->bbuilder, &polygon);
    }
    else
	pfdAddPoly(gstate->builder, &polygon);

    if (polygon.flags & PFD_CONCAVE)
	ConcaveCount++;
    PolyCount[polygon.numVerts]++;

    /*
     * Create replicated polygons 
    */
    if (replicate)
    {
	for (i = 0; i < replicate->count; i++)
	{
	    for (j = 0; j < polygon.numVerts; j++)
		pfXformPt3(polygon.coords[j], polygon.coords[j], matrix->matrix);

	    if (mgPoly->textureTemplate == 2) 
		pfdAddPoly(gstate->bbuilder, &polygon);
	    else
		pfdAddPoly(gstate->builder, &polygon);
	}
	PolyCount[polygon.numVerts] += replicate->count;
    }

    return ptr;
}


/*
 * This converts a FLIGHT object into a Performer sub-tree. The object
 * usually results in a single pfGeode but may generate pfLightPoints and/or
 * pfBillboards.
*/
pfNode*
MakeGeometry11(void)
{
    pfNode	*lp, *geode;

    lp = makeLPoint();
    geode = makeGeode();

    if(lp != NULL && geode != NULL)
    {
    	pfGroup	*group;

     	group = pfNewGroup();
	pfAddChild(group, lp);
	pfAddChild(group, geode);
    	return (pfNode *)group;
    }
    else if(lp != NULL)
	return lp;
    else if(geode != NULL)	
	return geode;
    else
	return NULL;
}


/*
 * Generate pfLightPoint nodes from LightPointList
*/
static pfNode * 
makeLPoint(void)
{
    pfLightPoint *lp;
    pfGroup  	*lpHead;
    LightPoint 	*lp2, *lp3;
    int 	i,k,j,q;
    int	dir, colorDiff;
    pfVec3	lc;
    pfVec3	vector;

    /* linked list of light points */
    if(LightPointList == NULL)
	return(NULL);

    lpHead = pfNewGroup();

    /* sort all light points by directionality */
    k = 0;
    while(1)
    {
	i = 0;
	lp2 = LightPointList;

        /* 
	 * Find first ungrouped light point in LightPointList.
	 * lp2->set = the set of points to which a point belongs, -1
	 * indicates light point is as yet ungrouped.
	*/
    	while(lp2 != NULL && lp2->set >= 0)
	    lp2 = lp2->next;

        if(lp2 == NULL)		/* All done */
	    break;

	dir = lp2->lightType;
	PFCOPY_VEC3(lc, lp2->color);
	PFCOPY_VEC3(vector, lp2->vector);
	
	colorDiff = 0;		/* Check if colors of lightpoints are same */
	while(lp2 != NULL)
	{
	    if(lp2->lightType == dir)
	    {
		if(!colorDiff && !PFALMOST_EQUAL_VEC3(lc, lp2->color, .01f))
		    colorDiff = 1;
		i += lp2->numPoints;
		lp2->set = k;
	    }
	    lp2 = lp2->next;
	}

	lp = pfNewLPoint(i);
	pfLPointSize(lp, 2.0);

	/* Convert direction to azim, elev, roll */
	if(dir != PFLP_OMNIDIRECTIONAL)
	{
	    float a, e;

	    if(PF_ABS(vector[PF_Y]) > PF_ABS(vector[PF_Z]))
		a = -PF_RAD2DEG(atan2f(vector[PF_X], vector[PF_Y]));
	    else
		a = -PF_RAD2DEG(atan2f(vector[PF_X], vector[PF_Z]));

	    if(PF_ABS(vector[PF_Y]) > PF_ABS(vector[PF_X]))
		e = PF_RAD2DEG(atan2f(vector[PF_Z], vector[PF_Y]));
	    else
		e = PF_RAD2DEG(atan2f(vector[PF_Z], vector[PF_X]));

	    pfLPointRot(lp, a, e, 0.);
	}

	/* Choose default of 90 x 90 degrees horizontal/vertical envelope. */
	pfLPointShape(lp, dir, 90.0, 90.0, 4.0);

        q = 0;
	lp2 = LightPointList;

	/* Set overall color if possible */
	if(!colorDiff)
	    pfLPointColor(lp, PFLP_OVERALL, lp2->color);

	while(lp2 != NULL)
	{
	    if(lp2->set == k)
	    {
		if(lp2->name[0] != 0)
		    pfNodeName(lp, lp2->name);

		if(colorDiff)
		{
		    for(j=0; j<lp2->numPoints; j++)
		    {
		    	pfLPointPos(lp, q+j, LPointVertList[lp2->indexList[j]]);
		    	pfLPointColor(lp, q+j, lp2->color);
		    }
		    q += j;
		}
		else
		{
		    for(j=0; j<lp2->numPoints; j++)
		    	pfLPointPos(lp, q+j, LPointVertList[lp2->indexList[j]]);
		    q += j;
		}
	    }
	    lp2 = lp2->next;
	}
	k++;
	pfAddChild(lpHead, lp);
    }
	

    /* free up the light list */
    lp2 = LightPointList;
    while(lp2 != NULL)
    {
	lp3 = lp2;
	lp2 = lp2->next;
	pfFree((void *)lp3->indexList);
	pfFree((void *)lp3);
    }
    LightPointList = NULL;
    LPVertCount = 0;
    return((pfNode*)lpHead);
}


/*
 * Convert FLIGHT polygons into pfGeoSets/pfGeoStates/pfGeodes
*/
static pfNode *
makeGeode(void)
{
    GeoState       *gstate;
    pfGeoState	   *pfGState;
    pfGeode	   *geode = NULL;
    pfBillboard	   *bboard = NULL;
    int	    i;
    const pfList    *gsets;

    if(CurMgFile11->gsList == NULL)
	return NULL;

    /* For all GeoStates */
    for (gstate = CurMgFile11->gsList; gstate != NULL; gstate = gstate->next)
    {
	pfGState = gstate->pfgs;
	if(pfGState == NULL)
	    gstate->pfgs = pfGState = makeGState(gstate);

	/* Add pfGeoSets to pfGeode */
	if (pfdGetNumTris(gstate->builder) > 0)
	{
	    gsets = pfdBuildGSets(gstate->builder);
	    
	    if (geode == NULL)
		geode = pfNewGeode();

	    for (i=0; i<pfGetNum(gsets); i++)
	    {
		pfGSetGState((pfGeoSet*)pfGet(gsets, i), pfGState);
		pfAddGSet(geode, (pfGeoSet*)pfGet(gsets, i));
	    }
	}
	/* Add pfGeoSets to pfBillboard */
	if (gstate->bbuilder && pfdGetNumTris(gstate->bbuilder) > 0)
	{
	    gsets = pfdBuildGSets(gstate->bbuilder);
	    
	    if (bboard == NULL)
		bboard = pfNewBboard();

	    for (i=0; i<pfGetNum(gsets); i++)
	    {
		pfGSetGState((pfGeoSet*)pfGet(gsets, i), pfGState);
		pfAddGSet(bboard, (pfGeoSet*)pfGet(gsets, i));
	    }
	}
    }	/* End while gstate */

    if(bboard != NULL && geode != NULL)
    {
	pfGroup		*grp;

	grp = pfNewGroup();
    	pfAddChild(grp, geode);
    	pfAddChild(grp, bboard);

	return (pfNode*)grp;
    }
    else if(geode != NULL)
	return ((pfNode*)geode);
    else if(bboard != NULL)
	return ((pfNode*)bboard);
    else
	return NULL;
}


/*
 * Create and initialize a new pfGeoState
*/
static pfGeoState*
makeGState(GeoState *gstate)
{
    pfMaterial	   *mtl;
    pfGeoState	   *pfGState;
    float	   alpha;

    pfGState = pfNewGState(Arena);    
    GStateCount++;

    if (gstate->texture >= 0)
    {
        unsigned int          *image;
        int             comp, sx, sy, sz;
    
        /* Assume alpha texture requires afunction */
        pfGetTexImage(CurMgFile11->pfTexList[gstate->texture], 
              &image, &comp, &sx, &sy, &sz);

        if (comp == 2 || comp == 4)
        {
	    /* Use multisample transparency if possible */
            if (GfxType & GFX_MULTISAMPLE)
	    {
            	pfGStateMode(pfGState, PFSTATE_ALPHAFUNC, PFAF_GEQUAL);
            	pfGStateVal(pfGState, PFSTATE_ALPHAREF, (3.0f/255.0f));    
                pfGStateMode(pfGState, PFSTATE_TRANSPARENCY, PFTR_ON);
	    }
	    else
	    {
            	pfGStateMode(pfGState, PFSTATE_ALPHAFUNC, PFAF_GREATER);
            	pfGStateVal(pfGState, PFSTATE_ALPHAREF, (70.0f/255.0f));    
	    }
        }

        pfGStateAttr(pfGState, PFSTATE_TEXTURE,
                CurMgFile11->pfTexList[gstate->texture]);

	/* 
	 * XXX - Inherit TV_MODULATE texture environment for better
	 * performance.
	*/
	if(CurMgFile11->pfTEnvList[gstate->texture] != ModTEnv)
            pfGStateAttr(pfGState, PFSTATE_TEXENV, 
			CurMgFile11->pfTEnvList[gstate->texture]);
                

        pfGStateMode(pfGState, PFSTATE_ENTEXTURE, 1);
    }
    else 
        pfGStateMode(pfGState, PFSTATE_ENTEXTURE, 0);

    mtl = getMaterial(gstate);
	
    if(mtl != NULL)
    {
        pfGStateAttr(pfGState, PFSTATE_FRONTMTL, mtl);

        alpha = pfGetMtlAlpha(mtl);
        if (alpha < 1. - 1./256.)
            pfGStateMode(pfGState, PFSTATE_TRANSPARENCY, PFTR_ON);

        pfGStateMode(pfGState, PFSTATE_ENLIGHTING, PF_ON);
    }
    else
        pfGStateMode(pfGState, PFSTATE_ENLIGHTING, PF_OFF);

    /* Backface culling should be on by default */
    if(!gstate->backFace)
        pfGStateMode(pfGState, PFSTATE_CULLFACE, PFCF_OFF);

    return pfGState;
}

	/*-----------------------------------------------------*/
/*
 * Return list of textures used by previously loaded .flt file
*/
int
pfdGetTexList_flt11(pfTexture ***t)
{
    *t = TexList;
    if(TexList == NULL)
	return 0;
    else
	return TexCount;
}

int
pfdGetNumSharedTex_flt11(void)
{
    return SharedTexCount;
}

/*
 * Return list of unique textures shared by all .flt files
*/
int
pfdGetSharedTexList_flt11(pfTexture *tlist[])
{
    int	i;

    for(i=0; i<SharedTexCount; i++)
	tlist[i] = SharedTexList[i];

    return SharedTexCount;
}


/* the comment ptr in mg_node must point to the comment record */
static int
parseComment(mgComment *n)
{
    int i;
    char *c,*c2;

    if(n == NULL)
	return(0);

    c = n->text;

    if(c == NULL)
	return(0);


    while(*c != 0)
    {
	if(*c++ == '@')
	{
	    if(strncasecmp(c,"sgi",3) == 0)
	    {
		c += 3;
		i = 0;
		/* create a list of terms */
		while(*c != '\n' && *c != '\r' && *c != 0)
		{
		    c2 = terms[i];
		    /* get past leading white space */
		    while(*c == ' ' || *c == '\t' || *c == ',')
			c++;		
		    /* copy the string up to the next delimiter */
		    while(*c != ' ' && 
			    *c != '\t' &&
			    *c != ',' && 
			    *c != '\n' &&
			    *c != '\r' &&
			    *c != 0)
			*c2++ = *c++;

		    *c2 = 0;		/* end the string */
		    if(++i >= MAXTERMS)
			return(i);
		}
	    }
	}	
    }
    return(i);
}

/*
 * Count number of independent triangles, quads, and tristrips in geoset
*/
void
CountGSet11(pfGeoSet *gset)
{
    int	i, l, n;
 
    GSetCount++;

    n = pfGetGSetNumPrims(gset);
    switch(pfGetGSetPrimType(gset))
    {
	case PFGS_TRIS:
	    TriCount += n;
	    break;
	case PFGS_QUADS:
	    QuadCount += n;
	    break;
	case PFGS_TRISTRIPS:
	case PFGS_FLAT_TRISTRIPS:

	    for(i=0; i<n; i++)
 	    {
		l = pfGetGSetPrimLength(gset, i);
	        if(l == 3) 
		    TriCount++;
		else if(l == 4)
		    QuadCount++;
		else
		{
		    StripCount++;
		    StripTriCount += l - 2;
		}
	    }
	    break;
	default:
	    break;
    }		
}

