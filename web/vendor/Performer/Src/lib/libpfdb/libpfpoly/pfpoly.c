/*
 * Copyright 1994, 1995, Silicon Graphics, Inc.
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
 * pfpoly.c: $Revision: 1.18 $ $Date: 2002/11/07 04:02:08 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#include <windows.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#ifdef	_POSIX_SOURCE
extern int strcasecmp (const char *s1, const char *s2);
#endif

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

/* length of longest input line in ".poly" file */
#define	BUFFER_SIZE	20000

/* initial allocation size for growable arrays */
#define	CHUNK		2048

/* case insensitive string equality test */
#ifdef WIN32
#define SAME(_a, _b)	(_stricmp(_a, _b) == 0)
#else
#define	SAME(_a, _b)	(strcasecmp(_a,_b) == 0)
#endif

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

typedef struct polyVertex 
{
    pfVec3	xyz;
    pfVec4	rgba;
    pfVec3	norm;
    pfVec2	st;
    int		valid;
#define		  VALID_XYZ	0x01
#define		  VALID_RGBA	0x02
#define		  VALID_NORM	0x04
#define		  VALID_ST	0x08
} polyVertex;

/*
 * pfdLoadFile_poly -- Load PRISMS ".poly" files 
 */

static char *
getNextLine (FILE *file, char *buffer, int bufferSize)
{
    char *next;

    /* read file */
    while (fgets(buffer, bufferSize, file) != NULL)
    {
	/* find first non-"space" character in line */
	for (next = buffer; *next != '\0' && isspace(*next); next++)
	    /* EMPTY */ {};
	
	/* skip blank lines and comments */
	if (*next == '\0' || *next == '#' || *next == '!' || *next == '$')
	    continue;
	
	/* return pointer to first non-"space" character in line */
	return next;
    }

    /* fgets() failed -- must be EOF or ERROR */
    return NULL;
}
	
