/*
 * Copyright 1996, Silicon Graphics, Inc.
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
 */


#include <math.h>
#include <assert.h>
#include <Performer/pr/pfLinMath.h>
#include "pyramid.h"


#define FOR(i,n) for ((i) = (0); (i) < (n); ++(i))
#define INRANGE(foo,bar,baz) ((foo(bar))&&((bar)baz))
#define EXPAND2(v) (v)[0], (v)[1]
#define EXPAND3(v) (v)[0], (v)[1], (v)[2]

static float
non_retarded_fmodf(float num, float denom)
{
    float result = fmodf(num, denom);
    return result < 0 ? result + denom : result;
}

static int
GetQuadFromSeg3d(pfVec3 quad[4], float x0, float y0, float x1, float y1)
{
    if (x0 == x1 || y0 == y1) return 0; /* failure-- zero-width or zero-height*/
    assert(x0 < x1 && y0 < y1);
    quad[0].set(x0,y0,0);
    quad[1].set(x1,y0,0);
    quad[2].set(x1,y1,0);
    quad[3].set(x0,y1,0);
    return 1; /* success */
}
static inline int
GetQuadFromSeg2d(pfVec2 quad[4], float x0, float y0, float x1, float y1)
{
    if (x0 == x1 || y0 == y1) return 0; /* failure-- zero-width or zero-height*/
    assert(x0 < x1 && y0 < y1);
    quad[0].set(x0,y0);
    quad[1].set(x1,y0);
    quad[2].set(x1,y1);
    quad[3].set(x0,y1);
    return 1; /* success */
}

static inline void
offsetAndScale(int nquads, pfVec3 quads[][4], pfVec3 offset, float scale)
{
    int i, j, k;
    FOR (i, nquads)
	FOR (j, 4)
	    FOR (k, 3)
	    {
		quads[i][j][k] = quads[i][j][k]*scale + offset[k];
		assert(INRANGE(-1e6f <, quads[i][j][k], < 1e6f));
	    }
}

/* XXX for dbx brain damage */
#define N_valid_quads _N_valid_quads
#define N_invalid_quads _N_invalid_quads

/*
 * Get the vertex cordinates and texture coordinates
 * needed to draw a picture of the physical layout of
 * texture memory occupied by a given clip level.
 */
