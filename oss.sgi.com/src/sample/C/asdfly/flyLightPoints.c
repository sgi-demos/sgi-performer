/*
 *	PROPRIETARY RIGHTS NOTICE: All rights reserved. This program
 *	contains proprietary information and trade secrets of 
 *	MultiGen of San Jose, CA., and embodies substantial creative
 *	efforts and confidential information, ideas, and expressions.
 *	No part of this program may be reproduced in any form, or by
 *	any means electronic, mechanical, or otherwise, without the
 *	written permission of MultiGen Inc.
 *
 *	COPYRIGHT NOTICE: Copyright (C) 1995, MultiGen Inc.,
 *	550 S Winchester Blvd. Suite 500, San Jose, CA 95128.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifndef __linux__
#include <bstring.h>
#endif
#include <math.h>
#include <ctype.h>

#include <Performer/pf.h>
#include <Performer/prmath.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#include <GL/glx.h>

#include "perfly.h"
#include "gui.h"
#include "env.h"
#include "terrain.h"
#include "flyLightPoints.h"
#include "pfuLightPointsPrototypes.h"

#define	NOF_LIGHT_POINT_SETS	4
#define	X_SIZE			20000.0
#define	Y_SIZE			20000.0
#define	X_BASE			3000.0
#define	Y_BASE			3000.0
#define	SET_X_SPAN		1000.0
#define	SET_Y_SPAN		1000.0
#define	LIGHT_POINT_SET_SIZE	30

static	float   random_0_1(void);

static lightPointSetList  	lightPoints;
static terrainObjectList	objects;
static terrainSegmentList	segments;
static terrainTriangleList 	triangles;
static terrainAnimationList 	animations;

void    generateLightPoints (lightPointSetList *lightPoints);
void    readLightPoints (
		lightPointSetList *lightPoints, 
		terrainObjectList *object, 
		terrainSegmentList *segmentList,
		terrainTriangleList *triangleList,
		terrainAnimationList *animationList,
		char *filename);
void    initLightPointSetList (lightPointSetList *lightPoints);
void    addLightPoint (lightPointSetList *lightPoints,
					int     setId,
					int     pointId,
					float   x,
					float   y,
					float   z);

void    initTerrainObjectList (terrainObjectList *objectList);
void    addTerrainObject (
                        terrainObjectList       *objectList,
                        float                   x,
                        float                   y,
                        float                   z,
			float			*down,
			float			*xy_direction,
                        char                    *filename,
			int			is_normal,
			int			is_azimuth);

void	addTerrainBillboard (
			terrainObjectList 	*objectList, 
			float			x, 
			float			y, 
			float			z, 
			float			*down,
			char			*filename,
			float			width,
			float			height);

void    initTerrainSegmentList (terrainSegmentList *segments);

void    addTerrainSegment (
			terrainSegmentList 	*segmentList,
			float           	*v0,
			float           	*v1);

void	initTerrainTriangleList (terrainTriangleList *triangles);

void 	addTerrainGeoSet (
			terrainTriangleList 	*triangleList,
			pfGeoSet            	*gset,
			float               	*base,
			float               	*down,
			float               	*azimuth,
			float               	*projection,
			char                	*texture_filename,
			float               	*c,
			unsigned long       	mask,
			unsigned long       	transform_mask);

void    initTerrainAnimationList (terrainAnimationList *animations);
void	addTerrainAnimation (
			terrainAnimationList 	*segmentList, 
			float			x, 
			float			y, 
			float 			z, 
			float			*down, 
			float 			radius, 
			float 			d_angle, 
			char			*name);

int 	flyIsectFunc (pfHit *hit);

void   composeGset (
			pfGeoSet		*gset, 
			float 			*v, 
			float 			*t, 
			float 			*c, 
			float 			*n, 
			int			nof_triangles);



/*===========================================================================*/
void  
flyLightsInit(pfASD *asd)
/*===========================================================================*/
{
    pfGroup 		*hook;
    int			i;
    unsigned long	bit;

    ViewState -> isIsect = 0;

    readLightPoints (&lightPoints, &objects, &segments, 
			&triangles, &animations,
			ViewState -> lightPoints . filename);

    for (i = 0 ; i < lightPoints . nofSets ; i ++)
    {
	hook = (pfGroup *) pfNewLightPointAlignNode (
			lightPoints . sets[i] . nofPoints,
			lightPoints . sets[i] . coord,
			lightPoints . sets[i] . color,
			lightPoints . sets[i] . normal,
                        (lightPoints . sets[i] . coord[2] == 0.0) ? 1 : 0, 
			asd);

	pfAddChild(ViewState->lightGroup,hook);
    }

    for (i = 0 ; i < objects . nofObjects ; i ++)
    {
	if (! objects . object[i].is_billboard)
	    hook = (pfGroup *) pfNewObjectAlignNode (
				objects . object[i].filename,
				objects . object[i].x, 
				objects . object[i].y,
				objects . object[i].z,
				objects . object[i].down,
				objects . object[i].is_normal,
				objects . object[i].is_azimuth,
				objects . object[i].xy_direction,
				asd);
	else
	    hook = (pfGroup *) pfNewBillboardAlignNode(
				objects . object[i].filename,
				objects . object[i].x, 
				objects . object[i].y,
				objects . object[i].z,
				objects . object[i].down,
				objects . object[i].width,
				objects . object[i].height,
				asd);

	pfAddChild(ViewState->lightGroup,hook);
    }

    /*
     *	If we have any segments, generate Isect structures and GeoSet.
     */
    if (segments . nofSegments > 0)
    {
	fprintf (stderr, "Found %d Isect segments\n", segments . nofSegments);

	ViewState -> isIsect = 1;
	ViewState -> segSet.mode = PFTRAV_IS_PRIM;
	ViewState -> segSet.activeMask = 0x0;
	ViewState -> segSet.isectMask = 0xffffffff; /* default pfNode value */
	ViewState -> segSet.bound = NULL;
	ViewState -> segSet.discFunc = flyIsectFunc;

	bit = 0x01;
	for (i = 0 ; i < segments . nofSegments ; i ++)
	{
	    ViewState -> segSet.activeMask |= bit;
	    pfSetVec3 (ViewState -> segSet.segs[i].pos, 
			segments . segment[i].v0[0], 
			segments . segment[i].v0[1], 
			segments . segment[i].v0[2]);

	    pfSetVec3 (ViewState -> segSet.segs[i].dir, 
			segments . segment[i].v1[0] 
					- segments . segment[i].v0[0], 
			segments . segment[i].v1[1] 
					- segments . segment[i].v0[1], 
			segments . segment[i].v1[2] 
					- segments . segment[i].v0[2]);

	    ViewState -> segSet.segs[i].length = 
	    		pfNormalizeVec3 (ViewState -> segSet.segs[i].dir);


	    bit <<= 1;
	}

	printf ("Active mask = 0x0%x\n", ViewState -> segSet.activeMask);
    }

    /*
     *	============================================================
     *	Add all query triangles.
     *	============================================================
     */
    for (i = 0 ; i < triangles . nofTriangles ; i ++)
    {
	hook = (pfGroup *) pfuNewGSetShadowNode (
		    triangles . triangle[i] . gset,
		    (float *) triangles . triangle[i] . base,
		    (float *) triangles . triangle[i] . down,
		    (float *) triangles . triangle[i] . azimuth,
		    (float *) triangles . triangle[i] . projection,
		    triangles . triangle[i] . texture_filename,
		    triangles . triangle[i] . mask,
		    triangles . triangle[i] . transformation_mask,
		    (float *) triangles . triangle[i] . c,
		    asd);
	

	pfAddChild(ViewState->lightGroup,hook);
    }

    /*
     *	============================================================
     *	Add all Animations
     *	============================================================
     */
    for (i = 0 ; i < animations . nofAnimations ; i ++)
    {
	hook = (pfGroup *) pfNewAnimationAlignNode (
			animations . animation[i] . model_filename, 
			animations . animation[i] . center[0], 
			animations . animation[i] . center[1], 
			animations . animation[i] . center[2], 
			animations . animation[i] . down, 
			animations . animation[i] . radius, 
			&animations . animation[i] . query_id, 
			&animations . animation[i] . azimuth_flux, 
			asd);

	animations . animation[i] . asd = asd;

	pfAddChild(ViewState->lightGroup,hook);
    }
}

