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
 * pfobj.c: $Revision: 1.103 $ $Date: 2002/11/17 01:40:32 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef sgi
#ifdef	_POSIX_SOURCE
extern int strcasecmp (const char *s1, const char *s2);
extern char *strdup (const char *s1);
#endif
#else
#ifdef WIN32
#define strcasecmp stricmp
#endif
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFOBJ_DLLEXPORT __declspec(dllexport)
#else
#define PFOBJ_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFOBJ_DLLEXPORT
#endif

/* length of longest input line in ".obj" file (including continuations) */
#define	BUFFER_SIZE	8192

/* maximum number of vertices in a single polygon */
#define	FACE_SIZE	4096

/* initial allocation size for growable arrays */
#define	CHUNK		4096

/* how many different material library files */
#define	MAX_MTL_FILES	512

/* case insensitive string equality test */
#define	SAME(_a, _b)	(strcasecmp(_a,_b) == 0)

/* list of textures defined by Wavefront material files */
typedef struct TEX
{
    char	*name;
    pfTexture	*texture;
    float	 su;		/* u-axis texture scale factor */
    float	 sv;		/* v-axis texture scale factor */
} TEX;

#define NUMTEX	 512
static TEX	 TexList[NUMTEX];
static int	 TexCount = 0;

/* list of materials defined by Wavefront material files */
typedef struct MTL
{
    char	    *name;
    pfGeoState	    *gstate;
    pfMaterial	    *material;
    pfTexture	    *texture;
    int 	    refl_mapped;

} MTL;

/* number of input lines skipped (recognized but not processed) or unknown */
static int 	 numSkip	= 0;
static int 	 numOther	= 0;

/* function type and argument declarations */
static void loadMtl (char *fileName);
static void useMtl (char *name);
static void forgetMaterials (void);
static void forgetMaterialFiles (void);

