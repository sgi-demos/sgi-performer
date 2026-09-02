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

#ifndef PFD_ASDGEN_H
#define PFD_ASDGEN_H

/*
 *      ==================================================
 *	Enums
 *      ==================================================
 */

enum	{
	ASDGEN_PLANE,
	ASDGEN_SPHERE
	};

/*
 *      ==================================================
 *      Types
 *      ==================================================
 */

typedef struct
{
    float               x, y;
} pos2;

typedef struct
{
    pfVec3              *points;
    int                 x_size, y_size;
    float               x_scale, y_scale;
    float               x_base, y_base;
} grid3;

typedef struct
{
    pfVec3              *points;
    int                 nof_points;
    int                 nof_allocated_points;
} section3;

typedef struct
{
    float               grid_x[3];
    float               grid_y[3];
    pfVec3              v[3];
    int                 v_index[3];
} triangle;

typedef struct
{
    int                 *vertex_in_tile;
    int                 *vertex_id;
    pfASDVert           *vertex;
    pos2                *pos;
    int                 nof_vertices;
    int                 nof_allocated_vertices;
} ASD_vertices;

typedef struct
{
    int                 *face_id;
    pfASDFace           *face;
    pfBox               *bbox;
    int                 nof_faces;
    int                 nof_allocated_faces;
} ASD_faces;

typedef struct
{
    int                 nof_levels;
    ASD_faces           *face_level;
    ASD_vertices        *vertex_level;
} ASD_levels;

typedef struct
{
    int                 *level_base_vertex;
    int                 *level_base_face;
    int                 *x_level_vertices_across;
    int                 *y_level_vertices_across;
    int                 *level_nof_faces;
    int                 *level_nof_vertices;

    int                 x_tiles_across, y_tiles_across;
    int                 x, y;
} ASD_tile_info;

typedef struct
{
    ASD_levels          l;
    ASD_tile_info       info;
} ASD_tile;

typedef struct
{
    int                 x, y;
    int                 x_size, y_size;
    int                 is_up;
    int                 *ts_base;
} tstrip_info;

typedef struct
{
    int                 *nof_faces;
    int                 *nof_vertices;
    int                 *base_face;
    int                 *base_vertex;
} ASD_tile_stats;
typedef struct
{
    ASD_faces           faces;
    ASD_vertices        vertices;
} ASD_level;

typedef struct
{
    int         	    x_tiles, y_tiles;
    float       	    x_size, y_size;
    int         	    max_faces, max_vertices;
} ASD_final_info;

typedef struct
{
    ASD_level           *center;
    ASD_level           *right, *left, *up, *down, *up_right;

    int                 nof_levels;

} ASD_tile_neighborhood;

/*
 *      ==================================================
 *      Constraining triangles/line-segments
 *      ==================================================
 */

typedef struct
{
    pfVec3		v[3];
} ASD_triangle;

typedef struct
{
    pfVec3 		v[2];
} ASD_segment;

typedef struct
{
    int			nof_triangles;
    int			nof_allocated_triangles;
    ASD_triangle	*triangle;
} ASD_triangles;

typedef struct
{
    int			nof_segments;
    int			nof_allocated_segments;
    ASD_segment		*segment;
} ASD_segments;

typedef struct
{
    ASD_triangles	triangles;
    ASD_segments	segments;
    
} ASD_constraints;

/*
 *      ==================================================
 *      Main Structure
 *      ==================================================
 */

struct _pfdASDGenerator
{
    int              		x_grid_size, y_grid_size;

    int              		enable_prune;
    float            		prune_epsilon;

    int              		tile_size;
    int              		nof_levels;

    char             		temp_base_filename[2000];
    char             		base_filename[2000];


    int              		*base_vertex;
    int              		*level_faces;
    int              		tile_lookahead[2];

    /* 
     *	=======================================
     *	When do we consider two points in 
     *	grid space a single point.
     *	=======================================
     */
    float			grid_epsilon;

    /*
     *	=======================================
     *	User supplied routine for extracting
     *	grid points (e.g. from Dted file).
     *	=======================================
     */
    pfd_grid_elevation_func	get_elevation;
    void			*user_data;

    /*
     *	=======================================
     *	Flat vs Spherical ASD.
     *	=======================================
     */
    int				asd_geometry;

