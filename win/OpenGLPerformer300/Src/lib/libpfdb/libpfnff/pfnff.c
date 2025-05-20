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
 * pfnff.c: $Revision: 1.38 $ $Date: 2002/11/17 01:40:32 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFNFF_DLLEXPORT __declspec(dllexport)
#else
#define PFNFF_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFNFF_DLLEXPORT
#endif

/* length of longest input line in ".nff" file */
#define	BUFFER_SIZE	256

/* static global data (shared by loader and output routines) */
static pfdGeom	*prim		= NULL;
static int	 primSize	= 0;
static int	 numTris	= 0;

/* default number of primitive patch subdivisions */
#define	OUTPUT_RESOLUTION	4
static int	 gU_resolution	= OUTPUT_RESOLUTION;
static int	 gV_resolution	= OUTPUT_RESOLUTION;

/***
 ***	S P D -- low-level math library
 ***/

#define	EPSILON 		1.0e-8
#define	EPSILON2		1.0e-6
#define	X			PF_X
#define	Y			PF_Y
#define	Z			PF_Z
#define	W			PF_W
#define	PI			PF_PI
#define	COORD3			pfVec3
#define	COORD4			pfVec4
#define	MATRIX			pfMatrix
#ifdef WIN32
#define fabsf			fabs
#endif
#define	PF_ABSOLUTE		fabsf
#define	CROSS			pfCrossVec3
#define	SUB3_COORD3		pfSubVec3
#define	ADD2_COORD3(_a,_b)	pfAddVec3(_a,_a,_b)
#define	ADD3_COORD3		pfAddVec3
#define	SET_COORD3		pfSetVec3
#define	COPY_COORD3		pfCopyVec3
#define	COPY_COORD4		pfCopyVec4
#define	DOT_PRODUCT		pfDotVec3
#define	SQR(_x)			((_x)*(_x))

#define X_AXIS  0
#define Y_AXIS  1
#define Z_AXIS  2

/* Basic math functions */
static void lib_zero_matrix(MATRIX mx);
static void lib_create_identity_matrix(MATRIX mx);
static void lib_copy_matrix(MATRIX mres, MATRIX mx);
static void lib_create_translate_matrix(MATRIX mx, COORD3 vec);
static void lib_create_scale_matrix(MATRIX mx, COORD3 vec);
static void lib_create_rotate_matrix(MATRIX mx, int axis, double angle);
static void lib_create_axis_rotate_matrix(MATRIX mx, COORD3 rvec, double angle);
static void lib_create_canonical_matrix(MATRIX trans, MATRIX itrans, COORD3 origin, COORD3 up);
static void lib_transform_point(COORD3 vres, COORD3 vec, MATRIX mx);
static void lib_transform_vector(COORD3 vres, COORD3 vec, MATRIX mx);
static void lib_transpose_matrix(MATRIX mxres, MATRIX mx);
static void lib_matrix_multiply(MATRIX mxres, MATRIX mx1, MATRIX mx2);
static void lib_rotate_cube_face(COORD3 vec, int major_axis, int mod_face);
static double lib_normalize_vector(COORD3 cvec);

/*
 * Normalize the vector (X,Y,Z) so that X*X + Y*Y + Z*Z = 1.
 *
 * The normalization divisor is returned.  If the divisor is zero, no
 * normalization occurs.
 *
 */
static double
lib_normalize_vector(cvec)
    COORD3 cvec;
{
    double dot;
    double divisor;

    dot = DOT_PRODUCT(cvec, cvec);
    divisor = sqrt( (double)dot );
    if (divisor > 0.0) {
	divisor = 1.0/divisor;
	cvec[X] *= divisor;
	cvec[Y] *= divisor;
	cvec[Z] *= divisor;
    }
    return divisor;
}


/*
 * Set all matrix elements to zero.
 */
static void
lib_zero_matrix(mx)
    MATRIX mx;
{
    int i, j;

    for (i=0;i<4;++i)
	for (j=0;j<4;++j)
	    mx[i][j] = 0.0;
}

/*
 * Create identity matrix.
 */
static void
lib_create_identity_matrix(mx)
    MATRIX mx;
{
    int i;

    lib_zero_matrix(mx);
    for (i=0;i<4;++i)
	mx[i][i] = 1.0;
}

/*
 * Create a rotation matrix along the given axis by the given angle in radians.
 */
static void
lib_create_rotate_matrix(mx, axis, angle)
    MATRIX mx;
    int axis;
    double angle;
{
    double cosine, sine;

    lib_zero_matrix(mx);
    cosine = cos((double)angle);
    sine = sin((double)angle);
    switch (axis) {
	case X_AXIS:
	    mx[0][0] = 1.0;
	    mx[1][1] = cosine;
	    mx[2][2] = cosine;
	    mx[1][2] = sine;
	    mx[2][1] = -sine;
	    break ;
	case Y_AXIS:
	    mx[1][1] = 1.0;
	    mx[0][0] = cosine;
	    mx[2][2] = cosine;
	    mx[2][0] = sine;
	    mx[0][2] = -sine;
	    break;
	case Z_AXIS:
	    mx[2][2] = 1.0;
	    mx[0][0] = cosine;
	    mx[1][1] = cosine;
	    mx[0][1] = sine;
	    mx[1][0] = -sine;
	    break;
    }
    mx[3][3] = 1.0;
}

/*
 * Create a rotation matrix along the given axis by the given angle in radians.
 * The axis is a set of direction cosines.
 */
static void
lib_create_axis_rotate_matrix(mx, axis, angle)
    MATRIX mx;
    COORD3 axis;
    double angle;
{
    double  cosine, one_minus_cosine, sine;

    cosine = cos((double)angle);
    sine = sin((double)angle);
    one_minus_cosine = 1.0 - cosine;

    mx[0][0] = SQR(axis[X]) + (1.0 - SQR(axis[X])) * cosine;
    mx[0][1] = axis[X] * axis[Y] * one_minus_cosine + axis[Z] * sine;
    mx[0][2] = axis[X] * axis[Z] * one_minus_cosine - axis[Y] * sine;
    mx[0][3] = 0.0;

    mx[1][0] = axis[X] * axis[Y] * one_minus_cosine - axis[Z] * sine;
    mx[1][1] = SQR(axis[Y]) + (1.0 - SQR(axis[Y])) * cosine;
    mx[1][2] = axis[Y] * axis[Z] * one_minus_cosine + axis[X] * sine;
    mx[1][3] = 0.0;

    mx[2][0] = axis[X] * axis[Z] * one_minus_cosine + axis[Y] * sine;
    mx[2][1] = axis[Y] * axis[Z] * one_minus_cosine - axis[X] * sine;
    mx[2][2] = SQR(axis[Z]) + (1.0 - SQR(axis[Z])) * cosine;
    mx[2][3] = 0.0;

    mx[3][0] = 0.0;
    mx[3][1] = 0.0;
    mx[3][2] = 0.0;
    mx[3][3] = 1.0;
}

/* Create translation matrix */
static void
lib_create_translate_matrix(mx, vec)
    MATRIX mx;
    COORD3 vec;
{
    lib_create_identity_matrix(mx);
    mx[3][0] = vec[X];
    mx[3][1] = vec[Y];
    mx[3][2] = vec[Z];
    mx[3][3] = 1.0;
}

/* Given a point and a direction, find the transform that brings a point
   in a canonical coordinate system into a coordinate system defined by
   that position and direction.    Both the forward and inverse transform
   matrices are calculated. */
