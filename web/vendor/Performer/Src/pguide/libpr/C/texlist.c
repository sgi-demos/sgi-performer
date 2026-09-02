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
 * texlist.c - textured-movie gset test
 *
 *  example run:
 *	texlist -R [string of texture names]
 *	ex:	texlist -R wood.rgb marble.rgb brick.rgba
 *
 * command line options:
 * -R - come up running the movie
 * -i - use the LIST_IDLE texture loading mode
 * -r - use the LIST_SUBLOAD texture loading mode - continuously
 *		downloads texture from host
 * -T use tiling to take offset portion of movie textures and
 *	map them to an offset destination
 * 
 *
 *
 * $Revision: 1.16 $
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

pfVec3          snorms[] ={{0., 0., 1.},
			   {0., 0., -1.},
			   {0., 1., 0.},
			   {0., -1., 0.},
			   {1., 0., 0.},
			   {-1., 0., 0.}};

pfVec2          tcoords[] ={{0., 0.},
			    {1., 0.},
			    {1., 1.},
			    {0., 1.}};			    

pfVec3          scoords[] ={{-.1, -.1,.1},
			    {.1, -.1,.1},
			    {.1,.1,.1},
			    {-.1,.1,.1},
			    {-.1, -.1, -.1},
			    {.1, -.1, -.1},
			    {.1,.1, -.1},
			    {-.1,.1, -.1}};

ushort             nindex[] ={0, 0, 0, 0,
			   5, 5, 5, 5,
			   1, 1, 1, 1,
			   4, 4, 4, 4,
			   2, 2, 2, 2,
			   3, 3, 3, 3};

ushort             vindex[] ={0, 1, 2, 3,	/* front */
			   0, 3, 7, 4,	/* left */
			   4, 7, 6, 5,	/* back */
			   1, 5, 6, 2,	/* right */
			   3, 2, 6, 7,	/* top */
			   0, 4, 5, 1};	/* bottom */


ushort             cindex[] ={0, 1, 2, 3,
			   0, 1, 2, 3,
			   0, 1, 2, 3,
			   0, 1, 2, 3,
			   0, 1, 2, 3,
			   0, 1, 2, 3};

static float head, pitch, roll, sign;
static pfDispList* dlist[4];
static pfGeoSet	*geom;
pfTexture *movie;
pfList *texList;
long listLength;
char **texFileNames;
static float e, f, g; /* for rots */
int FastDefine = 0;
int TexLoadMode = PFTEX_LIST_APPLY;
int RunMode = 1;
int TexFrame = -1;
int Tile = 0;

#ifndef WIN32
Display *Dsp;
#endif
pfWindow *Win;

static char OptionStr[] = "irFR?T";


void
Usage(int argc, char **argv)
{
    pfNotify(PFNFY_FATAL,PFNFY_USAGE,
		"%s [-irRfT] image0 image1 image2 ...", argv[0]);
	    exit(0);
}

static long
docmdline(int argc, char **argv)
{
     extern char *optarg;
     extern int  optind;
     int opt;
        
     while ((opt = getopt(argc,argv,OptionStr)) != -1) {
	 switch(opt) {
	 /* custom options */
	 case 'i':
	    TexLoadMode = PFTEX_LIST_AUTO_IDLE;
	    break;
	 case 'F':
	    FastDefine = 1;
	    break;
	 case 'r':
	    TexLoadMode = PFTEX_LIST_AUTO_SUBLOAD;
	    FastDefine = 1;
	    break;
	 case 'R':
	    RunMode ^= 1;
	    break;
	 case 'T':
	    Tile ^= 1;
	    break;
	 case '?':
	 default:
	    Usage(argc, argv);
	    break;
	} /* switch */
    } /* while */
    
    texFileNames = &(argv[optind]);
    return argc - optind;
}

