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
 * pfspherepatch3.c - textured sphere patch loader.
 * 
 * $Revision: 1.12 $ 
 * $Date: 2002/11/07 18:06:03 $
 */

/*
    Example:
	    #
	    #This file contains two patches.
	    #
	    # Overlapping patches are allowed;
	    # later patches appear "on top of" earlier ones.
	    # A texture name of NULL is treated specially;
	    # it doesn't get rendered but it cuts a hole
	    # in previous patches.
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
	    # The variables "lonsegs" and "latsegs" may
	    # be defined (for backwards compatibility
	    # with previous version) but are ignored.
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

    The structure of the resulting scene graph is:

    dcsParent (have to get traverser matrix here due to bug in
     |         querying traverser matrix on DCSs)
     |       
     +- pfDoubleDCS (pre-node APP func sets this matrix to be a translate by
	 |           -eye so the eye is always at the origin of the local space,
	 |           so that SP verts are adequate (we have no other choice).
	 |           Also recalculates (morphs) the global geometry based on
	 |           the eye position, empties each clipmap group,
	 |           and then re-builds each patch geometry based
	 |           on the global geometry, adding each patch geometry
	 |           into its respective clipmap group)
	 |
	 +- clipmap 0 group  (see clipringsnode.h for more detail on this)
	 |   +- clip ring 00 geode
	 |   +- clip ring 01 geode
	 |   +- clip ring 02 geode
	 |   +- ...
	 |
	 +- clipmap 1 group
	 |   +- clip ring 10 geode
	 |   +- clip ring 11 geode
	 |   +- ...
	 |
	 +- clipmap 2 group
	 |   +- clip ring 20 geode
	 |   +- clip ring 21 geode
	 |   +- ...
	 |
	 +- ...
*/

// Stuff you can set via environment variables...
// XXX Should be loader modes?
static int _PFSPHEREPATCH_MAXSEGSLON = 72; // 5 degree increments
static int _PFSPHEREPATCH_MAXSEGSLAT = 36; // 5 degree increments
static int _PFSPHEREPATCH_MINSEGSLON = 30;
static int _PFSPHEREPATCH_MINSEGSLAT = 30;
static float _PFSPHEREPATCH_RINGRATIO = 2.; // btwn concentric rings in tex space
static int _PFSPHEREPATCH_NRINGS = -1; // -1 means automatically calculate it
static double _PFSPHEREPATCH_RADIUS = 1.; // radius of whole sphere

// The rest also can also be changed at runtime via tty input...
static float _PFSPHEREPATCH_TRADEOFF = 1.; // tradeoff param to pfuCalcVirtualClipTexParams()
static int _PFSPHEREPATCH_DEBUG = 0;
static double _PFSPHEREPATCH_PATCHSHRINK = 1.; // set to < 1 to separate patches
static double _PFSPHEREPATCH_GLOBALSHRINK = 1.; // set to < 1 to see it all on front
static int _PFSPHEREPATCH_TTYINPUT = 0; // if set, accept tty input (see below)

static int _PFSPHEREPATCH_NO_TRANSLATE = 0; // if set, don't do translation to local origin (see what happens when we don't).
static int _PFSPHEREPATCH_NO_PRECISION_HACK = 0; // if set, don't use "local tex space origin"
static float _PFSPHEREPATCH_COLOR_RINGS = 0.; // saturation of debug colors



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#if !defined(__linux__) && !defined(WIN32)
#include <mutex.h>
#endif
#ifndef WIN32
#include <sys/time.h> // for struct timeval
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
#ifdef PFMATRIX4D
    #include <Performer/pf/pfDoubleDCS.h>
#else
    #include <Performer/pf/pfDCS.h>
    #define pfDoubleDCS pfDCS
    #define getMatD getMat
    #define pfMatrix4d pfMatrix
    #define pfVec3d pfVec3
    #define PFLENGTH_VEC3D(v) sqrt((v)[0]*(v)[0]+(v)[1]*(v)[1]+(v)[2]*(v)[2])
    #define PFLENGTH_VEC2D(v) sqrt((v)[0]*(v)[0]+(v)[1]*(v)[1])
    typedef double pfVec2d[2]; // not passed to anything, used for texCoordsD
    inline void pfSinCosd(double arg, double* s, double* c)
	    { *s = sin((arg)*(M_PI/180.)); *c = cos((arg)*(M_PI/180.)); }
    inline double pfArcTan2d(double y, double x)
	    { return atan2(y, x)*(180./M_PI); }
#endif /* !PFMATRIX4D */

#define PFNUMBEROF(things) ((int)(pfGetSize(things)/sizeof(*(things))))
#ifndef PF_MIN5
    #define PF_MIN5(a,b,c,d,e) ((a) < (b) ? PF_MIN4(a,c,d,e) : PF_MIN4(b,c,d,e))
    #define PF_MAX5(a,b,c,d,e) ((a) > (b) ? PF_MAX4(a,c,d,e) : PF_MAX4(b,c,d,e))
#endif

#define FOR(i,n) for ((i) = 0; (i) < (n); (i)++)
#define LERP(a,b,t) ((a) + (t)*((b)-(a)))
#define FRAC(x,a,b) ((b)==(a)?.5:((x)-(a))/((b)-(a))) // inverse of LERP
#define BIT(x,i) (((x)>>(i))&1)
#define SETBIT(x,i) ((x)|=(1<<(i)))
#define SWAP(a,b,temp) ((temp)=(a),(a)=(b),(b)=(temp))
#define SIGN(x) ((x)>0?1:(x)<0?-1:0)
#define ISINT(x, tol) (fabs((x)-round(x)) < (tol))
#define INRANGE(foo,bar,baz) ((foo(bar))&&((bar)baz))
#define INRANGE4(foo,bar,baz,zar,raz) ((foo(bar))&&((bar)baz(zar))&&((zar)raz))
#define numberof(things) (sizeof(things)/sizeof(*(things)))
#define streq(a,b) (strcmp(a,b) == 0)
#define strneq(a,b,n) (strncmp(a,b,n) == 0)
#define log2(x) (log(x)*M_LOG2E)
#define log2int(x) ((int)round(log2(x)))/* could make this faster if critical */
#define logbase(x,base) (log(x)/log(base))
#define intLogBase(x,base) ((int)round(logbase(x,base)))
#define round(x) floor((x)+.5)
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define ABS(x) ((x)<0?-(x):(x))
#define PRD(x) printf("    %s = %.17g\n", #x, (double)(x));

// XXX there seems to be a bug where alloca can return nonaligned pointers...
// XXX this only manifests in the 2.2 version (where pfVec3d is
// XXX defined as pfVec3).
#undef alloca

#ifdef WIN32
#define alloca(n) ((void *)(((intptr_t)_alloca((n)+7)+7)&~7))
#else
#define alloca(n) ((void *)(((intptr_t)__builtin_alloca((n)+7)+7)&~7))
#endif

static int dcsParentPreApp(pfTraverser *trav, void *data);
static int returnAllIn(pfTraverser *, void *);

static void splitPatches(int &nPatches, struct InputPatchParams *&patches);
static int intersectArcs(double a0, double a1,
			 double b0, double b1,
			 double *result0, double *result1);
static int intersectArcs(double a0, double a1,
			 double b0, double b1,
			 double *Result00, double *Result01,
			 double *Result10, double *Result11);

#include "clipringsnode.h"
#define assert PFASSERTALWAYS // spherepatchwarp uses assert
#include "spherepatchwarp.h"

#ifdef WIN32
#include <Winsock2.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

extern "C" PFPFB_DLLEXPORT void pfdInitConverter_spherepatch3()
{
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "In pfdInitConverter_spherepatch3");

    ClipRingsNode::init();

