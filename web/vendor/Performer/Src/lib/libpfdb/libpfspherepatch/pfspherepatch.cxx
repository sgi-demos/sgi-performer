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
 * pfspherepatch.c - textured sphere patch loader.
 * 
 * $Revision: 1.21 $ 
 * $Date: 2002/11/07 04:02:08 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#include <windows.h>
#include <limits.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#define MAXFLOAT FLT_MAX
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

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


#define FOR(i,n) for ((i) = 0; (i) < (n); (i)++)
#define LERP(a,b,t) ((a) + (t)*((b)-(a)))
#define BIT(x,i) (((x)>>(i))&1)
#define SETBIT(x,i) ((x)|=(1<<(i)))
#define SWAP(a,b,temp) ((temp)=(a),(a)=(b),(b)=(temp))
#define SIGN(x) ((x)>0?1:(x)<0?-1:0)
#define ISINT(x, tol) (fabs((x)-round(x)) < (tol))
#define INRANGE(foo,bar,baz) ((foo(bar))&&((bar)baz))
#define numberof(things) (sizeof(things)/sizeof(*(things)))
#define streq(a,b) (strcmp(a,b) == 0)
#define log2(x) (log(x)*M_LOG2E)
#define log2int(x) ((int)round(log2(x)))/* could make this faster if critical */
#define round(x) floor((x)+.5)


/*
    Example:
            #
            #This file contains two patches.
            #
            # All variables must be set for each patch.
            # Within the definition of a single patch,
            # the parameter settings can be in any order.
            # Parameter settings must be one per line.
            # Blanks lines and lines beginning with '#' are ignored.
            # White space is ignored.
	    # Fractions of the form numerator/denominator
	    # are allowed for the s,t,lat,lon params.
             #
            tex=allearthW.ct
            lon0 =-180
            lat0= -90
            s0 = 0
            t0 = 0
            lon1 = 0
            lat1 = 90.0
            s1 = 1
            t1 = 1
            lonsegs = 72
            latsegs = 72
            # 

            tex = allearthE.ct
            lon0 = 0
            lat0 = -90
            s0 = 0
            t0 = 0
            lon1 = 180
            lat1 = 90
            s1 = 10/ 10
            t1 = 1
            lonsegs = 72
            latsegs = 72

    The structure of the resulting scene graph is:
	group
	    clipmap 0 group  (post-node APP func sets per-frame clipmap params)
		patch 00 geode (pre-node CULL func calculates and applies
				per-tile clipmap params)
		patch 01 geode
		patch 02 geode
		...
	    clipmap 1 group
		patch 10 geode
		patch 11 geode
		patch 12 geode
		...
	    ...
*/

static pfNode *
makeGlobePatch(double lon0, double lat0, double s0, double t0,
               double lon1, double lat1, double s1, double t1,
               int lonsegs, int latsegs,
               pfTexture *tex);
static void stitchAll(pfGroup *bigGroup);
static int texGroupPostApp(pfTraverser *trav, void *data);
#define PRECISION_HACK
#ifdef PRECISION_HACK
static int texGroupPreCull(pfTraverser *trav, void *data);
#endif /* PRECISION_HACK */
static int intersectArcs(double a0, double a1,
			 double b0, double b1,
			 double *result0, double *result1);
static double arcIntersectionLength(double a0, double a1,
				    double b0, double b1);

struct PatchParams
{
    double lon0, lat0, s0, t0;
    double lon1, lat1, s1, t1;
    int lonsegs, latsegs;
#ifdef PRECISION_HACK
    double (*double_texcoords)[2];
#endif /* PRECISION_HACK */
};
struct InputPatchParams : public PatchParams // PatchParams with an extra...
{
    char texname[1024];
};
struct {const char *name, *fmt; int off;} variables[] =
{
    {"tex", "%s", offsetof(InputPatchParams, texname)},
    {"lon0", "%lf", offsetof(InputPatchParams, lon0)},
    {"lat0", "%lf", offsetof(InputPatchParams, lat0)},
    {"lon1", "%lf", offsetof(InputPatchParams, lon1)},
    {"lat1", "%lf", offsetof(InputPatchParams, lat1)},
    {"s0", "%lf", offsetof(InputPatchParams, s0)},
    {"t0", "%lf", offsetof(InputPatchParams, t0)},
    {"s1", "%lf", offsetof(InputPatchParams, s1)},
    {"t1", "%lf", offsetof(InputPatchParams, t1)},
    {"lonsegs", "%d", offsetof(InputPatchParams, lonsegs)},
    {"latsegs", "%d", offsetof(InputPatchParams, latsegs)},
};

