/*
 * WARNING! DO NOT EDIT!
 * cXformerD.C automatically generated from (../libpfui/cXformer.C)
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
 * cXformerD.C
 * --------------
 * $Revision: 1.17 $
 * $Date: 2002/07/24 01:09:01 $
 */

#include <math.h>
#ifndef WIN32
#include <X11/X.h>
#endif

#include <Performer/pf.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>

#define _PFI_XFORMER_C_
#include <Performer/pfuiD.h>
#include "pfiXformerD.h"

PFUIDLLEXPORT pfType*
pfiGetXformerDClassType(void)
{
    return pfiXformerD::getClassType();    
}

PFUIDLLEXPORT pfiXformerD *
pfiNewXformerD(void *arena)
{
    return new(arena) pfiXformerD;
}

PFUIDLLEXPORT void 
pfiXformerDModel(pfiXformerD* xf, int _index, pfiInputXformD* _model)
{
     if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDModel -- null pfiXformerD*");
	return;
    }
    xf->setModel(_index, _model);
}

PFUIDLLEXPORT void 
pfiSelectXformerDModel(pfiXformerD *xf, int _which)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiSelectXformerDModel -- null pfiXformerD*");
	return;
    }
    xf->selectModel(_which);
}

PFUIDLLEXPORT pfiInputXformD*  
pfiGetXformerDCurModel(pfiXformerD *xf)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDCurModel -- null pfiXformerD*");
	return NULL;
    }
    return xf->getCurModel();
}

PFUIDLLEXPORT int
pfiGetXformerDCurModelIndex(pfiXformerD *xf)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDCurModelIndex -- null pfiXformerD*");
	return -1;
    }
    return xf->getCurModelIndex();
}

PFUIDLLEXPORT void 
pfiRemoveXformerDModel(pfiXformerD* xf, pfiInputXformD* _model)
{
     if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiRemoveXformerDModel -- null pfiXformerD*");
	return;
    }
    xf->removeModel(_model);
}

PFUIDLLEXPORT void 
pfiRemoveXformerDModelIndex(pfiXformerD* xf, int _index)
{
     if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiRemoveXformerDModelIndex -- null pfiXformerD*");
	return;
    }
    xf->removeModelIndex(_index);
}

PFUIDLLEXPORT void
pfiStopXformerD(pfiXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiStopXformerD -- null pfiXformerD*");
	return;
    }
    xf->stop();
}

PFUIDLLEXPORT void
pfiResetXformerD(pfiXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetXformerD -- null pfiXformerD*");
	return;
    }
    xf->reset();
}

PFUIDLLEXPORT void
pfiResetXformerDPosition(pfiXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetXformerDPosition -- null pfiXformerD*");
	return;
    }
    xf->resetPosition();
}

PFUIDLLEXPORT void
pfiCenterXformerD(pfiXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCenterXformerD -- null pfiXformerD*");
	return;
    }
    xf->center();
}


PFUIDLLEXPORT void
pfiXformerDAutoInput(pfiXformerD *xf, pfChannel *_chan, pfuMouse *_mouse, 
    pfuEventStream *_events)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDAutoInput -- null pfiXformerD*");
	return;
    }

    xf->setAutoInputD(_chan, _mouse, _events);
}

PFUIDLLEXPORT void
pfiGetXformerDAutoInput(pfiXformerD *xf, pfChannel **_chan, pfuMouse **_mouse, 
    pfuEventStream **_events)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDAutoInput -- null pfiXformerD*");
	return;
    }

    xf->getAutoInputD(_chan, _mouse, _events);
}

PFUIDLLEXPORT void
pfiXformerDMat(pfiXformerD *xf, pfMatrix4d& _mat)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDMat -- null pfiXformerD*");
	return;
    }
    xf->setMat(_mat);
}


PFUIDLLEXPORT void
pfiGetXformerDMat(pfiXformerD *xf, pfMatrix4d& _mat)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDMat -- null pfiXformerD*");
	return;
    }

    xf->getMat(_mat);
}

