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
 * pffakeclip.c
 *
 * XXX should interact with the following widgets:
 *	Menus:
 *		Image:
 *			Leo
 *			Ed&TJ
 *			S.F.
 *			S.F. Gridified
 *		Clip Center:
 *			Auto
 *			Fixed
 *			Mouse Drag
 *		Virtual Tex Size:
 *		Clip Size:
 *			1x1
 *			2x2
 *			4x4
 *			8x8
 *			16x16
 *			32x32
 *			64x64
 *			128x128
 *			256x256
 *			(up through virtual texture size)
 *			(change with virt tex size)
 *		Clip Visualization
 *			Roam
 *			Wrap
 *		Interpolation
 *			Nearest Neighbor
 *			Bilinear
 *
 *	Sliders:
 *		Min LOD
 *		    [0..nlevels-1]
 *		Invalid Border
 *		    [1..clipsize/2], log scale (change with clip size)
 */
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfSCS.h>
#include     <Performer/pf/pfGroup.h>
#include         <Performer/pf/pfText.h>
#include             <Performer/pr/pfString.h>
#include         <Performer/pf/pfGeode.h>
#include             <Performer/pr/pfGeoSet.h>
#include                 <Performer/pr/pfGeoState.h>
#include                     <Performer/pr/pfMaterial.h>
#include                     <Performer/pr/pfTexture.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <stdlib.h>
#ifdef __linux__
#include <kpathsea/c-pathmx.h>
#endif
#include "pyramid.h"

#define streq(a,b) (strcmp(a,b) == 0)
#define FOR(i,n) for ((i) = (0); (i) < (n); ++(i))
#define INRANGE(foo,bar,baz) ((foo(bar))&&((bar)baz))
#define EXPAND2(v) (v)[0], (v)[1]
#define EXPAND3(v) (v)[0], (v)[1], (v)[2]
#define EXPAND4(v) (v)[0], (v)[1], (v)[2], (v)[3]
#define round(x) floor((x)+.5)
#define log2(x) (log(x)/M_LN2)

static int _PFFAKECLIP_TRY_IDENTMAT; /* XXX not shared! will not work multiprocess */
static int _PFFAKECLIP_TRY_SETTING_MAX_LOD;
static int _PFFAKECLIP_TRY_MESSING_WITH_FILTER_IN_DRAW;

#include "fakeclipguistate.h"
struct _FakeClipGuiState *FakeClipGuiState;
struct _FakeClipGuiState oldFakeClipGuiState;	/* so we can detect changes */

static pfFont *theFont = NULL;
static int haveLODRange = -1;

/*
 * Calculate 10 vertices forming a triangle strip (8 triangles)
 * in the shape of a square with a smaller square cut out of the middle.
 * Both squares are shrunk and then moved as necessary so that all coords
 * are in the range 0..1.
 */
static void
GetDonut(float centerx, float centery,
	 float bigdiameter, float littlediameter,
	 pfVec2 verts[10])
{
    float r0 = PF_MIN2(littlediameter*.5f, .5f);
    float r1 = PF_MIN2(bigdiameter*.5f, .5f);
    float c0x = PF_CLAMP(centerx, r0, 1.0f-r0);
    float c0y = PF_CLAMP(centery, r0, 1.0f-r0);
    float c1x = PF_CLAMP(centerx, r1, 1.0f-r1);
    float c1y = PF_CLAMP(centery, r1, 1.0f-r1);
    PFSET_VEC2(verts[0], c0x-r0, c0y-r0);
    PFSET_VEC2(verts[1], c1x-r1, c1y-r1);
    PFSET_VEC2(verts[2], c0x+r0, c0y-r0);
    PFSET_VEC2(verts[3], c1x+r1, c1y-r1);
    PFSET_VEC2(verts[4], c0x+r0, c0y+r0);
    PFSET_VEC2(verts[5], c1x+r1, c1y+r1);
    PFSET_VEC2(verts[6], c0x-r0, c0y+r0);
    PFSET_VEC2(verts[7], c1x-r1, c1y+r1);
    PFSET_VEC2(verts[8], c0x-r0, c0y-r0);
    PFSET_VEC2(verts[9], c1x-r1, c1y-r1);
}

struct FakeClipParams {
    pfVec2 center;	/* in texture coords */
    int clipSize;
    float invalid_border;
    float min_lod;

    int nlevels;
    pfTexture *texture;

    int visualize_wrap;
    int visualize_roam;
    int visualize_inlevel;
};

struct FakeClipLevelParams {
    int myindex;
    struct FakeClipParams *params;
};

