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
 * file: pfiDrive.C
 * ----------------------
 *
 *
 * pfiDrive motion model class routines
 *
 * $Revision: 1.11 $
 * $Date: 2002/09/18 02:47:08 $
 *
 */

#include <math.h>
#include <Performer/prmath.h>
#include <Performer/pr/pfLinMath.h>

#define _PFI_XFORMER_C_
#include <Performer/pfui.h>
#include <Performer/pfui/pfiInputXform.h>
#include <Performer/pfui/pfiDrive.h>



/******************************************************************************
 *		Drive Xform Routines
 ******************************************************************************
 */


PFUIDLLEXPORT pfType *pfiInputXformDrive::classType = NULL;

void
pfiInputXformDrive::init()
{
    if (classType == NULL)
    {
	pfiInputXformTravel::init();
	classType = 
	    new pfType(pfiInputXformTravel::getClassType(), "pfiInputXformDrive");
    }
}

pfiInputXformDrive::pfiInputXformDrive(void)
{
    setType(classType);
    xMotionCoord.startSpeed	= 10.0f;
    xMotionCoord.minSpeed	= 0.0f;
    xMotionCoord.maxSpeed	= 80.0f;
    xMotionCoord.angularVel	= 180.0f;
    xMotionCoord.startAccel	= xMotionCoord.accel = 1.0f;
    xMotionCoord.maxAccel	= 30.0f;
    xLimitPosMask		= PFIX_LIMIT_POS_BOTTOM;
    xResetDriveHeight = xDriveHeight = 2.0f;
}

void
pfiInputXformDrive::_setBSphere(pfSphere* _sphere)
{
    xDBSphere = *_sphere;
    xDBSize =  2.0f * xDBSphere.radius;
    xDBSizeLg = logf(xDBSize);
    if (!(xUserSetMask & _PFIX_USET_RESET_POS))
    {
	PFCOPY_VEC3(xMotionCoord.startPos.xyz, xDBSphere.center);
	xMotionCoord.startPos.xyz[1] -= xDBSize*1.5f;
	xMotionCoord.startPos.xyz[2] += xDBSphere.radius*0.5f;
	PFSET_VEC3(xMotionCoord.startPos.hpr, 0.0f, 0.0f, 0.0f);
    }
    if  (!(xUserSetMask & _PFIX_USET_START_MOTION))
    {
	xMotionCoord.startAccel	= xMotionCoord.accel = PF_MAX2(2.0f, xDBSize*0.0015f);
    }
    if  (!(xUserSetMask & _PFIX_USET_MOTION_LIMITS))
    {
	xMotionCoord.maxSpeed	= xDBSize*2.0f;
	if (xMotionCoord.minSpeed < 0.0f)
	    xMotionCoord.minSpeed = -xMotionCoord.maxSpeed;
	xMotionCoord.maxAccel	= xDBSize;
	if (xMotionCoord.minAccel < 0.0f)
	    xMotionCoord.minAccel = -xMotionCoord.maxAccel;
    }
}

void
pfiInputXformDrive::reset(void)
{
    pfiInputXformTravel::reset();
    resetDriveHeight();
    xMotionCoord.makeMat();
}

void
pfiInputXformDrive::resetPosition(void)
{
    setCoord(&xMotionCoord.startPos);
}

void
pfiInputXformDrive::setCoord(pfCoord *_coord)
{    
    xDriveHeight = xResetDriveHeight;
    xMotionCoord.setCoord(_coord);
}

