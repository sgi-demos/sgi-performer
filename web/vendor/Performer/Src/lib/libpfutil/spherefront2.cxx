/*
 * Copyright 2000, Silicon Graphics, Inc.
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
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that (i) the above copyright notices and this
 * permission notice appear in all copies of the software and related
 * documentation, and (ii) the name of Silicon Graphics may not be
 * used in any advertising or publicity relating to the software
 * without the specific, prior written permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL,
 * INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY
 * THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
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


//
// Conversion between various coordinate spaces...
//	x,y,z <-> lon,lat <-> s,t
// XXX need scale and bias for general conversion between s,t and lon,lat!
// 
static inline void xyz_to_lonlat(double x, double y, double z, double *Lon, double *Lat)
{
    *Lon = atan2(x,-y); // sic
    *Lat = asin(PF_CLAMP(z,-1,1));
}
static inline void lonlat_to_xyz(double lon, double lat, double *X, double *Y, double *Z)
{
    double coslat = cos(lat);
    *X = sin(lon) * coslat;
    *Y = -cos(lon) * coslat;
    *Z = sin(lat);
}
static inline void st_to_lonlat(double s, double t, double *Lon, double *Lat)
{
    *Lon = 2*M_PI * (s-.5);
    *Lat = M_PI * (t-.5);
}
static inline void lonlat_to_st(double lon, double lat, double *S, double *T)
{
    *S = lon * (1/(2*M_PI)) + .5;
    *T = lat * (1/M_PI) + .5;
}
static inline void xyz_to_st(double x, double y, double z, double *S, double *T)
{
    double lon,lat;
    xyz_to_lonlat(x,y,z,&lon,&lat);
    lonlat_to_st(lon,lat,S,T);
}
static inline void st_to_xyz(double s, double t, double *X, double *Y, double *Z)
{
    double lon,lat;
    st_to_lonlat(s,t,&lon,&lat);
    lonlat_to_xyz(lon,lat,X,Y,Z);
}


//
// Structure definitions...
//

// possible edge status values...
#define INELIGIBLE_TO_BE_SPLIT 0
#define ELIGIBLE_TO_BE_SPLIT 1
#define HAS_BEEN_SPLIT 2
struct Edge {
    int level;
    int status;	// one of the above constants
    int from, to;	// indices into vertices
    int children[2];	// pieces at next level (self if not split)
};
struct Tri {
    int level;
    int orientation; // 1 for CCW, -1 for CW
    int sides[3]; // indices into edges
    int sidedirection[3]; // -1 or 1, always 1 for a splittable edge
    int children[4]; // pieces at next level
};

struct Mesh {
    void *arena;
    int maxntris, maxnedges, maxnverts;
    int nverts, nedges, ntris;
    double (*vc)[3];
    double (*tc)[2];
    struct Edge *edges;
    struct Tri *tris;
};

#define INIT_EDGE(_edges, _edgei, _level, _status, _from, _to) \
	((_edges)[_edgei].level = (_level), \
	 (_edges)[_edgei].status = (_status), \
	 (_edges)[_edgei].from = (_from), \
	 (_edges)[_edgei].to = (_to), \
	 (_edges)[_edgei].children[0] = (_edgei), /* important */ \
	 (_edges)[_edgei].children[1] = (_edgei))  /* important */

static struct Mesh *
allocMesh(int maxntris, int maxnedges, int maxnverts, void *arena)
{
    struct Mesh *mesh = (struct Mesh *)pfMalloc(sizeof(*mesh), arena);
    assert(mesh != NULL);

    mesh->arena = arena;
    mesh->vc = (double (*)[3])pfMalloc(maxnverts * sizeof(*mesh->vc), arena);
    mesh->tc = (double (*)[2])pfMalloc(maxnverts * sizeof(*mesh->tc), arena);
    mesh->edges = (struct Edge *)pfMalloc(maxnedges * sizeof(*mesh->edges), arena);
    mesh->tris = (struct Tri *)pfMalloc(maxntris * sizeof(*mesh->tris), arena);
    assert(mesh->vc && mesh->tc && mesh->edges && mesh->tris);
    mesh->maxnverts = maxnverts;
    mesh->maxnedges = maxnedges;
    mesh->maxntris = maxntris;
    mesh->nverts = 0;
    mesh->nedges = 0;
    mesh->ntris = 0;
    return mesh;
}

