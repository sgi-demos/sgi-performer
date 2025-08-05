/*
 * Copyright 1998, 1999, 2000, Silicon Graphics, Inc.
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

//
// clipdemo.C
// 

#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <math.h>
#include <signal.h> // for sigset for forked X event handler process 
#include <getopt.h> // for cmdline handler
#include <sys/types.h>
#include <sys/sysmp.h>
#include <sys/sysinfo.h>
#include <X11/keysym.h>

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#define MAIN_MODULE
// joystick dead space
#define DEAD .1

//#define CHAN_OVER

#include "clipdemo.h"
#include "keybd.h"
#include "pov.h"
#include "dbase.h"

#include "BgPub.h"

SharedData *Shared;
pov *vehicle;

//
// APP process variables

// write out scene upon read-in - uses pfDebugPrint 
static int WriteScene = 0;
static int WinType = PFPWIN_TYPE_X;
static int NoBorder = 1;
static int ForkedXInput = 0;
static int NumPipes = 1;
static int FlyBox = 0;
static int WalkMouse = 0;
static float V_FOV = 46.0f;
static float H_FOV = 57.0f;
static float dtime = 5.0f;
static float H_OFF[3] = { 0.0f, -53.3f, 53.3f };
static int Lockdown = 1;

char ProgName[PF_MAXSTRING];

//static pfLightModel *LModel;
static void CullChannel(pfChannel *, void *);
static void DrawChannel(pfChannel *, void *);
static void OpenPipeWin(pfPipeWindow *);
static void UpdateSimulation(void);

extern pfMaterial *FindMaterial(pfNode *);

//
//	Usage() -- print usage advice and exit. This procedure
//	is executed in the application process.

static void Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, 
	     "%s -m -M -f -d -D -G -h <hfof> -v <vfof> -o <0off,1off,2off>\n"
             "   -m multipipe (assumes 3)\n"
             "   -M Mouse Fly, not Drive\n"
             "   -f flybox input\n"
             "   -G Enable 'G' key to sample 60 frames to stats file\n"
             "   -L disable CPU lockdown\n"
             "   -h <hfof> horizontal field of view\n"
             "   -v <vfof> vertical field of view\n"
             "   -o <0off,1off,2off> pipe horizontal offsets, eg 0,-50,50\n"
             , ProgName);

    exit(1);
}

//
//	docmdline() -- use getopt to get command-line arguments, 
//	executed at the start of the application process.


static int
docmdline(int argc, char *argv[])
{
    int	    opt;
    
    strcpy(ProgName, argv[0]);
    
    // process command-line arguments 
    while ((opt = getopt(argc, argv, "mMfFxbGLh:v:t:o:")) != -1)
    {
	switch (opt)
	{
	case 'm':
	    NumPipes = 3;
	    break;
	case 'M':
	    WalkMouse = 0;
	    break;
	case 'f':
	    FlyBox = 1;
	    break;
	case 'F':
	    ForkedXInput = 1;
	    break;
	case 'x':
	    WinType &= ~(PFPWIN_TYPE_X);
	    break;
	case 'b':
	    NoBorder ^= 1;
	    break;
	case 'L':
	    Lockdown = 0;
	    break;
	case 'h':
	    sscanf(optarg, "%f", &H_FOV);
	    break;
	case 'v':
	    sscanf(optarg, "%f", &V_FOV);
	    break;
	case 't':
	    sscanf(optarg, "%f", &dtime);
	    break;
	case 'o':
	    sscanf(optarg, "%f,%f,%f", &H_OFF[0], &H_OFF[1], &H_OFF[2]);
	    break;
	case '?':
	    Usage();
	}
    }
    return optind;
}


//
//	main() -- program entry point. this procedure
//	is executed in the application process.


int
main (int argc, char *argv[])
{
    int		    arg, i;
    int		    found;
    pfWSConnection  dsp=NULL;

    arg = docmdline(argc, argv);
    
    pfInit();
    pfMatrix *matey = new pfMatrix;
    
    // configure multi-process selection 
    pfMultiprocess(PFMP_APP_CULL_DRAW);
    pfMultipipe(NumPipes);
    
    // allocate shared before fork()'ing parallel processes 
    Shared = (SharedData*)pfMalloc(sizeof(SharedData), pfGetSharedArena());
    Shared->inWindow = 0;
    Shared->reset = 1;
    Shared->mouseX = 640;
    Shared->mouseY = 512;
    Shared->winSizeX = WIN_X;
    Shared->winSizeY = WIN_Y;
    Shared->exitFlag = 0;
    Shared->drawStats = 0;
    Shared->XInputInited = 0;
    Shared->texlist = NULL;
    Shared->sampleStats = 0;
    
    // Load all loader DSO's before pfConfig() forks 
    for (found = arg; found < argc; found++)
	pfdInitConverter(argv[found]);

    // initiate multi-processing mode set in pfMultiprocess call 
    // FORKs for Performer processes,  CULL and DRAW, etc. happen here.
    
    if(Lockdown)
      pfuLockDownApp();

    pfConfig();

    Shared->scene = new pfScene();
    //
    // make and add clip textured terrain
    //

    // specify directories where geometry and textures exist 
    if (!(getenv("PFPATH")))
        pfFilePath(
                   "DB"
                   );
    pfNotify(PFNFY_INFO, PFNFY_PRINT,"FilePath: %s\n", pfGetFilePath());

    pfGroup *terrain = NULL;
    // load files named by command line arguments 
    for (found = 0; arg < argc; arg++)
    {
        pfGroup *terrain = MakeClipDbase(argv[arg], dtime);
	if (terrain!= NULL)
	{
            cout << "Cliptexture was given a download time of " << dtime << "ms" << endl;
            Shared->scene->addChild(terrain);
	    found++;
	}
        else
        {
          pfNotify(PFNFY_WARN, PFNFY_PRINT,"NULL return from MakeClipDbase\n");
          pfuFreeCPUs();
          pfExit();
        }
    }

    if (found == 0)
    {
      pfNotify(PFNFY_FATAL, PFNFY_PRINT, "no clip texture found from command line input\n");
      pfuFreeCPUs();
      pfExit();
    }

    if (found > 1)
    {
      pfNotify(PFNFY_FATAL, PFNFY_PRINT,"more than one file argument found on command line\n");
      pfuFreeCPUs();
      pfExit();
    }

    if(FlyBox)
      InitFLYBOX();

//char *namestring[4] = { ":0.0", ":0.1", ":0.2", ":0.2");
    for(i = 0; i < NumPipes; i++)
    {
      pfuAddMPClipTextureToPipes(Shared->mpcliptex, pfGetPipe(i), NULL);

      // need to add a lot of this crap to get the visual I need,
      // many visuals blow away AA when blend is subsequently performed
      int FBattrs[] = { PFFB_RGBA, GLX_DOUBLEBUFFER,
                        GLX_SAMPLE_BUFFERS_SGIS, 1,
                        //GLX_BUFFER_SIZE, 30,
                        GLX_SAMPLES_SGIS, 4,
                        PFFB_DEPTH_SIZE, 1,
                        //PFFB_STENCIL_SIZE, 8,
                        //PFFB_STEREO,
                        PFFB_RED_SIZE, 10,
                        PFFB_GREEN_SIZE, 10,
                        PFFB_BLUE_SIZE, 10,
                        PFFB_ALPHA_SIZE, 0,
                        //GLX_ACCUM_RED_SIZE, 25,
                        //GLX_ACCUM_GREEN_SIZE, 25,
                        //GLX_ACCUM_BLUE_SIZE, 25,
                        //GLX_ACCUM_ALPHA_SIZE, 0,
                        None };

      // configure pipes and windows 
      Shared->p[i] = pfGetPipe(i);
      Shared->pw[i] = new pfPipeWindow(Shared->p[i]);
      Shared->p[i]->setScreen(i);
      //Shared->pw[i].setWSConnectionName(namestring[i]);
      Shared->pw[i]->setName("OpenGL Performer");
      Shared->pw[i]->setWinType(WinType);
      if (NoBorder)
        Shared->pw[i]->setMode(PFWIN_NOBORDER, 1);
      // Open and configure the GL window. 
      Shared->pw[i]->setConfigFunc(OpenPipeWin);

      //Shared->pw[i]->setFBConfigId(0x73); // iR 4samp rgb10 1 sten
      //Shared->pw[i]->setFBConfigId(0x9d); // iR 8samp rgb10 1 sten
      Shared->pw[i]->setFBConfigAttrs(FBattrs);
      Shared->pw[i]->config();

      //Shared->pw[i]->setFullScreen();
      //Shared->pw[i]->setOriginSize(0, 0, WIN_X, WIN_Y);
      Shared->pw[i]->setOriginSize(0, 0, 450, 350);
    }
    
    // create forked XInput handling process 
    // since the Shared pointer has already been initialized, that structure
    // will be visible to the XInput process. Nothing else created in the
    // application after this fork whose handles are not put in shared memory
    // (such as the database and channels) will be visible to the
    // XInput process.
    
    if (WinType & PFPWIN_TYPE_X)
    {
	pid_t	    fpid = 0;
	if (ForkedXInput)
	{
	    if ((fpid = fork()) < 0)
		pfNotify(PFNFY_FATAL, PFNFY_SYSERR, "Fork of XInput process failed.");
	    else if (fpid)
		pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"XInput running in forked process %d",
			 fpid);
	    else if (!fpid)
		DoXInput();
	}
	else
	{
	    dsp = pfGetCurWSConnection();
	}
    }

    vehicle = new pov();

    vehicle->accelerate(0.0f);
    
    // Write out nodes in scene (for debugging) 
    if (WriteScene)
    {
	FILE *fp;
	if (fp = fopen("scene.out", "w"))
	{
	    pfPrint(Shared->scene, PFTRAV_SELF|PFTRAV_DESCEND, PFPRINT_VB_DEBUG, fp);
	    fclose(fp);
	}
	else
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		     "Could not open scene.out for debug printing.");
    }
    
    pfFrameRate(60.0f);
    pfPhase(PFPHASE_LIMIT);

    Shared->texlist = pfuMakeSceneTexList(Shared->scene);


    // Create an earth/sky model that draws sky/ground/horizon 
    pfEarthSky *eSky = new pfEarthSky();
    eSky->setColor(PFES_CLEAR, 0.0f, 0.0f, 0.0f, 1.0f);
    eSky->setMode(PFES_BUFFER_CLEAR, PFES_FAST);

    pfChannel *chan_main[4];

    for(i = 0; i < NumPipes; i++)
    {
      pfVec3 cpos, chpr;

      cpos.set(0.0f, 0.0f, 0.0f);
      chpr.set(H_OFF[i], 0.0f, 0.0f);

      chan_main[i] = new pfChannel(Shared->p[i]);

      Shared->pw[i]->addChan(chan_main[i]);

      chan_main[i]->setViewport(0.0f, 1.0f, 0.0f, 1.0f);
      chan_main[i]->setTravFunc(PFTRAV_CULL, CullChannel);
      chan_main[i]->setTravFunc(PFTRAV_DRAW, DrawChannel);
      chan_main[i]->setScene(Shared->scene);
      chan_main[i]->setNearFar(0.0001f, 100000.0f);
      chan_main[i]->setESky(eSky);
      chan_main[i]->setTravMode(PFTRAV_CULL, PFCULL_VIEW|PFCULL_GSET);
      chan_main[i]->setFOV(H_FOV, V_FOV);
      chan_main[i]->setViewOffsets(cpos, chpr);

    }

    // just put a value in here for draw callback to see
    chan_main[0]->allocChanData(sizeof(int));
    chan_main[0]->passChanData();
#ifdef CHAN_OVER
    //InitEnvmapChannels();
#endif

    // Set initial view to be "in front" of scene 

    // view point at 0.0f, 0.0f
    Shared->view.xyz.set(0.0f, 0.0f, 0.0f);

    //chan_main[i]ZZ->setView(Shared->view.xyz, Shared->view.hpr);
    PFCOPY_VEC3(Shared->viewOrig.xyz, Shared->view.xyz);
    PFCOPY_VEC3(Shared->viewOrig.hpr, Shared->view.hpr);

    // main simulation loop
    while (!Shared->exitFlag)
    {
      static int count = 0;

      if(count < 50) count++;

      if(count == 5)
      {
        Shared->reset = 1;
      }

        if(Shared->elevels)
        {
            if(Shared->elevels > 0)
                incElevels();
            else
                decElevels();

            Shared->elevels = 0;
        }
        if(Shared->offset)
        {
            if(Shared->offset > 0)
                incOffset();
            else
                decOffset();

            Shared->offset = 0;
        }
	// Set view parameters for next frame 
	UpdateSimulation();
//Shared->view.xyz[0]
        matey->makeCoord(&(Shared->view));
        UpdateClipCenter(Shared->view.xyz[0], Shared->view.xyz[1]);
        for(i = 0; i < NumPipes; i++)
        {
	  chan_main[i]->setViewMat(*matey);
        }

	if (!ForkedXInput)
	{
	    if (!Shared->XInputInited)
		InitXInput(dsp);
	    if (Shared->XInputInited)
		GetXInput(dsp);
	}

	// wait until next frame boundary
	pfSync();

	pfFrame();
    }

    pfuFreeCPUs();

    // terminate cull and draw processes (if they exist) 
    pfExit();
    
#if 0
    // do free up manually
    int proccys = sysmp(MP_NPROCS);
    for(i = 0; i < proccys; i++)
    {
      sysmp(MP_EMPOWER, i);
      sysmp(MP_UNISOLATE, i);
      sysmp(MP_PREEMPTIVE, i);
    }
#endif

    if(FlyBox)
      ExitFLYBOX();

    // exit to operating system 
    return 0;
}


// 
//	UpdateSimulation() updates the eyepoint & other things
//      based on the information placed in shared memory by GetInput().

static void    
UpdateSimulation(void)
{
    static double thisTime = -1.0f;
    double prevTime;
    float deltaTime;
    float StickX, StickY, StickT, AC1, AC2;

    prevTime = thisTime;
    thisTime = pfGetTime();

    if (prevTime < 0.0f)
	return;

    if (/*!Shared->inWindow || */ Shared->reset)
    {
	//pfVec3 position(.0425f, .755f, 0.001f);
	pfVec3 position(22282.24f, 395837.44f, 5000.0f);
	Shared->reset = 0;
        PFCOPY_VEC3(Shared->view.xyz, Shared->viewOrig.xyz);
        PFCOPY_VEC3(Shared->view.hpr, Shared->viewOrig.hpr);
	vehicle->reset(position);
        Shared->view_mode = SUB_VIEW_POV;
    }

    deltaTime = thisTime - prevTime;

    switch (Shared->mouseButtons)
    {
    case Button1Mask: /* LEFTMOUSE: accelerate */
	  vehicle->accelerate(51.44444444f); // 100 knot increments
	break;
    case Button2Mask: /* UNUSED ! */
	break;
    case Button3Mask: /* RIGHTMOUSE: decelerate*/
	  vehicle->accelerate(-51.44444444f);
	break;
    }

    if(FlyBox)
    {
      ReadFLYBOX(&StickX, &StickY, &StickT, &AC1, &AC2);
      if(StickX > 0.0f)
      {
        if(StickX < DEAD)
          StickX = 0.0f;
        else
          StickX -= DEAD;
      }
      else
      {
        if(StickX > -DEAD)
          StickX = 0.0f;
        else
          StickX += DEAD;
      }
      if(StickY > 0.0f)
      {
        if(StickY < DEAD)
          StickY = 0.0f;
        else
          StickY -= DEAD;
      }
      else
      {
        if(StickY > -DEAD)
          StickY = 0.0f;
        else
          StickY += DEAD;
      }
      if(StickT > 0.0f)
      {
        if(StickT < DEAD)
          StickT = 0.0f;
        else
          StickT -= DEAD;
      }
      else
      {
        if(StickT > -DEAD)
          StickT = 0.0f;
        else
          StickT += DEAD;
      }
      if(AC1 > 0.0f)
      {
        if(AC1 < DEAD)
          AC1 = 0.0f;
        else
          AC1 -= DEAD;
      }
      else
      {
        if(AC1 > -DEAD)
          AC1 = 0.0f;
        else
          AC1 += DEAD;
      }
      if(AC2 > 0.0f)
      {
        if(AC2 < DEAD)
          AC2 = 0.0f;
        else
          AC2 -= DEAD;
      }
      else
      {
        if(AC2 > -DEAD)
          AC2 = 0.0f;
        else
          AC2 += DEAD;
      }

      vehicle->simulate(StickX, StickY, StickT, deltaTime);
      vehicle->accelerate(AC2);
      vehicle->speedlimit(AC2);

    }
    else
    {
      if(WalkMouse)
      {
        vehicle->simulate(0.0f, 0.0f,
                 2.0f * (Shared->mouseX / (float)Shared->winSizeX) - 1.0f,
                 deltaTime);
      }
      else
      {
        vehicle->simulate(2.0f * (Shared->mouseX / (float)Shared->winSizeX) - 1.0f,
                 2.0f * (Shared->mouseY / (float)Shared->winSizeY) - 1.0f, 0.0f,
                 deltaTime);
      }

    }

    if(Shared->halt)
    {
      Shared->halt = 0;
      vehicle->speedlimit(-0.00000001f);
      vehicle->speedlimit(0.00000001f);
    }

    vehicle->posview(&Shared->view, Shared->view_mode);
}

