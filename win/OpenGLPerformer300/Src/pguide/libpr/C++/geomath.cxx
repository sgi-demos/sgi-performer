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
 * geomath.fun.c
 *
 * $Revision: 1.6 $
 * $Date: 2002/11/13 01:04:54 $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#ifndef WIN32
#include <sys/time.h>
#endif
#include <Performer/pr/pfGeoMath.h>

#ifdef WIN32
#define fabsf     fabs
#define drand48() ((float)rand() / (float)RAND_MAX)
#define srand48   srand
#endif

static void 
MakeRandomVec3(pfVec3 &v)
{
    long i;
    
    for (i=0 ; i<3 ; i++) 
	v[i] = drand48();
}

static void 
PrintVec3(char *str, pfVec3 &v)
{
    printf("%s%10.5f %10.5f %10.5f\n", str, v[0], v[1], v[2]);
}

static void
PrintCyl(char *str, pfCylinder *cyl)
{
    printf("%s\n", str);
    PrintVec3("\taxis:\t", cyl->axis);
    PrintVec3("\tcent:\t", cyl->center);
    printf("\thlen:\t%10.5f\n", cyl->halfLength);
    printf("\tradi:\t%10.5f\n", cyl->radius);
}

static void
PrintSeg(char *str, pfSeg *seg)
{
    pfVec3 tmpvec;
    tmpvec.combine(1.0f, seg->pos, seg->length, seg->dir);
    printf("%s\n", str);
    PrintVec3("\tstart:\t", seg->pos);
    PrintVec3("\tend:\t", tmpvec);
    PrintVec3("\tdir:\t", seg->dir);
    printf("\tlength:\t%10.5f\n", seg->length);
}

static float epsilon = 0.0001f;


static void 
AssertVec3Eq(pfVec3 &v1, pfVec3 &v2, char *msg, 
	     char *file, long line)
{
    long i;
    for (i=0 ; i<3 ; i++) 
	if (fabsf(v1[i] - v2[i]) > epsilon)
	{
	    fprintf(stderr, "Assertion (%s) failed at line %ld in %s\n",
		    msg, line, file);
	    fprintf(stderr, "    vec[%ld]: %f != %f\n", i, v1[i], v2[i]);
	}
}

#define AssertVec3Eq(v1, v2, msg) AssertVec3Eq(v1, v2, msg, __FILE__, __LINE__)

static void 
AssertFloatEq(float f1, float f2, char *msg, char *file, long line)
{
    if (fabs(f1 - f2) > epsilon)
    {
	fprintf(stderr, "Assertion (%s) failed at line %ld in %s\n",
		msg, line, file);
	fprintf(stderr, "    %f != %f\n", f1, f2);
    }
}

#define AssertFloatEq(v1, v2, msg) AssertFloatEq(v1, v2, msg, __FILE__, __LINE__)

int verbose = 0;

