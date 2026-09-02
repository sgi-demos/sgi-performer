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
 * pf3ds.c: $Revision: 1.30 $ $Date: 2002/11/07 22:21:50 $
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#include "3dsftk.h"

static char *strlower(char *string);
static void setTexture(char *fileName);

extern int mergeTextures(char *rgbFileName, char *alphaFileName,
			 char *newFileName);

#ifdef WIN32
#define PFDB_DLLEXPORT __declspec(dllexport)
#else
#define PFDB_DLLEXPORT
#endif

/*
 * pfdLoadFile_3ds -- load AutoDesk 3DStudio ".3ds" files into IRIS Performer
 */

PFDB_DLLEXPORT pfNode *
pfdLoadFile_3ds (char *fileName)
{
    int		 i, j, k, f;
    pfNode	*node		= NULL;
    pfdGeom	*facet		= NULL;
    file3ds 	*dbFile		= NULL;
    database3ds *db		= NULL;
    namelist3ds *meshlist	= NULL;
    mesh3ds  	*mesh		= NULL;
    namelist3ds *materials	= NULL;
    int		 numTris	= 0;
    int		 numMaterials	= 0;
    int 	 txgn		= 0;
    char	filePath[512];

/* #define	BILLBOARD_SUPPORT */
#ifdef	BILLBOARD_SUPPORT
    int		 numBillboards  = 0;
    pfGroup	*bbGroup	= NULL;
    char	*normal		= "normal";
    char	*billboard	= "billboard";
#endif

    int		t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    txgn = pfdAddState(strdup("texgen"),
	8*sizeof(float), NULL, NULL, NULL, NULL, NULL);
    pfdStateCallback(txgn, PFDEXT_DRAW_PREFUNC, pfdPreDrawTexgenExt);
    pfdStateCallback(txgn, PFDEXT_DRAW_POSTFUNC, pfdPostDrawTexgenExt);

    /* 3ds loader uses strings to specify state names */
    pfdBldrAttr(PFDBLDR_STATE_NAME_COMPARE, strcmp);
    pfdBldrAttr(PFDBLDR_NODE_NAME_COMPARE, strcmp);
    pfdBldrAttr(PFDBLDR_EXTENSOR_NAME_COMPARE, strcmp);

    /* open ".3ds" file */
    if (fileName == NULL || *fileName == '\0')
	return NULL;
    if (!pfFindFile(fileName, filePath, R_OK))
	return NULL;
    if ((dbFile = OpenFile3ds(filePath, "rb")) == NULL)
	return NULL;

    /* Initializes the database structure */
    InitDatabase3ds(&db);

    /* at least one 3ds file has this problem ... */
    if (db == NULL)
	return NULL;

    /* Generate the database with the input 3DS file */
    CreateDatabase3ds(dbFile, db);

    /* allocate primitive buffer */
    facet = pfdNewGeom(3);

    /*
     * build material definitions
     */

    GetMaterialNameList3ds(db, &materials);
    for (i=0; i < materials->count; i++)
    {
	material3ds *material	= NULL;
	pfMaterial  *m		= NULL;
	char	     rgbFileName[256] = "\0";
        char	     alphaFileName[256] = "\0";
        char	     newFileName[256] = "\0";

	/* access next material */
	GetMaterialByName3ds(db, materials->list[i].name, &material);

	/* at least one 3ds file has this problem ... */
	if (material == NULL)
	    continue;

	/* 
	 * define a pfMaterial corresponding to 3ds material 
	 */

	m = (pfMaterial*)pfdGetTemplateObject(pfGetMtlClassType());

	pfMtlColor(m, PFMTL_AMBIENT,
	    material->ambient.r,
	    material->ambient.g,
	    material->ambient.b);
	pfMtlColor(m, PFMTL_DIFFUSE,
	    material->diffuse.r,
	    material->diffuse.g,
	    material->diffuse.b);
	pfMtlColor(m, PFMTL_SPECULAR,
	    material->specular.r,
	    material->specular.g,
	    material->specular.b);

	pfMtlShininess(m, material->shininess);
	pfMtlColorMode(m, PFMTL_FRONT, PFMTL_CMODE_OFF);
	pfdBldrStateAttr(PFSTATE_FRONTMTL, m);
	pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_ON);
	pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);

	/* 
	 * define a pfTexture corresponding to 3ds textures 
	 */

	/* is this textured geometry ? */
	if (strlen(material->texture.map.name) > 3)
	{
	    rgbFileName[0] = '\0';
	    if (strlen(material->texture.map.name) > 3)
		strcpy(rgbFileName, material->texture.map.name);
	    strlower(rgbFileName);

	    alphaFileName[0] = '\0';
	    if (strlen(material->opacity.map.name) > 3)
		strcpy(alphaFileName, material->opacity.map.name);
	    strlower(alphaFileName);

	    mergeTextures(rgbFileName, alphaFileName, newFileName);
	    setTexture(newFileName);

	    /* if material is black and textured, then make it white */
	    if (newFileName[0] != '\0' 
#ifdef ONLY_WHEN_BLACK
		&&
		material->ambient.r == 0 && material->ambient.g == 0 &&
		material->ambient.b == 0 && material->diffuse.r == 0 && 
		material->diffuse.g == 0 && material->diffuse.b == 0
#endif
		)
	    {
		pfMtlColor(m, PFMTL_AMBIENT,  0.2, 0.2, 0.2);
		pfMtlColor(m, PFMTL_DIFFUSE,  0.8, 0.8, 0.8);
		pfMtlColor(m, PFMTL_SPECULAR, 1.0, 1.0, 1.0);
		pfdBldrStateAttr(PFSTATE_FRONTMTL, m);
	    }
	}
	else
	{
	    /* disable texture mapping */
	    setTexture(NULL);
	}

	/* is this reflection mapped geometry ? */
	if (strlen(material->reflect.map.name) > 3)
	{
	    /* set reflection-map texture */
            strcpy(rgbFileName, material->reflect.map.name);
	    strlower(rgbFileName);

	    if (mergeTextures(rgbFileName, NULL, newFileName) == 0)
	    {
		setTexture(newFileName);
		pfdBldrStateMode(txgn, PFD_TEXGEN_REFLECTIVE);

		/* if material is black and textured, then make it white */
#ifdef ONLY_WHEN_BLACK
		if (material->ambient.r == 0 && material->ambient.g == 0 &&
		    material->ambient.b == 0 && material->diffuse.r == 0 && 
		    material->diffuse.g == 0 && material->diffuse.b == 0)
#endif
		{
		    pfMtlColor(m, PFMTL_AMBIENT,  0.2, 0.2, 0.2);
		    pfMtlColor(m, PFMTL_DIFFUSE,  0.8, 0.8, 0.8);
		    pfMtlColor(m, PFMTL_SPECULAR, 1.0, 1.0, 1.0);
		    pfdBldrStateAttr(PFSTATE_FRONTMTL, m);
		}
	    }
	    else
	    {
		setTexture(NULL);
		pfdBldrStateMode(txgn, PFD_TEXGEN_OFF);
	    }
	}
	else
	{
	    /* disable reflection mapping */
	    pfdBldrStateMode(txgn, PFD_TEXGEN_OFF);
	}

	/* remember state by name */
	pfdSaveBldrState(strdup(material->name));

	/* release material definition */
	ReleaseMaterial3ds(&material);
    }
    numMaterials = materials->count;

    /*
     * build geometry definitions
     */

    /* specify unchanging facet information */
    facet->numVerts = 3;
    facet->primtype = PFGS_POLYS;
    facet->nbind = PFGS_OFF;
    facet->cbind = PFGS_OFF;

    /* for each mesh in the file ... */
    GetMeshNameList3ds(db, &meshlist);

    /* at least one 3ds file has this problem ... */
    if (meshlist == NULL)
	return NULL;

    for (i=0; i < meshlist->count; i++)
    {
	/* access next mesh */
	GetMeshByName3ds(db, meshlist->list[i].name, &mesh);

#ifdef	BILLBOARD_SUPPORT
	/* send facets to the right builder */
        if (strncmp(meshlist->list[i].name, "Bboard", 6) == 0)
	    pfdNamePrims(billboard);
	else
	    pfdNamePrims(normal);
#endif

	/* for each material in the mesh ... */
	for (j = 0; j < mesh->nmats; j++)
	{
	    material3ds *material	= NULL;

	    /* use the named material definition */
	    pfdLoadBldrState(mesh->matarray[j].name);
	    GetMaterialByName3ds(db, mesh->matarray[j].name, &material);

	    if ((mesh->textarray) && 
		((material->texture.map.name) || (material->opacity.map.name)) )
            {
		t = 1;
		facet->tbind[0] = PFGS_PER_VERTEX;
	    } else 
		t = 0;

            for ( ; t < PF_MAX_TEXTURES ; t ++)
                   facet->tbind[t] = PFGS_OFF;

	    /* for each facet referencing this material ... */
	    for (k=0; k < mesh->matarray[j].nfaces; k++)
	    {
		/* f is the index of this facet */
		f = mesh->matarray[j].faceindex[k];

		/* set up facet vertex coordinates */
		pfSetVec3(facet->coords[0],
		    mesh->vertexarray[mesh->facearray[f].v1].x,
		    mesh->vertexarray[mesh->facearray[f].v1].y,
		    mesh->vertexarray[mesh->facearray[f].v1].z);
		pfSetVec3(facet->coords[1],
		    mesh->vertexarray[mesh->facearray[f].v2].x,
		    mesh->vertexarray[mesh->facearray[f].v2].y,
		    mesh->vertexarray[mesh->facearray[f].v2].z);
		pfSetVec3(facet->coords[2],
		    mesh->vertexarray[mesh->facearray[f].v3].x,
		    mesh->vertexarray[mesh->facearray[f].v3].y,
		    mesh->vertexarray[mesh->facearray[f].v3].z);

		/* set up facet texture coordinates */
		if ((mesh->textarray) &&
		    ((material->texture.map.name) ||
		     (material->opacity.map.name)))
	    	{
		    pfSetVec2(facet->texCoords[0][0],
			mesh->textarray[mesh->facearray[f].v1].u,
			mesh->textarray[mesh->facearray[f].v1].v);
		    pfSetVec2(facet->texCoords[0][1],
			mesh->textarray[mesh->facearray[f].v2].u,
			mesh->textarray[mesh->facearray[f].v2].v);
		    pfSetVec2(facet->texCoords[0][2],
			mesh->textarray[mesh->facearray[f].v3].u,
			mesh->textarray[mesh->facearray[f].v3].v);
         	}

		/* add facet to builder */
		pfdAddBldrGeom(facet, 1);
		numTris += facet->numVerts - 2;
	    }
	    ReleaseMaterial3ds(&material);
	}

#ifdef	BILLBOARD_SUPPORT
	/* extract facets associated with this billboard */
        if (strncmp(meshlist->list[i].name, "Bboard", 6) == 0)
	{
	    pfGeode	*geode = NULL;
	    pfBillboard	*bb = NULL;

	    geode = (pfGeode *)pfdBuildNode(billboard);
	    if (geode != NULL)
	    {
		int n, num;
		pfBox box;
		pfVec3 base;
		pfMatrix mat;
		pfSCS *scs;

		/* move geode to origin */
		pfGetGSetBBox(pfGetGSet(geode, 0), &box);
		pfSetVec3(base,
		    0.5*(box.min[PF_X] + box.max[PF_X]),
		    0.5*(box.min[PF_Y] + box.max[PF_Y]),
		         box.min[PF_Z]);
		pfMakeTransMat(mat, -base[PF_X], -base[PF_Y], -base[PF_Z]);
		scs = pfNewSCS(mat);
		pfAddChild(scs, geode);
		pfFlatten(scs, 0);
		pfRemoveChild(scs, geode);
		pfDelete(scs);

		/* make a new billboard node */
		bb = pfNewBboard();
		pfBboardPos(bb, 0, base);

		/* move geosets from "geom" to billboard */
		num = pfGetNumGSets(geode);
		for (n = 0; n < num; n++)
		{
		    pfGeoSet *gset = pfGetGSet(geode, 0);
		    pfRemoveGSet(geode, gset);

		    pfAddGSet(bb, gset);
		}
		pfDelete(geode);

		if (bbGroup == NULL)
		    bbGroup = pfNewGroup();

		pfAddChild(bbGroup, bb);
		++numBillboards;
	    }
	}
#endif

	/* release this mesh definition */
	RelMeshObj3ds(&mesh);
    }

    /* release database definition */
    ReleaseDatabase3ds(&db);

    /* close all open 3DStudio files */
