/*
 * Copyright 1995, Silicon Graphics, Inc.
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
 *
 *
** This program, cliptex, is a demo/test program for displaying a clip texture.
** it expects a clip texture configuration file as an argument. For information
** on clip texture configuration files, see pfdLoadClipTexture(3).
**
** The performer release has sample clipmap data you can run under
**  data/clipdata. The usual convention for naming clip texture configuration
** files is nameofdata.ct You should check the files for environment variable
** names. They may need to be set in order for the cliptexture config file
** to point to the data.
**
** cliptex shows a birds-eye view of the data, roaming the clip square back
** and forth. You can see the different levels of details as progressivly
** larger blurred torroidal zones around the clip center, which will
** be sharpest.
**
** You may want to use this program as a starting point to writing you
** own libpr-style cliptexture program. It is not as appropriate for
** libpf-style programs.
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <Performer/pfdu.h>
#include <Performer/pr.h>
#ifndef WIN32
#include <X11/keysym.h>
#include <sys/unistd.h>
#endif
#include <sys/types.h>
#ifndef WIN32
#include <sys/uio.h>
#endif
#include <fcntl.h>

#define ZMIN -.1
#define ZMAX .1
#define XMIN -.1
#define XMAX .1
#define YMIN -.1
#define YMAX .1

#define DLIST 1

pfClipTexture	*clip;
char		*clipfname;

static void 
do_events(Display *dsp, int *timing, int *move)
{
    static uint DTRMode = 0;
#ifndef WIN32
    while (XPending(dsp))
    {
	XEvent event;
	
	XNextEvent(dsp, &event);
	switch (event.type) 
	{
	case KeyPress:
	{
	    char buf[100];
	    int rv;
	    KeySym ks;

	    rv = XLookupString(&event.xkey, buf, sizeof(buf), &ks, 0);
	       
	    switch(ks) 
	    {
	    case XK_Escape:
		exit(0);
		break;
	    case XK_g:
	    case XK_G:
		if(*move) /* toggle movement */
		{
		    *move = 0;
		    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Stop Moving");
		}
		else
		{
		    *move = 1;
		    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Start Moving");
		}
		break;
	    case XK_t:
	    case XK_T:
		    if(DTRMode & PF_DTR_TEXLOAD)
		    {
			DTRMode &= ~PF_DTR_TEXLOAD;
			pfNotify(PFNFY_INFO, PFNFY_PRINT,
				 "Texture Download Load Control Disabled");
		    }
		    else
		    {
			DTRMode |=  PF_DTR_TEXLOAD;
			pfNotify(PFNFY_INFO, PFNFY_PRINT,
				 "Texture Download Load Control Enabled");
		    }
		    pfClipTextureDTRMode(clip, DTRMode);
		break;
	    case XK_m:
	    case XK_M:
		    if(DTRMode & PF_DTR_MEMLOAD)
		    {
			DTRMode &= ~PF_DTR_MEMLOAD;
			pfNotify(PFNFY_INFO, PFNFY_PRINT,
				 "Memory Download Load Control Disabled");
		    }
		    else
		    {
			DTRMode |=  PF_DTR_MEMLOAD;
			pfNotify(PFNFY_INFO, PFNFY_PRINT,
				 "Memory Download Load Control Enabled");
		    }
		    pfClipTextureDTRMode(clip, DTRMode);
		break;
	    case XK_r:
	    case XK_R:
		    /* toggle read queue sorting bit */
		    if(DTRMode & PF_DTR_READSORT)
		    {
			DTRMode &= ~PF_DTR_READSORT;
			pfNotify(PFNFY_INFO, PFNFY_PRINT,
				 "Read Queue Sorting Disabled");
		    }
		    else
		    {
			DTRMode |=  PF_DTR_READSORT;
			pfNotify(PFNFY_INFO, PFNFY_PRINT,
				 "Read Queue Sorting Enabled");
		    }
		    pfClipTextureDTRMode(clip, DTRMode);
		break;
	    case XK_s:
	    case XK_S: /* toggle performance statistics */
		*timing = !(*timing);
		if(*timing)
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			     "Performance Timing Enabled");
		else
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			     "Performance Timing Disabled");
		break;
	    case XK_d:
	    case XK_D: /* delete cliptexture */
		if(clip)
		{
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			     "Deleting Clip Texture");
		    pfDelete(clip); /* ref count problems */
		    clip = 0;
		    pfDisable(PFEN_TEXTURE);
		}
		break;
	    case XK_l:
	    case XK_L: /* load cliptexture */
		if(!clip)
		{
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			     "Loading Clip Texture");
		    clip = pfdLoadClipTexture(clipfname);
		    pfEnable(PFEN_TEXTURE);
		}
		break;
	    }/* key switch */
	}
	}/* dev switch */
    } /* while events */
