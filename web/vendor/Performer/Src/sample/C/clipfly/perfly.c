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
 *	perfly.c -- The Performer database fly-thru application
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef sgi
#include <bstring.h>
#endif  /* sgi */
#include <math.h>
#include <ctype.h>

#include <Performer/pf.h>
#include <Performer/prmath.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>
#if 0 /* PERFLY */
#include <Performer/pfui.h>
#endif /* PERFLY */
#if 1 /* CUSTOM */
#include <Performer/pfuiD.h>
#endif /* PERFLY */


/* paolo: display cliptexture emulation roaming textures in overlay */
#include <Performer/pf.h>


#include "generic.h"
#include "perfly.h"
#include "gui.h"
#include "keybd.h"
#if 1 /* CUSTOM */
#include "custom.h"
#endif /* CUSTOM */

#include <GL/glu.h>

#define streq(a,b) (strcmp(a,b) == 0)
#define strsuffix(s, suf) \
	(strlen(s) >= strlen(suf) && streq(s + (strlen(s)-strlen(suf)), suf))
#if 1 /* CUSTOM */
#define log2(x) (log(x)*M_LOG2E)
static int intlog2(unsigned n) { return n>1 ? 1+intlog2(n/2) : 0; }

static void
hsv_to_rgb(float h, float s, float v, float *R, float *G, float *B)
{
    int segment;
    float *rgb[3], major, minor, middle, frac;

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

/*
 * Usability routines to help the pfClipTexture and pfMPClipTexture APIs...
 */

static int _pfGetClipTextureNumLevels(pfClipTexture *clipTex)
{
    int sizeS, sizeT, sizeR;
    pfGetClipTextureVirtualSize(clipTex, &sizeS, &sizeT, &sizeR);
    return 1 + intlog2(PF_MAX3(sizeS, sizeT, sizeR));
}
static void _pfMPClipTextureMinLOD(pfMPClipTexture *mpClipTex, float _minLOD)
{
    float maxLOD;
    pfGetMPClipTextureLODRange(mpClipTex, NULL, &maxLOD);
    pfMPClipTextureLODRange(mpClipTex, _minLOD, maxLOD);
}
static void _pfMPClipTextureMaxLOD(pfMPClipTexture *mpClipTex, float _maxLOD)
{
    float minLOD;
    pfGetMPClipTextureLODRange(mpClipTex, &minLOD, NULL);
    pfMPClipTextureLODRange(mpClipTex, minLOD, _maxLOD);
}
static void _pfMPClipTextureLODBiasS(pfMPClipTexture *mpClipTex, float _biasS)
{
    float biasT, biasR;
    pfGetMPClipTextureLODBias(mpClipTex, NULL, &biasT, &biasR);
    pfMPClipTextureLODBias(mpClipTex, _biasS, biasT, biasR);
}
static void _pfMPClipTextureLODBiasT(pfMPClipTexture *mpClipTex, float _biasT)
{
    float biasS, biasR;
    pfGetMPClipTextureLODBias(mpClipTex, &biasS, NULL, &biasR);
    pfMPClipTextureLODBias(mpClipTex, biasS, _biasT, biasR);
}
static void _pfMPClipTextureLODBiasR(pfMPClipTexture *mpClipTex, float _biasR)
{
    float biasS, biasT;
    pfGetMPClipTextureLODBias(mpClipTex, &biasS, &biasT, NULL);
    pfMPClipTextureLODBias(mpClipTex, biasS, biasT, _biasR);
}
static void _pfMPClipTextureLODBiasSLimit(pfMPClipTexture *mpClipTex, float _biasS_lo, float _biasS_hi)
{
    float biasT_lo, biasT_hi, biasR_lo, biasR_hi;
    pfGetMPClipTextureLODBiasLimit(mpClipTex, NULL, NULL, &biasT_lo, &biasT_hi, &biasR_lo, &biasR_hi);
    pfMPClipTextureLODBiasLimit(mpClipTex, _biasS_lo, _biasS_hi, biasT_lo, biasT_hi, biasR_lo, biasR_hi);
}
static void _pfMPClipTextureLODBiasTLimit(pfMPClipTexture *mpClipTex, float _biasT_lo, float _biasT_hi)
{
    float biasS_lo, biasS_hi, biasR_lo, biasR_hi;
    pfGetMPClipTextureLODBiasLimit(mpClipTex, &biasS_lo, &biasS_hi, NULL, NULL, &biasR_lo, &biasR_hi);
    pfMPClipTextureLODBiasLimit(mpClipTex, biasS_lo, biasS_hi, _biasT_lo, _biasT_hi, biasR_lo, biasR_hi);
}
static void _pfMPClipTextureLODBiasRLimit(pfMPClipTexture *mpClipTex, float _biasR_lo, float _biasR_hi)
{
    float biasS_lo, biasS_hi, biasT_lo, biasT_hi;
    pfGetMPClipTextureLODBiasLimit(mpClipTex, &biasS_lo, &biasS_hi, &biasT_lo, &biasT_hi, NULL, NULL);
    pfMPClipTextureLODBiasLimit(mpClipTex, biasS_lo, biasS_hi, biasT_lo, biasT_hi, _biasR_lo, _biasR_hi);
}
	    
#endif /* CUSTOM */

	/*---------------------- Globals ------------------------*/

int PackedAttrFormat = 0 /* check out PFGS_PA_C4UBN3ST2FV3F == 1 */;

	/*----------------------------------------------------*/

/*
 * No custom channel initialization 
 */
void
initChannel(pfChannel *chan)
{
pfCalligraphic *callig;
int pipe,channel;

    if (chan == ViewState->masterChan)
    {
	pfPipeVideoChannel *vChan = pfGetChanPVChan(chan);
	pfGetPVChanSize(vChan, &ViewState->vChanXSize, 
				&ViewState->vChanYSize);
	if (ViewState->dvrXSize < 0)
	{
	    ViewState->dvrXSize = ViewState->vChanXSize;
	    ViewState->dvrYSize = ViewState->vChanYSize;
	}
    }
    /* Is there a Calligraphic channel attached to this Channel ? */
    callig = pfGetChanCurCallig(chan);
    if (callig != NULL)
    {
	/* this will force SwapReady signal to be active even for a single pipe */
	pfChanCalligEnable(chan,1);
	/* force swap vme twice so calligraphic stays in sync with SwapWin */
	/* we do it more than twice if more than on channel is on that board */
	pfGetCalligChannel(callig,&pipe,&channel);
        pfCalligSwapVME(pipe);
	pfCalligSwapVME(pipe);
    }
/* to indicate that the cliptexture center is to be set by masterChannel. this has to be done after channels are initialized */
    {
        pfList *mpcliptextures = pfNewList(sizeof(pfMPClipTexture*), 1, pfGetSharedArena());
        pfuProcessClipCentersWithChannel((pfNode *)ViewState->sceneGroup, mpcliptextures, ViewState->masterChan);
	pfDelete(mpcliptextures);
    }
}

/*
 * Initialize ViewState shared memory structure
 */
void
initViewState(void)
{
    pfWindow	*win;
    int		i, iR=0;
    int 	tmp;
    int		queryvals[4];
static int querytoks[4] = {PFQFTR_MULTISAMPLE, PFQFTR_TAG_CLEAR, PFQFTR_TEXTURE, 0};

    /* window needed for faster OpenGL queries */
    win = pfOpenNewNoPortWin("Query", -1);

#ifdef WIN32
#define strncasecmp strnicmp
#endif
    iR = (strncasecmp(pfGetMachString(), "IR", 2) == 0);

    /* query machine features */
    pfMQueryFeature(querytoks, queryvals);

    pfCloseWin(win);
    pfDelete(win);

    ViewState->haveMultisample = queryvals[0];
    ViewState->haveTagClear = queryvals[1];
    tmp = queryvals[2];


    if (tmp == PFQFTR_FAST)
	ViewState->texture = TRUE;
    else
	ViewState->texture = FALSE;

#if 1 /* CUSTOM */
    ViewState->tlut = FALSE;
#endif /* CUSTOM */

    /* Set up defaults */
    ViewState->aa		= ViewState->haveMultisample;
    ViewState->fade		= ViewState->haveMultisample;
    for (i = 0; i < MAX_PIPES; ++i)
	ViewState->visualIDs[i]	= -1;

    ViewState->exitFlag		= FALSE;
    ViewState->updateChannels	= TRUE;
    ViewState->gui		= TRUE;
    ViewState->guiFormat	= 0; /* marks unset */
#if 0 /* PERFLY */
    ViewState->rtView		= TRUE;
#endif /* PERFLY */
#if 1 /* CUSTOM */
    ViewState->rtView		      = FALSE; /* important! see README */
    ViewState->monitor_icaches        = FALSE;
    ViewState->magnify_icache_monitor = FALSE;
#endif /* CUSTOM */
    ViewState->phase		= PFPHASE_FREE_RUN;
    ViewState->frameRate	= 72.0f;
    ViewState->backface		= PF_ON;
#if 0 /* PERFLY */
    ViewState->collideMode	= PF_ON;
#endif /* PERFLY */
#if 1 /* CUSTOM */
    ViewState->collideMode	= PF_OFF;
#endif /* CUSTOM */
    ViewState->earthSkyMode	= PFES_FAST;
#ifdef	PERFORMER_1_2_BACKGROUND_COLOR
    pfSetVec4(ViewState->earthSkyColor, 0.5f, 0.5f, 0.8f, 1.0f);
#else
    pfSetVec4(ViewState->earthSkyColor, 0.5f*0.14f, 0.5f*0.22f, 0.5f*0.36f, 1.0f);
#endif
    pfSetVec4(ViewState->scribeColor, 1.0f, 1.0f, 1.0f, 1.0f);
    ViewState->fadeRange	= 15.0f;
    ViewState->nearPlane	= 0.0f;
    ViewState->farPlane		= 0.0f;
    ViewState->nearFogRange	= ViewState->nearPlane;
    ViewState->farFogRange	= ViewState->farPlane;
    ViewState->fog		= PFFOG_OFF;
    ViewState->timeOfDay	= 0.8f;
    ViewState->highLoad		= 0.8f;
    ViewState->lighting		= LIGHTING_EYE;
    ViewState->lowLoad		= 0.5f;
    ViewState->stats		= PF_OFF;
    ViewState->statsEnable	= 0;
    ViewState->stress		= PF_OFF;
    ViewState->stressMax	= 100.0f;
    ViewState->stressScale	= 1.0f;
    ViewState->tree		= 0;
    ViewState->drawStyle	= PFUSTYLE_FILLED;
    ViewState->xformerModel	= PFITDF_TRACKBALL;
    ViewState->resetPosition	= POS_ORIG;
#ifdef __linux__
    pfNotify(PFNFY_WARN, PFNFY_PRINT, 
        "Forcing Input model to NOFORK_X [%s:%d]\n", __FILE__, __LINE__);
    ViewState->input		= PFUINPUT_NOFORK_X;
#else
    ViewState->input		= PFUINPUT_X;
#endif /* linux */
    ViewState->explode		= 0.0f;
    for (i=0; i < MAX_PIPES; i++)
    {
	ViewState->drawFlags[i] = REDRAW_WINDOW;
	ViewState->firstPipeDraw[i] = 1;
    }
    ViewState->redrawOverlay	= 1;
    ViewState->redrawCount	= 0;
    ViewState->procLock		= 0;
    ViewState->cullMode		= PFCULL_ALL;
#if 1 /* CUSTOM */
if (getenv("_PFCT_SMART_CULLBACKS"))
ViewState->cullMode		= 0;
#endif /* CUSTOM */
    ViewState->LODscale		= 1.0f;
    ViewState->lamps		= 0;
#if 1 /* CUSTOM */
    ViewState->biasSMode = ViewState->biasTMode = ViewState->biasRMode=
    ViewState->minLODMode = ViewState->maxLODMode = 
	ViewState->elevelsMode = ViewState->clipOffsetMode = CLIP_CTRLMODE_AUTO;
    ViewState->mip_LODBiasS	= 0.0f;
    ViewState->mip_LODBiasT	= 0.0f;
    ViewState->mip_LODBiasR	= 0.0f;
    ViewState->clip_totalDTRTimeWasSpecified = 0;
#endif /* CUSTOM */
    strcpy(ViewState->welcomeText, "OpenGL\nPerformer");
    strcpy(ViewState->overlayText, "OpenGL Performer");
    ViewState->objFontType 	= PFDFONT_EXTRUDED;
    ViewState->objFontName      = strdup("Mistr");
    ViewState->optimizeGStates  = TRUE;    
    ViewState->optimizeTree     = 0;

    pfSetVec3(ViewState->panScale, 0.0f, 0.0f, 1.0f); 

    ViewState->rotateCenter	= 1;
    ViewState->iterate		= 1;

    ViewState->combineBillboards= 0;

    ViewState->printStats	= FALSE;
    ViewState->exitCount	= -1;

    ViewState->drawMode         = (iR ? DLIST_GEOSET : DLIST_OFF);
    ViewState->demoMode         = PF_OFF;
    ViewState->recordPath	= PF_OFF;
    ViewState->democnt		= 0;
    ViewState->doDemoPath	= 0;
    ViewState->snapImage        = 0;

    ViewState->doDVR		= DVR_OFF;
    ViewState->showDVR		= 0;
    ViewState->winXSize		= WinSizeX;
    ViewState->winYSize		= WinSizeY;
    ViewState->vChanXSize	= 1280; /* defaults - updated in initChan */
    ViewState->vChanYSize	= 1024;
    ViewState->dvrXSize		= -1;
    ViewState->dvrYSize		= -1;
    ViewState->MCO		= 0; /* map channels to video channels */

    ViewState->gridify		= FALSE;
    ViewState->gridified	= FALSE;

#if 1 /* CUSTOM */

    ViewState->drawCteTex	= FALSE;
    

    ViewState->invalidate	= FALSE;
    ViewState->invalidateFrame	= 0;
    ViewState->invalidateBarrier = pfuBarrierCreate(pfGetSemaArena());
#ifndef __linux__
    PFASSERTALWAYS(ViewState->invalidateBarrier != NULL);
#endif
#endif /* CUSTOM */

    memset(&(ViewState->events), 0, sizeof(ViewState->events));
    memset(&(ViewState->guiWidgets), 0, sizeof(ViewState->guiWidgets));
     
    pfMakeIdentMat(ViewState->viewMat);
    strcpy(ViewState->initViewNames[0], "Default");
    strcpy(ViewState->tetherViewNames[0], "Under");
    pfMakeIdentMat(ViewState->tetherViews[0]);

    /* special light points */
    ViewState->startCallig = 0;
    ViewState->calligDefocus = 0;
    ViewState->rasterDefocus = 0;
    ViewState->calligDrawTime = 600;
    ViewState->calligFilterSizeX = 1;
    ViewState->calligFilterSizeY = 1;
    ViewState->calligDirty = 1;
    ViewState->calligZFootPrint = 2.;
#if 1 /* CUSTOM  */
#ifdef DEBUG_DTR
    ViewState->DTRrecordreset = 1;
    ViewState->DTRrecorddump = 0;
#endif /* DEBUG_DTR */
#endif /* CUSTOM */
}


/* Initialize Calligraphic boards */
int
StartCalligraphic(int _numpipes)
{
int     found = 0;
int     i;
int     test;

    /* assume pipe starts a 0 to */
    for (i=0; i< _numpipes; i++)
    {
        if (pfCalligInitBoard(i) == TRUE)
        {
            found ++;
            /* get the memory size */
            pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
                "StartCalligraphic: board(%d) has %d Bytes of memory",
                i, pfGetCalligBoardMemSize(i));
        }
    }
    if (found)
    {
	pfQueryFeature(PFQFTR_LIGHTPOINT,&test);
	if (test != PFQFTR_FAST)
	  pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	      "StartCalligraphic: OpenGL does not have the lpoint_parameter"
	      " extension, VISI bus information will not be correct");
    }
      
    return found;
}

/* called before pfConfig() */ 

void
initConfig(void)
{
int numCalli;

    if (ViewState->startCallig)
    {
	/* lets start the calligraphic boards if any */
	numCalli = StartCalligraphic(NumPipes);
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
	    "InitConfig() - found %d calligraphic board(s)", numCalli);
	if (numCalli)
	    ViewState->calligBoard = 1;
	else
	    ViewState->calligBoard = 0;
    }
}