static void
lib_create_canonical_matrix(trans, itrans, origin, up)
    MATRIX trans, itrans;
    COORD3 origin;
    COORD3 up;
{
    MATRIX trans1, trans2;
    COORD3 tmpv;
    double ang;

    /* Translate "origin" to <0, 0, 0> */
    SET_COORD3(tmpv, -origin[X], -origin[Y], -origin[Z]);
    lib_create_translate_matrix(trans1, tmpv);

    /* Determine the axis to rotate about */
    if (fabs(up[Z]) == 1.0)
	SET_COORD3(tmpv, 1.0, 0.0, 0.0);
    else
	SET_COORD3(tmpv, -up[Y], up[X], 0.0);
    lib_normalize_vector(tmpv);
    ang = acos(up[Z]);

    /* Calculate the forward matrix */
    lib_create_axis_rotate_matrix(trans2, tmpv, -ang);
    lib_matrix_multiply(trans, trans1, trans2);

    /* Calculate the inverse transform */
    lib_create_axis_rotate_matrix(trans1, tmpv, ang);
    lib_create_translate_matrix(trans2, origin);
    lib_matrix_multiply(itrans, trans1, trans2);
}

/*
 * Multiply a vector by a matrix.
 */
static void
lib_transform_vector(vres, vec, mx)
    COORD3 vres, vec;
    MATRIX mx;
{
    COORD3 vtemp;
    vtemp[X] = vec[X]*mx[0][0] + vec[Y]*mx[1][0] + vec[Z]*mx[2][0];
    vtemp[Y] = vec[X]*mx[0][1] + vec[Y]*mx[1][1] + vec[Z]*mx[2][1];
    vtemp[Z] = vec[X]*mx[0][2] + vec[Y]*mx[1][2] + vec[Z]*mx[2][2];
    COPY_COORD3(vres, vtemp);
}

/*
 * Multiply a point by a matrix.
 */
static void
lib_transform_point(vres, vec, mx)
    COORD3 vres, vec;
    MATRIX mx;
{
    COORD3 vtemp;
    vtemp[X] = vec[X]*mx[0][0] + vec[Y]*mx[1][0] + vec[Z]*mx[2][0] + mx[3][0];
    vtemp[Y] = vec[X]*mx[0][1] + vec[Y]*mx[1][1] + vec[Z]*mx[2][1] + mx[3][1];
    vtemp[Z] = vec[X]*mx[0][2] + vec[Y]*mx[1][2] + vec[Z]*mx[2][2] + mx[3][2];
    COPY_COORD3(vres, vtemp);
}

/*
 * Multiply two 4x4 matrices.
 */
static void
lib_matrix_multiply(mxres, mx1, mx2)
    MATRIX mxres, mx1, mx2;
{
    int i, j;

    for (i=0;i<4;i++)
	for (j=0;j<4;j++)
	    mxres[i][j] = 
		mx1[i][0]*mx2[0][j] + 
		mx1[i][1]*mx2[1][j] +
		mx1[i][2]*mx2[2][j] + 
		mx1[i][3]*mx2[3][j];
}

/*
 * Rotate a vector pointing towards the major-axis faces (i.e. the major-axis
 * component of the vector is defined as the largest value) 90 degrees to
 * another cube face.  Mod_face is a face number.
 *
 * If the routine is called six times, with mod_face=0..5, the vector will be
 * rotated to each face of a cube.  Rotations are:
 *   mod_face = 0 mod 3, +Z axis rotate
 *   mod_face = 1 mod 3, +X axis rotate
 *   mod_face = 2 mod 3, -Y axis rotate
 */
static void
lib_rotate_cube_face(vec, major_axis, mod_face)
    COORD3 vec;
    int major_axis, mod_face;
{
    double swap;

    mod_face = (mod_face+major_axis) % 3 ;
    if (mod_face == 0) {
	swap	 = vec[X];
	vec[X] = -vec[Y];
	vec[Y] = swap;
    } else if (mod_face == 1) {
	swap	 = vec[Y];
	vec[Y] = -vec[Z];
	vec[Z] = swap;
    } else {
	swap	 = vec[X];
	vec[X] = -vec[Z];
	vec[Z] = swap;
    }
}

/***
 ***	S P D -- geometric object tessellation functions
 ***/

/*-----------------------------------------------------------------*/
/*
 * Output polygon.  A polygon is defined by a set of vertices.  With these
 * databases, a polygon is defined to have all points coplanar.  A polygon has
 * only one side, with the order of the vertices being counterclockwise as you
 * face the polygon (right-handed coordinate system).
 *
 * The output format is always:
 *     "p" total_vertices
 *     vert1.x vert1.y vert1.z
 *     [etc. for total_vertices polygons]
 *
 */
static void
lib_output_polygon (int tot_vert, COORD3 vert[])
{
    int num_vert, i, j;
    COORD3 x;
    int		 t; 

    /* First lets do a couple of checks to see if this is a valid polygon */
    for (i=0;i<tot_vert;) {
	/* If there are two adjacent coordinates that degenerate then
	   collapse them down to one */
	SUB3_COORD3(x, vert[i], vert[(i+1)%tot_vert]);
	if (lib_normalize_vector(x) < EPSILON2) {
	    for (j=i;j<tot_vert-1;j++)
		memcpy(&vert[j], &vert[j+1], sizeof(COORD3));
	    tot_vert--;
	}
	else {
	    i++;
	}
    }

    /* No such thing as a poly that only has two sides */
    if (tot_vert < 3)
	return;

    /* enlarge primitive buffer if required */
    if (tot_vert > primSize)
	pfdResizeGeom(prim, primSize = tot_vert);

    /* Build IRIS Performer geobuilder primitive */
    for (num_vert=0;num_vert<tot_vert;++num_vert)
	pfCopyVec3(prim->coords[num_vert], vert[num_vert]);
    prim->numVerts = tot_vert;
    prim->primtype = PFGS_POLYS;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	prim->tbind[t] = PFGS_OFF;
    prim->nbind = PFGS_OFF;
    prim->cbind = PFGS_PER_PRIM;
    pfdAddBldrGeom(prim, 1);
    numTris += prim->numVerts - 2;
}

/*-----------------------------------------------------------------*/
/*
 * Output polygonal patch.  A patch is defined by a set of vertices and their
 * normals.  With these databases, a patch is defined to have all points
 * coplanar.  A patch has only one side, with the order of the vertices being
 * counterclockwise as you face the patch (right-handed coordinate system).
 *
 * The output format is always:
 *   "pp" total_vertices
 *   vert1.x vert1.y vert1.z norm1.x norm1.y norm1.z
 *   [etc. for total_vertices polygonal patches]
 *
 */
static void
lib_output_polypatch(int tot_vert, COORD3 vert[], COORD3 norm[])
{
    int num_vert, i, j;
    COORD3 x;
    int		t;

    /* First lets do a couple of checks to see if this is a valid polygon */
    for (i=0;i<tot_vert;) {
	/* If there are two adjacent coordinates that degenerate then
	   collapse them down to one */
	SUB3_COORD3(x, vert[i], vert[(i+1)%tot_vert]);
	if (lib_normalize_vector(x) < EPSILON2) {
	    for (j=i;j<tot_vert-1;j++) {
		memcpy(&vert[j], &vert[j+1], sizeof(COORD3));
		memcpy(&norm[j], &norm[j+1], sizeof(COORD3));
	    }
	    tot_vert--;
	}
	else {
	    i++;
	}
    }

    /* No such thing as a poly that only has two sides */
    if (tot_vert < 3)
	return;

    /* enlarge primitive buffer if required */
    if (tot_vert > primSize)
	pfdResizeGeom(prim, primSize = tot_vert);

    /* Build IRIS Performer geobuilder primitive */
    for (num_vert=0;num_vert<tot_vert;++num_vert)
    {
	pfCopyVec3(prim->coords[num_vert], vert[num_vert]);
	pfCopyVec3(prim->norms[num_vert], norm[num_vert]);
    }
    prim->numVerts = tot_vert;
    prim->primtype = PFGS_POLYS;
    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	prim->tbind[t] = PFGS_OFF;
    prim->nbind = PFGS_PER_VERTEX;
    prim->cbind = PFGS_PER_PRIM;
    pfdAddBldrGeom(prim, 1);
    numTris += prim->numVerts - 2;
}

