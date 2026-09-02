/*
 * Copyright 1996, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
*/

/*
 * pfsphere.c
 *
 * Pseudo-loader that creates a scene graph consisting of a front-of-
 * sphere section with an approximately constant number of triangles,
 * that morphs to be good for the current
 * distance-to-ground (doesn't bother about back-facing stuff),
 * and doesn't have any triangles spanning the international date line.
 *
 * Usage:
 *	perfly <n>.sphere #tesselate starting with an n-gon (must be power of 2)
 *	perfly sphere	  #default 128
 *
 * With either of the above methods, the update can be made to
 * happen every n frames (so that once the closest point is highlighted,
 * the scene may be rotated and examined for a period of time before it is
 * updated again).  For example, to update every 120 frames,
 *	setenv PFSPHERE_UPDATE_FREQUENCY 120
 */

/*
METHOD...
1.=========================================================================

Approximating a unit circle with a polygon,
the max error will be in the middle of an edge,
and the error will be 1-cos(pi/n).
So for a given error tolerance E, n must satisfy
	1-cos(pi/n) <= E
	cos(pi/n) >= 1-E
	pi/n <= acos(1-E)
	n >= pi / acos(1-E)
So if the sphere has a radius of 1024 screen pixels (diameter 2048),
and we want:
< 1 pixel error:
	n = pi/acos(1-(1/1024))
	  = 71.08
< .5 pixel error:
	n = pi/acos(1-(.5/1024))
	  = 100.52
n=128 should be more than enough for all practical purposes.
(Definitely more than 64 is needed to avoid wavering around the edges]
when a screen-sized sphere is rotated).


Now we will approximate the visible part of the sphere (assuming unlimited
view frustum) by a bounding circle of size n (=128)
and concentric enclosed sub-polygons of decreasing radius.
The polygons have size:
		n
		n/2
		...
		128
		64
		32
		16
		8
		4
and there are log2(n)-1 (=6) of them.
Each one will be joined to the previous one by a ring of triangles
in the natural way.

Now the question is, what should the relative sizes of the rings
be?  For a small line segment roaming around on the sphere on a
line through sphere-front-point,
the maximum projection error (as a function of position on the sphere and the
segment length) is proportional to
the sine of the angle between the surface-to-eye ray and the surface normal
times 1-cos(.5 * segment length).
Call this error function Err(theta,segment length)
		= sin(theta) * (1-cos(.5 * seg length)).

(XXX Need to do the same thing with segments perpendicular to it).

For a given number of rings (=6), if we want to minimize the maximum
error, we should probably try to make the error equal everywhere
(XXX by minimizing the maximum of the above two functions).
For now we will just use the one function.

When the eye is very close to the sphere, intuitively the radiuses should
be uniformly decreasing.  So we will consider the most
extreme opposite case instead, namely when the eye is an infinite
distance away, i.e. orthogonal projection (which should simplify matters).
Calling the horizon of the sphere at angle 90 and the near sphere point 
at angle 0, we want a function F from the domain 0..6 
to the range 0..90 (0 is near point, 90 is horizon)
so that Err(F(x),dF(x))/dx is constant.
I.e.
	sin(F(x)) * (1-cos(.5 * dF(x))) / dx is constant
Hmm, that looks hard.
Okay, let's look at it a different way...
Starting at 90, the equator (circumference), given a maximum error E,
start to draw successive segments towards the near point 0 (north pole)
such that the max error of each segment remains constant.
The lengths of the segments at each theta
should be such that
	E = sin(theta) * (1-cos(.5 * len))
	E/sin(theta) = 1-cos(.5*len)
	cos(.5*len) = 1-E/sin(theta)
	.5*len = acos(1-E/sin(theta))
	len = 2*acos(1-E/sin(theta)).
Blah. Start at the first line again:
	E = sin(theta) * (1-cos(.5 * len))
the function 1-cos(.5*len) is zero at zero
and its derivative is .5*sin(.5*len);
since len is small, this is approximately len/4
and so 1-cos(.5*len) is approximately len*len/8 (note that this
gives an easier way to get the value of n derived at the top:
n = pi/sqrt(2*E))
so we have
	E = sin(theta) * len*len/8
	len = sqrt(8*E/sin(theta)).
Cool! The derivative of F should be proportional to this.  The 8*E
is irrelevant, so len should be proportional to sqrt(sin(theta)).
Len is really the incremental change in theta,
so dF(x)/dx = C*sqrt(sin(F(x)))
Bleah, let's look for the inverse function x = G(y) instead.
	dy/dx = C*sqrt(sin(y))
being cavalier about changing C to a different constant,
	dx/dy = C/sqrt(sin(y))
	G(y) = C * integral(1/sqrt(sin(y)))
Argh, don't tell me I have a friggin elliptic integral...
Screw it, just approximate the function by
	i / (log2(n)-1) * pi/2,
i.e. evenly space the rings along lines of latitude.  How crude.

2.=========================================================================
    Need the north pole to be represented explicitly.  So given the
    nice triangular mesh described in the previous section, we need
    to add one vertex inside one of the triangles, and
    integrate the new vertex into the mesh cleanly, and *continuously*,
    i.e. as the pole moves with respect to the eye, moving
    from one triangle to another, the resulting mesh should not pop.
    The algorithm to do this is essentially:
	split the "closest" edge into three parts by adding two more vertices
	connect the pole to the five adjacent vertices
	    (an alternate triangulation is possible, too)
	connect the two new vertices to the far corner of the other triangle.
    Overall effect:
	vertices: +3 (including the pole)
	edges: -1 +10
	faces: -2 +8
    Argh! Need edges along the international date line, too.
3.==========================================================================
    Okay, starting with what I had from 1. above,
    for each line that crosses the international date line
    (i.e. whose vertices lie on opposite sides of it)
    split the edge at the international date line,
    join the poles to all three verts of the respective triangles
    they landed in, and triangulate all resulting quads
    in a uniform way (pick a handedness).
    Argh!  Don't see any way to avoid popping at the poles,
    so just extend the international date line into a great circle.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>


#define FOR(i,n) for ((i) = 0; (i) < (n); ++(i))
#define LERP(a,b,t) ((a) + (t)*((b)-(a)))
#define is_power_of_2(x) (((x)&((x)-1))==0)
static int intlog2(int x) {return x == 1 ? 0 : 1+intlog2(x/2); }

struct sphereData
{
    int frequency; /* update every this many frames */

    int n; /* size of the polygon that generated it all */
    pfVec2 lonlat[2], st[2]; // tie points for texture mapping

    int max_nsimpletris, max_nsimpleverts;
    int max_nverts, max_ntris;

    pfVec3 *vc;
    pfVec2 *tc;
    ushort (*simpletris)[3];
    ushort (*tris)[3];
    ushort (*ttris)[3]; // tris for texture coords, some separated
    pfGeoSet *geoset;
};