#define	GROW(_v, _t) \
    if (_v == NULL) \
    { \
	_v ## Available = CHUNK; \
	_v = (_t *)pfMalloc(sizeof(_t)*_v ## Available, NULL); \
    } \
    else \
    if (_v ## Count >= _v ## Available) \
    { \
	_v ## Available *= 2; \
	_v = (_t *)pfRealloc(_v, sizeof(_t)*_v ## Available); \
    }

/*
 * pfdLoadFile_obj -- Load Wavefront ".obj" files into IRIS Performer
 */

PFOBJ_DLLEXPORT
/*extern */pfNode*
pfdLoadFile_obj (char *fileName)
{
    FILE	*objFile;

    pfdGeom	*polygon = NULL;
    int		 polySize = -1;

    char	 buffer[BUFFER_SIZE];
    char	 token[BUFFER_SIZE];
    char	*next		= NULL;
    char	*backslash	= NULL;

    pfGeode	*geode		= NULL;
    pfGeode	*geodeRefl	= NULL;
    pfGroup	*group		= NULL;
    pfNode	*node		= NULL;

    pfNode	*hierarchy	= NULL;

    int		 width		= 0;

    int 	 numTris	= 0;
    int 	 numPolys	= 0;
    int 	 numGroups	= 0;

    /* growable vertex coordinate list (X, Y, Z) */
    pfVec3	*v		= NULL;
    unsigned int	 vCount		= 0;
    unsigned int	 vAvailable	= 0;

    /* growable vertex normal list (Nx, Ny, Nz) */
    pfVec3	*n		= NULL;
    unsigned int	 nCount		= 0;
    unsigned int	 nAvailable	= 0;

    /* growable texture coordinate list (S, T )*/
    pfVec2	*t		= NULL;
    unsigned int	 tCount		= 0;
    unsigned int	 tAvailable	= 0;

    /* tmp count vars */
    int		 i, j;

    double       startTime      = pfGetTime();
    double       elapsedTime    = 0.0;

    int		k;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open Wavefront ".obj" file file */
    if ((objFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* Obj loader uses strings to specify state names */
    pfdBldrAttr(PFDBLDR_STATE_NAME_COMPARE, (void *)strcmp);
    pfdBldrAttr(PFDBLDR_NODE_NAME_COMPARE, (void *)strcmp);
    pfdBldrAttr(PFDBLDR_EXTENSOR_NAME_COMPARE, (void *)strcmp);

    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_ON);
    pfdBldrStateMode(PFSTATE_ENTEXTURE, PF_OFF);
    pfdBldrStateMode(PFSTATE_CULLFACE,PFCF_BACK);
    pfdCaptureDefaultBldrState();

    /* read Wavefront ".obj" file */
    while (fgets(buffer, BUFFER_SIZE, objFile) != NULL)
    {
	/* concatenate continuation lines */
	while ((backslash = strchr(buffer, '\\')) != NULL)
	{
	    /* replace backslash with space (thanks to Ken Sakai) */
	    *backslash++ = ' ';
	    *backslash   = '\0';

	    /* keep reading */
	    if (fgets(backslash, (int)(BUFFER_SIZE - strlen(buffer)), objFile)
		== NULL)
		break;
	}

	/* find first non-"space" character in line */
	for (next = buffer; *next != '\0' && isspace(*next); next++)
	    /* EMPTY */ {};

	/* skip blank lines and comments ('$' is comment in "cow.obj") */
	if (*next == '\0' || *next == '#' || *next == '!' || *next == '$')
	    continue;

	/* extract token */
	sscanf(next, "%s%n", token, &width);
	next += width;

	/* identify token */
	if (SAME(token, "v"))
	{
	    /* enlarge vertex coordinate list */
	    GROW(v, pfVec3);

	    /* set default values for vertex coordinate */
	    v[vCount][0] = v[vCount][1] = v[vCount][2] = 0.0f;

	    /* read vertex coordinate into list */
	    sscanf(next, "%f %f %f",
		&v[vCount][PF_X], &v[vCount][PF_Y], &v[vCount][PF_Z]);

	    /* advance vertex count */
	    ++vCount;
	}
	else
	if (SAME(token, "vn"))
	{
	    /* enlarge vertex normal list */
	    GROW(n, pfVec3);

	    /* set default values for vertex normal */
	    n[nCount][0] = n[nCount][1] = n[nCount][2] = 0.0f;

	    /* read vertex normal into list */
	    sscanf(next, "%f %f %f",
		&n[nCount][PF_X], &n[nCount][PF_Y], &n[nCount][PF_Z]);

	    /* advance normal count */
	    ++nCount;
	}
	else
	if (SAME(token, "vt"))
	{
	    /* enlarge texture coordinate list */
	    GROW(t, pfVec2);

	    /* set default values for vertex normal */
	    t[tCount][0] = t[tCount][1] = 0.0f;

	    /* read texture coordinate into list */
	    sscanf(next, "%f %f", &t[tCount][0], &t[tCount][1]);

	    /* advance texture coordinate count */
	    ++tCount;
	}
	else
	if (SAME(token, "g"))
 	{
	    ++numGroups;
	}
	else
	if (SAME(token, "f") ||
	    SAME(token, "fo"))
	{
	    int 	 count;
	    int		 textureValid = 1;
	    int		 normalsValid = 1;
	    int		 vi[FACE_SIZE];
	    int		 ti[FACE_SIZE];
	    int		 ni[FACE_SIZE];

	    char	*slash;
	    char	 vertexData[256];

	    /* parse vertex data from input buffer */
	    for (count = 0; count < FACE_SIZE; count++)
	    {
		/* read the next vertices' data packet */
		if (sscanf(next, "%s%n", vertexData, &width) != 1)
		    break;

		/* advance next pointer past data packet ("n/n/n") */
		next += width;

		/* get vertex coordinate index */
		vi[count] = (int)strtol(vertexData, NULL, 10);

		/* get texture coordinate index */
		ti[count] = 0;
		if ((slash = strchr(vertexData, '/')) == NULL ||
		    (ti[count] = (int)strtol(slash+1, NULL, 10)) == 0)
		    textureValid = 0;

		/* get vertex normal index */
		ni[count] = 0;
		if (slash == NULL || (slash = strchr(slash+1, '/')) == NULL ||
		    (ni[count] = (int)strtol(slash+1, NULL, 10)) == 0)
		    normalsValid = 0;

		/*
		 * form cannonical indices:
		 *   convert ".obj" 1-based indices to 0-based (subtract 1)
		 *   convert negative indices to positive (count from 0)
		 */
		if (vi[count] >= 0)
		    vi[count] -= 1;
		else
		    vi[count]  = vCount - vi[count];

		if (ti[count] >= 0)
		    ti[count] -= 1;
		else
		    ti[count]  = tCount - ti[count];

		if (ni[count] >= 0)
		    ni[count] -= 1;
		else
		    ni[count]  = nCount - ni[count];
	    }

	    /* resize geom as needed */
	    if (polygon == NULL)
		polygon = pfdNewGeom(polySize = count);
	    else 
	    if (polySize < count)
		pfdResizeGeom(polygon, polySize = count);

	    polygon->numVerts = count;
	    polygon->primtype = PFGS_POLYS;

	    /* setup vertex position  information*/
	    for (i = 0; i < count; i++)
		pfCopyVec3(polygon->coords[i], v[vi[i]]);

	    /* setup (disable) color information */
	    polygon->cbind = PFGS_OFF;

	    /* setup normal vector information */
	    if (normalsValid)
	    {
		polygon->nbind = PFGS_PER_VERTEX;
		for (i = 0; i < count; i++)
		    pfCopyVec3(polygon->norms[i], n[ni[i]]);
	    }
	    else
		polygon->nbind = PFGS_OFF;

	    /* setup texture coordinates information */
	    if (pfdGetBldrStateMode(PFSTATE_ENTEXGEN) > 0)
	    {
		polygon->tbind[0] = PFGS_PER_PRIM;
		for (k = 1 ; k < PF_MAX_TEXTURES ; k ++)
		    polygon->tbind[k] = PFGS_OFF;
	    }
	    else
	    if (textureValid)
	    {
		polygon->tbind[0] = PFGS_PER_VERTEX;
		for (k = 1 ; k < PF_MAX_TEXTURES ; k ++)
		    polygon->tbind[k] = PFGS_OFF;
		for (i = 0; i < count; i++)
		    pfCopyVec2(polygon->texCoords[0][i], t[ti[i]]);
	    }
	    else
		for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
		    polygon->tbind[k] = PFGS_OFF;

	    if (count > 2)
	    {
		pfdAddBldrGeom(polygon, 1);
		numTris += count - 2;
		numPolys++;
	    }
	}
	else
	if (SAME(token, "usemtl"))
	{
	    char	mtlName[PF_MAXSTRING];
	    sscanf(next, "%s", mtlName);
	    useMtl(mtlName);
	}
	else
	if (SAME(token, "mtllib"))
	{
	    char	libName[PF_MAXSTRING];
	    sscanf(next, "%s", libName);
	    loadMtl(libName);
	}
	else
	if (
	    SAME(token, "bevel")	||
	    SAME(token, "bmat")		||
	    SAME(token, "bsp")		||
	    SAME(token, "bzp")		||
	    SAME(token, "c_interp")	||
	    SAME(token, "cdc")		||
	    SAME(token, "con")		||
	    SAME(token, "cstype")	||
	    SAME(token, "ctech")	||
	    SAME(token, "curv")		||
	    SAME(token, "curv2")	||
	    SAME(token, "d_interp")	||
	    SAME(token, "deg")		||
	    SAME(token, "end")		||
	    SAME(token, "hole")		||
	    SAME(token, "l")		||
	    SAME(token, "lod")		||
	    SAME(token, "maplib")	||
	    SAME(token, "mg")		||
	    SAME(token, "o")		||
	    SAME(token, "p")		||
	    SAME(token, "param")	||
	    SAME(token, "parm")		||
	    SAME(token, "res")		||
	    SAME(token, "s")		||
	    SAME(token, "scrv")		||
	    SAME(token, "shadow_obj")	||
	    SAME(token, "sp")		||
	    SAME(token, "stech")	||
	    SAME(token, "step")		||
	    SAME(token, "surf")		||
	    SAME(token, "trace_obj")	||
	    SAME(token, "trim")		||
	    SAME(token, "usemap")	||
	    SAME(token, "vp"))
	{
	    ++numSkip;
	}
#ifndef	STRICT_OBJ_FORMAT
	/*
	 * reset vertex data array counters -- this is not
	 * part of the OBJ format, but proves quite handy.
	 */
	else
	if (SAME(token, "RESET"))
	{
	    vCount = 0;
	    nCount = 0;
	    tCount = 0;
	}
#endif
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_INTERNAL, "unrecognized: %s", buffer);
	    ++numOther;
	}
    }

    /* close Wavefront ".obj" file */
    fclose(objFile);

    /* release dynamically allocated vertex, normal, and texture data */
    if (v != NULL) pfFree(v);
    if (n != NULL) pfFree(n);
    if (t != NULL) pfFree(t);
    pfdDelGeom(polygon);

    /* 
     * note -- if you define this, then material files will be read anew
     *         for every file that references them. could be slow for big
     *         material files and many models. on the other hand, if you
     *         don't define it then the lists will linger after the files
     *         have all been loaded -- wasting space.
     */
#ifdef	FORGETFUL
    /* discard material info */
    forgetMaterials();
    forgetMaterialFiles();
#endif

    /* Build All geometry */
    node = pfdBuild();

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_obj: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    if (numSkip != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  
	    "    Skipped tokens:     %8ld", numSkip);
    if (numOther != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  
	    "    Unrecognized tokens:%8ld", numOther);
    if (numGroups != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  
	    "    Groups processed:   %8ld", numGroups);
    if (vCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  
	    "    Vertex coordinates: %8ld", vCount);
    if (nCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  
	    "    Vertex normals:     %8ld", nCount);
    if (tCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  
	    "    Texture coordinates:%8ld", tCount);
    if (numTris != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  
	    "    Input polygons:     %8ld", numPolys);
    if (numTris != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  
	    "    Total triangles:    %8ld", numTris);

    /* return address of hierarchy to caller */
    return node;
}

/***
 ***	P R O C E S S    M A T E R I A L    F I L E S
 ***/

#define		MTL_NOT_DEFINED		0x0001
#define		MTL_HAS_GEOSTATE	0x0002
#define		MTL_HAS_AMBIENT		0x0004
#define		MTL_HAS_DIFFUSE		0x0008
#define		MTL_HAS_ALPHA		0x0010
#define		MTL_HAS_SPECULAR	0x0020
#define		MTL_HAS_SHININESS	0x0040
#define		MTL_HAS_TEXTURE		0x0080
#define		MTL_HAS_REFLECTION	0x0100
#define		MTL_IS_TWO_SIDED	0x0200

/*
 * data for a single wavefront material
 */
typedef struct objMaterial
{
    char	name[PF_MAXSTRING];	/* material name */
    int		defined;		/* defined fields */
    pfVec3	ambient; 		/* Ka field */
    pfVec3	diffuse; 		/* Kd field */
    pfVec3	specular; 		/* Ks field */
    float	alpha; 			/* Tr field (actually, 1-Tr) */
    float	shininess; 		/* Ns field */
    char	texture[PF_MAXSTRING];	/* texture map name */
    float	su;			/* u-axis tc scale */
    float	sv;			/* v-axis tc scale */
    char	reflect[PF_MAXSTRING]; 	/* reflection mode */
} objMaterial;

static objMaterial	*mtlList	= NULL;
static int		 mtlListCount	= 0;
static int		 mtlListSize	= 0;

static void
resetMaterial (objMaterial *m)
{
    if (m == NULL)
	return;

    memset(m->name, '\0', sizeof(m->name));
    m->defined = 0;
    pfSetVec3(m->ambient,  0.0f, 0.0f, 0.0f);
    pfSetVec3(m->diffuse,  0.0f, 0.0f, 0.0f);
    pfSetVec3(m->specular, 0.0f, 0.0f, 0.0f);
    m->alpha = 1.0f;
    m->shininess = 0.0f;
    memset(m->texture, '\0', sizeof(m->texture));
    m->su = 0.0f;
    m->sv = 0.0f;
    memset(m->reflect, '\0', sizeof(m->reflect));
}

static void
rememberMaterial (objMaterial *m)
{
    int		i;

    if (m == NULL)
	return;

#ifdef	PRINT_MATERIALS_AS_PARSED
    printf("     name: %s\n", m->name);
    printf("  defined: 0x%x\n", m->defined);
    printf("  ambient: %g %g %g\n", m->ambient[0],  m->ambient[1],  m->ambient[2]);
    printf("  diffuse: %g %g %g\n", m->diffuse[0],  m->diffuse[1],  m->diffuse[2]);
    printf(" specular: %g %g %g\n", m->specular[0], m->specular[1], m->specular[2]);
    printf("    alpha: %g\n", m->alpha);
    printf("shininess: %g\n", m->shininess);
    printf("  texture: %s\n", m->texture);
    printf("       su: %g\n", m->su);
    printf("       sv: %g\n", m->sv);
    printf("  reflect: %s\n", m->reflect);
    printf("\n");
#endif

    /* check for "empty" definition -- don't add junk */
    if (m->name[0] == '\0' || m->defined == 0)
	return;
    
    /* look for name in material list -- don't add duplicates */
    for (i = 0; i < mtlListCount; i++)
	if (strcmp(m->name, mtlList[i].name) == 0)
	{
	    /* XXX the best thing is to do a compare and warn user */
	    /* XXX if the old and new definitions are not identical */
	    return;
	}
    
    /* grow material list */
    if (mtlListSize == 0)
    {
	/* create material list */
	mtlListSize = 1;
	mtlList = (objMaterial *)pfMalloc(mtlListSize*sizeof(objMaterial), NULL);
    }
    else
    if (mtlListCount >= mtlListSize)
    {
	/* grow material list */
	mtlListSize *= 2;
	mtlList = (objMaterial *)pfRealloc(mtlList, mtlListSize*sizeof(objMaterial));
    }
#ifdef	PRINT_MATERIALS_AS_PARSED
    printf("mtlListSize=%d, mtlListCount=%d, sizeof(objMaterial)=%d\n",
	mtlListSize, mtlListCount, sizeof(objMaterial));
#endif

    /* add material to list */
    mtlList[mtlListCount++] = *m;
}

static void
forgetMaterials (void)
{
    /* delete list */
    if (mtlList != NULL)
	pfFree(mtlList);
    mtlList = 0;
    mtlListSize = 0;
    mtlListCount = 0;
}

static void
defineMtl (objMaterial *m)
{
    static char	*suffix[] = 
    {
	".rgb", 
	".rgba", 
	".int", 
	".inta", 
	".bw"
    };
    static int numSuffix = sizeof(suffix)/sizeof(suffix[0]);

    pfMaterial *material   = (pfMaterial *) 
	pfdGetTemplateObject(pfGetMtlClassType());
    pfTexture  *texture    = (pfTexture *) 
	pfdGetTemplateObject(pfGetTexClassType());
    pfLightModel *lm 	   = (pfLightModel *) 
	pfdGetTemplateObject(pfGetLModelClassType());

    pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_OFF);
    pfdMakeDefaultObject((pfObject *)material);

    if (m->defined & (MTL_HAS_AMBIENT  | MTL_HAS_DIFFUSE  | 
	MTL_HAS_SPECULAR | MTL_HAS_ALPHA))
    {
	if (m->defined & MTL_HAS_AMBIENT)
	{
	    pfMtlColor(material, PFMTL_AMBIENT, 
		m->ambient[0], m->ambient[1], m->ambient[2]);
	}

	if (m->defined & MTL_HAS_DIFFUSE)
	{
	    pfMtlColor(material, PFMTL_DIFFUSE, 
		m->diffuse[0], m->diffuse[1], m->diffuse[2]);
	}

	if (m->defined & MTL_HAS_SPECULAR)
	{
	    pfMtlColor(material, PFMTL_SPECULAR, 
		m->specular[0], m->specular[1], m->specular[2]);
	}

	if (m->defined & MTL_HAS_SHININESS)
	{
	    pfMtlShininess(material, m->shininess);
	}

	if (m->defined & MTL_HAS_ALPHA)
	{
	    pfMtlAlpha(material, m->alpha);
	    pfdBldrStateMode(PFSTATE_TRANSPARENCY, PFTR_ON);
	}

	pfdBldrStateAttr(PFSTATE_FRONTMTL, material);

	if (m->defined & MTL_IS_TWO_SIDED)
	{
	    pfLModelTwoSide(lm, PF_ON);
	    pfdBldrStateAttr(PFSTATE_LIGHTMODEL, lm);
	    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_OFF);

	    pfdBldrStateAttr(PFSTATE_BACKMTL, material);
	}
    }

    if (m->defined & (MTL_HAS_TEXTURE | MTL_HAS_REFLECTION))
    {
	pfTexture	*tex = NULL;
	int	 	 j, comp, foo;
	unsigned int	*image;
	char		*sp;

	/* convert texture name from blah.rla to blah.rgb etc. */
	for (j = 0; j < numSuffix; j++)
	{
	    char	 texPath[PF_MAXSTRING];

	    /* remove ".rla" or whatever suffix from file name */
	    if ((sp = strrchr(m->texture, (int)'.')) != NULL)
		*sp = '\0';

	    /* append one of the known image file extensions */
	    strcat(m->texture, suffix[j]);

	    /* see if file is locatable with this extension */
	    if (pfFindFile(m->texture, texPath, R_OK))
		break;
	}

	if (j < numSuffix)
	{
	    pfTexture *realTexture;

	    pfTexName(texture, m->texture);
	    pfdBldrStateAttr(PFSTATE_TEXTURE, texture);

	    realTexture = (pfTexture *)pfdGetBldrStateAttr(PFSTATE_TEXTURE);
	    pfGetTexImage(realTexture, &image, &comp, &foo, &foo, &foo);

	    if (comp == 2 || comp == 4)
	    {
		pfdBldrStateMode(PFSTATE_ALPHAFUNC, PFAF_GREATER);
		pfdBldrStateVal(PFSTATE_ALPHAREF, (4.0f/255.0f));
		pfdBldrStateMode(PFSTATE_TRANSPARENCY, PFTR_ON);
	    }

	    if (m->defined & MTL_HAS_REFLECTION)
	    {
		pfTexGen   *texturegen = (pfTexGen *) 
		    pfdGetTemplateObject(pfGetTGenClassType());

		if (strcmp(m->reflect, "sphere") == 0)
		{
		    pfTGenMode(texturegen, PF_S, PFTG_SPHERE_MAP);
		    pfTGenMode(texturegen, PF_T, PFTG_SPHERE_MAP);
		}
		pfdBldrStateAttr(PFSTATE_TEXGEN, texturegen);
		pfdBldrStateMode(PFSTATE_ENTEXGEN, PF_ON);

		pfdResetObject((pfObject *)texturegen);
	    }
	}
	else
	{
	    /* file not found -- set name to NULL string */
	    if ((sp = strrchr(m->texture, (int)'.')) != NULL)
		*sp = '\0';
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		"can't find texture \"%s\" for material \"%s\"", 
		m->texture, m->name);
	}
    }

    pfdSaveBldrState(strdup(m->name));

    pfdResetObject((pfObject *)material);
    pfdResetObject((pfObject *)texture);
    pfdResetObject((pfObject *)lm);

    pfdResetBldrState();

    m->defined |= MTL_HAS_GEOSTATE;
}

