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
 * $Revision: 1.38 $
 * $Date: 2002/07/23 00:21:44 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#if 0 /* PERFLY */
#include <Performer/pfui.h>
#endif /* PERFLY */
#if 1 /* CUSTOM */
#include <Performer/pfuiD.h>
#endif /* PERFLY */
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
    "Trackball", 
    "Drive", 
    "Fly",
#if 1 /* CUSTOM */
    "Spheric",
#endif /* CUSTOM */
    "Tether"	/* option only appears if numVehicles > 0 */
};
static int xformerVals[] =
{
    PFITDF_TRACKBALL, 
    PFITDF_DRIVE, 
    PFITDF_FLY,
#if 1 /* CUSTOM */
    PFITDF_SPHERIC,
#endif /* CUSTOM */
    PFITDF_TETHER,
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
    "Spline"
    };
static int fogVals[] = 
{
    PFFOG_OFF,
    PFFOG_PIX_LIN,
    PFFOG_PIX_EXP,
    PFFOG_PIX_EXP2,
    PFFOG_PIX_SPLINE
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


#if 1 /* CUSTOM */
#ifdef NOTYET
static pfuGUIString clipCenteringStrs[] =
{
    "Fixed",
    "Auto",
    "Spiral",
    "Mouse Drag",
};

#define CLIP_VIS_ROAM (1<<0)
#define CLIP_VIS_WRAP (1<<1)
#define CLIP_VIS_INLEVEL (1<<2)

static pfuGUIString clipVisStrs[] =
{
    "Default",
    "Wrap",
    "Roam in Level",
    "Wrap in Level",
};
static int clipVisVals[] =
{
    0,
    CLIP_VIS_WRAP,
    CLIP_VIS_ROAM|CLIP_VIS_INLEVEL,
    CLIP_VIS_WRAP|CLIP_VIS_INLEVEL,
};
#endif /* NOTYET */

#if 1 /* CUSTOM */
static pfuGUIString clipCtrlModeStrs[] = 
{
    "AUTO", "= slider", "<= slider", ">= slider"
};
static int clipCtrlModeVals[] =
{
    CLIP_CTRLMODE_AUTO,
    CLIP_CTRLMODE_EQ, 
    CLIP_CTRLMODE_LEQ, 
    CLIP_CTRLMODE_GEQ
};
#endif /* CUSTOM */

/* every possible filter value */
static pfuGUIString allInterpStrs[] =
{
    "POINT",
    "LINEAR",
    "BILINEAR",
    "TRILINEAR",
    "QUADLINEAR",
    "MIPMAP_POINT",
    "MIPMAP_LINEAR",
    "MIPMAP_BILINEAR",
    "MIPMAP_TRILINEAR",
    "MIPMAP_QUADLINEAR",
    "CLIPMAP POINT",
    "CLIPMAP LINEAR",
    "CLIPMAP BILINEAR",
    "CLIPMAP TRILINEAR",
    "CLIPMAP QUADLINEAR",
    "CLIPMAP MIPMAP_POINT",
    "CLIPMAP MIPMAP_LINEAR",
    "CLIPMAP MIPMAP_BILINEAR",
    "CLIPMAP MIPMAP_TRILINEAR",
    "CLIPMAP MIPMAP_QUADLINEAR",
};
static int allInterpVals[] =
{
    PFTEX_POINT,
    PFTEX_LINEAR,
    PFTEX_BILINEAR,
    PFTEX_TRILINEAR,
    PFTEX_QUADLINEAR,
    PFTEX_MIPMAP_POINT,
    PFTEX_MIPMAP_LINEAR,
    PFTEX_MIPMAP_BILINEAR,
    PFTEX_MIPMAP_TRILINEAR,
    PFTEX_MIPMAP_QUADLINEAR,
    PFTEX_CLIPMAP|PFTEX_POINT,
    PFTEX_CLIPMAP|PFTEX_LINEAR,
    PFTEX_CLIPMAP|PFTEX_BILINEAR,
    PFTEX_CLIPMAP|PFTEX_TRILINEAR,
    PFTEX_CLIPMAP|PFTEX_QUADLINEAR,
    PFTEX_CLIPMAP|PFTEX_MIPMAP_POINT,
    PFTEX_CLIPMAP|PFTEX_MIPMAP_LINEAR,
    PFTEX_CLIPMAP|PFTEX_MIPMAP_BILINEAR,
    PFTEX_CLIPMAP|PFTEX_MIPMAP_TRILINEAR,
    PFTEX_CLIPMAP|PFTEX_MIPMAP_QUADLINEAR,
};

/* more succinct/reasonable/commonly-used values-- must include the defaults */
static pfuGUIString interpStrs[] =
{
    "POINT",
    "BILINEAR",
    "CLIPMAP_TRILINEAR",
};
static int interpVals[] =
{
    PFTEX_POINT,
    PFTEX_BILINEAR,
    PFTEX_CLIPMAP_TRILINEAR,
};

static pfuGUIString DTRmodeStrs[] =
{   /* NOTE: only choosing interesting subset of all permutations */
    "None", /* no load control */
    "MEMLOAD", /* valid mem tiles */
    "MEM_TEX", /* valid mem tiles, tex load time */
    "MEM_READSORT", /* valid mem tiles, sort read queue */
    "MEM_TEX_READSORT", /* valid mem tiles, tex load time, sort read q*/
};
static int DTRmodeVals[] =
{
    0,
    PF_DTR_MEMLOAD,
    PF_DTR_MEMLOAD | PF_DTR_TEXLOAD,
    PF_DTR_MEMLOAD | PF_DTR_READSORT,
    PF_DTR_MEMLOAD | PF_DTR_TEXLOAD | PF_DTR_READSORT,
};
#endif /* CUSTOM */


/*-----------------------------------------------------------*/

#if 1 /* CUSTOM */
static int
theresAVirtualClipTextureSelected(void)
{
    int i;
    if (ViewState->MPClipTexture != NULL)
	return pfIsClipTextureVirtual(
	 	    pfGetMPClipTextureClipTexture(ViewState->MPClipTexture));
    /* "ALL" is selected */
    for (i = 0; i < ViewState->numMPClipTextures; ++i)
	if (pfIsClipTextureVirtual(pfGetMPClipTextureClipTexture(
		(pfMPClipTexture*)pfGet(ViewState->MPClipTextures, i))))
	    return 1;
    return 0;
}
#endif /* CUSTOM */

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
#if 0 /* PERFLY */
    pfCoord	    *viewpt;
#endif /* PERFLY */
#if 1 /* CUSTOM */
    pfCoordd	    *viewpt;
#endif /* CUSTOM */

#if 1 /* CUSTOM */
    pfiInputXformSpheric *_spheric;
#endif /* CUSTOM */
    
    /* put in position */
    viewpt =  &ViewState->viewCoord;
    sprintf(posBuf," X%5.1f Y%5.1f Z%5.1f H%5.1f P%5.1f R%5.1f",
	    viewpt->xyz[PF_X],viewpt->xyz[PF_Y],viewpt->xyz[PF_Z],
	    viewpt->hpr[PF_H],viewpt->hpr[PF_P],viewpt->hpr[PF_R]);
    pfuWidgetLabel(ViewState->guiWidgets[GUI_MSG], posBuf);

#if 1 /* CUSTOM */
    if (ViewState->xformerModel == PFITDF_SPHERIC) {
	/* Display information about spheric motion model */
	/* put in path information */
	_spheric = pfiGetTDFXformerSpheric(ViewState->xformer);
	sprintf(posBuf," Ra%5.1f A%5.1f T%5.1f Ro%5.1f",
		pfiGetIXformSphericParameter(_spheric, PFIXSPH_RADIUS),
		pfiGetIXformSphericParameter(_spheric, PFIXSPH_AROUND),
		pfiGetIXformSphericParameter(_spheric, PFIXSPH_TILT),
		pfiGetIXformSphericParameter(_spheric, PFIXSPH_ROLL));
    }
    else
	posBuf[0] = '\0';
    pfuWidgetLabel(ViewState->guiWidgets[GUI_MSG1], posBuf);
#endif /* CUSTOM */
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
#if 1 /* CUSTOM */
    case GUI_CLIPTEX_PANE:
#endif /* CUSTOM */
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
	
    case GUI_FOG:
	ViewState->fog = pfuGetWidgetValue(w);
	if (ViewState->fog)
	{
	    pfuEnableWidget(ViewState->guiWidgets[GUI_NEAR_FOG_RANGE]);
	    pfuEnableWidget(ViewState->guiWidgets[GUI_FAR_FOG_RANGE]);
	}
	else
	{
	    pfuDisableWidget(ViewState->guiWidgets[GUI_NEAR_FOG_RANGE]);
	    pfuDisableWidget(ViewState->guiWidgets[GUI_FAR_FOG_RANGE]);
	}
	break;
	
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

#if 1 /* CUSTOM */
    case GUI_MINLOD_MODE:
	if (ViewState->minLODMode = (int)pfuGetWidgetValue(w))
	    pfuEnableWidget(ViewState->guiWidgets[GUI_MIP_MIN_LOD]);
	else
	    pfuDisableWidget(ViewState->guiWidgets[GUI_MIP_MIN_LOD]);
	ViewState->mip_minLOD_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_MAXLOD_MODE:
	
	if (ViewState->maxLODMode = (int)pfuGetWidgetValue(w))
	    pfuEnableWidget(ViewState->guiWidgets[GUI_MIP_MAX_LOD]);
	else
	    pfuDisableWidget(ViewState->guiWidgets[GUI_MIP_MAX_LOD]);
	ViewState->mip_maxLOD_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_CLIPOFFSET_MODE:
	if (ViewState->clipOffsetMode = (int)pfuGetWidgetValue(w))
	    pfuEnableWidget(ViewState->guiWidgets[GUI_CLIP_LOD_OFFSET]);
	else
	    pfuDisableWidget(ViewState->guiWidgets[GUI_CLIP_LOD_OFFSET]);
	ViewState->clip_LODOffset_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_ELEVELS_MODE:
	if (ViewState->elevelsMode = (int)pfuGetWidgetValue(w))
	    pfuEnableWidget(ViewState->guiWidgets[GUI_CLIP_EFFECTIVE_LEVELS]);
	else
	    pfuDisableWidget(ViewState->guiWidgets[GUI_CLIP_EFFECTIVE_LEVELS]);
	ViewState->clip_numEffectiveLevels_dirty = 1;
   	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_MIP_LOD_BIAS_S_MODE:
	if (ViewState->biasSMode = (int)pfuGetWidgetValue(w))
	    pfuEnableWidget(ViewState->guiWidgets[GUI_MIP_LOD_BIAS_S]);
	else
	    pfuDisableWidget(ViewState->guiWidgets[GUI_MIP_LOD_BIAS_S]);
	ViewState->mip_LODBiasS_dirty = 1;
   	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_MIP_LOD_BIAS_T_MODE:
	if (ViewState->biasTMode = (int)pfuGetWidgetValue(w))
	    pfuEnableWidget(ViewState->guiWidgets[GUI_MIP_LOD_BIAS_T]);
	else
	    pfuDisableWidget(ViewState->guiWidgets[GUI_MIP_LOD_BIAS_T]);
	ViewState->mip_LODBiasT_dirty = 1;
   	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_MIP_MIN_LOD:
	ViewState->mip_minLOD = pfuGetWidgetValue(w);
	ViewState->mip_minLOD_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_MIP_MAX_LOD:
	ViewState->mip_maxLOD = pfuGetWidgetValue(w);
	ViewState->mip_maxLOD_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_CLIP_INVALID_BORDER:
	ViewState->clip_invalidBorder = pfuGetWidgetValue(w);
	if (getenv("PFCLIP_AUTO_BIAS")!=NULL)
	{
#define log2(x) (log(x)*M_LOG2E)
#define round(x) floor((x)+.5)
	ViewState->mip_LODBiasS = -1.0f * log2((1024.0f-2.0f*(float)ViewState->clip_invalidBorder)/2048.0f);
	ViewState->mip_LODBiasT = ViewState->mip_LODBiasR = ViewState->mip_LODBiasS;
	pfuWidgetValue(ViewState->guiWidgets[GUI_MIP_LOD_BIAS_S], ViewState->mip_LODBiasS);
	pfuWidgetValue(ViewState->guiWidgets[GUI_MIP_LOD_BIAS_T], ViewState->mip_LODBiasT);
	ViewState->mip_LODBiasS_dirty = 1;
	ViewState->mip_LODBiasT_dirty = 1;
	}
	ViewState->clip_invalidBorder_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_CLIP_CENTERING:
	ViewState->clip_centering = (int)pfuGetWidgetValue(w);
	break;
    case GUI_CLIP_VISUALIZATION:
	ViewState->clip_visualize = (int)pfuGetWidgetValue(w);
	break;
    case GUI_MIP_LOD_BIAS_S:
	ViewState->mip_LODBiasS = pfuGetWidgetValue(w);
	ViewState->mip_LODBiasS_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_MIP_LOD_BIAS_T:
	ViewState->mip_LODBiasT = pfuGetWidgetValue(w);
	ViewState->mip_LODBiasT_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_CLIP_LOD_OFFSET:
	ViewState->clip_LODOffset = (int)pfuGetWidgetValue(w);
	ViewState->clip_LODOffset_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_CLIP_EFFECTIVE_LEVELS:
	ViewState->clip_numEffectiveLevels = (int)pfuGetWidgetValue(w);
	ViewState->clip_numEffectiveLevels_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_MIP_MAGFILTER:
	ViewState->mip_magFilter = (int)pfuGetWidgetValue(w);
	ViewState->mip_magFilter_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_MIP_MINFILTER:
	ViewState->mip_minFilter = (int)pfuGetWidgetValue(w);
	ViewState->mip_minFilter_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_CLIP_DTRMODE:
	ViewState->clip_DTRMode = (int)pfuGetWidgetValue(w);
	ViewState->clip_DTRMode_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_CLIP_DTR_TOTALTIME:
	ViewState->clip_totalDTRTime = (float)pfuGetWidgetValue(w);
	ViewState->clip_totalDTRTime_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_CLIP_DTR_TIME:
	ViewState->clip_DTRTime = (float)pfuGetWidgetValue(w);
	ViewState->clip_DTRTime_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_CLIP_DTR_BLURMARGIN:
	ViewState->clip_DTRBlurMargin = (float)pfuGetWidgetValue(w);
	ViewState->clip_DTRBlurMargin_dirty = 1;
	ViewState->clip_anyParams_dirty = 1;
	break;
    case GUI_CLIPTEX_SELECT:
	{
	    int i = (int)pfuGetWidgetValue(w);
	    if (i > 0)
	    {
		ViewState->MPClipTexture = pfGet(ViewState->MPClipTextures, i-1);
		getMPClipTextureValues(ViewState->MPClipTexture);
		updateClipTextureGUI();
	    }
	    else /* "ALL" */
	    {
		ViewState->MPClipTexture = NULL;
		/* leave the slider values whatever they were */
	    }
	    if (theresAVirtualClipTextureSelected())
	    {
		pfuEnableWidget(ViewState->guiWidgets[GUI_CLIPOFFSET_MODE]);
		pfuEnableWidget(ViewState->guiWidgets[GUI_ELEVELS_MODE]);
		if (ViewState->clipOffsetMode != 0)
		    pfuEnableWidget(ViewState->guiWidgets[GUI_CLIP_LOD_OFFSET]);
		else
		    pfuDisableWidget(ViewState->guiWidgets[GUI_CLIP_LOD_OFFSET]);
		if (ViewState->elevelsMode != 0)
		    pfuEnableWidget(ViewState->guiWidgets[GUI_CLIP_EFFECTIVE_LEVELS]);
		else
		    pfuDisableWidget(ViewState->guiWidgets[GUI_CLIP_EFFECTIVE_LEVELS]);
	    }
	    else
	    {
		pfuDisableWidget(ViewState->guiWidgets[GUI_CLIPOFFSET_MODE]);
		pfuDisableWidget(ViewState->guiWidgets[GUI_ELEVELS_MODE]);
		pfuDisableWidget(ViewState->guiWidgets[GUI_CLIP_LOD_OFFSET]);
		pfuDisableWidget(ViewState->guiWidgets[GUI_CLIP_EFFECTIVE_LEVELS]);
	    }
	}
	break;
    case GUI_CLIP_GRIDIFY:
	ViewState->gridify = (int)pfuGetWidgetValue(w);
	break;
    case GUI_CLIP_INVALIDATE:
	ViewState->invalidate = 1;
	break;
#endif /* CUSTOM */
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

#if 1 /* CUSTOM */
    /* added to support spheric motion model */
    /* the message bar with positions */
    wid = pfuNewWidget(panel, PFUGUI_MESSAGE, 0);
    pfuWidgetDim(wid, x, y, xSize, ySize);
    ViewState->guiWidgets[GUI_MSG1] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_MSG1;
    ViewState->guiState[ViewState->num_widgets].across = 1;
    ViewState->num_widgets++;
#endif /* CUSTOM */

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

#if 1 /* CUSTOM */
    /* Clip Texture Pane toggle */
    wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_CLIPTEX_PANE);
    pfuWidgetLabel(wid,"Clip Tex pane");
    pfuWidgetDim(wid, x, y, xSize, ySize);
    pfuWidgetActionFunc(wid, controlPanel);
    pfuWidgetDefaultValue(wid,TRUE);
    ViewState->guiWidgets[GUI_CLIPTEX_PANE] = wid;
    ViewState->guiState[ViewState->num_widgets].pane = -1;
    ViewState->guiState[ViewState->num_widgets].widget = GUI_CLIPTEX_PANE;
    ViewState->guiState[ViewState->num_widgets].across = 3;
    ViewState->num_widgets++;
