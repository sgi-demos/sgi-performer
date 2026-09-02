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
#include <Performer/pfui.h>

#include "generic.h"
#include "perfly.h"
#include "gui.h"
#include "keybd.h"

#include <GL/glu.h>

#define streq(a,b) (strcmp(a,b) == 0)
#define strsuffix(s, suf) \
	(strlen(s) >= strlen(suf) && streq(s + (strlen(s)-strlen(suf)), suf))

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
    int		queryvals[5];
    static int querytoks[5] = {PFQFTR_MULTISAMPLE, 
			       PFQFTR_TAG_CLEAR, 
			       PFQFTR_TEXTURE, 
			       PFQFTR_STIPPLE,
			       0};

    /* window needed for faster OpenGL queries */
    win = pfOpenNewNoPortWin("Query", -1);

#ifdef WIN32
#define strncasecmp _strnicmp
#endif
    iR = (strncasecmp(pfGetMachString(), "IR", 2) == 0);

    /* query machine features */
    pfMQueryFeature(querytoks, queryvals);

    pfCloseWin(win);
    pfDelete(win);
    
    tmp = queryvals[3];
    if(tmp == PFQFTR_FAST)
      ViewState->haveFastStipple = TRUE;
    else 
      ViewState->haveFastStipple = FALSE;

    ViewState->haveMultisample = queryvals[0];
    ViewState->haveTagClear = queryvals[1];
    tmp = queryvals[2];


    if (tmp == PFQFTR_FAST)
	ViewState->texture = TRUE;
    else
	ViewState->texture = FALSE;

    /* Set up defaults */
    ViewState->aa		= ViewState->haveMultisample;
    ViewState->fade		= ViewState->haveMultisample | ViewState->haveFastStipple;
    for (i = 0; i < MAX_PIPES; ++i)
	ViewState->visualIDs[i]	= -1;

    ViewState->exitFlag		= FALSE;
    ViewState->updateChannels	= TRUE;
    ViewState->gui		= TRUE;
    ViewState->guiFormat	= 0; /* marks unset */
    ViewState->rtView		= TRUE;
    ViewState->phase		= PFPHASE_FREE_RUN;
    ViewState->frameRate	= 72.0f;
    ViewState->backface		= PF_ON;
    ViewState->collideMode	= PF_ON;
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
#if defined(__linux__) && !defined(__linux__has_MP__)
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
    ViewState->LODscale		= 1.0f;
    ViewState->lamps		= 0;
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

    ViewState->viewportOutline	= FALSE;
    ViewState->compositorMode	= PFCOMP_2x2;

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

    ViewState -> optimizationMask = PFSK_FRUSTUM_CULL | PFSK_BACKFACE_CULL;
    ViewState -> sidekickPolicy = PFSK_CULL_DONE;
    ViewState -> sidekickParamDirty = 1; 
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
	/* Calculate near, far based on scene extent */
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

int
initSceneGraph(pfScene *scene)
{
    pfNode     *root;
    int		i;
    int		j;
    int		loaded	= 0;
    pfSphere    bsphere;

    /* Create a DCS for TRACKBALL pfiXformer */
    ViewState->sceneDCS = pfNewDCS();
    ViewState->sceneGroup = pfNewGroup();
    pfAddChild(ViewState->sceneDCS, ViewState->sceneGroup);
    if (ViewState->xformerModel == PFITDF_TRACKBALL)
    {
	pfAddChild(scene, ViewState->sceneDCS);
    }
    else
	pfAddChild(scene, ViewState->sceneGroup);

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
		pfGroup *group;
		
#ifdef WIN32
/* XXX Alex -- don't forget to initialize random number generator ... */
#define random rand
#endif
		x = ViewState->explode*((float)(random()&0xffff)/65535.f - .5f);
		y = ViewState->explode*((float)(random()&0xffff)/65535.f - .5f);
		z = ViewState->explode*((float)(random()&0xffff)/65535.f - .5f);
		
		/* move the object by placing it beneath an SCS node */
		pfMakeTransMat(matrix, x, y, z);
		scs = pfNewSCS(matrix);
		
		/* Add a group which will hold the SCS. SCS's should not be
		   the root node of a scenegraph that is flattened */
		group = pfNewGroup();
		pfAddChild(group, scs);
		pfAddChild(scs, root);
		
		/* go ahead and apply the SCS matrix to the geometry */
		pfFlatten(group, 0);

		/* Get new root */
		root = pfGetChild(group, 0);
		
		/* remove the new root from the group */
		pfRemoveChild(group, root);
		/* delete the group */
		pfDelete(group);
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
	    while(fscanf(pathfp, "%f %f %f %f %f %f",
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

	pfiXformerMat(ViewState->xformer, mat);
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
	    pfiGetXformerMat(ViewState->xformer, mat);
	    pfChanViewMat(chan, mat);
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
    }
    }
}

/*
 * Reset view and Xformer to original XYZ position and HPR Euler angles
 */