#define get_from_atof_getenv(VARNAME) \
	if (getenv(#VARNAME) != NULL) \
	    if (*getenv(#VARNAME) != '\0') \
		VARNAME = atof(getenv(#VARNAME)); \
	    else \
		VARNAME = 1; // empty string means 1
    get_from_atof_getenv(_PFSPHEREPATCH_DEBUG);
    get_from_atof_getenv(_PFSPHEREPATCH_NO_PRECISION_HACK);
    get_from_atof_getenv(_PFSPHEREPATCH_NO_TRANSLATE);
    get_from_atof_getenv(_PFSPHEREPATCH_TTYINPUT);
    get_from_atof_getenv(_PFSPHEREPATCH_MAXSEGSLON);
    get_from_atof_getenv(_PFSPHEREPATCH_MAXSEGSLAT);
    get_from_atof_getenv(_PFSPHEREPATCH_MINSEGSLON);
    get_from_atof_getenv(_PFSPHEREPATCH_MINSEGSLAT);
    get_from_atof_getenv(_PFSPHEREPATCH_RINGRATIO);
    get_from_atof_getenv(_PFSPHEREPATCH_NRINGS);
    get_from_atof_getenv(_PFSPHEREPATCH_RADIUS);
    get_from_atof_getenv(_PFSPHEREPATCH_PATCHSHRINK);
    get_from_atof_getenv(_PFSPHEREPATCH_GLOBALSHRINK);
    get_from_atof_getenv(_PFSPHEREPATCH_TRADEOFF);
    get_from_atof_getenv(_PFSPHEREPATCH_COLOR_RINGS);
}



struct PatchParamsBase {
    double lon0, lat0, s0, t0;
    double lon1, lat1, s1, t1;
#ifdef DEFUNCT
    int lonsegs, latsegs;
#endif // DEFUNCT
};

struct InputPatchParams : public PatchParamsBase // PatchParams with extras...
{
    int origIndex; // where it was in the input file, before splitting/joining
    char texname[1024];

    // comparison function for qsort...
    static int strcmpTexName(const void *_a, const void *_b)
    {
	struct InputPatchParams *a = (struct InputPatchParams *)_a;
	struct InputPatchParams *b = (struct InputPatchParams *)_b;
	return strcmp(a->texname, b->texname);
    }
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
#ifdef DEFUNCT
    {"lonsegs", "%d", offsetof(InputPatchParams, lonsegs)},
    {"latsegs", "%d", offsetof(InputPatchParams, latsegs)},
#endif // DEFUNCT
};

extern "C" PFPFB_DLLEXPORT pfNode *
pfdLoadFile_spherepatch3(char *fileName)
{
    FILE *fp = pfdOpenFile(fileName);

    // If we are .spherepatch3 loader, try with 2 or nothing
    if (fp == NULL)
    {
	if (fileName == NULL)
	    return NULL; // don't even deal with it
	char newFileName[PF_MAXSTRING];
	strcpy(newFileName, fileName);
	newFileName[strlen(newFileName)-1] = '2';
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "Trying \"%s\"...\n", newFileName);
	fp = pfdOpenFile(newFileName);
    }
    if (fp == NULL)
    {
	if (fileName == NULL)
	    return NULL; // don't even deal with it
	char newFileName[PF_MAXSTRING];
	strcpy(newFileName, fileName);
	newFileName[strlen(newFileName)-1] = '\0';
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "Trying \"%s\"...\n", newFileName);
	fp = pfdOpenFile(newFileName);
    }

    if (fp == NULL)
	return NULL; // XXX prints error message?

    pfDoubleDCS *dcs = new pfDoubleDCS;
    PFASSERTALWAYS(dcs != NULL);

    int nPatches = 0;
    struct InputPatchParams *patches = (struct InputPatchParams *)pfMalloc(1*sizeof(*patches), NULL);
    PFASSERTALWAYS(patches != NULL);

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
	if (strneq(p, "latsegs", strlen("latsegs"))
	 || strneq(p, "latsegs", strlen("lonsegs")))
	{
	    continue; // ignore defunct arg
	    // XXX should latsegs, lonsegs give a hint for
	    // XXX the global variables,
	    // XXX which currently have no interface
	    // XXX except env variables?
	}
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
			     "pfdLoadFile_spherepatch3(\"%s\", line %d): variable %s redefined before variable(s) defined:", fileName, lineNum, variables[i].name);
		    int j;
		    FOR (j, numberof(variables))
			if (!BIT(which_params_got, j))
			    pfNotify(PFNFY_WARN, PFNFY_MORE,
				 "    %s", variables[j].name);
		    pfDelete(patches);
		    pfDelete(dcs);
		    return NULL;
		}
	}
	if (which_params_got == (1<<numberof(variables)) - 1)
	{

	    //
	    // All variables defined-- append the params to the list
	    // and start over.
	    //
	    patches = (struct InputPatchParams *)pfRealloc(patches, (nPatches+1)*sizeof(*patches)); // inefficient but who cares
	    PFASSERTALWAYS(patches != NULL);
	    params.origIndex = nPatches;
	    patches[nPatches++] = params;

	    which_params_got = 0;
	}
    }
    if (which_params_got != 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadFile_spherepatch3(\"%s\"): end-of-file reached while the following variables are undefined:", fileName);
	int j;
	FOR (j, numberof(variables))
	    if (!BIT(which_params_got, j))
		pfNotify(PFNFY_WARN, PFNFY_MORE,
		     "    %s", variables[j].name);
	pfDelete(patches);
	pfDelete(dcs);
	return NULL;
    }

    //
    // Clean up so lon0<=lon1 and lat0<=lat1...
    //
    {
	int iPatch;
	FOR (iPatch, nPatches)
	{
	    double temp;
	    struct InputPatchParams *patch = &patches[iPatch];

	    PFASSERTALWAYS(INRANGE(-90 <=, patch->lat0, <= 90));
	    PFASSERTALWAYS(INRANGE(-90 <=, patch->lat1, <= 90));

	    if (patch->lon0 > patch->lon1)
	    {
		SWAP(patch->lon0, patch->lon1, temp);
		SWAP(patch->s0, patch->s1, temp);
	    }
	    if (patch->lat0 > patch->lat1)
	    {
		SWAP(patch->lat0, patch->lat1, temp);
		SWAP(patch->t0, patch->t1, temp);
	    }

	    // XXX the following is necessary
	    // XXX due to some hacky interpolation later on...
	    // XXX should really fix this, or output
	    // XXX a coherent message...
	    PFASSERTALWAYS(patch->s0 != patch->s1);
	    PFASSERTALWAYS(patch->t0 != patch->t1);
	}
    }

    //
    // Make it so there is no overlap...
    //
    splitPatches(nPatches, patches); // pass by reference

    //
    // Get rid of any patches with texture name "NULL"
    // (these are just used for cutting holes in other patches).
    // XXX should this go in splitPatches so it will come out in the printout?
    //
    {
	int iPatch;
	FOR (iPatch, nPatches)
	{
	    if (streq(patches[iPatch].texname, "NULL"))
	    {
		memmove(&patches[iPatch], &patches[iPatch+1],
			(nPatches-(iPatch+1)) * sizeof(*patches));
		iPatch--;
		nPatches--;
		patches = (struct InputPatchParams *)pfRealloc(patches, nPatches*sizeof(*patches)); // so that pfGetSize() will return the right number
		PFASSERTALWAYS(patches != NULL);
	    }
	}
    }

    //
    // Sort by texture name...
    //
    qsort(patches, nPatches, sizeof(*patches), InputPatchParams::strcmpTexName);

    // XXX need a way to set these!
    // XXX global variables in config file?
    // XXX environment variables?
    int maxSegsLon = _PFSPHEREPATCH_MAXSEGSLON;
    int maxSegsLat = _PFSPHEREPATCH_MAXSEGSLAT;

    //
    // Calculate params for creating the ClipRingsNodes
    // from the global minSegsLon, maxSegsLon, minSegsLat, maxSegsLat.
    // (2*nPatches occurs instead of nPatches, because
    // the intersection of a patch with the visible disk
    // can be in two parts, and we process the parts separately).
    //
    int maxNumVerts = (maxSegsLon+2)*(maxSegsLat+2) // num global verts
		    + 2*nPatches * (2*(maxSegsLon+2) + (2*(maxSegsLat+2)));
		    // XXX nPatches could be refined to just
		    // XXX be the number for the tex group being created
    int maxNumStripsPerRing = 2*nPatches * (2 * (maxSegsLat+2));
    int maxNumIndicesPerRing = maxNumVerts * 4;
				    


    int iPatch;
    FOR (iPatch, nPatches)
    {
	params = patches[iPatch]; // struct copy... inefficent but who cares.
	//
	// First, try to find an existing group
	// associated with the texture...
	//
	pfTexture *tex;
	ClipRingsNode *texGroup;
	int i;
	FOR (i, numClipTextures)
	{
	    texGroup = (ClipRingsNode *)dcs->getChild(i);
	    tex = texGroup->getTex();
	    if (streq(params.texname, tex->getName()))
		break;
	}
	if (i == numClipTextures)
	{
	    //
	    // Haven't already seen this texture... create the texture
	    // and a new ClipRingsNode for it.
	    //

	    if (strstr(params.texname, ".ct") != NULL)
	    {
		tex = pfdLoadClipTexture(params.texname);
		if (tex == NULL)
		{
		    pfNotify(PFNFY_WARN, PFNFY_PRINT,
			     "pfdLoadFile_spherepatch3(\"%s\"): couldn't load clip texture \"%s\"\n", fileName, params.texname);
		    pfDelete(patches);
		    pfDelete(dcs);
		    return NULL;
		}
	    }
	    else
	    {
		tex = new(pfGetSharedArena()) pfTexture;
		PFASSERTALWAYS(tex != NULL);
		if (!tex->loadFile(params.texname))
		{
		    pfNotify(PFNFY_WARN, PFNFY_PRINT,
			     "pfdLoadFile_spherepatch3(\"%s\"): couldn't load texture \"%s\"\n", fileName, params.texname);
		    pfDelete(patches);
		    pfDelete(dcs);
		    return NULL;
		}
	    }
	    tex->setName(params.texname); // maybe not necessary?

	    //
	    // Figure out the max number of rings this clip rings group
	    // needs XXX (should it figure this out itself?)
	    //
	    float ringRatio = _PFSPHEREPATCH_RINGRATIO; // could be made per-texture, but not sure anyone would want to
	    int maxNumRings;
	    if (tex->isOfType(pfClipTexture::getClassType()))
	    {
		pfClipTexture *cliptex = (pfClipTexture *)tex;
		int vSizeS, vSizeT;
		cliptex->getVirtualSize(&vSizeS, &vSizeT, NULL);
		maxNumRings = 1 + (int)ceil(logbase(MAX(vSizeS,vSizeT)/32., ringRatio)); // full tex size down to approx. 32 texels (XXX somewhat arbitrary)
		maxNumRings = MAX(maxNumRings, 1);
		printf("    %s: vSize=%d, ringRatio=%g, maxNumRings=%d\n", tex->getName(), vSizeS, ringRatio, maxNumRings);
	    }
	    else
		maxNumRings = 1;

	    // Create a geostate for the tex group...
	    // XXX should it create its own?
	    // XXX probably not a big deal either way...
	    pfGeoState *geostate = new(pfGetSharedArena()) pfGeoState;
	    if (tex != NULL)
	    {
		geostate->setAttr(PFSTATE_TEXTURE, tex);
		geostate->setMode(PFSTATE_ENTEXTURE, PF_ON);
	    }
	    texGroup = new ClipRingsNode(maxNumRings,
					 maxNumVerts,
					 maxNumStripsPerRing,
					 maxNumIndicesPerRing,
					 tex,
					 geostate);
	    PFASSERTALWAYS(texGroup != NULL);
	    dcs->addChild(texGroup);

	    numClipTextures++;
	} // end if (the tex group didn't already exist for this patch)
    } // end for each patch

    dcs->setUserData(patches);

    //
    // Attach the callback to parent of DCS rather than
    // the DCS itself, to work around a bug
    // that makes the result of pfTraverser::getMat()
    // unreliable on an SCS or DCS.
    //
    pfGroup *dcsParent = new pfGroup();
    dcsParent->addChild(dcs);
    dcsParent->setTravFuncs(PFTRAV_APP, dcsParentPreApp, NULL);

    pfSphere bsph;
    bsph.center = pfVec3(0,0,0);
    bsph.radius = _PFSPHEREPATCH_RADIUS;
    dcs->setBound(&bsph, PFBOUND_STATIC);

    // Rather than deal with crude bounding spheres of all
    // the children, let's just say that
    // if the culling makes it down to the doubleDCS
    // (whose bounding volume is explicitly set to the whole sphere)
    // then consider all descendents to be in.
    dcsParent->setTravFuncs(PFTRAV_CULL, returnAllIn, NULL);

    return dcsParent;
} // end pfdLoadFile_spherepatch3()



