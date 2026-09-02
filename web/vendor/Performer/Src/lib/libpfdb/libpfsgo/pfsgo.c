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
 * pfsgo.c: $Revision: 1.36 $ $Date: 2002/11/07 18:06:03 $
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef WIN32
#include <windows.h>
#include <limits.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#ifdef __linux__
#include <asm/byteorder.h>
#endif

typedef struct
{
    int	magic;
} sgoHeader;

typedef struct
{
    int	token;
    int	words;
} sgoData;

#define	OBJ_QUADLIST	1
#define	OBJ_TRILIST	2
#define	OBJ_TRIMESH	3
#define	OBJ_END		4

#define	OP_BGNTMESH	1
#define	OP_SWAPTMESH	2
#define	OP_ENDBGNTMESH	3
#define	OP_ENDTMESH	4

static void* readFile(char *file, int *size);

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
 * pfdLoadFile_sgo -- Load an SGI ".sgo" file into IRIS Performer
 */

PFPFB_DLLEXPORT pfNode *
pfdLoadFile_sgo (char *fileName)
{
    int		 i;
    int		 q;
    int		 t;
    int		 v;
    int		 count;
    int		 numQuads	= 0;
    int		 numTris	= 0;
    sgoHeader	*header		= NULL;
    sgoData	*data		= NULL;
    pfVec3	*array		= NULL;
    pfdGeom	*buffer		= NULL;
    pfNode	*node		= NULL;
    pfMaterial  *m      	= NULL;
    int		k		= 0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* load named file */
    if ((header = (sgoHeader *)readFile(fileName, NULL)) == NULL)
	return NULL;

    /* check file header for proper magic number */
    if (header->magic != 0x5424)
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "pfdLoadFile_sgo: Invalid SGO magic number in file \"%s\"", fileName);
	pfFree(header);
	return NULL;
    }

    /* create an (initially empty) triangle set */
    buffer = pfdNewGeom(4096);

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

    /* find start of SGO file's data section */
    data = (sgoData *)(header + 1);

    /* extract each polygon definition from SGO format */
    while (data->token != OBJ_END)
    {
	switch (data->token)
	{
	case OBJ_QUADLIST:
	    count = data->words/36;
	    array = (pfVec3 *)(data + 1);
	    data  = (sgoData *)(array + 4*count*3);

	    /* extract each quad */
	    for (q = 0; q < count; q++)
	    {
		/* specify attribute bindings */
		buffer->numVerts = 4;
		buffer->primtype = PFGS_POLYS;
		buffer->cbind = PFGS_PER_VERTEX;
		buffer->nbind = PFGS_PER_VERTEX;
		for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
		    buffer->tbind[k] = PFGS_OFF;

		/* build quad */
		for (v = 0; v < 4; v++)
		{
		    /* copy quad definition to builder's polygon */
		    pfCopyVec3(buffer->norms [v], array[3*(4*q+v)    ]);
		    pfCopyVec3(buffer->colors[v], array[3*(4*q+v) + 1]);
		    pfCopyVec3(buffer->coords[v], array[3*(4*q+v) + 2]);

		    /* assume all polygons are opaque */
		    buffer->colors[v][3] = 1.0f;
		}

		/* add quad to triangle set */
		pfdAddBldrGeom(buffer, 1);
	    }

	    numQuads += count;
	    break;

	case OBJ_TRILIST:
	    count = data->words/27;
	    array = (pfVec3 *)(data + 1);
	    data  = (sgoData *)(array + 3*count*3);

	    /* extract each triangle */
	    for (t = 0; t < count; t++)
	    {
		/* specify attribute bindings */
		buffer->numVerts = 3;
		buffer->primtype = PFGS_POLYS;
		buffer->cbind = PFGS_PER_VERTEX;
		buffer->nbind = PFGS_PER_VERTEX;
		for (k = 0 ; k < PF_MAX_TEXTURES ; k ++)
		    buffer->tbind[k] = PFGS_OFF;

		/* build triangle */
		for (v = 0; v < 3; v++)
		{
		    /* copy quad definition to builder's polygon */
		    pfCopyVec3(buffer->norms [v], array[3*(3*t+v)    ]);
		    pfCopyVec3(buffer->colors[v], array[3*(3*t+v) + 1]);
		    pfCopyVec3(buffer->coords[v], array[3*(3*t+v) + 2]);

		    /* assume all polygons are opaque */
		    buffer->colors[v][3] = 1.0f;
		}

		/* add quad to triangle set */
		pfdAddBldrGeom(buffer, 1);
	    }

	    numTris += count;
	    break;

	case OBJ_TRIMESH:
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		"pfdLoadFile_sgo: OBJ_TRIMESH token encountered");
	    data->token = OBJ_END;
	    break;
	
	default:
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		"pfdLoadFile_sgo: Invalid object token encountered");
	    data->token = OBJ_END;
	    break;
	}
    }

    /* free ".sgo" file in-memory buffer */
    pfFree(header);
    pfdDelGeom(buffer);

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_sgo: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    if (numTris != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  
	    "    Input triangles:    %8ld", numTris);
    if (numQuads != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  
	    "    Input quads:        %8ld", numQuads);

    /* return address of hierarchy to caller */
    return node;
}

/*
 * readFile -- read named file into memory
 */

static void*
readFile (char *fileName, int *fileSize)
{
    int		 fd;
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
	    "pfdLoadFile_sgo: Could not find \"%s\" in PFPATH", fileName);
	return NULL;
    }

    /* open the indicated file */
    if ((fd = open(filePath, O_RDONLY)) == -1)
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "pfdLoadFile_sgo: Could not open file \"%s\"", filePath);
	return NULL;
    }

    /* determine the file's fileSize */
    fstat(fd, &status);
    if (status.st_size == 0)
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "pfdLoadFile_sgo: File \"%s\" is empty", filePath);
	close(fd);
	return NULL;
    }

    /* allocate storage for entire file */
    buffer = (char *)pfMalloc((unsigned)status.st_size, NULL);

    /* read the entire file into memory */
    if (read(fd, buffer, (unsigned)status.st_size) != status.st_size)
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "pfdLoadFile_sgo: Could not read file \"%s\"", filePath);
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
	*fileSize = (unsigned)status.st_size;

    /* return buffer address to caller */
    return buffer;
}
