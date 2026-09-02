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
 * clipringsnode.h - definition of clip rings node
 * for spherepatch loader.
 * 
 * $Revision: 1.9 $ 
 * $Date: 2002/11/07 18:06:03 $
 */

extern "C" void _pfPrint(void*); // XXX

#ifndef __CLIPRINGSNODE_H__
#define __CLIPRINGSNODE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#if !defined(__linux__) && !defined(WIN32)
#include <mutex.h>
#else
#ifdef WIN32
#include <process.h>
typedef signed long int intptr_t;
#define get_pid _getpid
#endif
#endif

#ifdef WIN32
#include <windows.h>
#include <limits.h>

#define MAXFLOAT FLT_MAX
#define MAXDOUBLE FLT_MAX
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

#include <Performer/pfutil/pfuClipCenterNode.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfMPClipTexture.h>
#include <Performer/pr/pfClipTexture.h>
#include <Performer/pr/pfGeoSet.h>

// ======== PRIVATE STUFF (don't look at this, look down below instead) ======
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
#ifdef __linux__
#define get_pid getpid
#endif
static uint32_t *npfpids = NULL;
static int mypfpid = -1;
static inline int getpfpid()
{
    if (mypfpid == -1)
    {
        if (npfpids == NULL)
            pfNotify(PFNFY_FATAL, PFNFY_USAGE,
                     "pfdLoadFile_spherepatch3: pfdInitConverter_spherepatch3 has not been called\n");
        mypfpid = test_then_add32(npfpids, 1);
        PFASSERTALWAYS(mypfpid < MAXPFPIDS);
    }
    return mypfpid;
}
// generic multibuffered object, one for each process
//     (among other things, this is so multiple CULLs
//     can be working at the same time with their own scratch
//     areas without stepping on each other
//     XXX get clear on whether I really believe this...
//     this is from libpfct code; I don't really remember the
//     original motivation)
template <class T> class MP : public pfObject
{
public:
    MP()
    {
        int i;
        FOR (i, numberof(array))
            array[i] = NULL;
    }
    ~MP()
    {
        int i;
        FOR (i, numberof(array))
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
                // XXX assumes T is PerProcessCullData,
		// XXX i.e. that it actually has ptope and frust members :-(
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
    T *array[MAXPFPIDS+1];
};
// =========================== END OF PRIVATE STUFF =========================


/*
    ClipRingsNode
    This node type supports the notion of "clip rings",
    i.e. bins forming concentric square rings in texture space around
    the clip center of a clip texture.
    Each ring can have a different setting of the virtual
    clip texture parameters
    virtualLODOffset, numEffectiveLevels, minLOD, and maxLOD.

    The created structure looks like this:

           ClipRingsNode  (has methods for APP to update verts, normals,
             /   /   |     texcoordsD and topology for all gsets under me,
	    /   /    |     and set the clip texture's clip center)
        geode geode geode ...   (geode pre-node CULL func calculates and applies
	  |     |     |          per-ring clipmap params; also translates
      geoset geoset geoset       texcoordsD array towards origin by a multiple
				 of good area size induced by these params
				 and sets the geoset's texcoords to
				 the translated values in SP).

    Each ring geode contains one geoset, consisting of triangle strips.
    All the ring geosets share common
    verts, normals, and DP texcoordsD, but NOT the
    final SP texcoords computed by each ring's CULL callback;
    also each ring has its own topology (strip lengths and indices).

    The shared arrays and individual topologies&texcoords
    change every frame, so the geosets are fluxed
    (i.e. the geoset for a given geode is changed
    to a different recycled geoset each frame).
    (Actually it's the geodes that are fluxed;
    this turns out to involve less operations per frame.)

    There should be no need to flux the individual attribute
    arrays since a particular geoset is never accessed
    by more than one stage in the pipeline (since the geosets themselves
    are fluxed).  The geosets are marked with PFGS_COMPILE_GL off,
    so that OpenGL display lists will not be compiled since they aren't
    of any use
    (and because there's a bug that makes it so
    these dlists aren't considered dirty when the geoset's attribute arrays
    are changed, so it would come out looking all wrong if they were used).

    The ring geodes are marked PFN_CULL_SORT_CONTAINED so that
    the "per-tile" clipmap params that are applied in their pre-CULL funcs
    will not get scrambled by the sort
    (see the libpfct loader source for more explanation of this).
*/

class ClipRingsNode : public pfuClipCenterNode {

//==================== PUBLIC STUFF ==========================================

public:
    ClipRingsNode(
	int _maxNumRings,
	int _maxNumVerts,
	int _maxNumStripsPerRing,
	int _maxNumIndicesPerRing, // 4*maxNumVerts is safe for quad meshes
	pfTexture *_tex,
	pfGeoState *geostate);

    ~ClipRingsNode();

    //
    // The APP should call this every frame:
    //	   clipRingsNode->reset(...);
    //	   clipRingsNode->addSpherePatchMesh(...);
    //	   clipRingsNode->addSpherePatchMesh(...);
    //	    ...
    //	   clipRingsNode->complete();
    // (XXX could do it in DBASE if we sync the fluxes, I think)
    //
    void reset(
	double _centerS, double _centerT,
	int _nRings, float _smallestRingRadius, float _ringRadiusRatio);

    void complete(float _minLODTexPix); // XXX should be param to reset()?

    void addSpherePatchMesh(
	double lon0, double lon1, double lat0, double lat1,
	double s0, double s1, double t0, double t1,
	int nGlobalLons, int nGlobalLats,
	const double globalLats[/* nGlobalLons */],
	const double globalLons[/* nGlobalLats */],
	double sphereRadius,
	pfVec3d localOrigin); // pass by value so compiler can optimize better

    // conveniences to get back what was set...
    pfTexture *getTex() { return tex; }
    int getMaxNumRings() { return maxNumRings; }

    // class initialization...
    static void init();

//========== END OF PUBLIC STUFF-- YOU'LL PUKE IF YOU LOOK BELOW HERE ========

private:

    //
    // These are set in constructor, and are constant for life of the object...
    //
    int maxNumRings;
    int maxNumVerts;
    int maxNumStripsPerRing;
    int maxNumIndicesPerRing;
    pfTexture *tex;
    pfGeoState *geostate;

    //
    // Taken or computed directly from current reset() parameters...
    //
    double centerS, centerT;
    int nRings;
    float *ringRadii; // float ringRadii[nRings]; (last radius is infinity)

    //
    // Stuff that changes every frame (fluxed)...
    //
    struct FluxData {
	ClipRingsNode *parentClipRingsNode; // non-refcounted back pointer
	pfGeoSet **geosets;	  // pfGeoSet *geosets[maxNumRings];
	pfGeode **geodes;	  // pfGeode *geodes[maxNumRings];
	struct RingPreCullParams { // to be passed down from APP to ring pre-CULL
	    struct FluxData *parentFluxData; // non-refcounted back pointer
	    int indexInParentFluxData; // doesn't change
	    int indexInParentGroup; // can change every frame
	    float texbboxIntentMinDist;
	    float texbboxIntentMaxDist;
	    float texbboxExtentMinDist;
	    float texbboxExtentMaxDist;
	} **ringPreCullParamss;	  // ringPreCullParams *ringPreCullParamss[maxNumRings];
	// the following are shared among all the rings...
	pfVec3 *sharedVerts;	   // pfVec3 sharedVerts[maxNumVerts];
	pfVec3 *sharedNorms;	   // pfVec3 sharedNorms[maxNumVerts];
	pfVec2d *sharedTexCoordsD; // pfVec2d sharedTexCoordsD[maxNumVerts];

	float minLODTexPix;

	int _PFSPHEREPATCH_DEBUG; // to pass app's global down to CULL
	int _PFSPHEREPATCH_NO_PRECISION_HACK; // ditto
	float _PFSPHEREPATCH_TRADEOFF; // ditto

#define PFFLUX_DEBUG
#ifdef PFFLUX_DEBUG
	// For pfFlux sanity testing...
	// All processes (APP, CULL, DRAW) should
	// see the same frame count here.
	int frameCount;
#endif // PFFLUX_DEBUG
    }; // end struct FluxData
    pfFlux *flux; // contains FluxData

    // XXX this is big; maybe move it out somewhere else for readability
    struct PerProcessPreCullData : public pfObject // so gets allocated out of shared arena
    {
	// Constant stuff...
	pfClipTexture *cliptex;
	pfMPClipTexture *mpcliptex; // XXX per process? get this first frame? make sure we get the right one, different for each pipe
	int nLevels, vSize, clipSize;

	// Stuff that changes every frame.
	// We query it once per process per frame in the
	// texture group pre-cull callback,
	// then each ring geode pre-cull callback uses it.
	int invalidBorder;
	float minLODLoaded; // float for fade-in
	float maxLODLoaded;
	pfVirtualClipTexLimits limits;

	/* we might carry these from frame to frame, but probably not */
	float appliedMinLOD;
	float appliedMaxLOD;
	int appliedLODOffset;
	int appliedNumEffectiveLevels;

	/* scratch area so we don't have to reallocate each frame
	       (polytopes don't want to exist on the stack) */
	pfPolytope *ptope;
	pfFrustum *frust;

	PerProcessPreCullData & operator=(const PerProcessPreCullData &from) // no, virtual function override not indended; don't worry about it
	{
	    // Bytewise copy all but frust and ptope...
	    pfFrustum *save_frust = frust;
	    pfPolytope *save_ptope = ptope;
	    memcpy(this, &from, sizeof(PerProcessPreCullData));
	    frust = save_frust;
	    ptope = save_ptope;

	    if (from.frust != NULL) *frust = *from.frust;
	    if (from.ptope != NULL) *ptope = *from.ptope;
	    return *this;
	}

	// Dummy version matching pfMemory's virtual, to prevent cmplr warning.
	pfMemory* operator=(const pfMemory *) {PFASSERTALWAYS(0); return this;}

	PerProcessPreCullData()
	{
	    frust = new pfFrustum();
	    ptope = new pfPolytope();
	    PFASSERTALWAYS(frust != NULL && ptope != NULL);
	    pfRef(ptope);
	    pfRef(frust);
	}
	~PerProcessPreCullData()
	{
	    pfUnrefDelete(ptope);
	    pfUnrefDelete(frust);
	}
    };
    MP<PerProcessPreCullData> *MPperProcessPreCullData;

    //
    // Scratch area, used within a given frame
    // between reset() and complete()...
    //
    struct FluxData *curFluxData;
    int nVerts;
    int *nStripss;	 // nStripss[i] is ring i's number of strips so far
    int *nIndicess;	 // nIndicess[i] is ring i's number of indices so far
    int **stripLengthss; // stripLengthss[i] is ptr to ring i's strip lengths
    ushort **indicess; // indicess[i] is ptr to ring i's indices into verts etc.

    static int preCull(pfTraverser *, void *);
    static int ringPreCull(pfTraverser *trav, void *data);
#ifdef PFFLUX_DEBUG
    static int preDraw(pfTraverser *, void *);
    static int postDraw(pfTraverser *, void *);
#endif // PFFLUX_DEBUG
}; // end ClipRingsNode

static void
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

/* static */ void ClipRingsNode::init()
{
    // There isn't any classType, because I didn't feel like it
    // (getType() returns pfuClipRingsNode's class type).
    // However, there is stuff that needs to be allocated
    // in shared memory:
    if (npfpids == NULL)
    {
        npfpids = (uint32_t *)pfMalloc(sizeof(*npfpids), pfGetSharedArena());
        PFASSERTALWAYS(npfpids != NULL);
        // XXX should also make sure pfConfig() has not been called yet!
        *npfpids = 0;
    }
}

#define _PFCLIPRINGSNODE_LEAK_TEST
#ifdef _PFCLIPRINGSNODE_LEAK_TEST
static void ClipRingsNodeLeakTest(void);
#endif /* _PFCLIPRINGSNODE_LEAK_TEST */

#ifdef PFFLUX_DEBUG
/* static */ int ClipRingsNode::preDraw(pfTraverser *, void *data)
{
    ClipRingsNode::FluxData *curFluxData = (ClipRingsNode::FluxData *)data;
    int fc = curFluxData->frameCount;
    if (fc != pfGetFrameCount())
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "%s preDraw: frame %d: using flux data labeled with frame %d",
		 curFluxData->parentClipRingsNode->tex->getName(),
		 pfGetFrameCount(),
		 fc);
    else
    {
	if (curFluxData->_PFSPHEREPATCH_DEBUG)
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		     "%s preDraw frame %d okay\n",
		     curFluxData->parentClipRingsNode->tex->getName(), fc);
    }
    return PFTRAV_CONT;
}
/* static */ int ClipRingsNode::postDraw(pfTraverser *, void *data)
{
    ClipRingsNode::FluxData *curFluxData = (ClipRingsNode::FluxData *)data;
    int fc = curFluxData->frameCount;
    if (fc != pfGetFrameCount())
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "%s postDraw: frame %d: used flux data labeled with frame %d",
		 curFluxData->parentClipRingsNode->tex->getName(),
		 pfGetFrameCount(),
		 fc);
    else
    {
	if (curFluxData->_PFSPHEREPATCH_DEBUG)
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		     "%s postDraw frame %d okay\n",
		     curFluxData->parentClipRingsNode->tex->getName(), fc);
    }
    return PFTRAV_CONT;
}
#endif // PFFLUX_DEBUG


