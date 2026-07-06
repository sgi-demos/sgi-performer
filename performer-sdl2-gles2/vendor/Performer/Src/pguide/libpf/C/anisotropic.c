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
 * anisotropic.c: simple Performer program to demonstrate anisotropic texturing 
 *
 * $Revision: 1.8 $ $Date: 2002/11/19 01:22:38 $ 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#ifndef WIN32
#include <X11/keysym.h>
#endif
#include <Performer/pf.h>
#include <Performer/pfutil.h>



pfVec2          texcoords[]={   {0.0f, 0.0f},
                                {1.0f, 0.0f},
                                {1.0f, 1.0f},
                                {0.0f, 1.0f} };

ushort          texlist[] =     { 0, 1, 2, 3 };

pfVec3          coords[] ={     {-1.0f, -1.0f,   0.0f },
                                { 1.0f, -1.0f,   0.0f },
                                { 1.0f,  1.0f,   0.0f },
                                {-1.0f,  1.0f,   0.0f } };

ushort          vertexlist[] = { 0, 1, 2, 3 };

pfVec4          colors[] ={     {1.0f, 1.0f, 1.0f, 1.0f},
                                {1.0f, 1.0f, 1.0f, 1.0f},
                                {1.0f, 1.0f, 1.0f, 1.0f},
                                {1.0f, 1.0f, 1.0f, 1.0f} };

ushort          colorlist[] =   { 0, 1, 2, 3 };

typedef struct
{
    pfPipeWindow    *pw;
    int		    exitFlag;
    int		    pause;
    int		    XInputInited;
    int             aniso;
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
    pfScene     *scene1;
    pfScene     *scene2;
    pfPipe      *p;
    pfChannel   *chan1;
    pfChannel   *chan2;
    pfSphere	bsphere;
    pfGroup	*root1;
    pfGroup	*root2;
    pfGeoSet	*gset;
    pfGeoSet	*gset2;
    pfGeode     *geode1,*geode2;
    pfGeoState  *gstate;
    pfGeoState  *gstate2;
    pfTexture   *tex;
    pfTexture   *tex2;
    pfTexEnv    *tev;
    pfDCS       *dcs1,*dcs2;
    void	*arena;
    int         supported=0;
    int         *leftArg;
    int         *rightArg;
    pfWSConnection  dsp=NULL;

    /* Initialize Performer */
    pfInit();	

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess(PFMP_DEFAULT);			

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
    p = pfGetPipe(0);
    Shared->pw = pfNewPWin(p);
    pfPWinName(Shared->pw, "OpenGL Performer");
    pfPWinType(Shared->pw, PFPWIN_TYPE_X);
    pfPWinOriginSize(Shared->pw, 0, 0, 800, 600);
    /* Open and configure the GL window. */
    pfOpenPWin(Shared->pw);
    pfFrame();

    /* Does this platform support anisotropic filtering? */
    pfQueryFeature(PFQFTR_TEXTURE_ANISOTROPIC, &supported);
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Anisotropic filtering is %s supported on this platform\n", (supported!=0)?"":"not");

    /* What is the max anisotropy? */
    pfQuerySys(PFQSYS_MAX_ANISOTROPY, &Shared->aniso);
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Max Anisotropy is %d\n", Shared->aniso);

    /* Set up textures & gstates structures */
    arena = pfGetSharedArena();
    tex = pfNewTex (arena); 
    pfTexFilter(tex, PFTEX_MINFILTER, PFTEX_MIPMAP_TRILINEAR);
    pfTexAnisotropy(tex, Shared->aniso);
    gstate = pfNewGState (arena);
    if (pfLoadTexFile (tex, "rwb0.rgb"))
    {
	uint *i;
	int nc, sx, sy, sz;
	pfGetTexImage(tex, &i, &nc, &sx, &sy, &sz);
	/* if have alpha channel, enable transparency */
	if (nc != 3)
	    pfGStateMode (gstate, PFSTATE_TRANSPARENCY, PFTR_HIGH_QUALITY);
	/* set alpha function to block pixels of 0 alpha for 
	   transparent textures */
	pfGStateMode (gstate, PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
	pfGStateVal (gstate, PFSTATE_ALPHAREF, 0.0f);
	pfGStateAttr (gstate, PFSTATE_TEXTURE, tex);
	pfGStateMode (gstate, PFSTATE_ENTEXTURE, 1);
	pfGStateMode (gstate, PFSTATE_ENLIGHTING, 0);
	pfGStateMode (gstate, PFSTATE_CULLFACE, PFCF_OFF);
	tev = pfNewTEnv (arena);
	pfGStateAttr (gstate, PFSTATE_TEXENV, tev);
	
    }
    /* Set up geosets */
    gset = pfNewGSet(arena);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, vertexlist);
    pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, texlist);
    pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, colors, colorlist);
    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 1);
    pfGSetGState (gset, gstate);
    /* set up scene graph */
    geode1 = pfNewGeode(); 
    pfAddGSet(geode1, gset);
    
    tex2 = pfNewTex (arena); 
    pfTexFilter(tex2, PFTEX_MINFILTER, PFTEX_MIPMAP_TRILINEAR);
    gstate2 = pfNewGState (arena);
    if (pfLoadTexFile (tex2, "rwb0.rgb"))
    {
        uint *i;
        int nc, sx, sy, sz;
        pfGetTexImage(tex2, &i, &nc, &sx, &sy, &sz);
        /* if have alpha channel, enable transparency */
        if (nc != 3)
    	pfGStateMode (gstate2, PFSTATE_TRANSPARENCY, PFTR_HIGH_QUALITY);
        /* set alpha function to block pixels of 0 alpha for 
           transparent textures */
        pfGStateMode (gstate2, PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
        pfGStateVal (gstate2, PFSTATE_ALPHAREF, 0.0f);	
        pfGStateAttr (gstate2, PFSTATE_TEXTURE, tex2);
        tev = pfNewTEnv (arena);
        pfGStateAttr (gstate2, PFSTATE_TEXENV, tev);
        pfGStateMode (gstate2, PFSTATE_ENTEXTURE, 1);
        pfGStateMode (gstate2, PFSTATE_ENLIGHTING, 0);
        pfGStateMode (gstate2, PFSTATE_CULLFACE, PFCF_OFF);
    }
    gset2 = pfNewGSet(arena);
    pfGSetAttr(gset2, PFGS_COORD3, PFGS_PER_VERTEX, coords, vertexlist);
    pfGSetAttr(gset2, PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, texlist);
    pfGSetAttr(gset2, PFGS_COLOR4, PFGS_PER_VERTEX, colors, colorlist);
    pfGSetPrimType(gset2, PFGS_QUADS);
    pfGSetNumPrims(gset2, 1);
    pfGSetGState (gset2, gstate2);
    geode2 = pfNewGeode(); 
    pfAddGSet(geode2, gset2);

    dcs1 = pfNewDCS();
    pfDCSTrans (dcs1, -0.5f, 0.0f, 0.0f);
    pfAddChild(dcs1, geode1);

    dcs2 = pfNewDCS();
    pfDCSTrans (dcs2, -0.5f, 0.0f, 0.0f);
    pfAddChild(dcs2, geode2);

    root1 = pfNewGroup();
    pfAddChild(root1, dcs1);

    root2 = pfNewGroup();
    pfAddChild(root2, dcs2);

    scene1 = pfNewScene();
    pfAddChild(scene1, root1);

    scene2 = pfNewScene();
    pfAddChild(scene2, root2);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene1, &bsphere);

    /* Create and configure a pfChannel. */
    chan1 = pfNewChan(p);
    chan2 = pfNewChan(p);
    pfChanScene(chan1, scene1);
    pfChanTravFunc(chan1, PFTRAV_DRAW, DrawChannel);
    pfChanScene(chan2, scene2);
    pfChanTravFunc(chan2, PFTRAV_DRAW, DrawChannel);

    pfChanNearFar(chan1, 0.1f, 100.0f * bsphere.radius);
    pfChanNearFar(chan2, 0.1f, 100.0f * bsphere.radius);
    /* 45 degrees wide,  vertical=-1 to signal match window aspect */
    pfChanFOV(chan1, 45.0f, -1.0f);
    pfChanFOV(chan2, 45.0f, -1.0f);
    pfChanViewport(chan1, 0.0f, 0.5f, 0.0f, 1.0f);
    pfChanViewport(chan2, 0.5f, 1.0f, 0.0f, 1.0f);

    /* set up data to distinguish between left and right eye */
    leftArg = (int *)pfAllocChanData(chan1, sizeof(int));
    rightArg = (int *)pfAllocChanData(chan2, sizeof(int));

    *leftArg = 1;
    *rightArg = 0;

    /* data never changes, so we only need to pass it once */
    pfPassChanData(chan1);
    pfPassChanData(chan2);


    /* Create an earth/sky model that draws sky/ground/horizon */
    {
	pfEarthSky *esky = pfNewESky();
	pfESkyMode(esky, PFES_BUFFER_CLEAR, PFES_SKY_GRND );
	pfESkyAttr(esky, PFES_GRND_HT, -1.0f * bsphere.radius);
	pfESkyColor(esky, PFES_GRND_FAR, 0.0f, 0.15f, 0.2f, 1.0f);
	pfESkyColor(esky, PFES_GRND_NEAR, 0.0f, 0.15f, 0.2f, 1.0f);
	pfESkyColor(esky, PFES_CLEAR, .3f, .3f, .7f, 0.5f);
	pfChanESky(chan1, esky);
	pfChanESky(chan2, esky);
    }

    dsp = pfGetCurWSConnection();
	
