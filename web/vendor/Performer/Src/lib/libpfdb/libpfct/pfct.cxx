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
*/

/*
 * pfct.c
 *
 * Viewer for virtual clip textures.
 * Make it look like the whole virtual clip texture
 * is drawn on a single square, but it's really carved up
 * into concentric donuts (geodes) with varying LOD offsets and # of effective
 * levels so that the full spatial range and resolution range
 * can be viewed simultaneously (with no jello).
 *
 * We make the donuts the same size as the resolution bands (partly
 * for edification purposes),
 * but don't take into account the invalid border (don't
 * want the confusion of being on a triangle edge when examining
 * and pondering the hard edge between resolutions).
 * Also make sure there's a vertex at the clip center just for the heck of it.
 *
 * To get the best coordinate precision (i.e. to minimize the "jello"
 * effect), we want to make the "good area" as small as possible around
 * the geode.  So for the 1Kx1K geode, make the good area
 * 2Kx2K; for the 2Kx2K geode, make the good area 4Kx4K, etc.
 * (all these numbers are in finest-level texels).
 * We control the good area size by setting LODOffset and numEffectiveLevels
 * per geode, as follows:
 *
        Geode Size          Good Area Size      NumEffectiveLevels  LODOffset
        ----------          --------------      ------------------  ---------
        1Kx1K               2Kx2K               12                  0
        2Kx2K               4Kx4K               13                  0
        4Kx4K               8Kx8K               14                  0
        8Kx8K               16Kx16K             15                  0
        16Kx16K             32Kx32K             16                  0
        32Kx32K             64Kx64K             16                  1
        64Kx64K             128Kx128K           16                  2
        128Kx128K           256x256K            16                  3
        ...
        (vsize/4)x(vsize/4) (vsize/2)x(vsize/2) 16                  nlevels-17
        (vsize/2)x(vsize/2) (vsize)x(vsize)     16                  nlevels-16
        (vsize)x(vsize)     (vsize)x(vsize)     16                  nlevels-16
 *
 *
 * Listens to the following environment variables:
 *
 *	PFCT_STEPSIZE
 *		Specifies the ratio between adjacent donut sizes.
 *		The default is currently 8.
 *	PFCT_NDONUTS
 *		Specifies how many different donuts to draw,
 *		starting at size 1 and decreasing by a factor of
 *		PFCT_STEPSIZE each time.
 *		The default is enough so that the smallest donut
 *		is >= the clip size.
 *
 * XXX Current bugs:
    - Some tiles look bad (like max LOD is too high above LODOffset?)
    - pfi sensitivity is way too coarse (maybe check in env fudge thing)
    - morphing vertices get bizarre huge values sometimes? (when
      in scribe mode, white lines go off to infinity for a frame)
 */

#define MAXSUBDIV 100
static int SUBDIV = 1;
static double CURVATURE = 0;
static double LON0 = -.5;
static double LON1 = .5;
static double LAT0 = -.5;
static double LAT1 = .5;
static double S0 = 0.;
static double S1 = 1.;
static double T0 = 0.;
static double T1 = 1.;

#include <stdlib.h>
#include <math.h>
#if !defined( __linux__) && !defined(WIN32)
#include <mutex.h>
#endif

#ifdef WIN32
#include <windows.h>
//#include "wintypes.h"

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <Performer/pf.h>

#include <Performer/pr/pfGenericMatrix.h>

#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfDoubleDCS.h>
#include <Performer/pf/pfMPClipTexture.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfFlux.h>
#include <Performer/pr/pfCycleBuffer.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pr/pfTextureValidMap.h>
#include <Performer/pfutil/pfuClipCenterNode.h>
#include <Performer/pr/pfGeoMath.h>



PFPFB_DLLEXPORT pfNode* pfdLoadFile_cte(char *fileName);



#define FOR(i,n) for ((i) = 0; (i) < (n); ++(i))
#define round(x) floor((x)+.5)
#define log2(x) (log(x)*M_LOG2E)
#define intlog2(x) ((int)round(log2(x)))/* could make this faster if critical */
#define LERP(a,b,t) ((a) + (t)*((b)-(a)))
#define ATOF_GETENV(name, default) (getenv(name) ? atof(getenv(name)) : (default))
#define DTOR_ATOF_GETENV(name, default) (getenv(name) ? atof(getenv(name))*(M_PI/180.) : (default))

//
// XXX As of this writing, the implementation
// XXX using fluxes for the vertices is buggy--
// XXX the vertex positions seem to lag behind the
// XXX eye sometimes when DRAW is forked.  So
// XXX by default, use cycle buffers instead...
// XXX setenv _PFCT_USE_PFFLUX to use the flux implementation.
//
static int _PFCT_USE_PFFLUX = 0;

//
// getpfpid() quickly returns a unique index
// for each process that has ever asked for it.
// Note: this only works for forked processes, not sproced ones
// (to make it worked for sproced processes,
// we'd have to allocate mypfpid out of an explicitly
// nonshared arena).
// We only need it to differentiate between various CULL processes
// which are forked, so this is not a problem.
//
#define MAXPFPIDS 50
static uint32_t *npfpids = NULL;
static int mypfpid = -1;
static inline int getpfpid()
{
    if (mypfpid == -1)
    {
	if (npfpids == NULL)
	    pfNotify(PFNFY_FATAL, PFNFY_USAGE,
		     "pfdLoadFile_ct: pfdInitConverter_ct has not been called\n");
	mypfpid = test_then_add32(npfpids, 1);
	PFASSERTALWAYS(mypfpid < MAXPFPIDS);
    }
    return mypfpid;
}
extern "C" PFPFB_DLLEXPORT void pfdInitConverter_ct()
{

    {
	int hw_clipmaps;	
	pfQueryFeature( PFQFTR_TEXTURE_CLIPMAP, &hw_clipmaps );
	if( hw_clipmaps != PFQFTR_FAST )
	{
	    pfNotify( PFNFY_NOTICE, PFNFY_PRINT, 
		"pfdInitConverter_ct: cliptextures not available on this system. "
		"Using software emulation.");
	    return;		
	}
    }


    if (npfpids == NULL)
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "In pfdInitConverter_ct");
	npfpids = (uint32_t *)pfMalloc(sizeof(*npfpids), pfGetSharedArena());
	PFASSERTALWAYS(npfpids != NULL);
	// XXX should also make sure pfConfig() has not been called yet!
	*npfpids = 0;
    }

    {
	char *e = getenv("_PFCT_USE_PFFLUX");
	_PFCT_USE_PFFLUX = (e ? *e ? atoi(e) : 1 : 0);
    }
}

/*
 * Assume the channel cull polytope is in the shape of a standard frustum
 * (as it is in perfly- it will be the view frustum, possibly
 * shrunk by perfly's "Cull View FOV" controls).
 * Calculate that frustum.
 * (It's rather silly that we have to do this,
 * but we don't want to pollute perfly any more than is necessary)...
 * XXX actually, it's not necessary, if we change the API
 * XXX to pfuFindNearestWhatever so that it takes a list of planes
 * XXX rather than a pfFrustum or pfPolytope, since the implementation
 * XXX only uses the planes (assuming it knows which one is far)
 * XXX and not any other information.
 * XXX then we wouldn't need the stuff below, either
 */