ClipRingsNode::ClipRingsNode(
    int _maxNumRings,
    int _maxNumVerts,
    int _maxNumStripsPerRing,
    int _maxNumIndicesPerRing, // 4*maxNumVerts is safe for quad meshes
    pfTexture *_tex,
    pfGeoState *_geostate)
{
    PFASSERTALWAYS(_maxNumRings > 0);
    PFASSERTALWAYS(_maxNumVerts > 0);
    PFASSERTALWAYS(_maxNumStripsPerRing > 0);
    PFASSERTALWAYS(_maxNumIndicesPerRing > 0);
    PFASSERTALWAYS(INRANGE(0 <, _maxNumVerts, <= (1<<16))); // indexed by ushort

    maxNumRings = _maxNumRings;
    maxNumVerts = _maxNumVerts;
    maxNumStripsPerRing = _maxNumStripsPerRing;
    maxNumIndicesPerRing = _maxNumIndicesPerRing;
    tex = _tex;
    geostate = _geostate;

    //
    // We are subclassing from a pfuClipCenterNode
    // instead of just a group,
    // so that the texture will get attached
    // to a pipe when the app (e.g. perfly)
    // calls pfuProcessClipCenters.
    // We don't use its centering mechanism,
    // so set the callback to NULL.
    //
    if (tex->isOfType(pfClipTexture::getClassType()))
	setClipTexture((pfClipTexture *)tex); // pfuClipCenterNode's function
    setCallback(NULL); // pfuClipCenterNode's member function

    void *arena = pfGetSharedArena();

    // Allocate global arrays...
    ringRadii = (float *)pfMalloc(maxNumRings * sizeof(*ringRadii), arena);
    nStripss = (int *)pfMalloc(maxNumRings * sizeof(*nStripss), arena);
    nIndicess = (int *)pfMalloc(maxNumRings * sizeof(*nIndicess), arena);
    stripLengthss = (int **)pfMalloc(maxNumRings * sizeof(*stripLengthss), arena);
    indicess = (ushort **)pfMalloc(maxNumRings * sizeof(*indicess), arena);
    PFASSERTALWAYS(ringRadii != NULL);
    PFASSERTALWAYS(nStripss != NULL);
    PFASSERTALWAYS(nIndicess != NULL);
    PFASSERTALWAYS(stripLengthss != NULL);
    PFASSERTALWAYS(indicess != NULL);

    // XXX Flux problems:
    // XXX    1. An obnoxious flux bug makes the CULL get
    // XXX       data that is a frame behind.  The user probably
    // XXX	 only notices this as shaking when in wireframe mode.
    // XXX    2. Flux buffers are initialized to zeros.
    // XXX       This is preventing me from finding UMRs using purify.
    // XXX       Should probably have a custom init function to prevent this.
    // XXX    3. If num buffers increases at runtime,
    // XXX       this program will crash bigtime, since the new
    // XXX       buffer will not be properly initialized!
    // XXX       The custom init function should be made to deal with this too.
    // XXX	 I think this can only happen in FLOAT or LOCK phase,
    // XXX	 if at all.
    pfRef(flux = new pfFlux(sizeof(FluxData), PFFLUX_DEFAULT_NUM_BUFFERS));
    int nBuffers = flux->getNumBuffers(PFFLUX_BUFFERS_GENERATED);
    int iBuffer;
    FOR (iBuffer, nBuffers)
    {
	FluxData *fluxData = (FluxData *)flux->getBufferData(iBuffer);
	fluxData->parentClipRingsNode = this;
	//
	// The rings' attributes are shared
	// across all the rings in a particular buffer...
	//
	pfRef(fluxData->sharedVerts =
	    (pfVec3 *)pfMalloc(maxNumVerts * sizeof(pfVec3), arena));
	pfRef(fluxData->sharedNorms =
	    (pfVec3 *)pfMalloc(maxNumVerts * sizeof(pfVec3), arena));
	pfRef(fluxData->sharedTexCoordsD =
	    (pfVec2d *)pfMalloc(maxNumVerts * sizeof(pfVec2d), arena));
	PFASSERTALWAYS(fluxData->sharedVerts != NULL);
	PFASSERTALWAYS(fluxData->sharedNorms != NULL);
	PFASSERTALWAYS(fluxData->sharedTexCoordsD != NULL);

	pfRef(fluxData->geosets =
	    (pfGeoSet **)pfMalloc(maxNumRings * sizeof(pfGeoSet*), arena));
	pfRef(fluxData->geodes =
	    (pfGeode **)pfMalloc(maxNumRings * sizeof(pfGeode*), arena));
	pfRef(fluxData->ringPreCullParamss =
	    (FluxData::RingPreCullParams **)pfMalloc(maxNumRings * sizeof(FluxData::RingPreCullParams **), arena));
	PFASSERTALWAYS(fluxData->geosets != NULL);
	PFASSERTALWAYS(fluxData->geodes != NULL);
	PFASSERTALWAYS(fluxData->ringPreCullParamss != NULL);

	int iRing;
	FOR (iRing, maxNumRings)
	{
	    pfGeoSet *geoset = new(arena) pfGeoSet();
	    pfGeode *geode = new pfGeode();
	    pfRef(fluxData->geodes[iRing] = geode);
	    pfRef(fluxData->geosets[iRing] = geoset);

	    // Allocate stuff that's individual for this geoset...
	    int *stripLengths = (int *)
		pfMalloc(maxNumStripsPerRing * sizeof(*stripLengths), arena);
	    ushort *indices = (ushort *)
		pfMalloc(maxNumIndicesPerRing * sizeof(*indices), arena);
	    pfVec2 *texCoords = (pfVec2 *)
		pfMalloc(maxNumVerts * sizeof(*texCoords), arena);
	    pfVec4 *colors = (pfVec4 *)pfMalloc(1 * sizeof(*colors), arena);
	    ushort *colorIndices = (ushort *)pfMalloc(1 * sizeof(*colorIndices), arena);

	    PFASSERTALWAYS(stripLengths != NULL);
	    PFASSERTALWAYS(indices != NULL);
	    PFASSERTALWAYS(texCoords != NULL);
	    PFASSERTALWAYS(colors != NULL);
	    PFASSERTALWAYS(colorIndices != NULL);

	    PFSET_VEC4(colors[0], 1,1,1,1);
	    colorIndices[0] = 0;

	    geoset->setPrimType(PFGS_TRISTRIPS);
	    geoset->setNumPrims(0);
	    geoset->setPrimLengths(stripLengths);

	    // Coords, normals, texcoords are shared
	    // among all the big rings in the same buffer...
	    // Color (white or ring debug colors) are individual.
	    geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX,
				fluxData->sharedVerts, indices);
	    geoset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX,
				fluxData->sharedNorms, indices);
	    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX,
				texCoords, indices);
	    geoset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, colorIndices);

	    geoset->setGState(geostate);
	    geode->addGSet(geoset);

	    FluxData::RingPreCullParams *ringPreCullParams = (FluxData::RingPreCullParams *)pfMalloc(sizeof(*ringPreCullParams), arena);
	    PFASSERTALWAYS(ringPreCullParams != NULL);
	    pfRef(fluxData->ringPreCullParamss[iRing] = ringPreCullParams);
	    ringPreCullParams->indexInParentFluxData = iRing;
	    ringPreCullParams->parentFluxData = fluxData;

	    geode->setTravFuncs(PFTRAV_CULL, ringPreCull, NULL);
	    geode->setTravData(PFTRAV_CULL, ringPreCullParams);
	    geode->setTravMode(PFTRAV_CULL, PFN_CULL_SORT,
					    PFN_CULL_SORT_CONTAINED);
			/* see explanation in .ct loader source */
	} // end for iRing
    } // end for iBuffer

    setTravFuncs(PFTRAV_CULL, preCull, NULL);
    setTravData(PFTRAV_CULL, this); // faster than looking up trav->getNode();

    MPperProcessPreCullData = new MP<PerProcessPreCullData>; // XXX memory leak? think about how this is supposed to be deleted
    if (tex->isOfType(pfClipTexture::getClassType()))
    {
	pfClipTexture *cliptex = (pfClipTexture *)tex;
	int vSizeS, vSizeT;
	cliptex->getVirtualSize(&vSizeS, &vSizeT, NULL);
	PFASSERTALWAYS(vSizeS == vSizeT);

	struct PerProcessPreCullData *perProcessCullData =
		MPperProcessPreCullData->getTemplate();
	perProcessCullData->cliptex = cliptex;
	perProcessCullData->mpcliptex = NULL; // XXX get it first frame?
	perProcessCullData->nLevels = log2int(vSizeS)+1;
	perProcessCullData->vSize = vSizeS;
	perProcessCullData->clipSize = cliptex->getClipSize();
	perProcessCullData->appliedNumEffectiveLevels = -1; // i.e. uninitialized

	// the template doesn't need frust or ptope...
	perProcessCullData->frust->unrefDelete();
	perProcessCullData->ptope->unrefDelete();
	perProcessCullData->frust = NULL;
	perProcessCullData->ptope = NULL;
    }
    else
    {
	// XXX this is how the code was in a previous life.
	// XXX need to figure out exactly how everything is supposed
	// XXX to look for non-cliptextures.
	struct PerProcessPreCullData *perProcessCullData = MPperProcessPreCullData->getTemplate();
	perProcessCullData->cliptex = NULL; // this is the signal for the CULL
					  // not to do anything?
					  // XXX should just not call
					  // XXX the callback at all!
    }

    curFluxData = NULL; // denotes we are not between reset() and complete()