static void
useMtl (char *name)
{
    int i;

    /* reset to default state */
    pfdResetBldrState();

    /* look for name in material list */
    for (i = 0; i < mtlListCount; i++)
    {
	if (strcmp(name, mtlList[i].name) == 0)
	{
	    /* is this a "missing" material */
	    if (mtlList[i].defined & MTL_NOT_DEFINED)
	    {
	    }
	    else
	    {
		/* define state before first use */
		if ((mtlList[i].defined & MTL_HAS_GEOSTATE) == 0)
		    defineMtl(&mtlList[i]);

		/* specify the state by name */
		pfdLoadBldrState(name);
	    }

	    return;
	}
    }
    
    /* if we can't find material, then use default */
    if (i >= mtlListCount)
    {
	objMaterial missing;

	/* print warning once */
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "material \"%s\" not defined", name);

	/* remember missing material's name and status */
	resetMaterial(&missing);
	strcpy(missing.name, name);
	missing.defined = MTL_NOT_DEFINED;
	rememberMaterial(&missing);
    }
}

/*
 * parse wafefront material file texture definition
 */
static void
parseTexture (char *next, objMaterial *m)
{
    int		 width = 0;

    /* set default texture scale factors */
    m->su = 1.0f;
    m->sv = 1.0f;

    /* parse texture name */
    sscanf(next, "%s%n", m->texture, &width);

    /* parse texture option strings */
    do
    {
	next += width;

	if (strcmp(m->texture, "-mm") == 0)
	    sscanf(next, "%f %f %s%n",     &m->su, &m->sv, m->texture, &width);
	else
	if (strcmp(m->texture, "-s") == 0)
	    sscanf(next, "%f %f %*f %s%n", &m->su, &m->sv, m->texture, &width);
	else
	if (strcmp(m->texture, "-t") == 0)
	    sscanf(next, "%f %f %*f %s%n", &m->su, &m->sv, m->texture, &width);
	else
	if (strcmp(m->texture, "-type") == 0)
	    sscanf(next, "%s %s%n", m->reflect, m->texture, &width);
	else
	    break;
    } while (1);
}

