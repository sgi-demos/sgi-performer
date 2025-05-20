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
 * pfclosest.c
 *
 * Pseudo-loader that loads a scene graph from
 * from a file of any format and adds a callback
 * to highlight the closest point
 * (testing the pfutil function pfuGetClosestPoint()).
 *
 * By default, the closest point in the scene is highlighed during each frame.
 * If the environment variable PFCLOSEST_TEXTURE_I is set to an integer i,
 * the search for closest point is limited to the triangles textured
 * by the i'th texture encountered when traversing the scene graph.
 *
 * For example, to highlight the closest point in the scene file
 * esprit.flt, say:
 	perfly esprit.flt.closest
 * To highlight the closest point on the hubcaps, say:
	setenv PFCLOSEST_TEXTURE_I 0
 	perfly esprit.flt.closest
 * To highlight the closest point on the license plate, say:
	setenv PFCLOSEST_TEXTURE_I 1
 	perfly esprit.flt.closest
 *
 * Use the following to see a list of possible values
 * of PFCLOSEST_TEXTURE_I with corresponding texture names, say:
  	setenv PFCLOSEST_TEXTURE_I -1
	setenv PFNFYLEVEL 5
	perfly esprit.flt.closest
 *
 * ==========================================================================
 *
 * To instead highlight a point that is a frontward bound of the
 * intersection of the bounding sphere with the view frustum
 * (to be used in conjunction with perfly's "Cull View FOV" and the z key;
 * you'll want to turn off culling so you can see the ball)
 * say:
        setenv PFCLOSEST_FRUSTUM_INTERSECT_SPHERE
 * To approximate the view frustum with a cone, instead say:
        setenv PFCLOSEST_CONE_INTERSECT_SPHERE

 * Or to find the point on the object nearest to the eye plane:
	setenv PFCLOSEST_FRUSTUM_INTERSECT
 * (this mode also looks at PFCLOSEST_TEXTURE_I, as above)
 * 
 * WARNING: Do not use this mode unless you know the culling polytope
 * is a frustum (as it always is in perfly).
 *
 * (XXX should turn culling off explicitly for the ball)
 *
 * XXX notes:
 *	This traversal is kind of ridiculous since it clips every triangle
 *	in the entire tree and then throws away the result!
 *	Certainly this routine could benefit from the culler
 *	    (make it a post-cull function)
 *	and vice-versa (this function gives a more aggressive cull result).
 *
 * ==========================================================================
 *
 * With either of the above methods, the update can be made to
 * happen every n frames (so that once the closest point is highlighted,
 * the scene may be rotated and examined for a period of time before it is
 * updated again).  For example, to update every 120 frames,
  	setenv PFCLOSEST_UPDATE_FREQUENCY 120
 */

#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
/*
#include <Performer/pf/pfGroup.h>
#include <Performer/pr/pfTexture.h>
*/
#include <stdlib.h>
#include <assert.h>
#ifdef __linux__
#include <limits.h>
#endif

#ifdef WIN32
#include <windows.h>
//#include "wintypes.h"

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */


static int _tex_is_in_array(pfTexture *tex, pfTexture **texs, int n)
{
    int i;
    for (i = 0; i < n; ++i)
	if (texs[i] == tex)
	    return 1;
    return 0;
}

struct findTextureTraverser: public pfuTraverser {
    int n;		// input-- find the n'th texture in the graph
    int i;		// local-- number of textures seen so far
    pfTexture **texs;	// local-- textures seen so far
    pfTexture *tex;	// output-- the desired texture

    findTextureTraverser(int n)
	: n(n < 0 ? 1000 : n), i(0), tex(NULL)
    {
	if (n < 0)
	{
	    /*
	     * We won't find it, but traverse everything anyway
	     * so that the user can see what textures are available
	     * if in debug mode...
	     */
	    this->n = 1000;
	}

	pfuInitTraverser(this);
	preFunc = _find_texture;
	texs = new pfTexture* [this->n];
	if (texs == NULL)
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		     "pfdLoadFile_substclip: can't allocate %d pfTexture*'s",
		     this->n);
    }
    int bad() { return texs == NULL; }	// whether constructor failed
    ~findTextureTraverser()
    {
	delete[] texs; // okay if NULL
    }

    static int _find_texture(pfuTraverser *_trav)
    {
	struct findTextureTraverser *trav = (findTextureTraverser *)_trav;

	if (trav->node->isOfType(pfGeode::getClassType()))
	{
	    pfGeode *geode = (pfGeode *)trav->node;
	    int nGSets = geode->getNumGSets();
	    for (int i = 0; i < nGSets; ++i)
	    {
		pfGeoSet *gset = geode->getGSet(i);
		pfGeoState *gstate = gset->getGState();
		if (gstate != NULL
		 && gstate->getMode(PFSTATE_ENTEXTURE))
		{
		    pfTexture *tex = (pfTexture *)
			gstate->getAttr(PFSTATE_TEXTURE);
		    if (tex != NULL && !_tex_is_in_array(tex, trav->texs, trav->i))
		    {
			if (trav->i == trav->n)
			{
			    trav->tex = tex;
			    return PFTRAV_TERM;
			}
			else
			{
			    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
				     "Texture #%d: %s(0x%p)\n", trav->i,
				     tex->getName(),
				     (void *)tex);
			    trav->texs[trav->i++] = tex;
			}
		    }
		}
	    }
	}
	return PFTRAV_CONT;
    }
};