#ifdef PFFLUX_DEBUG
    if (getenv("_PFSPHEREPATCH_FLUXDEBUG"))
    {
	setTravFuncs(PFTRAV_DRAW, preDraw, postDraw);
	// every frame, APP func sets DRAW trav data to current flux
    }
#endif // PFFLUX_DEBUG


#ifdef _PFCLIPRINGSNODE_LEAK_TEST
    if (getenv("_PFCLIPRINGSNODE_LEAK_TEST"))
    {
	static int recursing = 0;
	if (!recursing)
	{
	    recursing = 1;
	    ClipRingsNodeLeakTest();
	    recursing = 0;
	}
    }
#endif // _PFCLIPRINGSNODE_LEAK_TEST

} // end ClipRingsNode::ClipRingsNode()

ClipRingsNode::~ClipRingsNode()
{
    pfDelete(ringRadii);
    pfDelete(nStripss);
    pfDelete(nIndicess);
    pfDelete(stripLengthss);
    pfDelete(indicess);

    int nBuffers = flux->getNumBuffers(PFFLUX_BUFFERS_GENERATED);
    int iBuffer;
    FOR (iBuffer, nBuffers)
    {
	FluxData *fluxData = (FluxData *)flux->getBufferData(iBuffer);
	pfUnrefDelete(fluxData->sharedVerts);
	pfUnrefDelete(fluxData->sharedNorms);
	pfUnrefDelete(fluxData->sharedTexCoordsD);
	int iRing;
	FOR (iRing, maxNumRings)
	{
	    // unrefdelete geoset before geode to exercise ref counting...
	    pfUnrefDelete(fluxData->geosets[iRing]);
	    pfUnrefDelete(fluxData->geodes[iRing]);
	    pfUnrefDelete(fluxData->ringPreCullParamss[iRing]);
	}
	pfUnrefDelete(fluxData->geosets);
	pfUnrefDelete(fluxData->geodes);
	pfUnrefDelete(fluxData->ringPreCullParamss);
    }
    flux->unrefDelete();
} // end ClipRingsNode::~ClipRingsNode()