static char	*mtlFileList[MAX_MTL_FILES];
static int	 mtlCount = 0;

static void
forgetMaterialFiles (void)
{
    int		i;

    for (i = 0; i < mtlCount; i++)
	if (mtlFileList[i] != NULL)
	    free(mtlFileList[i]);

    mtlCount = 0;
}

/*
 * load a wavefront-format material file (".mtl")
 */
static void
loadMtl (char *fileName)
{
    FILE	*mtlFile;
    char	 buffer[BUFFER_SIZE];
    char	 token[BUFFER_SIZE];
    char	*next;
    char	*backslash;
    int		 width	= 0;
    int		 i;
    int 	 inProgress = 0;
    objMaterial  current;

    /* have we already loaded this file ? */
    for (i = 0; i < mtlCount; i++)
	if (strcmp(fileName, mtlFileList[i]) == 0)
	{
#ifdef VERBOSE
	    /* indicate redundant load request */
	    pfNotify(PFNFY_DEBUG, PFNFY_INTERNAL,
		"material library %s already loaded", fileName);
#endif
	    /* bail out */
	    return;
	}

    /* remember file name for future reference */
    if (i < MAX_MTL_FILES)
    {
	mtlFileList[mtlCount++] = strdup(fileName);
    }
    else
    {
	pfNotify(PFNFY_DEBUG, PFNFY_INTERNAL,
	    "more than %d unique mtllibs. enlarge MAX_MTL_FILES", 
	    MAX_MTL_FILES);
    }

    /* open file */
    if ((mtlFile = pfdOpenFile(fileName)) == NULL)
    {
	pfNotify(PFNFY_DEBUG, PFNFY_INTERNAL,
	    "can't open material library %s", fileName);
	return;
    }

    /* read Wavefront ".mtl" file */
    while (fgets(buffer, BUFFER_SIZE, mtlFile) != NULL)
    {
	/* concatenate continuation lines */
	while ((backslash = strchr(buffer, '\\')) != NULL)
	    if (fgets(backslash, BUFFER_SIZE - strlen(buffer), mtlFile) == NULL)
		break;

	/* find first non-"space" character in line */
	for (next = buffer; *next != '\0' && isspace(*next); next++)
	    /* EMPTY */ {};

	/* skip blank lines and comments ('$' is comment in "cow.obj") */
	if (*next == '\0' || *next == '#' || *next == '!' || *next == '$')
	    continue;

	/* extract token */
	sscanf(next, "%s%n", token, &width);
	next += width;

	/* identify token */
	if (SAME(token, "newmtl"))
	{
	    /* save material definition */
	    if (inProgress)
		rememberMaterial(&current);

	    resetMaterial(&current);
	    inProgress = 1;
	    sscanf(next, "%s", current.name);
	}
	else
	if (SAME(token, "Ka"))
	{
	    sscanf(next, "%f %f %f", 
		&current.ambient[0], 
		&current.ambient[1], 
		&current.ambient[2]);
	    current.defined |= MTL_HAS_AMBIENT;
	}
	else
	if (SAME(token, "Kd"))
	{
	    sscanf(next, "%f %f %f", 
		&current.diffuse[0], 
		&current.diffuse[1], 
		&current.diffuse[2]);
	    current.defined |= MTL_HAS_DIFFUSE;
	}
	else
	if (SAME(token, "Ks"))
	{
	    sscanf(next, "%f %f %f", 
		&current.specular[0], 
		&current.specular[1], 
		&current.specular[2]);
	    current.defined |= MTL_HAS_SPECULAR;
	}
	else
	if (SAME(token, "Tr"))
	{
	    float alpha = 1.0f;
	    sscanf(next, "%f", &alpha);
	    current.alpha = 1.0f - alpha;
	    current.defined |= MTL_HAS_ALPHA;
	}
	else
	if (SAME(token, "Ns"))
	{
	    sscanf(next, "%f", &current.shininess);

	    /* clamp shininess range */
	    if (current.shininess <   0.0f)
		current.shininess =   0.0f;
	    else
	    if (current.shininess > 128.0f)
		current.shininess = 128.0f;

	    current.defined |= MTL_HAS_SHININESS;
	}
	else
	if (SAME(token, "map_Kd"))
	{
	    parseTexture(next, &current);
	    current.defined |= MTL_HAS_TEXTURE;
	}
	else
	if (SAME(token, "refl"))
	{
	    strcpy(current.reflect, "sphere");
	    parseTexture(next, &current);
	    current.defined |= MTL_HAS_REFLECTION;
	}
	else
	if (
	    SAME(token, "Ni")	        ||
	    SAME(token, "Tf")		||
	    SAME(token, "bump")	        ||
	    SAME(token, "d")		||
	    SAME(token, "decal")	||
	    SAME(token, "illum")	||
	    SAME(token, "map_Ka")	||
	    SAME(token, "map_Ks")	||
	    SAME(token, "map_Ns")	||
	    SAME(token, "map_d")	||
	    SAME(token, "sharpness")	||
	    SAME(token, "vp"))
	{
	    numSkip++;
	}
#ifndef	STRICT_OBJ_FORMAT
	/*
	 * indicate that this material is two-sided
	 */
	else
	if (SAME(token, "TWOSIDE"))
	{
	    current.defined |= MTL_IS_TWO_SIDED;
	}
#endif
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_INTERNAL, " unrecognized: %s", buffer);
	    numOther++;
	}
    }

    if (inProgress)
	rememberMaterial(&current);

    /* close Wavefront ".mtl" file */
    fclose(mtlFile);
}