extern "C" pfNode *
pfdLoadFile_spherepatch(char *fileName)
{
    FILE *fp = pfdOpenFile(fileName);
    if (fp == NULL)
        return NULL; // XXX prints error message?

    pfGroup *group = new pfGroup;
    PFASSERTALWAYS(group != NULL);

    char linebuf[1024];
    struct InputPatchParams params;
    PFASSERTALWAYS(sizeof(params.texname) >= sizeof(linebuf)); // %s into it
    PFASSERTALWAYS(numberof(variables) < 32); // strictly less
    uint32_t which_params_got = 0;
    int lineNum = 0;
    int numClipTextures = 0;

    while (fgets(linebuf, sizeof(linebuf), fp) != NULL)
    {
        lineNum++;
        char *p = linebuf;
        while (*p != '\0' && isspace(*p)) p++; // skip spaces
        if (*p == '#' || *p == '\0')
            continue;   // blank line or comment
        int i;
        FOR (i, numberof(variables))
        {
            char fmt[100];
            sprintf(fmt, " %s = %s", variables[i].name, variables[i].fmt);
            if (sscanf(p, fmt, (void *)((char *)&params + variables[i].off))==1)
                if (!BIT(which_params_got, i))
                {
                    SETBIT(which_params_got, i);
		    //
		    // Hack to make life a little easier:
		    // for those params that are doubles,
		    // allow explicit fractions (numerator / denominator)
		    //
		    if (streq(variables[i].fmt, "%lf"))
		    {
			double numerator, denominator;
			strcat(fmt, " / %lf");
			if (sscanf(p, fmt, &numerator, &denominator)==2)
			{
			    PFASSERTALWAYS(denominator != 0.);
			    *(double *)((char *)&params + variables[i].off)
				= numerator / denominator;
			}
		    }
                    break;
                }
                else // this variable is already defined
                {
                    pfNotify(PFNFY_WARN, PFNFY_PRINT,
                             "pfdLoadFile_spherepatch(\"%s\", line %d): variable %s redefined before variable(s) defined:", fileName, lineNum, variables[i].name);
                    int j;
                    FOR (j, numberof(variables))
                        if (!BIT(which_params_got, j))
                            pfNotify(PFNFY_WARN, PFNFY_MORE,
                                 "    %s", variables[j].name);
		    pfDelete(group);
		    return NULL;
                }
        }
        if (which_params_got == (1<<numberof(variables)) - 1)
        {
            //
            // All variables defined-- create a patch and start over.
            //

	    //
	    // First, try to find an existing group
	    // associated with the texture...
	    //
	    pfTexture *tex;
	    pfuClipCenterNode *texGroup;
	    FOR (i, numClipTextures)
	    {
		texGroup = (pfuClipCenterNode *)group->getChild(i);
		tex = texGroup->getClipTexture();
		if (streq(params.texname, tex->getName()))
		    break;
	    }
	    if (i == numClipTextures)
	    {
		//
		// Haven't already seen this texture... create the texture
		// and a new subgroup for it.
		//
		texGroup = new pfuClipCenterNode;
		PFASSERTALWAYS(texGroup != NULL);
		group->addChild(texGroup);

		if (strstr(params.texname, ".ct") != NULL)
		{
		    tex = pfdLoadClipTexture(params.texname);
		    if (tex == NULL)
		    {
			pfNotify(PFNFY_WARN, PFNFY_PRINT,
				 "pfdLoadFile_spherepatch(\"%s\"): couldn't load clip texture \"%s\"\n", fileName, params.texname);
			pfDelete(group);
			return NULL;
		    }
		    texGroup->setTravFuncs(PFTRAV_APP, texGroupPostApp, NULL);
#ifdef PRECISION_HACK
		    texGroup->setTravFuncs(PFTRAV_CULL, texGroupPreCull, NULL);
		    texGroup->setTravMode(PFTRAV_CULL, PFN_CULL_SORT,
						       PFN_CULL_SORT_CONTAINED);
			/* see explanation in .ct loader source */
#endif /* PRECISION_HACK */
		}
		else
		{
		    tex = new(pfGetSharedArena()) pfTexture;
		    PFASSERTALWAYS(tex != NULL);
		    if (!tex->loadFile(params.texname))
		    {
			pfNotify(PFNFY_WARN, PFNFY_PRINT,
				 "pfdLoadFile_spherepatch(\"%s\"): couldn't load texture \"%s\"\n", fileName, params.texname);
			pfDelete(group);
			return NULL;
		    }
		}
		tex->setName(params.texname); // maybe not necessary?

		//
		// We are using a pfuClipCenterNode
		// instead of just a group,
		// so that the texture will get attached
		// to a pipe when the app (e.g. perfly)
		// calls pfuProcessClipCenters.
		// We don't use its centering mechanism,
		// so set the callback to NULL.
		// (XXX although we should perhaps use its callback
		// slot rather than using the post APP func,
		// since sometimes irresponsible people
		// clobber the funcs)
		//
		if (tex->isOfType(pfClipTexture::getClassType()))
		    texGroup->setClipTexture((pfClipTexture *)tex);
		texGroup->setCallback(NULL);

		numClipTextures++;
	    }

// XXX The following works, but it messes up
// XXX the correspondence between patches as they appear in the file
// XXX and the indices used in debugging messages.
// XXX Also it might defeat some
// XXX stitching efforts (not sure about this).
#ifdef DONT_LET_PATCH_SPAN_MORE_THAN_HALF_IN_S_DIRECTION
	    //
	    // Don't let any single piece span
	    // more than .5 in the S direction.
	    // This restriction will be necessary when wrapping
	    // is implemented for InfiniteReality.
	    //
	    int nPiecesS = (PF_ABS(params.s1-params.s0) <= .5 ? 1 : 2);
#else
	    int nPiecesS = 1;
#endif
	    int nPiecesT = 1;
	    int pieceS, pieceT;
	    FOR (pieceT, nPiecesT)
	    FOR (pieceS, nPiecesS)
	    {
		pfNode *patch = makeGlobePatch(
		 /*lon0*/ LERP(params.lon0, params.lon1, (pieceS+0.)/nPiecesS),
		 /*lat0*/ LERP(params.lat0, params.lat1, (pieceT+0.)/nPiecesT),
		   /*s0*/ LERP(params.s0,   params.s1,   (pieceS+0.)/nPiecesS),
		   /*t0*/ LERP(params.t0,   params.t1,   (pieceT+0.)/nPiecesT),
		 /*lon1*/ LERP(params.lon0, params.lon1, (pieceS+1.)/nPiecesS),
		 /*lat1*/ LERP(params.lat0, params.lat1, (pieceT+1.)/nPiecesT),
		   /*s1*/ LERP(params.s0,   params.s1,   (pieceS+1.)/nPiecesS),
		   /*t1*/ LERP(params.t0,   params.t1,   (pieceT+1.)/nPiecesT),
			  (params.lonsegs+nPiecesS-1) / nPiecesS,
			  (params.latsegs+nPiecesT-1) / nPiecesT,
                          tex);
		texGroup->addChild(patch);
	    }

            which_params_got = 0;
        }
    }
    if (which_params_got != 0)
    {
        pfNotify(PFNFY_WARN, PFNFY_PRINT,
                 "pfdLoadFile_spherepatch(\"%s\"): end-of-file reached while the following variables are undefined:", fileName);
        int j;
        FOR (j, numberof(variables))
            if (!BIT(which_params_got, j))
                pfNotify(PFNFY_WARN, PFNFY_MORE,
                     "    %s", variables[j].name);
	pfDelete(group);
	return NULL;
    }

    pfSphere bsph;
    bsph.center = pfVec3(0,0,0);
    bsph.radius = 1;
    group->setBound(&bsph, PFBOUND_STATIC);

    //
    // Find all adjacent pairs of patches and tweak the finer one's
    // boundary to match the coarser one
    //
    if (!getenv("_PFSPHEREPATCH_DONT_STITCH"))
	stitchAll(group);

    return group;
}

// utility function for stitchAll
static void stitch(pfVec3 verts[],
		   int startInd,
		   int toNext, // stride from one fine vertex to next
		   int toNextsTwin, // to next fine vertex's twin
		   int nSegsFine,
		   int period) // how many fine segs in a coarse seg
{
    PFASSERTALWAYS(nSegsFine % period == 0); // XXX FIX THIS!!! shouldn't have gotten this far; print a warning instead
    int nSegsCoarse = nSegsFine / period;
    int toTwin = toNextsTwin - toNext;

    int segCoarse;
    FOR (segCoarse, nSegsCoarse)
    {
	// Find beginning and ending vertex for this segment...
	int i0 = startInd + segCoarse * period * toNext;
	int i1 = startInd + (segCoarse+1) * period * toNext + toTwin;

	int t;
	for (t = 1; t < period; ++t)
	{
	    int i = startInd + (segCoarse * period + t) * toNext;
	    verts[i].combine(1-(double)t/period, verts[i0],
			       (double)t/period, verts[i1]);
	    if (toTwin != 0)
		verts[i+toTwin] = verts[i]; // vector copy
	}
    }
}

