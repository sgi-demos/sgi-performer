/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 */


/*
 * file: pfiInputXform.C
 * ----------------------
 *
 * Libpr-based event tranform utlities for building UI components
 *
 * $Revision: 1.50 $
 * $Date: 2002/11/20 23:01:46 $
 *
 */
 
#define PF_C_API 1 // enable C API also 

#include <math.h>
#include <Performer/prmath.h>
#include <Performer/pf.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfMemory.h>

#define __PFI_INPUTXFORM_C__
#include <Performer/pfui.h>
#include <Performer/pfui/pfiInputXform.h>

#if 0
static void
printMat(pfMatrix& mat)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "\t%f %f %f %f\n\t%f %f %f %f\n\t%f %f %f %f\n\t%f %f %f %f\n", 
        mat[0][0], mat[0][1], mat[0][2], mat[0][3], 
        mat[1][0], mat[1][1], mat[1][2], mat[1][3],
        mat[2][0], mat[2][1], mat[2][2], mat[2][3],
        mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}
#endif

/******************************************************************************
 *			    pfiInputCoord Routines
 ******************************************************************************
 */


PFUIDLLEXPORT pfType *pfiInputCoord::classType = NULL;

void
pfiInputCoord::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfiInputCoord");
    }
}

pfiInputCoord::pfiInputCoord(void) 
{
    _pfiClassInitCheck(); 
    setType(classType); 
    focus = 1; 
}

void	
pfiInputCoord::setVec(float *)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoord::setVec.");
}
void	
pfiInputCoord::setPrev(float *)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoord::setVec.");
}

void	
pfiInputCoord::getVec(float *) const
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoord::getVec.");
}
void	
pfiInputCoord::getPrev(float *) const
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoord::getPrev.");
}
float	
pfiInputCoord::getCoord(int) const
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoord::getCoord.");
    return -2.0f;
}
float	
pfiInputCoord::getPrevCoord(int) const
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoord::getPrevCoord.");
    return -2.0f;
}
float	
pfiInputCoord::getDelta(int) const
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoord::getDelta.");
    return -2.0f;
}
void	
pfiInputCoord::updatePrev(void)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoord::updatePrev.");
}

PFUIDLLEXPORT pfType *pfi2DInputCoord::classType = NULL;

void
pfi2DInputCoord::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfiInputCoord::getClassType(), "pfi2DInputCoord");
    }
}

pfi2DInputCoord::pfi2DInputCoord(void) 
{ 
    setType(classType); 
    reset(); 
}

/******************************************************************************
 *			    pfiMotionCoord Routines
 ******************************************************************************
 */



PFUIDLLEXPORT pfType *pfiMotionCoord::classType = NULL;

void
pfiMotionCoord::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfiMotionCoord");
    }
}

 
pfiMotionCoord::pfiMotionCoord(void)
{
    _pfiClassInitCheck();
    setType(classType);
    speed				=   0.0f;
    maxSpeed				=  30.0f;	/* 30 meters/sec */
    minSpeed				=   0.0f;
    startAngVel = angularVel		=  180.0f;	/* 180 degrees/sec */
    angAccel				=  0.0f;
    startAccel = accel			=  10.0f;	/* 10 meters/sec^2 == gravity */
    maxAccel				=  100.0f;
    minAccel				=  0.0f;
    startTime = prevTime = time		= -1.0f;
    pfSetVec3(startPos.xyz, 0.0f, 0.0f, 0.0f);
    pfSetVec3(startPos.hpr, 0.0f, 0.0f, 0.0f); 	 
    reset();
}

void
pfiMotionCoord::reset(void)
{
    startTime				= -1.0f;
    prevTime				= -1.0f;
    speed				=  0.0f;
    pfSetVec3(pos.xyz, 0.0f, 0.0f, 0.0f); 
    pfSetVec3(pos.hpr, 0.0f, 0.0f, 0.0f); 
    pfSetVec3(prevPos.xyz, 0.0f, 0.0f, 0.0f); 
    pfSetVec3(prevPos.hpr, 0.0f, 0.0f, 0.0f); 
    pfMakeCoordMat(mat, &pos);
}

void
pfiMotionCoord::resetPosition(void)
{
    speed				=  0.0f;
    startTime				= -1.0f;
    prevTime				= -1.0f;
    PFCOPY_VEC3(pos.xyz, startPos.xyz);
    PFCOPY_VEC3(pos.hpr, startPos.hpr);
    PFCOPY_VEC3(prevPos.xyz, pos.xyz); 
    PFCOPY_VEC3(prevPos.hpr, pos.hpr); 
    pfMakeCoordMat(mat, &pos);
}

