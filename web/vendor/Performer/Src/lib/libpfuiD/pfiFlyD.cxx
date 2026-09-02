/*
 * WARNING! DO NOT EDIT!
 * pfiFlyD.C automatically generated from (../libpfui/pfiFly.C)
 */
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
 * file: pfiFlyD.C
 * ----------------------
 *
 * pfiFlyD motion model class routines
 *
 * $Revision: 1.11 $
 * $Date: 2002/11/21 01:34:12 $
 *
 */

#include <math.h>
#include <stdlib.h>
#include <Performer/prmath.h>

#define _PFI_XFORMER_C_
#include <Performer/pfuiD.h>
#include <Performer/pfuiD/pfiInputXformD.h>
#include <Performer/pfuiD/pfiFlyD.h>



#ifdef WIN32
#define FUDGE_DLLEXPORT __declspec(dllexport)
#else
#define FUDGE_DLLEXPORT
#endif
extern "C" {
extern FUDGE_DLLEXPORT double _PFI_FUDGE_D; // XXX
}

/******************************************************************************
 *		Fly XformerD Routines
 ******************************************************************************
 */
 

PFUIDLLEXPORT pfType *pfiInputXformDFly::classType = NULL;

void
pfiInputXformDFly::init()
{
    if (classType == NULL)
    {
	pfiInputXformDTravel::init();
	classType = 
	    new pfType(pfiInputXformDTravel::getClassType(), "pfiInputXformDFly");
    }
}

 
pfiInputXformDFly::pfiInputXformDFly(void)
{
    setType(classType);
    xPitchMode			= PFIXFLY_PITCH_FOLLOW;
    xMotionCoordD.startSpeed	= 15.0f;
    xMotionCoordD.maxSpeed	= 200.0f;
    xMotionCoordD.angularVel	= 180.0f;
    xMotionCoordD.startAccel     = xMotionCoordD.accel = 2.0f;
    xMotionCoordD.maxAccel	= 50.0f;
}

void
pfiInputXformDFly::_setBSphere(pfSphere* _sphere)
{
    xDBSphere = *_sphere;
    xDBSize =  2.0f * xDBSphere.radius;
    xDBSizeLg = logf(xDBSize);
    if (!(xUserSetMask & _PFIX_USET_RESET_POS))
    {
	PFCOPY_VEC3(xMotionCoordD.startPos.xyz, xDBSphere.center);
	xMotionCoordD.startPos.xyz[1] -= xDBSize*1.5f;
	xMotionCoordD.startPos.xyz[2] += xDBSphere.radius*0.5f;
	PFSET_VEC3(xMotionCoordD.startPos.hpr, 0.0f, 0.0f, 0.0f);
    }
    if  (!(xUserSetMask & _PFIX_USET_START_MOTION))
    {
	xMotionCoordD.startAccel	= xMotionCoordD.accel = PF_MAX2(5.0f, xDBSize*0.006f);
    }
    if  (!(xUserSetMask & _PFIX_USET_MOTION_LIMITS))
    {
	xMotionCoordD.maxSpeed	= PF_MAX2(200.0f, xDBSize*5.0f);
	if (xMotionCoordD.minSpeed < 0.0f)
	    xMotionCoordD.minSpeed = -xMotionCoordD.maxSpeed;
	xMotionCoordD.maxAccel	= xDBSize*2.0f;
	if (xMotionCoordD.minAccel < 0.0f)
	    xMotionCoordD.minAccel = -xMotionCoordD.maxAccel;
    }
}

void
pfiInputXformDFly::setMode(int _mode, int _val)
{
    switch(_mode)
    {
    case PFIXFLY_MODE_MOTION:
	if (xMotionMode == _val)
	    return;
	if (!xInMotion)
	    xMotionCoordD.speed = xMotionCoordD.startSpeed;
	xMotionMode = _val;
	break;
    case PFIXFLY_MODE_MOTION_MOD:
	xMotionMod = _val;
	break;
    case PFIXFLY_MODE_ACCEL:
	if (xAccelMode != _val)
	{
	    xAccelMode = _val;
		xMotionCoordD.accel = 0;
	}
	break;
    case PFIXFLY_MODE_AUTO:
	xAuto = _val;
	break;
    case PFIXFLY_MODE_PITCH:
	xPitchMode = _val;
	break;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfiInputXformDFly::setMode() - unknown mode %d", _mode);
	break;
    }
}