//
// For each patch0, patch1 in the entire group, if they are adjacent,
// translate the extra vertices of the boundary of the finer tesselated one
// so that they lie on the boundary of the coarser tesselated one.
//
static void stitchAll(pfGroup *bigGroup)
{
    //
    // For each patch patch0 in the whole thing...
    //
    int iPatch0 = -1;
    int numTexGroups = bigGroup->getNumChildren();
    int iTexGroup0;
    FOR (iTexGroup0, numTexGroups)
    {
	pfGroup *texGroup0 = (pfGroup *)bigGroup->getChild(iTexGroup0);
	int numPatches0 = texGroup0->getNumChildren();
	int iChild0;
	FOR (iChild0, numPatches0)
	{
	    pfGeode *geode0 = (pfGeode *)texGroup0->getChild(iChild0);
	    PatchParams *params0 = (struct PatchParams *)geode0->getUserData();

	    ++iPatch0;
	    int iPatch1 = -1;

	    pfVec3 *vcoords0;
	    ushort *vinds0;
	    geode0->getGSet(0)->getAttrLists(PFGS_COORD3,
					    (void **)&vcoords0, &vinds0);

	    //
	    // For each patch patch1 in the whole thing...
	    //
	    int iTexGroup1;
	    FOR (iTexGroup1, numTexGroups)
	    {
		pfGroup *texGroup1 = (pfGroup *)bigGroup->getChild(iTexGroup1);
		int numPatches1 = texGroup1->getNumChildren();
		int iChild1;
		FOR (iChild1, numPatches1)
		{
		    pfGeode *geode1 = (pfGeode *)texGroup1->getChild(iChild1);
		    PatchParams *params1 = (struct PatchParams *)geode1->getUserData();
		    ++iPatch1;

		    if (geode1 == geode0)
			continue;

		    //
		    // If the lon ranges overlap and
		    // patch0 is more finely tesselated than patch1 in lon...
		    // (don't worry about floating point error--
		    // it doesn't hurt to go either way if
		    // they are tesselated equally)
		    //
		    double periodS = /* how many patch0 segs in a patch1 seg */
		        (PF_ABS(params1->lon1-params1->lon0)/params1->lonsegs)
		      / (PF_ABS(params0->lon1-params0->lon0)/params0->lonsegs);
		    if (periodS > 1
		     && arcIntersectionLength(params0->lon0, params0->lon1,
				              params1->lon0, params1->lon1)>1e-6)
		    {
			if (ISINT((params0->lat1-params1->lat0)/360, 1e-6))
			{
			    if (!ISINT(periodS, .001))
				pfNotify(PFNFY_WARN, PFNFY_PRINT,
					 "pfdLoadFile_spherepatch: Can't stitch top side of patch%d to meet bottom side of patch%d-- ratio of segment sizes is not an integer (%g)", iPatch0, iPatch1, periodS);
			    else if (params0->lonsegs % (int)round(periodS) != 0)
				pfNotify(PFNFY_WARN, PFNFY_PRINT,
					 "pfdLoadFile_spherepatch: Can't stitch top side of patch%d to meet bottom side of patch%d-- number of coarse segments in fine patch is not an integer (%g)", iPatch0, iPatch1, (double)params0->lonsegs / (int)round(periodS));

			    else
			    {
				pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
					 "Stitching top side of patch%d to meet bottom side of patch%d, period = %g", iPatch0, iPatch1, periodS);
				stitch(vcoords0,
				       2*(params0->latsegs-1)*(params0->lonsegs+1),
				       2,
				       2,
				       params0->lonsegs,
				       (int)round(periodS));
			    }
			}
			if (ISINT((params0->lat0-params1->lat1)/360, 1e-6))
			{
			    if (!ISINT(periodS, .001))
				pfNotify(PFNFY_WARN, PFNFY_PRINT,
					 "pfdLoadFile_spherepatch: Can't stitch bottom side of patch%d to meet top side of patch%d-- ratio of segment sizes is not an integer (%g)", iPatch0, iPatch1, periodS);
			    else if (params0->lonsegs % (int)round(periodS) != 0)
				pfNotify(PFNFY_WARN, PFNFY_PRINT,
					 "pfdLoadFile_spherepatch: Can't stitch bottom side of patch%d to meet top side of patch%d-- number of coarse segments in fine patch is not an integer (%g)", iPatch0, iPatch1, (double)params0->lonsegs / (int)round(periodS));
			    else
			    {
				pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
					 "Stitching bottom side of patch%d to meet top side of patch%d, period = %g", iPatch0, iPatch1, periodS);
				stitch(vcoords0,
				       1,
				       2,
				       2,
				       params0->lonsegs,
				       (int)round(periodS));
			    }
			}
		    }
		    else
		    {
			pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
				 "  Patch%d is coarser than or does not overlap the range of patch%d in S", iPatch0, iPatch1);
		    }

		    //
		    // If the lat ranges overlap and
		    // patch1 is more finely tesselated than patch1 in lat...
		    //
		    double periodT = /* how many patch0 segs in a patch1 seg */
		        (PF_ABS(params1->lat1-params1->lat0)/params1->latsegs)
		      / (PF_ABS(params0->lat1-params0->lat0)/params0->latsegs);
		    if (periodT > 1
		     && arcIntersectionLength(params0->lat0, params0->lat1,
				              params1->lat0, params1->lat1)>1e-6)
		    {
			if (ISINT((params0->lon1-params1->lon0)/360, 1e-6))
			{
			    if (!ISINT(periodT, .001))
				pfNotify(PFNFY_WARN, PFNFY_PRINT,
					 "pfdLoadFile_spherepatch: Can't stitch right side of patch%d to meet left side of patch%d-- ratio of segment sizes is not an integer (%g)", iPatch0, iPatch1, periodT);
			    else if (params0->latsegs % (int)round(periodT) != 0)
				pfNotify(PFNFY_WARN, PFNFY_PRINT,
					 "pfdLoadFile_spherepatch: Can't stitch right side of patch%d to meet left side of patch%d-- number of coarse segments in fine patch is not an integer (%g)", iPatch0, iPatch1, (double)params0->latsegs / (int)round(periodT));
			    else
			    {
				pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
					 "Stitching right side of patch%d to meet left side of patch%d, period = %g", iPatch0, iPatch1, periodT);
				stitch(vcoords0,
				       2*(params0->lonsegs+1)-1,
				       2*(params0->lonsegs+1),
				       -1,
				       params0->latsegs,
				       (int)round(periodT));
			    }
			}
			if (ISINT((params0->lon0-params1->lon1)/360, 1e-6))
			{
			    if (!ISINT(periodT, .001))
				pfNotify(PFNFY_WARN, PFNFY_PRINT,
					 "pfdLoadFile_spherepatch: Can't stitch left side of patch%d to meet right side of patch%d-- ratio of segment sizes is not an integer (%g)", iPatch0, iPatch1, periodT);
			    else if (params0->latsegs % (int)round(periodT) != 0)
				pfNotify(PFNFY_WARN, PFNFY_PRINT,
					 "pfdLoadFile_spherepatch: Can't stitch left side of patch%d to meet right side of patch%d-- number of coarse segments in fine patch is not an integer (%g)", iPatch0, iPatch1, (double)params0->latsegs / (int)round(periodT));
			    {
				pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
					 "Stitching left side of patch%d to meet right side of patch%d, period = %g", iPatch0, iPatch1, periodT);
				stitch(vcoords0,
				       1,
				       2*(params0->lonsegs+1),
				       -1,
				       params0->latsegs,
				       (int)round(periodT));
			    }
			}
		    }
		    else
		    {
			pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
				 "  Patch%d is coarser than or does not overlap the range of patch%d in T", iPatch0, iPatch1);
		    }
		} // end for each patch1
	    }
	} // end for each patch0
    }
}