void
pfiMotionCoord::copyPos(pfiMotionCoord *_src)
{
    startTime				= _src->startTime;
    prevTime				= _src->prevTime;
    time				= _src->time;
    PFCOPY_VEC3(pos.xyz, _src->pos.xyz); 
    PFCOPY_VEC3(pos.hpr, _src->pos.hpr); 
    PFCOPY_VEC3(prevPos.xyz, _src->prevPos.xyz); 
    PFCOPY_VEC3(prevPos.hpr, _src->prevPos.hpr); 
    PFCOPY_MAT(mat, _src->mat);
}

void
pfiMotionCoord::copyMotion(pfiMotionCoord *_src)
{
    speed				= _src->speed;
    maxSpeed				= _src->maxSpeed;
    startSpeed				= _src->startSpeed;
    startAngVel				= _src->startAngVel;
    angularVel				= _src->angularVel;
    maxAngVel				= _src->maxAngVel;
    startAccel				= _src->startAccel;
    accel				= _src->accel;
    maxAccel				= _src->maxAccel;
}

void
pfiMotionCoord::copy(pfiMotionCoord *_src)
{
    copyPos(_src);
    copyMotion(_src);
}

void
pfiMotionCoord::makeMat(void)
{
    /* XXXX Better way to do this? */
    if (pos.hpr[2]  > 180.0f)
	pos.hpr[2] -= 360.0f;
    else if (pos.hpr[2]  < -180.0f)
	pos.hpr[2] += 360.0f;

    if (pos.hpr[1]  > 180.0f)
	pos.hpr[1] -= 360.0f;
    else if (pos.hpr[1]  < -180.0f)
	pos.hpr[1] += 360.0f;

    while (pos.hpr[0] > 180.0f)
	pos.hpr[0] -= 360.0f;
    while (pos.hpr[0] < -180.0f)
	pos.hpr[0] += 360.0f;

    /* IMPORTANT: prevent FP underflows */
    if (PF_ABSLT(pos.hpr[0], 1.0e-7f))
	pos.hpr[0] = 0.0f;
    if (PF_ABSLT(pos.hpr[1], 1.0e-7f))
	pos.hpr[1] = 0.0f;
    if (PF_ABSLT(pos.hpr[2], 1.0e-7f))
	pos.hpr[2] = 0.0f;

    pfMakeCoordMat(mat, &pos);
}

void
pfiMotionCoord::setMat(pfMatrix& _mat)
{
    pfCopyMat(mat, _mat);
    pfGetOrthoMatCoord(_mat, &pos);
    prevPos = pos;
}

void
pfiMotionCoord::setCoord(pfCoord *_coord)
{
    prevPos = pos = *_coord;
    pfMakeCoordMat(mat, &pos);
}



/******************************************************************************
 *			    pfiInput Routines
 ******************************************************************************
 */

PFUIDLLEXPORT pfType *pfiInput::classType = NULL;

void
pfiInput::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfiInput");
    }
}


pfiInput::pfiInput(void)
{
    _pfiClassInitCheck();
    setType(classType);
    xEventMask			    = 0;
    xFocus			    = 1;
    xEventStream		    = NULL;
    xEventStreamCollector	    = NULL;
    xEventStreamProcessor	    = NULL;
}

pfiInput::~pfiInput(void)
{
}

void
pfiInput::reset(void)
{
    if (xEventStream)
	pfuResetEventStream(xEventStream);
}

void
pfiInput::setName(const char *_name)
{
    strcpy(xName, _name);
}



/******************************************************************************
 *			    pfiInputXform Routines
 ******************************************************************************
 */

#ifdef WIN32
#define FUDGE_EXPORT __declspec(dllexport)
#else
#define FUDGE_EXPORT
#endif
extern "C" {
FUDGE_EXPORT float _PFI_FUDGE = -1; // XXX global scale for all controls
}

PFUIDLLEXPORT pfType *pfiInputXform::classType = NULL;

void
pfiInputXform::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfiInputXform");
    }
}