//
//	CullChannel() -- traverse the scene graph and generate a
// 	display list for the draw process.  This procedure is 
//	executed in the CULL process.


static void
CullChannel(pfChannel *chan, void *)
{
  static int locked = 0;

  if(!locked)
  {
    locked = 1; 
    if(Lockdown)
      pfuLockDownCull(chan->getPipe());
  }
  
  pfCull();
}

//
//	OpenPipeWin() -- create a win: setup the GL and OpenGL Performer.
//	This procedure is executed in the DRAW process 
//	(when there is a separate draw process).


static void
OpenPipeWin(pfPipeWindow *pw)
{
    pw->open();

    // CPU lockdown
    if(Lockdown)
      pfuLockDownDraw(pw->getPipe());

    if(Shared->texlist)
      pfuDownloadTexList(Shared->texlist, PFUTEX_SHOW);

    // create and modify light model, enabling localviewer calcs
    //LModel = new pfLightModel();
    //LModel->setAmbient(0.0f, 0.0f, 0.0f);
    //LModel->setLocal(PF_ON);
    //LModel->apply();

}


//
//	DrawChannel() -- draw a channel and read input queue. this
//	procedure is executed in the draw process (when there is a
//	separate draw process).

static void
DrawChannel (pfChannel *channel, void *data)
{
//    double drawtime;
//    drawtime = pfGetTime();

    pfPipeWindow *pw = channel->getPWin();

    // No channel clear, the reflection has done this
    // probably need to do something different in future
    // but will just move clip planes or something to
    // avoid overhead for depth clear time
    channel->clear();

    // invoke Performer draw-processing for this frame 
    pfDraw();

    // draw Performer throughput statistics 
    
    if (Shared->drawStats)
	channel->drawStats();
    
    // read window origin and size (it may have changed) 
    channel->getPWin()->getSize(&Shared->winSizeX, &Shared->winSizeY);

//    glFinish();
//    drawtime = pfGetTime() - drawtime;
//    cout << "time " << drawtime << endl;
}