/*
 * pfdLoadFile_poly -- Load ".poly" files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_poly (char *fileName)
{
    FILE	*polyFile;
    pfdGeom	*geom;
    pfNode	*node		= NULL;
    int		 width		= 0;
    int		 i;
    int		 numTris	= 0;
    int		 numPolys	= 0; 
    int		 geomSize	= 256;
    char	 input[BUFFER_SIZE];
    char	 token[BUFFER_SIZE];
    char	*next		= NULL;
    pfMaterial  *m      	= NULL;
    double	 startTime	= pfGetTime();

    /* growable vertex list (xyz, rgba, norm, st) */
    polyVertex	*v		= NULL; 
    unsigned int vCount		= 0;
    unsigned int vAvailable	= 0;

    int	 	 t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open Prisims ".poly" file file */
    if ((polyFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* allocate a new primitive buffer */
    geom = pfdNewGeom(geomSize);

#define RANDOM_COLOR
#ifdef  RANDOM_COLOR
    /* pick a random not-too-dark color */
    pfuRandomize(*(int *)&startTime);
    pfuRandomColor(geom->colors[0], 0.2f, 0.7f);
#endif

    /* establish material definition for model */
    m = (pfMaterial*)pfdGetTemplateObject(pfGetMtlClassType());
    pfMtlColor(m, PFMTL_AMBIENT,  0.2f, 0.2f, 0.2f);
    pfMtlColor(m, PFMTL_DIFFUSE,  0.8f, 0.8f, 0.8f);
    pfMtlColor(m, PFMTL_SPECULAR, 1.0f, 1.0f, 1.0f);
    pfMtlShininess(m, 30.0f);
    pfMtlColorMode(m, PFMTL_FRONT, PFMTL_CMODE_AD);
    pfdBldrStateAttr(PFSTATE_FRONTMTL, m);
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_ON);
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);

    /* 
     * read "POINTS" definition from ".poly" file 
     */

    /* get next significant line from file */
    next = getNextLine(polyFile, input, BUFFER_SIZE);

    /* some problem reading file */
    if (next == NULL)
	return NULL;

    /* extract token from line */
    sscanf(next, "%s%n", token, &width);
    next += width;

    /* if token is not "POINTS" then we're lost */
    if (!SAME(token, "points"))
	return NULL;

    /* read vertex definitions from file */
    while (1)
    {
	int	index;

	/* get next significant line from file */
	next = getNextLine(polyFile, input, BUFFER_SIZE);

	/* if token is not vertex number then look for polys */
	if (sscanf(next, "%d:%n", &index, &width) != 1)
	    break;
	next += width;

	/* enlarge vertex list */
	GROW(v, polyVertex);
	v[vCount].valid = 0;

	/* parse vertex coordinates */
	if (sscanf(next, "%f %f %f%n", 
	    &v[vCount].xyz[0], 
	    &v[vCount].xyz[1], 
	    &v[vCount].xyz[2], 
	    &width) == 3);
	{
	    v[vCount].valid |= VALID_XYZ;
	    next += width;
	}
	
	/* cycle through choices for other vertex attributes */
	while (next != NULL && *next != '\0')
	{
	    /* find first non-"space" character in line */
	    while (*next != '\0' && (isspace(*next) || *next == '<'))
		next++;
	    
	    /* break out of loop when end of string reached */
	    if (*next == '\0')
		break;
	
	    /* presume alpha of 1 */
	    v[vCount].rgba[3] = 1.0f;

	    /* look for a matching vertex attribute */
	    if (sscanf(next, "c(%f,%f,%f,%f)%n",
		&v[vCount].rgba[0], 
		&v[vCount].rgba[1],
		&v[vCount].rgba[2], 
		&v[vCount].rgba[3],
		&width) >= 3)
	    {
		v[vCount].valid |= VALID_RGBA;
		next += width;
	    }
	    else
	    if (sscanf(next, "n(%f,%f,%f)%n",
		&v[vCount].norm[0], 
		&v[vCount].norm[1], 
		&v[vCount].norm[2],
		&width) == 3)
	    {
		v[vCount].valid |= VALID_NORM;
		next += width;
	    }
	    else
	    if (sscanf(next, "uv(%f,%f)%n",
		&v[vCount].st[0], 
		&v[vCount].st[1], 
		&width) == 2)
	    {
		v[vCount].valid |= VALID_ST;
		next += width;
	    }
	    else
	    {
		/* unknown character - terminate scan for this vertex */
		next = NULL;
	    }
	}

#ifdef	DEBUG
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,  "vertex %d", vCount);
	if (v[vCount].valid & VALID_XYZ)
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "   xyz: %g %g %g", 
		v[vCount].xyz[0], 
		v[vCount].xyz[1], 
		v[vCount].xyz[2]);
	if (v[vCount].valid & VALID_RGBA)
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "  rgba: %g %g %g %g", 
		v[vCount].rgba[0], 
		v[vCount].rgba[1], 
		v[vCount].rgba[2],
		v[vCount].rgba[3]);
	if (v[vCount].valid & VALID_NORM)
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "  norm: %g %g %g", 
		v[vCount].norm[0], 
		v[vCount].norm[1], 
		v[vCount].norm[2]);
	if (v[vCount].valid & VALID_ST)
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "    st: %g %g", 
		v[vCount].st[0], 
		v[vCount].st[1]);