static void
SetFakeClipNodeParams(pfNode *node,
		      float centerx, float centery,
		      int clipSize,
		      float invalid_border,
		      float min_lod)
{
    pfGroup *group = (pfGroup *)node;
    struct FakeClipParams *params =
	(struct FakeClipParams *) group->getUserData();

    /* min_lod requires no further work */
    params->min_lod = min_lod;

    if (params->center[0] != centerx
     || params->center[1] != centery
     || params->clipSize != clipSize
     || params->invalid_border != invalid_border)
    {
	params->center[0] = centerx;
	params->center[1] = centery;
        params->clipSize = clipSize;
        params->invalid_border = invalid_border;

	int i, j, nlevels = group->getNumChildren();
	float bigdiam, littlediam = 1.;

	FOR (i, nlevels)
	{
	    bigdiam = littlediam;
	    if (i == nlevels-1)
		littlediam = 0.; /* it's the finest level; nothing smaller */
	    else if ((1<<(i+1)) <= clipSize)
		littlediam = 1.; /* next smaller one is still a pyramid level*/
	    else
		littlediam = (float)(clipSize-2*invalid_border)/(1<<(i+1));

	    pfNode *child = group->getChild(i);
	    PFASSERTALWAYS(child->isOfType(pfGeode::getClassType()));
	    pfGeode *geode = (pfGeode *)child;
	    pfGeoSet *geoset = geode->getGSet(0);

	    pfVec3 *vcoords;
	    pfVec2 *tcoords;
	    ushort *dummy;

	    geoset->getAttrLists(PFGS_COORD3, (void **)&vcoords, &dummy);
	    geoset->getAttrLists(PFGS_TEXCOORD2, (void **)&tcoords, &dummy);
	    PFASSERTALWAYS(vcoords != NULL && tcoords != NULL);
	    GetDonut(centerx, centery, bigdiam, littlediam, tcoords);
	    FOR (j, 10)
		PFSET_VEC3(vcoords[j], tcoords[j][0], tcoords[j][1], 0.0f);

	    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
	    geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, vcoords, NULL);
	}
    }
}

static int
levelPreDrawFunc(pfTraverser *, void *arg)
{
#ifdef DEFUNCT /* can't do this here, since the node (even the traverser?) is bogus in draw callbacks, but might put this back if moved to the cull or app */
    pfNode *node = trav->getNode();
    pfGroup *parent = node->getParent(0);
    PFASSERTALWAYS(parent->isOfType(pfGroup::getClassType()));
    int myindex = parent->searchChild(node);
    PFASSERTALWAYS(myindex >= 0);
    struct FakeClipParams *params = (struct FakeClipParams *)arg;
    pfTexture *texture = (pfTexture *)((pfGeode *)node)->getGSet(0)->getGState()->getAttr(PFSTATE_TEXTURE);
#endif /* DEFUNCT */
    struct FakeClipLevelParams *levelParams = (struct FakeClipLevelParams *)arg;
    int myindex = levelParams->myindex;
    struct FakeClipParams *params = levelParams->params;
    pfTexture *texture = params->texture;
    float clipmap_min_lod = params->min_lod;
    float min_lod = PF_MAX2(clipmap_min_lod,
			    params->nlevels-1-myindex);

    PFASSERTALWAYS(texture != NULL);
    PFASSERTALWAYS(texture->isOfType(pfTexture::getClassType()));
/* printf("%d: texture 0x%p\n", myindex, texture); */

    texture->setFilter(PFTEX_MINFILTER, FakeClipGuiState->minfilter);
    texture->setFilter(PFTEX_MAGFILTER, FakeClipGuiState->magfilter);

    /*
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "%dx%d donut level: Setting minlod to %g",
	     (1<<myindex), (1<<myindex),
	     min_lod);
    */
    if (haveLODRange)
    {
#ifdef NOTYET /* trying to set max LOD produces "OpenGL error 0x500 - invalid enumerant" */
	texture->setLODRange(min_lod,
			     1000.);
#else /* for now */
	glTexParameterf(GL_TEXTURE_2D,
		    GL_TEXTURE_MIN_LOD_SGIS,
		    min_lod);
if (_PFFAKECLIP_TRY_SETTING_MAX_LOD)
{
/* XXX all this does is produce "invalid enumerant" */
	glTexParameterf(GL_TEXTURE_2D,
		    GL_TEXTURE_MAX_LOD_SGIS,
		    min_lod);
}
#endif /* for now */
    }

    return PFTRAV_CONT;
}
static int
levelPostDrawFunc(pfTraverser *, void *)
{
    return PFTRAV_CONT;
}

pfTexture *textureLevels[32]; /* HACK-- shouldn't be global */

/*
 * Choose pyramid level positions that are aesthetically pleasing.
 * Return the appropriate coords of the lower-left corner of the level.
 * The main geometry goes from 0,0 .. 1,1;
 * put the largest pyramid level next to it on the right,
 * put the next one below it, etc.
 */
#define SEPARATION .2 /* separation between two objects, w.r.t. the
			  smaller object's diameter */
static pfVec3
calcOffsetPyramidLevel(int level, int nclippedlevels) /* BACKWARDS from the way I usually do it */
{
    float maxradius = .5 / (1<<nclippedlevels);

    float separation = SEPARATION;	/* gets smaller as we count */
    float radius = .5;			/* gets smaller as we count */

    int i;
    pfVec3 center(1 + SEPARATION, 1, 0);
    center += pfVec3(PF_MIN2(radius,maxradius), -PF_MIN2(radius,maxradius), 0);	/* down right */

    if (_PFFAKECLIP_TRY_IDENTMAT)
	center.set(.5,.5,0);

    FOR (i, level)
    {
	center[1] -= PF_MIN2(radius,maxradius);

	radius *= .5;
	separation = 2*radius * SEPARATION;

	center[1] -= PF_MIN2(radius,maxradius)+separation;
    }
    center -= pfVec3(PF_MIN2(radius,maxradius), PF_MIN2(radius,maxradius), 0.);


#ifdef DEFUNCT
    FOR (i, level)
    {
	center[1] -= .5 / (1<<i);  /* radius of this (bigger) object */
	center[1] -= (.5+SEPARATION) / (1<<(i+1)); /* radius of next (smaller) object */
    }
    center -= pfVec3(.5/(1<<level), .5/(1<<level), 0.);
#endif
    FOR (i, 3)
	PFASSERTALWAYS(INRANGE(-1e6f <, center[i], < 1e6f));

    return center;
}