/*===========================================================================*/
static	float    random_0_1(void)
/*===========================================================================*/
{
return (((float) (random() % 10000)) / 10000.0);
}


/*===========================================================================*/
void    generateLightPoints (lightPointSetList *lightPoints)
/*===========================================================================*/
{
int	i,j;
float	xmin, ymin, x, y;

initLightPointSetList (lightPoints);

/*
 *	First set: diagonal across database.
 *	Second set: another diagonal across database.
 */
for (i = 0 ; i < 300 ; i ++)
	{
	x = ((float) i) * (float) X_SIZE / 300.0;

	addLightPoint (lightPoints, 0, i, X_BASE + x, Y_BASE + x, 0.0);
	addLightPoint (lightPoints, 1, i, X_BASE + X_SIZE - x, Y_BASE + x, 0.0);
	}

for (i = 2 ; i < NOF_LIGHT_POINT_SETS ; i ++)
	{
	xmin = random_0_1() * X_SIZE;
	ymin = random_0_1() * Y_SIZE;

	for (j = 0 ; j < LIGHT_POINT_SET_SIZE ; j ++)
		{
		x = xmin + random_0_1() * SET_X_SPAN;
		y = ymin + random_0_1() * SET_Y_SPAN;

		addLightPoint (lightPoints, i, j, x, y, 0.0);
		}
	}
}

