/*
 * Copyright (c) 2000, Silicon Graphics, Inc.
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
	pagedASD

 	Input: 
		a *.fasd file containing instructions for the generation of
		a fractal terrain.

	Output: 
		input files for convasd

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#ifdef WIN32
#define random rand
#define srandom srand
#else
#include <strings.h>
#include <sys/resource.h>
#include <alloca.h>
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>

typedef struct	
{
    float	*data;
    int		size;
} level_type;

/*
 *	==================================================
 *	Fract prototypes.
 *	==================================================
 */

void 	init_globals(void);
void 	read_params(int argc, char *argv[]);
void 	init_mesh(void);
void 	make_mesh_level (int level, int new_size);
float 	fract (float v0, float v1);
void    read_file (char *filename);
float	array_point (void *user_data, int x, int y);
void	match_sphere_seams (level_type *level);

/*
 *	==================================================
 *	Global Variables
 *	==================================================
 */


static int		size;
static float		min_elevation, max_elevation;
static float		random_intensity;
static int		nof_levels;
static float		x_span, y_span;
static float		x_base, y_base;
static int		is_zero_edge;
static char		output_file[200];
static char		config_file[200];
static int		is_flatten_zero;
static int		tile_size;
static int		nof_asd_levels;
static char		temp_base_filename[300];
static char		base_filename[300];
static float		prune_epsilon;
static int		enable_prune;

static int		is_sphere;
static float		sphere_radius;
static pfVec3		sphere_center;

static int		tile_lookahead[2];


level_type		*levels;



/*===========================================================================*/
void main(int argc, char *argv[])
/*===========================================================================*/
{
    int			new_size, i;
    pfdASDGenerator	*asdgen;

    init_globals();

    read_params(argc, argv);

    /*
     *	=====================================================================
     *	Generate fractal mesh with nof_levels levels.
     *	=====================================================================
     */

    init_mesh();

    new_size = size;

    for (i = 1 ; i < nof_levels ; i ++)
	{
	new_size = new_size * 2 - 1;
	
	printf ("building level %d, size %d\n", i, new_size);
	make_mesh_level (i, new_size);
	}

    if (is_sphere && (x_span >= 359.99))
	match_sphere_seams (& levels [nof_levels - 1]);


    /*
     *	=====================================================================
     *	Generate an ASD from the fractal mesh.
     *	=====================================================================
     */

    asdgen = pfdNewASDGen();

    if (is_sphere)
	pfdASDGenSphereGeometry (asdgen, sphere_radius, sphere_center, 
					x_base * M_PI / 180.0, 
					y_base * M_PI / 180.0, 
					x_span * M_PI / 180.0, 
					y_span * M_PI / 180.0);
    else
	pfdASDGenPlaneGeometry (asdgen, x_base, y_base, 
					x_span, y_span);

    pfdASDGenElevationFunc (asdgen, array_point, 
				(void *) & levels[nof_levels - 1]);

    pfdASDGenPrune (asdgen, enable_prune, prune_epsilon);

    pfdASDGenGridSize (asdgen, levels[nof_levels - 1] . size, 
			       levels[nof_levels - 1] . size);

    pfdASDGenTileSize (asdgen, tile_size);

    pfdASDGenNofLevels (asdgen, nof_asd_levels);

    pfdASDGenTempFile (asdgen, temp_base_filename);

    pfdASDGenOutputFile (asdgen, base_filename);

    pfdASDGenLookahead (asdgen, tile_lookahead[0], tile_lookahead[1]);

    pfdASDGenerate (asdgen);

}

/*===========================================================================*/
void init_globals(void)
/*===========================================================================*/
{
min_elevation = -500.0;
max_elevation = 500.0;
random_intensity = 0.5;
x_span = 10000.0;
y_span = 10000.0;
x_base = 0.0;
y_base = 0.0;
is_zero_edge = 1;
is_flatten_zero = 1;
enable_prune = 0;
prune_epsilon = 0.001;

tile_lookahead [0] = 3;
tile_lookahead [1] = 3;

size = 5;
nof_levels = 5;
tile_size = 11;
nof_asd_levels = 5;

sprintf (temp_base_filename, "tmp/temp_new_asd");
sprintf (base_filename, "tmp/new_asd");

is_sphere = 0;
sphere_center[0] = 0.0;
sphere_center[1] = 0.0;
sphere_center[2] = 0.0;
sphere_radius = 20000.0;

}

