/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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
 * file: pfiSpheric.C
 * ----------------------
 *
 * pfiSpheric motion model class routines
 *
 * $Revision: 1.20 $
 * $Date: 2002/09/18 02:47:08 $
 *
 */

#include <math.h>
#include <stdlib.h>
#include <Performer/prmath.h>

#define _PFI_XFORMER_C_
#include <Performer/pfui.h>
#include <Performer/pfui/pfiInputXform.h>
#include <Performer/pfui/pfiSpheric.h>

typedef struct
{
  short                 numChildren;
  short                 hasIn;
  int                   children[PFIXSPH_MAX_CHILDREN];
  int                   parent;
  float                 minRadius, maxRadius;
  float                 farMinAround, farMaxAround;
  float                 nearMinAround, nearMaxAround;
  float                 farMinTilt, farMaxTilt;
  float                 nearMinTilt, nearMaxTilt;
  float                 farMinRoll, farMaxRoll;
  float                 nearMinRoll, nearMaxRoll;
  pfVec3                center;
  float                 inRadius[PFIXSPH_MAX_CHILDREN], outRadius;
  float                 inAround[PFIXSPH_MAX_CHILDREN], outAround;
  float                 inTilt[PFIXSPH_MAX_CHILDREN], outTilt;
  float                 farMaxLinearVelocity, farMaxAngularVelocity;
  float                 farMaxRollVelocity;
  float                 nearMaxLinearVelocity, nearMaxAngularVelocity;
  float                 nearMaxRollVelocity;
} PathInfo;

/******************************************************************************
 *		Spheric Xformer Routines
 ******************************************************************************
 */
 

PFUIDLLEXPORT pfType *pfiInputXformSpheric::classType = NULL;

float pfiInputXformSpheric::getParameter(int param) {
    switch(param) {
    case PFIXSPH_RADIUS:
	return radius;
    case PFIXSPH_MIN_RADIUS:
	return minRadius;
    case PFIXSPH_MAX_RADIUS:
	return maxRadius;

    case PFIXSPH_AROUND:
	return around;
    case PFIXSPH_NEAR_MIN_AROUND:
	return nearMinAround;
    case PFIXSPH_NEAR_MAX_AROUND:
	return nearMaxAround;
    case PFIXSPH_FAR_MIN_AROUND:
	return farMinAround;
    case PFIXSPH_FAR_MAX_AROUND:
	return farMaxAround;

    case PFIXSPH_TILT:
	return tilt;
    case PFIXSPH_NEAR_MIN_TILT:
	return nearMinTilt;
    case PFIXSPH_NEAR_MAX_TILT:
	return nearMaxTilt;
    case PFIXSPH_FAR_MIN_TILT:
	return farMinTilt;
    case PFIXSPH_FAR_MAX_TILT:
	return farMaxTilt;

    case PFIXSPH_ROLL:
	return roll;
    case PFIXSPH_NEAR_MIN_ROLL:
	return nearMinRoll;
    case PFIXSPH_NEAR_MAX_ROLL:
	return nearMaxRoll;
    case PFIXSPH_FAR_MIN_ROLL:
	return farMinRoll;
    case PFIXSPH_FAR_MAX_ROLL:
	return farMaxRoll;

    case PFIXSPH_NEAR_MAX_LINEAR_VELOCITY:
	return nearMaxLinearVelocity;
    case PFIXSPH_NEAR_MAX_ANGULAR_VELOCITY:
	return nearMaxAngularVelocity;
    case PFIXSPH_NEAR_MAX_ROLL_VELOCITY:
	return nearMaxRollVelocity;
    case PFIXSPH_FAR_MAX_LINEAR_VELOCITY:
	return farMaxLinearVelocity;
    case PFIXSPH_FAR_MAX_ANGULAR_VELOCITY:
	return farMaxAngularVelocity;
    case PFIXSPH_FAR_MAX_ROLL_VELOCITY:
	return farMaxRollVelocity;

    case PFIXSPH_CENTER_X:
	return center[PF_X];
    case PFIXSPH_CENTER_Y:
	return center[PF_Y];
    case PFIXSPH_CENTER_Z:
	return center[PF_Z];

    case PFIXSPH_INT_NUM_CHILDREN:
	return (float) numChildren;
    case PFIXSPH_INT_AT_RADIUS:
	return (float) atRadius;
    case PFIXSPH_INT_AT_AROUND:
	return (float) atAround;
    case PFIXSPH_INT_AT_TILT:
	return (float) atTilt;

    case PFIXSPH_INT_CHILD_NUM:
	return (float) childNum;

    case PFIXSPH_OUT_RADIUS:
	return outRadius;

    case PFIXSPH_IN_RADIUS_0:
    case PFIXSPH_IN_RADIUS_1:
	return inRadius[param - PFIXSPH_IN_RADIUS_0];

    case PFIXSPH_OUT_AROUND:
	return outAround;

    case PFIXSPH_IN_AROUND_0:
    case PFIXSPH_IN_AROUND_1:
	return inAround[param - PFIXSPH_IN_AROUND_0];

    case PFIXSPH_OUT_TILT:
	return outTilt;

    case PFIXSPH_IN_TILT_0:
    case PFIXSPH_IN_TILT_1:
	return inTilt[param - PFIXSPH_IN_TILT_0];

    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE, "unknown parameter - "
		 "pfiInputXformSpheric::getParameter(%i)", param);
    } // switch
    // XXX - is there a way to return NaN ??
    return 0.0f;
} // pfiInputXformSpheric::getParameter

