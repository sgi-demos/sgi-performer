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
 *	perfly.C -- The Performer database fly-thru application
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __sgi
#include <bstring.h>
#endif
#include <math.h>
#include <ctype.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfPipeWindow.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfMPClipTexture.h>
#include <Performer/pf/pfPartition.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfSCS.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfString.h>
#include <Performer/pr/pfFog.h>
#include <Performer/pr/pfLight.h>
#include <Performer/pr/pfMaterial.h>

#include <Performer/pfdu.h>
#include <Performer/pfui/pfiXformer.h>

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
    pfList *mpcliptextures;
    pfCalligraphic *callig;
    int pipe, channel;

    if (chan == ViewState->masterChan)
    {
	pfPipeVideoChannel *pVChan = chan->getPVChan();
	pVChan->getSize(&ViewState->vChanXSize, &ViewState->vChanYSize);
	if (ViewState->dvrXSize <= 0)
	{
	    ViewState->dvrXSize = ViewState->vChanXSize;
	    ViewState->dvrYSize = ViewState->vChanYSize;
	}
    }
    
    /* Is there a Calligraphic channel attached to this Channel ? */
    callig = chan->getCurCallig();
    if (callig != NULL)
    {
	/* this will force SwapReady signal to be active even for a single pipe */
	chan->setCalligEnable(1);
	/* force swap vme twice so calligraphic stays in sync with SwapWin */
	/* we do it more than twice if more than on channel is on that board */
	callig->getChannel(&pipe,&channel);
        pfCalligraphic::swapVME(pipe);
	pfCalligraphic::swapVME(pipe);
    }
    
    mpcliptextures = new(pfGetSharedArena()) pfList(sizeof(pfMPClipTexture*), 1);
    pfuProcessClipCentersWithChannel(ViewState->sceneGroup, mpcliptextures, ViewState->masterChan);
    pfDelete(mpcliptextures);
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


    /* window needed for faster queries */
    win = pfWindow::openNewNoPort("Query", -1);

#ifdef WIN32
#define strncasecmp _strnicmp
#endif

    iR = (strncasecmp(pfGetMachString(), "IR", 2) == 0);

    /* query machine features */
    pfMQueryFeature(querytoks, queryvals);

    win->close();
    pfDelete(win);

    ViewState->haveMultisample = queryvals[0];
    ViewState->haveTagClear = queryvals[1];
    tmp = queryvals[2];

    if (tmp == PFQFTR_FAST)
	ViewState->texture = TRUE;
    else
	ViewState->texture = FALSE;


    /* Set up defaults */
    ViewState->aa		= ViewState->haveMultisample;
    ViewState->fade		= ViewState->haveMultisample;
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
    ViewState->earthSkyColor.set(0.5f, 0.5f, 0.8f, 1.0f);
#else
    ViewState->earthSkyColor.set(0.5f*0.14f, 0.5f*0.22f, 0.5f*0.36f, 1.0f);
#endif
    ViewState->scribeColor.set(1.0f, 1.0f, 1.0f, 1.0f);
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
    ViewState->stats		= PF_ON;
    ViewState->statsEnable	= PFSTATS_OFF;
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
    ViewState->input            = PFUINPUT_NOFORK_X;
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

    ViewState->panScale.set(0.0f, 0.0f, 1.0f); 

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
    ViewState->MCO		= 0;

    ViewState->gridify		= FALSE;
    ViewState->gridified	= FALSE;

    memset(&(ViewState->events), 0, sizeof(ViewState->events));
    memset(&(ViewState->guiWidgets), 0, sizeof(ViewState->guiWidgets));
     
    ViewState->viewMat.makeIdent();
    strcpy(ViewState->initViewNames[0], "Default");
    strcpy(ViewState->tetherViewNames[0], "Under");
    ViewState->tetherViews[0].makeIdent();

    /* special light points */
    ViewState->startCallig = 0;
    ViewState->calligDefocus = 0;
    ViewState->rasterDefocus = 0;
    ViewState->calligFilterSizeX = 1;
    ViewState->calligFilterSizeY = 1;
    ViewState->calligDrawTime = 600;
    ViewState->calligDirty = 1;
    ViewState->calligZFootPrint = 2.;
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
        if (pfCalligraphic::initBoard(i) == TRUE)
        {
            found ++;
            /* get the memory size */
            pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
                "StartCalligraphic: board(%d) has %d Bytes of memory",
                i, pfCalligraphic::getBoardMemSize(i));
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