/* XXX assume knowledge of frustum internals */
#define PFFRUST_LEFT    0
#define PFFRUST_RIGHT   1
#define PFFRUST_NEAR    2
#define PFFRUST_FAR     3
#define PFFRUST_BOTTOM  4
#define PFFRUST_TOP     5
static void
calcViewFrustumFromCullPtope(const pfPolytope *ptope, pfFrustum *frust)
{
    PFASSERTALWAYS(ptope->getNumFacets() == 6);
    pfPlane facets[6];
    int i;
    for (i = 0; i < 6; ++i)
        ptope->getFacet(i, &facets[i]);

    //
    // These calculations should be valid whether it's perspective
    // or orthogonal (though the orthogonal case has not been tested)...
    //
    float mynear = -facets[PFFRUST_NEAR].offset;
    float myfar = facets[PFFRUST_FAR].offset;

#define GET_COORD_AT_NEAR(which, ax) \
                ((facets[which].offset - mynear*facets[which].normal[PF_Y]) \
                / facets[which].normal[ax])

    float right  = GET_COORD_AT_NEAR(PFFRUST_RIGHT, PF_X);
    float left   = GET_COORD_AT_NEAR(PFFRUST_LEFT, PF_X);
    float top    = GET_COORD_AT_NEAR(PFFRUST_TOP, PF_Z);
    float bottom = GET_COORD_AT_NEAR(PFFRUST_BOTTOM, PF_Z);

    // XXX The sequence setNearFar, makePersp, orthoXform
    // XXX will drift (this is a bug but the API is so
    // XXX convoluted that it may not be able to be fixed).
    // XXX The only way I know to prevent this
    // XXX is to start with a pristine frustum each time.
    static pfFrustum *pristineFrustum;
    if (pristineFrustum == NULL)
	pristineFrustum = new pfFrustum();
    frust->copy(pristineFrustum);

    if (facets[PFFRUST_LEFT].normal[PF_Y] < -.01) /* sine of 1/2 degree */
    {
        frust->setNearFar(mynear, myfar); /* must come before makePersp */
        frust->makePersp(left, right, bottom, top);
    }
    else
    {
        /* XXX not tested yet */
        pfNotify(PFNFY_DEBUG, PFNFY_USAGE,
                 ".ct loader: orthogonal frusta have not been tested");
        frust->setNearFar(mynear, myfar);
        frust->makeOrtho(left, right, bottom, top);
    }
}



/*
 * Calculate 10 vertices forming a triangle strip (8 triangles)
 * in the shape of a square with a smaller square cut out of the middle.
 */
static void
GetDonut(double centerx, double centery,
         float bigdiameter, float littlediameter,
	 int subdiv,
         pfVec2d verts[/* 8*subdiv + 2 */],
	 double s0, double t0,
	 double s1, double t1)
{
    double r0 = PF_MIN2(littlediameter*.5, .5);
    double r1 = PF_MIN2(bigdiameter*.5, .5);
    double c0x = PF_CLAMP(centerx, s0+r0, s1-r0);
    double c0y = PF_CLAMP(centery, t0+r0, t1-r0);
    double c1x = PF_CLAMP(centerx, s0+r1, s1-r1);
    double c1y = PF_CLAMP(centery, t0+r1, t1-r1);

    pfVec2d coarseVerts[10];
    PFSET_VEC2(coarseVerts[0], c0x-r0, c0y-r0);
    PFSET_VEC2(coarseVerts[1], c1x-r1, c1y-r1);
    PFSET_VEC2(coarseVerts[2], c0x+r0, c0y-r0);
    PFSET_VEC2(coarseVerts[3], c1x+r1, c1y-r1);
    PFSET_VEC2(coarseVerts[4], c0x+r0, c0y+r0);
    PFSET_VEC2(coarseVerts[5], c1x+r1, c1y+r1);
    PFSET_VEC2(coarseVerts[6], c0x-r0, c0y+r0);
    PFSET_VEC2(coarseVerts[7], c1x-r1, c1y+r1);
    PFSET_VEC2(coarseVerts[8], c0x-r0, c0y-r0);
    PFSET_VEC2(coarseVerts[9], c1x-r1, c1y-r1);

    int i;
    int j = 0;
    FOR (i, subdiv)
    {
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[0],
				   i/(double)subdiv, coarseVerts[2]);
	j++;
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[1],
				   i/(double)subdiv, coarseVerts[3]);
	j++;
    }
    FOR (i, subdiv)
    {
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[2],
				   i/(double)subdiv, coarseVerts[4]);
	j++;
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[3],
				   i/(double)subdiv, coarseVerts[5]);
	j++;
    }
    FOR (i, subdiv)
    {
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[4],
				   i/(double)subdiv, coarseVerts[6]);
	j++;
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[5],
				   i/(double)subdiv, coarseVerts[7]);
	j++;
    }
    FOR (i, subdiv+1)
    {
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[6],
				   i/(double)subdiv, coarseVerts[8]);
	j++;
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[7],
				   i/(double)subdiv, coarseVerts[9]);
	j++;
    }
}

static inline double sqr(double x) { return x*x; }
/* 1-cos(t), doesn't lose accuracy for small t */
static inline double cos1(double t) { return 2.*sqr(sin(.5*t)); }
/* 1-cos(a)*cos(b), doesn't lose accuracy for small a or b */
static inline double cos1(double a, double b) { return cos1(a) + cos(a)*cos1(b); }

static void
BendDonut(double curvature, int subdiv, pfVec2d tcoords[/*subdiv*/], pfVec3d vcoords[/*subdiv*/],
	double lon0, double lon1, double lat0, double lat1,
	double s0,   double s1,   double t0,   double t1)
{
    double c = curvature;
    int i;
    FOR (i, 8*subdiv+2)
    {
	double s = tcoords[i][PF_S];
	double t = tcoords[i][PF_T];
	if (c == 0) // XXX find the right condition!
	{
	    vcoords[i][PF_X] = LERP(LON0, LON1, s);
	    vcoords[i][PF_Y] = 0;
	    vcoords[i][PF_Z] = LERP(LAT0, LAT1, t);
	}
	else
	{
	    double lon = LERP(lon0, lon1, s) * c;
	    double lat = LERP(lat0, lat1, t) * c;

	    double z = sin(lat) / c;
	    double x = cos(lat)*sin(lon) / c;
	    double y = cos1(lat,lon) / c;

	    vcoords[i][PF_X] = x;
	    vcoords[i][PF_Y] = y;
	    vcoords[i][PF_Z] = z;

	    vcoords[i][PF_Y] -= 1/c; //XXX center at origin? WRONG! inconsistent
	    vcoords[i][PF_X] += .5;
	    vcoords[i][PF_Z] += .5;
	}
	// tcoords was in 0..1 x 0..1, changing to in s0..s1 x t0..t1
	tcoords[i][PF_S] = LERP(s0, s1, s);
	tcoords[i][PF_T] = LERP(t0, t1, t);
    }
}

struct CallbackParams : public pfObject
{
    // XXX Think about general way of allowing tiepoints
    // XXX and/or bounds... like, exactly 13 variables
    // XXX must be specified, and the rest are calculated from them
    double s0, t0, lon0, lat0;
    double s1, t1, lon1, lat1;
    double tieS, tieT, tieX, tieY, tieZ;

    pfClipTexture *cliptex;
    pfMPClipTexture *mpcliptex;
    int nLevels, vSize, clipSize;
    pfGroup *geodesGroup;
    pfDoubleDCS *dcs;
    pfGeode *proxyGeode;
    float minLODTexPix;

    // Stuff that changes every frame.
    // We query it once per process per frame in the big group
    // cull callback, then each donut cull callback uses it.
    int invalidBorder;
    float minLODLoaded; // float for fade-in?
    float maxLODLoaded;
    pfVirtualClipTexLimits limits;

