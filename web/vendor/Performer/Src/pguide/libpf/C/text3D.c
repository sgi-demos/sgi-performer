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
 * text3D.c: Performer program to demonstrate use of pfText, pfString,
 * 	     and pfFont.
 *        Based on simple.c
 *
 * $Revision: 1.5 $ $Date: 2000/10/06 21:26:39 $ 
 *
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <X11/keysym.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>

#define EXTRUDED	0
#define FILLED		1
#define OUTLINED	2
#define TEXTURED	3

char		*CharString;
pfString	**GSetStrings;
int		UseTex;

/*------------------------------------------------------------------*/

static void
updateStrings(void)
{
    if (GSetStrings[EXTRUDED])
	pfStringString(GSetStrings[EXTRUDED], CharString);
    if (GSetStrings[FILLED])
	pfStringString(GSetStrings[FILLED], CharString);
    if (GSetStrings[OUTLINED])
	pfStringString(GSetStrings[OUTLINED], CharString);
    if (GSetStrings[TEXTURED])
	pfStringString(GSetStrings[TEXTURED], CharString);
}


static pfText*
initText(const char *fontName)
{
    pfMatrix	mat;
    pfFont	*font;
    pfText	*text;
    void	*arena = pfGetSharedArena();

    text = pfNewText();

    font = pfdLoadFont_type1(fontName, PFDFONT_EXTRUDED);
    if (font)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE, 
		 "initText() Loaded extruded font %s", fontName);

	GSetStrings[EXTRUDED] = pfNewString(arena);
	pfStringFont(GSetStrings[EXTRUDED], font);
	pfStringMode(GSetStrings[EXTRUDED], PFSTR_JUSTIFY, PFSTR_CENTER);
	pfMakeTransMat(mat, 0.0f, -2.0f, 0.0f);
	pfStringMat(GSetStrings[EXTRUDED], mat);
	pfAddString(text, GSetStrings[EXTRUDED]);
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
		 "initText() Can't find file for extruded font %s", fontName);

    font = pfdLoadFont_type1(fontName, PFDFONT_FILLED);
    if (font)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE, 
		 "initText() Loaded filled font %s", fontName);

	GSetStrings[FILLED] = pfNewString(arena);

	pfStringFont(GSetStrings[FILLED], font);
	pfStringMode(GSetStrings[FILLED], PFSTR_JUSTIFY, PFSTR_CENTER);
	pfStringSpacingScale(GSetStrings[FILLED], 0.0f, 0.0f, -.5f);
	pfStringMode(GSetStrings[FILLED], PFSTR_AUTO_SPACING, 1);
	pfStringColor(GSetStrings[FILLED], 0.0f, .3f, 1.0f, 1.0f);

	pfAddString(text, GSetStrings[FILLED]);
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
		 "initText() Can't find file for filled font %s", fontName);
	    
    font = pfdLoadFont_type1(fontName, PFDFONT_TEXTURED);
    if (font)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE, 
		 "initText() Loaded textured font %s", fontName);

	GSetStrings[TEXTURED] = pfNewString(arena);

	pfStringFont(GSetStrings[TEXTURED], font);
	pfStringMode(GSetStrings[TEXTURED], PFSTR_JUSTIFY, PFSTR_LEFT);
	pfStringSpacingScale(GSetStrings[TEXTURED], 0.4f, .5f, -.4f);
	pfStringMode(GSetStrings[TEXTURED], PFSTR_AUTO_SPACING, 1);
	pfMakeScaleMat(mat, 1.0f, 1.0f, 0.5f);
	pfPostTransMat(mat, mat, 1.0f, -5.0f, 0.0f);
	pfStringMat(GSetStrings[TEXTURED], mat);
	pfStringColor(GSetStrings[TEXTURED], 1.0f, 0.0f, 0.0f, 1.0f);

	pfAddString(text, GSetStrings[TEXTURED]);
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
	    GSetStrings[OUTLINED] = pfNewString(arena);

	    pfStringFont(GSetStrings[OUTLINED], font);
	    pfMakeRotMat(mat, 60.0f, 0.0f, 0.0f, 1.0f);
	    pfPostTransMat(mat, mat, 3.0f, 2.0f, -1.0f);

	    pfStringMode(GSetStrings[OUTLINED], PFSTR_JUSTIFY, PFSTR_RIGHT);
	    pfStringMat(GSetStrings[OUTLINED], mat);
	    pfStringColor(GSetStrings[OUTLINED], 1.0f, 1.0f, 0.0f, 1.0f);

	    pfAddString(text, GSetStrings[OUTLINED]);
	}
	else
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
		     "initText() Can't find file for outlined font %s", fontName);
    }

    updateStrings();
    return text;
}

void 
OpenPipeWin (pfPipeWindow *pw)
{
    Display 	*dsp;
    Window 	win;

    pfOpenPWin(pw);

    dsp = pfGetCurWSConnection();
    win = pfGetPWinWSWindow(pw);
    XSelectInput(dsp, win, PointerMotionMask | KeyPressMask | 
			    ButtonPressMask | ButtonReleaseMask);
    XMapWindow(dsp, win);
}

void 
DrawChannel (pfChannel *chan, void *data)
{
    Display 	*dsp;
    pfVec4	clr;

    pfSetVec4(clr, .2f, .2f, .2f, 1.0f); 
    pfClear(PFCL_COLOR|PFCL_DEPTH, clr);
    pfDraw();

    dsp = pfGetCurWSConnection();
    /* examine the GL-event queue for keyboard events */
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

    /* Initialize Performer */
    pfInit();	

    if (!(getenv("PFPATH")))
        pfFilePath(".:/usr/share/Performer/data");
    pfNotify(PFNFY_INFO, PFNFY_PRINT,"FilePath: %s\n", pfGetFilePath());

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess(PFMP_DEFAULT);

    CharString = pfMalloc(sizeof(char)*1024, pfGetSharedArena());
    GSetStrings = pfCalloc(sizeof(pfString*), 4, pfGetSharedArena());
    sprintf(CharString, "Type away!");

    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    pfConfig();			

    pfQueryFeature(PFQFTR_TEXTURE, &res);
    UseTex = (res == PFQFTR_FAST);

    /* Create and attach text to a pfScene. */
    scene = pfNewScene();
    textDCS = pfNewDCS();

    if (argc > 1)
	pfAddChild(textDCS, initText(argv[1]));
    else
	pfAddChild(textDCS, initText("Times-Elfin"));

    pfAddChild(scene, textDCS);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene, &bsphere);

    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinType(pw, PFWIN_TYPE_X);
    pfPWinName(pw, argv[0]);
    pfPWinConfigFunc(pw, OpenPipeWin);	
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    pfConfigPWin(pw);	

    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere.radius);
    pfChanFOV(chan, 45.0f, 0.0f);
    pfCopyVec3(view.xyz, bsphere.center);
    view.xyz[PF_Y] -= 1.5f * bsphere.radius;
    pfSetVec3(view.hpr, 0.0f, 0.0f, 0.0f); 
    pfChanView(chan, view.xyz, view.hpr);
    pfChanTravFunc(chan, PFTRAV_DRAW, DrawChannel);

    /* Create a pfLightSource and attach it to scene. */
    ls = pfNewLSource();
    pfLSourcePos(ls, view.xyz[0], view.xyz[1], view.xyz[2], 1.0f); 
    pfAddChild(scene, ls);

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