/*
 * Initialize the view
 */
void
initView(pfScene *scene)
{
    pfSphere	bsphere;

    /* If asked to run for zero frames, exit now */    
    if (ViewState->exitCount == 0)
	pfExit();

    scene->getBound(&bsphere);

    /* Set initial view position from command line values */
    if (InitXYZ)
        ViewState->viewCoord.xyz.copy(ViewState->initViews[0].xyz);

    /* Otherwise fit view to encompass scene */
    else
    {
        ViewState->viewCoord.xyz.copy(bsphere.center);

        /* Offset view so all visible */
        ViewState->viewCoord.xyz[PF_Y] -= ViewState->sceneSize;

        /* Store initial XYZ position for future reset actions */	
        ViewState->initViews[0].xyz.copy(ViewState->viewCoord.xyz);
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
        ViewState->viewCoord.hpr.copy(ViewState->initViews[0].hpr);
    else
    {
	ViewState->viewCoord.hpr.set(0.0f, 0.0f, 0.0f);

	/* Store initial HPR angles for future reset actions */
	ViewState->initViews[0].hpr.copy(ViewState->viewCoord.hpr);
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
    ViewState->sceneDCS = new pfDCS();
    ViewState->sceneGroup = new pfGroup();
    ViewState->sceneDCS->addChild(ViewState->sceneGroup);
    if (ViewState->xformerModel == PFITDF_TRACKBALL)
    {
	scene->addChild(ViewState->sceneDCS);
    }
    else
	scene->addChild(ViewState->sceneGroup);


    /* Load each of the files named on the command line */
    for (i = 0;	i < NumFiles; i++)
    {
	for (j = 0; j < ViewState->iterate; j++)
	{
		// -- Catch any .perfly files here -- //
	    if (strsuffix(DatabaseFiles[i], ".perfly")) {
		if (loadConfigFile(DatabaseFiles[i]))
		    loaded++;
		continue;
	    }
	    
	    /* Load the database. create a hierarchy under node "root" */
	    root = (pfGroup *)pfdLoadFile(DatabaseFiles[i]);
	    
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

		x = ViewState->explode*((random() & 0xffff)/65535.0f - 0.5f);
		y = ViewState->explode*((random() & 0xffff)/65535.0f - 0.5f);
		z = ViewState->explode*((random() & 0xffff)/65535.0f - 0.5f);
		
		/* move the object by placing it beneath an SCS node */
		matrix.makeTrans(x, y, z);
		scs = new pfSCS(matrix);
		scs->addChild(root);
		
		/* go ahead and apply the SCS matrix to the geometry */
		scs->flatten(0);
		
		/* detach (transformed) object from SCS; discard SCS */
		scs->removeChild(root);
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
 		root->flatten(0);
 		
 		/* deinstance and apply all pfSCS transformations */
 		root = pfdCleanTree(root, NULL);

 		/* try a parition for intersections */
		if (ViewState->optimizeTree > 2)
		{
		    pfVec3 spacing;
		    pfSphere sphere;
 		    pfPartition *part = new pfPartition;
		    root->getBound(&sphere);
		    
		    /* use approx 100x100 partition */
		    PFSET_VEC3(spacing, 
			       0.01f*sphere.radius, 
			       0.01f*sphere.radius,
			       0.01f*sphere.radius);

		    part->setAttr(PFPART_ORIGIN, &sphere.center);
		    part->setAttr(PFPART_MIN_SPACING, &spacing);
		    part->setAttr(PFPART_MAX_SPACING, &spacing);

		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			     "Creating pfPartition for %s", 
			     DatabaseFiles[i]);
		    
		    if (pfGetNotifyLevel() >= PFNFY_DEBUG)
			part->setVal(PFPART_DEBUG, 1);

 		    part->addChild(root);
 		    part->build();
 		    part->update();
		    root = part;
		}
	    }
 	    
	    ViewState->sceneGroup->addChild(root);
 	    ++loaded;
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
        pfGeoState *gs = new (pfGetSharedArena()) pfGeoState;
        gs->copy((pfGeoState*)pfdGetDefaultGState());
	scene->setGState(gs);
    }

    /* Compute extent of scene */
    if (loaded)
    {
    	scene->getBound(&bsphere);
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
	pfLightSource *lamp = new pfLightSource();

	/* set direction of infinite light source */
	ViewState->lampXYZ[i].normalize();
	lamp->setPos(ViewState->lampXYZ[i][0],
		     ViewState->lampXYZ[i][1],
		     ViewState->lampXYZ[i][2],
		     0.0f);

	/* set light source color */
	lamp->setColor(PFLT_DIFFUSE,
		       ViewState->lampRGB[i][0],
		       ViewState->lampRGB[i][1],
		       ViewState->lampRGB[i][2]);

	/* add lamp to scene */
	scene->addChild(lamp);
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

    ViewState->cullVol = new pfGeode;
    gset = new(arena) pfGeoSet;
    gstate = new(arena) pfGeoState;

    gset->setPrimType(PFGS_LINESTRIPS);
    gset->setNumPrims(1);
    lengths = (int*)pfMalloc(sizeof(int), arena);
    lengths[0] = 5;
    gset->setPrimLengths(lengths);

    index = (ushort*)pfMalloc(sizeof(ushort)*5, arena);
    for (i=0; i<4; i++)
	index[i] = i;
    index[i] = 0;
    coords = (pfVec3*)pfCalloc(4, sizeof(pfVec3), arena);
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, index);
    colors = (pfVec4*)pfMalloc(sizeof(pfVec4), arena);
    colors[0].set(1.0f, 0.0f, 0.0f, 1.0f);
    gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, index);

    gstate->makeBasic();
    gset->setGState(gstate);
    ViewState->cullVol->addGSet(gset);
    ViewState->cullVolDCS = new pfDCS;
    ViewState->cullVolDCS->setTravMask(PFTRAV_CULL, 0, PFTRAV_SELF, PF_SET);
    ViewState->cullVolDCS->addChild(ViewState->cullVol);
}

    /* Process all the cliptextures in the scene */
    {
	int i;
        pfPipe *masterPipe = pfGetPipe(0);
        pfList *mpcliptextures;

        mpcliptextures = new(pfGetSharedArena()) pfList(
                                   sizeof(pfMPClipTexture*),
                                   1);
        pfuProcessClipCenters(scene, mpcliptextures);
        (void)pfuAddMPClipTexturesToPipes(mpcliptextures, masterPipe, NULL);
	pfDelete(mpcliptextures);
	
        
	/* set cliptexture incremental state to the visual channel - GUI is chan 0 */
	for(i = pfGetMultipipe() - 1; i >= 0; i--)
	{
	    pfPipe *p = pfGetPipe(i);
	    if(p == masterPipe)
		p->setIncrementalStateChanNum(1);
	    else
		p->setIncrementalStateChanNum(0);
	}
    }

    return(loaded);
}