void ClipRingsNode::reset(
    double _centerS, double _centerT,
    int _nRings, float _smallestRingRadius, float _ringRadiusRatio)
{
    PFASSERTALWAYS(curFluxData == NULL);

    //
    // Store current params...
    //
    centerS = _centerS;
    centerT = _centerT;
    nRings = _nRings;
    PFASSERTALWAYS(INRANGE(1 <=, nRings, <= maxNumRings));
    PFASSERTALWAYS(_smallestRingRadius > 0.);
    PFASSERTALWAYS(_ringRadiusRatio > 1.);
    float ringRadius = _smallestRingRadius;
    int iRing = 0;
    FOR (iRing, nRings-1) // all but last
    {
	ringRadii[iRing] = ringRadius;
	ringRadius *= _ringRadiusRatio;
    }
    ringRadii[iRing] = MAXFLOAT; // radius of last ring is infinite

    //
    // Initialize rest of scratch area for this frame...
    //
    curFluxData = (struct FluxData *)flux->getWritableData();
    PFASSERTALWAYS(curFluxData != NULL);
#ifdef PFFLUX_DEBUG
    curFluxData->frameCount = -pfGetFrameCount(); // negative while incomplete
#endif // PFFLUX_DEBUG
    nVerts = 0;
    FOR (iRing, nRings)
    {
	nStripss[iRing] = 0;
	nIndicess[iRing] = 0;
	pfGeoSet *geoset = curFluxData->geosets[iRing];
	stripLengthss[iRing] = geoset->getPrimLengths();
	void *dummy;
	geoset->getAttrLists(PFGS_COORD3, &dummy, &indicess[iRing]);
	FluxData::RingPreCullParams *ringPreCullParams =
	    curFluxData->ringPreCullParamss[iRing];
	ringPreCullParams->texbboxIntentMinDist = iRing==0 ? 0 : ringRadii[iRing-1];
	ringPreCullParams->texbboxIntentMaxDist = ringRadii[iRing];
	ringPreCullParams->texbboxExtentMinDist = 1.; // initialize
	ringPreCullParams->texbboxExtentMaxDist = 0.; // initialize
    }

    //
    // Set the clip center on the pfMPClipTexture...
    //
    pfMPClipTexture *mpcliptex = getMPClipTexture();
    if (mpcliptex != NULL) // only non-NULL if the texture is a clip texture
    {
	int vSizeS, vSizeT;
	mpcliptex->getClipTexture()->getVirtualSize(&vSizeS, &vSizeT, NULL);
	mpcliptex->setCenter((int)(centerS*vSizeS), (int)(centerT*vSizeT), 0);
    }

#ifdef DEBUG
    {
	printf("In ClipRingsNode::reset(centerS=%.17g, centerT=%.17g, nRings=%d, smallestRingRadius=%.9g, ringRadiusRatio=%.9g)\n",
		centerS, centerT,
		nRings, _smallestRingRadius, _ringRadiusRatio);
	printf("Ring radii:");
	FOR (iRing, nRings)
	    printf(" %.9g", ringRadii[iRing]);
	printf("\n");

	// If only I could do this:
	//     printf("Ring radii: %V\n",
        //	      "%.9g", ", ", nRings, ringRadii);
    }
#endif // DEBUG
} // end ClipRingsNode::reset()


void ClipRingsNode::complete(float _minLODTexPix)
{
    PFASSERTALWAYS(curFluxData != NULL); // make sure reset() has been called

    curFluxData->minLODTexPix = _minLODTexPix;
    // Pass down app's globals to CULL...
    curFluxData->_PFSPHEREPATCH_DEBUG = _PFSPHEREPATCH_DEBUG;
    curFluxData->_PFSPHEREPATCH_NO_PRECISION_HACK = _PFSPHEREPATCH_NO_PRECISION_HACK;
    curFluxData->_PFSPHEREPATCH_TRADEOFF = _PFSPHEREPATCH_TRADEOFF;

    //
    // Remove all children from previous frame...
    //
    for (int numChildren = getNumChildren(); numChildren > 0; --numChildren)
	removeChild(getChild(numChildren-1));
    //
    // Add only those geodes which have 1 or more tri strips,
    // to keep the final tree as lean as possible...
    // XXX if it turns out that switching geodes doesn't work,
    // XXX can try switching geosets instead, and
    // XXX leaving all maxNumRings geodes in the graph always.
    //
    int iRing;
    FOR (iRing, nRings)
    {
	if (nStripss[iRing] > 0)
	{
	    struct FluxData::RingPreCullParams *ringPreCullParams =
	        curFluxData->ringPreCullParamss[iRing];
	    ringPreCullParams->indexInParentGroup = getNumChildren();

	    addChild(curFluxData->geodes[iRing]);

	    pfGeoSet *geoset = curFluxData->geosets[iRing];

	    //
	    // Reset everything in the geoset to NULL
	    // to maximize the chance the Performer will
	    // figure out that it can't use an OpenGL display list
	    // XXX it still doesn't figure it out, so we have to run -T0...
	    // XXX need to find out why this is so!  maybe we could
	    // XXX trick it by using 1-buffer-fluxed attribute arrays?)
	    // XXX (figured it out, see below.  most of this can probably
	    // XXX be removed now)
	    //
	    void *texCoords;
	    ushort *dummy;
	    geoset->getAttrLists(PFGS_TEXCOORD2, &texCoords, &dummy);
	    pfRef(texCoords); // maybe not necessary, but it doesn't hurt
	    PFASSERTALWAYS(dummy == indicess[iRing]);

	    pfVec4 *colors;
	    ushort *colorIndices;
	    geoset->getAttrLists(PFGS_COLOR4, (void **)&colors, &colorIndices);
	    pfRef(colors);
	    pfRef(colorIndices);
	    {
		double h=-iRing/3., s=_PFSPHEREPATCH_COLOR_RINGS,v=1.;
		/* h += pfGetFrameCount()/200.; */
		double r,g,b;
		hsv_to_rgb(h,s,v,&r,&g,&b);
		PFSET_VEC4(colors[0], r,g,b,1);
	    }

	    geoset->setNumPrims(0);
	    geoset->setPrimLengths(NULL);
	    geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, NULL, NULL);
	    geoset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX, NULL, NULL);
	    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, NULL, NULL);
	    geoset->setAttr(PFGS_COLOR4, PFGS_OVERALL, NULL, NULL);

	    //
	    // Set geoset attributes back to the arrays...
	    //
	    geoset->setNumPrims(nStripss[iRing]);
	    geoset->setPrimLengths(stripLengthss[iRing]);
	    geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX,
			    curFluxData->sharedVerts, indicess[iRing]);
	    geoset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX,
			    curFluxData->sharedNorms, indicess[iRing]);
	    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX,
			    texCoords, indicess[iRing]);
	    geoset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, colorIndices);
	    pfUnrefDelete(texCoords);
	    pfUnrefDelete(colors);
	    pfUnrefDelete(colorIndices);

	    //
	    // To keep from compiling GL display lists,
	    // turn off the PFGS_COMPILE_GL mode on the geoset.
	    // It doesn't help to do this when the geoset is
	    // created, since perfly subsequently calls a traversal
	    // that turns them on again
	    // (and furthermore there's a bug
	    // that makes it so that our changing the attrs
	    // does not make it dirty the compiled dlist,
	    // which is why I even noticed this!)
	    // so we set the mode here (every frame).
	    //
	    geoset->setDrawMode(PFGS_COMPILE_GL, PF_OFF);
	} // end if it's not empty
    } // end for iRing

#ifdef PFFLUX_DEBUG
    curFluxData->frameCount = pfGetFrameCount();
    setTravData(PFTRAV_DRAW, curFluxData);
#endif // PFFLUX_DEBUG

    flux->writeComplete();
    curFluxData = NULL; // denotes we are not between reset() and complete()

    // _pfPrint(this); XXX
} // end ClipRingsNode::complete()

