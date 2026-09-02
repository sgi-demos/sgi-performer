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
 * pfbin.c: $Revision: 1.47 $ $Date: 2002/11/07 19:03:11 $
 */

#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

typedef struct
{
    int	magic;
    int	vertices;
    int	zero;
} binHeader;

typedef struct
{
    pfVec3	normal;
    pfVec3	vertex;
} binVertex;

static char * readFile(char *fileName, int *fileSize);

#ifndef P_32_SWAP
#define	P_32_SWAP(a) {							\
	uint _tmp = *(uint *)a;				\
	((char *)a)[0] = ((char *)&_tmp)[3];				\
	((char *)a)[1] = ((char *)&_tmp)[2];				\
	((char *)a)[2] = ((char *)&_tmp)[1];				\
	((char *)a)[3] = ((char *)&_tmp)[0];				\
}
#endif  /* P_32_SWAP */

void flipBytes32(char *buffer, int size)
{
	int i;
	char *p;

	for(i=0, p=buffer; i<size/4; i++, p+=4)
	{
		P_32_SWAP(p);
	}
}

/*
 * pfdLoadFile_bin -- Load an SGI ".bin" file into IRIS Performer
 */

PFPFB_DLLEXPORT pfNode *
pfdLoadFile_bin (char *fileName)
{
    int		 i, j, k;
    int	 	 numQuads;
    int	 	 numTris	= 0;
    binHeader	*header		= NULL;
    binVertex	*vertex		= NULL;
    pfNode	*node		= NULL;
    pfdGeom	*buffer;
    pfMaterial  *m      	= NULL;

    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;

    int		 t		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* load named file */
    if ((header = (binHeader *)readFile(fileName, NULL)) == NULL)
	return NULL;

    /* check file header for proper magic number */
    if (header->magic != 0x5423)
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "LoadBin: Invalid magic number in file \"%s\"", fileName);
	pfFree(header);
	return NULL;
    }

    /* determine number of independent quadrilaterals */
    if ((numQuads = header->vertices/4) < 1)
    {	
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "LoadBin: Invalid quadrilateral count in file \"%s\"", fileName);
	pfFree(header);
	return NULL;
    }

    /* create a primitive buffer big enough to hold a quad */
    buffer = pfdNewGeom(4);

    /* pick a random not-too-dark color */
    pfuRandomize(*(int *)&startTime);
    pfuRandomColor(buffer->colors[0], 0.2f, 0.7f);

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

    /* accumulate file's geometry in active primitive builder */
    vertex = (binVertex *)(header + 1);

    /* build polygon definition */
    buffer->primtype = PFGS_POLYS;
    buffer->cbind = PFGS_PER_PRIM;
    buffer->nbind = PFGS_PER_VERTEX;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
        buffer->tbind[t] = PFGS_OFF;

    for (i = 0; i < numQuads; i++)
    {
	/* copy triangle/quad definition */
	for (j = 0, k = 0; j < 4; j++)
	{
	    pfCopyVec3(buffer->coords[k], vertex[4*i + j].vertex);
	    pfCopyVec3(buffer->norms[k],  vertex[4*i + j].normal);

	    /* drop duplicate vertices */
	    if (k == 0 || !pfEqualVec3(buffer->coords[k], buffer->coords[k-1]))
		++k;
	}

	/* add triangle/quad to triangle set */
	if (k >= 3)
	{
	    buffer->numVerts = k;
	    pfdAddBldrGeom(buffer, 1);
	    numTris += buffer->numVerts - 2;
	}
    }

    /* delete primitive buffer now that all polygons have been added */
    pfdDelGeom(buffer);

    /* free ".bin" file in-memory buffer */
    pfFree(header);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_bin: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Raw quads:              %ld", numQuads);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Triangles:              %ld", numTris);

    /* return pointer to in-memory scene-graph */
    return node;
}

/*
 * readFile -- load named file into memory
 */

static char*
readFile (char *fileName, int *fileSize)
{
    int	 	 fd;
    struct stat	 status;
    char	*buffer;
    char	 filePath[PF_MAXSTRING];

    /* check file name */
    if (fileName == NULL || *fileName == '\0')
	return NULL;

    /* find file in IRIS Performer directory-search path */
    if (!pfFindFile(fileName, filePath, R_OK))
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "readFile: Could not find \"%s\" in PFPATH", fileName);
	return NULL;
    }

    /* open the indicated file */
    if ((fd = open(filePath, O_RDONLY)) == -1)
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "readFile: Could not open file \"%s\"", filePath);
	return NULL;
    }

    /* determine the file's fileSize */
    fstat(fd, &status);
    if (status.st_size == 0)
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "readFile: File \"%s\" is empty", filePath);
	close(fd);
	return NULL;
    }

    /* allocate storage for entire file */
    buffer = (char *)pfMalloc((unsigned)status.st_size, NULL);

    /* read the entire file into memory */
    if (read(fd, buffer, (unsigned)status.st_size) != status.st_size)
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "readFile: Could not read file \"%s\"", filePath);
	close(fd);
	pfFree(buffer);
	return NULL;
    }

#ifdef __LITTLE_ENDIAN
	flipBytes32(buffer, status.st_size);
#endif

    /* close the file */
    close(fd);

    /* bind buffer size to caller's variable */
    if (fileSize != NULL)
	*fileSize = status.st_size;

    /* return buffer address to caller */
    return buffer;
}
