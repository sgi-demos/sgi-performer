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
 * file: gui.c
 * --------------
 * $Revision: 1.6 $
 * $Date: 2002/07/23 00:21:44 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfui.h>
#include <math.h>
#include "main.h"
#include "perfly.h"
#include "gui.h"

#define streq(a,b) (strcmp(a,b) == 0)
#define numberof(things) (sizeof(things)/sizeof(*(things)))

/*-----------------------------------------------------------*/

static void initPanel(void);
static void updatePanel(void);

static pfuGUIString styleStrs[] = 
{
    "Points",
    "Lines", 
    "Dashed",
    "Haloed",
    "Hidden",
    "Filled",
    "Scribed"
    };
static int styleVals[] = 
{ 
    PFUSTYLE_POINTS,
    PFUSTYLE_LINES,
    PFUSTYLE_DASHED,
    PFUSTYLE_HALOED,
    PFUSTYLE_HIDDEN,
    PFUSTYLE_FILLED,
    PFUSTYLE_SCRIBED,
};

static pfuGUIString textureStrs[] = 
{
    "Off",
    "Default",
};

static int textureVals[] = {
    0,
    1,
};

static pfuGUIString lightingStrs[] = 
{
    "Off", 
    "Eye", 
    "Sun", 
};

static int lightingVals[] = {
    LIGHTING_OFF, 
    LIGHTING_EYE, 
    LIGHTING_SUN, 
};

static pfuGUIString xformerStrs[] = 
{
    "Drive", 
    "Fly",
    "Flight Stick",
    "Tether"	/* option only appears if numVehicles > 0 */
};
static int xformerVals[] =
{
    PFITDF_DRIVE, 
    PFITDF_FLY,
    PFITDF_FLIGHTSTICK,
    PFITDF_TETHER
};
 
static pfuGUIString eskyStrs[] = 
{
    "Sky/Clear",
    "Sky/Grnd",
    "Sky", 
    "Clear",
    "Tag"
    };
static int eskyVals[] = 
{ 
    PFES_SKY_CLEAR,
    PFES_SKY_GRND,
    PFES_SKY,
    PFES_FAST,
    PFES_TAG
    };

static pfuGUIString cullStrs[] = 
{
    "All", 
    "View",
    "View, GSet",
    "Sort", 
    "ignore lsources", 
    "View, Sort", 
    "View, GSet, Sort", 
    "View (no lsources)",
    "Sort (ignore lsources)", 
    "View, GSet (ignore lsources)",
    "View, Sort (ignore lsources)", 
    "View, GSet, Sort (ignore lsources)", 
    "None"
    };
static int cullVals[] = 
{ 
    PFCULL_ALL,
    PFCULL_VIEW,
    PFCULL_VIEW | PFCULL_GSET,
    PFCULL_SORT, 
    PFCULL_IGNORE_LSOURCES, 
    PFCULL_VIEW               | PFCULL_SORT, 
    PFCULL_VIEW | PFCULL_GSET | PFCULL_SORT, 
    PFCULL_VIEW                             | PFCULL_IGNORE_LSOURCES,
    PFCULL_SORT | PFCULL_IGNORE_LSOURCES,
    PFCULL_VIEW | PFCULL_GSET               | PFCULL_IGNORE_LSOURCES,
    PFCULL_VIEW               | PFCULL_SORT | PFCULL_IGNORE_LSOURCES,
    PFCULL_VIEW | PFCULL_GSET | PFCULL_SORT | PFCULL_IGNORE_LSOURCES,
    0
    };

static pfuGUIString highStrs[] = 
{
    "Lines", 
    "Line BBox", 
    "Fill BBox", 
    "Normals",
    "Points", 
    "Fill",  
    "Pattern",  
    "Texture"  
    };
static int highVals[] = 
{
    PFHL_LINES,
    PFHL_BBOX_LINES | PFHL_LINES,
    PFHL_BBOX_LINES | PFHL_BBOX_FILL | PFHL_LINES_R | PFHL_SKIP_BASE,
    PFHL_NORMALS,
    PFHL_POINTS,
    PFHL_FILL | PFHL_LINES | PFHL_LINES_R | PFHL_SKIP_BASE,
    PFHL_FILLPAT | PFHL_LINESPAT,
    PFHL_FILLTEX
    };

static pfuGUIString fogStrs[] = 
{
    "Off", 
    "Linear",  
    "Exp", 
    "Exp^2", 
    "Spline",
    "Layered",
    "Col Layer",
    "Patchy",
    "Combined",
    };
static int fogVals[] = 
{
    PFFOG_OFF,
    PFFOG_PIX_LIN,
    PFFOG_PIX_EXP,
    PFFOG_PIX_EXP2,
    PFFOG_PIX_SPLINE,
    PFVFOG_LAYERED_FOG,
    LAYERED_MULTICOLORED_FOG,
    PFVFOG_PATCHY_FOG,
    PFVFOG_PATCHY_FOG | PFVFOG_LAYERED_FOG,
    };

static pfuGUIString layeredFogModeStrs[] = 
{
    "Linear",  
    "Exp", 
    "Exp^2", 
    };
static int layeredFogModeVals[] = 
{
    PFVFOG_LINEAR,
    PFVFOG_EXP,
    PFVFOG_EXP2,
};

static pfuGUIString patchyFogModeStrs[] = 
{
    "Linear",  
    "Exp", 
    "Exp^2", 
    };
static int patchyFogModeVals[] = 
{
    PFVFOG_LINEAR,
    PFVFOG_EXP,
    PFVFOG_EXP2,
};

#define NUM_STATSVALS 4
static pfuGUIString statsStrs[] = 
{
    "Dft", 
    "Gfx", 
    "Pipe", 
    "Fill"
    };
static int statsVals[] = 
{
    PFFSTATS_ENPFTIMES,
    PFFSTATS_ENPFTIMES | PFSTATS_ENGFX | PFSTATS_ENTEXLOAD | PFFSTATS_ENDB | PFFSTATS_ENCULL,
    PFFSTATS_ENGFXPFTIMES,
    PFSTATSHW_ENGFXPIPE_FILL | PFFSTATS_ENPFTIMES,
    };

/*-----------------------------------------------------------*/


void
initGUI(pfChannel *masterChan)
{
    float 	a, b, c, d;
    
    /* Set up size and position of GUI control panel */
    pfGetChanViewport(masterChan, &a, &b, &c, &d);
    
    if (ViewState->guiFormat == GUI_VERTICAL)
    {
	pfuGUIViewport(a, a + 0.22 * (b-a), c, d);
	pfChanViewport(masterChan, a + 0.22 * (b-a), b, c, d);
    } 
    else 
    {
	pfuGUIViewport(a, b, c, c + 0.22 * (d-c));
	pfChanViewport(masterChan, a , b, c + 0.22 * (d-c), d);
    }
    
    initPanel();
    updatePanel();
    pfuEnablePanel(ViewState->panel);

    updateGUI(ViewState->gui);
}