PFUIDLLEXPORT void
pfiXformerDModelMat(pfiXformerD *xf, pfMatrix4d& _mat)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDModelMat -- null pfiXformerD*");
	return;
    }
    xf->setModelMat(_mat);
}


PFUIDLLEXPORT void
pfiGetXformerDModelMat(pfiXformerD *xf, pfMatrix4d& _mat)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDModelMat -- null pfiXformerD*");
	return;
    }

    xf->getModelMat(_mat);
}


PFUIDLLEXPORT void
pfiXformerDCoord(pfiXformerD *xf, pfCoordd *_coord)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDCoord -- null pfiXformerD*");
	return;
    }
    xf->setCoord(_coord);
}


PFUIDLLEXPORT void
pfiGetXformerDCoord(pfiXformerD *xf, pfCoordd *_coord)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDCoord -- null pfiXformerD*");
	return;
    }
    xf->getCoord(_coord); 
}

PFUIDLLEXPORT void
pfiXformerDResetCoord(pfiXformerD *xf, pfCoordd *_resetCoord)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDCoord -- null pfiXformerD*");
	return;
    }
    xf->setResetCoord(_resetCoord);
}


PFUIDLLEXPORT void
pfiGetXformerDResetCoord(pfiXformerD *xf, pfCoordd *_resetCoord)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDCoord -- null pfiXformerD*");
	return;
    }
    xf->getResetCoord(_resetCoord); 
}

PFUIDLLEXPORT void
pfiXformerDNode(pfiXformerD *xf, pfNode *_node)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDNode -- null pfiXformerD*");
	return;
    } 
    xf->setNode(_node);
}

PFUIDLLEXPORT pfNode *
pfiGetXformerDNode(pfiXformerD *xf)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDNode -- null pfiXformerD*");
	return NULL;
    } 
    return xf->getNode();
}

PFUIDLLEXPORT void
pfiXformerDAutoPosition(pfiXformerD *xf, pfChannel *_chan, pfDoubleDCS *_dcs)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDAutoPosition -- null pfiXformerD*");
	return;
    } 
    xf->setAutoPosition(_chan, _dcs);
}

PFUIDLLEXPORT void
pfiGetXformerDAutoPosition(pfiXformerD *xf, pfChannel **_chan, pfDoubleDCS **_dcs)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDAutoPosition -- null pfiXformerD*");
	return;
    } 
    xf->getAutoPosition(_chan, _dcs);
}


PFUIDLLEXPORT void
pfiXformerDLimits(pfiXformerD *xf, double _maxSpeed, double _angularVel, 
    double _accel, pfBox *_dbLimits)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDLimits -- null pfiXformerD*");
	return;
    }
    xf->setMotionLimits(_maxSpeed, _angularVel, _accel);
    xf->setDBLimits(_dbLimits);
}

PFUIDLLEXPORT void
pfiGetXformerDLimits(pfiXformerD *xf, double *_maxSpeed, double *_angularVel, 
    double *_accel, pfBox *_dbLimits)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDLimits -- null pfiXformerD*");
	return;
    }
    xf->getMotionLimits(_maxSpeed, _angularVel, _accel);
    xf->getDBLimits(_dbLimits);
}

PFUIDLLEXPORT void
pfiXformerDCollision(pfiXformerD *xf, int _mode, double _val, pfNode *_node)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDCollision -- null pfiXformerD*");
	return;
    }
    xf->setCollision(_mode, _val, _node);
}

PFUIDLLEXPORT void
pfiEnableXformerDCollision(pfiXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiEnableXformerDCollision -- null pfiXformerD*");
	return;
    }
    xf->enableCollision();
}

PFUIDLLEXPORT void
pfiDisableXformerDCollision(pfiXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiDisableXformerDCollision -- null pfiXformerD*");
	return;
    }
    xf->disableCollision();
}

