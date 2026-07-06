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
 * pfsubstclip.c
 *
 * Pseudo-loader that loads a scene graph from
 * from a file of any format and substitutes a given clip texture
 * for the i'th texture encountered in the scene.
 *
 * Pays attention to the following env variables:
 *	PFSUBSTCLIP_TEXTURE -- name of clip texture configuration file
 *	PFSUBSTCLIP_TEXTURE_I -- index of the texture to replace in the scene
 *			(to get a list of the valid indices with corresponding
 *		         texture names, setenv PFSUBSTCLIP_TEXTURE_I -1
 *			 and setenv PFNFYLEVEL 5)
 *	PFSUBSTCLIP_NORMAL_AXIS -- if set to 0, 1, or 2 (representing the
 *			    X, Y, or Z axis respectively),
 *			    a bounding rectangle perpendicular to the given
 *			    axis is calculated and used for clipmap centering
 *			    rather than traversing the entire geometry
 *			    on each frame.
 *			    If set to the empty string,
 *			    the axis used is the one in which the
 *			    bounding box is thinnest.
 *
 * For example, use the following commands to replace the hubcaps on esprit.flt
 * with the clip texture described by mycliptexture.ct:
 *
 	setenv PFSUBSTCLIP_TEXTURE   mycliptexture.ct
 	setenv PFSUBSTCLIP_TEXTURE_I 0
 	perfly_ogl esprit.flt.substclip
 *
 * To replace the license plate instead of the hubcaps, use 1 instead of 0
 * for PFSUBSTCLIP_TEXTURE_I.
 *
 * To also show a little red ball at the clip texture center,
 * use this in conjuction with the .closest pseudo-loader:
 	setenv PFSUBSTCLIP_TEXTURE   mycliptexture.ct
 	setenv PFSUBSTCLIP_TEXTURE_I 0
	setenv PFCLOSEST_TEXTURE_I   0
 	perfly_ogl esprit.flt.substclip.closest
 *
 * You can also specify more than one texture by giving comma-separated
 * lists for the environment variables:
  	setenv PFSUBSTCLIP_TEXTURE cliptexture0.ct,cliptexture1.ct
  	setenv PFSUBSTCLIP_TEXTURE_I 0,1	# this is the default
  	perfly_ogl esprit.flt.substclip
 */

#ifdef WIN32
#include <windows.h>
#include <limits.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pr/pfClipTexture.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pfutil/pfuClipCenterNode.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <stdlib.h>
#include <assert.h>

#define FOR(i,n) for ((i) = 0; (i) < (n); ++(i))
#define LERP(a,b,t) ((a)*(1-(t)) + (b)*(t))
/* find the index with minimum value */
#define MINI3(v) \
	((v)[0] < (v)[1] ? (v)[0] < (v)[2] ? 0 : 2 : (v)[1] < (v)[2] ? 1 : 2)


/*
 * Trick the pfuLowestCommonAncestorOfGeoSets() function into
 * simply traversing every geoset in the entire scene,
 * by providing a callback function that always returns FALSE.
 * This function replaces old texture with new in the given geoset.
 */
static int
_replaceTexture(pfGeoSet *gset, void *arg)
{
    pfTexture **old_and_new = (pfTexture **)arg;
    pfGeoState *gstate = gset->getGState();
    if (gstate != NULL
     && gstate->getMode(PFSTATE_ENTEXTURE))
    {
	pfTexture *tex = (pfTexture *)gstate->getAttr(PFSTATE_TEXTURE);
	if (tex == old_and_new[0])	/* change old to new */
	    gstate->setAttr(PFSTATE_TEXTURE, tex = old_and_new[1]);
	return tex == old_and_new[1];
    }
    else
	return FALSE;
}

/*
 * Traverse a scene and replace all references to oldtex by newtex.
 * The mode argument is the traversal mode to be passed to pfuTraverse().
 */