static void
SetFakeClipPyramidParams(pfNode *node,
		      float centerx, float centery,
		      int clipSize,
		      float invalid_border,
		      float min_lod,
		      float visualize_wrap,
		      float visualize_roam,
		      float visualize_inlevel)
{
    pfGroup *group = (pfGroup *)node;
    struct FakeClipParams *params =
	(struct FakeClipParams *) group->getUserData();

    /* min_lod requires no further work */
    params->min_lod = min_lod;

    if (params->center[0] != centerx
     || params->center[1] != centery
     || params->clipSize != clipSize
     || params->invalid_border != invalid_border
     || params->visualize_wrap != visualize_wrap
     || params->visualize_roam != visualize_roam
     || params->visualize_inlevel != visualize_inlevel)
    {
	params->center[0] = centerx;
	params->center[1] = centery;
        params->clipSize = clipSize;
        params->invalid_border = invalid_border;
        params->visualize_wrap = visualize_wrap;
        params->visualize_roam = visualize_roam;
        params->visualize_inlevel = visualize_inlevel;

	int i, nlevels = group->getNumChildren();
	FOR (i, nlevels)
	{
	    pfGroup *intermediate = (pfGroup *)group->getChild(i);
	    PFASSERTALWAYS(intermediate->isOfType(pfGroup::getClassType()));
		pfGeode *geode = (pfGeode *)intermediate->getChild(0);
		PFASSERTALWAYS(geode->isOfType(pfGeode::getClassType()));
		pfDCS *textdcs = (pfDCS *)intermediate->getChild(1);
		PFASSERTALWAYS(textdcs->isOfType(pfDCS::getClassType()));
		    pfText *text = (pfText  *)textdcs->getChild(0);
		    PFASSERTALWAYS(text->isOfType(pfText::getClassType()));

	    pfGeoSet *valid_geoset = geode->getGSet(0);
	    pfGeoSet *invalid_geoset = geode->getGSet(1);
	    pfGeoSet *outline_geoset = geode->getGSet(2);

	    pfVec3 (*valid_vcoords)[4];		/* quads */
	    pfVec2 (*valid_tcoords)[4];		/* quads */
	    pfVec3 (*invalid_vcoords)[4];	/* quads */
	    ushort *dummy;

	    valid_geoset->getAttrLists(PFGS_COORD3, (void **)&valid_vcoords, &dummy);
	    valid_geoset->getAttrLists(PFGS_TEXCOORD2, (void **)&valid_tcoords, &dummy);
	    invalid_geoset->getAttrLists(PFGS_COORD3, (void **)&invalid_vcoords, &dummy);
	    PFASSERTALWAYS(valid_vcoords != NULL && valid_tcoords != NULL);
	    PFASSERTALWAYS(invalid_vcoords != NULL);

	    int n_valid_quads, n_invalid_quads;
	    pfVec3 levelOffset = calcOffsetPyramidLevel(nlevels-1-i,
					    params->visualize_inlevel ? 0 : 
							(int)round(log2((1<<(nlevels-1))/params->clipSize)));

	    /*
	     * Make the height of the labels 1/20 of the virtual size
	     * for all levels down to the level
	     * whose size is 1/16 the virtual size,
	     * then shrink proportionately for the smaller levels.
	     */
	    float textscale = 1/20. * PF_MIN2(1., (1<<i) / ((1<<(nlevels-1))/16.) );
	    textdcs->setScale(textscale,textscale,textscale);
	    textdcs->setRot(0,-90,0);
	    textdcs->setTrans(levelOffset[0], levelOffset[1], levelOffset[2]);

	    GetClippedLevelCoords(pfVec2(centerx, centery),
					 1<<(nlevels-1), /* virtual size */
					 1<<i, /* level_size */
					 clipSize, invalid_border,
					 FakeClipGuiState->visualize_roam,
					 FakeClipGuiState->visualize_wrap,
					 levelOffset,
					 valid_vcoords,
					 valid_tcoords,
					 invalid_vcoords,
					 &n_valid_quads,
					 &n_invalid_quads);

	    valid_geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, valid_vcoords, NULL);
	    valid_geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, valid_tcoords, NULL);
	    invalid_geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, invalid_vcoords, NULL);
	    valid_geoset->setNumPrims(n_valid_quads);
	    invalid_geoset->setNumPrims(n_invalid_quads);

	    /*
	     * Draw outline only if it's a clip level
	     */
	    if ((1<<i) > clipSize && params->visualize_inlevel)
	    {
		pfVec3 (*outline_vcoords)[2];
		outline_geoset->getAttrLists(PFGS_COORD3, (void **)&outline_vcoords, &dummy);
		PFASSERTALWAYS(outline_vcoords != NULL);
		static float unitsquare[5][2]
			= {{0,0},{1,0},{1,1},{0,1},{0,0}};
		int j;
		FOR (j, 4)
		{
		    outline_vcoords[j][0] = levelOffset +
			(1<<i)/(float)(1<<(nlevels-1))*pfVec3(unitsquare[j][0], unitsquare[j][1], 0.);
		    outline_vcoords[j][1] = levelOffset +
			(1<<i)/(float)(1<<(nlevels-1))*pfVec3(unitsquare[(j+1)%4][0], unitsquare[(j+1)%4][1], 0.);
		}
		
		outline_geoset->setNumPrims(4);
	    }
	    else
		outline_geoset->setNumPrims(0);
	    /*
	     * Annotate if it's a clip level or the highest pyramid level
	     */
	    if ((1<<i) >= clipSize)
	    {
	    }
	}
    }
}


