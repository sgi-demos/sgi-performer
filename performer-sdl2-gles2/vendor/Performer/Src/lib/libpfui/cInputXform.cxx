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
 * file: cInputXform.C
 * -----------------------
 *
 * C-API for Libpr-based event tranform utlities for building UI components
 *
 * $Revision: 1.27 $
 * $Date: 2002/07/24 01:09:01 $
 *
 */

#include <Performer/pf.h>
#include <Performer/pr/pfMemory.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>

#define __PFI_INPUTXFORM_C__
#include <Performer/pfui.h>
#include <Performer/pfui/pfiInputXform.h>
#include <Performer/pfui/pfiTrackball.h>
#include <Performer/pfui/pfiDrive.h>
#include <Performer/pfui/pfiFly.h>
#include <Performer/pfui/pfiSpheric.h>



PFUIDLLEXPORT pfType*
pfiGetInputCoordClassType(void)
{
    return pfiInputCoord::getClassType();    
}

PFUIDLLEXPORT pfiInputCoord *
pfiNewInputCoord(void *arena)
{ 
    return new(arena) pfiInputCoord;
}

PFUIDLLEXPORT void
pfiInputCoordVec(pfiInputCoord *ic, float *_vec)
{
    if (ic == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputCoordVec -- null pfiInputCoord*");
	return;
    }
    ic->setVec(_vec);
}

PFUIDLLEXPORT void
pfiGetInputCoordVec(pfiInputCoord *ic, float *_vec)
{
    if (ic == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetInputCoordVec -- null pfiInputCoord*");
	return;
    }
    ic->getVec(_vec);
}

PFUIDLLEXPORT pfType*
pfiGetMotionCoordClassType(void)
{
    return pfiMotionCoord::getClassType();    
}

PFUIDLLEXPORT pfiMotionCoord *
pfiNewMotionCoord(void *arena)
{ 
    return new(arena) pfiMotionCoord;
}

PFUIDLLEXPORT pfType*
pfiGetInputClassType(void)
{
    return pfiInput::getClassType();    
}

PFUIDLLEXPORT pfiInput *
pfiNewInput(void *arena)
{ 
    return new(arena) pfiInput;
}

PFUIDLLEXPORT void
pfiInputName(pfiInputXform *in, const char *_name)
{
    in->setName(_name);
}

PFUIDLLEXPORT const char * 
pfiGetInputName(pfiInputXform *in)
{
    return in->getName();
}

PFUIDLLEXPORT void 
pfiInputFocus(pfiInput *in, int _focus)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputFocus -- null pfiInput*");
	return;
    }
    in->setFocus(_focus);
}

PFUIDLLEXPORT int 
pfiGetInputFocus(pfiInput *in)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetInputFocus -- null pfiInput*");
	return -1;
    }
    return in->getFocus();
}

PFUIDLLEXPORT void 
pfiInputEventMask(pfiInput *in, int _emask)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputEventMask -- null pfiInput*");
	return;
    }
    in->setEventMask(_emask);
}

PFUIDLLEXPORT int 
pfiGetInputEventMask(pfiInput *in)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetInputEventMask -- null pfiInput*");
	return -1;
    }
    return in->getEventMask();
}

PFUIDLLEXPORT void 
pfiResetInput(pfiInput *in)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetInput -- null pfiInput*");
	return;
    }
    in->reset();
}

PFUIDLLEXPORT void 
pfiProcessInputEvents(pfiInput *in)
{ 
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiProcessInputEvents -- null pfiInput*");
	return;
    }
    in->processEvents();
}

PFUIDLLEXPORT void 
pfiCollectInputEvents(pfiInput *in)
{ 
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiProcessInputEvents -- null pfiInput*");
	return;
    }
    in->collectEvents();
}

PFUIDLLEXPORT void 
pfiInputEventStreamProcessor(pfiInput *in, pfiEventStreamHandlerType _func, void *_data)
{
    
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputEventStreamProcessor -- null pfiInput*");
	return;
    }
    in->setEventStreamProcessor(_func, _data);
}

