//
// Copyright 1997, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
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
// simpleASD.C
// --------------
//
//  Simple Performer demo of basic ASD functionality
//
//  simpleASD [-T]
//
//	-T: Use texturing
//
//
//  $Revision: 1.22 $ 
//  $Date: 2000/10/06 21:26:26 $
//
//

/******************************************************************************
*				Includes
******************************************************************************
*/

// general includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream.h>

// proecess control includes
#include <sys/types.h>
#include <sys/prctl.h>
#ifndef __linux__
#include <sys/sysmp.h>
#include <sys/schedctl.h>
#endif

// X Window includes
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

// GL includes


// Performer includes
#include <Performer/pf.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pr/pfMemory.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pr/pfHighlight.h>

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <Performer/pfui/pfiXformer.h>
#include <Performer/pfui.h>
#include <Performer/pfui/pfiPick.h>

#include <Performer/pf/pfASD.h>

// ---------------------------------------------------- //
//		Defines and Typedefs                    //
// ---------------------------------------------------- //

// Panel Widget Control Tokens
enum PanelControlToken {
    CTL_RESET_ALL=1,
    CTL_REPOSITION,
    CTL_QUIT,
    CTL_MODEL,
    CTL_DRAWSTYLE,
    CTL_EVAL_METHOD,  
    CTL_DIAGS,
    CTL_TREE,
    CTL_LIGHTING,
    CTL_DRAW_NORMS,    
    CTL_ASD_DIST,
    CTL_MORPH_BUFFER, 
    CTL_MORPH_COORDS, 
    CTL_MORPH_NORMS, 
    CTL_MORPH_COLORS     
    };

    // --- Structs for ASD attribute data --- //
typedef struct normalAttr
{
    pfVec3 normal;
    pfVec3 normalDiff;	    // The difference between the vector that is normal to the 
			    // vertex and the vector that is normal to the reference point
} normalAttr;

typedef struct colorAttr
{
    pfVec4 color;
    pfVec4 colorDiff;	    // The difference between the color to the 
			    // color of the reference point
} colorAttr;

typedef struct ASDAttr
{
    normalAttr normal;
    colorAttr color;  
} ASDattr;

// SharedData:
// structure that resides in shared memory so that the
// APP, CULL, and DRAW processes can access it.
//
typedef struct SharedData
{
	// --- main channel stuff --- //
    pfChannel		*chan;
    int			drawStats;	// Should I draw stats
    int			exitFlag;	// Set when it is time to exit
    pfLightSource	*sun;		// sun light source
    int			drawTree;	// Should I draw the tree
    int			drawStyle;	// What special draw style are we using
    
	// --- Input handling structures --- //
    int			input;		// PFUINPUT_GL or PFUINPUT_X
    pfuMouse		mouse;		// mouse structure
    pfuEventStream	events;		// event structure
    
	// --- flight model and collision detection sructures --- //
    pfScene		*scene;
    pfGeoState*		globalSceneState;   // Global Scene state
    pfDCS		*sceneDCS;	// DCS for trackball interface
    pfCoord		viewCoord;	// current view position, direction
    pfCoord		initView;	// initial view position, direction
    float		near;		// near clipping plane
    float		far;		// far clipping plane
    pfiXformer		*xformer;	// interactive movement model
    int			xformerModel;	// Fly, Drive, or Trackball
    int			collideMode;	// collision & terrain following flag
    
	// --- GUI Flags --- //
    int			updateChannels;
    int			gui, redraw, guiDirty;
    
	// --- handles to the actual GUI objects --- //
    pfuPanel		*guiPanel;
    pfuWidget		*wResetAll, *wKillGUI, *wPosXYZ, *wStats, *wDrawStyle, *wTree, *wLighting, 
			*wFModel, *wEvalMethod, *wASDDist, *wMorphBuffer, *wMorphCoords, *wMorphNorms,
			*wDrawNorms, *wMorphColors;
    
	// --- ASD Data Structure --- //
    pfASD*	theASDNode;	    // Our ASD node
    int		geometryType;	    // Use orignal or textured geometry
    pfGeoState** geoStateArray;	    // An array of geo states that the ASD uses
    pfASDVert*	verts;		    // List of vertices for the ASD
    pfASDFace*	faces;		    // List of the faces used by the ASD
    pfASDLODRange* lods;		    // The LOD list for the ASD
    
//    normalAttr* normalAttrs;	    // ASD attributes for the normals
//    colorAttr* colorAttrs;	    // ASD attributes for the colors
    ASDAttr*	attrs;		    // All the ASD attributes
    
	// --- Normals stuff --- //
    pfHighlight*    normalHighlight;	// A highlight mode to draw normals	
    int		    drawNormals;		// Are we supposed to be drawing normals
    
    pfVec4  baseColor;		    // The "regular" color of the ASD object
    pfVec4  morphedColor;	    // The color that the morphed vertices are set to
	
} SharedData;



/******************************************************************************
*				Static Declarations
******************************************************************************
*/

// shared memory handle
static SharedData *Shared=0;


// X control variables
static int XInput = PFUINPUT_X;

static int WinXOrigin = 100, WinYOrigin = 100;
static int WinXSize = 800, WinYSize = 600;

// process control variables
static int ProcSplit =  PFMP_APPCULLDRAW;


static void CullChannel(pfChannel *chan, void *data);
static void DrawChannel(pfChannel *chan, void *data);
static void OpenPipeWin(pfPipeWindow *pw);
static pfuPanel *InitPanel(void);
static void ProcessInputIO(void);
static void PanelControl(pfuWidget *w);
static void resetPosition(void);
static void KillGUI(pfuWidget *w);
static void InitXformer(void);
static void IsectFunc(void *data);
static void UpdateView(pfChannel *chan, pfScene *scene);
static void updateGUIViewMsg(pfCoord *viewpt);
static void xformerModel(int model, int collideMode);


    // -- ASD Creation Routine -- //
pfASD* createASDNode(int whichGeometry);
    const int OrigGeom = 0;
    const int TexGeom = 1;
    
void buildGState(pfGeoState *gs);
void buildTexGState(pfGeoState *gs, char* texname);
pfMaterial* defaultMaterial();
void switchASDLighting();
void switchMorphAttrs();

void setNewSwitchingValue(float newValue);
void setMorphBufferValue(float newValue);

pfHighlight* createNormalHighlight();

//
//	Main and Commandline Processing
//

char ProgName[PF_MAXSTRING];
char OptionStr[] = "Tt?";


//
//	Usage() -- print usage advice and exit. This procedure
//	is executed in the application process.


static void
Usage (void)
{
    fprintf(stderr, 
	    "Usage: %s [-T]\n", ProgName);
    fprintf(stderr, "\t-T: Uses texture geometry.");	    
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
    while ((opt = getopt(argc, argv, OptionStr)) != -1)
    {
	switch (opt)
	{
	case 't': // Use Texturing
	case 'T': 
	    Shared->geometryType = TexGeom;
	    break;
	case '?': // get usage 
	    Usage();
	}
    }
    return optind;
}


//
//	main() -- program entry point. this procedure
//	is executed in the application process.
//