void
updateWidget(int widget, float val)
{
    if (widget == GUI_STATS)
	pfuWidgetOnOff(ViewState->guiWidgets[widget], val);
    else
	pfuWidgetValue(ViewState->guiWidgets[widget], val);

    (*pfuGetWidgetActionFunc(ViewState->guiWidgets[widget]))
			    (ViewState->guiWidgets[widget]);
}


void
updateGUIViewMsg(void)
{
    static char     posBuf[256];
    pfCoord	    *viewpt;
    
    /* put in position */
    viewpt =  &ViewState->viewCoord;
    sprintf(posBuf," X%5.1f Y%5.1f Z%5.1f H%5.1f P%5.1f R%5.1f",
	    viewpt->xyz[PF_X],viewpt->xyz[PF_Y],viewpt->xyz[PF_Z],
	    viewpt->hpr[PF_H],viewpt->hpr[PF_P],viewpt->hpr[PF_R]);
    pfuWidgetLabel(ViewState->guiWidgets[GUI_MSG], posBuf);
}


/*
 *    updateGUI-
 *	    This routine is called when channels are updated,
 *	    i.e. the GUI is turned ON/OFF
 */
void
updateGUI(int guiOn)
{
    float l, r, b, t;
    float ca, cb, cc, cd;
    
    if( guiOn )
    {
	pfuEnableGUI(TRUE);
    	pfuGetGUIViewport(&l, &r, &b, &t);
	pfGetChanViewport(ViewState->masterChan,&ca,&cb,&cc,&cd);
	
	if(ViewState->guiFormat == GUI_VERTICAL)
	    pfChanViewport(ViewState->masterChan, r, cb, cc, cd);
	else
	    pfChanViewport(ViewState->masterChan, ca, cb, t, cd);
    }
    else    /* turn off GUI */
    {
	pfuEnableGUI(FALSE);
	
	if (Multipipe || (ViewState->numChans == 1)) 
	    pfChanViewport(ViewState->masterChan, 0.0f, 1.0f, 0.0f, 1.0f);
	else if (!Multipipe && ((ViewState->numChans > 1) || ViewState->MCO))
	{
	    if (!ViewState->MCO)
		pfuConfigMCO(ViewState->chan, ViewState->numChans);
	    else
		pfuConfigMCO(ViewState->chan, -ViewState->numChans);
	}
	else
	{
	    pfuGetGUIViewport(&l, &r, &b, &t);
	    pfGetChanViewport(ViewState->masterChan,&ca,&cb,&cc,&cd);
	    ca = l;
	    cc = b;
	    pfChanViewport(ViewState->masterChan, ca, cb, cc, cd);
	}
    }
}

void
updateFog(int oldFog, int newFog);

