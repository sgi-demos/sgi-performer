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
 * file: pfiTrackball.C
 * ----------------------
 *
 * pfiTrackball motion model class routines
 *
 * $Revision: 1.11 $
 * $Date: 2002/11/21 01:34:12 $
 *
 */

#include <math.h>
#include <stdlib.h>
#include <Performer/prmath.h>

#define _PFI_XFORMER_C_
#include <Performer/pfui.h>
#include <Performer/pfui/pfiInputXform.h>
#include <Performer/pfui/pfiTrackball.h>


#ifdef WIN32
#define FUDGE_DLLEXPORT __declspec(dllexport)
#else
#define FUDGE_DLLEXPORT
#endif
extern "C" {
extern FUDGE_DLLEXPORT float _PFI_FUDGE; // XXX
}

/******************************************************************************
 *		Trackball Xformer Routines
 ******************************************************************************
 */


PFUIDLLEXPORT pfType *pfiInputXformTrackball::classType = NULL;

void
pfiInputXformTrackball::init()
{
    if (classType == NULL)
    {
	pfiInputXform::init();
	classType = 
	    new pfType(pfiInputXform::getClassType(), "pfiInputXformTrackball");
    }
}

pfiInputXformTrackball::pfiInputXformTrackball(void)
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
    xMotionCoord.startSpeed	= 30.0f;
    xMotionCoord.minSpeed	= 0.0f;
    xMotionCoord.maxSpeed	= 80.0f;
    xMotionCoord.angularVel	= 180.0f;
    xMotionCoord.startAccel	= xMotionCoord.accel = 1.0f;
    xMotionCoord.maxAccel	= 30.0f;

    reset();
}

void
pfiInputXformTrackball::reset(void)
{
    pfiInputXform::reset();
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
pfiInputXformTrackball::resetPosition(void)
{
    setCoord(&xMotionCoord.startPos);
}

void
pfiInputXformTrackball::resetRotation(void)
{
    xSpinMat.makeIdent();
    xPosMat.makeIdent();
}

void
pfiInputXformTrackball::setCoord(pfCoord *_coord)
{
    xHPRMat.makeEuler(_coord->hpr[0], _coord->hpr[1], _coord->hpr[2]);
    xHPRInvMat.invertOrtho(xHPRMat);
    PFSET_VEC3(xTranslation, 0.0f, 0.0f, 0.0f);
    resetRotation();
    xMotionCoord.setCoord(_coord);
}

void
pfiInputXformTrackball::setMat(pfMatrix& _mat)
{
    // trackball pos is independent of matrix so
    // don't make xform pos based on mat
    xMotionCoord.mat.copy(_mat);
}

void
pfiInputXformTrackball::stop(void)
{ 
    pfiInputXform::stop();
    xSpin			= 0.0f;
    xAccelMode			= PFIXTK_ACCEL_NONE;
    xPrevDeltaX = xPrevDeltaY	= -1.0f;
    xMotionCoord.startTime 	= -1.0f;
    xMotionMode			= PFIXTK_MOTION_STOP;
}

void
pfiInputXformTrackball::setFocus(int _focus)
{
    if (xFocus == _focus)
	return;
	
    xFocus = _focus;
    // this triggers the update() to set prevTime == Time
    xMotionCoord.startTime 	= -1;
    if (xFocus)
    {
	// if just got focus, through out old input
	xInputCoord->reset();
    }
}

void
pfiInputXformTrackball::setMode(int _mode, int _val)
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
		xMotionCoord.accel = 0.0f;
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
pfiInputXformTrackball::getMode(int _mode) const
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
pfiInputXformTrackball::_setBSphere(pfSphere* _sphere)
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
	setCoord(&xMotionCoord.startPos);
    }
    if  (!(xUserSetMask & _PFIX_USET_MOTION_LIMITS))
    {
	xMotionCoord.startSpeed = xDBSize*0.5f;
	xMotionCoord.maxSpeed = xDBSize*2.0f;
	if (xMotionCoord.minSpeed < 0.0f)
	    xMotionCoord.minSpeed = -xMotionCoord.maxSpeed;
    }
}
   
void		
pfiInputXformTrackball::setMotionLimits(float _maxSpeed, float _angularVel, float _accel)
{ 
    xMotionCoord.startSpeed = xMotionCoord.speed = xMotionCoord.maxSpeed =_maxSpeed;
    if (xMotionCoord.minSpeed < 0.0f)
	xMotionCoord.minSpeed = -xMotionCoord.maxSpeed;
    xMotionCoord.startAngVel = xMotionCoord.angularVel =  _angularVel;
    xMotionCoord.startAccel = xMotionCoord.accel = _accel; 
    xMotionCoord.maxAccel = 10.0f * _accel;
    xUserSetMask |= _PFIX_USET_MOTION_LIMITS;
}

void
pfiInputXformTrackball::getMat(pfMatrix& _mat)
{
    PFCOPY_MAT(_mat, xPosMat);
}