/*===========================================================================*/
void    readLightPoints (lightPointSetList *lightPoints, 
			terrainObjectList *objectList,
			terrainSegmentList *segmentList,
			terrainTriangleList *triangleList,
			terrainAnimationList *animationList,
			char *filename)
/*===========================================================================*/
{
    float		x, y, z;
    FILE		*fp;
    int			nof_points;
    int			set, point;
    float		from_x, from_y, from_z, to_x, to_y, to_z;
    float		f;
    char		line[200];
    char		name[200];
    int			l, nof_lines;
    float		xy_direction[3];
    float		down[3];
    float		width, height;
    float		v0[3], v1[3];
    float		v[MAX_POINTS][3], t[MAX_POINTS][2], c[MAX_POINTS][4];
    float		azimuth[3], base[3], projection[3];
    float		*c_p, *t_p, *azimuth_p, *down_p, *base_p, *projection_p;
    int			c_index, t_index, v_index;
    float		a0, a1, a2, a3;
    int			done_triangle;
    char		op[100];
    int			nof_triangles;
    int			mask;
    unsigned long	transform_mask;
    char		transform[200];
    char		model_filename[200];
    char		texture_filename[200];
    pfNode		*model;
    int				is_model;
    float			radius, d_angle;
    pfGeoSet			*gset;


    initLightPointSetList (lightPoints);
    initTerrainObjectList (objectList);
    initTerrainSegmentList (segmentList);
    initTerrainTriangleList (triangleList);
    initTerrainAnimationList (animationList);

    if ((fp = fopen (filename, "r")) == NULL)
    {
	perror (filename);
	return;
    }

    
    fgets (line, 200, fp);
    sscanf (line, "%d", &nof_lines);

    set = 0;
    for (l = 0 ; (l < nof_lines) && (! feof (fp)); l ++)
    {
	line[0] = '\0';
	fgets (line, 200, fp);
	switch (line[0])
	{
	    case 'L':
		sscanf (&line[1], "%d%f%f%f%f", 
			&nof_points, &from_x, &from_y, &to_x, &to_y);
		for (point = 0 ; point < nof_points ; point ++)
		{
		    f = ((float) point) / ((float) nof_points - 1.0);
		    x = from_x * (1-f) + to_x * f;
		    y = from_y * (1-f) + to_y * f;
		    addLightPoint (lightPoints, set, point, x, y, 0.0);
		}
		set ++;
		break;

	    case 'S':
		sscanf (&line[1], "%d%f%f%f%f", 
			&nof_points, &from_x, &from_y, &to_x, &to_y);
		for (point = 0 ; point < nof_points ; point ++)
		{
		    x = from_x + (to_x - from_x) 
				    * ((float) (random() % 10000)) / 10000.0;
		    y = from_y + (to_y - from_y) 
				    * ((float) (random() % 10000)) / 10000.0;
		    addLightPoint (lightPoints, set, point, x, y, 0.0);
		}
		set ++;
		break;

	    case 's':
		sscanf (&line[1], "%d%f%f%f%f%f%f", 
			&nof_points, &from_x, &from_y, &from_z, 
					&to_x, &to_y, &to_z);

		for (point = 0 ; point < nof_points ; point ++)
		{
		    x = from_x + (to_x - from_x) 
				    * ((float) (random() % 10000)) / 10000.0;
		    y = from_y + (to_y - from_y) 
				    * ((float) (random() % 10000)) / 10000.0;
		    z = from_z + (to_z - from_z) 
				    * ((float) (random() % 10000)) / 10000.0;
		    addLightPoint (lightPoints, set, point, x, y, z);
		}
		set ++;
		break;

	    case 'N':

		down[0] = 0.0;
		down[1] = 0.0;
		down[2] = -1.0;

		sscanf (&line[1], "%f%f%f%f%s", &x, &y, 
						&xy_direction[0],
						&xy_direction[1], 
						name);
		xy_direction[2] = 0.0;
		addTerrainObject (objectList, x, y, 0.0, down,
				    xy_direction, name, 1, 1);
		break;

	    case 'n':
		sscanf (&line[1], "%f%f%f%f%f%f%f%f%f%s", &x, &y, &z,
						&down[0],
						&down[1],
						&down[2],
						&xy_direction[0],
						&xy_direction[1], 
						&xy_direction[2], 
						name);
		addTerrainObject (objectList, x, y, z, 
					down, xy_direction, name, 0, 1);
		break;

	    case 'O':
		down[0] = 0.0;
		down[1] = 0.0;
		down[2] = -1.0;
		sscanf (&line[1], "%f%f%s", &x, &y, name);
		xy_direction[0] = 1.0;
		xy_direction[1] = 0.0;
		xy_direction[2] = 0.0;
		addTerrainObject (objectList, x, y, 0.0, 
					down,
					xy_direction, name,0, 0);
		break;

	    case 'o':

		down[0] = 0.0;
		down[1] = 0.0;
		down[2] = -1.0;
		sscanf (&line[1], "%f%f%f%f%f%s", &x, &y, 
						&xy_direction[0],
						&xy_direction[1], 
						&xy_direction[2], 
						name);
		addTerrainObject (objectList, x, y, 0.0, 
					down,
					xy_direction, name,0, 1);
		break;

	    case 'B':

		down[0] = 0.0;
		down[1] = 0.0;
		down[2] = -1.0;

		sscanf (&line[1], "%f%f%f%f%s", &x, &y, &width, &height, name);

		printf ("Billboard at (%f, %f), size (%f x %f), name (%s)\n", 
				x, y, width, height, name);

		addTerrainBillboard (objectList, x, y, 0.0, down,
				    name, width, height);
		break;

	    case 'b':

		sscanf (&line[1], "%f%f%f%f%f%f%f%f%s", &x, &y, &z,
						&down[0],
						&down[1],
						&down[2],
						&width, &height,
						name);

		addTerrainBillboard (objectList, x, y, z, 
					down, name, width, height);
		break;

	    case 'I':

		sscanf (&line[1], "%f%f%f%f%f%f", 
					&v0[0], &v0[1], &v0[2],
					&v1[0], &v1[1], &v1[2]);

		addTerrainSegment (segmentList, v0, v1);

		break;

	    case 'A': /* Animation */

		down[0] = 0.0;
		down[1] = 0.0;
		down[2] = -1.0;
		sscanf (&line[1], "%f%f%f%f%s", 
					&x, &y, &radius, &d_angle, name);

		addTerrainAnimation (animationList, x, y, 0.0, 
					down, radius, d_angle, 
					name);
		break;


	    case 'T':

		done_triangle = 0;
		v_index = 0;
		c_index = 0;
		t_index = 0;
		c_p = NULL;
		t_p = NULL;
		azimuth_p = NULL;
		down_p = NULL;
		base_p = NULL;
		texture_filename[0] = '\0';
		model_filename[0] = '\0';
		projection_p = NULL;
		transform_mask = PR_ALIGN_TRANSLATE;
		is_model = 0;

		while (! done_triangle)
		{
		    fgets (line, 200, fp);
		    l ++;
		    if (line[0] == '.')
			done_triangle = 1;
		    else
		    if (line[0] != ';')
		    {
			sscanf (line, "%s%f%f%f%f\n", op, &a0, &a1, &a2, &a3);
			if (strcmp(op, "v") == 0)
			{
			    v[v_index][0] = a0;
			    v[v_index][1] = a1;
			    v[v_index][2] = a2;
			    v_index ++; 
			    if (v_index >= MAX_POINTS)
				v_index = MAX_POINTS - 1;
			}
			else
			if (strcmp(op, "t") == 0)
			{
			    t[t_index][0] = a0;
			    t[t_index][1] = a1;
			    t_index ++; 
			    if (t_index >= MAX_POINTS)
				t_index = MAX_POINTS - 1;
			    t_p = &t[0][0];
			}
			else
			if (strcmp(op, "c") == 0)
			{
			    c[c_index][0] = a0;
			    c[c_index][1] = a1;
			    c[c_index][2] = a2;
			    c[c_index][3] = a3;
			    c_index ++; 
			    if (c_index >= MAX_POINTS)
				c_index = MAX_POINTS - 1;
			    c_p = &c[0][0];
			}
			else
			if (strcmp(op, "down") == 0)
			{
			    down[0] = a0;
			    down[1] = a1;
			    down[2] = a2;
			    down_p = down;
			}
			else
			if (strcmp(op, "azimuth") == 0)
			{
			    azimuth[0] = a0;
			    azimuth[1] = a1;
			    azimuth[2] = a2;
			    azimuth_p = azimuth;
			}
			else
			if (strcmp(op, "base") == 0)
			{
			    base[0] = a0;
			    base[1] = a1;
			    base[2] = a2;
			    base_p = base;
			}
			else
			if (strcmp(op, "projection") == 0)
			{
			    projection[0] = a0;
			    projection[1] = a1;
			    projection[2] = a2;
			    projection_p = projection;
			}
			else
			if (strcmp(op, "texture") == 0)
			{
			    sscanf (line, "%s%s", op, texture_filename);
			}
			else
			if (strcmp(op, "model") == 0)
			{
			    sscanf (line, "%s%s", op, model_filename);
			    is_model = 1;
			}
			else
			if (strcmp(op, "transform") == 0)
			{
			    sscanf (line, "%s%s", op, transform);
			    if (strcmp(transform, "AXIS_BILLBOARD") == 0)
				transform_mask |= PR_ALIGN_AXIS_BILLBOARD;
			    else
			    if (strcmp(transform, "TRANSLATE") == 0)
				transform_mask |= PR_ALIGN_TRANSLATE;
			    else
			    if (strcmp(transform, "NORMAL") == 0)
				transform_mask |= PR_ALIGN_NORMAL;
			    else
			    if (strcmp(transform, "AZIMUTH") == 0)
				transform_mask |= PR_ALIGN_AZIMUTH;
			}
			else
			    fprintf (stderr, "#### Bad Triangle cmd: %s\n", 
									line);
		    }
		}


		if (is_model)
		{
		    /*
		     *		If we have amodel - load it and extract its
		     *		geometry into a single GeoSet.
		     */
		    model = pfdLoadFile(model_filename);

		    if (texture_filename[0] != '\0')
		    	mask = PR_QUERY_TRI_COORD | PR_QUERY_TRI_TEXTURE;
		    else
			mask = PR_QUERY_TRI_COORD;

		    gset = pfNewGSet (pfGetSharedArena());
		    pfdExtractGraphTriangles (model, gset, mask);

		    addTerrainGeoSet (triangleList, 
					gset, base_p, down_p, azimuth_p, 
					projection_p, texture_filename, 
					c_p, mask, 
					transform_mask);
		}
		else
		{
		    /*
		     *		If we have an explicit list of triangles, 
		     *		compose into a GeoSet.
		     */
		    nof_triangles = v_index / 3;

		    mask = PR_QUERY_TRI_COORD;
		    if (t_index >= nof_triangles * 3)
			mask |= PR_QUERY_TRI_TEXTURE;

		    if (c_index >= nof_triangles * 3)
			mask |= PR_QUERY_TRI_COLOR;

		    if ((mask & PR_QUERY_TRI_TEXTURE) && 
			(texture_filename[0] == '\0'))
		    {
			fprintf (stderr, 
			    "#####  No shadow texture filename specified.\n");
			mask &= (~ PR_QUERY_TRI_TEXTURE);
		    }

		    gset = pfNewGSet (pfGetSharedArena());
		    composeGset (gset, 
				    (float *) v, 
				    (mask & PR_QUERY_TRI_TEXTURE) ? t_p : NULL, 
				    (mask & PR_QUERY_TRI_COLOR) ? c_p : NULL, 
				    NULL, 
				    nof_triangles);


		    addTerrainGeoSet (triangleList, 
					gset, base_p, down_p, azimuth_p, 
					projection_p, texture_filename, 
					c_p, mask, 
					transform_mask);

		}
		break;
	}
    }

    fclose (fp);
}

