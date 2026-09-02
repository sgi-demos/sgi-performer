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
 * texlod.c - texlod example/test
 *
 * example: texlod 
 *
 * $Revision: 1.8 $
 * $Date: 2002/11/13 00:23:21 $
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h>
#endif
#include <string.h>
#ifndef WIN32
#include <X11/keysym.h>
#endif
#include <Performer/pr.h>

static void do_events(void);
static void draw_scene(void);
static void draw_boxes(void);


pfVec4          scolors[] ={{1., 1., 1., 1.}, 
			    {1., 0., 0., 1.},
			    {0., 1., 0., 1.},
			    {0., 0., 1., 1.}};


pfVec3          vcoords[] ={{0., 0, 0.},
			    {1., 0, 0.},
			    {1., 1, 0.},
			    {0., 1, 0.}};	
			    
pfVec2          tcoords[] ={{0., 0.},
			    {1., 0.},
			    {1., 1.},
			    {0., 1.}};				    

#define MAX_LEVELS 10
static unsigned int clrs[] = {
	0x108f8fff, /* base is teal */
	0xff0000ff, /* level 1 is red */
	0x00ff00ff, /* level 2 is green */
	0x0000ffff, /* level 3 is blue */
	0xff00ffff, /* level 4 is magenta */
	0xfff00fff, /* level 5 is yellow */
	0x00ffffff, /* level 6 is cyan */
	0xf0f0f0ff, /* level 7 is grey */
	0xf0f010ff, /* level 8 is ground */
	0xf010f0ff, /* level 9 is sky */
};

static int WinXSize = 500, WinYSize = 700;
static pfDispList* dlist;
static pfGeoSet	*geom[10];
static int TexLevels = 6;
static int Enable = 1;
static int DListMode = 0;
static pfTexLOD *GlobalTLOD;
static int GlobalRange=0;

#ifndef WIN32
static Display *Dsp;
#endif
static pfWindow *Win;

static char OptionStr[] = "rdL:R?";

static long
docmdline(int argc, char **argv)
{
     extern char *optarg;
     extern int  optind;
     int opt;
        
     while ((opt = getopt(argc,argv,OptionStr)) != -1) {
	 switch(opt) {
	 /* custom options */
	 case 'd':
	    DListMode ^= 1;
	    break;
         case 'L':
	    TexLevels = atoi(optarg);
	    break;
	 case '?':
	 default:
	    pfNotify(PFNFY_FATAL,PFNFY_USAGE,
		"mipmap [-rRf] [-L num] \n");
	} /* switch */
    } /* while */
    
    return argc - optind;
}

