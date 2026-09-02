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

//
// texderivs.C
//
// Contains the single function pfuCalcTexDerivs().
//
// (This is a pure mathematical function that doesn't
// use any Performer structures; pfutil.h
// is only included to ensure that the function prototype
// is the same as what the outside world sees.)
//
#include <stdio.h>
#include <stdlib.h>
#include <Performer/pfutil.h>

//
// Calculate ds/dx, dt/dx, ds/dy, dt/dy
//
// NOTE: the vertex coords xa,ya, xb,yb, xc,yc
// are assumed to have already been divided by
// wa, wb, wc respectively (i.e. they are in actual screen pixels),
// but the texture coords are not, even though the
// values used by this function are actually sa/wa, etc.
//
// XXX need a way to clamp the result so that
//     we don't suffer from instability and bogus answers
//     as the derivatives approach infinity
//
PFUDLLEXPORT
int pfuCalcTexDerivs(
    float xa, float ya, float wa, float sa, float ta, // vertex a screen coords
    float xb, float yb, float wb, float sb, float tb, // vertex b screen coords
    float xc, float yc, float wc, float sc, float tc, // vertex c screen coords
    float x, float y,	// screen coords at which to evaluate
    float *s, float *t,	// return tex coords at x,y (optional)
    float *ds_dx,	// return ds/dx
    float *dt_dx,	// return dt/dx
    float *ds_dy,	// return ds/dy
    float *dt_dy) 	// return dt/dy
{
    float _1_wa = 1/wa;
    float _1_wb = 1/wb;
    float _1_wc = 1/wc;
    float sa_wa = sa*_1_wa;
    float sb_wb = sb*_1_wb;
    float sc_wc = sc*_1_wc;
    float ta_wa = ta*_1_wa;
    float tb_wb = tb*_1_wb;
    float tc_wc = tc*_1_wc;

    //
    // Compute homogeneous (i.e. not necessarily summing to 1)
    // barycentric coords at the point x,y.
    // These are proportional to the areas
    // of the triangles formed by connecting x,y to
    // each of the three edges of the original triangle.
    //
    float A = (x-xb)*(y-yc) - (x-xc)*(y-yb);
    float B = (x-xc)*(y-ya) - (x-xa)*(y-yc);
    float C = (x-xa)*(y-yb) - (x-xb)*(y-ya);

    //
    // Numerator and denominator of s and t...
    //
    float Ns = A*sa_wa + B*sb_wb + C*sc_wc;
    float Nt = A*ta_wa + B*tb_wb + C*tc_wc;
    float D  = A*_1_wa + B*_1_wb + C*_1_wc;
    if (D == 0.) // XXX threshold?
    {
	if (s != NULL) *s = 0.;
	if (t != NULL) *t = 0.;
	if (ds_dx != NULL) *ds_dx = 0.;
	if (dt_dx != NULL) *dt_dx = 0.;
	if (ds_dy != NULL) *ds_dy = 0.;
	if (dt_dy != NULL) *dt_dy = 0.;
	return 0; // failure, s,t undefined and derivatives are inf or nan
    }
    float _1_D = 1/D;

    if (s != NULL) *s = Ns * _1_D;
    if (t != NULL) *t = Nt * _1_D;

    if (ds_dx != NULL
     || dt_dx != NULL
     || ds_dy != NULL
     || dt_dy != NULL)
    {
	//
	// s' = (Ns/D)' = (Ns'*D - Ns*D') / D^2
	// t' = (Nt/D)' = (Nt'*D - Nt*D') / D^2
	//
	float dNs_dx = (yb-yc)*sa_wa + (yc-ya)*sb_wb + (ya-yb)*sc_wc;
	float dNt_dx = (yb-yc)*ta_wa + (yc-ya)*tb_wb + (ya-yb)*tc_wc;
	float dNs_dy = (xc-xb)*sa_wa + (xa-xc)*sb_wb + (xb-xa)*sc_wc;
	float dNt_dy = (xc-xb)*ta_wa + (xa-xc)*tb_wb + (xb-xa)*tc_wc;
	float dD_dx  = (yb-yc)*_1_wa + (yc-ya)*_1_wb + (ya-yb)*_1_wc;
	float dD_dy  = (xc-xb)*_1_wa + (xa-xc)*_1_wb + (xb-xa)*_1_wc;
	float _1_DD = _1_D*_1_D;

	if (ds_dx != NULL) *ds_dx = (dNs_dx*D - Ns*dD_dx) * _1_DD;
	if (dt_dx != NULL) *dt_dx = (dNt_dx*D - Nt*dD_dx) * _1_DD;
	if (ds_dy != NULL) *ds_dy = (dNs_dy*D - Ns*dD_dy) * _1_DD;
	if (dt_dy != NULL) *dt_dy = (dNt_dy*D - Nt*dD_dy) * _1_DD;
    }
    return 1; // success, all partial derivatives are finite
}

#ifdef MAIN
main(int argc, char **argv)
{
    if (argc < 18 || argc % 2 == 1)
    {
	fprintf(stderr, "Usage: %s <xa> <ya> <wa> <sa> <ta> <xb> <yb> <wb> <sb> <tb> <xc> <yc> <wc> <sc> <tc> <x> <y> [<x> <y> [...]]\n", argv[0]);
	return 1;
    }
    float xa = atof(argv[1]);
    float ya = atof(argv[2]);
    float wa = atof(argv[3]);
    float sa = atof(argv[4]);
    float ta = atof(argv[5]);

    float xb = atof(argv[6]);
    float yb = atof(argv[7]);
    float wb = atof(argv[8]);
    float sb = atof(argv[9]);
    float tb = atof(argv[10]);

    float xc = atof(argv[11]);
    float yc = atof(argv[12]);
    float wc = atof(argv[13]);
    float sc = atof(argv[14]);
    float tc = atof(argv[15]);

    printf("a = [%.9g %.9g %.9g] [%.9g %.9g]\n", xa, ya, wa, sa, ta);
    printf("b = [%.9g %.9g %.9g] [%.9g %.9g]\n", xb, yb, wb, sb, tb);
    printf("c = [%.9g %.9g %.9g] [%.9g %.9g]\n", xc, yc, wc, sc, tc);

    char **argp = argv+16;
    while (argp[0] != NULL && argp[1] != NULL)
    {
	float x = atof(*argp++);
	float y = atof(*argp++);

	float s, t, ds_dx, dt_dx, ds_dy, dt_dy;
	pfuCalcTexDerivs(xa, ya, wa, sa, ta,
		         xb, yb, wb, sb, tb,
		         xc, yc, wc, sc, tc,
		         x, y, &s, &t,
		         &ds_dx, &dt_dx, &ds_dy, &dt_dy);
	printf("p = [%.9g %.9g] [%.9g %.9g]\n", x, y, s, t);
	printf("\tds/dx = %.9g\tdt/dx = %.9g\n", ds_dx, dt_dx);
	printf("\tds/dy = %.9g\tdt/dy = %.9g\n", ds_dy, dt_dy);
    }
    return 0;
}
#endif // MAIN