#endif /* 1 */

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
#if 0 /* PERFLY */
			ViewState->numVehicles > 0 ? 4 : 3);
#endif /* PERFLY */
#if 1 /* CUSTOM */
			ViewState->numVehicles > 0 ? 5 : 4);
#endif /* CUSTOM */
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
#if 1 /* CUSTOM */
    if (getenv("_CLIPFLY_ADAPT"))
	    pfuWidgetLabel(wid, "Far Clip/Horizon");
#endif /* CUSTOM */
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
#if 1 /* CUSTOM */
    if (getenv("_CLIPFLY_ADAPT"))
	pfuWidgetLabel(wid, "Near Clip/Height");
#endif /* CUSTOM */
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


#if 1 /* CUSTOM */
    /*
     * Widgets to control the clip texture(s)
     */
    {
	int virtualSize = 1<<23; /* XXX arbitrary-- should always adapt to currently selected, ALL should go to largest size */
	int clipSize = 2048; /* XXX ditto */
	if (ViewState->numMPClipTextures > 0)
	{
	    pfGetClipTextureVirtualSize(pfGetMPClipTextureClipTexture((pfMPClipTexture *)pfGet(ViewState->MPClipTextures, 0)), &virtualSize, NULL, NULL);
	    clipSize = pfGetClipTextureClipSize(pfGetMPClipTextureClipTexture((pfMPClipTexture *)pfGet(ViewState->MPClipTextures, 0)));
	}

	/* cliptex select menu */
	ySize = PFUGUI_BUTTON_YSIZE;
	
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_CLIPTEX_SELECT);
	pfuWidgetLabel(wid,"Clip Tex");
	pfuWidgetDim(wid, x, y, fullSize, ySize);
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid,  ViewState->clipTexStrs, NULL, NULL, 
			    ViewState->numMPClipTextures+1);
	pfuWidgetDefaultValue(wid,0);
	ViewState->guiWidgets[GUI_CLIPTEX_SELECT] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_CLIPTEX_SELECT;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

	xSize = PFUGUI_BUTTON_VLONG_XSIZE;

	/* toggle Gridify */
	wid = pfuNewWidget(panel, PFUGUI_SWITCH, GUI_CLIP_GRIDIFY);
	pfuWidgetLabel(wid,"Gridify");
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetActionFunc(wid, controlPanel);
    	pfuWidgetDefaultValue(wid, ViewState->gridify);
	ViewState->guiWidgets[GUI_CLIP_GRIDIFY] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_CLIP_GRIDIFY;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;
	
	/* Invalidate */
	wid = pfuNewWidget(panel, PFUGUI_BUTTON, GUI_CLIP_INVALIDATE);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Reload Tex");
	pfuWidgetActionFunc(wid, controlPanel);
	ViewState->guiWidgets[GUI_CLIP_INVALIDATE] = wid;    
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_CLIP_INVALIDATE;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;


	ySize = PFUGUI_SLIDER_YSIZE;

	/* XXX this used to be the condition,
	   but it was wrong if the clip texture changed...
	   should probably always created these widgets,
	   but disable them whenever a non-virtual clip texture
	   is selected.  For now, just always create them
	   and always have them enabled (they will
	   have no effect on a non-virtual clip texture */
	/* if ((1<<(ViewState->clip_numEffectiveLevels-1)) < virtualSize) */
	{
	    /* Clip LOD Offset Slider */
	    /* range control menu */
	    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_CLIPOFFSET_MODE);
	    pfuWidgetLabel(wid,"Offset");
	    pfuWidgetDim(wid, x, y, xSize3, ySize);
	    pfuWidgetActionFunc(wid, controlPanel);
	    pfuWidgetSelections(wid, clipCtrlModeStrs, clipCtrlModeVals, NULL, 
				sizeof(clipCtrlModeStrs)/sizeof(clipCtrlModeStrs[0]));
	    pfuWidgetDefaultValue(wid, ViewState->clipOffsetMode);
	    ViewState->guiWidgets[GUI_CLIPOFFSET_MODE] = wid;
	    ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	    ViewState->guiState[ViewState->num_widgets].widget = GUI_CLIPOFFSET_MODE;
	    ViewState->guiState[ViewState->num_widgets].across = 2;
	    ViewState->num_widgets++;
	
	    wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_CLIP_LOD_OFFSET);
	    xSize = xSize3*2 + PFUGUI_PANEL_FRAME;
	    ySize = PFUGUI_SLIDER_YSIZE;
	    pfuWidgetDim(wid, x, y, xSize, ySize);
	    pfuWidgetLabel(wid, "Virtual LOD Offset");
	    pfuWidgetRange(wid, PFUGUI_FLOAT, 0, 20,
			   ViewState->clip_LODOffset);
	    pfuWidgetActionFunc(wid, controlPanel);
	    pfuWidgetMode(wid, PFUGUI_STICKY_DEFAULT, 0);
	    ViewState->guiWidgets[GUI_CLIP_LOD_OFFSET] = wid;
	    ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	    ViewState->guiState[ViewState->num_widgets].widget =
	GUI_CLIP_LOD_OFFSET;
	    ViewState->guiState[ViewState->num_widgets].across = 2;
	    ViewState->num_widgets++;

	    /* Effective Levels Slider */
	    /* range control menu */
	    wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_ELEVELS_MODE);
	    pfuWidgetLabel(wid,"Levels");
	    pfuWidgetDim(wid, x, y, xSize3, ySize);
	    pfuWidgetActionFunc(wid, controlPanel);
	    pfuWidgetSelections(wid, clipCtrlModeStrs, clipCtrlModeVals, NULL, 
				sizeof(clipCtrlModeStrs)/sizeof(clipCtrlModeStrs[0]));
	    pfuWidgetDefaultValue(wid, ViewState->elevelsMode);
	    ViewState->guiWidgets[GUI_ELEVELS_MODE] = wid;
	    ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	    ViewState->guiState[ViewState->num_widgets].widget = GUI_ELEVELS_MODE;
	    ViewState->guiState[ViewState->num_widgets].across = 2;
	    ViewState->num_widgets++;
	
	    wid = pfuNewWidget(panel, PFUGUI_SLIDER,
			       GUI_CLIP_EFFECTIVE_LEVELS);
	    xSize = xSize3*2 + PFUGUI_PANEL_FRAME;
	    ySize = PFUGUI_SLIDER_YSIZE;
	    pfuWidgetDim(wid, x, y, xSize, ySize);
	    pfuWidgetLabel(wid, "Effective Levels");
	    pfuWidgetRange(wid, PFUGUI_FLOAT,
	       1, 16, ViewState->clip_numEffectiveLevels);
	    pfuWidgetActionFunc(wid, controlPanel);
	    pfuWidgetMode(wid, PFUGUI_STICKY_DEFAULT, 0);
	    ViewState->guiWidgets[GUI_CLIP_EFFECTIVE_LEVELS] = wid;
	    ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	    ViewState->guiState[ViewState->num_widgets].widget =
	GUI_CLIP_EFFECTIVE_LEVELS;
	    ViewState->guiState[ViewState->num_widgets].across = 2;
	    ViewState->num_widgets++;
	}

	/* Min LOD Slider */
	/* range control menu */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_MINLOD_MODE);
	pfuWidgetLabel(wid,"Min");
	pfuWidgetDim(wid, x, y, xSize3 , ySize);
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid, clipCtrlModeStrs, clipCtrlModeVals, NULL, 
			    sizeof(clipCtrlModeStrs)/sizeof(clipCtrlModeStrs[0]));
	pfuWidgetDefaultValue(wid, ViewState->minLODMode);
	ViewState->guiWidgets[GUI_MINLOD_MODE] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_MINLOD_MODE;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;
    
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_MIP_MIN_LOD);
	xSize = xSize3*2 + PFUGUI_PANEL_FRAME;
	ySize = PFUGUI_SLIDER_YSIZE;
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Finest LOD");
	pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, log2(virtualSize), 0.0f);
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetMode(wid, PFUGUI_STICKY_DEFAULT, 0);
	ViewState->guiWidgets[GUI_MIP_MIN_LOD] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_MIP_MIN_LOD;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;

	
	/* Max LOD Slider */
	/* range control menu */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_MAXLOD_MODE);
	pfuWidgetLabel(wid,"Max");
	pfuWidgetDim(wid, x, y, xSize3, ySize);
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid, clipCtrlModeStrs, clipCtrlModeVals, NULL, 
			    sizeof(clipCtrlModeStrs)/sizeof(clipCtrlModeStrs[0]));
	pfuWidgetDefaultValue(wid, ViewState->maxLODMode);
	ViewState->guiWidgets[GUI_MAXLOD_MODE] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_MAXLOD_MODE;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;
	
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_MIP_MAX_LOD);
	xSize = xSize3*2 + PFUGUI_PANEL_FRAME;
	ySize = PFUGUI_SLIDER_YSIZE;
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Coarsest LOD");
	pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f, log2(virtualSize), log2(virtualSize));
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetMode(wid, PFUGUI_STICKY_DEFAULT, 0);
	ViewState->guiWidgets[GUI_MIP_MAX_LOD] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_MIP_MAX_LOD;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;

	/* LOD Bias S Slider */
	/* range control menu */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_MIP_LOD_BIAS_S_MODE);
	pfuWidgetLabel(wid,"BiasS");
	pfuWidgetDim(wid, x, y, xSize3, ySize);
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid, clipCtrlModeStrs, clipCtrlModeVals, NULL, 
			    sizeof(clipCtrlModeStrs)/sizeof(clipCtrlModeStrs[0]));
	pfuWidgetDefaultValue(wid, ViewState->biasSMode);
	ViewState->guiWidgets[GUI_MIP_LOD_BIAS_S_MODE] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_MIP_LOD_BIAS_S_MODE;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;
	
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_MIP_LOD_BIAS_S);
	xSize = xSize3*2 + PFUGUI_PANEL_FRAME;
	ySize = PFUGUI_SLIDER_YSIZE;
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Mipmap LOD Bias S");
	pfuWidgetRange(wid, PFUGUI_FLOAT, -10.0f, 10.0f,
		       ViewState->mip_LODBiasS);
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetMode(wid, PFUGUI_STICKY_DEFAULT, 0);
	ViewState->guiWidgets[GUI_MIP_LOD_BIAS_S] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget =
    GUI_MIP_LOD_BIAS_S;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;

	/* LOD Bias T Slider */
	/* range control menu */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_MIP_LOD_BIAS_T_MODE);
	pfuWidgetLabel(wid,"BiasT");
	pfuWidgetDim(wid, x, y, xSize3, ySize);
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid, clipCtrlModeStrs, clipCtrlModeVals, NULL, 
			    sizeof(clipCtrlModeStrs)/sizeof(clipCtrlModeStrs[0]));
	pfuWidgetDefaultValue(wid, ViewState->biasTMode);
	ViewState->guiWidgets[GUI_MIP_LOD_BIAS_T_MODE] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_MIP_LOD_BIAS_T_MODE;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;
	
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_MIP_LOD_BIAS_T);
	xSize = xSize3*2 + PFUGUI_PANEL_FRAME;
	ySize = PFUGUI_SLIDER_YSIZE;
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Mipmap LOD Bias T");
	pfuWidgetRange(wid, PFUGUI_FLOAT, -10.0f, 10.0f,
		       ViewState->mip_LODBiasT);
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetMode(wid, PFUGUI_STICKY_DEFAULT, 0);
	ViewState->guiWidgets[GUI_MIP_LOD_BIAS_T] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget =
    GUI_MIP_LOD_BIAS_T;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;

	/* Invalid Border Slider */
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_CLIP_INVALID_BORDER);
	xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
	ySize = PFUGUI_SLIDER_YSIZE;
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Invalid Border");
	pfuWidgetRange(wid, PFUGUI_FLOAT, 0.0f,
		       clipSize,
		       ViewState->clip_invalidBorder);
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetMode(wid, PFUGUI_STICKY_DEFAULT, 0);
	ViewState->guiWidgets[GUI_CLIP_INVALID_BORDER] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget =
    GUI_CLIP_INVALID_BORDER;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

    #ifdef NOTYET
	/* Clip Centering menu */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_CLIP_CENTERING);
	xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
    #ifdef NOTYET
	xSize = PFUGUI_BUTTON_VLONG_XSIZE;
    #endif
	ySize = PFUGUI_BUTTON_YSIZE;
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Clip Center");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid, clipCenteringStrs, NULL, NULL,
			    numberof(clipCenteringStrs));
	pfuWidgetDefaultValue(wid, 1);
	ViewState->guiWidgets[GUI_CLIP_CENTERING] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget =
    GUI_CLIP_CENTERING;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;

	/* Clip Visualization */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_CLIP_VISUALIZATION);
	xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
	ySize = PFUGUI_BUTTON_YSIZE;
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Clip Visualization");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid, clipVisStrs, clipVisVals, NULL,
				 numberof(clipVisStrs));
	pfuWidgetDefaultValue(wid, 0);
	ViewState->guiWidgets[GUI_CLIP_VISUALIZATION] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget =
    GUI_CLIP_VISUALIZATION;
	ViewState->guiState[ViewState->num_widgets].across = 2;
	ViewState->num_widgets++;

    #endif

	/* Mag filter */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_MIP_MAGFILTER);
	xSize = PFUGUI_BUTTON_VLONG_XSIZE + PFUGUI_BUTTON_VLONG_XINC;
	ySize = PFUGUI_BUTTON_YSIZE;
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Mag Filter");
	pfuWidgetActionFunc(wid, controlPanel);
	if (getenv("_PERFLY_ALL_FILTERS")) /* XXX */
	    pfuWidgetSelections(wid, allInterpStrs, allInterpVals, NULL,
				numberof(allInterpStrs));
	else
	    pfuWidgetSelections(wid, interpStrs, interpVals, NULL,
				numberof(interpStrs));
	pfuWidgetDefaultValue(wid, ViewState->mip_magFilter);
	ViewState->guiWidgets[GUI_MIP_MAGFILTER] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_MIP_MAGFILTER;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

    #ifdef NOTYET /* XXX if this gets put back, it will require rearranging */
	/* Min filter */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_MIP_MINFILTER);
	xSize = PFUGUI_BUTTON_VLONG_XSIZE;
	ySize = PFUGUI_BUTTON_YSIZE;
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "Min Filter");
	pfuWidgetActionFunc(wid, controlPanel);
	if (getenv("_PERFLY_ALL_FILTERS")) /* XXX */
	    pfuWidgetSelections(wid, allInterpStrs, allInterpVals, NULL,
				numberof(allInterpStrs));
	else
	    pfuWidgetSelections(wid, interpStrs, interpVals, NULL,
				numberof(interpStrs));
	pfuWidgetDefaultValue(wid, ViewState->mip_minFilter);
	ViewState->guiWidgets[GUI_MIP_MINFILTER] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_MIP_MINFILTER;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;
    #endif /* NOTYET */

	xSize = fullSize;
	ySize = PFUGUI_BUTTON_YSIZE;

	/* DTR mode */
	wid = pfuNewWidget(panel, PFUGUI_MENU_BUTTON, GUI_CLIP_DTRMODE);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetLabel(wid, "DTR Mode");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetSelections(wid, DTRmodeStrs, DTRmodeVals, NULL, 
			    numberof(DTRmodeVals));
	pfuWidgetDefaultValue(wid, ViewState->clip_DTRMode);
	ViewState->guiWidgets[GUI_CLIP_DTRMODE] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_CLIP_DTRMODE;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

	ySize = PFUGUI_SLIDER_YSIZE;

	/* Pipe Total DTR Time slider */
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_CLIP_DTR_TOTALTIME);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetRange(wid, PFUGUI_FLOAT, 0., 1000.0f/ViewState->frameRate,
			    ViewState->clip_totalDTRTime);
	pfuWidgetLabel(wid, "Pipe Total DTR Time");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetMode(wid, PFUGUI_STICKY_DEFAULT, 0);
	ViewState->guiWidgets[GUI_CLIP_DTR_TOTALTIME] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_CLIP_DTR_TOTALTIME;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

	/* DTR Time slider */
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_CLIP_DTR_TIME);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetRange(wid, PFUGUI_FLOAT, -1., 1000.0f/ViewState->frameRate,
			    ViewState->clip_DTRTime);
	pfuWidgetLabel(wid, "DTR Time");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetMode(wid, PFUGUI_STICKY_DEFAULT, 0);
	ViewState->guiWidgets[GUI_CLIP_DTR_TIME] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_CLIP_DTR_TIME;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;

	/* DTR Blur Margin slider */
	wid = pfuNewWidget(panel, PFUGUI_SLIDER, GUI_CLIP_DTR_BLURMARGIN);
	pfuWidgetDim(wid, x, y, xSize, ySize);
	pfuWidgetRange(wid, PFUGUI_FLOAT, 0., 1.,
			    ViewState->clip_DTRBlurMargin);
	pfuWidgetLabel(wid, "DTR Blur Margin");
	pfuWidgetActionFunc(wid, controlPanel);
	pfuWidgetMode(wid, PFUGUI_STICKY_DEFAULT, 0);
	ViewState->guiWidgets[GUI_CLIP_DTR_BLURMARGIN] = wid;
	ViewState->guiState[ViewState->num_widgets].pane = GUI_CLIPTEX_PANE;
	ViewState->guiState[ViewState->num_widgets].widget = GUI_CLIP_DTR_BLURMARGIN;
	ViewState->guiState[ViewState->num_widgets].across = 1;
	ViewState->num_widgets++;
    }
#endif /* CUSTOM */

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

