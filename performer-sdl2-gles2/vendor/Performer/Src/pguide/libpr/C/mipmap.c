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
 * mipmap.c - mipmap level test
 *
 * example: mipmap 
 *
 * Displays the different defined MIPmap levels of a MIPmapped texture
 *
 * mipmap texfile - will show the levels of the specified texture file
 *
 * -L num - makes MIPmapped texture with num levels and downloads
 *		them every frame
 * -r	- uses the subload format for downloading the mipmaps
 * -R 	- turn off the per-frame download 
 *
 *
 * $Revision: 1.13 $
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

static pfDispList* dlist[4];
static pfGeoSet	*geom;
pfTexture *base;
pfList *texList;
pfList *imageList; 
int TexLevels = 6;
char **texFileNames;
int FastDefine = 0;
int TexLoadMode = PFTEX_BASE_APPLY;
int WinXSize = 700, WinYSize = 700;
int RunMode = 1;

#ifndef WIN32
Display *Dsp;
#endif
pfWindow *Win;

static char OptionStr[] = "rFL:R?";

static long
docmdline(int argc, char **argv)
{
     extern char *optarg;
     extern int  optind;
     int opt;
        
     while ((opt = getopt(argc,argv,OptionStr)) != -1) {
	 switch(opt) {
	 /* custom options */
	 case 'F':
	    FastDefine = 1;
	    break;
	 case 'r':
	    TexLoadMode = PFTEX_BASE_AUTO_SUBLOAD;
	    FastDefine = 1;
	    break;
         case 'L':
	    TexLevels = atoi(optarg);
	    break;
	 case 'R':
	    RunMode ^= 1;
	    break;
	 case '?':
	 default:
	    pfNotify(PFNFY_FATAL,PFNFY_USAGE,
		"mipmap [-rRf] [-L num] \n");
	} /* switch */
    } /* while */
    
    texFileNames = &(argv[optind]);
    return argc - optind;
}