static int
pyramidLevelPreDrawFunc(pfTraverser *, void *arg)
{
    pfMatrix mat;
#ifdef DEFUNCT /* can't do this here, since the node (even the traverser?) is bogus in draw callbacks, but might put this back if moved to the cull or app */
    pfNode *node = trav->getNode();
    pfGroup *parent = node->getParent(0);
    PFASSERTALWAYS(parent->isOfType(pfGroup::getClassType()));
    int myindex = parent->searchChild(node);
    PFASSERTALWAYS(myindex >= 0);
    struct FakeClipParams *params = (struct FakeClipParams *)arg;
    pfTexture *texture = (pfTexture *)((pfGeode *)trav->getNode())->getGSet(0)->getGState()->getAttr(PFSTATE_TEXTURE);
#endif
    struct FakeClipLevelParams *levelParams = (struct FakeClipLevelParams *)arg;
    int myindex = levelParams->myindex;
    struct FakeClipParams *params = levelParams->params;
    pfTexture *texture = params->texture;
    float min_lod = params->nlevels-1-myindex;

    /*
     * Really want to see the texels...
     */
    if (_PFFAKECLIP_TRY_MESSING_WITH_FILTER_IN_DRAW)
    {
	texture->setFilter(PFTEX_MAGFILTER, PFTEX_POINT);
	texture->setFilter(PFTEX_MINFILTER, PFTEX_MIPMAP_POINT);
    }
    else
    {
	/* XXX work around bug (?) where changing the filters
	       causes a reformat (including mipmap generation,
	       if using it!!!) */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    /*
     * Set min lod to only draw with the exact level we're at
     */
    /*
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "%dx%d pyramid level: Setting minlod to %g",
	     (1<<myindex), (1<<myindex),
	     min_lod);
    */
    if (haveLODRange)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD_SGIS, min_lod);

if (_PFFAKECLIP_TRY_SETTING_MAX_LOD)
{
/* XXX all this does is produce "invalid enumerant" */
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD_SGIS, min_lod);
}

    if (_PFFAKECLIP_TRY_IDENTMAT)
    {
	mat.makeTrans(0.,0.,-1.);
	mat.preScale(.5,.5,.5,mat);
	pfPushMatrix();
	pfLoadMatrix(mat);
    }

    return PFTRAV_CONT;
}

static int
pyramidLevelPostDrawFunc(pfTraverser *, void *arg)
{
    struct FakeClipLevelParams *levelParams = (struct FakeClipLevelParams *)arg;
    struct FakeClipParams *params = levelParams->params;
    pfTexture *texture = params->texture;

    if (_PFFAKECLIP_TRY_MESSING_WITH_FILTER_IN_DRAW)
    {
	if (!getenv("_PFFAKECLIP_EVERYONE_SETS_THEIR_OWN_FILTER"))
	{
	    /* XXX the following shouldn't be necessary, since levelPreDrawFunc does it, but it seems to be necessary!??? */
	    texture->setFilter(PFTEX_MINFILTER, FakeClipGuiState->minfilter);
	    texture->setFilter(PFTEX_MAGFILTER, FakeClipGuiState->magfilter);
	}
    }
    else
    {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
	    FakeClipGuiState->minfilter == PFTEX_POINT ? GL_NEAREST :
	    FakeClipGuiState->minfilter == PFTEX_BILINEAR ? GL_LINEAR :
	    FakeClipGuiState->minfilter == PFTEX_MIPMAP_POINT ? GL_NEAREST_MIPMAP_NEAREST :
	    FakeClipGuiState->minfilter == PFTEX_MIPMAP_LINEAR ? GL_NEAREST_MIPMAP_LINEAR :
	    FakeClipGuiState->minfilter == PFTEX_MIPMAP_BILINEAR ? GL_LINEAR_MIPMAP_NEAREST :
	    FakeClipGuiState->minfilter == PFTEX_MIPMAP_TRILINEAR ? GL_LINEAR_MIPMAP_LINEAR :
	    GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
	    FakeClipGuiState->magfilter == PFTEX_POINT ? GL_NEAREST :
	    FakeClipGuiState->magfilter == PFTEX_BILINEAR ? GL_LINEAR :
	    GL_NEAREST);
    }

    if (_PFFAKECLIP_TRY_IDENTMAT)
	pfPopMatrix();

    return PFTRAV_CONT;
}