/* some of our favorite town textures and their detail partners */
pfuDetailInfo detailinfo[] = {
    {
	"road.rgb", "roaddet.rgb", -2, 0, 1,
	{
	    {  0.00, 0.00  },
	    { -1.33, 0.50  },
	    { -2.66, 0.80  },
	    { -4.00, 1.00  }
	},
    },
    {
	"4way.rgb", "4waydet.rgb", -4, 0, 1,
	{
	    {  0.00, 0.00  },
	    { -1.33, 0.20  },
	    { -2.66, 0.80  },
	    { -4.00, 1.00  }
	},
    },
    {
	"facade7.rgb", "facade7det.rgb", -2, 0, 1,
	{
	    {  0.00, 0.00  },
	    { -1.33, 0.70  },
	    { -2.66, 0.90  },
	    { -4.00, 1.00  }
	},
    },
    {
	"facade8.rgb", "facade8det.rgb", -2, 0, 1,
	{
	    {  0.00, 0.00  },
	    { -1.33, 0.50  },
	    { -2.66, 0.70  },
	    { -4.00, 1.00  }
	},
    },
};


/*
 * Initialize the view
 */
void
initView(pfScene *scene)
{
    pfSphere	bsphere;

#if 0
    pfuLoadDetailTextures(ViewState->texList, detailinfo,
		       sizeof(detailinfo)/sizeof(detailinfo[0]));
#endif

    /* If asked to run for zero frames, exit now */    
    if (ViewState->exitCount == 0)
	pfExit();

    pfGetNodeBSphere(scene, &bsphere);

    /* Set initial view position from command line values */
    if (InitXYZ)
        PFCOPY_VEC3(ViewState->viewCoord.xyz, ViewState->initViews[0].xyz);

    /* Otherwise fit view to encompass scene */
    else
    {
        PFCOPY_VEC3(ViewState->viewCoord.xyz, bsphere.center);

        /* Offset view so all visible */
        ViewState->viewCoord.xyz[PF_Y] -= ViewState->sceneSize;

        /* Store initial XYZ position for future reset actions */
        PFCOPY_VEC3(ViewState->initViews[0].xyz, ViewState->viewCoord.xyz);
    }

    if (ViewState->nearPlane == 0.0f)
    {
	/* Calculate nearPlane, farPlane based on scene extent */
	ViewState->nearPlane = PF_MIN2(ViewState->sceneSize / 10.0f, 1.0f);
	ViewState->farPlane = PF_MAX2(ViewState->sceneSize * 2.0f, 1000.0f);
	ViewState->nearFogRange = ViewState->nearPlane;
	ViewState->farFogRange = ViewState->farPlane;
    }

    /* Set initial view angles from command line values ? */
    if (InitHPR)
        PFCOPY_VEC3(ViewState->viewCoord.hpr, ViewState->initViews[0].hpr);
    else
    {
	PFSET_VEC3(ViewState->viewCoord.hpr, 0.0f, 0.0f, 0.0f);

	/* Store initial HPR angles for future reset actions */
	PFCOPY_VEC3(ViewState->initViews[0].hpr, ViewState->viewCoord.hpr);
    }
}

#if 1 /* CUSTOM */
/*
 * Make the given MP clip texture list
 * be the list of all MP clip textures attached to all pipes.
 * If this results in a change in the list, return 1,
 * else return 0.
 */
static int syncMPClipTexturesList(pfList *list)
{
    int somethingChanged = 0;
    int iList, nList = pfGetNum(list);
    int iPipe, nPipes = pfGetMultipipe();

    iList = 0;
    for (iPipe = 0; iPipe < nPipes; ++iPipe)
    {
	pfPipe *pipe = pfGetPipe(iPipe);
	int iTex, nTexs = pfGetPipeNumMPClipTextures(pipe);
	for (iTex = 0; iTex < nTexs; ++iTex)
	{
	    pfMPClipTexture *MPClipTex = pfGetPipeMPClipTexture(pipe, iTex);
	    if (pfGetMPClipTextureMaster(MPClipTex))
		continue; /* only interested in masters */
	    if (iList >= nList
	     || (pfMPClipTexture *)pfGet(list, iList) != MPClipTex)
	    {
		somethingChanged = 1;
		pfSet(list, iList, pfGetPipeMPClipTexture(pipe, iTex));
		nList = PF_MAX2(nList, iList+1);
	    }
	    iList++;
	}
    }
    if (iList < nList)
    {
	somethingChanged = 1;
	do
	    pfRemoveIndex(list, --nList);
	while (iList < nList);
    }

    return somethingChanged;
}

/*
 XXX hack that uses knowledge of the implementation
 XXX of pfuGridifyClipTexture...
 XXX A clip texture is gridified iff its image tiles
 XXX have non-NULL user data.
 */
static int _pfuIsGridifiedClipTexture(pfClipTexture *clipTex)
{
    pfObject *coarsestLevel;
    pfImageTile *itile;
    PFASSERTALWAYS(clipTex != NULL);
    coarsestLevel = pfGetClipTextureLevel(clipTex,
				  _pfGetClipTextureNumLevels(clipTex) - 1);
    PFASSERTALWAYS(coarsestLevel != NULL);
    if (pfIsOfType(coarsestLevel, pfGetImageCacheClassType()))
    {
	int sizeS, sizeT, sizeR;
	pfGetImageCacheCurMemRegionSize((pfImageCache *)coarsestLevel,
					&sizeS, &sizeT, &sizeR);
	if (sizeS*sizeT*sizeR == 0)
	    return 0;
	itile = pfGetImageCacheTile((pfImageCache *)coarsestLevel,0,0,0);
	if (itile == NULL)
	    return 0;
    }
    else
	itile = (pfImageTile *)coarsestLevel;
    return pfGetUserData(itile) != NULL;
}


/* get values from the given MP clip texture into ViewState members... */
extern void
getMPClipTextureValues(pfMPClipTexture *MPClipTex)
{
    int sizeS, sizeT, sizeR;
    pfClipTexture *clipTex = pfGetMPClipTextureClipTexture(MPClipTex);
    PFASSERTALWAYS(clipTex != NULL);
    pfNotify(PFNFY_INFO, PFNFY_PRINT,
	     "Getting clip texture widget values from %s",
	     pfGetTexName((pfTexture *)clipTex));
    ViewState->clip_LODOffset = pfGetMPClipTextureVirtualLODOffset(MPClipTex);
    ViewState->clip_numEffectiveLevels = pfGetMPClipTextureNumEffectiveLevels(MPClipTex);
    pfGetMPClipTextureLODRange(MPClipTex, &ViewState->mip_minLOD, &ViewState->mip_maxLOD);
    ViewState->mip_magFilter = pfGetTexFilter((pfTexture *)clipTex, PFTEX_MAGFILTER);
    ViewState->mip_minFilter = pfGetTexFilter((pfTexture *)clipTex, PFTEX_MINFILTER);
    if (!ViewState->clip_totalDTRTimeWasSpecified)
	ViewState->clip_totalDTRTime = pfGetPipeTotalTexLoadTime(pfGetPipe(0));
    ViewState->clip_DTRTime = pfGetMPClipTextureTexLoadTime(MPClipTex);
    ViewState->clip_DTRBlurMargin = pfGetMPClipTextureDTRBlurMargin(MPClipTex);
    ViewState->clip_DTRMode = pfGetMPClipTextureDTRMode(MPClipTex);
    ViewState->clip_invalidBorder = pfGetMPClipTextureInvalidBorder(MPClipTex);
    /* XXX the following is not MP safe */
    ViewState->gridify =
    ViewState->gridified = _pfuIsGridifiedClipTexture(clipTex);
    ViewState->invalidate = 0;

    /* values to use for slider limits... */
    pfGetClipTextureVirtualSize(clipTex, &sizeS, &sizeT, &sizeR);
    ViewState->mip_coarsestLevel = intlog2(PF_MAX3(sizeS, sizeT, sizeR));
    ViewState->clip_clipSize = pfGetClipTextureClipSize(clipTex);

    /*
     * Try to deduce the slider mode variables
     * from the limits stored on the MP clip texture.
     */
    {
	int lodoff_lo, lodoff_hi, elevels_lo, elevels_hi;
	float minLOD_lo, minLOD_hi, maxLOD_lo, maxLOD_hi;
	float biasS_lo, biasS_hi, biasT_lo, biasT_hi, biasR_lo, biasR_hi;

	pfGetMPClipTextureLODOffsetLimit(MPClipTex, &lodoff_lo, &lodoff_hi);
	pfGetMPClipTextureNumEffectiveLevelsLimit(MPClipTex, &elevels_lo,
							     &elevels_hi);
	pfGetMPClipTextureMinLODLimit(MPClipTex, &minLOD_lo, &minLOD_hi);
	pfGetMPClipTextureMaxLODLimit(MPClipTex, &maxLOD_lo, &maxLOD_hi);
	pfGetMPClipTextureLODBiasLimit(MPClipTex, &biasS_lo, &biasS_hi,
						  &biasT_lo, &biasT_hi,
						  &biasR_lo, &biasR_hi);
	ViewState->clipOffsetMode = (lodoff_lo>-1000)*CLIP_CTRLMODE_GEQ
			          + (lodoff_hi< 1000)*CLIP_CTRLMODE_LEQ;
	ViewState->elevelsMode = (elevels_lo>-1000)*CLIP_CTRLMODE_GEQ
			       + (elevels_hi< 1000)*CLIP_CTRLMODE_LEQ;
	ViewState->minLODMode = (minLOD_lo>-1000)*CLIP_CTRLMODE_GEQ
			      + (minLOD_hi< 1000)*CLIP_CTRLMODE_LEQ;
	ViewState->maxLODMode = (maxLOD_lo>-1000)*CLIP_CTRLMODE_GEQ
			      + (maxLOD_hi< 1000)*CLIP_CTRLMODE_LEQ;
	ViewState->biasSMode = (biasS_lo>-1000)*CLIP_CTRLMODE_GEQ
			     + (biasS_hi< 1000)*CLIP_CTRLMODE_LEQ;
	ViewState->biasTMode = (biasT_lo>-1000)*CLIP_CTRLMODE_GEQ
			     + (biasT_hi< 1000)*CLIP_CTRLMODE_LEQ;
	ViewState->biasRMode = (biasR_lo>-1000)*CLIP_CTRLMODE_GEQ
			     + (biasR_hi< 1000)*CLIP_CTRLMODE_LEQ;
    }
}

/* set GUI values to the ViewState members... */
/* (this currently has the side effect of
   setting the values on all selected clip textures,
   so don't call this when ALL is selected) */
extern void
updateClipTextureGUI(void)
{
    updateWidget(GUI_CLIP_GRIDIFY, ViewState->gridify);

    updateWidget(GUI_CLIPOFFSET_MODE, ViewState->clipOffsetMode);
    updateWidget(GUI_ELEVELS_MODE, ViewState->elevelsMode);
    updateWidget(GUI_MINLOD_MODE, ViewState->minLODMode);
    updateWidget(GUI_MAXLOD_MODE, ViewState->maxLODMode);
    updateWidget(GUI_MIP_LOD_BIAS_S_MODE, ViewState->biasSMode);
    updateWidget(GUI_MIP_LOD_BIAS_T_MODE, ViewState->biasTMode);

    updateWidget(GUI_CLIP_LOD_OFFSET, ViewState->clip_LODOffset);
    updateWidget(GUI_CLIP_EFFECTIVE_LEVELS, ViewState->clip_numEffectiveLevels);
    updateWidget(GUI_MIP_MIN_LOD, ViewState->mip_minLOD);
    updateWidget(GUI_MIP_MAX_LOD, ViewState->mip_maxLOD);
    updateWidget(GUI_MIP_LOD_BIAS_S, ViewState->mip_LODBiasS);
    updateWidget(GUI_MIP_LOD_BIAS_T, ViewState->mip_LODBiasT);
    updateWidget(GUI_CLIP_INVALID_BORDER, ViewState->clip_invalidBorder);

#if 0 /* changing min filter on clip texture not supported */
    /* updateWidget(GUI_MIP_MINFILTER, ViewState->clip_minFilter); */
#endif /* 0 */
    updateWidget(GUI_MIP_MAGFILTER, ViewState->mip_magFilter);
    updateWidget(GUI_CLIP_DTRMODE, ViewState->clip_DTRMode);
    updateWidget(GUI_CLIP_DTR_TOTALTIME, ViewState->clip_totalDTRTime);
    updateWidget(GUI_CLIP_DTR_TIME, ViewState->clip_DTRTime);
    updateWidget(GUI_CLIP_DTR_BLURMARGIN, ViewState->clip_DTRBlurMargin);

#define SETRANGE(widNum, is_int, min, max, val) \
	pfuWidgetRange(ViewState->guiWidgets[widNum], is_int, min, max, val)

    SETRANGE(GUI_CLIP_LOD_OFFSET, 1, 0, 20, ViewState->clip_LODOffset);
    SETRANGE(GUI_CLIP_EFFECTIVE_LEVELS, 1, 1, 16, ViewState->clip_numEffectiveLevels);
    SETRANGE(GUI_MIP_MIN_LOD, 0, 0, ViewState->mip_coarsestLevel, ViewState->mip_minLOD);
    SETRANGE(GUI_MIP_MAX_LOD, 0, 0, ViewState->mip_coarsestLevel, ViewState->mip_maxLOD);
    SETRANGE(GUI_MIP_LOD_BIAS_S, 0, -10, 10, ViewState->mip_LODBiasS);
    SETRANGE(GUI_MIP_LOD_BIAS_T, 0, -10, 10, ViewState->mip_LODBiasT);
    SETRANGE(GUI_CLIP_INVALID_BORDER, 1, 0, ViewState->clip_clipSize, ViewState->clip_invalidBorder);
}

#endif /* CUSTOM */