long
main(long argc, char *argv[])
{
    char*	     str;
    pfTexture*       tex;
    Window	     xwin;
    unsigned int     *image;
    int		     comp, xs, ys, zs;
    int	    	     i, k, xdist;
    int	    	     arg;
    int		     start = 0;
    int		     frame = 1;

     /* Initialize Performer */
    pfNotifyLevel(PFNFY_INFO);
    pfInit();
    pfInitState(NULL);
    
    /* cmdline options */
    arg = docmdline(argc, argv);
    
    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures");
    base = pfNewTex(NULL);
    if (arg)
    {
	str = texFileNames[0];
	fprintf(stderr, "Base tex %s is (frame -1)\n", str);
	pfTexName(base, str);
	pfLoadTexFile(base, str);
	if (arg == 1)
	{
	    int tsize;
	    pfGetTexImage(base, &image, &comp, &xs, &ys, &zs);
	    tsize = PF_MAX2(xs, ys);
	    TexLevels = 0;
	    while ( tsize > 1 ) { TexLevels++; tsize = tsize >> 1; };
	}
	else
	{
	    TexLevels = arg;
	}
	WinXSize =  (1 << TexLevels) + 10;
	WinYSize = WinXSize + (WinXSize * 0.5) + 10;
    }
    
    /* Initialize GL */
    Win = pfNewWin(NULL);
    pfWinOriginSize(Win, 0, 0, WinXSize, WinYSize);
    pfWinName(Win, "texlist");
    pfWinType(Win, PFWIN_TYPE_X);
    pfOpenWin(Win);
#ifndef WIN32
    Dsp = pfGetCurWSConnection();
    xwin = pfGetWinWSWindow(Win);
    XSelectInput(Dsp, xwin,  KeyPressMask);
    XMapWindow(Dsp, xwin);
    XSync(Dsp,FALSE);
#endif

    /* load in cmdline image files and make a mipmaps */
    texList = pfNewList(sizeof(pfTexture*), 16, NULL);
    imageList = pfNewList(sizeof(int*), 16, NULL);
    pfTexLoadMode(base, PFTEX_LOAD_BASE, TexLoadMode);
    if (TexLoadMode == PFTEX_BASE_AUTO_SUBLOAD)
	pfTexFormat(base, PFTEX_SUBLOAD_FORMAT, 1);
    pfTexFormat(base, PFTEX_GEN_MIPMAP_FORMAT, 0);
    
    if (arg)
    {
	for (i=1; i < arg; i++)
	{
	    str = texFileNames[i];
	    tex = pfNewTex(NULL);
	    pfTexName(tex, str);
	    fprintf(stderr, "Loading tex %s as level %d\n", str, i);
	    pfLoadTexFile(tex, str);
	    pfAdd(texList, tex);
	    pfGetTexImage(tex, &image, &comp, &xs, &ys, &zs);
	    pfAdd(imageList, image);
	    pfTexLevel(base, i, tex);
	}
	if (i == 1) /* display the levels of one texture */
	{
	    
	    fprintf(stderr, "Texture %s has %d levels\n", str, TexLevels);
	    pfTexFormat(base, PFTEX_GEN_MIPMAP_FORMAT, 1);
	    pfFree(texList);
	    texList = NULL;
	    RunMode = 0;
	}
	else
	    TexLevels = pfGetNum(texList) - 1;
    } 
    else 
    { 
	/* make RGBA MIPmaps of different colors */
	char tstr[PF_MAXSTRING];
	int num = 1 << TexLevels;
	int mapSize = num * num;
	image = pfMalloc(sizeof(int)*mapSize, NULL);
	for (k=0; k < mapSize; k++)
	    image[k] = clrs[0];
	sprintf(tstr, "base", i);
	pfTexName(base, tstr);
	pfTexImage(base, image, 4, num, num, 0);
	pfAdd(texList, base);
	pfAdd(imageList, image);
	for (i=1; i <= TexLevels; i++)
	{
	    num = 1 << (TexLevels - i);
	    image = pfMalloc(sizeof(int)*mapSize, NULL);
	    for (k=0; k < mapSize; k++)
		image[k] = clrs[i];
	    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"Level %d has color 0x%x", i, clrs[i]);
	    tex = pfNewTex(NULL);
	    sprintf(tstr, "level=%d", i);
	    pfTexName(tex, tstr);
	    pfTexImage(tex, image, 4, num, num, 0);
	    pfAdd(texList, tex);
	    pfAdd(imageList, image);
	    pfTexLevel(base, i, tex);
	}
        TexLevels = pfGetNum(texList) - 1;
    }
    fprintf(stderr, "Texture has %d levels\n", TexLevels);
    
    pfCullFace(PFCF_OFF);

    pfEnable(PFEN_TEXTURE);
    pfApplyTex(base);
    pfApplyTEnv(pfNewTEnv(NULL));

    geom = pfNewGSet(NULL);
    pfGSetAttr(geom, PFGS_COORD3, PFGS_PER_VERTEX, vcoords, NULL);
    pfGSetAttr(geom, PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
    pfGSetAttr(geom, PFGS_COLOR4, PFGS_OVERALL, scolors, NULL);

    pfGSetPrimType(geom, PFGS_QUADS);
    pfGSetNumPrims(geom, 1);

    /* make a square for each MIPmap level in the dlist */
    dlist[0] = pfNewDList(PFDL_FLAT, 0, NULL);
    pfOpenDList(dlist[0]);
    xdist = 0;
    pfPushMatrix();
    for (i = 0; i <= TexLevels; i++)
    {
	int s = 1 << (TexLevels - i);
	pfPushMatrix();
	if (xdist > 0)
	    pfTranslate(xdist, 0.0f, 0.0f);
	pfNotify(PFNFY_NOTICE,PFNFY_PRINT,
		"Trans Y: %d Trans X: %d", s, xdist);
	pfScale(s *.9f, s *.9f, 1.0f);
	pfDrawGSet(geom);
	pfPopMatrix();
	if (i == 0)
	    pfTranslate(0.0f, s, 0.0f);
        else if (i > 0)
	    xdist += s;
    }
    pfPopMatrix();
    pfCloseDList();


    while(1)
    {
	if (texList && RunMode && ((frame % 10) == 0))
	{
	    frame = 1;
	    start += 1;
	    start %= TexLevels;
	    for (i = 0; i < TexLevels; i++)
	    {
		unsigned int *image;
		pfTexture *tex;
		int ii = start + i;
		ii %= TexLevels;
		image = pfGet(imageList, ii);
		tex = pfGet(texList, i);
		pfTexLoadImage(tex, image);
	    }
	    pfLoadTex(base);
	}
	frame++;
	draw_scene();
	do_events();
    }
    return 0;
}

static void 
do_events(void)
{
#ifndef WIN32
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
	    case XK_r:
		RunMode ^= 1;
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

static void draw_scene(void) 
{
    static pfVec4 clr = {0.0f, 0.0f, 0.1f, 1.0f};

    pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
    
    pfDrawDList(dlist[0]);	/* draw all display lists */
    pfSwapWinBuffers(Win);  
   
}