//
// In case of overlap, later patches take precedence (appear to be "on top").
// The remaining visible part of the earlier patch is carved up
// into at most 4 pieces.
//
static void splitPatches(int &nPatches, struct InputPatchParams *&patches)
{
    {
	// XXX don't use printf
	printf("Before splitting:\n");
	printf("    %d patches:\n", nPatches);
	double totalArea = 0.;
	int i;
	FOR (i, nPatches)
	{
	    printf("        patch %d: tex name = \"%s\"\n", i, patches[i].texname);
	    printf("            lon0 = %.17g  lat0 = %.17g  s0 = %.17g  t0 = %.17g\n",
		patches[i].lon0, patches[i].lat0, patches[i].s0, patches[i].t0);
	    printf("            lon1 = %.17g  lat1 = %.17g  s1 = %.17g  t1 = %.17g\n",
		patches[i].lon1, patches[i].lat1, patches[i].s1, patches[i].t1);
	    totalArea += (patches[i].lon1-patches[i].lon0)
		       * (patches[i].lat1-patches[i].lat0);
#ifdef DEFUNCT
	    printf("            lonsegs = %d  latsegs = %d\n",
		patches[i].lonsegs, patches[i].latsegs);
#endif // DEFUNCT
	}
	printf("    total area = %.17g (%.17g of sphere)\n",
	       totalArea, totalArea / (360*180));
    }

    int splitter, splittee;
    FOR (splitter, nPatches)
    FOR (splittee, splitter)
    {
	// bounds of patch to be split
	double Lon0 = patches[splittee].lon0;
	double Lat0 = patches[splittee].lat0;
	double Lon1 = patches[splittee].lon1;
	double Lat1 = patches[splittee].lat1;

	// bounds of chunk to take out...
	// it's splitter moved by a multiple of 360 degrees lon
	// if necessary to get as close as possible to splittee,
	// and then intersected with splittee.
	double lon0 = patches[splitter].lon0;
	double lat0 = patches[splitter].lat0;
	double lon1 = patches[splitter].lon1;
	double lat1 = patches[splitter].lat1;
	while ((lon0+lon1)*.5 < (Lon0+Lon1)*.5 - 180.)
	{
	    lon0 += 360;
	    lon1 += 360;
	}
	while ((lon0+lon1)*.5 > (Lon0+Lon1)*.5 + 180.)
	{
	    lon0 -= 360;
	    lon1 -= 360;
	}
	lon0 = MAX(lon0, Lon0);
	lat0 = MAX(lat0, Lat0);
	lon1 = MIN(lon1, Lon1);
	lat1 = MIN(lat1, Lat1);
	if (lon0 < lon1 && lat0 < lat1)
	{
	    printf("patch %d(originally %d) splits %d(originally %d) into ", splitter, patches[splitter].origIndex, splittee, patches[splittee].origIndex);

	    struct InputPatchParams splitteePatch = patches[splittee]; // save
	    double S0 = patches[splittee].s0;
	    double T0 = patches[splittee].t0;
	    double S1 = patches[splittee].s1;
	    double T1 = patches[splittee].t1;

	    // Remove the splittee from the list...
	    memmove(patches+splittee, patches+splittee+1, (nPatches-splittee-1)*sizeof(*patches));
	    splittee--;
	    splitter--;
	    nPatches--;
	    patches = (struct InputPatchParams *)pfRealloc(patches, nPatches*sizeof(*patches)); // so that pfGetSize() will return the right number
	    PFASSERTALWAYS(patches != NULL);

#define INSERTPART(LON0, LAT0, LON1, LAT1) { \
		patches = (struct InputPatchParams *)pfRealloc(patches, (nPatches+1)*sizeof(*patches)); \
		PFASSERTALWAYS(patches != NULL); \
		memmove(patches+splittee+2, \
			patches+splittee+1, (nPatches-splittee-1)*sizeof(*patches)); \
		patches[splittee+1].lon0 = LON0; \
		patches[splittee+1].lon1 = LON1; \
		patches[splittee+1].lat0 = LAT0; \
		patches[splittee+1].lat1 = LAT1; \
		patches[splittee+1].s0 = LERP(S0, S1, ((LON0)-Lon0)/(Lon1-Lon0));\
		patches[splittee+1].t0 = LERP(T0, T1, ((LAT0)-Lat0)/(Lat1-Lat0));\
		patches[splittee+1].s1 = LERP(S0, S1, ((LON1)-Lon0)/(Lon1-Lon0));\
		patches[splittee+1].t1 = LERP(T0, T1, ((LAT1)-Lat0)/(Lat1-Lat0));\
		/*patches[splittee+1].lonsegs = (int)ceil(splitteePatch.lonsegs * ((LON0)-(LON1))/(Lon0-Lon1)); */\
		/*patches[splittee+1].latsegs = (int)ceil(splitteePatch.latsegs * ((LAT0)-(LAT1))/(Lat0-Lat1)); */\
		strcpy(patches[splittee+1].texname, splitteePatch.texname); \
		patches[splittee+1].origIndex = splitteePatch.origIndex; \
		splitter++; \
		nPatches++; \
	    }
	    // NOTE: do NOT increment splittee when inserting
	    // the pieces back in...
	    // The splitter must get another chance to split
	    // the resultant pieces (consider the case
	    // when the original splitter overlaps
	    // the original splittee from both the left and the right)!
	    // 

	    int nParts = 0;
	    if (Lat0 < lat0)
	    {
		printf("(bottom) ");
		INSERTPART(Lon0, Lat0, Lon1, lat0);
		nParts++;
	    }
	    if (Lon0 < lon0)
	    {
		printf("(left) ");
		INSERTPART(Lon0, lat0, lon0, lat1);
		nParts++;
	    }
	    if (lon1 < Lon1)
	    {
		printf("(right) ");
		INSERTPART(lon1, lat0, Lon1, lat1);
		nParts++;
	    }
	    if (lat1 < Lat1)
	    {
		printf("(top) ");
		INSERTPART(Lon0, lat1, Lon1, Lat1);
		nParts++;
	    }

	    printf("%d parts\n", nParts);
	}
    }

    //
    // Greedily join together adjacent patches
    // with the same lon range that came from the same original
    // patch.
    // (In general, it would be too difficult to test whether
    // the tex coord mappings match, but they definitely do
    // if the pieces came from the same original).
    //

    //
    int i, j;
    FOR (i, nPatches)
	for (j = i+1; j < nPatches; ++j)
	{
	    if (patches[i].origIndex == patches[j].origIndex
	     && patches[i].lon0 == patches[j].lon0
	     && patches[i].lon1 == patches[j].lon1
	     && MIN(patches[i].lat1, patches[j].lat1)
	     == MAX(patches[i].lat0, patches[j].lat0))
	    {
		printf("JOINING patch %d into patch %d (both originally %d) of %s\n", j, i, patches[i].origIndex, patches[i].texname);
		if (patches[i].lat1 == patches[j].lat0)
		{
		    // j is above i
		    patches[i].lat1 = patches[j].lat1;
		    patches[i].t1 = patches[j].t1;
		}
		else
		{
		    // j is below i
		    PFASSERTALWAYS(patches[i].lat0 == patches[j].lat1);
		    patches[i].lat0 = patches[j].lat0;
		    patches[i].t0 = patches[j].t0;
		}
		memmove(patches+j, patches+j+1, (nPatches-(j+1))*sizeof(*patches));
		j--;
		nPatches--;
		patches = (struct InputPatchParams *)pfRealloc(patches, nPatches*sizeof(*patches)); // so that pfGetSize() will return the right number
		PFASSERTALWAYS(patches != NULL);
	    }
	}

    //
    // Shrink patches so you can see them...
    //
    {
	int i;
	FOR (i, nPatches)
	{
	    double centerLon = (patches[i].lon0+patches[i].lon1)*.5;
	    double centerLat = (patches[i].lat0+patches[i].lat1)*.5;
	    patches[i].lon0 = LERP(centerLon, patches[i].lon0, _PFSPHEREPATCH_PATCHSHRINK);
	    patches[i].lon1 = LERP(centerLon, patches[i].lon1, _PFSPHEREPATCH_PATCHSHRINK);
	    patches[i].lat0 = LERP(centerLat, patches[i].lat0, _PFSPHEREPATCH_PATCHSHRINK);
	    patches[i].lat1 = LERP(centerLat, patches[i].lat1, _PFSPHEREPATCH_PATCHSHRINK);

	    // Shrink the whole mess by this amount
	    // so you can see it all on the front of the sphere
	    patches[i].lon0 *= _PFSPHEREPATCH_GLOBALSHRINK;
	    patches[i].lon1 *= _PFSPHEREPATCH_GLOBALSHRINK;
	    patches[i].lat0 *= _PFSPHEREPATCH_GLOBALSHRINK;
	    patches[i].lat1 *= _PFSPHEREPATCH_GLOBALSHRINK;
	}
    }


    {
	// XXX don't use printf
	printf("After splitting:\n");
	printf("    %d patches:\n", nPatches);
	double totalArea = 0.;
	int i;
	FOR (i, nPatches)
	{
	    printf("        patch %d(originally %d): tex name = \"%s\"\n", i, patches[i].origIndex, patches[i].texname);
	    printf("            lon0 = %.17g  lat0 = %.17g  s0 = %.17g  t0 = %.17g\n",
		patches[i].lon0, patches[i].lat0, patches[i].s0, patches[i].t0);
	    printf("            lon1 = %.17g  lat1 = %.17g  s1 = %.17g  t1 = %.17g\n",
		patches[i].lon1, patches[i].lat1, patches[i].s1, patches[i].t1);
	    totalArea += (patches[i].lon1-patches[i].lon0)
		       * (patches[i].lat1-patches[i].lat0);
#ifdef DEFUNCT
	    printf("            lonsegs = %d  latsegs = %d\n",
		patches[i].lonsegs, patches[i].latsegs);
#endif // DEFUNCT
	}
	printf("    total area = %.17g (%.17g of sphere)\n",
	       totalArea, totalArea / (360*180));
    }