/*-----------------------------------------------------------------*/
/* Generate a box as a set of 4-sided polygons */
static void 
lib_output_polygon_box(COORD3 p1, COORD3 p2)
{
    COORD3 box_verts[4];

    /* Sides */
    SET_COORD3(box_verts[0], p1[X], p1[Y], p1[Z]);
    SET_COORD3(box_verts[1], p1[X], p1[Y], p2[Z]);
    SET_COORD3(box_verts[2], p1[X], p2[Y], p2[Z]);
    SET_COORD3(box_verts[3], p1[X], p2[Y], p1[Z]);
    lib_output_polygon(4, box_verts);
    SET_COORD3(box_verts[0], p2[X], p1[Y], p2[Z]);
    SET_COORD3(box_verts[1], p2[X], p1[Y], p1[Z]);
    SET_COORD3(box_verts[2], p2[X], p2[Y], p1[Z]);
    SET_COORD3(box_verts[3], p2[X], p2[Y], p2[Z]);
    lib_output_polygon(4, box_verts);

    /* Front/Back */
    SET_COORD3(box_verts[0], p1[X], p1[Y], p1[Z]);
    SET_COORD3(box_verts[1], p1[X], p2[Y], p1[Z]);
    SET_COORD3(box_verts[2], p2[X], p2[Y], p1[Z]);
    SET_COORD3(box_verts[3], p2[X], p1[Y], p1[Z]);
    lib_output_polygon(4, box_verts);
    SET_COORD3(box_verts[0], p2[X], p1[Y], p2[Z]);
    SET_COORD3(box_verts[1], p2[X], p2[Y], p2[Z]);
    SET_COORD3(box_verts[2], p1[X], p2[Y], p2[Z]);
    SET_COORD3(box_verts[3], p1[X], p1[Y], p2[Z]);
    lib_output_polygon(4, box_verts);

    /* Top/Bottom */
    SET_COORD3(box_verts[0], p1[X], p1[Y], p1[Z]);
    SET_COORD3(box_verts[1], p2[X], p1[Y], p1[Z]);
    SET_COORD3(box_verts[2], p2[X], p1[Y], p2[Z]);
    SET_COORD3(box_verts[3], p1[X], p1[Y], p2[Z]);
    lib_output_polygon(4, box_verts);
    SET_COORD3(box_verts[0], p2[X], p2[Y], p1[Z]);
    SET_COORD3(box_verts[1], p1[X], p2[Y], p1[Z]);
    SET_COORD3(box_verts[2], p1[X], p2[Y], p2[Z]);
    SET_COORD3(box_verts[3], p2[X], p2[Y], p2[Z]);
    lib_output_polygon(4, box_verts);
}

/*-----------------------------------------------------------------*/
static void
lib_output_polygon_cylcone(COORD4 base_pt, COORD4 apex_pt)
{
    int    i, u_res, v_res;
    double angle, delta_angle, divisor;
    COORD3 axis, dir, norm_axis, start_norm, apex, base;
    COORD3 norm[4], vert[4];
    MATRIX mx;

    SUB3_COORD3(axis, apex_pt, base_pt);
    COPY_COORD3(norm_axis, axis);
    lib_normalize_vector(norm_axis);

    SET_COORD3(dir, 0.0, 0.0, 1.0);
    CROSS(start_norm, axis, dir);
    divisor = lib_normalize_vector(start_norm);
    if (PF_ABSOLUTE(divisor) < EPSILON2) {
	SET_COORD3(dir, 1.0, 0.0, 0.0);
	CROSS(start_norm, axis, dir);
	lib_normalize_vector(start_norm);
    }

    apex[X] = start_norm[X]*apex_pt[W];
    apex[Y] = start_norm[Y]*apex_pt[W];
    apex[Z] = start_norm[Z]*apex_pt[W];

    base[X] = start_norm[X]*base_pt[W];
    base[Y] = start_norm[Y]*base_pt[W];
    base[Z] = start_norm[Z]*base_pt[W];

    /* compute proper "tilted" surface normal for cylinder/cone */
    {
    COORD3 ap, bp, up, right;

    ADD3_COORD3(ap, apex, apex_pt);
    ADD3_COORD3(bp, base, base_pt);
    SUB3_COORD3(up, ap, bp);
    lib_normalize_vector(up);
    CROSS(right, up, start_norm);
    lib_normalize_vector(right);
    CROSS(start_norm, right, up);
    lib_normalize_vector(start_norm);
    }

    u_res = 4*gU_resolution;
    v_res = 1;
    delta_angle = 2.0*PI/(double)u_res;
    for (i=1,angle=delta_angle; i<=u_res; ++i,angle+=delta_angle) {
	lib_create_axis_rotate_matrix(mx, norm_axis, angle);
	lib_transform_point(vert[0], apex, mx);
	ADD3_COORD3(vert[0], vert[0], apex_pt);
	lib_transform_point(vert[1], base, mx);
	ADD3_COORD3(vert[1], vert[1], base_pt);
	lib_transform_vector(norm[0], start_norm, mx);
	COPY_COORD3(norm[1], norm[0]);

	lib_create_axis_rotate_matrix(mx, norm_axis, angle + delta_angle);
	lib_transform_point(vert[2], base, mx);
	ADD3_COORD3(vert[2], vert[2], base_pt);
	lib_transform_point(vert[3], apex, mx);
	ADD3_COORD3(vert[3], vert[3], apex_pt);
	lib_transform_vector(norm[2], start_norm, mx);
	COPY_COORD3(norm[3], norm[2]);

	lib_output_polypatch(4, vert, norm);
    }
}

/*-----------------------------------------------------------------*/
static void
disc_evaluator(MATRIX trans, double theta, double v, double r, COORD3 vert)
{
    COORD3 tvert;

    /* Compute the position of the point */
    SET_COORD3(tvert, (r + v)*cos(theta), (r + v)*sin(theta), 0.0);
    lib_transform_point(vert, tvert, trans);
}

/*-----------------------------------------------------------------*/
static void
lib_output_polygon_disc(COORD3 center, COORD3 normal, double ir, double or)
{
    int i, j, u_res, v_res;
    double u, v, delta_u, delta_v;
    MATRIX mx, imx;
    COORD3 norm, vert[4];

    COPY_COORD3(norm, normal);
    if (lib_normalize_vector(norm) < EPSILON2) 
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_nff: bad disc normal"); 
	return;
    }
    lib_create_canonical_matrix(mx, imx, center, norm);

    u_res = 4*gU_resolution;
    v_res = 1;
    delta_u = 2.0*PI/(double)u_res;
    delta_v = or-ir;

    /* Dump out polygons */
    for (i=0,u=0.0; i<u_res; i++,u+=delta_u) {
	disc_evaluator(imx, u,         0.0,     ir, vert[0]);
	disc_evaluator(imx, u,         delta_v, ir, vert[1]);
	disc_evaluator(imx, u+delta_u, delta_v, ir, vert[2]);
	disc_evaluator(imx, u+delta_u, 0.0,     ir, vert[3]);
	lib_output_polygon(4, vert);
    }
}