/*===========================================================================*/
void	initLightPointSetList (lightPointSetList *lightPoints)
/*===========================================================================*/
{
bzero (lightPoints, sizeof (lightPointSetList));
}

/*===========================================================================*/
void	addLightPoint (lightPointSetList *lightPoints, 
			int	setId, 
			int	pointId, 
			float	x, 
			float	y, 
			float	z)
/*===========================================================================*/
{
    int		new;
    lightPointSet	*set;

    if (lightPoints -> nofAllocatedSets <= setId)
    {
	new = (lightPoints -> nofAllocatedSets + 2) * 5 / 3;

	lightPoints -> sets = (lightPointSet *) realloc (lightPoints -> sets, 
					new * sizeof (lightPointSet));

	bzero (& lightPoints -> sets[lightPoints -> nofAllocatedSets], 
		(new - lightPoints -> nofAllocatedSets) 
		* sizeof (lightPointSet));

	lightPoints -> nofAllocatedSets = new;
    }

    if (lightPoints -> nofSets <= setId)
	lightPoints -> nofSets = setId + 1;

    set = & lightPoints -> sets[setId];

    if (set -> nofAllocatedPoints <= pointId)
    {
	new = (set -> nofAllocatedPoints + 2) * 5 / 3;

	set -> coord = (float *) realloc (set -> coord, 
						new * 3 * sizeof (float));

	set -> color = (float *) realloc (set -> color, 
						new * 4 * sizeof (float));
	set -> nofAllocatedPoints = new;
    }

    if (set -> nofPoints <= pointId)
	set -> nofPoints = pointId + 1;

    set -> isActive = 1;
    set -> coord[3 * pointId] = x;
    set -> coord[3 * pointId + 1] = y;
    set -> coord[3 * pointId + 2] = z;

    set -> color[4 * pointId    ] = 1.0;
    set -> color[4 * pointId + 1] = 1.0;
    set -> color[4 * pointId + 2] = 1.0;
    set -> color[4 * pointId + 3] = 1.0;
}