/* printf("Hit return to continue: "); */
/* while (getchar() != '\n') ; */
}

inline double Fmod(double a, double b)
{
    double x = fmod(a, b);
    if (x < 0)
	x += b;
    return x;
}
inline double FmodAvg(double a, double b)
{
    return Fmod(a+b*.5, b) - b*.5;
}
#ifdef NOT_NEEDED_NOW_BUT_MIGHT_BE_USEFUL
inline double FmodNeg(double a, double b)
{
    return -Fmod(-a, b);
}
#endif // NOT_NEEDED_NOW_BUT_MIGHT_BE_USEFUL
inline double FmodClosest(double a, double b, double c)
{
    return c + FmodAvg(a-c, b);
}


static void shrinkPatches(int nPatches, struct InputPatchParams *patches,
			  double origPatchShrink, double patchShrink,
			  double origGlobalShrink, double globalShrink)
{

    if (patchShrink != origPatchShrink)
    {
	double scale = patchShrink / origPatchShrink;
	int iPatch;
	FOR (iPatch, nPatches)
	{
	    struct InputPatchParams *patch = &patches[iPatch];
	    double centerLon = (patch->lon0+patch->lon1)*.5;
	    double centerLat = (patch->lat0+patch->lat1)*.5;

	    PFASSERTALWAYS(INRANGE(-90. <=, patch->lat0, <= 90.));
	    PFASSERTALWAYS(INRANGE(-90. <=, patch->lat1, <= 90.));

	    patch->lon0 = LERP(centerLon, patch->lon0, scale);
	    patch->lon1 = LERP(centerLon, patch->lon1, scale);
	    patch->lat0 = LERP(centerLat, patch->lat0, scale);
	    patch->lat1 = LERP(centerLat, patch->lat1, scale);

	    // To avoid assertion failure later, clamp.
	    // XXX The only reason this should be necessary
	    // is for roundoff error due to back&forth...
	    // Probably it would be better to always work
	    // from the original input, but I'm
	    // being sloppy since this won't happen too
	    // many times per program...
	    patch->lat0 = PF_CLAMP(patch->lat0, -90., 90.);
	    patch->lat1 = PF_CLAMP(patch->lat1, -90., 90.);
	}
    }
    if (globalShrink != origGlobalShrink)
    {
	double scale = globalShrink / origGlobalShrink;
	int iPatch;
	FOR (iPatch, nPatches)
	{
	    struct InputPatchParams *patch = &patches[iPatch];

	    PFASSERTALWAYS(INRANGE(-90. <=, patch->lat0, <= 90.));
	    PFASSERTALWAYS(INRANGE(-90. <=, patch->lat1, <= 90.));

	    patch->lon0 *= scale;
	    patch->lon1 *= scale;
	    patch->lat0 *= scale;
	    patch->lat1 *= scale;

	    patch->lat0 = PF_CLAMP(patch->lat0, -90., 90.);
	    patch->lat1 = PF_CLAMP(patch->lat1, -90., 90.);
	}
    }
} // end shrinkPatches()

