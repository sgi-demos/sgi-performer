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
 * stringBin.C: Performer program to demonstrate how pfStrings and Bins
 *        interacts. Based on text3d.c.
 *        Original example by J.R. Gloudemans
 *
 * $Revision: 1.2 $ $Date: 2000/10/06 21:26:26 $
 *
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <X11/keysym.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfLight.h>
#include <Performer/pr/pfFont.h>
#include <Performer/pr/pfString.h>
#include <Performer/pf/pfText.h>

#define EXTRUDED	0
#define FILLED		1
#define OUTLINED	2
#define TEXTURED	3

#define pfNew(type,num) (type *)(pfMalloc(sizeof(type) * num, pfGetSharedArena()))

char		*CharString;
pfString	**GSetStrings;
int		UseTex;
int 		cycle;
/*------------------------------------------------------------------*/

static void
updateStrings(void)
{
    if (GSetStrings[EXTRUDED])
	GSetStrings[EXTRUDED]->setString(CharString);
    if (GSetStrings[FILLED])
	GSetStrings[FILLED]->setString(CharString);
    if (GSetStrings[OUTLINED])
	GSetStrings[OUTLINED]->setString(CharString);
    if (GSetStrings[TEXTURED])
	GSetStrings[TEXTURED]->setString(CharString);
}

static pfGeode*
initPoly()
{
  pfGeoState * gstate = new pfGeoState;
  gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
  gstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);
  gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_OFF);

  pfVec3 * verts = pfNew(pfVec3, 4);
  verts[0].set(-1.0f,  0.0f, 1.0f);
  verts[1].set( 1.0f,  0.0f, 1.0f);
  verts[2].set( 1.0f,  0.0f,-1.0f);
  verts[3].set(-1.0f,  0.0f,-1.0f);

  pfVec2 * texcoords = pfNew(pfVec2, 4);
  texcoords[0].set(0.0f, 0.0f);
  texcoords[1].set(0.0f, 1.0f);
  texcoords[2].set(1.0f, 1.0f);
  texcoords[3].set(1.0f, 0.0f);

  pfVec4 * colors = pfNew(pfVec4, 1);
  colors[0].set(1.0, 1.0, 1.0, 1.0);

  // Create the geoset
  pfGeoSet * gset = new pfGeoSet;
  gset->setPrimType(PFGS_QUADS);
  gset->setNumPrims(1);
  gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, verts, 0);
  gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, 0);
  gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, 0);
  gset->setGState(gstate);

  // gset->setDrawBin(PFSORT_TRANSP_BIN);

  // Create a geode
  pfGeode * geode = new pfGeode;
  geode->addGSet(gset);

  return geode;
}



static pfText*
initText(const char *fontName)
{
    pfMatrix	mat;
    pfFont	*font;
    pfText	*text;
    void	*arena = pfGetSharedArena();
    pfGeoState *stringGS = new pfGeoState;


    stringGS->setMode(PFSTATE_TRANSPARENCY, PFTR_BLEND_ALPHA);
    stringGS->setMode(PFSTATE_ENTEXTURE, PF_ON );

    text = new pfText();

    font = pfdLoadFont_type1(fontName, PFDFONT_EXTRUDED);
    if (font)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
		 "initText() Loaded extruded font %s", fontName);

	GSetStrings[EXTRUDED] = new(arena) pfString();
	GSetStrings[EXTRUDED]->setFont(font);
	GSetStrings[EXTRUDED]->setMode(PFSTR_JUSTIFY, PFSTR_CENTER);
	mat.makeTrans(0.0f, -2.0f, 0.0f);
	GSetStrings[EXTRUDED]->setMat(mat);
	GSetStrings[EXTRUDED]->setGState(stringGS);

	text->addString(GSetStrings[EXTRUDED]);
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "initText() Can't find file for extruded font %s", fontName);

    font = pfdLoadFont_type1(fontName, PFDFONT_FILLED);
    if (font)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
		 "initText() Loaded filled font %s", fontName);

	GSetStrings[FILLED] =  new(arena) pfString();
	GSetStrings[FILLED]->setFont(font);
	GSetStrings[FILLED]->setMode(PFSTR_JUSTIFY, PFSTR_CENTER);
	GSetStrings[FILLED]->setSpacingScale(0.0f, 0.0f, -.5f);
	GSetStrings[FILLED]->setMode(PFSTR_AUTO_SPACING, 1);
	GSetStrings[FILLED]->setColor(0.0f, .3f, 1.0f, 1.0f);
	GSetStrings[FILLED]->setGState(stringGS);

	text->addString(GSetStrings[FILLED]);
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "initText() Can't find file for filled font %s", fontName);

    font = pfdLoadFont_type1(fontName, PFDFONT_TEXTURED);
    if (font)
    {

       //==== Create Geo State ====//



	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
		 "initText() Loaded textured font %s", fontName);

	GSetStrings[TEXTURED] =  new(arena) pfString();

	GSetStrings[TEXTURED]->setFont(font);
	GSetStrings[TEXTURED]->setMode(PFSTR_JUSTIFY, PFSTR_LEFT);
	GSetStrings[TEXTURED]->setSpacingScale(0.4f, .5f, -.4f);
	GSetStrings[TEXTURED]->setMode(PFSTR_AUTO_SPACING, 1);
	mat.makeScale(1.0f, 1.0f, 0.5f);
	mat.postTrans(mat, 1.0f, -5.0f, 0.0f);
	GSetStrings[TEXTURED]->setMat(mat);
	GSetStrings[TEXTURED]->setColor(1.0f, 0.0f, 0.0f, 1.0f);
	GSetStrings[TEXTURED]->setGState(stringGS);

	text->addString(GSetStrings[TEXTURED]);
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "initText() Can't find file for textured font %s", fontName);

    if (UseTex)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,
		 "initText() Loaded outlined font %s", fontName);

	font = pfdLoadFont_type1(fontName, PFDFONT_OUTLINED);
	if (font)
	{
	    GSetStrings[OUTLINED] =  new(arena) pfString();

	    GSetStrings[OUTLINED]->setFont(font);
	    mat.makeRot(60.0f, 0.0f, 0.0f, 1.0f);
	    mat.postTrans(mat, 3.0f, 2.0f, -1.0f);

	    GSetStrings[OUTLINED]->setMode(PFSTR_JUSTIFY, PFSTR_RIGHT);
	    GSetStrings[OUTLINED]->setMat(mat);
	    GSetStrings[OUTLINED]->setColor(1.0f, 1.0f, 0.0f, 1.0f);
	    GSetStrings[OUTLINED]->setGState(stringGS);

	    text->addString(GSetStrings[OUTLINED]);
	}
	else
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		     "initText() Can't find file for outlined font %s",
fontName);
    }

    updateStrings();
    return text;
}