void
resetPosition(int pos)
{
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
    pfiGetXformerMat(ViewState->xformer, ViewState->viewMat);
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
		pfFStatsClass(fsp, ViewState->statsEnable, PFSTATS_ON);
		pfFStatsClassMode(fsp, PFFSTATS_PFTIMES, PFFSTATS_PFTIMES_MASK, PFSTATS_ON);
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

    if (ViewState -> sidekickParamDirty)
    {
	ViewState -> sidekickParamDirty = 0;
	pfMultithreadParami(-1, PFSK_OPTIMIZATION, ViewState->optimizationMask);
	pfMultithreadParami(-1, PFSK_POLICY, ViewState->sidekickPolicy);
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

    if (model == PFITDF_FLY)
    {
    	pfiInputXformFly *fly; 
    	pfiSelectXformerModel(ViewState->xformer, model);
	fly = (pfiInputXformFly*) pfiGetXformerCurModel(ViewState->xformer);
	pfiIXformFlyMode(fly, PFIXFLY_MODE_PITCH, PFIXFLY_PITCH_FOLLOW);
    }
    else if (model == PFITDF_FLIGHTSTICK)
    {
    	pfiInputXformFly *fly; 
    	pfiSelectXformerModel(ViewState->xformer, PFITDF_FLY);
	fly = (pfiInputXformFly*) pfiGetXformerCurModel(ViewState->xformer);
	pfiIXformFlyMode(fly, PFIXFLY_MODE_PITCH, PFIXFLY_PITCH_FLIGHT);
    }
    else pfiSelectXformerModel(ViewState->xformer, model);

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
#ifndef WIN32
	static pfVec3 clrs[2] = { 
	            {0.4f, 0.0f, 0.5f},		/* tex - purple */
                    {0.125f, 0.06f, 00.125f}};	/* shadow - drk grey */
	pfuMapWinColors(pwOver, clrs, 1, 2);
	mapped = 1;
#else
	// color indexing on PCs? I don't think so ... 
        pfNotify(PFNFY_WARN,PFNFY_PRINT,
                 "pfuMapWinColors() not implemented in libpfutil on win32");
#endif
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
    
    if ((ViewState->firstPipeDraw[pipeId]) && 
	(ViewState->drawMode == DLIST_COMPILE_GEOSET))
    {
	pfuTravCompileDLists(ViewState->sceneGroup, PFU_ATTRS_NONE);
	ViewState->firstPipeDraw[pipeId] = 0;
    }

    if (ViewState->stats && 
	(ViewState->statsEnable & PFSTATSHW_ENGFXPIPE_FILL))
    {
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

	if(ViewState->viewportOutline) /* for debugging compositor support */
        {
            float color[4][4] = { { 1.0, 0.0, 0.0, 1.0 }, /* 0, red */
                                  { 0.0, 1.0, 0.0, 1.0 }, /* 1, green */
                                  { 1.0, 1.0, 0.0, 1.0 }, /* 2, yellow */
                                  { 0.0, 0.0, 1.0, 1.0 }  /* 3, blue */
                                };
            int pipe = pfGetPipeNum();

            glDisable( GL_DEPTH_TEST );
            glDisable( GL_BLEND );
            glMatrixMode( GL_PROJECTION );
            glPushMatrix();
            glLoadIdentity();
            glMatrixMode( GL_MODELVIEW );
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0 );

            glColor4fv(color[pipe]);

            glLineWidth( 12.0 );

            glBegin(GL_LINE_STRIP);
            glVertex3f( 0.0, 0.0, 0.0 );
            glVertex3f( 1.0, 0.0, 0.0 );
            glVertex3f( 1.0, 1.0, 0.0 );
            glVertex3f( 0.0, 1.0, 0.0 );
            glVertex3f( 0.0, 0.0, 0.0 );
            glEnd();
            glMatrixMode( GL_MODELVIEW );
            glPopMatrix();
            glMatrixMode( GL_PROJECTION );
            glPopMatrix();
            glEnable( GL_DEPTH_TEST );
        }
}
	

void
initXformer(void)
{
    ViewState->xformer = pfiNewTDFXformer(pfGetSharedArena());

    pfiXformerAutoInput(ViewState->xformer, ViewState->masterChan,
	&ViewState->mouse, &ViewState->events);

    pfiXformerNode(ViewState->xformer, ViewState->sceneGroup);
    pfiXformerAutoPosition(ViewState->xformer, NULL, ViewState->sceneDCS);
    pfiXformerResetCoord(ViewState->xformer, &ViewState->initViews[0]);
    xformerModel(ViewState->xformerModel, ViewState->collideMode);

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
    return;
}

/* ARGSUSED (suppress compiler warn) */
void
postApp(pfChannel *chan, void *data)
{
    return;
}

/* ARGSUSED (suppress compiler warn) */
void
preCull(pfChannel *chan, void *data)
{
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
    if(chan != ViewState->masterChan)
	return;

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