static pfNode *
NewFakeClipPyramid(int nlevels, pfTexture *tex)
{
    pfNotify(PFNFY_INFO, PFNFY_PRINT,
	     "Creating a pyramid with %d levels", nlevels);
    /*
     * Create a group with nlevels children.
     *     Child 0 (the 1x1 level) is a pfGeode with two geosets
     *	       geoset 1 is the textured valid window
     *	       geoset 2 is the non-textured invalid border.
     *	       geoset 3 is a wireframe around the level size
     *			(if it's a clip level).
     *		
     *     Child 1 (the 2x2 level) is a pfGeode with two geosets
     *	       geoset 1 is the textured valid window
     *	       geoset 2 is the non-textured invalid border.
     *	       geoset 3 is a wireframe around the level size
     *			(if it's a clip level).
     *	   etc.
     */

    if (haveLODRange < 0)
	pfQueryFeature(PFQFTR_TEXTURE_LOD_RANGE, &haveLODRange);

    /* XXX duplicated in other function */
    pfGroup *group = new pfGroup();
    PFASSERTALWAYS(group != NULL);
    struct FakeClipParams *params = (struct FakeClipParams *)
	pfMalloc(sizeof(*params), pfGetSharedArena());
    PFASSERTALWAYS(params != NULL);
    params->texture = tex;
    params->nlevels = nlevels;
    group->setUserData(params);

    pfGeoState *textured_geostate = new pfGeoState();
    PFASSERTALWAYS(textured_geostate != NULL);
    textured_geostate->setAttr(PFSTATE_TEXTURE, tex);
    textured_geostate->setMode(PFSTATE_ENTEXTURE, 1);

    pfGeoState *untextured_geostate = new pfGeoState();
    PFASSERTALWAYS(untextured_geostate != NULL);
    textured_geostate->setAttr(PFSTATE_TEXTURE, tex);
	/* XXX sort of groping around in the dark, but I
	   assume that will maximize the chance of getting
	   the glTexParamer calls to correspond to the right texture */
    untextured_geostate->setMode(PFSTATE_ENTEXTURE, 0);

    int i;
    FOR (i, nlevels)
    {
	/*
	 * Create the geode
	 */
	pfVec3 *valid_vcoords = (pfVec3 *)pfMalloc(4*4*sizeof(*valid_vcoords), NULL);
	pfVec2 *valid_tcoords = (pfVec2 *)pfMalloc(4*4*sizeof(*valid_tcoords), NULL);
	pfVec3 *invalid_vcoords = (pfVec3 *)pfMalloc(8*4*sizeof(*invalid_vcoords), NULL);
	pfVec3 *outline_vcoords = (pfVec3 *)pfMalloc(4*2*sizeof(*outline_vcoords), NULL);
	pfVec4 *white = (pfVec4 *)pfMalloc(1*sizeof(*white), NULL);
	pfVec4 *grey = (pfVec4 *)pfMalloc(1*sizeof(*grey), NULL);
	PFASSERTALWAYS(valid_vcoords != NULL && valid_tcoords != NULL); /* init later */
	PFASSERTALWAYS(valid_vcoords != NULL
	    && valid_tcoords != NULL
	    && invalid_vcoords != NULL);
	white->set(1,1,1,1);
	grey->set(.5,.5,.5,1);

	pfGeoSet *valid_geoset = new pfGeoSet();
	PFASSERTALWAYS(valid_geoset != NULL);
	valid_geoset->setGState(textured_geostate);
	valid_geoset->setPrimType(PFGS_QUADS);
	valid_geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, valid_vcoords, NULL);
	valid_geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, valid_tcoords, NULL);
	valid_geoset->setAttr(PFGS_COLOR4, PFGS_OVERALL, white, NULL);

	pfGeoSet *invalid_geoset = new pfGeoSet();
	PFASSERTALWAYS(invalid_geoset != NULL);
	invalid_geoset->setGState(untextured_geostate);
/* XXX this doubles the frame rate-- make it that fast anyway!! */
if (getenv("_PFFAKECLIP_NO_UNTEXTURED_GEOSTATE")) invalid_geoset->setGState(textured_geostate);
	invalid_geoset->setPrimType(PFGS_QUADS);
	invalid_geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, invalid_vcoords, NULL);
	invalid_geoset->setAttr(PFGS_COLOR4, PFGS_OVERALL, grey, NULL);

	pfGeoSet *outline_geoset = new pfGeoSet();
	PFASSERTALWAYS(outline_geoset != NULL);
	outline_geoset->setGState(untextured_geostate);
/* XXX this doubles the frame rate-- make it that fast anyway!! */
if (getenv("_PFFAKECLIP_NO_UNTEXTURED_GEOSTATE"))
outline_geoset->setGState(textured_geostate);
	outline_geoset->setPrimType(PFGS_LINES);
	outline_geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, outline_vcoords, NULL);
	outline_geoset->setAttr(PFGS_COLOR4, PFGS_OVERALL, white, NULL);

	pfGeode *geode = new pfGeode();
	PFASSERTALWAYS(geode != NULL);
	geode->addGSet(valid_geoset);
	geode->addGSet(invalid_geoset);
	geode->addGSet(outline_geoset);
	geode->setTravFuncs(PFTRAV_DRAW, pyramidLevelPreDrawFunc, pyramidLevelPostDrawFunc);
	struct FakeClipLevelParams *levelParams =
		(struct FakeClipLevelParams *)pfMalloc(sizeof(*levelParams));
	PFASSERTALWAYS(levelParams != NULL);
	levelParams->myindex = i;
	levelParams->params = params;
	geode->setTravData(PFTRAV_DRAW, levelParams);

	/*
	 * Create the text
	 */
	pfDCS *textdcs = new pfDCS();
	PFASSERTALWAYS(textdcs != NULL);

	    pfText *text = new pfText();
	    PFASSERTALWAYS(text != NULL);
		pfString *string = new pfString();
		PFASSERTALWAYS(string != NULL);
		    char buf[40];
		    sprintf(buf, "%dx%d", 1<<i, 1<<i);
		    if (theFont == NULL)
		    {
			theFont = pfdLoadFont_type1("Times-Elfin", PFDFONT_FILLED);
			PFASSERTALWAYS(theFont != NULL);
		    }
		    string->setFont(theFont);
		    string->setColor(1.0f, 1.0f, 0.0f, 1.0f); /* yellow */
		    string->setColor(1.0f, 0.7f, 0.7f, 1.0f); /* pink */
		    /* XXX non-fully-saturated colors come out white??? Well
			   white is okay anyway */
		    string->setMode(PFSTR_JUSTIFY, PFSTR_RIGHT);
		    string->setString(buf);
		    string->flatten();
		    string->setGState(untextured_geostate);

	    text->addString(string);
	    textdcs->addChild(text);
	    

	pfGroup *intermediate = new pfGroup();
	PFASSERTALWAYS(intermediate != NULL);
	intermediate->addChild(geode);
	intermediate->addChild(textdcs);
	group->addChild(intermediate);

