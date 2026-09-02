/*
 * Copyright 2000, Silicon Graphics, Inc.
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

#include <Performer/pr.h>
#include <Performer/pfutil.h>
#include <math.h>

#define log2(x) (log(x)*M_LOG2E)
#define round(x) floor((x)+.5)
#define INRANGE(foo,bar,baz) ((foo(bar))&&((bar)baz))

//
// Here is the logic that decides LODOffset, numEffectiveLevels, minLOD, maxLOD
// based on every piece of information I can think of.
//
// XXX Need to parametrize warning situations (when to emit a warning,
// XXX allow preventing multiple warnings per frame, ...)
//

// XXXX this is copied into pfASD.C. please update it when you
// make any changes.

extern "C"
PFUDLLEXPORT void pfuCalcVirtualClipTexParams(
	int nLevels,	    // total number of virtual levels
	int clipSize,
	int invalidBorder,
	float minLODTexPix, // lower bound on the resolution the hardware
			    // will access, based on calculation of
			    // tex/pix partial derivatives (don't
			    // forget to add in LODbiasS,LODbiasT!)
			    // or a height-above-terrain lookup table
	float minLODLoaded, // float for fade-in
	float maxLODLoaded, // may use someday if we get a way to reuse LODs
	float bboxMinDist,  // min distance from clip center in texture coords
	float bboxMaxDist,  // max distance from clip center in texture coords
	float tradeoff, // 0. means go fine (hi resolution at expense of area)
			// 1. means go coarse (area at expense of resolution)
	const struct pfVirtualClipTexLimits *limits,
	int *return_LODOffset,
	int *return_numEffectiveLevels,
	float *return_minLOD,
	float *return_maxLOD)
{
    static const pfVirtualClipTexLimits defaultLimits =
    {
	{-1000,1000}, {-1000,1000}, {-1000.f,1000.f}, {-1000.f,1000.f}
    };
    if (limits == NULL)
	limits = &defaultLimits;

    //
    // Always assert the following-- I have no idea what
    // this function will do if they don't hold.
    //
    PFASSERTALWAYS(clipSize >= 1); // doesn't need to be power of 2
    PFASSERTALWAYS(nLevels >= 1);
    PFASSERTALWAYS(limits->LODOffset.lo <= limits->LODOffset.hi);
    PFASSERTALWAYS(limits->numEffectiveLevels.lo <= limits->numEffectiveLevels.hi);
    PFASSERTALWAYS(limits->minLOD.lo <= limits->minLOD.hi);
    PFASSERTALWAYS(limits->maxLOD.lo <= limits->maxLOD.hi);

    tradeoff = PF_CLAMP(tradeoff, 0., 1.);
    invalidBorder = PF_CLAMP(invalidBorder, 0, clipSize/2);

    int effectiveClip = clipSize - invalidBorder; // XXX not 2*invalidBorder, due to iR bug
    float bboxMinDistInFinestLevelTexels = bboxMinDist * (1<<(nLevels-1));
    int minLODbbox = (bboxMinDistInFinestLevelTexels <= .5*effectiveClip ? 0 :
	   (int)ceil(log2(bboxMinDistInFinestLevelTexels/(.5*effectiveClip))));
    //
    // Anti-scintillation:
    //    For the best mipmapping (least scintillation) effect,
    //    we want the window of (usually 7 on iR) good levels
    //    to be placed so that it starts at the finest level
    //    we will want to access.
    //
    float LODOffset = (int)floor(PF_MAX4(minLODTexPix,          // float
					 minLODLoaded,		// int
					 minLODbbox,	        // int
					 limits->minLOD.lo));   // float
    LODOffset = PF_CLAMP(LODOffset, 0, PF_MIN2(nLevels-1, maxLODLoaded));

    //
    // But user-specified bounds take precedence...
    //
    LODOffset = PF_CLAMP(LODOffset, limits->LODOffset.lo, limits->LODOffset.hi);

    //
    // Anti-jello:
    //    For the best numerical stability, always make the good area as tight
    //    as possible around the texture space bbox by decreasing the
    //    mipmap numEffectiveLevels (now that LODOffset is fixed).
    //
    float numEffectiveLevels = nLevels - LODOffset; // this would make the good
						 // area be the whole texture,
						 // but that may not be
						 // possible or desireable...
        float safeRadius = 5./16.; // with a good area of size 1 moving
			           // around at a granularity of at most .25,
			           // the safe diameter centered at the clip
				   // center is 3/4, i.e. safe radius = 3/8.
			           // But this is just barely safe
				   // (i.e. not really) and radius 1/4
				   // is definitely safe, so for
			           // a threshold we use avg(1/4,3/8) = 5/16.
	//
	// How many times can safeRadius be halved and still contain
	// the bbox?
	// XXX make this 0-safe!!!
	//
	int howMany = (int)floor(log2(safeRadius/bboxMaxDist));
	if (howMany > 0)
	    numEffectiveLevels -= howMany;

    // Kind of hacky, but if we make numEffectiveLevels too small,
    // the good area size is so small that the movement granularity
    // is so small that fine levels get misaligned.
    // We probably don't need to shrink any smaller than 12 effective
    // levels (2Kx2K) for anti-jello purposes anyway, so...
    // XXX 12 is kind of arbitrary; should quantify and make this exact!
    numEffectiveLevels = PF_MAX2(numEffectiveLevels, 12);
    numEffectiveLevels = PF_MIN2(numEffectiveLevels, nLevels-LODOffset);

    //
    // In an ideal world, we'd now have the desired LODOffset
    // and numEffectiveLevels.
    // But the hardware and user-specified constraints may keep us from
    // getting the full numEffectiveLevels, in which case we have to shrink it.
    // There are a number of options, depending
    // on the tradeoff parameter:
    //		tradeoff=0.: just decrease numEffectiveLevels without changing
    //			     LODOffset (i.e. choose high res but
    //			     possibly garbage on the horizon)
    //		tradeoff=1.: decrease numEffectiveLevels but increase LODOffset
    //			     by the same amount (i.e. get the desired
    //			     area but possibly blur out fine levels)
    //		0.<tradeoff<1.: somewhere in between, approximately
    //			     proportional to tradeoff
    // 
    if (numEffectiveLevels > limits->numEffectiveLevels.lo)
    {
	int maxNumEffectiveLevels = PF_MIN2(limits->numEffectiveLevels.hi,
				       PF_MAX2(limits->numEffectiveLevels.lo, 16));
				       // XXX 16 is hardware-specific
	if (numEffectiveLevels > maxNumEffectiveLevels)
	{
	    int adjustment_needed = numEffectiveLevels - maxNumEffectiveLevels;
	    int adjust_coarse = (int)round((1.-tradeoff)*adjustment_needed);
			// when in doubt (e.g. adjustment_needed is odd
			// and tradeoff = .5), round toward coarse
			// (but don't count on this in future implementations).
	    int adjust_fine = adjustment_needed - adjust_coarse;
	    PFASSERTALWAYS(adjust_coarse >= 0 && adjust_fine >= 0);

	    LODOffset = PF_MIN2(LODOffset+adjust_fine, limits->LODOffset.hi);
	    numEffectiveLevels = maxNumEffectiveLevels;
	}
    }
    else // numEffectiveLevels <= limits->numEffectiveLevels.lo
    {
	numEffectiveLevels = limits->numEffectiveLevels.lo;
	LODOffset = PF_MIN2(LODOffset, nLevels-numEffectiveLevels);
	LODOffset = PF_MAX2(LODOffset, 0);
	LODOffset = PF_CLAMP(LODOffset, limits->LODOffset.lo,
					limits->LODOffset.hi);
    }

    //
    // The above code should guarantee that the following hold...
    //
    PFASSERTDEBUG(INRANGE(limits->numEffectiveLevels.lo
		      <=, numEffectiveLevels,
		      <=  limits->numEffectiveLevels.hi));
    PFASSERTDEBUG(INRANGE(limits->LODOffset.lo
		      <=, LODOffset,
		      <=  limits->LODOffset.hi));

    float minLOD = minLODLoaded; // XXX should soften by fade-in factor somehow
    minLOD = PF_CLAMP(minLOD, limits->minLOD.lo, limits->minLOD.hi);

    float maxLOD = LODOffset + 6;//XXX should be hairy function of invalidBorder
    maxLOD = PF_MIN2(maxLOD, maxLODLoaded);
    maxLOD = PF_CLAMP(maxLOD, limits->maxLOD.lo, limits->maxLOD.hi);

    *return_LODOffset = (int)LODOffset;
    *return_numEffectiveLevels = (int)numEffectiveLevels;
    *return_minLOD = minLOD;
    *return_maxLOD = maxLOD;
}