void
OpenPipeWin (pfPipeWindow *pw)
{
    Display 	*dsp;
    Window 	win;

    pw->open();

    dsp = pfGetCurWSConnection();
    win = pw->getWSWindow();
    XSelectInput(dsp, win, PointerMotionMask | KeyPressMask |
			    ButtonPressMask | ButtonReleaseMask);
    XMapWindow(dsp, win);
}

void
DrawChannel (pfChannel *, void *)
{
    Display 	*dsp;
    pfVec4	clr;

    clr.set(.2f, .2f, .2f, 1.0f);
    pfClear(PFCL_COLOR|PFCL_DEPTH, &clr);

// Switch pfDraw() and pfDrawBin() to see that the text is in the transparent bin

    if ( cycle % 30 > 15 )
      pfDraw();
    else
      pfDrawBin(PFSORT_TRANSP_BIN);
    cycle++;

    dsp = pfGetCurWSConnection();
    /* examine the GL-event queue for keyboard events */
    if (XEventsQueued(dsp, QueuedAfterFlush))
    while (XEventsQueued(dsp, QueuedAlready))
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
                switch (ks)
		{
                case XK_Escape:
		    sprintf(CharString, "quit");
                    break;
                case XK_BackSpace:
                case XK_Delete:
                    if (strlen(CharString) > 0)
			CharString[strlen(CharString)-1] = '\0';
                    break;
                case XK_Return:
		    buf[0] = '\n';
		    rv = 1;
		default:
		    strncat(CharString, buf, rv);
		    break;
		}
/*		fprintf(stderr, "%s\n", CharString);*/
	    }
	    break;
	} /* switch event */
    } /* while have events */
}


int
main (int argc, char *argv[])
{
    pfScene     *scene;
    pfDCS	*textDCS;
    pfPipe      *p;
    pfPipeWindow *pw;
    pfChannel   *chan;
    pfSphere 	bsphere;
    pfCoord	view;
    pfLightSource	*ls;
    int		res;

    cycle = 0;

    /* Initialize Performer */
    pfInit();

    if (!(getenv("PFPATH")))
        pfFilePath(".:/usr/share/Performer/data");
    pfNotify(PFNFY_INFO, PFNFY_PRINT,"FilePath: %s\n", pfGetFilePath());

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    //pfMultiprocess(PFMP_DEFAULT);
    pfMultiprocess(0);

    CharString = (char*) pfMalloc(sizeof(char)*1024, pfGetSharedArena());
    GSetStrings = (pfString**) pfCalloc(sizeof(pfString*), 4, pfGetSharedArena());
    sprintf(CharString, "Type away!");

    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    pfConfig();

    pfQueryFeature(PFQFTR_TEXTURE, &res);
    UseTex = (res == PFQFTR_FAST);

    /* Create and attach text to a pfScene. */
    scene = new pfScene();
    textDCS = new pfDCS();

    if (argc > 1)
	textDCS->addChild(initText(argv[1]));
    else
	textDCS->addChild(initText("Times-Elfin"));

    scene->addChild(textDCS);

    pfDCS * polyDCS = new pfDCS();

    polyDCS->addChild( initPoly() );
    scene->addChild(polyDCS);




    /* determine extent of scene's geometry */
    scene->getBound(&bsphere);

    // Configure and open GL window
    p = pfGetPipe(0);
    pw = new pfPipeWindow(p);
    pw->setWinType(PFWIN_TYPE_X);
    pw->setName(argv[0]);
    pw->setOriginSize(0,0,500,500);
    pw->setConfigFunc(OpenPipeWin);
    pw->config();

    // Create and configure a pfChannel.
    chan = new pfChannel(p);

    // Set up frustum
    chan->setFOV(45.0f, 0.0f);
    chan->setNearFar(1.0f, 10.0f * bsphere.radius);

    // Set up view
    view.xyz = bsphere.center;
    view.xyz[PF_Y] -= 1.5f * bsphere.radius;
    view.hpr.set(0.0f, 0.0f, 0.0f);
    chan->setView(view.xyz, view.hpr);

    chan->setScene(scene);
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);

    /* Create a pfLightSource and attach it to scene. */
    ls = new pfLightSource;
    ls->setPos(view.xyz[0], view.xyz[1], view.xyz[2], 1.0f);
    scene->addChild(ls);

    /* Simulate for twenty seconds. */
    while (strcmp(CharString, "quit"))
    {
	/* Initiate cull/draw for this frame. */
	pfFrame();
	updateStrings();
    }

    /* Terminate parallel processes and exit. */
    pfExit();

    return 0;
}

