//
// Copyright 1995, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// This source code ("Source Code") was originally derived from a
// code base owned by Silicon Graphics, Inc. ("SGI")
// 
// LICENSE: SGI grants the user ("Licensee") permission to reproduce,
// distribute, and create derivative works from this Source Code,
// provided that: (1) the user reproduces this entire notice within
// both source and binary format redistributions and any accompanying
// materials such as documentation in printed or electronic format;
// (2) the Source Code is not to be used, or ported or modified for
// use, except in conjunction with OpenGL Performer; and (3) the
// names of Silicon Graphics, Inc.  and SGI may not be used in any
// advertising or publicity relating to the Source Code without the
// prior written permission of SGI.  No further license or permission
// may be inferred or deemed or construed to exist with regard to the
// Source Code or the code base of which it forms a part. All rights
// not expressly granted are reserved.
// 
// This Source Code is provided to Licensee AS IS, without any
// warranty of any kind, either express, implied, or statutory,
// including, but not limited to, any warranty that the Source Code
// will conform to specifications, any implied warranties of
// merchantability, fitness for a particular purpose, and freedom
// from infringement, and any warranty that the documentation will
// conform to the program, or any warranty that the Source Code will
// be error free.
// 
// IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
// LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
// ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
// SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
// OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
// PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
// OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
// USE, THE SOURCE CODE.
// 
// Contact information:  Silicon Graphics, Inc., 
// 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
// or:  http://www.sgi.com
//
// 
// hlcube.C - highlighted gset example
//
// $Revision: 1.12 $
// $Date: 2001/03/22 00:22:38 $
//

//
// This example demonstrates how to create gsets and Performer display lists
// and demonstrates Performer highlighting functionality.
//
// Run-time controls:
// ------------------
//
// ESC-key - exits program.
//
// Keys 1-8 select different highlighting styles.
// c-key - cycle through some different highlighting foreground colors
// e-key - toggle highlighting enable
// g-key - toggle gset-level highlighting on/off
// o-key - toggle highlighting override
// m-key - cycle through some highlighting styles
// r-key - reverse colors for highlighting outlines
// t-key - enable transparency - arrow keys increase and 
//		decrease transparency.
// 
//


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <X11/X.h>
#include <X11/keysym.h>

#include <Performer/pr/pfWindow.h>
#include <Performer/pr/pfLight.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pr/pfHighlight.h>
#include <Performer/pr/pfGeoSet.h>


static void doEvents(pfWindow *win);
static void drawScene(void);

static float head, pitch, roll, sign;
static int haveTexture = 0;
static char texFileName[PF_MAXSTRING];
static pfDispList* dlist[4];
static pfHighlight *Highlight;

static float e, f, g;

#define NUM_HL_COLORS (sizeof(*hlcolors)/sizeof(pfVec4))
static int hlcolor = 0;

Display *Dsp;

static char OptionStr[] = "t:?";

static void docmdline(int argc, char **argv)
{
    extern char *optarg;
    int opt;
    
    while ((opt = getopt(argc,argv,OptionStr)) != -1) {
	switch(opt) {
	    // custom options
	    
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
	} // switch
    } // while
}

static pfGeoSet	*geom;

