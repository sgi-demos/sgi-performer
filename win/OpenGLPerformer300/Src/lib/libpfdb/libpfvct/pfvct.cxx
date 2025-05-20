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
 * pfvct.c - "Virtual Clip Texture" pseudo-loader
 *
 * Pseudo-loader that loads a scene graph from
 * from a file of any format,
 * traverses the scene, and makes all clip textures virtual.
 *
 * To use it, simply append ".vct" to the end of the scene graph file name.
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
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <stdlib.h>
#include <assert.h>

static int intlog2(int n) { return n == 1 ? 0 : 1 + intlog2(n/2); }
#define MAX_CLIPMAP_DEPTH 16 // XXX should be glGetIntegerv(GL_MAX_CLIPMAP_DEPTH_SGIX) or equivalent Performer query

/*
 * Trick the pfuLowestCommonAncestorOfGeoSets() function into
 * simply traversing every geoset in the entire scene,
 * by providing a callback function that always returns FALSE.
 * This function makes all clip textures virtual (by
 * making numEffectiveLevels be less than the number of levels).
 * It doesn't hurt if a clip texture is encountered more than once
 * in the traversal.
 */
static int _makeClipTexturesVirtual(pfGeoSet *gset, void *)
{
    pfGeoState *gstate = gset->getGState();
    if (gstate != NULL
     && gstate->getMode(PFSTATE_ENTEXTURE))
    {
	pfTexture *tex = (pfTexture *)gstate->getAttr(PFSTATE_TEXTURE);
	if (tex->isOfType(pfClipTexture::getClassType()))
	{
	    pfClipTexture *cliptex = (pfClipTexture *)tex;
	    int oldNumEffectiveLevels = cliptex->getNumEffectiveLevels();
	    int sizeS, sizeT, sizeR;
	    cliptex->getVirtualSize(&sizeS, &sizeT, &sizeR);
	    int numLevels = intlog2(PF_MAX3(sizeS, sizeT, sizeR)) + 1;
	    int newNumEffectiveLevels = PF_MIN2(numLevels-1, MAX_CLIPMAP_DEPTH);
	    if (newNumEffectiveLevels != oldNumEffectiveLevels)
	    {
		pfNotify(PFNFY_INFO, PFNFY_PRINT,
			 "pfdLoadFile_vct: changing numEffectiveLevels "
			 "of clip texture \"%s\""
			 "from %d to %d\n",
			 cliptex->getName(),
			 oldNumEffectiveLevels, newNumEffectiveLevels);
		cliptex->setNumEffectiveLevels(newNumEffectiveLevels);
	    }
	}
    }
    return FALSE;
}

extern "C" PFPFB_DLLEXPORT pfNode *
pfdLoadFile_vct(char *fileName)
{
    /*
     * Load the scene from the file name without the ".vct"
     */
    const char *lastdot = strrchr(fileName, '.');
    if (lastdot == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		"pfdLoadFile_substclip: bad file name %s", fileName);
	return NULL;
    }
    char realName[PF_MAXSTRING];
    strncpy(realName, fileName, lastdot-fileName);
    realName[lastdot-fileName] = '\0';

    pfNode *node= pfdLoadFile(realName);
    if (node != NULL)
    {
	int travmode = PFUTRAV_SW_ALL | PFUTRAV_SEQ_ALL |
		       PFUTRAV_LOD_ALL | PFUTRAV_LAYER_BOTH;
	(void) pfuLowestCommonAncestorOfGeoSets(node,
					        _makeClipTexturesVirtual,
					        NULL,
					        travmode);
    }
    return node;
}