static void
controlPanel(pfuWidget *w)
{
    switch(pfuGetWidgetId(w))
    {
    case GUI_COLLIDE:
	xformerModel(ViewState->xformerModel, 
		     ViewState->collideMode = (int)pfuGetWidgetValue(w));
	break;

    case GUI_LIGHT_PANE:
    case GUI_TEXTURE_PANE:
    case GUI_STRESS_PANE:
    case GUI_FOV_PANE:
	updatePanel();
	break;
	
    case GUI_CULL:
	ViewState->cullMode = (int)pfuGetWidgetValue(w);
	break;
	
    case GUI_ESKY:
	ViewState->earthSkyMode = (int)pfuGetWidgetValue(w);
	break;
	
    case GUI_FAR:
	ViewState->farPlane = pfuGetWidgetValue(w);
	pfChanNearFar(ViewState->masterChan, ViewState->nearPlane, ViewState->farPlane);
	break;
	
    case GUI_XFORMER:
        {
            int newValue        = (int)pfuGetWidgetValue(w);
            int newSelection    = (int)pfuGetWidgetSelection(w);
            ViewState->xformerModel = newValue;
            ViewState->rotateCenter = (newSelection == 0);
        }
        xformerModel(ViewState->xformerModel, ViewState->collideMode);
        break;
	
    case GUI_HIGH_STYLE:
	{
	    pfHighlight *hlight = pfuGetGUIHlight();
	    int mode = (int)pfuGetWidgetValue(w);
	    if (hlight != NULL)
		pfHlightMode(hlight, mode);
	}
	break;
	
    case GUI_LAYERED_FOG_MODE: {
	int newFogMode  = pfuGetWidgetValue(w);

	if(newFogMode != ViewState->layeredFogMode) {
	    ViewState->layeredFogMode = newFogMode;

	    updateFog(1,0); /* to delete the old volFog */
	    updateFog(0,ViewState->volFogType);
	}
	break;
    }
	
    case GUI_PATCHY_FOG_MODE: {
	int newFogMode  = pfuGetWidgetValue(w);

	if(newFogMode != ViewState->patchyFogMode) {
	    ViewState->patchyFogMode = newFogMode;
	    pfVolFogSetVal(ViewState->volFog, PFVFOG_PATCHY_MODE, newFogMode);

	    /* adjust the density */
	    switch(ViewState->patchyFogMode) {
	    case PFVFOG_LINEAR:
		pfVolFogSetDensity(ViewState->volFog,
				   ViewState->fogPatchyDensity);
		break;
	    case PFVFOG_EXP:
		pfVolFogSetDensity(ViewState->volFog,
				   ViewState->fogPatchyDensity*0.002);
		break;
	    case PFVFOG_EXP2:
		pfVolFogSetDensity(ViewState->volFog,
				   ViewState->fogPatchyDensity*0.002);
		break;
	    }
	    
	}
	break;
    }

    case GUI_FOG: {
	int newFog  = pfuGetWidgetValue(w);

	updateFog(ViewState->volFogType ? ViewState->volFogType : 
		  ViewState->fog, newFog);
	ViewState->fog = newFog;
	switch(ViewState->fog) {
	case PFVFOG_LAYERED_FOG:
	case LAYERED_MULTICOLORED_FOG:
	case PFVFOG_PATCHY_FOG:
	case PFVFOG_PATCHY_FOG | PFVFOG_LAYERED_FOG:
	    /* otherwise the regular esky fog would be set */
	    ViewState->volFogType = ViewState->fog;
	    ViewState->fog = PFFOG_OFF;
	    break;
	default:
	    ViewState->volFogType = 0;
	}

	if (ViewState->fog)
	{
	    pfuEnableWidget(ViewState->guiWidgets[GUI_NEAR_FOG_RANGE]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_FAR_FOG_RANGE]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_LAYERED_FOG_DENSITY]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_PATCHY_FOG_DENSITY]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_LAYERED_FOG_MODE]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_PATCHY_FOG_MODE]);
	}
	else
	{
	    pfuDisableWidget(ViewState->guiWidgets[GUI_NEAR_FOG_RANGE]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_FAR_FOG_RANGE]);
	    if(ViewState->volFogType & (PFVFOG_LAYERED_FOG | 
					LAYERED_MULTICOLORED_FOG)) {
		pfuEnableWidget(ViewState->guiWidgets[GUI_LAYERED_FOG_DENSITY]);
		pfuEnableWidget(ViewState->guiWidgets[GUI_LAYERED_FOG_MODE]);
	    }
	    else {
		pfuDisableWidget(ViewState->guiWidgets[GUI_LAYERED_FOG_DENSITY]);
		pfuDisableWidget(ViewState->guiWidgets[GUI_LAYERED_FOG_MODE]);
	    }
	    if(ViewState->volFogType & PFVFOG_PATCHY_FOG) {
		pfuEnableWidget(ViewState->guiWidgets[GUI_PATCHY_FOG_DENSITY]);
		pfuEnableWidget(ViewState->guiWidgets[GUI_PATCHY_FOG_MODE]);
	    }
	    else {
		pfuDisableWidget(ViewState->guiWidgets[GUI_PATCHY_FOG_DENSITY]);
		pfuDisableWidget(ViewState->guiWidgets[GUI_PATCHY_FOG_MODE]);
	    }
		
	}
	break;}
	
	/* Update Fog Far Range according to slider value */
    case GUI_FAR_FOG_RANGE:
	ViewState->farFogRange = pfuGetWidgetValue(w);
	if (ViewState->farFogRange < ViewState->nearFogRange)
	{
	    ViewState->farFogRange = 1.01f * ViewState->nearFogRange;
	    pfuWidgetValue(ViewState->guiWidgets[GUI_FAR_FOG_RANGE], 
			   ViewState->farFogRange);
	}
	break;
	
    case GUI_NEAR_FOG_RANGE:
	ViewState->nearFogRange = pfuGetWidgetValue(w);
	if (ViewState->farFogRange < ViewState->nearFogRange)
	{
	    ViewState->nearFogRange = (1.0f/1.01f) * ViewState->farFogRange;
	    pfuWidgetValue(ViewState->guiWidgets[GUI_NEAR_FOG_RANGE], 
			   ViewState->nearFogRange);
	}
	break;
	
    case GUI_PATCHY_FOG_DENSITY:
	ViewState->fogPatchyDensity = pfuGetWidgetValue(w);
	/* adjust the density */
	switch(ViewState->patchyFogMode) {
	case PFVFOG_LINEAR:
	    pfVolFogSetDensity(ViewState->volFog,
			       ViewState->fogPatchyDensity);
	    break;
	case PFVFOG_EXP:
	    pfVolFogSetDensity(ViewState->volFog,
			       ViewState->fogPatchyDensity*0.002);
	    break;
	case PFVFOG_EXP2:
	    pfVolFogSetDensity(ViewState->volFog,
			       ViewState->fogPatchyDensity*0.002);
	    break;
	}
	break;
	
    case GUI_LAYERED_FOG_DENSITY:
	ViewState->fogLayeredDensity = pfuGetWidgetValue(w);
	updateFog(1,0); /* to delete the old volFog */
	updateFog(0,ViewState->volFogType);
	break;
	
	/* Update Field of View according to slider value */
    case GUI_FOV:  
	{
	    int		 c;
	    float	 fov	= pfuGetWidgetValue(w)/ViewState->numChans;
	    pfVec3	 xyz;
	    pfVec3	 hpr;
	    
	    /* all channels share this new FOV */
	    if (InitFOV) { /* don't modify the fov */
		pfChanFOV (ViewState->masterChan, ViewState->fov[0],
			   ViewState->fov[1]);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			 "Using user-supplied FOV for masterChan = %f %f\n",
			 ViewState->fov[0], ViewState->fov[1]);
	    }
	    else
	    {
		pfChanFOV(ViewState->masterChan, fov, -1.0f);
		ViewState->fov[0] = fov;
		ViewState->fov[1] = -1.0;
		ViewState->fov[2] = 0.0;
	    }
	    
	    /* all channels have a positional offset of zero */
	    pfSetVec3(xyz, 0.0f, 0.0f, 0.0f);
	    
	    /* set each channel's rotational offset */
	    for (c = 0; c < ViewState->numChans; c++)
	    {
		if (InitFOV) {
		    pfSetVec3(hpr, 
			      (((ViewState->numChans - 1)*0.5f) - c) *
			      ViewState->fov[2],
			      0.0f, 0.0f);
		    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
			     "Using user-supplied offset for channel %d %f\n",
			     c, hpr[0]);
		} else
		    pfSetVec3(hpr, 
			      (((ViewState->numChans - 1)*0.5f) - c) *
			      fov, 0.0f, 0.0f);

		pfChanViewOffsets(ViewState->chan[ChanOrder[c]], xyz, hpr);
	    }
	}
	break;
	
	/* Update Cull Field of View Delta according to slider value */
    case GUI_FOV_CULL:  
	ViewState->cullDelta = pfuGetWidgetValue(w);
	break;
	
	/* Turns off GUI- F1 to turn back on */
    case GUI_GUI: 
	ViewState->gui = 0;
	ViewState->updateChannels = 1;
	break;
	
	/* Toggle Lighting */
    case GUI_LIGHTING:  
	{
	    ViewState->lighting = (int)pfuGetWidgetValue(w);

	    if (ViewState->lighting == LIGHTING_OFF)
		pfuDisableWidget(ViewState->guiWidgets[GUI_TOD]);
	    else
		pfuEnableWidget(ViewState->guiWidgets[GUI_TOD]);
	    
	    drawModesChanged();
	}
	break;
	
    case GUI_AA:
	{
	    static int prevFade = 1;
	    ViewState->aa = (int)pfuGetWidgetValue(w);
	    if (!ViewState->aa)
	    {
		prevFade = ViewState->fade;
		if (ViewState->fade)
		{
		    ViewState->fade = 0;
		    pfChanLODAttr(ViewState->masterChan, PFLOD_FADE, 0.0f);
		    pfuWidgetValue(ViewState->guiWidgets[GUI_LOD_FADE],0.0f);
		    pfuDisableWidget(ViewState->guiWidgets[GUI_LOD_FADE]);
		    pfuDisableWidget(ViewState->guiWidgets[GUI_LOD_FADE_RANGE]);
		}
	    }
	    else if (ViewState->aa && prevFade && ViewState->haveMultisample)
	    {
		ViewState->fade = 1;
		pfChanLODAttr(ViewState->masterChan, PFLOD_FADE, ViewState->fadeRange);
		pfuWidgetValue(ViewState->guiWidgets[GUI_LOD_FADE],1.0f);
		pfuEnableWidget(ViewState->guiWidgets[GUI_LOD_FADE]);
		pfuEnableWidget(ViewState->guiWidgets[GUI_LOD_FADE_RANGE]);
	    }
	    drawModesChanged();
	    break;
	}
	/* Update LOD Fade Range according to slider value */
    case GUI_LOD_FADE:
	ViewState->fade =  pfuGetWidgetValue(w);
	if (ViewState->haveMultisample)
	{
	    if (ViewState->fade)
	    {
		pfChanLODAttr(ViewState->masterChan, PFLOD_FADE, ViewState->fadeRange);
		pfuEnableWidget(ViewState->guiWidgets[GUI_LOD_FADE_RANGE]);
	    }
	    else 
	    {
		pfChanLODAttr(ViewState->masterChan, PFLOD_FADE, 0.0f);
		pfuDisableWidget(ViewState->guiWidgets[GUI_LOD_FADE_RANGE]);
	    }
	}
	break;
	
    case GUI_LOD_FADE_RANGE:
	if (ViewState->haveMultisample)
	{
	    if (ViewState->fade)
		pfChanLODAttr(ViewState->masterChan, PFLOD_FADE, 
			      ViewState->fadeRange = pfuGetWidgetValue(w));
	    else
		pfChanLODAttr(ViewState->masterChan, PFLOD_FADE, 0.0f);
	}
	break;
	
	/* Update LOD Scale according to slider value */
    case GUI_LOD_SCALE:
	ViewState->LODscale = pfuGetWidgetValue(w);
	break;
	
	/* Update Near Clip Plane according to slider value */
    case GUI_NEAR:
	ViewState->nearPlane = pfuGetWidgetValue(w);
	pfChanNearFar(ViewState->masterChan, ViewState->nearPlane, ViewState->farPlane);
	break;
	
    case GUI_PHASE:
	pfPhase((int)pfuGetWidgetValue(w));
	break;
	
    case GUI_PIX_LIMIT:
	pfChanLODAttr(ViewState->masterChan, PFLOD_STRESS_PIX_LIMIT,
		      (int)pfuGetWidgetValue(w));
	break;
	
    case GUI_REPOSITION:
	if (ViewState->numInitViews > 1)
	    ViewState->resetPosition = (int)pfuGetWidgetValue(w);
	else
	    ViewState->resetPosition = POS_ORIG;
	break;

    case GUI_TETHERVIEW:
	ViewState->curTetherView = (int)pfuGetWidgetValue(w);
	break;

    case GUI_VEHICLE:
	ViewState->curVehicle = (int)pfuGetWidgetValue(w);
	break;
	
    case GUI_CENTER:
	ViewState->resetPosition = POS_CENTER;
	break;
	
    case GUI_QUIT:
	ViewState->exitFlag = TRUE;
	break;
	
    case GUI_STATS:
	ViewState->stats = pfuIsWidgetOn(w) ? 1 : 0;
	ViewState->statsEnable = (uint) pfuGetWidgetValue(w);
	break;
	
    case GUI_STRESS:
	ViewState->stress = (int)pfuGetWidgetValue(w);
	if (ViewState->stress)
	{
	    pfuEnableWidget(ViewState->guiWidgets[GUI_STRESS_LOW]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_STRESS_HIGH]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_STRESS_SCALE]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_STRESS_MAX]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_PIX_LIMIT]);
	}
	else
	{
	    pfuDisableWidget(ViewState->guiWidgets[GUI_STRESS_LOW]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_STRESS_HIGH]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_STRESS_SCALE]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_STRESS_MAX]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_PIX_LIMIT]);
	}
	break;
	
    case GUI_STRESS_LOW:
	{
	    float low = pfuGetWidgetValue(w);
	    
	    if (low < ViewState->highLoad)
		ViewState->lowLoad = low;
	    else
	    {
		ViewState->lowLoad = ViewState->highLoad -0.1f;
		pfuWidgetValue(w,ViewState->lowLoad);
	    }
	}
	break;
	
    case GUI_STRESS_HIGH:
	{
	    float  high = pfuGetWidgetValue(w);
	    
	    if (high > ViewState->lowLoad)
		ViewState->highLoad = high;
	    else
	    {
		ViewState->highLoad = ViewState->lowLoad + 0.1f;
		pfuWidgetValue(w,ViewState->highLoad);
	    }
	}
	break;
	
    case GUI_STRESS_MAX:
	ViewState->stressMax = pfuGetWidgetValue(w);
	break;
	
    case GUI_STRESS_SCALE:
	ViewState->stressScale = pfuGetWidgetValue(w);
	break;
	
    case GUI_TEXTURE:
	ViewState->texture = (int)pfuGetWidgetValue(w);
	drawModesChanged();
	break;
	
    case GUI_TOD:
	ViewState->timeOfDay = pfuGetWidgetValue(w);
	updateTimeOfDay(ViewState->timeOfDay);
	break;
	
    case GUI_TREE:
	ViewState->tree = (int) pfuGetWidgetValue(w);
	if (ViewState->tree)
	{
	    pfuEnableWidget(ViewState->guiWidgets[GUI_PANX]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_PANY]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_SCALE]);
	}
	else
	{
	    pfuDisableWidget(ViewState->guiWidgets[GUI_PANX]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_PANY]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_SCALE]);
	}
	break;
	
    case GUI_DEMO:
      {
        ViewState->demoMode = pfuGetWidgetValue(w);
      }
      break;

    case GUI_PATH:
      {
	ViewState->recordPath = (pfuGetWidgetValue(w) ? 2 : 0);
      }
      break;


    case GUI_PANX:
	ViewState->panScale[0] = pfuGetWidgetValue(w);
	break;
	
    case GUI_PANY:
	ViewState->panScale[1] = pfuGetWidgetValue(w);
	break;
	
    case GUI_SCALE:
	ViewState->panScale[2] = pfuGetWidgetValue(w);
	break;
	
    case GUI_DRAW_STYLE:
	ViewState->drawStyle = (int)pfuGetWidgetValue(w);
	drawModesChanged();
	break;
	
    case GUI_SNAP:
	ViewState->snapImage = 1;
	break;

    case GUI_CALLIGRAPHIC_DEFOCUS:
        ViewState->calligDefocus = pfuGetWidgetValue(w);
        ViewState->calligDirty = 1;
        break;
    case GUI_RASTER_DEFOCUS:
        ViewState->rasterDefocus = pfuGetWidgetValue(w);
        ViewState->calligDirty = 1;
        break;
    case GUI_DRAW_TIME:
        ViewState->calligDrawTime = (int) pfuGetWidgetValue(w);
        ViewState->calligDirty = 1;
        break;
    case GUI_FILTER_SIZEX:
        ViewState->calligFilterSizeX = (int) pfuGetWidgetValue(w);
        ViewState->calligDirty = 1;
        break;
    case GUI_FILTER_SIZEY:
        ViewState->calligFilterSizeY = (int)pfuGetWidgetValue(w);
        ViewState->calligDirty = 1;
        break;
    case GUI_ZFOOT_PRINT:
        ViewState->calligZFootPrint = pfuGetWidgetValue(w);
        ViewState->calligDirty = 1;
        break;
	
    default:
	break;
    }
}