#if 0
    CloseAllFiles3ds();
#endif

    /* release primitive buffer */
    pfdDelGeom(facet);

    /* get a complete scene graph representing file's polygons */
    node = pfdBuild();

#ifdef	BILLBOARD_SUPPORT
    if (bbGroup != NULL)
    {
	if (node != NULL)
	    pfAddChild(bbGroup, node);
	node = (pfNode *)bbGroup;
    }
#endif

    /* use indicated name for top-level pfNode */
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_3ds: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input materials:    %8ld", numMaterials);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input triangles:    %8ld", numTris);

#ifdef	BILLBOARD_SUPPORT
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input billboards:   %8ld", numBillboards);
#endif

    return node;
}

/*
 * convert string to lower case
 */
static char *
strlower(char *string)
{
    char *cp = string;
    while (*cp)
    {
	*cp = tolower(*cp);
	++cp;
    }
}

static void
setTexture(char *fileName)
{
    if (fileName == NULL || *fileName == '\0')
    {
	/* disable texturing */
	pfdBldrStateAttr(PFSTATE_TEXTURE, NULL);
	pfdBldrStateMode(PFSTATE_ENTEXTURE, PFTR_OFF);
    }
    else
    {
	/* get texture state template */
	pfTexture *tex;
	tex = (pfTexture *)pfdGetTemplateObject(pfGetTexClassType());

	/* specify the texture */
	pfTexName(tex, fileName);
	pfdBldrStateAttr(PFSTATE_TEXTURE, tex);
	pfdBldrStateMode(PFSTATE_ENTEXTURE, PFTR_ON);
    }
}
