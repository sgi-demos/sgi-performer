#ifndef FLY_LIGHT_POINTS_H
#define	FLY_LIGHT_POINTS_H

#define	MAX_POINTS	48

enum 
{
    TYPE_GSET,
    TYPE_ARRAYS
};

typedef struct
	{
	int		nofPoints;
	int		nofAllocatedPoints;
	int		isActive;

	float		*coord;
	float		*color;
	float		*normal;
	} lightPointSet;

typedef struct
	{
	int		nofSets;
	int		nofAllocatedSets;
	lightPointSet	*sets;
 	} lightPointSetList;

typedef struct
	{
	int		is_normal;
	int		is_azimuth;
	float		x, y, z;
        float		down[3];
        float		xy_direction[3];
	char		filename[200];
	float		width, height;
	int		is_billboard;
	} terrainObject;

typedef struct
	{
	int			nofObjects;
	int			nofAllocatedObjects;
	terrainObject		*object;
 	} terrainObjectList;

typedef struct
	{
	float			v0[3], v1[3];
	} terrainSegment;

typedef struct
	{
	int			nofSegments;
	int			nofAllocatedSegments;
	terrainSegment		*segment;
 	} terrainSegmentList;

typedef struct
	{
	int		type;

	pfGeoSet	*gset;

	float		v[MAX_POINTS][3]; /* Vertex  */
	float		t[MAX_POINTS][2]; /* Texture */
	float		c[MAX_POINTS][4]; /* Color   */

	int		nof_triangles;
	unsigned long	mask;

	float		base[3]; /* Align to this */
        float		down[3];
        float		azimuth[3];
        float		projection[3];

        char		texture_filename[300];

	unsigned long	transformation_mask;
	} terrainTriangle;

typedef struct
	{
	int			nofTriangles;
	int			nofAllocatedTriangles;
	terrainTriangle		*triangle;
 	} terrainTriangleList;

typedef struct
	{
	float		center[3];
	float		down[3];
	float		radius;
	float		d_angle;
	char		model_filename[300];

	/* 
	 *	for use by animation code.
	 */
	pfASD		*asd;
	int		query_id;
	float		angle;
	pfFlux		*azimuth_flux;
	
	} terrainAnimation;

typedef struct
	{
	int			nofAnimations;
	int			nofAllocatedAnimations;
	terrainAnimation	*animation;
 	} terrainAnimationList;

void 	flyLightsInit(pfASD *asd);

void    advanceAnimations (void);

void 	advanceAnimation (terrainAnimation *animation);

#endif