#ifdef MAIN
static void
freeMesh(struct Mesh *mesh)
{
    if (mesh != NULL)
    {
	if (mesh->vc) pfDelete(mesh->vc);
	if (mesh->tc) pfDelete(mesh->tc);
	if (mesh->edges) pfDelete(mesh->edges);
	if (mesh->tris) pfDelete(mesh->tris);

	pfDelete(mesh);
    }
}
#endif // MAIN

static inline void expandVerts(struct Mesh *mesh, int n, double (**vc)[3], double (**tc)[2])
{
    if (n > mesh->maxnverts)
    {
	mesh->maxnverts = n*3/2;
	mesh->vc = (double (*)[3])pfRealloc(mesh->vc, mesh->maxnverts * sizeof(*mesh->vc));
	mesh->tc = (double (*)[2])pfRealloc(mesh->tc, mesh->maxnverts * sizeof(*mesh->tc));
	assert(mesh->vc != NULL && mesh->tc != NULL);
	fprintf(stderr, "Expanding to %d verts\n", mesh->maxnverts);
	*vc = mesh->vc;
	*tc = mesh->tc;
    }
}
static inline void expandEdges(struct Mesh *mesh, int n, struct Edge **edges)
{
    if (n > mesh->maxnedges)
    {
	mesh->maxnedges = n*3/2;
	mesh->edges = (struct Edge *)pfRealloc(mesh->edges, mesh->maxnedges * sizeof(*mesh->edges));
	assert(mesh->edges != NULL);
	fprintf(stderr, "Expanding to %d edges\n", mesh->maxnedges);
	*edges = mesh->edges;
    }
}
static inline void expandTris(struct Mesh *mesh, int n, struct Tri **tris)
{
    if (n > mesh->maxntris)
    {
	mesh->maxntris = n*3/2;
	mesh->tris = (struct Tri *)pfRealloc(mesh->tris, mesh->maxntris * sizeof(*mesh->tris));
	assert(mesh->tris != NULL);
	fprintf(stderr, "Expanding to %d tris\n", mesh->maxntris);
	*tris = mesh->tris;
    }
}

//
// Initialization function...
//
#define NORTHPOLE 0
#define GREENWICH 1
#define EASTPOLE 2
#define IDL 3
#define WESTPOLE 4
#define SOUTHPOLE 5
double octa_wrap_verts[8][3] = {
    /* NORTHPOLE */ {0,0,1},
    /* GREENWICH */ {0,-1,0},
    /* EASTPOLE */  {1,0,0},
    /* IDL */       {0,1,0},
    /* WESTPOLE */  {-1,0,0},
    /* SOUTHPOLE */ {0,0,-1},
};
// Adjacent tris must have opposite orientations (so icosa doesn't work)
// fourth coord is the orientation
static int octa_wrap_faces[8][4] = {
    {GREENWICH, NORTHPOLE, WESTPOLE, 1},
    {GREENWICH, NORTHPOLE, EASTPOLE, -1},
    {GREENWICH, SOUTHPOLE, WESTPOLE, -1},
    {GREENWICH, SOUTHPOLE, EASTPOLE, 1},
    {IDL, NORTHPOLE, WESTPOLE, -1},
    {IDL, NORTHPOLE, EASTPOLE, 1},
    {IDL, SOUTHPOLE, WESTPOLE, 1},
    {IDL, SOUTHPOLE, EASTPOLE, -1},
};