/*-----------------------------------------------------------------*/
static void
lib_output_polygon_sphere(COORD4 center_pt)
{
    double  angle;
    COORD3  edge_norm[3], edge_pt[3];
    long    num_face, num_edge, num_tri, num_vert;
    COORD3  *x_axis, *y_axis, **pt;
    COORD3  mid_axis;
    MATRIX  rot_mx;
    long    u_pol, v_pol;

    /* Allocate storage for the polygon vertices */
    x_axis = (COORD3 *)malloc((gU_resolution+1) * sizeof(COORD3));
    y_axis = (COORD3 *)malloc((gV_resolution+1) * sizeof(COORD3));
    pt     = (COORD3 **)malloc((gU_resolution+1) * sizeof(COORD3 *));
    if (x_axis == NULL || y_axis == NULL || pt == NULL) 
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_nff: allocation failed"); 
	exit(1);
    }

    for (num_edge=0;num_edge<gU_resolution+1;num_edge++) {
	pt[num_edge] = (COORD3 *)malloc((gV_resolution+1) * sizeof(COORD3));
	if (pt[num_edge] == NULL) 
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_nff: allocation failed"); 
	    exit(1);
	}
    }

    /* calculate axes used to find grid points */
    for (num_edge=0;num_edge<=gU_resolution;++num_edge) {
	angle = (PI/4.0) * (2.0*(double)num_edge/gU_resolution - 1.0);
	mid_axis[X] = 1.0; mid_axis[Y] = 0.0; mid_axis[Z] = 0.0;
	lib_create_rotate_matrix(rot_mx, Y_AXIS, angle);
	lib_transform_vector(x_axis[num_edge], mid_axis, rot_mx);
    }

    for (num_edge=0;num_edge<=gV_resolution;++num_edge) {
	angle = (PI/4.0) * (2.0*(double)num_edge/gV_resolution - 1.0);
	mid_axis[X] = 0.0; mid_axis[Y] = 1.0; mid_axis[Z] = 0.0;
	lib_create_rotate_matrix(rot_mx, X_AXIS, angle);
	lib_transform_vector(y_axis[num_edge], mid_axis, rot_mx);
    }

    /* set up grid of points on +Z sphere surface */
    for (u_pol=0;u_pol<=gU_resolution;++u_pol) {
	for (v_pol=0;v_pol<=gU_resolution;++v_pol) {
	    CROSS(pt[u_pol][v_pol], x_axis[u_pol], y_axis[v_pol]);
	    lib_normalize_vector(pt[u_pol][v_pol]);
	}
    }

    for (num_face=0;num_face<6;++num_face) {
	/* transform points to cube face */
	for (u_pol=0;u_pol<=gU_resolution;++u_pol) {
	    for (v_pol=0;v_pol<=gV_resolution;++v_pol) {
		lib_rotate_cube_face(pt[u_pol][v_pol], Z_AXIS, num_face);
	    }
	}

	/* output grid */
	for (u_pol=0;u_pol<gU_resolution;++u_pol) {
	    for (v_pol=0;v_pol<gV_resolution;++v_pol) {
		for (num_tri=0;num_tri<2;++num_tri) {
		    for (num_edge=0;num_edge<3;++num_edge) {
			num_vert = (num_tri*2 + num_edge) % 4;
			if (num_vert == 0) {
			    COPY_COORD3(edge_pt[num_edge], pt[u_pol][v_pol]);
			} else if ( num_vert == 1 ) {
			    COPY_COORD3(edge_pt[num_edge], pt[u_pol][v_pol+1]);
			} else if ( num_vert == 2 ) {
			    COPY_COORD3(edge_pt[num_edge],pt[u_pol+1][v_pol+1]);
			} else {
			    COPY_COORD3(edge_pt[num_edge], pt[u_pol+1][v_pol]);
			}
			COPY_COORD3(edge_norm[num_edge], edge_pt[num_edge]);
			edge_pt[num_edge][X] =
				edge_pt[num_edge][X] * center_pt[W] +
					       center_pt[X];
			edge_pt[num_edge][Y] =
				edge_pt[num_edge][Y] * center_pt[W] +
					       center_pt[Y];
			edge_pt[num_edge][Z] =
				edge_pt[num_edge][Z] * center_pt[W] +
					       center_pt[Z];

		    }
		    lib_output_polypatch(3, edge_pt, edge_norm);
		}
	    }
	}
    }

    /* Release any memory used */
    for (num_edge=0;num_edge<gU_resolution+1;num_edge++)
	free(pt[num_edge]);
    free(pt);
    free(y_axis);
    free(x_axis);
}

/*-----------------------------------------------------------------*/
static void
torus_evaluator(MATRIX trans, double theta, double phi, double r0, double r1, 
    COORD3 vert, COORD3 norm)
{
    COORD3 tvert, tnorm;
    double r1st = r1*sin(theta);
    double r1ct = r1*cos(theta);

    /* compute the position on the torus at (theta,phi) */
    SET_COORD3(tvert, 
	(r0 + r1st)*cos(phi),
	(r0 + r1st)*sin(phi),
	      r1ct);
    lib_transform_point(vert, tvert, trans);

    /* compute the normal of the torus at (theta,phi) */
    SET_COORD3(tnorm, 
	(r0 + r1st)*r1st*cos(phi),
	(r0 + r1st)*r1st*sin(phi),
	(r0 + r1st)*r1ct);
    lib_normalize_vector(tnorm);
    lib_transform_vector(norm, tnorm, trans);
}

/*-----------------------------------------------------------------*/
static void
lib_output_polygon_torus(COORD3 center, COORD3 normal, double ir, double or)
{
    int i, j, u_res, v_res;
    double u, v, delta_u, delta_v;
    MATRIX mx, imx;
    COORD3 vert[4], norm[4];

    if ( lib_normalize_vector(normal) < EPSILON2)
	SET_COORD3(normal, 0.0, 0.0, 1.0);

    lib_create_canonical_matrix(mx, imx, center, normal);

    u_res = 4*gU_resolution;
    v_res = 4*gV_resolution;
    delta_u = 2.0*PI/(double)u_res;
    delta_v = 2.0*PI/(double)v_res;

    /* Dump out polygons */
    for (i=0,u=0.0; i<u_res; i++,u+=delta_u) {
	for (j=0,v=0.0; j<v_res; j++,v+=delta_v) {
	    torus_evaluator(imx, u,         v,         ir, or, vert[0], norm[0]);
	    torus_evaluator(imx, u+delta_u, v,         ir, or, vert[1], norm[1]);
	    torus_evaluator(imx, u+delta_u, v+delta_v, ir, or, vert[2], norm[2]);
	    torus_evaluator(imx, u,         v+delta_v, ir, or, vert[3], norm[3]);
	    lib_output_polypatch(4, vert, norm);
	}
    }
}

