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
 * linmath.c
 *
 * $Revision: 1.33 $
 * $Date: 2002/11/13 00:23:21 $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef WIN32
#include <strings.h>
#include <sys/time.h>
#endif
#include <Performer/pr.h>

#ifdef WIN32
#define fabsf     fabs
#define drand48() ((float)rand() / (float)RAND_MAX)
#define srand48   srand
#endif

static void 
MakeRandomVec3(pfVec3 v)
{
    long i;

    for (i=0 ; i<3 ; i++) 
      v[i] = drand48();
}

static void 
MakeRandomVec4(pfVec4 v)
{
    long i;

    for (i=0 ; i<4 ; i++) 
      v[i] = drand48();
}

static void 
MakeZeroVec3(pfVec3 v)
{
    long i;

    for (i=0 ; i<3 ; i++) 
      v[i] = 0.0;
}

static void 
MakeZeroMat(pfMatrix m)
{
    long i, j;

    for (i=0 ; i<4 ; i++) 
      for (j=0 ; j<4 ; j++) 
	m[i][j] = 0.0;
}

static void 
PrintVec3(pfVec3 v)
{
    printf("%10.5f %10.5f %10.5f\n", v[0], v[1], v[2]);
}

static void 
PrintQuat(pfQuat v)
{
    printf("%10.5f %10.5f %10.5f %10.5f:  norm %10.5f\n", 
	   v[0], v[1], v[2], v[3], pfDotVec4(v, v));
}

static void 
PrintMat(pfMatrix m)
{
    long i;

    for (i=0 ; i<4 ; i++) 
      printf("%10.5f %10.5f %10.5f %10.5f\n",
	     m[i][0], m[i][1], m[i][2], m[i][3]);
}

static long verbose = 0;

static void 
DbgPrintVec3(char *msg, pfVec3 v)
{
    if (verbose)
    {
	printf("%s:\n", msg);
	PrintVec3(v);
    }
}

static void 
DbgPrintQuat(char *msg, pfQuat v)
{
    if (verbose)
    {
	printf("%s:\n", msg);
	PrintQuat(v);
    }
}

static void 
DbgPrintMat(char *msg, pfMatrix m)
{
    if (verbose)
    {
	printf("%s:\n", msg);
	PrintMat(m);
    }
}

static float epsilon = 0.0001;

static int
_AssertEqVec3(pfVec3 v1, pfVec3 v2, char *msg, 
			 char *file, long line)
{
    int ret = TRUE;
    long i;
    for (i=0 ; i<3 ; i++) 
	if (fabsf(v1[i] - v2[i]) > epsilon)
	{
	    ret = FALSE;
	    fprintf(stderr, "Assertion (%s) failed at line %ld in %s\n",
		    msg, line, file);
	    fprintf(stderr, "    vec[%ld]: %f != %f\n", i, v1[i], v2[i]);
	}
    return ret;
}

#define AssertEqVec3(v1, v2, msg) _AssertEqVec3(v1, v2, msg, __FILE__, __LINE__)

static int
_AssertEqQuat(pfQuat v1, pfQuat v2, char *msg, 
			 char *file, long line)
{
    pfQuat qq1;
    int ret = TRUE;
    long i;
    if (pfDotVec4(v1, v2) < 0.0f)
	PFNEGATE_VEC4(qq1, v1);
    else
	PFCOPY_VEC4(qq1, v1);

    for (i=0 ; i<3 ; i++) 
	if (fabsf(qq1[i] - v2[i]) > epsilon)
	{
	    ret = FALSE;
	    fprintf(stderr, "Assertion (%s) failed at line %ld in %s\n",
		    msg, line, file);
	    fprintf(stderr, "    vec[%ld]: %f != %f\n", i, qq1[i], v2[i]);
	}
    return ret;
}

#define AssertEqQuat(v1, v2, msg) _AssertEqQuat(v1, v2, msg, __FILE__, __LINE__)

static int
_AssertEqMat(pfMatrix m1, pfMatrix m2, char *msg, 
			 char *file, long line)
{
    int ret = TRUE;
    long i, j;
    for (i=0 ; i<4 ; i++) 
	for (j=0 ; j<4 ; j++) 
	    if (fabsf(m1[i][j] - m2[i][j]) > epsilon)
	    {
		ret = FALSE;
		fprintf(stderr, "Assertion (%s) failed at line %ld in %s\n",
			msg, line, file);
		fprintf(stderr, "    mat[%ld][%ld]: %f != %f\n",
			i, j, m1[i][j], m2[i][j]);
	    }
    return ret;
}