static void
meshOcta(struct Mesh *mesh)
{
    int nverts = mesh->nverts = 6;
    int ntris = mesh->ntris = 8;
    int nedges = mesh->nedges = 12;
    double (*vc)[3] = mesh->vc;
    double (*tc)[2] = mesh->tc;
    struct Edge *edges = mesh->edges;
    struct Tri *tris = mesh->tris;
    int verti, trii;

    expandTris(mesh, 8, &tris);
    expandEdges(mesh, 12, &edges);
    expandVerts(mesh, 6, &vc, &tc);

    FOR (verti, nverts)
    {
	PFCOPY_VEC3(vc[verti], octa_wrap_verts[verti]);
	xyz_to_st(vc[verti][0], vc[verti][1], vc[verti][2],
		  &tc[verti][0], &tc[verti][1]);
    }
    nedges = 0;
    FOR (trii, ntris)
    {
	tris[trii].level = 0;
	tris[trii].orientation = octa_wrap_faces[trii][3];
	PFSET_VEC4(tris[trii].children, -1,-1,-1,-1);

	int sidei;
	FOR (sidei, 3)
	{
	    //
	    // Attempt to find the side in the list.
	    //
	    int edgei;
	    FOR (edgei, nedges)
	    {
		if (edges[edgei].from == octa_wrap_faces[trii][sidei]
		 && edges[edgei].to == octa_wrap_faces[trii][(sidei+1)%3])
		    break;
	    }
	    if (edgei == nedges) // didn't find it
	    {
		INIT_EDGE(edges, nedges, 0, ELIGIBLE_TO_BE_SPLIT,
			  octa_wrap_faces[trii][sidei],
			  octa_wrap_faces[trii][(sidei+1)%3]);
		nedges++;
	    }
	    tris[trii].sides[sidei] = edgei;
	    tris[trii].sidedirection[sidei] = 1;
	}
    }
    assert(nedges == 12);
}

static inline double lerp(double a, double b, double t)
{
    return a + t*(b-a);
}
/*
 * Linear interpolation modulo 1; always take the closest answer
 */
static inline double lerpwrap(double a, double b, double t)
{
    if (PF_ABS(a-b) > .5)
    {
	if (a > b)
	    if (1-a < b-0)
		a -= 1;
	    else
		b += 1;
	else /* b > a */
	    if (1-b < a-0)
		b -= 1;
	    else
		a += 1;
    }
    return lerp(a,b,t);
}
static inline void lerp_st(double result[2], const double from[2],
					        const double to[2], double t)
{
    result[PF_S] = lerpwrap(from[PF_T]==0||from[PF_T]==1 ? to[PF_S]
							 : from[PF_S],
			    to[PF_T]==0||to[PF_T]==1 ? from[PF_S]
						     : to[PF_S], t);
    result[PF_T] = lerp(from[PF_T], to[PF_T], t);
}