#ifdef _PF_MULTI_VIEWSTATE
void config(pfuWidget *w)
{
    int which = (int)pfuGetWidgetValue(w);
    if (AllViewStates->states[which]->guiWidgets[GUI_CONFIG] == NULL)
    {
	/*
	 * ViewState has been further initialized since the
	 * alternate ViewState selections were cloned.
	 * states[0] contains a copy of before the clones,
	 * so for each i, apply the diffs states[0] -> ViewState
	 * to states[i].
	 * XXX nice try, but there's no way this idea can ever work bytewise
	 */
	int i;
	for (i = 1; i < AllViewStates->nstates; ++i)
	{
	    char *p0 = (char *)AllViewStates->states[0];	/* original */
	    char *p1 = (char *)ViewState;			/* fork 1 */
	    char *p2 = (char *)AllViewStates->states[i];	/* fork 2 */
	    char *p3 = (char *)AllViewStates->states[i];	/* merged */
	    int j, n = sizeof(*AllViewStates->states[0]);
	    for (j = 0; j < n; ++j)
	    {
		if (p2[j] != p0[j])
		    p3[j] = p2[j];
		else
		    p3[j] = p1[j];
	    }
	}
	*AllViewStates->states[which] = *ViewState;
    }
    *ViewState = *AllViewStates->states[which];	/* XXX no doubt illegal to do this here */
}
#endif