static pfNode *
makeGlobePatch(double lon0, double lat0, double s0, double t0,
               double lon1, double lat1, double s1, double t1,
               int lonsegs, int latsegs,
               pfTexture *tex)
{
    int i, j, k, verti, stripi;
    int nstrips = latsegs;
    int nverts = (lonsegs+1)*(2*nstrips);
    void *arena = pfGetSharedArena();
    pfGeode *geode = new pfGeode;
    pfGeoSet *geoset = new(arena) pfGeoSet;
    pfGeoState *geostate = new(arena) pfGeoState;
    int *lengths = (int *) pfMalloc(nstrips * sizeof(*lengths), arena);
    pfVec3 *vc = (pfVec3 *) pfMalloc(nverts * sizeof(*vc), arena);
#ifdef PRECISION_HACK
    double (*tc)[2] = (double (*)[2]) pfMalloc(nverts * sizeof(*tc), arena);
    pfFlux *tc_flux = new pfFlux(nverts*sizeof(pfVec2),
				 PFFLUX_DEFAULT_NUM_BUFFERS);
#else /* !PRECISION_HACK */
    pfVec2 *tc = (pfVec2 *) pfMalloc(nverts * sizeof(*tc), arena);
#endif /* !PRECISION_HACK */
    pfVec4 *colors = (pfVec4 *) pfMalloc(1 * sizeof(*colors), arena);

    PFASSERTALWAYS(geode && geoset && geostate && lengths && vc && tc);

    if (tex != NULL)
    {
        geostate->setAttr(PFSTATE_TEXTURE, tex);
        geostate->setMode(PFSTATE_ENTEXTURE, PF_ON);
    }

    /*
     * Set vertex and texture coords...
     */
    verti = 0;
    stripi = 0;
    for (i = 0; i < latsegs; ++i)
    {
        for (j = 0; j <= lonsegs; ++j) /* sic */
        {
            for (k = 0; k < 2; ++k)
            {
                double lat = lat0 + (lat1-lat0)*(i+1-k)/latsegs;
                double lon = lon0 + (lon1-lon0)*j/lonsegs;
                double t = t0 + (t1-t0)*(i+1-k)/latsegs;
                double s = s0 + (s1-s0)*j/lonsegs;
                float sinlon, coslon, sinlat, coslat;
                pfSinCos(lon, &sinlon, &coslon);
                pfSinCos(lat, &sinlat, &coslat);
                vc[verti][PF_X] = sinlon * coslat;
                vc[verti][PF_Y] = -coslon * coslat;
                vc[verti][PF_Z] = sinlat;
                tc[verti][PF_S] = s;
                tc[verti][PF_T] = t;
                verti++;
            }
        }
        lengths[stripi] = (lonsegs+1)*2;
        stripi++;
    }
    PFASSERTALWAYS(verti == nverts);
    PFASSERTALWAYS(stripi == nstrips);
    PFSET_VEC4(colors[0], 1,1,1,1); /* white */

    geoset->setPrimType(PFGS_TRISTRIPS);
    geoset->setNumPrims(nstrips);
    geoset->setPrimLengths(lengths);
    geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, vc, NULL);
    geoset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX, vc, NULL);
#ifdef PRECISION_HACK
    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tc_flux, NULL);
    // Make sure the tex coords in the flux are initialized here
    // (this only matters for non-cliptextures, for which there is no callback)
    pfVec2 *single_tc = (pfVec2 *)tc_flux->getWritableData();
    FOR (i, nverts)
	PFCOPY_VEC2(single_tc[i], tc[i]);
    tc_flux->writeComplete();
#else /* !PRECISION_HACK */
    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tc, NULL);
#endif /* !PRECISION_HACK */
    geoset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
    geoset->setGState(geostate);

    geode->addGSet(geoset);

    {
        /* XXX this seems to be necesssary for the trackball
           if displaying only half the earth,
           even though setting the bounding sphere of
           the whole scene makes it get centered properly...
           why are these two issues handled differently?? */
        pfBox box;
        PFSET_VEC3(box.min, -1, -1, -1);
        PFSET_VEC3(box.max, 1, 1, 1);
        geoset->setBound(&box, PFBOUND_STATIC);
    }

    struct PatchParams *params =
        (struct PatchParams *)pfMalloc(sizeof(PatchParams), arena);
    PFASSERTALWAYS(params != NULL);
    params->lon0 = lon0; params->lat0 = lat0; params->s0 = s0; params->t0 = t0;
    params->lon1 = lon1; params->lat1 = lat1; params->s1 = s1; params->t1 = t1;
    params->lonsegs = lonsegs;
    params->latsegs = latsegs;
#ifdef PRECISION_HACK
    params->double_texcoords = tc;
#endif /* PRECISION_HACK */
    geode->setUserData(params);

    return geode;
}