//
// Note, I put some thought into which
// should be floats and which doubles...
// don't go changing them indiscriminately
//
static void
meshSubdivide(struct Mesh *mesh,
	      double viewDir[3],
	      double viewRight[3],
	      double viewUp[3],
	      double viewOffset,
	      float lowtol, float hightol)
{
    int nverts = mesh->nverts;
    int ntris = mesh->ntris;
    int nedges = mesh->nedges;
    double (*vc)[3] = mesh->vc;
    double (*tc)[2] = mesh->tc;
    struct Edge *edges = mesh->edges;
    struct Tri *tris = mesh->tris;

    int edgei = 0;
    int trii = 0;
    int leveli;
    for (leveli = 0; edgei < nedges; leveli++) /* sic */
    {
	//
	// For each edge at this level,
	// if it's eligible for splitting
	// and the error at the edge center is > lowtol,
	// split the edge into two pieces at leveli+1.
	//
	for (;edgei < nedges && edges[edgei].level == leveli; edgei++)
	{
	    struct Edge *edge = &edges[edgei];
	    if (edge->status == ELIGIBLE_TO_BE_SPLIT)
	    {
		// XXX if we ever reuse the information,
		// XXX should store this info so it doesn't need to
		// XXX be recalculated
		// XXX also, can interpolate in latlon space
		// XXX and not waste the conversion time between
		// XXX lon,lat and s,t
		double initial[3], final[3]; // morph points
		double finaltc[2];
		PFCOMBINE_VEC3(initial, .5, vc[edge->from], .5, vc[edge->to]);
		lerp_st(finaltc, tc[edge->from], tc[edge->to], .5);

		st_to_xyz(finaltc[0],finaltc[1], &final[0],&final[1],&final[2]);

		float initialdivide = PFDOT_VEC3(initial, viewDir)-viewOffset;
		float finaldivide = PFDOT_VEC3(final, viewDir)-viewOffset;

		double projecteddelta[2];
		projecteddelta[0] = PFDOT_VEC3(final, viewRight)/finaldivide
				  - PFDOT_VEC3(initial, viewRight)/initialdivide;
		projecteddelta[1] = PFDOT_VEC3(final, viewUp)/finaldivide
				  - PFDOT_VEC3(initial, viewUp)/initialdivide;

		float errorsqrd = PFDOT_VEC2(projecteddelta, projecteddelta);

		//
		// Based on the error,
		// decide whether to split and,
		// if so, how much to morph.
		//
		if (errorsqrd < lowtol*lowtol)
		{
		    /* fprintf(stderr, "Edge %d (level %d) doesn't need splitting: error = %g (%g %g)\n", edgei, leveli, sqrt(errorsqrd), projecteddelta[0], projecteddelta[1]); */
		    edge->status = INELIGIBLE_TO_BE_SPLIT; // necessary
		    continue;	// edge doesn't need splitting
		}
		/* fprintf(stderr, "Edge %d (level %d) need splitting: error = %g (%g %g)\n", edgei, leveli, sqrt(errorsqrd), projecteddelta[0], projecteddelta[1]); */

		expandVerts(mesh, nverts+1, &vc, &tc);
		assert(nverts+1 <= mesh->maxnverts);
		double t;
		if (errorsqrd > hightol*hightol)
		{
		    t = 1.;
		    PFCOPY_VEC3(vc[nverts], final);
		    PFCOPY_VEC2(tc[nverts], finaltc);
		}
		else
		{
		    float error = pfSqrt(errorsqrd);
		    t = (error-lowtol)/(hightol-lowtol);
		    PFCOMBINE_VEC3(vc[nverts], (1-t), initial, t, final);
		    lerp_st(tc[nverts], initial, final, t);
		}
		int newverti = nverts++;

		expandEdges(mesh, nedges+2, &edges);
		assert(nedges+2 <= mesh->maxnedges);
		edge = &edges[edgei];

		edge->status = HAS_BEEN_SPLIT;
		edge->children[0] = nedges+0;
		edge->children[1] = nedges+1;
			  
		INIT_EDGE(edges, nedges+0, leveli+1,
			  t==1. ? ELIGIBLE_TO_BE_SPLIT : INELIGIBLE_TO_BE_SPLIT,
			  edge->from, newverti);
		INIT_EDGE(edges, nedges+1, leveli+1,
			  t==1. ? ELIGIBLE_TO_BE_SPLIT : INELIGIBLE_TO_BE_SPLIT,
			  newverti, edge->to);

		nedges += 2;
	    }
	}

	//
	// For each triangle at this level,
	// if any of its edges got split,
	// split the triangle into the appropriate number of sub-triangles.
	//
	for (; trii < ntris && tris[trii].level == leveli; trii++)
	{
	    struct Tri *tri = &tris[trii];

#if 0 // XXX get rid of this when it works
	    {
		int nsplit = (edges[tri->sides[0]].status == HAS_BEEN_SPLIT)
			   + (edges[tri->sides[1]].status == HAS_BEEN_SPLIT)
			   + (edges[tri->sides[2]].status == HAS_BEEN_SPLIT);
		if (nsplit > 0)
		    fprintf(stderr, "Splitting tri %d into %d parts\n", trii, nsplit+1);
	    }
#endif

	    int something_split = 0;
	    int sidei;
	    FOR (sidei, 3)
	    {
		if (edges[tri->sides[sidei]].status == HAS_BEEN_SPLIT)
		{
		    expandEdges(mesh, nedges+1, &edges);
		    expandTris(mesh, ntris+1, &tris);
		    assert(nedges+1 <= mesh->maxnedges);
		    assert(ntris+1 <= mesh->maxntris);
		    tri = &tris[trii];
		    int childside0 = edges[tri->sides[sidei]].children[0];
		    int childside1 = nedges;
		    int childside2 = edges[tri->sides[(sidei+2)%3]].children[1];

		    tris[ntris].level = leveli+1;
		    PFSET_VEC3(tris[ntris].sides,
			       childside0, childside1, childside2);
		    PFSET_VEC3(tris[ntris].sidedirection, 1,1,1);
		    PFSET_VEC4(tris[ntris].children, -1,-1,-1,-1);
		    tris[ntris].orientation = tri->orientation;
		    INIT_EDGE(edges, childside1, leveli+1,
			  edges[childside0].status == ELIGIBLE_TO_BE_SPLIT
		       && edges[childside2].status == ELIGIBLE_TO_BE_SPLIT ?
			      ELIGIBLE_TO_BE_SPLIT : INELIGIBLE_TO_BE_SPLIT,
			      edges[childside0].to, edges[childside2].from);

		    tri->children[sidei] = ntris;

		    nedges++;
		    ntris++;
		    something_split = 1;
		}
	    }
	    if (something_split)
	    {
		//
		// Add a reversed triangle in the middle...
		//
		expandTris(mesh, ntris+1, &tris);
		assert(ntris+1 <= mesh->maxntris);
		tri = &tris[trii];
		tris[ntris].level = leveli+1;
		tris[ntris].orientation = -tri->orientation;
		FOR (sidei, 3)
		    if (tri->children[sidei] != -1)
		    {
			tris[ntris].sides[2-sidei] =
				tris[tri->children[sidei]].sides[1];
			tris[ntris].sidedirection[2-sidei] = 1;
		    }
		    else
		    {
			tris[ntris].sides[2-sidei] =
			    edges[tri->sides[(sidei+2)%3]].children[1];
			    // if not split, child 1 is itself
			tris[ntris].sidedirection[2-sidei] = -1;
		    }
		PFSET_VEC4(tris[ntris].children, -1,-1,-1,-1);
		tri->children[3] = ntris;
if (trii == 15)
{
printf("THERE at tri=15\n");
sginap(1);
}
		ntris++;
	    }
	}
    }
    mesh->nverts = nverts;
    mesh->ntris = ntris;
    mesh->nedges = nedges;
}