//
// Every frame, this gets called to process a line of tty input
// if there is one.  (if env _PFSPHEREPATCH_TTYINPUT is set)
//
static void ttyInput(pfGroup *dcsParent)
{
    fd_set fdset;
    struct timeval zero_timeout = {0,0};
    FD_ZERO(&fdset);
    FD_SET(0, &fdset);
    switch(select(1, &fdset, NULL, NULL, &zero_timeout))
    {
	case -1:
	    pfNotify(PFNFY_WARN, PFNFY_SYSERR,
		     "pfSpherePatch ttyInput: select failed: %s",
		     strerror(oserror()));
	    break;
	case 0:
	    if (_PFSPHEREPATCH_DEBUG)
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
			 "pfSpherePatch ttyInput: not ready for reading");
	    break;
	case 1:
	{
	    char buf[1024];
	    char *p = fgets(buf, sizeof(buf), stdin);
	    if (p != NULL)
	    {
		while (isspace(*p))
		    p++; // discard spaces
		if (p[0] != '\0')
		    p[strlen(p)-1] = '\0'; // get rid of newline
		if (_PFSPHEREPATCH_DEBUG)
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
			"Got input \"%s\"", p);
		
		// XXX The following is a mess- clean it up!!!

		double orig_PFSPHEREPATCH_PATCHSHRINK = _PFSPHEREPATCH_PATCHSHRINK;
		double orig_PFSPHEREPATCH_GLOBALSHRINK = _PFSPHEREPATCH_GLOBALSHRINK;

		static char prompt[1024] = "--> ";
		if (strstr(p, "help") != NULL
		 || strstr(p, "?")) // XXX very loose condition
		{
		    printf("\n");
		    printf("  Commands:\n");
		    printf("    help (or ?)\n");
		    printf("    prompt = \"<new prompt string>\"\n");
		    printf("    d = <_PFSPHEREPATCH_DEBUG value>\n");
		    printf("    nph = <_PFSPHEREPATCH_NO_PRECISION_HACK value>\n");
		    printf("    nt = <_PFSPHEREPATCH_NO_TRANSLATE value>\n");
		    printf("    ps = <_PFSPHEREPATCH_PATCHSHRINK value>\n");
		    printf("    gs = <_PFSPHEREPATCH_GLOBALSHRINK value>\n");
		    printf("    t = <_PFSPHEREPATCH_TRADEOFF value>\n");
		    printf("    cr = <_PFSPHEREPATCH_COLOR_RINGS value>\n");
		    printf("\n");
		}
		else if (sscanf(p, " d = %d", &_PFSPHEREPATCH_DEBUG) == 1)
		    printf("_PFSPHEREPATCH_DEBUG set to %d\n",
			    _PFSPHEREPATCH_DEBUG);
		else if (sscanf(p, " nph = %d", &_PFSPHEREPATCH_NO_PRECISION_HACK) == 1)
		    printf("_PFSPHEREPATCH_NO_PRECISION_HACK set to %d\n",
			    _PFSPHEREPATCH_NO_PRECISION_HACK);
		else if (sscanf(p, " nt = %d", &_PFSPHEREPATCH_NO_TRANSLATE) == 1)
		    printf("_PFSPHEREPATCH_NO_TRANSLATE set to %d\n",
			    _PFSPHEREPATCH_NO_TRANSLATE);
		else if (sscanf(p, " t = %f", &_PFSPHEREPATCH_TRADEOFF) == 1)
		    printf("_PFSPHEREPATCH_TRADEOFF set to %.9g\n",
			    _PFSPHEREPATCH_TRADEOFF);
		else if (sscanf(p, " ps = %lf", &_PFSPHEREPATCH_PATCHSHRINK) == 1)
		{
		    if (!INRANGE(0 <, _PFSPHEREPATCH_PATCHSHRINK, <= 1))

			_PFSPHEREPATCH_PATCHSHRINK = orig_PFSPHEREPATCH_PATCHSHRINK; // rejected!
		    printf("_PFSPHEREPATCH_PATCHSHRINK set to %.17g\n",
			    _PFSPHEREPATCH_PATCHSHRINK);

		    struct InputPatchParams *patches = (struct InputPatchParams *)dcsParent->getChild(0)->getUserData();
		    PFASSERTALWAYS(patches != NULL);
		    shrinkPatches(PFNUMBEROF(patches), patches,
				  orig_PFSPHEREPATCH_PATCHSHRINK,
				  _PFSPHEREPATCH_PATCHSHRINK,
				  1., 1.);

		}
		else if (sscanf(p, " gs = %lf", &_PFSPHEREPATCH_GLOBALSHRINK) == 1)
		{
		    if (!INRANGE(0 <, _PFSPHEREPATCH_GLOBALSHRINK, <= 1))
			_PFSPHEREPATCH_GLOBALSHRINK = orig_PFSPHEREPATCH_GLOBALSHRINK; // rejected!

		    printf("_PFSPHEREPATCH_GLOBALSHRINK set to %.17g\n",
			    _PFSPHEREPATCH_GLOBALSHRINK);
		    struct InputPatchParams *patches = (struct InputPatchParams *)dcsParent->getChild(0)->getUserData();
		    PFASSERTALWAYS(patches != NULL);
		    shrinkPatches(PFNUMBEROF(patches), patches,
				  1., 1.,
				  orig_PFSPHEREPATCH_GLOBALSHRINK,
				  _PFSPHEREPATCH_GLOBALSHRINK);
		}
		else if (sscanf(p, " cr = %f", &_PFSPHEREPATCH_COLOR_RINGS) == 1)
		    printf("_PFSPHEREPATCH_COLOR_RINGS saturation set to %.9g\n",
			    _PFSPHEREPATCH_COLOR_RINGS);
		else if (sscanf(p, " prompt = \"%[^\"]\"", prompt) == 1)
		{
		    // nothing; it will show up in the prompt
		}
		else if (*p == '\0')
		{
		    // empty string is not an error
		}
		else
		    printf("Sorry, I don't understand input \"%s\".\n", p);
		printf("%s", prompt);
		fflush(stdout);
	    } // end if fgets succeeded
	    else
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "pfSpherePatch ttyInput: fgets failed!: %s",
			 strerror(oserror()));
	    break;
	} // end case select() returned 1
    } // end switch on select() return value
} // end ttyInput()


//
// We'd do this on the DCS node itself
// except that a Performer bug makes the result of trav->getMat()
// flaky (sometimes includes the current DCS node's matrix, but it shouldn't).
//
static int
dcsParentPreApp(pfTraverser *trav, void *)
{
    pfChannel *chan = trav->getChan();
    pfNode *dcsParent = trav->getNode();

    //
    // If there are multiple channels,
    // this will get called multiple times...
    // return if already called on this node this frame!
    // All App traversals are in the same process so there
    // are no races...
    // XXX I think maybe the right thing to do here is
    // XXX really just to check against the ClipCenterNode's channel...?
    // XXX I think that's what it's there for!
    //
    {
	int curFrame = pfGetFrameCount();
	int *prevFrame = (int *)(dcsParent->getUserData());
	if (prevFrame == NULL)
	{
	    prevFrame = (int *)pfMalloc(sizeof(*prevFrame), pfGetSharedArena());
	    PFASSERTALWAYS(prevFrame != NULL);
	    *prevFrame = -1;
	    dcsParent->setUserData(prevFrame);
	}
	if (curFrame == *prevFrame)
	{
	    // XXX should probably do this if some DEBUG is set
	    /* printf("(dcsParentPreApp frame %d: called already, returning)\n", curFrame); */
	    return PFTRAV_CONT;
	}
	*prevFrame = curFrame;

	// XXX should probably do this if some DEBUG is set
	/* printf("(dcsParentPreApp frame %d: first call on this frame, proceeding)\n", curFrame); */
    }

    //
    // Read a line of tty input if there is one...
    // (don't do this unless env _PFSPHEREPATCH_TTYINPUT is set;
    // it's only okay if we're in a program that doesn't do any input
    // processing of its own, like perfly or clipfly.)
    //
    if (_PFSPHEREPATCH_TTYINPUT)
	ttyInput((pfGroup *)dcsParent);

    //
    // Get the eye in world (this node's world i.e. above our top dcs) space...
    // Note that we are getting this at the parent of the DCS rather than
    // the DCS itself, to work around a bug
    // that makes the result of pfTraverser::getMat()
    // unreliable when the node is an SCS or DCS.
    //
    pfVec3d eye;
    {
	pfMatrix viewMatf;
	pfMatrix4d viewMat, travMat, invTravMat, eyeSpaceToLocalSpaceMat;
	chan->getOffsetViewMat(viewMatf);//XXXI think Offset is wrong
	PFCOPY_MAT(viewMat, viewMatf);
	trav->getMatD(travMat);
	invTravMat.invertAff(travMat);
	eyeSpaceToLocalSpaceMat.mult(viewMat, invTravMat);
	PFCOPY_VEC3(eye, eyeSpaceToLocalSpaceMat[3]);
	// XXX could have an alternate test mode where
	// XXX it just gets the eye position
	// XXX from the last row of the view mat
	// and pipes it through the inv trav mat

	// So we can always think of the sphere as having radius 1
	// for LOD calculations and such...
	eye /= _PFSPHEREPATCH_RADIUS;
    }

    //
    // Set the doubleDCS's translate to be the eye (our local origin).
    // This is the origin with respect to which the
    // vertices are expressed, in SP.
    //
    pfVec3d localOrigin = eye;

    {
	if (_PFSPHEREPATCH_NO_TRANSLATE)
	{
	    /* printf("NOT using local origin %.17g,%.17g,%.17g\n", eye[0], eye[1], eye[2]); */
	    localOrigin.set(0,0,0);
	}
	else
	{
	    /* printf("YES using local origin %.17g,%.17g,%.17g\n", eye[0], eye[1], eye[2]); */
	}
    }

    pfDoubleDCS *doubleDCS = (pfDoubleDCS *)((pfGroup *)dcsParent)->getChild(0);
    PFASSERTALWAYS(doubleDCS->isOfType(pfDoubleDCS::getClassType())); // don't flatten!
    doubleDCS->setTrans(localOrigin[0], localOrigin[1], localOrigin[2]);

    //
    // Calculate the bounds of the visible disk,
    // centered at eyeLon, eyeLat.
    //
    double eyeLength = PFLENGTH_VEC3D(eye);
    eyeLength = PF_MAX2(eyeLength, 1.); // XXX should be some min dist?
    double HAT = eyeLength - 1.;
    HAT = MAX(HAT, 1./pow(2., 30.)); // XXX see note below at other use of pow

    double eyeLon = pfArcTan2d(eye[PF_X], -eye[PF_Y]);
    double eyeLat = pfArcTan2d(eye[PF_Z],
			       PFLENGTH_VEC2D(eye));
#ifdef DEBUG
printf("=================== Frame %d ==================\n", pfGetFrameCount());
printf("eye = %.17g,%.17g,%.17g\n", eye[0], eye[1], eye[2]);
printf("eyeLon = %.17g, eyeLat = %.17g\n", eyeLon, eyeLat);
#endif // DEBUG
    double visDiskRadiusLon;
    double visDiskRadiusLat;
    {
	eyeLon *= M_PI/180; // change from degrees to radians
	eyeLat *= M_PI/180; // change from degrees to radians
	double r = acos(1./eyeLength); // angular radius of visible disk
		// XXX is this unstable? there might be a smarter formulation

	//
	// Don't make the visible disk radius any smaller
	// than, say, 1/(1<<30) (i.e. roughly the order of magnitude
	// of a centimeter on the earth, or 1 texel if the earth
	// is textured with a 31-level clipmap).
	// This is kind of arbitrary, but we'll get zero-divides
	// if we try to use 0.
	// XXX (should just consider nothing visible in that case,
	// XXX and produce empty geometry!)
	//
	r = MAX(r, 1./pow(2., 30.));

	//
	// By hairy calculations done on paper,
	// the "longitudinal radius" of the visible disk
	// is equal to
	// atan2(sin(r),
	//       sqrt(sqr(cos(lat))*sqr(cos(r)) - sqr(sin(lat))*sqr(sin(r))))
	// The expression inside the sqrt can be rewritten as:
	//      sqr(cos(r)) - sqr(sin(lat))
	// or:  sqr(cos(lat)) - sqr(sin(r))
	// for better stability, take the cosine of the bigger number
	// and the sine of the smaller number.
	//
	// XXX looks like I did this backwards??? have to check with gnuplot...
	//
	double cos_min_r_lat = cos(PF_MIN2(r, eyeLat));
	double sin_max_r_lat = sin(PF_MAX2(r, eyeLat));
	double denomsqrd = PF_SQUARE(cos_min_r_lat) - PF_SQUARE(sin_max_r_lat);
	if (denomsqrd < 0)
	    visDiskRadiusLon = M_PI; // strictly contains N or S pole (XXX do we want to make this even bigger so it can always contain all patches so we never have to show the edges?
	else if (denomsqrd == 0)
	    visDiskRadiusLon = M_PI/2; // circumference passes through N or S pole
	else
	    visDiskRadiusLon = atan2(sin(r), sqrt(denomsqrd));
	visDiskRadiusLat = r;
	if (_PFSPHEREPATCH_DEBUG)
	{
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		     "HAT = %.17g, eye Lon,Lat = %.17g,%.17g visDiskRadiusLon,Lat = %.17g,%.17g",
		     HAT, eyeLon, eyeLat,
		     visDiskRadiusLon,visDiskRadiusLat);
	}

	//
	// Convert back to degrees (this kinda sucks)
	//
	eyeLon *= 180/M_PI;
	eyeLat *= 180/M_PI;
	visDiskRadiusLon *= 180/M_PI;
	visDiskRadiusLat *= 180/M_PI;
    }