extern void
pfuReplaceTexture(pfNode *scene, pfTexture *oldtex, pfTexture *newtex, int mode)
{
    pfTexture *old_and_new[2];
    old_and_new[0] = oldtex;
    old_and_new[1] = newtex;
    (void)pfuLowestCommonAncestorOfGeoSets(scene,
					   _replaceTexture, (void*)old_and_new,
					   mode);
}

static int _tex_is_in_array(pfTexture *tex, pfTexture **texs, int n)
{
    int i;
    FOR (i, n)
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
	    int i;
	    FOR (i, nGSets)
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
 * Function to be passed to pfuLowestCommonAncestorOfGeoSets().
 * Returns TRUE if the given geoset is textured by the given texture.
 */
static int
_geosetContainsTexture(pfGeoSet *gset, void *arg)
{
    pfGeoState *gstate = gset->getGState();
#define DEBUG
#ifdef DEBUG
pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
"GEOSET 0x%p IS%s TEXTURED BY 0x%p", 
    gset,
    gstate != NULL
        && gstate->getMode(PFSTATE_ENTEXTURE)
        && gstate->getAttr(PFSTATE_TEXTURE) == (pfTexture *)arg ? "" : " NOT",
    arg);
#endif
    return gstate != NULL
        && gstate->getMode(PFSTATE_ENTEXTURE)
        && gstate->getAttr(PFSTATE_TEXTURE) == (pfTexture *)arg;
}

static void
pfuReplaceNodeEverywhere(pfNode *node, pfNode *replacement)
{
    int numParents;
    if (node == replacement)
        return;
    numParents = node->getNumParents();
    while (numParents > 0)
        node->getParent(--numParents)->replaceChild(node, replacement);
}


