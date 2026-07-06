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
 * hlcube.c - highlighted gset example
 *
 * $Revision: 1.25 $
 * $Date: 2002/11/13 00:23:21 $
 */

/*
 * This example demonstrates how to create gsets and Performer display lists
 * and demonstrates Performer highlighting functionality.
 *
 * Run-time controls:
 * ------------------
 *
 * ESC-key - exits program.
 *
 * Keys 1-8 select different highlighting styles.
 * c-key - cycle through some different highlighting foreground colors
 * e-key - toggle highlighting enable
 * g-key - toggle gset-level highlighting on/off
 * o-key - toggle highlighting override
 * m-key - cycle through some highlighting styles
 * r-key - reverse colors for highlighting outlines
 * t-key - enable transparency - arrow keys increase and 
 *		decrease transparency.
 * 
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
#include <Performer/pr.h>
#ifndef WIN32
#include <X11/X.h>
#include <X11/keysym.h>
#endif

#define NUM_CUBES 9

static void init_window(void);
static void do_events(pfWindow *win);
static void draw_scene(void);

static int haveTexture = 0;
static char texFileName[PF_MAXSTRING];
static pfTexture* tex=NULL;
static pfTexEnv* tev=NULL;
static pfTexGen *tgen=NULL;
static pfGeoSet	*geom;
static pfHighlight *hlight[10];

pfVec4          scolors[] ={{1., 0., 0., 1.},
			    {0., 1., 0., 1.},
			    {0., 0., 1., 1.},
			    {1., 1., 1., 1.}};

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

pfVec3          scoords[] ={{-1.0, -1.0,1.0},
			    {1.0, -1.0,1.0},
			    {1.0,1.0,1.0},
			    {-1.0,1.0,1.0},
			    {-1.0, -1.0, -1.0},
			    {1.0, -1.0, -1.0},
			    {1.0,1.0, -1.0},
			    {-1.0,1.0, -1.0}};

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


static pfVec4 hlcolors[] = {
    {1.0, 0.0, 0.0, 1.0}, 
    {0.0, 1.0, 1.0, 1.0}, 
    {1.0, 0.0, 1.0, 1.0}, 
    {1.0, 1.0, 0.0, 1.0}, 
    {1.0, 1.0, 1.0, 1.0}
};
#define NUM_HL_COLORS (sizeof(hlcolors)/sizeof(pfVec4))
static int hlcolor = 0;

#ifndef WIN32
Display *Dsp;
#endif

static char OptionStr[] = "t:?";

static void docmdline(int argc, char **argv)
{
     extern char *optarg;
     int opt;
        
     while ((opt = getopt(argc,argv,OptionStr)) != -1) {
	 switch(opt) {
	 /* custom options */
	
	 case '?':
	 case 't':
		haveTexture = 1;
		strcpy(texFileName,optarg);
		break;
	 default:
		printf("bad option... %c\n", opt);
		printf("cube \n");
		exit(0);
		break;
	} /* switch */
    } /* while */
}

