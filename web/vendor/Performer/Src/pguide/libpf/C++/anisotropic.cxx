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
 * anisotropic.C: simple Performer program to demonstrate anisotropic texturing 
 *
 * $Revision: 1.6 $ $Date: 2002/11/14 01:40:18 $ 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#ifndef WIN32
#include <X11/keysym.h>
#endif

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfEarthSky.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pfutil.h>


typedef struct
{
    pfPipeWindow    *pw;
    int		    exitFlag;
    int		    pause;
    int		    XInputInited;
    int		    aniso;
} SharedData;

static SharedData *Shared;
static int         ForkedXInput = 0;
#ifndef WIN32
static Atom	   wm_protocols, wm_delete_window;

static void InitXInput(pfWSConnection dsp);
static void DoXInput(void);
static void GetXInput(Display *dsp);
#endif
static void DrawChannel(pfChannel *chan, void *data);


int
main (int argc, char *argv[])
{
    pfTexEnv    *tev;
    int         supported=0;
    pfWSConnection  dsp=NULL;

    /* Initialize Performer */
    pfInit();	

    /* Use default multiprocessing mode based on number of
     * processors.
    pfMultiprocess(PFMP_DEFAULT);
     */
    pfMultiprocess(PFMP_APPCULLDRAW);

    /* allocate shared before fork()'ing parallel processes */
    Shared = (SharedData*)pfCalloc(1,sizeof(SharedData), pfGetSharedArena());
    Shared->pause=0;

    /* Configure multiprocessing mode and start parallel
     * processes.
     */
    pfConfig();			

    /* Append to PFPATH files in /usr/share/Performer/data */
    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures/");

    /* Configure and open GL window */
    pfPipe *p = pfGetPipe(0);
    Shared->pw = new pfPipeWindow(p);
    Shared->pw->setName("OpenGL Performer");
    Shared->pw->setWinType(PFPWIN_TYPE_X);
    Shared->pw->setOriginSize(0, 0, 800, 600);
    /* Open and configure the GL window. */
    Shared->pw->open();
    pfFrame();

    /* Does this platform support anisotropic filtering? */
    pfQueryFeature(PFQFTR_TEXTURE_ANISOTROPIC, &supported);
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Anisotropic filtering is %s supported on this platform\n", (supported!=0)?"":"not");


    /* What is the max anisotropy? */
    pfQuerySys(PFQSYS_MAX_ANISOTROPY, &Shared->aniso);
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Max Anisotropy is %d\n", Shared->aniso);

    /* Set up textures & gstates structures */
    pfTexture *tex = new pfTexture; 
    tex->setFilter(PFTEX_MINFILTER, PFTEX_MIPMAP_TRILINEAR);
    tex->setAnisotropy(Shared->aniso);
    pfGeoState *gstate = new pfGeoState;
    if (tex->loadFile ("rwb0.rgb"))
    {
	uint *i;
	int nc, sx, sy, sz;
	tex->getImage(&i, &nc, &sx, &sy, &sz);
	/* if have alpha channel, enable transparency */
	if (nc != 3)
	    gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_HIGH_QUALITY);
	/* set alpha function to block pixels of 0 alpha for 
	   transparent textures */
	gstate->setMode (PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
	gstate->setVal  (PFSTATE_ALPHAREF, 0.0f);
	gstate->setAttr (PFSTATE_TEXTURE, tex);
	gstate->setMode (PFSTATE_ENTEXTURE, 1);
	gstate->setMode (PFSTATE_ENLIGHTING, 0);
	gstate->setMode (PFSTATE_CULLFACE, PFCF_OFF);
	tev = new pfTexEnv;
	gstate->setAttr (PFSTATE_TEXENV, tev);
    }
	
    /* Set up geosets */
    pfVec3 *coords = (pfVec3*) new(4*sizeof(pfVec3)) pfMemory;
    coords[0].set(-1.0f, -1.0f,  -0.0f);
    coords[1].set( 1.0f, -1.0f,  -0.0f );
    coords[2].set( 1.0f,  1.0f,   0.0f );
    coords[3].set(-1.0f,  1.0f,   0.0f );
    
    ushort *vertexlist = (ushort*) new(4*sizeof(ushort)) pfMemory;
    vertexlist[0] = 0;
    vertexlist[1] = 1;
    vertexlist[2] = 2;
    vertexlist[3] = 3;
    
    pfVec4 *colors = (pfVec4*) new(4*sizeof(pfVec4)) pfMemory;
    colors[0].set(1.0f, 1.0f, 1.0f, 1.0f);
    colors[1].set(1.0f, 1.0f, 1.0f, 1.0f);
    colors[2].set(1.0f, 1.0f, 1.0f, 1.0f);
    colors[3].set(1.0f, 1.0f, 1.0f, 1.0f);
    
    ushort *colorlist = (ushort*) new(4*sizeof(ushort)) pfMemory;
    colorlist[0] = 0;
    colorlist[1] = 1;
    colorlist[2] = 2;
    colorlist[3] = 3;
    
    pfVec2 *texcoords = (pfVec2*) new(4*sizeof(pfVec2)) pfMemory;
    texcoords[0].set(0.0f, 0.0f);
    texcoords[1].set(1.0f, 0.0f);
    texcoords[2].set(1.0f, 1.0f);
    texcoords[3].set(0.0f, 1.0f);;
    
    ushort *texlist = (ushort*) new(4*sizeof(ushort)) pfMemory;
    texlist[0] = 0;
    texlist[1] = 1;
    texlist[2] = 2;
    texlist[3] = 3;


    pfGeoSet *gset = new pfGeoSet;
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, vertexlist);
    gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, texlist);
    gset->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, colors, colorlist);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(1);
    gset->setGState(gstate);
    /* set up scene graph */
    pfGeode *geode1 = new pfGeode; 
    geode1->addGSet(gset);
    
    pfTexture *tex2 = new pfTexture; 
    tex2->setFilter(PFTEX_MINFILTER, PFTEX_MIPMAP_TRILINEAR);
    pfGeoState *gstate2 = new pfGeoState;
    if (tex2->loadFile("rwb0.rgb"))
    {
        uint *i;
        int nc, sx, sy, sz;
        tex2->getImage(&i, &nc, &sx, &sy, &sz);
        /* if have alpha channel, enable transparency */
        if (nc != 3)
    	gstate2->setMode(PFSTATE_TRANSPARENCY, PFTR_HIGH_QUALITY);
        /* set alpha function to block pixels of 0 alpha for 
           transparent textures */
        gstate2->setMode(PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
        gstate2->setVal (PFSTATE_ALPHAREF, 0.0f);	
        gstate2->setAttr(PFSTATE_TEXTURE, tex2);
        tev = new pfTexEnv;
        gstate2->setAttr(PFSTATE_TEXENV, tev);
        gstate2->setMode(PFSTATE_ENTEXTURE, 1);
        gstate2->setMode(PFSTATE_ENLIGHTING, 0);
        gstate2->setMode(PFSTATE_CULLFACE, PFCF_OFF);
    }
    pfGeoSet *gset2 = new pfGeoSet;
    gset2->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, vertexlist);
    gset2->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, texlist);
    gset2->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, colors, colorlist);
    gset2->setPrimType(PFGS_QUADS);
    gset2->setNumPrims(1);
    gset2->setGState(gstate2);
    pfGeode *geode2 = new pfGeode;
    geode2->addGSet(gset2);

    pfDCS *dcs1 = new pfDCS;
    dcs1->setTrans(-0.5f, 0.0f, 0.0f);
    dcs1->addChild(geode1);

    pfDCS *dcs2 = new pfDCS;
    dcs2->setTrans(-0.5f, 0.0f, 0.0f);
    dcs2->addChild(geode2);

    pfGroup *root1 = new pfGroup;
    root1->addChild(dcs1);

    pfGroup *root2 = new pfGroup;
    root2->addChild(dcs2);

    pfScene *scene1 = new pfScene;
    scene1->addChild(root1);

    pfScene *scene2 = new pfScene;
    scene2->addChild(root2);

    /* determine extent of scene's geometry */
    pfSphere bsphere;
    scene1->getBound(&bsphere);

    /* Create and configure a pfChannel. */
    pfChannel *chan1 = new pfChannel(p);
    pfChannel *chan2 = new pfChannel(p);
    chan1->setScene(scene1);
    chan1->setTravFunc(PFTRAV_DRAW, DrawChannel);
    chan2->setScene(scene2);
    chan2->setTravFunc(PFTRAV_DRAW, DrawChannel);
    chan1->setNearFar(0.1f, 100.0f * bsphere.radius);
    chan2->setNearFar(0.1f, 100.0f * bsphere.radius);
    chan1->setFOV(45.0f, -1.0f);
    chan2->setFOV(45.0f, -1.0f);
    chan1->setViewport(0.0f, 0.5f, 0.0f, 1.0f);
    chan2->setViewport(0.5f, 1.0f, 0.0f, 1.0f);

    /* set up data to distinguish between left and right eye */
    int *leftArg = (int *)chan1->allocChanData(sizeof(int));
    int *rightArg = (int *)chan2->allocChanData(sizeof(int));

    *leftArg = 1;
    *rightArg = 0;

    /* data never changes, so we only need to pass it once */
    chan1->passChanData();
    chan2->passChanData();

    /* Create an earth/sky model that draws sky/ground/horizon */
    {
	pfEarthSky *esky = new pfEarthSky();
	esky->setMode(PFES_BUFFER_CLEAR, PFES_SKY_GRND );
	esky->setAttr(PFES_GRND_HT, -1.0f * bsphere.radius);
	esky->setColor(PFES_GRND_FAR, 0.0f, 0.15f, 0.2f, 1.0f);
	esky->setColor(PFES_GRND_NEAR, 0.0f, 0.15f, 0.2f, 1.0f);
	esky->setColor(PFES_CLEAR, .3f, .3f, .7f, 0.5f);
	chan1->setESky(esky);
	chan2->setESky(esky);
    }

