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
 * file: cliptexture.c
 * ----------------
 *
 * Contains generic cliptexture routines. Specifically the function
 *	pfuAddMPClipTextureToPipes()
 *  which adds mpcliptextures to one or more pipes properly. 
 * $Revision: 1.35 $
 * $Date: 2002/05/15 00:31:09 $
 */

#include <stdlib.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>
#include <math.h>
#ifndef WIN32
#include <values.h>
#endif

/* 
** Connects an mpcliptexture to a pipe. If there's more than one pipe
** then one pipe is given the cliptexture, and the rest of the pipes
** get slaves pointing to that cliptexture, which becomes the master.
**
** The mpcliptexture must not be a slave. All pipes must
** exist when this routine is called.
**
** Routine works "properly", by not adding slaves to
** pipes that already have the cliptexture, or have a slave that
** points to the cliptexture. Thus, the routine will not generate
** any debug messages, even if the routine is called multiple times with
** the same cliptexture and pipes.
**
** The routine returns the address of the pipe that has the master, or NULL
** if there are problems.
**
** If _masterpipe is not null, then that pipe will have the master
** mpcliptexture added to it. If the _masterpipe is NULL, or if the master
** or one of its slaves is already attached to _masterpipe, the routine will
** assign the master (if one hasn't already been assigned) to the first pipe
** it finds without a copy of this cliptexture or one of its slaves. 
**
** If _pipes is null, then all existing pipes will be used
**
*/
PFUDLLEXPORT pfPipe *
pfuAddMPClipTextureToPipes(pfMPClipTexture *_master, 
			   pfPipe * _masterpipe,
			   pfPipe *_pipes[])
{
    pfMPClipTexture *mpct, *master;
    pfPipe **pipearray, *masterpipe;
    pfPipe *allpipes[100]; /* assuming never more than 100 pipes */
    int cliptexi, ncliptex, npipes, pipei;
    int found;

    /* check arguments */
    if(_master == NULL)
        return NULL;

    /* if _master cliptexture is a slave, then give up */
    if(pfGetMPClipTextureMaster(_master))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		 "pfuAddMPClipTextureToPipes: pfMPClipTexture (%x) is a slave",
		 _master);
        return NULL;
    }
    if(_pipes)
        pipearray = _pipes;
    else 
    {
        pipearray = allpipes;

        npipes = pfGetMultipipe();
        for(pipei = 0; pipei < npipes; pipei++) 
	{
	    pipearray[pipei] = pfGetPipe(pipei);
	}
        pipearray[pipei] = NULL;
    }
      
    masterpipe = _masterpipe;

    /* use _masterpipe to set cliptexture master */
    if(masterpipe) {
        ncliptex = pfGetPipeNumMPClipTextures(_masterpipe);
	/* error checking */
	found = FALSE;
        for(cliptexi = 0; cliptexi < ncliptex; cliptexi++) {
	    mpct = pfGetPipeMPClipTexture(masterpipe, cliptexi);
	    if(mpct == _master) {
	        found = TRUE;
	        break; /* do nothing if master is already in this pipe */
	    }
	    master = pfGetMPClipTextureMaster(mpct);
	    if(_master == master) {
	        found = TRUE;
	        break; /* do nothing if slave of master is in this pipe */
	    }
	}
	/* if no problems found, add master to this pipe */
	if(!found)
	    pfAddMPClipTexture(masterpipe, _master);
	else 
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "pfuAddMPClipTextureToPipes: _master or slave to "
		     "master already found in masterpipe\n");
    }

    for(; *pipearray; pipearray++) {
        ncliptex = pfGetPipeNumMPClipTextures(*pipearray);
	found = FALSE;
        for(cliptexi = 0; cliptexi < ncliptex; cliptexi++) {
	    mpct = pfGetPipeMPClipTexture(*pipearray, cliptexi);
	    if(mpct == _master) {
	        masterpipe = *pipearray;
	        found = TRUE;
	        break; /* do nothing if master is already in this pipe */
	    }
	    master = pfGetMPClipTextureMaster(mpct);
	    if(_master == master) {
	        found = TRUE;
	        break; /* do nothing if slave of master is in this pipe */
	    }
	}
	/* after master added to one pipe, all other adds will create slaves */
	if(!found)
	    pfAddMPClipTexture(*pipearray, _master);
    }
    return masterpipe;
}


/*
** List version of routine above. Takes a list of mpcliptexture pointers
** and applies them to the pipe list. Routine assumes one pipe will be
** master for all cliptextures
**
** XXX TODO Should probably allow a list of masterpipes too...
*/

PFUDLLEXPORT void
pfuAddMPClipTexturesToPipes(pfList *_mpcliptextures, 
			    pfPipe * _masterpipe,
			    pfPipe *_pipes[])
{
    int n, ncliptextures;

    ncliptextures = pfGetNum(_mpcliptextures);
    for(n = 0; n < ncliptextures; n++)
    {
        (void)pfuAddMPClipTextureToPipes((pfMPClipTexture*)
					 pfGet(_mpcliptextures, n),
					 _masterpipe,
					 _pipes);
    }
}

