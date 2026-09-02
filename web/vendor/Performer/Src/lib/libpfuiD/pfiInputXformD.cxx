/*
 * WARNING! DO NOT EDIT!
 * pfiInputXformD.C automatically generated from (../libpfui/pfiInputXform.C)
 */
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
 * file: pfiInputXformD.C
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
#include <Performer/pfuiD.h>
#include <Performer/pfuiD/pfiInputXformD.h>

#if 0
static void
printMat(pfMatrix4d& mat)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "\t%lf %lf %lf %lf\n\t%lf %lf %lf %lf\n\t%lf %lf %lf %lf\n\t%lf %lf %lf %lf\n", 
        mat[0][0], mat[0][1], mat[0][2], mat[0][3], 
        mat[1][0], mat[1][1], mat[1][2], mat[1][3],
        mat[2][0], mat[2][1], mat[2][2], mat[2][3],
        mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}
#endif

/******************************************************************************
 *			    pfiInputCoordD Routines
 ******************************************************************************
 */


PFUIDLLEXPORT pfType *pfiInputCoordD::classType = NULL;

void
pfiInputCoordD::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfiInputCoordD");
    }
}

pfiInputCoordD::pfiInputCoordD(void) 
{
    _pfiClassInitCheck(); 
    setType(classType); 
    focus = 1; 
}

void	
pfiInputCoordD::setVec(double *)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoordD::setVec.");
}
void	
pfiInputCoordD::setPrev(double *)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoordD::setVec.");
}

void	
pfiInputCoordD::getVec(double *) const
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoordD::getVec.");
}
void	
pfiInputCoordD::getPrev(double *) const
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoordD::getPrev.");
}
double	
pfiInputCoordD::getCoord(int) const
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoordD::getCoord.");
    return -2.0f;
}
double	
pfiInputCoordD::getPrevCoord(int) const
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoordD::getPrevCoord.");
    return -2.0f;
}
double	
pfiInputCoordD::getDelta(int) const
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoordD::getDelta.");
    return -2.0f;
}
void	
pfiInputCoordD::updatePrev(void)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiInputCoordD::updatePrev.");
}

PFUIDLLEXPORT pfType *pfi2DInputCoordD::classType = NULL;

void
pfi2DInputCoordD::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfiInputCoordD::getClassType(), "pfi2DInputCoordD");
    }
}

pfi2DInputCoordD::pfi2DInputCoordD(void) 
{ 
    setType(classType); 
    reset(); 
}

/******************************************************************************
 *			    pfiMotionCoordD Routines
 ******************************************************************************
 */



PFUIDLLEXPORT pfType *pfiMotionCoordD::classType = NULL;

void
pfiMotionCoordD::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfiMotionCoordD");
    }
}

 
pfiMotionCoordD::pfiMotionCoordD(void)
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
    PFSET_VEC3(startPos.xyz, 0.0f, 0.0f, 0.0f);
    PFSET_VEC3(startPos.hpr, 0.0f, 0.0f, 0.0f); 	 
    reset();
}

void
pfiMotionCoordD::reset(void)
{
    startTime				= -1.0f;
    prevTime				= -1.0f;
    speed				=  0.0f;
    PFSET_VEC3(pos.xyz, 0.0f, 0.0f, 0.0f); 
    PFSET_VEC3(pos.hpr, 0.0f, 0.0f, 0.0f); 
    PFSET_VEC3(prevPos.xyz, 0.0f, 0.0f, 0.0f); 
    PFSET_VEC3(prevPos.hpr, 0.0f, 0.0f, 0.0f); 
    pfMakeCoorddMat4d(mat, &pos);
}

void
pfiMotionCoordD::resetPosition(void)
{
    speed				=  0.0f;
    startTime				= -1.0f;
    prevTime				= -1.0f;
    PFCOPY_VEC3(pos.xyz, startPos.xyz);
    PFCOPY_VEC3(pos.hpr, startPos.hpr);
    PFCOPY_VEC3(prevPos.xyz, pos.xyz); 
    PFCOPY_VEC3(prevPos.hpr, pos.hpr); 
    pfMakeCoorddMat4d(mat, &pos);
}

void
pfiMotionCoordD::copyPos(pfiMotionCoordD *_src)
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
pfiMotionCoordD::copyMotion(pfiMotionCoordD *_src)
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
pfiMotionCoordD::copy(pfiMotionCoordD *_src)
{
    copyPos(_src);
    copyMotion(_src);
}

void
pfiMotionCoordD::makeMat(void)
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

    pfMakeCoorddMat4d(mat, &pos);
}