int
main (int argc, char *argv[])
{
    pfNotifyLevel(PFNFY_DEBUG);	    // Set Error handling level

    pfInit();
    pfuInit();
    pfiInit();

    pfFilePath(".:/usr/share/Performer/data");

 
    pfMultiprocess(ProcSplit);
    
 
	// allocate shared before fork()'ing parallel processes
	// so that all processes have will the correct Shared address
	//
    Shared = (SharedData*)pfCalloc(1, sizeof(SharedData), pfGetSharedArena());
    
	// init shared memory structures
    Shared->xformerModel	= PFITDF_TRACKBALL;
    Shared->input		= XInput;
    Shared->collideMode		= PF_ON;
    Shared->gui			= 1; // enable gui
    Shared->drawTree		= 0;	// Don't start drawing the tree
    Shared->updateChannels	= 1;
    Shared->guiDirty = Shared->redraw = 1;
    Shared->near = 0.01f;
    Shared->far = 1000.0f;
    Shared->geometryType = OrigGeom;	// Set to a default
    
//    Shared->baseColor.set(0.15f, 0.8f, 0.87f, 0.0f);
//    Shared->morphedColor.set(0.87f, 0.0f, 0.0f, 0.0f);
    Shared->baseColor.set(0.0f, 0.0f, 0.9f, 0.0f);
    Shared->morphedColor.set(0.0f, 0.9f, 0.0f, 0.0f);
//    Shared->baseColor.set(1.0f, 1.0f, 1.0f, 1.0f);
//    Shared->morphedColor.set(1.0f, 0.0f, 0.0f, 1.0f);

    
    	// ----- parse commandline ----- //
    docmdline(argc, argv);


	// initiate multi-processing mode set in pfMultiprocess call
	// FORKS HAPPEN HERE !!!
    pfConfig();

    
    // Initialize input structure (X or GL) for mouse and event inputs
    // then Open and configure full screen GL or GLX window.
    //
    
    // Configure window for the pipe
    pfPipe *p = pfGetPipe(0);
    pfPipeWindow *pw = new pfPipeWindow(p);
    pw->setName("simpleASD");
    pw->setOriginSize(WinXOrigin, WinYOrigin,
		      WinXOrigin +  WinXSize, WinYOrigin +  WinYSize);
    if (Shared->input == PFUINPUT_X)
	pw->setWinType(PFPWIN_TYPE_X);
    
    pw->setConfigFunc(OpenPipeWin);
    pw->config();
    
    pfFrame();
    
    pfuInitGUI(pw); 
    pfuInitInput(pw, Shared->input);
    
    //
    // App Process Setup
    //
    
    pfScene *scene = new pfScene;	    // Declare pfScene Object
    Shared->scene = scene;
    Shared->sceneDCS = new pfDCS;	    // Declare a sceneDCS
    scene->addChild(Shared->sceneDCS);	    // Add the DCS to the top of tree
    
	// --- Add the ASD Data to the scene --- //
    Shared->theASDNode = createASDNode(Shared->geometryType);      // Create and return the ASD
    Shared->sceneDCS->addChild(Shared->theASDNode);

    Shared->normalHighlight = createNormalHighlight();
    pfuTravNodeHlight(Shared->scene, Shared->normalHighlight); 
	    
    // Set intersection callback.
    pfIsectFunc(IsectFunc);
    // do intersection setup
    pfuCollideSetup(scene, PFUCOLLIDE_STATIC, 0xffffffff);
    
    
    // Create pfChannels and assign draw callback functions.
    // Channels will automatically be assigned to the first created window
    // on the pfPipe.
    //
    
    pfChannel *chan = new pfChannel(p);
    Shared->chan = chan;
    
    chan->setTravFunc(PFTRAV_CULL, CullChannel);
    chan->setTravFunc(PFTRAV_DRAW, DrawChannel);
    chan->setScene(scene);
    chan->setNearFar(Shared->near, Shared->far);
    
	// vertical FOV is matched to window aspect ratio
    chan->setFOV(45.0f, -1.0f);
    
    // Initialize sun - create a light source in the "south-west" (QIII)
    Shared->sun = new pfLightSource;
    Shared->sun->setPos(-0.3f, -0.3f, 1.0f, 0.0f);
    Shared->scene->addChild(Shared->sun);
    
    pfFrameStats *stats = chan->getFStats();
    stats->setClass(PFSTATS_ENGFX, PF_ON);
    
    
	// --- Just set up a very basic view --- //
    pfVec3 zeros(0.0f, 0.0f, 0.0f);
    chan->setView(zeros, zeros);
    
       
    Shared->chan->setViewport(0.0f, 1.0f, 0.25f, 1.0f);
    pfuGUIViewport(0.0f, 1.0f, 0.0f, 0.25f);
    // init the control panel for perfly
    Shared->guiPanel = InitPanel();
    pfuEnablePanel(Shared->guiPanel);
    
    // Initialize channel viewing model for interactive motion
    InitXformer();
    
    pfEnable(PFEN_TEXTURE);
    
    // main simulation loop
    while (!Shared->exitFlag)
    {
	    // wait until next frame boundary
	pfSync();
	
	    //
	    // All latency critical processing should be done here. This is
	    // typically the viewing position.
	    // One should also read latency critical devices here.
	    //
	pfuGetMouse(&Shared->mouse);	    // Copy data into the mouse structure
	
	UpdateView(Shared->chan, Shared->scene);
	
        if (Shared->drawNormals)
	    pfuTravNodeHlight(Shared->scene, Shared->normalHighlight); 
	else
	    pfuTravNodeHlight(Shared->scene, NULL);
	    
	    // initiate traversal using current state
	pfFrame();
	
	    // Get snapshot of event/key queues
	pfuGetEvents(&Shared->events);
	ProcessInputIO();
	
	if (Shared->updateChannels)
	{
	    if (Shared->gui)
	    {
		float	foo, top;
		
		Shared->redraw = Shared->guiDirty = 1;
		pfuEnableGUI(TRUE);
		pfuGetGUIViewport(&foo, &foo, &foo, &top);
		Shared->chan->setViewport(0.0f, 1.0f, top, 1.0f);
	    }
	    else
	    {
		pfuEnableGUI(FALSE);
		Shared->chan->setViewport(0.0f, 1.0f, 0.0f, 1.0f);
	    }
	    Shared->guiDirty = Shared->updateChannels = 0;
	} else if (Shared->gui)
	{
	    updateGUIViewMsg(&(Shared->viewCoord));
	    pfuUpdateGUI(&(Shared->mouse));
	}
    }
    
    // --- EXIT THE PROGRAM --- //
    
	// exit GUI and print avg time to draw GUI - optional
    pfuExitGUI();
	// Exit from libpfutil and remove shared mem arenas
    pfuExitUtil();
    pfuExitInput();
    
	// terminate cull and draw processes (if they exist)
    pfExit();
    
	// exit to operating system
    return 0;
}

    // v0 is vertex, v1 and v2 are the other points on the triangle
pfVec3 vertNormal(pfVec3 v0, pfVec3 v1, pfVec3 v2)
{
    pfVec3 retval;
    retval.cross((v1-v0), (v2-v0));
    retval.normalize();
    return retval;
}

