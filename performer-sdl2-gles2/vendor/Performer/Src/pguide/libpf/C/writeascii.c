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
 *
 * writeascii: write OpenGL Performer data structures in ASCII
 *
 * $Revision: 1.24 $ 
 * $Date: 2000/10/06 21:26:39 $
 */

#include <stdio.h>
#include <stdlib.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>

/* define VERBOSE to print pfGeoSet contents (no pfGeoStates yet) */
#define	VERBOSE /* */

void writeNodes(FILE *fp, pfNode *node);

/*
 *	main() -- program entry point. 
 */

void
main (int argc, char *argv[])
{
    int		    k;
    pfNode	   *node;
    pfScene	   *scene;
    double	    readTime;
    double	    elapsedReadTime = 0.0;
    FILE	   *fp;
    double	    elapsedWriteTime = 0.0;
    char	   *fileName = "scene.ascii";

    /* initialize OpenGL Performer */
    pfInit();
    pfMultiprocess(PFMP_APPCULLDRAW);

    /* load files named on command line to scene graph */
    for (k = 1; k < argc; k++)
	pfdInitConverter(argv[k]);

    pfConfig();

    if (!(getenv("PFPATH")))
        pfFilePath(
                   "."
                   ":./data"
                   ":../data"
                   ":../../data"
                   ":../../../../data"
                   ":/usr/share/Performer/data"
                   );

    /* create root for scene graph */
    scene = pfNewScene();

    /* load files named on command line to scene graph */
    for (k = 1; k < argc; k++)
    {
	readTime = pfGetTime();
	if ((node = pfdLoadFile(argv[k])) != NULL)
	    pfAddChild(scene, node);
	readTime = pfGetTime() - readTime;
	printf("%10.6f sec reading %s\n", readTime, argv[k]);
	elapsedReadTime += readTime;
    }

    /* report elapsed reading time */
    printf("----------\n");
    printf("%10.6f sec total reading time\n\n", elapsedReadTime);

    /* perform printing traversal of scene */
    fp = fopen(fileName, "w");
    if (fp == NULL)
    {
	fprintf(stderr, "error: can't open file \"%s\" for output\n", fileName);
	exit(1);
    }
    elapsedWriteTime = pfGetTime();
    writeNodes(fp, (pfNode *)scene);
    elapsedWriteTime = pfGetTime() - elapsedWriteTime;

    /* report elapsed writing time */
    printf("----------\n");
    printf("%10.6f sec total writing time (%s)\n", elapsedWriteTime, fileName);

    /* terminate parallel processes and exit */
    pfExit();
    exit(0);
}

/***
 ***	Write OpenGL Performer Scene Graph in verbose ASCII format
 ***/

static FILE	*out		= NULL;
static int	indentLevel	= 0;

static int	numBillboard	= 0;
static int	numDCS		= 0;
static int	numGeode	= 0;
static int	numGroup	= 0;
static int	numLayer	= 0;
static int	numLightPoint	= 0;
static int	numLOD		= 0;
static int	numScene	= 0;
static int	numSCS		= 0;
static int	numSequence	= 0;
static int	numSwitch	= 0;
static int	numOther	= 0;

void writeNodesAux(pfNode *node);

void
indent (void)
{
    fprintf(out, "%*s", indentLevel, "");
}

void
writeMatrix (pfMatrix matrix)
{
    int		row;
    int		col;

    for (row = 0; row < 4; row++)
    {
	indent();
	for (col = 0; col < 4; col++)
	    fprintf(out, "%10.6f ", matrix[row][col]);
	fprintf(out, "\n");
    }
}