void
pfiMotionCoordD::setMat(pfMatrix4d& _mat)
{
    pfCopyMat4d(mat, _mat);
    pfGetOrthoMat4dCoordd(_mat, &pos);
    prevPos = pos;
}

void
pfiMotionCoordD::setCoord(pfCoordd *_coord)
{
    prevPos = pos = *_coord;
    pfMakeCoorddMat4d(mat, &pos);
}



/******************************************************************************
 *			    pfiInputD Routines
 ******************************************************************************
 */

PFUIDLLEXPORT pfType *pfiInputD::classType = NULL;

void
pfiInputD::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfiInputD");
    }
}


pfiInputD::pfiInputD(void)
{
    _pfiClassInitCheck();
    setType(classType);
    xEventMask			    = 0;
    xFocus			    = 1;
    xEventStream		    = NULL;
    xEventStreamCollector	    = NULL;
    xEventStreamProcessor	    = NULL;
}

pfiInputD::~pfiInputD(void)
{
}

void
pfiInputD::reset(void)
{
    if (xEventStream)
	pfuResetEventStream(xEventStream);
}

void
pfiInputD::setName(const char *_name)
{
    strcpy(xName, _name);
}



/******************************************************************************
 *			    pfiInputXformD Routines
 ******************************************************************************
 */

#ifdef WIN32
#define FUDGE_EXPORT __declspec(dllexport)
#else
#define FUDGE_EXPORT
#endif
extern "C" {
FUDGE_EXPORT double _PFI_FUDGE_D = -1; // XXX global scale for all controls
}

PFUIDLLEXPORT pfType *pfiInputXformD::classType = NULL;

void
pfiInputXformD::init()
{
    if (classType == NULL)
    {
	pfObject::init();
	classType = 
	    new pfType(pfObject::getClassType(), "pfiInputXformD");
    }
}


pfiInputXformD::pfiInputXformD(void)
{
    _pfiClassInitCheck();
    setType(classType);
    xUserSetMask		    = 0x0;
    xFocus			    = 0x0;
    xInputD			    = NULL;
    xInputCoordD			    = NULL;
    xUpdateCB			    = NULL;
    xUpdateCBData		    = NULL;
    xStartMotionCB		    = NULL;
    xStopMotionCB		    = NULL;
    xMotionCBData		    = NULL;
    xUpdateMode			    = PFI_CB_CONT;
    PFSET_VEC3(xDBLimits.min, -PFI_BIGDB, -PFI_BIGDB, -PFI_BIGDB);
    PFSET_VEC3(xDBLimits.max,  PFI_BIGDB,  PFI_BIGDB,  PFI_BIGDB);
    xDBSphere.radius = xDBSize   = PFI_BIGDB;
    xDBSizeLg = 1.0f; // ???
    PFSET_VEC3(xDBSphere.center, 0.0f, 0.0f, 0.0f);
    xLimitPosMask		    = PFIX_LIMIT_POS_NONE;
    xLimitSpeedMask		    = PFIX_LIMIT_SPEED_MAX;
    xLimitAccelMask		    = PFIX_LIMIT_ACCEL_MAX;
    reset();
}

void
pfiInputXformD::reset(void)
{
    xInMotion = 0;
    xMotionCoordD.reset();
}

void
pfiInputXformD::setName(const char *_name)
{
    strcpy(xName, _name);
}

void
pfiInputXformD::resetPosition(void)
{
    xMotionCoordD.resetPosition();
}

void
pfiInputXformD::setMode(int _mode, int _val)
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
		xMotionCoordD.accel = 0;
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
	    "pfiInputXformD:: unknown mode %d", _mode);
	break;
    }
}

int
pfiInputXformD::getMode(int _mode) const
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
	    "pfiInputXformD::getMode unknown mode %d", _mode);
	return -1;
    }
}

void
pfiInputXformD::setMotionCoordD(pfiMotionCoordD *_xcoord)
{ 
    if (_xcoord)
	xMotionCoordD.copy(_xcoord);
}

void
pfiInputXformD::getMotionCoordD(pfiMotionCoordD *_xcoord)
{ 
    if (_xcoord)
	_xcoord->copy(&xMotionCoordD);
}

void
pfiInputXformD::setCenter(double _x, double _y, double _z)
{
    xDBSphere.center[0] = _x;
    xDBSphere.center[1] = _y;
    xDBSphere.center[2] = _z;
    xUserSetMask |= _PFIX_USET_BSPHERE;
}

void
pfiInputXformD::setDBSize(double _size)
{
    xDBSize = _size;
    xDBSizeLg = logf(xDBSize);
    xUserSetMask |= _PFIX_USET_BSPHERE;
}