//-----------------------------------------------------------------
// createASDNode
//	
// PURPOSE:  Creates the ASD node and the Terrain Data structure
//	whichGeometry ==> 0  - Regular simple geometry (OrigGeom)
//		      ==> 1  - Geometry designed to show texture capabilities (TexGeom)
//-----------------------------------------------------------------
pfASD* createASDNode(int whichGeometry)
{
    const int numVerts = 9;
    const int numFaces = 10;
    pfASDVert* verts;	// A pfASDVert array. Contains all of the verts for the terrain
    
    verts = (pfASDVert*)pfCalloc(numVerts, sizeof(pfASDVert), pfGetSharedArena());
    Shared->verts = verts;
    
	// ------- Fill the verts array ------- //
    pfVec3 V1, V2, V3, V4, N1, N2, N3, N4, N5;
    
    V1.set(-2.0f, -1.0f, 0.0f); V2.set(-1.0f, 1.0f, 0.0f); V3.set(1.0f, -1.0f, 0.0f); V4.set(2.0f, 1.0f, 0.0f); 
    pfVec3 R1((V1+V2)/2), R2((V2+V3)/2), R3((V3+V1)/2), 
	       R4((V2+V4)/2), R5((V4+V3)/2);
	       	
    if(whichGeometry == OrigGeom) {
	N1.set(-3.0f, 0.0f, 1.0f); N2.set(0.0f, 0.0f, 1.0f); N3.set(-1.0f, -2.0f, 1.0f); 
	N4.set(1.0f, 2.0f, 1.0f); N5.set(3.0f, 0.0f, 1.0f);
    } else {
	N1 = R1; N1[0] -= 0.5f; N1[1] += 0.5f; N1[2] += 0.5f;
	N2 = R2; N2[0] -= 0.25f; N2[1] -= 0.25f; N2[2] -= 0.20f;
	N3 = R3; N3[0] -= 0.5f; N3[1] -= 0.5f; N3[2] += 0.5f;
	N4 = R4; N4[0] -= 0.0f; N4[1] += 0.1f; N4[2] += 0.0f;
	N5 = R5; N5[0] -= 0.0f; N5[1] -= 0.1f; N5[2] += 0.0f;
    }   
    		

    	   
	// Vertexes are entered in the array V1-V4,N1-N5
      // V1
    verts[0].v0 = V1;
    verts[0].vd.set(0.0f, 0.0f, 0.0f);	    // Set base to have zero diffs.
    verts[0].neighborid[0] = PFASD_NIL_ID;
    verts[0].neighborid[1] = PFASD_NIL_ID;
      // V2
    verts[1].v0 = V2;
    verts[1].vd.set(0.0f, 0.0f, 0.0f);
    verts[1].neighborid[0] = PFASD_NIL_ID;
    verts[1].neighborid[1] = PFASD_NIL_ID;
      // V3
    verts[2].v0 = V3;
    verts[2].vd.set(0.0f, 0.0f, 0.0f);
    verts[2].neighborid[0] = PFASD_NIL_ID;
    verts[2].neighborid[1] = PFASD_NIL_ID;
      // V4
    verts[3].v0 = V4;
    verts[3].vd.set(0.0f, 0.0f, 0.0f);
    verts[3].neighborid[0] = PFASD_NIL_ID;
    verts[3].neighborid[1] = PFASD_NIL_ID;
      // N1
    verts[4].v0 = N1;
    verts[4].vd.copy(R1-N1);
    verts[4].neighborid[0] = 0;
    verts[4].neighborid[1] = PFASD_NIL_ID;
      // N2
    verts[5].v0 = N2;
    verts[5].vd.copy(R2-N2);
    verts[5].neighborid[0] = 0;
    verts[5].neighborid[1] = 1;
      // N3
    verts[6].v0 = N3;
    verts[6].vd.copy(R3-N3);
    verts[6].neighborid[0] = 0;
    verts[6].neighborid[1] = PFASD_NIL_ID;
      // N4
    verts[7].v0 = N4;
    verts[7].vd.copy(R4-N4);
    verts[7].neighborid[0] = 1;
    verts[7].neighborid[1] = PFASD_NIL_ID;
      // N5
    verts[8].v0 = N5;
    verts[8].vd.copy(R5-N5);
    verts[8].neighborid[0] = 1;
    verts[8].neighborid[1] = PFASD_NIL_ID;
    
    
	
    
    
    pfASDFace* faces;	// A pfASDFae array. Contains all of the pfFaces for the terrain
    
    faces = (pfASDFace*)pfCalloc(numFaces, sizeof(pfASDFace), pfGetSharedArena());
    Shared->faces = faces;
	// ------- Fill the faces array ------- //
	    // --- LEVEL 0 --- //
	// 0
    faces[0].level = 0;
    faces[0].tsid = 0;
    faces[0].vert[0] = 0;  // V1
    faces[0].vert[1] = 1;  // V2
    faces[0].vert[2] = 2;  // V3
    faces[0].refvert[0] = 4; // N1
    faces[0].refvert[1] = 5; // N2
    faces[0].refvert[2] = 6; // N3
    faces[0].child[0] = 2;
    faces[0].child[1] = 3;
    faces[0].child[2] = 4;
    faces[0].child[3] = 5;
	// 1
    faces[1].level = 0;
    faces[1].tsid = 0;
    faces[1].vert[0] = 1;  // V2
    faces[1].vert[1] = 3;  // V4
    faces[1].vert[2] = 2;  // V3
    faces[1].refvert[0] = 7; // N4
    faces[1].refvert[1] = 8; // N5
    faces[1].refvert[2] = 5; // N2
    faces[1].child[0] = 6;
    faces[1].child[1] = 7;
    faces[1].child[2] = 8;
    faces[1].child[3] = 9;
    
    if(whichGeometry == OrigGeom) {
    
		// --- LEVEL 1 --- //
	    // 2
	faces[2].level = 1;
	faces[2].tsid = 0;
	faces[2].vert[0] = 0;  // V1
	faces[2].vert[1] = 4;  // N1
	faces[2].vert[2] = 6;  // N3
	faces[2].refvert[0] = PFASD_NIL_ID;
	faces[2].refvert[1] = PFASD_NIL_ID;
	faces[2].refvert[2] = PFASD_NIL_ID;
	faces[2].child[0] = PFASD_NIL_ID;
	faces[2].child[1] = PFASD_NIL_ID;
	faces[2].child[2] = PFASD_NIL_ID;
	faces[2].child[3] = PFASD_NIL_ID;
	    // 3
	faces[3].level = 1;
	faces[3].tsid = 0;
	faces[3].vert[0] = 4;  // N1
	faces[3].vert[1] = 1;  // V2
	faces[3].vert[2] = 5;  // N2
	faces[3].refvert[0] = PFASD_NIL_ID;
	faces[3].refvert[1] = PFASD_NIL_ID;
	faces[3].refvert[2] = PFASD_NIL_ID;
	faces[3].child[0] = PFASD_NIL_ID;
	faces[3].child[1] = PFASD_NIL_ID;
	faces[3].child[2] = PFASD_NIL_ID;
	faces[3].child[3] = PFASD_NIL_ID;
	    // 4
	faces[4].level = 1;
	faces[4].tsid = 0;
	faces[4].vert[0] = 4;  // N1
	faces[4].vert[1] = 5;  // N2
	faces[4].vert[2] = 6;  // N3
	faces[4].refvert[0] = PFASD_NIL_ID;
	faces[4].refvert[1] = PFASD_NIL_ID;
	faces[4].refvert[2] = PFASD_NIL_ID;
	faces[4].child[0] = PFASD_NIL_ID;
	faces[4].child[1] = PFASD_NIL_ID;
	faces[4].child[2] = PFASD_NIL_ID;
	faces[4].child[3] = PFASD_NIL_ID;
	    // 5
	faces[5].level = 1;
	faces[5].tsid = 0;
	faces[5].vert[0] = 6;  // N3
	faces[5].vert[1] = 5;  // N2
	faces[5].vert[2] = 2;  // V3
	faces[5].refvert[0] = PFASD_NIL_ID;
	faces[5].refvert[1] = PFASD_NIL_ID;
	faces[5].refvert[2] = PFASD_NIL_ID;
	faces[5].child[0] = PFASD_NIL_ID;
	faces[5].child[1] = PFASD_NIL_ID;
	faces[5].child[2] = PFASD_NIL_ID;
	faces[5].child[3] = PFASD_NIL_ID;
	    // 6
	faces[6].level = 1;
	faces[6].tsid = 0;
	faces[6].vert[0] = 1;  // V2
	faces[6].vert[1] = 7;  // N4
	faces[6].vert[2] = 5;  // N2
	faces[6].refvert[0] = PFASD_NIL_ID;
	faces[6].refvert[1] = PFASD_NIL_ID;
	faces[6].refvert[2] = PFASD_NIL_ID;
	faces[6].child[0] = PFASD_NIL_ID;
	faces[6].child[1] = PFASD_NIL_ID;
	faces[6].child[2] = PFASD_NIL_ID;
	faces[6].child[3] = PFASD_NIL_ID;
	
	    // 7
	faces[7].level = 1;
	faces[7].tsid = 0;
	faces[7].vert[0] = 5;  // N2
	faces[7].vert[1] = 7;  // N4
	faces[7].vert[2] = 8;  // N5
	faces[7].refvert[0] = PFASD_NIL_ID;
	faces[7].refvert[1] = PFASD_NIL_ID;
	faces[7].refvert[2] = PFASD_NIL_ID;
	faces[7].child[0] = PFASD_NIL_ID;
	faces[7].child[1] = PFASD_NIL_ID;
	faces[7].child[2] = PFASD_NIL_ID;
	faces[7].child[3] = PFASD_NIL_ID;
	
	    // 8
	faces[8].level = 1;
	faces[8].tsid = 0;
	faces[8].vert[0] = 5;  // N2
	faces[8].vert[1] = 8;  // N5
	faces[8].vert[2] = 2;  // V3
	faces[8].refvert[0] = PFASD_NIL_ID;
	faces[8].refvert[1] = PFASD_NIL_ID;
	faces[8].refvert[2] = PFASD_NIL_ID;
	faces[8].child[0] = PFASD_NIL_ID;
	faces[8].child[1] = PFASD_NIL_ID;
	faces[8].child[2] = PFASD_NIL_ID;
	faces[8].child[3] = PFASD_NIL_ID;
	
	    // 9
	faces[9].level = 1;
	faces[9].tsid = 0;
	faces[9].vert[0] = 7;  // N4
	faces[9].vert[1] = 3;  // V4
	faces[9].vert[2] = 8;  // N5
	faces[9].refvert[0] = PFASD_NIL_ID;
	faces[9].refvert[1] = PFASD_NIL_ID;
	faces[9].refvert[2] = PFASD_NIL_ID;
	faces[9].child[0] = PFASD_NIL_ID;
	faces[9].child[1] = PFASD_NIL_ID;
	faces[9].child[2] = PFASD_NIL_ID;
	faces[9].child[3] = PFASD_NIL_ID;
    
    } else {	    // ************* TEXTURABLE GEOMETRY *************** //
    
		// --- LEVEL 1 --- //
	    // 2
	faces[2].level = 1;
	faces[2].tsid = 0;
	faces[2].vert[0] = 4;  // V1
	faces[2].vert[1] = 1;  // N1
	faces[2].vert[2] = 5;  // N3
	faces[2].refvert[0] = PFASD_NIL_ID;
	faces[2].refvert[1] = PFASD_NIL_ID;
	faces[2].refvert[2] = PFASD_NIL_ID;
	faces[2].child[0] = PFASD_NIL_ID;
	faces[2].child[1] = PFASD_NIL_ID;
	faces[2].child[2] = PFASD_NIL_ID;
	faces[2].child[3] = PFASD_NIL_ID;
	    // 3
	faces[3].level = 1;
	faces[3].tsid = 0;
	faces[3].vert[0] = 4;  // N1
	faces[3].vert[1] = 5;  // V2
	faces[3].vert[2] = 0;  // N2
	faces[3].refvert[0] = PFASD_NIL_ID;
	faces[3].refvert[1] = PFASD_NIL_ID;
	faces[3].refvert[2] = PFASD_NIL_ID;
	faces[3].child[0] = PFASD_NIL_ID;
	faces[3].child[1] = PFASD_NIL_ID;
	faces[3].child[2] = PFASD_NIL_ID;
	faces[3].child[3] = PFASD_NIL_ID;
	    // 4
	faces[4].level = 1;
	faces[4].tsid = 0;
	faces[4].vert[0] = 0;  // N1
	faces[4].vert[1] = 5;  // N2
	faces[4].vert[2] = 6;  // N3
	faces[4].refvert[0] = PFASD_NIL_ID;
	faces[4].refvert[1] = PFASD_NIL_ID;
	faces[4].refvert[2] = PFASD_NIL_ID;
	faces[4].child[0] = PFASD_NIL_ID;
	faces[4].child[1] = PFASD_NIL_ID;
	faces[4].child[2] = PFASD_NIL_ID;
	faces[4].child[3] = PFASD_NIL_ID;
	    // 5
	faces[5].level = 1;
	faces[5].tsid = 0;
	faces[5].vert[0] = 6;  // N3
	faces[5].vert[1] = 5;  // N2
	faces[5].vert[2] = 2;  // V3
	faces[5].refvert[0] = PFASD_NIL_ID;
	faces[5].refvert[1] = PFASD_NIL_ID;
	faces[5].refvert[2] = PFASD_NIL_ID;
	faces[5].child[0] = PFASD_NIL_ID;
	faces[5].child[1] = PFASD_NIL_ID;
	faces[5].child[2] = PFASD_NIL_ID;
	faces[5].child[3] = PFASD_NIL_ID;
	    // 6
	faces[6].level = 1;
	faces[6].tsid = 0;
	faces[6].vert[0] = 1;  // V2
	faces[6].vert[1] = 7;  // N4
	faces[6].vert[2] = 5;  // N2
	faces[6].refvert[0] = PFASD_NIL_ID;
	faces[6].refvert[1] = PFASD_NIL_ID;
	faces[6].refvert[2] = PFASD_NIL_ID;
	faces[6].child[0] = PFASD_NIL_ID;
	faces[6].child[1] = PFASD_NIL_ID;
	faces[6].child[2] = PFASD_NIL_ID;
	faces[6].child[3] = PFASD_NIL_ID;
	
	    // 7
	faces[7].level = 1;
	faces[7].tsid = 0;
	faces[7].vert[0] = 5;  // N2
	faces[7].vert[1] = 7;  // N4
	faces[7].vert[2] = 3;  // N5
	faces[7].refvert[0] = PFASD_NIL_ID;
	faces[7].refvert[1] = PFASD_NIL_ID;
	faces[7].refvert[2] = PFASD_NIL_ID;
	faces[7].child[0] = PFASD_NIL_ID;
	faces[7].child[1] = PFASD_NIL_ID;
	faces[7].child[2] = PFASD_NIL_ID;
	faces[7].child[3] = PFASD_NIL_ID;
	
	    // 8
	faces[8].level = 1;
	faces[8].tsid = 0;
	faces[8].vert[0] = 5;  // N2
	faces[8].vert[1] = 3;  // N5
	faces[8].vert[2] = 8;  // V3
	faces[8].refvert[0] = PFASD_NIL_ID;
	faces[8].refvert[1] = PFASD_NIL_ID;
	faces[8].refvert[2] = PFASD_NIL_ID;
	faces[8].child[0] = PFASD_NIL_ID;
	faces[8].child[1] = PFASD_NIL_ID;
	faces[8].child[2] = PFASD_NIL_ID;
	faces[8].child[3] = PFASD_NIL_ID;
	
	    // 9
	faces[9].level = 1;
	faces[9].tsid = 0;
	faces[9].vert[0] = 5;  // N4
	faces[9].vert[1] = 8;  // V4
	faces[9].vert[2] = 2;  // N5
	faces[9].refvert[0] = PFASD_NIL_ID;
	faces[9].refvert[1] = PFASD_NIL_ID;
	faces[9].refvert[2] = PFASD_NIL_ID;
	faces[9].child[0] = PFASD_NIL_ID;
	faces[9].child[1] = PFASD_NIL_ID;
	faces[9].child[2] = PFASD_NIL_ID;
	faces[9].child[3] = PFASD_NIL_ID;
    }
    
    faces[0].gstateid = 0;
    faces[1].gstateid = 1;
    faces[2].gstateid = 0;
    faces[3].gstateid = 0;
    faces[4].gstateid = 0;
    faces[5].gstateid = 0;
    faces[6].gstateid = 1;
    faces[7].gstateid = 1;
    faces[8].gstateid = 1;
    faces[9].gstateid = 1;
    
    pfASDLODRange* lods;	// A pfASDLODRange array. 
    
    lods = (pfASDLODRange*)pfCalloc(2, sizeof(pfASDLODRange), pfGetSharedArena());
    Shared->lods = lods;
    
	// -- How the heck does this work??? --- //
    lods[0].switchin = 0.0f;	// lod[0] does not matter
    lods[0].morph = 0.0f;
    lods[1].switchin = 15.0f;	// This is the cut off where morphing starts	
    lods[1].morph = 7.0f;	// This is the size of the morphing "buffer"
    
    
	// ----- ATTRIBUTE SETUP ----- //
	// Set the attribute for the face vertex to point into the attribute arrays in
	// such a was that the attribute array has one attribute per vertex
    for(int fIndex =0;fIndex<numFaces;fIndex++)
    {
	faces[fIndex].attr[0] = faces[fIndex].vert[0];
	faces[fIndex].attr[1] = faces[fIndex].vert[1];
	faces[fIndex].attr[2] = faces[fIndex].vert[2];
	
	if (fIndex <= 1) {
		// --- Setup refvert attrs --- //
	    faces[fIndex].sattr[0] = faces[fIndex].refvert[0];
	    faces[fIndex].sattr[1] = faces[fIndex].refvert[1];
	    faces[fIndex].sattr[2] = faces[fIndex].refvert[2];
	} else {
	    faces[fIndex].sattr[0] = PFASD_NIL_ID;
	    faces[fIndex].sattr[1] = PFASD_NIL_ID;
	    faces[fIndex].sattr[2] = PFASD_NIL_ID;  
	}
    }
    
	
    
    
    // ----- FILL UP ATTRIBUTE ARRAY ----- //
    pfVec3* norms = (pfVec3*)pfCalloc(9, sizeof(pfVec3), pfGetSharedArena());;
    /* NV1 */    norms[0] = (vertNormal(V1, V3, V2));
    /* NV2 */    norms[1] = (vertNormal(V2, V1, V4));
    /* NV3 */    norms[2] = (vertNormal(V3, N4, V1));
    /* NV4 */    norms[3] = (vertNormal(V4, V2, V3));

if(whichGeometry == OrigGeom) {	

    /* NN1 */    norms[4] = ((vertNormal(N1, V1, N3) + vertNormal(N1, N3, N2) + vertNormal(N1, N2, V2))/3);
    /* NN2 */    norms[5] = ((vertNormal(N2, N4, V2) + vertNormal(N2, V2, N1) + vertNormal(N2, N1, N3) + vertNormal(N2, N3, V3) + vertNormal(N2, V3, N5) + vertNormal(N2, N5, N4)) / 6.0);  
    /* NN3 */    norms[6] = ((vertNormal(N3, V3, N2) + vertNormal(N3, N2, N1) + vertNormal(N3, N1, V1)) / 3.0f);
    /* NN4 */    norms[7] = ((vertNormal(N4, V2, N2) + vertNormal(N4, N2, N5) + vertNormal(N4, N5, V4)) / 3.0f);
    /* NN5 */    norms[8] = ((vertNormal(N5, V4, N4) + vertNormal(N5, N4, N2) + vertNormal(N5, N2, V3)) / 3.0f);

} else {

    /* NN1 */    norms[4] = ((vertNormal(N1, V1, N2) + vertNormal(N1, N3, N2)) / 2.0f);
    /* NN2 */    norms[5] = ((vertNormal(N2, N4, V2) + vertNormal(N2, V2, N1) + vertNormal(N2, N1, V1) + vertNormal(N2, V1, N3) + vertNormal(N2, N3, V3) + vertNormal(N2, V3, N5) + vertNormal(N2, N5, V4) + vertNormal(N2, V4, N4)) / 8.0f);  
    /* NN3 */    norms[6] = ((vertNormal(N3, V3, N2) + vertNormal(N3, N2, V1)) / 2.0f);
    /* NN4 */    norms[7] = ((vertNormal(N4, V2, N2) + vertNormal(N4, N2, V4)) / 2.0f);
    /* NN5 */    norms[8] = ((vertNormal(N5, V4, N2) + vertNormal(N5, N2, V3)) / 2.0f);

}    

    for(int nIndex=0;nIndex<9;nIndex++)
	norms[nIndex].normalize();
    
    ASDAttr* attrs = (ASDAttr*)pfCalloc(numVerts, sizeof(ASDAttr), pfGetSharedArena());
    Shared->attrs = attrs;
    
	// -- Fill with normal data -- //
    pfVec3 referencePtNormal(0.0, 0.0, 1.0);	// All points have the same reference normal point
    
    for(int vIndex=0;vIndex<numVerts;vIndex++)
    {
	attrs[vIndex].normal.normal = norms[vIndex];
	attrs[vIndex].normal.normalDiff = (norms[vIndex] - referencePtNormal);
    }
    
    attrs[0].normal.normalDiff = pfVec3(0.3f, 0.4f, 0.2f);
	// --- Fill with color data -- //
    
    for(int cIndex=0;cIndex<numVerts;cIndex++)
    {
	if (cIndex < 4)  // One of the base verts
	{
	    attrs[cIndex].color.color = Shared->baseColor;
	    attrs[cIndex].color.colorDiff.set(0.0, 0.0, 0.0, 0.0f);   
	} else {
	    attrs[cIndex].color.color = Shared->morphedColor;
	    attrs[cIndex].color.colorDiff = (Shared->baseColor - Shared->morphedColor);     
	}
    }    
    
    pfASD* asdNode = new pfASD;
    asdNode->setAttr(PFASD_LODS, 0, 2, Shared->lods);
    asdNode->setAttr(PFASD_COORDS, 0, numVerts, Shared->verts);
    asdNode->setAttr(PFASD_FACES, 0, numFaces, Shared->faces);
//    asdNode->setAttr(PFASD_PER_VERTEX_ATTR, PFASD_NORMALS, numVerts, (float*)Shared->normalAttrs);
//    asdNode->setAttr(PFASD_PER_VERTEX_ATTR, PFASD_COLORS, numVerts, (float*)Shared->colorAttrs);
    asdNode->setAttr(PFASD_PER_VERTEX_ATTR, PFASD_NORMALS | PFASD_COLORS, numVerts, (float*)Shared->attrs);
    asdNode->setNumBaseFaces(2);
    
    asdNode->setEvalMethod(PFASD_DIST);
    asdNode->setMorphAttrs(PFASD_COORDS | PFASD_NORMALS | PFASD_COLORS);	
    asdNode->setMaxMorphDepth(1, 0.0);	    // 1 is the finest level I want to use 

    pfGeoState** gsa;
    pfGeoState *gs1, *gs2;
    gsa = (pfGeoState**)pfMalloc(2*sizeof(pfGeoState*));
    Shared->geoStateArray = gsa;
    gs1 = new pfGeoState;
    gs2 = new pfGeoState;
    
    if(whichGeometry == OrigGeom) {
	buildGState(gs1);
	buildGState(gs2);
    } else {    
	buildTexGState(gs1, "wood.rgb");
	buildTexGState(gs2, "bark.rgb");
    }
    
    gsa[0] = gs1;
    gsa[1] = gs2;
    
    asdNode->setGStates(gsa,2);
    asdNode->config();		    // Tell the ASD it can setup itself now
    return asdNode;
    
}