/*===========================================================================*/
void read_params(int argc, char *argv[])
/*===========================================================================*/
{
    int	i;

    if (argc > 0)
	{
	    i = 1;
	    while (i < argc)
	    {
		if (argv[i][0] == '-')
		{
		    switch (argv[i][1])
		    {
			case 'o':
			    i ++;
			    strcpy(output_file, argv[i]);
			    break;

			case 'c':
			    i ++;
			    strcpy(config_file, argv[i]);
			    break;

			default:
			    fprintf (stderr, "Unknown option -%c\n", 
				argv[i][1]);
			    break;
		    }
		}
		else
		    read_file (argv[i]);

		i ++;
	    }
	}
}

/*===========================================================================*/
void init_mesh(void)
/*===========================================================================*/
{
int		i, j;

srandom (getpid());

levels = (level_type *) malloc (nof_levels * sizeof (level_type));

levels[0].data = (float *) malloc (size * size * sizeof (float));
levels[0].size = size;

for (i = 0 ; i < size ; i ++)
    for (j = 0 ; j < size; j ++)
	{
	levels[0].data[j * size + i] = 
				((float) (random() % 100000) / 100000.0) * 
				(max_elevation - min_elevation) + 
				min_elevation;
	}

if (is_zero_edge)
	{
	for (i = 0 ; i < size ; i ++)
		{
		levels[0].data[i] = 0.0;
		levels[0].data[(size - 1) * size + i] = 0.0;

		levels[0].data[i * size] = 0.0;
		levels[0].data[i * size + size - 1] = 0.0;
		}
	}
}

/*===========================================================================*/
void make_mesh_level (int level, int new_size)
/*===========================================================================*/
{
int		i, j;
int		old_i, old_j;
int		old_size;

old_size = (new_size + 1) / 2;

levels[level].data = (float *) malloc (new_size * new_size * sizeof (float));
levels[level].size = new_size;

for (i = 0 ; i < new_size ; i += 2)
    for (j = 0 ; j < new_size ; j += 2)
	{
	old_i = i / 2;
	old_j = j / 2;

	levels[level].data[j * new_size + i] = 
		levels[level - 1].data[old_j * old_size + old_i];
	}

for (i = 0 ; i < (new_size - 1) ; i += 2)
    for (j = 0 ; j < (new_size - 1) ; j += 2)
	{
	levels[level].data[(j + 1) * new_size + (i    )] = 
		fract ( levels[level].data[(j    ) * new_size + (i    )], 
			levels[level].data[(j + 2) * new_size + (i    )]);

	levels[level].data[(j + 1) * new_size + (i + 1)] = 
		fract ( levels[level].data[(j    ) * new_size + (i + 2)], 
			levels[level].data[(j + 2) * new_size + (i    )]);

	levels[level].data[(j    ) * new_size + (i + 1)] = 
		fract ( levels[level].data[(j    ) * new_size + (i    )], 
			levels[level].data[(j    ) * new_size + (i + 2)]);

	levels[level].data[(j + 2) * new_size + (i + 1)] = 
		fract ( levels[level].data[(j + 2) * new_size + (i    )], 
			levels[level].data[(j + 2) * new_size + (i + 2)]);

	levels[level].data[(j + 1) * new_size + (i + 2)] = 
		fract ( levels[level].data[(j    ) * new_size + (i + 2)], 
			levels[level].data[(j + 2) * new_size + (i + 2)]);

	}
}

/*===========================================================================*/
float fract (float v0, float v1)
/*===========================================================================*/
{
float	r;

/*
 *	random in [-1, 1]
 */
r = ((float) (random() % 10000)) / 5000.0 - 1.0;

return ((v0 + v1) * 0.5 + r * random_intensity * fabs (v1 - v0) * 0.5);
}



#define	NOF_COLORS 7