int
initSceneGraph(pfScene *scene)
{
    pfNode     *root;
    int		i;
    int		j;
    int		loaded	= 0;
    pfSphere    bsphere;

    /* Create a DCS for TRACKBALL pfiXformer */
#if 0 /* PERFLY */
    ViewState->sceneDCS = pfNewDCS();
#endif /* PERFLY */
#if 1 /* CUSTOM */
    ViewState->sceneDCS = pfNewDoubleDCS();
#endif /* CUSTOM */
    ViewState->sceneGroup = pfNewGroup();
    pfAddChild(ViewState->sceneDCS, ViewState->sceneGroup);

#if 0 /* PERFLY */
    if (ViewState->xformerModel == PFITDF_TRACKBALL)
    {
	pfAddChild(scene, ViewState->sceneDCS);
    }
    else
	pfAddChild(scene, ViewState->sceneGroup);
#endif /* PERFLY */
#if 1 /* CUSTOM */
    /* in clipfly, ALWAYS use sceneDCS instead of channel view matrix
       because it allows double precision */
    pfAddChild(scene, ViewState->sceneDCS);
#endif /* CUSTOM */

    /* Load each of the files named on the command line */
    for (i = 0;	i < NumFiles; i++)
    {
	for (j = 0; j < ViewState->iterate; j++)
	{
	    if (strsuffix(DatabaseFiles[i], ".perfly")) {
		if (loadConfigFile(DatabaseFiles[i]))
		    loaded++;
		continue;
	    }

	    /* Load the database. create a hierarchy under node "root" */
	    root = pfdLoadFile(DatabaseFiles[i]);

	    if (root == NULL)
	    {
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
			 "WARNING: could not load \"%s\"", DatabaseFiles[i]);
		continue;
	    }
	    
	    /* explode the object (move it from the origin) */
	    if (ViewState->explode > 0.0f)
	    {
		float	 x;
		float	 y;
		float	 z;
		pfMatrix matrix;
		pfSCS	*scs;
		
#ifdef WIN32
#define random rand
#endif
		x = ViewState->explode*((float)(random()&0xffff)/65535.f - .5f);
		y = ViewState->explode*((float)(random()&0xffff)/65535.f - .5f);
		z = ViewState->explode*((float)(random()&0xffff)/65535.f - .5f);
		
		/* move the object by placing it beneath an SCS node */
		pfMakeTransMat(matrix, x, y, z);
		scs = pfNewSCS(matrix);
		pfAddChild(scs, root);
		
		/* go ahead and apply the SCS matrix to the geometry */
		pfFlatten(scs, 0);
		
		/* detach (transformed) object from SCS; discard SCS */
		pfRemoveChild(scs, root);
		pfDelete(scs);
	    }
	    
	    /* optimize the newly loaded hierarchy */
	    if (ViewState->optimizeTree > 0)
	    {
		if (ViewState->optimizeTree > 1)
		{
		    /* convert ordinary pfDCS nodes to pfSCS nodes */
		    root = pfdFreezeTransforms(root, NULL);
		}
		/* discard all redundant nodes and identity pfSCS nodes */
		pfFlatten(root, 0);
		
		/* deinstance and apply all pfSCS transformations */
		root = pfdCleanTree(root, NULL);
		
 		/* try a parition for intersections */
		if (ViewState->optimizeTree > 2)
		{
		    pfPartition *part = pfNewPart();

		    if (ViewState->optimizeTree == 3)
		    {
			/* use approx 100x100 partition */
			/* don't try to build a perfect fit */

			pfVec3 spacing;
			pfSphere sphere;
			pfGetNodeBSphere(root, &sphere);
			
			PFSET_VEC3(spacing, 
				   0.01f*sphere.radius, 
				   0.01f*sphere.radius,
				   0.01f*sphere.radius);
			pfPartAttr(part, PFPART_ORIGIN, sphere.center);
			pfPartAttr(part, PFPART_MIN_SPACING, spacing);
			pfPartAttr(part, PFPART_MAX_SPACING, spacing);
		    }

		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			     "Creating pfPartition for %s", 
			     DatabaseFiles[i]);
		    
		    if (pfGetNotifyLevel() >= PFNFY_DEBUG)
			pfPartVal(part, PFPART_DEBUG, 1);

		    pfAddChild(part, root);
		    pfBuildPart(part);
		    pfUpdatePart(part);
		    root = (pfNode *)part;
		}
	    }
	    
	    pfAddChild(ViewState->sceneGroup, root);
	    ++loaded;
	}
    }

    if (ViewState->doDemoPath)
    {
	FILE *pathfp;
	int d;
	
	ViewState->demoxyz = pfMalloc(100000*sizeof(*ViewState->demoxyz), pfGetSharedArena());
        ViewState->demohpr = pfMalloc(100000*sizeof(*ViewState->demohpr), pfGetSharedArena());
        if (pathfp = (FILE *)fopen(ViewState->demoPathFileName, "r"))
	{
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Reading demo path file %s", ViewState->demoPathFileName);
	    d = 0;
#if 0 /* PERFLY */
	    while(fscanf(pathfp, "%f %f %f %f %f %f",
#endif /* PERFLY */
#if 1 /* CLIPFLY */
	    while(fscanf(pathfp, "%lf %lf %lf %lf %lf %lf",
#endif /* CLIPFLY */
	    &ViewState->demoxyz[d][0], &ViewState->demoxyz[d][1], &ViewState->demoxyz[d][2],
	    &ViewState->demohpr[d][0], &ViewState->demohpr[d][1], &ViewState->demohpr[d][2]) != EOF)
		d++;
	    ViewState->democnt = d;
	    fclose(pathfp);
	}
	else
	{
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "Could not open demo path file %s", ViewState->demoPathFileName);
	}
    }

#if 0
    /* discard DSO-based loaders if any have been bound to executable */
    pfdFreeFileLoaders();
#endif

    /* 
     * optimize sharing of geostate structures and components. 
     * Wait until all files are loaded so sharing encompasses 
     * entire scene. 
     */
    if (loaded)
    {
        pfdMakeShared((pfNode *)ViewState->sceneGroup);

        /* optimize pfLayer nodes via "DISPLACE POLYGON" */
        pfdCombineLayers((pfNode *)ViewState->sceneGroup);

        /* combine sibling pfBillboard nodes */
        if (ViewState->combineBillboards)
	    pfdCombineBillboards((pfNode *)ViewState->sceneGroup, 
	        ViewState->combineBillboards);
    }

    /* write out nodes in sceneGroup */
    if (WriteFileDbg)
    {
        if (!strstr(WriteFileDbg, ".out") && pfdInitConverter(WriteFileDbg))
        {
            /* it's a valid extension, go for it */
            if (!pfdStoreFile((pfNode *)ViewState->sceneGroup, WriteFileDbg))
                pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
                         "No store function available for %s\n", WriteFileDbg);
        }
        else
        {
            /* write the file as ASCII */
            FILE *fp;
            if (fp = fopen(WriteFileDbg, "w"))
            {
                pfPrint(ViewState->sceneGroup, PFTRAV_SELF|PFTRAV_DESCEND, 
		    PFPRINT_VB_OFF, fp);
                fclose(fp);
            }
            else
                pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
                         "Could not open scene.out for debug printing.");
        }

	/* remember that file's already been written */
	WriteFileDbg = NULL;
    }

    /*
     * make an optimizing scene geostate that represents the
     * most common modes and attributes present in the scene
     * graph.
     */
    if (loaded && ViewState->optimizeGStates)
    {
	/* compute an optimum scene geostate */
	pfdMakeSharedScene(scene);
    }
    else
    {
	/* use default builder state as scene geostate */
	pfGeoState *gs;
	gs = pfNewGState(pfGetSharedArena());
	pfCopy(gs, (pfGeoState*)pfdGetDefaultGState());
	pfSceneGState(scene, gs);
    }

    /* Compute extent of scene */
    if (loaded)
    {
    	pfGetNodeBSphere(scene, &bsphere);
    	ViewState->sceneSize = PF_MIN2(2.5f*bsphere.radius, 80000.0f);
    }
    else
    	ViewState->sceneSize = 1000.0f;

    /* convert scene to vertex arrays */
    if (loaded && PackedAttrFormat)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Making Vertex Arrays.");
	/* don't delete any extra attribute arrays since they will be needed
	 * for wireframe mode
	 */
	pfuTravCreatePackedAttrs(scene, PackedAttrFormat, 0x0);
    }

    /* add light sources to the scene */
    for (i = 0; i < ViewState->lamps; i++)
    {
	pfLightSource *lamp = pfNewLSource();

	/* set direction of infinite light source */
	pfNormalizeVec3(ViewState->lampXYZ[i]);
	pfLSourcePos(lamp,
		     ViewState->lampXYZ[i][0],
		     ViewState->lampXYZ[i][1],
		     ViewState->lampXYZ[i][2],
		     0.0f);

	/* set light source color */
	pfLSourceColor(lamp,PFLT_DIFFUSE,
		       ViewState->lampRGB[i][0],
		       ViewState->lampRGB[i][1],
		       ViewState->lampRGB[i][2]);

	/* add lamp to scene */
	pfAddChild(scene, lamp);
    }

    /* Create geode/gset to draw box representing shrunken 
     * culling frustum.
     */
    {
	int		*lengths, i;
	pfGeoSet	*gset;
	pfGeoState	*gstate;
	pfVec3	*coords;
	pfVec4	*colors;
	ushort	*index;
	void	*arena = pfGetSharedArena();
	
	ViewState->cullVol = pfNewGeode();
	gset = pfNewGSet(arena);
	gstate = pfNewGState(arena);
	
	pfGSetPrimType(gset, PFGS_LINESTRIPS);
	pfGSetNumPrims(gset, 1);
	lengths = (int*)pfMalloc(sizeof(int), arena);
	lengths[0] = 5;
	pfGSetPrimLengths(gset, lengths);
	
	index = (ushort*)pfMalloc(sizeof(ushort)*5, arena);
	for (i=0; i<4; i++)
	    index[i] = i;
	index[i] = 0;
	coords = (pfVec3*)pfCalloc(4, sizeof(pfVec3), arena);
	pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, index);
	colors = (pfVec4*)pfMalloc(sizeof(pfVec4), arena);
	pfSetVec4(colors[0], 1.0f, 0.0f, 0.0f, 1.0f);
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_OVERALL, colors, index);
	
	pfMakeBasicGState(gstate);
	pfGSetGState(gset, gstate);
	pfAddGSet(ViewState->cullVol, gset);
	ViewState->cullVolDCS = pfNewDCS();
	pfNodeTravMask(ViewState->cullVolDCS, PFTRAV_CULL, 0, PFTRAV_SELF, PF_SET);
	pfAddChild(ViewState->cullVolDCS, ViewState->cullVol);
	ViewState->cullVolColors = colors;
    }

    /* Process all the cliptextures in the scene */

    {
#if 1 /* CUSTOM */
	int i, num = 0;
	/*
	    XXX I believe the following is just wrong--
	    in a common case, pipe 1 is the middle pipe
	    and it's the one with the gui, and it's the one
	    with the master channel (I think).
	    However, it's farPlane from clear what this is trying to do.
	 */
#endif /* CUSTOM */
	void *arena = pfGetSharedArena();
	pfPipe *masterPipe = pfGetPipe(0);

	pfList *mpcliptextures = pfNewList(sizeof(pfMPClipTexture*), 1, arena);
	pfuProcessClipCenters((pfNode *)scene, mpcliptextures);
	(void)pfuAddMPClipTexturesToPipes(mpcliptextures, masterPipe, NULL);
	pfDelete(mpcliptextures);

	/* set cliptexture incremental state to the visual channel - GUI is chan 0 */
	for(i = pfGetMultipipe() - 1; i >= 0; i--)
	{
	    pfPipe *p = pfGetPipe(i);
	    if(p == masterPipe)
		pfPipeIncrementalStateChanNum(masterPipe, 1);
	    else
		pfPipeIncrementalStateChanNum(p, 0);
	}
#if 1 /* CUSTOM */
	/*
	 * Make sure these are initialized even if there are no
	 * clip textures...
	 */
	ViewState->mip_minFilter = PFTEX_MIPMAP_TRILINEAR;
	ViewState->mip_magFilter = PFTEX_BILINEAR;

	/*
	 * Traverse the pipe list and compile a list of all MP clip textures
	 * found.
	 */
	ViewState->MPClipTextures = pfNewList(sizeof(pfMPClipTexture*),1,arena);
	(void)syncMPClipTexturesList(ViewState->MPClipTextures);
	PFASSERTDEBUG(syncMPClipTexturesList(ViewState->MPClipTextures) == 0);
	num = ViewState->numMPClipTextures = pfGetNum(ViewState->MPClipTextures);

	/*
	 * If there are any clip textures in the scene, get values from
	 * the first one.
	 */
	if (num > 0)
	    getMPClipTextureValues(
		(pfMPClipTexture *)pfGet(ViewState->MPClipTextures, 0));

	/* set up the GUI menu structures for the clip textures */
	ViewState->clipTexStrs = (pfuGUIString *) pfMalloc(sizeof(pfuGUIString)*(num+1), arena);
	strcpy(ViewState->clipTexStrs[0], "(ALL)");
	for (i=0; i < num; i++)
	{
	    const char *name = pfGetTexName(
	      (pfTexture*) pfGetMPClipTextureClipTexture(
			(pfMPClipTexture*)pfGet(ViewState->MPClipTextures, i)));
	    strcpy(ViewState->clipTexStrs[i+1], name);
	}

	/*
	 * If per-pipe totalDTRTime was specified on the command line,
	 * the individual MP clip textures' times should be set to -1
	 * so that they will defer to the pipe's time.
	 * In general this does something random; we only
	 * care about startup (first couple of frames)
	 * when the user hasn't changed the selection from ALL
	 * so the -1 will get propagated to all the MP clip textures.
	 */
	if (ViewState->clip_totalDTRTimeWasSpecified)
	{
	    ViewState->clip_DTRTime = -1; /* so it will defer to pipe time */
	    ViewState->clip_DTRTime_dirty = 1;
	    ViewState->clip_anyParams_dirty = 1;
	}
	    
#endif /* CUSTOM */
    }
    return(loaded);
}

/*
 * Pre-Frame latency critical real-time operations
*/
/* ARGSUSED (suppress compiler warn) */
void
localPreFrame(pfChannel *chan)
{
    
    /* do video resize for all channels */
    if (pfGetFrameCount() > 3)
    {
	int dvrMode;
	int i, newSize, newMode;
	int oxs,  oys;
	
	dvrMode = ViewState->doDVR;
	
	for (i=0; i < ViewState->numChans; i++)
	{
	    pfChannel *chan;
	    pfPipeVideoChannel *pvchan;
	    
	    chan = ViewState->chan[i];
	    pvchan = pfGetChanPVChan(chan);

	    if (pvchan)
	    {

		newMode = (pfGetPVChanDVRMode(pvchan) != dvrMode);
		if (newMode)
		{
		    /* must also set REDRAW_WINDOW for each pipe with redrawOverlay */
		    ViewState->redrawOverlay	= 1;
		    ViewState->redrawCount	= pfGetPipeDrawCount(0);
		}

		switch (dvrMode)
		{
		case DVR_OFF:
		    if (newMode)
		    {
			int pipeId = pfGetId(pfGetChanPipe(chan));
			ViewState->drawFlags[pipeId] |= REDRAW_WINDOW;
			pfNotify(PFNFY_NOTICE,PFNFY_PRINT, "DVR_OFF");
			pfPVChanDVRMode(pvchan, PFPVC_DVR_OFF);
			pfuCursorType(PFU_CURSOR_X);
			pfChanProjMode(chan,PFCHAN_PROJ_VIEWPORT);
			if (ViewState->gui && (chan == ViewState->masterChan))
				pfuRedrawGUI();
		    }
		    break;
		case DVR_MANUAL:
		    /* set pfPipeVideoChannel scale factor for next draw */
		    {
			if (newMode)
			{
			    pfNotify(PFNFY_NOTICE,PFNFY_PRINT, "DVR_MANUAL");
			    pfPVChanDVRMode(pvchan, dvrMode);
			    /*pfPVChanStressFilter(pvchan,1.0f, 0.80, 0.95f, 0.0f, 1.0f, 1.0f);*/
			    pfuCursorType(PFU_CURSOR_2D);
			}
			pfGetPVChanOutputSize(pvchan, &oxs, &oys);
			newSize =  (newMode || (oxs != ViewState->dvrXSize) || (oys != ViewState->dvrYSize));
			if (newSize)
			{
			    int pipeId = pfGetId(pfGetChanPipe(chan));
			    ViewState->drawFlags[pipeId] |= REDRAW_WINDOW;
			    pfNotify(PFNFY_INFO,PFNFY_PRINT,"MANUAL RESIZE pipe %d: %d %d", 
				pfGetId(pfGetChanPipe(chan)),
				ViewState->dvrXSize, ViewState->dvrYSize);
			    pfPVChanOutputSize(pvchan, ViewState->dvrXSize, ViewState->dvrYSize);
			    pfChanProjMode(chan,PFCHAN_PROJ_WINDOW);
			}
			if (ViewState->gui && (chan == ViewState->masterChan) && 
			    (newSize || pfuInGUI(ViewState->mouse.xpos, ViewState->mouse.ypos)))
			{
			    pfuRedrawGUI();
			}
		    }
		    break;
		case DVR_AUTO:
		    {
			if (newMode)
			{
			    float frac=1.0f;
			    int pipeId = pfGetId(pfGetChanPipe(chan));
			    ViewState->drawFlags[pipeId] |= REDRAW_WINDOW;
			    pfNotify(PFNFY_NOTICE,PFNFY_PRINT, "DVR_AUTO");
			    if (!Multipipe)
				frac = 1.0f / (float) NumChans;
			    pfPVChanDVRMode(pvchan, dvrMode);
			    pfPVChanStressFilter(pvchan, frac, 0.80, 0.95f, 0.0f, 1.0f, 10.0f);
			    pfuCursorType(PFU_CURSOR_2D);
			    pfChanProjMode(chan,PFCHAN_PROJ_WINDOW);
			}
			/* indicates that pfPipe should calc stress automatically */
			if (ViewState->gui && (chan == ViewState->masterChan))
			    pfuRedrawGUI();
		    }
		    break;
		}
		pfGetPVChanOutputSize(pvchan, &ViewState->dvrXSize, &ViewState->dvrYSize);
	    }
	}
    }
    
}