void pfiInputXformSpheric::setParameter(int param, float num) {
    switch(param) {
    case PFIXSPH_RADIUS:
	radius = num;
	break;
    case PFIXSPH_MIN_RADIUS:
	minRadius = num;
	break;
    case PFIXSPH_MAX_RADIUS:
	maxRadius = num;
	break;

    case PFIXSPH_AROUND:
	around = num;
	break;
    case PFIXSPH_NEAR_MIN_AROUND:
	nearMinAround = num;
	break;
    case PFIXSPH_NEAR_MAX_AROUND:
	nearMaxAround = num;
	break;
    case PFIXSPH_FAR_MIN_AROUND:
	farMinAround = num;
	break;
    case PFIXSPH_FAR_MAX_AROUND:
	farMaxAround = num;
	break;

    case PFIXSPH_TILT:
	tilt = num;
	break;
    case PFIXSPH_NEAR_MIN_TILT:
	nearMinTilt = num;
	break;
    case PFIXSPH_NEAR_MAX_TILT:
	nearMaxTilt = num;
	break;
    case PFIXSPH_FAR_MIN_TILT:
	farMinTilt = num;
	break;
    case PFIXSPH_FAR_MAX_TILT:
	farMaxTilt = num;
	break;

    case PFIXSPH_ROLL:
	roll = num;
	break;
    case PFIXSPH_NEAR_MIN_ROLL:
	nearMinRoll = num;
	break;
    case PFIXSPH_NEAR_MAX_ROLL:
	nearMaxRoll = num;
	break;
    case PFIXSPH_FAR_MIN_ROLL:
	farMinRoll = num;
	break;
    case PFIXSPH_FAR_MAX_ROLL:
	farMaxRoll = num;
	break;

    case PFIXSPH_NEAR_MAX_LINEAR_VELOCITY:
	nearMaxLinearVelocity = num;
	break;
    case PFIXSPH_NEAR_MAX_ANGULAR_VELOCITY:
	nearMaxAngularVelocity = num;
	break;
    case PFIXSPH_NEAR_MAX_ROLL_VELOCITY:
	nearMaxRollVelocity = num;
	break;
    case PFIXSPH_FAR_MAX_LINEAR_VELOCITY:
	farMaxLinearVelocity = num;
	break;
    case PFIXSPH_FAR_MAX_ANGULAR_VELOCITY:
	farMaxAngularVelocity = num;
	break;
    case PFIXSPH_FAR_MAX_ROLL_VELOCITY:
	farMaxRollVelocity = num;
	break;

    case PFIXSPH_CENTER_X:
	center[PF_X] = num;
	break;
    case PFIXSPH_CENTER_Y:
	center[PF_Y] = num;
	break;
    case PFIXSPH_CENTER_Z:
	center[PF_Z] = num;
	break;

    case PFIXSPH_INT_NUM_CHILDREN:
	numChildren = (int) num;
	break;
    case PFIXSPH_INT_AT_RADIUS:
	atRadius = (int) num;
	break;
    case PFIXSPH_INT_AT_AROUND:
	atAround = (int) num;
	break;
    case PFIXSPH_INT_AT_TILT:
	atTilt = (int) num;
	break;

    case PFIXSPH_INT_CHILD_NUM:
	childNum = (int) childNum;
	break;

    case PFIXSPH_OUT_RADIUS:
	outRadius = num;
	break;

    case PFIXSPH_IN_RADIUS_0:
    case PFIXSPH_IN_RADIUS_1:
	inRadius[param - PFIXSPH_IN_RADIUS_0] = num;
	hasIn = TRUE;
	break;

    case PFIXSPH_OUT_AROUND:
	outAround = num;
	break;

    case PFIXSPH_IN_AROUND_0:
    case PFIXSPH_IN_AROUND_1:
	inAround[param - PFIXSPH_IN_AROUND_0] = num;
	hasIn = TRUE;
	break;

    case PFIXSPH_OUT_TILT:
	outTilt = num;
	break;

    case PFIXSPH_IN_TILT_0:
    case PFIXSPH_IN_TILT_1:
	inTilt[param - PFIXSPH_IN_TILT_0] = num;
	hasIn = TRUE;
	break;

    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE, "unknown parameter - "
		 "pfiInputXformSpheric::setParameter(%i)", param);
    } // switch
} // pfiInputXformSpheric::setParameter

void
pfiInputXformSpheric::init()
{
    if (classType == NULL)
    {
	pfiInputXform::init();
	classType = 
	    new pfType(pfiInputXformTravel::getClassType(), "pfiInputXformSpheric");
    }
}

 
pfiInputXformSpheric::pfiInputXformSpheric(void)
{
    setType(classType);

    xTracking = FALSE;

    radius = 200.0f;
    minRadius = 0.0f;
    maxRadius = 100.0f;
    around = 0.0f;
    nearMinAround = -180.0f;
    nearMaxAround = 180.0f;
    farMinAround = -180.0f;
    farMaxAround = 180.0f;
    tilt = 0.0f;
    nearMinTilt = -180.0f;
    nearMaxTilt = 180.0f;
    farMinTilt = -180.0f;
    farMaxTilt = 180.0f;
    roll = 0.0f;
    nearMinRoll = -180.0f;
    nearMaxRoll = 180.0f;
    farMinRoll = -180.0f;
    farMaxRoll = 180.0f;

    nearMaxLinearVelocity = 30.0f;
    nearMaxAngularVelocity = 30.0f;
    nearMaxRollVelocity = 30.0f;
    
    farMaxLinearVelocity = 30.0f;
    farMaxAngularVelocity = 30.0f;
    farMaxRollVelocity = 30.0f;

    pathList = new pfList;
    
    numChildren = 0;
    hasIn = FALSE;
    int i;
    for(i = 0; i < PFIXSPH_MAX_CHILDREN; i++)
	inRadius[i] = 0.0f;
    outRadius = 10.0f;
    for(i = 0; i < PFIXSPH_MAX_CHILDREN; i++)
	inAround[i] = 180.0f;
    outAround = 0.0f;
    for(i = 0; i < PFIXSPH_MAX_CHILDREN; i++)
	inTilt[i] = 0.0f;
    outTilt = 30.0f;

    // set up some path stuff
    numWorlds = -1;
    currentWorld = -1;
}

