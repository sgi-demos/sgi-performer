/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 */

#include <stdlib.h>
#include <string.h>
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include "xwin.h"

static int MaxScreenX = 0;
static int MaxScreenY = 0;

#ifdef WIN32
#define strncasecmp _strnicmp
#endif


PFUDLLEXPORT
int pfuGetNumMCOChannels(pfPipe *p)
{

    int nVChans = 0;
    int scr=0;
    const char *gstr = NULL;

    /* if we can use XSGIvc then great ! */
    scr = pfGetPipeScreen(p);
    if (scr < 0)
    {
#ifdef WIN32
        scr = 0;
#else
	scr = DefaultScreen(pfGetCurWSConnection());
#endif
    }
    nVChans = pfGetNumScreenVChans(scr);

    if (nVChans > 0)
	return nVChans;


    /* XSGIvc didn't exist so now we hack it based on machine type */

    /* get machine name */
    gstr = pfGetMachString();

    if (!gstr)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	"pfuGetNumMCOChans - error in determining machine type; could not config");
	return -1;
    }

    if (strncasecmp(gstr, "IMP", 3) == 0) /* IMPACT */
	return 4;

    /* Assume RE MCO modes */
    pfGetPipeSize(p, &MaxScreenX, &MaxScreenY);
    
    switch (MaxScreenX)
    {
	/* 3 Screens @ 960x680 - MCO Tiles Vertically*/
	case 2040:
	    return 3;
	    	
	case 1280:
	    switch (MaxScreenY)
	    {
		    /* 4 @ 640x480 (VS2) - MCO Uses Tiled Square */
		case 960:
		    return 4;
		    
		    /* 1@1280x1024 + 2@640x480 (VS2) - MCO Hacked Coords*/
		case 1504:
		    return 3;
		    
		    /* 2@1280x1024 (VS2) - MCO Tiled Vertically */
		case 2048:
		    return 2;
		    
		    /* 3@1025x768 (VS2) - More MCO Funky Tiles */
		case 2055:
		    return 3;
		    
		    /* 1@1280x1024 - No MCO so Tile Horizontal */
		case 1024:
		default:
		    return 1;
	    }
	/* 1 Screen @ ... - No MCO So Tile Horizontally */
	case 960:
	case 1024:
	case 1025:
	case 1600:
	default:
 	    return 1;
    }
    /* IMPACT 1280x1024 cut into for quadrants */
}

