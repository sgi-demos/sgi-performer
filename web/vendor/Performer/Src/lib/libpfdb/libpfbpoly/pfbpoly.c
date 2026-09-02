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
 * pfbpoly.c: $Revision: 1.18 $ $Date: 2002/11/07 19:03:11 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#include <windows.h>
#include <limits.h>

typedef unsigned int uint;
typedef unsigned short ushort;

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#ifdef __linux__
#include <asm/byteorder.h>
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


#define OLD_BPOLY_MAGIC	0x62706F6C      /* "bpol" */
#define NEW_BPOLY_MAGIC	0x62706F32      /* "bpo2" */

#define VERTEX_COLOR	0x1
#define VERTEX_TEXTURE	0x4
#define VERTEX_NORMAL	0x8
#define POLYGON_COLOR	0x2

typedef struct polyVertex 
{
    pfVec3	xyz;
    pfVec4	rgba;
    pfVec3	norm;
    pfVec2	st;
} polyVertex;

/*
 * pfdLoadFile_bpoly -- Load Prisims ".bpoly" files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_bpoly (char *fileName)
{
    FILE	*bpolyFIle	= NULL;
    pfdGeom	*geom		= NULL;
    pfNode	*node		= NULL;
    polyVertex	*v		= NULL; 
    int		 geomSize	= 256;
    int		 i		= 0;
    int		 j		= 0;
    int		 magic		= 0;
    int		 bpolyPoints	= 0;
    int		 bpolyPolys	= 0;
    int		 numTris	= 0;
    short	 flag		= 0;
    pfMaterial  *m      	= NULL;
    double	 startTime	= pfGetTime();
    int		 t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open Prisims ".bpoly" file file */
    if ((bpolyFIle = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* allocate a new primitive geom */
    geom = pfdNewGeom(geomSize);

#define	RANDOM_COLOR
#ifdef	RANDOM_COLOR
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
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_ON);

    /* read "header" definition from ".bpoly" file */
    if (fread(&magic, sizeof(magic), 1, bpolyFIle) != 1)
    {
	fclose(bpolyFIle);
	return NULL;
    }

    /* magic number determines size of vertex indices */
    if (magic != OLD_BPOLY_MAGIC && magic != NEW_BPOLY_MAGIC)
    {
	pfdDelGeom(geom);
	fclose(bpolyFIle);
	return NULL;
    }

    /* read counts from file */
    if (magic == OLD_BPOLY_MAGIC)
    {
	short	shortPoints	= 0;
	short	shortPolys	= 0;

	if ((fread(&shortPoints, sizeof(shortPoints), 1, bpolyFIle) != 1) ||
	    (fread(&shortPolys,  sizeof(shortPolys ), 1, bpolyFIle) != 1))
	{
	    pfdDelGeom(geom);
	    fclose(bpolyFIle);
	    return NULL;
	}

	bpolyPoints = shortPoints;
	bpolyPolys  = shortPolys;
    }
    else
    {
	if ((fread(&bpolyPoints, sizeof(bpolyPoints), 1, bpolyFIle) != 1) ||
	    (fread(&bpolyPolys,  sizeof(bpolyPolys ), 1, bpolyFIle) != 1))
	{
	    pfdDelGeom(geom);
	    fclose(bpolyFIle);
	    return NULL;
	}
    }

    /* read vertex-type flag */
    if (fread(&flag, sizeof(flag), 1, bpolyFIle) != 1)
    {
	pfdDelGeom(geom);
	fclose(bpolyFIle);
	return NULL;
    }

    /* read "POINTS" definition from ".bpoly" file */
    v = (polyVertex *)pfMalloc(bpolyPoints*sizeof(polyVertex), NULL);

    for (i = 0; i < bpolyPoints; i++)
    {
	if ((                           fread(v[i].xyz,  sizeof(float), 3, bpolyFIle) != 3) ||
	    ((flag & VERTEX_COLOR  ) && fread(v[i].rgba, sizeof(float), 3, bpolyFIle) != 3) ||
	    ((flag & VERTEX_TEXTURE) && fread(v[i].st,   sizeof(float), 2, bpolyFIle) != 2) ||
	    ((flag & VERTEX_NORMAL ) && fread(v[i].norm, sizeof(float), 3, bpolyFIle) != 3))
	{
	    pfFree(v);
	    pfdDelGeom(geom);
	    fclose(bpolyFIle);
	    return NULL;
	}

	/* set alpha-component to fully opaque */
	v[i].rgba[3] = 1.0f;

#ifdef	DEBUG
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,  "vertex %d", i);
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE,  "   xyz: %g %g %g", 
		v[i].xyz[0],  
		v[i].xyz[1],  
		v[i].xyz[2]);
	if (flag & VERTEX_COLOR)
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE,  "  rgba: %g %g %g %g", 
		v[i].rgba[0], 
		v[i].rgba[1], 
		v[i].rgba[2], 
		v[i].rgba[3]);
	if (flag & VERTEX_NORMAL)
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE,  "  norm: %g %g %g", 
		v[i].norm[0], 
		v[i].norm[1], 
		v[i].norm[2]);
	if (flag & VERTEX_TEXTURE)
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE,  "    st: %g %g", 
		v[i].st[0], 
		v[i].st[1]);