    float appliedMinLOD;
    float appliedMaxLOD;
    int appliedLODOffset;
    int appliedNumEffectiveLevels;

    /* scratch area so we don't have to reallocate each frame
	   (polytopes don't want to exist on the stack) */
    pfPolytope *ptope;
    pfFrustum *frust;

    CallbackParams & operator=(const CallbackParams &from)
    {
	// Bytewise copy all but frust and ptope...
	pfFrustum *save_frust = frust;
	pfPolytope *save_ptope = ptope;
	memcpy(this, &from, sizeof(CallbackParams));
	frust = save_frust;
	ptope = save_ptope;

	if (from.frust != NULL) *frust = *from.frust;
	if (from.ptope != NULL) *ptope = *from.ptope;
	return *this;
    }

    // Dummy version matching pfMemory's virtual, to prevent cmplr warning.
    pfMemory* operator=(const pfMemory *) {PFASSERTALWAYS(0); return this;}

    CallbackParams()
    {
	frust = new pfFrustum();
	ptope = new pfPolytope();
	PFASSERTALWAYS(frust != NULL && ptope != NULL);
	pfRef(ptope);
	pfRef(frust);
    }
    ~CallbackParams()
    {
	pfUnrefDelete(ptope);
	pfUnrefDelete(frust);
    }
};

// generic multibuffered object, one for each process
template <class T> class MP : public pfObject
{
public:
    MP()
    {
	int i;
	FOR (i, MAXPFPIDS)
	    array[i] = NULL;
    }
    ~MP()
    {
	int i;
	FOR (i, MAXPFPIDS)
	    if (array[i] != NULL)
		pfUnrefDelete(array[i]);
    }
    T *get(int ind = getpfpid()+1)
    {
	if (array[ind] == NULL)
	{
	    array[ind] = new(pfGetSharedArena()) T;
	    PFASSERTALWAYS(array[ind] != NULL);
	    pfRef(array[ind]);
	    if (ind != 0)
	    {
	        // copy the constant stuff (all but ptope and frust)
		// from template
		pfPolytope *ptope = array[ind]->ptope;
		pfFrustum *frust = array[ind]->frust;
		*array[ind] = *array[0];
		array[ind]->ptope = ptope;
		array[ind]->frust = frust;
	    }
	}
	return array[ind];
    }
    T *getTemplate()
    {
	return get(0);
    }
private:
    T *array[MAXPFPIDS];
};

//
// The following are constant per-geode parameters,
// so we can use a single one per node (across all processes).
//
struct DonutCullParams
{
    int index;
    float bigDiameter, littleDiameter;
    MP<CallbackParams> *MPglobalParams;
};



static pfMPClipTexture *findMPClipTexture(pfClipTexture *cliptex)
{
    int npipes = pfGetMultipipe();
    int i;
    for (i = 0; i < npipes; ++i)
    {
        pfPipe *pipe = pfGetPipe(i);
        int n_mpcliptextures = pipe->getNumMPClipTextures();
        int j;
        for (j = 0; j < n_mpcliptextures; ++j)
        {
            pfMPClipTexture *mpcliptex = pipe->getMPClipTexture(j);
            if (mpcliptex->getClipTexture() == cliptex)
                return mpcliptex;
        }
    }
    return NULL;
}

//
// We'd do this on the DCS node itself
// except that a Performer bug makes the result of trav->getMat()
// flaky (includes the current DCS node's matrix, but it shouldn't).
//

// This seems to pe transforming the model space matrix to eye space
// I don't see how this could work without identing the view matrix
// when dcs node is applied and that doesn't happen anywhere. This
// causes some culling problems when everything is drawn.
static int
dcsParentPreApp(pfTraverser *trav, void *)
{
    pfMatrix viewMatf;
    pfMatrix4d viewMat, travMat, invTravMat, eyeSpaceToLocalSpaceMat;
    trav->getChan()->getOffsetViewMat(viewMatf); // XXX I think Offset is wrong
    PFCOPY_MAT(viewMat, viewMatf);
    trav->getMatD(travMat);
    invTravMat.invertAff(travMat);
    eyeSpaceToLocalSpaceMat.mult(viewMat, invTravMat);

    // XXX should have an alternate test mode where it just gets the eye position
    // and pipes it through the inv trav mat

    pfDoubleDCS *dcs = (pfDoubleDCS *)((pfGroup *)trav->getNode())->getChild(0);
    PFASSERTALWAYS(dcs->isOfType(pfDoubleDCS::getClassType())); // don't flatten!
    dcs->setTrans(
	eyeSpaceToLocalSpaceMat[3][0],
	eyeSpaceToLocalSpaceMat[3][1],
	eyeSpaceToLocalSpaceMat[3][2]);
    // XXX need to fix the bounding volume! (do we? think about it)
    return PFTRAV_CONT;
}

//
// Post-app callback on the top node
// morphs the geometry to adapt to the current clip center.
//
static int
callbackGroupPostApp(pfTraverser *, void *arg)
{
    struct CallbackParams *params = ((MP<CallbackParams> *)arg)->get();
    if (params->mpcliptex == NULL)
    {
	params->mpcliptex = findMPClipTexture(params->cliptex);
	PFASSERTALWAYS(params->mpcliptex != NULL);
    }

    int centerS, centerT;
    params->mpcliptex->getCenter(&centerS, &centerT, NULL);
    double centerX = ((double)centerS / params->vSize - S0) / (S1 - S0);
    double centerY = ((double)centerT / params->vSize - T0) / (T1 - T0);

    pfGroup *geodesGroup = params->geodesGroup;
    int i, j, nDonuts = geodesGroup->getNumChildren();;

    FOR (i, nDonuts)
    {
	pfNode *child = geodesGroup->getChild(i);
	PFASSERTALWAYS(child->isOfType(pfGeode::getClassType()));
	pfGeode *geode = (pfGeode *)child;
	pfGeoSet *geoset = geode->getGSet(0);
	struct DonutCullParams *cullParams = (struct DonutCullParams *)
		geode->getTravData(PFTRAV_CULL);
	float bigdiameter = cullParams->bigDiameter;
	float littlediameter = cullParams->littleDiameter;

	pfVec3 *vcoords;
	pfVec2d *tcoords; // in double for now, demote later
	ushort *dummy;
	geoset->getAttrLists(PFGS_COORD3, (void **)&vcoords, &dummy);
	geoset->getAttrLists(PFGS_TEXCOORD2, (void **)&tcoords, &dummy);
	PFASSERTALWAYS(vcoords != NULL && tcoords != NULL && dummy == NULL);
	pfFlux *vcoords_flux, *tcoords_flux;
	if (_PFCT_USE_PFFLUX)
	{
	    vcoords_flux = pfFlux::getFlux(vcoords);
	    tcoords_flux = pfFlux::getFlux(tcoords);
	    vcoords = (pfVec3 *)vcoords_flux->getWritableData();
	    tcoords = (pfVec2d *)tcoords_flux->getWritableData();
	}
	pfVec2d tcoordsD[8*MAXSUBDIV+2];
	pfVec3d vcoordsD[8*MAXSUBDIV+2];
	GetDonut(centerX, centerY, bigdiameter, littlediameter, SUBDIV, tcoordsD, 0., 0., 1., 1.);
	BendDonut(CURVATURE, SUBDIV, tcoordsD, vcoordsD,
		  LON0, LON1, LAT0, LAT1,
		  S0, S1, T0, T1);

	// Subtract what the DCS added (namely, the eye in the parent space
	// of that DCS).  This will put the eye at (0,0,0).
	pfDoubleDCS *dcs = params->dcs;
	pfMatrix4d mat;
	dcs->getMat(mat);
	FOR (j, (8*SUBDIV+2))
	{
	    PFSUB_VEC3(vcoords[j], vcoordsD[j], mat[3]);
	    PFCOPY_VEC2(tcoords[j], tcoordsD[j]);
	}
	*(int32_t *)&tcoords[j] = 0; // use as a lock so only one cull process will convert it to single

	/*
	printf("Donut %d global to local adjustment:\n", i);
	FOR (j, (8*SUBDIV+2))
	    printf("    %.17g %.17g %.17g -> %.9g %.9g %.9g\n", vcoordsD[j][0],vcoordsD[j][1],vcoordsD[j][2], vcoords[j][0],vcoords[j][1],vcoords[j][2]);
	*/

	if (_PFCT_USE_PFFLUX)
	{
	    vcoords_flux->writeComplete();
	    tcoords_flux->writeComplete();
	}
	else // use cycle buffers
	{
	    pfCycleBuffer::getCBuffer(vcoords)->changed();
	    pfCycleBuffer::getCBuffer(tcoords)->changed();
	}
    }
    return PFTRAV_CONT;
}