void switchMorphAttrs()		// Call to update which params are to be morphed
{
    Shared->theASDNode->setMorphAttrs(
	(pfuGetWidgetValue(Shared->wMorphCoords) == 1.0 ? PFASD_COORDS : 0) |
	(pfuGetWidgetValue(Shared->wMorphNorms) == 1.0 ? PFASD_NORMALS : 0) |
	(pfuGetWidgetValue(Shared->wMorphColors) == 1.0 ? PFASD_COLORS : 0) );
	   
}

void setNewSwitchingValue(float newValue)
{
    Shared->lods[1].switchin = newValue;
    Shared->theASDNode->setAttr(PFASD_LODS, 0, 2, Shared->lods);    
}

void setMorphBufferValue(float newValue)
{
    Shared->lods[1].morph = newValue;
    Shared->theASDNode->setAttr(PFASD_LODS, 0, 2, Shared->lods);    
}


void buildGState(pfGeoState *gs)
{
    gs->setMode(PFSTATE_ENLIGHTING, PF_ON);
    gs->setMode(PFSTATE_CULLFACE, PFCF_OFF);	// Turn off backface culling
    //pfGStateMode(gs, PFSTATE_ENTEXTURE, PF_ON);
    //ViewState->mtl = defaultMaterial();
    gs->setAttr(PFSTATE_FRONTMTL, defaultMaterial());
    gs->setAttr(PFSTATE_BACKMTL, defaultMaterial());   
}