/*
 * Find the n'th texture in the scene graph
 * and return the corresponding geostate and containing geode.
 * XXX there's now a pfu utility that does this... need to convert to using it
 * XXX (currently, this one is better, I think)
 */
static pfTexture *
pfuFindTexture(pfNode *node, int n)
{
    findTextureTraverser trav(n);
    if (trav.bad())
	return NULL;

    pfuTraverse(node, &trav);
    return trav.tex;
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
#define PFFRUST_LEFT	0
#define PFFRUST_RIGHT	1
#define PFFRUST_NEAR	2
#define PFFRUST_FAR	3
#define PFFRUST_BOTTOM	4    
#define PFFRUST_TOP	5
static void
calcViewFrustumFromCullPtope(const pfPolytope *ptope, pfFrustum *frust)
{
    assert(ptope->getNumFacets() == 6);
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

    if (facets[PFFRUST_LEFT].normal[PF_Y] < -.01) /* sine of 1/2 degree */
    {
	frust->setNearFar(mynear, myfar); /* must come before makePersp */
	frust->makePersp(left, right, bottom, top);
    }
    else
    {
	/* XXX not tested yet */
	pfNotify(PFNFY_DEBUG, PFNFY_USAGE,
		 "closest loader: orthogonal frusta have not been tested");
	frust->setNearFar(mynear, myfar);
	frust->makeOrtho(left, right, bottom, top);
    }
}

// Possible modes...
#define FIND_CONE_INTERSECT_SPHERE 0
#define FIND_FRUSTUM_INTERSECT_SPHERE 1
#define FIND_FRUSTUM_INTERSECT_TRIANGLES 2
#define FIND_CLOSEST_POINT_TO_EYE 3

struct Data {
    int frequency;	/* update every this-many frames */
    int mode;
    pfTexture *tex; /* only used when doing pfuGetClosestPoint */

    /* scratch area so we don't have to reallocate each frame
       (polytopes don't want to exist on the stack) */
    pfPolytope *ptope;
    pfFrustum *frust;
};

/*
 * post-app callback to adjust the size and position of the little red sphere
 */
static int
highlightClosestPoint(pfTraverser *trav, void *_data)
{
    struct Data *data = (struct Data *)_data;
    int framecount = pfGetFrameCount();
    if (framecount % data->frequency != 0)
	return PFTRAV_CONT;
    pfTexture *tex = data->tex;
    pfGroup *parent = (pfGroup *)trav->getNode();
    pfNode *child = parent->getChild(0);
    pfDCS *dcs = (pfDCS *)parent->getChild(1);
    int travmode = PFUTRAV_SW_CUR | PFUTRAV_SEQ_CUR
		 | PFUTRAV_LOD_RANGE0 | PFUTRAV_LAYER_BOTH;
	/* XXX would really like PFUTRAV_LOD_CUR but it doesn't exist */

    /*
     * There is a single sphere under the dcs.
     * Scale it so that it is 1/100 the size of the
     * bounding sphere of child,
     * and translate it so that it is centered at the
     * point in child closest to the eye
     * (if the environment variable PFCLOSEST_TEXTURE_I is set to an integer i,
     * limit the search for closest point to the triangles textured
     * by the i'th texture encountered).
     */

    pfSphere bsph;
    child->getBound(&bsph);
    parent->setBound(&bsph, PFBOUND_STATIC); // so red ball doesn't affect it

    pfVec3 eye(0,0,0);
    pfMatrix viewmat, travmat;
    trav->getChan()->getViewMat(viewmat);
    trav->getMat(travmat);
    pfMatrix invtravmat;
    invtravmat.invertOrtho(travmat);
    pfMatrix eyespace_to_objspace_mat = viewmat;
    eyespace_to_objspace_mat.postMult(invtravmat);

#ifdef DEBUG
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Channel matrix:");
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t%g %g %g %g",
	    viewmat[0][0],viewmat[0][1],viewmat[0][2],viewmat[0][3]);
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t%g %g %g %g",
	    viewmat[1][0],viewmat[1][1],viewmat[1][2],viewmat[1][3]);
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t%g %g %g %g",
	    viewmat[2][0],viewmat[2][1],viewmat[2][2],viewmat[2][3]);
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t%g %g %g %g",
	    viewmat[3][0],viewmat[3][1],viewmat[3][2],viewmat[3][3]);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Traverser matrix:");
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t%g %g %g %g",
	    travmat[0][0],travmat[0][1],travmat[0][2],travmat[0][3]);
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t%g %g %g %g",
	    travmat[1][0],travmat[1][1],travmat[1][2],travmat[1][3]);
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t%g %g %g %g",
	    travmat[2][0],travmat[2][1],travmat[2][2],travmat[2][3]);
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "\t%g %g %g %g",
	    travmat[3][0],travmat[3][1],travmat[3][2],travmat[3][3]);