#ifdef PRECISION_HACK
static int texGroupPreCull(pfTraverser *trav, void *)
{
    //
    // Warp the clip center and all the tex coords
    // to the lower-left corner of the virtual
    // clip texture, where we have enough precision in a float.
    // We warp by a multiple of the good area size; this
    // guarantees all texture addressing will be unchanged
    // modulo the clip sizes at all levels, which is
    // all that matters in a clip texture.
    //
    pfuClipCenterNode *texGroup = (pfuClipCenterNode *)trav->getNode();
    pfClipTexture *cliptex = texGroup->getMPClipTexture()->getClipTexture();
    int virtSize; // assume square
    cliptex->getVirtualSize(&virtSize, NULL, NULL);
    int s,t,r, newS, newT;
    int virtualLODOffset = cliptex->getVirtualLODOffset();
    int numEffectiveLevels = cliptex->getNumEffectiveLevels();
    while ((1<<(numEffectiveLevels-1)) > virtSize)
	--numEffectiveLevels;
    cliptex->getCurCenter(&s, &t, &r);
    {
	int goodAreaSize = 1<<(numEffectiveLevels+virtualLODOffset-1);
	newS = ((s-(goodAreaSize>>1))&(goodAreaSize-1))+(goodAreaSize>>1);
	newT = ((t-(goodAreaSize>>1))&(goodAreaSize-1))+(goodAreaSize>>1);
	newS = PF_MIN2(newS, s); // don't increase
	newT = PF_MIN2(newT, t); // don't increase
    }

    /*
     * For demonstration, if env _PFSPHEREPATCH_PRECISION_HACK_TEST is set,
     * turn on and off this fix every 5 seconds or so.
     */
    static int _PFSPHEREPATCH_PRECISION_HACK_TEST = -1;
    if (_PFSPHEREPATCH_PRECISION_HACK_TEST == -1)
    {
	char *e = getenv("_PFSPHEREPATCH_PRECISION_HACK_TEST");
	_PFSPHEREPATCH_PRECISION_HACK_TEST = (e ? *e ? atoi(e) : 1 : 0);
    }
    if (_PFSPHEREPATCH_PRECISION_HACK_TEST)
    {
	int f = pfGetFrameCount();
	if (f % 600 == 0)
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Turning on precision hack");
	}
	else if (f % 600 == 300)
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Turning off precision hack");
	    newS = s; // what it was
	    newT = t; // what it was
	    // translate by 0 for one frame to initialize the fluxed texCoords
	}
	else if (f % 600 >= 300)
	{
	    // The fluxed texCoords are initialized; just leave everything
	    // alone
	    return PFTRAV_CONT;
	}
    }

    //
    // Translate the clip center (applies to the scene graph under this node)...
    //
    cliptex->applyCenter(newS, newT, r);

    //
    // Translate all the tex coords...
    // (XXX Note this could be done in the APP instead
    // if it turns out the CULL takes too much time)
    //
    double fudgeS = (double)(newS - s) / virtSize;
    double fudgeT = (double)(newT - t) / virtSize;
    int numChildren = texGroup->getNumChildren();
    int i;
    FOR (i, numChildren)
    {
	pfGeode *geode = (pfGeode *)texGroup->getChild(i);
	PatchParams *params = (struct PatchParams *)geode->getUserData();
	double (*double_tcoords)[2] = params->double_texcoords;
	pfVec2 *tcoords;
	ushort *dummy;
	geode->getGSet(0)->getAttrLists(PFGS_TEXCOORD2, (void **)&tcoords, &dummy);
	pfFlux *tcoords_flux = pfFlux::getFlux(tcoords);
	tcoords = (pfVec2 *)tcoords_flux->getWritableData();
	int nverts = (params->lonsegs+1)*(2*params->latsegs);
	int j;
	FOR (j, nverts)
	{
	    tcoords[j][0] = double_tcoords[j][0] + fudgeS;
	    tcoords[j][1] = double_tcoords[j][1] + fudgeT;
	}
	tcoords_flux->writeComplete();
    }
    return PFTRAV_CONT;
}
#endif /* PRECISION_HACK */