/*-----------------------------------------------------------------*/
static void
lib_output_polygon_height(int height, int width, float *data,
    double x0, double x1, double y0, double y1, double z0, double z1)
{
    int i, j;
    double dx = (x1 - x0)/((double)(width  - 1));
    double dy = (y1 - y0)/((double)(height - 1));
    double dz = (z1 - z0);
    COORD3 verts[3];

#define	D(_r,_c) data[(_r)*width + (_c)]

    for (i=0;i<height-1;i++) {
	for (j=0;j<width-1;j++) {
	    SET_COORD3(verts[0], x0+dx*(j  ), y0+dy*(i  ), z0+dz*D(i  , j  ));
	    SET_COORD3(verts[1], x0+dx*(j+1), y0+dy*(i  ), z0+dz*D(i  , j+1));
	    SET_COORD3(verts[2], x0+dx*(j+1), y0+dy*(i+1), z0+dz*D(i+1, j+1));
	    lib_output_polygon(3, verts);

	    SET_COORD3(verts[0], x0+dx*(j  ), y0+dy*(i  ), z0+dz*D(i  , j  ));
	    SET_COORD3(verts[1], x0+dx*(j+1), y0+dy*(i+1), z0+dz*D(i+1, j+1));
	    SET_COORD3(verts[2], x0+dx*(j  ), y0+dy*(i+1), z0+dz*D(i+1, j  ));
	    lib_output_polygon(3, verts);
	}
    }
}

/*-----------------------------------------------------------------*/
static double
c (double w, double m)
{
    double cosine = cos(w);
    if (cosine < 0.0f)
	return -pow(fabs(cosine), m);
    else
	return  pow(fabs(cosine), m);
}

/*-----------------------------------------------------------------*/
static double
s (double w, double m)
{
    double sine = sin(w);
    if (sine < 0.0f)
	return -pow(fabs(sine), m);
    else
	return  pow(fabs(sine), m);
}

/*-----------------------------------------------------------------*/
static void
sq_sphere_val(double a1, double a2, double a3, double n, double e, 
    double u, double v, COORD3 P, double alpha)
{
    double	cu = c(u, e);
    double	su = s(u, e);
    double	cv = c(v, n) + alpha;
    double	sv = s(v, n);

    P[X] = a1*cv*cu;
    P[Y] = a2*cv*su;
    P[Z] = a3*sv;
}

/*-----------------------------------------------------------------*/
static void
sq_sphere_norm(double a1, double a2, double a3, double n, double e, 
    double u, double v, COORD3 N)
{
    double	cu = c(u, 2.0f - e);
    double	su = s(u, 2.0f - e);
    double	cv = c(v, 2.0f - n);
    double	sv = s(v, 2.0f - n);

    N[X] = a2*a3*cv*cu;
    N[Y] = a1*a3*cv*su;
    N[Z] = a1*a2*sv;

    if (N[X] > -0.00001 && N[X] < 0.00001) N[X] = 0.0f;
    if (N[Y] > -0.00001 && N[Y] < 0.00001) N[Y] = 0.0f;
    if (N[Z] > -0.00001 && N[Z] < 0.00001) N[Z] = 0.0f;

    lib_normalize_vector(N);
}

/*-----------------------------------------------------------------*/
static void
lib_output_sq_sphere(COORD3 center_pt, double a1, double a2, double a3, 
    double n, double e)
{
    int i, j, u_res, v_res;
    double u, delta_u, v, delta_v;
    COORD3 verts[4], norms[4];

    u_res = 4*gU_resolution;
    v_res = 4*gV_resolution;
    delta_u = 2.0*PI/(double)u_res;
    delta_v =     PI/(double)v_res;

    for (i=0,u=0.0; i<u_res; i++,u+=delta_u) {
	for (j=0,v=-PI/2.0; j<v_res; j++,v+=delta_v) {
	    if (j == 0) {
		sq_sphere_val( a1, a2, a3, n, e, u,         v,         verts[0], 0.0f);
		sq_sphere_val( a1, a2, a3, n, e, u+delta_u, v+delta_v, verts[1], 0.0f);
		sq_sphere_val( a1, a2, a3, n, e, u,         v+delta_v, verts[2], 0.0f);
		pfSetVec3(verts[0], 0.0f, 0.0f, -a3);

		sq_sphere_norm(a1, a2, a3, n, e, u,         v,         norms[0]);
		sq_sphere_norm(a1, a2, a3, n, e, u+delta_u, v+delta_v, norms[1]);
		sq_sphere_norm(a1, a2, a3, n, e, u,         v+delta_v, norms[2]);

		ADD3_COORD3(verts[0], verts[0], center_pt);
		ADD3_COORD3(verts[1], verts[1], center_pt);
		ADD3_COORD3(verts[2], verts[2], center_pt);

		lib_output_polypatch(3, verts, norms);
	    } else if (j == v_res-1) {
		sq_sphere_val( a1, a2, a3, n, e, u,         v,         verts[0], 0.0f);
		sq_sphere_val( a1, a2, a3, n, e, u+delta_u, v,         verts[1], 0.0f);
		sq_sphere_val( a1, a2, a3, n, e, u,         PI/2,      verts[2], 0.0f);
		pfSetVec3(verts[2], 0.0f, 0.0f, a3);

		sq_sphere_norm(a1, a2, a3, n, e, u,         v,         norms[0]);
		sq_sphere_norm(a1, a2, a3, n, e, u+delta_u, v,         norms[1]);
		sq_sphere_norm(a1, a2, a3, n, e, u,         PI/2,      norms[2]);

		ADD3_COORD3(verts[0], verts[0], center_pt);
		ADD3_COORD3(verts[1], verts[1], center_pt);
		ADD3_COORD3(verts[2], verts[2], center_pt);

		lib_output_polypatch(3, verts, norms);
	    } else {
		sq_sphere_val( a1, a2, a3, n, e, u,         v,         verts[0], 0.0f);
		sq_sphere_val( a1, a2, a3, n, e, u+delta_u, v,         verts[1], 0.0f);
		sq_sphere_val( a1, a2, a3, n, e, u+delta_u, v+delta_v, verts[2], 0.0f);
		sq_sphere_val( a1, a2, a3, n, e, u,         v+delta_v, verts[3], 0.0f);

		sq_sphere_norm(a1, a2, a3, n, e, u,         v,         norms[0]);
		sq_sphere_norm(a1, a2, a3, n, e, u+delta_u, v,         norms[1]);
		sq_sphere_norm(a1, a2, a3, n, e, u+delta_u, v+delta_v, norms[2]);
		sq_sphere_norm(a1, a2, a3, n, e, u,         v+delta_v, norms[3]);

		ADD3_COORD3(verts[0], verts[0], center_pt);
		ADD3_COORD3(verts[1], verts[1], center_pt);
		ADD3_COORD3(verts[2], verts[2], center_pt);
		ADD3_COORD3(verts[3], verts[3], center_pt);

		lib_output_polypatch(4, verts, norms);
	    }
	}
    }
}

/*-----------------------------------------------------------------*/
static void
lib_output_sq_torus(COORD3 center_pt, double a1, double a2, double a3, 
    double n, double e, double alpha)
{
    int i, j, u_res, v_res;
    double u, delta_u, v, delta_v;
    COORD3 verts[4], norms[4];

    u_res = 4*gU_resolution;
    v_res = 4*gV_resolution;
    delta_u = 2.0*PI/(double)u_res;
    delta_v = 2.0*PI/(double)v_res;

    for (i=0,u=-PI; i<u_res; i++,u+=delta_u) {
	for (j=0,v=-PI; j<v_res; j++,v+=delta_v) {
	    {
		sq_sphere_val( a1, a2, a3, n, e, u,         v,         verts[0], alpha);
		sq_sphere_val( a1, a2, a3, n, e, u+delta_u, v,         verts[1], alpha);
		sq_sphere_val( a1, a2, a3, n, e, u+delta_u, v+delta_v, verts[2], alpha);
		sq_sphere_val( a1, a2, a3, n, e, u        , v+delta_v, verts[3], alpha);

		sq_sphere_norm(a1, a2, a3, n, e, u,         v,         norms[0]);
		sq_sphere_norm(a1, a2, a3, n, e, u+delta_u, v,         norms[1]);
		sq_sphere_norm(a1, a2, a3, n, e, u+delta_u, v+delta_v, norms[2]);
		sq_sphere_norm(a1, a2, a3, n, e, u        , v+delta_v, norms[3]);

		ADD3_COORD3(verts[0], verts[0], center_pt);
		ADD3_COORD3(verts[1], verts[1], center_pt);
		ADD3_COORD3(verts[2], verts[2], center_pt);
		ADD3_COORD3(verts[3], verts[3], center_pt);

		lib_output_polypatch(4, verts, norms);
	    }
	}
    }
}

