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
 * pfdxf.c: $Revision: 1.48 $ $Date: 2002/11/17 01:40:32 $
 */

#include <Performer/pf.h>
#include <Performer/pfdu.h>

/*
 * This AutoCAD reader was created almost verbatim from the
 * AutoCAD-DXF-file to DKB-data-file converter written and
 * placed in the public domain 8/13/90 by Aaron A. Collins.
 *
 * It parses a limited, but useful, subset of the AutoCAD
 * DXF file format. No effort has been made to handle the
 * complete range of possible DXF opcodes and commands.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFDXF_DLLEXPORT __declspec(dllexport)
#else
#define PFDXF_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFDXF_DLLEXPORT
#endif

#define BUFFER_SIZE	2048

static int groupcode;
static char linbuf[BUFFER_SIZE];
static char curobj[80];
static int curcolor;
static float curr;
static float curg;
static float curb;
static float xcoords[BUFFER_SIZE];
static float ycoords[BUFFER_SIZE];
static float zcoords[BUFFER_SIZE];
static float floats[10];
static float angles[10];
static int ints[10];
static int numTris;

static pfdGeom *buffer = NULL;

/* function type and argument declarations */
static pfGeoState* defaultGeoState(void);
static int getLine(FILE *fp);
static void addGeometry(void);
static void getColor(int ndx, float *red, float *green, float *blue);
static int findEntity(FILE *fp);
static int readEntity(FILE *fp);

/*
 * pfdLoadFile_dxf -- load AutoDesk ".dxf" files into IRIS Performer
 */

