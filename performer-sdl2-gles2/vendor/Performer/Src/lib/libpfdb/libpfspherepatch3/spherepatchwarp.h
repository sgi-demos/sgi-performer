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
    spherepatchwarp.h
 */
#ifdef WIN32
#define M_E 2.71828182845904523536
#endif

struct SpherePatchWarp
{
    double slope; // slope of linear (initial) part of function starting at x=0
    double base;  // base of exponential (final) part of function ending at x=1
    double breakpoint;  // x value where the two parts meet

    SpherePatchWarp()
    {
	// default values...
	slope = 1.;
	base = M_E;
	breakpoint = 1./log(base);
    }

    void makeFromSlope(double _slope)
    {
	slope = _slope;

	// XXX, maybe should have a special case evaluation for 0
	if (slope <= 0.)
	    slope = 1.;
	
	if (slope > 1.)
	{
	    // Use the inverse function
	    makeFromSlope(1./slope);
	    breakpoint = func(breakpoint);
	    slope = 1./slope;
	    return;
	}

	base = _calcBaseFromSlope(slope);
	breakpoint = 1./log(base);
    }

    void makeFromBase(double _base)
    {
	base = _base;
	slope = M_E*log(base)/base;
	breakpoint = 1./log(base);
    }

    void makeFromSamplePoint(double x, double y)
    {
	if (x == 0. || x == 1.)
	    return; // don't change

	if (y > x)
	{
	    // Use the inverse function
	    makeFromSamplePoint(y, x);
	    breakpoint = func(breakpoint);
	    slope = 1./slope;
	    return;
	}

	makeFromBase(pow(y, 1/(x-1.)));
	if (x < breakpoint)
	    makeFromSlope(y/x);
    }

    void makeFromBreakPoint(double _breakpoint)
    {
	assert(_breakpoint > 0.);
	breakpoint = _breakpoint;
	base = exp(1./breakpoint);
	slope = M_E*log(base)/base;
    }

    double func(double x)
    {
	if (x < 0)
	    return -func(-x);

	// XXX this comparison is giving the error in O32:
	// as1: Warning: spherepatchwarp.C, line 205: number outside range for double precision floating point values
	if (x <= breakpoint)
	    return slope*x;
	else
	{
	    if (slope > 1.) // use the inverse function
		return logbase(x, base) + 1;
	    else
		return pow(base, (x-1.));
	}
    }
    double inverseFunc(double y)
    {
	struct SpherePatchWarp inverseWarp;
	inverseWarp.breakpoint = func(breakpoint);
	inverseWarp.slope = 1./slope;
	inverseWarp.base = base;
	return inverseWarp.func(y);
    }

private:
    static double _calcBaseFromSlope(double slope)
    {
	//
	// Find base satisfying
	//      slope = M_E*log(base)/base.
	// We want the value on the part
	// of this function (base as a function of slope) that's decreasing,
	// from [base=e,slope=1] to [base=infinity,slope=0].
	// Do it by binary search.
	// (Could improve by using newton's method or something, but who cares.)
	//
	assert(INRANGE(0. <, slope, <= 1.));
	double basemin = M_E;
	double basemax = M_E*2.; // increase until it bounds
	while (M_E*log(basemax)/basemax > slope)
	    basemax *= 2.;
	while (1)
	{
	    double baseGuess = (basemin+basemax)/2.;
	    double slopeGuess = M_E*log(baseGuess)/baseGuess;
	    if (!INRANGE(basemin <, baseGuess, < basemax))
	    {
		/*
		printf("slope = %.17g -> base = %.17g -> slope = %.17g\n",
		       slope, baseGuess, slopeGuess);
		*/
		return baseGuess;
	    }
	    // remember that the function is decreasing...
	    if (slope > slopeGuess)
		basemax = baseGuess;	// move to the left
	    else
		basemin = baseGuess;	// move to the right
	}
    }
};