#endif
    }

    /* read "POLYS" definition from ".bpoly" file */
    for (j = 0; j < bpolyPolys; j++)
    {
	char	closed	= 0;
	int	vCount	= 0;
	pfVec4	rgba;

	if (fread(&closed, sizeof(closed), 1, bpolyFIle) != 1) 
	{
	    pfFree(v);
	    pfdDelGeom(geom);
	    fclose(bpolyFIle);
	    return NULL;
	}

	/* color specified at polygon level */
	if (flag & POLYGON_COLOR) 
	{
	    if (fread(rgba, sizeof(float), 3, bpolyFIle) != 3)
	    {
		pfFree(v);
		pfdDelGeom(geom);
		fclose(bpolyFIle);
		return NULL;
	    }

	    /* set alpha-component to fully opaque */
	    rgba[3] = 1.0f;
	}

	/* number of vertices in polygon */
	if (magic == OLD_BPOLY_MAGIC)
	{
	    short	shortCount	= 0;

	    if (fread(&shortCount, sizeof(shortCount), 1, bpolyFIle) != 1)
	    {
		pfFree(v);
		pfdDelGeom(geom);
		fclose(bpolyFIle);
		return NULL;
	    }

	    vCount = shortCount;
	}
	else
	{
	    if (fread(&vCount, sizeof(vCount), 1, bpolyFIle) != 1)
	    {
		pfFree(v);
		pfdDelGeom(geom);
		fclose(bpolyFIle);
		return NULL;
	    }
	}

	/* resize geometry geom if necessary */
	if (geomSize < vCount)
	    pfdResizeGeom(geom, geomSize = vCount);

	/* read vertex indices from file */
	for (i = 0; i < vCount; i++)
	{
	    int		index	= 0;

	    if (magic == OLD_BPOLY_MAGIC)
	    {
		unsigned short	shortIndex	= 0;

		if (fread(&shortIndex, sizeof(shortIndex), 1, bpolyFIle) != 1)
		{
		    pfFree(v);
		    pfdDelGeom(geom);
		    pfdResetBldrGeometry();
		    fclose(bpolyFIle);
		    return NULL;
		}

		index = shortIndex;
	    }
	    else
	    {
		if (fread(&index, sizeof(index), 1, bpolyFIle) != 1)
		{
		    pfFree(v);
		    pfdDelGeom(geom);
		    pfdResetBldrGeometry();
		    fclose(bpolyFIle);
		    return NULL;
		}
	    }

	    /* index is 1-based in file, 0-based in array */
	    --index;

	    if (index < 0 || index >= bpolyPoints)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		    "pfdLoadFile_bpoly: index %d out of range [0 .. %d]", 
		    index, bpolyPoints);
		pfFree(v);
		pfdDelGeom(geom);
		pfdResetBldrGeometry();
		fclose(bpolyFIle);
		return NULL;
	    }

	    /* copy valid vertex attributes to builder */
	    pfCopyVec3(geom->coords[i], v[index].xyz);

	    if (flag & VERTEX_COLOR)
		pfCopyVec4(geom->colors[i], v[index].rgba);

	    if (flag & VERTEX_NORMAL)
		pfCopyVec3(geom->norms[i], v[index].norm);

	    if (flag & VERTEX_TEXTURE)
		pfCopyVec2(geom->texCoords[i], v[index].st);
	}

	/* add polygon to builder */
	if (i >= 3)
	{
	    geom->numVerts = i;
	    geom->primtype = PFGS_POLYS;
	    geom->cbind = (flag & VERTEX_COLOR)   ? PFGS_PER_VERTEX : 
#ifdef  RANDOM_COLOR
                                                    PFGS_PER_PRIM;
#else
			  (flag & POLYGON_COLOR)  ? PFGS_PER_PRIM   : 
			                            PFGS_OFF;
#endif
	    geom->nbind = (flag & VERTEX_NORMAL)  ? PFGS_PER_VERTEX : PFGS_OFF;
	    geom->tbind[0] = (flag & VERTEX_TEXTURE) ? PFGS_PER_VERTEX : PFGS_OFF;
	    for (t = 1 ; t < PF_MAX_TEXTURES ; t ++)
		geom->tbind[t] = PFGS_OFF;

#ifdef  POLYGONS_ARE_IN_CW_ORDER
	    /* poly files seem to have CW polygon orientation so reverse */
	    pfdReverseGeom(buffer);
#endif

	    pfdAddBldrGeom(geom, 1);
	    numTris += geom->numVerts - 2;
	}
    }

    /* close ".bpoly" file */
    fclose(bpolyFIle);

    /* release dynamically allocated vertex data */
    pfFree(v);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* release memory and reset builder */
    pfdDelGeom(geom);
    pfdResetBldrGeometry();

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_bpoly: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input vertices:     %8ld", 
	bpolyPoints);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input polygons:     %8ld", 
	bpolyPolys);

    /* return pointer to in-memory scene-graph */
    return node;
}