int
pfiInputXformDFly::getMode(int _mode) const
{
    switch(_mode)
    {
    case PFIXFLY_MODE_MOTION:
	return xMotionMode;
    case PFIXFLY_MODE_MOTION_MOD:
	return xMotionMod;
    case PFIXFLY_MODE_ACCEL:
	return xAccelMode;
    case PFIXFLY_MODE_PITCH:
	return xPitchMode;
    case PFIXFLY_MODE_AUTO:
	return xAuto;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfi2DInputXformDFly::getMode() - unknown mode %d", _mode);
	return -1;
    }
}

int
pfiUpdate2DIXformDFly(pfiInputXformD *_ix, pfiInputCoordD *_ic, void *)
{
    pfiInputXformDFly *_fly = (pfiInputXformDFly *)_ix;
    pfi2DInputCoordD *_icoord = (pfi2DInputCoordD *)_ic;
    double	    deltaTime;
    double           cosHeading;
    double           sinHeading;
    double           cosPitch;
    double           sinPitch;
    double	    x, y;
    
    if (_fly->xUpdateMode != PFI_CB_CONT)
	return _fly->xUpdateMode;


    if (_fly->xMotionMode == PFIXFLY_MOTION_STOP)
    {
	_fly->stop();
	_fly->xAccelMode = PFIXFLY_ACCEL_NONE; 
    }
    else
    {
	double absSpeed = PF_ABS(_fly->xMotionCoordD.speed);
	double distance=0.0f;
	
	deltaTime = _fly->xMotionCoordD.time - _fly->xMotionCoordD.prevTime;

	if ((absSpeed < 1e-10))
	{
	    absSpeed = _fly->xMotionCoordD.speed = 0.0f;
	}
	
	if (_fly->xInMotion)
	{
	    /* determine new acceleration and speed */
	    switch (_fly->xAccelMode)
	    {
	    case PFIXFLY_ACCEL_INCREASE:
		if (_fly->xMotionCoordD.accel <= _fly->xMotionCoordD.startAccel)
		    _fly->xMotionCoordD.accel = _fly->xMotionCoordD.startAccel;
		_fly->xMotionCoordD.accel += _fly->xAccelRate * deltaTime;
		break;
	    case PFIXFLY_ACCEL_DECREASE:
		if (_fly->xMotionCoordD.accel >= -_fly->xMotionCoordD.startAccel)
		    _fly->xMotionCoordD.accel = -_fly->xMotionCoordD.startAccel;
		_fly->xMotionCoordD.accel -= _fly->xAccelRate * deltaTime;
		break;
	    case PFIXFLY_ACCEL_NONE:
		_fly->xMotionCoordD.accel = 0.0f;
		break;
	    }
	    _fly->xMotionCoordD.speed += _fly->xMotionCoordD.accel * deltaTime;
	    _fly->limitSpeed();
	}
	
	/* Apply quadratic-scaling to create deadened center */
	x = 0.5f - _icoord->getCoord(PF_X);
	x *= PF_ABS(x);
	y = 0.5f - _icoord->getCoord(PF_Y);
	y *= PF_ABS(y);
    
        if (_fly->xPitchMode == PFIXFLY_PITCH_FLIGHT)
        {
            pfMatrix4d &m = _fly->xMotionCoordD.mat;
	    m.preRot(x*_fly->xMotionCoordD.angularVel*deltaTime, 0.0, -1.0, 0.0,m);
	    m.preRot(y*_fly->xMotionCoordD.angularVel*deltaTime*0.75, 1.0, 0.0, 0.0,m);

	    switch(_fly->xMotionMode)
	    {
	    case PFIXFLY_MOTION_HP:
	        if (_fly->xInMotion)
	        {
		    _fly->xInMotion = 0;
		    _fly->pfiInputXformD::stop();
	        }
	        break;	
	    case PFIXFLY_MOTION_FORWARD:	
	    case PFIXFLY_MOTION_REVERSE:
                _fly->xMotionCoordD.mat.preTrans(0.0,_fly->xMotionCoordD.speed * deltaTime, 0.0,m);
	        _fly->xInMotion = 1;
	        break;
	    }

            m.getOrthoCoordd(&_fly->xMotionCoordD.pos);

        }
        else
	{
            if (!(_fly->xMotionMod & PFIXFLY_MOTION_MOD_VERTICAL))
            {

	        /* Mouse x steers vehicle */
	        _fly->xMotionCoordD.pos.hpr[0] += x * _fly->xMotionCoordD.angularVel * deltaTime;
	        if (_fly->xPitchMode == PFIXFLY_PITCH_FLIGHT)
		    _fly->xMotionCoordD.pos.hpr[1] += y * _fly->xMotionCoordD.angularVel * deltaTime;
	        else
		    _fly->xMotionCoordD.pos.hpr[1] -= y * _fly->xMotionCoordD.angularVel * deltaTime;
	        if (absSpeed > 0.0f)  
	        {
		    pfSinCosd(_fly->xMotionCoordD.pos.hpr[0], &sinHeading, &cosHeading);
		    pfSinCosd(_fly->xMotionCoordD.pos.hpr[1], &sinPitch,   &cosPitch);
		    distance = _fly->xMotionCoordD.speed * deltaTime;
	        }
	    }
	    _fly->xMotionCoordD.pos.hpr[2] *= 0.9f;

            if (_PFI_FUDGE_D < 0)
            {
                char *e = getenv("_PFI_FUDGE_D");
                _PFI_FUDGE_D = (e ? *e ? atof(e) : .1 : 1);
            }
	    
	    switch(_fly->xMotionMode)
	    {
	    case PFIXFLY_MOTION_HP:
	        if (_fly->xInMotion)
	        {
		    _fly->xInMotion = 0;
		    _fly->pfiInputXformD::stop();
	        }
	        break;	
	    case PFIXFLY_MOTION_FORWARD:	
	        if (absSpeed > 0.0f)  
	        {
		    _fly->xMotionCoordD.pos.xyz[PF_X] -= distance * cosPitch * sinHeading * _PFI_FUDGE_D;
		    _fly->xMotionCoordD.pos.xyz[PF_Y] += distance * cosPitch * cosHeading * _PFI_FUDGE_D;
		    _fly->xMotionCoordD.pos.xyz[PF_Z] += distance * sinPitch * _PFI_FUDGE_D;
	        }
	        _fly->xInMotion = 1;
	        break;
	    case PFIXFLY_MOTION_REVERSE:
	        if (absSpeed > 0.0f)  
	        {
		    _fly->xMotionCoordD.pos.xyz[PF_X] += distance * cosPitch * sinHeading * _PFI_FUDGE_D;
		    _fly->xMotionCoordD.pos.xyz[PF_Y] -= distance * cosPitch * cosHeading * _PFI_FUDGE_D;
		    _fly->xMotionCoordD.pos.xyz[PF_Z] -= distance * sinPitch * _PFI_FUDGE_D;
	        }
	        _fly->xInMotion = 1;
	        break;
	    }
        }
    // additional motion modifiers
    if (_fly->xMotionMod & PFIXFLY_MOTION_MOD_VERTICAL)
    {
	if (_fly->getFocus())
	{
	    double y = 0.5f - _icoord->getCoord(PF_Y);
	    y *= PF_ABS(y);
	    _fly->xMotionCoordD.pos.xyz[PF_Z] -= y * _fly->xDBSize * deltaTime;
	}
    }

    }
    _fly->xMotionCoordD.makeMat();

    return PFI_CB_CONT;
}

pfiInputXformDFly *
pfiCreate2DIXformDFly(void *arena)
{
    pfiInputXformDFly *fly;
    
    fly = new(arena) pfiInputXformDFly;
    fly->setInputCoordDPtr(new(arena) pfi2DInputCoordD);
    fly->setUpdateFunc(pfiUpdate2DIXformDFly, NULL);
    return fly;
}