int
pfiUpdate2DIXformDrive(pfiInputXform *_ix, pfiInputCoord *_ic, void *)
{
    pfiInputXformDrive *_drive = (pfiInputXformDrive *)_ix;
    pfi2DInputCoord *_icoord = (pfi2DInputCoord *)_ic;
    float	    deltaTime;
    float           cosHeading;
    float           sinHeading;
    float           cosPitch;
    float           sinPitch;
    float	    x;
    
    if (_drive->xUpdateMode != PFI_CB_CONT)
	return _drive->xUpdateMode;

    if (_drive->xMotionMode == PFIXDR_MOTION_STOP)
    {
	_drive->xInMotion = 0;
	_drive->xMotionCoord.speed = 0.0f;
	_drive->xAccelMode = PFIXDR_ACCEL_NONE; 
    }
    else
    {
	float absSpeed = PF_ABS(_drive->xMotionCoord.speed);
	float distance=0.0f;
	
	deltaTime = _drive->xMotionCoord.time - _drive->xMotionCoord.prevTime;
	if (deltaTime < 1e-10f)
	    deltaTime = 0.0f;

	if ((absSpeed < 1e-10f))
	{
	    absSpeed = _drive->xMotionCoord.speed = 0.0f;
	}
	
	// Do acceleration
	if (_drive->xInMotion)
	{
	    /* determine new acceleration and speed */
	    switch (_drive->xAccelMode)
	    {
	    case PFIXDR_ACCEL_INCREASE:
		if (_drive->xMotionCoord.accel <= _drive->xMotionCoord.startAccel)
		    _drive->xMotionCoord.accel = _drive->xMotionCoord.startAccel;
		_drive->xMotionCoord.accel += _drive->xAccelRate * deltaTime;
		break;
	    case PFIXDR_ACCEL_DECREASE:
		if (_drive->xMotionCoord.accel >= -_drive->xMotionCoord.startAccel)
		    _drive->xMotionCoord.accel = -_drive->xMotionCoord.startAccel;
		_drive->xMotionCoord.accel -= _drive->xAccelRate * deltaTime;
		break;
	    case PFIXDR_ACCEL_NONE:
		_drive->xMotionCoord.accel = 0.0f;
		break;
	    }
	    _drive->xMotionCoord.speed += _drive->xMotionCoord.accel * deltaTime;
	    _drive->limitSpeed();
	}
	
	/* Apply quadratic-scaling to create deadened center */
	x = 0.5f - _icoord->getCoord(PF_X);
	x *= PF_ABS(x);

	if (!(_drive->xMotionMod & PFIXDR_MOTION_MOD_VERTICAL))
	{
	    /* Mouse x steers vehicle */
	    _drive->xMotionCoord.pos.hpr[0] += x * _drive->xMotionCoord.angularVel * deltaTime;
	    pfSinCos(_drive->xMotionCoord.pos.hpr[0], &sinHeading, &cosHeading);
	    pfSinCos(_drive->xMotionCoord.pos.hpr[1], &sinPitch,   &cosPitch);
	    distance = _drive->xMotionCoord.speed * deltaTime;
	}

	// primary motion controls
	switch(_drive->xMotionMode)
	{
	case PFIXDR_MOTION_HEADING:
	    if (_drive->xInMotion)
	    {
		_drive->xInMotion = 0;
		_drive->pfiInputXform::stop();
		_drive->xAccelMode = PFIXDR_ACCEL_NONE;
	    }
	    
	    break;	
	case PFIXDR_MOTION_FORWARD:
	    _drive->xInMotion = 1;
	    if (absSpeed > 0.0f)  
	    {	
		_drive->xMotionCoord.pos.xyz[PF_X] -= distance * cosPitch * sinHeading;
		_drive->xMotionCoord.pos.xyz[PF_Y] += distance * cosPitch * cosHeading;
	    }
	    break;
	case PFIXDR_MOTION_REVERSE:
	    _drive->xInMotion = 1;
	    if (absSpeed > 0.0f)  
	    {
		_drive->xMotionCoord.pos.xyz[PF_X] += distance * cosPitch * sinHeading;
		_drive->xMotionCoord.pos.xyz[PF_Y] -= distance * cosPitch * cosHeading;
	    }
	    break;
	}
	// steady heading and pitch when we are moving
	if (_drive->xInMotion)
	{
	    _drive->xMotionCoord.pos.hpr[1] *= 0.95f;
	    _drive->xMotionCoord.pos.hpr[2] *= 0.95f;
	}
    }
    // additional motion modifiers
    if (_drive->xMotionMod & PFIXDR_MOTION_MOD_VERTICAL)
    {
	if (_drive->getFocus())
	{
	    float z = _drive->xMotionCoord.pos.xyz[2];
	    float y = 0.5f - _icoord->getCoord(PF_Y);
	    y *= PF_ABS(y);
	    z -= y * _drive->xDBSize * deltaTime;
	    _drive->setCurDriveHeight(z);
	}
    }
    _drive->xMotionCoord.makeMat();
    return PFI_CB_CONT;
}


pfiInputXformDrive *
pfiCreate2DIXformDrive(void *arena)
{
    pfiInputXformDrive *drive;
    
    drive = new(arena) pfiInputXformDrive;
    drive->setInputCoordPtr(new(arena) pfi2DInputCoord);
    drive->setUpdateFunc(pfiUpdate2DIXformDrive, NULL);
    return drive;
}