void
main(int argc, char *argv[])
{
    int             i, j, k;    
    pfTexture*       tex[4];
    
    docmdline(argc, argv);
    
    // initialize Performer
    pfInit();
    pfInitState(NULL);
    
    // Initialize GL
    pfWindow *win = new pfWindow;
    win->setOriginSize(0, 0, 300, 300);
    win->setName("Performer cubes");
    win->setWinType(PFWIN_TYPE_X);
    win->open();
    
    pfFrustum *frust = new pfFrustum;
    frust->makeSimple(45.0f);
    frust->setNearFar(1.0f, 100.0f);
    frust->apply();
    delete frust;
    
    // set up X input event handling
    Dsp = pfGetCurWSConnection();
    Window xwin = win->getWSWindow();
    XSelectInput(Dsp, xwin,  KeyPressMask );
    XMapWindow(Dsp, xwin);
    XSync(Dsp,FALSE);
    
    pfVec4 *scolors = new pfVec4[4];
    scolors[0].set(1.0f, 0.0f, 0.0f, 1.0f);
    scolors[1].set(0.0f, 1.0f, 0.0f, 1.0f);
    scolors[2].set(0.0f, 0.0f, 1.0f, 1.0f);
    scolors[3].set(1.0f, 1.0f, 1.0f, 1.0f);
    
    pfVec3 *snorms = new pfVec3[6];
    snorms[0].set( 0.0f,  0.0f,  1.0f);
    snorms[1].set( 0.0f,  0.0f, -1.0f);
    snorms[2].set( 0.0f,  1.0f,  0.0f);
    snorms[3].set( 0.0f, -1.0f,  0.0f);
    snorms[4].set( 1.0f,  0.0f,  0.0f);
    snorms[5].set(-1.0f,  0.0f,  0.0f);
    
    pfVec2 *tcoords = new pfVec2[4];
    tcoords[0].set(0.0f, 0.0f);
    tcoords[1].set(1.0f, 0.0f);
    tcoords[2].set(1.0f, 1.0f);
    tcoords[3].set(0.0f, 1.0f);

    
    pfVec3 *scoords = new pfVec3[8];
    scoords[0].set(-1.0f, -1.0f,  1.0f);
    scoords[1].set( 1.0f, -1.0f,  1.0f);
    scoords[2].set( 1.0f,  1.0f,  1.0f);
    scoords[3].set(-1.0f,  1.0f,  1.0f);
    scoords[4].set(-1.0f, -1.0f, -1.0f);
    scoords[5].set( 1.0f, -1.0f, -1.0f);
    scoords[6].set( 1.0f,  1.0f, -1.0f);
    scoords[7].set(-1.0f,  1.0f, -1.0f);
    
    ushort *nindex = new ushort[24];
    
    nindex[0]  = nindex[1]  = nindex[2]  = nindex[3]  = 0;
    nindex[4]  = nindex[5]  = nindex[6]  = nindex[7]  = 5;
    nindex[8]  = nindex[9]  = nindex[10] = nindex[11] = 1;
    nindex[12] = nindex[13] = nindex[14] = nindex[15] = 4;
    nindex[16] = nindex[17] = nindex[18] = nindex[19] = 2;
    nindex[20] = nindex[21] = nindex[22] = nindex[23] = 3;
    
    ushort *vindex = new ushort[24];
    
    vindex[0]  = 0; vindex[1]  = 1; vindex[2]  = 2; vindex[3]  = 3; /* front */
    vindex[4]  = 0; vindex[5]  = 3; vindex[6]  = 7; vindex[7]  = 4; /* left */
    vindex[8]  = 4; vindex[9]  = 7; vindex[10] = 6; vindex[11] = 5; /* back */
    vindex[12] = 1; vindex[13] = 5; vindex[14] = 6; vindex[15] = 2; /* right */
    vindex[16] = 3; vindex[17] = 2; vindex[18] = 6; vindex[19] = 7; /* top */
    vindex[20] = 0; vindex[21] = 4; vindex[22] = 5; vindex[23] = 1; /* bottom */
    
    ushort *cindex = new ushort[24];
    
    cindex[0]  = 0; cindex[1]  = 1; cindex[2]  = 2; cindex[3]  = 3;
    cindex[4]  = 0; cindex[5]  = 1; cindex[6]  = 2; cindex[7]  = 3;
    cindex[8]  = 0; cindex[9]  = 1; cindex[10] = 2; cindex[11] = 3;
    cindex[12] = 0; cindex[13] = 1; cindex[14] = 2; cindex[15] = 3;
    cindex[16] = 0; cindex[17] = 1; cindex[18] = 2; cindex[19] = 3;
    cindex[20] = 0; cindex[21] = 1; cindex[22] = 2; cindex[23] = 3;
    
    geom = new pfGeoSet;
    
    geom->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, scolors, cindex);
    geom->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, scoords, vindex);
    if (haveTexture)
    {
	geom->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, cindex);
    }
    else
    {
	geom->setAttr(PFGS_TEXCOORD2, PFGS_OFF, tcoords, cindex);
    }
    geom->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX, snorms, nindex);
    
    geom->setPrimType(PFGS_QUADS);
    geom->setNumPrims(6);
    
    // set up global state
    if (haveTexture)
    {
	tex[0] = new pfTexture;
	tex[0]->loadFile(texFileName);
	pfTexEnv *tev = new pfTexEnv;
	tev->apply();
	pfEnable(PFEN_TEXTURE);
    }
    // light
    pfLight *light = new pfLight;
    // light model
    pfLightModel *lm = new pfLightModel;
    lm->setAmbient(0.0f, 0.0f, 0.0f);
    lm->apply();
    // material
    pfMaterial *mat = new pfMaterial;
    mat->setColor(PFMTL_AMBIENT, 0.0f, 0.0f, 0.0f);
    mat->setColor(PFMTL_SPECULAR, 0.5f, 0.5f, 0.5f);
    mat->setShininess(10.0f);
    mat->setColorMode(PFMTL_BOTH, PFMTL_CMODE_AD);
    mat->apply();
    // enable lighting
    pfEnable(PFEN_LIGHTING);
    light->on();
    
    dlist[0] = new pfDispList(PFDL_FLAT, 0);
    dlist[1] = new pfDispList(PFDL_FLAT, 0);
    dlist[2] = new pfDispList(PFDL_FLAT, 0);
    dlist[3] = new pfDispList(PFDL_FLAT, 0);
    
    // make three display lists with 9 cubes in each
    for (i = 0; i < 3; i++)
    {
	dlist[i+1]->open();
	if (haveTexture)
	    tex[0]->apply();
	if (i == 2)
	    pfAlphaFunc(0, PFAF_NOTEQUAL);
	for (j = 0; j < 3; j++)
	{
	    for (k = 0; k < 3; k++)
	    {
		pfPushMatrix();
		geom->draw();
		pfPopMatrix();
		pfTranslate(0.0f, 0.0f, 3.0f);
	    }
	    pfTranslate(0.0f, 3.0f, -9.0f);
	}
	pfTranslate(3.0f, -9.0f, 0.0f);
	if (i == 2)
	    pfAlphaFunc(0, PFAF_ALWAYS);
	dlist[i+1]->close();
    }
    
    // make one display list calling the other three
    dlist[0]->open();
    pfTranslate(-3.0f, -3.0f, -3.0f);
    dlist[1]->draw();
    dlist[2]->draw();
    dlist[3]->draw();
    dlist[0]->close();
    
    
    head = pitch = roll = 0.0f;
    sign = 1;
    pfTranslate(0.0f, 0.0f, -25.0f);
    
    // un-comment this to see the pfGeoSet
    //pfPrint(geom, 0, PFPRINT_VB_INFO, 0);
    
    doEvents(win);
}

