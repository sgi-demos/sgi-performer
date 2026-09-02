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

** This program, icache, is a demo/test program for displaying a single image
** cache. It expects a image cache configuration file as an argument. For more
** information on image cache configuration files, see pfdLoadImageCache(3).
**
** The performer release has sample image cache data you can run under
** data/clipdata. The usual convention for naming image cache configuration
** files is nameofdata.ic You should check the files for environment variable
** names. They may need to be set in order for the image cache config file
** to point to the data.
**
** icache shows an oblique view of the data on a square polygon, then 
** appears to roam the data from one corner of the texture to the other by
** translating the texture coordinates using the texture matrix.
**
** This test is a good way to test image cache configuration files for each
** level of a clip texture before trying to run with the entire clip texture.
**
** NOTE: Even though this is a libpr program, since it uses libpfdb, it
** is a libpf program (in a sense) because pfInit get's called by the
** libpfdb library. Beware! This won't usually be a problem, but it's possible
** to get in trouble if you are depending on pfInit not being called.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef WIN32
#include <string.h>
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

#define FILENAME_ARG 1
#define MAXLOADSIZE_ARG 2	       	     

#define ZMIN -.1
#define ZMAX .1
#define XMIN -.1
#define XMAX .1
#define YMIN -.1
#define YMAX .1

#define XSIZ (XMAX - XMIN)
#define YSIZ (YMAX - YMIN)
#define ZSIZ (ZMAX - ZMIN)