long
main(long argc, char *argv[])
{
    char*	     str;
    pfFrustum*	     frust;
    pfTexture*       tex[10];
    pfTexLOD	    *tlod;
    pfGeoState	    *gstate;
    Window	     xwin;
    unsigned int    *image[10];
    char tstr[PF_MAXSTRING];
    int		     num,  mapSize;
    int		     comp, xs, ys, zs;
    int	    	     i, j, k, t;
    int	    	     arg;
    int		     start = 0;
    int		     frame = 1;
    float min,max,bs,bt,br;


     /* Initialize Performer */
    pfNotifyLevel(PFNFY_INFO);
    pfInit();
    pfInitState(NULL);
    
    
    /* cmdline options */
    arg = docmdline(argc, argv);
    
    /* Initialize GL */
    Win = pfNewWin(NULL);
    pfWinOriginSize(Win, 0, 0, WinXSize, WinYSize);
    pfWinName(Win, "texlod");
    pfWinType(Win, PFWIN_TYPE_X);
    pfOpenWin(Win);
#ifndef WIN32
    Dsp = pfGetCurWSConnection();
    xwin = pfGetWinWSWindow(Win);
    XSelectInput(Dsp, xwin,  KeyPressMask);
    XMapWindow(Dsp, xwin);
    XSync(Dsp,FALSE);
#endif
    
    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures");
    /* make RGBA MIPmaps of different colors */
    
    num = 1 << TexLevels;
    for (i=0; i <= TexLevels; i++)
    {
	mapSize = num * num;
	image[i] = pfMalloc(sizeof(int)*mapSize, NULL);
	for (k=0; k < mapSize; k++)
	    image[i][k] = clrs[i];
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"Level %d has color 0x%x", i, clrs[i]);
	num >>= 1;
    }
    
    for (t=0; t < 6; t++)
    {
	tex[t] = pfNewTex(NULL);
	if (t == 0)
	{
	    pfGetTexLODRange(tex[0], &min, &max);
	    pfGetTexLODBias(tex[0], &bs, &bt, &br);
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Default Tex LOD values: [%.1f-%.1f], %.1f %.1f %.1f",
		min,max,bs,bt,br);
	}
	pfTexFormat(tex[t], PFTEX_GEN_MIPMAP_FORMAT, 0);
	sprintf(tstr, "level %d", i);
	pfTexName(tex[t], tstr);
	num = 1 << TexLevels;
	pfTexImage(tex[t], image[0], 4, num, num, 1);
	for (i=1; i <= TexLevels; i++)
	{
	    pfTexture *ttex;
	    ttex = pfNewTex(NULL);
	    sprintf(tstr, "level=%d", i);
	    pfTexName(ttex, tstr);
	    num >>= 1;
	    pfTexImage(ttex, image[i], 4, num, num, 1);
	    pfTexLevel(tex[t], i, ttex);
	}
	
	geom[t] = pfNewGSet(NULL);
	pfGSetAttr(geom[t], PFGS_COORD3, PFGS_PER_VERTEX, vcoords, NULL);
	pfGSetAttr(geom[t], PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
	pfGSetAttr(geom[t], PFGS_COLOR4, PFGS_OVERALL, scolors, NULL);
	pfGSetPrimType(geom[t], PFGS_QUADS);
	pfGSetNumPrims(geom[t], 1);
	gstate = pfNewGState(NULL);

	switch(t)
	{
	case 0:
	case 2:
	    pfTexLODRange(tex[t], t+1, t+1);
	    break;
	case 3:
	    pfGStateMode (gstate, PFSTATE_ENTEXLOD, 1);
	case 1:
	    tlod = pfNewTLOD(NULL);
	    pfTLODRange(tlod, t+1, t+1);
	    pfGStateAttr (gstate, PFSTATE_TEXLOD, tlod); 
	    break;
        case 4:
	    pfGStateMode (gstate, PFSTATE_ENTEXLOD, 0);
	    break;
	default:
	    break;
	}
	pfGStateAttr (gstate, PFSTATE_TEXTURE, tex[t]);
	pfGSetGState(geom[t], gstate);
    }
    
    fprintf(stderr, "Texture has %d levels\n", TexLevels);
    
    pfCullFace(PFCF_OFF);
    pfEnable(PFEN_TEXTURE);
    pfApplyTEnv(pfNewTEnv(NULL));
    
    GlobalTLOD = tlod = pfNewTLOD(NULL);
    pfGetTLODRange(tlod, &min, &max);
    pfGetTLODBias(tlod, &bs, &bt, &br);
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Default TexLOD values: [%.1f-%.1f], %.1f %.1f %.1f",
	min,max,bs,bt,br);
    pfTLODBias(tlod, .1, .1, .1);
    pfTLODRange(tlod, 0, TexLevels);
    GlobalRange = TexLevels;
    pfApplyTLOD(GlobalTLOD);
    /* get and set test */
    for (i=0; i < 6; i++)
    {
	pfApplyGState(pfGetGSetGState(geom[i]));
	pfGetTexLODRange(tex[i], &min, &max);
	pfGetTexLODBias(tex[i], &bs, &bt, &br);
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Tex %d LOD values: [%.1f-%.1f], %.1f %.1f %.1f",
	    i,min,max,bs,bt,br);
	pfGetCurTexLODRange(tex[i], &min, &max);
	pfGetCurTexLODBias(tex[i], &bs, &bt, &br);
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Cur Tex %d LOD values: [%.1f-%.1f], %.1f %.1f %.1f",
	    i,min,max,bs,bt,br);
    }

    /* make a square for each MIPmap level in the dlist */
    if (DListMode)
    {
	dlist = pfNewDList(PFDL_FLAT, 32, NULL);
	pfOpenDList(dlist);
	draw_boxes();
	pfCloseDList();
    }

    while(1)
    {
	if (Enable)
	    pfEnable(PFEN_TEXLOD);
	else
	    pfDisable(PFEN_TEXLOD);
	pfGetTLODRange(GlobalTLOD, &min, &max);
	if (Enable && (min != GlobalRange))
	{
	    pfTLODRange(GlobalTLOD, GlobalRange, GlobalRange);
	}
	pfApplyTLOD(GlobalTLOD);
	draw_scene();
	do_events();
	{
       int err;
       if ((err = glGetError()) != GL_NO_ERROR)
        pfNotify(PFNFY_NOTICE,PFNFY_USAGE,"OpenGL Error 0x%x - %s",err, gluErrorString(err));
	}
    }
    return 0;
}

static void 
do_events(void)
{
#ifndef WIN32
    long do_draw = 0;
    long hlmode = 0;
    long ov = 0;
    int dev;
    short val;
    
    while (XPending(Dsp))
    {
	XEvent event;
	
	XNextEvent(Dsp, &event);
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
	    case XK_e:
		Enable ^= 1;
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"TexLOD enable = %d", Enable);
		break;
	    case XK_Up:
		GlobalRange++;
		if (GlobalRange > TexLevels)
			GlobalRange = TexLevels;
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"TexLOD global range = %d", GlobalRange);
		break;
	    case XK_Down:
		GlobalRange--;
		if (GlobalRange < -1)
			GlobalRange = -1;
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"TexLOD global range = %d", GlobalRange);
		break;
	    case XK_Escape:
		exit(0);
		break;
	    default:
		break;
	    }/* key switch */
	    }
	}/* dev switch */
    } /* while events */
#endif
}

static void draw_boxes()
{
    int i, j;
    float xdist = 1.1f;

    pfPushMatrix();    
    pfScale(200.0f, 200.0f, 0.0f);
    for (i=0; i < 3; i++)
    {
	pfPushMatrix();    
	for (j=0; j < 2; j++)
	{
	    pfDrawGSet(geom[i*2 + j]);
	    pfTranslate(xdist, 0.0f, 0.0f);
	}
	pfPopMatrix();
	pfTranslate(0.0f, xdist, 0.0f);
    }
    pfPopMatrix();
}

static void draw_scene(void) 
{
    static pfVec4 clr = {0.0f, 0.0f, 0.1f, 1.0f};

    pfClear(PFCL_COLOR | PFCL_DEPTH, clr);

    if (DListMode)
	pfDrawDList(dlist);	/* draw all display lists */
    else
	draw_boxes();

    pfSwapWinBuffers(Win);  
   
}