void
updatePath(void)
{
    static FILE *fp = NULL;

    if (!ViewState->demoMode && !ViewState->recordPath)
	return;

    if (ViewState->doDemoPath && ViewState->democnt && (ViewState->demoMode))
    {
        static int cnt=0;

        if(cnt >= ViewState->democnt)
	{
            cnt = 0;
	    if (ViewState->demoMode == 2)
		ViewState->demoMode = 0;
	}
        pfChanView(ViewState->masterChan, ViewState->demoxyz[cnt], ViewState->demohpr[cnt]);
        cnt++;
    }
    else if (ViewState->recordPath)
    {
	pfVec3 xyz, hpr;
	
	if(fp == NULL)
	{
	    fp = (FILE *)fopen("perfly.path", "w");
	}
	else if (fp && (ViewState->recordPath == 2)) /* creating new path */
	{
	    fclose(fp);
	    fp = (FILE *)fopen("perfly.path", "w");
	    ViewState->recordPath = 1;
	}
	
	if (fp)
	{
    	    pfGetChanView(ViewState->masterChan, xyz, hpr);
    	    fprintf(fp, "%f %f %f %f %f %f\n", xyz[0], xyz[1], xyz[2],
        	hpr[0], hpr[1], hpr[2]);
	}
    }
    else if (fp && !ViewState->recordPath)
    {
	fclose(fp);
	fp = NULL;
    }
}

#if 1 /* CUSTOM */
/* debugging print */
/*
static void printMatrices(pfChannel *chan, const char *old_or_new)
{
    pfMatrix viewMat;
    pfMatrix4d dcsMatD, xformerMatD, xformerModelMatD;
    pfGetChanViewMat(chan, viewMat);
    pfGetDoubleDCSMat(ViewState->sceneDCS, dcsMatD);
    pfiGetXformerMat(ViewState->xformer, xformerMatD);
    pfiGetXformerModelMat(ViewState->xformer, xformerModelMatD);

    pfNotify(PFNFY_INFO, PFNFY_PRINT,
	     "   Frame %d:", pfGetFrameCount());
    pfNotify(PFNFY_INFO, PFNFY_MORE,
	    "    %s XformerD Model Matrix:\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n",
	    old_or_new,
	    xformerModelMatD[0][0], xformerModelMatD[0][1], xformerModelMatD[0][2], xformerModelMatD[0][3],
	    xformerModelMatD[1][0], xformerModelMatD[1][1], xformerModelMatD[1][2], xformerModelMatD[1][3],
	    xformerModelMatD[2][0], xformerModelMatD[2][1], xformerModelMatD[2][2], xformerModelMatD[2][3],
	    xformerModelMatD[3][0], xformerModelMatD[3][1], xformerModelMatD[3][2], xformerModelMatD[3][3]);
    pfNotify(PFNFY_INFO, PFNFY_MORE,
	    "    %s XformerD Matrix:\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n",
	    old_or_new,
	    xformerMatD[0][0], xformerMatD[0][1], xformerMatD[0][2], xformerMatD[0][3],
	    xformerMatD[1][0], xformerMatD[1][1], xformerMatD[1][2], xformerMatD[1][3],
	    xformerMatD[2][0], xformerMatD[2][1], xformerMatD[2][2], xformerMatD[2][3],
	    xformerMatD[3][0], xformerMatD[3][1], xformerMatD[3][2], xformerMatD[3][3]);
    pfNotify(PFNFY_INFO, PFNFY_MORE,
	    "    %s DoubleDCS Matrix:\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n"
	    "\t\t\t\t\t\t%.17g %.17g %.17g %.17g\n",
	    old_or_new,
	    dcsMatD[0][0], dcsMatD[0][1], dcsMatD[0][2], dcsMatD[0][3],
	    dcsMatD[1][0], dcsMatD[1][1], dcsMatD[1][2], dcsMatD[1][3],
	    dcsMatD[2][0], dcsMatD[2][1], dcsMatD[2][2], dcsMatD[2][3],
	    dcsMatD[3][0], dcsMatD[3][1], dcsMatD[3][2], dcsMatD[3][3]);
    pfNotify(PFNFY_INFO, PFNFY_MORE,
	    "    %s Channel Matrix:\n"
	    "\t\t\t\t\t\t%.9g %.9g %.9g %.9g\n"
	    "\t\t\t\t\t\t%.9g %.9g %.9g %.9g\n"
	    "\t\t\t\t\t\t%.9g %.9g %.9g %.9g\n"
	    "\t\t\t\t\t\t%.9g %.9g %.9g %.9g\n",
	    old_or_new,
	    viewMat[0][0], viewMat[0][1], viewMat[0][2], viewMat[0][3],
	    viewMat[1][0], viewMat[1][1], viewMat[1][2], viewMat[1][3],
	    viewMat[2][0], viewMat[2][1], viewMat[2][2], viewMat[2][3],
	    viewMat[3][0], viewMat[3][1], viewMat[3][2], viewMat[3][3]);
    printf("\n");
}
*/

void fixSceneDCSAndChanViewForPrecision(pfChannel *chan)
{
    /*
     * This function gets called after updating the xformer
     * or changing its motion model.
     * (It's not enough to just call this function after updating
     * the xformer, because changing the motion model
     * happens after that.)
     *
     * Single-precision X,Y,Z in the channel viewpoint
     * can destroy the double precision of the entire
     * chain, so we want to make sure the
     * channel view matrix is the identity and
     * all transformations (both the model matrix and the inv viewing matrix)
     * are always expressed in double precision in the scene DCS instead.
     * (Strictly speaking it would suffice to put
     * only the translate in the DCS and leave the rotate
     * in the channel viewmat, but it's simpler
     * to put the whole thing in the DCS.)
     *
     * In general, the transformer models
     * can't be told to modify the DCS and not the channel viewpoint
     * (only trackball can) so we take whatever the transformer did
     * and fix it here.
     *
     * The various transformer models have different (kind of random)
     * semantics which we have to take into account;
     * here's a summary (from observation and looking at the code)
     * of what happens during pfiUpdateXformer():

       Trackball:
	    Xformer's modelMat is modified.
		    If there is a DCS attached, the modelMat
		    value is copied into the DCS.
	    Xformer's viewMat (eye position and orientation)
		    is always left at whatever value
		    it was when trackball mode was entered.

	Fly and Drive (the subclasses of pfiInputXformTravel):
	    Xformer's modelMat is irrelevant (actually is set to viewMat).
	    Xformer's viewMat (eye position and orientation)
		    gets modified, and copied into the channel's
		    viewing matrix.
		    (Note that in this case, perfly also copies
		    the xformer's viewMat into the channel;
		    this is redundant.)

	Spheric and Geosphere (the new replacement for spheric):
	    Xformer's modelMat is irrelevant (actually is set to viewMat)
	    Xformer's viewMat (eye position and orientation)
		    gets modified, and copied into the channel's
		    viewing matrix.

	Tether:
	    XXX not sure, I may have broken this.

	Changing from Trackball to anything else:
	    The Xformer's modelMat is applied
	    in reverse to the Xformer's viewMat.
	    The DCS (if attached) matrix is set to the identity matrix.

	Changing from anything else to Trackball:
	    Xformer's modelMat and viewMat are not changed.

     *
     * So here's what we do do "fix" it in all cases:
     *
	If in trackball mode:
	    Set the DCS matrix to be the Xformer's model matrix
	    (which is the same as the current value of the DCS matrix)
	    times the inverse of the Xformer's view matrix.
	    Set the channel's view matrix to the identity (as always).
	Otherwise:
	    Set the DCS matrix to be
	    the inverse of the Xformer's viewing matrix.
	    Set the channel's view matrix to the identity (as always).
     *
     * Note we do not explicitly try to change the xformer's
     * model or view matrix; I tried that once
     * and it caused me a world of pain.
     */

#define XFORMER_MODELMAT_IS_MEANINGFUL(xformer) \
  pfIsOfType(pfiGetXformerCurModel(xformer), \
	     pfiGetIXformTrackballClassType())

    if (ViewState->xformerModel != PFITDF_TETHER)
    {
	pfMatrix4d xformerViewMatD, xformerInvViewMatD, dcsMatD;

	/* printMatrices(chan, "Old"); */

	pfiGetXformerMat(ViewState->xformer, xformerViewMatD);
	pfInvertOrthoNMat4d(xformerInvViewMatD, xformerViewMatD);

	if (XFORMER_MODELMAT_IS_MEANINGFUL(ViewState->xformer))
	{
	    /* set doubleDCS = modelMat times inverse of viewing matrix */
	    pfMatrix4d xformerModelMatD;
	    pfiGetXformerModelMat(ViewState->xformer, xformerModelMatD);
	    pfMultMat4d(dcsMatD, xformerModelMatD, xformerInvViewMatD);
	}
	else
	{
	    /* just set doubleDCS matrix to be inverse of viewing matrix */
	    PFCOPY_MAT(dcsMatD, xformerInvViewMatD);
	}
	pfDoubleDCSMat(ViewState->sceneDCS, dcsMatD);

	/* in all cases, set channel's view mat to identity */
	PFSET_VEC3(ViewState->viewCoord.xyz, 0, 0, 0);
	PFSET_VEC3(ViewState->viewCoord.hpr, 0, 0, 0);
	pfMakeIdentMat(ViewState->viewMat);
	pfChanViewMat(chan, ViewState->viewMat);

	/* printMatrices(chan, "New"); */

	if (ViewState->lighting ==  LIGHTING_EYE)
	{
	    pfDCSMat(ViewState->sunDCS, ViewState->viewMat); /* now identity */
	}
    }
    /* printMatrices(chan, "Current"); */
} /* end fixSceneDCSAndChanViewForPrecision() */
#endif /* CUSTOM */

/*
 * Update the current view 
*/
void
updateView(pfChannel *chan)
{
    pfMatrix 		mat;

    if (ViewState->resetPosition != POS_NORESET)
    {
	resetPosition(ViewState->resetPosition);
	ViewState->resetPosition = POS_NORESET;
    }

    if (ViewState->xformerModel == PFITDF_TETHER)
    {
	pfMatrix mat;
	pfGetDCSMat(ViewState->vehicleDCSs[ViewState->curVehicle], mat);
	/* adjust for pfuPath rotating the object CCW */
	pfPreRotMat(mat, -90., 0., 0., 1., mat);

	pfPreMultMat(mat,
		     ViewState->tetherViews[ViewState->curTetherView]);

#if 0 /* PERFLY */
	pfiXformerMat(ViewState->xformer, mat);
#endif /* PERFLY */
#if 1 /* CUSTOM */
	{
	pfMatrix4d mat4d;
	PFCOPY_MAT(mat4d, mat); /* XXX cop out */
	pfiXformerMat(ViewState->xformer, mat4d);
	}
#endif /* CUSTOM */
    }
    else
    {
	if (!ViewState->demoMode)
	{
	    pfiUpdateXformer(ViewState->xformer);
	    ViewState->xformerModel = pfiGetXformerCurModelIndex(ViewState->xformer);
	}
	updatePath();
    }

    /* if have moving-eyepoint motion model, update eyepoint */
    if (ViewState->xformerModel == PFITDF_TETHER
     || pfIsOfType(pfiGetXformerCurModel(ViewState->xformer), 
	    pfiGetIXformTravelClassType()))
    {

	if ((!ViewState->demoMode)) /* && (!(ViewState->xformerModel == PFITDF_TETHER))) */
	{
	    pfiGetXformerCoord(ViewState->xformer, &ViewState->viewCoord);
#if 0 /* PERFLY */
	    pfiGetXformerMat(ViewState->xformer, mat);
#endif /* PERFLY */
#if 1 /* CUSTOM */
	    {
	    pfMatrix4d mat4d;
	    pfiGetXformerMat(ViewState->xformer, mat4d);
	    PFCOPY_MAT(mat, mat4d); /* XXX cop out */
	    }
#endif /* CUSTOM */
#if 0 /* PERFLY */
	    pfChanViewMat(chan, mat);
#endif /* PERFLY */
	    pfCopyMat(ViewState->viewMat, mat);
	}
	else
	{
	    pfGetChanViewMat(chan, ViewState->viewMat);
	}
	
	/* move the sun to the eyepoint */
	if (ViewState->lighting ==  LIGHTING_EYE)
	    pfDCSMat(ViewState->sunDCS, ViewState->viewMat);
    }

#if 1 /* CUSTOM */
    fixSceneDCSAndChanViewForPrecision(chan);
#endif /* CUSTOM */
       
    {
   /*
     * Update box which represents shrunken culling frustum
     */
    static float	prevCullDelta = -1.0f;
    static pfFrustum	*frust = NULL;
    if (prevCullDelta != ViewState->cullDelta)
    {
	if (ViewState->cullDelta < 1.0f)
	{
	    pfVec3			*coords;
	    ushort			*foo;
	    int				i;
	    float			nearPlane, farPlane, fudge;

	    if (frust == NULL)
		frust = pfNewFrust(NULL);

	    pfGetGSetAttrLists(pfGetGSet(ViewState->cullVol, 0), 
			       PFGS_COORD3, (void**)&coords, &foo);

	    pfGetChanBaseFrust(ViewState->masterChan, frust);
	    pfGetFrustNearFar(frust, &nearPlane, &farPlane);
	    pfGetFrustNear(frust, coords[0], coords[1], 
			          coords[3], coords[2]); 
	    
	    /*
	     * Need a fudge factor to multiply the
	     * coords of the rectangle by
	     * so they are safely away from the real front clip plane.
	     *
	     * fudge-1 (the percentage increase) should be proportional
	     * to (farPlane/nearPlane); choose the constant so that in the default
	     * case (nearPlane=1, farPlane=2000) the increase is the same as adding .001
	     * (somewhat arbitrarily; actually it would be more appropriate
	     * to choose this constant based on the z depth...)
	     *
	     * I.e. (fudge-1) = C * (farPlane/nearPlane)
	     *	         .001 = C * 2000/1
	     *              C = 5e-7.
	     * So fudge = 1 + 5e-7 * farPlane/nearPlane;
	     * Multiply the x and z coords by this same scale factor
	     * so that the vertices will end up at the
	     * exact same position on the screen that
	     * they would have otherwise.
	     */
	    fudge = 1.0f + 5e-7f*farPlane/nearPlane;

	    for (i=0; i<4; i++)
	    {
		coords[i][0] *= ViewState->cullDelta * fudge;
		coords[i][1] *= fudge;
		coords[i][2] *= ViewState->cullDelta * fudge;
	    }

	    pfFrustNearFar(frust, nearPlane*fudge, farPlane);
	    pfMakePerspFrust(frust, coords[0][0], coords[2][0], 
			            coords[0][2], coords[2][2]); 

	    pfChanCullPtope(ViewState->masterChan, (pfPolytope*)frust);

	    if (prevCullDelta >= 1.0f)
		pfAddChild(ViewState->scene, ViewState->cullVolDCS);
	}
	else	
	{
	    /* Don't draw culling box */
	    if (prevCullDelta < 1.0f)
		pfRemoveChild(ViewState->scene, ViewState->cullVolDCS);

	    /* Disable special cull region */
	    pfChanCullPtope(ViewState->masterChan, NULL);
	}

	prevCullDelta = ViewState->cullDelta;
    }

    /* Place cull volume geometry in front of viewer */
    if (ViewState->cullDelta < 1.0f)	
    {
	pfMatrix 		mat;
	pfGetChanOffsetViewMat(chan, mat);
	pfDCSMat(ViewState->cullVolDCS, mat);

#if 1 /* CUSTOM */
	/*
	 * XXX Should probably not be in the latency-critical part?
	 * XXX since the bounding-sphere calculation can be
	 * XXX time-consuming.
	 *
	 * XXX The following depends on frust, which is
	 * XXX only set if using reduced culling FOV...
	 * XXX keep that in mind if moving this block of code out.
	 */

	{
	    /*
	     * This value should be
	     *	  min(ranging over all geometry) max(ds/d(x,y,z), dt/d(x,y,z)).
	     * For the earth at radius r with a square texture,
	     * it will be 1/(2*pi*r) (the value at the equator).
	     */
	    static float min_dst_dxyz = -2.0f; /* uninitialized */
	    if (min_dst_dxyz == -2.)
	    {
		char *e = getenv("_PERFLY_AUTO_CLIP_MIN_LOD_DST_DXYZ");
		min_dst_dxyz = (e ? *e ? atof(e) : 1.0f : -1.0f);
	    }
	    if (min_dst_dxyz >= 0)
	    {
		int viewportWidth, viewportHeight;
		pfSphere bsph;
		float heightAboveTerrain;
		int texture_size;
		float max_possible_miplevel_size;
		float minLOD;

		pfGetChanSize(chan, &viewportWidth, &viewportHeight);
		viewportWidth *= ViewState->cullDelta;
		viewportHeight *= ViewState->cullDelta;

 /* XXX hack */ pfRemoveChild(ViewState->scene, ViewState->cullVolDCS);
		pfGetNodeBSphere(ViewState->scene, &bsph);
 /* XXX hack */ pfAddChild(ViewState->scene, ViewState->cullVolDCS);

		if (ViewState->MPClipTexture != NULL)
		{
		    int w,h,d;
		    pfGetClipTextureVirtualSize(
			pfGetMPClipTextureClipTexture(ViewState->MPClipTexture),
			&w,&h,&d);
		    texture_size = PF_MAX2(w,h);
		}
		else
		    texture_size = (1<<28); /* kind of arbitrary */

		heightAboveTerrain =
			PFDISTANCE_PT3(ViewState->viewCoord.xyz, bsph.center)
			- bsph.radius;

		max_possible_miplevel_size =
			pfuCalcSizeFinestMipLOD(frust,
					    viewportWidth, viewportHeight,
					    heightAboveTerrain,
					    min_dst_dxyz,
					    texture_size);
		minLOD = log2(texture_size / max_possible_miplevel_size);

		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "finest mip level size = %.9g, minLOD = %.9g, HAT = %.9g",
			 max_possible_miplevel_size, minLOD, heightAboveTerrain);

		/*
		 * Color the culling frame to match gridify's LOD colors:
		 *	green	32x32 2048x2048 131072x131072 8388608x8388608
		 *	yellow	16x16 1024x1024  65535x65535  4194304x4194304
		 *	red	 8x8   512x512 	 32767x32768  2097152x2097152
		 *	magenta  4x4   256x256   16384x16384  1048576x1048576
		 *	blue	 2x2   128x128	  8192x8192    524288x524288
		 *	cyan	 1x1    64x64	  4096x4096    262144x262144
		 */
		{
		    float hue = (log2(max_possible_miplevel_size)-3)/6.0f;
		    float sat = .9f;
		    float val = 1.0f;
		    float red, green, blue;
		    hsv_to_rgb(hue, sat, val, &red, &green, &blue);
		    PFSET_VEC3(ViewState->cullVolColors[0], red, green, blue);
		}
		if (ViewState->mip_auto_minLOD)
		{
#if 0 /* PERFLY */ /* XXX old behavior */
		    ViewState->clip_LODOffset = (int)minLOD;
		    ViewState->clip_LODOffset_dirty = 1;
		    ViewState->clip_anyParams_dirty = 1;
#endif /* PERFLY */
		}
	    }
	}
#endif /* CUSTOM */
    }
    }
}