/***
 ***	S P D -- IRIS Performer Database Loader
 ***/

/*
 *	skip comments and blank space in input file
 */

static int
skipComments (FILE *fp)
{
    int		next;

    while ((next = getc(fp)) != EOF)
    {
	/* consume blank space */
	if (isspace(next))
	    continue;
	else
	/* consume comments up to and including newline */
	if (next == '#')
	{
	    while ((next = getc(fp)) != EOF && next != '\n')
		;
	}
	else
	/* at next token; return */
	{
	    ungetc(next, fp);
	    return next;
	}
    }

    /* reached EOF */
    return EOF;
}

/*
 * pfdLoadFile_nff -- Load Neutral File Format ".nff" files into IRIS Performer
 */

/*
 * CLASSIC NFF objects:
 *  b - background color
 *  c - cylinder or cone
 *  f - material properties
 *  l - light source
 *  p - polygon with no normals
 *  pp - polygon with normals per vertex
 *  s - sphere
 *  v - view information
 *
 * NEW object types:
 *  h - box/cube/hexahedron
 *  t - torus
 *  g - grid (height field)
 *  ss - super quadric sphere
 *  d - disc (washer shape)
 *  r - set U & V output resolution (tessellation factor)
 *
 * PERFORMER SPECIFIC hacks:
 *  build [name] - go ahead and build a node (for culling)
 *  geodes - make each primitive into a separate geode (toggle)
 */