void ClipRingsNode::addSpherePatchMesh(
    double lon0, double lon1, double lat0, double lat1,
    double s0, double s1, double t0, double t1,
    int nGlobalLons, int nGlobalLats,
    const double globalLons[/* nGlobalLats */],
    const double globalLats[/* nGlobalLons */],
    double sphereRadius,
    pfVec3d localOrigin) // pass by value so compiler can optimize better
{
#ifdef DEBUG
    pfNotify(PFNFY_INFO, PFNFY_PRINT,
	     "In ClipRingsNode::addSpherePatchMesh(lon0=%.17g, lon1=%.17g, lat0=%.17g, lat1=%.17g, s0=%.17g, s1=%.17g, t0=%.17g, t1=%.17g)",
	     lon0, lon1, lat0, lat1, s0, s1, t0, t1);
    pfNotify(PFNFY_INFO, PFNFY_MORE,
	     " Global: lon0 = %.17g, lon1 = %.17g, lat0 = %.17g, lat1 = %.17g",
	     globalLons[0], globalLons[nGlobalLons-1],
	     globalLats[0], globalLats[nGlobalLats-1]);
    pfNotify(PFNFY_INFO, PFNFY_MORE,
	     " Current params: tex=%s, center=%.17g,%.17g, nRings=%d, smallestRingRadius=%.9g",
	     tex->getName(), centerS, centerT, nRings, ringRadii[0]);
#endif // DEBUG

    PFASSERTALWAYS(curFluxData != NULL); // make sure reset() has been called

    //
    // Patch must be oriented correctly, have positive area,
    // and be contained in the global region (caller should
    // guarantee this by clipping if necessary).
    //
    PFASSERTALWAYS(nGlobalLons >= 1);
    PFASSERTALWAYS(nGlobalLats >= 1);
    // XXX something like the following would be appropriate,
    // XXX but the following fails due to roundoff error...
    //PFASSERTALWAYS(INRANGE4(globalLons[0] <=, lon0, <, lon1, <= globalLons[nGlobalLons-1]));
    //PFASSERTALWAYS(INRANGE4(globalLats[0] <=, lat0, <, lat1, <= globalLats[nGlobalLats-1]));

    //
    // Find the bounds of the patch in global mesh indices.
    // XXX this should be binary search,
    // XXX though I doubt this is a bottleneck
    //
    int boundingGlobalLonIndex0 = 0;
    int boundingGlobalLonIndex1 = nGlobalLons-1;
    int boundingGlobalLatIndex0 = 0;
    int boundingGlobalLatIndex1 = nGlobalLats-1;
    while (boundingGlobalLonIndex0+1 < nGlobalLons
	&& lon0 >= globalLons[boundingGlobalLonIndex0+1])
	boundingGlobalLonIndex0++;
    while (boundingGlobalLonIndex1-1 > 0
	&& lon1 <= globalLons[boundingGlobalLonIndex1-1])
	boundingGlobalLonIndex1--;
    while (boundingGlobalLatIndex0+1 < nGlobalLats
	&& lat0 >= globalLats[boundingGlobalLatIndex0+1])
	boundingGlobalLatIndex0++;
    while (boundingGlobalLatIndex1-1 > 0
	&& lat1 <= globalLats[boundingGlobalLatIndex1-1])
	boundingGlobalLatIndex1--;
    PFASSERTALWAYS(INRANGE(0 <=, boundingGlobalLonIndex0, <= nGlobalLons));
    PFASSERTALWAYS(INRANGE(0 <=, boundingGlobalLonIndex1, <= nGlobalLons));
    PFASSERTALWAYS(INRANGE(0 <=, boundingGlobalLatIndex0, <= nGlobalLats));
    PFASSERTALWAYS(INRANGE(0 <=, boundingGlobalLatIndex1, <= nGlobalLats));
    
    int nVertsS = boundingGlobalLonIndex1 - boundingGlobalLonIndex0 + 1;
    int nVertsT = boundingGlobalLatIndex1 - boundingGlobalLatIndex0 + 1;
    if (nVertsS <= 1 || nVertsT <= 1)
	return; // don't bother sending zero-length strips (XXX probably can't happen)

    //
    // Make local arrays of sin and cos values
    // so we don't have to recalculate for each vertex...
    // (XXX do this globally on globalLats, globalLons before we even get here?
    // no, I don't think so... it works better to do it here
    // in the case that the eye is in a big blank area far away
    // from any patch. )
    //
    double *sinLons = (double *)alloca(nVertsS*(int)sizeof(double));
    double *cosLons = (double *)alloca(nVertsS*(int)sizeof(double));
    double *sinLats = (double *)alloca(nVertsT*(int)sizeof(double));
    double *cosLats = (double *)alloca(nVertsT*(int)sizeof(double));
    double *Ss = (double *)alloca(nVertsS*(int)sizeof(double));
    double *Ts = (double *)alloca(nVertsT*(int)sizeof(double));
    PFASSERTALWAYS(sinLons && cosLons && sinLats && cosLats && Ss && Ts);
    {
	int i, j;
	FOR (j, nVertsS)
	{
	    double globalLon = globalLons[boundingGlobalLonIndex0+j];
	    pfSinCosd(globalLon, &sinLons[j], &cosLons[j]);
	    Ss[j] = LERP(s0, s1, FRAC(globalLon, lon0, lon1));
	}
	FOR (i, nVertsT)
	{
	    double globalLat = globalLats[boundingGlobalLatIndex0+i];
	    pfSinCosd(globalLat, &sinLats[i], &cosLats[i]);
	    Ts[i] = LERP(t0, t1, FRAC(globalLat, lat0, lat1));
	}
    }

#ifdef DEBUG
    {
	int i, j;
	printf("Bounding lons & lats of this patch:\n");
	printf("    %d lons:", nVertsS);
	FOR (j, nVertsS)
	    printf(" %.17g", pfArcTan2d(sinLons[j], cosLons[j]));
	printf("\n");
	printf("    %d lats:", nVertsT);
	FOR (i, nVertsT)
	    printf(" %.17g", pfArcTan2d(sinLats[i], cosLats[i]));
	printf("\n");
	printf("    %d Ss:", nVertsS);
	FOR (j, nVertsS)
	    printf(" %.17g", Ss[j]);
	printf("\n");
	printf("    %d Ts:", nVertsT);
	FOR (i, nVertsT)
	    printf(" %.17g", Ts[i]);
	printf("\n");
    }
#endif // DEBUG

    int startIndex = nVerts; // before we add any
    //
    // Fill the sharedVerts, sharedNorms, sharedTexCoordsD arrays,
    // without clamping boundary vertices to patch borders yet
    //
    pfVec3 *sharedVerts = curFluxData->sharedVerts;
    pfVec3 *sharedNorms = curFluxData->sharedNorms;
    pfVec2d *sharedTexCoordsD = curFluxData->sharedTexCoordsD;

#define IND(i,j) (startIndex + (i)*nVertsS + (j))
    PFASSERTALWAYS(nVerts + nVertsS*nVertsT <= maxNumVerts);
    int i, j;
    FOR (i, nVertsT)
    {
	double sinLat = sinLats[i];
	double cosLat = cosLats[i];
	double t = Ts[i];
	FOR (j, nVertsS)
	{
	    double sinLon = sinLons[j];
	    double cosLon = cosLons[j];
	    double s = Ss[j];

	    double x = sinLon * cosLat;
	    double y = -cosLon * cosLat;
	    double z = sinLat;

	    PFSET_VEC3(sharedVerts[nVerts],
		       (x*sphereRadius)-localOrigin[0],
		       (y*sphereRadius)-localOrigin[1],
		       (z*sphereRadius)-localOrigin[2]);
	    PFSET_VEC3(sharedNorms[nVerts], x, y, z);
	    PFSET_VEC2(sharedTexCoordsD[nVerts], s, t);
	    PFASSERTDEBUG(nVerts == IND(i,j));
	    nVerts++;
	}
    }
    PFASSERTALWAYS(nVerts < maxNumVerts);

    //
    // Now take the verts along the bottom, top, and sides
    // and pull them in linearly (bilinearly for the corners)
    // so that we cover the patch exactly
    // (while still conforming exactly to the global geometry).
    // Note that this causes the four corners to be bilinearly interpolated,
    // which is exactly what we want.
    //
    // XXX this will have big mistakes
// if (Fmod(pfGetTime()/5, 1.) > .5) // on for a couple secs, off for a couple secs XXX
    {
#define IND(i,j) (startIndex + (i)*nVertsS + (j))
	double frac;
	// bottom...
	frac = FRAC(t0, Ts[0], Ts[1]); // XXX bad! assumes T is not constant or almost constant
	// PFASSERTALWAYS(INRANGE(0 <=, frac, <= 1)); XXX fails due to roundoff
	Ts[0] = LERP(Ts[0], Ts[1], frac);
	FOR (j, nVertsS)
	{
	    PFCOMBINE_VEC3(sharedVerts[IND(0,j)],
			   1-frac, sharedVerts[IND(0,j)],
			     frac, sharedVerts[IND(1,j)]);
	    PFCOMBINE_VEC3(sharedNorms[IND(0,j)],
			   1-frac, sharedNorms[IND(0,j)],
			     frac, sharedNorms[IND(1,j)]);
	    PFCOMBINE_VEC2(sharedTexCoordsD[IND(0,j)],
			   1-frac, sharedTexCoordsD[IND(0,j)],
			     frac, sharedTexCoordsD[IND(1,j)]);
	}
	// top...
	frac = FRAC(t1, Ts[nVertsT-2], Ts[nVertsT-1]); // XXX bad! assumes T is not constant or almost constant
	// PFASSERTALWAYS(INRANGE(0 <=, frac, <= 1));  XXX fails due to roundoff
	Ts[nVertsT-1] = LERP(Ts[nVertsT-2], Ts[nVertsT-1], frac);
	FOR (j, nVertsS)
	{
	    PFCOMBINE_VEC3(sharedVerts[IND(nVertsT-1,j)],
			   1-frac, sharedVerts[IND(nVertsT-2,j)],
			     frac, sharedVerts[IND(nVertsT-1,j)]);
	    PFCOMBINE_VEC3(sharedNorms[IND(nVertsT-1,j)],
			   1-frac, sharedNorms[IND(nVertsT-2,j)],
			     frac, sharedNorms[IND(nVertsT-1,j)]);
	    PFCOMBINE_VEC2(sharedTexCoordsD[IND(nVertsT-1,j)],
			   1-frac, sharedTexCoordsD[IND(nVertsT-2,j)],
			     frac, sharedTexCoordsD[IND(nVertsT-1,j)]);
	}
	// left...
	frac = FRAC(s0, Ss[0], Ss[1]); // XXX bad! assumes S is not constant or almost constant
	// PFASSERTALWAYS(INRANGE(0 <=, frac, <= 1)); XXX fails due to roundoff
	Ss[0] = LERP(Ss[0], Ss[1], frac);
	FOR (i, nVertsT)
	{
	    PFCOMBINE_VEC3(sharedVerts[IND(i,0)],
			   1-frac, sharedVerts[IND(i,0)],
			     frac, sharedVerts[IND(i,1)]);
	    PFCOMBINE_VEC3(sharedNorms[IND(i,0)],
			   1-frac, sharedNorms[IND(i,0)],
			     frac, sharedNorms[IND(i,1)]);
	    PFCOMBINE_VEC2(sharedTexCoordsD[IND(i,0)],
			   1-frac, sharedTexCoordsD[IND(i,0)],
			     frac, sharedTexCoordsD[IND(i,1)]);
	}
	// right...
	frac = FRAC(s1, Ss[nVertsS-2], Ss[nVertsS-1]); // XXX bad! assumes S is not constant or almost constant
	// PFASSERTALWAYS(INRANGE(0 <=, frac, <= 1)); XXX fails due to roundoff
	Ss[nVertsS-1] = LERP(Ss[nVertsS-2], Ss[nVertsS-1], frac);
	FOR (i, nVertsT)
	{
	    PFCOMBINE_VEC3(sharedVerts[IND(i,nVertsS-1)],
			   1-frac, sharedVerts[IND(i,nVertsS-2)],
			     frac, sharedVerts[IND(i,nVertsS-1)]);
	    PFCOMBINE_VEC3(sharedNorms[IND(i,nVertsS-1)],
			   1-frac, sharedNorms[IND(i,nVertsS-2)],
			     frac, sharedNorms[IND(i,nVertsS-1)]);
	    PFCOMBINE_VEC2(sharedTexCoordsD[IND(i,nVertsS-1)],
			   1-frac, sharedTexCoordsD[IND(i,nVertsS-2)],
			     frac, sharedTexCoordsD[IND(i,nVertsS-1)]);
	}
    }

#ifdef DEBUG
    {
	int i, j;
	printf("Clamped to this patch by bilinear interpolation along the global faces:\n");
	printf("    %d Ss:", nVertsS);
	FOR (j, nVertsS)
	    printf(" %.17g", Ss[j]);
	printf("\n");
	printf("    %d Ts:", nVertsT);
	FOR (i, nVertsT)
	    printf(" %.17g", Ts[i]);
	printf("\n");
    }
#endif // DEBUG

    int numQuadsAdded = 0; // keep a tally for assertion afterwards

    //
    // Carve out strips for the rings, from the center outward
    //
    {
	int S0, T0, S1, T1;  // bounding indices of current ring
	int s0, t0, s1, t1;  // bounding indices of hole (previous smaller ring)

	// Initialize S0,T0 and S1,T1
	// to be any vertex of the mesh quad that contains the
	// clip center or is as close as possible to it...
	for (S0 = 0; S0 < nVertsS-1 && Ss[S0+1] <= centerS; S0++)
	    ;
	S1 = S0;
	for (T0 = 0; T0 < nVertsT-1 && Ts[T0+1] <= centerT; T0++)
	    ;
	T1 = T0;

	int iRing;
	FOR (iRing, nRings) // from innermost outward
	{
	    // This iteration's hole is previous iteration's ring...
	    s0 = S0;
	    t0 = T0;
	    s1 = S1;
	    t1 = T1;

	    //
	    // Expand S0, T0, S1, T1 as far as we can
	    // while staying within the designated ring radius...
	    // (note, be careful not to add or subtract ringRadius
	    // from anything, since it might be MAXFLOAT).
	    //
	    float ringRadius = ringRadii[iRing];
	    while (S0 > 0 && (centerS-Ss[S0-1]) <= ringRadius) S0--;
	    while (T0 > 0 && (centerT-Ts[T0-1]) <= ringRadius) T0--;
	    while (S1 < nVertsS-1 && (Ss[S1+1]-centerS) <= ringRadius) S1++;
	    while (T1 < nVertsT-1 && (Ts[T1+1]-centerT) <= ringRadius) T1++;

#ifdef DEBUG
	    printf("    ring %d: S0=%d S1=%d T0=%d T1=%d  s0=%d s1=%d t0=%d t1=%d\n",
		   iRing, S0, S1, T0, T1, s0, s1, t0, t1);
#endif // DEBUG

	    // Expand bbox to be used by ring pre-cull...
	    struct FluxData::RingPreCullParams *ringPreCullParams = curFluxData->ringPreCullParamss[iRing];
	    ringPreCullParams->texbboxExtentMaxDist =
		PF_MAX5(ringPreCullParams->texbboxExtentMaxDist,
			Ss[S1]-centerS, centerS-Ss[S0],
		        Ts[T1]-centerT, centerT-Ts[T0]);
	    ringPreCullParams->texbboxExtentMinDist =
		PF_MIN5(ringPreCullParams->texbboxExtentMinDist,
		        Ss[s1]-centerS, centerS-Ss[s0],
		        Ts[t1]-centerT, centerT-Ts[t0]);
	    // but don't let it go below zero (not sure whether it matters)
	    ringPreCullParams->texbboxExtentMinDist =
		PF_MAX2(ringPreCullParams->texbboxExtentMinDist, 0.);

	    if (T0 < T1 && S0 < S1) // don't output zero-length strips
	    {
		// cache array elements in local vars to aid optimization...
		int nStrips = nStripss[iRing];
		int ii = nIndicess[iRing];
		int *stripLengths = stripLengthss[iRing];
		ushort *indices = indicess[iRing];

		int i, j;
#define IND(i,j) (startIndex + (i)*nVertsS + (j))
#define ADDSTRIP(i, j0, j1) \
		    { \
			for (j = (j0); j <= (j1); ++j) \
			{ \
			    PFASSERTDEBUG(ii < maxNumIndicesPerRing); \
			    PFASSERTDEBUG(IND((i)+1,j) < nVerts); \
			    indices[ii++] = IND((i)+1,j); \
			    PFASSERTDEBUG(ii < maxNumIndicesPerRing); \
			    PFASSERTDEBUG(IND((i)+0,j) < nVerts); \
			    indices[ii++] = IND((i)+0,j); \
			} \
			PFASSERTDEBUG(nStrips < maxNumStripsPerRing); \
			stripLengths[nStrips++] = 2*((j1)-(j0)+1); \
			numQuadsAdded += (j1)-(j0); \
		    }
		//
		// Add part below the hole...
		//
		for (i = T0; i <= t0-1; ++i)
		{
		    /* printf("        strip of %d quads on bottom\n", S1-S0); */
		    ADDSTRIP(i, S0, S1);
		}
		//
		// Add parts to left and right of hole...
		// Interleave left and right parts
		// for some vague attempt at cache coherency
		//
		for (i = t0; i <= t1-1; ++i)
		{
		    if (S0 < s0) // don't output zero-length strip
		    {
			/* printf("        strip of %d quads on left\n", s0-S0); */
			ADDSTRIP(i, S0, s0);
		    }
		    if (s1 < S1) // don't output zero-length strip
		    {
			/* printf("        strip of %d quads on right\n", S1-s1); */
			ADDSTRIP(i, s1, S1);
		    }
		}
		//
		// Add part above the hole...
		//
		for (i = t1; i <= T1-1; ++i)
		{
		    /* printf("        strip of %d quads on top\n", S1-S0); */
		    ADDSTRIP(i, S0, S1);
		}

		PFASSERTALWAYS(ii < maxNumIndicesPerRing);
		PFASSERTALWAYS(nStrips <= maxNumStripsPerRing);

		// flush cached local values...
		nStripss[iRing] = nStrips;
		nIndicess[iRing] = ii;
	    } // end if 

	    if (S0==0 && S1==nVertsS-1
	     && T0==0 && T1==nVertsT-1)
		break; // did it all
	} // end for iRing
    } // end carving out strips

    PFASSERTALWAYS(numQuadsAdded == (nVertsS-1)*(nVertsT-1));

} // end ClipRingsNode::addSpherePatchMesh()