// visDiskRadiusLon *= .5; // XXX fudge
// visDiskRadiusLat *= .5; // XXX fudge

    int maxSegsLon = _PFSPHEREPATCH_MAXSEGSLON;
    int maxSegsLat = _PFSPHEREPATCH_MAXSEGSLAT;
    int minSegsLon = _PFSPHEREPATCH_MINSEGSLON;
    int minSegsLat = _PFSPHEREPATCH_MINSEGSLAT;
    // The eye lon,lat mod the grid
    // should stay constant no matter
    // how we resize things...
    double eyeLonModGrid = Fmod(eyeLon / (360./maxSegsLon), 1.);
    double eyeLatModGrid = Fmod(eyeLat / (180./maxSegsLat), 1.);
    double uniformSegLengthLon = MIN(360./maxSegsLon,
				     2*visDiskRadiusLon/minSegsLon);
    double uniformSegLengthLat = MIN(180./maxSegsLat,
				     2*visDiskRadiusLat/minSegsLat);
    double targetNumSegsLon = 2*visDiskRadiusLon / uniformSegLengthLon;
    double targetNumSegsLat = 2*visDiskRadiusLat / uniformSegLengthLat;
    PFASSERTALWAYS(INRANGE(minSegsLon <=, (int)round(targetNumSegsLon), <= maxSegsLon));
    PFASSERTALWAYS(INRANGE(minSegsLat <=, (int)round(targetNumSegsLat), <= maxSegsLat));
	// XXX the above comparisons should be exact, but then it might fail
	// XXX due to roundoff error, so we compensate
	// XXX by rounding to nearest int.  This is much looser
	// XXX than necessary.

    // "grid space" is where 1 unit = grid spacing
    // and origin is at eyeLon,eyeLat.
    double visDiskLon0InGridSpace = -(visDiskRadiusLon/uniformSegLengthLon);
    double visDiskLat0InGridSpace = -(visDiskRadiusLat/uniformSegLengthLat);

    double visDiskLon0InGridSpaceModGrid = Fmod(eyeLonModGrid + visDiskLon0InGridSpace, 1.);
    double visDiskLat0InGridSpaceModGrid = Fmod(eyeLatModGrid + visDiskLat0InGridSpace, 1.);
    double boundingGridLon0InGridSpace = visDiskLon0InGridSpace - visDiskLon0InGridSpaceModGrid;
    double boundingGridLat0InGridSpace = visDiskLat0InGridSpace - visDiskLat0InGridSpaceModGrid;
    int nGlobalLons = (int)ceil(targetNumSegsLon) + 2;
    int nGlobalLats = (int)ceil(targetNumSegsLat) + 2;

    double *globalLons = (double *)alloca(nGlobalLons * (int)sizeof(double));
    double *globalLats = (double *)alloca(nGlobalLats * (int)sizeof(double));
    PFASSERTALWAYS(globalLons != NULL && globalLats != NULL);

    int i, j;
    FOR (j, nGlobalLons)
	globalLons[j] = eyeLon + (boundingGridLon0InGridSpace+j) * uniformSegLengthLon;
    FOR (i, nGlobalLats)
	globalLats[i] = eyeLat + (boundingGridLat0InGridSpace+i) * uniformSegLengthLat;

    if (nGlobalLons >= 2
     && globalLons[nGlobalLons-1 - 1] >= eyeLon+visDiskRadiusLon)
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "(Overestimated nGlobalLons by 1)\n"); // XXX have to find a clean way to prevent this from ever happening-- it's gross!
	nGlobalLons--; // overestimated by 1
	if (nGlobalLons >= 2
	 && globalLons[nGlobalLons-1 - 1] >= eyeLon+visDiskRadiusLon)
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "(Overestimated nGlobalLons by 2)\n"); // XXX have to find a clean way to prevent this from ever happening-- it's gross!
	    nGlobalLons--; // overestimated by 1
	}
    }
    if (nGlobalLats >= 2
     && globalLats[nGlobalLats-1 - 1] >= eyeLat+visDiskRadiusLat)
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "(Overestimated nGlobalLats by 1)\n"); // XXX have to find a clean way to prevent this from ever happening-- it's gross!
	nGlobalLats--; // overestimated by 1
	if (nGlobalLats >= 2
	 && globalLats[nGlobalLats-1 - 1] >= eyeLat+visDiskRadiusLat)
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "(Overestimated nGlobalLats by 2)\n"); // XXX have to find a clean way to prevent this from ever happening-- it's gross!
	    nGlobalLats--; // overestimated by 1
	}
    }

#ifdef DEBUG
    {
	printf("visDiskRadius lon=%.17g lat=%.17g\n",
		visDiskRadiusLon, visDiskRadiusLat);
	printf("Before clamping edges to visible disk:\n");
	printf("    %d globalLons:", nGlobalLons);
	FOR (j, nGlobalLons)
	    printf(" %.17g", globalLons[j]);
	printf("\n");
	printf("    %d globalLats:", nGlobalLats);
	FOR (i, nGlobalLats)
	    printf(" %.17g", globalLats[i]);
	printf("\n");
    }
#endif // DEBUG

    // XXX bleah... everything up to here is way too complicated!!!

    // clamp grid coords to visible disk rectangle...
    globalLons[0] = MAX(globalLons[0], eyeLon-visDiskRadiusLon);
    globalLats[0] = MAX(globalLats[0], eyeLat-visDiskRadiusLat);
    globalLons[nGlobalLons-1] = MIN(globalLons[nGlobalLons-1], eyeLon+visDiskRadiusLon);
    globalLats[nGlobalLats-1] = MIN(globalLats[nGlobalLats-1], eyeLat+visDiskRadiusLat);

#ifdef DEBUG
    {
	printf("After clamping edges to visible disk:\n");
	printf("    %d globalLons:", nGlobalLons);
	FOR (j, nGlobalLons)
	    printf(" %.17g", globalLons[j]);
	printf("\n");
	printf("    %d globalLats:", nGlobalLats);
	FOR (i, nGlobalLats)
	    printf(" %.17g", globalLats[i]);
	printf("\n");
    }