void
writeGeoSet (pfGeoSet *geoset)
{
    int	primType = pfGetGSetPrimType(geoset);
    int	numPrims = pfGetGSetNumPrims(geoset);
    int *lengths = NULL;
    int	i;
    int	binding;
    int	vCount = 0;
    int	nCount = 0;
    int	tCount = 0;
    int	cCount = 0;
    pfVec3	*vArray;
    pfVec3	*nArray;
    pfVec2	*tArray;
    pfVec4	*cArray;
    ushort	*index;

    indent();
    fprintf(out, "pfGeoSet\n");

#ifdef	VERBOSE

    ++indentLevel;
    indent();
    fprintf(out, "begin\n");

    indent();
    fprintf(out, "address %p\n", (size_t)geoset);

    indent();
    fprintf(out, "type %d ", primType);
    switch (primType)
    {
    case PFGS_POINTS:
	fprintf(out, "(PFGS_POINTS)");
	vCount = numPrims;
	break;
    case PFGS_LINES:
	fprintf(out, "(PFGS_LINES)");
	vCount = 2*numPrims;
	break;
    case PFGS_LINESTRIPS:
	fprintf(out, "(PFGS_LINESTRIPS)");
	lengths = pfGetGSetPrimLengths(geoset);
	for (i = 0; i < numPrims; i++)
	    vCount += pfGetGSetPrimLength(geoset, i);
	break;
    case PFGS_TRIS:
	fprintf(out, "(PFGS_TRIS)");
	vCount = 3*numPrims;
	break;
    case PFGS_QUADS:
	fprintf(out, "(PFGS_QUADS)");
	vCount = 4*numPrims;
	break;
    case PFGS_TRISTRIPS:
	fprintf(out, "(PFGS_TRISTRIPS)");
	lengths = pfGetGSetPrimLengths(geoset);
	for (i = 0; i < numPrims; i++)
	    vCount += pfGetGSetPrimLength(geoset, i);
	break;
    }
    fprintf(out, "\n");

    indent();
    fprintf(out, "count %d\n", numPrims);

    /* strip-primitive lengths */
    if (lengths != NULL)
    {
	indent();
	fprintf(out, "lengths\n");

	++indentLevel;
	indent();
	fprintf(out, "begin\n");

	for (i = 0; i < numPrims; i++)
	{
	    indent();
	    fprintf(out, "%d\n", lengths[i]);
	}

	indent();
	fprintf(out, "end\n");
	--indentLevel;
    }

    /* vertex coordinates */
    binding = pfGetGSetAttrBind(geoset, PFGS_COORD3);
    pfGetGSetAttrLists(geoset, PFGS_COORD3, (void *)&vArray, &index);
    if (vArray != NULL && binding != PFGS_OFF)
    {
	indent();
	fprintf(out, "vertex\n");
	indent();
	fprintf(out, "binding %d", binding);
	switch (binding)
	{
	case PFGS_OFF:
	    fprintf(out, " (PFGS_OFF)");
	    break;
	case PFGS_OVERALL:
	    fprintf(out, " (PFGS_OVERALL)");
	    break;
	case PFGS_PER_PRIM:
	    fprintf(out, " (PFGS_PER_PRIM)");
	    break;
	case PFGS_PER_VERTEX:
	    fprintf(out, " (PFGS_PER_VERTEX)");
	    break;
	}
	fprintf(out, "\n");

	++indentLevel;
	indent();
	fprintf(out, "begin\n");

	if (index == NULL)
	{
	    for (i = 0; i < vCount; i++)
	    {
		indent();
		fprintf(out, "%10.6f %10.6f %10.6f\n", 
		    vArray[i][PF_X], 
		    vArray[i][PF_Y], 
		    vArray[i][PF_Z]);
	    }
	}
	else 
	{
	    for (i = 0; i < vCount; i++)
	    {
		indent();
		fprintf(out, "%10.6f %10.6f %10.6f\n", 
		    vArray[index[i]][PF_X], 
		    vArray[index[i]][PF_Y], 
		    vArray[index[i]][PF_Z]);
	    }
	}

	indent();
	fprintf(out, "end\n");
	--indentLevel;
    }

    /* normal vectors */
    binding = pfGetGSetAttrBind(geoset, PFGS_NORMAL3);
    pfGetGSetAttrLists(geoset, PFGS_NORMAL3, (void *)&nArray, &index);
    if (nArray != NULL && binding != PFGS_OFF)
    {
	indent();
	fprintf(out, "normal\n");
	indent();
	fprintf(out, "binding %d", binding);
	switch (binding)
	{
	case PFGS_OFF:
	    fprintf(out, " (PFGS_OFF)");
	    nCount = 0;
	    break;
	case PFGS_OVERALL:
	    fprintf(out, " (PFGS_OVERALL)");
	    nCount = 1;
	    break;
	case PFGS_PER_PRIM:
	    fprintf(out, " (PFGS_PER_PRIM)");
	    nCount = numPrims;
	    break;
	case PFGS_PER_VERTEX:
	    fprintf(out, " (PFGS_PER_VERTEX)");
	    nCount = vCount;
	    break;
	}
	fprintf(out, "\n");

	++indentLevel;
	indent();
	fprintf(out, "begin\n");

	if (index == NULL)
	{
	    for (i = 0; i < nCount; i++)
	    {
		indent();
		fprintf(out, "%10.6f %10.6f %10.6f\n", 
		    nArray[i][PF_X], 
		    nArray[i][PF_Y], 
		    nArray[i][PF_Z]);
	    }
	}
	else 
	{
	    for (i = 0; i < nCount; i++)
	    {
		indent();
		fprintf(out, "%10.6f %10.6f %10.6f\n", 
		    nArray[index[i]][PF_X], 
		    nArray[index[i]][PF_Y], 
		    nArray[index[i]][PF_Z]);
	    }
	}

	indent();
	fprintf(out, "end\n");
	--indentLevel;
    }

    /* color values */
    binding = pfGetGSetAttrBind(geoset, PFGS_COLOR4);
    pfGetGSetAttrLists(geoset, PFGS_COLOR4, (void *)&cArray, &index);
    if (cArray != NULL && binding != PFGS_OFF)
    {
	indent();
	fprintf(out, "color\n");
	indent();
	fprintf(out, "binding %d", binding);
	switch (binding)
	{
	case PFGS_OFF:
	    fprintf(out, " (PFGS_OFF)");
	    cCount = 0;
	    break;
	case PFGS_OVERALL:
	    fprintf(out, " (PFGS_OVERALL)");
	    cCount = 1;
	    break;
	case PFGS_PER_PRIM:
	    fprintf(out, " (PFGS_PER_PRIM)");
	    cCount = numPrims;
	    break;
	case PFGS_PER_VERTEX:
	    fprintf(out, " (PFGS_PER_VERTEX)");
	    cCount = vCount;
	    break;
	}
	fprintf(out, "\n");

	++indentLevel;
	indent();
	fprintf(out, "begin\n");

	if (index == NULL)
	{
	    for (i = 0; i < cCount; i++)
	    {
		indent();
		fprintf(out, "%10.6f %10.6f %10.6f %10.6f\n", 
		    cArray[i][0], 
		    cArray[i][1], 
		    cArray[i][2],
		    cArray[i][3]);
	    }
	}
	else 
	{
	    for (i = 0; i < cCount; i++)
	    {
		indent();
		fprintf(out, "%10.6f %10.6f %10.6f %10.6f\n", 
		    cArray[index[i]][0], 
		    cArray[index[i]][1], 
		    cArray[index[i]][2],
		    cArray[index[i]][3]);
	    }
	}

	indent();
	fprintf(out, "end\n");
	--indentLevel;
    }

    /* texture values */
    binding = pfGetGSetAttrBind(geoset, PFGS_TEXCOORD2);
    pfGetGSetAttrLists(geoset, PFGS_TEXCOORD2, (void *)&tArray, &index);
    if (tArray != NULL && binding != PFGS_OFF)
    {
	indent();
	fprintf(out, "texture\n");
	indent();
	fprintf(out, "binding %d", binding);
	switch (binding)
	{
	case PFGS_OFF:
	    fprintf(out, " (PFGS_OFF)");
	    tCount = 0;
	    break;
	case PFGS_OVERALL:
	    fprintf(out, " (PFGS_OVERALL)");
	    tCount = 1;
	    break;
	case PFGS_PER_PRIM:
	    fprintf(out, " (PFGS_PER_PRIM)");
	    tCount = numPrims;
	    break;
	case PFGS_PER_VERTEX:
	    fprintf(out, " (PFGS_PER_VERTEX)");
	    tCount = vCount;
	    break;
	}
	fprintf(out, "\n");

	++indentLevel;
	indent();
	fprintf(out, "begin\n");

	if (index == NULL)
	{
	    for (i = 0; i < tCount; i++)
	    {
		indent();
		fprintf(out, "%10.6f %10.6f\n", 
		    tArray[i][0],
		    tArray[i][1]);
	    }
	}
	else 
	{
	    for (i = 0; i < tCount; i++)
	    {
		indent();
		fprintf(out, "%10.6f %10.6f\n", 
		    tArray[index[i]][0],
		    tArray[index[i]][1]);
	    }
	}

	indent();
	fprintf(out, "end\n");
	--indentLevel;
    }

    indent();
    fprintf(out, "end\n");
    --indentLevel;
#endif
}