void
pfiInputXformD::_setBSphere(pfSphere* _sphere)
{
    xDBSphere = *_sphere;
    xDBSize =  2.0f * xDBSphere.radius;
    xDBSizeLg = logf(xDBSize);
    if (!(xUserSetMask & _PFIX_USET_RESET_POS))
    {
	PFCOPY_VEC3(xMotionCoordD.startPos.xyz, xDBSphere.center);
	xMotionCoordD.startPos.xyz[PF_Y] -= xDBSize*1.5f;
	xMotionCoordD.startPos.xyz[PF_Z] += xDBSphere.radius*0.5f;
	PFSET_VEC3(xMotionCoordD.startPos.hpr, 0.0f, 0.0f, 0.0f);
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
pfiInputXformD::_setDBLimits(pfBox* _dbLimits)
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
pfiInputXformD::setMotionLimits(double _maxSpeed, double _angularVel, double _accel)
{ 
    xMotionCoordD.maxSpeed = _maxSpeed;
    if (xMotionCoordD.minSpeed < 0.0f)
	    xMotionCoordD.minSpeed = -xMotionCoordD.maxSpeed;
    xMotionCoordD.startAngVel = xMotionCoordD.angularVel =  _angularVel;
    xMotionCoordD.startAccel = xMotionCoordD.accel = _accel; 
    if (xMotionCoordD.minAccel == xMotionCoordD.maxAccel)
	xMotionCoordD.minAccel = -10.0f * _accel;
    xMotionCoordD.maxAccel = 10.0f * _accel;
    xUserSetMask |= _PFIX_USET_MOTION_LIMITS;
}

void		
pfiInputXformD::getMotionLimits(double *_maxSpeed, double *_angularVel, double *_accel) const
{ 
    *_maxSpeed = xMotionCoordD.maxSpeed;
    *_angularVel = xMotionCoordD.angularVel;
    *_accel = xMotionCoordD.accel;
}

void
pfiInputXformD::limitPos(void)
{
    if (xLimitPosMask & PFIX_LIMIT_POS_HORIZ)
    {
	if (xMotionCoordD.pos.xyz[PF_X] < xDBLimits.min[PF_X])
	    xMotionCoordD.pos.xyz[PF_X] = xDBLimits.min[PF_X];
	if (xMotionCoordD.pos.xyz[PF_X] > xDBLimits.max[PF_X])
	    xMotionCoordD.pos.xyz[PF_X] = xDBLimits.max[PF_X];
	    
	if (xMotionCoordD.pos.xyz[PF_Y] < xDBLimits.min[PF_Y])
	    xMotionCoordD.pos.xyz[PF_Y] = xDBLimits.min[PF_Y];
	if (xMotionCoordD.pos.xyz[PF_Y] > xDBLimits.max[PF_Y])
	    xMotionCoordD.pos.xyz[PF_Y] = xDBLimits.max[PF_Y];
    }
    
    if (xLimitPosMask & PFIX_LIMIT_POS_BOTTOM)
    {
	if (xMotionCoordD.pos.xyz[PF_Z] < xDBLimits.min[PF_Z])
	    xMotionCoordD.pos.xyz[PF_Z] = xDBLimits.min[PF_Z];
    }
    if (xLimitPosMask & PFIX_LIMIT_POS_TOP)
    {
	if (xMotionCoordD.pos.xyz[PF_Z] > xDBLimits.max[PF_Z])
	    xMotionCoordD.pos.xyz[PF_Z] = xDBLimits.max[PF_Z];
    }
}

void
pfiInputXformD::limitSpeed(void)
{
    double s;

    /* Restrict speed if obeying max */
  
    if (xLimitSpeedMask & PFIX_LIMIT_SPEED_MAX)
    {
	if (xMotionCoordD.speed > xMotionCoordD.maxSpeed)
	    s = xMotionCoordD.maxSpeed;
	else if (xMotionCoordD.speed < xMotionCoordD.minSpeed)
	    s = xMotionCoordD.minSpeed;
	else
	    s = xMotionCoordD.speed;
    }
    else
	s = xMotionCoordD.speed;
	
    if (xLimitAccelMask & PFIX_LIMIT_ACCEL_MAX)
    {
	if (xMotionCoordD.accel > xMotionCoordD.maxAccel)
	    xMotionCoordD.accel = xMotionCoordD.maxAccel;
	if (xMotionCoordD.accel < xMotionCoordD.minAccel)
	    xMotionCoordD.accel = xMotionCoordD.minAccel;
    }
    
    /* Slow down if approaching limits of database */
    if (xLimitSpeedMask & PFIX_LIMIT_SPEED_DB)
    {
	pfBox	*box = &xDBLimits;
	if (!1 && !!fprintf(stderr, "XXXpfBoxContainsPt3d called", box, xMotionCoordD.pos.xyz))
	{
	    double 	dot, ch, sh;
	    pfVec2d 	fromCenter;
    
	    /* XXX - Should clamp Z range in addition to X, Y */
    
	    fromCenter[0] = (xMotionCoordD.pos.xyz[0] -
			     0.5f * (box->min[0] + box->max[0]));
				     
	    fromCenter[1] = (xMotionCoordD.pos.xyz[1] -
			     0.5f * (box->min[1] + box->max[1]));
    
	    pfSinCosd(xMotionCoordD.pos.hpr[0], &sh, &ch);
				     
	    dot = fromCenter[1]*ch - fromCenter[0]*sh;
    
	    /* Slow down */
	    if (dot * s > 0.01f)
	    {
		s *= 0.96f;
	    }
	}
    }
    xMotionCoordD.speed = s;     
}

void
pfiInputXformD::update(void)
{
    xMotionCoordD.prevTime	    = xMotionCoordD.time;
    xMotionCoordD.time		    = pfGetTime();
    if (xMotionCoordD.startTime < 0.0f)
    {
	xMotionCoordD.prevTime = xMotionCoordD.startTime = xMotionCoordD.time;
    }
    xMotionCoordD.prevPos = xMotionCoordD.pos;
    if (xUpdateCB)
	xUpdateMode = xUpdateCB(this, xInputCoordD, xUpdateCBData);
    else
	xUpdateMode=PFI_CB_CONT;
}

void
pfiInputXformD::setMat(pfMatrix4d& _mat)
{
    xMotionCoordD.setMat(_mat);
}

void
pfiInputXformD::setCoord(pfCoordd *_coord)
{
    xMotionCoordD.setCoord(_coord);
}

void
pfiInputXformD::stop(void)
{
    xInMotion			= 0; 
    xMotionCoordD.speed		= 0.0f;
}

/******************************************************************************
 *		Travel XformerD Routines
 ******************************************************************************
 */
 
 

PFUIDLLEXPORT pfType *pfiInputXformDTravel::classType = NULL;

void
pfiInputXformDTravel::init()
{
    if (classType == NULL)
    {
	pfiInputXformD::init();
	classType = 
	    new pfType(pfiInputXformD::getClassType(), "pfiInputXformDTravel");
    }
}

pfiInputXformDTravel::pfiInputXformDTravel(void)
{
    setType(classType);
    xAuto			= 0;
    xMotionCoordD.startSpeed	= 12.0f;
    xMotionCoordD.minSpeed	= 0.0f;
    xMotionCoordD.maxSpeed	= 40.0f;
    xMotionCoordD.angularVel	= 90.0f;
    xMotionCoordD.startAccel = xMotionCoordD.accel = 1.25f;
    xMotionCoordD.maxAccel	= 10.0f;
    xMotionCoordD.minAccel	= 0.0f;
    reset();
}

void
pfiInputXformDTravel::reset(void)
{
    pfiInputXformD::reset();
    xMotionMode			= PFIXTR_MOTION_STOP;
    xMotionMod			= 0x0;
    xAccelMode			= PFIXTR_ACCEL_NONE;
    xAccelRate			= 0.0f;
}

void
pfiInputXformDTravel::stop(void)
{ 
    pfiInputXformD::stop();
    xAccelMode			= PFIXTR_ACCEL_NONE;
    xMotionMode			= PFIXTR_MOTION_STOP;
    xMotionMod			= 0x0;
}

void
pfiInputXformDTravel::setMode(int _mode, int _val)
{
    switch(_mode)
    {
    case PFIXTR_MODE_MOTION:
	if (xMotionMode == _val)
	    return;
	if (!xInMotion)
	    xMotionCoordD.speed = xMotionCoordD.startSpeed;
	xMotionMode = _val;
	break;
    case PFIXTR_MODE_MOTION_MOD:
	xMotionMod = _val;
	break;
    case PFIXTR_MODE_ACCEL:
	if (xAccelMode != _val)
	{
	    xAccelMode = _val;
		xMotionCoordD.accel = 0;
	}
	break;
    case PFIXTR_MODE_AUTO:
	xAuto = _val;
	break;
    default:
	pfNotify(PFNFY_WARN,PFNFY_USAGE,
	    "pfiInputXformDTravel::setMode() - unknown mode %d", _mode);
	break;
    }
}

int
pfiInputXformDTravel::getMode(int _mode) const
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
	    "pfiInputXformDTravel::getMode() - unknown mode %d", _mode);
	return -1;
    }
}
