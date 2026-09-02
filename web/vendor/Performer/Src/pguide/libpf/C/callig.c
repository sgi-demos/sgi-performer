/*
 * Copyright 2000 - Silicon Graphics, Inc.
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
 */

#include <stdlib.h>
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

 static int NumScreens=1;
static int NumPipes=1;
 
static void ConfigPipeDraw(int pipe, uint stage);
static void OpenPipeWin(pfPipeWindow *pw);
static void OpenXWin(pfPipeWindow *pw);
static void DrawChannel(pfChannel *chan, void *data);
 
/*
 * Usage() -- print usage advice and exit. This
 *      procedure is executed in the application process.
 */
static void
Usage ()
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: callig <file.ext> ...\n");
    exit(1);
}
int
main (int argc, char *argv[])
{
    float       t = 0.0f;
    pfScene     *scene;
    pfPipe      *pipe[4];
    pfChannel   *chan[4];
    pfNode *root;
    pfSphere bsphere;
    int loop;
    
    if(argc < 2) {
      Usage();
    }

    /* Initialize Performer */
    pfInit(); 
    pfuInitUtil(); 
    
    /* specify the number of pfPipes */
    /* Configure and open GL windows */
#ifndef WIN32
    if ((NumScreens = ScreenCount(pfGetCurWSConnection())) > 1)
    {
        NumPipes = NumScreens;
    }
#else
    NumScreens = 1;
    NumPipes = 1;
#endif
    
    pfMultipipe (NumPipes);
    
    /* Initialize Calligraphic HW */
    for (loop=0; loop < NumPipes; loop++)
    {
        int q;
        pfQueryFeature(PFQFTR_CALLIGRAPHIC, &q);
        if (!q)
       {
            pfNotify(PFNFY_NOTICE,PFNFY_RESOURCE, "Calligraphic points are NOT supported on this platform");
        }
        
        if (pfCalligInitBoard(loop) == TRUE)
        {
            /* get the memory size */
            pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE,"StartCalligraphic: board(%d) has %d Bytes of memory",0, pfGetCalligBoardMemSize(loop));
        }
        else
        {
            pfNotify(PFNFY_NOTICE, PFNFY_RESOURCE, "Could not init calligraphic board %d", loop);
        }
    }
 
    /* Force Multiprocessor mode to use the light point process */
    
    pfMultiprocess(PFMP_APP_CULL_DRAW |  PFMP_FORK_LPOINT);
    
    /* Load all loader DSO's before pfConfig() forks */

    pfdInitConverter(argv[1]);
    
    /* Configure multiprocessing mode and start parallel processes.*/
    
    pfConfig();
    /* Append to PFPATH additional standard directories where geometry and textures exist */
    
    scene = pfNewScene();

    if (!(getenv("PFPATH")))
      pfFilePath(".:./data:../data:../../data:/usr/share/Performer/data");
    
    /* Read a single file, of any known type. */
    if ((root = pfdLoadFile(argv[1])) == NULL) 
      {
	pfExit();
	exit(-1);
      }
    
    /* Attach loaded file to a pfScene. */
    pfAddChild(scene, root);
    
    /* determine extent of scene's geometry */
    pfGetNodeBSphere (scene, &bsphere);
    /* Create a pfLightSource and attach it to scene. */
    pfAddChild(scene, pfNewLSource());
    
    
    for (loop=0; loop < NumPipes; loop++) {
      
      pfPipeWindow *pw;
      char str[PF_MAXSTRING];
      
      pipe[loop] = pfGetPipe(loop);
      pfPipeScreen(pipe[loop], loop);
      pw = pfNewPWin(pipe[loop]);
      pfPWinType(pw, PFPWIN_TYPE_X);
      sprintf(str, "OpenGL Performer - Pipe %d", loop);
      pfPWinName(pw, str);
      
      if (NumScreens > 1)
	{
	  pfPWinOriginSize(pw, 0, 0, 300, 300);
	} else
	  pfPWinOriginSize(pw, (loop&0x1)*315, ((loop&0x2)>>1)*340, 300, 300);
    
      pfPWinConfigFunc(pw, OpenPipeWin);
      pfConfigPWin(pw);
        
    }
    
    
    /* set up optional DRAW pipe stage config callback */
    pfStageConfigFunc(-1 /* selects all pipes */, PFPROC_DRAW /* stage bitmask */, ConfigPipeDraw);
    pfConfigStage(-1, PFPROC_DRAW);
    
    pfFrame();
    
    /* Create and configure pfChannels - by default, channels are placed in the first window on their pipe */
    
    for (loop=0; loop < NumPipes; loop++)
    {
        chan[loop] = pfNewChan(pipe[loop]); 
        pfChanScene(chan[loop], scene);
        pfChanFOV(chan[loop], 45.0f, 0.0f);
        pfChanTravFunc(chan[loop], PFTRAV_DRAW, DrawChannel);
        pfChanCalligEnable(chan[loop], 1);
        /* synchronize the lpoint board with the swap ready signal */
        pfCalligSwapVME(loop);
        pfCalligSwapVME(loop);
    }
    
    /* Simulate for twenty seconds. */
    while (t < 30.0f)
    {
        pfCoord view;
        float      s, c;
        
        /* Go to sleep until next frame time. */
        pfSync();  
        /* Initiate cull/draw for this frame. */
        pfFrame();  
        
        pfSinCos(45.0f*t, &s, &c);
        pfSetVec3(view.hpr, 45.0f*t, -10.0f, 0);
        pfSetVec3(view.xyz, 2.0f * bsphere.radius * s, -2.0f *         bsphere.radius *c, 0.5f * bsphere.radius);
        for (loop=0; loop < NumPipes; loop++)
            pfChanView(chan[loop], view.xyz, view.hpr);
    }
    
    /* Terminate parallel processes and exit. */
    pfExit();
    return 0;
}
    