//
// Pre-cull callback on the top node
// finds out global stuff about the clip texture
// and current orientation
// for use by the individual (per-geode) callbacks.
//
static int
callbackGroupPreCull(pfTraverser *trav, void *arg)
{
    struct CallbackParams *params = ((MP<CallbackParams> *)arg)->get();
    if (params->mpcliptex == NULL)
    {
	params->mpcliptex = findMPClipTexture(params->cliptex);
	PFASSERTALWAYS(params->mpcliptex != NULL);
    }

    pfChannel *chan = trav->getChan();

    pfMatrix viewmat, travmat;
    chan->getViewMat(viewmat);
    trav->getMat(travmat);
    pfMatrix invtravmat;
    invtravmat.invertOrtho(travmat);
    pfMatrix eyespace_to_objspace_mat = viewmat * invtravmat;
    pfMatrix invviewmat;
    invviewmat.invertOrtho(viewmat);
    pfMatrix objspace_to_eyespace_mat = travmat * invviewmat;

    //
    // Find the width and height of the near frustum face
    // in eye space (channel dimensions)
    // and the screen space viewport size.
    // and
    //
    int viewportWidth, viewportHeight;
    chan->getSize(&viewportWidth, &viewportHeight);
    pfPolytope *ptope = params->ptope;
    pfFrustum *frust = params->frust;
    chan->getCullPtope(ptope, PF_EYESPACE);
    calcViewFrustumFromCullPtope(ptope, frust);
    pfVec3 ll, lr, ul, ur; /* vertices of front frust face */
    frust->getNear(ll, lr, ul, ur);
    float nearwidth = PF_ABS(lr[PF_X] - ll[PF_X]);
    float nearheight = PF_ABS(ul[PF_Z] - ll[PF_Z]);

    //
    // Figure out the eyespace-to-homogeneous-screenspace matrix.
    // Eye space is y-forward,
    // screen space is -z-forward.
    //
    pfMatrix eyespace_to_screenspace_mat(
	viewportWidth/nearwidth*ll[1],    0, 0, 0,
	viewportWidth/2., viewportHeight/2., 0, 1,
	0,  viewportHeight/nearheight*ll[1], 0, 0,
	0,                                0, 0, 0);

    pfMatrix objspace_to_screenspace_mat = objspace_to_eyespace_mat
					 * eyespace_to_screenspace_mat;

    //
    // At the top level, just prepare the frustum (get it and transform it
    // into object space) so that the various children can use it
    // with minimal calculations
    //
    params->frust->orthoXform(frust, eyespace_to_objspace_mat);

    //
    // No on second thought, for now we'll just do a single
    // calculation on the whole square
    // instead of individual calculations for the donuts...
    //
    // XXX we calculate based on the point nearest to the eye plane,
    //     but this isn't necessarily the point with the min partial derivative
    //     if the coord-to-texcoord mapping is warped!  Should
    //     really clip the square against the frustum,
    //     take the partial derivatives at each clipped vertex,
    //     and take the min of the results!
    //
    int travmode = PFUTRAV_SW_CUR | PFUTRAV_SEQ_CUR
                 | PFUTRAV_LOD_RANGE0 | PFUTRAV_LAYER_BOTH;
        /* XXX would really like PFUTRAV_LOD_CUR but it doesn't exist */
    float x,y,z,s,t,r;
    if (pfuGetNearestVisiblePointToEyePlane(params->proxyGeode,
					    params->frust,
					    params->cliptex,
					    travmode,
					    &x, &y, &z, &s, &t, &r))
    {
	pfVec4 verts3d[3];
	// XXX this is only right for CURVATURE==0
	verts3d[0].set(LON0,0,LAT0,1);
	verts3d[1].set(LON1,0,LAT0,1);
	verts3d[2].set(LON0,0,LAT1,1);

	pfVec4 p3d;
	p3d.set(x, y, z, 1);

	//
	// Transform the problem into homogeneous screen space...
	//
	pfVec4 verts2d[3], p2d;
	int i;
	FOR (i, 3)
	    verts2d[i] = verts3d[i] * objspace_to_screenspace_mat;
	p2d = p3d * objspace_to_screenspace_mat;

	float LODBiasS, LODBiasT;
	params->mpcliptex->getLODBias(&LODBiasS, &LODBiasT, NULL);

        // XXX workaround for bizarre values of LODBiasS and LODBiasT...
        // XXX If I set the values to 0 after pfdLoadClipTexture,
        // XXX they don't seem to hold, so have to special case here.
        if (LODBiasS == PFTEX_DEFAULT) LODBiasS = 0.;
        if (LODBiasT == PFTEX_DEFAULT) LODBiasT = 0.;

	float ss, tt;
	float ds_dx, dt_dx, ds_dy, dt_dy;
	if (pfuCalcTexDerivs(
		      verts2d[0][0]/verts2d[0][3], verts2d[0][1]/verts2d[0][3],
		      verts2d[0][3], S0, T0,
		      verts2d[1][0]/verts2d[1][3], verts2d[1][1]/verts2d[1][3],
		      verts2d[1][3], S1, T0,
		      verts2d[2][0]/verts2d[2][3], verts2d[2][1]/verts2d[2][3],
		      verts2d[2][3], S0, T1,
		      p2d[0]/p2d[3], p2d[1]/p2d[3],
		      &ss, &tt,
		      &ds_dx, &dt_dx, &ds_dy, &dt_dy))
	{
	    //
	    // XXX Verify that we got the same answer for ss,tt as s,t!
	    //
	    float log2_ds_dxy = LODBiasS + log2(PF_MAX2(PF_ABS(ds_dx),
							PF_ABS(ds_dy)));
	    float log2_dt_dxy = LODBiasT + log2(PF_MAX2(PF_ABS(dt_dx),
							PF_ABS(dt_dy)));
	    params->minLODTexPix = params->nLevels-1
				 + PF_MAX2(log2_ds_dxy, log2_dt_dxy);

	    //
	    // Don't want popping of invalid border as a level
	    // changes from being the HW level 0 to not (iR bug
	    // that makes invalid border have half the effect it should
	    // on all but the finest level)
	    // so always try to set the LOD offset 1 level finer
	    // than we can actually see.
	    // XXX this is not the right place for this--
	    // XXX it should probably be in pfuCalcVirtualClipTexParams
	    //
	    params->minLODTexPix -= 1.;
	}
	else
	{
	    // One or more of the partial derivatives is infinite--
	    // use the coarsest LOD
	    params->minLODTexPix = params->nLevels-1;	// coarsest
	}

	static int _PFCT_DEBUG = -1;
	if (_PFCT_DEBUG == -1)
	{
	    char *e = getenv("_PFCT_DEBUG");
	    _PFCT_DEBUG = (e ? *e ? atoi(e) : 1 : 0);
	}
	if (_PFCT_DEBUG)
	{
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Nearest point: object (%f,%f,%f)(%f,%f)",
		     x, y, z, s, t);
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE,
		     "               homo   (%f,%f,%f,%f)",
		     p2d[0], p2d[1], p2d[2], p2d[3]);
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE,
		     "               screen (%f,%f)(%f,%f) derivs %f,%f,%f,%f max %f\n",
		     p2d[0]/p2d[3], p2d[1]/p2d[3], ss, tt,
		     ds_dx, dt_dx, ds_dy, dt_dy, 
		     params->minLODTexPix);
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE,
		     "               viewport: %d,%d near w,h: %f,%f, near depth: %f",
		     viewportWidth, viewportHeight, nearwidth, nearheight, ll[1]);
	}

	params->invalidBorder = params->mpcliptex->getInvalidBorder();
	params->minLODLoaded = params->cliptex->getMinDTRLOD();
	params->maxLODLoaded = params->cliptex->getNumAllocatedLevels()-1;
	params->mpcliptex->getLODOffsetLimit(
				&params->limits.LODOffset.lo,
				&params->limits.LODOffset.hi);
	params->mpcliptex->getNumEffectiveLevelsLimit(
				&params->limits.numEffectiveLevels.lo,
				&params->limits.numEffectiveLevels.hi);
	params->mpcliptex->getMinLODLimit(
				&params->limits.minLOD.lo,
				&params->limits.minLOD.hi);
	params->mpcliptex->getMaxLODLimit(
				&params->limits.maxLOD.lo,
				&params->limits.maxLOD.hi);

	// Make sure the per-tile optimization
	// doesn't try to remember stuff from last frame...
	params->appliedNumEffectiveLevels = -1;

	pfCullResult(PFIS_TRUE);
	return PFTRAV_CONT;
    }
    else // no visible point
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "pfct loader: CULLING!\n\n");
	pfCullResult(PFIS_FALSE);
	return PFTRAV_CONT; //XXX I think I wanted prune! but it behaves weirdly
    //	return PFTRAV_PRUNE; //commented out to remove warning
    }
}

