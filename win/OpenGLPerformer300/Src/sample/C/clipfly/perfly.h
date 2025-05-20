/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
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

/* 
 * file: perfly.h	
 * --------------
 * $Revision: 1.38 $
 * $Date: 2002/07/23 00:21:44 $
 */

#ifndef __PERFLY_H__
#define __PERFLY_H__

/******************************************************************************
 *			    Constants           
 *******************************************************************************/

#if 0 /* PERFLY */
#include <Performer/pfui.h>
#endif /* PERFLY */
#if 1 /* CUSTOM */
#include <Performer/pfuiD.h>
#endif /* PERFLY */
#include "generic.h"
#include "env.h"

#define UPDATE_MODES	0x1
#define REDRAW_WINDOW	0x2

/* resetPosition indices */
#define POS_NORESET	-2
#define POS_CENTER	-1
#define POS_ORIG	0

/* alternate motion models */
#define PFITDF_TETHER 4  /* != PFITDF_TRACKBALL, PFITD_DRIVE, PFITD_FLY */

#define DEVLIST_LENGTH 	9
#define BOUND		1000

/* Window visual selection indices */
#define PERFLY_WIN_NOMS	0

/* limits and sizes */
#define	MAX_LAMPS	8
#define MAX_LOCS	20
#define MAX_VEHICLES	50
#define MAX_TETHERVIEWS	20

#define	LIGHTING_OFF	0
#define	LIGHTING_EYE	1
#define	LIGHTING_SUN	2
#define	LIGHTING_COUNT	3

#define	DLIST_OFF	0
#define	DLIST_GEOSET	1
#define	DLIST_SCENE	2
#define	DLIST_COMPILE_GEOSET	3


/* Dynamic Resolution controls */
#define DVR_OFF		PFPVC_DVR_OFF
#define DVR_MANUAL	PFPVC_DVR_MANUAL
#define DVR_AUTO	PFPVC_DVR_AUTO

#if 1 /* CUSTOM */
/* Clip texture slider mode controls */
#define CLIP_CTRLMODE_AUTO    0
#define CLIP_CTRLMODE_LEQ     1
#define CLIP_CTRLMODE_GEQ     2
#define CLIP_CTRLMODE_EQ      (CLIP_CTRLMODE_LEQ|CLIP_CTRLMODE_GEQ)

#endif /* CUSTOM */


typedef struct 
{
    int             pane;		/* pane this widget is grouped with */
    int		    widget;		/* enum of this widget - guiWidgets */
    int             across;
} SharedGUIState;

/*
 *  Structure for data shared between processes.
 *  Note that booleans in this structure cannot be declared as int :1,
 *  since that will mess up concurrent accesses.
 */