/*===========================================================================*/
void	initTerrainObjectList (terrainObjectList *objectList)
/*===========================================================================*/
{
    bzero (objectList, sizeof (terrainObjectList));
}

/*===========================================================================*/
void	addTerrainObject (
			terrainObjectList 	*objectList, 
			float			x, 
			float			y, 
			float			z, 
			float			*down,
			float			*xy_direction,
			char			*filename,
			int			is_normal,
			int			is_azimuth)
/*===========================================================================*/
{
    int		new;
    terrainObject	*object;
    float		mag, mag2;

    if (objectList -> nofAllocatedObjects <= objectList -> nofObjects)
    {
	new = (objectList -> nofAllocatedObjects + 2) * 5 / 3;

	objectList -> object = (terrainObject *) realloc (objectList -> object, 
					new * sizeof (terrainObject));

	bzero (& objectList -> object[objectList -> nofAllocatedObjects], 
		(new - objectList -> nofAllocatedObjects) 
		* sizeof (terrainObject));

	objectList -> nofAllocatedObjects = new;
    }

    object = & (objectList -> object[objectList -> nofObjects]);

    objectList -> nofObjects ++;

    object -> x = x;
    object -> y = y;
    object -> z = z;

    mag2 = down[0] * down[0] + down[1] * down[1] + down[2] * down[2];
    if (fabs (mag2 - 1.0) > 0.001)
    {
        mag = sqrtf(mag2);
	object -> down[0] = down[0] / mag;
	object -> down[1] = down[1] / mag;
	object -> down[2] = down[2] / mag;
    }
    else
    {
	object -> down[0] = down[0];
	object -> down[1] = down[1];
	object -> down[2] = down[2];
    }

    mag2 = xy_direction[0] * xy_direction[0] 
	 + xy_direction[1] * xy_direction[1] 
	 + xy_direction[2] * xy_direction[2];

    if (fabs (mag2 - 1.0) > 0.001)
    {
        mag = sqrtf(mag2);
	object -> xy_direction[0] = xy_direction[0] / mag;
	object -> xy_direction[1] = xy_direction[1] / mag;
	object -> xy_direction[2] = xy_direction[2] / mag;
    }
    else
    {
	object -> xy_direction[0] = xy_direction[0];
	object -> xy_direction[1] = xy_direction[1];
	object -> xy_direction[2] = xy_direction[2];
    }

    strcpy (object -> filename, filename);
    object -> is_normal = is_normal;
    object -> is_azimuth = is_azimuth;
    object -> is_billboard = 0;
}