PFUIDLLEXPORT void 
pfiGetInputEventStreamProcessor(pfiInput *in, pfiEventStreamHandlerType *_func, void **_data)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputEventStreamProcessor -- null pfiInput*");
	return;
    }
    in->getEventStreamProcessor(_func, _data);
}

PFUIDLLEXPORT void 
pfiInputEventStreamCollector(pfiInput *in, pfiEventStreamHandlerType _func, void *_data)
{
    
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputEventStreamCollector -- null pfiInput*");
	return;
    }
    in->setEventStreamCollector(_func, _data);
}

PFUIDLLEXPORT void 
pfiGetInputEventStreamCollector(pfiInput *in, pfiEventStreamHandlerType *_func, void **_data)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputEventStreamCollector -- null pfiInput*");
	return;
    }
    in->getEventStreamCollector(_func, _data);
}

PFUIDLLEXPORT void 
pfiInputEventHandler(pfiInput *in, pfuEventHandlerFuncType _func, void *_data)
{
    
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputEventHandler -- null pfiInput*");
	return;
    }
    in->setEventHandler(_func, _data);
}

PFUIDLLEXPORT void 
pfiGetInputEventHandler(pfiInput *in, pfuEventHandlerFuncType *_func, void **_data)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputEventHandler -- null pfiInput*");
	return;
    }
    in->getEventHandler(_func, _data);
}


PFUIDLLEXPORT pfType*
pfiGetIXformClassType(void)
{
    return pfiInputXform::getClassType();    
}

PFUIDLLEXPORT pfiInputXform *
pfiNewIXform(void *arena)
{ 
    return new(arena) pfiInputXform;
}


PFUIDLLEXPORT void 
pfiIXformFocus(pfiInputXform *ix, int _focus)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformFocus -- null pfiInputXform*");
	return;
    }
    ix->setFocus(_focus);
}

PFUIDLLEXPORT int 
pfiGetIXformFocus(pfiInputXform *ix)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformFocus -- null pfiInputXform*");
	return -1;
    }
    return ix->getFocus();
}

PFUIDLLEXPORT int 
pfiIsIXformInMotion(pfiInputXform *ix)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIsIXformInMotion -- null pfiInputXform*");
	return -1;
    }
    return ix->isInMotion();
}

PFUIDLLEXPORT void 
pfiResetIXform(pfiInputXform *ix)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetIXform -- null pfiInputXform*");
	return;
    }
    ix->reset();
}

PFUIDLLEXPORT void 
pfiResetIXformPosition(pfiInputXform *ix)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetIXformPosition -- null pfiInputXform*");
	return;
    }
    ix->resetPosition();
}

PFUIDLLEXPORT void 
pfiStopIXform(pfiInputXform *ix)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIsIXformInMotion -- null pfiInputXform*");
	return;
    }
    ix->stop();
}

PFUIDLLEXPORT void 
pfiIXformMotionFuncs(pfiInputXform *ix, 
		pfiInputXformFuncType _start, pfiInputXformFuncType _stop, 
		void *_data)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformFuncs -- null pfiInputXform*");
	return;
    }
    ix->setMotionFuncs(_start, _stop, _data);
}

PFUIDLLEXPORT void 
pfiGetIXformMotionFuncs(pfiInputXform *ix, 
		pfiInputXformFuncType *_start, pfiInputXformFuncType *_stop, 
		void **_data)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformFuncs -- null pfiInputXform*");
	return;
    }
    ix->getMotionFuncs(_start, _stop, _data);
}


PFUIDLLEXPORT void 
pfiIXformUpdateFunc(pfiInputXform *ix, 
		pfiInputXformUpdateFuncType _func, void *_data)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformFuncs -- null pfiInputXform*");
	return;
    }
    ix->setUpdateFunc(_func, _data);
}

PFUIDLLEXPORT void 
pfiGetIXformUpdateFunc(pfiInputXform *ix, 
		pfiInputXformUpdateFuncType *_func, void **_data)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformFuncs -- null pfiInputXform*");
	return;
    }
    ix->getUpdateFunc(_func, _data);
}