void buildTexGState(pfGeoState *gs, char* texname)
{
	// --- Setup pfTexture --- //
    pfTexture *tex = new pfTexture;
    
    tex->setFilter(PFTEX_MINFILTER, PFTEX_BILINEAR);
    
    if (!tex->loadFile(texname))
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "TEXTURE: Texture not loaded.\n");
    else
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "TEXTURE: Texture %s was loaded.\n", texname);
	
    uint *image;
    int nc, sx, sy, sz;
    
    tex->getImage(&image, &nc, &sx, &sy, &sz);
	
	// if have alpha channel, enable transparency
    if (nc != 3)
	gs->setMode(PFSTATE_TRANSPARENCY, PFTR_FAST);
    
	// set alpha function to block pixels of 0 alpha for 
	//   transparent textures
    //gs->setMode(PFSTATE_ALPHAFUNC, PFAF_NOTEQUAL);
    //gs->setVal(PFSTATE_ALPHAREF, 0.0f);
    gs->setAttr(PFSTATE_TEXTURE, tex);
    gs->setMode(PFSTATE_ENTEXTURE,1);
    gs->setMode(PFSTATE_ENLIGHTING,0);
    gs->setMode(PFSTATE_CULLFACE,PFCF_OFF);
    
	// --- Setup Texture Environment --- //
    pfTexEnv* tev = new pfTexEnv;
    
    tev->setMode(PFTE_DECAL);
    //tev->setMode(PFTE_BLEND);
    //tev->setBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    gs->setAttr(PFSTATE_TEXENV, tev);
    
	// ---- Setup TexGen ---- //
    pfTexGen* texGen = new pfTexGen;
    texGen->setMode(PF_S, PFTG_OBJECT_LINEAR);
    texGen->setMode(PF_T, PFTG_OBJECT_LINEAR);   
    
    gs->setAttr(PFSTATE_TEXGEN, texGen);
    gs->setMode(PFSTATE_ENTEXGEN, PF_ON);
}