/* is each dimension in array a power of two ? */
static int isPow2(int val[])
{
    int i;
    for(i = PF_S; i <= PF_R;i++)
    {
	if(val[i] & (val[i] - 1))
	    return FALSE;
    }
    return TRUE;
}


static int checkClipTex(pfuClipTexConfig *config)
{

    if(config->clipSize * 2 < config->minICache[PF_S] ||
       config->clipSize * 2 < config->minICache[PF_T] ||
       config->clipSize * 2 < config->minICache[PF_R])
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: clipsize %d is too small "
		 "relative to minimum image cache size (%d, %d, %d). "
		 "Set sizes so that all clipped levels are image caches.",
		 config->clipSize,
		 config->minICache[PF_S],
		 config->minICache[PF_T],
		 config->minICache[PF_R]);
	return 1;
    }

    /* check to see if mandatory values were filled in */
    if(config->extFormat < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: mandatory field "
		 "external format wasn't set");
	return 1;
    }

    if(config->intFormat < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: mandatory field "
		 "internal format wasn't set");
	return 1;
    }

    if(config->imgFormat < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: mandatory field "
		 "image format wasn't set");
	return 1;
    }

    if(config->components < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: mandatory field "
		 "number of components wasn't set");
	return 1;
    }

    if(config->imgSize[PF_S] < 0 ||
       config->imgSize[PF_T] < 0 ||
       config->imgSize[PF_R] < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: mandatory field "
		 "image size wasn't set");
	return 1;
    }

    if(config->imgSize[PF_S] == 0 ||
       config->imgSize[PF_T] == 0 ||
       config->imgSize[PF_R] == 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: "
		 "one or more image size dimensions are zero (%d %d %d)",
		 config->imgSize[PF_S], 
		 config->imgSize[PF_T], 
		 config->imgSize[PF_R]); 
	return 1;
    }

    if(config->clipSize < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: mandatory field "
		 "clip size wasn't set");
	return 1;
    }

    if(config->minICache[PF_S] == 0 ||
       config->minICache[PF_T] == 0 ||
       config->minICache[PF_R] == 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: "
		 "one or more minimum image cache size dimensions are zero "
		 "(%d %d %d)",
		 config->minICache[PF_S], 
		 config->minICache[PF_T], 
		 config->minICache[PF_R]); 
	return 1;
    }

    /* We'll let you do this, even though it doesn't make much sense */
    if(config->clipSize == 0)
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: clip size is zero");

    return 0; /* checks passed */
}