void
writeGeode (pfGeode *geode)
{
    int		numGeoSets = pfGetNumGSets(geode);
    int		child;

    ++indentLevel;
    indent();
    fprintf(out, "begin\n");

    for (child = 0; child < numGeoSets; child++)
	writeGeoSet(pfGetGSet(geode, child));

    indent();
    fprintf(out, "end\n");
    --indentLevel;
}

void
writeGroup (pfGroup *group)
{
    int		numChildren = pfGetNumChildren(group);
    int		child;

    ++indentLevel;
    indent();
    fprintf(out, "begin\n");

    for (child = 0; child < numChildren; child++)
	writeNodesAux(pfGetChild(group, child));

    indent();
    fprintf(out, "end\n");
    --indentLevel;
}

/*
 *	writeNodesAux() -- hierarchical printing traversal (auxilliary)
 */

void
writeNodesAux (pfNode *node)
{
    const char  *nodeName;
    pfMatrix	matrix;
    int		numChildren;
    int		child;
    pfVec3	center;
    int		mode;
    int		begin;
    int		end;
    float	speed;
    int		reps;
    int		count;
    int		numGeoSets;
    pfVec3	pos;
    int         i, n;

    if (node != NULL)
    {
	indent();
	fprintf(out, "%s", pfGetTypeName(node));
	nodeName = pfGetNodeName(node);
	if (nodeName != NULL)
	{
	    indent();
	    fprintf(out, "name %s\n", nodeName);
	}
	else
	    fprintf(out, "\n");
	indent();
	n = pfGetNumParents(node);
	fprintf(out, "  Num Parents: %d\n", n);
	for (i=0; i < n; i++)
	{
	    indent();
	    fprintf(out, "  \"%s\"=0x%p\n", pfGetNodeName(pfGetParent(node, i)), pfGetParent(node, i));
	}

	/* geode class */
	if (pfIsOfType(node, pfGetGeodeClassType()))
	{
	    numGeoSets = pfGetNumGSets((pfGeode *)node);
	    indent();
	    fprintf(out, "Num pfGeoSets: %d", numGeoSets);
	    /* billboard */
	    if (pfIsOfType(node, pfGetBboardClassType()))
	    {
		indent();
		fprintf(out, "position ");
		for (child = 0; child < numGeoSets; child++)
		{
		    pfGetBboardPos((pfBillboard *)node, child, pos);
		    fprintf(out, "%10.6f %10.6f %10.6f", pos[PF_X], pos[PF_Y], pos[PF_Z]);
		}
		fprintf(out, "\n");
		++numBillboard;
	    }
	    /* other pfGeodes */
	    else 
		++numGeode;
	    writeGeode((pfGeode *)node);
	}
	/* Group class */
	else if (pfIsOfType(node, pfGetGroupClassType()))
	{
	    n = pfGetNumChildren(node);
	    indent();
	    fprintf(out, "Num Children: %d", n);
	    /* billboard */
	    if (pfIsOfType(node, pfGetSCSClassType()))
	    {
		pfGetSCSMat((pfSCS *)node, matrix);
		writeMatrix(matrix);
		writeGroup((pfGroup *)node);
		/* DCS type */
		if (pfIsOfType(node, pfGetDCSClassType()))
		    ++numDCS;
		/* other SCS types */
		else
		    ++numSCS;
	    }
	    /* Layer class */
	    else if (pfIsOfType(node, pfGetLayerClassType()))
		++numLayer;
	    /* LOD class */
	    else if (pfIsOfType(node, pfGetLODClassType()))
	    {
		pfGetLODCenter((pfLOD *)node, center);
		indent();
		fprintf(out, "center %10.6f %10.6f %10.6f\n", center[PF_X], center[PF_Y], center[PF_Z]); 
		numChildren = pfGetNumChildren((pfGroup *)node);
		indent();
		fprintf(out, "range ");
		for (child = 0; child <= numChildren; child++)
		    fprintf(out, "%10.6f ", pfGetLODRange((pfLOD *)node, child));
		fprintf(out, "\n");
		++numLOD;
	    }
	    /* Scene class */
	    else if (pfIsOfType(node, pfGetSceneClassType()))
		++numScene;
	    /* Sequence class */
	    else if (pfIsOfType(node, pfGetSeqClassType()))
	    {
		indent();
		fprintf(out, "time ");
		numChildren = pfGetNumChildren((pfGroup *)node);
		for (child = 0; child < numChildren; child++)
		    fprintf(out, "%10.6f ", 
			    pfGetSeqTime((pfSequence *)node, child));
		fprintf(out, "\n");
		pfGetSeqInterval((pfSequence *)node, &mode, &begin, &end);
		indent();
		fprintf(out, "interval %d %d %d\n", mode, begin, end);
		pfGetSeqDuration((pfSequence *)node, &speed, &reps);
		indent();
		fprintf(out, "duration %10.6f %d\n", speed, reps);
		indent();
		fprintf(out, "mode %d\n", pfGetSeqMode((pfSequence *)node));
		indent();
		fprintf(out, "frame %d\n", pfGetSeqFrame((pfSequence *)node, &count));
		++numSequence;
	    }
	    /* Switch class */
	    else if (pfIsOfType(node, pfGetSwitchClassType()))
	    {
		indent();
		fprintf(out, "value %d\n", pfGetSwitchVal((pfSwitch *)node));
		++numSwitch;
	    }
	    /* other groups */
	    else
		++numGroup;
	    writeGroup((pfGroup *)node);
	}
	else if (pfIsOfType(node, pfGetLPointClassType()))
	    ++numLightPoint;
	else 
	{
	    fprintf(out, 
		    "warning: node type (%s) is unknown. pruning.\n",
		    pfGetTypeName(node));
	    ++numOther;
	}
    }
}

