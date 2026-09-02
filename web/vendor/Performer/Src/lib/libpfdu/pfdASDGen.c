/*
 * Copyright 2000, Silicon Graphics, Inc.
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
 *	====================================================================
 *	pfdASDGen - routines for generating a paging pfASD.
 *	====================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#ifndef WIN32
#include <strings.h>
#include <sys/resource.h>
#include <alloca.h>
#endif

#define FRACT_DEBUG		0

#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pfdu.h>

#include "pfdASDGen.h"

#define distance_2(x1,y1,z1,x2,y2,z2) (((x2) - (x1)) * ((x2) - (x1)) \
                                     + ((y2) - (y1)) * ((y2) - (y1)) \
                                     + ((z2) - (z1)) * ((z2) - (z1)))



/*===========================================================================*/
PFDUDLLEXPORT pfdASDGenerator *pfdNewASDGen (void)
/*===========================================================================*/
{
    pfdASDGenerator	*gen;
    char		*t;

    gen = (pfdASDGenerator *) calloc (1, sizeof (pfdASDGenerator));

    gen -> x_grid_size = 100;
    gen -> y_grid_size = 100;

    gen -> tile_size = 11;

    gen -> asd_geometry = ASDGEN_PLANE;
    gen -> x_span = 10000.0;
    gen -> y_span = 10000.0;
    gen -> x_base = 0.0;
    gen -> y_base = 0.0;

    gen -> enable_prune = 0;
    gen -> prune_epsilon = 0.01;

    gen -> tile_lookahead [0] = 3;
    gen -> tile_lookahead [1] = 3;

    gen -> nof_levels = 5;

    gen -> grid_epsilon = 0.001;

    /* 
     *	If not specified otherwise, grid is a sinus.
     */
    gen -> user_data = NULL;
    gen -> get_elevation = default_grid_elevation;

    if (t = getenv ("TMPDIR"))
	sprintf (gen -> temp_base_filename, "%s/new_asd_temp", t);
    else
	sprintf (gen -> temp_base_filename, "/usr/tmp/new_asd_temp");

    sprintf (gen -> base_filename, "tmp/new_asd");


    init_constraints (& gen -> constraints);

    return (gen);
}

/*===========================================================================*/
PFDUDLLEXPORT void pfdASDGenerate (pfdASDGenerator *gen)
/*===========================================================================*/
{
    int			i, j;
    ASD_tile		tile;
    int			x_nof_tiles_across, y_nof_tiles_across;
    grid3       	grid;
    ASD_tile_stats	*tile_stats;
    int			virgin_box;
    pfBox		asd_box;
    char		cfg_filename[1000], ranges_filename[1000];


    /*
     *	=====================================================================
     *	Subdivide final fractal mesh into tiles, and write each one in the 
     *	input format of `convasd'
     *
     *	Write each tile independently into a set if files: one for each LOD 
     *	=====================================================================
     */

    init_asd_tile (&tile);


    if (gen -> x_wraparound)
	x_nof_tiles_across = gen -> x_grid_size / (gen -> tile_size - 1);
    else
	x_nof_tiles_across = (gen -> x_grid_size - 1) / (gen -> tile_size - 1);

    y_nof_tiles_across = (gen -> y_grid_size - 1) / (gen -> tile_size - 1);

    tile_stats = init_asd_tile_stats (x_nof_tiles_across * y_nof_tiles_across, 
				    gen -> nof_levels);

    printf ("Will process %dx%d tiles\n", 
			x_nof_tiles_across, y_nof_tiles_across);

    if (x_nof_tiles_across * (gen -> tile_size - 1) + 1 != gen -> x_grid_size)
	printf ("### Will only use %d grid posts on X axis (%d available).\n", 
		x_nof_tiles_across * (gen -> tile_size - 1) + 1, 
		gen -> x_grid_size);

    if (y_nof_tiles_across * (gen -> tile_size - 1) + 1 != gen -> y_grid_size)
	printf ("### Will only use %d grid posts on Y axis (%d available).\n", 
		y_nof_tiles_across * (gen -> tile_size - 1) + 1,
		gen -> y_grid_size);

    grid . points = (pfVec3 *) malloc 
			(gen -> tile_size * gen -> tile_size * sizeof (pfVec3));

    grid . x_size = gen -> tile_size;
    grid . y_size = gen -> tile_size;
    grid . x_scale = gen -> x_span / (float) gen -> x_grid_size;
    grid . y_scale = gen -> y_span / (float) gen -> y_grid_size;


    calc_asd_tile_info (& tile . info, 
			x_nof_tiles_across, 
			y_nof_tiles_across, 
			gen -> nof_levels);

    /*
     *	Note: We only cover full tiles. We ignore any extra points on 
     *	the orignal grid.
     */

    virgin_box = 1;
    for (i = 0 ; i < x_nof_tiles_across ; i ++)
	for (j = 0 ; j < y_nof_tiles_across ; j ++)
	{
	extract_subgrid (
		gen, 
		i * (gen -> tile_size - 1), 
		j * (gen -> tile_size - 1), & grid);

	grid . x_base = i * (gen -> tile_size - 1) * grid . x_scale;
	grid . y_base = j * (gen -> tile_size - 1) * grid . y_scale;

	tile . info . x = i;
	tile . info . y = j;

        if (gen -> asd_geometry == ASDGEN_PLANE)
	    printf ("tile (%d, %d) ... base (%f, %f)\n", 
		i, j, grid . x_base, grid . y_base);
        else
	    printf ("sphere tile (%d, %d): grid base (%d, %d)\n", 
		i, j, i * (gen -> tile_size - 1), j * (gen -> tile_size - 1));

        grid2ASD (gen, & tile, & grid, gen -> nof_levels);

	printf ("\tCalculate bbox...\n");
	calc_tile_face_bbox (& tile);

        collect_asd_tile_stats (
			& tile_stats[j * x_nof_tiles_across + i], 
			& tile, 
			gen -> nof_levels);

    	save_tile (&tile, gen -> temp_base_filename, i, j);

	expand_asd_box (& tile, &asd_box, virgin_box);
	virgin_box = 0;
	}

    printf ("Done pass 1.\n");

    printf ("Calculate tile base ... \n");
    calc_asd_tile_stats (
		tile_stats, 
		x_nof_tiles_across * y_nof_tiles_across, 
		gen -> nof_levels);

    if (gen -> enable_prune)
    {
	printf ("Prune tiles ... \n");
	prune_tiles (
	    gen -> temp_base_filename, 
	    gen -> nof_levels, 
	    x_nof_tiles_across, 
	    y_nof_tiles_across, 
	    gen -> prune_epsilon);
    }

/*
 *	===========================================================
 *	Collect all level # 0 faces/vertices from tile files
 *
 *	Starting at highest resolution tiles, collect faces to 
 *	make final tiles. 
 *	Let N be highest level of detail:
 *	1 tile of level N --> one final ASD tile.
 *	4 tiles of level (N-1) --> one final ASD tile. 
 *	16 tiles of level (N-2) --> one final ASD tile. 
 *	...
 *	
 *	===========================================================
 */
    printf ("Rearrange Tiles ... (%d x %d) tiles\n", 
			x_nof_tiles_across, y_nof_tiles_across);

    sprintf (cfg_filename, "%s.cfg", gen -> base_filename);
    rearrange_tiles (
		gen -> temp_base_filename, 
		gen -> base_filename, 
		gen -> nof_levels + 1, 
		x_nof_tiles_across, 
		y_nof_tiles_across, 
		& asd_box, 
		cfg_filename);

    sprintf (ranges_filename, "%s.ranges", gen -> base_filename);
    pfdPagingLookahead (cfg_filename, ranges_filename, gen -> tile_lookahead);

}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenPlaneGeometry (
			pfdASDGenerator 	*gen, 
			float 			x_base, 
			float 			y_base, 
			float 			x_span, 
			float 			y_span)