/*===========================================================================*/
void	addTerrainBillboard (
			terrainObjectList 	*objectList, 
			float			x, 
			float			y, 
			float			z, 
			float			*down,
			char			*filename,
			float			width,
			float			height)
/*===========================================================================*/
{
    int		new;
    terrainObject	*object;
    float		mag, mag2;

    if (objectList -> nofAllocatedObjects <= objectList -> nofObjects)
    {
	new = (objectList -> nofAllocatedObjects + 2) * 5 / 3;

	objectList -> object = (terrainObject *) realloc (objectList -> object, 
					new * sizeof (terrainObject));

	bzero (& objectList -> object[objectList -> nofAllocatedObjects], 
		(new - objectList -> nofAllocatedObjects) 
		* sizeof (terrainObject));

	objectList -> nofAllocatedObjects = new;
    }

    object = & (objectList -> object[objectList -> nofObjects]);

    objectList -> nofObjects ++;

    object -> x = x;
    object -> y = y;
    object -> z = z;

    mag2 = down[0] * down[0] + down[1] * down[1] + down[2] * down[2];
    if (fabs (mag2 - 1.0) > 0.001)
    {
        mag = sqrtf(mag2);
	object -> down[0] = down[0] / mag;
	object -> down[1] = down[1] / mag;
	object -> down[2] = down[2] / mag;
    }
    else
    {
	object -> down[0] = down[0];
	object -> down[1] = down[1];
	object -> down[2] = down[2];
    }

    strcpy (object -> filename, filename);
    object -> is_billboard = 1;
    object -> width = width;
    object -> height = height;
}

/*===========================================================================*/
void	initTerrainSegmentList (terrainSegmentList *segments)
/*===========================================================================*/
{
bzero (segments, sizeof (terrainSegmentList));
}

/*===========================================================================*/
void	addTerrainSegment (terrainSegmentList *segmentList, 
			float		*v0, 
			float		*v1) 
/*===========================================================================*/
{
    int			new;
    terrainSegment	*segment;
    float		mag, mag2;

    if (segmentList -> nofAllocatedSegments <= segmentList -> nofSegments)
    {
	new = (segmentList -> nofAllocatedSegments + 2) * 5 / 3;

	segmentList -> segment = (terrainSegment *) 
					realloc (segmentList -> segment, 
					new * sizeof (terrainSegment));

	bzero (& segmentList -> segment[segmentList -> nofAllocatedSegments], 
		(new - segmentList -> nofAllocatedSegments) 
		* sizeof (terrainSegment));

	segmentList -> nofAllocatedSegments = new;
    }

    segment = & (segmentList -> segment[segmentList -> nofSegments]);

    segmentList -> nofSegments ++;

    segment -> v0[0] = v0[0];
    segment -> v0[1] = v0[1];
    segment -> v0[2] = v0[2];

    segment -> v1[0] = v1[0];
    segment -> v1[1] = v1[1];
    segment -> v1[2] = v1[2];
}