pfiInputXform::pfiInputXform(void)
{
    _pfiClassInitCheck();
    setType(classType);
    xUserSetMask		    = 0x0;
    xFocus			    = 0x0;
    xInput			    = NULL;
    xInputCoord			    = NULL;
    xUpdateCB			    = NULL;
    xUpdateCBData		    = NULL;
    xStartMotionCB		    = NULL;
    xStopMotionCB		    = NULL;
    xMotionCBData		    = NULL;
    xUpdateMode			    = PFI_CB_CONT;
    pfSetVec3(xDBLimits.min, -PFI_BIGDB, -PFI_BIGDB, -PFI_BIGDB);
    pfSetVec3(xDBLimits.max,  PFI_BIGDB,  PFI_BIGDB,  PFI_BIGDB);
    xDBSphere.radius = xDBSize   = PFI_BIGDB;
    xDBSizeLg = 1.0f; // ???
    pfSetVec3(xDBSphere.center, 0.0f, 0.0f, 0.0f);
    xLimitPosMask		    = PFIX_LIMIT_POS_NONE;
    xLimitSpeedMask		    = PFIX_LIMIT_SPEED_MAX;
    xLimitAccelMask		    = PFIX_LIMIT_ACCEL_MAX;
    reset();
}

void
pfiInputXform::reset(void)
{
    xInMotion = 0;
    xMotionCoord.reset();
}

void
pfiInputXform::setName(const char *_name)
{
    strcpy(xName, _name);
}

void
pfiInputXform::resetPosition(void)
{
    xMotionCoord.resetPosition();
}

void
pfiInputXform::setMode(int _mode, int _val)
{
    switch(_mode)
    {
    case PFIX_MODE_MOTION:
	xMotionMode = _val;
	break;
    case PFIX_MODE_ACCEL:
	if (xAccelMode != _val)
	{
	    xAccelMode = _val;
		xMotionCoord.accel = 0;
	}
	break;
    case PFIXTK_MODE_AUTO:
	xAuto = _val;
	break;
    case PFIX_MODE_LIMIT_POS:
	xLimitPosMask = _val;
	break;
    case PFIX_MODE_LIMIT_SPEED:
	xLimitSpeedMask = _val;
	break;
    case PFIX_MODE_LIMIT_ACCEL:
	xLimitAccelMask = _val;
	break;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfiInputXform:: unknown mode %d", _mode);
	break;
    }
}

int
pfiInputXform::getMode(int _mode) const
{
    switch(_mode)
    {
    case PFIX_MODE_MOTION:
	return xMotionMode;
    case PFIX_MODE_ACCEL:
	return xAccelMode;
    case PFIXTK_MODE_AUTO:
	return xAuto;
    case PFIX_MODE_LIMIT_POS:
	return xLimitPosMask;
    case PFIX_MODE_LIMIT_SPEED:
	return xLimitSpeedMask;
    case PFIX_MODE_LIMIT_ACCEL:
	return xLimitAccelMask;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfiInputXform::getMode unknown mode %d", _mode);
	return -1;
    }
}

void
pfiInputXform::setMotionCoord(pfiMotionCoord *_xcoord)
{ 
    if (_xcoord)
	xMotionCoord.copy(_xcoord);
}

void
pfiInputXform::getMotionCoord(pfiMotionCoord *_xcoord)
{ 
    if (_xcoord)
	_xcoord->copy(&xMotionCoord);
}

void
pfiInputXform::setCenter(float _x, float _y, float _z)
{
    xDBSphere.center[0] = _x;
    xDBSphere.center[1] = _y;
    xDBSphere.center[2] = _z;
    xUserSetMask |= _PFIX_USET_BSPHERE;
}

void
pfiInputXform::setDBSize(float _size)
{
    xDBSize = _size;
    xDBSizeLg = logf(xDBSize);
    xUserSetMask |= _PFIX_USET_BSPHERE;
}