/*============================================================================*/
{
    gen -> asd_geometry = ASDGEN_PLANE;
    gen -> x_base = x_base;
    gen -> y_base = y_base;
    gen -> x_span = x_span;
    gen -> y_span = y_span;

    gen -> x_wraparound = 0;
    gen -> y_combine = 0;
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenSphereGeometry (
			pfdASDGenerator 	*gen, 
			float			radius,
			PFVEC3			center, 
			float 			x_base, 
			float 			y_base, 
			float 			x_span, 
			float 			y_span)
/*============================================================================*/
{
    gen -> asd_geometry = ASDGEN_SPHERE;
    gen -> x_base = x_base;
    gen -> y_base = y_base;
    gen -> x_span = x_span;
    gen -> y_span = y_span;
    gen -> sphere_radius = radius;
    pfCopyVec3 (gen -> sphere_center, center);

    if (x_span >= 359.99)
	gen -> x_wraparound = 1;
    else
	gen -> x_wraparound = 0;

    if (y_span >= 179.99)
	gen -> y_combine = 1;
    else
	gen -> y_combine = 0;
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenElevationFunc (
			pfdASDGenerator 	*gen, 
			pfd_grid_elevation_func	func, 
			void			*user_data)
/*============================================================================*/
{
    gen -> get_elevation = func;
    gen -> user_data = user_data;
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenPrune (
			pfdASDGenerator 	*gen, 
			int			mode, 
			float			epsilon)
/*============================================================================*/
{
    gen -> enable_prune = mode;
    gen -> prune_epsilon = epsilon;
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenGridSize (
			pfdASDGenerator 	*gen, 
			int			x_size, 
			int			y_size)
/*============================================================================*/
{
    gen -> x_grid_size = x_size;
    gen -> y_grid_size = y_size;
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenTileSize (
			pfdASDGenerator 	*gen, 
			int			size)
/*============================================================================*/
{
    gen -> tile_size = size;
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenNofLevels (
			pfdASDGenerator 	*gen, 
			int			n)
/*============================================================================*/
{
    gen -> nof_levels = n;
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenTempFile (
			pfdASDGenerator 	*gen, 
			char			*tmp)
/*============================================================================*/
{
    strcpy (gen -> temp_base_filename, tmp);
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenOutputFile (
			pfdASDGenerator 	*gen, 
			char			*out)
/*============================================================================*/
{
    strcpy (gen -> base_filename, out);
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenLookahead (
			pfdASDGenerator 	*gen, 
			int			x, 
			int			y)
/*============================================================================*/
{
    gen -> tile_lookahead[0] = x;
    gen -> tile_lookahead[1] = y;
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenAddSegment (
			pfdASDGenerator 	*gen, 
			PFVEC3			v0,
			PFVEC3			v1)
/*============================================================================*/
{
    int			num;
    ASD_segments	*seg;

    seg = & gen -> constraints . segments;

    if (seg -> nof_allocated_segments <= seg -> nof_segments)
    {
	num = (seg -> nof_allocated_segments + 2) * 4 / 3;
	seg -> segment = (ASD_segment *) 
			realloc (seg -> segment, num * sizeof (ASD_segment));
	seg -> nof_allocated_segments = num;
    }

    pfCopyVec3(seg -> segment[seg -> nof_segments] . v[0], v0);
    pfCopyVec3(seg -> segment[seg -> nof_segments] . v[1], v1);
    seg -> nof_segments ++;
}

/*============================================================================*/
PFDUDLLEXPORT
void  pfdASDGenAddTriangle (
			pfdASDGenerator 	*gen, 
			PFVEC3			v0,
			PFVEC3			v1,
			PFVEC3			v2)
/*============================================================================*/
{
    int                 num;
    ASD_triangles        *tri;

    tri = & gen -> constraints . triangles;

    if (tri -> nof_allocated_triangles <= tri -> nof_triangles)
    {
        num = (tri -> nof_allocated_triangles + 2) * 4 / 3;
        tri -> triangle = (ASD_triangle *)
                        realloc (tri -> triangle, num * sizeof (ASD_triangle));
        tri -> nof_allocated_triangles = num;
    }

    pfCopyVec3(tri -> triangle[tri -> nof_triangles] . v[0], v0);
    pfCopyVec3(tri -> triangle[tri -> nof_triangles] . v[1], v1);
    pfCopyVec3(tri -> triangle[tri -> nof_triangles] . v[2], v2);
    tri -> nof_triangles ++;
}


/*============================================================================*/
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                      Internal Routines (Not API)                           */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*============================================================================*/



/*============================================================================*/
void select_split_point_1D (section3 *section, pfVec3 result, float *t)
/*============================================================================*/
{
    int		i, j;
    int		min_i;
    float	min_error,max_error;
    int		virgin;
    float	dist;
    float	sum_error;
    int		from, to;

#if 0
    from = 1;
    to = section -> nof_points -1;
#else

    from = section -> nof_points / 3;
    to = 2 * section -> nof_points / 3;
    if (from >= to)
    {
	printf ("select_split_point_1D: from = %d, to = %d\n", from , to);
	from = 1;
	to = section -> nof_points -1;
    }
    if (from < 1) from = 1;
    if (to > section -> nof_points -1) to = section -> nof_points -1;

#endif

    virgin = 1;
    for (i = from ; i < to ; i ++)
    {
	/*
	 *	Check error against each point in array. Keep maximum.
	 */
	max_error = -1.0;
        sum_error = 0.0;

	for (j = 1 ; j < i ; j ++)
	{
	    dist = calc_point_line_distance_2 (
				section -> points[j], 
				section -> points[0], 
				section -> points[i]);

	    if (dist > max_error)
		max_error = dist;

	    sum_error += dist;
	}

	for (j = (i+1) ; j < (section -> nof_points - 1) ; j ++)
	{
	    dist = calc_point_line_distance_2 (
				section -> points[j], 
				section -> points[i], 
				section -> points[section -> nof_points - 1]);

	    if (dist > max_error)
		max_error = dist;

	    sum_error += dist;
	}

#if 0
	if (virgin || (max_error < min_error))
	{
	    min_i = i;
	    min_error = max_error;
	    virgin = 0;
	}
#else
	/*
  	 *	Pick minimum among sums of all distnces.
	 */
	if (virgin || (sum_error < min_error))
	{
	    min_i = i;
	    min_error = sum_error;
	    virgin = 0;
	}
#endif
    }

    /*
     *	If edge is completely flat - take middle point.
     */
    if (min_error <= 0.0000001)
    {
	min_i = section -> nof_points / 2;
	if (min_i < 1)
	    min_i = 1;
    }

    pfCopyVec3 (result, section -> points[min_i]);

    *t = ((float) min_i) / ((float) (section -> nof_points -1));
}

/*============================================================================*/
void    calculate_section_midpoint (
			section3 *section, float t, pfVec3 midpoint)
/*============================================================================*/
{
    pfCombineVec3 (midpoint, 1 - t, section -> points[0], 
			     t, section -> points[section -> nof_points - 1]);
}

/*============================================================================*/
void calc_point_2D (grid3 *grid, float x, float y, pfVec3 result)
/*============================================================================*/
{
    float	ax, ay, cx, cy;
    int	ix, iy;
    pfVec3  *v00, *v01, *v10, *v11;
    float   EPS;
    pfVec3 dummy;

    EPS = 0.001;
    dummy [0] = 0.0;
    dummy [1] = 0.0;
    dummy [2] = 0.0;

    if ((x + EPS < 0.0) || (x - EPS >= grid -> x_size))
	printf ("calc_point_2D: X Point out of bound (%f, %f)\n", x, y);

    if ((y + EPS < 0.0) || (y - EPS >= grid -> y_size))
	printf ("calc_point_2D: Y Point out of bound (%f, %f)\n", x, y);

    if (x >= grid -> x_size) x = grid -> x_size - 1.0;
    if (y >= grid -> y_size) y = grid -> y_size - 1.0;
    if (x < 0.0) x = 0.0;
    if (y < 0.0) y = 0.0;

    ix = (int) x;
    iy = (int) y;
    ax = x - ix;
    ay = y - iy;

    cx = 1.0 - ax;
    cy = 1.0 - ay;

    if ((ix >= grid -> x_size) || (ix < 0))
	printf ("calc_point_2D: ix out of bounds (%d)\n", ix);

    if ((iy >= grid -> y_size) || (iy < 0))
	printf ("calc_point_2D: iy out of bounds (%d)\n", iy);

    v00 = &(grid -> points [ iy      * grid -> x_size + ix]);

    if (iy + 1 >= grid -> y_size)
	v01 = & dummy;
    else
	v01 = &(grid -> points [(iy + 1) * grid -> x_size + ix]);

    if (ix + 1 >= grid -> x_size)
	v10 = & dummy;
    else
	v10 = &(grid -> points [ iy      * grid -> x_size + ix + 1]);

    if ((ix + 1 >= grid -> x_size) || (iy + 1 >= grid -> y_size))
	v11 = & dummy;
    else
	v11 = &(grid -> points [(iy + 1) * grid -> x_size + ix + 1]);

    /*
     *	Scale X and Y by grid spacing.
     */
    result [0] =  ((*v00) [0] * cx * cy + 
		   (*v01) [0] * cx * ay + 
		   (*v10) [0] * ax * cy + 
		   (*v11) [0] * ax * ay);

    result [1] =  ((*v00) [1] * cx * cy + 
		   (*v01) [1] * cx * ay + 
		   (*v10) [1] * ax * cy + 
		   (*v11) [1] * ax * ay);

    result [2] =  ((*v00) [2] * cx * cy + 
		   (*v01) [2] * cx * ay + 
		   (*v10) [2] * ax * cy + 
		   (*v11) [2] * ax * ay);
}

/*============================================================================*/
void compute_section_1D (
			grid3 		*grid, 
			section3 	*section, 
			float 		from_x, 
			float 		from_y, 
			float 		to_x, 
			float 		to_y) 
/*============================================================================*/
{
    float	dx, dy, distance;
    float	t;
    int		i;
    int		nof_points;

    dx = to_x - from_x;
    dy = to_y - from_y;

    distance = fsqrt (dx * dx + dy * dy);
    nof_points = distance;
    if (nof_points < 9) 
	nof_points = 9;

    extend_section (section, nof_points + 5);

    section -> nof_points = nof_points;

    for (i = 0 ; i < section -> nof_points ; i ++)
    {
	t = ((float) i) / (float) (section -> nof_points - 1);

	calc_point_2D (grid, 
		from_x * (1 - t) + to_x * t, 
		from_y * (1 - t) + to_y * t, 
		&section -> points[i]);
    }
}

/*============================================================================*/
float       calc_point_line_distance_2 (
                        pfVec3                 p,
                        pfVec3                 p1,
                        pfVec3                 p2
                        )
/*============================================================================*/
{
    float   	sqr_l, l;
    float   	sqr_d1, sqr_d2;
    float   	t, d;
    float 	lx1, ly1, lz1;
    float 	lx2, ly2, lz2;
    float	px, py, pz;

    lx1 = p1 [0] ; ly1 = p1 [1]; lz1 = p1 [2];
    lx2 = p2 [0] ; ly2 = p2 [1]; lz2 = p2 [2];
    px = p [0] ; py = p [1]; pz = p [2];

    sqr_d1 = distance_2 (lx1, ly1, lz1, px, py, pz);
    sqr_d2 = distance_2 (lx2, ly2, lz2, px, py, pz);

    sqr_l = distance_2 (lx1, ly1, lz1, lx2, ly2, lz2);
    l = sqrtf (sqr_l);

    t = (sqr_l - sqr_d2 + sqr_d1) / (2.0 * l);
    d = (sqr_d1 - t * t);

    return (d); /* This is the square of the distance */
}


/*============================================================================*/
void init_section (section3 *section)
/*============================================================================*/
{
    section -> points = NULL;
    section -> nof_points = 0;
    section -> nof_allocated_points = 0;
}

/*============================================================================*/
void extend_section (section3 *section, int n)
/*============================================================================*/
{
    if (section -> nof_allocated_points >= n)
	return;

    section -> points = (pfVec3 *) realloc (
					section -> points, 
					n * sizeof (pfVec3));

    section -> nof_allocated_points = n;
}


/*============================================================================*/
void	grid2ASD (
		pfdASDGenerator *gen, 
		ASD_tile *tile, 
		grid3 *grid, 
		int nof_levels)
/*============================================================================*/
{
    triangle 	tri0, tri1;
    pfVec3		d;
    int		v0, v1, v2, v3;
    tstrip_info	ts_info_0, ts_info_1;
    int		*ts_base;
    int		nof_triangles, nof_columns;
    int		i;
    int		bx, by;

    if (!gen)
	printf ("grid2ASD: gen is NULL\n");

    zero_asd_tile (tile);

    /*
     *	====================================================================
     *	Calculate base tstrip ID for each level
     *	Assume a single tri-strip allocation for the entire collection of
     *	tiles. Use tile -> info to figure out where this tile lies.  
     *	====================================================================
     */
    ts_base = (int *) alloca ((nof_levels + 1) * sizeof (int));

    ts_base[0] = 2;
    nof_columns = tile -> info . x_tiles_across;
    nof_triangles = tile -> info . x_tiles_across 
		  * tile -> info . y_tiles_across * 2;

    for (i = 1 ; i <= nof_levels ; i ++)
    {
	ts_base[i] = ts_base[i - 1] + nof_triangles + nof_columns * 10;

	nof_columns = nof_columns * 2;
        nof_triangles = nof_triangles * 4;
    }

    /*
     *	====================================================================
     *	Add four corner vertices to ASD structure.
     *
     *	The diagram shows the order of the vertices and child faces:
     *
     *		V3                 V2
     *	       	---------------------
     *		|\1       |\       2|             
     *		|2\ ch#0  | \ ch#2  |              
     *		|  \      |  \      |               
     *		|   \     |   \     |                
     *		|    \    |    \    |                 
     *		|     \   |     \   |                  
     *		|      \  |      \  |                   
     *		| ch#1  \ | ch#3  \ |                    
     *		|        \|________\|                     
     *		|---------\         |                      
     *		|\  ch#3 | \ ch#1   |                       
     *		| \      |  \       |                        
     *		|  \     |   \      |                         
     *		|   \    |    \     |                          
     *		|    \   |     \    |                           
     *		|ch#0 \  | ch#2 \   |                            
     *		|      \ |       \  |                             
     *		|0      \|       1\0|                              
     *	       	---------------------                              
     *		V0                 V1
     *
     *	====================================================================
     */
    d [0] = 0.0;
    d [1] = 0.0;
    d [2] = 0.0;

    bx = tile -> info . x;
    by = tile -> info . y;

    v0 = make_asd_vertex (gen, tile, grid, 0, 
				0.0, 			0.0, 
				bx, 			by, &d, 
				PFASD_NIL_ID, PFASD_NIL_ID);

    v1 = make_asd_vertex (gen, tile, grid, 0, 
				grid -> x_size - 1.0, 	0.0, 
				bx + 1, 		by, &d, 
				PFASD_NIL_ID, PFASD_NIL_ID);

    v2 = make_asd_vertex (gen, tile, grid, 0, 
				grid -> x_size - 1.0, 	grid -> y_size - 1.0, 
				bx + 1, 		by + 1, &d, 
				PFASD_NIL_ID, PFASD_NIL_ID);

    v3 = make_asd_vertex (gen, tile, grid, 0, 
				0.0, 			grid -> y_size - 1.0, 
				bx,    	 		by + 1, &d, 
				PFASD_NIL_ID, PFASD_NIL_ID);


    /*
     *	====================================================================
     *	Top level triangle # 0, vertices in CW order.
     *	====================================================================
     */

    tri0 . grid_x[0] = 0.0;  
    tri0 . grid_y[0] = 0.0;

    tri0 . grid_x[1] = grid -> x_size - 1.0;
    tri0 . grid_y[1] = 0.0;

    tri0 . grid_x[2] = 0.0;
    tri0 . grid_y[2] = grid -> y_size - 1.0;

    tri0 . v_index[0] = v0;
    tri0 . v_index[1] = v1;
    tri0 . v_index[2] = v3;

    ts_info_0 . x = tile -> info . x;
    ts_info_0 . y = tile -> info . y;
    ts_info_0 . x_size = tile -> info . x_tiles_across;
    ts_info_0 . y_size = tile -> info . y_tiles_across;
    ts_info_0 . is_up = 0;
    ts_info_0 . ts_base = ts_base;


    /*
     *	====================================================================
     *	Top level triangle # 1, vertices in CW order.
     *	====================================================================
     */

    tri1 . grid_x[0] = 0.0;
    tri1 . grid_y[0] = grid -> y_size - 1.0;

    tri1 . grid_x[1] = grid -> x_size - 1.0;  
    tri1 . grid_y[1] = 0.0;

    tri1 . grid_x[2] = grid -> x_size - 1.0;
    tri1 . grid_y[2] = grid -> y_size - 1.0;


    tri1 . v_index[0] = v3;
    tri1 . v_index[1] = v1;
    tri1 . v_index[2] = v2;

    ts_info_1 . x = tile -> info . x;
    ts_info_1 . y = tile -> info . y;
    ts_info_1 . x_size = tile -> info . x_tiles_across;
    ts_info_1 . y_size = tile -> info . y_tiles_across;
    ts_info_1 . is_up = 1;
    ts_info_1 . ts_base = ts_base;

    /*
     *	start recursion on two main triangles.
     */

    triangle2ASD (gen, tile, grid, 0, nof_levels, &tri0, &ts_info_0);
    triangle2ASD (gen, tile, grid, 0, nof_levels, &tri1, &ts_info_1);
}

/*============================================================================*/
int triangle2ASD (pfdASDGenerator *gen, ASD_tile *tile, grid3 *grid, 
			int level, int nof_levels, triangle *tri, 
			tstrip_info *ts_info)
/*============================================================================*/
{
    static section3  	section_0, section_1, section_2;
    static int		virgin = 1;
    pfVec3 		split_0, split_1, split_2;
    triangle		tri_0, tri_1, tri_2, tri_3;
    float		t_0, t_1, t_2;
    float		x_split_0, y_split_0;
    float		x_split_1, y_split_1;
    float		x_split_2, y_split_2;
    int			v_0, v_1, v_2;
    pfVec3		midpoint_0, midpoint_1, midpoint_2;
    pfVec3		d_0, d_1, d_2;
    pfASDFace		face;
    int			child_0, child_1, child_2, child_3;
    int			face_id;
    tstrip_info		ts_info_0, ts_info_1, ts_info_2, ts_info_3;
    int			ts_x_size, ts_y_size;
    int			bx, by;

    if (virgin)
    {
	virgin = 0;
	init_section (&section_0);
	init_section (&section_1);
	init_section (&section_2);
    }

    if (level >= nof_levels)
	return (PFASD_NIL_ID);

    face_id = ts_info -> ts_base[level] + 
		(ts_info -> y * ts_info -> x_size + ts_info -> x) * 2 +
		ts_info -> is_up;

#if 0
    if (level + 1 >= nof_levels)
    {
	v_0 = PFASD_NIL_ID;
	v_1 = PFASD_NIL_ID;
	v_2 = PFASD_NIL_ID;
	child_0 = PFASD_NIL_ID;
	child_1 = PFASD_NIL_ID;
	child_2 = PFASD_NIL_ID;
	child_3 = PFASD_NIL_ID;
    }
    else
    {
#endif
	/*
	 *	=======================================================
	 *	Generate sections along triangle edges.
	 *	=======================================================
	 */
	compute_section_1D (grid, &section_0, 
			    tri -> grid_x[0], tri -> grid_y[0], 
			    tri -> grid_x[1], tri -> grid_y[1]);

	compute_section_1D (grid, &section_1, 
			    tri -> grid_x[1], tri -> grid_y[1], 
			    tri -> grid_x[2], tri -> grid_y[2]);

	compute_section_1D (grid, &section_2, 
			    tri -> grid_x[2], tri -> grid_y[2], 
			    tri -> grid_x[0], tri -> grid_y[0]);

	/* 
	 *	=======================================================
	 *	Pick best split point along each edge.
	 *	=======================================================
	 */

	select_split_point_1D (&section_0, split_0, &t_0);
	select_split_point_1D (&section_1, split_1, &t_1);
	select_split_point_1D (&section_2, split_2, &t_2);

	if ((t_0 < -0.0001) || (t_0 > 1.000001))
	    printf ("Bad select_split_point_1D: t_0 = %f\n", t_0);

	if ((t_1 < -0.0001) || (t_1 > 1.000001))
	    printf ("Bad select_split_point_1D: t_1 = %f\n", t_1);

	if ((t_2 < -0.0001) || (t_2 > 1.000001))
	    printf ("Bad select_split_point_1D: t_2 = %f\n", t_2);

	/*
	 *	=======================================================
	 *	Calculate 2D grid position of new split points.
	 *	=======================================================
	 */
	x_split_0 = tri -> grid_x[0] * (1.0 - t_0) + tri -> grid_x[1] * t_0;
	y_split_0 = tri -> grid_y[0] * (1.0 - t_0) + tri -> grid_y[1] * t_0;

	x_split_1 = tri -> grid_x[1] * (1.0 - t_1) + tri -> grid_x[2] * t_1;
	y_split_1 = tri -> grid_y[1] * (1.0 - t_1) + tri -> grid_y[2] * t_1;

	x_split_2 = tri -> grid_x[2] * (1.0 - t_2) + tri -> grid_x[0] * t_2;
	y_split_2 = tri -> grid_y[2] * (1.0 - t_2) + tri -> grid_y[0] * t_2;

	/*
	 *	=======================================================
	 *	Calculate morphing vectors for split points:
	 *	=======================================================
	 */

	calculate_section_midpoint (&section_0, t_0, &midpoint_0);
	calculate_section_midpoint (&section_1, t_1, &midpoint_1);
	calculate_section_midpoint (&section_2, t_2, &midpoint_2);

	pfSubVec3 (d_0, midpoint_0, split_0);
	pfSubVec3 (d_1, midpoint_1, split_1);
	pfSubVec3 (d_2, midpoint_2, split_2);

	/*
	 *	=======================================================
	 *	Add vertices (if non exist already) for split points
	 *	=======================================================
	 */
	bx = ts_info -> x;
	by = ts_info -> y;
	if (ts_info -> is_up)
	{
	    v_0 = make_asd_vertex 
			(gen, tile, grid, level + 1, x_split_0, y_split_0, 
			    bx * 2 + 1, by * 2 + 1, &d_0,
			    face_id, face_id - 1);

	    v_1 = make_asd_vertex 
			(gen, tile, grid, level + 1, x_split_1, y_split_1,
			    bx * 2 + 2, by * 2 + 1, &d_1,
			    face_id,
			    (ts_info -> y + 1 < ts_info -> y_size) ? 
				face_id + ts_info -> x_size * 2 - 1 : 
								PFASD_NIL_ID);

	    v_2 = make_asd_vertex 
			(gen, tile, grid, level + 1,  x_split_2, y_split_2,
			    bx * 2 + 1, by * 2 + 2, &d_2,
			    face_id,
			    (ts_info -> x + 1 < ts_info -> x_size) ? 
				face_id + 1 : PFASD_NIL_ID);
	}
	else
	{
	    v_0 = make_asd_vertex 
			(gen, tile, grid, level + 1, x_split_0, y_split_0, 
			    bx * 2 + 1, by * 2    , &d_0,
			    face_id, 
			    (ts_info -> y > 0) ? 
			    face_id - ts_info -> x_size * 2 + 1 : PFASD_NIL_ID);

	    v_1 = make_asd_vertex 
			(gen, tile, grid, level + 1, x_split_1, y_split_1,
			    bx * 2 + 1, by * 2 + 1, &d_1,
			    face_id, face_id + 1);

	    v_2 = make_asd_vertex 
			(gen, tile, grid, level + 1, x_split_2, y_split_2,
			    bx * 2    , by * 2 + 1, &d_2,
			    face_id,
			    (ts_info -> x > 0) ? face_id - 1 : PFASD_NIL_ID);
	}

#if 1
    if (level + 1 >= nof_levels)
    {
	child_0 = PFASD_NIL_ID;
	child_1 = PFASD_NIL_ID;
	child_2 = PFASD_NIL_ID;
	child_3 = PFASD_NIL_ID;
    }
    else
    {
#endif
	/* 
	 *	=======================================================
	 *	Compute child triangles.
	 *	=======================================================
	 */

	if (ts_info -> is_up)
	{
	    tri_0 . grid_x[0] = tri -> grid_x[0];
	    tri_0 . grid_y[0] = tri -> grid_y[0];

	    tri_0 . grid_x[1] = x_split_0;
	    tri_0 . grid_y[1] = y_split_0;

	    tri_0 . grid_x[2] = x_split_2;
	    tri_0 . grid_y[2] = y_split_2;

	    tri_0 . v_index[0] = tri -> v_index[0];
	    tri_0 . v_index[1] = v_0;
	    tri_0 . v_index[2] = v_2;
	}
	else
	{
	    tri_0 . grid_x[0] = tri -> grid_x[0];
	    tri_0 . grid_y[0] = tri -> grid_y[0];

	    tri_0 . grid_x[1] = x_split_0;
	    tri_0 . grid_y[1] = y_split_0;

	    tri_0 . grid_x[2] = x_split_2;
	    tri_0 . grid_y[2] = y_split_2;

	    tri_0 . v_index[0] = tri -> v_index[0];
	    tri_0 . v_index[1] = v_0;
	    tri_0 . v_index[2] = v_2;
	}


	if (ts_info -> is_up)
	{
	    tri_1 . grid_x[0] = x_split_0;
	    tri_1 . grid_y[0] = y_split_0;

	    tri_1 . grid_x[1] = tri -> grid_x[1];
	    tri_1 . grid_y[1] = tri -> grid_y[1];

	    tri_1 . grid_x[2] = x_split_1;
	    tri_1 . grid_y[2] = y_split_1;

	    tri_1 . v_index[0] = v_0;
	    tri_1 . v_index[1] = tri -> v_index[1];
	    tri_1 . v_index[2] = v_1;
	}
	else
	{
	    tri_1 . grid_x[0] = x_split_0;
	    tri_1 . grid_y[0] = y_split_0;

	    tri_1 . grid_x[1] = tri -> grid_x[1];
	    tri_1 . grid_y[1] = tri -> grid_y[1];

	    tri_1 . grid_x[2] = x_split_1;
	    tri_1 . grid_y[2] = y_split_1;

	    tri_1 . v_index[0] = v_0;
	    tri_1 . v_index[1] = tri -> v_index[1];
	    tri_1 . v_index[2] = v_1;
	}


	if (ts_info -> is_up)
	{
	    tri_2 . grid_x[0] = x_split_2;
	    tri_2 . grid_y[0] = y_split_2;

	    tri_2 . grid_x[1] = x_split_1;
	    tri_2 . grid_y[1] = y_split_1;

	    tri_2 . grid_x[2] = tri -> grid_x[2];
	    tri_2 . grid_y[2] = tri -> grid_y[2];

	    tri_2 . v_index[0] = v_2;
	    tri_2 . v_index[1] = v_1;
	    tri_2 . v_index[2] = tri -> v_index[2];
	}
	else
	{
	    tri_2 . grid_x[0] = x_split_2;
	    tri_2 . grid_y[0] = y_split_2;

	    tri_2 . grid_x[1] = x_split_1;
	    tri_2 . grid_y[1] = y_split_1;

	    tri_2 . grid_x[2] = tri -> grid_x[2];
	    tri_2 . grid_y[2] = tri -> grid_y[2];

	    tri_2 . v_index[0] = v_2;
	    tri_2 . v_index[1] = v_1;
	    tri_2 . v_index[2] = tri -> v_index[2];
	}


	if (ts_info -> is_up)
	{
	    tri_3 . grid_x[0] = x_split_0;
	    tri_3 . grid_y[0] = y_split_0;

	    tri_3 . grid_x[1] = x_split_1;
	    tri_3 . grid_y[1] = y_split_1;

	    tri_3 . grid_x[2] = x_split_2;
	    tri_3 . grid_y[2] = y_split_2;

	    tri_3 . v_index[0] = v_0;
	    tri_3 . v_index[1] = v_1;
	    tri_3 . v_index[2] = v_2;
	}
	else
	{
	    tri_3 . grid_x[0] = x_split_2;
	    tri_3 . grid_y[0] = y_split_2;

	    tri_3 . grid_x[1] = x_split_0;
	    tri_3 . grid_y[1] = y_split_0;

	    tri_3 . grid_x[2] = x_split_1;
	    tri_3 . grid_y[2] = y_split_1;

	    tri_3 . v_index[0] = v_2;
	    tri_3 . v_index[1] = v_0;
	    tri_3 . v_index[2] = v_1;
	}

	/*
	 *	=======================================================
	 *	Calculate tri-strip information for child triangles
	 *	At each level we maintain rows of triangle meshes. 
	 *	The number of rows doubles every level. 
	 *	=======================================================
	 */
	 ts_x_size = ts_info -> x_size * 2;
	 ts_y_size = ts_info -> y_size * 2;

	 ts_info_0 . ts_base = ts_info -> ts_base; 
	 ts_info_0 . x_size = ts_x_size; 
	 ts_info_0 . y_size = ts_y_size; 
	 ts_info_0 . is_up = ts_info -> is_up;
	 ts_info_0 . x = (ts_info -> x * 2) + (ts_info -> is_up ? 0 : 0);
	 ts_info_0 . y = (ts_info -> y * 2) + (ts_info -> is_up ? 1 : 0);

	 ts_info_1 . ts_base = ts_info -> ts_base; 
	 ts_info_1 . x_size = ts_x_size; 
	 ts_info_1 . y_size = ts_y_size; 
	 ts_info_1 . is_up = ts_info -> is_up;
	 ts_info_1 . x = (ts_info -> x * 2) + (ts_info -> is_up ? 1 : 1);
	 ts_info_1 . y = (ts_info -> y * 2) + (ts_info -> is_up ? 0 : 0);

	 ts_info_2 . ts_base = ts_info -> ts_base; 
	 ts_info_2 . x_size = ts_x_size; 
	 ts_info_2 . y_size = ts_y_size; 
	 ts_info_2 . is_up = ts_info -> is_up;
	 ts_info_2 . x = (ts_info -> x * 2) + (ts_info -> is_up ? 1 : 0);
	 ts_info_2 . y = (ts_info -> y * 2) + (ts_info -> is_up ? 1 : 1);

	 ts_info_3 . ts_base = ts_info -> ts_base; 
	 ts_info_3 . x_size = ts_x_size; 
	 ts_info_3 . y_size = ts_y_size; 
	 ts_info_3 . is_up = 1 - ts_info -> is_up;
	 ts_info_3 . x = (ts_info -> x * 2) + (ts_info -> is_up ? 1 : 0);
	 ts_info_3 . y = (ts_info -> y * 2) + (ts_info -> is_up ? 1 : 0);

	/* 
	 *	=======================================================
	 *	Recurse for finer level faces.
	 *	=======================================================
	 */
	child_0 = triangle2ASD 
		(gen, tile, grid, level + 1, nof_levels, &tri_0, &ts_info_0);
	child_1 = triangle2ASD 
		(gen, tile, grid, level + 1, nof_levels, &tri_1, &ts_info_1);
	child_2 = triangle2ASD 
		(gen, tile, grid, level + 1, nof_levels, &tri_2, &ts_info_2);
	child_3 = triangle2ASD 
		(gen, tile, grid, level + 1, nof_levels, &tri_3, &ts_info_3);
    }

    /*
     *	=======================================================
     *	Now that we have all the child face id's, 
     *	Prepare face structure, and add to tile. 
     *	=======================================================
     */
    face . level = level;

    if (ts_info -> ts_base[level] % 2 != 0)
	printf ("BAD ts_base[%d] = %d\n", level, ts_info -> ts_base[level]);

    face . tsid = ts_info -> ts_base[level] 
		+ ts_info -> x * (ts_info -> y_size + 4) * 2 
		+ ts_info -> y * 2 
		+ ts_info -> is_up;

    face . vert[0] = tri -> v_index[0];
    face . vert[1] = tri -> v_index[1];
    face . vert[2] = tri -> v_index[2];

    face . attr[0] = tri -> v_index[0];
    face . attr[1] = tri -> v_index[1];
    face . attr[2] = tri -> v_index[2];

    face . refvert[0] = v_0;
    face . refvert[1] = v_1;
    face . refvert[2] = v_2;

    face . sattr[0] = v_0;
    face . sattr[1] = v_1;
    face . sattr[2] = v_2;

    face . gstateid = 0;
    face . mask = 0;

    face . child[0] = child_0;
    face . child[1] = child_1;
    face . child[2] = child_2;
    face . child[3] = child_3;

    add_level_face (& tile -> l, &face, face_id);

    return (face_id);
}


/*============================================================================*/
void	init_levels (ASD_levels *levels)
/*============================================================================*/
{
    levels -> nof_levels = 0;
    levels -> face_level = NULL;
    levels -> vertex_level = NULL;
}

/*============================================================================*/
void	zero_levels (ASD_levels *levels)
/*============================================================================*/
{
    int 	i;

    for (i = 0 ; i < levels -> nof_levels ; i ++)
    {
	zero_vertices (&levels -> vertex_level[i]);
	zero_faces (&levels -> face_level[i]);
    }
}

/*============================================================================*/
int	add_level_face (ASD_levels *levels, pfASDFace *face, int id)
/*============================================================================*/
{
    int		level, new_nof_levels;
    int		i;

    level = face -> level;

    if (level >= levels -> nof_levels)
    {
 	new_nof_levels = level + 1;

	levels -> face_level = (ASD_faces *) realloc (levels -> face_level, 
			new_nof_levels * sizeof (ASD_faces));

	levels -> vertex_level = (ASD_vertices *) 
			realloc (levels -> vertex_level, 
			new_nof_levels * sizeof (ASD_vertices));

	for (i = levels -> nof_levels ; i < new_nof_levels ; i ++)
	{
	    init_faces (&levels -> face_level[i]);
	    init_vertices (&levels -> vertex_level[i]);
	}

	levels -> nof_levels = new_nof_levels;
    }

    return (add_face (& levels -> face_level[level], face, id));
}

/*============================================================================*/
int	add_level_vertex (ASD_levels *levels, int level, pfASDVert *vert, 
				float x, float y, int is_in_tile)
/*============================================================================*/
{
    int		new_nof_levels;
    int		i;

    if (level >= levels -> nof_levels)
    {
 	new_nof_levels = level + 1;

	levels -> face_level = (ASD_faces *) realloc (levels -> face_level, 
			new_nof_levels * sizeof (ASD_faces));

	levels -> vertex_level = (ASD_vertices *) 
			realloc (levels -> vertex_level, 
			new_nof_levels * sizeof (ASD_vertices));

	for (i = levels -> nof_levels ; i < new_nof_levels ; i ++)
	{
	    init_faces (&levels -> face_level[i]);
	    init_vertices (&levels -> vertex_level[i]);
	}

	levels -> nof_levels = new_nof_levels;
    }

    return (add_vertex (& levels -> vertex_level[level], 
		vert, x, y, is_in_tile));
}

/*============================================================================*/
void	init_faces (ASD_faces *faces)
/*============================================================================*/
{
    faces -> nof_faces = 0;
    faces -> nof_allocated_faces = 0;
    faces -> face = NULL;
    faces -> bbox = NULL;
    faces -> face_id = NULL;
}

/*============================================================================*/
void	zero_faces (ASD_faces *faces)
/*============================================================================*/
{
    faces -> nof_faces = 0;
}

/*============================================================================*/
int	add_face (ASD_faces *faces, pfASDFace *face, int id)
/*============================================================================*/
{
    int		new_alloc;

    if (faces -> nof_faces >= faces -> nof_allocated_faces)
    {
	new_alloc = (faces -> nof_allocated_faces + 2) * 4 / 3;
	faces -> face = (pfASDFace *) realloc 
			(faces -> face, new_alloc * sizeof (pfASDFace));

	faces -> bbox = (pfBox *) realloc 
			(faces -> bbox, new_alloc * sizeof (pfBox));

	faces -> face_id = (int *) realloc 
			(faces -> face_id, new_alloc * sizeof (int));

	faces -> nof_allocated_faces = new_alloc;
    }

#if 0
    tabs (face -> level); 
	printf ("Added face %d at level %d\n", id, face -> level);
    tabs (face -> level);
	printf ("Vertices are: %d, %d, %d\n", 
		face -> vert[0], face -> vert[1], face -> vert[2]);
#endif


    faces -> face[faces -> nof_faces] = *face;
    faces -> face_id[faces -> nof_faces] = id;
    faces -> nof_faces ++;
    return (faces -> nof_faces - 1);
}

/*============================================================================*/
void	init_vertices (ASD_vertices *verts)
/*============================================================================*/
{
    verts -> nof_vertices = 0;
    verts -> nof_allocated_vertices = 0;
    verts -> vertex = NULL;
    verts -> vertex_id = NULL;
    verts -> vertex_in_tile = NULL;
    verts -> pos = NULL;
}

/*============================================================================*/
void	zero_vertices (ASD_vertices *verts)
/*============================================================================*/
{
    verts -> nof_vertices = 0;
}

/*============================================================================*/
int	add_vertex (ASD_vertices *verts, pfASDVert *v, float x, float y, 
			int is_in_tile)
/*============================================================================*/
{
    int		new_alloc;

    if (verts -> nof_vertices >= verts -> nof_allocated_vertices)
    {
	new_alloc = (verts -> nof_allocated_vertices + 2) * 4 / 3;

	verts -> vertex = (pfASDVert *) realloc 
			(verts -> vertex, new_alloc * sizeof (pfASDVert));

	verts -> pos = (pos2 *) realloc 
			(verts -> pos, new_alloc * sizeof (pos2));

	verts -> vertex_id = (int *) realloc 
			(verts -> vertex_id, new_alloc * sizeof (int));

	verts -> vertex_in_tile = (int *) realloc 
			(verts -> vertex_in_tile, new_alloc * sizeof (int));

	verts -> nof_allocated_vertices = new_alloc;
    }

    verts -> vertex[verts -> nof_vertices] = *v;
    verts -> pos[verts -> nof_vertices] . x = x;
    verts -> pos[verts -> nof_vertices] . y = y;
    verts -> vertex_id[verts -> nof_vertices] = v -> vertid;
    verts -> vertex_in_tile[verts -> nof_vertices] = is_in_tile;

    verts -> nof_vertices ++;
    return (verts -> nof_vertices - 1);
}


/*============================================================================*/
int	lookup_vertex_by_vertid (ASD_vertices *verts, int vertid)
/*============================================================================*/
{
    int		i;
    pfASDVert	*v;

    v = verts -> vertex;

    for (i = 0 ; i < verts -> nof_vertices ; i ++)
    {
	if (v -> vertid == vertid)
	    return (i);

	v ++;
    }
	
    return (-1);
}

/*============================================================================*/
int 	global_lookup_vertex_by_vertid (
		    	ASD_vertices 	*v_levels, 
			int		max_level, 
			int		vertid, 
			int		*vertex_level, 
			int		*vertex_id) 
/*============================================================================*/
{
    int	i, j;

    for (i = 0 ; i <= max_level ; i ++)
	if ((j = lookup_vertex_by_vertid (&v_levels[i], vertid)) >= 0)
	{
	    *vertex_level = i;
	    *vertex_id = j;
	    return (1);
 	}
    
    return (0);
}

/*============================================================================*/
int 	lookup_face_by_id (ASD_faces *faces, int id)
/*============================================================================*/
{
    int		i;
    int		*face_id;

    face_id = faces -> face_id;

    for (i = 0 ; i < faces -> nof_faces ; i ++)
    {
	if (*face_id == id)
	    return (i);

	face_id ++;
    }
	
    return (-1);
}

/*============================================================================*/
void	init_asd_tile (ASD_tile *t)
/*============================================================================*/
{
init_levels (& t -> l);
}

/*============================================================================*/
void	zero_asd_tile (ASD_tile *t)
/*============================================================================*/
{
zero_levels (& t -> l);
}


/*============================================================================*/
int	make_asd_vertex (pfdASDGenerator *gen, 
				ASD_tile *tile, 
				grid3 *grid, int level, float px, float py, 
				int x, int y, pfVec3 d, int n0, int n1)
/*============================================================================*/
{
    pfASDVert		v;
    pfVec3		pos;
    int			index;
    int			vertid;
    int			is_in_tile;

    /*
     *	===================================================================
     *	Calculate vertex unique ID (across entire ASD).
     *	===================================================================
     */
    vertid = calc_asd_vertex_index (gen, & tile -> info, level, x, y);

    /*
     *	===================================================================
     *	Do we already have this vertex ?  If we do, don't add it twice.
     *	===================================================================
     */
    if (tile -> l . nof_levels > level)
    {
	index = lookup_vertex_by_vertid 
			(&(tile -> l . vertex_level[level]) , vertid);
	if (index >= 0)
	    return (vertid);
    }

    /*
     *	===================================================================
     *	This is the first time we encounter this vertex. Add it to list.
     *	Compute world location and morph vector of vertex.
     *	===================================================================
     */
    calc_point_2D (grid, px, py, pos);
    
    pfCopyVec3 (v . v0, pos);
    pfCopyVec3 (v . vd, d);
    v . neighborid[0] = n0;
    v . neighborid[1] = n1;
    v . vertid = vertid;

    /*
     *	===================================================================
     *	Vertex may be on a neighbor tile. 
     *	If so, mark it for not writing to file.
     *	===================================================================
     */
    is_in_tile = is_vertex_in_tile (tile, level, x, y);

    add_level_vertex (&tile -> l, level, & v, x, y, is_in_tile);
    return (vertid);
}

/*============================================================================*/
void extract_subgrid (pfdASDGenerator *gen, int from_x, int from_y, grid3 *grid)
/*============================================================================*/
{
    int		x, y;
    int		grid_x, grid_y;
    int		grid_offset;
    float	wx, wy, wz;
    float	dx, dy, radius, r;
    int		gx, gy;

    grid_x = gen -> tile_size;
    grid_y = gen -> tile_size;

    for (y = 0 ; y < grid_y ; y ++)
	for (x = 0 ; x < grid_x ; x ++)
	{
	    grid_offset = y * grid_x + x;

	    gx = from_x + x;
	    gy = from_y + y;

	    if (gen -> asd_geometry == ASDGEN_PLANE)
	    {
		wx = gen -> x_base + gx * gen -> x_span 
						    / (gen -> x_grid_size - 1);
		wy = gen -> y_base + gy * gen -> y_span 
						    / (gen -> y_grid_size - 1);

		wz = (*gen -> get_elevation)(gen -> user_data, 
						    from_x + x, from_y + y);
	    }
	    else
	    {
		dx = gen -> x_base + gx * gen -> x_span 
						    / (gen -> x_grid_size - 1);
		dy = gen -> y_base + gy * gen -> y_span 
						    / (gen -> y_grid_size - 1);

		radius = gen -> sphere_radius + 
				(*gen -> get_elevation)(gen -> user_data, 
						    from_x + x, from_y + y);

		r = radius * cos(dy);
		wx = gen -> sphere_center[0] + r * cos (dx);
		wy = gen -> sphere_center[1] + r * sin (dx);

		wz = gen -> sphere_center[2] + radius * sin (dy);
   	    }

	    pfSetVec3(grid -> points [grid_offset], wx, wy, wz);
	}
}


/*============================================================================*/
void	tabs (int n)
/*============================================================================*/
{
int i;

for (i = 0 ; i < n ; i ++)
    printf ("  ");
}

/*============================================================================*/
void save_tile (ASD_tile *tile, char *base_name, int x, int y)
/*============================================================================*/
{
    char 	name[600];
    int		fd;
    int		i, j;
    int		nof_faces, nof_vertices;



    /*	
     *	=============================================================
     *	Correct Format.
     *	=============================================================
     */
    for (i = 0 ; i < tile -> l . nof_levels ; i ++)
    {
	nof_faces = tile -> l . face_level[i] . nof_faces; 

	nof_vertices = 0;
	for (j = 0 ; j < tile -> l . vertex_level[i] . nof_vertices ; j ++)
	    if (tile -> l . vertex_level[i] . vertex_in_tile[j])
		nof_vertices ++;

	if (nof_faces + nof_vertices > 0)
	{
	    /*
	     *	=============================
	     *	Open file. Write header.
	     *	=============================
	     */
	    sprintf(name, "%s%02d%03d%03d", base_name, i, x, y);
	    if ((fd = open (name, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0)
	    {
		perror (name);
		return;
	    }

	    write (fd, & nof_faces, sizeof (int));
	    write (fd, & nof_vertices, sizeof (int));

	    /*
	     *	========================
	     *	Write faces.
	     *	========================
	     */
	    for (j = 0 ; j < nof_faces ; j ++)
	    {
		write (fd, & tile -> l . face_level[i] . face_id[j], 
			sizeof (int));
		write (fd, & tile -> l . face_level[i] . face[j], 
			sizeof (pfASDFace));
	    }

	    /*
	     *	========================
	     *	Write BBox'es.
	     *	========================
	     */
	    write (fd, tile -> l . face_level[i] . bbox, 
					nof_faces * sizeof (pfBox));

	    /*
	     *	========================
	     *	Write vertices.
	     *	========================
	     */
	    for (j = 0 ; j < tile -> l . vertex_level[i] . nof_vertices ; j ++)
	    {
		if (tile -> l . vertex_level[i] . vertex_in_tile[j])
		{
		    write (fd, & tile -> l . vertex_level[i] . vertex_id[j], 
			sizeof (int));
		    write (fd, & tile -> l . vertex_level[i] . vertex[j], 
			sizeof (pfASDVert));
		}
	    }

	    close (fd);
	}
    }
}

/*============================================================================*/
void calc_tile_face_bbox (ASD_tile *tile)
/*============================================================================*/
{
    int		i;

    for (i = 0 ; i < tile -> l . face_level[0] . nof_faces ; i ++)
    {
	calc_face_bbox (tile -> l . face_level, 
			tile -> l . vertex_level, 
			0, i);
    }
}


/*============================================================================*/
void	calc_face_bbox (
		    ASD_faces 		*f_levels, 
		    ASD_vertices 	*v_levels, 
		    int 		level, 
		    int 		face_id)
/*============================================================================*/
{
    pfASDFace   *face;
    pfBox	*bbox, *child_bbox;
    pfVec3	points[3];
    int		i;
    int		child_id;
    int		vertex_id, cid;
    int		vertex_level;

    face = & f_levels[level] . face[face_id];
    bbox = & f_levels[level] . bbox[face_id];

    /*
     *	==========================================
     *	Build basic bbox around face endpoints.
     *	==========================================
     */
    for (i = 0 ; i < 3 ; i ++)
    {
        if (global_lookup_vertex_by_vertid (
		    	v_levels, 
			level, 		/* Max level to search */
			face -> vert[i], 
			&vertex_level, 
			&vertex_id))
	{
	    pfCopyVec3 (
			points[i], 
			v_levels[vertex_level] . vertex[vertex_id] . v0);
	}
	else
	{
	    printf ("calc_face_bbox: vertex %d not found. level = %d\n", 
		face -> vert[i], level);
	    dump_vertices (v_levels, level + 1);

	    points[i][0] = 0.0;
	    points[i][1] = 0.0;
	    points[i][2] = 0.0;
        }
    }

    pfBoxAroundPts (bbox, points, 3);

    /*
     *	==========================================
     *	Extend bbox to children bbox'es.
     *	==========================================
     */
    for (i = 0 ; i < 4 ; i ++ )
    {
	cid = face -> child[i];

	if (cid != PFASD_NIL_ID)
	{
	    child_id = lookup_face_by_id (& f_levels[level+1], cid);

	    if (child_id < 0)
		printf ("calc_face_bbox: Missing child_id %d, level = %d\n", 
			cid, level);

	    calc_face_bbox (f_levels, v_levels, level+1, child_id);

	    child_bbox = & f_levels[level + 1] . bbox[child_id];

	    pfBoxExtendByBox (bbox, child_bbox);
	}
    }
}

/*============================================================================*/
void calc_tile_face_neighbors (ASD_tile *tile)
/*============================================================================*/
{
    int	i;

    for (i = 0 ; i < tile -> l . face_level[0] . nof_faces ; i ++)
    {
	calc_face_neighbors (tile -> l . face_level, 
			tile -> l . vertex_level, 0, 
			tile -> l . face_level[0].face_id[i]);
    }
}


/*============================================================================*/
void	calc_face_neighbors (
		    ASD_faces 		*f_levels, 
		    ASD_vertices 	*v_levels, 
		    int 		level, 
		    int 		face_id)
/*============================================================================*/
{
    pfASDFace   *face;
    pfASDVert	*vert;
    int		i;
    int		cid;
    int		split_id, sid;
    int		fid;

    fid = lookup_face_by_id (& f_levels[level] , face_id);
    face = & f_levels[level] . face[fid];

    /*
     *	==========================================
     *	Use this face to update its split point 
     *	neighbors (add this face as one of the 
     *	neighbors). 
     *
     *	We use the knowledge that split points 
     *	of level i are vertices of level (i+1).
     *	==========================================
     */
    for (i = 0 ; i < 3 ; i ++)
    {
	sid = face -> refvert[i];
	if (sid != PFASD_NIL_ID)
	{
   	    split_id = lookup_vertex_by_vertid (& v_levels[level + 1], sid);

	    if (split_id < 0)
		printf ("calc_face_neighbors: vrtx %d not found in level %d\n",
			sid, level + 1);

	    vert = & v_levels[level + 1] . vertex [split_id];
	    if (vert -> neighborid[0] == PFASD_NIL_ID)
		vert -> neighborid[0] = face_id;
	    else
	    if ((vert -> neighborid[1] == PFASD_NIL_ID) && 
		(vert -> neighborid[0] != face_id))
		vert -> neighborid[1] = face_id;
	}
    }

    
    /*
     *	==========================================
     *	Recurse on children
     *	==========================================
     */
    for (i = 0 ; i < 4 ; i ++ )
    {
	cid = face -> child[i];

	if (cid != PFASD_NIL_ID)
	    calc_face_neighbors (f_levels, v_levels, level+1, cid);
    }
}

/*============================================================================*/
ASD_tile_stats *init_asd_tile_stats (int nof_tiles, int nof_levels)
/*============================================================================*/
{
    ASD_tile_stats 	*info;
    int			i;

    info = (ASD_tile_stats *) 
			malloc (nof_tiles * sizeof (ASD_tile_stats));

    for (i = 0 ; i < nof_tiles ; i ++)
    {
        info[i] . nof_faces = (int *) 
				malloc ((1 + nof_levels) * sizeof (int));
        info[i] . nof_vertices = (int *) 
				malloc ((1 + nof_levels) * sizeof (int));
        info[i] . base_face = (int *) 
				malloc ((1 + nof_levels) * sizeof (int));
        info[i] . base_vertex = (int *) 
				malloc ((1 + nof_levels) * sizeof (int));
    }

    return (info);
}

/*============================================================================*/
void  collect_asd_tile_stats (
			ASD_tile_stats 	*stats, 
			ASD_tile 	*tile, 
			int 		nof_levels)
/*============================================================================*/
{
    int 	i;

    for (i = 0 ; i < nof_levels ; i ++)
    {
	stats -> nof_faces[i] = 
			tile -> l . face_level[i] . nof_faces;

	stats -> nof_vertices[i] = 
			tile -> l . vertex_level[i] . nof_vertices;
    }
}
/*============================================================================*/
void  calc_asd_tile_stats (ASD_tile_stats *stats, int nof_tiles, int nof_levels)
/*============================================================================*/
{
    int 	i, j;
    int		base_face, base_vertex;

    base_face = 0;
    base_vertex = 0;

    for (i = 0 ; i < nof_levels ; i ++)
	for (j = 0 ; j < nof_tiles ; j ++)
	{
	    stats[j] . base_face[i] = base_face;

	    stats[j] . base_vertex[i] = base_vertex;
 
	    base_face += stats[j] . nof_faces[i];
	    base_vertex += stats[j] . nof_vertices[i];
	}
}

/*============================================================================*/
void  calc_asd_tile_info (	
		    ASD_tile_info *info, 
		    int x_nof_tiles_across, 
		    int y_nof_tiles_across, 
		    int nof_levels)
/*============================================================================*/
{
    int 	i;

    info -> x_tiles_across = x_nof_tiles_across;
    info -> y_tiles_across = y_nof_tiles_across;

    /*
     *	=============================================================
     *	Malloc memory for all levels.
     *	=============================================================
     */
    info -> level_base_vertex = 
		(int *) malloc ((1 + nof_levels) * sizeof (int));
    info -> level_base_face = 
		(int *) malloc ((1 + nof_levels) * sizeof (int));
    info -> x_level_vertices_across = 
		(int *) malloc ((1 + nof_levels) * sizeof (int));
    info -> y_level_vertices_across = 
		(int *) malloc ((1 + nof_levels) * sizeof (int));
    info -> level_nof_faces = 
		(int *) malloc ((1 + nof_levels) * sizeof (int));
    info -> level_nof_vertices = 
		(int *) malloc ((1 + nof_levels) * sizeof (int));


    /*
     *	=============================================================
     *	Calculate total number of vertices across in each level.
     *	=============================================================
     */
    info -> x_level_vertices_across[0] = x_nof_tiles_across + 1;
    info -> y_level_vertices_across[0] = y_nof_tiles_across + 1;

    for (i = 1 ; i <= nof_levels ; i ++)
    {
	info -> x_level_vertices_across[i] = 
		info -> x_level_vertices_across[i-1] * 2 - 1;

	info -> y_level_vertices_across[i] = 
		info -> y_level_vertices_across[i-1] * 2 - 1;
    }

    /* 
     *	=============================================================
     *	Calculate number of new vertices/faces to each level.
     *	=============================================================
     */
    info -> level_nof_faces[0] = x_nof_tiles_across * y_nof_tiles_across * 2;
    info -> level_nof_vertices[0] = (x_nof_tiles_across + 1) * 
					(y_nof_tiles_across + 1);

    for (i = 1 ; i <= nof_levels ; i ++)
    {
	info -> level_nof_faces[i] = info -> level_nof_faces[i - 1] * 4;
	info -> level_nof_vertices[i] = 
		info -> x_level_vertices_across[i - 1] * 
			(info -> y_level_vertices_across[i - 1] - 1) + 
		info -> y_level_vertices_across[i - 1] * 
			(info -> x_level_vertices_across[i - 1] - 1) + 
		(info -> x_level_vertices_across[i - 1] - 1) * 
		    (info -> y_level_vertices_across[i - 1] - 1);
    }

    /*
     *	=============================================================
     *	Calculate base face/vertex index for each level.
     *	=============================================================
     */
    info -> level_base_vertex[0] = 0;
    info -> level_base_face[0] = 0;

    for (i = 1 ; i <= nof_levels ; i ++)
    {
	info -> level_base_vertex[i] = 
		info -> level_base_vertex[i - 1] + 
		info -> level_nof_vertices[i - 1];

	info -> level_base_face[i] = 
		info -> level_base_face[i - 1] + 
		info -> level_nof_faces[i - 1];
    }
}

/*===========================================================================*/
int	calc_asd_vertex_index (pfdASDGenerator *gen, 
			ASD_tile_info *info, int level, int x, int y)
/*===========================================================================*/
{
int		x_level_size, y_level_size;
int		block_base;

x_level_size = info -> x_level_vertices_across[level];
y_level_size = info -> y_level_vertices_across[level];

if (level > 0)
    {
    /*
     *	Level > 0.
     */
    if (y & 0x01)
	{
	/*
     	 *	Second block (odd lines).
	 */
	block_base = (y_level_size + 1) / 2 * (x_level_size - 1) / 2;
	return ((int) 
		(info -> level_base_vertex[level] 
			+ block_base + ((y - 1) / 2) * x_level_size + x));
	}
    else
	{
	/*
 	 *	First block (even lines).]
	 */
	return ((int) 
		(info -> level_base_vertex[level] + (y / 2) * 
					((x_level_size - 1) / 2) 
						+ (x - 1) / 2));
	}
    }
else
    {
    /*
     *	Level zero.
     */
    return ((int) (y * x_level_size + x));
    }
}

/*===========================================================================*/
int	is_vertex_in_tile (ASD_tile *tile, int level, int x, int y)
/*===========================================================================*/
{
    int			x_from, x_to;
    int			y_from, y_to;
    ASD_tile_info	*info;
    int			x_v_across, y_v_across;
    int			x_tiles, y_tiles;
    register int	x_tile_size, y_tile_size;

    info = & tile -> info;

    x_v_across = info -> x_level_vertices_across[level];
    y_v_across = info -> y_level_vertices_across[level];
    x_tiles = info -> x_tiles_across;
    y_tiles = info -> y_tiles_across;

    x_tile_size = 1 + (x_v_across - 1) / x_tiles;
    y_tile_size = 1 + (y_v_across - 1) / y_tiles;

    x_from = info -> x * (x_tile_size - 1);
    y_from = info -> y * (y_tile_size - 1);

    if (info -> x >= (x_tiles - 1))
	x_to = (info -> x + 1) * (x_tile_size - 1);
    else
	x_to = (info -> x + 1) * (x_tile_size - 1) - 1;

    if (info -> y >= (y_tiles - 1))
	y_to = (info -> y + 1) * (y_tile_size - 1);
    else
	y_to = (info -> y + 1) * (y_tile_size - 1) - 1;

#if 0
    if (level == 0)
    {
	printf ("$$$$ is_vertex_in_tile: level = %d, x,y = (%d, %d)\n", 
			level, x, y);
	printf ("\tx_tiles = %d, y_tiles = %d\n", x_tiles, y_tiles);
	printf ("\tv_across = %d, %d\n", x_v_across, y_v_across);
	printf ("\tx_tile_size = %d, y_tile_size = %d\n", 
				x_tile_size, y_tile_size);
	printf ("\tx_from = %d, y_from = %d\n", x_from, y_from);
	printf ("\tx_to = %d, y_to = %d\n", x_to, y_to);
    }
#endif


    if ((x >= x_from) && (x <= x_to) && (y >= y_from) && (y <= y_to))
	return (1);
    else
	return (0);
}

/*===========================================================================*/
void	calc_vertex_tile (
			ASD_tile_info 	*info, 
			int 		level, 
			int 		x, 
			int 		y, 
			int 		*tx, 
			int 		*ty)
/*===========================================================================*/
{
int		x_v_across, y_v_across;
int		x_tiles, y_tiles;
register int	x_tile_size, y_tile_size;

x_v_across = info -> x_level_vertices_across[level];
y_v_across = info -> y_level_vertices_across[level];
x_tiles = info -> x_tiles_across;
y_tiles = info -> y_tiles_across;

x_tile_size = 1 + (x_v_across - 1) / x_tiles;
y_tile_size = 1 + (y_v_across - 1) / y_tiles;

*tx = (int) (x / (x_tile_size - 1));
*ty = (int) (y / (y_tile_size - 1));
}

/*===========================================================================*/
void    save_configuration_file(
			    ASD_tile_stats 	*stats, 
			    ASD_tile_info 	*info, 
			    int			nof_levels,
			    char 		*base_filename, 
			    char 		*filename)
/*===========================================================================*/
{
    FILE	*fp;
    int		level_0;
    int		i;

    if ((fp = fopen (filename, "w")) == NULL)
    {
	perror(filename);
	return;
    }
    
    /*
     *	============================================
     *	Tile file prefix.
     *	============================================
     */
    fprintf (fp, "%s\n", base_filename);

    /*
     *	============================================
     *	Total number of faces in level 0.
     *	============================================
     */
    level_0 = 0;
    for (i = 0 ; i < info -> x_tiles_across * info -> x_tiles_across ; i ++)
	level_0 += stats[i] . nof_faces[0];

    fprintf (fp, "%d\n", level_0);

    /*
     *	============================================
     *	How many levels ? 
     *	============================================
     */

    fprintf (fp, "%d\n", nof_levels);
    

    fclose (fp);
}

/*============================================================================*/
void init_level (ASD_level *level)
/*============================================================================*/
{
    init_faces (&level -> faces);
    init_vertices (&level -> vertices);
}

/*============================================================================*/
void zero_level (ASD_level *level)
/*============================================================================*/
{
    zero_faces (&level -> faces);
    zero_vertices (&level -> vertices);
}

/*============================================================================*/
void save_level (ASD_level *level, char *filename)
/*============================================================================*/
{
    int		fd;
    int		j;
    int		nof_faces, nof_vertices;

    /*	
     *	=============================================================
     *	Correct Format.
     *	=============================================================
     */
    nof_faces = level -> faces . nof_faces; 
    nof_vertices = level -> vertices . nof_vertices;

    if (nof_faces + nof_vertices > 0)
    {
	/*
	 *	=============================
	 *	Open file. Write header.
	 *	=============================
	 */
	if ((fd = open (filename, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0)
	{
	    perror (filename);
	    return;
	}

	write (fd, & nof_faces, sizeof (int));
	write (fd, & nof_vertices, sizeof (int));

	/*
	 *	========================
	 *	Write faces.
	 *	========================
	 */
	for (j = 0 ; j < nof_faces ; j ++)
	{
	    write (fd, & level -> faces . face_id[j], sizeof (int));
	    write (fd, & level -> faces . face[j], sizeof (pfASDFace));
	}

	/*
	 *	========================
	 *	Write BBox'es.
	 *	========================
	 */
	write (fd, level -> faces . bbox, nof_faces * sizeof (pfBox));

	/*
	 *	========================
	 *	Write vertices.
	 *	========================
	 */
	for (j = 0 ; j < level -> vertices . nof_vertices ; j ++)
	{
	    write (fd, & level -> vertices . vertex_id[j], sizeof (int));
	    write (fd, & level -> vertices . vertex[j], sizeof (pfASDVert));
	}

	close (fd);
    }
    else
    {
	printf ("Skipping empty tile: %s\n", filename);
    }
}

/*============================================================================*/
void merge_level (ASD_level *level, char *filename)
/*============================================================================*/
{
    int		fd;
    int		j;
    int		file_nof_faces, file_nof_vertices;
    int		new_nof_faces, new_nof_vertices;
    int		old_nof_faces, old_nof_vertices;

    /*
     *	=============================
     *	Open file. Write header.
     *	=============================
     */
    if ((fd = open (filename, O_RDONLY)) < 0)
    {
	perror (filename);
	return;
    }

    read (fd, & file_nof_faces, sizeof (int));
    read (fd, & file_nof_vertices, sizeof (int));

    old_nof_faces = level -> faces . nof_faces;
    old_nof_vertices = level -> vertices . nof_vertices;

    new_nof_faces = old_nof_faces + file_nof_faces;
    new_nof_vertices = old_nof_vertices + file_nof_vertices;

    /*
     *	============================================
     *	Realloc memory to match new size.
     *	============================================
     */

    if (new_nof_faces >= level -> faces . nof_allocated_faces)
    {
	level -> faces . face_id = (int *) realloc (level -> faces . face_id, 
			new_nof_faces * sizeof (int));

	level -> faces . face = (pfASDFace *) realloc (level -> faces . face, 
			new_nof_faces * sizeof (pfASDFace));

	level -> faces . bbox = (pfBox *) realloc (level -> faces . bbox, 
			new_nof_faces * sizeof (pfBox));

	level -> faces . nof_allocated_faces = new_nof_faces;
    }

    if (new_nof_vertices >= level -> vertices . nof_allocated_vertices)
    {
	level -> vertices . vertex_id = (int *) realloc 
	    (level -> vertices . vertex_id, new_nof_vertices * sizeof (int));

	level -> vertices . vertex = (pfASDVert *) realloc 
	    (level -> vertices . vertex, new_nof_vertices * sizeof (pfASDVert));

	level -> vertices . nof_allocated_vertices = new_nof_vertices;
    }

    /*
     *	========================
     *	Read faces.
     *	========================
     */
    for (j = 0 ; j < file_nof_faces ; j ++)
    {
	read (fd, & level -> faces . face_id[old_nof_faces + j], sizeof (int));
	read (fd, & level -> faces . face[old_nof_faces + j], 
						sizeof (pfASDFace));
    }

    /*
     *	========================
     *	Read BBox'es.
     *	========================
     */
    read (fd, & level -> faces . bbox[old_nof_faces] , 
					file_nof_faces * sizeof (pfBox));

    /*
     *	========================
     *	Read vertices.
     *	========================
     */
    for (j = 0 ; j < file_nof_vertices ; j ++)
    {
	read (fd, & level -> vertices . vertex_id[old_nof_vertices + j], 
							sizeof (int));
	read (fd, & level -> vertices . vertex[old_nof_vertices + j], 
							sizeof (pfASDVert));
    }


    level -> vertices . nof_vertices = new_nof_vertices;

    level -> faces . nof_faces = new_nof_faces;

    close (fd);
}

/*===========================================================================*/
void    rearrange_tiles (
			char *in_filename, 
			char *out_filename, 
			int nof_levels, 
			int x_level_0_across, 
			int y_level_0_across, 
			pfBox *box, 
			char *cfg_filename)
/*===========================================================================*/
{
    ASD_level 		level;
    int			i, j;
    int			l;
    char		name[500];
    FILE		*fp;
    pfASDLODRange	*lods;
    int			x_size, y_size;
    int			x, y;
    int			ii, jj;
    float		span;
    ASD_final_info	*info;
    int			max_faces, max_vertices;
    int			nof_faces_level_0;
    int			fx, fy;
    float		x_span, y_span;

    x_span = (box -> max[0] - box -> min[0]);
    y_span = (box -> max[1] - box -> min[1]);

    info = (ASD_final_info *) alloca 
				((1 + nof_levels) * sizeof (ASD_final_info));

    /*
     *	=============================================================
     *	Merge all level # 0 tiles into a single tile.
     *	=============================================================
     */

    printf ("rearrange level 0\n");

    init_level (&level);
    zero_level (&level);

    for ( i = 0 ; i < x_level_0_across; i ++)
	for (j = 0 ; j < y_level_0_across; j ++)
	{
	    sprintf(name, "%s%02d%03d%03d", in_filename, 0, i, j);
	    merge_level (& level, name);
	}

    sprintf(name, "%s%02d%03d%03d", out_filename, 0, 0, 0);
    save_level (&level, name);

    nof_faces_level_0 = level . faces . nof_faces;


    info[0] . x_tiles = 1;
    info[0] . y_tiles = 1;
    info[0] . x_size = x_span;
    info[0] . y_size = y_span;
    info[0] . max_faces = level . faces . nof_faces;
    info[0] . max_vertices = level . vertices . nof_vertices;

    /*
     *	======================================================================
     * 	Starting from highest LOD, merge 1, 2, 4, 8, ... tiles into a single 
     *	tiles. This will make the avg number of faces per tiles the same for
     *	all levels.
     *	======================================================================
     */
    x_size = 1;
    y_size = 1;
    for (l = nof_levels - 1 ; l > 0 ; l --)
    {
        max_faces = 0;
        max_vertices = 0;
	printf ("rearrange level %d\n", l);
	x = 0;
	for (i = 0 ; i < x_level_0_across ; i += x_size)
	{
	    y = 0;
	    for (j = 0 ; j < y_level_0_across ; j += y_size)
	    {
		zero_level (& level);

		for (ii = 0 ; ii < x_size ; ii ++)
		    for (jj = 0 ; jj < y_size ; jj ++)
		    {
			fx = i + ii;
			fy = j + jj;
			if ((fx < x_level_0_across) && (fy < y_level_0_across))
			{
			    sprintf(name, "%s%02d%03d%03d", 
					in_filename, l, i + ii, j + jj);
			    merge_level (&level, name);
			}
		    }

		if (level . faces . nof_faces > max_faces)
		    max_faces = level . faces . nof_faces;

		if (level . vertices . nof_vertices > max_vertices)
		    max_vertices = level . vertices . nof_vertices;

		printf ("\tSave final tile: (%d, %d)\n", x, y);
		sprintf(name, "%s%02d%03d%03d", out_filename, l, x, y);
		save_level (&level, name);
		y ++;
	    }
	    x ++;
	}

	info[l] . max_faces = max_faces;
	info[l] . max_vertices = max_vertices;

	info[l] . x_tiles = x;
	info[l] . y_tiles = y;

	info[l] . x_size = x_span / (float) x;
	info[l] . y_size = y_span / (float) y;

        x_size = x_size * 2;
	if (x_size > x_level_0_across) 
		x_size = x_level_0_across;

        y_size = y_size * 2;
	if (y_size > y_level_0_across) 
		y_size = y_level_0_across;
    }

    /*
     *	=======================================
     *	Calculate morphing parameters of LOD's.
     *	=======================================
     */


    lods = (pfASDLODRange *) alloca ((1 + nof_levels) * sizeof (pfASDLODRange));

#if 0
    span = (box -> max[0] - box -> min[0]) + 
	   (box -> max[1] - box -> min[1]) +
	   (box -> max[2] - box -> min[2]);

    for (i = 0 ; i < nof_levels ; i ++)
        lods[i].switchin = 0.001 * span * (nof_levels - i) / nof_levels;
#else
    span = 2.0 * (info[nof_levels - 1] . x_size + 
		  info[nof_levels - 1] . y_size);

    for (i = nof_levels - 1 ; i >= 0 ; i --)
    {
        lods[i].switchin = span;
	span = span * 2.0;
    }
#endif

    if (lods[0].switchin < 100000.0)
	lods[0].switchin = 100000.0;

    lods[0].morph = 100.0;

    for (i = 1 ; i < (nof_levels - 1) ; i ++)
        lods[i].morph = 0.1 * (lods[i].switchin - lods[i + 1].switchin);

    lods[nof_levels - 1].morph = 0.1 * lods[nof_levels - 1].switchin;

    /*
     *	=======================================================
     *	Create and fill configuration file.
     *	=======================================================
     */
    if ((fp = fopen (cfg_filename, "w")) == NULL)
    {
	perror (cfg_filename);
	return;
    }

    fprintf (fp, "%s\n", out_filename);

    fprintf (fp, "%d\n", nof_faces_level_0);
    fprintf (fp, "%d\n", nof_levels);

    for (i = 0 ; i < nof_levels ; i ++)
        fprintf (fp, "%f %f\n", lods[i].switchin, lods[i].morph);

    fprintf(fp, "%f %f %f\n", box -> min[0], box -> min[1], box -> min[2]);
    fprintf(fp, "%f %f %f\n", box -> max[0], box -> max[1], box -> max[2]);

    for(i = 0; i < nof_levels; i++)
        fprintf(fp, "%d %d\n", info[i] . x_tiles, info[i] . y_tiles);

    for(i = 0; i < nof_levels; i++)
        fprintf(fp, "%f %f\n", info[i] . x_size, info[i] . y_size);

    for(i = 0; i < nof_levels; i++)
        fprintf(fp, "%d %d\n", info[i] . max_faces, info[i] . max_vertices);

    fclose (fp);
}

/*===========================================================================*/
void	expand_asd_box (ASD_tile *tile, pfBox *box, int virgin)
/*===========================================================================*/
{
    pfVec3 	*points;
    int		nof_vertices;
    pfBox	tmp_box;
    int		base;
    int		i, j;

    nof_vertices = 0;
    for (i = 0 ; i < tile -> l . nof_levels ; i ++)
	nof_vertices += tile -> l . vertex_level[i] . nof_vertices;

    printf ("\texpand_asd_box: Tile has %d vertices\n", nof_vertices);

    points = (pfVec3 *) alloca (nof_vertices * sizeof (pfVec3));

    base = 0;
    for (i = 0 ; i < tile -> l . nof_levels ; i ++)
    {
	for (j = 0 ; j < tile -> l . vertex_level[i] . nof_vertices ; j ++)
	{
	    pfCopyVec3 (points[base + j], 
		    tile -> l . vertex_level[i] . vertex[j] . v0);

	    if (base + j >= nof_vertices)
		printf ("#### >>>>>> Overflow # 1\n");

	    if (0) printf ("\t\t[%d, %d] = (%f, %f, %f)\n", 
			i, 
			j, 
			points[base + j][0], 
			points[base + j][1], 
			points[base + j][2]);
	}

	base += tile -> l . vertex_level[i] . nof_vertices;
    }

    if (virgin)
	pfBoxAroundPts (box, points, nof_vertices);
    else
    {
	pfBoxAroundPts (&tmp_box, points, nof_vertices);
	pfBoxExtendByBox (box, &tmp_box);
    }

    printf ("\t\tTile Bbox: (%f, %f)x(%f, %f)x(%f, %f)\n", 
		box -> min[0], box -> max[0], 
		box -> min[1], box -> max[1], 
		box -> min[2], box -> max[2]);
}

/*===========================================================================*/
void	dump_vertices (ASD_vertices *v_levels, int nof_levels)
/*===========================================================================*/
{
    pfASDVert	*v;
    int level;
    int	i;

    printf ("##### DUMP VERTICES\n");
    for (level = 0 ; level < nof_levels ; level ++)
    {
 	printf ("======================= LEVEL %d\n", level);
	v = v_levels[level] . vertex;

	for (i = 0 ; i < v_levels[level] . nof_vertices ; i ++)
	{
	    printf ("\tvertid[%d] = %d\n",  i, v -> vertid);
	    v ++;
	}
    }
}

/*===========================================================================*/
void 	init_tile_neighborhood (ASD_tile_neighborhood *tiles, int nof_levels)
/*===========================================================================*/
{
    int		i;

    tiles -> nof_levels = nof_levels;

    tiles -> center = (ASD_level *) malloc (nof_levels * sizeof (ASD_level));
    tiles -> right = (ASD_level *) malloc (nof_levels * sizeof (ASD_level));
    tiles -> left = (ASD_level *) malloc (nof_levels * sizeof (ASD_level));
    tiles -> up = (ASD_level *) malloc (nof_levels * sizeof (ASD_level));
    tiles -> down = (ASD_level *) malloc (nof_levels * sizeof (ASD_level));
    tiles -> up_right = (ASD_level *) malloc (nof_levels * sizeof (ASD_level));

    for (i = 0 ; i < nof_levels ; i ++)
    {
	init_level (&tiles -> center[i]);
	init_level (&tiles -> right[i]);
	init_level (&tiles -> left[i]);
	init_level (&tiles -> up[i]);
	init_level (&tiles -> down[i]);
	init_level (&tiles -> up_right[i]);
    }
}

/*===========================================================================*/
int  prune_tile_level (ASD_tile_neighborhood *tiles, int level, float epsilon)
/*===========================================================================*/
{
    int		i, j;
    int		nof_faces;
    ASD_level	*t_level;
    int		children[4], grands[4];
    int		nof_children, nof_grands;
    int		neighbors[4][3];
    pfASDFace	*face;
    pfASDVert	*vert[3], *split[3];
    int		f;
    int		found;
    int		dummy[4];
    int		prune_count;
    int		condensed[12];
    int		ii, jj, kk;
    int		nof_neighbors;
    int		test_neighbor;
    int		n;

    t_level = &tiles -> center[level];
    nof_faces = t_level -> faces . nof_faces;
    prune_count = 0;

    for (i = 0 ; i < nof_faces ; i ++)
    {
	f = t_level -> faces . face_id[i];
	face = & t_level -> faces . face[i];
	/*
	 *	===========================================================
	 *    	If face has no children - case closed (already pruned)
	 *	===========================================================
	 */
	if ((nof_children = get_face_children
				(tiles, level, f, children)) > 0)
	{
	    if (nof_children < 0)
		printf ("################ 1 faceid not found\n");
	    /*
	     *	==========================================================
	     *	Count grand children of face. If got any - can not prune.
	     *	==========================================================
	     */
	    nof_grands = 0;
	    for (j = 0 ; j < nof_children ; j ++)
	    {
		n = get_face_children(tiles, level + 1, children[j], grands);

		nof_grands += n;

		if (n < 0)
		    printf ("###################### 2 faceid not found\n");
	    }

	    /*
	     *	================================================
	     *	Face has no grandchildren - OK to continue. 
	     *	================================================
	     */
	    if (nof_grands == 0)
	    {
		/*	
		 *	===============================================
		 *	Check only neighbors of child 0, 1, 2. Child # 3
		 *	is the center child - no need to check.
		 *	===============================================
		 */
		for (j = 0 ; j < 3 ; j ++)
		    get_face_neighbors (tiles, 
					level + 1, 
					children[j], 
					neighbors[j]);

		/*
		 *	===============================================
		 *	Condense neighbor list.	
		 *	===============================================
		 */
		nof_neighbors = 0;
		for (ii = 0 ; ii < 3 ; ii ++)
		    for (jj = 0 ; jj < 3 ; jj ++)
		    {
			test_neighbor = neighbors[ii][jj];

			found = 0;
			for (kk = 0 ; (kk < nof_neighbors) && (! found) ; kk ++)
			    if (condensed[kk] == test_neighbor)
				found = 1;

			if ((! found) && 
			    (test_neighbor != children[0]) &&
			    (test_neighbor != children[1]) &&
			    (test_neighbor != children[2]) &&
			    (test_neighbor != children[3]))
			    condensed[nof_neighbors++] = test_neighbor;
		    }
		
#if 0
		printf ("Found %d NEIGHBORS: ", nof_neighbors);
		for (j = 0 ; j < nof_neighbors ; j ++)
		    printf ("%d, ", condensed[j]);
		printf ("\n");
#endif

                found = 0;
		for (j = 0 ; (j < nof_neighbors) && (! found) ; j ++)
		{
		    if ((n = get_face_children(
			    tiles, level + 1, condensed[j], dummy)) > 0)
			found = 1;

		    if (n < 0)
			printf ("# 3 faceid not found\n");
		}

		/*
		 *	==============================================
 		 *	Neighbors of children have no children - 
		 *	May finally get to the pruning test
		 *	==============================================
		 */
		if (! found)
		{
   		    int		triangle_ok;

		    triangle_ok = 1;
		    for (j = 0 ; j < 3 ; j ++)
		    {
			vert[j] = get_nvertex_by_vertid (
						tiles, face -> vert[j]);
			split[j] = get_nvertex_by_vertid (
						tiles, face -> refvert[j]);

			if ((vert[j] == NULL) || (split[j] == NULL))
			{

			    printf ("Missing vertex:\n");
			    printf ("\tvertex %d --> 0x%x\n", 
					face -> vert[j], (int) vert[j]);
			    printf ("\tsplit %d --> 0x%x\n", 
					face -> refvert[j], (int) split[j]);
			    triangle_ok = 0;
			}
		    }

		    if (triangle_ok)
			if (prune_face_ok (vert, split, epsilon)) 
			{
			    face -> child[0] = PFASD_NIL_ID;
			    face -> child[1] = PFASD_NIL_ID;
			    face -> child[2] = PFASD_NIL_ID;
			    face -> child[3] = PFASD_NIL_ID;
			    prune_count ++;
			}
		}
   	    }
	}
    }

    return (prune_count);
}

/*===========================================================================*/
int	get_face_children(ASD_tile_neighborhood *tiles, 
				int level, int face_id, int children[4])
/*===========================================================================*/
{
    int		f;
    ASD_level	*tile;
    int		i, j;
    pfASDFace	*face;

    if (face_id == PFASD_NIL_ID)
	return (0);
    /*
     *	First, find face with correct face_id in some tile. 
     */
    if ((f = lookup_face_by_id (&tiles -> center[level] . faces, face_id)) >= 0)
	tile = tiles -> center;
    else
    if ((f = lookup_face_by_id (&tiles -> right[level] . faces, face_id)) >= 0)
	tile = tiles -> right;
    else
    if ((f = lookup_face_by_id (&tiles -> left[level] . faces, face_id)) >= 0)
	tile = tiles -> left;
    else
    if ((f = lookup_face_by_id (&tiles -> up[level] . faces, face_id)) >= 0)
	tile = tiles -> up;
    else
    if ((f = lookup_face_by_id (&tiles -> down[level] . faces, face_id)) >= 0)
	tile = tiles -> down;
    else
    {
	int	lev, found;

	printf ("get_face_children: level %d, face_id %d, Not found .\n", 
			level, face_id);
	found = 0;
	for (i = 0 ; i < tiles -> nof_levels ; i ++)
	{
	    if (lookup_face_by_id (&tiles -> center[i] . faces, face_id) >= 0)
	    {
		lev = i;
		found = 1;
	    }
	    else
	    if (lookup_face_by_id (&tiles -> right[i] . faces, face_id) >= 0)
	    {
		lev = i;
		found = 1;
	    }
	    else
	    if (lookup_face_by_id (&tiles -> left[i] . faces, face_id) >= 0)
	    {
		lev = i;
		found = 1;
	    }
	    else
	    if (lookup_face_by_id (&tiles -> up[i] . faces, face_id) >= 0)
	    {
		lev = i;
		found = 1;
	    }
	    else
	    if (lookup_face_by_id (&tiles -> down[i] . faces, face_id) >= 0)
	    {
		lev = i;
		found = 1;
	    }
	}

	if (found)
	    printf ("\tFOUND instead in level %d\n", lev);
	else
	    printf ("\tNot found anywhere\n");

#if 1
	printf ("======================= CENTER:\n");
	dump_levels (tiles -> center, tiles -> nof_levels);
	printf ("======================= UP:\n");
	dump_levels (tiles -> up, tiles -> nof_levels);
	printf ("======================= DOWN:\n");
	dump_levels (tiles -> down, tiles -> nof_levels);
	printf ("======================= RIGHT:\n");
	dump_levels (tiles -> right, tiles -> nof_levels);
	printf ("======================= LEFT:\n");
	dump_levels (tiles -> left, tiles -> nof_levels);
#endif

	return (-1);
    }


    face = &tile[level] . faces . face[f];

    i = 0;
    for (j = 0 ; j < 4 ; j ++)
	if (face -> child[j] != PFASD_NIL_ID)
	{
	    children[i] = face -> child[j];
	    i ++;
	}
     
    return (i);
}

/*===========================================================================*/
void    get_face_neighbors (
			ASD_tile_neighborhood 	*tiles, 
			int 			level, 
			int			face_id, 
			int			neighbors[3])
/*===========================================================================*/
{
    int		i;
    pfASDFace	*face;
    int		f;
    ASD_level 	*tile;

    if (face_id == PFASD_NIL_ID)
    {
	neighbors[0] = PFASD_NIL_ID;
	neighbors[1] = PFASD_NIL_ID;
	neighbors[2] = PFASD_NIL_ID;
	return;
    }

    /*
     *	Find face with correct face_id in some tile. 
     */
    if ((f = lookup_face_by_id (&tiles -> center[level] . faces, face_id)) >= 0)
	tile = tiles -> center;
    else
    if ((f = lookup_face_by_id (&tiles -> right[level] . faces, face_id)) >= 0)
	tile = tiles -> right;
    else
    if ((f = lookup_face_by_id (&tiles -> left[level] . faces, face_id)) >= 0)
	tile = tiles -> left;
    else
    if ((f = lookup_face_by_id (&tiles -> up[level] . faces, face_id)) >= 0)
	tile = tiles -> up;
    else
    if ((f = lookup_face_by_id (&tiles -> down[level] . faces, face_id)) >= 0)
	tile = tiles -> down;
    else
	printf ("get_face_neighbors: %d, No such face in tiles.\n", face_id);


    face = &tile[level] . faces . face[f];

    for (i = 0 ; i < 3 ; i ++)
    {
	if (face -> refvert[i] != PFASD_NIL_ID)
	{
	    if (lookup_vertex_neighbor_by_vertid (
					tiles -> center, 
					tiles -> nof_levels, 
					face -> refvert[i],
					face_id,
					&neighbors[i]) < 0)
		    if (lookup_vertex_neighbor_by_vertid (
					tiles -> up, 
					tiles -> nof_levels, 
					face -> refvert[i],
					face_id,
					&neighbors[i]) < 0)
		    if (lookup_vertex_neighbor_by_vertid (
					tiles -> right, 
					tiles -> nof_levels, 
					face -> refvert[i],
					face_id, 
					&neighbors[i]) < 0)
		    if (lookup_vertex_neighbor_by_vertid (
					tiles -> down, 
					tiles -> nof_levels, 
					face -> refvert[i],
					face_id, 
					&neighbors[i]) < 0)
		    if (lookup_vertex_neighbor_by_vertid (
					tiles -> left, 
					tiles -> nof_levels, 
					face -> refvert[i],
					face_id, 
					&neighbors[i]) < 0)
		
		    {
			printf ("get_face_neighbors: face %d, "
				"refvert[%d]=%d, not found.\n",
				face_id, i, face -> refvert[i]);

			neighbors[i] = PFASD_NIL_ID;
		    }
	}
	else
	    neighbors[i] = PFASD_NIL_ID;
    }
}

/*===========================================================================*/
int lookup_vertex_neighbor_by_vertid (
				ASD_level	*tile, 
				int		nof_levels, 
				int		vert_id,
				int		ignore_face_id,
				int		*neighbor)
/*===========================================================================*/
{
    int		found, i, v;
    pfASDVert	*vertex;

    found = 0;
    for (i = 0 ; (i < nof_levels) && (! found) ; i ++)
    {
	if ((v = lookup_vertex_by_vertid (& tile[i].vertices, vert_id)) >= 0)
	{
	    vertex = & tile[i].vertices.vertex[v];
	    found = 1;
	}
    }

    if (! found)
	return (-1);

    if (vertex -> neighborid[0] == ignore_face_id)
	*neighbor = vertex -> neighborid[1];
    else
	*neighbor = vertex -> neighborid[0];

    return (1);
}

/*===========================================================================*/
pfASDVert *get_nvertex_by_vertid (ASD_tile_neighborhood *tiles, int vert_id)
/*===========================================================================*/
{
    int	i, v;

    /*
     *	We always query vertices used by the center tile, so they
     *	may be in either the center, right or up tiles.
     */
    for (i = 0 ; i < tiles -> nof_levels ; i ++)
	if ((v = lookup_vertex_by_vertid (& tiles -> center[i].vertices, 
		vert_id)) >= 0)
	    return (& tiles -> center[i].vertices.vertex[v]);

    for (i = 0 ; i < tiles -> nof_levels ; i ++)
	if ((v = lookup_vertex_by_vertid (& tiles -> right[i].vertices, 
		vert_id)) >= 0)
	    return (& tiles -> right[i].vertices.vertex[v]);

    for (i = 0 ; i < tiles -> nof_levels ; i ++)
	if ((v = lookup_vertex_by_vertid (& tiles -> up[i].vertices, 
		vert_id)) >= 0)
	    return (& tiles -> up[i].vertices.vertex[v]);

    for (i = 0 ; i < tiles -> nof_levels ; i ++)
	if ((v = lookup_vertex_by_vertid (& tiles -> up_right[i].vertices, 
		vert_id)) >= 0)
	    return (& tiles -> up_right[i].vertices.vertex[v]);

    printf ("get_nvertex_by_vertid: Vertex %d not found\n", vert_id);
    return (NULL);
}

/*===========================================================================*/
int	prune_face_ok (pfASDVert *vert[], pfASDVert *split[], float epsilon)
/*===========================================================================*/
{
    pfVec3	d0, d1, n;
    float	area;
    float	a, b, c, d;
    float	x, y, z;
    int		i;
    float	dist;
    static	int	count=0;

    count ++;
    if ((count % 500) == 0)
	printf ("prune_face_ok  .... %d times\n", count);

    /*
     *  =============================================
     *  Calculate plane equation of parent face
     *  =============================================
     */
    pfSubVec3 (d0, vert[1] -> v0, vert[0] -> v0);
    pfSubVec3 (d1, vert[2] -> v0, vert[0] -> v0);

    pfCrossVec3 (n, d0, d1);

    area = pfNormalizeVec3 (n);

    /*
     *  Face plane equation.
     */
    a = n[0];
    b = n[1];
    c = n[2];
    d = - (a * vert[0] -> v0[0] + b * vert[0] -> v0[1] + c * vert[0] -> v0[2]);

    /*
     *  =============================================
     *  Look for a ref-vertex far enough from parent
     *  =============================================
     */
    for (i = 0 ; i < 3 ; i ++)
    {
        if (split[i] != NULL)
        {
            x = split[i] -> v0[0];
            y = split[i] -> v0[1];
            z = split[i] -> v0[2];

            dist = x * a + y * b + z * c + d;
            if (dist > area * epsilon)
                return (0);
        }
    }

    return (1);
}

/*===========================================================================*/
void    prune_tiles (
		char 	*base_filename, 
		int 	nof_levels, 
		int	x_nof_tiles_across, 
		int	y_nof_tiles_across, 
		float	prune_epsilon)
/*===========================================================================*/
{
    int		i, j, k, level;
    char	filename[1000];
    int		c;
    
    ASD_tile_neighborhood	tiles;

    init_tile_neighborhood (&tiles, nof_levels + 1);

    for (level = nof_levels - 2 ; level >= 0 ; level --)
    {
	printf ("Prune level %d\n", level);
	for (i = 0 ; i < x_nof_tiles_across ; i ++)
	    for (j = 0 ; j < y_nof_tiles_across ; j ++)
	    {
		/*
		 *	=====================================
	 	 *	Cleanup for new tile/level
		 *	=====================================
		 */
		for (k = 0 ; k < nof_levels ; k ++)
		{
		    zero_level (&tiles . center[k]);
		    zero_level (&tiles . up[k]);
		    zero_level (&tiles . down[k]);
		    zero_level (&tiles . right[k]);
		    zero_level (&tiles . left[k]);
		    zero_level (&tiles . up_right[k]);
		}

		/*
		 *	================================================
		 *	Populate neighborhood with center tile and four
		 *	neighbor tiles.
		 *	Load (nof_levels+1) levels because last one 
		 *	contains reference vertices.
		 *	================================================
		 */
		for (k = 0 ; k <= nof_levels ; k ++)
		{
		    sprintf(filename, "%s%02d%03d%03d", base_filename, k, i, j);
		    merge_level (& tiles . center[k], filename);

		    if (i + 1 < x_nof_tiles_across)
		    {
			sprintf(filename, "%s%02d%03d%03d", 
					base_filename, k, i + 1, j);
			merge_level (& tiles . right[k], filename);
		    }

		    if (j + 1 < y_nof_tiles_across)
		    {
			sprintf(filename, "%s%02d%03d%03d", 
					base_filename, k, i, j + 1);
			merge_level (& tiles . up[k], filename);
		    }

		    if (i - 1 >= 0)
		    {
			sprintf(filename, "%s%02d%03d%03d", 
					base_filename, k, i - 1, j);
			merge_level (& tiles . left[k], filename);
		    }


		    if (j - 1 >= 0)
		    {
			sprintf(filename, "%s%02d%03d%03d", 
					base_filename, k, i, j - 1);
			merge_level (& tiles . down[k], filename);
		    }

		    if ((j + 1 < y_nof_tiles_across) && 
			(i + 1 < x_nof_tiles_across))
		    {
			sprintf(filename, "%s%02d%03d%03d", 
					base_filename, k, i + 1, j + 1);
			merge_level (& tiles . up_right[k], filename);
		    }
		}

		c = prune_tile_level (&tiles, level, prune_epsilon);

		printf ("\tlevel %d, tile (%d, %d), prune = %d\n", 
						level, i, j, c);

		/*
		 *	================================================
		 *	If we managed to prune something - save result.
		 *	================================================
		 */
		if (c > 0)
		{
		    sprintf(filename, 
				"%s%02d%03d%03d", base_filename, level, i, j);
		    save_level (&tiles . center[level], filename);
		}
	    }
    }
}

/*===========================================================================*/
void dump_levels (ASD_level *level, int nof_levels)
/*===========================================================================*/
{
    int		i, j;

    for (i = 0 ; i < nof_levels ; i ++)
    {
	printf ("LEVEL %d\n", i);
	for (j = 0 ; j < level[i] . faces . nof_faces ; j ++)
	    printf ("\tface_id [%d] = %d, ref = (%d, %d, %d)\n", 
	    j, level[i] . faces . face_id[j],
	    level[i] . faces . face[j] . refvert[0], 
	    level[i] . faces . face[j] . refvert[1], 
	    level[i] . faces . face[j] . refvert[2]);

	for (j = 0 ; j < level[i] . vertices . nof_vertices ; j ++)
	    printf ("\tneighborid [%d] = %d, %d, vertid = %d\n", j, 
			level[i] . vertices . vertex[j].neighborid[0], 
			level[i] . vertices . vertex[j].neighborid[1],
			level[i] . vertices . vertex[j].vertid);
    }
}


/*===========================================================================*/
float	default_grid_elevation (void *user_data, int x, int y)
/*===========================================================================*/
{
    if (!user_data)
    {
    }

    return (200.0 * sin (((float) (x * y)) / 100.0));
}


/*===========================================================================*/
void	init_line_segments (ASD_segments *seg)
/*===========================================================================*/
{
    seg -> segment = NULL;
    seg -> nof_segments = 0;
    seg -> nof_allocated_segments = 0;
}

/*===========================================================================*/
void	init_triangles (ASD_triangles *tri)
/*===========================================================================*/
{
    tri -> triangle = NULL;
    tri -> nof_triangles = 0;
    tri -> nof_allocated_triangles = 0;
}

/*===========================================================================*/
void	init_constraints (ASD_constraints *con)
/*===========================================================================*/
{
    init_line_segments (&con -> segments);
    init_triangles (&con -> triangles);
}
