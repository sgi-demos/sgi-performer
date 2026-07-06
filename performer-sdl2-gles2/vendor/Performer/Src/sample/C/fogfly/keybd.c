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
 * keybd.c -- keyboard commands
 */

#include <stdio.h>
#include <Performer/pfutil.h>
#include <Performer/pfui.h>

#include "perfly.h"
#include "gui.h"


void 
printCommands(void)
{
    fprintf(stderr, " Key     Function\n");
    fprintf(stderr, "-----    --------------\n");
    fprintf(stderr, " ESC     Exit program \n");
    fprintf(stderr, "  ?      Print help\n");
    fprintf(stderr, "SPACE    Stop moving\n");
    fprintf(stderr, " F1      Toggle graphical user interface\n");
    fprintf(stderr, " F12     Print current view position to stderr\n");
    fprintf(stderr, "  b      Toggle backface mode\n");
    fprintf(stderr, "  c      Toggle collision detection\n");
    fprintf(stderr, "  C      Change culling mode\n");
    fprintf(stderr, "  d      make and use pfGeoSet display lists\n");
    fprintf(stderr, "  D      make and use pfScene display list\n");
    fprintf(stderr, "  f      Decrease the target frame rate\n");
    fprintf(stderr, "  F      Increase the target frame rate\n");
    fprintf(stderr, "  g      Toggle clip texture gridification\n");
    fprintf(stderr, "  i      use immediate mode\n");
    fprintf(stderr, "  l      Toggle lighting\n");
    fprintf(stderr, "  m      follow path\n");
    fprintf(stderr, "  M      Make movie of path file\n");
    fprintf(stderr, "  k      dynamic video resize: manual->auto->off\n");
    fprintf(stderr, "  p      Change frame phase\n");
    fprintf(stderr, "  r      Reset positions\n");
    fprintf(stderr, "  s      Toggle diagnostics display\n");
    fprintf(stderr, "  S      Print stats graph contents to stderr\n");
    fprintf(stderr, "  t      Toggle texturing\n");
    fprintf(stderr, "  v      Toggle vehicles\n");
    fprintf(stderr, "  w      Toggle draw mode between solid/wire\n");
    fprintf(stderr, "  W      Cycle draw mode between all styles\n");
    fprintf(stderr, "  x      Snapshot RGB screen image to file\n");
    fprintf(stderr, "  X      Snapshot RGBA screen image to file\n");
    fprintf(stderr, "  z      Show culling frustum\n");
    fflush(stderr);
}

void
toggleDVR(void)
{
    static int oldDVR = DVR_MANUAL;
    /* can't toggle DVR mode when fill stats are enabled since we
     * are using a different window than the one the video channel
     * is bound to.
     */
    if (ViewState->stats && (ViewState->statsEnable & PFSTATSHW_ENGFXPIPE_FILL))
    {
pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"DVR - can't change mode with fill stats enabled.");
	return;
    }

    if (!ViewState->doDVR)
	ViewState->doDVR = oldDVR;
    else
    {
	oldDVR = ViewState->doDVR;
	ViewState->doDVR = 0;
    }
}

void
changeDVR(void)
{
    /* can't toggle DVR mode when fill stats are enabled since we
     * are using a different window than the one the video channel
     * is bound to.
     */
    if (ViewState->statsEnable & PFSTATSHW_ENGFXPIPE_FILL) 
    {
pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"DVR - can't change mode with fill stats enabled.");
	return;
    }
    switch(ViewState->doDVR)
    {
	case 0: 
	    ViewState->doDVR = DVR_MANUAL; 
pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"DVR - MANUAL [up/down arrow keys control size]");
	    break;
	case DVR_MANUAL: 
	    ViewState->doDVR = DVR_AUTO; 
pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"DVR - AUTO");
	    break;
	case DVR_AUTO: 
	    ViewState->doDVR = DVR_MANUAL; 
pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"DVR - OFF");
	    break;
    }
}

void
updateManualDVR(int dir)
{
    if (dir > 0)
    {
	if (ViewState->doDVR)
	{
	   ViewState->dvrXSize += 4;
	   ViewState->dvrYSize += 4;
	}
    }
    else if (dir < 0)
    {
	if (ViewState->doDVR)
	{
	   ViewState->dvrXSize -= 4;
	   ViewState->dvrYSize -= 4;
	}
    }
}

void
updateDrawMode (int newStyle)
{
    if (newStyle == ViewState->drawMode)
	return;
    ViewState->drawMode = newStyle;
}

void 
toggleBackface(void)
{
    ViewState->backface = !ViewState->backface;
    drawModesChanged();
}


void 
toggleDrawStyle(int justToggle)
{
    if (justToggle)
    {
	if (ViewState->drawStyle == PFUSTYLE_FILLED)
	    ViewState->drawStyle = PFUSTYLE_LINES;
	else
	    ViewState->drawStyle = PFUSTYLE_FILLED;
    }
    else
    {
	ViewState->drawStyle = (ViewState->drawStyle + 1) % PFUSTYLE_COUNT;
    }

    /* The GUI Controls this mode also, so update the Widget */
    updateWidget(GUI_DRAW_STYLE, (float)ViewState->drawStyle);
}