/*==========================================================================*/
int 	flyIsectFunc (pfHit *hit)
/*==========================================================================*/
{
    int		faceIndex;
    int		segmentIndex;

    pfQueryHit(hit, PFQHIT_FACEINDEX, &faceIndex);
    pfQueryHit(hit, PFQHIT_SEGNUM, &segmentIndex);

    fprintf (stderr, "Segment %d hits Face %d\n", segmentIndex, faceIndex);

    return PFTRAV_CONT;
}

/*===========================================================================*/
void	initTerrainTriangleList (terrainTriangleList *triangles)
/*===========================================================================*/
{
bzero (triangles, sizeof (terrainTriangleList));
}

/*===========================================================================*/
void addTerrainGeoSet (
		    terrainTriangleList *triangleList,
		    pfGeoSet 		*gset, 
		    float		*base, 
		    float		*down, 
		    float		*azimuth, 
		    float		*projection, 
		    char		*texture_filename, 
		    float		*c,
		    unsigned long	mask, 
		    unsigned long 	transform_mask)
/*===========================================================================*/
{
    int			new;
    terrainTriangle	*triangle;
    float		mag, mag2;
    int			i;
    ushort		*dummy_ushort;
    float		*v;

    if (triangleList -> nofAllocatedTriangles <= triangleList -> nofTriangles)
    {
	new = (triangleList -> nofAllocatedTriangles + 2) * 5 / 3;

	triangleList -> triangle = (terrainTriangle *) 
					realloc (triangleList -> triangle, 
					new * sizeof (terrainTriangle));

	bzero (& triangleList -> 
		triangle[triangleList -> nofAllocatedTriangles], 
		(new - triangleList -> nofAllocatedTriangles) 
		* sizeof (terrainTriangle));

	triangleList -> nofAllocatedTriangles = new;
    }

    triangle = & (triangleList -> triangle[triangleList -> nofTriangles]);

    triangleList -> nofTriangles ++;

    triangle -> mask = mask;
    triangle -> transformation_mask = transform_mask;
    triangle -> gset = gset;

    triangle -> type = TYPE_GSET;

    if (c) /* If we have color - grab it anyway (ignore mask) */
    {
	triangle -> c[0][0] = c[0];
	triangle -> c[0][1] = c[1];
	triangle -> c[0][2] = c[2];
	triangle -> c[0][3] = c[3];
    }
    else
    {
	triangle -> c[0][0] = 0.0;
	triangle -> c[0][1] = 0.0;
	triangle -> c[0][2] = 0.0;
	triangle -> c[0][3] = 0.6;
    }

    if (base)
    {
	triangle -> base[0] = base[0];
	triangle -> base[1] = base[1];
	triangle -> base[2] = base[2];
    }
    else
    {
	pfGetGSetAttrLists (gset, PFGS_COORD3, (void **) &v, &dummy_ushort);
	if (v)
	{
	    triangle -> base[0] = v[0];
	    triangle -> base[1] = v[1];
	    triangle -> base[2] = v[2];
	}
	else
	{
	    triangle -> base[0] = 0.0;
	    triangle -> base[1] = 0.0;
	    triangle -> base[2] = 0.0;
	}
    }

    if (down)
    {
	triangle -> down[0] = down[0];
	triangle -> down[1] = down[1];
	triangle -> down[2] = down[2];
    }
    else
    {
	triangle -> down[0] = 0.0;
	triangle -> down[1] = 0.0;
	triangle -> down[2] = -1.0;
    }

    if (azimuth)
    {
	triangle -> azimuth[0] = azimuth[0];
	triangle -> azimuth[1] = azimuth[1];
	triangle -> azimuth[2] = azimuth[2];
    }
    else
    {
	triangle -> azimuth[0] = 1.0;
	triangle -> azimuth[1] = 0.0;
	triangle -> azimuth[2] = 0.0;
    }

    if (projection)
    {
	triangle -> projection[0] = projection[0];
	triangle -> projection[1] = projection[1];
	triangle -> projection[2] = projection[2];
    }
    else
    {
	triangle -> projection[0] = 0.0;
	triangle -> projection[1] = 1.0;
	triangle -> projection[2] = -1.0;
    }

    strcpy (triangle -> texture_filename, texture_filename);
}


/*===========================================================================*/
void	initTerrainAnimationList (terrainAnimationList *animations)
/*===========================================================================*/
{
bzero (animations, sizeof (terrainAnimationList));
}