/*
 * Pre-Frame - latency critical operations
 */
void
localPreFrame(pfChannel *)
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
	    pvchan = chan->getPVChan();

	    if (pvchan) 
	    {
		newMode = (pvchan->getDVRMode() != dvrMode);
		if (newMode)
		{
		    /* must also set REDRAW_WINDOW for each pipe with redrawOverlay */
		    ViewState->redrawOverlay	= 1;
		    ViewState->redrawCount	= 0;
		}

		switch (dvrMode)
		{
		case DVR_OFF:
		    if (newMode)
		    {
			int pipeId = pfGetId(chan->getPipe());
			ViewState->drawFlags[pipeId] |= REDRAW_WINDOW;
			pvchan->setDVRMode(PFPVC_DVR_OFF);
			chan->setProjMode(PFCHAN_PROJ_VIEWPORT);
			pfuCursorType(PFU_CURSOR_X);
			if (ViewState->gui && (chan == ViewState->masterChan))
			    pfuRedrawGUI();
		    }
		    break;
		case DVR_MANUAL:
		    /* set pfPipeVideoChannel scale factor for next draw */
		    {
			if (newMode)
			{
			    pvchan->setDVRMode(PFPVC_DVR_MANUAL);
			    /* pvchan->setStressFilter(1.0f, 0.8f, 0.95f, 0.0f, 1.0f, 1.0f); */
			    pfuCursorType(PFU_CURSOR_2D);
			}
			pvchan->getOutputSize(&oxs, &oys);
			newSize =  (newMode || (oxs != ViewState->dvrXSize) || (oys != ViewState->dvrYSize));
			if (newSize)
			{
			    int pipeId = pfGetId(chan->getPipe());
			    ViewState->drawFlags[pipeId] |= REDRAW_WINDOW;
			    if (ViewState->dvrXSize > ViewState->vChanXSize)
				ViewState->dvrXSize = ViewState->vChanXSize;
			    if (ViewState->dvrYSize > ViewState->vChanYSize)
				ViewState->dvrYSize = ViewState->vChanYSize;
			    pvchan->setOutputSize(ViewState->dvrXSize, ViewState->dvrYSize);
			    chan->setProjMode(PFCHAN_PROJ_WINDOW);
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
			int pipeId = pfGetId(chan->getPipe());
			ViewState->drawFlags[pipeId] |= REDRAW_WINDOW;
			if (newMode)
			{
			    float frac=1.0f;
			    if (!Multipipe)
				frac = 1.0f / (float) NumChans;
			    pvchan->setDVRMode(PFPVC_DVR_AUTO);
			    pvchan->setStressFilter(frac, 0.80, 0.95f, 1.5f, 2.0f, 10.0f);
			    /* indicates that pfPipe should calc stress automatically */
			    pfuCursorType(PFU_CURSOR_2D);
			    chan->setProjMode(PFCHAN_PROJ_WINDOW);
			}
			if (ViewState->gui && (chan == ViewState->masterChan))
			    pfuRedrawGUI();
			
		    }
		    break;
		}
		pvchan->getOutputSize(&ViewState->dvrXSize, &ViewState->dvrYSize);
	    }
	}
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
	ViewState->vehicleDCSs[ViewState->curVehicle]->getMat(mat);
	    /* adjust for pfuPath rotating the object CCW */
	mat.preRot(-90.0, 0.0, 0.0, 1.0, mat);

	mat.preMult(ViewState->tetherViews[ViewState->curTetherView]);

	ViewState->xformer->setMat(mat);
    }
    else
    {
	ViewState->xformer->update();
	ViewState->xformerModel = ViewState->xformer->getCurModelIndex();
    }
    
    

    /* if have moving-eyepoint motion model, update eyepoint */
    if ((ViewState->xformer->getCurModel()->isOfType(pfiInputXformTravel::getClassType()))
       || (ViewState->xformerModel == PFITDF_TETHER))
    {
	ViewState->xformer->getCoord(&ViewState->viewCoord);
	ViewState->xformer->getMat(mat);
	chan->setViewMat(mat);
	ViewState->viewMat = mat;

	/* move the sun to the eyepoint */
	if (ViewState->lighting ==  LIGHTING_EYE)
	    ViewState->sunDCS->setMat(ViewState->viewMat);
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
		frust = new pfFrustum;

	    ViewState->cullVol->getGSet(0)->getAttrLists(PFGS_COORD3, 
						(void**)&coords, &foo);

	    ViewState->masterChan->getBaseFrust(frust);
	    frust->getNearFar(&nearPlane, &farPlane);
	    frust->getNear(coords[0], coords[1], coords[3], coords[2]); 
	    
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

	    frust->setNearFar(nearPlane*fudge, farPlane);
	    frust->makePersp(coords[0][0], coords[2][0], 
			            coords[0][2], coords[2][2]); 

	    ViewState->masterChan->setCullPtope((pfPolytope*)frust);

	    if (prevCullDelta >= 1.0f)
		ViewState->scene->addChild(ViewState->cullVolDCS);
	}
	else	
	{
	    /* Don't draw culling box */
	    if (prevCullDelta < 1.0f)
		ViewState->scene->removeChild(ViewState->cullVolDCS);

	    /* Disable special cull region */
	    ViewState->masterChan->setCullPtope((pfPolytope*)NULL);
	}

	prevCullDelta = ViewState->cullDelta;
    }

    /* Place cull volume geometry in front of viewer */
    if (ViewState->cullDelta < 1.0f)	
    {
	pfMatrix 		mat;
	chan->getOffsetViewMat( mat);
	ViewState->cullVolDCS->setMat(mat);
    }
    }
    
}