pfClipTexture *
pfuMakeClipTexture(pfuClipTexConfig *config)
{
    int level;
    int levelSizeT, levelSizeS;
    int minImageDim;
    int ratio[3];
    pfClipTexture *ct;
    pfImageCache *ic;
    pfImageTile *it;
    void *arena = pfGetSharedArena();

    if(checkClipTex(config))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: parameter checking failed");
	return NULL;
    }

    ct = pfNewClipTexture(arena);
    
    /* give the clip texture a name */
    /* XXX it would be nice if we could confirm it's a valid string */
    if(config->name) 
	pfTexName((pfTexture *)ct, config->name);

    if(!isPow2(config->imgSize))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: imageSize dimension(s) not power of two: "
		 "(%d, %d, %d)", 
		 config->imgSize[PF_S],
		 config->imgSize[PF_T],
		 config->imgSize[PF_R]);
	return NULL;
    }
    pfClipTextureVirtualSize(ct, 
			     config->imgSize[PF_S], 
			     config->imgSize[PF_T],
			     config->imgSize[PF_R]);


    /* Find min and max dimension size (S & T only) */
    if(config->imgSize[PF_S] < config->imgSize[PF_T])
	minImageDim = config->imgSize[PF_S];
    else
	minImageDim = config->imgSize[PF_T];

    /* make sure clip size is a power of two in each dimension */
    if(config->clipSize & (config->clipSize - 1))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeClipTexture: clipSize %d not power of two",
		 config->clipSize);
	return NULL;
    }
    pfClipTextureClipSize(ct, config->clipSize);


    pfTexImage((pfTexture *)ct, NULL, config->components, 
	       config->clipSize, config->clipSize, 1);

    pfTexFormat((pfTexture *)ct, PFTEX_EXTERNAL_FORMAT, config->extFormat);
    pfTexFormat((pfTexture *)ct, PFTEX_INTERNAL_FORMAT,	config->intFormat);
    pfTexFormat((pfTexture *)ct, PFTEX_IMAGE_FORMAT, config->imgFormat);


    if(minImageDim < config->clipSize)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "Minimum dimension of Virtual Size %d "
		 "is smaller than Clip Size %d",
		 minImageDim, config->clipSize);
	
    }

    /* XXX shouldn't this be checked from system config info? */
    if(config->invBorder < 8) {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		 "Invalid Border Value %d too small: setting to 8\n",
		 config->invBorder);
	config->invBorder = 8;
    }
    pfClipTextureInvalidBorder(ct, config->invBorder);

    /* XXX this should be checked from system info too, but I don't want
       to risk getting it wrong */
    pfClipTextureNumEffectiveLevels(ct, config->effLevels);
    pfClipTextureNumAllocatedLevels(ct, config->allocLevels);

    /* 
    ** If minIcache size isn't set, pick a reasonable default.
    ** Two Cases:
    ** First case; cliptexture is square.
    ** If neither are S nor T is set, we choose the default (one level
    ** bigger than clipsize). If either S or T are set, we use the set
    ** value for both.
    **
    ** Second case: cliptexture isn't square.
    **
    ** XXX We don't even know if the hardware supports non-square clipmaps
    **
    ** If S or T is set we compute the clipmap size ratio, and use it
    ** to compute the missing coordinate. If both are missing, we set
    ** the smaller coordinate to the clipsize, and the bigger coordinate
    ** to clipsize scaled by the ratio.
    */

    if(config->minICache[PF_R] < 0)
	config->minICache[PF_R] = 1;

    if(config->minICache[PF_S] < 0 ||
       config->minICache[PF_T] < 0)
    {
	if(config->imgSize[PF_S] == config->imgSize[PF_T]) /* square */
	{
	    if(config->minICache[PF_S] >= 0) /* s is set: t <- s */
		config->minICache[PF_T] = config->minICache[PF_S];
	    else if(config->minICache[PF_T] >= 0)
		/* t is set: set s <- t */
		config->minICache[PF_S] = config->minICache[PF_T];
	    else /* neither set; use the default */
		config->minICache[PF_S] = config->minICache[PF_T] =
		    config->clipSize * 2;

	}
	else /* not square */
	{
	    int ratio;
	    ratio = config->imgSize[PF_S]/config->imgSize[PF_T];
	    if(ratio) /* S > T */
	    {
		if(config->minICache[PF_S] >= 0) /* s is set */
		    config->minICache[PF_T] = config->minICache[PF_S] / ratio;
		else if(config->minICache[PF_T] >= 0) /* t is set */
		    config->minICache[PF_S] = config->minICache[PF_T] * ratio;
		else /* neither are set */
		{
		    config->minICache[PF_S] = config->clipSize * ratio;
		    config->minICache[PF_T] = config->clipSize;
		}
	    }
	    else /* T > S */
	    {
		ratio = config->imgSize[PF_T]/config->imgSize[PF_S];
		if(config->minICache[PF_S] >= 0) /* s is set */
		    config->minICache[PF_T] = config->minICache[PF_S] * ratio;
		else if(config->minICache[PF_T] >= 0) /* t is set */
		    config->minICache[PF_S] = config->minICache[PF_T] / ratio;
		else /* neither are set */
		{
		    config->minICache[PF_S] = config->clipSize;
		    config->minICache[PF_T] = config->clipSize * ratio;
		}

	    }
	}
    }

    ratio[PF_S] = config->imgSize[PF_S]/config->minICache[PF_S];
    ratio[PF_T] = config->imgSize[PF_T]/config->minICache[PF_T];

    if(ratio[PF_S] != ratio[PF_T])
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadClipTexture: smallest_icache dimensions "
		 "aren't proportional to virt_size dimensions");
	return NULL;
    }

    if(ratio[PF_S] & (ratio[PF_S] - 1))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdLoadClipTexture: smallest_icache dimensions"
		 "aren't a power of two ratio of virt_size\n");
	return NULL;
    }


    /* Configure all the cliptexture's image caches */
    for(levelSizeS = config->imgSize[PF_S], levelSizeT = config->imgSize[PF_T],
	    level = 0;
	levelSizeS >= config->minICache[PF_S]; /* T's should match */
	level++, levelSizeS >>= 1, levelSizeT >>= 1)
    {
	ic = config->icFunc(ct, level, config);
	
	if(ic == NULL)
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfdLoadClipTexture: level %d image cache, size"
		     "(%d,%d), failed. Giving up on parsing cliptexture",
		     level, levelSizeS, levelSizeT);
	    return NULL;
	}    

	if(config->clipSize >= levelSizeS &&
	   config->clipSize >= levelSizeT)
	{
	    /*
	    ** XXX TODO check for the existence of tile file pointed to by 
	    ** pyramid image cache
	    */
	}

	/* ensure texture and texregion size is no larger than clip size */
	/* XXX TODO this should handle imgSize; check */

	pfTexImage((pfTexture *)ct, NULL, config->components, 
		   config->clipSize,
		   config->clipSize,
		   1);

	pfImageCacheTexRegionSize(ic, 
				  PF_MIN2(config->clipSize, levelSizeS),
				  PF_MIN2(config->clipSize, levelSizeT),
				  1);
	pfImageCacheTexSize(ic,
			    PF_MIN2(config->clipSize, levelSizeS),
			    PF_MIN2(config->clipSize, levelSizeT),
			    1);

	pfClipTextureLevel(ct, level, ic);

    }
    
    /* Load Image Tiles, if not specified by config file */
    

    for (levelSizeS = config->minICache[PF_S]/2,
	     levelSizeT = config->minICache[PF_T]/2;
	 levelSizeS && levelSizeT;
	 level++, levelSizeS >>= 1, levelSizeT >>= 1)
    {
	it = config->tileFunc(ct, level, config);
	if(!it)
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfuMakeClipTexture: "
		     "image tile configuration of level %d failed; aborting",
		     level);
	    return NULL;
	}

	pfImageTileMemImageFormat(it, config->imgFormat);
	pfImageTileFileImageFormat(it, config->imgFormat);
	pfImageTileMemImageType(it, config->extFormat);
	pfImageTileFileImageType(it, config->extFormat);
	pfImageTileSize(it, levelSizeS, levelSizeT, 1);
	pfImageTileHeaderOffset(it, config->hdrOffset);
	pfClipTextureLevel(ct, level, it);
	pfLoadImageTile(it);
    }
    return ct;
}