PFUIDLLEXPORT void 
pfiUpdateIXform(pfiInputXform *ix)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiUpdateIXform -- null pfiInputXform*");
	return;
    }
    ix->update();
}


PFUIDLLEXPORT void 
pfiIXformMode(pfiInputXform *ix, int _mode, int _val)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXCoordformMode -- null pfiInputXform*");
	return;
    }
    ix->setMode(_mode, _val);
}

PFUIDLLEXPORT int 
pfiGetIXformMode(pfiInputXform *ix, int _mode)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXCoordformMode -- null pfiInputXform*");
	return -1;
    }
    return ix->getMode(_mode);
}


PFUIDLLEXPORT void 
pfiIXformMat(pfiInputXform *ix, pfMatrix& _mat)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXCoordformMat -- null pfiInputXform*");
	return;
    }
    ix->setMat(_mat);
}

PFUIDLLEXPORT void 
pfiGetIXformMat(pfiInputXform *ix, pfMatrix& _mat)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXCoordformMat -- null pfiInputXform*");
	return;
    }
    ix->getMat(_mat);
}

PFUIDLLEXPORT void 
pfiIXformInput(pfiInputXform *ix, pfiInput *in)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformInput -- null pfiInputXform*");
	return;
    }
    ix->setInput(in);
}

PFUIDLLEXPORT pfiInput * 
pfiGetIXformInput(pfiInputXform *ix)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformInput -- null pfiInputXform*");
	return NULL;
    }
    return ix->getInput();
}

PFUIDLLEXPORT void 
pfiIXformInputCoordPtr(pfiInputXform *ix, 
		    pfiInputCoord *_xcoord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformInputCoordPtr -- null pfiInputXform*");
	return;
    }
    ix->setInputCoordPtr(_xcoord);
}

PFUIDLLEXPORT pfiInputCoord * 
pfiGetIXformInputCoordPtr(pfiInputXform *ix)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformIInputCoordPtr -- null pfiInputXform*");
	return NULL;
    }
    return ix->getInputCoordPtr();
}

PFUIDLLEXPORT void 
pfiIXformMotionCoord(pfiInputXform *ix, 
		    pfiMotionCoord *_xcoord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformIMotionCoord -- null pfiInputXform*");
	return;
    }
    ix->setMotionCoord(_xcoord);
}

PFUIDLLEXPORT void 
pfiGetIXformMotionCoord(pfiInputXform *ix, 
		    pfiMotionCoord *_xcoord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformIMotionCoord -- null pfiInputXform*");
	return;
    }
    ix->getMotionCoord(_xcoord);
}

PFUIDLLEXPORT void 
pfiIXformResetCoord(pfiInputXform *ix, pfCoord *_coord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformResetCoord -- null pfiInputXform*");
	return;
    }
    ix->setResetCoord(_coord);
}

PFUIDLLEXPORT void 
pfiGetIXformResetCoord(pfiInputXform *ix, pfCoord *_coord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformResetCoord -- null pfiInputXform*");
	return;
    }
    ix->getResetCoord(_coord);
}

PFUIDLLEXPORT void 
pfiIXformInputCoordPtr(pfiInputXformTrackball *tb, pfiInputCoord *_ip)
{ 
    if (tb == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformTrackballInputCoordPtr -- null pfiIXformTrackball*");
	return;
    }
    tb->setInputCoordPtr(_ip);
}

PFUIDLLEXPORT pfiInputCoord *
pfiGetIXformInputCoordPtr(pfiInputXformTrackball *tb)
{ 
    if (tb == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformTrackballInputCoordPtr -- null pfi2DInputXformTrackball*");
	return NULL;
    }
    return tb->getInputCoordPtr();
}

PFUIDLLEXPORT void 
pfiIXformCoord(pfiInputXform *ix, pfCoord *_coord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformPos -- null pfiInputXform*");
	return;
    }
    ix->setCoord(_coord);
}

