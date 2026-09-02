/*
 * WARNING! DO NOT EDIT!
 * pfiTrackballD.C automatically generated from (../libpfui/pfiTrackball.C)
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
 * file: pfiTrackballD.C
 * ----------------------
 *
 * pfiTrackballD motion model class routines
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
#include <Performer/pfuiD/pfiTrackballD.h>


#ifdef WIN32
#define FUDGE_DLLEXPORT __declspec(dllexport)
#else
#define FUDGE_DLLEXPORT
#endif
extern "C" {
extern FUDGE_DLLEXPORT double _PFI_FUDGE_D; // XXX
}

/******************************************************************************
 *		Trackball XformerD Routines
 ******************************************************************************
 */


PFUIDLLEXPORT pfType *pfiInputXformDTrackball::classType = NULL;

void
pfiInputXformDTrackball::init()
{
    if (classType == NULL)
    {
	pfiInputXformD::init();
	classType = 
	    new pfType(pfiInputXformD::getClassType(), "pfiInputXformDTrackball");
    }
}

pfiInputXformDTrackball::pfiInputXformDTrackball(void)
{
    setType(classType);
    xSpeedScale			= 0.9f;
    xDBSize			= 1000.0f;
    xDBSizeLg			= 10.0f;	    
    xStartMotionCB		= NULL;
    xStartMotionCB		= NULL;
    xStopMotionCB		= NULL;
    xMotionCBData		= NULL;
    xPrevDeltaX = xPrevDeltaY	= -1.0f;
    xAuto			= 0;
    xMotionCoordD.startSpeed	= 30.0f;
    xMotionCoordD.minSpeed	= 0.0f;
    xMotionCoordD.maxSpeed	= 80.0f;
    xMotionCoordD.angularVel	= 180.0f;
    xMotionCoordD.startAccel	= xMotionCoordD.accel = 1.0f;
    xMotionCoordD.maxAccel	= 30.0f;

    reset();
}

void
pfiInputXformDTrackball::reset(void)
{
    pfiInputXformD::reset();
    resetPosition();
    PFSET_VEC3(xTranslation, 0.0f, 0.0f, 0.0f);
    xSpin  			= 0;
    xMotionMode			= PFIXTK_MOTION_STOP;
    xAccelMode			= PFIXTK_ACCEL_NONE;
    xAccelRate			= 0.0f;
    xHPRMat.makeIdent();
    xHPRInvMat.makeIdent();
    xPosMat.makeIdent();
}

void
pfiInputXformDTrackball::resetPosition(void)
{
    setCoord(&xMotionCoordD.startPos);
}

void
pfiInputXformDTrackball::resetRotation(void)
{
    xSpinMat.makeIdent();
    xPosMat.makeIdent();
}

void
pfiInputXformDTrackball::setCoord(pfCoordd *_coord)
{
    xHPRMat.makeEuler(_coord->hpr[0], _coord->hpr[1], _coord->hpr[2]);
    xHPRInvMat.invertOrtho(xHPRMat);
    PFSET_VEC3(xTranslation, 0.0f, 0.0f, 0.0f);
    resetRotation();
    xMotionCoordD.setCoord(_coord);
}

void
pfiInputXformDTrackball::setMat(pfMatrix4d& _mat)
{
    // trackball pos is independent of matrix so
    // don't make xform pos based on mat
    xMotionCoordD.mat.copy(_mat);
}

void
pfiInputXformDTrackball::stop(void)
{ 
    pfiInputXformD::stop();
    xSpin			= 0.0f;
    xAccelMode			= PFIXTK_ACCEL_NONE;
    xPrevDeltaX = xPrevDeltaY	= -1.0f;
    xMotionCoordD.startTime 	= -1.0f;
    xMotionMode			= PFIXTK_MOTION_STOP;
}

void
pfiInputXformDTrackball::setFocus(int _focus)
{
    if (xFocus == _focus)
	return;
	
    xFocus = _focus;
    // this triggers the update() to set prevTime == Time
    xMotionCoordD.startTime 	= -1;
    if (xFocus)
    {
	// if just got focus, through out old input
	xInputCoordD->reset();
    }
}

void
pfiInputXformDTrackball::setMode(int _mode, int _val)
{
    switch(_mode)
    {
    case PFIXTK_MODE_MOTION:
	xMotionMode = _val;
	break;
    case PFIXTK_MODE_ACCEL:
	if (xAccelMode != _val)
	{
	    xAccelMode = _val;
		xMotionCoordD.accel = 0.0f;
	}
	break;
    case PFIXTK_MODE_AUTO:
	xAuto = _val;
	break;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfi2DIXTrackballMode:: unknown mode %d", _mode);
	break;
    }
}