void
main(int argc, char *argv[])
{
    int		     i, j;    
    pfLight*         light;
    pfLightModel*    lm;
    pfMaterial*      mat;
    pfWindow         *win;
    pfFrustum	     *frust;
    Window	     xwin;

    docmdline(argc, argv);
    
    /* initialize Performer */
    pfInit();
    pfInitState(NULL);
    
    pfFilePath("/usr/share/Performer/data:/usr/demos/data/textures");
    
    /* Initialize GL */
    win = pfNewWin(NULL);
    pfWinOriginSize(win, 0, 0, 500, 500);
    pfWinName(win, "Performer cubes");
    pfWinType(win, PFWIN_TYPE_X);
    pfOpenWin(win);

    frust = pfNewFrust(NULL);
    pfMakeSimpleFrust(frust, 45.0f);
    pfFrustNearFar(frust, 1.0f, 100.0f);
    pfApplyFrust(frust);
    pfDelete(frust);
    
    /* set up X input event handling */
#ifndef WIN32
    Dsp = pfGetCurWSConnection();
    xwin = pfGetWinWSWindow(win);
    XSelectInput(Dsp, xwin,  KeyPressMask );
    XMapWindow(Dsp, xwin);
    XSync(Dsp,FALSE);
#endif

    geom = pfNewGSet(NULL);
    pfGSetAttr(geom, PFGS_COLOR4, PFGS_PER_VERTEX, scolors, cindex);
    pfGSetAttr(geom, PFGS_COORD3, PFGS_PER_VERTEX, scoords, vindex);
    if (haveTexture)
    {
	pfGSetAttr(geom, PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, cindex);
    } else {
	pfGSetAttr(geom, PFGS_TEXCOORD2, PFGS_OFF, tcoords, cindex);
    }
    pfGSetAttr(geom, PFGS_NORMAL3, PFGS_PER_VERTEX, snorms, nindex);

    pfGSetPrimType(geom, PFGS_QUADS);
    pfGSetNumPrims(geom, 6);

    /* set up global state */
    if (haveTexture)
    {
	tex = pfNewTex(NULL);
	if (pfLoadTexFile(tex, texFileName))
	{
	    tev = pfNewTEnv(NULL);
	    pfApplyTEnv(tev);
	    pfEnable(PFEN_TEXTURE);
	}
	else
	{
	    pfDelete(tex);
	}
    }
    /* light */
    light = pfNewLight(NULL);
    /* light model */
    lm = pfNewLModel(NULL);
    pfLModelAmbient(lm, 0.0, 0.0, 0.0);
    pfApplyLModel(lm);
    /* material */
    mat = pfNewMtl(NULL);
    pfMtlColor(mat, PFMTL_AMBIENT, 0.0, 0.0, 0.0);
    pfMtlColor(mat, PFMTL_SPECULAR, 0.5, 0.5, 0.5);
    pfMtlShininess(mat, 10.0);
    pfMtlColorMode(mat, PFMTL_BOTH, PFMTL_CMODE_AD);
    pfApplyMtl(mat);
    /* enable lighting */
    pfEnable(PFEN_LIGHTING);
    pfLightOn(light);
    
    pfEnable(PFEN_HIGHLIGHTING);
    
    tgen = pfNewTGen(NULL);
    pfTGenMode(tgen, PF_S, PFTG_OBJECT_LINEAR);
    pfTGenPlane(tgen, PF_S, 1.0f, 0.0f, 0.0f, 0.5f); 
    pfTGenMode(tgen, PF_T, PFTG_OBJECT_LINEAR);
    pfTGenPlane(tgen, PF_T, 0.0f, 0.0f, 1.0f, 0.5f);
    /* create 9 different hlight structures */
    for (i=0; i < NUM_CUBES; i++)
    {
	hlight[i] = pfNewHlight(NULL);
	pfHlightColor(hlight[i], PFHL_BGCOLOR, 0.1, 0.2, 0.5);
	switch(i)
	{
#if 1
	case 0:
	    pfHlightMode(hlight[i], PFHL_FILL);
	    break;
	case 1:
	    pfHlightMode(hlight[i], PFHL_BBOX_LINES | PFHL_POINTS | PFHL_NORMALS);
	    pfHlightPntSize(hlight[i], 4);
	    break;
	case 2:
	    pfHlightMode(hlight[i], PFHL_LINES);
	    pfHlightLineWidth(hlight[i], 4);
	    break;
	case 3:
	    pfHlightMode(hlight[i], PFHL_FILLPAT | PFHL_LINESPAT);
	    break;
	case 4:
	    pfHlightMode(hlight[i], PFHL_FILLPAT2 | PFHL_LINESPAT2);
	    break;
	case 5:
	    pfHlightMode(hlight[i], PFHL_FILLPAT2 | PFHL_FILL_R | PFHL_LINESPAT2 | PFHL_LINES_R);
	    break;
	case 6:
	    pfHlightMode(hlight[i], PFHL_LINES | PFHL_SKIP_BASE);
	    break;
	case 7:
	    pfHlightMode(hlight[i], PFHL_FILLTEX);
	    pfHlightTGen(hlight[i], tgen);
	    break;
	case 8:
#endif
	default:
	    pfHlightMode(hlight[i], PFHL_FILLTEX);
	    pfHlightTGen(hlight[i], tgen);
	    break;
	}
    }
    /* un-comment this to see the pfGeoSet */
    /*pfPrint(geom, 0, PFPRINT_VB_INFO, 0);*/
    
    do_events(win);
}