PFUDLLEXPORT void
pfuInitClipTexConfig(pfuClipTexConfig *config)
{
    config->name = NULL;

    /* illegal values to see if they were filled in */

    config->extFormat = -1;
    config->intFormat = -1;
    config->imgFormat = -1;
    config->components = -1;

    config->imgSize[PF_S] = -1;
    config->imgSize[PF_T] = -1;
    config->imgSize[PF_R] = -1;

    config->clipSize = -1;

    config->minICache[PF_S] = -1; /* needed if image caches in pyramid */
    config->minICache[PF_T] = -1;
    config->minICache[PF_R] = -1;

    config->tileSize[PF_S] = -1; /* only needed if auto configing icaches */
    config->tileSize[PF_T] = -1;
    config->tileSize[PF_R] = -1;

    /* reasonable default values */

    config->invBorder = 8;
    config->effLevels = 16;
    config->allocLevels = 1000;
    config->hdrOffset = 0;
    config->sStreams = NULL;
    config->tStreams = NULL;
    config->rStreams = NULL;

    config->icData = NULL;
    config->tileData = NULL;
    
    config->icFunc = NULL;
    config->tileFunc = NULL;

    config->pageSize = -1; /* "not initialized" value */

    config->readFunc = NULL; 
    config->lookahead = 1; /* default value; 1 tile border */
}

/* Don't try to free struct or data specific struct pointers */
PFUDLLEXPORT void
pfuFreeClipTexConfig(pfuClipTexConfig *config)
{
    int i;

    /* XXX Todo add default tile list */


    /* NOTE streams used for all icache levels */
    if(config->sStreams)
    {
	for(i = 0; i < pfGetNum(config->sStreams); i++)
	    free(pfGet(config->sStreams, i));
	pfDelete(config->sStreams);
    }
    if(config->tStreams)
    {
	for(i = 0; i < pfGetNum(config->tStreams); i++)
	    free(pfGet(config->tStreams, i));
	pfDelete(config->tStreams);
    }
    if(config->rStreams)
    {
	for(i = 0; i < pfGetNum(config->rStreams); i++)
	    free(pfGet(config->rStreams, i));
	pfDelete(config->rStreams);
    }
}