/*
 * Reset view and Xformer to original XYZ position and HPR Euler angles
 */
void
resetPosition(int pos)
{
    pfMatrix	mat;

    ViewState->xformer->stop();
    switch (pos)
    {
    case POS_CENTER:
	ViewState->xformer->center();
	break;

    default:
    case POS_ORIG:
	ViewState->xformer->setCoord(&(ViewState->initViews[pos]));
	break;
    }
    
    ViewState->xformer->getMat(ViewState->viewMat);
    ViewState->masterChan->setViewMat(ViewState->viewMat);
}

static void
updateWin(int pipeId)
{
    static int	winIndex = PFWIN_GFX_WIN;
    int 	doWinIndex = 0;
    int		i;

    /* Select proper visual in the window stack to render to  */
    if (ViewState->stats && (ViewState->statsEnable & PFSTATSHW_ENGFXPIPE_FILL))
    {
	pfPipeVideoChannel *pvchan;
	ViewState->doDVR = PFPVC_DVR_OFF;
	for (i=0; i < ViewState->numChans; i++)
	{
	    pvchan = ViewState->chan[i]->getPVChan();
	    pvchan->setDVRMode(PFPVC_DVR_OFF);
	}
	ViewState->doDVR = PFPVC_DVR_OFF;
	pvchan->setDVRMode(PFPVC_DVR_OFF);
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
	int pipeCount = Multipipe != HYPERPIPE ? NumPipes : 1;
	for (i=0; i < pipeCount; i++)
	{
	    ViewState->pw[i]->setIndex(winIndex);
	    if (!(ViewState->pw[i]->getSelect()))
	    {
		ViewState->pw[i]->setConfigFunc(ConfigPWin);
		ViewState->pw[i]->config();
	    }
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
		    fsp = chan->getFStats();
		    /* put everything back to defaults */
		    fsp->setClass(PFSTATS_ALL, PFSTATS_DEFAULT); 
		    /* set the fast process timing stats */
		    fsp->setClassMode(PFFSTATS_PFTIMES, PFFSTATS_PFTIMES_BASIC, PFSTATS_SET);
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

	    /* Set new stats enables */
	    fsp = chan->getFStats();

	    /* First turn off old stats */
	    if (en)
		fsp->setClass(en & ~(PFSTATSHW_ENGFXPIPE_TIMES), PFSTATS_OFF);
	    if (ViewState->statsEnable & PFSTATSHW_ENGFXPIPE_FILL)
		fsp->setClass(PFSTATSHW_ENGFXPIPE_TIMES, PFSTATS_OFF);

	    /* Now enable new stats graph */
            if (ViewState->statsEnable)
	    {
	        fsp->setClass(ViewState->statsEnable, PFSTATS_ON);
		fsp->setClassMode(PFFSTATS_PFTIMES, PFFSTATS_PFTIMES_MASK, PFSTATS_ON);
		if (ViewState->statsEnable & PFSTATSHW_ENGFXPIPE_TIMES) /* do just the status line */
		    chan->setStatsMode(PFCSTATS_DRAW, 0);
		else
		    chan->setStatsMode(PFCSTATS_DRAW, PFSTATS_ALL);
		if (first)
		{       /* set some stats class modes that are not
		     * on by default. this only needs to be done
		     * the first time */
		    fsp->setClassMode(PFSTATSHW_GFXPIPE_FILL,
				      PFSTATSHW_GFXPIPE_FILL_DEPTHCMP |
				      PFSTATSHW_GFXPIPE_FILL_TRANSPARENT,
				      PFSTATS_ON);
		    /*
		     * This will also enable the tmesh statistics - not on by
		     * default because they are somewhat expensive
		     */
		    fsp->setClassMode(PFSTATS_GFX,
				      PFSTATS_GFX_ATTR_COUNTS |
				      PFSTATS_GFX_TSTRIP_LENGTHS,
				      PFSTATS_ON);
		    fsp->setAttr(PFFSTATS_UPDATE_SECS, 2.0f);
		    callig = chan->getCurCallig();
		    if (callig != NULL)
		        fsp->setClass(PFSTATS_ENCALLIG, PF_ON);
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
	    fsp = ViewState->masterChan->getFStats();
	    pfPrint(fsp, PFFSTATS_BUF_AVG | PFFSTATS_BUF_PREV |
		    fsp->getClass(PFSTATS_ALL),
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
    int pipeId;
    
    pipeId = pfGetId(chan->getPipe());
    
    /* Process keyboard input */
    processKeybdInput();

    /* Adjust load management filter */
    if (ViewState->stress)
	chan->setStressFilter(1.0f, ViewState->lowLoad, ViewState->highLoad,
	    ViewState->stressScale, ViewState->stressMax);
    else
	chan->setStressFilter(1.0f, ViewState->lowLoad, ViewState->highLoad,
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

    if ((ViewState->cullMode != -1)&&
	    (ViewState->cullMode !=
	     ViewState->masterChan->getTravMode(PFTRAV_CULL)))
	ViewState->masterChan->setTravMode(PFTRAV_CULL, ViewState->cullMode);

    if (ViewState->LODscale != ViewState->masterChan->getLODAttr(PFLOD_SCALE))
	ViewState->masterChan->setLODAttr(PFLOD_SCALE, ViewState->LODscale);


    { /* remove light source from scene when lighting is off */
	static int lightInScene = 1;
	if ((ViewState->lighting != 0) != lightInScene)
	{
	    if (ViewState->lighting)
	    {
		ViewState->scene->addChild(ViewState->sunDCS);
		lightInScene = 1;
	    }
	    else
	    {
		ViewState->scene->removeChild(ViewState->sunDCS);
		lightInScene = 0;
	    }
	}
    }

    updateTimeOfDay(ViewState->timeOfDay);
    updateStats();
    updateEnvironment();
    updateWin(pipeId);

    ViewState->masterChan->getViewMat(ViewState->viewMat);
    ViewState->masterChan->getFOV(&ViewState->fov[0],
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

    if (model == PFITDF_FLY)
    {
    	ViewState->xformer->selectModel(model);
        pfiInputXformFly *fly = (pfiInputXformFly*)ViewState->xformer->getCurModel();
	fly->setMode(PFIXFLY_MODE_PITCH, PFIXFLY_PITCH_FOLLOW);
    }
    else if (model == PFITDF_FLIGHTSTICK)
    {
        ViewState->xformer->selectModel(PFITDF_FLY);
        pfiInputXformFly *fly = (pfiInputXformFly*)ViewState->xformer->getCurModel();
	fly->setMode(PFIXFLY_MODE_PITCH, PFIXFLY_PITCH_FLIGHT);
    }
    else ViewState->xformer->selectModel(model);

    /* If not in trackball model, remove trackball DCS for better performance */
    if ((oldModel != model) && (oldModel == PFITDF_TRACKBALL))
    {
	ViewState->scene->removeChild(ViewState->sceneDCS);
	ViewState->scene->addChild(ViewState->sceneGroup);
    }
    /* Restore trackball DCS to scene */
    else if ((oldModel != model) && (model == PFITDF_TRACKBALL))
    {
	ViewState->scene->removeChild(ViewState->sceneGroup);
	if (ViewState->sceneDCS->getNumParents() == 0)
	    ViewState->scene->addChild(ViewState->sceneDCS);
    }
 
    /* Collide with objects in scene */
    if (oldCollMode != collideMode)
    {
	if (collideMode == PF_ON)
	    ViewState->xformer->enableCollision();
	else
	    ViewState->xformer->disableCollision();
	oldCollMode = collideMode;
    }
    
    oldModel = model;
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
    
    str = new((void*)NULL) pfString;
    str->setMode(PFSTR_JUSTIFY, PFSTR_MIDDLE);
    str->setFont(fnt);
    str->setColor(1.0f, 0.0f, 0.8f, 1.0f);
    str->setString(ViewState->welcomeText);
    mat.makeScale(1.0f, 1.0f, 1.5f);
    str->setMat(mat);
    str->flatten();
    bbox = str->getBBox();
    dist = PF_MAX2(bbox->max[PF_Z] - bbox->min[PF_Z],
		   bbox->max[PF_X] - bbox->min[PF_X]);

    dist = (dist/2.0f) / pfTan(45.0f/2.0f);

    frust = new pfFrustum();

    if (InitFOV) {
	frust->setNearFar(1.0f, 1000.0f);
	frust->makePersp(
		- pfTan (ViewState->fov[0]/2.0f),
		  pfTan (ViewState->fov[0]/2.0f),
		- pfTan (ViewState->fov[1]/2.0f),
		  pfTan (ViewState->fov[1]/2.0f) );
    } else
	frust->makeSimple(45.0f);

    frust->apply();
    pfDelete(frust);

    pfRotate(PF_X, -90);

    pfEnable(PFEN_LIGHTING);
    lt = new pfLight();
    lt->setPos(.2f, -1.0f, 1.0f, 0.0f);
    lt->on();

    start = pfGetTime();

    /* Twirl text for 0.25f seconds */
    do
    {
	pfClear(PFCL_COLOR | PFCL_DEPTH, NULL);
	
	t = (pfGetTime() - start) / 0.25f;
	t = PF_MIN2(t, 1.0f);

	mat.makeRot((1.0f - 0.5f*t) * -90.0f, 
		     1.0f, 0.0f, 0.0f); 

	t *= t;
	mat.postTrans(mat, 
		       0.0f, 
 		       3.0f * t * dist +  6.0f * (1.0f - t) * dist,
		       0.0f);

	pfPushMatrix();
	pfMultMatrix(mat);
	str->draw();
	pfPopMatrix();

	pw->swapBuffers();

    } while (t < 1.0f);

    // Get image in both front and back buffers
    pfClear(PFCL_COLOR | PFCL_DEPTH, NULL);
    pfPushMatrix();
    pfMultMatrix(mat);
    str->draw();
    pfPopMatrix();

    pw->swapBuffers();

    lt->off();

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

    if (Multipipe == HYPERPIPE) return;

    pfPushState();
    pfBasicState();

    Overlay = ((pw->getOverlayWin()) ? 1 : 0);

    if ((ViewState->welcomeText[0] == '\0') || (!(pfFindFile("Times-Elfin.of", path, R_OK))))
    {
    	// Why is this repeated?
	pfClear(PFCL_COLOR, NULL);
	/* Message will be draw in in purple - the pfuDrawMessage() color */
	pfuDrawMessage(NULL, msg, PFU_MSG_PIPE, PFU_CENTER_JUSTIFIED,
	    0.5f, 0.5f, PFU_FONT_BIG, PFU_RGB);
	pw->swapBuffers();

	pfClear(PFCL_COLOR, NULL);
	/* Message will be draw in in purple - the pfuDrawMessage() color */
	pfuDrawMessage(NULL, msg, PFU_MSG_PIPE, PFU_CENTER_JUSTIFIED,
	    0.5f, 0.5f, PFU_FONT_BIG, PFU_RGB);
	pw->swapBuffers();
    }
    else
	drawMessage(pw);

    pfPopState();
}

static void
snapImage(int snapAlpha)
{
    FILE 		*fp;
    static char 	str[80];
    static int 		count = 0;
    pfPipe* 		cpipe = ViewState->masterChan->getPipe();
    int    		id = pfGetId(cpipe)*10 + count++;

    sprintf(str, "perfly.%d.%s", id,
	    (snapAlpha) ? "rgba" : "rgb");
    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Saving pipe %d image in file %s\n",
	    pfGetId(cpipe), str);

    pfuSaveImage(str, 0, 0,
		 ViewState->mouse.winSizeX,
		 ViewState->mouse.winSizeY,
		 (snapAlpha) ? 1 : 0);

    /* create info file to go with image */
    sprintf(str, "perfly.%d.info", id);
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
	ViewState->masterChan->getFOV(&h, &v);
	fprintf(fp, "FOV: horiz=%f vert=%f\n", h, v);
	fclose(fp);
    }

    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Done\n");
}


static void
drawOverlayName(pfPipeWindow *pw)
{
    static int 	mapped = 0;
    pfWindow 	*pwOver = pw->getOverlayWin();

    pfPushState();
    pfBasicState();

    
    if(ViewState->doDVR)
    {
	/* set the viewport to the window to completely clear the overlays */
	int xs, ys;
	pw->getSize(&xs, &ys);
	glViewport(0, 0, xs, ys);
    }

    {
	if (pwOver)
	    pwOver->select();
	else
	    return;
    }

	pfClear(PFCL_COLOR, NULL);
	
    if(!ViewState->doDVR)
    {

    if (!mapped)
    {
	static pfVec3 clrs[2] =
 	{
 	    pfVec3(0.4f, 0.0f, 0.5f), 	   /* tex - purple */
 	    pfVec3(0.125f, 0.06f, 00.125f) /* shadow - drk grey */
        };
	pfuMapWinColors(pwOver, clrs, 1, 2);
	mapped = 1;
    }	
    pfuDrawMessageCI(ViewState->masterChan,
	    ViewState->overlayText, PFU_MSG_CHAN, PFU_RIGHT_JUSTIFIED,
	    1.0f, 1.0f, PFU_FONT_SMALL, 1, 2);
	    	    

    }

    pw->select();

    pfPopState();
}

/* convey draw style from localPreDraw to localPostDraw */
static int selectedDrawStyle = 0;

void
localPreDraw(pfChannel *chan, void *)
{
    pfPipe 		*pipe = chan->getPipe();
    pfPipeWindow 	*pw = chan->getPWin();
    int 		 pipeId = pfGetId(pipe);
    
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

void
localPostDraw(pfChannel *chan, void *)
{
    pfPipe *mPipe, *cPipe;
    pfPipeWindow 	*pw = chan->getPWin();
    int 		 pipeId;
    
    /* handle draw style */
    pfuPostDrawStyle(selectedDrawStyle);

    mPipe = ViewState->masterChan->getPipe();
    cPipe = chan->getPipe();
    pipeId = pfGetId(cPipe);

    
    if (ViewState->exitCount == 0)
	ViewState->exitFlag = TRUE;

    if (ViewState->exitCount > 0)
	ViewState->exitCount--;

    if (ViewState->printStats > 0)
	ViewState->printStats--;

    /* take snapshot of image */
    if (ViewState->snapImage)
    {
	snapImage(ViewState->snapImage == 2);
	ViewState->snapImage = 0;
    }

    /* Handle REDRAW event */
    if (ViewState->drawFlags[pipeId] & REDRAW_WINDOW)
    {
	/* master channel controls master pipe */
	if (pipeId == pfGetId(ViewState->masterChan->getPipe()))
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
		    ViewState->drawFlags[pipeId] &= ~REDRAW_WINDOW;
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

    if (cPipe == mPipe)
    {
	/* draw scene-graph hierarchy */
	if (ViewState->tree && chan == ViewState->masterChan)
	    pfuDrawTree(chan, (pfNode*)ViewState->scene, ViewState->panScale);
	
	/* draw 2D cursor */
	if ((pfuGetCursorType() == PFU_CURSOR_2D) && ViewState->mouse.inWin)
	{
	    pfPipeWindow *pw = chan->getPWin();
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
    ViewState->xformer = new(pfGetSharedArena()) pfiTDFXformer;

    ViewState->xformer->setAutoInput(ViewState->masterChan,
	&ViewState->mouse, &ViewState->events);

    ViewState->xformer->setNode(ViewState->sceneGroup);
    ViewState->xformer->setAutoPosition(NULL, ViewState->sceneDCS);
    ViewState->xformer->setResetCoord(&(ViewState->initViews[0]));
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
	    for (j = pipe->getNumMPClipTextures()-1; j >= 0; --j)
	    {
		pfClipTexture *cliptex =
		  pipe->getMPClipTexture(j)->getClipTexture();
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
	calligraphic->setDrawTime(ViewState->calligDrawTime);
	calligraphic->setFilterSize(ViewState->calligFilterSizeX,
	    ViewState->calligFilterSizeY);
	calligraphic->setDefocus(ViewState->calligDefocus);
	calligraphic->setRasterDefocus(ViewState->rasterDefocus);
	calligraphic->setZFootPrintSize(ViewState->calligZFootPrint);
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