extern "C" void
GetClippedLevelCoords(
	const pfVec2 center,	/* in texture coords */
	int virtual_size,
	int level_size,
	int clip_size,
	float border,
	int do_roam,
	int do_wrap,
	const pfVec3 level_lower_left,
	pfVec3 valid_vcoords_quads[4][4],  /* returns at most 4 valid quads */
	pfVec2 valid_tcoords_quads[4][4],  /* returns at most 4 valid quads */
	pfVec3 invalid_vcoords_quads[8][4],/* returns at most 8 invalid quads */
	int *N_valid_quads,	      /* number of valid quads returned */
	int *N_invalid_quads	      /* number of invalid quads returned */
	)
{
    if (level_size <= clip_size)
    {
	/*
	 * It's a pyramid level.
	 * This is a very simple problem--
	 * there is no border and no roaming or wrapping ever needed;
	 * return 1 valid quad and no invalid ones
	 */
	*N_valid_quads = 1;
	*N_invalid_quads = 0;

	valid_vcoords_quads[0][0].set(0,0,0);
	valid_vcoords_quads[0][1].set(level_size,0,0);
	valid_vcoords_quads[0][2].set(level_size,level_size,0);
	valid_vcoords_quads[0][3].set(0,level_size,0);

	valid_tcoords_quads[0][0].set(0.,0.);
	valid_tcoords_quads[0][1].set(1.,0.);
	valid_tcoords_quads[0][2].set(1.,1.);
	valid_tcoords_quads[0][3].set(0.,1.);

	offsetAndScale(*N_valid_quads, valid_vcoords_quads, level_lower_left, 1.0f/virtual_size);
	offsetAndScale(*N_invalid_quads, invalid_vcoords_quads, level_lower_left, 1.0f/virtual_size);

	return;
    }

    pfVec2 clip_radius2(clip_size/2, clip_size/2);
    pfVec2 border2(border, border);

    /*
     * Coordinates of the valid region w.r.t. the level in texels,
     * i.e. in the range 0..level_size
     */
    pfVec2 level_center = center * level_size;	/* center of valid window */
    level_center[0] = PF_CLAMP(level_center[0], clip_size/2, level_size-clip_size/2);
    level_center[1] = PF_CLAMP(level_center[1], clip_size/2, level_size-clip_size/2);
    pfVec2 level_clip_ll = level_center - clip_radius2;
    pfVec2 level_clip_ur = level_center + clip_radius2;
    pfVec2 level_valid_ll = level_clip_ll + border2;
    pfVec2 level_valid_ur = level_clip_ur - border2;

    if (border >= clip_size/2)
    {
	/*
	 * It's all border
	 */
	*N_valid_quads = 0;
	*N_invalid_quads = 1;
	invalid_vcoords_quads[0][0].set(0,0,0);
	invalid_vcoords_quads[0][1].set(clip_size,0,0);
	invalid_vcoords_quads[0][2].set(clip_size,clip_size,0);
	invalid_vcoords_quads[0][3].set(0,clip_size,0);
	if (do_roam)
	{
	    int j;
	    FOR (j, 4)
		invalid_vcoords_quads[0][j] +=
		    pfVec3(level_clip_ll[PF_X], level_clip_ll[PF_Y], 0);
	}
	offsetAndScale(*N_valid_quads, valid_vcoords_quads, level_lower_left, 1.0f/virtual_size);
	offsetAndScale(*N_invalid_quads, invalid_vcoords_quads, level_lower_left, 1.0f/virtual_size);
	return;
    }

    if (!do_wrap)
    {
	/*
	 * It's still pretty simple.
	 * Return one valid quad,
	 * and four invalid ones forming the border around it.
	 * XXX Could use tuning (get rid of the zero-area quads)
	 * but then again who really cares
	 */

	*N_valid_quads = 1;
	*N_invalid_quads = 4;

	valid_vcoords_quads[0][0].set(border, border, 0);
	valid_vcoords_quads[0][1].set(clip_size-border, border, 0);
	valid_vcoords_quads[0][2].set(clip_size-border, clip_size-border, 0);
	valid_vcoords_quads[0][3].set(border, clip_size-border,0);
	valid_tcoords_quads[0][0].set(level_valid_ll[0] / (float)level_size,
				      level_valid_ll[1] / (float)level_size);
	valid_tcoords_quads[0][1].set(level_valid_ur[0] / (float)level_size,
				      level_valid_ll[1] / (float)level_size);
	valid_tcoords_quads[0][2].set(level_valid_ur[0] / (float)level_size,
				      level_valid_ur[1] / (float)level_size);
	valid_tcoords_quads[0][3].set(level_valid_ll[0] / (float)level_size,
				      level_valid_ur[1] / (float)level_size);

	/* bottom */
	invalid_vcoords_quads[0][0].set(0,0,0);
	invalid_vcoords_quads[0][1].set(clip_size,0,0);
	invalid_vcoords_quads[0][2].set(clip_size-border, border, 0);
	invalid_vcoords_quads[0][3].set(border, border, 0);

	/* left */
	invalid_vcoords_quads[1][0].set(clip_size-border, border, 0);
	invalid_vcoords_quads[1][1].set(clip_size, 0, 0);
	invalid_vcoords_quads[1][2].set(clip_size, clip_size, 0);
	invalid_vcoords_quads[1][3].set(clip_size-border, clip_size-border, 0);

	/* top */
	invalid_vcoords_quads[2][0].set(border, clip_size-border, 0);
	invalid_vcoords_quads[2][1].set(clip_size-border, clip_size-border, 0);
	invalid_vcoords_quads[2][2].set(clip_size, clip_size, 0);
	invalid_vcoords_quads[2][3].set(0, clip_size, 0);

	/* right */
	invalid_vcoords_quads[3][0].set(0,0, 0);
	invalid_vcoords_quads[3][1].set(border, border, 0);
	invalid_vcoords_quads[3][2].set(border, clip_size-border, 0);
	invalid_vcoords_quads[3][3].set(0, clip_size, 0);

	/*
	 * If do_roam, return coords in space of the level;
	 * otherwise return coords in space of the little image we
	 * are drawing.  Units are texels of this level in any case.
	 */
	if (do_roam)
	{
	    int i, j;
	    FOR (i, *N_valid_quads)
		FOR (j, 4)
		    valid_vcoords_quads[i][j] +=
			pfVec3(level_clip_ll[PF_X], level_clip_ll[PF_Y], 0);
	    FOR (i, *N_invalid_quads)
		FOR (j, 4)
		    invalid_vcoords_quads[i][j] +=
			pfVec3(level_clip_ll[PF_X], level_clip_ll[PF_Y], 0);
	}

	offsetAndScale(*N_valid_quads, valid_vcoords_quads, level_lower_left, 1.0f/virtual_size);
	offsetAndScale(*N_invalid_quads, invalid_vcoords_quads, level_lower_left, 1.0f/virtual_size);
	return;
    }

    /*
     * Coordinates of the valid region w.r.t. this texture piece in texels,
     * i.e. wrapped to the range 0..clip_size
     */
    pfVec2 clip_valid_ll(non_retarded_fmodf(level_valid_ll[PF_X], clip_size),
			 non_retarded_fmodf(level_valid_ll[PF_Y], clip_size));
    pfVec2 clip_valid_ur(non_retarded_fmodf(level_valid_ur[PF_X], clip_size),
			 non_retarded_fmodf(level_valid_ur[PF_Y], clip_size));
    if (border == 0)
    {
	/* don't risk them coming out a little different... it
	   was too much headache to do approximate equality testing. */
	clip_valid_ur = clip_valid_ll;
    }

    if (clip_valid_ur[PF_X] == 0)
	clip_valid_ur[PF_X] = clip_size;
    if (clip_valid_ur[PF_Y] == 0)
	clip_valid_ur[PF_Y] = clip_size;

    pfVec2 level_wrap_pos(0,0);
    /* XXX slow uningenious way... */
    while (level_wrap_pos[PF_X] < level_clip_ll[PF_X])
	level_wrap_pos[PF_X] += clip_size;
    while (level_wrap_pos[PF_Y] < level_clip_ll[PF_Y])
	level_wrap_pos[PF_Y] += clip_size;

    /*
     * The problem is separable.  First solve the one-dimensional
     * problem in x and in y...
     * (coordinates are all w.r.t. this texture piece in texels)
     */
    pfVec2 valid_vcoords_segs[2][2];	/* 2 segments, begin&end */
    pfVec2 valid_tcoords_segs[2][2];	/* tex coords of those segments */
    pfVec2 invalid_vcoords_segs[2][2];	/* 2 segments, begin&end */

    int n_valid_segs[2]; /* number of valid segs in each coord direction */
    int n_invalid_segs[2]; /* number of invalid segs in each coord direction */

    for (int ax = PF_X; ax <= PF_Y; ++ax)
    {
	n_valid_segs[ax] = n_invalid_segs[ax] = 0;
/* XXX this problem will probably go away when we round borders denominators to
   powers of two or something */
#define LT(a,b,eps) ((b)-(a)>(eps))
	if (LT(clip_valid_ll[ax], clip_valid_ur[ax], level_size*1e-4))
	{
	    /*
	     * The wrap position in coordinate ax is in the invalid part,
	     * so the valid part is contiguous.
	     */
	    valid_vcoords_segs[0][0][ax] = clip_valid_ll[ax];	/* begin seg */
	    valid_vcoords_segs[0][1][ax] = clip_valid_ur[ax];	/* end seg */
	    valid_tcoords_segs[0][0][ax] = level_valid_ll[ax]/(float)level_size;
	    valid_tcoords_segs[0][1][ax] = level_valid_ur[ax]/(float)level_size;
	    n_valid_segs[ax] = 1;

	    invalid_vcoords_segs[0][0][ax] = clip_valid_ur[ax]; /* begin seg 0*/
	    invalid_vcoords_segs[0][1][ax] = clip_size;		/* end seg 0 */
	    invalid_vcoords_segs[1][0][ax] = 0;			/* begin seg 1*/
	    invalid_vcoords_segs[1][1][ax] = clip_valid_ll[ax]; /* end seg 1 */
	    n_invalid_segs[ax] = 2;			
	}
	else
	{
	    /*
	     * The wrap position in coordinate ax is in the valid part,
	     * so the invalid part is contiguous
	     */
	    valid_vcoords_segs[0][0][ax] = clip_valid_ll[ax];	/* begin seg 0*/
	    valid_vcoords_segs[0][1][ax] = clip_size;		/* end seg 0 */
	    valid_vcoords_segs[1][0][ax] = 0;			/* begin seg 1*/
	    valid_vcoords_segs[1][1][ax] = clip_valid_ur[ax];	/* end seg 1 */

	    valid_tcoords_segs[0][0][ax] = level_valid_ll[ax]/(float)level_size;
	    valid_tcoords_segs[0][1][ax] = level_wrap_pos[ax]/(float)level_size;
	    valid_tcoords_segs[1][0][ax] = level_wrap_pos[ax]/(float)level_size;
	    valid_tcoords_segs[1][1][ax] = level_valid_ur[ax]/(float)level_size;

	    n_valid_segs[ax] = 2;

	    invalid_vcoords_segs[0][0][ax] = clip_valid_ur[ax]; /* begin seg */
	    invalid_vcoords_segs[0][1][ax] = clip_valid_ll[ax]; /* end seg */
	    n_invalid_segs[ax] = 1;
	}
    }

    /*
     * Now cross each segment in y with each segment in x
     * to get a whole mess of quads.
     * A quad is valid iff both of its factor segments is valid.
     */
    int n_valid_quads = 0;
    int n_invalid_quads = 0;
    int i, j;
    /* valid x and y -> valid quads */
    FOR (i, n_valid_segs[PF_X])
    FOR (j, n_valid_segs[PF_Y])
    {
	assert(n_valid_quads < 4);
	int n0 = 
	 GetQuadFromSeg3d(valid_vcoords_quads[n_valid_quads],
	    valid_vcoords_segs[i][0][PF_X], valid_vcoords_segs[j][0][PF_Y],
	    valid_vcoords_segs[i][1][PF_X], valid_vcoords_segs[j][1][PF_Y]);
	int n1 =
	 GetQuadFromSeg2d(valid_tcoords_quads[n_valid_quads],
	    valid_tcoords_segs[i][0][PF_X], valid_tcoords_segs[j][0][PF_Y],
	    valid_tcoords_segs[i][1][PF_X], valid_tcoords_segs[j][1][PF_Y]);
	assert(n0 == n1);
	n_valid_quads += n0;
    }
    /* valid x, invalid y -> invalid quads */
    FOR (i, n_valid_segs[PF_X])
    FOR (j, n_invalid_segs[PF_Y])
    {
	assert(n_invalid_quads < 8);
	n_invalid_quads +=
	  GetQuadFromSeg3d(invalid_vcoords_quads[n_invalid_quads], valid_vcoords_segs[i][0][PF_X], invalid_vcoords_segs[j][0][PF_Y],
	    valid_vcoords_segs[i][1][PF_X], invalid_vcoords_segs[j][1][PF_Y]);
    }
    /* invalid x, valid y -> invalid quads */
    FOR (i, n_invalid_segs[PF_X])
    FOR (j, n_valid_segs[PF_Y])
    {
	assert(n_invalid_quads < 8);
	n_invalid_quads +=
	  GetQuadFromSeg3d(invalid_vcoords_quads[n_invalid_quads],
	    invalid_vcoords_segs[i][0][PF_X], valid_vcoords_segs[j][0][PF_Y],
	    invalid_vcoords_segs[i][1][PF_X], valid_vcoords_segs[j][1][PF_Y]);
    }
    /* invalid x and y -> invalid quads */
    FOR (i, n_invalid_segs[PF_X])
    FOR (j, n_invalid_segs[PF_Y])
    {
	assert(n_invalid_quads < 8);
	n_invalid_quads +=
	  GetQuadFromSeg3d(invalid_vcoords_quads[n_invalid_quads],
	    invalid_vcoords_segs[i][0][PF_X], invalid_vcoords_segs[j][0][PF_Y],
	    invalid_vcoords_segs[i][1][PF_X], invalid_vcoords_segs[j][1][PF_Y]);
    }

    if (border == 0)
    {
	assert(n_valid_quads == 4
	    || n_valid_quads == 2
	    || n_valid_quads == 1);
	if (n_invalid_quads != 0)
	    fprintf(stderr, "UH OH... n_invalid_quads = %d\n", n_invalid_quads);
	assert(n_invalid_quads == 0);
    }
    else if (clip_size-2*border == 0) /* no valid part */
    {
	assert(n_valid_quads == 0);
	assert(n_invalid_quads == 1
	    || n_invalid_quads == 4);
    }
    else
    {
	assert(n_valid_quads == 4   /* wrap inside valid win in both dirs*/
	    || n_valid_quads == 2   /* wrap inside valid win in one dir */
	    || n_valid_quads == 1); /* wrap not inside valid win in both */

	assert(n_invalid_quads == 8
	    || n_invalid_quads == 7
	    || n_invalid_quads == 5
	    || n_invalid_quads == 4
	    || n_invalid_quads == 3); /* wrap right at the corner of valid */
    }

    *N_valid_quads = n_valid_quads;
    *N_invalid_quads = n_invalid_quads;

    /*
     * If do_roam, return coords in space of the level;
     * otherwise return coords in space of the clip texture memory.
     * Units are texels of this level in any case.
     */
    if (do_roam)
    {
	FOR (i, n_valid_quads)
	    FOR (j, 4)
		valid_vcoords_quads[i][j] +=
		    pfVec3(level_clip_ll[PF_X], level_clip_ll[PF_Y], 0);
	FOR (i, n_invalid_quads)
	    FOR (j, 4)
		invalid_vcoords_quads[i][j] +=
		    pfVec3(level_clip_ll[PF_X], level_clip_ll[PF_Y], 0);
    }
    offsetAndScale(*N_valid_quads, valid_vcoords_quads, level_lower_left, 1.0f/virtual_size);
    offsetAndScale(*N_invalid_quads, invalid_vcoords_quads, level_lower_left, 1.0f/virtual_size);
}