#ifndef WIN32
    dsp = pfGetCurWSConnection();
	
    if (ForkedXInput)
    {
    	pid_t    fpid = 0;
	if ((fpid = fork()) < 0)
		pfNotify(PFNFY_FATAL, PFNFY_SYSERR, "Fork of XInput process failed.");
	else if (fpid)
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"XInput running in forked process %d", fpid);
	if (!fpid)
		DoXInput();
    }
#endif


    /* Simulate for twenty seconds. */
    while (!Shared->exitFlag)
    {
	float      s, c;
	pfCoord	   view;
	static float t=0.0f;

	/* Go to sleep until next frame time. */
	pfSync();		

	/* Initiate cull/draw for this frame. */
	pfFrame();

	if(!Shared->pause)
	    t+=0.01;
	
	/* Compute new view position for next frame. */
	pfSinCos(5.0f*t, &s, &c);
	view.hpr.set(5.0f*t, 5.0f, 0);
	view.xyz.set(0.7f * bsphere.radius * s, 
		-0.7f * bsphere.radius *c, 
		 0.05f * bsphere.radius);
	
	/* set view position for next frame */
	chan1->setView(view.xyz, view.hpr);
	chan2->setView(view.xyz, view.hpr);

#ifndef WIN32
	if (!ForkedXInput)
	{
	    if (!Shared->XInputInited)
		InitXInput(dsp);
	    if (Shared->XInputInited)
		GetXInput(dsp);
	}
#endif
    }

    /* Terminate parallel processes and exit. */
    pfExit();

    return 0;
}