#endif // DEBUG

    //
    // When in really close, uniform grid spacing is not sufficient--
    // the spacing might be coarser than the minimum required
    // clipring size (for per-ring clip texture param adjustment).
    // If that is the case,
    // warp the grid by a smooth bi-"linear-exponential" function
    // (constant shrinkage by the required amount
    // in a neighborhood of the center, exponential expansion near the edges)
    // that keeps the center and visdisk rectangle perimeter constant.
    //
    // Aim for the seg length that fills a 1Kx1K
    // screen when looking straight at it with 90 degree FOV.
    // XXX need to use actual FOV!! this will screw up for very narrow FOVs!)

    double shrunkSegLengthLon, shrunkSegLengthLat;
    {
	shrunkSegLengthLon = 2*HAT * 180/M_PI;
	shrunkSegLengthLat = 2*HAT * 180/M_PI;
#ifdef DEBUG
PRD(uniformSegLengthLon);
PRD(uniformSegLengthLat);
PRD(shrunkSegLengthLon);
PRD(shrunkSegLengthLat);
#endif // DEBUG
	if (shrunkSegLengthLon < uniformSegLengthLon)
	{
#ifdef DEBUG
printf("WARPING IN S! slope = %.17g\n", shrunkSegLengthLon/uniformSegLengthLon);
#endif // DEBUG
	    SpherePatchWarp warpLon;
	    warpLon.makeFromSlope(shrunkSegLengthLon/uniformSegLengthLon);
	    int j;
	    FOR (j, nGlobalLons)
		globalLons[j] = warpLon.func((globalLons[j]-eyeLon)/visDiskRadiusLon) * visDiskRadiusLon + eyeLon;
	}
	else
	    shrunkSegLengthLon = uniformSegLengthLon;
	if (shrunkSegLengthLat < uniformSegLengthLat)
	{
#ifdef DEBUG
printf("WARPING IN T! slope = %.17g\n", shrunkSegLengthLat/uniformSegLengthLat);
#endif // DEBUG
	    SpherePatchWarp warpLat;
	    warpLat.makeFromSlope(shrunkSegLengthLat/uniformSegLengthLat);
	    int i;
	    FOR (i, nGlobalLats)
		globalLats[i] = warpLat.func((globalLats[i]-eyeLat)/visDiskRadiusLat) * visDiskRadiusLat + eyeLat;
	}
	else
	    shrunkSegLengthLat = uniformSegLengthLat;
    }
#ifdef DEBUG
    {
	printf("After warping:\n");
	printf("    %d globalLons:", nGlobalLons);
	FOR (j, nGlobalLons)
	    printf(" %.17g", globalLons[j]);
	printf("\n");
	printf("    %d globalLats:", nGlobalLats);
	FOR (i, nGlobalLats)
	    printf(" %.17g", globalLats[i]);
	printf("\n");
    }
#endif // DEBUG

    struct InputPatchParams *patches = (struct InputPatchParams *)doubleDCS->getUserData();
    PFASSERTALWAYS(patches != NULL);
    int nPatches = PFNUMBEROF(patches);
    int iPatch = 0;

    int nTextures = doubleDCS->getNumChildren();
    int iTexture;
    FOR (iTexture, nTextures)
    {
	ClipRingsNode *clipRingsNode = (ClipRingsNode *)doubleDCS->getChild(iTexture);
	pfTexture *tex = clipRingsNode->getTex();
	const char *texName = tex->getName();

	//
	// Search through all the patches of this texture
	// and find the one closest to eyeLat,eyeLon.
	//
	double centerS=.5, centerT=.5;
	int closestPatchIndex = -1;
	double closestDistSqrd = MAXDOUBLE;
	float min_dst_dxyz = MAXFLOAT;
	{
	    int iMin = iPatch;
	    int iMax = iPatch-1;
	    while (iMax+1 < nPatches
		&& streq(texName, patches[iMax+1].texname))
		iMax++;
	    // iMin,iMax are now the indices of the first and last patch
	    // on this texture (XXX this is a lame way of doing this; should
	    // have per-texture patch lists that don't interact with
	    // each other)
	    if (iMax < iMin)
		continue; // no patches for this texture?
#ifdef DEBUG
	    printf("Finding center for texture %s...\n", patches[iMin].texname);
#endif // DEBUG
	    int i;
	    for (i = iMin; i <= iMax; ++i)
	    {
		struct InputPatchParams *patch = &patches[i];

		/* XXX this calculation is not quite right */
		min_dst_dxyz = PF_MIN3(min_dst_dxyz,
		ABS((patch->s1-patch->s0)/((patch->lon1-patch->lon0)*M_PI/180)),
		ABS((patch->t1-patch->t0)/((patch->lat1-patch->lat0)*M_PI/180)));

		double patchCenterLon = (patch->lon0+patch->lon1)*.5;
		double _eyeLon = FmodClosest(eyeLon, 360., patchCenterLon);
		double thisPatchClosestLon  = PF_CLAMP(_eyeLon, patch->lon0,
							        patch->lon1);
		double thisPatchClosestLat  = PF_CLAMP(eyeLat, patch->lat0,
							       patch->lat1);
		pfVec3d thisPatchClosestPoint;
		{
		    double sinLon, cosLon, sinLat, cosLat;
		    pfSinCosd(thisPatchClosestLon, &sinLon, &cosLon);
		    pfSinCosd(thisPatchClosestLat, &sinLat, &cosLat);
		    thisPatchClosestPoint[PF_X] = sinLon * cosLat;
		    thisPatchClosestPoint[PF_Y] = -cosLon * cosLat;
		    thisPatchClosestPoint[PF_Z] = sinLat;
		}

		double thisPatchDistSqrd = PFSQR_DISTANCE_PT3(eye,
							thisPatchClosestPoint);
#ifdef DEBUG
		printf("    checking patch lon0=%.17g lon1=%.17g lat0=%.17g lat1=%.17g\n",
		       patch->lon0, patch->lon1, patch->lat0, patch->lat1);
		printf("        eye lon,lat = %.17g,%.17g\n", _eyeLon, eyeLat);
		printf("        thisPatchDistSqrd = %.17g\n", thisPatchDistSqrd);
		printf("        (thisPatchDist = %.17g)\n", sqrt(thisPatchDistSqrd));
		PRD(_eyeLon-patch->lon1);
		PRD(patch->lon0-_eyeLon);
		PRD(eyeLat-patch->lat1);
		PRD(patch->lat1-eyeLat);
#endif // DEBUG

		if (thisPatchDistSqrd < closestDistSqrd)
		{
		    centerS = LERP(patch->s0, patch->s1,
			      FRAC(_eyeLon, patch->lon0, patch->lon1));
		    centerT = LERP(patch->t0, patch->t1,
			      FRAC(eyeLat, patch->lat0, patch->lat1));
		    closestDistSqrd = thisPatchDistSqrd;
		    closestPatchIndex = i;
#ifdef DEBUG
		    printf("        closer than previous one.\n");
		    printf("            center = %.17g,%.17g\n", centerS, centerT);
#endif // DEBUG
		}
		else
		{
#ifdef DEBUG
		    printf("        not as close as a previous one.\n");
#endif // DEBUG
		}
	    } // end for each patch of this texture
	} // end finding closest patch on this texture


	//
	// Calculate minLODTexPix... (the min LOD
	// that could possibly be selected by the graphics hardware,
	// on the basis of partial derivatives ds/dx, ds/dy, dt/dx, dt/dy
	//
	float minLODTexPix = 0.;
	if (clipRingsNode->getTex()->isOfType(pfClipTexture::getClassType()))
	{
	    pfClipTexture *cliptex = (pfClipTexture *)clipRingsNode->getTex();
	    pfMPClipTexture *mpcliptex = clipRingsNode->getMPClipTexture(); // XXX is this legit? did I want to expose the derivation from pfuClipCenterNode?
	    PFASSERTALWAYS(mpcliptex != NULL);
	    int vSizeS, vSizeT;
	    cliptex->getVirtualSize(&vSizeS, &vSizeT, NULL);


	    pfFrustum *appfrust = new pfFrustum; // XXX per-frame malloc!!! get rid of this!
	    int viewportWidth, viewportHeight;
	    chan->getBaseFrust(appfrust);
	    chan->getSize(&viewportWidth, &viewportHeight);
		/* XXX wrong-- should use culling frustum! */
	    float minlod_size = pfuCalcSizeFinestMipLOD(appfrust,
							viewportWidth, viewportHeight,
							HAT,
							min_dst_dxyz,
							vSizeS);
	    pfDelete(appfrust); // XXX per-frame delete! get rid of this!

	    float LODBiasS, LODBiasT;
	    mpcliptex->getLODBias(&LODBiasS, &LODBiasT, NULL);

		// XXX workaround for bizarre values of LODBiasS and LODBiasT...
		// XXX If I set the values to 0 after pfdLoadClipTexture,
		// XXX they don't seem to hold, so have to special case here.
		if (LODBiasS == PFTEX_DEFAULT) LODBiasS = 0.;
		if (LODBiasT == PFTEX_DEFAULT) LODBiasT = 0.;

	    int nLevels = log2int(vSizeS)+1;
	    minLODTexPix = nLevels-1 - log2(minlod_size) + PF_MIN2(LODBiasS, LODBiasT);
	    {
	    //
	    // While we're at it...
	    // Set this mp clip texture's texLoadTimeFrac
	    // equal to 1/HAT, so that near textures
	    // will get proportionally more time than far textures.
	    // Only proportions are relevant (all of the pipe's
	    // pfMPClipTextures' fracs are normalized so that their sum is 1).
	    //
	    float closestDist = sqrtf((float)closestDistSqrd);
	    float texLoadTimeFrac = 1.f/PF_MAX2(closestDist, 1e-10f);
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
		mpcliptex->setTexLoadTime(-1.); // so it doesn't override our frac,
						// but gui tex load time will
						// be ignored
	    }

	}

	int nRings;
	float smallestRingRadius;
	float ringRatio;
	{
	    ringRatio = _PFSPHEREPATCH_RINGRATIO;
	    nRings = _PFSPHEREPATCH_NRINGS;
	    if (nRings < 0) // means automatically calculate it
	    {
		if (tex->isOfType(pfClipTexture::getClassType()))
		{
		    PFASSERTALWAYS(closestPatchIndex != -1);
		    struct InputPatchParams *patch = &patches[closestPatchIndex];
		    double lon0 = patch->lon0;
		    double lon1 = patch->lon1;
		    double lat0 = patch->lat0;
		    double lat1 = patch->lat1;
		    double s0 = patch->s0;
		    double s1 = patch->s1;
		    double t0 = patch->t0;
		    double t1 = patch->t1;
		    // Smallest ring should be twice the size
		    // of the smallest quad, so it will contain that
		    // quad no matter where the center is modulo the grid.
		    smallestRingRadius = 
			MAX(shrunkSegLengthLon * ABS((s1-s0)/(lon1-lon0)),
			    shrunkSegLengthLat * ABS((t1-t0)/(lat1-lat0)));
		    nRings = (int)ceil(logbase(1./smallestRingRadius,ringRatio));
		    nRings = PF_MAX2(nRings, 1);
		    if (nRings > clipRingsNode->getMaxNumRings())
		    {
			if (_PFSPHEREPATCH_DEBUG)
			    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
				     "Clamping nRings from %d to %d for %s\n",
				     nRings, clipRingsNode->getMaxNumRings(),
				     patch->texname);
			// XXX This is the wrong thing to do in this case--
			// XXX shrunkSegLength should simply never be allowed
			// XXX to get this small, it doesn't do any good!
			// XXX Well actually this gets subtle near
			// XXX the boundary of two differing-resolution
			// XXX clip textures...
			// XXX The smallest ring radius
			// XXX should probably depend on the resolution
			// XXX of the clip texture in question!
			// XXX this be fixed!
			nRings = clipRingsNode->getMaxNumRings();
		    }
#ifdef DEBUG
printf("Texture name: %s\n", patch->texname);
PRD(shrunkSegLengthLon);
PRD(shrunkSegLengthLat);
PRD(smallestRingRadius);
PRD(ringRatio);
PRD(logbase(1./smallestRingRadius,ringRatio));
PRD(ceil(logbase(1./smallestRingRadius,ringRatio)));
PRD((int)ceil(logbase(1./smallestRingRadius,ringRatio)));
PRD(nRings);
#endif // DEBUG
		}
		else
		{
		    nRings = 1;
		    smallestRingRadius = 1.; // arbitrary; it's ignored
		}
	    }
	    else
	    {
		// nRings was specified; make it so that
		// the biggest ring has radius 1...
		smallestRingRadius = 1. / pow(ringRatio, nRings-1);
	    }
	}
	
	clipRingsNode->reset(centerS, centerT,
			     nRings,
			     smallestRingRadius,
			     ringRatio);

	for (; iPatch < nPatches && streq(texName, patches[iPatch].texname); iPatch++)
	{
	    struct PatchParamsBase *patch = &patches[iPatch];
	    double lat0 = patch->lat0;
	    double lat1 = patch->lat1;
	    double t0 = patch->t0;
	    double t1 = patch->t1;

	    //
	    // There is only one space for lat...
	    // (XXX not really true, but this makes it true,
	    // and it's not too unreasonable)
	    //
	    PFASSERTALWAYS(INRANGE4(-90. <=, lat0, <=, lat1, <= 90.));
	    lat0 = PF_CLAMP(lat0, eyeLat-visDiskRadiusLat,
				  eyeLat+visDiskRadiusLat);
	    lat1 = PF_CLAMP(lat1, eyeLat-visDiskRadiusLat,
				  eyeLat+visDiskRadiusLat);
	    if (lat0 >= lat1)
		continue; // no part of this patch is visible in lat

	    t0 = LERP(patch->t0, patch->t1,
		      FRAC(lat0, patch->lat0, patch->lat1));
	    t1 = LERP(patch->t0, patch->t1,
		      FRAC(lat1, patch->lat0, patch->lat1));


	    //
	    // Intersection of patch with visDisk rectangle
	    // can have 0, 1, or 2 pieces, due to wrapping in lon...
	    // send the pieces down separately.
	    //
	    double lonPieces[2][2];
	    int nPieces = intersectArcs(patch->lon0, patch->lon1,
				      eyeLon-visDiskRadiusLon,
				      eyeLon+visDiskRadiusLon,
				      &lonPieces[0][0], &lonPieces[0][1],
				      &lonPieces[1][0], &lonPieces[1][1]);
	    double sphereRadius = _PFSPHEREPATCH_RADIUS;
	    int iPiece;
	    FOR (iPiece, nPieces)
	    {
		double lon0 = lonPieces[iPiece][0];
		double lon1 = lonPieces[iPiece][1];
		double s0 = LERP(patch->s0, patch->s1,
				 FRAC(lon0, patch->lon0, patch->lon1));
		double s1 = LERP(patch->s0, patch->s1,
				 FRAC(lon1, patch->lon0, patch->lon1));

		//
		// Convert lon0,lon1 to global space
		// by reversing direction if necessary, and
		// adding/subtracting multiples of 360.
		//
		if (lon0 > lon1)
		{
		    double temp;
		    SWAP(lon0, lon1, temp);
		    SWAP(s0, s1, temp);
		}
		double centerLon = (lon0+lon1)*.5;
		while (centerLon < eyeLon-180.)
		{
		    centerLon += 360;
		    lon0 += 360;
		    lon1 += 360;
		}
		while (centerLon > eyeLon+180.)
		{
		    centerLon -= 360;
		    lon0 -= 360;
		    lon1 -= 360;
		}
		    
		clipRingsNode->addSpherePatchMesh(
			lon0, lon1, lat0, lat1,
			s0, s1, t0, t1,
			nGlobalLons,
			nGlobalLats,
			globalLons,
			globalLats,
			sphereRadius,
			localOrigin);
	    } // end for iPiece
	} // end for iPatch
	clipRingsNode->complete(minLODTexPix);
    } // end for iTexture

    return PFTRAV_CONT;
} // end dcsParentPreApp()