#ifndef WIN32
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
	pfSetVec3(view.hpr, 5.0f*t, 5.0f, 0);
	pfSetVec3(view.xyz, 0.7f * bsphere.radius * s, 
		-0.7f * bsphere.radius *c, 
		 0.05f * bsphere.radius);
	
	/* set view position for next frame */
	pfChanView(chan1, view.xyz, view.hpr);
	pfChanView(chan2, view.xyz, view.hpr);

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
    if (w = pfGetPWinWSWindow(Shared->pw))
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

    /* erase framebuffer and draw Earth-Sky model */
    pfClearChan(channel);

    /* invoke Performer draw-processing for this frame */
    pfDraw();

    if(firsttime)
    {
	pfuMakeRasterXFont("-*-courier-bold-r-normal--14-*-*-*-m-90-iso8859-1", &fnt);
	pfuSetXFont(&fnt);
	sprintf(anisotropy, "Max Anisotropy: %d", Shared->aniso);
	firsttime=0;
    }

    glDepthFunc(GL_ALWAYS); /* always draw */
    glDepthMask(GL_FALSE);

    pfPushState();
    pfBasicState();

    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)tempmat);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10.0, 10.0, -10.0, 10.0, 1.0, -1.0);

    glMatrixMode(GL_MODELVIEW);

    pfPushIdentMatrix();

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
    glLoadMatrixf((GLfloat *)tempmat);
    glMatrixMode(GL_MODELVIEW);

    pfPopState();
    
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
}