void
pfiInputXformSpheric::_setBSphere(pfSphere* _sphere)
{
    xDBSphere = *_sphere;
    xDBSize =  2.0f * xDBSphere.radius;
    xDBSizeLg = logf(xDBSize);
    if (!(xUserSetMask & _PFIX_USET_RESET_POS))	    // -- If user did not specify reset_pos -- //
    {
	PFCOPY_VEC3(xMotionCoord.startPos.xyz, xDBSphere.center);
	xMotionCoord.startPos.xyz[1] -= xDBSize*1.5f;
	xMotionCoord.startPos.xyz[2] += xDBSphere.radius*0.5f;
	PFSET_VEC3(xMotionCoord.startPos.hpr, 0.0f, 0.0f, 0.0f);
    }
    if  (!(xUserSetMask & _PFIX_USET_START_MOTION))
    {
	xMotionCoord.startAccel	= xMotionCoord.accel = PF_MAX2(5.0f, xDBSize*0.006f);
    }
    if  (!(xUserSetMask & _PFIX_USET_MOTION_LIMITS))
    {
	xMotionCoord.maxSpeed	= PF_MAX2(200.0f, xDBSize*5.0f);
	if (xMotionCoord.minSpeed < 0.0f)
	    xMotionCoord.minSpeed = -xMotionCoord.maxSpeed;
	xMotionCoord.maxAccel	= xDBSize*2.0f;
	if (xMotionCoord.minAccel < 0.0f)
	    xMotionCoord.minAccel = -xMotionCoord.maxAccel;
    }
    if (!(xUserSetMask & _PFIX_USET_SPHERIC_LIMITS))	// If users have not set spheric props
    {
	    // --- Should set up all global spheric params
	minRadius = 1.01f * xDBSphere.radius;
	maxRadius = 10.0f * xDBSphere.radius;
	radius = (minRadius + maxRadius)/2.0f;	

	//
	// Values that seemed all right to me at the time...
	// Choose minRadius to correspond to the data available (or make it 0).
	// maxLinearVelocity is always 1/20 HAT per update (i.e. per frame?)
	// maxAngularVelocity is always the same as maxLinearVelocity
	// (i.e. can achieve same max airspeed) when you're at the equator.
	// -DH
	//
	static float _PFI_SPHERIC_MIN_RADIUS = -1;
	static float _PFI_SPHERIC_MAX_RADIUS = -1;
	if (_PFI_SPHERIC_MIN_RADIUS == -1)
	{
	    char *e;
	    e = getenv("_PFI_SPHERIC_MIN_RADIUS");
	    _PFI_SPHERIC_MIN_RADIUS = (e ? atof(e) : 1.001f);
	    e = getenv("_PFI_SPHERIC_MAX_RADIUS");
	    _PFI_SPHERIC_MAX_RADIUS = (e ? atof(e) : 8.0f);
	    /*
	    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT,
		"_PFI_SPHERIC_MIN_RADIUS = %f times sphere radius",
		_PFI_SPHERIC_MIN_RADIUS);
	    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_PRINT,
		"_PFI_SPHERIC_MAX_RADIUS = %f times sphere radius",
		_PFI_SPHERIC_MAX_RADIUS);
	    */
	}
	hasIn = FALSE;
	minRadius = _PFI_SPHERIC_MIN_RADIUS * xDBSphere.radius;
	maxRadius = _PFI_SPHERIC_MAX_RADIUS * xDBSphere.radius;
	radius = (minRadius + maxRadius)/2.0f; // XXX ??? clobber prev value?
	nearMaxLinearVelocity = (minRadius - xDBSphere.radius) / 20;
	farMaxLinearVelocity = (maxRadius - xDBSphere.radius) / 20;
	nearMaxAngularVelocity = nearMaxLinearVelocity/xDBSphere.radius * 180/M_PI;
	farMaxAngularVelocity = farMaxLinearVelocity/xDBSphere.radius * 180/M_PI;
    }
}

    // PURPOSE: Makes parameters valid for current location
    //	        Use if coord suddenly jumps or changes. (Reposition, collision, etc.)