static void
meshPrint(struct Mesh *mesh)
{
    int i;
    printf("%d verts:\n", mesh->nverts);
    FOR (i, mesh->nverts)
    {
	printf("\t%3i: (%.9g %.9g %.9g)  (%.9g %.9g)\n",
		i,
		mesh->vc[i][0],
		mesh->vc[i][1],
		mesh->vc[i][2],
		mesh->tc[i][0],
		mesh->tc[i][1]);

    }
    printf("%d edges\n", mesh->nedges);
    FOR (i, mesh->nedges)
    {
	printf("\t%3i: from=%d to=%d level=%d status=%d children=%d,%d\n",
		i,
		mesh->edges[i].from,
		mesh->edges[i].to,
		mesh->edges[i].level,
		mesh->edges[i].status,
		mesh->edges[i].children[0],
		mesh->edges[i].children[1]);
    }
    printf("%d tris\n", mesh->ntris);
    FOR (i, mesh->ntris)
    {
	printf("\t%3i: level=%d orient=%d sides=%d,%d,%d sidedirs=%d,%d,%d children=%d,%d,%d,%d\n",
		i,
		mesh->tris[i].level,
		mesh->tris[i].orientation,
		mesh->tris[i].sides[0],
		mesh->tris[i].sides[1],
		mesh->tris[i].sides[2],
		mesh->tris[i].sidedirection[0],
		mesh->tris[i].sidedirection[1],
		mesh->tris[i].sidedirection[2],
		mesh->tris[i].children[0],
		mesh->tris[i].children[1],
		mesh->tris[i].children[2],
		mesh->tris[i].children[3]);
    }
}