PFUIDLLEXPORT void 
pfiGetIXformCoord(pfiInputXform *ix, pfCoord *_coord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformPos -- null pfiInputXform*");
	return;
    }
    ix->getCoord(_coord);
}

PFUIDLLEXPORT void 
pfiIXformStartMotion(pfiInputXform *ix, float _startSpeed,
				 float _accel)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformStartMotion -- null pfiInputXform*");
	return;
    }
    ix->setStartMotion(_startSpeed, _accel);
}

PFUIDLLEXPORT void 
pfiGetIXformStartMotion(pfiInputXform *ix, float *_startSpeed,
				 float *_accel)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformStartMotion -- null pfiInputXform*");
	return;
    }
    ix->getStartMotion(_startSpeed, _accel);
}

PFUIDLLEXPORT void 
pfiIXformMotionLimits(pfiInputXform *ix, float _maxSpeed,
				 float _angularVel, float _maxAccel)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformMotionLimits -- null pfiInputXform*");
	return;
    }
    ix->setMotionLimits(_maxSpeed, _angularVel, _maxAccel);
}

PFUIDLLEXPORT void 
pfiIXformMotionLimits(pfiInputXform *ix, float *_maxSpeed,
				 float *_angularVel, float *_maxAccel)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformMotionLimits -- null pfiInputXform*");
	return;
    }
    ix->getMotionLimits(_maxSpeed, _angularVel, _maxAccel);
}

PFUIDLLEXPORT void 
pfiIXformDBLimits(pfiInputXform *ix, pfBox *_dbLimits)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDBLimits -- null pfiInputXform*");
	return;
    }
    ix->setDBLimits(_dbLimits);
}

PFUIDLLEXPORT void 
pfiGetIXformDBLimits(pfiInputXform *ix, pfBox *_dbLimits)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDBLimits -- null pfiInputXform*");
	return;
    }
    ix->getDBLimits(_dbLimits);
}

PFUIDLLEXPORT void 
pfiIXformBSphere(pfiInputXform *ix, pfSphere *_sphere)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformBSphere -- null pfiInputXform*");
	return;
    }
    ix->setBSphere(_sphere);
}

PFUIDLLEXPORT void 
pfiGetIXformBSphere(pfiInputXform *ix, pfSphere *_sphere)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformBSphere -- null pfiInputXform*");
	return;
    }
    ix->getBSphere(_sphere);
}

PFUIDLLEXPORT pfType*
pfiGetIXformTrackballClassType(void)
{
    return pfiInputXformTrackball::getClassType();    
}

PFUIDLLEXPORT pfiInputXformTrackball *
pfiNewIXformTrackball(void *arena)
{ 
    return new(arena) pfiInputXformTrackball;
}

PFUIDLLEXPORT void 
pfiIXformTrackballMode(pfiInputXformTrackball *tb, int _mode, int _val)
{ 
    if (tb == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformTrackballMode -- null pfiInputXformTrackball*");
	return;
    }
    tb->setMode(_mode, _val);
}

PFUIDLLEXPORT int
pfiGetIXformTrackballMode(pfiInputXformTrackball *tb, int _mode)
{ 
    if (tb == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetTrackballMode -- null pfiInputXformTrackball*");
	return -1;
    }
    return tb->getMode(_mode);
}

PFUIDLLEXPORT pfType*
pfiGetIXformTravelClassType(void)
{
    return pfiInputXformTravel::getClassType();    
}

PFUIDLLEXPORT pfType*
pfiGetIXformDriveClassType(void)
{
    return pfiInputXformDrive::getClassType();    
}

PFUIDLLEXPORT pfiInputXformDrive *
pfiNewIXformDrive(void *arena)
{ 
    return new(arena) pfiInputXformDrive;
}

PFUIDLLEXPORT void 
pfiIXformDriveMode(pfiInputXformDrive *drive, int _mode, int _val)
{ 
    if (drive == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDriveMode -- null pfiInputXformDrive*");
	return;
    }
    drive->setMode(_mode, _val);
}