pfImageCache *icache; /* image cache */
char *icfname; /* image cache file name */
pfTexture *tex; /* texture */


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
		    pfImageCacheDTRMode(icache, DTRMode);
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
		    pfImageCacheDTRMode(icache, DTRMode);
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
		    pfImageCacheDTRMode(icache, DTRMode);
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
		if(icache)
		{
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			     "Deleting Clip Texture");
		    pfDelete(tex);
		    tex = 0;
		    pfDelete(icache);
		    icache = 0;
		    pfDisable(PFEN_TEXTURE);
		}
		break;
	    case XK_l:
	    case XK_L: /* load cliptexture */
		if(!icache)
		{
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			     "Loading Clip Texture");
		    tex = pfNewTex(NULL);
		    pfTexFilter(tex, PFTEX_MINFILTER, PFTEX_BILINEAR);
		    icache = pfdLoadImageCache(icfname, tex, 0);
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
    int texRegOrgS, texRegOrgT;
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
    int			is5551,LSIZE;
    float		sign;
    int 		i;
    int                 timing = FALSE;
    double              time = 0.;
    int                 ncomps;
    int ldS = 128, ldT = 128, srcS = 0, srcT = 0, dstS = 0, dstT =0;
    int			quit = 0;
    int icsize, nTiles, maxLoadSize;
    int memRegOrgS, memRegOrgT;
    int tileS, tileT;
    int texRegSizeS, texRegSizeT; /* current texture region size */
    int dummyarg; /* used to ignore arguments */
    char *loadsize;
    pfVec3	*coords; /* geometry attributes for quad */
    pfVec3	*norms;
    pfVec2	*tcoords;
    int         texSizeS,texSizeT,texSizeR;
    uint        *img;
    float       remaining, msecs = 16.f;
    int         move = 1;

    if(argc == 1) {
	fprintf(stderr, "Usage: %s icache_config_file [maxloadsize]\n", 
		argv[0]);
	return -1;
    }

    icfname = strdup(argv[FILENAME_ARG]);

    if (argc > MAXLOADSIZE_ARG)
        maxLoadSize = atoi(argv[MAXLOADSIZE_ARG]);
    else 
	maxLoadSize = 32;


    pfNotifyLevel(PFNFY_INFO);
    pfInit();
    pfInitArenas();
    pfInitState(NULL);
    pfInitClock(0.0);

    win=pfNewWin(NULL);
    pfWinOriginSize(win, 0, 0, 512, 512);
    pfWinName(win, "Performer image cache");
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
    pfFrustNearFar(frust, .1f, 100.0f);
    pfApplyFrust(frust);
    pfDelete(frust);

    pfAntialias(PFAA_ON);  
    pfCullFace(PFCF_OFF);
    
    pfFilePath(".:/usr/share/Performer/data/clipdata/hunter:"
	       "/usr/share/Performer/data/clipdata/moffett");
    
    geom = pfNewGSet(NULL); 


    coords = (pfVec3 *)pfMalloc(sizeof(pfVec3) * 4, NULL);
    tcoords = (pfVec2 *)pfMalloc(sizeof(pfVec2) * 4, NULL);

    pfSetVec3(coords[0], XMIN, YMAX, ZMAX);
    pfSetVec3(coords[1], XMAX, YMAX, ZMAX);
    pfSetVec3(coords[2], XMAX, YMAX, ZMIN);
    pfSetVec3(coords[3], XMIN, YMAX, ZMIN);

    pfSetVec2(tcoords[0], 0., 0.);
    pfSetVec2(tcoords[1], 1., 0.);
    pfSetVec2(tcoords[2], 1., 1.);
    pfSetVec2(tcoords[3], 0., 1.);

    pfGSetAttr(geom, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(geom, PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
                           
    pfGSetPrimType(geom, PFGS_QUADS);
    pfGSetNumPrims(geom, 1);

    /* Setup Texture Environment */
    tev = pfNewTEnv(NULL);
    pfApplyTEnv(tev);
    pfEnable(PFEN_TEXTURE);

    
    /* SETUP IMAGE ROAM */

    tex = pfNewTex(NULL);
    pfTexFilter(tex, PFTEX_MINFILTER, PFTEX_BILINEAR);

    icache = pfdLoadImageCache(icfname, tex, 0);
    if(!icache) {
	fprintf(stderr, 
		"%s Loading ImageCache failed. Please check config file\n",
                 argv[0]);
	return -1;
    }

    /* DONE WITH CACHE SETUP */

    draw_quad = pfNewDList(PFDL_FLAT, 0, NULL);
    increment_position = pfNewDList(PFDL_FLAT, 0, NULL);

    /* create display list to draw a quad */
    pfOpenDList(draw_quad);
    pfPushMatrix();
    pfScale(40.0,10.0,100.0); /* scale quad to reasonable size */
    pfDrawGSet(geom);
    pfPopMatrix();
    pfCloseDList();

    sign = 1;
    pfTranslate(0.0,-1.6,-4.0);

    LSIZE = 32; /* default load size */
    loadsize = getenv("LOADSIZE"); /* LOADSIZE environment variable overrides */
    if(loadsize) /* if LOADSIZE is defined, use it */
	LSIZE = atoi(loadsize);
    if (LSIZE < 8) /* clamp LOADSIZE to 8 or greater */
	LSIZE = 8;

    pfGetImageCacheImageSize(icache, &icsize, NULL, NULL);
    pfGetImageCacheTexRegionSize(icache, &texRegSizeS, &texRegSizeT, NULL);
    pfGetImageTileSize(pfGetImageCacheProtoTile(icache), &tileS, &tileT, NULL);
    pfGetImageCacheMemRegionSize(icache, &nTiles, NULL, NULL);
    pfGetTexImage(tex,&img,&ncomps,&texSizeS,&texSizeT,&texSizeR);

    texRegOrgS = icsize/2 - texRegSizeS/2;
    texRegOrgT = 0;

    while(!quit)
    {
	int readSort = FALSE;
	int offS, offT, offR;
	int sizeS, sizeT;
	float ratS, ratT;
	pfMatrix tmat;
	static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};
	pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
        pfPushMatrix();
	pfRotate(PF_X, 10.0);

	pfDrawDList(draw_quad);
	pfPopMatrix();
	pfSwapWinBuffers(win);

	
	{
	    static int signS = 1;
	    static int signT = 1;

	    texRegOrgS += LSIZE * signS * move;
	    texRegOrgT += LSIZE * signT * move;
	    if (texRegOrgS >= icsize - texRegSizeS - 2*LSIZE )
		signS = -1;
	    if (texRegOrgS <= 0)
		signS = 1;
	    if (texRegOrgT >= icsize - texRegSizeT - 2*LSIZE)
		signT = -1;
	    if (texRegOrgT <= 0)
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
	/* Keep memRegion center close to texRegion center */
	/* use destinate texture size since texregion varies */

	memRegOrgS = PF_MAX2((int)
			 ((texRegOrgS + texSizeS/2)/(float)tileS - 
			  (float)nTiles/2.0),
			 0);
	memRegOrgT = PF_MAX2((int)
			 ((texRegOrgT + texSizeT/2)/(float)tileT - 
			  (float)nTiles/2.0),
			 0);
	memRegOrgS = PF_MIN2(memRegOrgS, icsize/tileS - nTiles);
	memRegOrgT = PF_MIN2(memRegOrgT, icsize/tileT - nTiles);

	if(icache)
	{
	    /* Set NEW mem Region ORIGIN  (based on texRegion origin) */
	    pfImageCacheMemRegionOrg(icache, memRegOrgS, memRegOrgT, 0);

	    /* Set NEW texRegion Origin (based on incremental change) */
	    pfImageCacheTexRegionOrg(icache, texRegOrgS, texRegOrgT, 0);

	    /* Apply IMAGE CACHE - start new disk reads, generate texloads, 
	    ** apply them.
	    */
	    remaining = pfDTRApplyImageCache(icache, msecs);
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		     "timing: given %.2f, remaining %.2f", msecs, remaining);

	    /* Get torroidal offset and use it to set the texture matrix */
	
	    glMatrixMode(GL_TEXTURE);
	    pfGetImageCacheTexRegionOffset(icache, &offS, &offT, &offR);
	    pfMakeTransMat(tmat,
			   offS/(float)texSizeS,
			   offT/(float)texSizeT,
			   offR/(float)texSizeR);
	    pfLoadMatrix(tmat);
	    glMatrixMode(GL_MODELVIEW);
	    pfGetImageCacheCurTexRegionSize(icache, 
					    &texRegSizeS, &texRegSizeT, NULL);

	    /* how much did we have to shrink to keep on schedule? */
	    ratS = texRegSizeS/(float)texSizeS;
	    ratT = texRegSizeT/(float)texSizeT;
	    pfSetVec2(tcoords[1], ratS, 0.f);
	    pfSetVec2(tcoords[2], ratS, ratT);
	    pfSetVec2(tcoords[3], 0.f,  ratT);
	    
	    pfSetVec3(coords[0], XMIN, YMAX, ZMAX);
	    pfSetVec3(coords[1], XMIN + XSIZ * ratS, YMAX, ZMAX);
	    pfSetVec3(coords[2], XMIN + XSIZ * ratS, YMAX, ZMAX - ZSIZ * ratT);
	    pfSetVec3(coords[3], XMIN, YMAX, ZMAX - ZSIZ * ratT);

	}
	if(!timing) /* to avoid X11 overhead screwing up timing */
	    do_events(dsp, &timing, &move);
    }
    exit(0);
}