void pfiInputXformSpheric::recalc(void)
{
  float sinAround, cosAround;
  float sinTilt, cosTilt;

  // reset postion - generate radius, around, tilt - recalculate hpr...

  pfVec3 vec3 = xMotionCoord.pos.xyz;
  vec3 -= center;
      
  radius = vec3.length();
  tilt = pfArcSin(xMotionCoord.pos.xyz[PF_Z] /
			   radius);
  pfSinCos(tilt, &sinTilt, &cosTilt);
  around = pfArcSin(xMotionCoord.pos.xyz[PF_Y] /
			     (cosTilt * radius));
  roll = 0.0f;
  
  /* generate position and orientation */
  pfSinCos(around, &sinAround, &cosAround);
  pfSinCos(tilt, &sinTilt, &cosTilt);
  xMotionCoord.pos.xyz[PF_X] = cosAround * cosTilt * radius;
  xMotionCoord.pos.xyz[PF_Y] = sinAround * cosTilt * radius;
  xMotionCoord.pos.xyz[PF_Z] = sinTilt * radius;

  xMotionCoord.pos.hpr[PF_H] = around + 90.0f;
  xMotionCoord.pos.hpr[PF_P] = -tilt;
  xMotionCoord.pos.hpr[PF_R] = roll;

  xMotionCoord.makeMat();
}

    // PURPOSE: This actually calculates the new Coord
void pfiInputXformSpheric::generateLocation(void)
{
  float sinAround, cosAround;
  float sinTilt, cosTilt;

  /* generate position and orientation */
  pfSinCos(around, &sinAround, &cosAround);
  pfSinCos(tilt, &sinTilt, &cosTilt);
  xMotionCoord.pos.xyz[PF_X] = cosAround * cosTilt * radius;
  xMotionCoord.pos.xyz[PF_Y] = sinAround * cosTilt * radius;
  xMotionCoord.pos.xyz[PF_Z] = sinTilt * radius;

  xMotionCoord.pos.xyz += center;

  xMotionCoord.pos.hpr[PF_H] = around + 90.0f;
  xMotionCoord.pos.hpr[PF_P] = -tilt;
  xMotionCoord.pos.hpr[PF_R] = roll;

  xMotionCoord.makeMat();
}

void
pfiInputXformSpheric::resetPosition(void)
{
  radius = outRadius;
  around = outTilt;
  tilt = outTilt;

  xMotionCoord.prevPos = xMotionCoord.pos;

  generateLocation();
}

void
pfiInputXformSpheric::setMode(int _mode, int _val)
{
    switch(_mode)
    {
    case PFIXSPH_MODE_MOTION:
	if (xMotionMode == _val)
	    return;
	if (!xInMotion)
	    xMotionCoord.speed = xMotionCoord.startSpeed;
	xMotionMode = _val;
	break;
    case PFIXSPH_MODE_MOTION_MOD:
	xMotionMod = _val;
	break;
    case PFIXSPH_MODE_AUTO:
	xAuto = _val;
	break;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfiInputXformSpheric::setMode() - unknown mode %d", _mode);
	break;
    }
}

int
pfiInputXformSpheric::getMode(int _mode) const
{
    switch(_mode)
    {
    case PFIXSPH_MODE_MOTION:
	return xMotionMode;
    case PFIXSPH_MODE_MOTION_MOD:
	return xMotionMod;
    case PFIXSPH_MODE_AUTO:
	return xAuto;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfi2DInputXformSpheric::getMode() - unknown mode %d", _mode);
	return -1;
    }
}

void pfiInputXformSpheric::setWorld(int worldNumber, int in_or_out) {

  if ((worldNumber > numWorlds) || (worldNumber < 0))
    return;
    
#if 0
  if (worldNumber == (numWorlds)+1)
    worldNumber = 0;
  else if (worldNumber == -1)
    worldNumber = numWorlds;
#endif

  PathInfo *current = (PathInfo *) pathList->get(worldNumber);
  if (current == NULL) {
//    pfNotify(PFNFY_WARN, PFNFY_PRINT, "setWorld called with out of bounds world number %i\n", worldNumber);
    return;
  }

//   if (in_or_out == PFIXSPH_PATH_OUT)
//       pfNotify(PFNFY_INFO, PFNFY_PRINT, "setWorld %i OUT", worldNumber);
//   else
//       pfNotify(PFNFY_INFO, PFNFY_PRINT, "setWorld %i IN", worldNumber);

  int prevWorldNumber = currentWorld;
  currentWorld = worldNumber;

  if (in_or_out == PFIXSPH_PATH_OUT) {
      radius = current->outRadius;
      around = current->outAround;
      tilt = current->outTilt;
  }
  else {
      int childNum = 0;
      int i;
      for(i = 1; i < current->numChildren; i++)
	  if (current->children[i] == prevWorldNumber)
	      childNum = i;
      radius = current->inRadius[childNum];
      around = current->inAround[childNum];
      tilt = current->inTilt[childNum];
  }
  minRadius = current->minRadius;
  maxRadius = current->maxRadius;
  nearMinAround = current->nearMinAround;
  nearMaxAround = current->nearMaxAround;
  farMinAround = current->farMinAround;
  farMaxAround = current->farMaxAround;
  nearMinTilt = current->nearMinTilt;
  nearMaxTilt = current->nearMaxTilt;
  farMinTilt = current->farMinTilt;
  farMaxTilt = current->farMaxTilt;
  nearMinRoll = current->nearMinRoll;
  nearMaxRoll = current->nearMaxRoll;
  farMinRoll = current->farMinRoll;
  farMaxRoll = current->farMaxRoll;
  center = current->center;
  numChildren = current->numChildren;
  hasIn = current->hasIn;
  int i;
  for(i = 0; i < PFIXSPH_MAX_CHILDREN; i++)
      inRadius[i] = current->inRadius[i];
  outRadius = current->outRadius;
  for(i = 0; i < PFIXSPH_MAX_CHILDREN; i++)
      inAround[i] = current->inAround[i];
  outAround = current->outAround;
  for(i = 0; i < PFIXSPH_MAX_CHILDREN; i++)
      inTilt[i] = current->inTilt[i];
  outTilt = current->outTilt;

  nearMaxLinearVelocity = current->nearMaxLinearVelocity;
  nearMaxAngularVelocity = current->nearMaxAngularVelocity;
  nearMaxRollVelocity = current->nearMaxRollVelocity;
  farMaxLinearVelocity = current->farMaxLinearVelocity;
  farMaxAngularVelocity = current->farMaxAngularVelocity;
  farMaxRollVelocity = current->farMaxRollVelocity;
  
  generateLocation();
  xMotionCoord.prevPos = xMotionCoord.pos;
  xMotionCoord.startPos = xMotionCoord.pos;
//   ViewState->xformer->xMotionCoord.pos = xMotionCoord.pos;

//   // Pass data here to draw process - in main.C
//   if (current->blend) {
//       for (chani=0; chani < ViewState->numChans; chani++) {
// 	  pd[chani]->transitioning = 1;
// 	  ViewState->chan[chani]->passChanData();
//       }
//   }
//   else if (pd[0]->transitioning == 1) { // just changed back
//       for (chani=0; chani < ViewState->numChans; chani++) {
// 	  pd[chani]->transitioning = 0;
// 	  ViewState->chan[chani]->passChanData();
//       }
//   }
  generateLocation();
  xMotionCoord.prevPos = xMotionCoord.pos;
  xMotionCoord.startPos = xMotionCoord.pos;

} // setWorld