typedef struct
{
	/* required by common/main.c */
    pfChannel	    *masterChan;	/* Main channel */
    int	    	    phase;		/* phase: FREE, FLOAT, or LOCK */
    float	    frameRate;		/* target frame rate	 */
    uint	    procLock;		/* flags to lock down processors */

	/* required by common/generic.c */
    pfPipeWindow    *pw[MAX_PIPES];	/* one pfPipeWindow per pfPipe */
    pfChannel	    *chan[MAX_CHANS];	/* array of all of the channels */
    int		    numChans;		/* number of channels */
    int		    MCO;		/* align to hw video channels */
    int	    	    input;		/* input type: GL or X */
    int	    	    gui;		/* ui mode flag */
    int	    	    rtView;		/* update view at last possible min */
    int	    	    exitFlag;		/* exit application flag */
    int	    	    updateChannels;	/* update channel configuration */
    float	    nearPlane;		/* nearPlane clipping plane  */
    float	    farPlane;		/* farPlane clipping plane  */
    float	    sceneSize;		/* maximum scene bound */
    int	    	    stats;		/* diag graph on/ff flag */
    pfList	    *texList;		/* texture list to download */

	/* required by common/env.c */
    pfEarthSky	    *eSky;		/* current EarthSky model */
    int	    	    earthSkyMode;	/* Earth-Sky mode */
    pfVec4	    earthSkyColor;	/* Earth-Sky color */
    pfVec4	    scribeColor;	/* scribe-mode color */
    int	    	    fog;		/* fog on/off */
    float	    farFogRange;	/* far fog range */
    float	    nearFogRange;	/* near fog range */
    pfDCS	    *sunDCS;		/* position of the sun */
    pfLightSource   *sun;		/* sun light source */
    float	    timeOfDay;		/* time of day */


	/* required by perfly/gui.c */
    int		    num_widgets;
    SharedGUIState  guiState[PFUGUI_MAXWIDGETS];
    pfuPanel	    *panel;		/* to allow regenerating GUI */

	/* used for libpfutil input handling */
    pfuWidget       *guiWidgets[PFUGUI_MAXWIDGETS];
    int	    	    guiFormat;		/* GUI vertical, or horizontal */
    pfuGLXWindow    xWinInfo;		/* GLX window info */
    pfuMouse	    mouse;		/* mouse structure */
    pfuEventStream  events;		/* event structure */
#if 0 /* PERFLY */
    pfiTDFXformer   *xformer;		/* interactive movement model */
#endif /* PERFLY */
#if 1 /* CUSTOM */
    pfiTDFXformerD  *xformer;		/* interactive movement model */
#endif /* CUSTOM */
    int	    	    xformerModel;	/* Fly, Drive, or Trackball   */

	/* used in perfly to define light sources */
    pfVec3	    lampXYZ[MAX_LAMPS];	/* light source direction */
    pfVec3	    lampRGB[MAX_LAMPS];	/* light source color */
    int		    lamps;		/* number of light sources */

	/* "OpenGL Performer" overlay-plane text */
    char	    welcomeText[256];
    char	    overlayText[256];

	/* Viewpoint and modes control */
    pfScene	    *scene;		/* visual database */
#if 0 /* PERFLY */
    pfDCS	    *sceneDCS;		/* DCS for trackball interface */ 
#endif /* PERFLY */
#if 1 /* CUSTOM */
    pfDoubleDCS	    *sceneDCS;		/* DoubleDCS for trackball interface */ 
#endif /* CUSTOM */
    pfGroup	    *sceneGroup;	/* root of all loaded files */
    const GLubyte   *glExtString;	/* OpenGL extensions string */ 
    int	    	    haveMultisample;	/* does window support multisample */
    int	    	    haveTagClear;	/* does window support tag clear */
    pfDCS	    *vehicleDCSs[MAX_VEHICLES];/* vehicles the eye can follow */
    char	    vehicleNames[MAX_VEHICLES][256]; /* names of vehicles */
    int		    numVehicles;	/* number of vehicles */
    int		    curVehicle;		/* focus vehicle */
    pfMatrix	    tetherViews[MAX_TETHERVIEWS]; /* above, inside, behind, etc. */
    char	    tetherViewNames[MAX_TETHERVIEWS][256];/* tetherViews names*/
    int		    numTetherViews;	/* number of tether views */
    int		    curTetherView;	/* which tether view currently in use */
    pfMatrix	    viewMat;	
#if 0 /* PERFLY */
    pfCoord	    viewCoord;		/* current view position, direction */ 
    pfCoord	    initViews[MAX_LOCS];/* initial view positions, directions */
#endif /* PERFLY */
#if 1 /* CUSTOM */
    pfCoordd	    viewCoord;		/* current view position, direction */ 
    pfCoordd	    initViews[MAX_LOCS];/* initial view positions, directions */
#endif /* CUSTOM */
    char	    initViewNames[MAX_LOCS][256]; /* names of initViews */
    int		    numInitViews;	/* number of initViews */
    int		    resetPosition;	/* flag for reset position  */
    int	    	    drawFlags[MAX_PIPES];/* Flags for draw process(es) */
    int	    	    redrawOverlay;	/* signal to redraw overlay */
    int	    	    redrawCount;	/* signal to redraw overlay */
    int	    	    stress;		/* Enable/disable stress */
    int	    	    stressMax;		/* stress bound */
    float	    lowLoad; 		/* stress parameters- low bound */
    float	    highLoad;		/* high bound */
    float	    stressScale;	/* stress scale amount */
    float	    LODscale;		/* LOD scale value */
    float	    fadeRange;		/* LOD fade range- RE Graphics */
    int	    	    fade;		/* Fade on/off -   RE Graphics */
    int	    	    collideMode;	/* collision & terrain following flag*/
    int	    	    statsEnable;	/* diag graph mode flag */
    int	    	    printStats;		/* flag to print stats to stderr */
    int	    	    exitCount;		/* exit after this many frames */
    int 	    drawStyle;		/* draw style */
    int 	    aa;			/* anti-alias flag */
    int 	    backface;		/* backface culling flag */
    int	    	    lighting;		/* lighting mode */
    int	    	    texture;		/* texture enable/disable */
#if 1 /* CUSTOM */
    int             tlut;               /* TLUT enable */
#endif /* CUSTOM */
    unsigned char   gridify;		/* whether to gridify the clip texture*/
    unsigned char   gridified;		/* whether clip texture is gridified */
#if 1 /* CUSTOM */

    int		    drawCteTex;		/* whether to draw emulated cliptexture
					   roaming textures in overlay */
    
    unsigned char   invalidate;		/* whether to invalidate clip texture */
    int             invalidateFrame;    /* for multipipe invalidate */
    pfuBarrier	    *invalidateBarrier; /* for multipipe invalidate */
    pfList	    *MPClipTextures;
    int		    numMPClipTextures;	/* num in list */
    pfMPClipTexture *MPClipTexture;	/* currently selected (NULL means all)*/
    int		    mip_coarsestLevel;  /* of most recently selected clip tex */
    int		    clip_clipSize;      /* of most recently selected clip tex */

    /* cliptexture select slider strs, values, and dirty flags */
    pfuGUIString    *clipTexStrs;
    int		    minLODMode, maxLODMode, elevelsMode, clipOffsetMode;
    int		    biasSMode, biasTMode, biasRMode;
    int		    mip_minFilter;
    int		    mip_magFilter;
    float	    mip_minLOD;
    float	    mip_maxLOD;
    float	    mip_LODBiasS;
    float	    mip_LODBiasT;
    float	    mip_LODBiasR;
    float	    clip_invalidBorder;
    int		    clip_LODOffset;
    int		    clip_numEffectiveLevels;
    int		    clip_DTRMode;
    float	    clip_totalDTRTime; /* really a pipe parameter */
    float	    clip_totalDTRTimeWasSpecified;
    float	    clip_DTRTime;
    float	    clip_DTRBlurMargin;
    int		    mip_auto_minLOD;
    int		    clip_centering;
    int		    clip_visualize;
    int		    monitor_icaches;
    int		    magnify_icache_monitor;
    char	    clip_anyParams_dirty,
		    mip_minFilter_dirty,
		    mip_magFilter_dirty,
		    mip_minLOD_dirty,
		    mip_maxLOD_dirty,
		    mip_LODBiasS_dirty,
		    mip_LODBiasT_dirty,
		    mip_LODBiasR_dirty,
		    clip_invalidBorder_dirty,
		    clip_LODOffset_dirty,
		    clip_numEffectiveLevels_dirty,
		    clip_DTRMode_dirty,
		    clip_totalDTRTime_dirty,
		    clip_DTRTime_dirty,
		    clip_DTRBlurMargin_dirty;
		    
#endif /* CUSTOM */

    int	    	    cullMode;		/* culling mode */
    float	    explode;		/* explode files */
    int	    	    snapImage;		/* save screen to file */
    int		    tree;		/* flag for tree display */
    pfVec3	    panScale;		/* pan in x,y and scale for tree display*/

    float	     cullDelta;		/* How much to shrink cull vol */
    pfGeode	    *cullVol;		/* Geode shows culling volume */
    pfDCS	    *cullVolDCS;	/* Position geode along view */
    pfVec4	    *cullVolColors;	/* Color of geode showing culling vol */

    int		    rotateCenter;	/* 0=origin, 1=center */
    int		    iterate;		/* number of times to load files */
    int		    optimizeGStates;    /* Optimize All gstates into a scene gstate */
    int		    optimizeTree;       /* Flatten and clean tree of empty nodes */
    int		    objFontType;
    char*	    objFontName;

    int		    combineBillboards;	/* maximum */
    int 	    visualIDs[MAX_PIPES]; /* GLX Visual IDs */

    int		    drawMode;		/* immediate mode or display list style */
    int		    doDVR, showDVR;
    int		    winXSize, winYSize; /* window size for master pipe */
    int		    vChanXSize, vChanYSize; /* master channel full size */
    int	    	    dvrXSize, dvrYSize; /* dvr control for master channel */
    float	    fov[3];             /* fovH, fovV, and channel offset */

    int		    firstPipeDraw[MAX_PIPES];	/* mark first draw on pipe */

    int		    doDemoPath;
    int             recordPath;
    int		    demoMode;
    int		    democnt;
#if 0 /* PERFLY */
    pfVec3	    *demoxyz, *demohpr;
#endif /* PERFLY */
#if 1 /* CUSTOM */
    pfVec3d	    *demoxyz, *demohpr;
    char	    sphericPathFileName[PF_MAXSTRING];
#endif /* CUSTOM */
    char	    demoPathFileName[PF_MAXSTRING];

    int		    startCallig;		/* start calligraphics (-U option) */
    float	    calligDefocus;	 	/* defocus value for calligraphic lp */
    float	    rasterDefocus;		/* defocus for the raster display */
    int		    calligDrawTime;		/* lp draw time in usec */
    int		    calligFilterSizeX;		/* Horizontal size of the debunching filter */
    int		    calligFilterSizeY;		/* Vertical size of the debunching filter */
    float	    calligZFootPrint;		/* size of the ms coverage for each lp */
    int		    calligDirty;		/* The user asked for changes in the GUI */
    int		    calligBoard;		/* A board has been detected */

#if 1 /* CUSTOM */
#ifdef DEBUG_DTR
    int		    DTRrecordreset;
    int		    DTRrecorddump;
#endif /* DEBUG_DTR */
#endif /* CUSTOM */
} SharedViewState;