static void 
doEvents(pfWindow *win)
{
    int hlmode = 0;
    float alpha = 1.0f;
    int ov = 0;
    
    static pfVec4 *hlcolors = new pfVec4[4];
    hlcolors[0].set(1.0f, 0.0f, 0.0f, 1.0f); 
    hlcolors[1].set(0.0f, 1.0f, 1.0f, 1.0f); 
    hlcolors[2].set(1.0f, 0.0f, 1.0f, 1.0f); 
    hlcolors[3].set(1.0f, 1.0f, 0.0f, 1.0f);
    
    Highlight = new pfHighlight;
    pfEnable(PFEN_HIGHLIGHTING);
    // default color is white - this sets it to something interesting
    Highlight->setColor( PFHL_FGCOLOR, 
			hlcolors[hlcolor][0], 
			hlcolors[hlcolor][1],
			hlcolors[hlcolor][2]);
    Highlight->apply();
    
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
			Highlight->setColor(PFHL_FGCOLOR, 
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
		    case XK_g:
			if (geom->getHlight() == NULL)
			    geom->setHlight(Highlight);
			else
			    geom->setHlight(NULL);
			break;
		    case XK_G:
			geom->setHlight(PFGS_OFF);
			break;
		    case XK_h:
		    case XK_m:
			switch (Highlight->getMode())
			{
			default:
			case PFHL_OFF:
			    hlmode = PFHL_LINES;
			    break;
			case PFHL_LINES:
			    hlmode = PFHL_LINESPAT;
			    break;
			case PFHL_LINESPAT:
			    hlmode = PFHL_FILL;
			    break;
			case PFHL_FILL:
			    hlmode = PFHL_FILLPAT;
			    break;
			case PFHL_FILLPAT:
			    hlmode |= PFHL_LINES;
			    break;
			case PFHL_FILLPAT | PFHL_LINES:
			    hlmode |= PFHL_LINESPAT;
			    break;
			case PFHL_FILLPAT | PFHL_LINESPAT:
			    hlmode |= PFHL_LINESPAT2;
			    break;
			case PFHL_FILLPAT | PFHL_LINESPAT2:
			    hlmode |= PFHL_FILLPAT2;
			    break;
			case PFHL_FILLPAT2 | PFHL_LINESPAT2:
			    hlmode = PFHL_FILL;
			    break;
			}
			Highlight->setMode(hlmode);
			break;
		    case XK_r:
		    case XK_R:
			hlmode = Highlight->getMode();
			if (hlmode & PFHL_LINES_R)
			    Highlight->setMode(hlmode & ~PFHL_LINES_R);
			else
			    Highlight->setMode(hlmode | PFHL_LINES_R);
			break;
		    case XK_t: // enable transparency
			{
			    static int t;
			    t = pfGetTransparency();
			    if (t) 
				pfTransparency(PFTR_OFF);
			    else
				pfTransparency(PFTR_ON);
			}
			break;
		    case XK_1:
			hlmode = PFHL_LINES;
			Highlight->setMode(hlmode);
			break;
		    case XK_2:
			hlmode = PFHL_LINES | PFHL_LINESPAT;
			Highlight->setMode(hlmode);
			break;
		    case XK_3:
			hlmode = PFHL_FILL;
			Highlight->setMode(hlmode);
			break;
		    case XK_4:
			hlmode = PFHL_FILL | PFHL_FILLPAT;
			Highlight->setMode(hlmode);
			break;
		    case XK_5:
			hlmode = PFHL_FILL | PFHL_FILLPAT | PFHL_LINES;
			Highlight->setMode(hlmode);
			break;
		    case XK_6:
			hlmode = PFHL_FILL | PFHL_FILLPAT |
			    PFHL_LINES | PFHL_LINESPAT;
			Highlight->setMode(hlmode);
			break;
		    case XK_7:
			hlmode = PFHL_FILL | PFHL_FILLPAT2 |
			    PFHL_LINES | PFHL_LINESPAT;
			Highlight->setMode(hlmode);
			break;
		    case XK_8:
			hlmode = PFHL_FILL | PFHL_FILLPAT2 |
			    PFHL_LINES | PFHL_LINESPAT2;
			Highlight->setMode(hlmode);
			break;
		    case XK_0:
			hlmode = PFHL_OFF;
			Highlight->setMode(hlmode);
			break;
		    case XK_Up: // up arrow key
			if (alpha < 1.0f)
			{
			    alpha += 0.1f;
			    Highlight->setAlpha(alpha);
			}
			break;
		    case XK_Down: // down arrow key
			if (alpha > 0.0f)
			{
			    alpha -= 0.1f;
			    Highlight->setAlpha(alpha);
			}
			break;
		    default:
			break;
		    }
		}// keypress switch
	    }// event switch
	} // while event
	drawScene();
	win->swapBuffers();
    } // while 1
}

static void drawScene(void) 
{
    static pfVec4 clr(0.1f, 0.0f, 0.4f, 1.0f);
    pfClear(PFCL_COLOR | PFCL_DEPTH, &clr);
    
    pfPushMatrix();
    
    pfRotate(PF_Y, head);
    pfRotate(PF_X, pitch);
    pfRotate(PF_Z, roll);
    pfRotate(PF_X, e);
    pfRotate(PF_Y, f);
    pfRotate(PF_Z, g);
    
    e += 0.2f;
    f += 0.4f;
    g += 0.8f;
    dlist[0]->draw();	// draw all display lists
    pfPopMatrix();
}