int
pfiInputXformDTrackball::getMode(int _mode) const
{
    switch(_mode)
    {
    case PFIXTK_MODE_MOTION:
	return xMotionMode;
    case PFIXTK_MODE_ACCEL:
	return xAccelMode;
    case PFIXTK_MODE_AUTO:
	return xAuto;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfi2DIXTrackballMode:: unknown mode %d", _mode);
	return -1;
    }
}

void
pfiInputXformDTrackball::_setBSphere(pfSphere* _sphere)
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
	setCoord(&xMotionCoordD.startPos);
    }
    if  (!(xUserSetMask & _PFIX_USET_MOTION_LIMITS))
    {
	xMotionCoordD.startSpeed = xDBSize*0.5f;
	xMotionCoordD.maxSpeed = xDBSize*2.0f;
	if (xMotionCoordD.minSpeed < 0.0f)
	    xMotionCoordD.minSpeed = -xMotionCoordD.maxSpeed;
    }
}
   
void		
pfiInputXformDTrackball::setMotionLimits(double _maxSpeed, double _angularVel, double _accel)
{ 
    xMotionCoordD.startSpeed = xMotionCoordD.speed = xMotionCoordD.maxSpeed =_maxSpeed;
    if (xMotionCoordD.minSpeed < 0.0f)
	xMotionCoordD.minSpeed = -xMotionCoordD.maxSpeed;
    xMotionCoordD.startAngVel = xMotionCoordD.angularVel =  _angularVel;
    xMotionCoordD.startAccel = xMotionCoordD.accel = _accel; 
    xMotionCoordD.maxAccel = 10.0f * _accel;
    xUserSetMask |= _PFIX_USET_MOTION_LIMITS;
}

void
pfiInputXformDTrackball::getMat(pfMatrix4d& _mat)
{
    PFCOPY_MAT(_mat, xPosMat);
}