static int checkICache(pfuImgCacheConfig *state)
{


    /* check to see if mandatory values were filled in */

    if(state->extFormat < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: "
		 "mandatory field external format wasn't set or is invalid %d",
		 state->extFormat);
	return 1;
    }

    if(state->intFormat < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: "
		 "mandatory field internal format wasn't set or is invalid %d",
		 state->intFormat);
	return 1;
    }

    if(state->imgFormat < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: "
		 "mandatory field image format wasn't set or is invalid %d",
		 state->imgFormat);
	return 1;
    }

    if(state->components < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: "
		 "mandatory field number of components wasn't set "
		 "or is invalid %d",
		 state->components);
	return 1;
    }

    if(state->size[PF_S] < 0 ||
       state->size[PF_T] < 0 ||
       state->size[PF_R] < 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: "
		 "mandatory field image cache size wasn't set or is invalid "
		 "(%d %d %d)",
		 state->size[PF_S],
		 state->size[PF_T],
		 state->size[PF_R]);
	return 1;
    }

    if(state->size[PF_S] == 0 ||
       state->size[PF_T] == 0 ||
       state->size[PF_R] == 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: "
		 "one or more image cache size dimensions are zero "
		 "(%d %d %d)",
		 state->size[PF_S],
		 state->size[PF_T],
		 state->size[PF_R]);
	return 1;
    }

    if(state->size[PF_S] & (state->size[PF_S] - 1)  || 
       state->size[PF_T] & (state->size[PF_T] - 1)  || 
       state->size[PF_R] & (state->size[PF_R] - 1))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: image size (%d %d %d) "
		 "not a power of 2\n",
		 state->size[PF_S],
		 state->size[PF_T],
		 state->size[PF_R]);
	return 1; /* we failed */    
    }

    if(state->memRegSize[PF_S] == 0 ||
       state->memRegSize[PF_T] == 0 ||
       state->memRegSize[PF_R] == 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: "
		 "one or more dimensions of memregion size are zero "
		 "(%d %d %d)",
		 state->memRegSize[PF_S],
		 state->memRegSize[PF_T],
		 state->memRegSize[PF_R]);
	return 1; /* we failed */
    }

    /* if mem region size is provided, do checking */
    if(state->memRegSize[PF_S] > 0 ||
       state->memRegSize[PF_T] > 0 ||
       state->memRegSize[PF_R] > 0)
    {

	if(state->texRegSize[PF_S] > 
	   state->memRegSize[PF_S] * state->tileSize[PF_S] ||
	   state->texRegSize[PF_T] > 
	   state->memRegSize[PF_T] * state->tileSize[PF_T] ||
	   state->texRegSize[PF_R] > 
	   state->memRegSize[PF_R] * state->tileSize[PF_R])
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfuMakeImageCache: memregion too small for texregion\n");
	    return 1; /* we failed */
	}
    
	if((state->memRegSize[PF_S] < 2 && 
	    state->memRegSize[PF_S] * state->tileSize[PF_S] < 
	    state->size[PF_S]) ||
	   (state->memRegSize[PF_T] < 2 && 
	    state->memRegSize[PF_T] * state->tileSize[PF_T] <
	    state->size[PF_T]) ||
	    (state->memRegSize[PF_R] < 2 && 
	     state->memRegSize[PF_R] * state->tileSize[PF_R] <
	     state->size[PF_R]))
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfuMakeImageCache: "
		     "a 1 x 1 memregion must cover entire cache image\n");
	    return 1; /* we failed */
	}

	/*
	** Round texregion size to tile boundary, and assume we need two extra
	** tile sizes of border in the memregion.
	*/

	if(state->memRegSize[PF_S] * state->tileSize[PF_S] <
	   state->size[PF_S] &&
	   (state->texRegSize[PF_S] + state->tileSize[PF_S] - 1)/
	   state->tileSize[PF_S] + 2 > state->memRegSize[PF_S])
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfuMakeImageCache: "
		     "too few memregion S tiles to freely move texregion\n");

	if(state->memRegSize[PF_T] * state->tileSize[PF_T] <
	   state->size[PF_T] &&
	   (state->texRegSize[PF_T] + state->tileSize[PF_T] - 1)/
	   state->tileSize[PF_T] + 2 > state->memRegSize[PF_T])
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfuMakeImageCache: "
		     "too few memregion T tiles to freely move texregion\n");


	if(state->memRegSize[PF_R] * state->tileSize[PF_R] <
	   state->size[PF_R] &&
	   (state->texRegSize[PF_R] + state->tileSize[PF_R] - 1)/
	   state->tileSize[PF_R] + 2 > state->memRegSize[PF_R])
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "pfuMakeImageCache: "
		     "too few memregion R tiles to freely move texregion\n");
    
    }
    return NULL; /* ok if we made it to here */
}


PFUDLLEXPORT void
pfuInitImgCacheConfig(pfuImgCacheConfig *config)
{

    /* set illegal values to see if they were set by application */

    config->name = NULL;

    /* mandatory values */

    config->extFormat = -1;
    config->intFormat = -1;
    config->imgFormat = -1;
    config->components = -1;

    config->memRegSize[PF_S] = -1;
    config->memRegSize[PF_T] = -1;
    config->memRegSize[PF_R] = -1;

    config->texRegSize[PF_S] = -1;
    config->texRegSize[PF_T] = -1;
    config->texRegSize[PF_R] = -1;

    config->size[PF_S] = -1;
    config->size[PF_T] = -1;
    config->size[PF_R] = -1;

    config->tileSize[PF_S] = -1;
    config->tileSize[PF_T] = -1;
    config->tileSize[PF_R] = -1;

    /* optional values */

    config->texRegOrg[PF_S] = -1;
    config->texRegOrg[PF_T] = -1;
    config->texRegOrg[PF_R] = -1;

    config->memRegOrg[PF_S] = -1;
    config->memRegOrg[PF_T] = -1;
    config->memRegOrg[PF_R] = -1;


    config->args[0] = -1;

    config->texSize[PF_S] = -1;
    config->texSize[PF_T] = -1; 
    config->texSize[PF_R] = -1;

    config->tilesInFile[PF_S] = 0;
    config->tilesInFile[PF_T] = 0;
    config->tilesInFile[PF_R] = 0;

    /* reasonable defaults */

    config->header = 0;
    config->base = NULL; /* default icache base path/name is empty string */
    config->format = NULL;
    config->numParams = 0;
    config->defaultTile = NULL;
    config->sStreams = NULL;
    config->tStreams = NULL;
    config->rStreams = NULL;
    config->pageSize = -1; /* "not initialized" value */
    config->readFunc = NULL;
    config->lookahead = 1; /* default value; 1 tile border */
    config->clipSize = -1;
}