static pfGeoSet *makeSphere(int n, struct sphereData *data)
{
    void *arena = pfGetSharedArena();

    int maxsimpleverts = 2*n - 3;
    int maxsimpletris = 3*n - 8;
    int maxverts = maxsimpleverts + maxsimpletris*3; // be safe and assume each edge gets split in two
    int maxtris = maxsimpletris * 3; // be safe and assume each triangle gets split in thirds

    pfGeoSet *geoset = new pfGeoSet();
    pfVec3 *vc = (pfVec3 *)pfMalloc(maxverts * sizeof(*vc), arena);
    pfVec2 *tc = (pfVec2 *)pfMalloc(maxverts * sizeof(*tc), arena);
    ushort (*simpletris)[3] = (ushort (*)[3])pfMalloc(maxsimpletris * sizeof(*simpletris), arena);
    ushort (*tris)[3] = (ushort (*)[3])pfMalloc(maxtris * sizeof(*tris), arena);
    ushort (*ttris)[3] = (ushort (*)[3])pfMalloc(maxtris * sizeof(*ttris), arena);
    assert(geoset && vc && tc && tris && ttris);

    geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, vc, &tris[0][0]);
    geoset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX, vc, &tris[0][0]); /* sic */
    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tc, &ttris[0][0]);
    geoset->setPrimType(PFGS_TRIS);
    geoset->setNumPrims(0);

    /*
     * Don't need to initialize the vertices yet...
     * that's done in the app callback.
     */

    /*
     * Set the triangles
     */

    assert(is_power_of_2(n));
    int nstrips = intlog2(n) - 1;

    int stripi;
    int trii = 0;