/* Keep a default Material Handy */
pfMaterial* defaultMaterial()
{
    static pfMaterial* material = NULL;

    if (material == NULL)
    {
        material = new pfMaterial;
        material->setColor(PFMTL_AMBIENT,  0.15f, 0.8f, 0.87f);
        material->setColor(PFMTL_DIFFUSE,  0.15f, 0.8f, 0.87f);
        material->setColor(PFMTL_SPECULAR, 1.0f, 1.0f, 1.0f);
        material->setShininess(0.0f);
	material->setAlpha(1.0f);

        //material->setColorMode( PFMTL_FRONT, PFMTL_CMODE_AMBIENT_AND_DIFFUSE);
    }

	// -- return pointer to default material --- //
    return material;
}

pfHighlight* createNormalHighlight()
{
    pfHighlight* hLight = new pfHighlight;
    hLight->setMode(PFHL_NORMALS);
    hLight->setLineWidth(1.0f);
    hLight->setNormalLength(1.0f, 0.0f);
    hLight->setColor(PFHL_FGCOLOR, 1.0f, 0.0f, 0.0f);
    hLight->setColor(PFHL_BGCOLOR, 1.0f, 0.0f, 0.0f);
    return hLight;
}

void switchASDLighting()
{
    if(Shared->geoStateArray[0]->getMode(PFSTATE_ENLIGHTING) == PF_ON) {
	Shared->geoStateArray[0]->setMode(PFSTATE_ENLIGHTING, PF_OFF);
	Shared->geoStateArray[1]->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    } else {
	Shared->geoStateArray[0]->setMode(PFSTATE_ENLIGHTING, PF_ON);
	Shared->geoStateArray[1]->setMode(PFSTATE_ENLIGHTING, PF_ON);
    }    
}

// -------------- XFORMER STUFF --------------- //
static void
InitXformer(void)
{
    Shared->xformer = (pfiXformer *)pfiNewTDFXformer(pfGetSharedArena());
    Shared->xformer->setAutoInput(Shared->chan, &Shared->mouse, &Shared->events);
    Shared->xformer->setNode(Shared->scene);
    Shared->xformer->setAutoPosition(Shared->chan, Shared->sceneDCS);
    Shared->xformer->setResetCoord(&Shared->initView);
    xformerModel(Shared->xformerModel, Shared->collideMode);

}

static void
IsectFunc(void *)
{
//    pfiCollideXformer(Shared->xformer);
}


/******************************************************************************
*			    App Viewing Model Routines
******************************************************************************
*/

//
// Thes routines use the viewing and tranformation utilities in
// libpfutil to implement several fiewing modes: drive, fly, and trackball.
//

static void
xformerModel(int model, int collideMode)
{
    static int		oldModel = -1, oldCollMode = -1;

    if ((model == oldModel && oldCollMode == collideMode) || !Shared->xformer)
	return;

    if (oldModel != model)
	Shared->xformer->selectModel(model);
    
    /* Collide with objects in scene */
    if (oldCollMode != collideMode)
    {
	if (collideMode == PF_ON)
	    Shared->xformer->enableCollision();
	else
	    Shared->xformer->disableCollision();
	oldCollMode = collideMode;
    }
}

//
// Update the current view
//
static void
UpdateView(pfChannel *, pfScene *)
{    
    pfiUpdateXformer(Shared->xformer);
    Shared->xformerModel = Shared->xformer->getCurModelIndex();
    
    /* if have moving-eyepoint motion model, update eyepoint */
    if (Shared->xformer->getCurModel()->isOfType(
	    pfiInputXformTravel::getClassType()))
    {
	Shared->xformer->getCoord(&Shared->viewCoord);
    }
}


//
// Reset view to original XYZ position and HPR Euler angles
//
static void
resetPosition(void)
{
    Shared->xformer->stop();
    Shared->xformer->resetPosition();
}


/******************************************************************************
*			    App Input Handling
******************************************************************************
*/