void 
toggleLighting(void)
{
    ViewState->lighting = (ViewState->lighting + 1) % LIGHTING_COUNT;
    /* The GUI Controls this mode also, so update the Widget */
    updateWidget(GUI_LIGHTING, (float)ViewState->lighting);
}


void 
toggleTexture(void)
{
    ViewState->texture = !ViewState->texture;
    /* The GUI Controls this mode also, so update the Widget */
    updateWidget(GUI_TEXTURE, (float)ViewState->texture);
}


void 
toggleStats(void)
{
    ViewState->stats = !ViewState->stats;
    /* The GUI Controls this mode also, so update the Widget */
    updateWidget(GUI_STATS,(float)ViewState->stats);
}

void 
toggleTree(void)
{
    ViewState->tree = !ViewState->tree;
    /* The GUI Controls this mode also, so update the Widget */
    updateWidget(GUI_TREE, (float)ViewState->tree);
}


void 
changeFieldRate(int inc)
{
    int fields, vrate;
    
    vrate = pfGetVideoRate();

    fields =  pfGetFieldRate();		/* Get current field rate */

    fields += inc;

    /* Clamp fields */
    if (fields <= 0)
	fields = 1;
    if (fields > vrate)
	fields = vrate;

    pfFieldRate(fields);

    ViewState->frameRate = pfGetFrameRate();	/* Get current frame rate */
}


void
toggleCullMode(void)
{
    int	oldMode	= 0;
    int	newMode	= 0;

    /* get old cull mode */
    oldMode = ViewState->cullMode;

    /* advance to next feature combination */
    newMode = ((oldMode + 1) & 0x0f) | (oldMode & ~0x0f);

    /* GeoSet culling only makes sense when view culling is enabled */
    if (newMode & PFCULL_GSET) newMode |= PFCULL_VIEW;

    /* set new cull mode */
    ViewState->cullMode = newMode;
    
    updateWidget(GUI_CULL, newMode);
}

void
togglePhase(u_int incr)
{
    u_int oldPhase = pfGetPhase(), newPhase;
    if (incr == PFPHASE_SPIN_DRAW)	/* toggle the PFPHASE_SPIN_DRAW bit */
	newPhase = (~oldPhase&PFPHASE_SPIN_DRAW)
		 | (oldPhase&PFPHASE_MODE_MASK);
    else				/* cycle the rest of the mode */
	newPhase = (oldPhase&PFPHASE_SPIN_DRAW)
		 | (((oldPhase&PFPHASE_MODE_MASK) + incr) % 4);
    pfPhase(newPhase);
}

void
toggleCollide(void)
{
    ViewState->collideMode = !ViewState->collideMode;
    updateWidget(GUI_COLLIDE, ViewState->collideMode);
}

void toggleVehicles(void)
{
    int i;

    if (ViewState->numVehicles > 0)
	if (pfRemoveChild(ViewState->sceneGroup, ViewState->vehicleDCSs[0]))
	    for (i = 1; i < ViewState->numVehicles; ++i)
		pfRemoveChild(ViewState->sceneGroup, ViewState->vehicleDCSs[i]);
	else
	    for (i = 0; i < ViewState->numVehicles; ++i)
		pfAddChild(ViewState->sceneGroup, ViewState->vehicleDCSs[i]);
}

void 
toggleGUI(void)
{
    int i;
    ViewState->gui = !ViewState->gui;
    ViewState->updateChannels = 1;
    ViewState->redrawOverlay = 1;
    ViewState->redrawCount = pfGetPipeDrawCount(0);
    for (i=0; i<NumPipes; i++)
	ViewState->drawFlags[i] |= REDRAW_WINDOW;
}

void
doMovie(void)
{
    if (!ViewState->demoMode)
    {
	ViewState->snapImage = ViewState->democnt + 3;
	ViewState->demoMode = 2;
    }
    else
    {
	ViewState->snapImage = 0;
	ViewState->demoMode = 0;
    }
}

static int theresAClipTexture(void)
{
    int i;
    for (i = pfGetMultipipe()-1; i >= 0; --i)
	if (pfGetPipeNumMPClipTextures(pfGetPipe(i)) > 0)
	    return 1;
    return 0;
}