#define ADDTRI(a,b,c) \
	(simpletris[trii][0]=(a),simpletris[trii][1]=(b),simpletris[trii][2]=(c),trii++)
    ADDTRI(0,1,2);
    ADDTRI(0,2,3);
    ADDTRI(0,3,4);
    ADDTRI(0,4,1);
    for (stripi = 1; stripi < nstrips; ++stripi)
    {
	while (trii < 3*(1<<(stripi+2)) - 11)
	{
	    ADDTRI((trii-1)/3, (2*trii+7)/3, (2*trii+10)/3);
	    ADDTRI((trii-2)/3, (2*trii+8)/3, (trii+1)/3);
	    ADDTRI((trii-0)/3, (2*trii+6)/3, (2*trii+9)/3);
	}
	ADDTRI((trii-1)/3, (2*trii+7)/3, (2*trii+10)/3);
	ADDTRI((trii-2)/3, (2*trii+8)/3, (trii+1)/3 - (1<<(stripi+1)));
	ADDTRI((trii-0)/3 - (1<<(stripi+1)), (2*trii+6)/3, (2*trii+9)/3-(1<<(stripi+2)));
    }
    assert(trii == 3*n - 8);

#if 0
    /*
     * Print the triangles out as .im
     */
    printf("%s",
	   "new root Root\n"
	   "end_root\n"
	   "new geode Geode\n"
	   "end\n"
	   "attach Root Geode\n"
	   "load_geode Geode\n"
	  );

    FOR (trii, ntris)
    {
	printf("  poly 3\n");
	printf("#   %d %d %d\n",
		simpletris[trii][0],
		simpletris[trii][1],
		simpletris[trii][2]);
	printf("    vc %.9g %.9g %.9g  %.9g %.9g %.9g  %.9g %.9g %.9g\n",
	        vc[simpletris[trii][0]][0],
	        vc[simpletris[trii][0]][1],
	        vc[simpletris[trii][0]][2],
	        vc[simpletris[trii][1]][0],
	        vc[simpletris[trii][1]][1],
	        vc[simpletris[trii][1]][2],
	        vc[simpletris[trii][2]][0],
	        vc[simpletris[trii][2]][1],
	        vc[simpletris[trii][2]][2]);
	printf("    nc %.9g %.9g %.9g  %.9g %.9g %.9g  %.9g %.9g %.9g\n",
	        vc[simpletris[trii][0]][0],
	        vc[simpletris[trii][0]][1],
	        vc[simpletris[trii][0]][2],
	        vc[simpletris[trii][1]][0],
	        vc[simpletris[trii][1]][1],
	        vc[simpletris[trii][1]][2],
	        vc[simpletris[trii][2]][0],
	        vc[simpletris[trii][2]][1],
	        vc[simpletris[trii][2]][2]);
	printf("  end\n");
    }
#endif

    data->n = n;
    data->max_nsimpleverts = maxsimpleverts;
    data->max_nsimpletris = maxsimpletris;
    data->max_nverts = maxverts;
    data->max_ntris = maxtris;
    data->vc = vc;
    data->tc = tc;
    data->simpletris = simpletris;
    data->tris = tris;
    data->ttris = ttris;
    return geoset;
}