static void
ProcessInputIO(void)
{
    int i,j,key;
    int dev, val, numDevs;
    pfuEventStream *pfevents;
    
    pfevents = &(Shared->events);
    
    numDevs = pfevents->numDevs;
    for (j=0; j < numDevs; j++)
    {
	dev = pfevents->devQ[j];
	val = pfevents->devVal[j];
	if (pfevents->devCount[dev] > 0)
	{
	    switch(dev)
	    {
	    case PFUDEV_REDRAW:
		// worst case in APP_CULL_DRAW mode it takes 3 frames to
		// propagate new window size back down to draw process.
		//
		Shared->redraw = 3;
		pfuRedrawGUI();
		pfevents->devCount[dev] = 0; // mark device done
		break;
		
	    case PFUDEV_WINQUIT:
		Shared->exitFlag = 1;
		pfevents->devCount[dev] = 0; // mark device done
		break;
		
		// Main Keyboard
	    case PFUDEV_KEYBD:
		for(i=0; i<pfevents->numKeys; i++)
		{
		    key = pfevents->keyQ[i];
		    if (pfevents->keyCount[key])
		    {	// key was pressed count times
			switch(key)
			{
			case 27:	// ESC
			    Shared->exitFlag = 1;
			    break;
			case 'g':
			case 'G':
			    Shared->drawStats = !Shared->drawStats;
			    pfuWidgetValue(Shared->wStats,
					   Shared->drawStats);
			    break;
			case 'r':
			case 'R':
			    resetPosition();
			    break;
			case ' ':
			    Shared->xformer->stop();
			    break;
			default:
			    break;
			}
		    }
		}
		// XXX this is very important or else future keybd events
                // will be lost !!!
                //
                pfevents->devCount[dev] = 0; // mark device done
		break;
	    case PFUDEV_F1KEY:
		Shared->gui = !Shared->gui;
		pfuWidgetValue(Shared->wKillGUI, Shared->gui);
		Shared->updateChannels = 1;
		pfevents->devCount[dev] = 0; // mark device done
		break;
	    } // switch on device
	} // if have device
    } // for each device
    pfevents->numDevs = 0;
    pfevents->numKeys = 0;
}

/******************************************************************************
*			    Panel Creation and Management
******************************************************************************
*/

static pfuGUIString viewModel_strs[] = {"Trackball", "Fly", "Drive"};
static int viewModel_vals[] = {
    PFITDF_TRACKBALL, PFITDF_FLY, PFITDF_DRIVE};
static pfuGUIString drawStyle_strs[] = {"Filled", "Scribed", "Lines", "Hidden"};
static int drawStyle_vals[] = { PFUSTYLE_FILLED, PFUSTYLE_SCRIBED, PFUSTYLE_LINES, PFUSTYLE_HIDDEN};

static pfuGUIString evalMethod_strs[] = {"Dist", "Dist^2"};
static int evalMethod_vals[] = { PFASD_DIST, PFASD_DIST_SQUARE};


static pfuPanel *
InitPanel(void)
{
    float  xOrigin, yOrigin, ySize, xSize;
    int x, y, yTop;
    pfuWidget *wid;
    pfuPanel *pan;
    
    pan = pfuNewPanel();
    
    pfuGetPanelOriginSize(pan, &xOrigin, &yOrigin, &xSize, &ySize);
    
    x = xOrigin;
    yTop = (yOrigin + ySize);
    y = yTop - PFUGUI_BUTTON_YINC;

	// -------------------- //
	// ------- ROW 1 ------ //
	// -------------------- //
    // action  buton - disable the gui (F1KEY to get it back)
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, 0);
    pfuWidgetDim(wid,  x,  y,  24,  24);
    pfuWidgetValue(wid, 1.0f);
    pfuWidgetDefaultValue(wid, 1.0f);
    pfuWidgetActionFunc(wid, KillGUI);
    Shared->wKillGUI = wid;
    x += 24 + PFUGUI_PANEL_FRAME;
    
    // action button - quit
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_QUIT);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_XSIZE,  PFUGUI_BUTTON_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetLabel(wid, "Quit");
    x += PFUGUI_BUTTON_XINC;
    
    // action  buton - reset to origin button
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_REPOSITION);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_VLONG_XSIZE,
		 PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Reposition");
    pfuWidgetActionFunc(wid, PanelControl);
    x += PFUGUI_BUTTON_VLONG_XINC;
    
    // the message bar with positions
    wid = pfuNewWidget(pan, PFUGUI_MESSAGE, 0);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_MESSAGE_XSIZE,
		 PFUGUI_MESSAGE_YSIZE);
    pfuWidgetActionFunc(wid, PanelControl);
    x += PFUGUI_MESSAGE_XINC;
    Shared->wPosXYZ = wid;
    
    
    // action button - reset all
    wid = pfuNewWidget(pan, PFUGUI_BUTTON, CTL_RESET_ALL);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_VLONG_XSIZE,
		 PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Reset All");
    pfuWidgetActionFunc(wid, PanelControl);
    Shared->wResetAll = wid;
    
    x = xOrigin;
    y = yTop - 2*PFUGUI_BUTTON_YINC;
    
	// -------------------- //
	// ------- ROW 2 ------ //
	// -------------------- //
    
    // menu button - view model
    wid = pfuNewWidget(pan, PFUGUI_MENU_BUTTON, CTL_MODEL);
    pfuWidgetDim(wid,  x,  y,  2*PFUGUI_BUTTON_LONG_XSIZE,
		 PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Motion");
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetSelections(wid, viewModel_strs, viewModel_vals, NULL, 3);
    Shared->wFModel = wid;
    x += (2*PFUGUI_BUTTON_LONG_XINC);
    
    // menu button - draw style
    wid = pfuNewWidget(pan, PFUGUI_MENU_BUTTON, CTL_DRAWSTYLE);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_VLONG_XSIZE,
		 PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Style:");
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetSelections(wid, drawStyle_strs, drawStyle_vals, NULL, 4);
    Shared->wDrawStyle = wid;
    x += PFUGUI_BUTTON_VLONG_XINC;
    
    // menu button - ASD Eval Method
    wid = pfuNewWidget(pan, PFUGUI_MENU_BUTTON, CTL_EVAL_METHOD);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_BUTTON_VLONG_XSIZE,
		 PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Eval:");
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetSelections(wid, evalMethod_strs, evalMethod_vals, NULL, 2);
    Shared->wEvalMethod = wid;
    x += PFUGUI_BUTTON_VLONG_XINC;
    
    
    // switch button - stats
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_DIAGS);
    pfuWidgetDim(wid, x, y, PFUGUI_BUTTON_MED_XSIZE, PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Stats");
    pfuWidgetActionFunc(wid, PanelControl);
    Shared->wStats = wid;
    x += PFUGUI_BUTTON_MED_XINC;
    
    // switch button - tree
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_TREE);
    pfuWidgetDim(wid, x, y, PFUGUI_SCALE_XSIZE(80), PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Tree");
    pfuWidgetActionFunc(wid, PanelControl);
    Shared->wTree = wid;
    x += PFUGUI_BUTTON_MED_XINC;
    
    // switch button - Lighting
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_LIGHTING);
    pfuWidgetDim(wid, x, y, PFUGUI_BUTTON_MED_XSIZE, PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "No Light");
    pfuWidgetActionFunc(wid, PanelControl);
    Shared->wLighting = wid;
    x += PFUGUI_BUTTON_MED_XINC;
    
    // switch button - draw Normals
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_DRAW_NORMS);
    pfuWidgetDim(wid, x, y, PFUGUI_SCALE_XSIZE(80), PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Draw Norms");
    pfuWidgetActionFunc(wid, PanelControl);
    Shared->wDrawNorms = wid;
    x += PFUGUI_BUTTON_MED_XINC;
    
    
    x = xOrigin;
    y -= PFUGUI_SLIDER_YINC;

	// -------------------- //
	// ------- ROW 3 ------ //
	// -------------------- //
	
	// --- Morph switchin value --- //
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_ASD_DIST);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_BIG_XSIZE, PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "ASD SwitchIn Distance");
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetRange(wid, PFUGUI_SLIDER, 0.0, 25.0, 15.0);
    Shared->wASDDist = wid;
    x += PFUGUI_SLIDER_BIG_XINC;
    
	// --- Morph Buffer value --- //
    wid = pfuNewWidget(pan, PFUGUI_SLIDER, CTL_MORPH_BUFFER);
    pfuWidgetDim(wid,  x,  y,  PFUGUI_SLIDER_BIG_XSIZE, PFUGUI_SLIDER_YSIZE);
    pfuWidgetLabel(wid, "ASD Morph Buffer");
    pfuWidgetActionFunc(wid, PanelControl);
    pfuWidgetRange(wid, PFUGUI_SLIDER, 0.0, 20.0, 7.0);
    Shared->wMorphBuffer = wid;
    x += PFUGUI_SLIDER_BIG_XINC;
    
    
    x = xOrigin;
    y -= PFUGUI_SLIDER_YINC;
	// -------------------- //
	// ------- ROW 4 ------ //
	// -------------------- //
    
    // switch button - morph Coords
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_MORPH_COORDS);
    pfuWidgetDim(wid, x, y, PFUGUI_BUTTON_MED_XSIZE, PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Coords");
    pfuWidgetActionFunc(wid, PanelControl);
    Shared->wMorphCoords = wid;
    x += PFUGUI_BUTTON_MED_XINC;
    
     // switch button - morph Norms
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_MORPH_NORMS);
    pfuWidgetDim(wid, x, y, PFUGUI_BUTTON_MED_XSIZE, PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Norms");
    pfuWidgetActionFunc(wid, PanelControl);
    Shared->wMorphNorms = wid;
    x += PFUGUI_BUTTON_MED_XINC;
    
     // switch button - morph Colors
    wid = pfuNewWidget(pan, PFUGUI_SWITCH, CTL_MORPH_COLORS);
    pfuWidgetDim(wid, x, y, PFUGUI_BUTTON_MED_XSIZE, PFUGUI_BUTTON_YSIZE);
    pfuWidgetLabel(wid, "Colors");
    pfuWidgetActionFunc(wid, PanelControl);
    Shared->wMorphColors = wid;
    x += PFUGUI_BUTTON_MED_XINC;
         
    return pan;
}


