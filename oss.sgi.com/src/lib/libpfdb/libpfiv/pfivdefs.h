/*
 * Copyright 1992, 1993, 1994, 1995, Silicon Graphics, Inc.
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
 * pfivdefs.h: $Revision: 1.1 $
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef	_POSIX_SOURCE
extern double trunc (double x);
#ifndef __linux__
struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};
#endif
#endif

#include <GL/glu.h>	/* currently must be included before SoShape.h */

#include <Inventor/SbLinear.h>
#include <Inventor/SbString.h>
#include <Inventor/SbColor.h>
#include <Inventor/SbPList.h>
#include <Inventor/SoDB.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoSwitchElement.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/nodes/SoNodes.h>
#include <Inventor/SoType.h>
#include <Inventor/SoInteraction.h>

#include "pfSoClasses.h"

#define _TYPES_SET_UP_
#define _LIGHTS_SET_UP_

#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfLight.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfSequence.h>
#include <Performer/pf/pfSwitch.h>
#include <Performer/pf/pfLOD.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pfdu.h>

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

/*-------------------------*/

#define NUM_START_ALLOC_BLOCK 16
#define MYPOINTS 	0
#define MYLINES		1
#define MYTRIS		2

#define PFIV_MAX_STACK	256

#ifndef PF_MAX_LIGHTS
#define PF_MAX_LIGHTS MAXLIGHTS
#endif

typedef struct 
{
    pfLight		*lights[PF_MAX_LIGHTS];
    int			localLightFlag;
    SoTexture2		*soTex;
    SoShapeHints 	*shapeHints;
    int			xformDirty;
    pfMatrix		currentXform;
} pfivState;

typedef struct cbdata 
{
    SoCallbackAction *action;
    void             *arena;
    pfGeode          *geode;
    pfGeoState       *geostate;
    pfGeoSet         *gset;
    pfGroup          *parent;
    SbPList          *parentStack;
    SoNode	     *node;
    pfdGeoBuilder    *builder;
    int		     primtype; /* LINES or TRIS or POINTS */
    int              numVerts;
    int              numPrims;
    int		     pbSize;
    pfdGeom	     *geom;
    int              normBind;
    int              colorBind;
    int              doTextures;
    pfMatStack	     *mstack, *invmstack;
    pfTexEnv	     *modTEnv, *decalTEnv;
    pfLightModel     *twoSideLM;
    pfMaterial	     *dftMtl;
    pfGeoState	     *dummyGState;
    pfMaterial	     *dummyMtl;
    pfTexture	     *dummyTex;
    pfdShare	     *share;
    pfivState	     stateStack[PFIV_MAX_STACK];
    int		     stateDepth;
    SoGetMatrixAction	*getMatrixAction;
    
} CbData;