#endif
    eye.xformPt(eye, eyespace_to_objspace_mat);

    switch (data->mode)
    {
	case FIND_CLOSEST_POINT_TO_EYE:
	{
	    float x,y,z,s,t,r;
	    if (pfuGetClosestPoint(child, eye[0], eye[1], eye[2], tex,
				   travmode,
				   &x, &y, &z, &s, &t, &r))
	    {
		/* duplicate code below */
		dcs->setScale(bsph.radius*.01,bsph.radius*.01,bsph.radius*.01);
		dcs->setTrans(x, y, z);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "highlightClosestPoint:");
		pfNotify(PFNFY_DEBUG, PFNFY_MORE,"        v=(%g %g %g)", x,y,z);
		if (tex != NULL)
		    pfNotify(PFNFY_DEBUG, PFNFY_MORE,
			     "        t=(%g %g %g)", s,t,r);
	    }
	    else
	    {
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
			 "highlightClosestPoint: no closest point?\n");
		/* just leave it like it was */
	    }
	    break;
	}
	case FIND_CONE_INTERSECT_SPHERE:
	case FIND_FRUSTUM_INTERSECT_SPHERE:
	case FIND_FRUSTUM_INTERSECT_TRIANGLES:
	{
	    /* essentially local variables; they're kept around
	       so we don't have to malloc each frame */
	    pfPolytope *ptope = data->ptope;
	    pfFrustum *frust = data->frust;
	    trav->getChan()->getCullPtope(ptope, PF_EYESPACE);
	    calcViewFrustumFromCullPtope(ptope, frust);

	    //
	    // For a confidence test of recommendNear(),
	    // we should do this in both eye space
	    // and object space, and verify that the answers
	    // are the same
	    // XXX do this
	    // XXX (the problem is that the current implmentation
	    // XXX of recommendNear is discontinuous,
	    // XXX so near the discontinuity the answers
	    // XXX returned might be different
	    //

	    if (data->mode == FIND_FRUSTUM_INTERSECT_TRIANGLES)
	    {
		// Transform frustum into world space...
		frust->orthoXform(frust, eyespace_to_objspace_mat);
		    // XXX this isn't really world space; it would be better
		    // XXX to really do it in world space since
		    // XXX then nonuniform transformations would
		    // XXX be handled correctly

		float x,y,z,s,t,r;
		if (pfuGetNearestVisiblePointToEyePlane(child, frust, tex,
				    travmode,
				    &x, &y, &z, &s, &t, &r))
		{
		    /* duplicate code above */
		    dcs->setScale(bsph.radius*.01,bsph.radius*.01,bsph.radius*.01);
		    dcs->setTrans(x, y, z);
		    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "highlightClosestPoint:");
		    pfNotify(PFNFY_DEBUG, PFNFY_MORE,"        v=(%g %g %g)", x,y,z);
		    if (tex != NULL)
			pfNotify(PFNFY_DEBUG, PFNFY_MORE,
				 "        t=(%g %g %g)", s,t,r);

		}
	    }
	    else /* data->mode == FIND_FRUSTUM_INTERSECT_SPHERE or FIND_CONE_INTERSECT_SPHERE */
	    {
		//
		// Transform the bounding sphere from object space into eye space
		//
		pfMatrix objspace_to_eyespace_mat;
		objspace_to_eyespace_mat.invertOrtho(eyespace_to_objspace_mat);
		pfSphere bsphInEyeSpace;
		bsphInEyeSpace.orthoXform(&bsph, objspace_to_eyespace_mat);

		pfVec3 frontwardBound;
		frust->recommendNear(&bsphInEyeSpace, frontwardBound,
				     data->mode == FIND_CONE_INTERSECT_SPHERE);
		//
		// Transform the point we got back into object space
		//
		frontwardBound.xformPt(frontwardBound, eyespace_to_objspace_mat);

		dcs->setScale(bsph.radius*.01, bsph.radius*.01, bsph.radius*.01);
		dcs->setTrans(frontwardBound[0],
			      frontwardBound[1],
			      frontwardBound[2]);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "highlightClosestPoint(calculated in eye space):");
		pfNotify(PFNFY_DEBUG, PFNFY_MORE, "        v=(%g %g %g)",
			  frontwardBound[0], frontwardBound[1], frontwardBound[2]);
	    }