int
main(int argc, char **argv)
{
    pfCylinder cyl1;
    pfSeg seg1;
    pfSeg clipSeg;
    pfVec3 pt1, pt2;
    float t1, t2;
    long isect;
    
    if (argc > 1)
	verbose = 1;
    
    /*
     * simple test of pfCylIsectSeg 
     */
    {
	pfVec3 tmpvec;
	pt1.set( -2.0f, 0.0f, 0.0f);
	pt2.set( 2.0f, 0.0f, 0.0f);
	
	seg1.makePts(pt1, pt2);
	
	cyl1.axis.set(1.0f, 0.0f, 0.0f);
	cyl1.center.set(0.0f, 0.0f, 0.0f);
	cyl1.radius = 0.5f;
	cyl1.halfLength = 1.0f;
	
	isect = cyl1.isect(&seg1, &t1, &t2);
	
	clipSeg.clip(&seg1, t1, t2);
	if (verbose)
	{
	    printf("Segment against cylinder #1.\n");
	    PrintSeg("oseg:\t", &seg1);
	    PrintCyl("cyl:\t", &cyl1);
	    printf("isect: 0x%x, t1: %f, t2: %f\n", isect, t1, t2);
	    PrintSeg("cseg:\t", &clipSeg);
	}
	AssertFloatEq(clipSeg.length, 2.0f, "clipSeg.length");
	tmpvec.set( 1.0f, 0.0f, 0.0f);
	AssertVec3Eq(clipSeg.dir, tmpvec, "clipSeg.dir");
	tmpvec.set( -1.0f, 0.0f, 0.0f);
	AssertVec3Eq(clipSeg.pos, tmpvec, "clipSeg.pos");
    }
    
    /*
     * simple test of pfCylIsectSeg 
     */
    {
	pfVec3 tmpvec;
	pt1.set( 0.0f, -2.0f, 0.0f);
	pt2.set( 0.0f, 2.0f, 0.0f);
	
	seg1.makePts(pt1, pt2);
	
	cyl1.axis.set(1.0f, 0.0f, 0.0f);
	cyl1.center.set(0.0f, 0.0f, 0.0f);
	cyl1.halfLength = 1.0f;
	cyl1.radius = 0.5f;
	
	isect = cyl1.isect(&seg1, &t1, &t2);
	
	clipSeg.clip(&seg1, t1, t2);
	if (verbose)
	{
	    printf("Segment against cylinder #2.\n");
	    PrintSeg("oseg:\t", &seg1);
	    PrintCyl("cyl:\t", &cyl1);
	    printf("isect: 0x%x, t1: %f, t2: %f\n", isect, t1, t2);
	    PrintSeg("cseg:\t", &clipSeg);
	}
	AssertFloatEq(clipSeg.length, 1.0f, "clipSeg.length");
	tmpvec.set( 0.0f, 1.0f, 0.0f);
	AssertVec3Eq(clipSeg.dir, tmpvec, "clipSeg.dir");
	tmpvec.set( 0.0f, -0.5f, 0.0f);
	AssertVec3Eq(clipSeg.pos, tmpvec, "clipSeg.pos");
    }
    
    /*
     * another simple test of pfCylIsectSeg 
     */
    {
	pfVec3 tmpvec;
	pt1.set( -2.0f, -1.0f, -2.0f);
	pt2.set( 2.0f,  3.0f, 2.0f);
	
	seg1.makePts(pt1, pt2);
	
	cyl1.axis.set(1.0f, 0.0f, 0.0f);
	cyl1.center.set(0.0f, 1.0f, 0.0f);
	cyl1.halfLength = 2.0f;
	cyl1.radius = 1.0f;
	
	isect = cyl1.isect(&seg1, &t1, &t2);
	
	clipSeg.clip(&seg1, t1, t2);
	if (verbose)
	{
	    printf("Segment against cylinder #2.\n");
	    PrintSeg("oseg:\t", &seg1);
	    PrintCyl("cyl:\t", &cyl1);
	    printf("isect: 0x%x, t1: %f, t2: %f\n", isect, t1, t2);
	    PrintSeg("cseg:\t", &clipSeg);
	}
	tmpvec.set( 0.0f, 1.0f, 0.0f);
	AssertVec3Eq(clipSeg.dir, seg1.dir, "clipSeg.dir");
	tmpvec.set( 
		  -0.5f*pfSqrt(2.0f), 
		  -0.5f*pfSqrt(2.0f) + 1.0f,
		  -0.5f*pfSqrt(2.0f));
	AssertVec3Eq(clipSeg.pos, tmpvec, "clipSeg.pos");
    }
    
    /*
     * simple test of pfTriIsectSeg 
     */
    {
	pfVec3 tr1, tr2, tr3;
	pfSeg seg;
	float d = 0.0f;
	long i;
	
	for (i = 0 ; i < 30 ; i++)
	{
	    float alpha = 2.0f * drand48() - 0.5f;
	    float beta = 2.0f * drand48() - 0.5f;
	    float lscale = 2.0f * drand48();
	    float target;
	    long shouldisect;
	    
	    MakeRandomVec3(tr1);
	    MakeRandomVec3(tr2);
	    MakeRandomVec3(tr3);
	    MakeRandomVec3(pt1);
	    pt2.combine(alpha, tr2, beta, tr3);
	    pt2.combine(1.0f, pt2, 1.0f - alpha - beta, tr1);
	    
	    seg.makePts(pt1, pt2);
	    target = seg.length;
	    seg.length = lscale * seg.length;
	    
	    isect = pfTriIsectSeg(tr1, tr2, tr3, &seg, &d);
	    shouldisect = (alpha >= 0.0f && 
			   beta >= 0.0f &&
			   alpha + beta <= 1.0f &&
			   lscale >= 1.0f);
	    
	    if (verbose)
	    {
		pfVec3 tmp1, tmp2;
		if (shouldisect)
		    printf("should ISECT\n");
		else
		    printf("should NOT isect\n");
		PrintVec3("\ttr1:", tr1);
		PrintVec3("\ttr2:", tr2);
		PrintVec3("\ttr3:", tr3);
		PrintSeg("seg:", &seg);
		printf("ab: (%f %f) s: %f targ: %f\n",
		       alpha, beta, lscale, target);
		
		PrintVec3("ptintri:", pt2);
		tmp1.combine(1.0f, seg.pos, target, seg.dir);
		PrintVec3("ptonseg:", tmp1);
	    }
	    
	    if (shouldisect)
	    {
		if (!isect)
		    fprintf(stderr, 
			    "bad triangle miss ab:(%f %f) s:%f d:%f targ: %f\n",
			    alpha, beta, lscale, d, target);
		AssertFloatEq(d, target, "isect at wrong distance");
	    }
	    else
	    {
		if (isect)
		    fprintf(stderr, 
			    "bad triangle isect ab:(%f %f) s:%f d:%f targ: %f\n",
			    alpha, beta, lscale, d, target);
	    }
	    
	}
    }
    
    /* 
     * simple test of pfCylContainsPt 
     */
    {
	pfCylinder cyl;
	pfVec3 pt;
	pfVec3 perp;
	
	cyl.center.set(1.0f, 10.0f, 5.0f);
	cyl.axis.set(0.0f, 0.0f, 1.0f);
	perp.set( 1.0f, 0.0f, 0.0f);
	cyl.halfLength = 2.0f;
	cyl.radius = 0.5f;
	
	pt = cyl.center;
	if (!cyl.contains(pt))
	    fprintf(stderr, "center of cylinder not in cylinder!!!!\n");
	
	pt.addScaled(cyl.center, 0.9f*cyl.halfLength, cyl.axis);
	if (!cyl.contains(pt))
	    fprintf(stderr, "0.9*halfLength not in cylinder!!!!\n");
	
	pt.addScaled(cyl.center, -0.9f*cyl.halfLength, cyl.axis);
	if (!cyl.contains(pt))
	    fprintf(stderr, "-0.9*halfLength not in cylinder!!!!\n");
	
	pt.addScaled(cyl.center, -0.9f*cyl.halfLength, cyl.axis);
	pt.addScaled(pt, 0.9f*cyl.radius, perp);
	if (!cyl.contains(pt))
	    fprintf(stderr, "-0.9*halfLength not in cylinder!!!!\n");
	
	pt.addScaled(cyl.center, 0.9f*cyl.halfLength, cyl.axis);
	pt.addScaled(pt, -0.9f*cyl.radius, perp);
	if (!cyl.contains(pt))
	    fprintf(stderr, "-0.9*halfLength not in cylinder!!!!\n");
	
	pt.addScaled(cyl.center, 1.1f*cyl.halfLength, cyl.axis);
	if (cyl.contains(pt))
	    fprintf(stderr, "1.1*halfLength in cylinder!!!!\n");
	
	pt.addScaled(cyl.center, -1.1f*cyl.halfLength, cyl.axis);
	if (cyl.contains(pt))
	    fprintf(stderr, "-1.1*halfLength in cylinder!!!!\n");
	
	pt.addScaled(cyl.center, -0.9f*cyl.halfLength, cyl.axis);
	pt.addScaled(pt, 1.1f*cyl.radius, perp);
	if (cyl.contains(pt))
	    fprintf(stderr, "1.1*radius in cylinder!!!!\n");
	
	pt.addScaled(cyl.center, 0.9f*cyl.halfLength, cyl.axis);
	pt.addScaled(pt, -1.1f*cyl.radius, perp);
	if (cyl.contains(pt))
	    fprintf(stderr, "1.1*radius in cylinder!!!!\n");
	
    }
    return 0;
}