#if 1 /* CUSTOM */
static float prev_heightAboveTerrain = -1;
#endif /* CUSTOM */

#ifdef WIN32
#define CLIPFLY_DLLIMPORT __declspec(dllimport)
#else
#define CLIPFLY_DLLIMPORT
#endif
/*
 * Reset view and Xformer to original XYZ position and HPR Euler angles
 */
void
resetPosition(int pos)
{
#if 1 /* CUSTOM */
    extern CLIPFLY_DLLIMPORT double _PFI_FUDGE_D;
    _PFI_FUDGE_D = 1; /* XXX destroys effect of env variable */
    if (getenv("_CLIPFLY_ADAPT"))
	prev_heightAboveTerrain = -1.;
#endif /* CUSTOM */

    pfiStopXformer(ViewState->xformer);
    switch (pos)
    {
    case POS_CENTER:
	pfiCenterXformer(ViewState->xformer);
	break;

    default:
    case POS_ORIG:
	pfiXformerCoord(ViewState->xformer, &ViewState->initViews[pos]);
	break;
    }
#if 0 /* PERFLY */
    pfiGetXformerMat(ViewState->xformer, ViewState->viewMat);
#endif /* PERFLY */
#if 1 /* CUSTOM */
    {
    pfMatrix4d viewMat4d;
    pfiGetXformerMat(ViewState->xformer, viewMat4d);
    PFCOPY_MAT(ViewState->viewMat, viewMat4d); /* XXX cop out */
    }
#endif /* CUSTOM */
    pfChanViewMat(ViewState->masterChan, ViewState->viewMat);
}


static void
updateWin(int pipeId)
{
    static int	winIndex = PFWIN_GFX_WIN;
    int 	doWinIndex = 0;
    int 	i;

    /* Select proper visual in the window stack to render to  */
    if (ViewState->stats && 
	(ViewState->statsEnable & PFSTATSHW_ENGFXPIPE_FILL))
    {
	pfPipeVideoChannel *pvchan;
	ViewState->doDVR = PFPVC_DVR_OFF;
	for (i=0; i < ViewState->numChans; i++)
	{
	    pvchan = pfGetChanPVChan(ViewState->chan[i]);
	    pfPVChanDVRMode(pvchan, PFPVC_DVR_OFF);
	}
	if(winIndex != PFWIN_STATS_WIN)
	{
	    winIndex = PFWIN_STATS_WIN;
	    doWinIndex = 1;
	}
    }
    else 
    {
	if (winIndex != PFWIN_GFX_WIN)
	{
	    winIndex = PFWIN_GFX_WIN;
	    doWinIndex = 1;
	}
    }
    if (doWinIndex)
    {
	int	i;
	int	lPipe;

	lPipe = 0;
	for (i=0; i < NumPipes; lPipe++)
	{
	    pfPWinIndex(ViewState->pw[i], winIndex);
	    if (!(pfGetPWinSelect(ViewState->pw[i])))
	    {
		pfPWinConfigFunc(ViewState->pw[i], ConfigPWin);
		pfConfigPWin(ViewState->pw[i]);
	    }

	    i += i < TotHyperpipePipes ? NumHyperpipePipes[lPipe] : 1;
	}
	ViewState->drawFlags[pipeId] |= UPDATE_MODES;
    }  
}


static void
updateStats(void)
{
    /*
     * Note: if turning on or off fill stats, we must redraw panel because
     * multisampling might be toggled as well.
    */
    static int     on = -1, en = 0;
    static int     first = 1;
    int i;

    pfFrameStats   *fsp;
    pfCalligraphic *callig;

    if (on && (!ViewState->stats))	/* Reset to minimal fast stats collection */
    {

#if 1 /* CUSTOM */
if (!getenv("_CLIPFLY_DONT_RESET_STATS"))
#endif /* CUSTOM */
	for (i=0; i < ViewState->numChans; i++)
	{
	    pfChannel *chan;
	    
	    chan = ViewState->chan[i];
	    fsp = pfGetChanFStats(chan);
	    /* put everything back to defaults */
	    pfFStatsClass(fsp, PFSTATS_ALL, PFSTATS_DEFAULT); 
	    /* set the fast process timing stats */
	    pfFStatsClassMode(fsp, PFFSTATS_PFTIMES, PFFSTATS_PFTIMES_BASIC, PFSTATS_SET);
	    on = 0;
	    en = 0;
	}
    }
    else if (ViewState->stats && (en != ViewState->statsEnable))
    {
	for (i=0; i < ViewState->numChans; i++)
	{
	    pfChannel *chan;

	    chan = ViewState->chan[i];

	    /* Get the stats structure for this channel */
	    fsp = pfGetChanFStats(chan);

	    /* First turn off old stats if not just gfx pipe stats */
	    if (en)
		pfFStatsClass(fsp, en & ~(PFSTATSHW_ENGFXPIPE_TIMES), PFSTATS_OFF);
	    if (ViewState->statsEnable & PFSTATSHW_ENGFXPIPE_FILL)
		pfFStatsClass(fsp, PFSTATSHW_ENGFXPIPE_TIMES, PFSTATS_OFF);

	    /* Now enable new stats graph */
	    if (ViewState->statsEnable)
	    {
#if 0 /* PERFLY */
		pfFStatsClass(fsp, ViewState->statsEnable, PFSTATS_ON);
		pfFStatsClassMode(fsp, PFFSTATS_PFTIMES, PFFSTATS_PFTIMES_MASK, PFSTATS_ON);
#endif /* PERFLY */
#if 1 /* CUSTOM */
		pfFStatsClass(fsp, ViewState->statsEnable | PFSTATS_ENTEXLOAD, PFSTATS_ON);
		pfFStatsClassMode(fsp, PFFSTATS_PFTIMES, PFFSTATS_PFTIMES_MASK, PFSTATS_ON);
		/* turn on full texture loading stats */
#if 0
		/* Drawing full histogram takes a long time,
		 * and pretty much always causes frame rate drops,
		 * so we no longer draw the histogram,
		 * so default stats don't have to suffer.
		 * XXX should have another stats gui option
		 * XXX to do "Dft+Texload" or something.
		 */
		pfFStatsClassMode(fsp, PFSTATS_TEXLOAD, PFSTATS_TEXLOAD_MASK, PFSTATS_ON);
#endif
#endif /* CUSTOM */
		if (ViewState->statsEnable & PFSTATSHW_ENGFXPIPE_TIMES) /* do just the status line */
		    pfChanStatsMode(chan, PFCSTATS_DRAW, 0);
		else
		    pfChanStatsMode(chan, PFCSTATS_DRAW, PFSTATS_ALL);
		if (first)
		{	/* set some stats class modes that are not
		     * on by default. this only needs to be done
		     * the first time on each channel */
		    pfFStatsClassMode(fsp, PFSTATSHW_GFXPIPE_FILL,
				     PFSTATSHW_GFXPIPE_FILL_DEPTHCMP |
				     PFSTATSHW_GFXPIPE_FILL_TRANSPARENT,
				     PFSTATS_ON);
		    /*
		     * This will also enable the tmesh statistics - not on by
		     * default because they are somewhat expensive
		     */
		    pfFStatsClassMode(fsp, PFSTATS_GFX,
				     PFSTATS_GFX_ATTR_COUNTS |
				     PFSTATS_GFX_TSTRIP_LENGTHS,
				     PFSTATS_ON);
		    pfFStatsAttr(fsp, PFFSTATS_UPDATE_SECS, 2.0f);

                    callig = pfGetChanCurCallig(chan);
		    if (callig != NULL)
		    {
			pfFStatsClass(fsp, PFSTATS_ENCALLIG, PF_ON);
		    }

		    if (i == ViewState->numChans-1)
			first = 0; /* did it on all the channels once */
		}
		    
	    }
	}
	en = ViewState->statsEnable;
	on = 1;
    }
    if (ViewState->printStats)
    {
	if (!ViewState->stats)
	        ViewState->stats = TRUE;
	else
	{
	    fsp = pfGetChanFStats(ViewState->masterChan);
	    pfPrint(fsp, PFFSTATS_BUF_AVG | PFFSTATS_BUF_PREV |
		    pfGetFStatsClass(fsp, PFSTATS_ALL),
		    PFPRINT_VB_INFO, 0);
	    ViewState->printStats=0;
	}
#if 1 /* CUSTOM */
/* #define XXXHACKHACKHACK_TEXLOADSTATS */
#ifdef XXXHACKHACKHACK_TEXLOADSTATS
	/* XXX crude stats 'cause we gotta see the texloads... always collect them, reset each time we print them */
	{
	    extern void _pfEnableTexLoadStats(void);
	    extern void _pfPrintTexLoadStats(void);
	    extern void _pfResetTexLoadStats(void);
	    _pfEnableTexLoadStats();
	    _pfPrintTexLoadStats();
	    _pfResetTexLoadStats();
	}
#endif /* XXXHACKHACKHACK_TEXLOADSTATS */

	if (ViewState->MPClipTexture)
	{
	    pfClipTexture *clipTex = pfGetMPClipTextureClipTexture(ViewState->MPClipTexture);
	    pfTextureValidMap *validMap = pfGetTexValidMap((pfTexture *)clipTex);
	    if (validMap == NULL)
	    {
		int virtSize, mapSize, clipSize;
		pfGetClipTextureVirtualSize(clipTex, &virtSize, NULL, NULL);
		clipSize = pfGetClipTextureClipSize(clipTex);
		mapSize = PF_MIN2(2*clipSize, virtSize);

		pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
			 "clipfly: creating a new texture valid map, won't know about already loaded areas\n");
		validMap = pfNewTextureValidMap(
			virtSize, virtSize,
			mapSize, mapSize,
			clipSize, clipSize,
			8, 8,
			pfGetSharedArena());
		PFASSERTALWAYS(validMap != NULL);
		pfTexValidMap((pfTexture *)clipTex, validMap);
	    }
	    pfTextureValidMapPrint(validMap, stdout, 1, 1<<30);
	    {
		int verbose = 1;
		int centerS, centerT, invalidBorder;
		pfGetClipTextureCenter(clipTex, &centerS, &centerT, NULL);
		invalidBorder = pfGetClipTextureInvalidBorder(clipTex);
		/* Print out which levels are invalid... */
		(void)pfTextureValidMapIsValidCenter(validMap,
			centerS, centerT, invalidBorder, verbose);
	    }
	}
#endif /* CUSTOM */
    }
}

    
/*
 * Perform non-latency-critical per-frame updates
 */
void
updateSim(pfChannel *chan)
{
    int 	pipeId;    
    int		i;
    
    pipeId = pfGetId(pfGetChanPipe(chan));
    
    /* Process keyboard input */
    processKeybdInput();

    /* Adjust load management filter */
    if (ViewState->stress)
	pfChanStressFilter(chan, 1.0f, ViewState->lowLoad, ViewState->highLoad,
	    ViewState->stressScale, ViewState->stressMax);
    else
	pfChanStressFilter(chan,1.0f, ViewState->lowLoad, ViewState->highLoad,
	    0.0f, 1.0f);

    if (ViewState->updateChannels)
    {
	updateGUI(ViewState->gui);
	ViewState->updateChannels = 0;
    }
    else if (ViewState->gui || ViewState->tree)
	pfuUpdateGUI(&ViewState->mouse);
    else
	pfuUpdateGUICursor();

    /* need to set cullMode on all channels since it is not in chanShare */
    if (ViewState->cullMode != -1)
	for (i = 0; i < ViewState->numChans; ++i)
	    if (ViewState->cullMode
	     != pfGetChanTravMode(ViewState->chan[i], PFTRAV_CULL))
		pfChanTravMode(ViewState->chan[i], PFTRAV_CULL, ViewState->cullMode);

    /* only set LODscale on master channel since it is in chanShare */
    if (ViewState->LODscale != pfGetChanLODAttr(ViewState->masterChan, PFLOD_SCALE))
	pfChanLODAttr(ViewState->masterChan, PFLOD_SCALE, ViewState->LODscale);

    { /* remove light source from scene when lighting is off */
	static int lightInScene = 1;
	if ((ViewState->lighting != 0) != lightInScene)
	{
	    if (ViewState->lighting)
	    {
		pfAddChild(ViewState->scene, ViewState->sunDCS);
		lightInScene = 1;
	    }
	    else
	    {
		pfRemoveChild(ViewState->scene, ViewState->sunDCS);
		lightInScene = 0;
	    }
	}
    }

    updateTimeOfDay(ViewState->timeOfDay);
    updateStats();
    updateEnvironment();
    updateWin(pipeId);

    pfGetChanViewMat(ViewState->masterChan, ViewState->viewMat);
    pfGetChanFOV(ViewState->masterChan, &ViewState->fov[0],
					&ViewState->fov[1]);
 
    /* update display lists based on draw mode */
    {
    static int oldDrawMode = -1;

    if (ViewState->drawMode != oldDrawMode)
    {
	oldDrawMode = ViewState->drawMode;

	switch(oldDrawMode)
	{
	case DLIST_OFF:
	    pfuTravSetDListMode(ViewState->scene, 0);
	    /*fprintf(stderr, "**** geoset display list DISABLED\n");*/
	    break;
	case DLIST_GEOSET: /* set dlist mode on pfGeoSets */
	    pfuTravSetDListMode(ViewState->scene, 1);
	    /*fprintf(stderr, "**** geoset display list COMPILING\n");*/
	    ViewState->drawMode = DLIST_COMPILE_GEOSET;
	    break;
	case DLIST_COMPILE_GEOSET: 
	    /* keep getting in here so long as draw procs are compiling */
	    oldDrawMode = DLIST_GEOSET;
	    {
		int done=1;
		int i;
		for (i=0; i < NumPipes; i++)
		{
		    if (ViewState->firstPipeDraw[i])
		    {
			done = 0;
			break;
		    }
		}
		if (done)
		{
		    ViewState->drawMode = DLIST_GEOSET;
		    /*fprintf(stderr, "**** geoset display list DONE\n");*/
		}
	    }
	    break;
	}
    }
    }
}