/* static */ int ClipRingsNode::preCull(pfTraverser *trav, void *arg)
{
    ClipRingsNode *clipRingsNode = (ClipRingsNode *)arg;
    if (clipRingsNode->tex->isOfType(pfClipTexture::getClassType()))
    {
	struct ClipRingsNode::PerProcessPreCullData *data =
	    clipRingsNode->MPperProcessPreCullData->get();
	if (data->mpcliptex == NULL)
	{
	    //
	    // Find it.  It's the per-pipe one,
	    // either a master or a slave.
	    //
	    PFASSERTALWAYS(data->cliptex != NULL);
	    pfChannel *chan = trav->getChan();
	    pfPipe *pipe = chan->getPipe();
	    int n = pipe->getNumMPClipTextures();
	    int i;
	    FOR (i, n)
	    {
		pfMPClipTexture *mpcliptex = pipe->getMPClipTexture(i);
		pfMPClipTexture *master = mpcliptex->getMaster();
		if (master == NULL)
		    master = mpcliptex; // it's the master to begin with
		if (master->getClipTexture() == data->cliptex)
		{
		    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			     "CULL(%d): mpcliptex for %s = 0x%p", get_pid(), data->cliptex->getName(), mpcliptex);
		    data->mpcliptex = mpcliptex;
		    break;
		}
	    }
	    if (i == n)
	    {
		// XXX this should never happen
		pfNotify(PFNFY_WARN, PFNFY_INTERNAL,
			 "CULL(%d): can't find MPClipTexture for ClipTexture 0x%p(%s) -- was pfuProcessClipCenters() not called?\n", get_pid(), data->cliptex, data->cliptex->getName());
		return PFTRAV_CONT;
	    }
	}
	data->invalidBorder = data->mpcliptex->getInvalidBorder();
	data->minLODLoaded = data->cliptex->getMinDTRLOD();
	data->maxLODLoaded = data->cliptex->getNumAllocatedLevels()-1;
	data->mpcliptex->getLODOffsetLimit(
				&data->limits.LODOffset.lo,
				&data->limits.LODOffset.hi);
	data->mpcliptex->getNumEffectiveLevelsLimit(
				&data->limits.numEffectiveLevels.lo,
				&data->limits.numEffectiveLevels.hi);
	data->mpcliptex->getMinLODLimit(
				&data->limits.minLOD.lo,
				&data->limits.minLOD.hi);
	data->mpcliptex->getMaxLODLimit(
				&data->limits.maxLOD.lo,
				&data->limits.maxLOD.hi);

	// Make sure the per-tile optimization
	// doesn't try to remember stuff from last frame...
	data->appliedNumEffectiveLevels = -1;
    }
    else
    {
	// It's not a clip texture.
	// XXX should this function even be registered in this case?
    }
    return PFTRAV_CONT;
} // end ClipRingsNode::preCull()