int
pfiUpdate2DIXformDTrackball(pfiInputXformD *_ix, pfiInputCoordD *_ic, void *)
{
    pfiInputXformDTrackball *_tb = (pfiInputXformDTrackball *)_ix;
    pfi2DInputCoordD *_icoord = (pfi2DInputCoordD *)_ic;
    int			inputMotion;
    double		deltaTime = _tb->xMotionCoordD.time - _tb->xMotionCoordD.prevTime;
    double		deltax = _icoord->getDelta(PF_X);
    double		deltay = _icoord->getDelta(PF_Y);
    
    if (!_tb->getFocus() || deltaTime <= 1e-5f)
    {
	 deltax = deltay = deltaTime = 0.0f;
	 inputMotion = 0;
    }
    else
	inputMotion = _icoord->inMotion();

    if (_tb->xUpdateMode != PFI_CB_CONT)
	return _tb->xUpdateMode;
    
    if (_tb->xMotionMode == PFIXTK_MOTION_STOP)
    {
	_tb->xInMotion = 0;
	_tb->xMotionCoordD.speed = 0.0f;
    }
    else
    {
	double           spinx;
	double           spiny;
	double           spinz;
	
	/* determine new acceleration and speed */
	// Do acceleration
	if (_tb->xInMotion)
	{
	    switch (_tb->xAccelMode)
	    {
	    case PFIXTK_ACCEL_INCREASE:
		if (_tb->xMotionCoordD.accel <= _tb->xMotionCoordD.startAccel)
		    _tb->xMotionCoordD.accel = _tb->xMotionCoordD.startAccel;
		_tb->xMotionCoordD.accel += _tb->xAccelRate * deltaTime;
		break;
	    case PFIXTK_ACCEL_DECREASE:
		if (_tb->xMotionCoordD.accel >= -_tb->xMotionCoordD.startAccel)
		    _tb->xMotionCoordD.accel = -_tb->xMotionCoordD.startAccel;
		_tb->xMotionCoordD.accel -= _tb->xAccelRate * deltaTime;
		break;
	    case PFIXTK_ACCEL_NONE:
		break;
	    }
	    _tb->xMotionCoordD.speed += _tb->xMotionCoordD.accel * deltaTime;
	    _tb->limitSpeed();
	}
			    
	if (!inputMotion && _tb->xAuto)
	{
	    deltax = _tb->xPrevDeltaX;
	    deltay = _tb->xPrevDeltaY;
	}

        _tb->xSpin = 0;

if (_PFI_FUDGE_D < 0)
{
    char *e = getenv("_PFI_FUDGE_D");
    _PFI_FUDGE_D = (e ? *e ? atof(e) : .1 : 1);
}

#define MIN_DELTA 0.00001f	
	switch(_tb->xMotionMode)
	{
	case PFIXTK_MOTION_ROTXZ:	
	    _tb->xSpin = 1;
	    if (PF_ABSGT(deltax,MIN_DELTA) || PF_ABSGT(deltay,MIN_DELTA))
	    {
		spinz =  deltax * _tb->xMotionCoordD.angularVel * _PFI_FUDGE_D;
		spinx = -deltay * _tb->xMotionCoordD.angularVel * _PFI_FUDGE_D;
		_tb->xSpinMat.postRot(_tb->xSpinMat, spinx, 1.0f, 0.0f, 0.0f);
		_tb->xSpinMat.postRot(_tb->xSpinMat, spinz, 0.0f, 0.0f, 1.0f);
	    }
	    break;

	case PFIXTK_MOTION_ROTY:	
	    _tb->xSpin = 1;
	    if (PF_ABSGT(deltax,MIN_DELTA) || PF_ABSGT(deltay,MIN_DELTA))
	    {
		double d;
		if (PF_ABSGT(deltax,MIN_DELTA));
		    d = deltax;
		if (PF_ABSGT(deltay,MIN_DELTA) && 
			(PF_ABS(deltay) > PF_ABS(deltax)))
		    d = deltay;
		spiny = d  * _tb->xMotionCoordD.angularVel;
		_tb->xSpinMat.postRot(_tb->xSpinMat, spiny, 0.0f, 1.0f, 0.0f);
	    }
	    break;

	case PFIXTK_MOTION_TRANSXZ:
	    if (!_tb->xInMotion) /* set starting speed */
	    {
		_tb->xMotionCoordD.speed = _tb->xMotionCoordD.startSpeed;
	    }
	    _tb->xInMotion = 1;
	    if (PF_ABSGT(deltax,MIN_DELTA) || PF_ABSGT(deltay,MIN_DELTA))
	    {
		_tb->xTranslation[0] += deltax * _tb->xMotionCoordD.speed * _PFI_FUDGE_D;
		_tb->xTranslation[2] += deltay * _tb->xMotionCoordD.speed * _PFI_FUDGE_D;
	    }
	    break;

	case PFIXTK_MOTION_TRANSY:
	    if (!_tb->xInMotion) /* set starting speed */
	    {
		_tb->xMotionCoordD.speed = _tb->xMotionCoordD.startSpeed;
	    }
	    _tb->xInMotion = 1;
	    if (PF_ABSGT(deltay,MIN_DELTA))
	    {		
		_tb->xTranslation[1] += deltay * _tb->xMotionCoordD.speed * _PFI_FUDGE_D;
	    }
	    break;

	
	default:
	    break;
	}

	// compose final positioning matrix
	{
	    PFCOPY_MAT(_tb->xPosMat, _tb->xSpinMat);
	    // make object rotation viewer-relative
	    _tb->xPosMat.preMult(_tb->xHPRInvMat);
	    // apply new translation as viewer-relative
	    _tb->xPosMat.postTrans(_tb->xPosMat, 
		 _tb->xTranslation[0], 
		 _tb->xTranslation[1], 
		 _tb->xTranslation[2]);
            _tb->xPosMat.postMult(_tb->xHPRMat);
	    // center obj rotation 
	    _tb->xPosMat.preTrans( 
		-_tb->xDBSphere.center[0], 
		-_tb->xDBSphere.center[1], 
		-_tb->xDBSphere.center[2], 
		 _tb->xPosMat);
	    _tb->xPosMat.postTrans(_tb->xPosMat, 
		 _tb->xDBSphere.center[0], 
		 _tb->xDBSphere.center[1], 
		 _tb->xDBSphere.center[2]);
	    
	}

    }
    // pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Trackball mat pos: %lf %lf %lf", 
    // _tb->xPosMat[3][0], _tb->xPosMat[3][1], _tb->xPosMat[3][2]);
    
    if (!_tb->xAuto || PF_ABSGT(deltax,MIN_DELTA) || PF_ABSGT(deltay,MIN_DELTA))
    {
	_tb->xPrevDeltaX = deltax;
	_tb->xPrevDeltaY = deltay;
    }
    return PFI_CB_CONT;
}


pfiInputXformDTrackball *
pfiCreate2DIXformDTrackball(void *arena)
{
    pfiInputXformDTrackball *tb;
    
    tb = new(arena) pfiInputXformDTrackball;
    tb->setInputCoordDPtr(new(arena) pfi2DInputCoordD);
    tb->setUpdateFunc(pfiUpdate2DIXformDTrackball, NULL);
    return tb;
}
