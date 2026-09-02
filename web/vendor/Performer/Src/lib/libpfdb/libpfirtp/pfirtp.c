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
 * pfirtp.c: $Revision: 1.38 $ $Date: 2002/11/07 04:02:08 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#include <windows.h>
//#include "wintypes.h"

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#ifdef	_POSIX_SOURCE
extern int strcasecmp (const char *s1, const char *s2);
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>

/* string comparison that avoids function call when possible */
#ifdef WIN32
#define SAME(_a, _b)	(_stricmp(_a, _b) == 0)
#else
#define SAME(_a, _b)	(strcasecmp(_a, _b) == 0)
#endif

/* define program limits and sizes */
#define	BUFFER_SIZE	4096
#define	MAX_STRING	4096

/*
 * pfdLoadFile_irtp -- Load Graphicon-2000 ".irtp" files into IRIS Performer
 */

PFPFB_DLLEXPORT extern pfNode * 
pfdLoadFile_irtp (char *fileName)
{
    FILE	*irtpFile	= NULL;
    pfNode	*node		= NULL;
    pfdGeom	*buffer		= NULL;
    char	 input[MAX_STRING];
    char	 command[MAX_STRING];
    int		 numTris	= 0;
    int		 numSkip	= 0;
    int		 numOther	= 0;
    float	 pr		= 0.2f;
    float	 pg		= 0.4f;
    float	 pb		= 0.8f;
    pfMaterial  *m      	= NULL;
    int		 k 		= 0;
    int		 t		= 0;

    /*
     *	PREPARE FOR CONVERSION
     */

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* find file in IRIS Performer directory-search path */
    if ((irtpFile = pfdOpenFile(fileName)) == NULL)
        return NULL;

    /* initialize utility library triangle/geoset builder */
    buffer = pfdNewGeom(1024);

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
     *	READ THROUGH INPUT FILE
     */

    /* parse each IRTP command in file */
    while (fgets(input, MAX_STRING, irtpFile) != NULL)
    {	
	char 	*comment;

	/* strip comments from irtpFile */
	comment = strchr(input, '#');
	if (comment != NULL)
	    *comment = '\0';

	/* skip blank lines */
	if (sscanf(input, "%s", command) != 1)
	    continue;

	/* identify command */
	if (SAME(command, "bsphere"))
	    ++numSkip;
	else
	if (SAME(command, "closestruct"))
	    ++numSkip;
	else
	if (SAME(command, "exestr"))
	    ++numSkip;
	else
	if (SAME(command, "intclr"))
	{
	    /* read primary color */
	    sscanf(input, "%*s %*d %f %f %f", &pr, &pg, &pb);
	}
	else
	if (SAME(command, "intclrs"))
	{
	    /* skip base color */
	    fgets(input, MAX_STRING, irtpFile);

	    /* read primary color */
	    fgets(input, MAX_STRING, irtpFile);
	    sscanf(input, "%f %f %f", &pr, &pg, &pb);

	    /* skip secondary color */
	    fgets(input, MAX_STRING, irtpFile);
	}
	else
	if (SAME(command, "intrefl"))
	    ++numSkip;
	else
	if (SAME(command, "intshd"))
	    ++numSkip;
	else
	if (SAME(command, "objectclass"))
	    ++numSkip;
	else
	if (SAME(command, "openphigs"))
	    ++numSkip;
	else
	if (SAME(command, "openstruct"))
	    ++numSkip;
	else
	if (SAME(command, "pgn"))
	{
	    int		count = 0;
	    float	 x,  y,  z;
	    float	nx, ny, nz;

	    /* read supplied normal */
	    sscanf(input, "%*s %f %f %f", &nx, &ny, &nz);

	    /* read verticies */
	    while (fgets(input, MAX_STRING, irtpFile) != NULL &&
		   sscanf(input, "%f %f %f", &x, &y, &z) == 3)
	    {
		buffer->coords[count][PF_X] = x;
		buffer->coords[count][PF_Y] = y;
		buffer->coords[count][PF_Z] = z;
		++count;
	    }

	    /* generate triangles */
	    buffer->numVerts = count;
	    buffer->primtype = PFGS_POLYS;
	    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
		buffer->tbind[t] = PFGS_OFF;
	    buffer->nbind = PFGS_OFF;
	    buffer->cbind = PFGS_PER_PRIM;
	    pfSetVec4(buffer->colors[0], pr, pg, pb, 1.0);
	    pfdAddBldrGeom(buffer, 1);
	    numTris += buffer->numVerts - 2;
	}
	else
	if (SAME(command, "pgnn"))
	{
	    int		count = 0;
	    float	 x,  y,  z;
	    float	nx, ny, nz;

	    /* read verticies */
	    while (fgets(input, MAX_STRING, irtpFile) != NULL &&
		   sscanf(input, "%f %f %f %f %f %f", 
		       &x, &y, &z, &nx, &ny, &nz) == 6)
	    {
		buffer->coords[count][PF_X] = x;
		buffer->coords[count][PF_Y] = y;
		buffer->coords[count][PF_Z] = z;

		buffer->norms[count][PF_X] = nx;
		buffer->norms[count][PF_Y] = ny;
		buffer->norms[count][PF_Z] = nz;
		++count;
	    }

	    /* generate triangles */
	    buffer->numVerts = count;
	    buffer->primtype = PFGS_POLYS;
	    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
		buffer->tbind[t] = PFGS_OFF;
	    buffer->nbind = PFGS_PER_VERTEX;
	    buffer->cbind = PFGS_PER_PRIM;
	    pfSetVec4(buffer->colors[0], pr, pg, pb, 1.0);
	    pfdAddBldrGeom(buffer, 1);
	    numTris += buffer->numVerts - 2;
	}
	else
	if (SAME(command, "pgnnt"))
	    {
	    int		count = 0;
	    float	 x,  y,  z;
	    float	nx, ny, nz;
	    float    	 s,  t;

	    /* read verticies */
	    while (fgets(input, MAX_STRING, irtpFile) != NULL &&
		sscanf(input, "%f %f %f %f %f %f %f %f", 
		    &x, &y, &z, &nx, &ny, &nz, &s, &t) == 8)
	    {
		buffer->coords[count][PF_X] = x;
		buffer->coords[count][PF_Y] = y;
		buffer->coords[count][PF_Z] = z;
		
		buffer->norms[count][PF_X] = nx;
		buffer->norms[count][PF_Y] = ny;
		buffer->norms[count][PF_Z] = nz;
		
		buffer->texCoords[0][count][0] = s;
		buffer->texCoords[0][count][1] = t;
		
		++count;
	    }

	    /* generate triangles */
	    buffer->numVerts = count;
	    buffer->primtype = PFGS_POLYS;
	    buffer->tbind[0] = PFGS_PER_VERTEX;
	    for (k = 1 ; k < PF_MAX_TEXTURES ; k ++)
		buffer->tbind[k] = PFGS_OFF;
	    buffer->nbind = PFGS_PER_VERTEX;
	    buffer->cbind = PFGS_PER_PRIM;
	    pfSetVec4(buffer->colors[0], pr, pg, pb, 1.0);
	    pfdAddBldrGeom(buffer, 1);
	    numTris += buffer->numVerts - 2;
	}
	else
	if (SAME(command, "pgnt"))
	{
	    int		count = 0;
	    float	 x,  y,  z;
	    float	nx, ny, nz;
	    float    	 s,  t;

	    /* read supplied normal */
	    sscanf(input, "%*s %f %f %f", &nx, &ny, &nz);

	    /* read verticies */
	    while (fgets(input, MAX_STRING, irtpFile) != NULL &&
		   sscanf(input, "%f %f %f %f %f", 
			  &x, &y, &z, &s, &t) == 5)
	    {
		buffer->coords[count][PF_X] = x;
		buffer->coords[count][PF_Y] = y;
		buffer->coords[count][PF_Z] = z;
		
		buffer->texCoords[0][count][0] = s;
		buffer->texCoords[0][count][1] = t;
		
		++count;
	    }

	    /* generate triangles */
	    buffer->numVerts = count;
	    buffer->primtype = PFGS_POLYS;
	    buffer->tbind[0] = PFGS_PER_VERTEX;
	    for (k = 1 ; k < PF_MAX_TEXTURES ; k ++)
		buffer->tbind[k] = PFGS_OFF;
	    buffer->nbind = PFGS_OFF;
	    buffer->cbind = PFGS_PER_PRIM;
	    pfSetVec4(buffer->colors[0], pr, pg, pb, 1.0);
	    pfdAddBldrGeom(buffer, 1);
	    numTris += buffer->numVerts - 2;
	}
	else
	if (SAME(command, "sepplaneexestr"))
	{
	    /* skip plane equation */
	    fgets(input, MAX_STRING, irtpFile);
	    ++numSkip;
	}
	else
	if (SAME(command, "sepplanegridexestr"))
	{
	    int child;

	    /* skip child nodes */
	    while (fgets(input, MAX_STRING, irtpFile) != NULL &&
		sscanf(input, "%d", &child) == 1)
		{} /* ignore */
	    ++numSkip;
	}
	else
	if (SAME(command, "texmapind"))
	    ++numSkip;
	else
	if (SAME(command, "texmtxindirect"))
	    ++numSkip;
	else
	if (SAME(command, "transcoef"))
	    ++numSkip;
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_irtp: unrecognized: %s", input);
	    ++numOther;
	}
    }

    /* close file */
    fclose(irtpFile);
    pfdDelGeom(buffer);

    /*
     *	GENERATE IRIS PERFORMER DATA STRUCTURES
     */

    /* get a complete scene graph representing file's primitives */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_irtp: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Skipped tokens:     %8ld", numSkip);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Unknown tokens:     %8ld", numOther);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input triangles:    %8ld", numTris);

    /* return pointer to in-memory scene-graph */
    return node;
}