PFUDLLEXPORT void
pfuFreeImgCacheConfig(pfuImgCacheConfig *config)
{
    char *str;
    int i;

    if(config->base)
	free(config->base);
    if(config->format)
	free(config->format);
    if(config->defaultTile)
	free(config->defaultTile);

    if(config->sStreams)
    {
	for(i = 0; i < pfGetNum(config->sStreams); i++)
	    free(pfGet(config->sStreams, i));
	pfDelete(config->sStreams);
    }
    if(config->tStreams)
    {
	for(i = 0; i < pfGetNum(config->tStreams); i++)
	    free(pfGet(config->tStreams, i));
	pfDelete(config->tStreams);
    }
    if(config->rStreams)
    {
	for(i = 0; i < pfGetNum(config->rStreams); i++)
	    free(pfGet(config->rStreams, i));
	pfDelete(config->rStreams);
    }
}


/* Return 0 on failure */
PFUDLLEXPORT pfImageCache *
pfuMakeImageCache(pfTexture *texture, int level, pfuImgCacheConfig *state)
{
    int i;
    pfImageTile *proto;
    pfImageCache *ic;
    int prevNP = pfGetNumGlobalQueueServiceProcs();
    void *arena = pfGetSharedArena();

    if(checkICache(state))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: parameter checking failed");
	return NULL;
    }

    /* ensure a reasonable texregion size */
    if(state->texRegSize[PF_S] < 0 || /* not defined, or defined wrong */
       state->clipSize != -1 &&
       state->texRegSize[PF_S] != PF_MIN2(state->clipSize, state->size[PF_S]))
    {
       if(state->clipSize != -1)
	   state->texRegSize[PF_S] = 
	       PF_MIN2(state->clipSize, state->size[PF_S]);
       else 
       {
	   pfNotify(PFNFY_WARN, PFNFY_PRINT,
		    "pfuMakeImageCache(); you must either set the "
		    "tex region size properly or use this image cache "
		    "in a cliptexture to get a clipsize");
	   return NULL;
       }
    }
    if(state->texRegSize[PF_T] < 0 || /* not defined, or defined wrong */
       state->clipSize != -1 &&
       state->texRegSize[PF_T] != PF_MIN2(state->clipSize, state->size[PF_T]))
    {
       if(state->clipSize != -1)
	   state->texRegSize[PF_T] = 
	       PF_MIN2(state->clipSize, state->size[PF_T]);
       else 
       {
	   pfNotify(PFNFY_WARN, PFNFY_PRINT,
		    "pfuMakeImageCache(); you must either set the "
		    "tex region size properly or use this image cache "
		    "in a cliptexture to get a clipsize");
	   return NULL;
       }
    }
    if(state->texRegSize[PF_R] < 0 || /* not defined, or defined wrong */
       state->clipSize != -1 &&
       state->texRegSize[PF_R] != PF_MIN2(state->clipSize, state->size[PF_R]))
    {
       if(state->clipSize != -1)
	   state->texRegSize[PF_R] = 
	       PF_MIN2(state->clipSize, state->size[PF_R]);
       else 
       {
	   pfNotify(PFNFY_WARN, PFNFY_PRINT,
		    "pfuMakeImageCache(); you must either set the "
		    "tex region size properly or use this image cache "
		    "in a cliptexture to get a clipsize");
	   return NULL;
       }
    }


    ic = pfNewImageCache(arena);
    if(!ic)
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: couldn't create image cache");
    proto = pfNewImageTile(arena);
    if(!proto)
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: couldn't create proto tile");

    /* user-defined read function */
    if(state->readFunc)
	pfImageTileReadFunc(proto, state->readFunc);


    /* set the page size from the structure if it isn't -1 (the default) */
    if(state->pageSize >= 0)
    {
	int lock;
	pfGetImageTileMemInfo(proto, NULL, &lock);
	pfImageTileMemInfo(proto, state->pageSize, lock);
    }
    /*
    ** It would be nice to check if name is pointing to
    ** a valid string
    */
    if(state->name)
	pfImageCacheName(ic, state->name);

    /*
    ** /// XXX Should make setable parameter whether to 
    **default to parallel Reads or not 
    **
    ** Set proto tile to default parallel Read queue.
    */
    pfImageTileReadQueue(proto, pfGetGlobalReadQueue());



    /* image cache size */

    /* XXX check sizes (pow of 2) */

    /* if mem Region Size isn't given, default to reasonable values */

    if(state->memRegSize[PF_S] <= 0)
	if(state->tileSize[PF_S] == state->size[PF_S])
	    state->memRegSize[PF_S] = 1;
	else
	    state->memRegSize[PF_S] = 
		PF_MIN2((state->texRegSize[PF_S] + state->tileSize[PF_S] - 1) /
			state->tileSize[PF_S] + 1 + state->lookahead,
			(state->size[PF_S] + state->tileSize[PF_S] - 1)/
			state->tileSize[PF_S]);
	    
    if(state->memRegSize[PF_T] <= 0)
	if(state->tileSize[PF_T] == state->size[PF_T])
	    state->memRegSize[PF_T] = 1;
	else
	    state->memRegSize[PF_T] = 
		PF_MIN2((state->texRegSize[PF_T] + state->tileSize[PF_T] - 1) /
			state->tileSize[PF_T] + 1 + state->lookahead,
			(state->size[PF_T] + state->tileSize[PF_T] - 1)/
			state->tileSize[PF_T]);

    if(state->memRegSize[PF_R] <= 0)
	if(state->tileSize[PF_R] == state->size[PF_R])
	    state->memRegSize[PF_R] = 1;
	else
	    state->memRegSize[PF_R] = 
		PF_MIN2((state->texRegSize[PF_R] + state->tileSize[PF_R] - 1) /
			state->tileSize[PF_R] + 1 + state->lookahead,
			(state->size[PF_R] + state->tileSize[PF_R] - 1)/
			state->tileSize[PF_R]);
	    
    /*
    ** These values aren't set by the default parser, but can be set
    ** by the user of this routine if desired
    ** XXX Should check parameter values to see if they're in range
    */

    if(state->texRegOrg[PF_S] >= 0 &&
       state->texRegOrg[PF_T] >= 0 &&
       state->texRegOrg[PF_R] >= 0)
	pfImageCacheTexRegionOrg(ic, 
				 state->texRegOrg[PF_S], 
				 state->texRegOrg[PF_T],
				 state->texRegOrg[PF_R]);


    if(state->memRegOrg[PF_S] >= 0 &&
       state->memRegOrg[PF_T] >= 0 &&
       state->memRegOrg[PF_R] >= 0)
	pfImageCacheMemRegionOrg(ic, 
				 state->memRegOrg[PF_S], 
				 state->memRegOrg[PF_T],
				 state->memRegOrg[PF_R]);


    pfImageCacheImageSize(ic, 
			  state->size[PF_S],
			  state->size[PF_T],
			  state->size[PF_R]);

    pfImageCacheMemRegionSize(ic, 
			      state->memRegSize[PF_S], 
			      state->memRegSize[PF_T],
			      state->memRegSize[PF_R]);

    /* header tile offset (optional) */
    if(state->header)
	pfImageTileHeaderOffset(proto, state->header);

    /* tiles in file (optional) */
    /* XXX check tiles in file range */
    if(state->tilesInFile[PF_S] ||
       state->tilesInFile[PF_T] ||
       state->tilesInFile[PF_R])
	pfImageTileNumFileTiles(proto, 
				state->tilesInFile[PF_S],
				state->tilesInFile[PF_T],
				state->tilesInFile[PF_R]);


    /* tilesize */
    pfImageTileSize(proto,
		    state->tileSize[PF_S],
		    state->tileSize[PF_T],
		    state->tileSize[PF_R]);


    /* base name */

    /* XXX base name a valid string? */
    pfImageCacheName(ic, state->base);
    pfImageTileFileName(proto, state->base);

    /* file name format */
    /* XXX format a valid string? */
    /* chek number of params doesn't exceed max */
    if(state->format)
	pfImageCacheTileFileNameFormat(ic, 
				       state->format,
				       state->numParams,
				       state->args);


    /* use texsize for texture if texsize isn't defined */
    if(state->texSize[PF_S] == -1)
    {
	state->texSize[PF_S] = state->texRegSize[PF_S];
	state->texSize[PF_T] = state->texRegSize[PF_T];
	state->texSize[PF_R] = state->texRegSize[PF_R];
    }
    /* Make sure destination texture has power of two sized dimensions */
    if(state->texSize[PF_S] & (state->texSize[PF_S] - 1) ||
       state->texSize[PF_T] & (state->texSize[PF_T] - 1) ||
       state->texSize[PF_R] & (state->texSize[PF_R] - 1))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: destination texture size "
		 "isn't power of two in each dimension (%d, %d %d)",
		 state->texSize[PF_S],
		 state->texSize[PF_T],
		 state->texSize[PF_R]);
	return NULL; /* we failed */
    }


    pfTexImage(texture, NULL, state->components,
	       state->texSize[PF_S],
	       state->texSize[PF_T],
	       state->texSize[PF_R]);

    /* ext format */
    pfTexFormat(texture, PFTEX_SUBLOAD_FORMAT, PF_ON);

    pfTexFormat(texture, PFTEX_EXTERNAL_FORMAT, state->extFormat);
    pfImageTileFileImageType(proto, state->extFormat);
    pfImageTileMemImageType(proto, state->extFormat);

    /* int format */
    pfTexFormat(texture, PFTEX_INTERNAL_FORMAT, state->intFormat);

    /* img format */
    pfTexFormat(texture, PFTEX_IMAGE_FORMAT, state->imgFormat);
    pfImageTileFileImageFormat(proto, state->imgFormat);
    pfImageTileMemImageFormat(proto, state->imgFormat);

    pfImageCacheTexRegionSize(ic,
			      state->texRegSize[PF_S],
			      state->texRegSize[PF_T],
			      state->texRegSize[PF_R]);

    pfImageCacheTex(ic, texture, level, PFTLOAD_DEST_TEXTURE);
    pfImageCacheTexSize(ic, 
			state->texSize[PF_S],
			state->texSize[PF_T],
			state->texSize[PF_R]);


    if(state->texSize[PF_S] < state->texRegSize[PF_S] ||
       state->texSize[PF_T] < state->texRegSize[PF_T] ||
       state->texSize[PF_R] < state->texRegSize[PF_R])
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: destination texture size (%d %d %d) "
		 "is smaller than tex region size (%d %d %d)",
		 state->texSize[PF_S],
		 state->texSize[PF_T],
		 state->texSize[PF_R],
		 state->texRegSize[PF_S], 
		 state->texRegSize[PF_T], 
		 state->texRegSize[PF_R]);
	return NULL;
    }