static int
updateSimpleSphere(pfTraverser *trav, void *_data)
{
    struct sphereData *data = (struct sphereData *)_data;

    int n = data->n;
    pfVec3 *vc = data->vc;

    pfVec3 eye(0,0,0);
    pfMatrix viewmat, travmat;
    trav->getChan()->getViewMat(viewmat);
    trav->getMat(travmat);
    pfMatrix invtravmat;
    invtravmat.invertOrtho(travmat);
    pfMatrix eyespace_to_objspace_mat = viewmat;
    eyespace_to_objspace_mat.postMult(invtravmat);
    eye.xformPt(eye, eyespace_to_objspace_mat);

    double eyez = PFLENGTH_VEC3(eye); /* in object space */
    double lowestlat = asin(1./eyez);

    assert(is_power_of_2(n));
    int nstrips = intlog2(n) - 1;

    int verti = 0;
    vc[verti][0] = 0;
    vc[verti][1] = -1;
    vc[verti][2] = 0;
    verti++;
    int stripi;
    FOR (stripi, nstrips)
    {
	double lat = LERP(M_PI/2, lowestlat, (double)(stripi+1)/nstrips);
	double sinlat = sin(lat);
	double coslat = cos(lat);
	int polysize = 1<<(stripi+2); // 4, 8, etc.
	int j;
	FOR (j, polysize)
	{
	    double lon = 2*M_PI * (double)j/polysize;
	    // XXX only a couple of sin/cos calculations should be necessary,
	    // XXX if we are clever...
	    // XXX Exactitude is important... probably should
	    // XXX be using doubles for everything instead of floats.
	    vc[verti][0] = cos(lon) * coslat;
	    vc[verti][1] = -sinlat;
	    vc[verti][2] = sin(lon) * coslat;
	    /* printf("%d: %.9g %.9g %.9g\n", verti, vc[verti][0],vc[verti][1],vc[verti][2]); */
	    verti++;
	}
    }
    assert(verti == 2*n-3);

    /*
     * Put it back into object space from the strange space
     * we were in...
     * Move it forward by the distance from the eye to object center.
     * Then rotate it by the rotation that takes the +y axis
     * to the object center in eye space.
     * Then translate the result back into object coords.
     *
     */
    pfMatrix invviewmat;
    invviewmat.invertOrtho(viewmat);
    pfMatrix objspace_to_eyespace_mat = travmat;
    objspace_to_eyespace_mat.postMult(invviewmat);
    pfVec3 towards_object_center_in_eye_space(0,0,0);
    towards_object_center_in_eye_space.xformPt(
	towards_object_center_in_eye_space,
        objspace_to_eyespace_mat);
    towards_object_center_in_eye_space.normalize();
    pfMatrix mat;
    pfVec3 yaxis(0,1,0);
    mat.makeVecRotVec(yaxis, towards_object_center_in_eye_space);
    mat.postMult(eyespace_to_objspace_mat);

    FOR (verti, 2*n-3)
	vc[verti].xformVec(vc[verti], mat); // so it ignores the translation

    return PFTRAV_CONT;
}

/*
 * post-app callback to adjust the sphere for the current position
 */