static int forceALL_INCull(pfTraverser *, void *)
{
	pfCullResult(PFIS_MAYBE | PFIS_TRUE | PFIS_ALL_IN);
	return PFTRAV_CONT;
}

static int allgeodesPreCull(pfTraverser *, void *)
{
	pfPushState();
	return PFTRAV_CONT;
}

static int allgeodesPostCull(pfTraverser *, void *)
{
	pfPopState();
	return PFTRAV_CONT;
}

static int
donutPreCull(pfTraverser *trav, void *arg)
{
#ifdef NOTYET // not sure whether to do this globally or per-geode
    float x,y,z,s,t,r;
    if (pfuGetNearestVisiblePointToEyePlane(refnode, frust, tex,
					    travmode, &x, &y, &z, &s, &t, &r))
    {
	pfCullResult(PFIS_TRUE);
    }
    else
    {
	pfCullResult(PFIS_FALSE);
    }
    return PFTRAV_CONT; // not that it matters, since this is a leaf;
#endif // NOTYET
    struct DonutCullParams *cullParams = (struct DonutCullParams *)arg;
    struct CallbackParams *globalParams = cullParams->MPglobalParams->get();

    /*
    {
        static int prevFrameCount = 0;
        int frameCount = pfGetFrameCount();
        if (frameCount != prevFrameCount)
        {
            pfNotify(PFNFY_WARN, PFNFY_PRINT,
                     "CULL %d: %f\n",
                     frameCount,
                     globalParams->cliptex->getMinDTRLOD());
            prevFrameCount = frameCount;
        }
    }
    */

    float tradeoff = 1.; // for now, go coarse if there's a conflict

    int LODOffset, numEffectiveLevels;
    float minLOD, maxLOD;

    pfuCalcVirtualClipTexParams(
	globalParams->nLevels,
	globalParams->clipSize,
	globalParams->invalidBorder,
	globalParams->minLODTexPix,
	globalParams->minLODLoaded,
	globalParams->maxLODLoaded,
	.5*cullParams->littleDiameter,
	.5*cullParams->bigDiameter,
	tradeoff,
	&globalParams->limits,

	&LODOffset,
	&numEffectiveLevels,
	&minLOD,
	&maxLOD);

    static int _PFCT_DEBUG = -1;
    if (_PFCT_DEBUG == -1)
    {
        char *e = getenv("_PFCT_DEBUG");
        _PFCT_DEBUG = (e ? *e ? atoi(e) : 1 : 0);
    }
    if (_PFCT_DEBUG)
    {
	if (cullParams->index == 0)
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT,
		    "Constant params: nLevels=%d clipSize=%d invalidBorder=%d",
		    globalParams->nLevels,
		    globalParams->clipSize,
		    globalParams->invalidBorder);
	    pfNotify(PFNFY_INFO, PFNFY_MORE,
		    "Global params: minLODTexPix=%f minLODLoaded=%f maxLODLoaded=%f",
		    globalParams->minLODTexPix,
		    globalParams->minLODLoaded,
		    globalParams->maxLODLoaded);
	}
	pfNotify(PFNFY_INFO, PFNFY_MORE,
		"Donut %d params: bboxMinDist=%f bboxMaxDist=%f tradeoff=%f",
		cullParams->index,
		.5*cullParams->littleDiameter,
		.5*cullParams->bigDiameter,
		tradeoff);
	pfNotify(PFNFY_INFO, PFNFY_MORE,
		"        limits: LODOffset=%d..%d effectiveLevels=%d..%d minLOD=%f..%f maxLOD=%f..%f\n",
		globalParams->limits.LODOffset.lo,
		globalParams->limits.LODOffset.hi,
		globalParams->limits.numEffectiveLevels.lo,
		globalParams->limits.numEffectiveLevels.hi,
		globalParams->limits.minLOD.lo,
		globalParams->limits.minLOD.hi,
		globalParams->limits.maxLOD.lo,
		globalParams->limits.maxLOD.hi);
	pfNotify(PFNFY_INFO, PFNFY_MORE,
		"       results: LODOffset=%d effectiveLevels=%d minLOD=%f maxLOD=%f",
		LODOffset,
		numEffectiveLevels,
		minLOD,
		maxLOD);
    }

    //
    // Hack for precision in huge clipmaps:
    // Translate the texture coords by a multiple of the
    // good area size towards the lower left of the whole virtual clipmap.
    //
    {
	int s, t, r;
	globalParams->cliptex->getCurCenter(&s, &t, &r);
        int goodAreaSize = 1<<(numEffectiveLevels+LODOffset-1);
        int newS = ((s-(goodAreaSize>>1))&(goodAreaSize-1))+(goodAreaSize>>1);
        int newT = ((t-(goodAreaSize>>1))&(goodAreaSize-1))+(goodAreaSize>>1);
        newS = PF_MIN2(newS, s); // don't increase
        newT = PF_MIN2(newT, t); // don't increase

// newS = s;
// newT = t;
	double fudgeS = (double)(newS - s) / globalParams->vSize;
	double fudgeT = (double)(newT - t) / globalParams->vSize;

	pfFlux *tcoords_flux;
	pfVec2 *tcoords;

	pfGeode *geode = (pfGeode *)trav->getNode();
	PFASSERTALWAYS(geode->isOfType(pfGeode::getClassType()));
	pfGeoSet *geoset = geode->getGSet(0);
	ushort *dummy;
	geoset->getAttrLists(PFGS_TEXCOORD2, (void **)&tcoords, &dummy);
	if (_PFCT_USE_PFFLUX)
	{
	    tcoords_flux = pfFlux::getFlux(tcoords);
	    tcoords = (pfVec2 *)tcoords_flux->getWritableData();
	}
	pfVec2d *double_tcoords = (pfVec2d *)tcoords; // shrink in place
	PFASSERTALWAYS(double_tcoords != NULL);

	int j;

/*
printf("Donut %d tcoords were:\n", cullParams->index);
FOR (j, 8*SUBDIV+2)
    printf("    %.17g,%.17g\n",  double_tcoords[j][0], double_tcoords[j][1]);
*/
/* (for cbuffer problem I was having)...
printf("Donut %d tcoords were (interpreted as float):\n", cullParams->index);
FOR (j, 8*SUBDIV+2)
    printf("    %.9g,%.9g\n",  tcoords[j][0], tcoords[j][1]);
*/

	//
	// Only do the in-place double-to-single conversion
	// if this is the first cull callback looking at this array.
	// Find this out by atomically setting a lock which is
	// placed at the end of the DP array.
	// It's okay for the other CULL threads to go ahead
	// while the first one is still working on this conversion,
	// since in the case when multiple CULLs are going at once,
	// there is a synchronization before the DRAWs start.
	//
	if (test_and_set32((uint32_t *)&double_tcoords[8*SUBDIV+2][0], 1) == 0)
	{
	    FOR (j, 8*SUBDIV+2)
	    {
		tcoords[j][0] = double_tcoords[j][0] + fudgeS;
		tcoords[j][1] = double_tcoords[j][1] + fudgeT;
	    }
	}

/*
printf("Donut %d tcoords are:\n", cullParams->index);
FOR (j, 8*SUBDIV+2)
    printf("    %.9g,%.9g\n",  tcoords[j][0], tcoords[j][1]);
*/
	
	if (_PFCT_USE_PFFLUX)
	{
	    tcoords_flux->writeComplete();
	}
	else // use cycle buffers
	{
	    // XXX if we do this here in the CULL, all hell breaks loose... ???
	    // XXX fortunately the APP already did it so it
	    // XXX prevented previous frame's contents
	    // XXX from coming down and clobbering this one.
	    // XXX I don't really understand this... ?
	    // XXX Should I not do it for the flux version either???
	    //pfCycleBuffer::getCBuffer(tcoords)->changed();
	}

	globalParams->cliptex->applyCenter(newS, newT, r); // override real center
    }
    //
    // Apply those parameters that changed since last time
    //
    if (globalParams->appliedNumEffectiveLevels == -1)
    {
	// first time, or _PFCT_SMART_CULLBACKS is off
	globalParams->cliptex->applyVirtualParams(LODOffset,numEffectiveLevels);
	globalParams->cliptex->applyMinLOD(minLOD);
	globalParams->cliptex->applyMaxLOD(maxLOD);
/*
pfNotify(PFNFY_WARN, PFNFY_PRINT,
	"%d frame %d: applying LODOffset=%d, effectiveLevels=%d, minLOD=%f, maxLOD=%f",
		get_pid(), pfGetFrameCount(),
		LODOffset, numEffectiveLevels, minLOD, maxLOD);
*/
    }
    else
    {
	if (LODOffset != globalParams->appliedLODOffset
	 || numEffectiveLevels != globalParams->appliedNumEffectiveLevels)
	{
	    globalParams->cliptex->applyVirtualParams(LODOffset, numEffectiveLevels);
	    /* fprintf(stderr, "V"); */
	}
	else
	{
	    /* fprintf(stderr, "v"); */
	}
	if (minLOD != globalParams->appliedMinLOD)
	{
	    globalParams->cliptex->applyMinLOD(minLOD);
	    /* fprintf(stderr, "I"); */
	}
	else
	{
	    /* fprintf(stderr, "i"); */
	}
	if (maxLOD != globalParams->appliedMaxLOD)
	{
	    globalParams->cliptex->applyMaxLOD(maxLOD);
	    /* fprintf(stderr, "A"); */
	}
	else
	{
	    /* fprintf(stderr, "a"); */
	}
    }
    static int _PFCT_SMART_CULLBACKS = -1;
    if (_PFCT_SMART_CULLBACKS == -1)
    {
	const char *e = getenv("_PFCT_SMART_CULLBACKS");
	_PFCT_SMART_CULLBACKS = (e ? *e ? atoi(e) : 1 : 0);
    }
    if (_PFCT_SMART_CULLBACKS)
    {
	globalParams->appliedNumEffectiveLevels = numEffectiveLevels;
	globalParams->appliedLODOffset = LODOffset;
	globalParams->appliedMinLOD = minLOD;
	globalParams->appliedMaxLOD = maxLOD;
    }

    return PFTRAV_CONT; // not that it matters, since this is a leaf;
}