PFNFF_DLLEXPORT /*extern */pfNode *
pfdLoadFile_nff (char *fileName)
{
    FILE	*nffFile	= NULL;
    char	*buffer		= NULL;
    pfGroup	*group		= NULL;
    pfNode	*node		= NULL;
    pfMaterial  *m      	= NULL;
    int		 i		= 0;
    int		 more		= 0;
    int		 geodes		= 0;
    int		 bCount		= 0;
    int		 cCount		= 0;
    int		 dCount		= 0;
    int		 fCount		= 0;
    int		 gCount		= 0;
    int		 hCount		= 0;
    int		 lCount		= 0;
    int		 pCount		= 0;
    int		 ppCount	= 0;
    int		 rCount		= 0;
    int		 sCount		= 0;
    int		 ssCount	= 0;
    int		 tCount		= 0;
    int		 stCount	= 0;
    int		 uCount		= 0;
    int		 vCount		= 0;
    double	 startTime	= pfGetTime();
    double	 elapsedTime	= 0.0;

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* establish a default material color */
    m = (pfMaterial*)pfdGetTemplateObject(pfGetMtlClassType());
    pfMtlColor(m, PFMTL_AMBIENT,  0.2f, 0.2f, 0.2f);
    pfMtlColor(m, PFMTL_DIFFUSE,  0.8f, 0.8f, 0.8f);
    pfMtlColor(m, PFMTL_SPECULAR, 1.0f, 1.0f, 1.0f);
    pfMtlShininess(m, 30.0f);
    pfMtlColorMode(m, PFMTL_FRONT, PFMTL_CMODE_OFF);
    pfdBldrStateAttr(PFSTATE_FRONTMTL, m);
    pfdBldrStateMode(PFSTATE_ENLIGHTING, PF_ON);
    pfdBldrStateMode(PFSTATE_CULLFACE, PFCF_BACK);

    /* reset global output resolution */
    gU_resolution = OUTPUT_RESOLUTION;
    gV_resolution = OUTPUT_RESOLUTION;

    /* open ".nff" file */
    if ((nffFile = pfdOpenFile(fileName)) == NULL)
	return NULL;

    /* allocate primitive buffer */
    prim = pfdNewGeom(primSize = 256);

    /* reset global primitive count */
    numTris = 0;

    /* all geometry will go into "group" */
    group = pfNewGroup();
    pfNodeName(group, fileName);

    /* read polygons from ".nff" file */
    while (skipComments(nffFile) != EOF)
    {
	char	entity[BUFFER_SIZE];

	/* extract entity type */
	fscanf(nffFile, "%s", entity);

	/* viewing vectors and angles */
	if (strcmp(entity, "v") == 0)
	{
	    float fx = 0.0f, fy = 0.0f, fz = 0.0f;
	    float ax = 0.0f, ay = 0.0f, az = 0.0f;
	    float ux = 0.0f, uy = 0.0f, uz = 0.0f;
	    float angle = 0.0f;
	    float hither = 0.0f;
	    int xres = 0.0f, yres = 0.0f;

	    /* 
	     * From: the eye location in XYZ 
	     */
	    skipComments(nffFile);
	    fscanf(nffFile, "%*s %f %f %f", &fx, &fy, &fz);

	    /* 
	     * At: a position to be at the center of the image, 
	     * in XYZ world coordinates. A.k.a. "lookat".
	     */
	    skipComments(nffFile);
	    fscanf(nffFile, "%*s %f %f %f", &ax, &ay, &az);

	    /*
	     * Up: a vector defining which direction is up, 
	     * as an XYZ vector.
	     */
	    skipComments(nffFile);
	    fscanf(nffFile, "%*s %f %f %f", &ux, &uy, &uz);

	    /*
	     * Angle: in degrees, defined as from the center of top 
	     * pixel row to bottom pixel row and left column to right 
	     * column.
	     */
	    skipComments(nffFile);
	    fscanf(nffFile, "%*s %f", &angle);

	    /*
	     * Hither: distance of the hither plane (if any) from the 
	     * eye.  Mostly needed for hidden surface algorithms.
	     */
	    skipComments(nffFile);
	    fscanf(nffFile, "%*s %f", &hither);

	    /*
	     * Resolution: in pixels, in x and in y.
	     */
	    skipComments(nffFile);
	    fscanf(nffFile, "%*s %d %d", &xres, &yres);
	    ++vCount;
	}
	else
	/* background color */
	if (strcmp(entity, "b") == 0)
	{
	    float r = 0.0f, g = 0.0f, b = 0.0f;

	    /*
	     * Background color.  A color is simply RGB with values between 
	     * 0 and 1. If no background color is set, assume RGB = {0,0,0}.
	     */
	    fscanf(nffFile, "%f %f %f", &r, &g, &b);
	    ++bCount;
	}
	else
	/* positional light location */
	if (strcmp(entity, "l") == 0)
	{
	    float x = 0.0f, y = 0.0f, z = 0.0f;
	    float r = 0.0f, g = 0.0f, b = 0.0f;

	    /*
	     * Positional light.  A light is defined by XYZ position. All light 
	     * entities must be defined before any objects are defined (this
	     * requirement is so that NFF files can be used by hidden surface 
	     * machines).  Lights have a non-zero intensity of no particular 
	     * value, if not specified (i.e. the program can determine a useful 
	     * intensity as desired); the red/green/blue color of the light can 
	     * optionally be specified.
	     */
	    fscanf(nffFile, "%f %f %f %f %f %f", &x, &y, &z, &r, &g, &b);
	    ++lCount;
	}
	else
	/* object material properties */
	if (strcmp(entity, "f") == 0)
	{
	    float r = 0.0f, g = 0.0f, b = 0.0f;
	    float ka = 0.05f;
	    float kd = 0.0f;
	    float ks = 0.0f;
	    float s = 0.0f;
	    float t = 0.0f;
	    float f = 0.0f;
	    pfMaterial  *m	= NULL;

	    /*
	     * RGB is in terms of 0.0 to 1.0.  Kd is the diffuse component, 
	     * Ks the specular, S is the Phong cosine power for highlights, 
	     * T is transmittance (fraction of contribution of the transmitting 
	     * ray).  Usually, 0 <= Kd <= 1 and 0 <= Ks <= 1, though it is not 
	     * required that Kd + Ks == 1.  Note that transmitting objects 
	     * ( T > 0 ) are considered to have two sides for algorithms that 
	     * need these (normally objects have one side).  The fill color is 
	     * used to color the objects following it until a new color is 
	     * assigned.
	     */
	    fscanf(nffFile, "%f %f %f %f %f %f %f %f", 
		&r, &g, &b, &kd, &ks, &s, &t, &f);
	    ++fCount;

	    /* color is set with each primitive (using PFMTL_CMODE_AD) */
	    pfSetVec4(prim->colors[0], r, g, b, 1.0f);

	    /* other properties are in the material */
	    m = (pfMaterial*)pfdGetTemplateObject(pfGetMtlClassType());

	    pfMtlColor(m, PFMTL_AMBIENT,  1.0f, 1.0f, 1.0f);
	    pfMtlColor(m, PFMTL_DIFFUSE,  1.0f, 1.0f, 1.0f);
	    pfMtlColor(m, PFMTL_SPECULAR, ks,   ks,   ks);
	    if (s > 128.0f) s = 128.0f;
	    pfMtlShininess(m, s);
	    pfMtlColorMode(m, PFMTL_FRONT, PFMTL_CMODE_AD);
	    pfdBldrStateAttr(PFSTATE_FRONTMTL, m);
	}
	else
	/* cone or cylinder primitive */
	if (strcmp(entity, "c") == 0)
	{
	    pfVec4 a;
	    pfVec4 b;

	    /*
	     * Cylinder or cone.  A cylinder is defined as having a radius and 
	     * an axis defined by two points, which also define the top and 
	     * bottom edge of the cylinder.  A cone is defined similarly, the 
	     * difference being that the apex and base radii are different.  
	     * The apex radius is defined as being smaller than the base radius.  
	     * Note that the surface exists without endcaps.  A negative value 
	     * for both radii means that only the inside of the object is visible 
	     * (objects are normally considered one sided, with the outside 
	     * visible).  Note that the base and apex cannot be coincident for a 
	     * cylinder or cone.  Making them coincident could be used to define
	     * endcaps, but none of the SPD scenes currently make use of this
	     * definition.
	     */
	    pfSetVec4(a, 0.0f, 0.0f, 0.0f, 0.0f);
	    pfSetVec4(b, 0.0f, 0.0f, 0.0f, 0.0f);

	    fscanf(nffFile, "%f %f %f %f %f %f %f %f", 
		&a[PF_X], &a[PF_Y], &a[PF_Z], &a[PF_W],
		&b[PF_X], &b[PF_Y], &b[PF_Z], &b[PF_W]);

	    lib_output_polygon_cylcone(a, b);
	    ++cCount;
	    ++more;
	}
	else
	/* sphere primitive */
	if (strcmp(entity, "s") == 0)
	{
	    pfVec4 c;

	    /*
	     * Sphere.  A sphere is defined by a radius and center position.
	     * If the radius is negative, then only the sphere's inside is 
	     * visible (objects are normally considered one sided, with the 
	     * outside visible).  Currently none of the SPD scenes make use 
	     * of negative radii.
	     */
	    pfSetVec4(c, 0.0f, 0.0f, 0.0f, 0.0f);

	    fscanf(nffFile, "%f %f %f %f", 
		&c[PF_X], &c[PF_Y], &c[PF_Z], &c[PF_W]);

	    lib_output_polygon_sphere(c);
	    ++sCount;
	    ++more;
	}
	else
	/* polygon primitive */
	if (strcmp(entity, "p") == 0)
	{
	    int v, nv;
	    float x = 0.0f, y = 0.0f, z = 0.0f;

	    /*
	     * Polygon.  A polygon is defined by a set of vertices.  With 
	     * these databases, a polygon is defined to have all points 
	     * coplanar.  A polygon has only one side, with the order of the 
	     * vertices being counterclockwise as you face the polygon 
	     * (right-handed coordinate system).  The first two edges must 
	     * form a non-zero convex angle, so that the normal and side 
	     * visibility can be determined by using just the first three 
	     * vertices.
	     */
	    fscanf(nffFile, "%d", &nv);

	    /* enlarge primitive buffer if required */
	    if (nv > primSize)
		pfdResizeGeom(prim, primSize = nv);

	    /* read vertices from file */
	    for (v = 0; v < nv; v++)
	    {
		skipComments(nffFile);
		fscanf(nffFile, "%f %f %f", &x, &y, &z);
		pfSetVec3(prim->coords[v], x, y, z);
	    }

	    lib_output_polygon(nv, prim->coords);
	    ++pCount;
	    ++more;
	}
	else
	/* polygonal patch primitive */
	if (strcmp(entity, "pp") == 0)
	{
	    int v, nv;
	    float x = 0.0f, y = 0.0f, z = 0.0f;
	    float nx = 0.0f, ny = 0.0f, nz = 0.0f;

	    /* Polygonal patch.  A patch is defined by a set of vertices and 
	     * their normals.  With these databases, a patch is defined to have 
	     * all points coplanar.  A patch has only one side, with the order 
	     * of the vertices being counterclockwise as you face the patch 
	     * (right-handed coordinate system).  The first two edges must form 
	     * a non-zero convex angle, so that the normal and side visibility 
	     * can be determined.
	     */
	    fscanf(nffFile, "%d", &nv);

	    /* enlarge primitive buffer if required */
	    if (nv > primSize)
		pfdResizeGeom(prim, primSize = nv);

	    /* read vertices from file */
	    for (v = 0; v < nv; v++)
	    {
		skipComments(nffFile);
		fscanf(nffFile, "%f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz);
		pfSetVec3(prim->coords[v], x,  y,  z);
		pfSetVec3(prim->norms[v], nx, ny, nz);
	    }
	    lib_output_polypatch(nv, prim->coords, prim->norms);
	    ++ppCount;
	    ++more;
	}
	else
	/* hexahedron (box/cube) primitive */
	if (strcmp(entity, "h") == 0)
	{
	    pfVec3 p1;
	    pfVec3 p2;

	    /*
	     * Hexahedron.  A hexahedron (aka "box") is defined by two
	     * points on diagonally opposite corners.
	     */
	    pfSetVec3(p1, 0.0f, 0.0f, 0.0f);
	    pfSetVec3(p2, 0.0f, 0.0f, 0.0f);

	    fscanf(nffFile, "%f %f %f %f %f %f", 
		&p1[PF_X], &p1[PF_Y], &p1[PF_Z],
		&p2[PF_X], &p2[PF_Y], &p2[PF_Z]);

	    lib_output_polygon_box(p1, p2);
	    ++hCount;
	    ++more;
	}
	else
	/* torus primitive */
	if (strcmp(entity, "t") == 0)
	{
	    pfVec3 c;
	    pfVec3 n;
	    float innerRadius = 0.0f;
	    float outerRadius = 0.0f;

	    /*
	     * Torus.  A torus is defined by a center position, a normal
	     * that is tangent to the plane in which the torus sits, and
	     * the inner and outer radii which are distances from the
	     * center to the inner and outer edges of the surface, resp.
	     */
	    pfSetVec3(c, 0.0f, 0.0f, 0.0f);
	    pfSetVec3(n, 0.0f, 0.0f, 0.0f);

	    fscanf(nffFile, "%f %f %f %f %f %f %f %f", 
		&c[PF_X], &c[PF_Y], &c[PF_Z],
		&n[PF_X], &n[PF_Y], &n[PF_Z],
		&innerRadius, &outerRadius);

	    lib_output_polygon_torus(c, n, innerRadius, outerRadius);
	    ++tCount;
	    ++more;
	}
	else
	/* grid (height field) primitive */
	if (strcmp(entity, "g") == 0)
	{
	    int nx = 0, ny = 0;
	    float x0 = 0.0f, x1 = 0.0f;
	    float y0 = 0.0f, y1 = 0.0f;
	    float z0 = 0.0f, z1 = 0.0f;
	    float *data = NULL;
	    int i, j;

	    /*
	     * Grid (height field). Defined by a two-dimensional table
	     * of values. The table has ny rows and nx columns. The
	     * nx*ny elevations are interpreted as follows:
	     *  dx = (x1 - x0)/(nx - 1)
	     *  dy = (y1 - y0)/(ny - 1)
	     *  dz = (z1 - z0)
	     *	x[i][j] = x0 + dx*j
	     *	y[i][j] = y0 + dy*i
	     *  z[i][j] = z0 + dz*data[i][j]
	     */
	    fscanf(nffFile, "%d %d %f %f %f %f %f %f", 
		&nx, &ny, &x0, &x1, &y0, &y1, &z0, &z1);

	    /* read data array */
	    data = (float *)malloc(nx*ny*sizeof(float));
	    if (data == NULL)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_nff: allocation failed"); 
		exit(1);
	    }
	    for (j = 0; j < ny; j++)
		for (i = 0; i < nx; i++)
		{
		    skipComments(nffFile);
		    fscanf(nffFile, "%f", &data[j*nx + i]);
		}

	    lib_output_polygon_height(ny, nx, data, x0, x1, y0, y1, z0, z1);
	    free((void *)data);
	    ++gCount;
	    ++more;
	}
	else
	/* superquadric sphere primitive */
	if (strcmp(entity, "ss") == 0)
	{
	    pfVec3 c;
	    float a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
	    float n = 0.0f, e = 0.0f;

	    /*
	     * Superquadric sphere (a la Al Barr). 
	     */
	    pfSetVec3(c, 0.0f, 0.0f, 0.0f);

	    fscanf(nffFile, "%f %f %f %f %f %f %f %f", 
		&c[PF_X], &c[PF_Y], &c[PF_Z], &a1, &a2, &a3, &n, &e);

	    lib_output_sq_sphere(c, a1, a2, a3, n, e);
	    ++ssCount;
	    ++more;
	}
	else
	/* superquadric torus primitive */
	if (strcmp(entity, "st") == 0)
	{
	    pfVec3 c;
	    float a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
	    float n = 0.0f, e = 0.0f, alpha = 2.0;

	    /*
	     * Superquadric torus (a la Al Barr). 
	     */
	    pfSetVec3(c, 0.0f, 0.0f, 0.0f);

	    fscanf(nffFile, "%f %f %f %f %f %f %f %f %f", 
		&c[PF_X], &c[PF_Y], &c[PF_Z], &a1, &a2, &a3, &n, &e, &alpha);

	    lib_output_sq_torus(c, a1, a2, a3, n, e, alpha);
	    ++stCount;
	    ++more;
	}
	else
	/* disc primitive */
	if (strcmp(entity, "d") == 0)
	{
	    pfVec3 c;
	    pfVec3 n;
	    float innerRadius = 0.0f;
	    float outerRadius = 0.0f;

	    /*
	     * Disc.  A disc is defined by a center position, a normal
	     * that is tangent to the plane in which the disc lies, and
	     * the inner and outer radii which are distances from the
	     * center to the inner and outer edges of the surface, resp.
	     */
	    pfSetVec3(c, 0.0f, 0.0f, 0.0f);
	    pfSetVec3(n, 0.0f, 0.0f, 0.0f);

	    fscanf(nffFile, "%f %f %f %f %f %f %f %f", 
		&c[PF_X], &c[PF_Y], &c[PF_Z],
		&n[PF_X], &n[PF_Y], &n[PF_Z],
		&innerRadius, &outerRadius);

	    lib_output_polygon_disc(c, n, innerRadius, outerRadius);
	    ++dCount;
	    ++more;
	}
	else
	/* set output resolution */
	if (strcmp(entity, "r") == 0)
	{
	    /*
	     * Resolution.  Set the U & V direction tessellation factor.
	     */
	    fscanf(nffFile, "%d %d", &gU_resolution, &gV_resolution);
	    ++rCount;
	}
	else
	/* go ahead and build a geode */
	if (strcmp(entity, "build") == 0)
	{
	    char	name[PF_MAXSTRING];
	    /*
	     * Build.  Build all current geometry.
	     */
	    name[0] = '\0';
	    fscanf(nffFile, "%s", name);
	    if (more && (node = pfdBuild()) != NULL)
	    {
		pfAddChild(group, node);
		pfNodeName(node, name);
		more = 0;
	    }
	}
	else
	/* build each primitive as an independent geode */
	if (strcmp(entity, "geodes") == 0)
	{
	    /*
	     * Geodes. Toggle "geode per primitive" mode.
	     */
	    geodes = !geodes;
	}
	else
	/* unknown entity type */
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_nff: unknown: %s", buffer);
	    ++uCount;
	}

	/* build a node if we're in the optional "geodes" mode */
	if (geodes && more && (node = pfdBuild()) != NULL)
	{
	    pfAddChild(group, node);
	    pfNodeName(node, NULL);
	    more = 0;
	}
    }

    /* close ".nff" file */
    fclose(nffFile);

    /* release dynamic storage */
    pfdDelGeom(prim);

    /* get a complete scene graph representing file's polygons */
    if (more && (node = pfdBuild()) != NULL)
    {
	pfAddChild(group, node);
	pfNodeName(node, NULL);
	more = 0;
    }

    /* print statistics */
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_nff: %s", fileName);

    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "  Input Data:");
    if (vCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Viewpoints:         %8ld", vCount);
    if (bCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Backgrounds:        %8ld", bCount);
    if (lCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Lights:             %8ld", lCount);
    if (fCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Colors:             %8ld", fCount);
    if (rCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Resolutions:        %8ld", rCount);

    if (pCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Polygons:           %8ld", pCount);
    if (ppCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Polygonal patches:  %8ld", ppCount);
    if (cCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Cylinders or cones: %8ld", cCount);
    if (dCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Discs:              %8ld", dCount);
    if (gCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Grids:              %8ld", gCount);
    if (hCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Hexahedrons:        %8ld", hCount);
    if (sCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Spheres:            %8ld", sCount);
    if (ssCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Superquadric spheres:%7ld", ssCount);
    if (tCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Tori:               %8ld", tCount);
    if (stCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Superquadric tori:  %8ld", stCount);

    if (uCount != 0)
	pfNotify(PFNFY_INFO,   PFNFY_MORE,  "    Unknown commands:   %8ld", uCount);

    return (pfNode *)group;
}