static void 
do_events(pfWindow *win)
{
#ifndef WIN32
    int do_draw = 0;
    int hlmode = 0;
    float alpha = 1.0;
    int ov = 0;
    int dev;
    int i;
    short val;
    
           
    while (1) {
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
		case XK_c:
		    hlcolor = (hlcolor + 1) % NUM_HL_COLORS;
		    for (i=0; i < NUM_CUBES; i++)
			pfHlightColor(hlight[i], PFHL_FGCOLOR, 
			    hlcolors[hlcolor][0], 
			    hlcolors[hlcolor][1],
			    hlcolors[hlcolor][2]);
		    break;
		case XK_e:
		    if (pfGetEnable(PFEN_HIGHLIGHTING))
			pfDisable(PFEN_HIGHLIGHTING);
		    else
			pfEnable(PFEN_HIGHLIGHTING);
		    fprintf(stderr, "Highlight enable = %d\n", 
				pfGetEnable(PFEN_HIGHLIGHTING));
		    break;
		case XK_o:
		    ov ^= 1;
		    pfOverride(PFSTATE_ENHIGHLIGHTING, ov);
		    fprintf(stderr, "Highlight enable override = %d %d\n", 
				pfGetOverride() & PFSTATE_ENHIGHLIGHTING,  ov);
		    break;
		
		case XK_Up: /* up arrow key */
		    if (alpha < 1.0)
		    {
			alpha += 0.1;
			for (i=0; i < NUM_CUBES; i++)
			    pfHlightAlpha(hlight[i], alpha);
		    }
		    break;
		case XK_Down: /* down arrow key */
		    if (alpha > 0.0)
		    {
			alpha -= 0.1;
			for (i=0; i < NUM_CUBES; i++)
			    pfHlightAlpha(hlight[i], alpha);
		    }
		    break;
		default:
		    break;
		}
	        }/* keypress switch */
	    }/* event switch */
	} /* while event */
	draw_scene();
	pfSwapWinBuffers(win);
    } /* while 1 */
#endif
}

static void draw_scene(void) 
{
    int i, j;
    static float e=0.0f, f=0.0f, g=0.0f;
    static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};
    pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
    
    pfPushMatrix();
    
    pfTranslate(-3.0, 3.0, -25.0);
    
    e += 0.2;
    f += 0.4;
    g += 0.8;
    
    /* 3x3 grid of cubes - one for each hlight stucture */
    for (i = 0; i < 3; i++)
    {
	if (haveTexture && tex)
	    pfApplyTex(tex);
	pfPushMatrix();
	for (j = 0; j < 3; j++)
	{
	    if (i == 2 && j == 2)
		pfAlphaFunc(0, PFAF_NOTEQUAL);
	    pfPushMatrix();
	    pfRotate(PF_X, e);
	    pfRotate(PF_Y, f);
	    pfRotate(PF_Z, g);
	    pfApplyHlight(hlight[i*3 + j]);
	    pfDrawGSet(geom);
	    pfPopMatrix();
	    pfTranslate(3.0, 0.0, 0.0);
	    if (i == 2 && j == 2)
		pfAlphaFunc(0, PFAF_ALWAYS);
	}
	pfPopMatrix();
	pfTranslate(0.0, -3.0, 0.0);
	
    }
    
    pfPopMatrix();
}