static int
clipCenterNodePostApp(pfTraverser *trav)
{
    //
    // Could just call pfuGetClosestPoint
    // on the proxy node (a square)
    // but we want double precision accuracy,
    // so just ignore the proxy and calculate it.
    //

    //
    // Get the eye in the coordinate space of refnode...
    //
    pfMatrix viewMatf;
    pfMatrix4d viewMat, travMat, invTravMat, eyeSpaceToLocalSpaceMat;
    trav->getChan()->getOffsetViewMat(viewMatf); // XXX I think Offset is wrong
    PFCOPY_MAT(viewMat, viewMatf);
    trav->getMatD(travMat);
    invTravMat.invertAff(travMat);
    eyeSpaceToLocalSpaceMat.mult(viewMat, invTravMat);
    pfVec3d eye(0,0,0);
    eye.fullXformPt(eye, eyeSpaceToLocalSpaceMat);

    pfuClipCenterNode *ccn = (pfuClipCenterNode *)trav->getNode();
    pfMPClipTexture *mpcliptex = ccn->getMPClipTexture();
    pfClipTexture *cliptex = ccn->getClipTexture();
    int vSizeS, vSizeT, vSizeR;
    cliptex->getVirtualSize(&vSizeS, &vSizeT, &vSizeR);

    double s, t, r;
    if (CURVATURE == 0.)
    {
	s = LERP(S0, S1, (eye[PF_X]-LON0)/(LON1-LON0));
	t = LERP(T0, T1, (eye[PF_Z]-LAT0)/(LAT1-LAT0));
	r = 0.;
	s = PF_CLAMP(s, S0, S1);
	t = PF_CLAMP(t, T0, T1);
    }
    else
    {
	PFASSERTALWAYS(CURVATURE == 1.); // XXX the following isn't right otherwise
	double h = hypot(eye[PF_X], eye[PF_Y]);
	double eyeLat = atan2(eye[PF_Z], h);
	double eyeLon = atan2(eye[PF_X], -eye[PF_Y]);
	// XXX add multiple of 360 degrees to make it as close as possible?
	eyeLat = PF_CLAMP(eyeLat, LAT0, LAT1);
	eyeLon = PF_CLAMP(eyeLon, LON0, LON1);
	s = LERP(S0, S1, (eyeLon-LON0)/(LON1-LON0));
	t = LERP(T0, T1, (eyeLat-LAT0)/(LAT1-LAT0));
	r = 0.;
    }
    int clipCenterS = s * vSizeS;
    int clipCenterT = t * vSizeT;
    int clipCenterR = r * vSizeR;

    static int _PFCT_DEBUG = -1;
    if (_PFCT_DEBUG == -1)
    {
	char *e = getenv("_PFCT_DEBUG");
	_PFCT_DEBUG = (e ? *e ? atoi(e) : 1 : 0);
    }
    if (_PFCT_DEBUG)
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		 "eye in local space: %.17g %.17g %.17g\n",
		 eye[0], eye[1], eye[2]);
	pfNotify(PFNFY_DEBUG, PFNFY_MORE,
		 "clip center: %.17g %.17g -> %d %d %d\n",
		 s, t,
		 clipCenterS, clipCenterT, clipCenterR);
    }
    mpcliptex->setCenter(clipCenterS, clipCenterT, clipCenterR);

    return PFTRAV_CONT;
}