#ifdef DEFUNCT /* XXX */
	pfVec3 offset = calcOffsetPyramidLevel(nlevels-1-i);
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		 "Offset of pyramid level %dx%d is %g,%g,%g",
		 i,i, offset[0], offset[1], offset[2]);
#endif
    }

    /* XXX duplicated below */
    int clipSize = 1<<(nlevels-1);
    float invalid_border = 0;
    float minlod = 0;
    SetFakeClipPyramidParams(group, .5, .5, clipSize, invalid_border,
			     nlevels-1 - minlod,
			     FakeClipGuiState->visualize_wrap,
			     FakeClipGuiState->visualize_roam,
			     FakeClipGuiState->visualize_inlevel);
    return group;
}

/*
 * Clip texture size is pow(2, nlevels-1).
 * Or to put it another way, nlevels is 1 + log2(clip texture size).
 */
static pfNode *
NewFakeClipNode(int nlevels, pfTexture *tex)
{
    pfNotify(PFNFY_INFO, PFNFY_PRINT,
	     "Creating a fake clip texture with %d levels", nlevels);

    /*
     * Create a group with nlevels children.
     * Child i is a pfGeode
     *     with a single geoset
     *         with an 8-triangle strip in the shape of a donut
     *	       textured by the mipmap
     *	   with a pre-draw callback
     *         that sets min_lod to be max(globalmin, nlevels-1-i).
     *
     * XXX if changing this mid-frame doesn't work,
     * we may need to have separate mipmap texture for each level!
     */
    int i;

    pfGeoState *geostate = new pfGeoState();
    PFASSERTALWAYS(geostate != NULL);
    geostate->setMode(PFSTATE_ENTEXTURE, 1);
    geostate->setAttr(PFSTATE_TEXTURE, tex);

    pfGroup *group = new pfGroup();
    PFASSERTALWAYS(group != NULL);
    struct FakeClipParams *params = (struct FakeClipParams *)
	pfMalloc(sizeof(*params), pfGetSharedArena());
    PFASSERTALWAYS(params != NULL);
    params->texture = tex;
    params->nlevels = nlevels;
    group->setUserData(params);

    FOR (i, nlevels)
    {
	pfVec3 *vcoords = (pfVec3 *)pfMalloc(10*sizeof(*vcoords), NULL);
	pfVec2 *tcoords = (pfVec2 *)pfMalloc(10*sizeof(*tcoords), NULL);
	int *lengths = (int *)pfMalloc(1*sizeof(*lengths), NULL);
	PFASSERTALWAYS(vcoords != NULL && tcoords != NULL && lengths != NULL);
	lengths[0] = 10;
	pfVec4 *white = (pfVec4 *)pfMalloc(1*sizeof(*white), NULL);
	PFASSERTALWAYS(white != NULL);
	white->set(1,1,1,1);

	/* no need to initialize vcoords&tcoords, that will be done later */

	pfGeoSet *geoset = new pfGeoSet();
	PFASSERTALWAYS(geoset != NULL);
	geoset->setGState(geostate);
	geoset->setNumPrims(1);
	geoset->setPrimLengths(lengths);
	geoset->setPrimType(PFGS_TRISTRIPS);
	geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, vcoords, NULL);
	geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
	geoset->setAttr(PFGS_COLOR4, PFGS_OVERALL, white, NULL);

	pfGeode *geode = new pfGeode();
	PFASSERTALWAYS(geode != NULL);
	geode->addGSet(geoset);
	geode->setTravFuncs(PFTRAV_DRAW, levelPreDrawFunc, levelPostDrawFunc);
	struct FakeClipLevelParams *levelParams =
		(struct FakeClipLevelParams *)pfMalloc(sizeof(*levelParams));
	PFASSERTALWAYS(levelParams != NULL);
	levelParams->myindex = i;
	levelParams->params = params;
	geode->setTravData(PFTRAV_DRAW, levelParams);

	group->addChild(geode);
    }

    /* XXX duplicated above */
    int clipSize = 1<<(nlevels-1);
    float invalid_border = 0;
    float minlod = 0;
    SetFakeClipNodeParams(group, .5, .5, clipSize, invalid_border,
			  nlevels-1 - minlod);

    return group;
}

/*
 * Get the eye position in the local coordinate space
 * at this point in the traversal.
 */
static void GetEye(pfTraverser *trav, pfVec3 &eye)
{
    pfMatrix viewmat, travmat, invtravmat, mat;
    eye[0] = eye[1] = eye[2] = 0;
    trav->getChan()->getViewMat(viewmat);
    trav->getMat(travmat);
    invtravmat.invertAff(travmat);
    PFCOPY_MAT(mat, viewmat);
    mat.postMult(invtravmat);
    eye.xformPt(eye, mat);
    PFASSERTALWAYS(eye == eye);	/* catch NaNs */
}