void pfiInputXformSpheric::readPathFile(char *filename) {
  int i, j;
  FILE *fp;
  char str[80];
  char tag[80];
  float fnum;
  int inum;
  PathInfo *current = NULL;
  int localNumWorlds = -1;
  int lines = 0;

  if (!(fp = fopen(filename, "r"))) {
    pfNotify(PFNFY_WARN, PFNFY_PRINT,
	     "spheric readPathFile couldn't open %s.\n", filename);
    return;
  }
  pfNotify(PFNFY_INFO, PFNFY_PRINT,
	     "spheric readPathFile reading %s...\n", filename);

  while (!feof(fp)) {
    fscanf(fp, "%[^\n]", str);
    fscanf(fp, "\n");
     lines++;
    sscanf(str, "%s %f", tag, &fnum);
    inum = (int) fnum;
    if (tag[0] == '#')
      /* comment - do nothing */ ;
    else if (strcmp(tag, "world") == 0) {
      if (current != NULL) 
	pathList->set(localNumWorlds, current);
      localNumWorlds++;
      if (localNumWorlds != inum) {
	  pfNotify(PFNFY_WARN, PFNFY_PRINT, "World order in file must start "
		   "at 0 and go up by one each world...");
	  pfExit();
      }
      current = (PathInfo *) pathList->get(localNumWorlds);
      if (current == NULL)
	current = (PathInfo *) pfMalloc( sizeof(PathInfo),
					 pfGetSharedArena() );
      int i;
      current->numChildren = 0;
      current->hasIn = FALSE;
      for (i = 0; i < PFIXSPH_MAX_CHILDREN; i++)
	  current->children[i] = -1;
      current->parent = -1;
      current->minRadius = 0.0f;
      current->maxRadius = 5000.0f;
      current->nearMinAround = -180.0f;
      current->nearMaxAround = 180.0f;
      current->farMinAround = -180.0f;
      current->farMaxAround = 180.0f;
      current->nearMinTilt = -180.0f;
      current->nearMaxTilt = 180.0f;
      current->farMinTilt = -180.0f;
      current->farMaxTilt = 180.0f;
      current->nearMinRoll = -180.0f;
      current->nearMaxRoll = 180.0f;
      current->farMinRoll = -180.0f;
      current->farMaxRoll = 180.0f;
      current->center.set(0.0f, 0.0f, 0.0f);
      for (i = 0; i < PFIXSPH_MAX_CHILDREN; i++)
	  current->inRadius[i] = 20.0f;
      current->outRadius = 5000.0f;
      for (i = 0; i < PFIXSPH_MAX_CHILDREN; i++)
	  current->inAround[i] = 0.0f;
      current->outAround = 0.0f;
      for (i = 0; i < PFIXSPH_MAX_CHILDREN; i++)
	  current->inTilt[i] = 0.0f;
      current->outTilt = 0.0f;
      current->nearMaxLinearVelocity = 1.0f;
      current->nearMaxAngularVelocity = 30.0f;
      current->nearMaxRollVelocity = 30.0f;
      current->farMaxLinearVelocity = 1.0f;
      current->farMaxAngularVelocity = 30.0f;
      current->farMaxRollVelocity = 30.0f;
    }
    else if (current == NULL) {
      pfNotify(PFNFY_FATAL, PFNFY_PRINT,
	       "First tag in paths must be <world>\n");
      pfExit();
    }
    else if (strcmp(tag, "child") == 0) {
	if (current->numChildren == PFIXSPH_MAX_CHILDREN)
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "Too many children (MAX %i)",
		     PFIXSPH_MAX_CHILDREN);
	else {
	    current->children[current->numChildren] = inum;
	    current->numChildren++;
	}
    }
    else if (strcmp(tag, "minRadius") == 0)
      current->minRadius = fnum;
    else if (strcmp(tag, "maxRadius") == 0)
      current->maxRadius = fnum;
    else if (strcmp(tag, "nearMinAround") == 0)
      current->nearMinAround = fnum;
    else if (strcmp(tag, "nearMaxAround") == 0)
      current->nearMaxAround = fnum;
    else if (strcmp(tag, "farMinAround") == 0)
      current->farMinAround = fnum;
    else if (strcmp(tag, "farMaxAround") == 0)
      current->farMaxAround = fnum;
    else if (strcmp(tag, "nearMinTilt") == 0)
      current->nearMinTilt = fnum;
    else if (strcmp(tag, "nearMaxTilt") == 0)
      current->nearMaxTilt = fnum;
    else if (strcmp(tag, "farMinTilt") == 0)
      current->farMinTilt = fnum;
    else if (strcmp(tag, "farMaxTilt") == 0)
      current->farMaxTilt = fnum;
    else if (strcmp(tag, "nearMinRoll") == 0)
      current->nearMinRoll = fnum;
    else if (strcmp(tag, "farMinRoll") == 0)
      current->farMinRoll = fnum;
    else if (strcmp(tag, "nearMaxRoll") == 0)
      current->nearMaxRoll = fnum;
    else if (strcmp(tag, "farMaxRoll") == 0)
      current->farMaxRoll = fnum;
    else if (strcmp(tag, "center") == 0)
      sscanf(str, "%s %f %f %f", tag, &current->center[PF_X],
	     &current->center[PF_Y], &current->center[PF_Z]);
    else if (strcmp(tag, "inRadius") == 0) {
      current->inRadius[0] = fnum;
      current->hasIn = TRUE;
    }
    else if (strcmp(tag, "inRadius1") == 0) {
      current->inRadius[1] = fnum;
      current->hasIn = TRUE;
    }
    else if (strcmp(tag, "outRadius") == 0)
      current->outRadius = fnum;
    else if (strcmp(tag, "inAround") == 0) {
      current->inAround[0] = fnum;
      current->hasIn = TRUE;
    }
    else if (strcmp(tag, "inAround1") == 0) {
      current->inAround[1] = fnum;
      current->hasIn = TRUE;
    }
    else if (strcmp(tag, "outAround") == 0)
      current->outAround = fnum;
    else if (strcmp(tag, "inTilt") == 0) {
      current->inTilt[0] = fnum;
      current->hasIn = TRUE;
    }
    else if (strcmp(tag, "inTilt1") == 0) {
      current->inTilt[1] = fnum;
      current->hasIn = TRUE;
    }
    else if (strcmp(tag, "outTilt") == 0)
      current->outTilt = fnum;
    else if (strcmp(tag, "nearMaxLinearVelocity") == 0)
      current->nearMaxLinearVelocity = fnum;
    else if (strcmp(tag, "farMaxLinearVelocity") == 0)
      current->farMaxLinearVelocity = fnum;
    else if (strcmp(tag, "nearMaxAngularVelocity") == 0)
      current->nearMaxAngularVelocity = fnum;
    else if (strcmp(tag, "farMaxAngularVelocity") == 0)
      current->farMaxAngularVelocity = fnum;
    else if (strcmp(tag, "nearMaxRollVelocity") == 0)
      current->nearMaxRollVelocity = fnum;
    else if (strcmp(tag, "farMaxRollVelocity") == 0)
      current->farMaxRollVelocity = fnum;
    else
      pfNotify(PFNFY_WARN, PFNFY_PRINT,
	       "Unknown tag <%s> in paths on line %i.", tag, lines);
  }
  fclose(fp);

  if (current != NULL)
    pathList->set(localNumWorlds, current);

  numWorlds = localNumWorlds;

  // manage tree stuff
  for(i = 0; i <= (numWorlds); i++) {
      current = (PathInfo *) pathList->get(i);
      if (current == NULL) {
	  pfNotify(PFNFY_WARN, PFNFY_PRINT, "pathList[%i] returned NULL\n", i);
	  pfExit();
      }
      for(j = 0; j < current->numChildren; j++) {
	  PathInfo *child = (PathInfo *) pathList->get(current->children[j]);
	  if (child == NULL) {
	      pfNotify(PFNFY_WARN, PFNFY_PRINT, "pathList[%i] returned NULL\n",
		       j);
	      pfExit();
	  }
	  child->parent = i;
      }
  }
  // can't go backwards from the root
  current = (PathInfo *) pathList->get(0);
  current->parent = -1;

  pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Done reading paths");
} // readPathFile