extern "C" PFPFB_DLLEXPORT pfNode *
pfdLoadFile_ct(char *fileName)
{


    {
	int hw_clipmaps;	
	pfQueryFeature( PFQFTR_TEXTURE_CLIPMAP, &hw_clipmaps );
	if( hw_clipmaps != PFQFTR_FAST )
	{
	    pfNotify( PFNFY_NOTICE, PFNFY_PRINT, 
		"pfdLoadFile_ct: cliptextures not available on this system. "
		"Using software emulation.");
	    return pfdLoadFile_cte(fileName);		
	}
    }




    CURVATURE = ATOF_GETENV("_PFCT_CURVATURE", CURVATURE);
    SUBDIV    = ATOF_GETENV("_PFCT_SUBDIV", SUBDIV);
    LON0      = DTOR_ATOF_GETENV("_PFCT_LON0", LON0);
    LON1      = DTOR_ATOF_GETENV("_PFCT_LON1", LON1);
    LAT0      = DTOR_ATOF_GETENV("_PFCT_LAT0", LAT0);
    LAT1      = DTOR_ATOF_GETENV("_PFCT_LAT1", LAT1);
    S0        = ATOF_GETENV("_PFCT_S0", S0);
    S1        = ATOF_GETENV("_PFCT_S1", S1);
    T0        = ATOF_GETENV("_PFCT_T0", T0);
    T1        = ATOF_GETENV("_PFCT_T1", T1);


    pfGeoSet *proxyGeoSet;
    pfClipTexture *cliptex;

    /*
     * Read the .ct file...
     */
    char *cliptexFileName = fileName;
    cliptex = pfdLoadClipTexture(cliptexFileName);
    if (cliptex == NULL)
    {
        pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
                 "pfdLoadFile_ct: Couldn't load clip texture from file \"%s\"",
                 cliptexFileName);
	return NULL;
    }
    cliptex->setName(cliptexFileName);

    /*
     * Make sure it's virtual so we can set
     * the LODOffset and numEffectiveLevels...
     */
    int vSize;
    cliptex->getVirtualSize(&vSize, NULL, NULL);
    int nLevels = intlog2(vSize) + 1;
    if (cliptex->getNumEffectiveLevels() >= nLevels)
    {
        pfNotify(PFNFY_INFO, PFNFY_PRINT,
                "pfdLoadFile_ct: clip texture \"%s\" is not virtual; you can make it virtual (so that numEffectiveLevels can be adjusted for better stability) by adding the line \"effective_levels %d\"",
                cliptexFileName, nLevels-1);
    }

    int clipSize = cliptex->getClipSize();

    char *e = getenv("_PFCT_VALID_MAP");
    if (e != NULL)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		 "pfdLoadFile_ct: creating a texture valid map");
	pfTextureValidMap *validMap = new pfTextureValidMap(
	    vSize, vSize,
	    PF_MIN2(2*clipSize, vSize), PF_MIN2(2*clipSize, vSize),
	    clipSize, clipSize,
	    8, 8); // XXX machine dependent
	PFASSERTALWAYS(validMap != NULL);
	validMap->setTracing(*e ? atoi(e) : 0);
	cliptex->setValidMap(validMap);
    }

    /*
     * Make a geostate referring to the clip texture;
     * this will be used for all geosets including the proxy.
     */
    pfGeoState *geostate = new pfGeoState();
    PFASSERTALWAYS(geostate != NULL);
    geostate->setAttr(PFSTATE_TEXTURE, cliptex);
    geostate->setMode(PFSTATE_ENTEXTURE, TRUE);

    /*
     * Make a proxy geode for calculating the closest point for
     * clip centering; this is just a square textured by the clip texture.
     */
    pfGeode *proxyGeode;
    {
	static float square[4][2] = {{-.5,-.5},{.5,-.5},{.5,.5},{-.5,.5}};
	pfVec3 *proxyVertCoords =
		(pfVec3 *)pfMalloc(4*sizeof(pfVec3), pfGetSharedArena());
	pfVec2 *proxyTexCoords =
		(pfVec2 *)pfMalloc(4*sizeof(pfVec2), pfGetSharedArena());
	PFASSERTALWAYS(proxyVertCoords != NULL && proxyTexCoords != NULL);
	int i;
	FOR (i, 4)
	{
	    PFSET_VEC3(proxyVertCoords[i], square[i][0], 0.0f, square[i][1]);
	    PFSET_VEC2(proxyTexCoords[i], square[i][0], square[i][1]);
	}
	proxyGeoSet = new pfGeoSet();
	PFASSERTALWAYS(proxyGeoSet != NULL);
	proxyGeoSet->setGState(geostate);
	proxyGeoSet->setPrimType(PFGS_QUADS);
	proxyGeoSet->setNumPrims(1);
	proxyGeoSet->setAttr(PFGS_COORD3, PFGS_PER_VERTEX,
			     proxyVertCoords, NULL);
	proxyGeoSet->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX,
			     proxyTexCoords, NULL);
	proxyGeode = new pfGeode();
	PFASSERTALWAYS(proxyGeode != NULL);
	proxyGeode->addGSet(proxyGeoSet);
    }

    /*
     * The structure will be as follows:
     *
                callbackGroup (post-app callback updates geodes' geometry
                  |            based on clip center)
                clipCenterNode (post-app callback sets clip center)
                  |
		 dcsParent
		  |
		 dcs	       represents local origin at the eye
		  |
                geodesGroup
             /   /  |
        geode geode geode ...	(the donuts)
     */

    MP<CallbackParams> *MPcallbackParams = new(pfGetSharedArena()) MP<CallbackParams>();
    PFASSERTALWAYS(MPcallbackParams != NULL);

    pfGroup *geodesGroup = new pfGroup();
    PFASSERTALWAYS(geodesGroup != NULL);
    int nDonuts = 1 + intlog2(vSize/clipSize);
    double donutStep = 8.; // each donut is 8 times the size of the next one
    {
	// XXX make these names and semantics more natural,
	// XXX probably adapting to clip size
	char *e;
	e = getenv("PFCT_STEPSIZE");
	if (e != NULL)
	    donutStep = atof(e);
	if (donutStep < 1.)
	    donutStep = 1./donutStep; // so can specify either as 8 or .125
	PFASSERTALWAYS(donutStep > 1.);
	e = getenv("PFCT_NDONUTS");
	if (e != NULL)
	    nDonuts = atoi(e);
	else
	{
	    // go down to the clip size if possible but no farther
	    nDonuts = 1;
	    while (vSize/pow(donutStep, nDonuts) > clipSize-1.)
		nDonuts++;
	}
    }

    int i;

    FOR (i, nDonuts)
    {
        int *lengths = (int *)pfMalloc(1*sizeof(*lengths), pfGetSharedArena());
	PFASSERTALWAYS(lengths != NULL);
        lengths[0] = (8*SUBDIV+2);
        pfVec4 *white = (pfVec4 *)pfMalloc(1*sizeof(*white), pfGetSharedArena());
        PFASSERTALWAYS(white != NULL);
        white->set(1,1,1,1);

        /* no need to initialize vcoords&tcoords, that will be done later */

        pfGeoSet *geoset = new pfGeoSet();
        PFASSERTALWAYS(geoset != NULL);
        geoset->setGState(geostate);
        geoset->setNumPrims(1);
        geoset->setPrimLengths(lengths);
        geoset->setPrimType(PFGS_TRISTRIPS);
	if (_PFCT_USE_PFFLUX)
	{
	    geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX,
			    new pfFlux((8*SUBDIV+2)*sizeof(pfVec3),
				       PFFLUX_DEFAULT_NUM_BUFFERS),
			    NULL);
	    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX,
			    new pfFlux((8*SUBDIV+2)*sizeof(pfVec2d) + sizeof(uint32_t), // sic
				       PFFLUX_DEFAULT_NUM_BUFFERS),
			    NULL);
	}
	else // use cycle buffers
	{
	    geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX,
			    new pfCycleBuffer((8*SUBDIV+2)*sizeof(pfVec3)), NULL);
	    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX,
			    new pfCycleBuffer((8*SUBDIV+2)*sizeof(pfVec2d) + sizeof(uint32_t)), NULL); // sic
	}
        geoset->setAttr(PFGS_COLOR4, PFGS_OVERALL, white, NULL);

        pfGeode *geode = new pfGeode();
        PFASSERTALWAYS(geode != NULL);
        geode->addGSet(geoset);

	struct DonutCullParams *donutCullParams =
		(struct DonutCullParams *)pfMalloc(sizeof(*donutCullParams),
						   pfGetSharedArena());
	PFASSERTALWAYS(donutCullParams != NULL);
	donutCullParams->index = i;
	donutCullParams->bigDiameter = 1./pow(donutStep, i);
	donutCullParams->littleDiameter = (i==nDonuts-1 ? 0. : 1./pow(donutStep, i+1));
	donutCullParams->MPglobalParams = MPcallbackParams;
        geode->setTravFuncs(PFTRAV_CULL, donutPreCull, NULL);
        geode->setTravData(PFTRAV_CULL, donutCullParams);

	//
	// By default, pre- and post-CULL funcs on a node
	// do not prevent the contents of the node
	// from being interleaved with the contents
	// of another node during CULL sorting.
	// This means that (surprise!) the contents of the node are not
	// necessarily bracketted by the graphics calls made in
	// the pre- and post-node CULL funcs.
	//     (This behavior is for the benefit of CULL funcs
	//     whose purpose is to return a cull result
	//     but that don't make any graphics calls.
	//     Note that in contrast, pre- and post-DRAW
	//     funcs on a node are guaranteed to bracket the
	//     contents of the node, and therefore
	//     they prevent the interleaving of the contents of the
	//     node with the contents of other nodes.)
	//
	// We need to make sure the the bracketing occurs
	// so that the calls to applyVirtualParams() etc.
	// will be applied to the appropriate geometry,
	// so we make the following setTravMode call.
	// (Try omitting this call by "setenv _PFCT_DONT_CONTAIN_SORTING";
	// note that the virtual LOD offset is wrong much of the time.)
	//
	// Other (slower) ways of making sure this bracketting occurs are:
	//	- Turning off sorting altogether (use the "Cull Mode"
	//	  selector in perfly or clipfly, or the equivalent
	//	  Performer calls)
	//	- Give the node a pre- or post-DRAW func
	//

	if (!getenv("_PFCT_DONT_CONTAIN_SORTING"))
	    geode->setTravMode(PFTRAV_CULL, PFN_CULL_SORT,
					    PFN_CULL_SORT_CONTAINED);

        geodesGroup->addChild(geode);
    }

    // for the same reason as the above we need to disable sorting around the
    // geodes here. This isn't a big deal, we know where the clip texture state 
    // is being applied
    geodesGroup->setTravFuncs(PFTRAV_CULL, allgeodesPreCull, allgeodesPostCull);

    if (!getenv("_PFCT_DONT_CONTAIN_SORTING"))
	geodesGroup->setTravMode(PFTRAV_CULL, PFN_CULL_SORT, PFN_CULL_SORT_CONTAINED);
    //
    // Add a DCS above us so that
    // we can make the local origin be close to the eye.
    // Attach the callback to its parent rather than itself
    // to work around a bug that makes the result
    // of pfTraverser::getTravMat() unreliable on an SCS or DCS.
    //
    pfDoubleDCS *dcs = new pfDoubleDCS();
    dcs->addChild(geodesGroup);
    pfGroup *dcsParent = new pfGroup();
    dcsParent->addChild(dcs);
    // This callback causes culling problems, it's unclear
    // what it's trying to do.
    dcsParent->setTravFuncs(PFTRAV_APP, dcsParentPreApp, NULL);

    pfuClipCenterNode *clipCenterNode = pfuNewClipCenterNode();
    PFASSERTALWAYS(clipCenterNode != NULL);

    clipCenterNode->setTravFuncs(PFTRAV_CULL, forceALL_INCull, NULL);

    clipCenterNode->setClipTexture(cliptex);