int
pfiUpdate2DIXformTrackball(pfiInputXform *_ix, pfiInputCoord *_ic, void *)
{
    pfiInputXformTrackball *_tb = (pfiInputXformTrackball *)_ix;
    pfi2DInputCoord *_icoord = (pfi2DInputCoord *)_ic;
    int			inputMotion;
    float		deltaTime = _tb->xMotionCoord.time - _tb->xMotionCoord.prevTime;
    float		deltax = _icoord->getDelta(PF_X);
    float		deltay = _icoord->getDelta(PF_Y);
    
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
	_tb->xMotionCoord.speed = 0.0f;
    }
    else
    {
	float           spinx;
	float           spiny;
	float           spinz;
	
	/* determine new acceleration and speed */
	// Do acceleration
	if (_tb->xInMotion)
	{
	    switch (_tb->xAccelMode)
	    {
	    case PFIXTK_ACCEL_INCREASE:
		if (_tb->xMotionCoord.accel <= _tb->xMotionCoord.startAccel)
		    _tb->xMotionCoord.accel = _tb->xMotionCoord.startAccel;
		_tb->xMotionCoord.accel += _tb->xAccelRate * deltaTime;
		break;
	    case PFIXTK_ACCEL_DECREASE:
		if (_tb->xMotionCoord.accel >= -_tb->xMotionCoord.startAccel)
		    _tb->xMotionCoord.accel = -_tb->xMotionCoord.startAccel;
		_tb->xMotionCoord.accel -= _tb->xAccelRate * deltaTime;
		break;
	    case PFIXTK_ACCEL_NONE:
		break;
	    }
	    _tb->xMotionCoord.speed += _tb->xMotionCoord.accel * deltaTime;
	    _tb->limitSpeed();
	}
			    
	if (!inputMotion && _tb->xAuto)
	{
	    deltax = _tb->xPrevDeltaX;
	    deltay = _tb->xPrevDeltaY;
	}

        _tb->xSpin = 0;

if (_PFI_FUDGE < 0)
{
    char *e = getenv("_PFI_FUDGE");
    _PFI_FUDGE = (e ? *e ? atof(e) : .1 : 1);
}

#define MIN_DELTA 0.00001f	
	switch(_tb->xMotionMode)
	{
	case PFIXTK_MOTION_ROTXZ:	
	    _tb->xSpin = 1;
	    if (PF_ABSGT(deltax,MIN_DELTA) || PF_ABSGT(deltay,MIN_DELTA))
	    {
		spinz =  deltax * _tb->xMotionCoord.angularVel * _PFI_FUDGE;
		spinx = -deltay * _tb->xMotionCoord.angularVel * _PFI_FUDGE;
		_tb->xSpinMat.postRot(_tb->xSpinMat, spinx, 1.0f, 0.0f, 0.0f);
		_tb->xSpinMat.postRot(_tb->xSpinMat, spinz, 0.0f, 0.0f, 1.0f);
	    }
	    break;

	case PFIXTK_MOTION_ROTY:	
	    _tb->xSpin = 1;
	    if (PF_ABSGT(deltax,MIN_DELTA) || PF_ABSGT(deltay,MIN_DELTA))
	    {
		float d;
		if (PF_ABSGT(deltax,MIN_DELTA));
		    d = deltax;
		if (PF_ABSGT(deltay,MIN_DELTA) && 
			(PF_ABS(deltay) > PF_ABS(deltax)))
		    d = deltay;
		spiny = d  * _tb->xMotionCoord.angularVel;
		_tb->xSpinMat.postRot(_tb->xSpinMat, spiny, 0.0f, 1.0f, 0.0f);
	    }
	    break;

	case PFIXTK_MOTION_TRANSXZ:
	    if (!_tb->xInMotion) /* set starting speed */
	    {
		_tb->xMotionCoord.speed = _tb->xMotionCoord.startSpeed;
	    }
	    _tb->xInMotion = 1;
	    if (PF_ABSGT(deltax,MIN_DELTA) || PF_ABSGT(deltay,MIN_DELTA))
	    {
		_tb->xTranslation[0] += deltax * _tb->xMotionCoord.speed * _PFI_FUDGE;
		_tb->xTranslation[2] += deltay * _tb->xMotionCoord.speed * _PFI_FUDGE;
	    }
	    break;

	case PFIXTK_MOTION_TRANSY:
	    if (!_tb->xInMotion) /* set starting speed */
	    {
		_tb->xMotionCoord.speed = _tb->xMotionCoord.startSpeed;
	    }
	    _tb->xInMotion = 1;
	    if (PF_ABSGT(deltay,MIN_DELTA))
	    {		
		_tb->xTranslation[1] += deltay * _tb->xMotionCoord.speed * _PFI_FUDGE;
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
    // pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Trackball mat pos: %f %f %f", 
    // _tb->xPosMat[3][0], _tb->xPosMat[3][1], _tb->xPosMat[3][2]);
    
    if (!_tb->xAuto || PF_ABSGT(deltax,MIN_DELTA) || PF_ABSGT(deltay,MIN_DELTA))
    {
	_tb->xPrevDeltaX = deltax;
	_tb->xPrevDeltaY = deltay;
    }
    return PFI_CB_CONT;
}


pfiInputXformTrackball *
pfiCreate2DIXformTrackball(void *arena)
{
    pfiInputXformTrackball *tb;
    
    tb = new(arena) pfiInputXformTrackball;
    tb->setInputCoordPtr(new(arena) pfi2DInputCoord);
    tb->setUpdateFunc(pfiUpdate2DIXformTrackball, NULL);
    return tb;
}