/* ARGSUSED (suppress compiler warn) */
void resetAll(pfuWidget *w)
{
    pfuResetGUI();
    ViewState->resetPosition = POS_ORIG;
}

void
initPanel(void)
{
    float  	xo, yo, ys, xs;
    int    	x, y;
    int   	xSize, ySize, xSize3, fullSize;
    pfuWidget 	*wid;
    pfuPanel 	*panel;

    /* init the GUI panel for perfly */
    ViewState->panel = pfuNewPanel();
    ViewState->num_widgets = 0;

    panel = ViewState->panel;
    pfuGetPanelOriginSize(panel, &xo, &yo, &xs, &ys);
    
    x = (int)xo;
    y = (int)ys;

    xSize = fullSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
    ySize = PFUGUI_MESSAGE_YSIZE;
    
    /* the message bar with positions */
    wid = pfuNewWidget(panel, PFUGUI_MESSAGE, 0);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    ViewState->guiWidgets[GUI_MSG] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_MSG;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;

    xSize = xSize3 = (xSize - 2*PFUGUI_PANEL_FRAME)/3.0f;
    ySize = PFUGUI_BUTTON_YSIZE;

    /* a quit button */
    wid = pfuNewWidget(panel, PFUGUI_BUTTON, GUI_QUIT);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Quit");
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_QUIT] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_QUIT;
    ViewState->guiState[ViewState->num_widgets].across = 3;
    ViewState->num_widgets++;
    
    /* kill the gui */
    wid = pfuNewWidget(panel, PFUGUI_BUTTON, GUI_GUI);
    pfuWidgetLabel(wid,"GUI Off");
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_GUI] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_GUI;
    ViewState->guiState[ViewState->num_widgets].across = 3;
    ViewState->num_widgets++;
    
#ifdef _PF_MULTI_VIEWSTATE /* now allow multiple configurations */
    if (AllViewStates != NULL && AllViewStates->nstates > 0)
    {
	/* Reset to a particular configuration */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_CONFIG);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Config");
	pfuWidgetActionFunc(wid, config);
	/* XXX this should be dynamically changeable */
	pfuWidgetSelections(wid, AllViewStates->statenames, NULL, NULL, 
			    AllViewStates->nstates);
			    /* XXX configvals is wrong! */
	pfuWidgetDefaultValue(wid, 0);
	ViewState->guiWidgets[GUI_CONFIG] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_CONFIG;
	ViewState->guiState[ViewState->num_widgets].across = 3;
	ViewState->num_widgets++;
    }
    else
#endif
    {
	/* Reset all */
	wid = pfuNewWidget(panel, PFUGUI_BUTTON, GUI_RESET_ALL);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Reset All");
	pfuWidgetActionFunc(wid, resetAll);
	ViewState->guiWidgets[GUI_RESET_ALL] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_RESET_ALL;
	ViewState->guiState[ViewState->num_widgets].across = 3;
	ViewState->num_widgets++;
    }

    /* Pane toggles */
    xSize = xSize3;

    /* Light Pane toggle */
/*     wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_LIGHT_PANE); */
/*     pfuWidgetLabel(wid,"Light pane"); */
/*     pfuWidgetDim(wid, x, y, xSize, ySize); */
/*     pfuWidgetActionFunc(wid, controlPanel); */
/*     ViewState->guiWidgets[GUI_LIGHT_PANE] = wid; */
/*     ViewState->guiState[ViewState->num_widgets].pane = -1; */
/*     ViewState->guiState[ViewState->num_widgets].widget = GUI_LIGHT_PANE; */
/*     ViewState->guiState[ViewState->num_widgets].across = 3; */
/*     ViewState->num_widgets++; */

    /* Texture Pane toggle */