/*
 *	writeNodes() -- hierarchical printing traversal
 */

void
writeNodes (FILE *fp, pfNode *node)
{
    /* output destination is a global */
    out = fp;

    /* initialize statistics */
    numBillboard  	= 0;
    numDCS		= 0;
    numGeode		= 0;
    numGroup		= 0;
    numLayer		= 0;
    numLightPoint 	= 0;
    numLOD		= 0;
    numScene		= 0;
    numSCS		= 0;
    numSequence		= 0;
    numSwitch		= 0;
    numOther		= 0;

    /* do the actual work */
    writeNodesAux(node);

    printf(" %6d Billboard node%s\n", numBillboard, (numBillboard==1) ?"":"s");
    printf(" %6d DCS node%s\n", numDCS, (numDCS==1) ?"":"s");
    printf(" %6d Geode node%s\n", numGeode, (numGeode==1) ?"":"s");
    printf(" %6d Group node%s\n", numGroup, (numGroup==1) ?"":"s");
    printf(" %6d Layer node%s\n", numLayer, (numLayer==1) ?"":"s");
    printf(" %6d LightPoint node%s\n", numLightPoint, (numLightPoint==1) ?"":"s");
    printf(" %6d LOD node%s\n", numLOD, (numLOD==1) ?"":"s");
    printf(" %6d Scene node%s\n", numScene, (numScene==1) ?"":"s");
    printf(" %6d SCS node%s\n", numSCS, (numSCS==1) ?"":"s");
    printf(" %6d Sequence node%s\n", numSequence, (numSequence==1) ?"":"s");
    printf(" %6d Switch node%s\n", numSwitch, (numSwitch==1) ?"":"s");
    printf(" %6d Other node type%s\n", numOther, (numOther==1) ?"":"s");
    printf("\n");
}