#if 0
    /*
    ** Commenting this out, since I don't think there's a corrolation
    ** between tilesize and texsize. Will remove when certain -tomcat
    */
    if(state->tileSize[PF_S] == state->size[PF_S] &&
       state->texSize[PF_S] < state->tileSize[PF_S] ||
       state->tileSize[PF_T] == state->size[PF_T] &&
       state->texSize[PF_T] < state->tileSize[PF_T] ||
       state->tileSize[PF_R] == state->size[PF_R] &&
       state->texSize[PF_R] < state->tileSize[PF_R])
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfuMakeImageCache: destination texture size (%d %d %d) "
		 "is smaller than tile size (%d %d %d)",
		 state->texSize[PF_S],
		 state->texSize[PF_T],
		 state->texSize[PF_R],
		 state->tileSize[PF_S], 
		 state->tileSize[PF_T], 
		 state->tileSize[PF_R]);
	return NULL;
    }
#endif

    /* XXX check streams are valid pathnames */
    if(state->sStreams)
	for(i = 0; i <  pfGetNum(state->sStreams); i++)
	{
	    pfImageCacheFileStreamServer(ic, PFIMAGECACHE_S_DIMENSION,
					 i,
					 pfGet(state->sStreams, i));
	}

    if(state->tStreams)
	for(i = 0; i <  pfGetNum(state->tStreams); i++)
	{
	    pfImageCacheFileStreamServer(ic, PFIMAGECACHE_T_DIMENSION,
					 i,
					 pfGet(state->tStreams, i));
	}

    if(state->rStreams)
	for(i = 0; i <  pfGetNum(state->rStreams); i++)
	{
	    pfImageCacheFileStreamServer(ic, PFIMAGECACHE_R_DIMENSION,
					 i,
					 pfGet(state->rStreams, i));
	}



    /* XXX check if default tile is a valid file */
    if(state->defaultTile)
    {
	pfImageTile *defaultTile;
	char *tmp;
	int baselen;

	if(state->base)
	    baselen = strlen(state->base);
	else
	    baselen = 0;

	tmp = malloc(baselen + strlen(state->defaultTile) + 1);

	if(state->base)
	    strcpy(tmp, state->base);
	else
	    strcpy(tmp, "");

	strcat(tmp, state->defaultTile);
	defaultTile = pfNewImageTile(arena);
	pfCopy(defaultTile, proto);
	pfImageTileFileName(defaultTile, tmp);
	free(tmp);

	/* Read the default tile now */
	pfImageTileReadQueue(defaultTile, NULL);
	pfLoadImageTile(defaultTile);
	pfImageTileDefaultTile(proto, defaultTile);
    }

    pfImageCacheProtoTile(ic, proto);
    {
	int n = pfGetNumGlobalQueueServiceProcs();

	for(i = prevNP; i < n; i++)
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Added Process with PID %d to service read queue 0x%x",
		     pfGetGlobalQueueServiceProcPID(i),
		     pfGetGlobalQueueServiceProcQueue(i));
    }

    pfDelete(proto);
    return ic;
}
