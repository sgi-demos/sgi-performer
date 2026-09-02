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
 * Functions:
 *	hsv_to_rgb(h,s,v,&r,&g,&b)
 *	rgb_to_hsv(r,g,b,&h,&s,&v)
 *	cmy_to_hsv(c,m,y,&h,&s,&v)
 *	cmy_to_rgb(c,m,y,&r,&g,&b)
 *	rgb_to_cmy(r,g,b,&c,&m,&y)
 *	hsv_to_cmy(h,s,v,&c,&m,&y)
 * Author:
 *	Don Hatch (hatch@sgi.com)
 * Date:
 *	A Long Time Ago
 */

extern void
hsv_to_rgb(double h, double s, double v, double *R, double *G, double *B)
{
    int segment;
    double *rgb[3], major, minor, middle, frac;

    rgb[0] = R;
    rgb[1] = G;
    rgb[2] = B;

    while (h < 0)
	h++;
    while (h >= 1)
	h--;

    segment = (int)(h*6);

    frac = (6*h)-segment;
    if (segment % 2)
	frac = 1 - frac;

    major = v;
    minor = (1-s)*v;
    middle = frac * major + (1-frac) * minor;

    *rgb[(segment+1)/2 % 3] = major;
    *rgb[(segment+4)/2 % 3] = minor;
    *rgb[(7-segment) % 3] = middle;
}

#define swap(a,b) (temp = (a), (a) = (b), (b) = temp)
#define rshift(a,b,c) (temp = (c), (c) = (b), (b) = (a), (a) = temp)
extern void
rgb_to_hsv(double r, double g, double b, double *H, double *S, double *V)
{
    int segment;
    double *rgb[3], *temp, frac;

    rgb[0] = &r;
    rgb[1] = &g;
    rgb[2] = &b;

    /* sort rgb into increasing order */

    if (*rgb[0] > *rgb[1])
	swap(rgb[0], rgb[1]);
    if (*rgb[1] > *rgb[2])
	if (*rgb[0] > *rgb[2])
	    rshift(rgb[0], rgb[1], rgb[2]);
	else
	    swap(rgb[1], rgb[2]);

    if (*V = *rgb[2])
	if (*S = 1 - *rgb[0] / *rgb[2]) {
	    /* segment = 3*(b>g) + 2 * (rgb[1]==&b) + (rgb[1]==&r); */
	    segment = 3 * (rgb[0]==&g||rgb[2]==&b)
		    + 2 * (rgb[1]==&b) + (rgb[1]==&r);
	    frac = (*rgb[1] - *rgb[0]) / (*rgb[2] - *rgb[0]);
	    if (segment % 2)
		frac = 1 - frac;
	    *H = (segment + frac) / 6;
	    if (*H >= 1)
		--*H;
	}
}



extern void
cmy_to_rgb(double c, double m, double y, double *R, double *G, double *B)
{
    *R = 1 - c;
    *G = 1 - m;
    *B = 1 - y;
}

extern void
rgb_to_cmy(double r, double g, double b, double *C, double *M, double *Y)
{
    *C = 1 - r;
    *M = 1 - g;
    *Y = 1 - b;
}

extern void
hsv_to_cmy(double h, double s, double v, double *C, double *M, double *Y)
{
    double r,g,b;
    hsv_to_rgb(h,s,v,&r,&g,&b);
    rgb_to_cmy(r,g,b,C,M,Y);
}

extern void
cmy_to_hsv(double c, double m, double y, double *H, double *S, double *V)
{
    double r,g,b;
    cmy_to_rgb(c,m,y,&r,&g,&b);
    rgb_to_hsv(r,g,b,H,S,V);
}

/*
   rgb_to_cmyk is taken from IL source.
   I am trying to understand what it does, and where the redundancy lies,
   if any.
*/

#define LERP(v1,v2,p) (v1 * (1.0-p) + v2*p);

/*
// converts from rgb to cmyk where all values range from 0.0 to 1.0
*/
extern void
rgb_to_cmyk(double r, double g, double b, double *C, double *M, double *Y, double *K)
{

    /*
    // brightMin is the minimum brightness (or maximum "blackness") 
    // that can be achieved with just the "K" component; values blacker 
    // than this require a little extra cyan (C) and magenta (M) to be added 
    // into the color, while some of the yellow component (Y) is diluted.
    */
    const double brightMin = 4./255.;

    /*
    // the ultra value is the amount to be added at full black; between brightMin
    // and full black the ultra value is interpolated into the C and M colors.
    */
    const double ultraVal = 160./255.;

    double i = 0.;

    /*
    // compute max of r, g, b
    */
    if (i<r) i = r;
    if (i<g) i = g;
    if (i<b) i = b;

    if (i == 0.0) {
	*C = *M = ultraVal;
	*Y = 0.0;
	*K = 1.0;
	return;
    }

    *C = 1.0 - r/i;
    *M = 1.0 - g/i;
    *Y = 1.0 - b/i;
    *K = 1.0 - i;

    if (i < brightMin) {
	double bp = i/brightMin;
	*C = LERP(ultraVal, *C, bp);
	*M = LERP(ultraVal, *M, bp);
	*Y = LERP(0.0, *Y, bp);
    }
}

extern void
cmy_to_cmyk(double c, double m, double y, double *C, double *M, double *Y, double *K)
{
    double r,g,b;
    cmy_to_rgb(c,m,y,&r,&g,&b);
    rgb_to_cmyk(r,g,b,C,M,Y,K);
}

extern void
hsv_to_cmyk(double h, double s, double v, double *C, double *M, double *Y, double *K)
{
    double r,g,b;
    cmy_to_rgb(h,s,v,&r,&g,&b);
    rgb_to_cmyk(r,g,b,C,M,Y,K);
}


extern void
cmyk_to_rgb(double c, double m, double y, double k, double *R, double *G, double *B)
{
    double br = 1-k;
    *R = br*(1-c);
    *G = br*(1-m);
    *B = br*(1-y);
}

extern void
cmyk_to_cmy(double c, double m, double y, double k, double *C, double *M, double *Y)
{
    double r,g,b;
    cmyk_to_rgb(c,m,y,k,&r,&g,&b);
    rgb_to_cmy(r,g,b,C,M,Y);
}

cmyk_to_hsv(double c, double m, double y, double k, double *H, double *S, double *V)
{
    double r,g,b;
    cmyk_to_rgb(c,m,y,k,&r,&g,&b);
    rgb_to_hsv(r,g,b,H,S,V);
}