static int texGroupPostApp(pfTraverser *trav, void *)
{
    //
    // Make sure the app has called pfuProcessClipCenters
    // and pfuAddMPClipTexturesToPipes()...
    //
    pfuClipCenterNode *texGroup = (pfuClipCenterNode *)trav->getNode();
    pfMPClipTexture *mpcliptex = texGroup->getMPClipTexture();
    PFASSERTALWAYS(mpcliptex != NULL);

#ifdef DEFUNCT
    XXX is it foolproof now? can this just go away???
    pfObject *texobj = (pfObject *)data;
    if (!texobj->isOfType(pfMPClipTexture::getClassType())
     || ((pfMPClipTexture *)texobj)->getPipe() == NULL)
    {
        pfNotify(PFNFY_WARN, PFNFY_USAGE,
                 "pfdLoadFile_spherefront post-app callback called,");
        pfNotify(PFNFY_WARN, PFNFY_MORE,
                 "but clip texture is not attached to a pipe.");
        pfNotify(PFNFY_WARN, PFNFY_MORE,
                 "Use pfuProcessClipCenters() and pfuAddMPClipTexturesToPipes().");
        return PFTRAV_CONT;
    }
    pfMPClipTexture *mpcliptex = (pfMPClipTexture *)texobj;
#endif

    //
    // Get the eye in local coordinate space...
    //
    pfVec3 eye(0,0,0);
    pfMatrix viewmat, travmat;
    pfChannel *chan = trav->getChan();
    chan->getViewMat(viewmat);
    trav->getMat(travmat);
    pfMatrix invtravmat;
    invtravmat.invertOrtho(travmat);
    pfMatrix eyespace_to_objspace_mat = viewmat;
    eyespace_to_objspace_mat.postMult(invtravmat);
    eye.xformPt(eye, eyespace_to_objspace_mat);
    //
    // Compute lon,lat of the eye from its x,y,z position...
    //
    double lon = pfArcTan2(eye[PF_X],-eye[PF_Y]);
    double lat = pfArcTan2(eye[PF_Z], PFLENGTH_VEC2(eye));

    //
    // Find the closest point to the eye
    // on any of the patches (children) defined
    // for this clip texture
    //
    float min_dst_dxyz = MAXFLOAT; // XXX move this to per-patch stuff
    float closest_distsqrd = MAXFLOAT;
    float centerS = .5, centerT = .5;
    int numChildren = texGroup->getNumChildren();
    int isOverPatch = 0;
    int i;
    FOR (i, numChildren)
    {
	pfGeode *geode = (pfGeode *)texGroup->getChild(i);
	PatchParams *params = (struct PatchParams *)geode->getUserData();
	double lon0=params->lon0,lat0=params->lat0;
	double lon1=params->lon1,lat1=params->lat1;
	int lonsegs = params->lonsegs, latsegs = params->latsegs;

	// work with the lon+n*360 closest to midpoint of the patch
	double thisLon = lon, thisLat = lat;
	while (thisLon > .5*(lon0+lon1) + 180.) thisLon -= 360.;
	while (thisLon < .5*(lon0+lon1) - 180.) thisLon += 360.;

	double fracS = (thisLon-lon0) / (lon1-lon0);
	double fracT = (thisLat-lat0) / (lat1-lat0);

	//
	// Determine which little quad (two triangles)
	// is the closest in lon,lat space.
	//
	int col = (int)floor(lonsegs * fracS);
	int row = (int)floor(latsegs * fracT);
	col = PF_CLAMP(col, 0, lonsegs-1);
	row = PF_CLAMP(row, 0, latsegs-1);
	ushort *dummy;
	pfVec3 *vcoords;
	geode->getGSet(0)->getAttrLists(PFGS_COORD3, (void **)&vcoords, &dummy);
#ifdef PRECISION_HACK
	double (*tcoords)[2] = params->double_texcoords;
#else /* !PRECISION_HACK */
	pfVec2 *tcoords;
	geode->getGSet(0)->getAttrLists(PFGS_TEXCOORD2, (void **)&tcoords, &dummy);
#endif /* !PRECISION_HACK */
	// figure out indices into the tri strips created in makeGlobePatch()...
	int i01 = row * (2 * (lonsegs+1))  + 2*col+0; // upper-left vertex
	int i00 = i01 + 1;                            // lower-left vertex
	int i11 = i00 + 1;                            // upper-right vertex
	int i10 = i11 + 1;                            // lower-right vertex
	float w0,w1,w2;
	if (pfuGetClosestPointOnTriangle(eye,
					 vcoords[i00],vcoords[i10],vcoords[i11],
					 &closest_distsqrd,
					 &w0, &w1, &w2))
	{
	    centerS = w0*tcoords[i00][PF_S]
		    + w1*tcoords[i10][PF_S]
		    + w2*tcoords[i11][PF_S];
	    centerT = w0*tcoords[i00][PF_T]
		    + w1*tcoords[i10][PF_T]
		    + w2*tcoords[i11][PF_T];
	    if (!(INRANGE(-1 <= , fracS*lonsegs, <= lonsegs+1)
	       && INRANGE(-1 <= , fracT*latsegs, <= latsegs+1)))
	    {
		//
		// Dumb heuristic:
		// only use actual closest point on triangle
		// if over the triangle (so that if the
		// eye gets very close to the polygon, the
		// clip center is guaranteed to be close to the eye)
		// or within a triangle width of it (to try to
		// get some reasonable continuity at the border)
		// but otherwise just use the unclamped lat and lon.
		//
		centerS = LERP(params->s0, params->s1, fracS);
		centerT = LERP(params->t0, params->t1, fracT);
		isOverPatch = 0;
	    }
	    else
		isOverPatch = 1;
	}
	if (pfuGetClosestPointOnTriangle(eye,
					 vcoords[i00],vcoords[i11],vcoords[i01],
					 &closest_distsqrd,
					 &w0, &w1, &w2))
	{
	    centerS = w0*tcoords[i00][PF_S]
		    + w1*tcoords[i11][PF_S]
		    + w2*tcoords[i01][PF_S];
	    centerT = w0*tcoords[i00][PF_T]
		    + w1*tcoords[i11][PF_T]
		    + w2*tcoords[i01][PF_T];
	    if (!(INRANGE(-1 <= , fracS*lonsegs, <= lonsegs+1)
	       && INRANGE(-1 <= , fracT*latsegs, <= latsegs+1)))
	    {
		//
		// Dumb heuristic:
		// only use actual closest point on triangle
		// if over the triangle (so that if the
		// eye gets very close to the polygon, the
		// clip center is guaranteed to be close to the eye)
		// or within a triangle width of it (to try to
		// get some reasonable continuity at the border)
		// but otherwise just use the unclamped lat and lon.
		//
		centerS = LERP(params->s0, params->s1, fracS);
		centerT = LERP(params->t0, params->t1, fracT);
		isOverPatch = 0;
	    }
	    else
		isOverPatch = 1;
	}

	/* XXX this calculation is not quite right */
	min_dst_dxyz = PF_MIN3(min_dst_dxyz,
			       (params->s1-params->s0)/((lon1-lon0)*M_PI/180),
			       (params->t1-params->t0)/((lat1-lat0)*M_PI/180));
    }


    //
    // Compute clip center for the clip texture...
    //
    pfClipTexture *cliptex = mpcliptex->getClipTexture();
    int virtSizeS, virtSizeT;
    cliptex->getVirtualSize(&virtSizeS, &virtSizeT, NULL);
    mpcliptex->setCenter(centerS * virtSizeS,
                         centerT * virtSizeT,
                         0);
    //
    // Compute the HAT (height-above-terrain) of the eye
    // for this patch.
    // Can't just use eye.length() - 1 (i.e. the distance
    // to the actual sphere),
    // since the closest point in the tesselation might be much
    // farther away (either because the eye is not directly over the patch,
    // or because the tesselation does not come near the surface
    // at that point).
    // XXX should do all this in double-precision...
    //

    // XXX this really
    // XXX should be in the CULL, and need a HAT for each patch,
    // XXX not just the whole clip texture.

    float HAT = pfSqrt(closest_distsqrd);

    //
    // Set this mp clip texture's texLoadTimeFrac
    // equal to 1/HAT, so that near textures
    // will get proportionally more time than far textures.
    // Only proportions are relevant (all of the pipe's pfMPClipTextures'
    // fracs are normalized so that their sum is 1).
    //
    float texLoadTimeFrac = 1.f/PF_MAX2(HAT, 1e-10f);
    mpcliptex->setTexLoadTimeFrac(texLoadTimeFrac);

	//
	// XXX ARGH! If we set the texLoadTime to -1,
	// XXX it overrides the clipfly gui, which is bad.
	// XXX But if we don't set it to -1,
	// XXX it stays the default which is a positive number,
	// XXX which means our frac will be ignored, which is worse!
	// XXX Maybe the default for mp clip textures should be -1
	// XXX (i.e. defer to normalized frac * total pipe time)?
	// XXX But that doesn't work either because
	// XXX the pfMPClipTexture pulls the values out of the pfClipTexture
	// XXX when it is attached to it, and negative texLoadTimes
	// XXX is not a reasonable default for a pfClipTexture!
	//
	mpcliptex->setTexLoadTime(-1.); // so it doesn't override our frac



    pfFrustum *appfrust = new pfFrustum; // XXX per-frame malloc!!!
    int viewportWidth, viewportHeight;
    chan->getBaseFrust(appfrust);
    chan->getSize(&viewportWidth, &viewportHeight);
	/* XXX wrong-- should use culling frustum! */
    float minlod_size = pfuCalcSizeFinestMipLOD(appfrust,
					        viewportWidth, viewportHeight,
					        HAT,
					        min_dst_dxyz,
					        virtSizeS);
    pfDelete(appfrust); // XXX per-frame delete! get rid of this!

    //
    // Compute the texture-space bounding box
    // of the visible part of the patch (i.e. the
    // intersection of the patch with the spherical
    // disk of visibility).
    // To simplify, we will find the analytic solution and
    // add 1 segment in each direction.
    // To simplify again, we find the texture-space
    // bounding box of the visibility disk, and intersect *that*
    // with the patch (this may give a looser bound than
    // would be possible if we were smarter).
    //
    float bboxMinDist;
    float bboxMaxDist;
    float tradeoff = 1.; // choose coarse when conflict (never show garbage)
    {
	lon *= M_PI/180;	// change from degrees to radians
	lat *= M_PI/180;	// change from degrees to radians
	double eyeLength = sqrt(eye[0]*eye[0]+eye[1]*eye[1]+eye[2]*eye[2]);
	eyeLength = PF_MAX2(eyeLength, 1.);
	double r = acos(1./eyeLength); // angular radius of visible disk
	//
	// By hairy calculations done on paper,
	// the "longitudinal radius" of the visible disk
	// is equal to
	// atan2(sin(r),
	//	 sqrt(sqr(cos(lat))*sqr(cos(r)) - sqr(sin(lat))*sqr(sin(r))))
	// The expression inside the sqrt can be rewritten as:
	//	sqr(cos(r)) - sqr(sin(lat))
	// or:  sqr(cos(lat)) - sqr(sin(r))
	// for better stability, take the cosine of the bigger number
	// and the sine of the smaller number.
	//
	double cos_min_r_lat = cos(PF_MIN2(r, lat));
	double sin_max_r_lat = sin(PF_MAX2(r, lat));
	double denomsqrd = PF_SQUARE(cos_min_r_lat) - PF_SQUARE(sin_max_r_lat);
	double lonRadius;
	if (denomsqrd < 0)
	    lonRadius = 2*M_PI; // very big (strictly contains N or S pole)
	else if (denomsqrd == 0)
	    lonRadius = M_PI/2; // circumference passes through N or S pole
	else
	    lonRadius = atan2(sin(r), sqrt(denomsqrd));

	//
	// Bounds of the visible disk (in radians)
	//
	double visDiskMinLon = lon - lonRadius;
	double visDiskMaxLon = lon + lonRadius;
	double visDiskMinLat = PF_CLAMP(lat - r, -M_PI/2, M_PI/2);
	double visDiskMaxLat = PF_CLAMP(lat + r, -M_PI/2, M_PI/2);

	//
	// Convert back to degrees (this kinda sucks)
	//
	visDiskMinLon *= 180/M_PI;
	visDiskMaxLon *= 180/M_PI;
	visDiskMinLat *= 180/M_PI;
	visDiskMaxLat *= 180/M_PI;


	//
	// Accumulate texture-space bounding box
	// of intersection with each patch
	//
	float minS = 1.f, maxS = 0.f;
	float minT = 1.f, maxT = 0.f;
	FOR (i, numChildren)
	{
	    pfGeode *geode = (pfGeode *)texGroup->getChild(i);
	    PatchParams *params = (struct PatchParams *)geode->getUserData();
	    double lon0=params->lon0, lat0=params->lat0, s0=params->s0, t0=params->t0;
	    double lon1=params->lon1, lat1=params->lat1, s1=params->s1, t1=params->t1;
	    double patchIsectLon0, patchIsectLon1;
	    double patchIsectLat0, patchIsectLat1;
	    if (intersectArcs(lon0, lon1, visDiskMinLon, visDiskMaxLon,
			      &patchIsectLon0, &patchIsectLon1)
	     && intersectArcs(lat0, lat1, visDiskMinLat, visDiskMaxLat,
			      &patchIsectLat0, &patchIsectLat1))
	    {
		float patchIsectS0 = LERP(s0, s1,
					   (patchIsectLon0-lon0)/(lon1-lon0));
		float patchIsectS1 = LERP(s0, s1,
					   (patchIsectLon1-lon0)/(lon1-lon0));
		float patchIsectT0 = LERP(t0, t1,
					   (patchIsectLat0-lat0)/(lat1-lat0));
		float patchIsectT1 = LERP(t0, t1,
					   (patchIsectLat1-lat0)/(lat1-lat0));
		// XXX need to add a segment length on either side, as described above!
		minS = PF_MIN3(minS, patchIsectS0, patchIsectS1);
		maxS = PF_MAX3(maxS, patchIsectS0, patchIsectS1);
		minT = PF_MIN3(minT, patchIsectT0, patchIsectT1);
		maxT = PF_MAX3(maxT, patchIsectT0, patchIsectT1);
	    }
	}
	bboxMaxDist = PF_MAX4(maxS-centerS,centerS-minS,
			      maxT-centerT,centerT-minT);
	bboxMinDist = PF_MAX4(minS-centerS,centerS-maxS,
			      minT-centerT,centerT-maxT);

	// XXX Bleah! this is not right yet; if we choose tradeoff=1,
	// XXX we'll never see
	// XXX the finest stuff until we're too close.
	// XXX For now, just guarantee that if we
	// XXX can see any of the international date line (which includes
	// XXX the N and S pole) then there is no bad area.
	// XXX (This is right for geosphere, but probably overkill
	// XXX for multi-clipmap databases.)
	if (PF_ABS(eye[2]) >= 1
	 || eye[1] > 0 && eye[1]*eye[1]+eye[2]*eye[2] > 1)
	    tradeoff = 1.; // can see date line-- choose coarse if conflict
	else
	    tradeoff = 0.; // choose fine if conflict
    }

    pfVirtualClipTexLimits limits;
    mpcliptex->getLODOffsetLimit(&limits.LODOffset.lo,
			         &limits.LODOffset.hi);
    mpcliptex->getNumEffectiveLevelsLimit(&limits.numEffectiveLevels.lo,
			                  &limits.numEffectiveLevels.hi);
    mpcliptex->getMinLODLimit(&limits.minLOD.lo,
			      &limits.minLOD.hi);
    mpcliptex->getMaxLODLimit(&limits.maxLOD.lo,
			      &limits.maxLOD.hi);

    float LODBiasS, LODBiasT;
    mpcliptex->getLODBias(&LODBiasS, &LODBiasT, NULL);

	// XXX workaround for bizarre values of LODBiasS and LODBiasT...
	// XXX If I set the values to 0 after pfdLoadClipTexture,
	// XXX they don't seem to hold, so have to special case here.
	if (LODBiasS == PFTEX_DEFAULT) LODBiasS = 0.;
	if (LODBiasT == PFTEX_DEFAULT) LODBiasT = 0.;

    int nLevels = log2int(virtSizeS)+1;
    int clipSize = cliptex->getClipSize();
    int invalidBorder = mpcliptex->getInvalidBorder();
    float minLODTexPix = nLevels-1 - log2(minlod_size) + PF_MIN2(LODBiasS, LODBiasT);
    float minLODLoaded = 0; // XXX just let DTR do it for now
    float maxLODLoaded = cliptex->getNumAllocatedLevels()-1;

    {
	// XXX Stopgap to get the program working so
	// XXX that maxLOD is not less than minLOD...
	// XXX It is illegal to do this in the APP!!!!
	// XXX it needs to be moved to the CULL.
	// XXX the value here may be a frame out of date.
	minLODLoaded = cliptex->getMinDTRLOD();
    }

    int LODOffset, numEffectiveLevels;
    float minLOD, maxLOD;
    pfuCalcVirtualClipTexParams(
	nLevels,
	clipSize,
	invalidBorder,
	minLODTexPix,
	minLODLoaded,
	maxLODLoaded,
	bboxMinDist,
	bboxMaxDist,
	tradeoff,
	&limits,
	&LODOffset,
	&numEffectiveLevels,
	&minLOD,
	&maxLOD);
    mpcliptex->setLODRange(minLOD, maxLOD);
    mpcliptex->setVirtualLODOffset(LODOffset);
    mpcliptex->setNumEffectiveLevels(numEffectiveLevels);

    static int _PFSPHEREPATCH_DEBUG = -1;
    if (_PFSPHEREPATCH_DEBUG == -1)
    {
	char *e = getenv("_PFSPHEREPATCH_DEBUG");
	_PFSPHEREPATCH_DEBUG = (e ? *e ? atoi(e) : 1 : 0);
    }
    if (_PFSPHEREPATCH_DEBUG)
    {
	pfNotify(PFNFY_INFO, PFNFY_PRINT,
		"Frame %d post-texGroup node app func\n",
		pfGetFrameCount());
	pfNotify(PFNFY_INFO, PFNFY_MORE,
		"Constant params: nLevels=%d clipSize=%d invalidBorder=%d",
		nLevels,
		clipSize,
		invalidBorder);
	pfNotify(PFNFY_INFO, PFNFY_MORE,
		"Global params: minLODTexPix=%f minLODLoaded=%f maxLODLoaded=%f",
		minLODTexPix,
		minLODLoaded,
		maxLODLoaded);
	pfNotify(PFNFY_INFO, PFNFY_MORE,
		"Clip Center (in tex coords): (%f,%f) (%s)",
		centerS, centerT,
		isOverPatch ? "over a patch" : "not over a patch");
	pfNotify(PFNFY_INFO, PFNFY_MORE,
		"CurTexLoadTime: %f/%f (only accurate for last cliptexture)",
		mpcliptex->getCurTexLoadTime(),
		mpcliptex->getPipe()->getTotalTexLoadTime());
	pfNotify(PFNFY_INFO, PFNFY_MORE,
		"Local params: bboxMinDist=%f bboxMaxDist=%f tradeoff=%f",
		bboxMinDist,
		bboxMaxDist,
		tradeoff);
        pfNotify(PFNFY_INFO, PFNFY_MORE,
                "        limits: LODOffset=%d..%d effectiveLevels=%d..%d minLOD=%f..%f maxLOD=%f..%f\n",
                limits.LODOffset.lo,
                limits.LODOffset.hi,
                limits.numEffectiveLevels.lo,
                limits.numEffectiveLevels.hi,
                limits.minLOD.lo,
                limits.minLOD.hi,
                limits.maxLOD.lo,
                limits.maxLOD.hi);
        pfNotify(PFNFY_INFO, PFNFY_MORE,
                "       results: LODOffset=%d effectiveLevels=%d minLOD=%f maxLOD=%f",
                LODOffset,
                numEffectiveLevels,
                minLOD,
                maxLOD);
    }

    return PFTRAV_CONT;
}