#define FROMV(edge,dir) ((dir) == -1 ? (edge).from : (edge).to)
#define TOV(edge,dir) ((dir) == -1 ? (edge).to : (edge).from)

#ifdef MAIN // little test program
//
// Print in .im format
//
static void
meshPrint_im(struct Mesh *mesh)
{
    int ntris = mesh->ntris;
    double (*vc)[3] = mesh->vc;
    double (*tc)[2] = mesh->tc;
    struct Edge *edges = mesh->edges;
    struct Tri *tris = mesh->tris;

    printf("%s",
	   "new root Root\n"
	   "end_root\n"
	   "new geode Geode\n"
	   "end\n"
	   "attach Root Geode\n"
	   "load_geode Geode\n"
	  );

    int trii;
    FOR (trii, ntris)
    {
	struct Tri *tri = &tris[trii];
	if (tri->children[3] != -1)
	    continue; // if there are any children, [3] will be one
	printf("  poly 3\n");
	if (tri->orientation == 1)
	{
	    printf("    vc %.9g %.9g %.9g  %.9g %.9g %.9g  %.9g %.9g %.9g\n",
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][0],
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][1],
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][2],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][0],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][1],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][2],
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][0],
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][1],
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][2]);
	    printf("    nc %.9g %.9g %.9g  %.9g %.9g %.9g  %.9g %.9g %.9g\n",
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][0],
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][1],
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][2],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][0],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][1],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][2],
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][0],
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][1],
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][2]);
	    printf("    tc %.9g %.9g  %.9g %.9g  %.9g %.9g\n",
		tc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][0],
		tc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][1],
		tc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][0],
		tc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][1],
		tc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][0],
		tc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][1]);
	}
	else
	{
	    printf("    vc %.9g %.9g %.9g  %.9g %.9g %.9g  %.9g %.9g %.9g\n",
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][0],
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][1],
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][2],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][0],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][1],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][2],
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][0],
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][1],
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][2]);
	    printf("    nc %.9g %.9g %.9g  %.9g %.9g %.9g  %.9g %.9g %.9g\n",
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][0],
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][1],
		vc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][2],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][0],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][1],
		vc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][2],
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][0],
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][1],
		vc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][2]);
	    printf("    tc %.9g %.9g  %.9g %.9g  %.9g %.9g\n",
		tc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][0],
		tc[FROMV(edges[tri->sides[2]], tri->sidedirection[2])][1],
		tc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][0],
		tc[FROMV(edges[tri->sides[1]], tri->sidedirection[1])][1],
		tc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][0],
		tc[FROMV(edges[tri->sides[0]], tri->sidedirection[0])][1]);
	}
	printf("  end\n");
    }
}