/*     wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_TEXTURE_PANE); */
/*     pfuWidgetLabel(wid,"Texture pane"); */
/*     pfuWidgetDim(wid, x, y, xSize, ySize); */
/*     pfuWidgetActionFunc(wid, controlPanel); */
/*     ViewState->guiWidgets[GUI_TEXTURE_PANE] = wid; */
/*     ViewState->guiState[ViewState->num_widgets].pane = -1; */
/*     ViewState->guiState[ViewState->num_widgets].widget = GUI_TEXTURE_PANE; */
/*     ViewState->guiState[ViewState->num_widgets].across = 3; */
/*     ViewState->num_widgets++; */

    /* Stress Pane toggle */
    wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_STRESS_PANE);
    pfuWidgetLabel(wid,"Stress pane");
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_STRESS_PANE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_STRESS_PANE;
    ViewState->guiState[ViewState->num_widgets].across = 3;
    ViewState->num_widgets++;

    /* FOV Pane toggle */
    wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_FOV_PANE);
    pfuWidgetLabel(wid,"FOV pane");
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_FOV_PANE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_FOV_PANE;
    ViewState->guiState[ViewState->num_widgets].across = 3;
    ViewState->num_widgets++;

    /* Snap screen */
    wid = pfuNewWidget(panel, PFUGUI_BUTTON, GUI_SNAP);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Snap Screen");
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_SNAP] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_SNAP;
    ViewState->guiState[ViewState->num_widgets].across = 3;
    ViewState->num_widgets++;
    
    /* toggle Collision */
    wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_COLLIDE);
    pfuWidgetLabel(wid,"Collide");
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetDefaultValue(wid,ViewState->collideMode);
    ViewState->guiWidgets[GUI_COLLIDE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_COLLIDE;
    ViewState->guiState[ViewState->num_widgets].across = 3;
    ViewState->num_widgets++;

    /* Reset to center */
    wid = pfuNewWidget(panel, PFUGUI_BUTTON, GUI_CENTER);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Center");
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_CENTER] = wid;    
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_CENTER;
    ViewState->guiState[ViewState->num_widgets].across = 3;
    ViewState->num_widgets++;
   
    xSize = fullSize;
    
    if (ViewState->numInitViews > 1)
    {
	/* Reposition menu */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_REPOSITION);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Reposition");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid, ViewState->initViewNames, NULL, NULL,
				 ViewState->numInitViews);
	pfuWidgetDefaultValue(wid, 0);
    }
    else
    {
	/* Reposition button */
	wid = pfuNewWidget(panel, PFUGUI_BUTTON, GUI_REPOSITION);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, streq(ViewState->initViewNames[0], "Default") ?
			    "Reposition" : ViewState->initViewNames[0]);
	pfuWidgetActionFunc(wid, controlPanel);
    }
    ViewState->guiWidgets[GUI_REPOSITION] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_REPOSITION;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    
    /* Xformer model */
    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_XFORMER);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Motion");
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetSelections(wid, xformerStrs, xformerVals, NULL, 
			ViewState->numVehicles > 0 ? 4 : 3);
    pfuWidgetDefaultValue(wid, ViewState->xformerModel);
    ViewState->guiWidgets[GUI_XFORMER] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_XFORMER;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;

    
    xSize = PFUGUI_BUTTON_VLONG_XSIZE;

    if (ViewState->numVehicles > 0) {
	xSize = PFUGUI_BUTTON_VLONG_XSIZE;
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_TETHERVIEW);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "View");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid, ViewState->tetherViewNames, NULL, NULL, 
			    PF_MAX2(ViewState->numTetherViews, 1));
	pfuWidgetDefaultValue(wid, 0);
	ViewState->guiWidgets[GUI_TETHERVIEW] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_TETHERVIEW;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;
	
	
	xSize = PFUGUI_BUTTON_VLONG_XSIZE;
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_VEHICLE);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Vehicle");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid, ViewState->vehicleNames, NULL, NULL,
			    ViewState->numVehicles);
	pfuWidgetDefaultValue(wid, 0);
	ViewState->guiWidgets[GUI_VEHICLE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_VEHICLE;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;

    }

    /* select (fancy) draw style */
    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_DRAW_STYLE);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid,"Style");
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetSelections(wid, styleStrs, styleVals, NULL, 
			sizeof(styleStrs)/sizeof(styleStrs[0]));
    pfuWidgetDefaultValue(wid, PFUSTYLE_FILLED);
    ViewState->guiWidgets[GUI_DRAW_STYLE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_DRAW_STYLE;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;
    
    /* select (fancy) highlight style */
    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_HIGH_STYLE);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Hlight");
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetSelections(wid, highStrs, highVals, NULL, 
			sizeof(highStrs)/sizeof(highStrs[0]));
    pfuWidgetDefaultValue(wid, PFHL_LINES);
    ViewState->guiWidgets[GUI_HIGH_STYLE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_HIGH_STYLE;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;
    
    /* toggle Texture */
    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_TEXTURE);
    pfuWidgetLabel(wid,"Texture");
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetSelections(wid, textureStrs, textureVals, NULL,
			sizeof(textureStrs)/sizeof(textureStrs[0]));
    pfuWidgetDefaultValue(wid,ViewState->texture);
    ViewState->guiWidgets[GUI_TEXTURE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_TEXTURE;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;
    
    /* Earth/Sky Model*/
    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_ESKY);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "BG");
    pfuWidgetActionFunc(wid, controlPanel);
    /* if on RE, add PFES_TAG option */
    if (ViewState->haveTagClear) 
	pfuWidgetSelections(wid, eskyStrs, eskyVals, NULL,
			    sizeof(eskyStrs)/sizeof(eskyStrs[0]));
    else
	pfuWidgetSelections(wid, eskyStrs, eskyVals, NULL,
			    sizeof(eskyStrs)/sizeof(eskyStrs[0]) - 1);
    pfuWidgetDefaultValue(wid, ViewState->earthSkyMode);
    ViewState->guiWidgets[GUI_ESKY] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_ESKY;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;
    
    xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
    
    /* XXX - for some reason the stat options aren't showing up correctly */

    /* Stats mode */
    wid = pfuNewWidget(panel,
		       PFUGUI_RADIO_BUTTON_TOGGLE | PFUGUI_HORIZONTAL, GUI_STATS);
    pfuWidgetLabel(wid, "Stats");
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetSelections(wid, statsStrs, statsVals, NULL,
			sizeof(statsStrs)/sizeof(statsStrs[0]));
    ViewState->guiWidgets[GUI_STATS] = wid;
    pfuWidgetDefaultValue(wid, PFFSTATS_ENPFTIMES);
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_STATS;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    pfuWidgetDefaultOnOff(wid, ViewState->stats);
    ViewState->num_widgets++;
    
    ySize = PFUGUI_BUTTON_YSIZE;
    
    /* cull mode */
    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_CULL);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Cull Mode");
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetSelections(wid, cullStrs, cullVals, NULL, 
			sizeof(cullStrs)/sizeof(cullStrs[0]));
    pfuWidgetDefaultValue(wid, ViewState->cullMode);
    ViewState->guiWidgets[GUI_CULL] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_CULL;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;

    ySize = PFUGUI_SLIDER_YSIZE;
     
    /* Slider for FOV */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_FOV);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Field of View");
    {
	float minFOV, maxFOV, curFOV;
	switch (ViewState->numChans)
	{
	case 1:
	    minFOV = 10.0f;
	    maxFOV = 160.0f;
	    curFOV = 45.0f;
	    break;
	case 2:
	    minFOV = 10.0f;
	    maxFOV = 320.0f;
	    curFOV = 90.0f;
	    break;
	default:
	    minFOV = 10.0f;
	    maxFOV = 360.0f;
	    curFOV = PF_MIN2(ViewState->numChans * 45.0f, 360.0f);
	    break;
	}
	/* If user gave an explicit FOV, let it expand the range */
	if (InitFOV)
	{
	    minFOV = PF_MIN2(minFOV, ViewState->fov[1]);
	    maxFOV = PF_MAX2(maxFOV, ViewState->fov[1]);
	}
	pfuWidgetRange(wid, PFUGUI_FLOAT, minFOV, maxFOV, curFOV);
    }
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_FOV] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = GUI_FOV_PANE;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_FOV;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    
    /* Slider to set farPlane clip plane distance */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER_LOG, GUI_FAR);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Far Clip");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 
		   0.5f * ViewState->nearPlane,
		   10.0f * ViewState->farPlane,
		   ViewState->farPlane);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_FAR] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = GUI_FOV_PANE;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_FAR;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    
    /* Slider to set nearPlane clip plane distance */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER_LOG, GUI_NEAR);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Near Clip");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 
		   0.00001f * ViewState->nearPlane,
		   2.0f * ViewState->farPlane,
		   ViewState->nearPlane);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_NEAR] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = GUI_FOV_PANE;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_NEAR;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;

    /* Slider for cull-view FOV */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_FOV_CULL);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Cull View FOV");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 1.0f, 1.0f);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_FOV_CULL] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = GUI_FOV_PANE;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_FOV_CULL;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    
    xSize = PFUGUI_BUTTON_VLONG_XSIZE;
    ySize = PFUGUI_BUTTON_YSIZE;
    
    /* preset fogs */
    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_FOG);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Fog");
    pfuWidgetActionFunc(wid, controlPanel);
    {
	int haveSplineFog;
	pfQueryFeature(PFQFTR_FOG_SPLINE, &haveSplineFog);
	if (haveSplineFog)
	    pfuWidgetSelections(wid, fogStrs, fogVals, NULL, 
				sizeof(fogStrs)/sizeof(fogStrs[0]));
	else
	    pfuWidgetSelections(wid, fogStrs, fogVals, NULL, 
				sizeof(fogStrs)/sizeof(fogStrs[0]) - 1);
    }
    pfuWidgetDefaultValue(wid, ViewState->fog);
    ViewState->guiWidgets[GUI_FOG] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_FOG;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;

    /* toggle Lighting*/
    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_LIGHTING);
    pfuWidgetLabel(wid,"Lighting");
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetSelections(wid, lightingStrs, lightingVals, NULL, 
			sizeof(lightingStrs)/sizeof(lightingStrs[0]));
    pfuWidgetDefaultValue(wid, ViewState->lighting);
    ViewState->guiWidgets[GUI_LIGHTING] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_LIGHTING;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;
    
    ySize = PFUGUI_SLIDER_YSIZE;
    xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
 
    /* Slider for fog near range */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER_LOG, GUI_NEAR_FOG_RANGE);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Near Fog");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, ViewState->farPlane*8.0f, ViewState->nearFogRange);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_NEAR_FOG_RANGE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_NEAR_FOG_RANGE;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->fog)
	pfuDisableWidget(ViewState->guiWidgets[GUI_NEAR_FOG_RANGE]);
    
    /* Slider for fog far range */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER_LOG, GUI_FAR_FOG_RANGE);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Far Fog");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, ViewState->farPlane*8.0f, ViewState->farFogRange);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_FAR_FOG_RANGE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_FAR_FOG_RANGE;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->fog)
	pfuDisableWidget(ViewState->guiWidgets[GUI_FAR_FOG_RANGE]);
    
    /* Slider for layered fog density */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_LAYERED_FOG_DENSITY);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Layered density");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 4, 1);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_LAYERED_FOG_DENSITY] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_LAYERED_FOG_DENSITY;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->volFogType)
	pfuDisableWidget(ViewState->guiWidgets[GUI_LAYERED_FOG_DENSITY]);
   
    /* Slider for patchy fog density */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_PATCHY_FOG_DENSITY);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Patchy density");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 15, 3);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_PATCHY_FOG_DENSITY] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_PATCHY_FOG_DENSITY;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->volFogType)
	pfuDisableWidget(ViewState->guiWidgets[GUI_PATCHY_FOG_DENSITY]);
   
    xSize = PFUGUI_BUTTON_VLONG_XSIZE;
    ySize = PFUGUI_BUTTON_YSIZE;
    
    /* Toggle layered fog mode */
    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_LAYERED_FOG_MODE);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Layered");
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetSelections(wid, layeredFogModeStrs, layeredFogModeVals, NULL, 
			sizeof(layeredFogModeStrs)/sizeof(layeredFogModeStrs[0]));
    pfuWidgetDefaultValue(wid, ViewState->layeredFogMode);
    ViewState->guiWidgets[GUI_LAYERED_FOG_MODE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_LAYERED_FOG_MODE;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;

    /* Toggle patchy fog mode */
    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_PATCHY_FOG_MODE);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Patchy");
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetSelections(wid, patchyFogModeStrs, patchyFogModeVals, NULL, 
			sizeof(patchyFogModeStrs)/sizeof(patchyFogModeStrs[0]));
    pfuWidgetDefaultValue(wid, ViewState->patchyFogMode);
    ViewState->guiWidgets[GUI_PATCHY_FOG_MODE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_PATCHY_FOG_MODE;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;

    ySize = PFUGUI_SLIDER_YSIZE;
    xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
 
    /* Slider for Time of Day */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_TOD);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Time Of Day");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 1.0f, 0.8f);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_TOD] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_TOD;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    
    /* Global LOD Scale Slider */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER_LOG, GUI_LOD_SCALE);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "LOD Scale");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 500.0f, ViewState->LODscale);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_LOD_SCALE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_LOD_SCALE;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;

    if (ViewState->haveMultisample)
    {
	
	xSize = PFUGUI_BUTTON_VLONG_XSIZE;
	ySize = PFUGUI_BUTTON_YSIZE;

	/* toggle Anti-Aliasing*/
	wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_AA);
	pfuWidgetLabel(wid,"Anti-Alias");
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetActionFunc(wid, controlPanel);
    	pfuWidgetDefaultValue(wid, ViewState->aa);
	ViewState->guiWidgets[GUI_AA] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_AA;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;
	
	/* LOD Fade button */
	wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_LOD_FADE);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Fade LOD");
	pfuWidgetActionFunc(wid, controlPanel);
    	pfuWidgetDefaultValue(wid, ViewState->fade);
	if (!ViewState->fade)
	    pfuDisableWidget(wid);
	ViewState->guiWidgets[GUI_LOD_FADE] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_LOD_FADE;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;
	
	xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
	ySize = PFUGUI_SLIDER_YSIZE;
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_LOD_FADE_RANGE);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "LOD Fade Scale");
	pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 200.0f, ViewState->fadeRange);
	pfuWidgetActionFunc(wid, controlPanel);
	ViewState->guiWidgets[GUI_LOD_FADE_RANGE] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget =
    GUI_LOD_FADE_RANGE;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;
	if (!ViewState->fade)
	    pfuDisableWidget(wid);
    }
    
    xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
    ySize = PFUGUI_SLIDER_YSIZE;
    
    /* Slider for Show Tree X-Pan */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_PANX);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Show Tree X Pan");
    pfuWidgetRange(wid, PFUGUI_FLOAT, -1.0f, 1.0f, 0.0f);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_PANX] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_PANX;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->tree)
	pfuDisableWidget(wid);
    
    /* Slider for Show Tree Y-Pan */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_PANY);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Show Tree Y Pan");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 1.0f, 0.0f);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_PANY] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_PANY;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->tree)
	pfuDisableWidget(wid);
    
    /* Slider for Show Tree Scale */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_SCALE);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Show Tree Scale");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.01f, 2.0f, 1.0f);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_SCALE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_SCALE;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->tree)
	pfuDisableWidget(wid);
    
    xSize = PFUGUI_BUTTON_VLONG_XSIZE;
    ySize = PFUGUI_BUTTON_YSIZE;

    /* Tree display */
    wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_TREE);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Show Tree");
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetDefaultValue(wid, ViewState->tree);
    ViewState->guiWidgets[GUI_TREE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_TREE;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;

    /* Stress button */
    wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_STRESS);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Stress");
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_STRESS] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = GUI_STRESS_PANE;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_STRESS;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;

    /* Record Path button */
    wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_PATH);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Path");
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_PATH] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_PATH;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;

    /* Path Follow button */
    wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_DEMO);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Demo");
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_DEMO] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_DEMO;
    ViewState->guiState[ViewState->num_widgets].across = 2;
    ViewState->num_widgets++;

    /* Stress Low Slider */
    xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
    ySize = PFUGUI_SLIDER_YSIZE;
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_STRESS_LOW);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Stress Low");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 1.0f, ViewState->lowLoad);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_STRESS_LOW] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = GUI_STRESS_PANE;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_STRESS_LOW;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->stress)
	pfuDisableWidget(wid);
    
    /* Stress High Slider */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_STRESS_HIGH);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Stress High");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 1.0f, ViewState->highLoad);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_STRESS_HIGH] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = GUI_STRESS_PANE;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_STRESS_HIGH;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->stress)
	pfuDisableWidget(wid);
    
    /* Stress Max Slider */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_STRESS_MAX);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Stress Max");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 500.0f, ViewState->stressMax);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_STRESS_MAX] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = GUI_STRESS_PANE;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_STRESS_MAX;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->stress)
	pfuDisableWidget(wid);
    
    /* Stress Scale Slider */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_STRESS_SCALE);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Stress Scale");
    pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, 4.0f, ViewState->stressScale);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_STRESS_SCALE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = GUI_STRESS_PANE;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_STRESS_SCALE;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
    if (!ViewState->stress)
	pfuDisableWidget(wid);
    
    /* Pixel Limit Slider */
    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_PIX_LIMIT);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetLabel(wid, "Stress Pixel Limit");
    pfuWidgetRange(wid, PFUGUI_FLOAT, -1.0f, 1280.0f, -1.0f);
    pfuWidgetActionFunc(wid, controlPanel);
    ViewState->guiWidgets[GUI_PIX_LIMIT] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = GUI_STRESS_PANE;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_PIX_LIMIT;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;

    /* if the calligraphic option is set */
    if (ViewState->startCallig)
    {
	ySize = PFUGUI_SLIDER_YSIZE;
	xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;

	/* raster defocus */
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_RASTER_DEFOCUS);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Raster Focus");
	pfuWidgetRange(wid, PFUGUI_FLOAT,0., 1., ViewState->rasterDefocus);
	pfuWidgetActionFunc(wid, controlPanel);
	ViewState->guiWidgets[GUI_RASTER_DEFOCUS] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_RASTER_DEFOCUS;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

	/* callig defocus */
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_CALLIGRAPHIC_DEFOCUS);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Calligraphic Focus");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetRange(wid, PFUGUI_FLOAT,0., 1., ViewState->rasterDefocus);
	ViewState->guiWidgets[GUI_CALLIGRAPHIC_DEFOCUS] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget =
		GUI_CALLIGRAPHIC_DEFOCUS;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

	/* Draw Time */
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_DRAW_TIME);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Draw time usec");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetRange(wid, PFUGUI_FLOAT,100. , 2000.,
		       ViewState->calligDrawTime);
	ViewState->guiWidgets[GUI_DRAW_TIME] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_DRAW_TIME;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;


	/* filter Size */
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_FILTER_SIZEX);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Filter SizeX");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetRange(wid, PFUGUI_FLOAT,1. , 2048.,
		       ViewState->calligFilterSizeX);
	ViewState->guiWidgets[GUI_FILTER_SIZEX] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_FILTER_SIZEX;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

	/* same on Y */
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_FILTER_SIZEY);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Filter SizeY");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetRange(wid, PFUGUI_FLOAT,1. , 2048.,
		       ViewState->calligFilterSizeY);
	ViewState->guiWidgets[GUI_FILTER_SIZEY] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_FILTER_SIZEY;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

	/* z foot print */

	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_ZFOOT_PRINT);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Z foot print");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetRange(wid, PFUGUI_FLOAT, 0.5 , 10.,
		       ViewState->calligZFootPrint);
	ViewState->guiWidgets[GUI_ZFOOT_PRINT] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = -1;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_ZFOOT_PRINT;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

	if (ViewState->calligBoard)
	{
	    pfuEnableWidget(ViewState->guiWidgets[GUI_CALLIGRAPHIC_DEFOCUS]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_RASTER_DEFOCUS]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_DRAW_TIME]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_FILTER_SIZEX]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_FILTER_SIZEY]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_ZFOOT_PRINT]);
	}
	else
	{
	    pfuDisableWidget(ViewState->guiWidgets[GUI_CALLIGRAPHIC_DEFOCUS]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_RASTER_DEFOCUS]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_DRAW_TIME]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_FILTER_SIZEX]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_FILTER_SIZEY]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_ZFOOT_PRINT]);
	}
    }

    
    /* Reset GUI so that it will come up with the specified default values */
    pfuResetGUI();
}