    /*
     *	=======================================
     *	Flat model parameters. reuse for 
     *	spherical model.
     *	=======================================
     */
    float            		x_span, y_span;
    float			x_base, y_base;

    /*
     *	=======================================
     *	Spherical model parameters.
     *	=======================================
     */
    float			sphere_radius;
    pfVec3			sphere_center;
    int				x_wraparound, y_combine;

    /*
     *	===============================================
     *	Additional geometric constraints to 
     *	the mesh. 
     *	e.g. road/runway surfaces and edges.
     *	===============================================
     */
 
    ASD_constraints		constraints;
    

};


/*
 *	==================================================
 *	Prototypes
 *	==================================================
 */

void 	save_tile (
		ASD_tile 	*tile, 
		char 		*base_name, 
		int		x, 
		int		y);

void    save_configuration_file(
		ASD_tile_stats 	*stats, 
		ASD_tile_info 	*info, 
		int		nof_levels,
		char 		*base_filename, 
		char 		*filename);

void	expand_asd_box (ASD_tile *tile, pfBox *box, int virgin);

/*
 *	==================================================
 *	Debug routines.
 *	==================================================
 */
void	dump_vertices (ASD_vertices *v_levels, int nof_levels);
void 	dump_levels (ASD_level *level, int nof_levels);
void 	tabs (int n);


/*
 *	==================================================
 *	Level manipulation and file I/O
 *	==================================================
 */

void zero_level (ASD_level *level);

void init_level (ASD_level *level);

void merge_level (ASD_level *level, char *filename);

void save_level (ASD_level *level, char *filename);

void    rearrange_tiles (
		char *in_filename, 
		char *out_filename, 
		int nof_levels, 
		int x_level_0_across, 
		int y_level_0_across, 
		pfBox *box, 
		char *cfg_filename);

/*
 *	==================================================
 *	Face BBoxes. 
 *	==================================================
 */
void calc_tile_face_bbox (ASD_tile *tile);

void calc_face_bbox (
		    ASD_faces 		*f_levels, 
		    ASD_vertices 	*v_levels, 
		    int 		level, 
		    int 		face_id);

/*
 *	==================================================
 *	Face Neighbors. 
 *	==================================================
 */
void calc_tile_face_neighbors (ASD_tile *tile);

void	calc_face_neighbors (
		    ASD_faces 		*f_levels, 
		    ASD_vertices 	*v_levels, 
		    int 		level, 
		    int 		face_id);

/*
 *	==================================================
 *	2D grid and 1D grid sections.
 *	==================================================
 */
void select_split_point_1D (
		section3 	*section, 
		pfVec3 		result, 
		float 		*t);

void calculate_section_midpoint (
		section3 	*section, 
		float 		t, 
		pfVec3 		midpoint);

void calc_point_2D (
		grid3 		*grid, 
		float 		x, 
		float 		y, 
		pfVec3 		result);

void compute_section_1D (
		grid3 		*grid, 
		section3 	*section, 
		float 		from_x, 
		float 		from_y, 
		float 		to_x, 
		float 		to_y);

float calc_point_line_distance_2 (
		pfVec3          p,
		pfVec3          p1,
		pfVec3          p2);

void init_section (	
		section3 	*section);

void extend_section (
		section3 	*section, 
		int 		n);

void extract_subgrid (
		pfdASDGenerator 	*gen, 
		int 			from_x, 
		int 			from_y, 
		grid3 			*grid);

/*
 *	============================================
 *	Top level routines to handle grids.
 *	============================================
 */
void grid2ASD (
		pfdASDGenerator		*gen,
		ASD_tile 		*tile, 
		grid3 			*grid, 
		int 			nof_levels);

int  triangle2ASD (
		pfdASDGenerator		*gen, 
		ASD_tile 		*tile, 
		grid3 			*grid, 
		int 			level, 
		int 			nof_levels, 
		triangle 		*tri,
		tstrip_info 		*ts_info);

/*
 *	============================================
 *	Dynamic Level Stuff.
 *	============================================
 */
void	init_levels (ASD_levels *levels);
void	zero_levels (ASD_levels *levels);

int	add_level_face (
		ASD_levels 		*levels, 
		pfASDFace 		*face,
		int			id);