/*===========================================================================*/
void	addTerrainAnimation (terrainAnimationList 	*animationList, 
					float		x, 
					float		y, 
					float 		z, 
					float		*down, 
					float 		radius, 
					float 		d_angle, 
					char		*name)
/*===========================================================================*/
{
    int			new;
    terrainAnimation	*animation;

    if (animationList -> nofAllocatedAnimations <= 
	animationList -> nofAnimations)
    {
	new = (animationList -> nofAllocatedAnimations + 2) * 5 / 3;

	animationList -> animation = (terrainAnimation *) 
					realloc (animationList -> animation, 
					new * sizeof (terrainAnimation));

	bzero (& animationList -> 
		animation[animationList -> nofAllocatedAnimations], 
		(new - animationList -> nofAllocatedAnimations) 
		* sizeof (terrainAnimation));

	animationList -> nofAllocatedAnimations = new;
    }

    animation = & (animationList -> animation[animationList -> nofAnimations]);

    animationList -> nofAnimations ++;

    animation -> center[0] = x;
    animation -> center[1] = y;
    animation -> center[2] = z;

    animation -> down[0] = down[0];
    animation -> down[1] = down[1];
    animation -> down[2] = down[2];

    strcpy (animation -> model_filename, name);

    animation -> radius = radius;
    animation -> d_angle = d_angle;

    animation -> angle = ((float) (random() % 1000)) * M_PI * 2.0 / 1000.0 ;
}

/*===========================================================================*/
void	advanceAnimations (void)
/*===========================================================================*/
{
    int		i;

    for (i = 0 ; i < animations . nofAnimations ; i ++)
	advanceAnimation (&animations . animation[i]);
}

/*===========================================================================*/
void advanceAnimation (terrainAnimation *animation)
/*===========================================================================*/
{
    float	*azimuth;
    float	position[3];

    /*
     *	=============================================
     *	Advance angle around center of motion
     *	=============================================
     */
    animation -> angle += animation -> d_angle;
    if (animation -> angle > 2.0 * M_PI)
	animation -> angle -= 2.0 * M_PI;

    /*
     *	=============================================
     *	Set new position. 
     *	=============================================
     */
    position[0] = animation -> center[0] + 
			animation -> radius * cos (animation -> angle);
    position[1] = animation -> center[1] + 
			animation -> radius * sin (animation -> angle);
    position[2] = animation -> center[2];

    pfASDQueryArrayElement (animation -> asd, animation -> query_id, 
				    0, position, animation -> down);

    /*
     *	=============================================
     *	Set new azimuth. 
     *	=============================================
     */
    azimuth = (float *) pfGetFluxWritableData (animation -> azimuth_flux);

    azimuth[0] = cos (animation -> angle + M_PI * 0.5);
    azimuth[1] = sin (animation -> angle + M_PI * 0.5);

    pfFluxWriteComplete (animation -> azimuth_flux);
}

/*===========================================================================*/
void   composeGset (
		pfGeoSet		*gset, 
		float 			*_v, 
		float 			*_t, 
		float 			*_c, 
		float 			*_n, 
		int			nof_triangles)
/*===========================================================================*/
{
    float	*v, *t, *n, *c;

    if (_v)
    {
	v = (float *) pfMalloc (nof_triangles * 3 * 3 * sizeof (float), 
						    pfGetSharedArena());
	memcpy (v, _v, nof_triangles * 3 * 3 * sizeof (float));
        pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, (void *) v, NULL);
    }
    else
    {
	fprintf (stderr, "composeGset: no COORD3 !\n");
        pfGSetAttr(gset, PFGS_COORD3, PFGS_OFF, NULL, NULL);
    }

    if (_t)
    {
	t = (float *) pfMalloc (nof_triangles * 2 * 3 * sizeof (float), 
						    pfGetSharedArena());
	memcpy (t, _t, nof_triangles * 2 * 3 * sizeof (float));
        pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, (void *) t, NULL);
    }
    else
        pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_OFF, NULL, NULL);

    if (_c)
    {
	c = (float *) pfMalloc (nof_triangles * 4 * 3 * sizeof (float), 
						    pfGetSharedArena());
	memcpy (c, _c, nof_triangles * 4 * 3 * sizeof (float));
        pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, (void *) c, NULL);
    }
    else
        pfGSetAttr(gset, PFGS_COLOR4, PFGS_OFF, NULL, NULL);

    if (_n)
    {
	n = (float *) pfMalloc (nof_triangles * 3 * 3 * sizeof (float), 
						    pfGetSharedArena());
	memcpy (n, _n, nof_triangles * 3 * 3 * sizeof (float));
        pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, (void *) n, NULL);
    }
    else
        pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OFF, NULL, NULL);

    pfGSetNumPrims(gset, nof_triangles);

    pfGSetPrimType (gset, PFGS_TRIS);
}