static int
postAppFunc(pfTraverser *trav, void *)
{
    pfNode *node = trav->getNode();
    PFASSERTALWAYS(node->isOfType(pfGroup::getClassType()));
    pfGroup *group = (pfGroup *)node;

    pfGroup *child0 = (pfGroup *)group->getChild(0);
    PFASSERTALWAYS(child0 != NULL);
    PFASSERTALWAYS(child0->isOfType(pfGroup::getClassType()));
    /* child1 may be NULL; it means don't do the pyramid */

    switch (FakeClipGuiState->clip_centering)
    {
	/* XXX hard-coded values must match gui! */
	case 0: /* fixed */
	    /* do nothing */
	    break;
	case 1: /* auto */
	    {
	    pfVec3 eye;
	    GetEye(trav, eye);
	    FakeClipGuiState->clip_center[0] = eye[0];
	    FakeClipGuiState->clip_center[1] = eye[1];
	    }
	    break;
	case 2: /* spiral */
	    /* spiral animation */
#define SPIRALSPEED 4.
	    {
	    float n = pfGetFrameCount() * SPIRALSPEED;
	    float s, c, r;
	    pfSinCos(n, &s, &c);

	    r = fabs(fmod(n/360./4.+1, 2.)-1.);

	    FakeClipGuiState->clip_center[0] = r*c + .5f;
	    FakeClipGuiState->clip_center[1] = r*s + .5f;
	    }
	    break;
	case 3: /* mouse drag */
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		"Mouse drag centering not implemented");
	    break;
    }

    SetFakeClipNodeParams(child0,
			  FakeClipGuiState->clip_center[0],
			  FakeClipGuiState->clip_center[1],
			  FakeClipGuiState->clip_size,
			  FakeClipGuiState->invalid_border,
			  FakeClipGuiState->min_lod);

    if (group->getNumChildren() > 1)
    {
	pfGroup *child1 = (pfGroup *)group->getChild(1);
	PFASSERTALWAYS(child1 != NULL);
	PFASSERTALWAYS(child1->isOfType(pfGroup::getClassType()));
	SetFakeClipPyramidParams(child1,
			  FakeClipGuiState->clip_center[0],
			  FakeClipGuiState->clip_center[1],
			  FakeClipGuiState->clip_size,
			  FakeClipGuiState->invalid_border,
			  FakeClipGuiState->min_lod,
			  FakeClipGuiState->visualize_wrap,
			  FakeClipGuiState->visualize_roam,
			  FakeClipGuiState->visualize_inlevel);
    }
    else
    {
	/* XXX MAJOR HACK-- it crashes when I try it at the start,
	   so add it after the fact */
	int nlevels = child0->getNumChildren();
	pfTexture *texture = (pfTexture *)((pfGeode *)child0->getChild(0))->getGSet(0)->getGState()->getAttr(PFSTATE_TEXTURE);
/* XXX this makes it go at full-speed-- figure out what is wrong with the pyramid! */
if (!getenv("_PFFAKECLIP_NO_PYRAMID"))
{
	pfNode *pyramidnode = NewFakeClipPyramid(nlevels, texture);
	group->addChild(pyramidnode);
}

#ifdef NOTYET
	/* XXX make sure this never gets culled */
	pfSphere boundingsphere;
	boundingsphere.center.set(.0,.0,0.);
	boundingsphere.radius = 10000.;
	pyramidnode->setBound(&boundingsphere, PFBOUND_STATIC);
#endif
    }
/*
if (pfGetNotifyLevel() >= PFNFY_DEBUG)
{
printf("=============================== FRAME %d ========================\n",
	pfGetFrameCount());
pfPrint(group, PFTRAV_SELF | PFTRAV_DESCEND, PFPRINT_VB_DEBUG, stdout);
}
*/

    return PFTRAV_CONT;
}

/* nlevels==0 means generate mipmap levels from single top-level file name */
extern "C" pfTexture *makeTheMipMap(int *nlevels, char *texFileNameFormat)
{
    _PFFAKECLIP_TRY_IDENTMAT = (getenv("_PFFAKECLIP_TRY_IDENTMAT") != NULL);
    _PFFAKECLIP_TRY_SETTING_MAX_LOD = (getenv("_PFFAKECLIP_TRY_SETTING_MAX_LOD") != NULL);
    _PFFAKECLIP_TRY_MESSING_WITH_FILTER_IN_DRAW = (getenv("_PFFAKECLIP_TRY_MESSING_WITH_FILTER_IN_DRAW") != NULL);
    pfTexture *base;
    if (texFileNameFormat[0] != '\0')
    {
	char texFileName[1024];
	int TexLoadMode = PFTEX_BASE_APPLY; /* mipmap demo has -r option to change it to PFTEX_BASE_AUTO_SUBLOAD */
	TexLoadMode = PFTEX_BASE_AUTO_SUBLOAD;

	base = new pfTexture();
	base->setName(texFileNameFormat);
	sprintf(texFileName, texFileNameFormat, 1<<(*nlevels-1), 1<<(*nlevels-1));

	base->setLoadMode(PFTEX_LOAD_BASE, TexLoadMode);
	if (TexLoadMode == PFTEX_BASE_AUTO_SUBLOAD)
	    base->setFormat(PFTEX_SUBLOAD_FORMAT, 1);
	if (getenv("_FAKECLIP_4"))
	    base->setFormat(PFTEX_INTERNAL_FORMAT, PFTEX_RGBA_4);
	else
	    base->setFormat(PFTEX_INTERNAL_FORMAT, PFTEX_RGBA_8);

	if (*nlevels == 0)
	{
	    /*
	     * Calculate number of levels from the size of the given image,
	     * and generate mipmap levels (don't load them from files).
	     */
	    int sizeS, sizeT, maxsize;
	    PFASSERTALWAYS(!strchr(texFileNameFormat, '%'));
	    pfNotify(PFNFY_INFO, PFNFY_PRINT,
		     "Loading image file \"%s\" into base mipmap level\n",
		     texFileNameFormat);
	    PFASSERTALWAYS(base->loadFile(texFileNameFormat));
	    /*
	     * Calculate the actual number of levels...
	     */
	    base->getImage(NULL, NULL, &sizeS, &sizeT, NULL);
	    maxsize = PF_MAX2(sizeS, sizeT);
	    *nlevels = (int)round(log2(maxsize))+1;
	}
	else
	{
	    /*
	     * Turn off automatic mipmap level generation,
	     * and load the mipmap levels from files,
	     */
	    base->setFormat(PFTEX_GEN_MIPMAP_FORMAT, 0);
	    for (int i = *nlevels-1; i >= 0; --i)
	    {
		sprintf(texFileName, texFileNameFormat, 1<<i, 1<<i);
		pfNotify(PFNFY_INFO, PFNFY_PRINT,
			 "Loading image file \"%s\" into mipmap level %d\n",
			 texFileName, i);

		if (i < *nlevels-1)
		{
		    pfTexture *tex = new pfTexture();
		    int sts = tex->loadFile(texFileName);
		    PFASSERTALWAYS(sts);
		    textureLevels[i] = tex;
		    base->setLevel(*nlevels-1-i, tex);
		}
		else /* i == *nlevels-1 */
		{
		    /* purposely don't set textureLevels[0] to base;
		       I don't think we need it */
		    int sts = base->loadFile(texFileName);
		    PFASSERTALWAYS(sts);
		}
	    }
	}
    }
    return base;
}