#if 0 // calcViewFrustumFromPolytope doesn't work unless in eye space
	    {
		// Now do it in object space
		frust->orthoXform(frust, eyespace_to_objspace_mat);
		pfVec3 altFrontwardBound;
		frust->recommendNear(&bsph, altFrontwardBound);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "highlightClosestPoint(calculated in object space):");
		pfNotify(PFNFY_DEBUG, PFNFY_MORE, "        v=(%g %g %g)",
		  altFrontwardBound[0], altFrontwardBound[1], altFrontwardBound[2]);
	    }
#endif
	    break;
	}
    }

    return PFTRAV_CONT;
}

extern "C" PFPFB_DLLEXPORT pfNode *
pfdLoadFile_closest(char *fileName)
{
    char *lastdot = strrchr(fileName, '.');
    if (lastdot == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		"pfdLoadFile_closest: bad file name %s", fileName);
	return NULL;
    }
    char realName[PF_MAXSTRING];
    strncpy(realName, fileName, lastdot-fileName);
    realName[lastdot-fileName] = '\0';

    pfNode *child = pfdLoadFile(realName);
    if (child == NULL)
	return NULL; /* error message printed already */

    pfGroup *parent = new pfGroup();
    parent->addChild(child);

    pfDCS *dcs = new pfDCS();

    pfGeode *geode = new pfGeode();
    int subdiv = 2;
    pfGeoSet *sphere = pfdNewSphere(subdiv*subdiv * 6*2, pfGetSharedArena());
    pfGeoState *gstate = new pfGeoState();
    gstate->setMode(PFSTATE_ENTEXTURE, PF_OFF);
    gstate->setMode(PFSTATE_TRANSPARENCY, PF_ON);
    sphere->setGState(gstate);
    pfVec4 *transparentred = new pfVec4(1., .25, .25, .5);
    sphere->setAttr(PFGS_COLOR4, PFGS_OVERALL, transparentred, NULL);
    geode->addGSet(sphere);

    dcs->addChild(geode);
    parent->addChild(dcs);

    struct Data *data = (struct Data *)pfMalloc(sizeof(*data),
						pfGetSharedArena());
    assert(data != NULL);
    char *e = getenv("PFCLOSEST_FREQUENCY");
    data->frequency = (e ? *e ? atoi(e) : 1 : 1);
    data->frust = new pfFrustum();
    data->ptope = new pfPolytope();
    assert(data->frust != NULL && data->ptope != NULL);

    if (getenv("PFCLOSEST_FRUSTUM_INTERSECT_SPHERE"))
	data->mode = FIND_FRUSTUM_INTERSECT_SPHERE;
    else if (getenv("PFCLOSEST_FRUSTUM_INTERSECT"))
	data->mode = FIND_FRUSTUM_INTERSECT_TRIANGLES;
    else if (getenv("PFCLOSEST_CONE_INTERSECT_SPHERE"))
	data->mode = FIND_CONE_INTERSECT_SPHERE;
    else /* original default mode that we know and love */
	data->mode = FIND_CLOSEST_POINT_TO_EYE;

    {
	pfTexture *tex;
	e = getenv("PFCLOSEST_TEXTURE_I");
	int PFCLOSEST_TEXTURE_I = (e ? *e ? atoi(e) : 0 : -1);
	if (PFCLOSEST_TEXTURE_I >= 0)
	{
	    tex = pfuFindTexture(child, PFCLOSEST_TEXTURE_I);
	    if (tex == NULL)
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "highlightClosestPoint: no texture with index %d\n",
			 PFCLOSEST_TEXTURE_I);
	}
	else
	    tex = NULL;
	data->tex = tex;
    }

    parent->setTravFuncs(PFTRAV_APP, NULL, highlightClosestPoint);
    parent->setTravData(PFTRAV_APP, data);

    return parent;
}