/* XXX define when configuration states are implemented */
/* #define _PF_MULTI_VIEWSTATE */

#ifdef _PF_MULTI_VIEWSTATE
#define MAXVIEWSTATES 50
typedef struct SharedViewStates
{
    int nstates;
    SharedViewState **states;
    char (*statenames)[256];
} SharedViewStates;
extern SharedViewStates *AllViewStates; /* alternate selectable viewstates */
#endif

/* Structures visible to all processes 
 * - must be initialized before pfConfig() 
 */
extern SharedViewState	*ViewState;   /* holds data shared between processes */
extern int PackedAttrFormat;
/* 
 * Function prototypes  - called by common 
 */

    /* Application process */
extern int initSceneGraph(pfScene *scene);
extern void initViewState(void);
extern void initConfigFile(const char *file);
extern void initConfig(void);
extern int loadConfigFile(const char *file);
extern void initView(pfScene *scene);
extern void initChannel(pfChannel *chan);
extern void localPreFrame(pfChannel *chan);
extern void updateView(pfChannel *chan);
extern void resetPosition(int pos);
extern void initXformer(void);
extern void updateSim(pfChannel *chan);
extern void xformerModel(int mode, int collideMode);
extern void drawModesChanged(void);
#if 1 /* CUSTOM */
extern void getMPClipTextureValues(pfMPClipTexture *MPClipTex);
extern void updateClipTextureGUI(void);
#endif /* CUSTOM */
extern void preApp(pfChannel *chan, void *data);
extern void postApp(pfChannel *chan, void *data);

    /* Cull process */