#ifdef MAIN /* little test program */

#include <stdio.h>
#include <stdlib.h>

main(int argc, char **argv)
{
    if (argc != 9)
    {
	fprintf(stderr, "Usage: %s centerx centery level_size clip_size border do_roam do_wrap z\n",
		argv[0]);
	exit(1);
    }

    /* Inputs... */
    pfVec2 center(atof(argv[1]), atof(argv[2]));
    int level_size = atoi(argv[3]);
    int clip_size = atoi(argv[4]);
    int border = atoi(argv[5]);
    int do_roam = atoi(argv[6]);
    int do_wrap = atoi(argv[7]);
    float z = atof(argv[8]);

#define print(x) printf("%s = %d\n", #x, x)
#define prfloat(x) printf("%s = %f\n", #x, x)

    printf("===============================");
    prfloat(center[0]);
    prfloat(center[1]);
    print(level_size);
    print(clip_size);
    print(border);
    print(do_roam);
    print(do_wrap);
    prfloat(z);

    /* Outputs... */
    pfVec3 valid_vcoords[4][4];
    pfVec2 valid_tcoords[4][4];
    pfVec3 invalid_vcoords[8][4];
    int n_valid_quads;
    int n_invalid_quads;

    GetClippedLevelCoords(
        center, level_size, clip_size, border, do_roam, do_wrap,pfVec3(0,0,z),
	valid_vcoords, valid_tcoords, invalid_vcoords,
	&n_valid_quads, &n_invalid_quads);

    /* Print out results... */

    int i;
    printf("    %d valid quads:\n", n_valid_quads);
    FOR (i, n_valid_quads)
    {
	printf("\t%2d vc: %g,%g,%g .. %g,%g,%g\n",
		i,
		EXPAND3(valid_vcoords[i][0]),
		EXPAND3(valid_vcoords[i][2]));
	printf("\t%2d tc: %g,%g .. %g,%g\n",
		i,
		EXPAND2(valid_tcoords[i][0]),
		EXPAND2(valid_tcoords[i][2]));
	assert(valid_vcoords[i][1] == pfVec3(valid_vcoords[i][2][PF_X],
					     valid_vcoords[i][0][PF_Y], z));
	assert(valid_vcoords[i][3] == pfVec3(valid_vcoords[i][0][PF_X],
					     valid_vcoords[i][2][PF_Y], z));
	assert(valid_tcoords[i][1] == pfVec2(valid_tcoords[i][2][PF_X],
					     valid_tcoords[i][0][PF_Y]));
	assert(valid_tcoords[i][3] == pfVec2(valid_tcoords[i][0][PF_X],
					     valid_tcoords[i][2][PF_Y]));
    }
    printf("    %d invalid quads:\n", n_invalid_quads);
    FOR (i, n_invalid_quads)
    {
	printf("\t%2d vc: %g,%g,%g %g,%g,%g",
		i,
		EXPAND3(invalid_vcoords[i][0]),
		EXPAND3(invalid_vcoords[i][2]));
	/*
	XXX should have some kind of assertion here,
	but this one fails because stuff is not square
	assert(invalid_vcoords[i][1] == pfVec3(invalid_vcoords[i][2][PF_X],
					       invalid_vcoords[i][0][PF_Y], z));
	assert(invalid_vcoords[i][3] == pfVec3(invalid_vcoords[i][0][PF_X],
					       invalid_vcoords[i][2][PF_Y], z));
	*/
	if (!(invalid_vcoords[i][1] == pfVec3(invalid_vcoords[i][2][PF_X], invalid_vcoords[i][0][PF_Y], z) && invalid_vcoords[i][3] == pfVec3(invalid_vcoords[i][0][PF_X], invalid_vcoords[i][2][PF_Y], z)))
	    printf(" (not rectangular)");
	printf("\n");
    }

    return 0;
}
#endif