//
// Intersect two circular arcs, measured in degrees,
// where values that are equal mod 360 are considered
// equal.  Arcs may go in either direction.
// The result is in the space of the first arc argument,
// e.g. intersectArcs([100..0], [-100..0]) == [100..80].
// If the actual intersection is in two pieces,
// the entire first arc argument is returned.
// The endpoints are not included.
//
static int intersectArcs(double a0, double a1,
			 double b0, double b1,
			 double *Result0, double *Result1)
{
    double temp;
    int reversed = 0;

    if (a0 > a1)
    {
	SWAP(a0, a1, temp);
	reversed = 1;
    }
    if (b0 > b1)
	SWAP(b0, b1, temp); // order of b0,b1 in args is irrelevant

    //
    // Move the b interval by a multiple of 360
    // so its center is 180 degrees or less from a's center
    //
    double aAvg = (a0+a1)*.5;
    double bAvg = (b0+b1)*.5;
    while (bAvg > aAvg + 180)
    {
	b0 -= 360;
	b1 -= 360;
	bAvg -= 360;
    }
    while (bAvg < aAvg - 180)
    {
	b0 += 360;
	b1 += 360;
	bAvg += 360;
    }

    double result0, result1;
    int returnval;
    double lo = PF_MIN2(a0, b0);
    double hi = PF_MAX2(a1, b1);
    if (hi-lo > 360) // endpoints not included
    {
	// arcs meet from both directions; return a
	result0 = a0;
	result1 = a1;
	returnval = 2; // two parts intersection
    }
    else
    {
	result0 = PF_MAX2(a0, b0);
	result1 = PF_MIN2(a1, b1);
	if (b0 == b1)
	    returnval = (a0<b0 && b0<a1 ? 1 : 0);
	else
	    returnval = (result1 > result0 ? 1 : 0); // endpoints not included
    }
    if (reversed)
	SWAP(result0, result1, temp);

    if (Result0 != NULL) *Result0 = result0;
    if (Result1 != NULL) *Result1 = result1;

    return returnval;
}

static double arcIntersectionLength(double a0, double a1,
				    double b0, double b1)
{
    double result0, result1;
    (void)intersectArcs(a0, a1, b0, b1, &result0, &result1);
    return SIGN(a1-a0) * (result1-result0);
    // not very useful if there's two parts in the intersection
}