static void
KillGUI(pfuWidget *w)
{
    if (pfuGetWidgetValue(w) == 0.0f)
    {
	Shared->gui = 0;
	Shared->updateChannels = 1;
    }
}

//
// PanelControl
// -------------
// This function is called from the draw process from the currently
// active widget.  
//

static void
PanelControl(pfuWidget *w)
{
    float val;
    switch(pfuGetWidgetId(w))
    {
    case CTL_RESET_ALL:
	pfuResetGUI();
	resetPosition();
	//Detail_mod = TRUE;
	break;
    case CTL_REPOSITION:
	resetPosition();
	//Detail_mod = TRUE;
	break;
    case CTL_QUIT:
	Shared->exitFlag = 1;
	
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     " Have Quit button - bye...\n");
	
	break;
    case CTL_MODEL:
	val = pfuGetWidgetValue(w);
	Shared->xformerModel = val;
	xformerModel(val, Shared->collideMode);
	
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     " Have Viewing Model: %.0f\n", val);
	
	break;
    
    case CTL_DRAWSTYLE:
	val = pfuGetWidgetValue(w);
	Shared->drawStyle = val;
	break;
	
    case CTL_EVAL_METHOD:
	Shared->theASDNode->setEvalMethod(pfuGetWidgetValue(Shared->wEvalMethod));
	break;
	
    case CTL_DIAGS:
	Shared->drawStats = (pfuGetWidgetValue(w) == 1.0f);
	break;
    
    case CTL_TREE:
	Shared->drawTree = (pfuGetWidgetValue(w) == 1.0f);
	break;
	
    case CTL_LIGHTING:
	switchASDLighting();
	break;
	
    case CTL_DRAW_NORMS:
	Shared->drawNormals = (pfuGetWidgetValue(w) == 1.0f);
	break;
	
    case CTL_ASD_DIST:
	setNewSwitchingValue(pfuGetWidgetValue(w));
	break;
	
    case CTL_MORPH_BUFFER:
	setMorphBufferValue(pfuGetWidgetValue(w));
	break;
    
    case CTL_MORPH_COORDS:
    case CTL_MORPH_NORMS:
    case CTL_MORPH_COLORS:
	switchMorphAttrs();	// Call funtion to set the new morph mode
	break;

    default:
	
	    pfNotify(PFNFY_DEBUG, PFNFY_PRINT,
		     "Have GUI Widget \"%s\": id=%d, val = %d\n",
		     pfuGetWidgetLabel(w), pfuGetWidgetId(w), pfuGetWidgetValue(w));
	
	break;
    }

}

static void
updateGUIViewMsg(pfCoord *viewpt)
{
    static char     posBuf[256];
    
    sprintf(posBuf," X%5.1f Y%5.1f Z%5.1f H%5.1f P%5.1f R%5.1f",
	    viewpt->xyz[PF_X],viewpt->xyz[PF_Y],viewpt->xyz[PF_Z],
	    viewpt->hpr[PF_H],viewpt->hpr[PF_P],viewpt->hpr[PF_R]);
    pfuWidgetLabel(Shared->wPosXYZ, posBuf);
}

/******************************************************************************
*			    Draw Process: Graphics Initialization
******************************************************************************
*/

//
//	OpenPipeWin() -- create a pipeline: setup the window system,
//	the OpenGL, and OpenGL Performer. this procedure is executed in
//	the draw process (when there is a separate draw process).
//

static void
OpenPipeWin(pfPipeWindow *pw)
{
    // use pfu utility to open the window so that the pfu input
    // handler will be able to attach to it
    //
    pw->open();
    
}

/******************************************************************************
*			    Draw Process Routines
******************************************************************************
*/

//
//	DrawChannel() -- draw a channel and read input queue. this
//	procedure is executed in the draw process (when there is a
//	separate draw process).
//
static void
DrawChannel (pfChannel *channel, void *)
{
    static pfVec4 bgclr(0.0f, 0.0f, 0.0f, 1.0f);
    static pfVec4 scribeColor(1.0f, 0.0f, 0.0f, 1.0f);
    
    // erase framebuffer and draw Earth-Sky model
    pfClear(PFCL_COLOR | PFCL_DEPTH, &bgclr);
    
    // invoke Performer draw-processing and style for this frame
    pfuPreDrawStyle(Shared->drawStyle, scribeColor); 
    pfDraw();
    pfuPostDrawStyle(Shared->drawStyle);
    
    {
	int err = glGetError();
	if (err != GL_NO_ERROR)
	    pfNotify(PFNFY_NOTICE,PFNFY_USAGE,
		     "OpenGL Error 0x%x - %s",err, gluErrorString(err));
    }
    
    
    // Enable drawing of Performer throughput statistics
    if (Shared->drawStats)
	channel->drawStats();
    
    static pfVec3 treeScale(0.0f, 0.0f, 1.0f);
    if(Shared->drawTree)
	pfuDrawTree(channel, (pfNode*)Shared->scene, treeScale);
	
    // Check the mouse and keyboard = only needed for GL input
    // This is done after the draw becuase it can happen in parallel
    // with the pipe finishing the rendering of the frame.
    //
    pfuCollectInput();
}

/******************************************************************************
*			    Cull Process Routines
******************************************************************************
*/

//
//	CullChannel() -- traverse the scene graph and generate a
// 	display list for the draw process.  This procedure is
//	executed in the cull process.
//
static void
CullChannel(pfChannel *, void *)
{
    //
    // pfDrawGeoSet or other display listable Performer routines
    // could be invoked before or after pfCull()
    // 
    
    pfCull();
}