if (!getenv("_PFCT_NOPROXY"))
    clipCenterNode->setRefNode(proxyGeode);
    clipCenterNode->addChild(dcsParent);
    clipCenterNode->setCallback(clipCenterNodePostApp);

    struct CallbackParams *callbackParams = MPcallbackParams->getTemplate();
    callbackParams->cliptex = cliptex;
    callbackParams->mpcliptex = NULL;
    callbackParams->nLevels = nLevels;
    callbackParams->vSize = vSize;
    callbackParams->clipSize = clipSize;
    callbackParams->geodesGroup = geodesGroup;
    callbackParams->proxyGeode = proxyGeode;
if (getenv("_PFCT_NOPROXY"))
callbackParams->proxyGeode = (pfGeode*)geodesGroup;
    callbackParams->dcs = dcs;
    callbackParams->appliedNumEffectiveLevels = -1; // i.e. uninitialized

    callbackParams->frust->unrefDelete();
    callbackParams->ptope->unrefDelete();
    callbackParams->frust = NULL;
    callbackParams->ptope = NULL;

    pfGroup *callbackGroup = new pfGroup();
    callbackGroup->addChild(clipCenterNode);
    callbackGroup->setTravFuncs(PFTRAV_APP, NULL, callbackGroupPostApp);
    callbackGroup->setTravData(PFTRAV_APP, MPcallbackParams);
    callbackGroup->setTravFuncs(PFTRAV_CULL, callbackGroupPreCull, NULL);
    callbackGroup->setTravData(PFTRAV_CULL, MPcallbackParams);

    pfSphere sph;
    sph.center.set(0.0f, 0.0f, 0.0f);
#ifdef WIN32
    sph.radius=fsqrt(0.5);
#else
    sph.radius=M_SQRT1_2;
#endif
    callbackGroup->setBound(&sph, PFBOUND_STATIC);

    return callbackGroup;
}