/* static */ int ClipRingsNode::ringPreCull(pfTraverser *trav, void *arg)
{
    struct FluxData::RingPreCullParams *ringPreCullParams =
	(FluxData::RingPreCullParams *)arg;
    PFASSERTALWAYS(ringPreCullParams != NULL);
    struct FluxData *parentFluxData = ringPreCullParams->parentFluxData;
    PFASSERTALWAYS(parentFluxData != NULL);
    ClipRingsNode *clipRingsNode = parentFluxData->parentClipRingsNode;
    PFASSERTALWAYS(clipRingsNode != NULL);
    struct ClipRingsNode::PerProcessPreCullData *texData =
	clipRingsNode->MPperProcessPreCullData->get();
    PFASSERTALWAYS(texData != NULL);

#ifdef PFFLUX_DEBUG
    if (parentFluxData->frameCount != pfGetFrameCount())
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "CULL frame %d using APP frame %d's flux data\n",
		 pfGetFrameCount(), parentFluxData->frameCount);
    }
    /*
    else
	pfNotify(PFNFY_INFO, PFNFY_PRINT,
		 "cull using correct app frame %d's flux data\n",
		 parentFluxData->frameCount);
    */
#endif // PFFLUX_DEBUG

    double fudgeS = 0., fudgeT = 0.;

    if (texData->cliptex != NULL
     && texData->cliptex->isVirtual())
    {
	float tradeoff = parentFluxData->_PFSPHEREPATCH_TRADEOFF;

	int LODOffset, numEffectiveLevels;
	float minLOD, maxLOD;

	pfuCalcVirtualClipTexParams(
	    texData->nLevels,
	    texData->clipSize,
	    texData->invalidBorder,
	    parentFluxData->minLODTexPix,
	    texData->minLODLoaded,
	    texData->maxLODLoaded,
	    ringPreCullParams->texbboxExtentMinDist,
	    ringPreCullParams->texbboxExtentMaxDist,
	    tradeoff,
	    &texData->limits,

	    &LODOffset,
	    &numEffectiveLevels,
	    &minLOD,
	    &maxLOD);

	if (parentFluxData->_PFSPHEREPATCH_DEBUG)
	{
	    if (ringPreCullParams->indexInParentGroup == 0)
	    {
		pfNotify(PFNFY_NOTICE, PFNFY_MORE, "");
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
			"Constant params: name=%s nLevels=%d clipSize=%d invalidBorder=%d",
			texData->cliptex->getName(),
			texData->nLevels,
			texData->clipSize,
			texData->invalidBorder);
		pfNotify(PFNFY_NOTICE, PFNFY_MORE,
			"Global params: minLODTexPix=%f minLODLoaded=%f maxLODLoaded=%f",
			parentFluxData->minLODTexPix,
			texData->minLODLoaded,
			texData->maxLODLoaded);
	    }
	    pfNotify(PFNFY_NOTICE, PFNFY_MORE,
		    "Ring %d (was %d) params: bboxMinDist=%f bboxMaxDist=%f tradeoff=%f",
		    ringPreCullParams->indexInParentGroup,
		    ringPreCullParams->indexInParentFluxData,
		    ringPreCullParams->texbboxExtentMinDist,
		    ringPreCullParams->texbboxExtentMaxDist,
		    tradeoff);
	    pfNotify(PFNFY_NOTICE, PFNFY_MORE,
		    "        limits: LODOffset=%d..%d effectiveLevels=%d..%d minLOD=%f..%f maxLOD=%f..%f\n",
		    texData->limits.LODOffset.lo,
		    texData->limits.LODOffset.hi,
		    texData->limits.numEffectiveLevels.lo,
		    texData->limits.numEffectiveLevels.hi,
		    texData->limits.minLOD.lo,
		    texData->limits.minLOD.hi,
		    texData->limits.maxLOD.lo,
		    texData->limits.maxLOD.hi);
	    pfNotify(PFNFY_NOTICE, PFNFY_MORE,
		    "       results: LODOffset=%d effectiveLevels=%d minLOD=%f maxLOD=%f",
		    LODOffset,
		    numEffectiveLevels,
		    minLOD,
		    maxLOD);
	}


	//
	// Apply those parameters that changed since last time
	//
	if (texData->appliedNumEffectiveLevels == -1)
	{
	    // first time, or _SPHEREPATCH_SMART_CULLBACKS is off
	    texData->cliptex->applyVirtualParams(LODOffset,numEffectiveLevels);
	    texData->cliptex->applyMinLOD(minLOD);
	    texData->cliptex->applyMaxLOD(maxLOD);
    /*
    pfNotify(PFNFY_WARN, PFNFY_PRINT,
	    "%d frame %d: applying LODOffset=%d, effectiveLevels=%d, minLOD=%f, maxLOD=%f",
		    get_pid(), pfGetFrameCount(),
		    LODOffset, numEffectiveLevels, minLOD, maxLOD);
    */
	}
	else
	{
	    if (LODOffset != texData->appliedLODOffset
	     || numEffectiveLevels != texData->appliedNumEffectiveLevels)
	    {
		texData->cliptex->applyVirtualParams(LODOffset, numEffectiveLevels);
		/* fprintf(stderr, "V"); */
	    }
	    else
	    {
		/* fprintf(stderr, "v"); */
	    }
	    if (minLOD != texData->appliedMinLOD)
	    {
		texData->cliptex->applyMinLOD(minLOD);
		/* fprintf(stderr, "I"); */
	    }
	    else
	    {
		/* fprintf(stderr, "i"); */
	    }
	    if (maxLOD != texData->appliedMaxLOD)
	    {
		texData->cliptex->applyMaxLOD(maxLOD);
		/* fprintf(stderr, "A"); */
	    }
	    else
	    {
		/* fprintf(stderr, "a"); */
	    }
	}
	static int _SPHEREPATCH_SMART_CULLBACKS = -1;
	if (_SPHEREPATCH_SMART_CULLBACKS == -1)
	{
	    const char *e = getenv("_SPHEREPATCH_SMART_CULLBACKS");
	    _SPHEREPATCH_SMART_CULLBACKS = (e ? *e ? atoi(e) : 1 : 0);
	}
	if (_SPHEREPATCH_SMART_CULLBACKS)
	{
	    texData->appliedNumEffectiveLevels = numEffectiveLevels;
	    texData->appliedLODOffset = LODOffset;
	    texData->appliedMinLOD = minLOD;
	    texData->appliedMaxLOD = maxLOD;
	}

	//
	// Hack for precision in huge clipmaps:
	// Translate the texture coords by a multiple of the
	// good area size towards the lower left of the whole virtual clipmap,
	// and pretend the clip center is down there.
	// XXX Non-clipmaps don't shouldn't even be doing anything
	// XXX in the preCull callback...
	// XXX should do this in the APP like for the big rings.
	// XXX And non-virtual clipmaps don't need to do this part anyway...
	//
	{
	    int s, t, r;
	    texData->cliptex->getCurCenter(&s, &t, &r);
	    int goodAreaSize = 1<<(numEffectiveLevels+LODOffset-1);
	    int newS = ((s-(goodAreaSize>>1))&(goodAreaSize-1))+(goodAreaSize>>1);
	    int newT = ((t-(goodAreaSize>>1))&(goodAreaSize-1))+(goodAreaSize>>1);
	    newS = PF_MIN2(newS, s); // don't increase
	    newT = PF_MIN2(newT, t); // don't increase

	    if (parentFluxData->_PFSPHEREPATCH_NO_PRECISION_HACK) // XXX put this further above?
	    {
		 newS = s;
		 newT = t;
	    }

	    fudgeS = (double)(newS - s) / texData->vSize;
	    fudgeT = (double)(newT - t) / texData->vSize;

	    texData->cliptex->applyCenter(newS, newT, r); // override real center
	    // XXX should only do that if numEffectiveLevels or LODOffset changed?
	}
    } // end if it's a virtual clip texture.
    else
    {
	// It's not a virtual clip texture (maybe even not even a texture)...
	// nothing to do there.
    }

    //
    // Set tcoords from tcoordsD, with huge-clipmap-precision fudge applied
    // if appropriate.
    //
    {
	pfVec2d *tcoordsD = parentFluxData->sharedTexCoordsD;
	// XXX probably ony do this in master pipe? different for flux and cyclebuffer?
	pfVec2 *tcoords;

	pfGeode *geode = (pfGeode *)trav->getNode();
	PFASSERTALWAYS(geode->isOfType(pfGeode::getClassType()));
	pfGeoSet *geoset = geode->getGSet(0);
	ushort *indices;
	int nStrips = geoset->getNumPrims();
	int *stripLengths = geoset->getPrimLengths();
	geoset->getAttrLists(PFGS_TEXCOORD2, (void **)&tcoords, &indices);

	// Figure out how many indices there are...
	int nIndices = 0;
	{
	    int iStrip;
	    FOR (iStrip, nStrips)
		nIndices += stripLengths[iStrip];
	    // XXX in general, if there are less vertices than indices,
	    // XXX should iterate through those instead!
	}

	int iIndex;
/*
printf("RingGeode %d tcoords were:\n", ringGeodeParams->index);
FOR (iIndex, nIndices)
    printf("    %.17g,%.17g\n",  tcoordsD[indices[iIndex]][0], tcoordsD[indices[iIndex]][1]);
*/
	// Translate the texcoords referred to by the indices.
	FOR (iIndex, nIndices)
	{
	    int index = indices[iIndex];
	    tcoords[index][0] = tcoordsD[index][0] + fudgeS;
	    tcoords[index][1] = tcoordsD[index][1] + fudgeT;
	}
/*
printf("RingGeode %d tcoords are:\n", ringGeodeParams->index);
FOR (iIndex, nIndices)
    printf("    %.9g,%.9g\n",  tcoords[indices[iIndex]][0], tcoords[indices[iIndex]][1]);
*/
    }
    return PFTRAV_CONT;
} // end ClipRingsNode::ringPreCull()