PFUIDLLEXPORT int
pfiGetXformerDCollisionEnable(pfiXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDCollisionEnable -- null pfiXformerD*");
	return -1;
    }
    return xf->getCollisionEnable();
}


PFUIDLLEXPORT int
pfiGetXformerDCollisionStatus(pfiXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDCollisionStatus -- null pfiXformerD*");
	return -1;
    }
    return xf->getCollisionStatus();
}


PFUIDLLEXPORT void
pfiUpdateXformerD(pfiXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiUpdateXformerD -- null pfiXformerD*");
	return;
    }
    xf->update();
}

PFUIDLLEXPORT int
pfiCollideXformerD(pfiXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideXformerD -- null pfiXformerD*");
	return NULL;
    }
    return xf->collide();
}


/* TDF XformerD Routines */


PFUIDLLEXPORT pfType*
pfiGetTDFXformerDClassType(void)
{
    return pfiTDFXformerD::getClassType();    
}

PFUIDLLEXPORT pfiTDFXformerD *
pfiNewTDFXformerD(void *arena)
{
    return new(arena) pfiTDFXformerD;
}

PFUIDLLEXPORT pfiXformerD *
pfiCreateTDFXformerD(pfiInputXformDTrackball *_tb,
				pfiInputXformDDrive *_drive,
				pfiInputXformDFly *_fly,
		                pfiInputXformDSpheric *_spheric, void *arena)
{ 
    return new(arena) pfiTDFXformerD(_tb, _drive, _fly, _spheric);
}

PFUIDLLEXPORT void
pfiTDFXformerDFastClickTime(pfiTDFXformerD *xf, double _clickTime)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDHeight -- null pfiXformerD*");
	return;
    } 
    xf->setFastClickTime(_clickTime);
}

PFUIDLLEXPORT double
pfiGetTDFXformerDFastClickTime(pfiTDFXformerD *xf)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetTDFXformerDFastClickTime -- null pfiXformerD*");
	return -1;
    } 
    return xf->getFastClickTime();
}

PFUIDLLEXPORT void 
pfiTDFXformerDTrackball(pfiTDFXformerD *xf, pfiInputXformDTrackball *_tb)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDTrackball -- null pfiXformerD*");
	return;
    }
    xf->setTrackball(_tb);
}

PFUIDLLEXPORT pfiInputXformDTrackball  *
pfiGetTDFXformerDTrackball(pfiTDFXformerD *xf)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDTrackball -- null pfiXformerD*");
	return NULL;
    }
    return xf->getTrackball();
}

PFUIDLLEXPORT void 
pfiTDFXformerDDrive(pfiTDFXformerD *xf, pfiInputXformDDrive *_tb)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDTrackball -- null pfiXformerD*");
	return;
    }
    xf->setDrive(_tb);
}

PFUIDLLEXPORT pfiInputXformDDrive *
pfiGetTDFXformerDDrive(pfiTDFXformerD *xf)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDTrackball -- null pfiXformerD*");
	return NULL;
    }
    return xf->getDrive();
}

PFUIDLLEXPORT void pfiTDFXformerDFly(pfiTDFXformerD *xf, pfiInputXformDFly *_fly)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDFly -- null pfiXformerD*");
	return;
    }
    xf->setFly(_fly);
}

PFUIDLLEXPORT pfiInputXformDFly * 
pfiGetTDFXformerDFly(pfiTDFXformerD *xf)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerDFly -- null pfiXformerD*");
	return NULL;
    }
    return xf->getFly();
}

PFUIDLLEXPORT void pfiTDFXformerDSpheric(pfiTDFXformerD *xf, pfiInputXformDSpheric *_spheric)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiXformerDSpheric -- null pfiXformerD*");
	return;
    }
    xf->setSpheric(_spheric);
}

PFUIDLLEXPORT pfiInputXformDSpheric *
pfiGetTDFXformerDSpheric(pfiTDFXformerD *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiGetXformerDSpheric -- null pfiXformerD*");
	return NULL;
    }
    return xf->getSpheric();
}