#define MAX_CLIP_TEXS 100
extern "C" PFPFB_DLLEXPORT pfNode *
pfdLoadFile_substclip(char *fileName)
{
    /*
     * Make sure these are initialized so we can abort any time
     * an error occurs...
     */
    int ncliptexs = 0;
    pfClipTexture *cliptexs[MAX_CLIP_TEXS];
    pfNode *scene = NULL;

    char *texture_index_env = getenv("PFSUBSTCLIP_TEXTURE_I");
    char *normal_axis_env = getenv("PFSUBSTCLIP_NORMAL_AXIS");
    int texture_index = -1; /* so incrementing will get to 0 (the default) */
    int normal_axis_from_env = -1;

    /*
     * Get the clip texture from the configuration file named
     * by the environment variable "PFSUBSTCLIP_TEXTURE"
     * (can be a comma-separated list).
     */
    char *cliptex_filenames = getenv("PFSUBSTCLIP_TEXTURE");
    if (cliptex_filenames == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfdLoadFile_substclip: env variable PFSUBSTCLIP_TEXTURE not set");
	goto return_NULL;
    }

    for (ncliptexs = 0; *cliptex_filenames != '\0'; ncliptexs++)
    {
	/*
	 * Read the next clip texture filename (up to ',' or end of string)
	 * and advance cliptex_filenames to the beginning of the next one...
	 */
	char cliptex_filename[PF_MAXSTRING];
	sscanf(cliptex_filenames, "%[^,]", cliptex_filename);
	while (*cliptex_filenames != '\0'
	    && *cliptex_filenames != ',')
	    cliptex_filenames++;
	if (*cliptex_filenames == ',')
	    cliptex_filenames++;
	
	PFASSERTALWAYS(ncliptexs < MAX_CLIP_TEXS);
	cliptexs[ncliptexs] = pfdLoadClipTexture(cliptex_filename);
	if (cliptexs[ncliptexs] == NULL)
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfdLoadFile_substclip: Couldn't load clip texture from file \"%s\"",
		     cliptex_filename);
	    goto return_NULL;
	}
	cliptexs[ncliptexs]->setName(cliptex_filename);
    }


    /*
     * Load the scene from the file name without the ".substclip"
     */
    {
	char *lastdot = strrchr(fileName, '.');
	if (lastdot == NULL)
	{
	    pfNotify(PFNFY_WARN, PFNFY_USAGE,
		    "pfdLoadFile_substclip: bad file name %s", fileName);
	    goto return_NULL;
	}
	char realName[PF_MAXSTRING];
	strncpy(realName, fileName, lastdot-fileName);
	realName[lastdot-fileName] = '\0';

	scene = pfdLoadFile(realName);
	if (scene == NULL)
	    goto return_NULL; /* error message printed already */
    }

    int i;
    FOR (i, ncliptexs)
    {
	/*
	 * If the environment variable PFSUBSTCLIP_TEXTURE_I is set,
	 * let that be the index of the texture in the scene to replace
	 * with the clip texture.
	 * If there is more than one clip texture, this should be
	 * a comma-separated list.
	 * If the environment variable is not set, the list
	 * is taken to be 0,1,2,3,4,...
	 */
	if (texture_index_env != NULL && *texture_index_env != '\0')
	{
	    /*
	     * Read the next texture index from the env PFSUBSTCLIP_TEXTURE_I
	     * list and advance to the beginning of the next entry...
	     */
	    texture_index = atoi(texture_index_env);
	    while (*texture_index_env != '\0' && *texture_index_env != ',')
		texture_index_env++;
	    if (*texture_index_env == ',')
		texture_index_env++;
	}
	else
	    texture_index++; /* by default, step through indices in the scene */

	pfTexture *tex = pfuFindTexture(scene, texture_index);

	if (tex == NULL)
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfdLoadFile_substclip: no texture with index %d\n",
		     texture_index);
	    goto return_NULL;
	}

	/*
	 * Change the selected texture to cliptex,
	 * and insert a clip centering node above the lowest common ancestor node
	 * of all occurances of cliptex in the result.
	 */
	int travmode = PFUTRAV_SW_ALL | PFUTRAV_SEQ_ALL |
		       PFUTRAV_LOD_ALL | PFUTRAV_LAYER_BOTH;
	pfuReplaceTexture(scene, tex, cliptexs[i], travmode);
	pfNode *lca = pfuLowestCommonAncestorOfGeoSets(scene,
						       _geosetContainsTexture,
						       cliptexs[i],
						       travmode);
	assert(lca != NULL);
	pfNode *refnode = lca;

	if (normal_axis_env != NULL)
	{
	    if (*normal_axis_env != '\0')
	    {
		/*
		 * Read a number from the env PFSUBSTCLIP_NORMAL_AXIS list
		 * and advance to the beginning of the next entry...
		 */
		normal_axis_from_env = atoi(normal_axis_env);
		while (*normal_axis_env != '\0' && *normal_axis_env != ',')
		    normal_axis_env++;
		if (*normal_axis_env == ',')
		    normal_axis_env++;
	    }
	    /* else just leave normal_axis_from_env what it was from last time */

	    /*
	     *  refnode = a geode
	     *	with a single geoset
	     *	    with a single rectangle parallel to the xy plane
	     *	    textured by the given texture
	     *
	     * Use pfuGetClosestPoint() to get the vertex and texture coords
	     * of the four corners (kind of heavyweight, but the api exists
	     * and it's only at startup).
	     */
	    pfBox bbox;
	    pfuTravCalcBBox(lca, &bbox);
	    pfVec3 bboxsize = bbox.max - bbox.min;
	    int ax2 = normal_axis_from_env >= 0 ? normal_axis_from_env%3
				    : MINI3(bboxsize); /* usually the Z axis */
	    int ax0 = (ax2+1)%3;	/* usually the X axis */
	    int ax1 = (ax0+1)%3;	/* usually the Y axis */

	    float eyes[4][3];
	    /* vertex #0: lower left */
	    eyes[0][ax0] = LERP(bbox.min[ax0], bbox.max[ax0], -1.0f);
	    eyes[0][ax1] = LERP(bbox.min[ax1], bbox.max[ax1], -1.0f);
	    eyes[0][ax2] = LERP(bbox.min[ax2], bbox.max[ax2],  0.5f);
	    /* vertex #1: lower right */
	    eyes[1][ax0] = LERP(bbox.min[ax0], bbox.max[ax0],  2.0f);
	    eyes[1][ax1] = LERP(bbox.min[ax1], bbox.max[ax1], -1.0f);
	    eyes[1][ax2] = LERP(bbox.min[ax2], bbox.max[ax2],  0.5f);
	    /* vertex #2: upper right */
	    eyes[2][ax0] = LERP(bbox.min[ax0], bbox.max[ax0],  2.0f);
	    eyes[2][ax1] = LERP(bbox.min[ax1], bbox.max[ax1],  2.0f);
	    eyes[2][ax2] = LERP(bbox.min[ax2], bbox.max[ax2],  0.5f);
	    /* vertex #3: upper left */
	    eyes[3][ax0] = LERP(bbox.min[ax0], bbox.max[ax0], -1.0f);
	    eyes[3][ax1] = LERP(bbox.min[ax1], bbox.max[ax1],  2.0f);
	    eyes[3][ax2] = LERP(bbox.min[ax2], bbox.max[ax2],  0.5f);

	    float (*vcoords)[3] = (float (*)[3])pfMalloc(4*sizeof(*vcoords), NULL);
	    float (*tcoords)[2] = (float (*)[2])pfMalloc(4*sizeof(*tcoords), NULL);
	    assert(vcoords != NULL && tcoords != NULL);
	    int j;
	    FOR (j, 4)
		pfuGetClosestPoint(lca, eyes[j][0], eyes[j][1], eyes[j][2],
				   cliptexs[i], travmode,
				   &vcoords[j][0], &vcoords[j][1], &vcoords[j][2],
				   &tcoords[j][0], &tcoords[j][1], NULL);

	    pfGeoState *geostate = new pfGeoState();
	    assert(geostate != NULL);
	    geostate->setAttr(PFSTATE_TEXTURE, cliptexs[i]);
	    geostate->setMode(PFSTATE_ENTEXTURE, 1);

	    pfGeoSet *geoset = new pfGeoSet();
	    assert(geoset != NULL);
	    geoset->setGState(geostate);
	    geoset->setNumPrims(1);
	    geoset->setPrimType(PFGS_QUADS);
	    geoset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
	    geoset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, vcoords, NULL);

	    pfGeode *geode = new pfGeode();
	    assert(geode != NULL);
	    geode->addGSet(geoset);

	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Bounding box: (%g,%g,%g)..(%g,%g,%g)",
		     bbox.min[0], bbox.min[1], bbox.min[2],
		     bbox.max[0], bbox.max[1], bbox.max[2]);
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Proxy rectangle for clip centering:");
	    FOR (j, 4)
		pfNotify(PFNFY_DEBUG, PFNFY_MORE,
		     "        x=%g y=%g z=%g    s=%g t=%g",
		     vcoords[j][0], vcoords[j][1], vcoords[j][2],
		     tcoords[j][0], tcoords[j][1]);

	    refnode = geode;
	}

	pfuClipCenterNode *clipcenter_node = new pfuClipCenterNode();
	clipcenter_node->setRefNode(refnode);
	clipcenter_node->setClipTexture(cliptexs[i]);
	/*
	 * For each parent of child,
	 * sever the relationship and add clipcenter_node to the
	 * parent in place of child.
	 * Don't add child to clipcenter_node until done
	 * with this replacement, and keep a ref to child throughout.
	 */
	pfRef(lca);
	pfuReplaceNodeEverywhere(lca, clipcenter_node);
	clipcenter_node->addChild(lca);
	pfUnrefDelete(lca);

	if (lca == scene)
	    scene = clipcenter_node;
    }

    return scene;

return_NULL:
    if (scene)   pfDelete(scene);
    FOR (i, ncliptexs)
	pfDelete(cliptexs[i]);
    return NULL;
}