/* ConfigPipeDraw() --
 * Application state for the draw process can be initialized
 * here. This is also a good place to do real-time
 * configuration for the drawing process, if there is one.
 * There is no graphics state or pfState at this point so no
 * rendering calls or pfApply*() calls can be made.
 */
    
static void
ConfigPipeDraw(int pipe, uint stage)
{
    pfPipe *p = pfGetPipe(pipe);
    int x, y;
    
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Initializing stage 0x%x of     pipe %d on screen %d, connection \"%s\"", stage,     pipe,pfGetPipeScreen(p),pfGetPipeWSConnectionName(p));
    
    pfGetPipeSize(p, &x, &y);
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Pipe %d size: %dx%d", pipe, x,y);
}
    
/*
 * OpenPipeWin() -- create a GL window: set up the
 *        window system, IRIS GL, and IRIS Performer. This
 *        procedure is executed for each window in the draw process 
 *        for that pfPipe.
 */
    
    
void
OpenXWin(pfPipeWindow *pw)
{
    /* -1 -> use default screen or that specified by shell DISPLAY variable */
    pfuGLXWindow     *win;
    pfPipe  *p;
    
    p = pfGetPWinPipe(pw);
    if (!(win = pfuGLXWinopen(p, pw, "TESTIN")))
        pfNotify(PFNFY_FATAL, PFNFY_RESOURCE,"OpenXPipeline: Could not create GLX Window.");
    
    win = win; /* suppress compiler warn */
    
    
}
    
static void
OpenPipeWin(pfPipeWindow *pw)
{
    pfPipe *p;
    pfLight *Sun;
    
    p = pfGetPWinPipe(pw);
    
    /* open the window on the specified screen. By default,
    * if a screen has not yet been set and we are in multipipe mode,
    * a window of pfPipeID i will now open on screen i
    */
    pfOpenPWin(pw);
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
	     "PipeWin of Pipe %d opened on screen %d",     
	     pfGetId(p),pfGetPipeScreen(p));
    
    /* create a light source in the "south-west" (QIII) */
    Sun = pfNewLight(NULL);
    pfLightPos(Sun, -0.3f, -0.3f, 1.0f, 0.0f);
}
    
static void
DrawChannel (pfChannel *channel, void *data)
{
    static int first = 1;
    
    if (first)
    {
        first = 0;
        pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Calligraphics: 0x%p", pfGetChanCurCallig(channel));
    }
    
    /* erase framebuffer and draw Earth-Sky model */
    pfClearChan(channel);
    
    /* invoke Performer draw-processing for this frame */
    pfDraw();
}