void
updatePanel(void)
{
    float  	xo, yo, ys, xs;
    int    	x, y;
    int		i;
    int		fit_across;
    int		across_sofar;
    pfuPanel 	*panel;
    int   	xtmp, ytmp, xSize, ySize;

    static int total_y = 0;
    int tmp_total_y;

    panel = ViewState->panel;
    pfuGetPanelOriginSize(panel, &xo, &yo, &xs, &ys);
    
    x = (int)xo;
    y = (int)ys;
    fit_across = 0;
    across_sofar = 0;
    tmp_total_y = 0;

    for (i = 0; i < ViewState->num_widgets; i++) 
    {
	if ((ViewState->guiState[i].pane == -1) ||
	    ((int)pfuGetWidgetValue(
		 ViewState->guiWidgets[ ViewState->guiState[i].pane ] ))) 
	{
	    /* widget enabled */

	    /* get size information */
	    pfuGetWidgetDim( ViewState->
			     guiWidgets[ ViewState->guiState[i].widget ],
			     &xtmp, &ytmp, &xSize, &ySize );

	    /* start new row */
	    if ((ViewState->guiState[i].across != fit_across) ||
		(across_sofar >= fit_across)) 
	    {
		fit_across = ViewState->guiState[i].across;
		across_sofar = 0;

		/* generate 5 equal sized columns when horizontal GUI */
		if ((ViewState->guiFormat == GUI_HORIZONTAL) &&
		    (y - (int)ys < -(total_y/5))) 
		{
		    y = (int)ys;
		    xo += 2*PFUGUI_BUTTON_VLONG_XINC;
		}

		x = (int)xo;
		y -= ySize + PFUGUI_PANEL_FRAME;
		tmp_total_y += ySize + PFUGUI_PANEL_FRAME;
	    }

	    across_sofar++;

/* 	    fprintf(stderr, "Widget %i - %i %i\n", i, x, y); */
	    pfuUnhideWidget( ViewState->
			    guiWidgets[ ViewState->guiState[i].widget ]);
	    pfuWidgetDim( ViewState->
			  guiWidgets[ ViewState->guiState[i].widget ],
			  x, y, xSize, ySize );
	    x += xSize + PFUGUI_PANEL_FRAME;

	} /* widget enabled */
	else 
	{ /* widget not enabled */
	    pfuHideWidget( ViewState->
			   guiWidgets[ ViewState->guiState[i].widget ]);
	} /* else - widget not enabled */
    }

    /* recurse to get horizontal GUI right */
    if ((ViewState->guiFormat == GUI_HORIZONTAL) &&
	(tmp_total_y != total_y)) 
    {
	total_y = tmp_total_y;
	updatePanel();
    }
    else
	pfuRedrawGUI();
}