void pfiInputXformSpheric::printPathStuff(void) {
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Path stuff **********************\n");
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "numWorlds = %i\n", numWorlds);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "currentWorld = %i\n", currentWorld);

    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "center = %.12f %.12f %.12f\n", center[PF_X],
	     center[PF_Y], center[PF_Z]);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "radius = %.12f\n", radius);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "around = %.12f\n", around);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "tilt = %.12f\n", tilt);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "roll = %.12f\n", roll);

    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "min/max radius = %.12f / %.12f\n",
	     minRadius, maxRadius);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "far min/max around = %.12f / %.12f\n",
	     farMinAround, farMaxAround);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "near min/max around = %.12f / %.12f\n",
	     nearMinAround, nearMaxAround);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "far min/max tilt = %.12f / %.12f\n",
	     farMinTilt, farMaxTilt);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "near min/max tilt = %.12f / %.12f\n",
	     nearMinTilt, nearMaxTilt);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "far min/max roll = %.12f / %.12f\n",
	     farMinRoll, farMaxRoll);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "near min/max roll = %.12f / %.12f\n",
	     nearMinRoll, nearMaxRoll);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE,
	     "far/near max linear velocity = %.12f / %.12f\n",
	     farMaxLinearVelocity, nearMaxLinearVelocity);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE,
	     "far/near max angular velocity = %.12f / %.12f\n",
	     farMaxAngularVelocity, nearMaxAngularVelocity);
    pfNotify(PFNFY_NOTICE, PFNFY_MORE,
	     "far/near max roll velocity = %.12f / %.12f\n",
	     farMaxRollVelocity, nearMaxRollVelocity);