PFDXF_DLLEXPORT /*extern */pfNode *
pfdLoadFile_dxf (char *fileName)
{
    FILE	*dxfFile	= NULL;
    pfNode	*node		= NULL;
    pfMaterial  *m      	= NULL;

    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* open DXF file file */
    if ((dxfFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* create a primitive buffer */
    buffer = pfdNewGeom(2048);

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

    /* initialize current DXF object state */
    numTris = 0;
    curobj[0] = '\0';
    curcolor = 7;
    getColor(curcolor, &curr, &curg, &curb);

    /* read DXF file */
    while (findEntity(dxfFile))
	readEntity(dxfFile);

    /* close DXF file */
    fclose(dxfFile);

    /* release primitive buffer */
    pfdDelGeom(buffer);

    /* signal failure if no geometry loaded */
    if (numTris == 0)
	return NULL;

    /* construct geode */
    node = pfdBuild();
    if (node != NULL)
	pfNodeName(node, fileName);

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_dxf: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Input triangles:    %8ld", numTris);

    /* return pointer to in-memory scene-graph */
    return node;
}


/*
 * read a group code and the next line from dxfFile
 */
static int
getLine(FILE *fp)
{
    /* get a line from DXF file */
    fgets(linbuf, BUFFER_SIZE, fp);
    if (feof(fp))
	return 1;

    /* scan out group code */
    sscanf(linbuf, "%3d", &groupcode);

    /* get a line from DXF file */
    fgets(linbuf, BUFFER_SIZE, fp);
    if (feof(fp))
	return 1;

    return 0;
}

/*
 * dump out current object
 */
static void
addGeometry(void)
{
    static int state=0;
    static int nb_points;
    static int nb_cotes;
    static int indice_point;
    int v;
    int	t;

    /* 2 back-to-back triangles */
    if (strstr(curobj, "TRACE"))
    {
	buffer->numVerts = 4;
	buffer->primtype = PFGS_POLYS;
	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    buffer->tbind[t] = PFGS_OFF;
	buffer->nbind = PFGS_OFF;
	buffer->cbind = PFGS_PER_PRIM;
	for (v = 0; v < 4; v++)
	    pfSetVec3(buffer->coords[v], xcoords[v], ycoords[v], zcoords[v]);
	pfSetVec3(buffer->colors[0], curr, curg, curb);
	pfdAddBldrGeom(buffer, 1);
	numTris += buffer->numVerts - 2;
	return;
    }
    /* 1 or 2 triangles */
    else if (strstr(curobj, "SOLID"))
    {
	buffer->numVerts = 4;
	buffer->primtype = PFGS_POLYS;
	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    buffer->tbind[t] = PFGS_OFF;
	buffer->nbind = PFGS_OFF;
	buffer->cbind = PFGS_PER_PRIM;
	for (v = 0; v < 4; v++)
	    pfSetVec3(buffer->coords[v], xcoords[v], ycoords[v], zcoords[v]);
	pfSetVec3(buffer->colors[0], curr, curg, curb);
	pfdAddBldrGeom(buffer, 1);
	numTris += buffer->numVerts - 2;
	return;
    }
    /* 1 or 2 triangles */
    else if (strstr(curobj, "3DFACE"))
    {
	buffer->numVerts = 4;
	buffer->primtype = PFGS_POLYS;
	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    buffer->tbind[t] = PFGS_OFF;
	buffer->nbind = PFGS_OFF;
	buffer->cbind = PFGS_PER_PRIM;
	for (v = 0; v < 4; v++)
	    pfSetVec3(buffer->coords[v], xcoords[v], ycoords[v], zcoords[v]);
	pfSetVec3(buffer->colors[0], curr, curg, curb);
	pfdAddBldrGeom(buffer, 1);
	numTris += buffer->numVerts - 2;
	return;
    }
    else if (strstr(curobj, "SEQEND"))
    {
	if (state == 1 && buffer->numVerts > 2)
	{
	    pfSetVec3(buffer->colors[0], curr, curg, curb);
	    pfdAddBldrGeom(buffer, 1);
	    numTris += buffer->numVerts - 2;
	}
	state = 0;	/* end of something */
    }
    else if (strstr(curobj, "POLYLINE"))
    {
	state = 1;	/* begin of a polyline */
	indice_point = 1;
	nb_points = ints[1];
	nb_cotes = ints[2];

	if (nb_points >= (BUFFER_SIZE-1))
	{
	    state = 0;
	    return;
	}

	buffer->numVerts = 0;
	buffer->primtype = PFGS_POLYS;
	for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	    buffer->tbind[t] = PFGS_OFF;
	buffer->nbind = PFGS_OFF;
	buffer->cbind = PFGS_PER_PRIM;
    }
    /* a new vertex in a polyline */
    else if (strstr(curobj, "VERTEX") && state == 1)
    {
	/* is it a new vertex ? */
	if (indice_point <= nb_points)
	{
	    /* copying point */
	    xcoords[indice_point] = xcoords[0];
	    ycoords[indice_point] = ycoords[0];
	    zcoords[indice_point] = zcoords[0];
	    indice_point++;
	} 
	else
	/* creating edges */
	if (nb_cotes > 0)
	{
	    /* closing an older polygon */
	    if ((ints[1] > 0) && (buffer->numVerts > 2))
	    {
		pfSetVec3(buffer->colors[0], curr, curg, curb);
		pfdAddBldrGeom(buffer, 1);
		numTris += buffer->numVerts - 2;
	    }

	    /* creating a new polygon */
	    if (ints[1] > 0)
	    {
		buffer->numVerts = 0;
		pfSetVec3(buffer->coords[buffer->numVerts++],
		    xcoords[ints[1]], ycoords[ints[1]], zcoords[ints[1]]);
	    }
	    if (ints[2] > 0)
	    {
		pfSetVec3(buffer->coords[buffer->numVerts++],
		    xcoords[ints[2]], ycoords[ints[2]], zcoords[ints[2]]);
	    }
	    if (ints[3] > 0)
	    {
		pfSetVec3(buffer->coords[buffer->numVerts++],
		    xcoords[ints[3]], ycoords[ints[3]], zcoords[ints[3]]);
	    }
	}
    }
    /* not implemented */
    else if (
	strstr(curobj, "LINE")    || strstr(curobj, "POINT")    ||
	strstr(curobj, "CIRCLE")  || strstr(curobj, "ARC")      ||
	strstr(curobj, "TEXT")    || strstr(curobj, "SHAPE")    ||
        strstr(curobj, "BLOCK")   || strstr(curobj, "ENDBLK")   ||
        strstr(curobj, "INSERT")  || strstr(curobj, "ATTDEF")   ||
        strstr(curobj, "ATTRIB")  || strstr(curobj, "3DLINE")   ||
	strstr(curobj, "DIMENSION"))
    {
	return;
    }
    /* no current object defined */
    else
	return;
}

/*
 * colors defined by basic AutoCAD color table
 */
static void
getColor (int ndx, float *red, float *green, float *blue)
{
    /* AutoCAD color table */
    static float colors[][3] =
    {
       {0.0f,  0.0f,  0.0f},	/* 0 == black   */
       {1.0f,  0.0f,  0.0f},	/* 1 == red     */
       {1.0f,  1.0f,  0.0f},	/* 2 == yellow  */
       {0.0f,  1.0f,  0.0f},	/* 3 == green   */
       {0.0f,  1.0f,  1.0f},	/* 4 == cyan    */
       {0.0f,  0.0f,  1.0f},	/* 5 == blue    */
       {1.0f,  0.0f,  1.0f},	/* 6 == magenta */
       {1.0f,  1.0f,  1.0f},	/* 7 == white   */
       {0.5f,  0.5f,  0.5f},	/* 8 == dk grey */
       {0.75f, 0.75f, 0.75f} 	/* 9 == lt grey */
    };

    /* use white (color 7) as default color */
    if (ndx < 0 || ndx > 9)
	ndx = 7;

    /* set colors in reference arguments */
    *red   = colors[ndx][0];
    *green = colors[ndx][1];
    *blue  = colors[ndx][2];

    return;
}

/*
 * move forward to ENTITIES section
 */
static int
findEntity(FILE *fp)
{
    while (1)
    {
	/* is there more to read */
	if (feof(fp))
	    return 0;

	/* get a group code and a line */
	if (getLine(fp))
	    return 0;

	/* file section mark */
	if (groupcode == 0)
	{
	    if (strstr(linbuf, "EOF"))
		return 0;

	    if (strstr(linbuf, "SECTION"))
	    {
		if (getLine(fp))
		    return 0;
		if (groupcode != 2)
		    continue;
		if (strstr(linbuf, "ENTITIES"))
		    break;
	    }
	}
    }

    return 1;
}

/*
 * read objects from ENTITIES section
 */
static int
readEntity(FILE *fp)
{
    while (1)
    {
	/* is there more to read */
	if (feof(fp))
	    break;

	/* get a group code and a line */
	if (getLine(fp))
	    break;

	/* cardinal group codes */
	if (groupcode < 10)
	{
	    switch(groupcode)
	    {
	    /* start of entity, table, file sep */
	    case 0:
		/* add object's polygons to builder */
		addGeometry();

		if (strstr(linbuf, "EOF"))
		    return 0;
		if (strstr(linbuf, "ENDSEC"))
		    continue;

		/* reset object */
		curobj[0] = '\0';
		curcolor = 7;
		getColor(curcolor, &curr, &curg, &curb);

		/* get new */
		strcpy(curobj, linbuf);
		break;

	    /* primary text value for entity (?) */
	    case 1:
		break;

	    /* block name, attribute tag, etc */
	    case 2:
	    case 3:
	    case 4:
		break;

	    /* entity handle (hex string) */
	    case 5:
		break;

	    /* line type name */
	    case 6:
		break;

	    /* text style name */
	    case 7:
		break;

	    /* layer name */
	    case 8:
		break;

	    /* variable name ID (only in header) */
	    case 9:
		break;
	    }
	}
	/* some X coord */
	else if (groupcode >= 10 && groupcode < 19)
	    sscanf(linbuf, "%f", &(xcoords[groupcode-10]));
	/* some Y coord */
	else if (groupcode >= 20 && groupcode < 29)
	    sscanf(linbuf, "%f", &(ycoords[groupcode-20]));
	/* some Z coord */
	else if (groupcode >= 30 && groupcode < 38)
	    sscanf(linbuf, "%f", &(zcoords[groupcode-30]));
	/* entity elevation if nonzero */
	else if (groupcode == 38)
	{
	}
	/* entity thickness if nonzero */
	else if (groupcode == 39)
	{
	}
	/* misc floats */
	else if (groupcode >= 40 && groupcode < 49)
	    sscanf(linbuf, "%f", &(floats[groupcode-40]));
	/* repeated value groups */
	else if (groupcode == 49)
	{
	}
	/* misc angles */
	else if (groupcode >= 50 && groupcode < 59)
	    sscanf(linbuf, "%f", &(angles[groupcode-50]));
	/* color number */
	else if (groupcode == 62)
	{
	    sscanf(linbuf, "%6d", &curcolor);
	    getColor(curcolor, &curr, &curg, &curb);
	}
	/* "entities follow" flag */
	else if (groupcode == 66)
	{
	}
	/* misc ints */
	else if (groupcode >= 70 && groupcode < 79)
	    sscanf(linbuf, "%d", &(ints[groupcode-70]));
	/* X, Y, Z components of extrusion direction */
	else if (groupcode == 210 || groupcode == 220 || groupcode == 230)
	{
	}
    }
    return 0;
}