#endif

	/* advance vertex count */
	++vCount;
    }

    /* 
     * read "POLYS" definition from ".poly" file 
     */

    /* extract token from line */
    sscanf(next, "%s%n", token, &width);
    next += width;

    /* if token is not "POLYS" then we're lost */
    if (!SAME(token, "polys"))
	return NULL;

    /* read polygon definitions from file */
    while (next = getNextLine(polyFile, input, BUFFER_SIZE))
    {
	int	index;
	int	valid;
	pfVec4	rgba;
	
	/* find next non-"space" character in line */
	while (*next != '\0' && (isspace(*next) || *next == '<'))
	    next++;

	/* if token is not vertex number then we are done */
	if (sscanf(next, "%d:%n", &index, &width) != 1)
	    break;
	next += width;

	/* setup vertex attribute validity mask for this poly */
	valid = VALID_XYZ | VALID_RGBA | VALID_NORM | VALID_ST;

	/* dereference each vertex */
	for (i = 0; sscanf(next, "%d%n", &index, &width) == 1; i++)
	{
	    /* resize geometry geom if necessary */
	    if (geomSize < i)
		pfdResizeGeom(geom, geomSize = i*2);

	    /* advance past just-read index */
	    next += width;
	
	    /* index is 1-based in file, 0-based in array */
	    --index;

            if (index < 0 || index >= vCount)
            {
                pfNotify(PFNFY_WARN, PFNFY_PRINT,
                    "pfdLoadFile_poly: index %d out of range [0 .. %d]",
                    index, vCount);
		if (v != NULL) 
		    pfFree(v);
                pfdDelGeom(geom);
                pfdResetBldrGeometry();
                fclose(polyFile);
                return NULL;
            }

	    /* copy valid vertex attributes to builder */
	    if (v[index].valid & VALID_XYZ)
		pfCopyVec3(geom->coords[i], v[index].xyz);
	    else
		valid &= ~VALID_XYZ;

	    if (v[index].valid & VALID_RGBA)
		pfCopyVec4(geom->colors[i], v[index].rgba);
	    else
		valid &= ~VALID_RGBA;

	    if (v[index].valid & VALID_NORM)
		pfCopyVec3(geom->norms[i], v[index].norm);
	    else
		valid &= ~VALID_NORM;

	    if (v[index].valid & VALID_ST)
		pfCopyVec2(geom->texCoords[i], v[index].st);
	    else
		valid &= ~VALID_ST;
	}

	/* presume alpha of 1 */
	rgba[3] = 1.0f;

	/* find next non-"space" character in line */
	while (*next != '\0' && (isspace(*next) || *next == '<'))
	    next++;
	
	/* does this polygon have a specified color */
	if (sscanf(next, "c(%f,%f,%f,%f)",
	    &rgba[0], 
	    &rgba[1], 
	    &rgba[2], 
	    &rgba[3]) >= 3)
	{
	    int j;
	    for (j = 0; j < i; j++)
		pfCopyVec4(geom->colors[j], rgba);
	    valid |= VALID_RGBA;
	}

	/* 
	 * discard uninteresting polygon attributes. this is often
	 * reasonable, at least for the DISCARD_ST case, since the
	 * Prisims ".poly" files don't define or reference a texture 
	 * but merely provide texture coordinates.
	 */
#ifdef	DISCARD_ST
	if (valid & VALID_ST)
	{
	    static alreadyDone = 0;
	    if (alreadyDone == 0)
	    {
		valid &= ~VALID_ST;		/* discard textures */
                pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		    "pfdLoadFile_poly: discarding texture coordinates");
		alreadyDone = 1;
	    }
	}
#endif
#ifdef	DISCARD_NORM
	if (valid & VALID_NORM)
	{
	    static alreadyDone = 0;
	    if (alreadyDone == 0)
	    {
		valid &= ~VALID_NORM;	/* discard normals */
                pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		    "pfdLoadFile_poly: discarding supplied normals");
		alreadyDone = 1;
	    }
	}
#endif
#ifdef	DISCARD_RGBA
	if (valid & VALID_RGBA)
	{
	    static alreadyDone = 0;
	    if (alreadyDone == 0)
	    {
		valid &= ~VALID_RGBA;	/* discard textures */
                pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		    "pfdLoadFile_poly: discarding supplied colors");
		alreadyDone = 1;
	    }
	}
#endif

	/* add polygon to builder */
	if ((i >= 3) && (valid & VALID_XYZ))
	{
	    geom->numVerts = i;
	    geom->primtype = PFGS_POLYS;
	    geom->cbind = (valid & VALID_RGBA) ? PFGS_PER_VERTEX : 
#ifdef	RANDOM_COLOR
						 PFGS_PER_PRIM;
#else
						 PFGS_OFF;
#endif
	    geom->nbind = (valid & VALID_NORM) ? PFGS_PER_VERTEX : PFGS_OFF;
	    geom->tbind[0] = (valid & VALID_ST)   ? PFGS_PER_VERTEX : PFGS_OFF;

	    for (t = 1 ; t < PF_MAX_TEXTURES ; t ++)
		geom->tbind[t] = PFGS_OFF;
  
#ifdef	POLYGONS_ARE_IN_CW_ORDER
	    /* poly files seem to have CW polygon orientation so reverse */
	    pfdReverseGeom(geom);
#endif

	    pfdAddBldrGeom(geom, 1);
	    numTris += geom->numVerts - 2;
	    numPolys++;
	}
    }

    /* close ".poly" file */
    fclose(polyFile);
    pfdDelGeom(geom);

    /* release dynamically allocated vertex data */
    if (v != NULL) 
	pfFree(v);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_poly: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", vCount);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input polygons:     %8ld", numPolys);

    /* return pointer to in-memory scene-graph */
    return node;
}