#ifdef _PFCLIPRINGSNODE_LEAK_TEST
static void ClipRingsNodeLeakTest(void)
{
    // Flush out the "Using 72Hz video rate" message...
    pfFrame();
    pfFrame();
    pfFrame();

    printf("Testing ClipRingsNode, please wait...\n");

    int maxNumRings = 32;
    int maxNumVerts = 101*101;
    int maxNumStripsPerRing = 100*2;
    int maxNumIndicesPerRing = 4*maxNumVerts;
    pfClipTexture *cliptex = new pfClipTexture();
	// XXX should try with regular texture too
    int vSize = 1<<25;
    cliptex->setVirtualSize(vSize, vSize, 1);

    pfGeoState *geostate = new(pfGetSharedArena()) pfGeoState();

    cliptex->ref();
    geostate->ref();

    int nIterations = 100;
    nIterations = 5;
    int iIteration;
    FOR (iIteration, nIterations)
    {
	printf("    ");
	ClipRingsNode *clipRingsNode = new ClipRingsNode(
	    maxNumRings,
	    maxNumVerts,
	    maxNumStripsPerRing,
	    maxNumIndicesPerRing,
	    cliptex,
	    geostate);

	int nFrames = 10;
	int iFrame;
	FOR (iFrame, nFrames)
	{
	    printf(" "); fflush(stdout);

	    double centerS = 1/3.;
	    double centerT = 1/3.;
	    int nRings = 16;
	    float smallestRingRadius = 128./vSize;
	    float ringRadiusRatio = 2.;
	    clipRingsNode->reset(centerS, centerT,
				 nRings, smallestRingRadius, ringRadiusRatio);
	    int nPatches = 3;
	    int iPatch;
	    FOR (iPatch, nPatches)
	    {
		#define nVertsS 31
		#define nVertsT 20
		double lons[nVertsS];
		double lats[nVertsT];
		pfVec3d localOrigin(1e9+1, 1e9+1, 1e9+1);

		// XXX should try to set these to something reasonable
		memset(lons, '\0', sizeof(lons));
		memset(lats, '\0', sizeof(lats));
		double sphereRadius = 5.;

		clipRingsNode->addSpherePatchMesh(
		    -180, 180, -90, 90,
		    0, 1, 0, 1,
		    nVertsS, nVertsT,
		    lons,
		    lats,
		    sphereRadius,
		    localOrigin);
		printf("."); fflush(stdout);
	    }
	    clipRingsNode->complete(0.);
	}


	pfGroup *group = new pfGroup();
	group->addChild(clipRingsNode);

#ifdef SHOW_PFBUFFER_PF_PREPARENEWUPDATABLES_BUG
	group->ref();
	group->unrefDelete(); // XXX hmm... bad to do this if updatable list hasn't been processed? this is weird... screws up when running purified
#else
	pfDelete(group);
#endif

	pfFrame();
	pfFrame();
	pfFrame();
	pfFrame();
	pfFrame();
	pfFrame();
	pfFrame();
	pfFrame();
	pfFrame();
	pfFrame();

	printf(" Hit return to continue: ");
	getchar();
	//printf("\n");
    }

    geostate->unrefDelete();
    cliptex->unrefDelete();

    printf("Done testing ClipRingsNode.\n");

    printf("But since I called pfFrame a bunch of times I must die now.\n");
    exit(1);
} // ClipRingsNodeLeakTest()
#endif // _PFCLIPRINGSNODE_LEAK_TEST

#endif // __CLIPRINGSNODE_H__