//     cerr << "xMotionCoord.pos.xyz = "
// 	 << xMotionCoord.pos.xyz[PF_X] << ", "
// 	 << xMotionCoord.pos.xyz[PF_Y] << ", "
// 	 << xMotionCoord.pos.xyz[PF_Z] << "\n";
}

int
pfiUpdate2DIXformSpheric(pfiInputXform *_ix, pfiInputCoord *_ic, void *)
{
    pfiInputXformSpheric *_spheric = (pfiInputXformSpheric *)_ix;

    if (_spheric->xUpdateMode != PFI_CB_CONT)
	return _spheric->xUpdateMode;

    if (_spheric->xMotionMode == PFIXSPH_MOTION_STOP)
    {
	_spheric->stop();
	_spheric->xTracking = FALSE;
    }
    else if (_spheric->xInMotion)
	return _spheric->doUpdate(_ic);

    return PFI_CB_CONT;
}

pfiInputXformSpheric *
pfiCreate2DIXformSpheric(void *arena)
{
    pfiInputXformSpheric *spheric;
    
    spheric = new(arena) pfiInputXformSpheric;
    spheric->setInputCoordPtr(new(arena) pfi2DInputCoord);
    spheric->setUpdateFunc(pfiUpdate2DIXformSpheric, NULL);
    return spheric;
}