main(int argc, char **argv)
{
    double viewDir[3] = {0,1,0};
    double viewRight[3] = {1,0,0};
    double viewUp[3] = {0,0,1};
    double viewOffset = -2;	/* dot prod of viewDir with eye point */
    float tol = 1;

    if (argc > 1)
	tol = atof(argv[1]);

    float hightol = tol;
    float lowtol = tol*.5;

    if (argc > 2)
	lowtol = atof(argv[1]);

    struct Mesh *mesh = allocMesh(1,1,1, NULL);
    meshOcta(mesh);
    meshSubdivide(mesh, viewDir, viewOffset, lowtol, hightol);
    fprintf(stderr, "%d tris, %d edges, %d vertices\n",
		mesh->ntris, mesh->nedges, mesh->nverts);
    if (getenv("_PFSPHEREFRONT_DEBUG"))
	meshPrint(mesh);
    else
	meshPrint_im(mesh);
    freeMesh(mesh);
    return 0;
}
#endif


struct sphereData
{
    int frequency; /* update every this many frames */

    float hightol, lowtol;
    pfVec2 lonlat[2], st[2]; // tie points for texture mapping

    struct Mesh *mesh;
			
    int maxntris;
    pfVec3 (*vc)[3];
    pfVec2 (*tc)[3];
    pfGeoSet *geoset;
};


/*
 * post-app callback to adjust the sphere for the current position
 */
static int
updateSphereFunc(pfTraverser *trav, void *_data)
{
    struct sphereData *data = (struct sphereData *)_data;

    int framecount = pfGetFrameCount();
    if (framecount % data->frequency != 0
     && framecount != 1) /* wacky on first frame */
	return PFTRAV_CONT;

    pfVec3 eye(0,0,0);
    pfMatrix viewmat, travmat;
    trav->getChan()->getViewMat(viewmat);
    trav->getMat(travmat);
    pfMatrix invtravmat;
    invtravmat.invertOrtho(travmat);
    pfMatrix eyespace_to_objspace_mat = viewmat;
    eyespace_to_objspace_mat.postMult(invtravmat);
    eye.xformPt(eye, eyespace_to_objspace_mat);

    pfVec3 viewDir(0,1,0);
    pfVec3 viewRight(1,0,0);
    pfVec3 viewUp(0,0,1);
    viewDir.xformVec(viewDir, eyespace_to_objspace_mat);
    viewUp.xformVec(viewUp, eyespace_to_objspace_mat);
    viewRight.xformVec(viewRight, eyespace_to_objspace_mat);
    viewDir.normalize();
    viewRight.normalize();
    viewUp.normalize();

    double doubleViewDir[3], doubleViewRight[3], doubleViewUp[3];;
    PFCOPY_VEC3(doubleViewDir, viewDir);
    PFCOPY_VEC3(doubleViewUp, viewUp);
    PFCOPY_VEC3(doubleViewRight, viewRight);

    struct Mesh *mesh = data->mesh;
    meshOcta(mesh);
    meshSubdivide(mesh,
		  doubleViewDir,
		  doubleViewRight,
		  doubleViewUp,
		  PFDOT_VEC3(doubleViewDir, eye),
		  data->lowtol, data->hightol);

    static int debug = -1;
    if (debug == -1)
    {
	char *e = getenv("_PFSPHEREFRONT_DEBUG");
	debug = (e ? *e  ? atoi(e) : 1 : 0);
    }
    if (debug)
	meshPrint(mesh);

    //
    // Expand attribute arrays if necessary...
    //
    if (mesh->ntris > data->maxntris)
    {
	// By braille...
	pfRef(data->vc);
	pfRef(data->tc);
	/*
	assert(pfGetRef(data->vc) == 1);
	assert(pfGetRef(data->tc) == 1);
	*/
	data->geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, NULL, NULL);
	data->geoset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX, NULL, NULL);
	data->geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, NULL, NULL);
	assert(pfGetRef(data->vc) == 1);
	assert(pfGetRef(data->tc) == 1);
	pfUnrefDelete(data->vc);
	pfUnrefDelete(data->tc);

	data->vc = (pfVec3 (*)[3])pfMalloc(mesh->ntris * sizeof(*data->vc),
					   mesh->arena);
	data->tc = (pfVec2 (*)[3])pfMalloc(mesh->ntris * sizeof(*data->tc),
					   mesh->arena);
	data->maxntris = mesh->ntris;
    }

    //
    // Copy stuff from the mesh structure into the attribute arrays...
    //
    int meshntris = mesh->ntris; // can have holes;
    int datantris = 0;

    struct Tri *meshtris = mesh->tris;
    struct Edge *meshedges = mesh->edges;
    double (*meshvc)[3] = mesh->vc;
    double (*meshtc)[2] = mesh->tc;
    pfVec3 (*datavc)[3] = data->vc;
    pfVec2 (*datatc)[3] = data->tc;

    datantris = 0;
    int meshtrii;
    FOR (meshtrii, meshntris)
    {
	struct Tri *tri = &meshtris[meshtrii];
	if (tri->children[3] != -1)
	    continue; // if there are any children, [3] will be one
	int sidei;
	FOR (sidei, 3)
	{
	    int verti =
		FROMV(meshedges[tri->sides[tri->orientation == 1 ? sidei : 2-sidei]], tri->sidedirection[tri->orientation == 1 ? sidei : 2-sidei]);
	    PFCOPY_VEC3(datavc[datantris][sidei], meshvc[verti]);
	    PFCOPY_VEC2(datatc[datantris][sidei], meshtc[verti]);
	}
	/*
	{
	printf("tri %d(%d) = %g %g %g  %g %g %g  %g %g %g\n",
		datantris, meshtrii,
		datavc[datantris][0][0],
		datavc[datantris][0][1],
		datavc[datantris][0][2],
		datavc[datantris][1][0],
		datavc[datantris][1][1],
		datavc[datantris][1][2],
		datavc[datantris][2][0],
		datavc[datantris][2][1],
		datavc[datantris][2][2]);
	}
	*/
	datantris++;
    }
    assert(datantris <= data->maxntris);

    /*
     * XXX is the following frame-accurate? I think not.
     */

    data->geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, datavc, NULL);
    data->geoset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX, datavc, NULL); /*sic*/
    data->geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, datatc, NULL);
    data->geoset->setPrimType(PFGS_TRIS);
    data->geoset->setNumPrims(datantris);

    return PFTRAV_CONT;
}