extern void preCull(pfChannel *chan, void *data);
extern void postCull(pfChannel *chan, void *data);

    /* Lpoint process */
extern void preLpoint(pfChannel *chan, void *data);
extern void postLpoint(pfChannel *chan, void *data);

    /* Draw process */
extern void initPipe(pfPipeWindow *pw);
extern void localPreDraw(pfChannel *chan, void *data);
extern void localPostDraw(pfChannel *chan, void *data);

#if 1 /* CUSTOM */
    /* XXX Secret communication with read sort prioritization */
#define ALLOW_DEPTH_FIRST
#endif /* CUSTOM */

#if 1 /* CUSTOM */
    /* Use double precision versions of all pfui functions used...
       XXX if we use more, have to add them (this is error prone) */
    /* XXX consider just making a DP version of libpfui with the same names */
    /* XXX The only reason for the undefs is that some
	   source files in the common directory unnecessarily
	   include pfui.h... maybe that should be changed? */
    #undef pfiInputXformSpheric
    #define pfiInputXformSpheric pfiInputXformDSpheric
    #undef pfiGetTDFXformerSpheric
    #define pfiGetTDFXformerSpheric pfiGetTDFXformerDSpheric
    #undef pfiGetIXformSphericParameter
    #define pfiGetIXformSphericParameter pfiGetIXformDSphericParameter
    #undef pfiStopXformer
    #define pfiStopXformer pfiStopXformerD
    #undef pfiIXformSphericReadPathFile
    #define pfiIXformSphericReadPathFile pfiIXformDSphericReadPathFile
    #undef pfiIXformSphericSetWorld
    #define pfiIXformSphericSetWorld pfiIXformDSphericSetWorld
    #undef pfiGetXformerCoord
    #define pfiGetXformerCoord pfiGetXformerDCoord
    #undef pfiXformerCoord
    #define pfiXformerCoord pfiXformerDCoord
    #undef pfiIXformSphericPrintPathStuff
    #define pfiIXformSphericPrintPathStuff pfiIXformDSphericPrintPathStuff
    #undef pfiGetIXformSphericParameter
    #define pfiGetIXformSphericParameter pfiGetIXformDSphericParameter
    #undef pfiGetXformerMat
    #define pfiGetXformerMat pfiGetXformerDMat
    #undef pfiXformerMat
    #define pfiXformerMat pfiXformerDMat
    #undef pfiGetXformerModelMat
    #define pfiGetXformerModelMat pfiGetXformerDModelMat
    #undef pfiXformerModelMat
    #define pfiXformerModelMat pfiXformerDModelMat
    #undef pfiUpdateXformer
    #define pfiUpdateXformer pfiUpdateXformerD
    #undef pfiGetXformerCurModelIndex
    #define pfiGetXformerCurModelIndex pfiGetXformerDCurModelIndex
    #undef pfiGetXformerCurModel
    #define pfiGetXformerCurModel pfiGetXformerDCurModel
    #undef pfiGetIXformTravelClassType
    #define pfiGetIXformTravelClassType pfiGetIXformDTravelClassType
    #undef pfiGetIXformTrackballClassType
    #define pfiGetIXformTrackballClassType pfiGetIXformDTrackballClassType
    #undef pfiCenterXformer
    #define pfiCenterXformer pfiCenterXformerD
    #undef pfiSelectXformerModel
    #define pfiSelectXformerModel pfiSelectXformerDModel
    #undef pfiIXformBSphere
    #define pfiIXformBSphere pfiIXformDBSphere
    #undef pfiEnableXformerCollision
    #define pfiEnableXformerCollision pfiEnableXformerDCollision
    #undef pfiDisableXformerCollision
    #define pfiDisableXformerCollision pfiDisableXformerDCollision
    #undef pfiNewTDFXformer
    #define pfiNewTDFXformer pfiNewTDFXformerD
    #undef pfiXformerAutoInput
    #define pfiXformerAutoInput pfiXformerDAutoInput
    #undef pfiXformerNode
    #define pfiXformerNode pfiXformerDNode
    #undef pfiXformerAutoPosition
    #define pfiXformerAutoPosition pfiXformerDAutoPosition
    #undef pfiXformerResetCoord
    #define pfiXformerResetCoord pfiXformerDResetCoord
    #undef pfiTDFXformer
    #define pfiTDFXformer pfiTDFXformerD
    #undef pfiCollideXformer
    #define pfiCollideXformer pfiCollideXformerD
    #undef pfiResetXformer
    #define pfiResetXformer pfiResetXformerD

    #define pfChanView pfChanViewD
    #define pfCopyVec3 PFCOPY_VEC3
    /* NOT pfGetChanView! */

#endif /* CUSTOM */

#endif