#ifndef WIN32
static void 
InitXInput(pfWSConnection dsp)
{
    Window w;
    
    /* wait for X Window to exist in Performer shared memory */
    if (w = Shared->pw->getWSWindow())
    {
	XSelectInput(dsp, w, KeyPressMask);
	wm_protocols = XInternAtom(dsp, "WM_PROTOCOLS", 1);
	wm_delete_window = XInternAtom(dsp, "WM_DELETE_WINDOW", 1);
	XMapWindow(dsp, w);
	XFlush(dsp);
	Shared->XInputInited = 1;
    }
}

/*
 * DoXInput() runs an asychronous forked even handling process.
 *  Shared memory structures can be read from this process
 *  but NO performer calls that set any structures should be 
 *  issues by routines in this process.
 */
static void
DoXInput(void)
{
    /* windows from draw should now exist so can attach X input handling
     * to the X window 
     */
    pfWSConnection dsp = pfGetCurWSConnection();


#ifdef __linux__
    prctl(PR_SET_PDEATHSIG, SIGHUP, 0, 0, 0);
#else
    prctl(PR_TERMCHILD);        /* Exit when parent does */
    sigset(SIGHUP, SIG_DFL);    /* Exit when sent SIGHUP by TERMCHILD */
#endif  /* __linux__ */
        
    while (1)
    {
	XEvent          event;
	if (!Shared->XInputInited)
	    InitXInput(dsp);
	if (Shared->XInputInited)
	{
	    XPeekEvent(dsp, &event);
	    GetXInput(dsp);
	}
    }
}