PFUDLLEXPORT
void pfuConfigMCO(pfChannel **chn, int nChans)
{
    pfPipe *p = pfGetChanPipe(chn[0]);
    pfPipeWindow *pw = pfGetChanPWin(chn[0]);
    const char *gstr = NULL;
    int	wxo, wyo, wxs, wys;
    int i, scr, nVChans, mapVChans=0;
    int pipeNum = -1;
    static	int inited[MAX_PIPES] = {
	    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0
    };


    scr = pfGetPipeScreen(p);
    pipeNum = pfGetId(p);
    if (scr < 0)
    {
#ifndef WIN32
	scr = DefaultScreen(pfGetCurWSConnection());
#endif
    }
    nVChans = pfGetNumScreenVChans(scr);

    /*
     * If pipe-window has no screen, its pfPipeVideoChannel's won't be able
     * to getSize. 
     */
    if (pfGetPWinScreen(pw) < 0)
	pfPWinScreen(pw, scr);

    if (nChans < 0) 
    { /* force XSGIvc mapping */
	nChans *= -1;
	if (nVChans < nChans)
	{
	    mapVChans = 0;
	    if (!inited[pipeNum])
		pfNotify(PFNFY_WARN,PFNFY_USAGE,
		"pfuConfigMCO() - too many channels requested %d. Only have %d.", 
		    nChans, nVChans); 
		pfNotify(PFNFY_WARN,PFNFY_MORE, 
		"Tiling withing window but not binding multiple XSGIvc video channels.");
		
	}
	else
	    mapVChans = 1;
    }

    /* default is original behavior - tile within window. So,
     * don't do the XSGIvc  mapping unleess they have a full screen
     * window and valid num of vchans; just tile within the window.
     */
    if (!mapVChans && (nVChans >= nChans))
    {
	int xs, ys, sxs, sys;
	pfGetPWinSize(pw, &xs, &ys);
	pfGetScreenSize(scr, &sxs, &sys);
	if (xs == sxs || ys == sys)
	    mapVChans = 1;
    }

    /* if we can use XSGIvc then great ! */
    if (mapVChans)
    {

    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"pfuConfigMCO: %d active video channels", nVChans);
    pfGetPWinOrigin(pw, &wxo, &wyo);
    pfGetPWinSize(pw, &wxs, &wys);

    if (nChans <= nVChans) 
    {
	for (i=0; i< nChans; i++)
	{
	    pfChannel *chan = chn[i];
	    pfPipeVideoChannel *pvc;
	    int 	index;
	    int	vchxo, vchyo, vchxs, vchys;
	    float	vpl, vpr, vpb, vpt;

	    /* This config doesn't check for old pvchans and unbind
	     * so this part can only be called once 
	     */
	    if (!inited[pipeNum])
	    {
		pvc = NULL;

		/* Create a video channel for each pfChannel on the current
		 *  pipe window 
		 */
		pfChanProjMode(chan, PFCHAN_PROJ_WINDOW);

		if (i==0) /* grab default video channel on pfPipeWindow */
		{
		    index = 0;
		    pvc = pfGetPWinPVChan(pw, 0);
		}
		if (!pvc)
		{
		    pvc = pfNewPVChan(p);
		    index = pfPWinAddPVChan(pw, pvc);
		}
		/* assign the video channel to a pipe window - will automatically
		 * be assigned a hw video channel Id
		 */
		pfNotify(PFNFY_DEBUG,PFNFY_PRINT, 
		    "Adding pipe video channel: %d with index: %d, active = %d\n", 
			pfGetPVChanId(pvc), index, pfIsPVChanActive(pvc)); 
		/* assign the pfChannel to the video channel */
		pfChanPWinPVChanIndex(chan, index);
	    }
	    else
		pvc = pfGetChanPVChan(chan);

	    /* align pfChannel Viewport to video channel area */
	    pfGetPVChanOrigin(pvc, &vchxo, &vchyo);
	    pfGetPVChanSize(pvc, &vchxs, &vchys);
	    vpl = (vchxo - wxo) / (float) wxs;
	    vpb = (vchyo - wyo) / (float) wys;
	    vpr = vpl + ((vchxs) / (float) wxs);
	    vpt = vpb + ((vchys) / (float) wys);
	    if (!inited[pipeNum])
		pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"Channel Viewport: %.2f %.2f %.2f %.2f", 
		    vpl, vpr, vpb, vpt);
	    pfChanViewport(chan, vpl, vpr, vpb, vpt);
	}
	if (!inited[pipeNum])
	    inited[pipeNum] = 1;
	return;
    }
    } /* map to video channels with Xsgivc */

    
    /* XSGIvc didn't exist so now we hack it based on machine type */

    /* get machine name */
    gstr = pfGetMachString();

    if (!gstr)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	"pfuConfigMCO - error in determining machine type; could not config");
	return;
    }

    if (strncasecmp(gstr, "IMP", 3) == 0) /* IMPACT */
    {
	 /* IMPACT MCO Uses Tiled Square */
	if (nChans == 2)
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "IMPACT MCO Channels:  2");
	    pfuTileChan(chn, 0, nChans, 0.0f, 0.5f, 0.5, 1.0f);
	    pfuTileChan(chn, 1, nChans, 0.5f, 1.0f, 0.5, 1.0f);
	}
	else if (nChans == 3)
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "IMPACT MCO Channels:  2,1");
	    pfuTileChan(chn, 0, nChans, 0.0f, 0.5f, 0.0, 0.5f);
	    pfuTileChan(chn, 1, nChans, 0.0f, 0.5f, 0.5, 1.0f);
	    pfuTileChan(chn, 2, nChans, 0.5f, 1.0f, 0.5, 1.0f);
	}
	else
	{
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, "IMPACT MCO Channels:  2x2");
	    pfuTileChans(chn, nChans, 2, 2);
	}
	return;
    }
    
    /* RE MCO modes */

    pfGetPipeSize(p, &MaxScreenX, &MaxScreenY);
    
    pfNotify(PFNFY_DEBUG,  PFNFY_PRINT, 
    		"Screen Info - Size: %d %d",MaxScreenX,MaxScreenY);
    switch (MaxScreenX)
    {
	/* 3 Screens @ 960x680 - MCO Tiles Vertically*/
	case 960:
	    if (MaxScreenY != 2040)
		pfNotify(PFNFY_INFO, PFNFY_PRINT,
		"Unknown MCO configuration");
    	    pfNotify(PFNFY_INFO,  PFNFY_PRINT, 
	             "Channels: Using 3@960x680");
	    pfuTileChans(chn, nChans, 1, 3);
	    break;
	    	
	case 1280:
	    switch (MaxScreenY)
	    {
		    /* 4 @ 640x480 (VS2) - MCO Uses Tiled Square */
		case 960:
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
				"Channels: Using 4@640x480");
		    pfuTileChans(chn, nChans, 2, 2);
		    break;
		    
		    /* 1@1280x1024 + 2@640x480 (VS2) - MCO Hacked Coords*/
		case 1504:
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
				"Channels: Using 1@1280x1024 + 2@640x480");
		    pfuTileChan(chn, 0, nChans, 0.0f, 1.0f, 0.319148936f, 1.0f);
		    pfuTileChan(chn, 1, nChans, 0.0f, 0.5f, 0.0f, 0.319148936f);
		    pfuTileChan(chn, 2, nChans, 0.5f, 1.0f, 0.0f, 0.319148936f);
		    break; 
		    
		    /* 2@1280x1024 (VS2) - MCO Tiled Vertically */
		case 2048:
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
				"Channels: Using 2@1280x1024");
		    pfuTileChans(chn, nChans, 1, 2);
		    break;
		    
		    /* 3@1025x768 (VS2) - More MCO Funky Tiles */
		case 2055:
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
				"Channels: Using 3@1025x768");
		    pfuTileChan(chn, 0, nChans, 0.0f, 0.498296837f, 0.0f, 0.5f);
		    pfuTileChan(chn, 1, nChans, 0.0f, 0.498296837f, 0.5f, 1.0f);
		    pfuTileChan(chn, 2, nChans, 0.501216545f, 1.0f, 0.0f, 0.5f);
		    break;
		    
		    /* 1@1280x1024 - No MCO so Tile Horizontal */
		case 1024:
		default:
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
				"Channels: Using 1@1280x1024");
		    pfuTileChans(chn, nChans, nChans, 1);
		    break;    
	    }
	    break;
	/* 1 Screen @ ... - No MCO So Tile Horizontally */
	case 1024:
	case 1025:
	case 1600:
	default:
	    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			"Channels: Using 1@%dx%d",MaxScreenX,MaxScreenY);
	    pfuTileChans(chn, nChans, nChans, 1);
	    break;
    }
}

PFUDLLEXPORT
void pfuTileChans(pfChannel **chn,  int nChans, int ntilesx, int ntilesy)
{
    int i=0, j=0;

    for(j=0;j<ntilesy;j++)
       for(i=0;i<ntilesx;i++)
	    if ((j*ntilesx+i) < nChans) {
	        pfNotify(PFNFY_INFO, PFNFY_PRINT,
			"MCO Chan[%d]:x(%f,%f) y(%f,%f)",
			j*ntilesx+i,
		    	(float)i/(float)ntilesx, 
		    	(float)(i+1)/(float)ntilesx, 
		    	(float)j/(float)ntilesy, 
		    	(float)(j+1)/(float)ntilesy);
		pfChanViewport(chn[j*ntilesx+i], 
		    (float)i/(float)ntilesx, 
		    (float)(i+1)/(float)ntilesx, 
		    (float)j/(float)ntilesy, 
		    (float)(j+1)/(float)ntilesy);
	    }

}

PFUDLLEXPORT
void pfuTileChan(pfChannel **chn, 
		int thisChan, int nChans, 
		float l, float r, float b, float t)
{
    if (thisChan < nChans)
		pfChanViewport(chn[thisChan], l, r, b, t);
}