long
main(long argc, char *argv[])
{
    long	    i, j, k;
    long	    arg;
    char*	    str;
    pfFrustum	    *frust;
    pfTexture*       tex;
    pfLight*         light;
    pfLightModel*    lm;
    pfMaterial*      mat;
    Window	     xwin;

    pfInit();
     /* Initialize Performer */
    pfNotifyLevel(PFNFY_INFO);
    pfInit();

    /* cmdline options */
    arg = docmdline(argc, argv);

    if (!arg)
	Usage(argc, argv);
    
    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures");

    pfInitState(NULL);
    
    /* Initialize GL */
    Win = pfNewWin(NULL);
    pfWinOriginSize(Win, 0, 0, 800, 800);
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

    frust = pfNewFrust(NULL);
    pfMakeSimpleFrust(frust, 60.0f);
    pfApplyFrust(frust);
    pfDelete(frust);
    pfAntialias(PFAA_ON);
    pfCullFace(PFCF_OFF);
    
    /* load in cmdline image files and make a movie */
    movie = pfNewTex(NULL);
    pfTexList(movie, pfNewList(sizeof(pfTexture*), 16, pfGetArena(movie)));
    texList = pfGetTexList(movie);
    pfTexLoadMode(movie, PFTEX_LOAD_LIST, TexLoadMode);
    
    if (arg)
    {
	unsigned int *image;
	int comp,xsize, ysize, zsize;
	int baseXSize, baseYSize;
	
	str = texFileNames[0];
	fprintf(stderr, "Default tex %s is (frame -1)\n", str);
	pfTexName(movie, str);
	pfLoadTexFile(movie, str);
	pfGetTexImage(movie, &image, &comp, &baseXSize, &baseYSize, &zsize);
	if (FastDefine || (TexLoadMode & PFTEX_LIST_AUTO_SUBLOAD))
	{
	    pfTexFormat(movie, PFTEX_FAST_DEFINE, 1);
	    pfTexFilter(movie, PFTEX_MINFILTER, PFTEX_BILINEAR);
	}
	for (i=1; i < arg; i++)
	{
	    str = texFileNames[i];
	    tex = pfNewTex(NULL);
	    pfTexName(tex, str);
	    fprintf(stderr, "Loading tex %s as frame %d\n", str, i-1);
	    pfLoadTexFile(tex, str);
	    pfGetTexImage(tex, &image, &comp, &xsize, &ysize, &zsize);
	    if (FastDefine || (TexLoadMode & PFTEX_LIST_AUTO_SUBLOAD))
	    {
		pfTexFormat(tex, PFTEX_FAST_DEFINE, 1);
		pfTexFilter(tex, PFTEX_MINFILTER, PFTEX_BILINEAR);
	    }
	    if (Tile)
	    {
		pfTexLoadOrigin(tex,PFTEX_ORIGIN_SOURCE,
			    xsize * 0.5, ysize * 0.5);
		pfTexLoadSize(tex, xsize * 0.5, ysize * 0.5);
		pfTexLoadOrigin(tex, PFTEX_ORIGIN_DEST, 
			    baseXSize * 0.25, baseYSize * 0.25);
	    }
	    pfAdd(texList, tex);
	}
    } else {
	char tstr[PF_MAXSTRING];
	pfTexName(movie, "movie:frame=-1");
	for (i=0; i < 10; i++)
	{
	    tex = pfNewTex(NULL);
	    sprintf(tstr, "frame=%d", i);
	    pfTexName(tex, tstr);
	    pfAdd(texList, tex);
	}
    }
    listLength = pfGetNum(texList);
    fprintf(stderr, "Movie has %d frames\n", listLength);
    
    pfEnable(PFEN_TEXTURE);
    pfApplyTex(movie);
    pfApplyTEnv(pfNewTEnv(NULL));
    
    /* light */
    light = pfNewLight(NULL);
    /* light model */
    lm = pfNewLModel(NULL);
    pfApplyLModel(lm);
    /* material */
    mat = pfNewMtl(NULL);
    pfMtlColor(mat, PFMTL_SPECULAR, 0.5, 0.5, 0.5);
    pfMtlShininess(mat, 10.0);
    pfApplyMtl(mat);
    /* enable */
    pfEnable(PFEN_LIGHTING);
    pfLightOn(light);

    dlist[0] = pfNewDList(PFDL_FLAT, 0, NULL);
    dlist[1] = pfNewDList(PFDL_FLAT, 0, NULL);
    dlist[2] = pfNewDList(PFDL_FLAT, 0, NULL);
    dlist[3] = pfNewDList(PFDL_FLAT, 0, NULL);


    geom = pfNewGSet(NULL);
    pfGSetAttr(geom, PFGS_COORD3, PFGS_PER_VERTEX, scoords, vindex);
    pfGSetAttr(geom, PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, cindex);
    pfGSetAttr(geom, PFGS_NORMAL3, PFGS_PER_VERTEX, snorms, nindex);
    pfGSetAttr(geom, PFGS_COLOR4, PFGS_OVERALL, scolors, cindex);

    pfGSetPrimType(geom, PFGS_QUADS);
    pfGSetNumPrims(geom, 6);

    /* make three display lists with 9 cubes in each */
    for (i = 0; i < 3; i++)
    {
	pfOpenDList(dlist[i + 1]);
	if (i == 2)
	    pfAlphaFunc(0, PFAF_NOTEQUAL);
	for (j = 0; j < 3; j++)
	{
	    for (k = 0; k < 3; k++)
	    {
		pfPushMatrix();
		pfScale(10.0, 10.0, 10.0);
		pfDrawGSet(geom);
		pfPopMatrix();
		pfTranslate(0.0, 0.0, 3.0);
	    }
	    pfTranslate(0.0, 3.0, -9.0);
	}
	pfTranslate(3.0, -9.0, 0.0);
	if (i == 2)
	    pfAlphaFunc(0, PFAF_ALWAYS);
	pfCloseDList();
    }
    
    /* make one display list calling the other three */
    pfOpenDList(dlist[0]);
    pfTranslate(-3.0, -3.0, -3.0);
    pfDrawDList(dlist[1]);
    pfDrawDList(dlist[2]);
    pfDrawDList(dlist[3]);
    pfCloseDList();


    head = pitch = roll = 0.0;
    sign = 1;
    pfTranslate(0.0, 0.0, -25.0);

    while(1)
    {
	if (RunMode)
	{
	    /*pfNotify(PFNFY_NOTICE,PFNFY_PRINT," MOVIE FRAME %d", TexFrame);*/
	    pfTexFrame(movie, TexFrame);
	    pfApplyTex(movie);
	    TexFrame++;
	    if (TexFrame == listLength)
		TexFrame = -1;
	}
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
	    case XK_Escape:
		exit(0);
		break;
	    case XK_space:
		TexFrame = -1;
		RunMode = 0;
		pfTexFrame(movie, TexFrame);
		TexFrame = pfGetTexFrame(movie);
		fprintf(stderr, "Tex frame %d: \"%s\"\n",
		     TexFrame, pfGetTexName(movie));
		pfApplyTex(movie);
		break;
	    case XK_r:
		TexFrame = 0;
		RunMode = 1;
		break;
	    case XK_1: case XK_2: case XK_3: case XK_4: case XK_5:
	    case XK_6: case XK_7: case XK_8: case XK_0:
	    {
		TexFrame = ks - ((int) XK_0);
		if (TexFrame < listLength)
		{
		    pfTexFrame(movie, TexFrame);
		    TexFrame = pfGetTexFrame(movie);
		    fprintf(stderr, "Tex frame %d: \"%s\"\n",
			 TexFrame, pfGetTexName(pfGet(texList, TexFrame)));
		    pfApplyTex(movie);
		} else {
		    fprintf(stderr, "frame %d too high. max is %d.\n", 
			    TexFrame, listLength-1);
		}
		break;
	    }
	    case XK_Up:
		TexFrame++;
		if (TexFrame < listLength)
		{
		    pfTexFrame(movie, TexFrame);
		    TexFrame = pfGetTexFrame(movie);
		    fprintf(stderr, "Tex frame %d: \"%s\"\n",
			    TexFrame, pfGetTexName(pfGet(texList, TexFrame)));
		    pfApplyTex(movie);
		} else {
		    fprintf(stderr, "frame %d too high. max is %d.\n", 
			    TexFrame, listLength-1);
		    TexFrame = listLength-1;
		}
		break;   
	    case XK_Down:
		if (TexFrame > -1)
		{
		    TexFrame--;
		    if (TexFrame < -1)
			TexFrame = -1;
		    pfTexFrame(movie, TexFrame);
		    TexFrame = pfGetTexFrame(movie);
		    fprintf(stderr, "Tex frame %d: \"%s\"\n",
			 TexFrame, pfGetTexName(pfGet(texList, TexFrame)));
		    pfApplyTex(movie);
		}
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
    static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};
    pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
    
    pfPushMatrix();
    pfScale(3.1, 3.1, 3.1);
#if 0
    pfRotate(PF_Y, head);
    pfRotate(PF_X, pitch);
    pfRotate(PF_Z, roll);
    pfRotate(PF_X, e);
    pfRotate(PF_Y, f);
    pfRotate(PF_Z, g);
    e += 0.2;
    f += 0.4;
    g += 0.8;
#endif
    pfDrawDList(dlist[0]);	/* draw all display lists */
    pfPopMatrix();
    pfSwapWinBuffers(Win);  
   
}