static void
GetXInput(pfWSConnection dsp)
{
    if (XEventsQueued(dsp, QueuedAfterFlush))
	while (XEventsQueued(dsp, QueuedAlready))
	{
		XEvent event;
	    
		XNextEvent(dsp, &event);

		switch (event.type) 
		{
		case ClientMessage:
		    if ((event.xclient.message_type == wm_protocols) &&
			(event.xclient.data.l[0] == wm_delete_window)) 
		    {
			pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Window exit !!");
			pfExit();
		    } 
		break;
		case KeyPress:
		{
		    char buf[100];
		    KeySym ks;
		    XLookupString(&event.xkey, buf, sizeof(buf), &ks, 0);
		    switch(ks) 
		    {
		    case XK_Escape: 
			Shared->exitFlag = 1;
			pfExit();
			exit(0);
		    break;
		    case XK_space:
			Shared->pause = !Shared->pause;
			break;
		    default:
			break;
		    } /* switch(ks) */
		} /* case KeyPress */
		break;
		default:
		    break;
		}/* switch(event.type) */
        } /* while() */
} /* GetXInput() */
#endif

static pfuXFont fnt;

static void
DrawChannel(pfChannel *channel, void *left)
{
    static int firsttime=1;
    static pfMatrix tempmat;
    static char anisotropy[40];

    // erase framebuffer and draw Earth-Sky model
    channel->clear();

    // invoke Performer draw-processing for this frame
    pfDraw();

    if(firsttime)
    {
	pfuMakeRasterXFont("-*-courier-bold-r-normal--14-*-*-*-m-90-iso8859-1", &fnt);
	pfuSetXFont(&fnt);
	sprintf(anisotropy, "Max Anisotropy: %d", Shared->aniso);
	firsttime=0;
    }

    glDepthFunc(GL_ALWAYS); // always draw
    glDepthMask(GL_FALSE);

    pfPushState();
    pfBasicState();

    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)tempmat.mat);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat *)pfIdentMat.mat);
    glOrtho(-10.0, 10.0, -10.0, 10.0, 1.0, -1.0);

    glMatrixMode(GL_MODELVIEW);

    pfPushMatrix();
    pfLoadMatrix(pfIdentMat);

    if(*(int *)left)
    {
	pfuDrawStringPos("Spacebar to pause.", -10.0f, 9.5f, 0.0f);
	pfuDrawStringPos("Esc to quit.", -10.0f, 9.0f, 0.0f);
	pfuDrawStringPos(anisotropy, -4.0f, -8.5f, 0.0f);
    }
    else
    {
	pfuDrawStringPos("No Anisotropy.", -4.0f, -8.5f, 0.0f);
    }

    pfPopMatrix();
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat *)tempmat.mat);
    glMatrixMode(GL_MODELVIEW);

    pfPopState();
    
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
}