void
xformerModel(int model, int collideMode)
{
    static int		oldModel = -1, oldCollMode = -1;

    if (model == PFITDF_TETHER)
    {
	return; /* xformer model and mouse is ignored while model is tether */
    }
    if ((model == oldModel && oldCollMode == collideMode) || !ViewState->xformer)
	return;

    pfiSelectXformerModel(ViewState->xformer, model);

#if 0 /* PERFLY */
    /* If not in trackball mode, remove trackball DCS for better performance */
    if ((oldModel != model) && (oldModel == PFITDF_TRACKBALL))
    {
	pfRemoveChild(ViewState->scene, ViewState->sceneDCS);
	pfAddChild(ViewState->scene, ViewState->sceneGroup);
    }
    /* Restore trackball DCS to scene */
    else if ((oldModel != model) && (model == PFITDF_TRACKBALL))
    {
	pfRemoveChild(ViewState->scene, ViewState->sceneGroup);
	if (pfGetNumParents(ViewState->sceneDCS) == 0)
	    pfAddChild(ViewState->scene, ViewState->sceneDCS);
    }
#endif /* PERFLY */

#if 1 /* CUSTOM */
    if ((oldModel != model) && (model == PFITDF_SPHERIC))
    {
	/*
	 * Set the bounding sphere associated
	 * with the spheric motion model--
	 * this makes it behave a lot better
	 * XXX I'm unsure about when this is supposed to be done
	 * and for what motion models -DH
	 */
	pfiInputXformSpheric *_spheric =
	      pfiGetTDFXformerSpheric(ViewState->xformer);
	pfSphere bsph;
	pfGetNodeBSphere(ViewState->scene, &bsph);
	pfiIXformBSphere(_spheric, &bsph);
    }
#endif /* CUSTOM */

    oldModel = model;
    
    /* Collide with objects in scene */
    if (oldCollMode != collideMode)
    {
	if (collideMode == PF_ON)
	    pfiEnableXformerCollision(ViewState->xformer);
	else
	    pfiDisableXformerCollision(ViewState->xformer);
	oldCollMode = collideMode;
    }
#if 1 /* CUSTOM */
    fixSceneDCSAndChanViewForPrecision(ViewState->masterChan);
#endif /* CUSTOM */
}


/******************************************************************************
 *			    Draw Process Routines
 *************************************************************************** */


static void
drawMessage(pfPipeWindow *pw)
{
    pfFont	*fnt;
    pfString	*str;
    pfLight	*lt;
    pfMatrix	mat;
    pfFrustum	*frust;
    const pfBox	*bbox;
    float	dist;
    double	start, t;

    fnt = pfdLoadFont_type1(ViewState->objFontName, ViewState->objFontType);

    if (!fnt)
	return;

    pfPushState();
    pfPushIdentMatrix();
    
    str = pfNewString(NULL);
    pfStringMode(str, PFSTR_JUSTIFY, PFSTR_MIDDLE);
    pfStringFont(str, fnt);
    pfStringColor(str, 1.0f, 0.0f, 0.8f, 1.0f);
    pfStringString(str, ViewState->welcomeText);
    pfMakeScaleMat(mat, 1.0f, 1.0f, 1.5f);
    pfStringMat(str, mat);
    pfFlattenString(str);
    bbox = pfGetStringBBox(str);
    dist = PF_MAX2(bbox->max[PF_Z] - bbox->min[PF_Z],
		   bbox->max[PF_X] - bbox->min[PF_X]);

    dist = (dist/2.0f) / pfTan(45.0f/2.0f);

    frust = pfNewFrust(NULL);

    if (InitFOV) {
	pfFrustNearFar(frust, 1.0f, 1000.0f);
	pfMakePerspFrust(frust, 
		- pfTan (ViewState->fov[0]/2.0f),
		  pfTan (ViewState->fov[0]/2.0f),
		- pfTan (ViewState->fov[1]/2.0f),
		  pfTan (ViewState->fov[1]/2.0f) );
    } else
	pfMakeSimpleFrust(frust, 45.0f);

    pfApplyFrust(frust);
    pfDelete(frust);

    /* Convert from GL to Performer space */
    pfRotate(PF_X, -90.0f);

    pfEnable(PFEN_LIGHTING);
    lt = pfNewLight(NULL);
    pfLightPos(lt, .2f, -1.0f, 1.0f, 0.0f);
    pfLightOn(lt);

    start = pfGetTime();

    /* Twirl text for 0.25f seconds */
    do
    {
	pfClear(PFCL_COLOR | PFCL_DEPTH, NULL);
	
	t = (pfGetTime() - start) / 0.25f;
	t = PF_MIN2(t, 1.0f);

	pfMakeRotMat(mat, (1.0f - 0.5f*t) * -90.0f, 
		     1.0f, 0.0f, 0.0f); 

	t *= t;
	pfPostTransMat(mat, mat, 
		       0.0f, 
		       3.0f * t * dist +  6.0f * (1.0f - t) * dist,
		       0.0f);

	pfPushMatrix();
	pfMultMatrix(mat);
	pfDrawString(str);
	pfPopMatrix();

	pfSwapPWinBuffers(pw);

    } while (t < 1.0f);

    /* Get image in both front and back buffers */
    pfClear(PFCL_COLOR | PFCL_DEPTH, NULL);
    pfPushMatrix();
    pfMultMatrix(mat);
    pfDrawString(str);
    pfPopMatrix();

    pfSwapPWinBuffers(pw);

    pfLightOff(lt);

    pfPopMatrix();
    pfPopState();

    pfDelete(lt);
    pfDelete(str);
}

/* this will only be initialized in the draw process */
static int Overlay = -1;

void
initPipe(pfPipeWindow *pw)
{
    char	path[PF_MAXSTRING];

    static char msg[] = "Welcome to OpenGL Performer";

    /* Do not draw the welcome message in hyperpipe mode */
    if (Multipipe == HYPERPIPE)
	return;

    pfPushState();
    pfBasicState();


	Overlay = (pfGetPWinOverlayWin(pw) ? 1 : 0);

    if ((ViewState->welcomeText[0] == '\0') ||
	(!(pfFindFile("Times-Elfin.of", path, R_OK))))
    {

	pfClear(PFCL_COLOR, NULL);
	/* Message will be draw in in purple - the pfuDrawMessage() color */
	pfuDrawMessage(NULL, msg, PFU_MSG_PIPE, PFU_CENTER_JUSTIFIED,
	    0.5f, 0.5f, PFU_FONT_BIG, PFU_RGB);
	pfSwapPWinBuffers(pw);

	pfClear(PFCL_COLOR, NULL);
	/* Message will be draw in in purple - the pfuDrawMessage() color */
	pfuDrawMessage(NULL, msg, PFU_MSG_PIPE, PFU_CENTER_JUSTIFIED,
	    0.5f, 0.5f, PFU_FONT_BIG, PFU_RGB);
	pfSwapPWinBuffers(pw);
    }
    else
	drawMessage(pw);

   pfPopState();
}

static void
snapImage(int snapAlpha)
{
    FILE 	*fp;
    static  char str[80];
    static  int count = 0;
    pfPipe* cpipe = pfGetChanPipe(ViewState->masterChan);
    int    	id = pfGetId(cpipe)*10 + count++;
#if 1 /* CUSTOM */
    pfiInputXformSpheric *_spheric;
#endif /* CUSTOM */

    sprintf(str, "perfly.%05d.%s", id,
	    (snapAlpha) ? "rgba" : "rgb");
    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Saving pipe %d image in file %s\n",
	    pfGetId(cpipe), str);

    /* read from FRONT-buffer to get channel stats info */
    pfuSaveImage(str, 0, 0,
		 ViewState->mouse.winSizeX,
		 ViewState->mouse.winSizeY,
		 (snapAlpha) ? 1 : 0);

    /* create info file to go with image */
    sprintf(str, "perfly.%05d.info", id);
    if (fp = fopen(str,"w"))
    {
	float h, v;
	fprintf(fp, "Viewing parameters for snap %d of perfly\n\n", id);
	fprintf(fp, "XYZ: %f %f %f\n",
		ViewState->viewCoord.xyz[0],
		ViewState->viewCoord.xyz[1],
		ViewState->viewCoord.xyz[2]);
	fprintf(fp, "HPR: %f %f %f\n",
		ViewState->viewCoord.hpr[0],
		ViewState->viewCoord.hpr[1],
		ViewState->viewCoord.hpr[2]);

#if 1 /* CUSTOM */
	if (ViewState->xformerModel == PFITDF_SPHERIC) {
	    /* --- Save info about spheric */
	    _spheric = pfiGetTDFXformerSpheric(ViewState->xformer);
	    fprintf(fp, "Radius/Around/Tilt/Roll: %f %f %f %f\n",
		pfiGetIXformSphericParameter(_spheric, PFIXSPH_RADIUS),
		pfiGetIXformSphericParameter(_spheric, PFIXSPH_AROUND),
		pfiGetIXformSphericParameter(_spheric, PFIXSPH_TILT),
		pfiGetIXformSphericParameter(_spheric, PFIXSPH_ROLL));
	}
#endif /* CUSTOM */

	fprintf(fp, "NEAR/FAR: %f %f \n",
		ViewState->nearPlane, ViewState->farPlane);
	pfGetChanFOV(ViewState->masterChan, &h, &v);
	fprintf(fp, "FOV: horiz=%f vert=%f\n", h, v);
	fclose(fp);
    }

    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Done\n");
}

static void
drawOverlayName(pfPipeWindow *pw)
{
    pfWindow 		*pwOver = pfGetPWinOverlayWin(pw);
    static int 		mapped = 0;

    pfPushState();
    pfBasicState();
    
    if(ViewState->doDVR)
    {
	/* set the viewport to the window to completely clear the overlays */
	int xs, ys;
	pfGetPWinSize(pw, &xs, &ys);
	glViewport(0, 0, xs, ys);
    }

    {
	if (pwOver)
	    pfSelectWin(pwOver);
	else
	    return;
    }

	pfClear(PFCL_COLOR, NULL);
	
    if(!ViewState->doDVR)
    {

    if (!mapped)
    {
	static pfVec3 clrs[2] = { 
	            {0.4f, 0.0f, 0.5f},		/* tex - purple */
                    {0.125f, 0.06f, 00.125f}};	/* shadow - drk grey */
	pfuMapWinColors(pwOver, clrs, 1, 2);
	mapped = 1;
    }	
    pfuDrawMessageCI(ViewState->masterChan,
	    ViewState->overlayText, PFU_MSG_CHAN, PFU_RIGHT_JUSTIFIED,
	    1.0f, 1.0f, PFU_FONT_SMALL, 1, 2);
	    	    

    }

    pfSelectPWin(pw);

    pfPopState();

}



/* convey draw style from localPreDraw to localPostDraw */
static int selectedDrawStyle = 0;

/* ARGSUSED (suppress compiler warn) */
void
localPreDraw(pfChannel *chan, void *data)
{
    pfPipe *pipe = pfGetChanPipe(chan);
    int pipeId = pfGetId(pipe);
    
#if 1 /* CUSTOM */
#ifdef NOTYET /* was getting invalid enums and UMRs from glColorTableSGI */
    static int first = 1;

    if (first) {
#if 1
#define COLOR_TABLE_SIZE 256
	static unsigned char table[COLOR_TABLE_SIZE][4];
	int i, c;
	for (i = 0; i < COLOR_TABLE_SIZE; i++) {
	    table[i][0] = 0;
	    table[i][1] = 0;
	    table[i][2] = 0xff;
	    table[i][3] = 0xff;
	}
	
	glColorTableSGI(GL_TEXTURE_COLOR_TABLE_SGI, GL_RGBA8_EXT,
			COLOR_TABLE_SIZE, GL_RGBA,
			GL_UNSIGNED_BYTE, table);
#endif

#if 0
#define COLOR_TABLE_SIZE 16
#define COLOR_TABLE_SCALE (256/COLOR_TABLE_SIZE)
	static unsigned char table[COLOR_TABLE_SIZE][4];
	int i, c;
	for (i = 0; i < COLOR_TABLE_SIZE; i++) {
	    table[i][0] = 255;
	    table[i][1] = 0;
	    table[i][2] = 0;
	    table[i][3] = 0;
/* 	    table[i][0] = (i << 4) | i; */
/* 	    table[i][1] = (i << 4) | i; */
/* 	    table[i][2] = (i << 4) | i; */
/* 	    table[i][3] = (i << 4) | i; */
	}
	
	glColorTableSGI(GL_TEXTURE_COLOR_TABLE_SGI, GL_RGBA,
			COLOR_TABLE_SIZE, GL_RGBA,
			GL_UNSIGNED_BYTE, table);
#endif	    
 	first = 0;
    }
    
    if (ViewState->tlut)
	glEnable(GL_TEXTURE_COLOR_TABLE_SGI);
    else
	glDisable(GL_TEXTURE_COLOR_TABLE_SGI);
#endif /* NOTYET */
#endif /* CUSTOM */

    if ((ViewState->firstPipeDraw[pipeId]) && 
	(ViewState->drawMode == DLIST_COMPILE_GEOSET))
    {
	pfuTravCompileDLists(ViewState->sceneGroup, PFU_ATTRS_NONE);
	ViewState->firstPipeDraw[pipeId] = 0;
    }

    /*
     * Update new draw modes
     */
    if (ViewState->drawFlags[pipeId] & UPDATE_MODES)
    {
	{ /* put this first because it may destroy the old win and
	   * create a new one (in GLX mode) which will require a
	   * window redraw.
	   */
	    static int aa=-1;
	    if (aa == -1)
		aa = ViewState->aa;
	    else if (aa != ViewState->aa)
	    {
		if (ViewState->aa)
		    pfAntialias(PFAA_ON);
		else
		    pfAntialias(PFAA_OFF);
		aa = ViewState->aa;
	    }
	}

	if (ViewState->backface)
	{
	    /* leave backface removal to loader state */
	    pfOverride(PFSTATE_CULLFACE, 0); 
	    pfCullFace(PFCF_BACK);
	}
	else
	{
	    pfCullFace(PFCF_OFF);
	    /* really force backface removal off */
	    pfOverride(PFSTATE_CULLFACE, 1); 
	}

	if (ViewState->lighting == LIGHTING_OFF)
	{
	    pfOverride(PFSTATE_ENLIGHTING | PFSTATE_FRONTMTL, 0);
	    pfDisable(PFEN_LIGHTING);
	    pfOverride(PFSTATE_ENLIGHTING | PFSTATE_FRONTMTL, 1);
	}
	else
	    pfOverride(PFSTATE_ENLIGHTING | PFSTATE_FRONTMTL, 0);

	if (ViewState->texture == FALSE)
	{
	    pfOverride(PFSTATE_ENTEXTURE | PFSTATE_TEXTURE | PFSTATE_ENTEXGEN, 0);
	    pfDisable(PFEN_TEXTURE);
	    pfDisable(PFEN_TEXGEN);
	    pfOverride(PFSTATE_ENTEXTURE | PFSTATE_TEXTURE | PFSTATE_ENTEXGEN, 1);
	}
	else
	    pfOverride(PFSTATE_ENTEXTURE | PFSTATE_TEXTURE | PFSTATE_ENTEXGEN, 0);

	ViewState->drawFlags[pipeId] &= ~UPDATE_MODES;
    }

    /* 
     * remember draw style in case it changes between now 
     * and the time localPostDraw() gets called.
     */
    selectedDrawStyle = ViewState->drawStyle;

    /* handle draw style */
    pfuPreDrawStyle(selectedDrawStyle, ViewState->scribeColor);

#if 1 /* CUSTOM */
    {
    static int _CLIPFLY_DUMP_DLIST = -1;
    if (_CLIPFLY_DUMP_DLIST == -1)
    {
	char *e = getenv("_CLIPFLY_DUMP_DLIST");
	_CLIPFLY_DUMP_DLIST = e ? *e ? atoi(e) : PFPRINT_VB_DEBUG : 0;
    }
    if (_CLIPFLY_DUMP_DLIST)
    {
	pfDispList *dlist = pfNewDList(PFDL_FLAT, 1, pfGetSharedArena());
	pfOpenDList(dlist);

	printf("(begin drawing frame %d into temporary dlist)\n", pfGetFrameCount());
	pfDraw();
	printf("(end drawing frame %d into temporary dlist)\n", pfGetFrameCount());

	pfCloseDList();
	printf("======= Frame %d dlist: ================\n", pfGetFrameCount());
	pfPrint(dlist, PFTRAV_SELF | PFTRAV_DESCEND, _CLIPFLY_DUMP_DLIST, stdout);
	printf("===========================================\n");
	pfDelete(dlist);
    }
    }
#endif /* CUSTOM */

#if 1 /* CUSTOM */
    /*
     * Exercise every page of shared memory,
     * to flush out first-time-accessed overhead.
     * Can't do this by default, since if
     * the arena is bigger than physical memory
     * we will swap like crazy (and crash if the swap space is virtual).
     * So only do this if the environment variable
     * _CLIPFLY_EXERCISE_MEMORY is set.
     */
    {
	static int done_already = 0;
	if (!done_already)
	{
	    int i, n = 0; /* zero times by default, so we don't risk swapping */
	    char *e = getenv("_CLIPFLY_EXERCISE_MEMORY");
	    if (e != NULL)
		n = (*e ? atoi(e) : 1);
	    for (i = 0; i < n; ++i) /* first time slow, subsequently fast */
	    {
		int pageSize = getpagesize();
		char *arenaStart = (char *)pfGetSharedArena();
		size_t arenaSize = pfGetSharedArenaSize();
		char *arenaEnd = arenaStart + arenaSize;
		char *p;
		double startTime, endTime;
		int sum;

		pfNotify(PFNFY_INFO, PFNFY_PRINT,
			 "DRAW Exercising shared memory");
		pfNotify(PFNFY_INFO, PFNFY_MORE,
			 "  pageSize=%d", pageSize);
		pfNotify(PFNFY_INFO, PFNFY_MORE,
			 "  arenaSize=%.17g (%.17g pages)",
			 (double)arenaSize, (double)(arenaSize/pageSize));

		startTime = pfGetTime();
		sum = 0;
		for (p = arenaStart; p < arenaEnd; p += pageSize)
		{
		    /*
		       Read the first byte from each page
		       and actually do something with it,
		       so the compiler can't optimize this loop away...
		     */
		    sum += *p;
		}
		endTime = pfGetTime();
		/* Must do something with the result... */
		sprintf(NULL, "%0.0d", sum);

		pfNotify(PFNFY_INFO, PFNFY_PRINT,
			 "  Elapsed time: %.9f sec",
			 endTime-startTime);

	    }
	    done_already = 1;
	}
    }
#endif /* CUSTOM */
}