/*=========================================================================*/
void    read_file (char *filename)
/*=========================================================================*/
{
    FILE 	*fp;
    char	s[200];

    if ((fp = fopen (filename, "r")) == NULL)
    {
	perror (filename);
	return;
    }

    while (! feof (fp))
    {
	s[0] = ';';
	fgets (s, 190, fp);

	if ((s[0] != ';') && (s[0] != ' ') && (s[0] != '\n'))
	{
	    if (strncmp (s, "root_size", 9) == 0)
		size = atoi (&s[10]);
	    else
	    if (strncmp (s, "min_elevation", 13) == 0)
		min_elevation = atof (&s[14]);
	    else
	    if (strncmp (s, "max_elevation", 13) == 0)
		max_elevation = atof (&s[14]);
	    else
	    if (strncmp (s, "random_intensity", 16) == 0)
		random_intensity = atof (&s[17]);
	    else
	    if (strncmp (s, "nof_levels", 10) == 0)
		nof_levels = atoi (&s[11]);
	    else
	    if (strncmp (s, "x_span", 6) == 0)
		x_span = atof (&s[7]);
	    else
	    if (strncmp (s, "y_span", 6) == 0)
		y_span = atof (&s[7]);
	    else
	    if (strncmp (s, "x_base", 6) == 0)
		x_base = atof (&s[7]);
	    else
	    if (strncmp (s, "y_base", 6) == 0)
		y_base = atof (&s[7]);
	    else
	    if (strncmp (s, "zero_edge", 9) == 0)
		is_zero_edge = atoi (&s[10]);
	    else
	    if (strncmp (s, "flatten_zero", 12) == 0)
		is_flatten_zero = atoi (&s[13]);
	    else
	    if (strncmp (s, "tile_size", 9) == 0)
		sscanf (&s[10], "%d", &tile_size);
	    else
	    if (strncmp (s, "nof_asd_levels", 14) == 0)
		sscanf (&s[15], "%d", &nof_asd_levels);
	    else
	    if (strncmp (s, "tile_lookahead", 14) == 0)
		sscanf (&s[15], "%d%d", &tile_lookahead[0], &tile_lookahead[1]);
	    else
	    if (strncmp (s, "prune_epsilon", 13) == 0)
		sscanf (&s[13], "%f", &prune_epsilon);
	    else
	    if (strncmp (s, "enable_prune", 12) == 0)
		sscanf (&s[13], "%d", &enable_prune);
	    else
	    if (strncmp (s, "temp_base_filename", 18) == 0)
		sscanf (&s[18], "%s", temp_base_filename);
	    else
	    if (strncmp (s, "base_filename", 13) == 0)
		sscanf (&s[13], "%s", base_filename);
	    else
	    if (strncmp (s, "sphere", 6) == 0)
	    {
		is_sphere = 1;
		sscanf (&s[7], "%f%f%f%f", &sphere_radius, 
					   &sphere_center[0],
					   &sphere_center[1],
					   &sphere_center[2]);
	    }
	    else
		fprintf (stderr, "Unknown option %s\n", s);
	}
    }

    printf ("Option Summary:\n");
    printf ("root_size %d\n", size);
    printf ("min_elevation %f\n", min_elevation);
    printf ("max_elevation %f\n", max_elevation);
    printf ("random_intensity %f\n", random_intensity);
    printf ("nof_levels %d\n", nof_levels);
    printf ("x_span %f\n", x_span);
    printf ("y_span %f\n", y_span);
    printf ("zero_edge %d\n", is_zero_edge);
    printf ("flatten_zero %d\n", is_flatten_zero);
    printf ("tile_size %d\n", tile_size);
    printf ("nof_asd_levels %d\n", nof_asd_levels);
    printf ("tile_lookahead [%d, %d]\n", tile_lookahead[0], tile_lookahead[1]);
    printf ("prune = %d, epsilon = %f\n", enable_prune, prune_epsilon);
    if (is_sphere)
    {
	printf ("sphere radius = %f\n", sphere_radius);
	printf ("sphere center = (%f, %f, %f)\n", 
			sphere_center[0], sphere_center[1], sphere_center[2]);
    }
    printf ("temp_base_filename = %s\n", temp_base_filename);
    printf ("base_filename = %s\n", base_filename);

}



/*=========================================================================*/
float	array_point (void *user_data, int x, int y)
/*=========================================================================*/
{
    level_type *level;

    level = (level_type *) user_data;

    /*
     *	Wraparound the X coordinate in case we have a sphere ASD.
     */
    return (level -> data[y * level -> size + (x % level -> size)]);
}

/*=========================================================================*/
void	match_sphere_seams (level_type *level)
/*=========================================================================*/
{
    int		size;
    int		i;

    size = level -> size;

    for (i = 0 ; i < size ; i ++)
    {
	level -> data[i] = 0.0;
	level -> data[size * (size - 1) + i] = 0.0;
    }

    for (i = 0 ; i < size ; i ++)
	level -> data[size * i] = level -> data[size * i + size - 1];
}