// Pre-cull func for all of a node's children to
// be trivially accepted...
static int
returnAllIn(pfTraverser *, void *)
{
    pfCullResult(PFIS_MAYBE|PFIS_TRUE|PFIS_ALL_IN);
    return PFTRAV_CONT;
}

//
// Intersect two circular arcs, measured in degrees,
// where values that are equal mod 360 are considered
// equal.  Arcs may go in either direction.
// The result is in the space of the first arc argument,
// e.g. intersectArcs([100..0], [80..130]) == [100..80].
// If the actual intersection is in two pieces,
// the entire first arc argument is returned.
// The endpoints are not included.
// The function return value is the number of pieces (0, 1, or 2).
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
    // calculate lo,hi of union...
    double lo = PF_MIN2(a0, b0);
    double hi = PF_MAX2(a1, b1);
    if (hi-lo > 360 // endpoints not included
     && hi-lo > a1-a0 // to rule out case where a slightly overlaps itself
     && hi-lo > b1-b0) // to rule out case where b slightly overlaps itself
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

//
// Intersect two circular arcs, measured in degrees,
// where values that are equal mod 360 are considered
// equal.  Arcs may go in either direction.
// The result is in the space of the first arc argument,
// e.g. intersectArcs([100..0], [80..130]) == [100..80].
// If the actual intersection is in two pieces,
// the entire first arc argument is returned.
// The endpoints are not included.
// The function return value is the number of pieces (0, 1, or 2).
//
static int intersectArcs(double a0, double a1,
			 double b0, double b1,
			 double *Result00, double *Result01,
			 double *Result10, double *Result11)
{
    int returnval;
    returnval = intersectArcs(a0, a1, b0, b1, Result00, Result01);
    if (returnval == 2)
    {
	//
	// Really need to figure out the parts...
	//

	// Figure out the complement of b in the full circle...
	// (order of b's endpoints is irrelevant)
	double compl_b0 = MIN(b0,b1);
	double compl_b1 = MAX(b0,b1) - 360;
	// Intersect that with a to get a-b (i.e. a intersect complement(b))
	double a_minus_b_0, a_minus_b_1;
	int a_minus_b_returnval = intersectArcs(a0, a1, compl_b0, compl_b1,
						&a_minus_b_0, &a_minus_b_1);
	PFASSERTALWAYS(a_minus_b_returnval <= 1);
	if (a_minus_b_returnval == 1)
	{
	    *Result11 = *Result01;
	    *Result01 = a_minus_b_0;
	    *Result10 = a_minus_b_1;
	    return 2;
	}
	else
	{
	    PFASSERTALWAYS(a_minus_b_returnval == 0);
	    // the difference was zero-length, I guess...
	    // in this case we return a after all.
	    return 1;
	}
    }
    else
    {
	// 0 or 1 pieces
	return returnval;
    }
}
