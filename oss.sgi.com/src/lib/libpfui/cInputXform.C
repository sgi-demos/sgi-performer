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
 * $Revision: 1.1 $
 * $Date: 2000/11/21 21:39:35 $
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



pfType*
pfiGetInputCoordClassType(void)
{
    return pfiInputCoord::getClassType();    
}

pfiInputCoord *
pfiNewInputCoord(void *arena)
{ 
    return new(arena) pfiInputCoord;
}

void
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

void
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

pfType*
pfiGetMotionCoordClassType(void)
{
    return pfiMotionCoord::getClassType();    
}

pfiMotionCoord *
pfiNewMotionCoord(void *arena)
{ 
    return new(arena) pfiMotionCoord;
}

pfType*
pfiGetInputClassType(void)
{
    return pfiInput::getClassType();    
}

pfiInput *
pfiNewInput(void *arena)
{ 
    return new(arena) pfiInput;
}

void
pfiInputName(pfiInputXform *in, const char *_name)
{
    in->setName(_name);
}

const char * 
pfiGetInputName(pfiInputXform *in)
{
    return in->getName();
}

void 
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

int 
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

void 
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

int 
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

void 
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

void 
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

void 
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

void 
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

void 
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

void 
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

void 
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

void 
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

void 
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


pfType*
pfiGetIXformClassType(void)
{
    return pfiInputXform::getClassType();    
}

pfiInputXform *
pfiNewIXform(void *arena)
{ 
    return new(arena) pfiInputXform;
}


void 
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

int 
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

int 
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

void 
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

void 
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

void 
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

void 
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

void 
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


void 
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

void 
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

void 
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


void 
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

int 
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


void 
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

void 
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

void 
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

pfiInput * 
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

void 
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

pfiInputCoord * 
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

void 
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

void 
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

void 
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

void 
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

void 
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

pfiInputCoord *
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

void 
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

void 
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

void 
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

void 
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

void 
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

void 
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

void 
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

void 
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

void 
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

void 
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

pfType*
pfiGetIXformTrackballClassType(void)
{
    return pfiInputXformTrackball::getClassType();    
}

pfiInputXformTrackball *
pfiNewIXformTrackball(void *arena)
{ 
    return new(arena) pfiInputXformTrackball;
}

void 
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

int
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

pfType*
pfiGetIXformTravelClassType(void)
{
    return pfiInputXformTravel::getClassType();    
}

pfType*
pfiGetIXformDriveClassType(void)
{
    return pfiInputXformDrive::getClassType();    
}

pfiInputXformDrive *
pfiNewIXformDrive(void *arena)
{ 
    return new(arena) pfiInputXformDrive;
}

void 
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

int  
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

void
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

float
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

pfType*
pfiGetIXformFlyClassType(void)
{
    return pfiInputXformFly::getClassType();    
}

pfiInputXformFly *
pfiNewIXformFly(void *arena)
{ 
    return new(arena) pfiInputXformFly;
}

void 
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

int  
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

pfType*
pfiGetIXformSphericClassType(void)
{
    return pfiInputXformSpheric::getClassType();
}

pfiInputXformSpheric *
pfiNewIXformSpheric(void *arena)
{
    return new(arena) pfiInputXformSpheric;
}

void
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

int
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

void
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

float
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

void pfiIXformSphericPrintPathStuff(pfiInputXformSpheric *spheric) {
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformSphericPrintPathStuff -- null pfiInputXformSpheric*");
	return;
    }
    spheric->printPathStuff();
}