/* ARGSUSED (suppress compiler warn) */
void
localPostDraw(pfChannel *chan, void *data)
{
    pfPipe *mPipe, *cPipe;
    pfPipeWindow *pw;
    int pipeId;

    mPipe = pfGetChanPipe(ViewState->masterChan);
    cPipe = pfGetChanPipe(chan);
    pw = pfGetChanPWin(chan);
    pipeId = pfGetId(cPipe);
    
    /* handle draw style */
    pfuPostDrawStyle(selectedDrawStyle);

    if (ViewState->exitCount == 0)
	ViewState->exitFlag = TRUE;

    if (ViewState->exitCount > 0)
	ViewState->exitCount--;

    if (ViewState->printStats > 0)
	ViewState->printStats--;

    /* take snapshot of image */
    if (ViewState->snapImage)
    {
	snapImage(ViewState->snapImage == -2);
	if (ViewState->snapImage > 0)
	    ViewState->snapImage--;
	else
	    ViewState->snapImage = 0;
    }



    /* paolo: drawing selected cliptex roaming levels in overlay */
    if( (ViewState->drawCteTex) &&
        (chan == ViewState->masterChan) &&
	(ViewState->MPClipTexture != NULL) )
    {
	pfClipTexture* cliptex = 	
	    pfGetMPClipTextureClipTexture(ViewState->MPClipTexture);
	if( (cliptex!=NULL) && pfIsClipTextureEmulated(cliptex) )
	{
	    pfNotify( PFNFY_DEBUG, PFNFY_PRINT, "Drawing Roaming Textures in Overlay");
	    cteDrawOverlayTextures( chan, cliptex );
	}
    }		        














    /* Handle REDRAW event */
    if (ViewState->drawFlags[pipeId] & REDRAW_WINDOW)
    {
	/* master channel controls master pipe */
	if (pipeId == (pfGetId(pfGetChanPipe(ViewState->masterChan))))
	{
	    /* Only draw message into master channel */
	    if (chan == ViewState->masterChan)
	    {
		if (ViewState->redrawOverlay)
		{
		    if (Overlay)
			drawOverlayName(pw);
		    /* make sure enough draw frames happened to fill both buffers with 
		     * info trickled down from the draw (2 frames potential latency
		     */
		    if ((pfGetPipeDrawCount(pipeId) - ViewState->redrawCount) > 4) 
			ViewState->redrawOverlay = 0;
		}
		else
		{
		    ViewState->drawFlags[pipeId] &= ~REDRAW_WINDOW;
		    ViewState->redrawOverlay = 0;
	   	}	
	    }
	}
	else
	    ViewState->drawFlags[pipeId] &= ~REDRAW_WINDOW;


	pfClear(PFCL_DEPTH, NULL); /* needed for Extreme when move window 
				       * with sky-ground earth-sky
				       */
    }

    if (ViewState->gui)
	updateGUIViewMsg();

    if (ViewState->doDVR && ViewState->showDVR)
	pfuDrawChanDVRBox(chan);

    if (cPipe == mPipe)
    {

	/* draw scene-graph hierarchy */
	if (ViewState->tree && (chan == ViewState->masterChan))
	    pfuDrawTree(chan, (pfNode*)ViewState->scene, ViewState->panScale);

	/* draw 2D cursor */
	if ((pfuGetCursorType() == PFU_CURSOR_2D) && ViewState->mouse.inWin)
	{
	    pfPipeWindow *pw = pfGetChanPWin(chan);
	    pfuGUICursor(PFU_CURSOR_MAIN, PFU_CURSOR_OFF);
	    pfuGUICursor(PFU_CURSOR_GUI, PFU_CURSOR_OFF);
	    pfuDrawPWin2DCursor(pw, ViewState->mouse.xpos, ViewState->mouse.ypos);
	}
	else
	{
	    pfuGUICursor(PFU_CURSOR_MAIN, PFU_CURSOR_DEFAULT);
	    pfuGUICursor(PFU_CURSOR_GUI, PFU_CURSOR_DEFAULT);
	}

    }
}
	

void
initXformer(void)
{
#if 1 /* CUSTOM */
    pfiInputXformSpheric *_spheric;
#endif /* CUSTOM */

    ViewState->xformer = pfiNewTDFXformer(pfGetSharedArena());

    pfiXformerAutoInput(ViewState->xformer, ViewState->masterChan,
	&ViewState->mouse, &ViewState->events);

    pfiXformerNode(ViewState->xformer, ViewState->sceneGroup);
    pfiXformerAutoPosition(ViewState->xformer, NULL, ViewState->sceneDCS);
    pfiXformerResetCoord(ViewState->xformer, &ViewState->initViews[0]);
    xformerModel(ViewState->xformerModel, ViewState->collideMode);

#if 1 /* CUSTOM */
    if (ViewState->sphericPathFileName[0]) {
	_spheric = pfiGetTDFXformerSpheric(ViewState->xformer);
	pfiIXformSphericReadPathFile(_spheric,
				     ViewState->sphericPathFileName);
	pfiIXformSphericSetWorld(_spheric, 0, PFIXSPH_PATH_OUT);
    }
#endif /* CUSTOM */

    resetPosition(POS_ORIG);
}

void
drawModesChanged(void)
{
    int	i;

    for (i=0; i<NumPipes; i++)
	ViewState->drawFlags[i] |= UPDATE_MODES;
}

/* ARGSUSED (suppress compiler warn) */
void
preApp(pfChannel *chan, void *data)
{
#if 1 /* CUSTOM */
    /*
     * Check whether any MP clip textures
     * got added or deleted from pipes...
     */
    if (syncMPClipTexturesList(ViewState->MPClipTextures))
    {
	int i, num;
	PFASSERTDEBUG(syncMPClipTexturesList(ViewState->MPClipTextures) == 0);
	num = ViewState->numMPClipTextures =pfGetNum(ViewState->MPClipTextures);
	pfNotify(PFNFY_INFO, PFNFY_PRINT,
	    "Frame %d: mp clip texture list changed; now %d mp clip textures",
	    pfGetFrameCount(), num);

	/*
	 * Keep the same selected clip texture if possible,
	 * but revert to ALL if it disappeared
	 */
	if (ViewState->MPClipTexture != NULL
	 && pfSearch(ViewState->MPClipTextures, ViewState->MPClipTexture) < 0)
	    ViewState->MPClipTexture = NULL;

	/*
	 * If now ALL and there are any clip textures in the scene,
	 * get values from the first one.
	 */
	if (ViewState->MPClipTexture == NULL && num > 0)
	    getMPClipTextureValues(
		(pfMPClipTexture *)pfGet(ViewState->MPClipTextures, 0));

	/*
	 * Update the clip texture selection menu widget
	 */
	ViewState->clipTexStrs = (pfuGUIString *) pfRealloc(ViewState->clipTexStrs, sizeof(pfuGUIString)*(num+1));
	strcpy(ViewState->clipTexStrs[0], "(ALL)");
	for (i=0; i < num; i++)
	{
	    const char *name = pfGetTexName(
	      (pfTexture*) pfGetMPClipTextureClipTexture(
			(pfMPClipTexture*)pfGet(ViewState->MPClipTextures, i)));
	    strcpy(ViewState->clipTexStrs[i+1], name);
	}
	pfuWidgetSelections(ViewState->guiWidgets[GUI_CLIPTEX_SELECT],
			    ViewState->clipTexStrs, NULL, NULL, 
			    ViewState->numMPClipTextures+1);
	updateWidget(GUI_CLIPTEX_SELECT,
		     pfSearch(ViewState->MPClipTextures,
			      ViewState->MPClipTexture)+1);

	/*
	 * If per-pipe totalDTRTime was specified on the command line,
	 * the individual MP clip textures' times should be set to -1
	 * so that they will defer to the pipe's time.
	 * In general this does something random; we only
	 * care about startup (first couple of frames)
	 * when the user hasn't changed the selection from ALL
	 * so the -1 will get propagated to all the MP clip textures.
	 */
	if (ViewState->clip_totalDTRTimeWasSpecified)
	{
	    ViewState->clip_DTRTime = -1; /* so it will defer to pipe time */
	    ViewState->clip_DTRTime_dirty = 1;
	    ViewState->clip_anyParams_dirty = 1;
	}
    }

    if (ViewState->clip_anyParams_dirty)
    {
	int minSelected, maxSelected, i;
	/*
	 * Always unset dirty flag before using the variables,
	 * to avoid a race (the other process sets the dirty
	 * flag after setting the variables).
	 */
	ViewState->clip_anyParams_dirty = 0; /* always unset dirty first */

	if (ViewState->MPClipTexture != NULL)
	{
	    minSelected = maxSelected = pfSearch(ViewState->MPClipTextures,
					         ViewState->MPClipTexture);
	}
	else /* "ALL" clip textures are selected */
	{
	    minSelected = 0;
	    maxSelected = pfGetNum(ViewState->MPClipTextures)-1;
	}

#define MPCLIPTEXTURE(i) ((pfMPClipTexture*)pfGet(ViewState->MPClipTextures,i))
#define CLIPTEXTURE(i) pfGetMPClipTextureClipTexture(MPCLIPTEXTURE(i))
#define DEFINE_CLIP_LIMITS(type, lo, hi, sliderval, mode) \
   type lo = ((mode)&CLIP_CTRLMODE_GEQ) ? (sliderval) : (type)-1000;\
   type hi = ((mode)&CLIP_CTRLMODE_LEQ) ? (sliderval) : (type)1000


	if (ViewState->mip_minLOD_dirty)
	{
	    DEFINE_CLIP_LIMITS(float, minLOD_lo, minLOD_hi,
			      ViewState->mip_minLOD,
			      ViewState->minLODMode);
	    ViewState->mip_minLOD_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
	    {
		pfMPClipTexture *MPClipTex = MPCLIPTEXTURE(i);
		_pfMPClipTextureMinLOD(MPClipTex, ViewState->mip_minLOD);
		pfMPClipTextureMinLODLimit(MPClipTex,
					minLOD_lo, minLOD_hi);
	    }
	}

	if (ViewState->mip_maxLOD_dirty)
	{
	    DEFINE_CLIP_LIMITS(float, maxLOD_lo, maxLOD_hi,
			      ViewState->mip_maxLOD,
			      ViewState->maxLODMode);
	    ViewState->mip_maxLOD_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
	    {
		pfMPClipTexture *MPClipTex = MPCLIPTEXTURE(i);
		_pfMPClipTextureMaxLOD(MPClipTex, ViewState->mip_maxLOD);
		pfMPClipTextureMaxLODLimit(MPClipTex,
					maxLOD_lo, maxLOD_hi);
	    }
	}

	if (ViewState->clip_LODOffset_dirty)
	{
	    DEFINE_CLIP_LIMITS(int, LODOffset_lo, LODOffset_hi,
			      ViewState->clip_LODOffset,
			      ViewState->clipOffsetMode);
	    ViewState->clip_LODOffset_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
	    {
		pfMPClipTexture *MPClipTex = MPCLIPTEXTURE(i);
		pfMPClipTextureVirtualLODOffset(MPClipTex,
					 ViewState->clip_LODOffset);
		pfMPClipTextureLODOffsetLimit(MPClipTex,
					LODOffset_lo, LODOffset_hi);
	    }
	}

	if (ViewState->clip_numEffectiveLevels_dirty)
	{
	    DEFINE_CLIP_LIMITS(int, effectiveLevels_lo, effectiveLevels_hi,
			      ViewState->clip_numEffectiveLevels,
			      ViewState->elevelsMode);
	    ViewState->clip_numEffectiveLevels_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
	    {
		pfMPClipTexture *MPClipTex = MPCLIPTEXTURE(i);
		pfMPClipTextureNumEffectiveLevels(MPClipTex,
					  ViewState->clip_numEffectiveLevels);
	        pfMPClipTextureNumEffectiveLevelsLimit(MPClipTex,
				        effectiveLevels_lo, effectiveLevels_hi);
	    }
	}

	if (ViewState->mip_LODBiasS_dirty)
	{
	    DEFINE_CLIP_LIMITS(float, LODBiasS_lo, LODBiasS_hi,
			      ViewState->mip_LODBiasS,
			      ViewState->biasSMode);
	    ViewState->mip_LODBiasS_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
	    {
		pfMPClipTexture *MPClipTex = MPCLIPTEXTURE(i);
		_pfMPClipTextureLODBiasS(MPClipTex,
				        ViewState->mip_LODBiasS);
		_pfMPClipTextureLODBiasSLimit(MPClipTex,
					     LODBiasS_lo, LODBiasS_hi);
	    }
	}

	if (ViewState->mip_LODBiasT_dirty)
	{
	    DEFINE_CLIP_LIMITS(float, LODBiasT_lo, LODBiasT_hi,
			      ViewState->mip_LODBiasT,
			      ViewState->biasTMode);
	    ViewState->mip_LODBiasT_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
	    {
		pfMPClipTexture *MPClipTex = MPCLIPTEXTURE(i);
		_pfMPClipTextureLODBiasT(MPClipTex,
				        ViewState->mip_LODBiasT);
		_pfMPClipTextureLODBiasTLimit(MPClipTex,
					     LODBiasT_lo, LODBiasT_hi);
	    }
	}

	if (ViewState->mip_LODBiasR_dirty)
	{
	    DEFINE_CLIP_LIMITS(float, LODBiasR_lo, LODBiasR_hi,
			      ViewState->mip_LODBiasR,
			      ViewState->biasRMode);
	    ViewState->mip_LODBiasR_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
	    {
		pfMPClipTexture *MPClipTex = MPCLIPTEXTURE(i);
		_pfMPClipTextureLODBiasR(MPClipTex,
				        ViewState->mip_LODBiasR);
		_pfMPClipTextureLODBiasRLimit(MPClipTex,
					     LODBiasR_lo, LODBiasR_hi);
	    }
	}

	if (ViewState->clip_invalidBorder_dirty)
	{
	    ViewState->clip_invalidBorder_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
		pfMPClipTextureInvalidBorder(MPCLIPTEXTURE(i),
					     ViewState->clip_invalidBorder);
	}

	if (ViewState->mip_magFilter_dirty)
	{
	    ViewState->mip_magFilter_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
		pfMPClipTextureMagFilter(MPCLIPTEXTURE(i),
					     ViewState->mip_magFilter);
	}

	/* XXX changing min filter on a clip texture is not supported */
#if 0
	if (ViewState->mip_minFilter_dirty)
	{
	    ViewState->mip_minFilter_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
		pfMPClipTextureMinFilter(MPCLIPTEXTURE(i),
					 ViewState->mip_minFilter);
	}
#endif

	if (ViewState->clip_DTRMode_dirty)
	{
	    ViewState->clip_DTRMode_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
		pfMPClipTextureDTRMode(MPCLIPTEXTURE(i),
				       ViewState->clip_DTRMode);
	}

	if (ViewState->clip_totalDTRTime_dirty)
	{
	    ViewState->clip_totalDTRTime_dirty = 0;
	    for (i = pfGetMultipipe()-1; i >= 0; --i)
		pfPipeTotalTexLoadTime(pfGetPipe(i), ViewState->clip_totalDTRTime);
	}

	if (ViewState->clip_DTRTime_dirty)
	{
	    ViewState->clip_DTRTime_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
		pfMPClipTextureTexLoadTime(MPCLIPTEXTURE(i),
				           ViewState->clip_DTRTime);
	}

	if (ViewState->clip_DTRBlurMargin_dirty)
	{
	    ViewState->clip_DTRBlurMargin_dirty = 0;
	    for (i = minSelected; i <= maxSelected; ++i)
		pfMPClipTextureDTRBlurMargin(MPCLIPTEXTURE(i),
				           ViewState->clip_DTRBlurMargin);
	}

    }

    {
	static double _CLIPFLY_ADAPT = -1;
	if (_CLIPFLY_ADAPT == -1)
	{
	    char *e = getenv("_CLIPFLY_ADAPT");
	    _CLIPFLY_ADAPT = (e ? *e ? atof(e) : 1. : 0.);
	}
	if (_CLIPFLY_ADAPT != 0.)
	{
	    /*
	     * Want eye in space of model.
	     * Why must this always be so difficult?
	     * XXX there is other code in here to do this,
	     * look for heightAboveTerrain
	     */
	    pfVec3d eyeInModelSpace;
	    double eyeLengthSqrd, eyeLength;
	    float heightAboveTerrain;

	    {
		pfMatrix4d DCSMat, invDCSMat;
		pfGetDoubleDCSMat(ViewState->sceneDCS, DCSMat);
		pfInvertOrthoNMat4d(invDCSMat, DCSMat);
		pfXformPt3d(eyeInModelSpace,
			    ViewState->viewCoord.xyz, invDCSMat);
	    }
	    eyeLengthSqrd = PFDOT_VEC3(eyeInModelSpace, eyeInModelSpace);
	    eyeLength = sqrt(eyeLengthSqrd);
	    heightAboveTerrain = eyeLength
			       - _CLIPFLY_ADAPT; /* radius */

	    /*
	     * Re-interpret the nearPlane and farPlane clip plane slider
	     * values to be multipiers times
	     * HAT and horizonDist respectively,
	     * instead of actual values.
	     * XXX this should probably depend on some other flag
	     * XXX or environment variable, so we don't
	     * XXX risk breaking existing apps!
	     */
	    {
		float horizonDist;
		if (eyeLength > 1.) /* outside sphere */
		    horizonDist = sqrtf((float)(eyeLengthSqrd-1.));
		else /* inside or at surface of sphere */
		    horizonDist = eyeLength + 1.;

		pfChanNearFar(ViewState->masterChan,
			      ViewState->nearPlane * PF_ABS(heightAboveTerrain),
			      ViewState->farPlane * horizonDist);
		/*
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "nearPlane=%.9g * HAT=%.9g == %.9g",
			 ViewState->nearPlane, PF_ABS(heightAboveTerrain),
			 ViewState->nearPlane * PF_ABS(heightAboveTerrain));
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "farPlane=%.9g * horizonDist=%.9g == %.9g",
			 ViewState->farPlane, horizonDist,
			 ViewState->farPlane * horizonDist);
		*/
	    }

	    heightAboveTerrain = PF_CLAMP(heightAboveTerrain, 0., _CLIPFLY_ADAPT);
	    if (heightAboveTerrain > 0
	     && heightAboveTerrain != prev_heightAboveTerrain)
	    {
		if (prev_heightAboveTerrain > 0)
		{
		    extern CLIPFLY_DLLIMPORT double _PFI_FUDGE_D;
		    _PFI_FUDGE_D *= heightAboveTerrain / prev_heightAboveTerrain;
		}
		prev_heightAboveTerrain = heightAboveTerrain;
	    }
	} /* end if _CLIPFLY_ADAPT != 0. */
    }

    /*
     * Workaround for initial "sticking"
     * in which the pyramid levels, which can't be split
     * up into slivers, can't be loaded inside
     * the allotted time.
     * This can still fail if the pyramid tiles
     * are not yet loaded into memory when these initial frames run out.
     */
#define DEFAULT_CLIPFLY_UNSTICK_INITIAL_FRAMES 120
    {
	static int _CLIPFLY_UNSTICK_INITIAL_FRAMES = -1;
	if (_CLIPFLY_UNSTICK_INITIAL_FRAMES == -1)
	{
	    char *e = getenv("_CLIPFLY_UNSTICK_INITIAL_FRAMES");
	    _CLIPFLY_UNSTICK_INITIAL_FRAMES = (e ? *e
		? atoi(e) /* if set to a nonempty string */
		: 10	/* if set to empty string */
		: DEFAULT_CLIPFLY_UNSTICK_INITIAL_FRAMES); /* if not set */
	}
	if (_CLIPFLY_UNSTICK_INITIAL_FRAMES > 0
	 && pfGetFrameCount() < _CLIPFLY_UNSTICK_INITIAL_FRAMES)
	{
	    int i;
	    for (i = pfGetMultipipe()-1; i >= 0; --i)
		pfPipeTotalTexLoadTime(pfGetPipe(i), 1e6f); /* lots of time */
	    ViewState->clip_totalDTRTime_dirty = 1;
	    ViewState->clip_anyParams_dirty = 1;

	    if (pfGetFrameCount() <= 1)
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "Initial frame %d: allowing unlimited pipe texloadtime for first %d frames to try to decrease the probability of getting stuck at coarse levels and never being able to load them.  Setenv _CLIPFLY_UNSTICK_INITIAL_FRAMES <nframes> to alter this behavior.", pfGetFrameCount(), _CLIPFLY_UNSTICK_INITIAL_FRAMES);

	    if (pfGetFrameCount() >= _CLIPFLY_UNSTICK_INITIAL_FRAMES-1)
	    {
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "Frame %d: allowing unlimited pipe texloadtime for the last time (reverting to normal DTR texloadtime behavior)", pfGetFrameCount());
		_CLIPFLY_UNSTICK_INITIAL_FRAMES = 0; /* avoid check hereafter */
	    }
	}
    }