int	add_level_vertex (
		ASD_levels 		*levels, 
		int			level,
		pfASDVert 		*vert, 
		float 			x, 
		float 			y, 
		int			is_in_tile);

int	lookup_vertex_by_vertid (
		ASD_vertices 		*verts, 
		int 			vertid);

int     global_lookup_vertex_by_vertid (
		ASD_vertices    	*v_levels,
		int             	max_level, 
		int             	vertid, 
		int             	*vertex_level, 
		int             	*vertex_id);

int 	lookup_face_by_id (
		ASD_faces 		*faces, 
		int 			id);

/*
 *	============================================
 *	Dynamic Face Stuff.
 *	============================================
 */
void	init_faces (ASD_faces *faces);
void	zero_faces (ASD_faces *faces);
int 	add_face (ASD_faces *faces, pfASDFace *face, int id);

/*
 *	============================================
 *	Dynamic Vertex Stuff.
 *	============================================
 */
void	init_vertices (ASD_vertices *verts);
void	zero_vertices (ASD_vertices *verts);
int	add_vertex (ASD_vertices *verts, pfASDVert *v, 
			float x, float y, int is_in_tile);
/*
 *	============================================
 *	ASD tile handling
 *	============================================
 */
void	init_asd_tile (ASD_tile *t);
void	zero_asd_tile (ASD_tile *t);
int	make_asd_vertex (
		pfdASDGenerator *gen,
		ASD_tile 	*tile, 
		grid3 		*grid, 
		int 		level, 
		float  		px, 
		float  		py, 
		int   		x, 
		int   		y, 
		pfVec3 		d, 
		int		n0, 
		int		n1);

ASD_tile_stats *init_asd_tile_stats (
		int 		nof_tiles, 
		int 		nof_levels);

void  	collect_asd_tile_stats (
		ASD_tile_stats 	*stats, 
		ASD_tile 	*tile, 
		int 		nof_levels);

void  	calc_asd_tile_stats (
		ASD_tile_stats 	*stats, 
		int 		nof_tiles, 
		int 		nof_levels);

int   	calc_asd_vertex_index (
		pfdASDGenerator *gen,
		ASD_tile_info 	*info, 
		int 		level, 
		int 		x, 
		int 		y);

void  	calc_asd_tile_info (	
		ASD_tile_info 	*info, 
		int 		x_nof_tiles_across, 
		int 		y_nof_tiles_across, 
		int 		nof_levels);

void	calc_vertex_tile (
		ASD_tile_info 	*info, 
		int 		level, 
		int 		x, 
		int 		y, 
		int 		*tx, 
		int 		*ty);

int	is_vertex_in_tile (
		ASD_tile 	*tile, 
		int 		level, 
		int 		x, 
		int 		y);
/*
 *	==================================================
 *	Pruning the tile files ... 
 *	==================================================
 */

void    prune_tiles (
		char 				*base_filename, 
		int 				nof_levels, 
		int				x_nof_tiles_across, 
		int				y_nof_tiles_across, 
		float				prune_epsilon);

void    init_tile_neighborhood (
		ASD_tile_neighborhood 		*tiles, 
		int 				nof_levels);

int  	prune_tile_level (
		ASD_tile_neighborhood 		*tiles, 
		int 				level, 
		float 				epsilon);

int	get_face_children(
		ASD_tile_neighborhood 		*tiles, 
		int 				level, 
		int 				face_id, 
		int 				children[4]);

void    get_face_neighbors (
		ASD_tile_neighborhood 		*tiles, 
		int 				level, 
		int				face_id, 
		int				neighbors[3]);

int 	lookup_vertex_neighbor_by_vertid (
		ASD_level			*tile, 
		int				nof_levels, 
		int				vert_id,
		int				ignore_face_id, 
		int				*neighbor);

pfASDVert *get_nvertex_by_vertid (
		ASD_tile_neighborhood 		*tiles, 
		int 				vert_id);

int	prune_face_ok (
		pfASDVert 			*vert[], 
		pfASDVert 			*split[], 
		float 				epsilon);

float   default_grid_elevation (void *user_data, int x, int y);

void   	init_line_segments (ASD_segments *seg);

void   	init_triangles (ASD_triangles *tri);

void   	init_constraints (ASD_constraints *con);

 
#endif