void
pfiInputXform::_setBSphere(pfSphere* _sphere)
{
    xDBSphere = *_sphere;
    xDBSize =  2.0f * xDBSphere.radius;
    xDBSizeLg = logf(xDBSize);
    if (!(xUserSetMask & _PFIX_USET_RESET_POS))
    {
	PFCOPY_VEC3(xMotionCoord.startPos.xyz, xDBSphere.center);
	xMotionCoord.startPos.xyz[PF_Y] -= xDBSize*1.5f;
	xMotionCoord.startPos.xyz[PF_Z] += xDBSphere.radius*0.5f;
	PFSET_VEC3(xMotionCoord.startPos.hpr, 0.0f, 0.0f, 0.0f);
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
pfiInputXform::_setDBLimits(pfBox* _dbLimits)
{
    pfSphere sphere;
    
    xDBLimits = *_dbLimits;
    
    // if user hasn't fix the sphere, keep the sphere as bound on db bounding box
    if (!(xUserSetMask & _PFIX_USET_BSPHERE))
    {
	pfSphereAroundBoxes(&sphere, (const pfBox **)&_dbLimits, 1);
	_setBSphere(&sphere);
    }
}
    
void		
pfiInputXform::setMotionLimits(float _maxSpeed, float _angularVel, float _accel)
{ 
    xMotionCoord.maxSpeed = _maxSpeed;
    if (xMotionCoord.minSpeed < 0.0f)
	    xMotionCoord.minSpeed = -xMotionCoord.maxSpeed;
    xMotionCoord.startAngVel = xMotionCoord.angularVel =  _angularVel;
    xMotionCoord.startAccel = xMotionCoord.accel = _accel; 
    if (xMotionCoord.minAccel == xMotionCoord.maxAccel)
	xMotionCoord.minAccel = -10.0f * _accel;
    xMotionCoord.maxAccel = 10.0f * _accel;
    xUserSetMask |= _PFIX_USET_MOTION_LIMITS;
}

void		
pfiInputXform::getMotionLimits(float *_maxSpeed, float *_angularVel, float *_accel) const
{ 
    *_maxSpeed = xMotionCoord.maxSpeed;
    *_angularVel = xMotionCoord.angularVel;
    *_accel = xMotionCoord.accel;
}

void
pfiInputXform::limitPos(void)
{
    if (xLimitPosMask & PFIX_LIMIT_POS_HORIZ)
    {
	if (xMotionCoord.pos.xyz[PF_X] < xDBLimits.min[PF_X])
	    xMotionCoord.pos.xyz[PF_X] = xDBLimits.min[PF_X];
	if (xMotionCoord.pos.xyz[PF_X] > xDBLimits.max[PF_X])
	    xMotionCoord.pos.xyz[PF_X] = xDBLimits.max[PF_X];
	    
	if (xMotionCoord.pos.xyz[PF_Y] < xDBLimits.min[PF_Y])
	    xMotionCoord.pos.xyz[PF_Y] = xDBLimits.min[PF_Y];
	if (xMotionCoord.pos.xyz[PF_Y] > xDBLimits.max[PF_Y])
	    xMotionCoord.pos.xyz[PF_Y] = xDBLimits.max[PF_Y];
    }
    
    if (xLimitPosMask & PFIX_LIMIT_POS_BOTTOM)
    {
	if (xMotionCoord.pos.xyz[PF_Z] < xDBLimits.min[PF_Z])
	    xMotionCoord.pos.xyz[PF_Z] = xDBLimits.min[PF_Z];
    }
    if (xLimitPosMask & PFIX_LIMIT_POS_TOP)
    {
	if (xMotionCoord.pos.xyz[PF_Z] > xDBLimits.max[PF_Z])
	    xMotionCoord.pos.xyz[PF_Z] = xDBLimits.max[PF_Z];
    }
}

void
pfiInputXform::limitSpeed(void)
{
    float s;

    /* Restrict speed if obeying max */
  
    if (xLimitSpeedMask & PFIX_LIMIT_SPEED_MAX)
    {
	if (xMotionCoord.speed > xMotionCoord.maxSpeed)
	    s = xMotionCoord.maxSpeed;
	else if (xMotionCoord.speed < xMotionCoord.minSpeed)
	    s = xMotionCoord.minSpeed;
	else
	    s = xMotionCoord.speed;
    }
    else
	s = xMotionCoord.speed;
	
    if (xLimitAccelMask & PFIX_LIMIT_ACCEL_MAX)
    {
	if (xMotionCoord.accel > xMotionCoord.maxAccel)
	    xMotionCoord.accel = xMotionCoord.maxAccel;
	if (xMotionCoord.accel < xMotionCoord.minAccel)
	    xMotionCoord.accel = xMotionCoord.minAccel;
    }
    
    /* Slow down if approaching limits of database */
    if (xLimitSpeedMask & PFIX_LIMIT_SPEED_DB)
    {
	pfBox	*box = &xDBLimits;
	if (!pfBoxContainsPt(box, xMotionCoord.pos.xyz))
	{
	    float 	dot, ch, sh;
	    pfVec2 	fromCenter;
    
	    /* XXX - Should clamp Z range in addition to X, Y */
    
	    fromCenter[0] = (xMotionCoord.pos.xyz[0] -
			     0.5f * (box->min[0] + box->max[0]));
				     
	    fromCenter[1] = (xMotionCoord.pos.xyz[1] -
			     0.5f * (box->min[1] + box->max[1]));
    
	    pfSinCos(xMotionCoord.pos.hpr[0], &sh, &ch);
				     
	    dot = fromCenter[1]*ch - fromCenter[0]*sh;
    
	    /* Slow down */
	    if (dot * s > 0.01f)
	    {
		s *= 0.96f;
	    }
	}
    }
    xMotionCoord.speed = s;     
}

void
pfiInputXform::update(void)
{
    xMotionCoord.prevTime	    = xMotionCoord.time;
    xMotionCoord.time		    = pfGetTime();
    if (xMotionCoord.startTime < 0.0f)
    {
	xMotionCoord.prevTime = xMotionCoord.startTime = xMotionCoord.time;
    }
    xMotionCoord.prevPos = xMotionCoord.pos;
    if (xUpdateCB)
	xUpdateMode = xUpdateCB(this, xInputCoord, xUpdateCBData);
    else
	xUpdateMode=PFI_CB_CONT;
}

void
pfiInputXform::setMat(pfMatrix& _mat)
{
    xMotionCoord.setMat(_mat);
}

void
pfiInputXform::setCoord(pfCoord *_coord)
{
    xMotionCoord.setCoord(_coord);
}

void
pfiInputXform::stop(void)
{
    xInMotion			= 0; 
    xMotionCoord.speed		= 0.0f;
}

/******************************************************************************
 *		Travel Xformer Routines
 ******************************************************************************
 */
 
 

PFUIDLLEXPORT pfType *pfiInputXformTravel::classType = NULL;

void
pfiInputXformTravel::init()
{
    if (classType == NULL)
    {
	pfiInputXform::init();
	classType = 
	    new pfType(pfiInputXform::getClassType(), "pfiInputXformTravel");
    }
}

pfiInputXformTravel::pfiInputXformTravel(void)
{
    setType(classType);
    xAuto			= 0;
    xMotionCoord.startSpeed	= 12.0f;
    xMotionCoord.minSpeed	= 0.0f;
    xMotionCoord.maxSpeed	= 40.0f;
    xMotionCoord.angularVel	= 90.0f;
    xMotionCoord.startAccel = xMotionCoord.accel = 1.25f;
    xMotionCoord.maxAccel	= 10.0f;
    xMotionCoord.minAccel	= 0.0f;
    reset();
}

void
pfiInputXformTravel::reset(void)
{
    pfiInputXform::reset();
    xMotionMode			= PFIXTR_MOTION_STOP;
    xMotionMod			= 0x0;
    xAccelMode			= PFIXTR_ACCEL_NONE;
    xAccelRate			= 0.0f;
}

void
pfiInputXformTravel::stop(void)
{ 
    pfiInputXform::stop();
    xAccelMode			= PFIXTR_ACCEL_NONE;
    xMotionMode			= PFIXTR_MOTION_STOP;
    xMotionMod			= 0x0;
}

void
pfiInputXformTravel::setMode(int _mode, int _val)
{
    switch(_mode)
    {
    case PFIXTR_MODE_MOTION:
	if (xMotionMode == _val)
	    return;
	if (!xInMotion)
	    xMotionCoord.speed = xMotionCoord.startSpeed;
	xMotionMode = _val;
	break;
    case PFIXTR_MODE_MOTION_MOD:
	xMotionMod = _val;
	break;
    case PFIXTR_MODE_ACCEL:
	if (xAccelMode != _val)
	{
	    xAccelMode = _val;
		xMotionCoord.accel = 0;
	}
	break;
    case PFIXTR_MODE_AUTO:
	xAuto = _val;
	break;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfiInputXformTravel::setMode() - unknown mode %d", _mode);
	break;
    }
}

int
pfiInputXformTravel::getMode(int _mode) const
{
    switch(_mode)
    {
    case PFIXTR_MODE_MOTION:
	return xMotionMode;
    case PFIXTR_MODE_MOTION_MOD:
	return xMotionMod;
    case PFIXTR_MODE_ACCEL:
	return xAccelMode;
    case PFIXTR_MODE_AUTO:
	return xAuto;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfiInputXformTravel::getMode() - unknown mode %d", _mode);
	return -1;
    }
}
