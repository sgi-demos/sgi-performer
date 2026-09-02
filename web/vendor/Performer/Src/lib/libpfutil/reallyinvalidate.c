/*
 * Copyright 1999, Silicon Graphics, Inc.
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
 * reallyinvalidate.c
 * $Revision: 1.4 $
 *
 * Utility function to force complete reloading of a clip texture from disk.
 */


#include "reallyinvalidate.h"

#define MASTER_THEN_SLAVES(stuff) \
    { \
	if (master != NULL) \
	    pfuBarrierEnter(barr, totalNum); /* slaves wait here */ \
	{ stuff } \
	if (master == NULL) \
	    pfuBarrierEnter(barr, totalNum); /* master waits here */ \
    }

#define SLAVES_THEN_MASTER(stuff) \
    { \
	if (master == NULL) \
	    pfuBarrierEnter(barr, totalNum); /* master waits here */ \
	{ stuff } \
	if (master != NULL) \
	    pfuBarrierEnter(barr, totalNum); /* slaves wait here */ \
    }

static int intlog2(unsigned n) { return n>1 ? 1+intlog2(n/2) : 0; }
static int _pfGetClipTextureNumLevels(pfClipTexture *clipTex)
{
    int sizeS, sizeT, sizeR;
    pfGetClipTextureVirtualSize(clipTex, &sizeS, &sizeT, &sizeR);
    return 1 + intlog2(PF_MAX3(sizeS, sizeT, sizeR));
}


extern PFUDLLEXPORT void pfuReallyInvalidateClipTexture(pfClipTexture *clipTex, pfuBarrier *barr)
{
    int nLevels, i;
    int totalNum; /* total # of slaves, masters, master's slaves, and self */
    pfClipTexture *master = pfGetClipTextureMaster(clipTex);

    /*
     * Calculate totalNum, i.e. the number of participating processes.
     */
    {
	pfList *slaves = pfGetClipTextureSlaves(clipTex);
	int numSlaves = (slaves != NULL ? pfGetNum(slaves)
					: 0);
	pfList *mastersSlaves = (master != NULL ? pfGetClipTextureSlaves(master)
						: NULL);
	int numMastersSlaves = (mastersSlaves != NULL ? pfGetNum(mastersSlaves)
						      : 0);
	totalNum = 1 + numSlaves + numMastersSlaves;
    }
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Number of processess participating in barrier = %d\n", totalNum);


    nLevels = _pfGetClipTextureNumLevels(clipTex);
    for (i = 0; i < nLevels; ++i) /* coarsest to finest (really) */
    {
	int levelSize = 1<<i;
	pfObject *level = pfGetClipTextureLevel(clipTex, nLevels-1 - i);
	if (pfIsOfType(level, pfGetImageCacheClassType()))
	{
	    /*
	     * NOTE: We could just set the MemRegionSize
	     * to zero and back, and not worry about the origin
	     * or the tex region size and origin at all,
	     * but that produces debug warnings
	     * during the first apply() to the effect
	     * that the mem region doesn't contain the tex region.
	     * Furthermore just setting the texReg size to 0,0
	     * doesn't help since the warning criteria is kind of stupid.
	     * Therefore we carefully arrange both regions so
	     * that they both have size 0,0 in the same location.
	     * The easiest way to do this is to place both their origins at 0,0.
	     */

	    /*
	     * NOTE: pfApplyImageCache(ic) is the same as
	     * pfDTRApplyImageCache(ic, -1.f), which is
	     * NOT the same as pfDTRApplyImageCache(ic, big number).
	     * The difference is that pfDTRApplyImageCache(ic, big number)
	     * does the DTR check and therefore won't block on tiles
	     * that are waiting to be read.
	     */

	    pfImageCache *ic = (pfImageCache *)level;
	    int memOrgS, memOrgT, memOrgR;
	    int memSizeS, memSizeT, memSizeR;
	    int texOrgS, texOrgT, texOrgR;
	    int texSizeS, texSizeT, texSizeR;
	    pfGetImageCacheMemRegionOrg(ic, &memOrgS, &memOrgT, &memOrgR);
	    pfGetImageCacheMemRegionSize(ic, &memSizeS, &memSizeT, &memSizeR);
	    pfGetImageCacheTexRegionOrg(ic, &texOrgS, &texOrgT, &texOrgR);
	    pfGetImageCacheTexRegionSize(ic, &texSizeS, &texSizeT, &texSizeR);

	    /*
	     * NOTE: If this was time-critical,
	     * we'd put the synchronization stuff outside the loop.
	     * Then we'd have to do two loops, saving the old
	     * sizes in an array or something.
	     */
	    MASTER_THEN_SLAVES(
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "    %s Really invalidating %dx%d level",
			 master==NULL ? "master" : "slave",
			 levelSize, levelSize);
		pfImageCacheMemRegionOrg(ic, 0, 0, 0);
		pfImageCacheMemRegionSize(ic, 0, 0, 0);
		pfImageCacheTexRegionOrg(ic, 0, 0, 0);
		pfImageCacheTexRegionSize(ic, 0, 0, 0);
		pfDTRApplyImageCache(ic, 0.);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "    %s Applied %dx%d level to invalidate",
			 master==NULL ? "master" : "slave",
			 levelSize, levelSize);
	    )
	    SLAVES_THEN_MASTER(;) /* make sure slaves are done with the above */
	    MASTER_THEN_SLAVES(
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "    %s Restoring %dx%d level",
			 master==NULL ? "master" : "slave",
			 levelSize, levelSize);
		pfImageCacheMemRegionOrg(ic, memOrgS, memOrgT, memOrgR);
		pfImageCacheMemRegionSize(ic, memSizeS, memSizeT, memSizeR);
		pfImageCacheTexRegionOrg(ic, texOrgS, texOrgT, texOrgR);
		pfImageCacheTexRegionSize(ic, texSizeS, texSizeT, texSizeR);
		pfDTRApplyImageCache(ic, 0.);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "    %s Applied %dx%d level to restore",
			 master==NULL ? "master" : "slave",
			 levelSize, levelSize);
	    )
	    SLAVES_THEN_MASTER(;) /* make sure slaves are done with the above */
	}
	else /* it's an image tile. this is not supported */
	{
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		     "pfuReallyInvalidateClipTexture: can't reallyInvalidate %dx%d level, try making it an image cache instead of an image tile", levelSize, levelSize);
	}
    }

    /*
     * Apparently need the following to get it downloaded
     * into texture.  I don't claim to understand what this does
     * or what it's supposed to do.
     */
    if (master == NULL)
	pfInvalidateClipTexture(clipTex);
}