PFUIDLLEXPORT int  
pfiGetIXformDriveMode(pfiInputXformDrive *drive, int _mode)
{ 
    if (drive == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDriveMode -- null pfiInputXformDrive*");
	return NULL;
    }
    return drive->getMode(_mode);
}

PFUIDLLEXPORT void
pfiIXformDriveHeight(pfiInputXformDrive *dr, float _height)
{
   if (dr == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDriveHeight -- null pfiInputXformDrive*");
	return;
    } 
    dr->setDriveHeight(_height);
}

PFUIDLLEXPORT float
pfiGetIXformDriveHeight(pfiInputXformDrive *dr)
{
   if (dr == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGeIXformDriveHeight -- null pfiInputXformDrive*");
	return -1;
    } 
    return dr->getDriveHeight();
}

PFUIDLLEXPORT pfType*
pfiGetIXformFlyClassType(void)
{
    return pfiInputXformFly::getClassType();    
}

PFUIDLLEXPORT pfiInputXformFly *
pfiNewIXformFly(void *arena)
{ 
    return new(arena) pfiInputXformFly;
}

PFUIDLLEXPORT void 
pfiIXformFlyMode(pfiInputXformFly *fly, int _mode, int _val)
{ 
    if (fly == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformFlyMode -- null pfiInputXformFly*");
	return;
    }
    fly->setMode(_mode, _val);
}

PFUIDLLEXPORT int  
pfiGetIXformFlyMode(pfiInputXformFly *fly, int _mode)
{ 
    if (fly == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformFlyMode -- null pfiInputXformFly*");
	return NULL;
    }
    return fly->getMode(_mode);
}

PFUIDLLEXPORT pfType*
pfiGetIXformSphericClassType(void)
{
    return pfiInputXformSpheric::getClassType();
}

PFUIDLLEXPORT pfiInputXformSpheric *
pfiNewIXformSpheric(void *arena)
{
    return new(arena) pfiInputXformSpheric;
}

PFUIDLLEXPORT void
pfiIXformSphericMode(pfiInputXformSpheric *spheric, int _mode, int _val)
{
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformSphericMode -- null pfiInputXformSpheric*");
	return;
    }
    spheric->setMode(_mode, _val);
}

PFUIDLLEXPORT int
pfiGetIXformSphericMode(pfiInputXformSpheric *spheric, int _mode)
{
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiGetIXformSphericMode -- null pfiInputXformSpheric*");
	return NULL;
    }
    return spheric->getMode(_mode);
}

PFUIDLLEXPORT void
pfiIXformSphericParameter(pfiInputXformSpheric *spheric, int _param,
			  float _val)
{
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformSphericParameter -- null pfiInputXformSpheric*");
	return;
    }
    spheric->setParameter(_param, _val);
}

PFUIDLLEXPORT float
pfiGetIXformSphericParameter(pfiInputXformSpheric *spheric, int _param)
{
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiGetIXformSphericParameter -- null pfiInputXformSpheric*");
#ifndef __linux__
	return NULL;
#else
	return 0;
#endif
    }
    return  spheric->getParameter(_param);
}

PFUIDLLEXPORT
void pfiIXformSphericSetWorld(pfiInputXformSpheric *spheric, int worldNumber,
			      int in_or_out) {
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformSphericSetWorld -- null pfiInputXformSpheric*");
	return;
    }
    spheric->setWorld(worldNumber, in_or_out);
}

PFUIDLLEXPORT
void pfiIXformSphericReadPathFile(pfiInputXformSpheric *spheric,
				  char *filename) {
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformSphericReadPathFile -- null pfiInputXformSpheric*");
	return;
    }
    spheric->readPathFile(filename);
}

PFUIDLLEXPORT
void pfiIXformSphericPrintPathStuff(pfiInputXformSpheric *spheric) {
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformSphericPrintPathStuff -- null pfiInputXformSpheric*");
	return;
    }
    spheric->printPathStuff();
}
