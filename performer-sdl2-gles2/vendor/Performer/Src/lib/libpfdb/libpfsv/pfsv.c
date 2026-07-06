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
 * pfsv.c: $Revision: 1.42 $ $Date: 2002/11/17 01:40:32 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef WIN32
#define strcasecmp _stricmp
#endif

#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFSV_DLLEXPORT __declspec(dllexport)
#else
#define PFSV_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFSV_DLLEXPORT
#endif

#ifdef	_POSIX_SOURCE
extern int strcasecmp (const char *s1, const char *s2);
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>

/* case insensitive string equality test */
#define	SAME(_a, _b)	(strcasecmp(_a,_b) == 0)

/* program capacity constants */
#define	MAX_MATERIALS	512
#define	MAX_TEXTURES	512
#define	MAX_STRING	256

/*
 * pfdLoadFile_sv -- Load i3dm ".sv" files into IRIS Performer
 */

PFSV_DLLEXPORT /*extern */pfNode *
pfdLoadFile_sv (char *fileName)
{
    FILE	*svFile		= NULL;
    pfdGeom	*buffer		= NULL;
    char	*next		= NULL;
    pfNode	*node		= NULL;
    int		 width		= 0;
    int		 i;
    int		 cullDefault	= PFCF_BACK;
    int		 numTris	= 0;
    int		 numVerts	= 0; 
    int 	 numSkip	= 0;
    int		 numOther	= 0;
    char	 input[MAX_STRING];
    char	 token[MAX_STRING];
    int		 t;

    pfMaterial	*material[MAX_MATERIALS] = {NULL};
    pfTexture	*texture[MAX_TEXTURES] = {NULL};

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_sv: %s", fileName);

    /* open ".sv" file */
    if ((svFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* initialize utility library triangle/geoset builder */
    buffer = pfdNewGeom(4096);

    /* read ".sv" file */
    while (fgets(input, MAX_STRING, svFile) != NULL)
    {
	/* find first non-"space" character in line */
	for (next = input; *next != '\0' && isspace(*next); next++)
	    /* EMPTY */ {};
	
	/* skip blank lines and comments */
	if (*next == '\0' || *next == '#' || *next == '/')
	    continue;
	
	/* extract token */
	sscanf(next, "%s%n", token, &width);
	next += width;

	/* 
	 * identify token 
	 */

	if (SAME(token, "model"))
	{
	    pfdBldrStateMode(PFSTATE_CULLFACE, cullDefault);
	}
	else
	if (SAME(token, "endmodel"))
	{
	}
	else
	if (SAME(token, "backface"))
	{
	    int		  cullMode;
	    char	  mode[MAX_STRING];
	    pfLightModel *lm = (pfLightModel *)
		pfdGetTemplateObject(pfGetLModelClassType());

	    /* determine new cull mode ("off" means "backfaces are off") */
	    sscanf(next, "%s", mode);

	    if (SAME(mode, "off"))
	    {
		pfLModelTwoSide(lm, PF_OFF);
		pfdBldrStateAttr(PFSTATE_LIGHTMODEL, lm);
		pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);
	    }
	    else
	    {
		pfLModelTwoSide(lm, PF_ON);
		pfdBldrStateAttr(PFSTATE_LIGHTMODEL, lm);
		pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_OFF);
	    }
	}
	else
	if (SAME(token, "material"))
	{
	    int		 parsed;
	    int		 index;
	    float	 ar =  0.2f, ag = 0.2f, ab = 0.2f;
	    float	 dr =  0.8f, dg = 0.8f, db = 0.8f;
	    float	 sr =  0.3f, sg = 0.3f, sb = 0.3f;
	    float	 sh = 30.0f;

	    /* read material definition from input */
	    parsed = sscanf(next, "%d %f%f%f %f%f%f %f%f%f %f", 
		&index,  &ar, &ag, &ab,  &dr, &dg, &db,  &sr, &sg, &sb,  &sh);

	    /* skip invalid material definitions */
	    if (parsed < 1 || index < 0 || index >= MAX_MATERIALS)
		continue;

	    /* define a new current material */
	    if (parsed > 1)
	    {
		/* allocate material definition */
		material[index] = pfNewMtl(pfGetSharedArena());

		/* specify values in material definition */
		pfMtlColor(material[index], PFMTL_AMBIENT,  ar, ag, ab);
		pfMtlColor(material[index], PFMTL_DIFFUSE,  dr, dg, db);
		pfMtlColor(material[index], PFMTL_SPECULAR, sr, sg, sb);
		pfMtlShininess(material[index], sh);
		pfMtlColorMode(material[index], PFMTL_FRONT, PFMTL_CMODE_OFF);
		pfMtlSide(material[index], PFMTL_FRONT);
	    }

	    /* specify updated material as builder's current material */
	    if (parsed  > 0)
	    {
		pfdBldrStateAttr(PFSTATE_FRONTMTL, material[index]);
		pfdBldrStateAttr(PFSTATE_BACKMTL,  material[index]);
	    }
	}
	else
	if (SAME(token, "texture"))
	{
	    int		parsed;
	    int		index;
	    char	textureName[MAX_STRING];
	    char	texturePath[MAX_STRING];

	    /* read texture definition from input */
	    parsed = sscanf(next, "%d %s", &index, textureName);

	    /* skip invalid texture definitions */
	    if (parsed < 1 || index < 0 || index >= MAX_TEXTURES)
		continue;
	    
	    /* define a new current texture */
	    if (parsed > 1)
	    {
		/* find texture file in file search path */
		if (pfFindFile(textureName, texturePath, R_OK) == 0)
		    continue;

		/* specify the texture */
		texture[index] = pfNewTex(pfGetSharedArena());
		pfTexName(texture[index], texturePath);
	    }

	    /* specify the texture */
	    pfdBldrStateAttr(PFSTATE_TEXTURE, texture[index]);
	    pfdBldrStateMode(PFSTATE_ENTEXTURE, PFTR_ON);
	}
	else
	if (SAME(token, "poly3dn") || SAME(token, "poly3d"))
	{
	    int		count;
	    int		parsed;
	    int		hasNormals	= 1;
	    int		hasTextures	= 1;

	    /* get number of vertices in polygon */
	    if (sscanf(next, "%d", &count) != 1)
		continue;

	    /* read vertex definitions */
	    for (i = 0; i < count; i++)
	    {
		/* get next line */
		if (fgets(input, MAX_STRING, svFile) == NULL)
		    break;

		/* read whatever values are present */
		parsed = sscanf(input, "%f%f%f %f%f%f %f%f", 
		    &buffer->coords[i][0], 
		    &buffer->coords[i][1], 
		    &buffer->coords[i][2],
		    &buffer->norms [i][0], 
		    &buffer->norms [i][1], 
		    &buffer->norms [i][2],
		    &buffer->texCoords[0][i][0], 
		    &buffer->texCoords[0][i][1]);
		
		/* lowest-common-denominator vertex determines polygon type */
		if (parsed < 8)
		    hasTextures = 0;
		if (parsed < 6)
		    hasNormals = 0;
		if (parsed < 3)
		    break;
	    }

	    /* send polygon to the builder */
	    buffer->numVerts = i;
	    buffer->primtype = PFGS_POLYS;
	    buffer->cbind = PFGS_OFF;
	    buffer->nbind = hasNormals  ? PFGS_PER_VERTEX : PFGS_OFF;
	    buffer->tbind[0] = hasTextures ? PFGS_PER_VERTEX : PFGS_OFF;
            for (t = 1 ; t < PF_MAX_TEXTURES ; t ++)
		buffer->tbind[t] = PFGS_OFF;

	    pfdAddBldrGeom(buffer, 1);

	    numTris  += buffer->numVerts - 2;
	    numVerts += buffer->numVerts;
	}
	else
	if (SAME(token, "tmeshn"))
	{
	    int		count = 0;
	    pfVec3	pArray[3];
	    pfVec3	nArray[3];
	    pfVec2	tArray[3];

	    /* get number of vertices in triangle strip */
	    if (sscanf(next, "%d", &count) != 1)
		continue;

	    /* read triangle strip definition */
	    for (i = 0; i < count; i++)
	    {
		int		n = i % 3;
		int		parsed = 0;
		int		hasNormals = 1;
		int		hasTextures = 1;

		/* get next line */
		if (fgets(input, MAX_STRING, svFile) == NULL)
		    break;

		/* read whatever values are present */
		parsed = sscanf(input, "%f%f%f %f%f%f %f%f", 
		    &pArray[n][0], &pArray[n][1], &pArray[n][2],
		    &nArray[n][0], &nArray[n][1], &nArray[n][2],
		    &tArray[n][0], &tArray[n][1]);
		
		/* lowest-common-denominator vertex determines polygon type */
		if (parsed < 8)
		    hasTextures = 0;
		if (parsed < 6)
		    hasNormals = 0;
		if (parsed < 3)
		    break;

		/* send polygon to the builder */
		if (i >= 2)
		{
		    int 	j;

		    buffer->numVerts = 3;
		    buffer->primtype = PFGS_POLYS;
		    buffer->cbind = PFGS_OFF;
		    buffer->nbind = hasNormals  ? PFGS_PER_VERTEX : PFGS_OFF;
		    for (t = 1 ; t < PF_MAX_TEXTURES ; t ++)
			buffer->tbind[t] = PFGS_OFF;
		    buffer->tbind[0] = hasTextures ? PFGS_PER_VERTEX : PFGS_OFF;

		    /* copy last three vertices (CCW or CW) */
		    if (((i - 2) & 1) == 0)
		    {
			for (j = 0; j < 3; j++)
			{
			    int		k = (n + j + 2) % 3;
			    pfCopyVec3(buffer->coords[j], pArray[k]);
			    pfCopyVec3(buffer->norms [j], nArray[k]);
			    pfCopyVec2(buffer->texCoords[0][j], tArray[k]);
			}
		    }
		    else
		    {
			for (j = 0; j < 3; j++)
			{
			    int		k = (n + 2 - j + 2) % 3;
			    pfCopyVec3(buffer->coords[j], pArray[k]);
			    pfCopyVec3(buffer->norms [j], nArray[k]);
			    pfCopyVec2(buffer->texCoords[0][j], tArray[k]);
			}
		    }

		    pfdAddBldrGeom(buffer, 1);

		    numTris  += buffer->numVerts - 2;
		    numVerts += buffer->numVerts;
		}
	    }
	}
	else
	if (SAME(token, "line3d"))
	{
	    int		count;

	    /* get number of vertices in line strip */
	    if (sscanf(next, "%d", &count) != 1)
		continue;

	    ++numSkip;

	    /* read line strip definition */
	    for (i = 0; i < count; i++)
	    {
		if (fgets(input, MAX_STRING, svFile) == NULL)
		    break;
	    }
	}
	else
	if (SAME(token, "action") || 
	    SAME(token, "background_color") || 
	    SAME(token, "backset") || 
	    SAME(token, "concave") || 
	    SAME(token, "environment_map") || 
	    SAME(token, "eye_distance") || 
	    SAME(token, "gouraud") || 
	    SAME(token, "initial") || 
	    SAME(token, "jklogo") || 
	    SAME(token, "light") || 
	    SAME(token, "origin") || 
	    SAME(token, "reference") || 
	    SAME(token, "shade") || 
	    SAME(token, "shademode") || 
	    SAME(token, "texture_env") || 
	    SAME(token, "trans_init") || 
	    SAME(token, "transparent") || 
	    SAME(token, "window"))
	    ++numSkip;
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_sv: unrecognized: %s", input);
	    ++numOther;
	}
    }

    /* close ".sv" file */
    fclose(svFile);
    pfdDelGeom(buffer);

    /* get a complete scene graph representing file's polygons */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Commands skipped:   %8ld", numSkip);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Commands unknown:   %8ld", numOther);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", numVerts);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input triangles:    %8ld", numTris);

    return node;
}