#define AssertEqMat(m1, m2, msg) _AssertEqMat(m1, m2, msg, __FILE__, __LINE__)

long seed = 0xe58a3e61;

int
main(int argc, char **argv)
{
    pfMatrix nullmat, ident;
    pfVec3 nullvec;
    long i;
    float s;

    argv++; argc--;
    while (argc > 0)
    {
	if (!strcmp(*argv, "-s"))
	{
	    argv++; argc--; 
	    if (argc > 0)
	    {
		seed = atoi(*argv);
		argv++; argc--; 
	    }
	}
	else if (!strcmp(*argv, "-v"))
	{
	    verbose = 1;
	    argv++; argc--; 
	}
	else 
	{
	    fprintf(stderr, "linmath.fun [-s <seed>] [-v]\n");
	    exit(-1);
	}
    }
	    


    srand48(seed);
    MakeZeroVec3(nullvec);
    MakeZeroMat(nullmat);
    pfMakeIdentMat(ident);

    /*
     * test vector scale, add, sub, combine
     */
    {
	pfVec3 v1, v2, v3, v4;

	MakeRandomVec3(v1);
	MakeRandomVec3(v2);
	
	DbgPrintVec3("v1", v1);
	DbgPrintVec3("v2", v2);
	
	pfCombineVec3(v4, 2.0, v1, 3.0, v2);
	pfScaleVec3(v1, 2.0, v1);
	pfScaleVec3(v2, 3.0, v2);
	pfAddVec3(v3, v1, v2);
	
	DbgPrintVec3("comb: 2 v1 + 3 v2", v4);
	DbgPrintVec3("add:  2 v1 + 3 v2", v3);
	
	pfSubVec3(v3, v3, v4);
	DbgPrintVec3("sub", v3);
	AssertEqVec3(v3, nullvec, "Vec3 scale/add/sub/combine");
    }
  
    for (i=0 ; i<3 ; i++)	/* PFX, PFY, PFZ */
    {
	pfVec3 v1, v2;
	pfVec3 rotax;
	pfMatrix m1, m2, m3;

	MakeRandomVec3(v1);
	pfNormalizeVec3(v1);
	/* Rot about axis */
	pfSetVec3(rotax, 0.f, 0.f, 0.f);
	rotax[i] = 1.0;

	pfMakeRotMat(m1, 30.0f, rotax[0], rotax[1], rotax[2]);
	DbgPrintMat("rot30", m1);

	/* same mat from Rot about a specified axis */
	pfGetMatRowVec3(ident, i, v1);
	pfMakeRotMat(m2, 30.0f, v1[0], v1[1], v1[2]);
	DbgPrintMat("rot30about", m2);
	AssertEqMat(m2, m1, "Rot/RotAbout");

	/* same mat from Rot A onto B */
	pfGetMatRowVec3(ident, (i+1)%3, v1);
	pfXformVec3(v2, v1, m1); 
	pfMakeVecRotVecMat(m2, v1, v2);
	DbgPrintMat("rot30to", m2);
	AssertEqMat(m2, m1, "Rot/RotTo");

	/* check successive transforms against a rotation */
	pfMultMat(m2, m1, m1);
	DbgPrintMat("rot30*rot30", m2);

	pfPostMultMat(m2, m1);
	DbgPrintMat("rot30*rot30*rot30", m2);
	
	pfMakeRotMat(m3, 90.0f, rotax[0], rotax[1], rotax[2]);
	DbgPrintMat("rot90", m3);
	
	pfSubMat(m2, m2, m3);
	DbgPrintMat("sub", m2);
	AssertEqMat(m2, nullmat, "mat Rot/mul/sub");
    }
    /*
     * test Rot of A onto B
     */
    {
	pfVec3 v1, v2, v3;
	pfMatrix m1;

	MakeRandomVec3(v1);
	MakeRandomVec3(v2);
	pfNormalizeVec3(v1);
	pfNormalizeVec3(v2);
	pfMakeVecRotVecMat(m1, v1, v2);
	pfXformVec3(v3, v1, m1);
	DbgPrintVec3("from", v1);
	DbgPrintVec3("to", v2);
	DbgPrintVec3("result", v3);
	AssertEqVec3(v3, v2, "Arb Rot To");
    }
    /*
     * test inversion of Orthonormal Matrix
     */
    {
	pfVec3 v1, v2, v3;
	pfMatrix m1, m2, m3;
	
	MakeRandomVec3(v1);
	pfNormalizeVec3(v1);
	MakeRandomVec3(v2);
	pfNormalizeVec3(v2);
	pfMakeVecRotVecMat(m1, v1, v2);
	s = pfLengthVec3(v2)/pfLengthVec3(v1);
	pfPreScaleMat(m1, s, s, s, m1);
	
	MakeRandomVec3(v3);
	pfMakeTransMat(m2, v3[0], v3[1], v3[2]);
	pfPreMultMat(m1, m2);
	
	pfInvertOrthoNMat(m3, m1);
	DbgPrintMat("ON", m1);
	DbgPrintMat("inv(ON)", m3);
	pfPostMultMat(m3, m1);
	DbgPrintMat("prod", m3);
	AssertEqMat(m3, ident, "Orthonormal inverse");
    }
    /*
     * test inversion of Ortho Matrix
     */
    {
	pfVec3 v1, v2, v3;
	pfMatrix m1, m2, m3;
	
	MakeRandomVec3(v1);
	pfNormalizeVec3(v1);
	MakeRandomVec3(v2);
	pfNormalizeVec3(v2);
	pfMakeVecRotVecMat(m1, v1, v2);
	s = pfLengthVec3(v2)/pfLengthVec3(v1);
	pfPreScaleMat(m1, s, s, s, m1);

	MakeRandomVec3(v1);
	pfNormalizeVec3(v1);
	MakeRandomVec3(v2);
	pfNormalizeVec3(v2);
	pfMakeVecRotVecMat(m2, v1, v2);
	
	MakeRandomVec3(v3);
	pfMakeTransMat(m2, v3[0], v3[1], v3[1]);
	pfPreMultMat(m1, m2);
	
	pfInvertOrthoMat(m3, m1);
	DbgPrintMat("Ortho", m1);
	DbgPrintMat("inv(ortho)", m3);
	pfPostMultMat(m3, m1);
	DbgPrintMat("prod", m3);
	AssertEqMat(m3, ident, "Ortho inverse");
    }
	
    /*
     * test inversion of Affine Matrix
     */
    {
	pfVec3 v1, v2, v3;
	pfMatrix m1, m2, m3;

	MakeRandomVec3(v3);
	pfMakeScaleMat(m2, v3[0], v3[1], v3[2]);
	pfPreMultMat(m1, m2);
	
	MakeRandomVec3(v1);
	pfNormalizeVec3(v1);
	MakeRandomVec3(v2);
	pfNormalizeVec3(v2);
	pfMakeVecRotVecMat(m1, v1, v2);
	s = pfLengthVec3(v2)/pfLengthVec3(v1);
	pfPreScaleMat(m1, s, s, s, m1);

	MakeRandomVec3(v1);
	pfNormalizeVec3(v1);
	MakeRandomVec3(v2);
	pfNormalizeVec3(v2);
	pfMakeVecRotVecMat(m2, v1, v2);
	
	MakeRandomVec3(v3);
	pfMakeTransMat(m2, v3[0], v3[1], v3[1]);
	pfPreMultMat(m1, m2);
	
	pfInvertAffMat(m3, m1);
	DbgPrintMat("aff1", m1);
	DbgPrintMat("inv(aff1)", m3);
	pfPostMultMat(m3, m1);
	DbgPrintMat("prod", m3);
	AssertEqMat(m3, ident, "affine inverse");
    }	
	
    /*
     * test inversion of Full Matrix
     */
    {
	pfVec3 v3;
	pfMatrix m1, m2, m3;

	MakeRandomVec3(v3);
	pfSetMatRowVec3(m1, 0, v3);
	MakeRandomVec3(v3);
	pfSetMatRowVec3(m1, 1, v3);
	MakeRandomVec3(v3);
	pfSetMatRowVec3(m1, 2, v3);
	MakeRandomVec3(v3);
	pfSetMatRowVec3(m1, 3, v3);
	MakeRandomVec3(v3);
	pfSetMatColVec3(m1, 3, v3);
	m1[3][3] = 1.23456;
	pfInvertFullMat(m3, m1);
	DbgPrintMat("inv1", m1);
	DbgPrintMat("inv(inv1)", m3);
	pfPostMultMat(m3, m1);
	DbgPrintMat("prod", m3);
	AssertEqMat(m3, ident, "general inverse");
	
	for (i = 0 ; i < 5 ; i++)
	{
	    pfCoord c1,c2;
	    
	    MakeRandomVec3(c1.xyz);
	    MakeRandomVec3(c1.hpr);
	    pfScaleVec3(c1.hpr, 100.0f, c1.hpr);
	    
	    pfMakeCoordMat(m1, &c1);
	    
	    /* construct one by components manually */
	    pfMakeTransMat(m2, c1.xyz[0], c1.xyz[1], c1.xyz[2]);
	    
	    pfMakeRotMat(m3, c1.hpr[0], 0.0, 0.0, 1.0);
	    pfPreMultMat(m2, m3);
	    
	    pfMakeRotMat(m3, c1.hpr[1], 1.0, 0.0, 0.0);
	    pfPreMultMat(m2, m3);
	    
	    pfMakeRotMat(m3, c1.hpr[2], 0.0, 1.0, 0.0);
	    pfPreMultMat(m2, m3);
	    
	    AssertEqMat(m2, m1, "hpr by construction");
	    
	    /* check get coords */
	    pfGetOrthoMatCoord(m1, &c2);
	    pfMakeCoordMat(m3, &c2);
	    AssertEqMat(m1, m3, "coord inverse");
	    DbgPrintMat("orig:",m1);
	    DbgPrintMat("redo:",m3);
	    DbgPrintVec3("xyz:",c2.xyz);
	    DbgPrintVec3("hpr:",c2.hpr);
	}
    }
	
	
    /*
     * test quaternions
     */
    {
	pfQuat q1, q2, q3;	
	pfMatrix m1, m2, m3, m3q;
	pfVec3 axis;
	float angle1, angle2, angle, t;
	
	MakeRandomVec4(q1);
	pfNormalizeVec4(q1);
	MakeRandomVec4(q2);
	pfNormalizeVec4(q2);
	
	pfMakeQuatMat(m1, q1);
	pfMakeQuatMat(m2, q2);
	
	pfGetOrthoMatQuat(m1, q3);
	AssertEqQuat(q1, q3, "q1 == q1 -> matrix -> q1");
	
	pfGetOrthoMatQuat(m2, q3);
	AssertEqQuat(q2, q3, "q2 == q2 -> matrix -> q2");
	
	pfMultQuat(q3, q1, q2);
	pfMakeQuatMat(m3q, q3);
	pfMultMat(m3, m1, m2);
	
	if (!AssertEqMat(m3, m3q, "quaternion xform"))
	{
	    DbgPrintMat("m1", m1);
	    DbgPrintMat("m2", m2);
	    DbgPrintMat("m3", m3);
	    DbgPrintMat("m3q", m3q);
	    pfMultMat(m3, m2, m1);
	    DbgPrintMat("m3", m3);
	}
	
	pfInvertQuat(q2, q1);
	pfMultQuat(q3, q2, q1);
	pfMakeQuatMat(m3q, q3);
	
	AssertEqMat(m3q, ident, "quaternion inverse");
	
	MakeRandomVec3(axis);
	pfNormalizeVec3(axis);
	angle1 = -drand48()*90.0f;
	angle2 =  drand48()*90.0f;
	t = drand48();
	
	pfMakeRotMat(m1, angle1, axis[0], axis[1], axis[2]);
	pfMakeRotQuat(q1, angle1, axis[0], axis[1], axis[2]);
	pfMakeQuatMat(m3q, q1);
	if (!AssertEqMat(m1, m3q, "make rot quat q1"))
	{
	    DbgPrintMat("m1", m1);
	    DbgPrintQuat("q1", q1);
	    DbgPrintMat("m3q", m3q);
	}
	
	pfMakeRotMat(m2, angle2, axis[0], axis[1], axis[2]);
	pfMakeRotQuat(q2, angle2, axis[0], axis[1], axis[2]);
	pfMakeQuatMat(m3q, q2);
	if (!AssertEqMat(m2, m3q, "make rot quat q2"))
	{
	    DbgPrintMat("m2", m2);
	    DbgPrintQuat("q2", q2);
	    DbgPrintMat("m3q", m3q);
	}
	
	angle = (1.0f-t) * angle1 + t * angle2;
	pfMakeRotMat(m3, angle, axis[0], axis[1], axis[2]);
	
	pfMakeRotQuat(q1, angle1, axis[0], axis[1], axis[2]);
	pfMakeRotQuat(q2, angle2, axis[0], axis[1], axis[2]);
	pfSlerpQuat(q3, t, q1, q2);
	pfMakeQuatMat(m3q, q3);
	
	if (!AssertEqMat(m3q, m3, "quaternion slerp"))
	{
	    printf ("%7.2f between (%7.2f deg, %7.2f deg)\n",
		    angle, angle1, angle2);
	    DbgPrintVec3("axis", axis);
	    DbgPrintMat("m1", m1);
	    DbgPrintQuat("q1", q1);
	    DbgPrintMat("m2", m2);
	    DbgPrintQuat("q2", q2);
	    DbgPrintMat("m3", m3);
	    DbgPrintQuat("q3", q3);
	    DbgPrintMat("m3q", m3q);
	}
    }
    return 0;
}