static int
updateSphere(pfTraverser *trav, void *_data)
{
    struct sphereData *data = (struct sphereData *)_data;

    int framecount = pfGetFrameCount();
    if (framecount % data->frequency != 0
     && framecount != 1) /* wacky on first frame */
	return PFTRAV_CONT;

    (void)updateSimpleSphere(trav, _data);

    int n = data->n;
    int nsimpleverts = 2*n-3;
    int nsimpletris = 3*n-8;
    pfVec3 *vc = data->vc;
    pfVec2 *tc = data->tc;
    ushort (*tris)[3] = data->tris;
    ushort (*ttris)[3] = data->ttris;
    ushort (*simpletris)[3] = data->simpletris;
    memcpy(tris, simpletris, nsimpletris * sizeof(*tris));

    int ntris = nsimpletris;   // gets incremented as we add more
    int nverts = nsimpleverts; // gets incremented as we add more

    int trii;
    FOR (trii, nsimpletris)
    {
	int tri[3];
	PFCOPY_VEC3(tri, tris[trii]);
	/*
	 * which longitudinal hemisphere a point is in
	 * is determined by the sign of its x component...
	 */
	float x[3]; // x component of each triangle vertex
	int i;
	FOR (i,3)
	    x[i] = vc[tri[i]][0];

	int edge_gets_cut[3];
	pfVec3 cutpoints[3];
	FOR (i,3)
	{
	    if (x[i]*x[(i+1)%3] < 0)
	    {
		/*
		 * The two vertices are on opposite sides
		 * of the plane.
		 */

		/*
		 * To make sure the results come out the same
		 * for both tris that include this edge,
		 * do the arithmetic in a canonical way...
		 */
		if (x[i] < 0)
		{
		    float t = (0-x[i])/(x[(i+1)%3]-x[i]);
		    cutpoints[i].combine(1-t, vc[tri[i]],
				            t, vc[tri[(i+1)%3]]);
		}
		else
		{
		    float t = (0-x[(i+1)%3])/(x[i]-x[(i+1)%3]);
		    cutpoints[i].combine(1-t, vc[tri[(i+1)%3]],
				           t, vc[tri[i]]);
		}
		edge_gets_cut[i] = 1;
	    }
	    else
		edge_gets_cut[i] = 0;
	}
	switch (edge_gets_cut[0] + edge_gets_cut[1] + edge_gets_cut[2])
	{
	    case 2:
	    {
		int uncut_edge = !edge_gets_cut[0] ? 0 :
				 !edge_gets_cut[1] ? 1 : 2;
		#define EDGE_OPPOSITE_VERT(i) (((i)+1)%3)
		#define VERT_OPPOSITE_EDGE(i) (((i)+2)%3)
		#define VERT_STARTING_EDGE(i) (i)
		#define VERT_ENDING_EDGE(i) (((i)+1)%3)
		/*
		 * Add two vertices...
		 */
		vc[nverts] = cutpoints[(uncut_edge+2)%3];
		vc[nverts++].normalize();
		vc[nverts] = cutpoints[(uncut_edge+1)%3];
		vc[nverts++].normalize();
		/*
		 * Replace the one triangle
		 * and add two more at the end.
		 */
		PFSET_VEC3(tris[trii], /* replace */
			   nverts-2,
			   nverts-1,
			   tri[VERT_OPPOSITE_EDGE(uncut_edge)]);
		PFSET_VEC3(tris[ntris],
			   nverts-2, 
			   tri[VERT_STARTING_EDGE(uncut_edge)],
			   tri[VERT_ENDING_EDGE(uncut_edge)]);
		ntris++;
		PFSET_VEC3(tris[ntris],
			   tri[VERT_ENDING_EDGE(uncut_edge)],
			   nverts-1, 
			   nverts-2);
		ntris++;
		break;
	    }
	    case 1:
	    {
		int cut_edge = edge_gets_cut[0] ? 0 :
			       edge_gets_cut[1] ? 1 : 2;
		/*
		 * The vertex opposite the cut edge
		 * lies on the cutting plane.
		 */

		/*
		 * Add one vertex...
		 */
		vc[nverts] = cutpoints[cut_edge];
		vc[nverts++].normalize();
		/*
		 * Replace one triangle and add another at the end.
		 */
		PFSET_VEC3(tris[trii], /* replace */
			   nverts-1,
			   tri[VERT_ENDING_EDGE(cut_edge)],
			   tri[VERT_OPPOSITE_EDGE(cut_edge)]);
		PFSET_VEC3(tris[ntris],
			   nverts-1,
			   tri[VERT_OPPOSITE_EDGE(cut_edge)],
			   tri[VERT_STARTING_EDGE(cut_edge)]);
		ntris++;
	    }
	}
    }
    assert(nverts < data->max_nverts); // XXX should check at every iteration
    assert(ntris < data->max_ntris); // XXX should check at every iteration

    /*
     * Put in texture coords.
     */
    int verti;
    pfVec2 *lonlat = data->lonlat, *st = data->st;
    double s0 = st[0][PF_S], s1 = st[1][PF_S];
    double t0 = st[0][PF_T], t1 = st[1][PF_T];
    double lon0 = lonlat[0][PF_S], lon1 = lonlat[1][PF_S];
    double lat0 = lonlat[0][PF_T], lat1 = lonlat[1][PF_T];
    double scaleS = (s1-s0)/(lon1-lon0);
    double scaleT = (t1-t0)/(lat1-lat0);
    FOR (verti, nverts)
    {
	double lat = asin(vc[verti][PF_Z]);
	double lon = atan2(vc[verti][PF_X], -vc[verti][PF_Y]);
	/* tc[verti].set(lon/(2*M_PI)+.5, lat/M_PI+.5); */
	tc[verti].set((lon-lon0)*scaleS + s0,
		      (lat-lat0)*scaleT + t0);
    } 

    /*
     * "Fix" texture coords for triangles with vertices
     * on the international date line so that they don't wrap wrongly.
     * That is, if the interior of the triangle (just test the centroid)
     * is on one side of the IDL, pull the tex coords there too.
     * This means separating the texture coords into two copies.
     */
    // XXX fix me!
    int ntverts = nverts;
    FOR (trii, ntris)
    {
	PFCOPY_VEC3(ttris[trii], tris[trii]);
	float xcentroid = vc[tris[trii][0]][PF_X]
			+ vc[tris[trii][1]][PF_X]
			+ vc[tris[trii][2]][PF_X];
	int i;
	FOR (i, 3)
	{
	    int ind = tris[trii][i];
	    if ((tc[ind][PF_X]-.5) * xcentroid < 0) /* it's on the wrong side */
	    {
		assert(ntverts <= data->max_nverts);
		tc[ntverts][0] = 1.0 - tc[ind][0];
		tc[ntverts][1] = tc[ind][1];
		tc[ntverts][2] = tc[ind][2];
		ind = ntverts++;
	    }
	    ttris[trii][i] = ind;
	}
    }
    assert(ntverts <= data->max_nverts); // XXX should check at every iteration

    /*
     * XXX is the following frame-accurate? I think not.
     */
    data->geoset->setNumPrims(ntris);

    return PFTRAV_CONT;
}