#endif /* CUSTOM */
    return;
}

/* ARGSUSED (suppress compiler warn) */
void
postApp(pfChannel *chan, void *data)
{
    return;
}


#if 1 /* CUSTOM */

pfClipTexture *findThisPipesMasterOrSlaveClipTexture(pfPipe *pipe, pfClipTexture *clipTex)
{
    int i;
    for (i = pfGetPipeNumMPClipTextures(pipe)-1; i >= 0; --i)
    {
	pfClipTexture *cliptex = pfGetMPClipTextureClipTexture(pfGetPipeMPClipTexture(pipe,i));
	if (clipTex == cliptex
	 || pfGetClipTextureMaster(clipTex) == cliptex
	 || pfGetClipTextureMaster(cliptex) == clipTex)
	{
	    /*
	    if (clipTex == cliptex)
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "Copy of clipTex 0x%p on pipe 0x%p(%d) is 0x%p (%s)",
			 clipTex, 
	    */
	    return cliptex;
	}
    }
    pfNotify(PFNFY_FATAL, PFNFY_ASSERT, "Couldn't find this pipe's copy of cliptex");
    return NULL;
}

#endif /* CUSTOM */

/* ARGSUSED (suppress compiler warn) */
void
preCull(pfChannel *chan, void *data)
{
#if 1 /* CUSTOM */
    int ViewState_gridify;
    /* get a snapshot of ViewState->gridify which may be changing */
    ViewState_gridify = ViewState->gridify;
    if (ViewState->invalidateFrame > 0
     || ViewState->invalidate
     || ViewState_gridify != ViewState->gridified)
    {
	int minSelected, maxSelected, i;
	if (ViewState->MPClipTexture != NULL)
	{
	    minSelected = maxSelected = pfSearch(ViewState->MPClipTextures,
					         ViewState->MPClipTexture);
	}
	else /* "ALL" clip textures are selected */
	{
	    minSelected = 0;
	    maxSelected = pfGetNum(ViewState->MPClipTextures)-1;
	}

	/*
	    All pipes need to rendezvous at the beginning of an
	    invalidation (and at various times during the invalidation).
	    There are already rendezvouses within libpf
	    at the beginning and end of each CULL frame.
	    If we just say:
		if (ViewState->invalidate)
		{
		    rendezvous with all other CULLs;
		    do the invalidation;
		}
	    we will deadlock if some but not all of the CULL processes
	    see that invalidate is set
	    (this can happen since invalidate is set
	    in an asynchronous process).
	    So instead, when we see that invalidate has been set,
	    we set invalidateFrame to pfGetFrameCount()+1
	    (it's okay if more than one process does this).
	    Then when all the CULL processes reach a frame
	    >= invalidateFrame, they will all be at the same
	    frame and they can all proceed.
	    (The test must be for >= rather then ==
	    since the CULL can drop frames;
	    however if one pipe's CULL drops a particular frame
	    then they all will, so they will all proceed
	    during the same frame.)
	 */
	if (ViewState->invalidateFrame > 0)
	{
	    if (pfGetFrameCount() >= ViewState->invalidateFrame)
	    {
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Frame %d proceeding with invalidate", pfGetFrameCount());
		for (i = minSelected; i <= maxSelected; ++i)
		{
		    pfClipTexture *clipTex = CLIPTEXTURE(i);
		    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Frame %d", pfGetFrameCount());
		    pfNotify(PFNFY_INFO, PFNFY_PRINT, "REALLY REALLY Invalidating clip texture %s",
			     pfGetTexName((pfTexture *)clipTex));

		    pfuReallyInvalidateClipTexture(
			    findThisPipesMasterOrSlaveClipTexture(pfGetChanPipe(chan), clipTex),
			    ViewState->invalidateBarrier);
		    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Done REALLY REALLY Invalidating clip texture %s",
			     pfGetTexName((pfTexture *)clipTex));
		}
		ViewState->invalidateFrame = 0; /* ok for all CULLs to do this, as long as they do it after the barriered pfuReallyInvalidateClipTexture */
	    }
	}
	else if (ViewState->invalidate)
	{
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "Frame %d recieved invalidate, setting invalidate frame to %d", pfGetFrameCount(), pfGetFrameCount()+1);
	    ViewState->invalidate = 0;
	    ViewState->invalidateFrame = pfGetFrameCount()+1; /* ok for more than one CULL process to do this */
	}
#endif /* CUSTOM */

    /*
     * Handle gridify toggle.
     * Only the input process modifies ViewState->gridify, and only the
     * cull process modifies ViewState->gridified (always attempting to make
     * it equal to ViewState->gridify, and doing the actual gridification/
     * ungridification).  This way there are no races
     * and the current state of gridification is unambiguous from the
     * point of view of either process.
     *
     * Gridify should only do master cliptextures; the slaves own no
     * tiles, and invalidating the master will invalidate slaves as well.
     */

#if 1 /* CUSTOM */
    /* XXX this is NOT the right condition!  Typically
       ViewState->masterChan is on pipe 1 (the middle of three)
       but the master clip texture is on pipe 0!
       I suspect there is a race here and we are just lucky it
       usually works out. */
#endif /* CUSTOM */
    if(chan != ViewState->masterChan)
	return;

#if 0 /* PERFLY */
    if (ViewState->gridify != ViewState->gridified)
    {
	int i;
	for (i = pfGetMultipipe()-1; i >= 0; --i)
	{
	    pfPipe *pipe = pfGetPipe(i);
	    int j;
	    for (j = pfGetPipeNumMPClipTextures(pipe)-1; j >= 0; --j)
	    {
		pfClipTexture *cliptex =
		  pfGetMPClipTextureClipTexture(pfGetPipeMPClipTexture(pipe,j));
		if (ViewState->gridified)
		    (void)pfuUnGridifyClipTexture(cliptex);
		else
		    (void)pfuGridifyClipTexture(cliptex);
	    }
	}
	ViewState->gridified = !ViewState->gridified;
    }
#endif /* PERFLY */
#if 1 /* CUSTOM */
	if (ViewState_gridify != ViewState->gridified)
	{
	    for (i = minSelected; i <= maxSelected; ++i)
	    {
		pfClipTexture *cliptex = CLIPTEXTURE(i);
		if (ViewState->gridified)
		{
		    if (_pfuIsGridifiedClipTexture(cliptex))
			(void)pfuUnGridifyClipTexture(cliptex);
		}
		else
		{
		    if (!_pfuIsGridifiedClipTexture(cliptex))
			(void)pfuGridifyClipTexture(cliptex);
		}
	    }
	    ViewState->gridified = !ViewState->gridified;
	}
#endif /* CUSTOM */
    }

#if 1 /* CUSTOM */
#ifdef DEBUG_DTR
    {
	extern volatile int _pfDTRrecordN;
	extern struct {int frame; int loadedLevel; float blurLevel; float minDTRLOD;} _pfDTRrecord[];
	if (ViewState->DTRrecordreset)
	{
	    printf("Resetting DTR recording\n");
	    ViewState->DTRrecordreset = 0;
	    _pfDTRrecordN = 0;
	}

	if (ViewState->DTRrecorddump)
	{
	    const char *DTRrecordFile = "/usr/tmp/DTRrecord";
	    FILE *fp;
	    ViewState->DTRrecorddump = 0;
	    printf("Ending DTR recording, dumping to %s...", DTRrecordFile);
	    fflush(stdout);
	    fp = fopen(DTRrecordFile, "w");
	    if (fp != NULL)
	    {
		int i, n = _pfDTRrecordN;
		for (i = 0; i < n; ++i)
		    fprintf(fp, "%d %d %f %f\n", 
			   (int)_pfDTRrecord[i].frame,
			   (int)_pfDTRrecord[i].loadedLevel,
			   (double)_pfDTRrecord[i].blurLevel,
			   (double)_pfDTRrecord[i].minDTRLOD);
		fclose(fp);
		printf(" wrote %d frames\n", n);
	    }
	    else
		printf("Couldn't open %s: %s\n", DTRrecordFile, strerror(oserror()));
	}
    }
#endif /* DEBUG_DTR */
#endif /* CUSTOM */
}

/* ARGSUSED (suppress compiler warn) */
void
postCull(pfChannel *chan, void *data)
{
    return;
}

/* ARGSUSED (suppress compiler warn) */
void
preLpoint(pfChannel *chan, void *data)
{
   pfCalligraphic *calligraphic = pfGetCurCallig();

   if (calligraphic != NULL && ViewState->calligDirty)
   {
	pfCalligDrawTime(calligraphic,
	    ViewState->calligDrawTime);
	pfCalligFilterSize(calligraphic,
	    ViewState->calligFilterSizeX,
	    ViewState->calligFilterSizeY);
	pfCalligDefocus(calligraphic,
	    ViewState->calligDefocus);
	pfCalligRasterDefocus(calligraphic,
	    ViewState->rasterDefocus);
	pfCalligZFootPrintSize(calligraphic,
	    ViewState->calligZFootPrint);
	ViewState->calligDirty = 0;
   }
   return;
}

/* ARGSUSED (suppress compiler warn) */
void
postLpoint(pfChannel *chan, void *data)
{
    pfCalligraphic *calligraphic = pfGetCurCallig();

    if (calligraphic != NULL)
    {
    }
}

