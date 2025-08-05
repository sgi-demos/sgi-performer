/*
 * WARNING! DO NOT EDIT!
 * cXformerD.C automatically generated from ../../lib/libpfui/cXformer.C
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
 * $Revision: 1.1 $
 * $Date: 2000/11/21 21:39:36 $
 */

#include <math.h>
#include <X11/X.h>

#include <Performer/pf.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>

#define _PFI_XFORMER_C_
#include <Performer/pfuiD.h>
#include "pfiXformerD.h"


pfType*
pfiGetXformerDClassType(void)
{
    return pfiXformerD::getClassType();    
}

pfiXformerD *
pfiNewXformerD(void *arena)
{
    return new(arena) pfiXformerD;
}

void 
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

void 
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

pfiInputXformD*  
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

int
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

void 
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

void 
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

void
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

void
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

void
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

void
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


void
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

void
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

void
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


void
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

void
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


void
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


void
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


void
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

void
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


void
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

void
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

pfNode *
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

void
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

void
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


void
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

void
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

void
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

void
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

void
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

int
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


int
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


void
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

int
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


pfType*
pfiGetTDFXformerDClassType(void)
{
    return pfiTDFXformerD::getClassType();    
}

pfiTDFXformerD *
pfiNewTDFXformerD(void *arena)
{
    return new(arena) pfiTDFXformerD;
}

pfiXformerD *
pfiCreateTDFXformerD(pfiInputXformDTrackball *_tb,
				pfiInputXformDDrive *_drive,
				pfiInputXformDFly *_fly,
		                pfiInputXformDSpheric *_spheric, void *arena)
{ 
    return new(arena) pfiTDFXformerD(_tb, _drive, _fly, _spheric);
}

void
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

double
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

void 
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

pfiInputXformDTrackball  *
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

void 
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

pfiInputXformDDrive *
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

void pfiTDFXformerDFly(pfiTDFXformerD *xf, pfiInputXformDFly *_fly)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerDFly -- null pfiXformerD*");
	return;
    }
    xf->setFly(_fly);
}

pfiInputXformDFly * 
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

void pfiTDFXformerDSpheric(pfiTDFXformerD *xf, pfiInputXformDSpheric *_spheric)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiXformerDSpheric -- null pfiXformerD*");
	return;
    }
    xf->setSpheric(_spheric);
}

pfiInputXformDSpheric *
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