int pfiInputXformSpheric::doUpdate(pfiInputCoord *_ic) {

    pfi2DInputCoord *_icoord = (pfi2DInputCoord *)_ic;
    float	    x, y;

    /* Apply quadratic-scaling to create deadened center */
    x = 0.5f - _icoord->getCoord(PF_X);
    x *= PF_ABS(x)*4.0f;
    y = 0.5f - _icoord->getCoord(PF_Y);
    y *= PF_ABS(y)*4.0f;

	// x and y should be between -1 and 1

    if (y > 1.0f)
	y = 1.0f;
    else if (y < -1.0f)
	y = -1.0;
    if (x > 1.0f)
	x = 1.0f;
    else if (x < -1.0f)
	x = -1.0f;

//	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "x = %f, y = %f", x, y);
    
    float percentage = (radius - minRadius) / 
(maxRadius - minRadius);
	
	// a(c) + b(1-c) == ac + b - bc == (a-b)*c + b
    float maxAround = (farMaxAround - nearMaxAround) * 
percentage + nearMaxAround;
    float minAround = (farMinAround - nearMinAround) * 
percentage + nearMinAround;
    float maxTilt = (farMaxTilt - nearMaxTilt) * 
percentage + nearMaxTilt;
    float minTilt = (farMinTilt - nearMinTilt) * 
percentage + nearMinTilt;
			    
    float maxLinearVelocity =
(farMaxLinearVelocity - nearMaxLinearVelocity) *
percentage + nearMaxLinearVelocity;
    float maxAngularVelocity =
(farMaxAngularVelocity -
 nearMaxAngularVelocity) *
percentage + nearMaxAngularVelocity;
			    
    if (xMotionMod & PFIXSPH_MOTION_MOD_SPIN)
    {
	float maxRoll = (farMaxRoll - nearMaxRoll) *
    percentage + nearMaxRoll;
	float minRoll = (farMinRoll - nearMinRoll) *
    percentage + nearMinRoll;
	float maxRollVelocity =
    (farMaxRollVelocity - nearMaxRollVelocity) *
    percentage + nearMaxRollVelocity;

	roll += x * maxRollVelocity;
	if (roll >= 180.0f)
	    roll -= 360.0f;
	else if (roll <= -180.0f)
	    roll += 360.0f;
	if (roll >= maxRoll)
	    roll = maxRoll;
	else if (roll <= minRoll)
	    roll = minRoll;
    }
    else {
	roll = 0.0f;
    }

    switch(xMotionMode)
    {
    case PFIXSPH_MOTION_INOUT:
	radius += y * maxLinearVelocity;

	if (radius >= maxRadius)
	    radius = maxRadius;
	else if (radius <= minRadius)
	    radius = minRadius;

	// stay inside the shrinking lines
	if (around >= maxAround)
	    around = maxAround;
	else if (around <= minAround)
	    around = minAround;
	    
	if (tilt >= maxTilt)
	    tilt = maxTilt;
	else if (tilt <= minTilt)
	    tilt = minTilt;

	generateLocation();
	break;

    case PFIXSPH_MOTION_AROUND:
	around += x * maxAngularVelocity;
	tilt += y * maxAngularVelocity;

	if (around >= 180.0f)
	    around -= 360.0f;
	else if (around <= -180.0f)
	    around += 360.0f;
	if (around >= maxAround)
	    around = maxAround;
	else if (around <= minAround)
	    around = minAround;

	if (tilt >= 180.0f)
	    tilt -= 360.0f;
	else if (tilt <= -180.0f)
	    tilt += 360.0f;
	if (tilt >= maxTilt)
	    tilt = maxTilt;
	else if (tilt <= minTilt)
	    tilt = minTilt;

	generateLocation();
	break;

    case PFIXSPH_MOTION_TRACK_INOUT:	// --- Goto area of interest --- //
	// don't go to an area of interest if there isn't one...
	if (!hasIn)
	    return PFI_CB_CONT;

	int lastTracking = xTracking;
	if (y > 0.0f)
	    xTracking = PFIXSPH_PATH_OUT;
	else
	    xTracking = PFIXSPH_PATH_IN;

	if ((lastTracking == xTracking) &&
	    (atRadius && atAround && atTilt)) {
	    PathInfo *current = (PathInfo *) pathList->get(currentWorld);
	    if (current)
		if (xTracking == PFIXSPH_PATH_IN)
		    setWorld(current->children[childNum], PFIXSPH_PATH_OUT);
		else
		    setWorld(current->parent, PFIXSPH_PATH_IN);
	}

	float mult;
	float rLen[PFIXSPH_MAX_CHILDREN], aLen[PFIXSPH_MAX_CHILDREN],
    tLen[PFIXSPH_MAX_CHILDREN];
	int i;

	if (y > 0.0f) {
	    for (i = 0; i < PF_MAX2(1,numChildren); i++) {
		rLen[i] = outRadius - radius;
		aLen[i] = outAround - around;
		tLen[i] = outTilt - tilt;
	    }
	}
	else {
	    for (i = 0; i < PF_MAX2(1,numChildren); i++) {
		rLen[i] = inRadius[i] - radius;
		aLen[i] = inAround[i] - around;
		tLen[i] = inTilt[i] - tilt;
	    }
	}
	    
	// pick the shortest distance around the circle
	// remember:  -180 and 180 are the same...
	for (i = 0; i < PF_MAX2(1,numChildren); i++) {
	    if (aLen[i] > 180.0f)
		aLen[i] -= 360.0f;
	    else if (aLen[i] < -180.0f)
		aLen[i] += 360.0f;
	    if (tLen[i] > 180.0f)
		tLen[i] -= 360.0f;
	    else if (tLen[i] < -180.0f)
		tLen[i] += 360.0f;
	}
	    
	float len = aLen[0] * aLen[0] + tLen[0] * tLen[0];
	childNum = 0;
	for (i = 1; i < numChildren; i++) {
	    float newlen = aLen[i] * aLen[i] + tLen[i] * tLen[i];
	    if (newlen < len) {
		newlen = len;
		childNum = i;
	    }
	}

	float scaledRLen =
    PF_ABS(rLen[childNum]/maxLinearVelocity);
	float scaledALen =
    PF_ABS(aLen[childNum]/maxAngularVelocity);
	float scaledTLen =
    PF_ABS(tLen[childNum]/maxAngularVelocity);

	float rStride = PF_ABS(y) * maxLinearVelocity;
	float aStride = PF_ABS(y) * maxAngularVelocity;
	float tStride = PF_ABS(y) * maxAngularVelocity;

	float longLen = scaledRLen;
	if (longLen < scaledALen)
	    longLen = scaledALen;
	if (longLen < scaledTLen)
	    longLen = scaledTLen;

	if (longLen > 0.0f) {
	    mult = scaledRLen/longLen;
	    if (mult < 1.0f)
		rStride *= mult;

	    mult = scaledALen/longLen;
	    if (mult < 1.0f)
		aStride *= mult;

	    mult = scaledTLen/longLen;
	    if (mult < 1.0f)
		tStride *= mult;
	}

	/**** radius ****/
	if (PF_ABS(rLen[childNum]) <= rStride) {
	    atRadius = TRUE;
	    if (y > 0.0f)
		radius = outRadius;
	    else
		radius = inRadius[childNum];
	}
	else {
	    atRadius = FALSE;
	    if (rLen[childNum] > 0.0f)
		radius += rStride;
	    else
		radius -= rStride;
	}

	/**** around ****/
	if (PF_ABS(aLen[childNum]) <= aStride) {
	    atAround = TRUE;
	    if (y > 0.0f)
		around = outAround;
	    else
		around = inAround[childNum];
	}
	else {
	    atAround = FALSE;
	    if (aLen[childNum] > 0.0f)
		around += aStride;
	    else
		around -= aStride;
	}

	/**** tilt ****/
	if (PF_ABS(tLen[childNum]) <= tStride) {
	    atTilt = TRUE;
	    if (y > 0.0f)
		tilt = outTilt;
	    else
		tilt = inTilt[childNum];
	}
	else {
	    atTilt = FALSE;
	    if (tLen[childNum] > 0.0f)
		tilt += tStride;
	    else
		tilt -= tStride;
	}

	xMotionCoord.pos.hpr[PF_R] = roll;
	generateLocation();

	/****  This works - which means that it currently does nothing with
	  rotation...
	  pfQuat quat;
	  pfMatrix rot;

	  xMotionCoord.mat.getOrthoQuat(quat);
	  rot.makeQuat(quat);
	  \
	  xMotionCoord.mat.postTrans(rot,
	  xMotionCoord.pos.xyz[PF_X],
	  xMotionCoord.pos.xyz[PF_Y],
	  xMotionCoord.pos.xyz[PF_Z]);
	  \
	  xMotionCoord.mat.getOrthoCoord(&xMotionCoord.pos);
	  ****/
	    
	break;
    }

    return PFI_CB_CONT;
} // pfiInputXformSpheric::doUpdate