/*
 * Every frame, an octahedron is recursively subdivided from scratch.
 * For each edge, the error (in pixels) is calculated at its center.
 *	If error <= lowtol, the edge is not split.
 *	If lowtol < error < hightol, the edge is split by a new vertex
 *		placed somewhere between the edge center and the desired
 *		position, proportional to where error is along
 *		the range from lowtol to hightol.  The new edges
 *		are *not* eligible for further subdivision.
 *	If error >= hightol, the edge is split by a new vertex
 *		placed at the desired position.  The two new 
 *		edges are eligible for further subdivision.
 *
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
pfuNewSphereFront(float hightol,
		  float lowtol,
		  pfTexture *texture,
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

    data->mesh = allocMesh(1,1,1,pfGetSharedArena());
    data->hightol = hightol;
    data->lowtol = lowtol;
    data->maxntris = 1;
    data->vc = (pfVec3 (*)[3])pfMalloc(data->maxntris * sizeof(*data->vc));
    data->tc = (pfVec2 (*)[3])pfMalloc(data->maxntris * sizeof(*data->tc));

    pfGeoSet *geoset = new pfGeoSet();
    assert(geoset != NULL);
    geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, data->vc, NULL);
    geoset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX, data->vc, NULL);
    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, data->tc, NULL);
    geoset->setPrimType(PFGS_TRIS);
    geoset->setNumPrims(0);


    pfGeoState *geostate = new pfGeoState();
    assert(geostate != NULL);
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
    geode->setTravFuncs(PFTRAV_APP, NULL, updateSphereFunc);
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