/*
 * Start with an n-sided polygon for the circumference.
 * n must be a power of 2.
 * Update geometry every "update_frequency" frames
 * (so that the scene may be rotated and examined for a period of time
 * before it is updated again).
 *
 * The texture is assumed to be such that
 * lon/lat is a straight scale&bias function of s,t;
 * the inputs lonlat and st specify the
 * lower left and upper right of some rectangle,
 * to be used as tie points.  lonlat is in degrees.
 *
 * If either or both of these are not specified (i.e. NULL),
 * the following defaults will be used:
 *	lonlat[0] = -180,-90	st[0] = 0,0
 *	lonlat[1] = 180,90	st[1] = 1,1
 */
extern /*"C"*/ PFUDLLEXPORT pfNode *
pfuNewSphereFrontOld(int n, pfTexture *texture,
		  pfVec2 lonlat[2], pfVec2 st[2],
		  int update_frequency)
{
    struct sphereData *data = (struct sphereData*)pfMalloc(
			       sizeof(struct sphereData),
			       pfGetSharedArena());
    assert(data != NULL);
    data->frequency = update_frequency;
    data->lonlat[0] = (lonlat ? lonlat[0]*(M_PI/180) : pfVec2(-M_PI,-M_PI/2));
    data->lonlat[1] = (lonlat ? lonlat[1]*(M_PI/180) : pfVec2(M_PI,M_PI/2));
    data->st[0] = (st ? st[0] : pfVec2(0,0));
    data->st[1] = (st ? st[1] : pfVec2(1,1));

    pfGeoSet *geoset = makeSphere(n, data);

    pfGeoState *geostate = new pfGeoState();
    geostate->setMode(PFSTATE_ENLIGHTING, PF_ON);
    if (texture != NULL)
    {
	geostate->setAttr(PFSTATE_TEXTURE, texture);
	geostate->setMode(PFSTATE_ENTEXTURE, PF_ON);
    }
    geoset->setGState(geostate);
    data->geoset = geoset;

    pfGeode *geode = new pfGeode();
    assert(geode != NULL);

    pfSphere bsph;
    bsph.center.set(0,0,0);
    bsph.radius = 1;
    geode->setBound(&bsph, PFBOUND_STATIC);
    geode->setTravFuncs(PFTRAV_APP, NULL, updateSphere);
    geode->setTravData(PFTRAV_APP, data);
    geode->addGSet(geoset);

    /*
     * setBound doesn't take when it's a geode, so...
     */
    pfGroup *protector = new pfGroup();
    protector->addChild(geode);
    protector->setBound(&bsph, PFBOUND_STATIC);

    return protector;
}