extern "C" pfNode *
pfdLoadFile_fakeclip(char *fileName)
{
    /*
     * Get params from the file name without the ".fakeclip"
     */
    char *lastdot = strrchr(fileName, '.');
    if (lastdot == NULL || !streq(lastdot, ".fakeclip"))
	lastdot = fileName + strlen(fileName);

    char params[PATH_MAX];
    strncpy(params, fileName, lastdot-fileName);
    params[lastdot-fileName] = '\0';

    int nlevels = 11;	/* 1024x1024 texture by default */
    char texFileNameFormat[1024];
    texFileNameFormat[0] = '\0';
    (void)sscanf(params, "%d,%s", &nlevels, texFileNameFormat);

    pfTexture *texture = makeTheMipMap(&nlevels, texFileNameFormat);

    pfGroup *group = new pfGroup();
    PFASSERTALWAYS(group != NULL);

    pfNode *fakeclipnode = NewFakeClipNode(nlevels, texture);
    group->addChild(fakeclipnode);

if (getenv("_PFFAKECLIP_NO_HACK")) /* XXX disgusting-- it crashed if I do it here (don't know why) so do it later */
if (!getenv("_PFFAKECLIP_NO_PYRAMID"))
{
	pfNode *pyramidnode = NewFakeClipPyramid(nlevels, texture);
	group->addChild(pyramidnode);
}

    group->setTravFuncs(PFTRAV_APP, NULL, postAppFunc);
    group->setTravData(PFTRAV_APP, NULL);

    pfSphere boundingsphere;
    boundingsphere.center.set(.5,.5,0);
    boundingsphere.radius = 1.5 * (1+SEPARATION) * M_SQRT2;
    boundingsphere.radius *= .5; /* XXX so it will fill up the screen on start */
    group->setBound(&boundingsphere, PFBOUND_STATIC);

    /* XXX .closest loader starts by making the ball too big and putting
       it at 0,0,0, which messes up the center of rotation.
       Fudge by centering us at 0,0,0 too.
       Also rotate so we are looking from the +z axis. */
    pfMatrix mat;
    mat.makeTrans(-.5f, -.5f, .0f);
    mat.postRot(mat, 90.0f, 1.0f, .0f, .0f);
    pfSCS *scs = new pfSCS(mat);
    scs->addChild(group);

    /*
     * Initialize FakeClipGuiState to something usable
     */
    PFASSERTALWAYS(FakeClipGuiState != NULL);
    strcpy(FakeClipGuiState->imageFileName, fileName);
    FakeClipGuiState->clip_center.set(.5, .5);
    FakeClipGuiState->clip_centering = 1; /* XXX auto */
    FakeClipGuiState->image_size =   (1<<(nlevels-1));
    FakeClipGuiState->virtual_size = (1<<(nlevels-1));
    FakeClipGuiState->clip_size =  64; /* XXX reasonable? doesn't matter, it gets clobbered right away */
    FakeClipGuiState->visualize_roam = 0;
    FakeClipGuiState->visualize_wrap = 0;
    FakeClipGuiState->visualize_inlevel = 0;
    /*
    FakeClipGuiState->minfilter = PFTEX_MIPMAP_LINEAR;
    FakeClipGuiState->magfilter = PFTEX_POINT;
    */
    FakeClipGuiState->minfilter = PFTEX_MIPMAP_TRILINEAR;
    FakeClipGuiState->magfilter = PFTEX_BILINEAR;
    FakeClipGuiState->min_lod = 0.;
    FakeClipGuiState->invalid_border = 8.;
    oldFakeClipGuiState = *FakeClipGuiState; /* bytewise copy */

if (pfGetNotifyLevel() >= PFNFY_DEBUG)
pfPrint(scs, PFTRAV_SELF | PFTRAV_DESCEND, PFPRINT_VB_DEBUG, stdout);

    return scs;
}