/* process input in the app process space */
void
processKeybdInput(void)
{
    int 	i;
    int		j;
    int	 	key;
    int 	dev;
    int	 	val;
    int	 	numDevs;
    pfuEventStream *events;
    
    events = &ViewState->events;
    numDevs = events->numDevs;

    for (j = 0; j < numDevs; j++)
    {
	dev = events->devQ[j];
	val = events->devVal[j];
	val = val; /* suppress compiler warn */

	if (events->devCount[dev] > 0)
	{
	    switch(dev)
	    {
	    /* Window-system events */
	    case PFUDEV_REDRAW:
		events->devCount[dev] = 0; /* mark device done */
		pfuRedrawGUI();
		ViewState->redrawOverlay = 3;
		for (i=0; i<NumPipes; i++)
		    ViewState->drawFlags[i] |= REDRAW_WINDOW;
		break;
		
	    case PFUDEV_WINQUIT:
		ViewState->exitFlag = 1;
		break;
		
	    /* Main keyboard */
	    case PFUDEV_KEYBD:
		for (i = 0; i < events->numKeys; i++)
		{
		    key = events->keyQ[i];
		    if (events->keyCount[key])
		    {
			switch(key)
			{
			case 27:	/* ESC  exit program */
			    ViewState->exitFlag = 1;
			    break;

			case '?':	/* print commands */
			    printCommands();
			    break;
		
			case ' ':	/* Stop moving */
			    pfiStopXformer(ViewState->xformer);
			    break;

			case 'b':	/* Toggle backface mode */
			case 'B':
			    toggleBackface();
			    break;

			case 'c':	/* Change collide mode */
			    toggleCollide();
			    break;

			case 'C':	/* Change culling mode */
			    toggleCullMode();
			    break;

			case 'd':	/* make/use pfGeoState display lists */
			    updateDrawMode(DLIST_GEOSET);
			    break;

			case 'D':	/* make/use pfScene display list */
			    updateDrawMode(DLIST_SCENE);
			    break;

			case 'f':	/* Increment fields per frame */
			    changeFieldRate(1);
			    break;

			case 'F':	/* Decrement fields per frame */
    			    changeFieldRate(-1);
			    break;

			case 'h':
			    toggleTree();
			    break;

			case 'i':	/* use immediate mode */
			    updateDrawMode(DLIST_OFF);
			    break;

			case 'I':	/* Toggle show DVR */
			    ViewState->showDVR ^= 0x1;
			    break;

			case 'k':	/* Toggle DVR */
			    toggleDVR();
			    break;

			case 'K':
			    changeDVR();
			    break;

			case 'l':	/* Toggle lighting */
			case 'L':
			    toggleLighting();
			    break;
			
			case 'm':
			    ViewState->demoMode ^= 1;
			    break;
			    
			case 'M':
			    doMovie();
			    break;
			    
			case 'p':	/* Change frame phase */
			    togglePhase(1);
			    break; 
			case 'P':
			    togglePhase(PFPHASE_SPIN_DRAW);
			    break;

			case 'r':	/* Reset positions */
			case 'R':
			    ViewState->resetPosition = POS_ORIG;
			    break;
			
			case 's':	/* Toggle diagnostics display */
			    toggleStats();
			    break;
			
			case 'S':	/* Print stats display to stderr */
			    if (!ViewState->stats)
				toggleStats();
			    ViewState->printStats = 1;
			    break;

			case 'g':	/* Toggle gridification */
			    /* for old fogies... */
			    if (!theresAClipTexture())
			    {
				toggleStats();
				break;
			    }
			    ViewState->gridify = !ViewState->gridify;
			    break;
			    
			case 't':	/* Toggle texturing */
			case 'T':
			    toggleTexture();
			    break;
			
			case 'v':
			case 'V':
			    toggleVehicles();
			    break;
	
			case 'w':	/* Toggle draw mode */
			case 'W':
			    toggleDrawStyle((key == 'w') ? 1 : 0);
			    break;
			    
			case 'x':	/* Snap RGB screen image */
			    ViewState->snapImage = -1;
			    break;

			case 'X':	/* Snap RGBA screen image */
			    ViewState->snapImage = -2;
			    break;

			case 'z':	/* toggle cull-frustum visualization */
			case 'Z':
			    if (ViewState->cullDelta < 1.0f)
				ViewState->cullDelta = 1.0f;
			    else
				ViewState->cullDelta = .5f;
			    updateWidget(GUI_FOV_CULL, ViewState->cullDelta);
			    break;

			default:	/* Unsupported keyboard input */ 
			    break;
			} 			
			/* reset the key as done */
			events->keyCount[key] = 0;
		    } 		    
		} 
		break; 

	    case PFUDEV_UPARROWKEY:
		updateManualDVR(1);
		break;

	    case PFUDEV_DOWNARROWKEY:
		updateManualDVR(-1);
		break;

	    case PFUDEV_PRINTSCREENKEY:
		ViewState->snapImage = 1;
		break;
		
	    /* Function keys */
	    case PFUDEV_F1KEY:		/* GUI on / off */
		toggleGUI();
		break;
	    case PFUDEV_F12KEY:		/* Print current position */
	    {
	        pfCoord coord;
		pfiGetXformerCoord(ViewState->xformer, &coord);
		fprintf(stderr, "Viewpoint: XYZ(%f,%f,%f) HPR(%f,%f,%f)\n",
			coord.xyz[PF_X], coord.xyz[PF_Y], coord.xyz[PF_Z],
			coord.hpr[PF_H], coord.hpr[PF_P], coord.hpr[PF_R]);
		break;
	    }
	    }
	}
    }
    events->numDevs = 0;
}
