/*
 * WARNING! DO NOT EDIT!
 * cInputXformD.C automatically generated from (../libpfui/cInputXform.C)
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
 * file: cInputXformD.C
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
#include <Performer/pfuiD.h>
#include <Performer/pfuiD/pfiInputXformD.h>
#include <Performer/pfuiD/pfiTrackballD.h>
#include <Performer/pfuiD/pfiDriveD.h>
#include <Performer/pfuiD/pfiFlyD.h>
#include <Performer/pfuiD/pfiSphericD.h>



PFUIDLLEXPORT pfType*
pfiGetInputCoordDClassType(void)
{
    return pfiInputCoordD::getClassType();    
}

PFUIDLLEXPORT pfiInputCoordD *
pfiNewInputCoordD(void *arena)
{ 
    return new(arena) pfiInputCoordD;
}

PFUIDLLEXPORT void
pfiInputCoordDVec(pfiInputCoordD *ic, double *_vec)
{
    if (ic == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputCoordDVec -- null pfiInputCoordD*");
	return;
    }
    ic->setVec(_vec);
}

PFUIDLLEXPORT void
pfiGetInputCoordDVec(pfiInputCoordD *ic, double *_vec)
{
    if (ic == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetInputCoordDVec -- null pfiInputCoordD*");
	return;
    }
    ic->getVec(_vec);
}

PFUIDLLEXPORT pfType*
pfiGetMotionCoordDClassType(void)
{
    return pfiMotionCoordD::getClassType();    
}

PFUIDLLEXPORT pfiMotionCoordD *
pfiNewMotionCoordD(void *arena)
{ 
    return new(arena) pfiMotionCoordD;
}

PFUIDLLEXPORT pfType*
pfiGetInputDClassType(void)
{
    return pfiInputD::getClassType();    
}

PFUIDLLEXPORT pfiInputD *
pfiNewInputD(void *arena)
{ 
    return new(arena) pfiInputD;
}

PFUIDLLEXPORT void
pfiInputDName(pfiInputXformD *in, const char *_name)
{
    in->setName(_name);
}

PFUIDLLEXPORT const char * 
pfiGetInputDName(pfiInputXformD *in)
{
    return in->getName();
}

PFUIDLLEXPORT void 
pfiInputDFocus(pfiInputD *in, int _focus)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputDFocus -- null pfiInputD*");
	return;
    }
    in->setFocus(_focus);
}

PFUIDLLEXPORT int 
pfiGetInputDFocus(pfiInputD *in)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetInputDFocus -- null pfiInputD*");
	return -1;
    }
    return in->getFocus();
}

PFUIDLLEXPORT void 
pfiInputDEventMask(pfiInputD *in, int _emask)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputDEventMask -- null pfiInputD*");
	return;
    }
    in->setEventMask(_emask);
}

PFUIDLLEXPORT int 
pfiGetInputDEventMask(pfiInputD *in)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetInputDEventMask -- null pfiInputD*");
	return -1;
    }
    return in->getEventMask();
}

PFUIDLLEXPORT void 
pfiResetInputD(pfiInputD *in)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetInputD -- null pfiInputD*");
	return;
    }
    in->reset();
}

PFUIDLLEXPORT void 
pfiProcessInputDEvents(pfiInputD *in)
{ 
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiProcessInputDEvents -- null pfiInputD*");
	return;
    }
    in->processEvents();
}

PFUIDLLEXPORT void 
pfiCollectInputDEvents(pfiInputD *in)
{ 
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiProcessInputDEvents -- null pfiInputD*");
	return;
    }
    in->collectEvents();
}

PFUIDLLEXPORT void 
pfiInputDEventStreamProcessor(pfiInputD *in, pfiEventStreamHandlerTypeD _func, void *_data)
{
    
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputDEventStreamProcessor -- null pfiInputD*");
	return;
    }
    in->setEventStreamProcessor(_func, _data);
}

PFUIDLLEXPORT void 
pfiGetInputDEventStreamProcessor(pfiInputD *in, pfiEventStreamHandlerTypeD *_func, void **_data)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputDEventStreamProcessor -- null pfiInputD*");
	return;
    }
    in->getEventStreamProcessor(_func, _data);
}

PFUIDLLEXPORT void 
pfiInputDEventStreamCollector(pfiInputD *in, pfiEventStreamHandlerTypeD _func, void *_data)
{
    
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputDEventStreamCollector -- null pfiInputD*");
	return;
    }
    in->setEventStreamCollector(_func, _data);
}

PFUIDLLEXPORT void 
pfiGetInputDEventStreamCollector(pfiInputD *in, pfiEventStreamHandlerTypeD *_func, void **_data)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputDEventStreamCollector -- null pfiInputD*");
	return;
    }
    in->getEventStreamCollector(_func, _data);
}

PFUIDLLEXPORT void 
pfiInputDEventHandler(pfiInputD *in, pfuEventHandlerFuncType _func, void *_data)
{
    
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputDEventHandler -- null pfiInputD*");
	return;
    }
    in->setEventHandler(_func, _data);
}

PFUIDLLEXPORT void 
pfiGetInputDEventHandler(pfiInputD *in, pfuEventHandlerFuncType *_func, void **_data)
{
    if (in == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiInputDEventHandler -- null pfiInputD*");
	return;
    }
    in->getEventHandler(_func, _data);
}


PFUIDLLEXPORT pfType*
pfiGetIXformDClassType(void)
{
    return pfiInputXformD::getClassType();    
}

PFUIDLLEXPORT pfiInputXformD *
pfiNewIXformD(void *arena)
{ 
    return new(arena) pfiInputXformD;
}


PFUIDLLEXPORT void 
pfiIXformDFocus(pfiInputXformD *ix, int _focus)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDFocus -- null pfiInputXformD*");
	return;
    }
    ix->setFocus(_focus);
}

PFUIDLLEXPORT int 
pfiGetIXformDFocus(pfiInputXformD *ix)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDFocus -- null pfiInputXformD*");
	return -1;
    }
    return ix->getFocus();
}

PFUIDLLEXPORT int 
pfiIsIXDformInMotion(pfiInputXformD *ix)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIsIXDformInMotion -- null pfiInputXformD*");
	return -1;
    }
    return ix->isInMotion();
}

PFUIDLLEXPORT void 
pfiResetIXformD(pfiInputXformD *ix)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetIXformD -- null pfiInputXformD*");
	return;
    }
    ix->reset();
}

PFUIDLLEXPORT void 
pfiResetIXformDPosition(pfiInputXformD *ix)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetIXformDPosition -- null pfiInputXformD*");
	return;
    }
    ix->resetPosition();
}

PFUIDLLEXPORT void 
pfiStopIXformD(pfiInputXformD *ix)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIsIXDformInMotion -- null pfiInputXformD*");
	return;
    }
    ix->stop();
}

PFUIDLLEXPORT void 
pfiIXformDMotionFuncs(pfiInputXformD *ix, 
		pfiInputXformDFuncType _start, pfiInputXformDFuncType _stop, 
		void *_data)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDFuncs -- null pfiInputXformD*");
	return;
    }
    ix->setMotionFuncs(_start, _stop, _data);
}

PFUIDLLEXPORT void 
pfiGetIXformDMotionFuncs(pfiInputXformD *ix, 
		pfiInputXformDFuncType *_start, pfiInputXformDFuncType *_stop, 
		void **_data)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformFuncs -- null pfiInputXformD*");
	return;
    }
    ix->getMotionFuncs(_start, _stop, _data);
}


PFUIDLLEXPORT void 
pfiIXformDUpdateFunc(pfiInputXformD *ix, 
		pfiInputXformDUpdateFuncType _func, void *_data)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDFuncs -- null pfiInputXformD*");
	return;
    }
    ix->setUpdateFunc(_func, _data);
}

PFUIDLLEXPORT void 
pfiGetIXformDUpdateFunc(pfiInputXformD *ix, 
		pfiInputXformDUpdateFuncType *_func, void **_data)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformFuncs -- null pfiInputXformD*");
	return;
    }
    ix->getUpdateFunc(_func, _data);
}

PFUIDLLEXPORT void 
pfiUpdateIXformD(pfiInputXformD *ix)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiUpdateIXformD -- null pfiInputXformD*");
	return;
    }
    ix->update();
}


PFUIDLLEXPORT void 
pfiIXformDMode(pfiInputXformD *ix, int _mode, int _val)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXCoordformMode -- null pfiInputXformD*");
	return;
    }
    ix->setMode(_mode, _val);
}

PFUIDLLEXPORT int 
pfiGetIXformDMode(pfiInputXformD *ix, int _mode)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXCoordformMode -- null pfiInputXformD*");
	return -1;
    }
    return ix->getMode(_mode);
}


PFUIDLLEXPORT void 
pfiIXformDMat(pfiInputXformD *ix, pfMatrix4d& _mat)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXCoordformMat -- null pfiInputXformD*");
	return;
    }
    ix->setMat(_mat);
}

PFUIDLLEXPORT void 
pfiGetIXformDMat(pfiInputXformD *ix, pfMatrix4d& _mat)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXCoordformMat -- null pfiInputXformD*");
	return;
    }
    ix->getMat(_mat);
}

PFUIDLLEXPORT void 
pfiIXformDInputD(pfiInputXformD *ix, pfiInputD *in)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDInputD -- null pfiInputXformD*");
	return;
    }
    ix->setInputD(in);
}

PFUIDLLEXPORT pfiInputD * 
pfiGetIXformDInputD(pfiInputXformD *ix)
{
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDInputD -- null pfiInputXformD*");
	return NULL;
    }
    return ix->getInputD();
}

PFUIDLLEXPORT void 
pfiIXformDInputCoordDPtr(pfiInputXformD *ix, 
		    pfiInputCoordD *_xcoord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDInputCoordDPtr -- null pfiInputXformD*");
	return;
    }
    ix->setInputCoordDPtr(_xcoord);
}

PFUIDLLEXPORT pfiInputCoordD * 
pfiGetIXformDInputCoordDPtr(pfiInputXformD *ix)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDIInputCoordDPtr -- null pfiInputXformD*");
	return NULL;
    }
    return ix->getInputCoordDPtr();
}

PFUIDLLEXPORT void 
pfiIXformDMotionCoordD(pfiInputXformD *ix, 
		    pfiMotionCoordD *_xcoord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDIMotionCoordD -- null pfiInputXformD*");
	return;
    }
    ix->setMotionCoordD(_xcoord);
}

PFUIDLLEXPORT void 
pfiGetIXformDMotionCoordD(pfiInputXformD *ix, 
		    pfiMotionCoordD *_xcoord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDIMotionCoordD -- null pfiInputXformD*");
	return;
    }
    ix->getMotionCoordD(_xcoord);
}

PFUIDLLEXPORT void 
pfiIXformDResetCoord(pfiInputXformD *ix, pfCoordd *_coord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDResetCoord -- null pfiInputXformD*");
	return;
    }
    ix->setResetCoord(_coord);
}

PFUIDLLEXPORT void 
pfiGetIXformDResetCoord(pfiInputXformD *ix, pfCoordd *_coord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDResetCoord -- null pfiInputXformD*");
	return;
    }
    ix->getResetCoord(_coord);
}

PFUIDLLEXPORT void 
pfiIXformDInputCoordDPtr(pfiInputXformDTrackball *tb, pfiInputCoordD *_ip)
{ 
    if (tb == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDTrackballInputCoordDPtr -- null pfiIXformDTrackball*");
	return;
    }
    tb->setInputCoordDPtr(_ip);
}

PFUIDLLEXPORT pfiInputCoordD *
pfiGetIXformDInputCoordDPtr(pfiInputXformDTrackball *tb)
{ 
    if (tb == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDTrackballInputCoordDPtr -- null pfi2DInputXformDTrackball*");
	return NULL;
    }
    return tb->getInputCoordDPtr();
}

PFUIDLLEXPORT void 
pfiIXformDCoord(pfiInputXformD *ix, pfCoordd *_coord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDPos -- null pfiInputXformD*");
	return;
    }
    ix->setCoord(_coord);
}

PFUIDLLEXPORT void 
pfiGetIXformDCoord(pfiInputXformD *ix, pfCoordd *_coord)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDPos -- null pfiInputXformD*");
	return;
    }
    ix->getCoord(_coord);
}

PFUIDLLEXPORT void 
pfiIXformDStartMotion(pfiInputXformD *ix, double _startSpeed,
				 double _accel)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDStartMotion -- null pfiInputXformD*");
	return;
    }
    ix->setStartMotion(_startSpeed, _accel);
}

PFUIDLLEXPORT void 
pfiGetIXformDStartMotion(pfiInputXformD *ix, double *_startSpeed,
				 double *_accel)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDStartMotion -- null pfiInputXformD*");
	return;
    }
    ix->getStartMotion(_startSpeed, _accel);
}

PFUIDLLEXPORT void 
pfiIXformDMotionLimits(pfiInputXformD *ix, double _maxSpeed,
				 double _angularVel, double _maxAccel)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDMotionLimits -- null pfiInputXformD*");
	return;
    }
    ix->setMotionLimits(_maxSpeed, _angularVel, _maxAccel);
}

PFUIDLLEXPORT void 
pfiIXformDMotionLimits(pfiInputXformD *ix, double *_maxSpeed,
				 double *_angularVel, double *_maxAccel)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDMotionLimits -- null pfiInputXformD*");
	return;
    }
    ix->getMotionLimits(_maxSpeed, _angularVel, _maxAccel);
}

PFUIDLLEXPORT void 
pfiIXformDDBLimits(pfiInputXformD *ix, pfBox *_dbLimits)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDDBLimits -- null pfiInputXformD*");
	return;
    }
    ix->setDBLimits(_dbLimits);
}

PFUIDLLEXPORT void 
pfiGetIXformDDBLimits(pfiInputXformD *ix, pfBox *_dbLimits)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDDBLimits -- null pfiInputXformD*");
	return;
    }
    ix->getDBLimits(_dbLimits);
}

PFUIDLLEXPORT void 
pfiIXformDBSphere(pfiInputXformD *ix, pfSphere *_sphere)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDBSphere -- null pfiInputXformD*");
	return;
    }
    ix->setBSphere(_sphere);
}

PFUIDLLEXPORT void 
pfiGetIXformDBSphere(pfiInputXformD *ix, pfSphere *_sphere)
{ 
    if (ix == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDBSphere -- null pfiInputXformD*");
	return;
    }
    ix->getBSphere(_sphere);
}

PFUIDLLEXPORT pfType*
pfiGetIXformDTrackballClassType(void)
{
    return pfiInputXformDTrackball::getClassType();    
}

PFUIDLLEXPORT pfiInputXformDTrackball *
pfiNewIXformDTrackball(void *arena)
{ 
    return new(arena) pfiInputXformDTrackball;
}

PFUIDLLEXPORT void 
pfiIXformDTrackballMode(pfiInputXformDTrackball *tb, int _mode, int _val)
{ 
    if (tb == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDTrackballMode -- null pfiInputXformDTrackball*");
	return;
    }
    tb->setMode(_mode, _val);
}

PFUIDLLEXPORT int
pfiGetIXformDTrackballMode(pfiInputXformDTrackball *tb, int _mode)
{ 
    if (tb == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetTrackballMode -- null pfiInputXformDTrackball*");
	return -1;
    }
    return tb->getMode(_mode);
}

PFUIDLLEXPORT pfType*
pfiGetIXformDTravelClassType(void)
{
    return pfiInputXformDTravel::getClassType();    
}

PFUIDLLEXPORT pfType*
pfiGetIXformDDriveClassType(void)
{
    return pfiInputXformDDrive::getClassType();    
}

PFUIDLLEXPORT pfiInputXformDDrive *
pfiNewIXformDDrive(void *arena)
{ 
    return new(arena) pfiInputXformDDrive;
}

PFUIDLLEXPORT void 
pfiIXformDDriveMode(pfiInputXformDDrive *drive, int _mode, int _val)
{ 
    if (drive == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDDriveMode -- null pfiInputXformDDrive*");
	return;
    }
    drive->setMode(_mode, _val);
}

PFUIDLLEXPORT int  
pfiGetIXformDDriveMode(pfiInputXformDDrive *drive, int _mode)
{ 
    if (drive == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDDriveMode -- null pfiInputXformDDrive*");
	return NULL;
    }
    return drive->getMode(_mode);
}

PFUIDLLEXPORT void
pfiIXformDDriveHeight(pfiInputXformDDrive *dr, double _height)
{
   if (dr == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDDriveHeight -- null pfiInputXformDDrive*");
	return;
    } 
    dr->setDriveHeight(_height);
}

PFUIDLLEXPORT double
pfiGetIXformDDriveHeight(pfiInputXformDDrive *dr)
{
   if (dr == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGeIXformDDriveHeight -- null pfiInputXformDDrive*");
	return -1;
    } 
    return dr->getDriveHeight();
}

PFUIDLLEXPORT pfType*
pfiGetIXformDFlyClassType(void)
{
    return pfiInputXformDFly::getClassType();    
}

PFUIDLLEXPORT pfiInputXformDFly *
pfiNewIXformDFly(void *arena)
{ 
    return new(arena) pfiInputXformDFly;
}

PFUIDLLEXPORT void 
pfiIXformDFlyMode(pfiInputXformDFly *fly, int _mode, int _val)
{ 
    if (fly == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiIXformDFlyMode -- null pfiInputXformDFly*");
	return;
    }
    fly->setMode(_mode, _val);
}

PFUIDLLEXPORT int  
pfiGetIXformDFlyMode(pfiInputXformDFly *fly, int _mode)
{ 
    if (fly == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetIXformDFlyMode -- null pfiInputXformDFly*");
	return NULL;
    }
    return fly->getMode(_mode);
}

PFUIDLLEXPORT pfType*
pfiGetIXformDSphericClassType(void)
{
    return pfiInputXformDSpheric::getClassType();
}

PFUIDLLEXPORT pfiInputXformDSpheric *
pfiNewIXformDSpheric(void *arena)
{
    return new(arena) pfiInputXformDSpheric;
}

PFUIDLLEXPORT void
pfiIXformDSphericMode(pfiInputXformDSpheric *spheric, int _mode, int _val)
{
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformDSphericMode -- null pfiInputXformDSpheric*");
	return;
    }
    spheric->setMode(_mode, _val);
}

PFUIDLLEXPORT int
pfiGetIXformDSphericMode(pfiInputXformDSpheric *spheric, int _mode)
{
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiGetIXformDSphericMode -- null pfiInputXformDSpheric*");
	return NULL;
    }
    return spheric->getMode(_mode);
}

PFUIDLLEXPORT void
pfiIXformDSphericParameter(pfiInputXformDSpheric *spheric, int _param,
			  double _val)
{
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformDSphericParameter -- null pfiInputXformDSpheric*");
	return;
    }
    spheric->setParameter(_param, _val);
}

PFUIDLLEXPORT double
pfiGetIXformDSphericParameter(pfiInputXformDSpheric *spheric, int _param)
{
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiGetIXformDSphericParameter -- null pfiInputXformDSpheric*");
#ifndef __linux__
	return NULL;
#else
	return 0;
#endif
    }
    return  spheric->getParameter(_param);
}

PFUIDLLEXPORT
void pfiIXformDSphericSetWorld(pfiInputXformDSpheric *spheric, int worldNumber,
			      int in_or_out) {
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformDSphericSetWorld -- null pfiInputXformDSpheric*");
	return;
    }
    spheric->setWorld(worldNumber, in_or_out);
}

PFUIDLLEXPORT
void pfiIXformDSphericReadPathFile(pfiInputXformDSpheric *spheric,
				  char *filename) {
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformDSphericReadPathFile -- null pfiInputXformDSpheric*");
	return;
    }
    spheric->readPathFile(filename);
}

PFUIDLLEXPORT
void pfiIXformDSphericPrintPathStuff(pfiInputXformDSpheric *spheric) {
    if (spheric == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiIXformDSphericPrintPathStuff -- null pfiInputXformDSpheric*");
	return;
    }
    spheric->printPathStuff();
}