#endif
}


int main(int argc, char **argv)
{
    static int		centerS,centerT;
    char		namebuf[256];
    Display             *dsp;
    Window	         xwin;
    pfImageTile		*itile;
    pfTexEnv 		*tev; 	
    pfFrustum 		*frust;
    pfLight		*light;
    pfLightModel 	*lm;
    pfGeoSet	       	*geom;   
    pfMaterial 		*mat;
    pfDispList		*draw_quad, *increment_position;
    pfWindow		*win;
    int			is5551;
    float            	head, pitch, roll, sign;
    int 		i;
    int			ldS = 128, ldT = 128, srcS = 0, srcT = 0, dstS = 0, dstT =0;
    float            	e = 0.0f, f = 0.0f, g = 0.0f;
    int			quit = 0;
    int			vSize,cSize,nTiles,tSize,maxLoadSize,cacheS,cacheT;
    int                 dummy;
    pfVec3		*coords;
    pfVec3		*norms;
    pfVec2		*tcoords;
    int                 move = 1;
    int                 timing = FALSE;
    double              time = 0.;

    if(argc == 1) {
	fprintf(stderr, "Usage: %s cliptexture_config_file\n", argv[0]);
	return -1;
    }

    clipfname = strdup(argv[1]);

    pfNotifyLevel(PFNFY_INFO);
    pfInit();
    pfInitArenas();
    pfInitState(NULL);
    pfInitClock(0.0);


    win=pfNewWin(NULL);
    pfWinOriginSize(win, 0, 0, 1024, 1024);
    pfWinName(win, "Performer Roam");
    pfWinType(win, PFWIN_TYPE_X);
    pfOpenWin(win);
    dsp = pfGetCurWSConnection();
    xwin = pfGetWinWSWindow(win);
#ifndef WIN32
    XSelectInput(dsp, xwin,  KeyPressMask);
#endif
    pfInitGfx();
    frust = pfNewFrust(NULL);
    pfMakeSimpleFrust(frust, 45.0f);
    pfFrustNearFar(frust, 1.0f, 100.0f);
    pfApplyFrust(frust);
    pfDelete(frust);

    pfAntialias(PFAA_ON);  
    pfCullFace(PFCF_OFF);
    
    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures");
    
    geom = pfNewGSet(NULL); 

    coords = (pfVec3 *)pfMalloc(sizeof(pfVec3) * 3 * 4, pfGetArena(NULL));
    norms = (pfVec3 *)pfMalloc(sizeof(pfVec3) * 3 * 4, pfGetArena(NULL));
    tcoords = (pfVec2 *)pfMalloc(sizeof(pfVec3) * 2 * 4, pfGetArena(NULL));

    pfSetVec3(coords[0], XMIN, YMIN, ZMAX);
    pfSetVec3(coords[1], XMAX, YMIN, ZMAX);
    pfSetVec3(coords[2], XMAX, YMAX, ZMAX);
    pfSetVec3(coords[3], XMIN, YMAX, ZMAX);

    pfSetVec3(norms[0], 0., 1., 0.);
    pfSetVec3(norms[1], 0., 1., 0.);
    pfSetVec3(norms[2], 0., 1., 0.);
    pfSetVec3(norms[3], 0., 1., 0.);

    pfSetVec2(tcoords[0], 0., 0.);
    pfSetVec2(tcoords[1], 1., 0.);
    pfSetVec2(tcoords[2], 1., 1.);
    pfSetVec2(tcoords[3], 0., 1.);


    pfGSetAttr(geom, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(geom, PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
    pfGSetAttr(geom, PFGS_NORMAL3, PFGS_PER_PRIM, norms, NULL);
                           
    pfGSetPrimType(geom, PFGS_QUADS);
    pfGSetNumPrims(geom, 1);

	/* Setup Texture Environment */
    tev = pfNewTEnv(NULL);
    pfApplyTEnv(tev);
    pfEnable(PFEN_TEXTURE);

    /* Setup Clipped Texture */

    clip = pfdLoadClipTexture(clipfname);
    if(!clip) {
	fprintf(stderr, 
		"%s pfdLoadClipTexture failed; "
		"please check configuration file\n",
		argv[0]);
	return -1;
    }

    /* DONE WITH CACHE SETUP */

#if DLIST
    draw_quad = pfNewDList(PFDL_FLAT, 0,NULL);
    increment_position = pfNewDList(PFDL_FLAT, 0,NULL);
    pfOpenDList(draw_quad);
    pfPushMatrix();
    pfScale(10.0,10.0,10.0);
    pfDrawGSet(geom);
    pfPopMatrix();
    pfCloseDList();
#endif

    head = pitch = roll = 0.0;
    sign = 1;
    pfTranslate(0.0,0.0,-4.0);


    /* XXX the following is temporary initialization constraint */
    /* The Initial Center Position must be such that all of the tiles */
    /* Are able to have non clamped origins - ie each tile must have */
    /* implied origin >= 0 based on initial center */
  
    cSize = pfGetClipTextureClipSize(clip);
    pfGetClipTextureVirtualSize(clip, &vSize, &dummy, &dummy);
    centerS = centerT = vSize/2;

    while(!quit)
    {
	int		offS,offT, offR;
	int		tSizeS,tSizeT,tSizeR,ncomps;
	uint		*img;
	pfMatrix	tmat;
	static pfVec4	clr = {0.1f, 0.0f, 0.4f, 1.0f};

	pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
        pfPushMatrix();

		/* Reset and Open CACHE Display List */
#ifdef DLIST
	pfResetDList(increment_position);
	pfOpenDList(increment_position);
#endif
		/* READ IN NEW CLIP CENTER */
	{
	    static int signS = 1;
	    static int signT = 1;
#define LSIZE 8
	centerS += LSIZE* signS * move;
	centerT += LSIZE* signT * move;
	if (centerS >= vSize-cSize/2)
	    signS = -1;
	if (centerS <= 0)
	    signS = 1;
	if (centerT >= vSize-cSize/2)
	    signT = -1;
	if (centerT <= 0)
	{
	    /* start of timing cycle */
	    if(timing && signT == -1)
	    {
		double end;
		end = (float)pfGetTime();
		fprintf(stderr, 
			"elapsed time: %.3f seconds\n",
			(float)(end - time));
		do_events(dsp, &timing, &move);
		time = pfGetTime();
	    }
	    signT = 1;
	}
	}
#if 0
	fprintf(stderr,"CMAP - Center %d %d\n",centerS,centerT);
#endif



	if(clip)
	{
	    pfClipTextureCenter(clip, centerS, centerT, 0);
	    pfApplyClipTexture(clip);
	}

	/* Close Cache update Display List */
#ifdef DLIST
	pfCloseDList();
#endif

	/* Apply IMAGECACHE update DLIST - actually perform subloads */
#ifdef DLIST
	pfDrawDList(increment_position);

	pfDrawDList(draw_quad);
#else
    	pfPushMatrix();
    	pfScale(10.0,10.0,10.0);
    	pfDrawGSet(geom);
    	pfPopMatrix();
#endif

    	pfPopMatrix();
    	pfSwapWinBuffers(win);
	do_events(dsp, &timing, &move);
    }
    exit(0);
}





